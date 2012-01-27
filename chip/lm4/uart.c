/* Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* UART module for Chrome EC */

#include <stdarg.h>

#include "board.h"
#include "console.h"
#include "gpio.h"
#include "lpc.h"
#include "registers.h"
#include "task.h"
#include "uart.h"
#include "util.h"

/* Baud rate for UARTs */
#define BAUD_RATE 115200

void uart_tx_start(void)
{
	/* Re-enable the transmit interrupt, then forcibly trigger the
	 * interrupt.  This works around a hardware problem with the
	 * UART where the FIFO only triggers the interrupt when its
	 * threshold is _crossed_, not just met. */
	LM4_UART_IM(0) |= 0x20;
	task_trigger_irq(LM4_IRQ_UART0);
}

void uart_tx_stop(void)
{
	LM4_UART_IM(0) &= ~0x20;
}

int uart_tx_stopped(void)
{
	return !(LM4_UART_IM(0) & 0x20);
}

void uart_tx_flush(void)
{
	/* Wait for transmit FIFO empty */
	while (!(LM4_UART_FR(0) & 0x80))
		;
}

int uart_tx_ready(void)
{
	return !(LM4_UART_FR(0) & 0x20);
}

int uart_rx_available(void)
{
	return !(LM4_UART_FR(0) & 0x10);
}

void uart_write_char(char c)
{
	LM4_UART_DR(0) = c;
}

int uart_read_char(void)
{
	return LM4_UART_DR(0);
}

void uart_disable_interrupt(void)
{
	task_disable_irq(LM4_IRQ_UART0);
}

void uart_enable_interrupt(void)
{
	task_enable_irq(LM4_IRQ_UART0);
}

/* Interrupt handler for UART0 */
static void uart_0_interrupt(void)
{
	/* Clear transmit and receive interrupt status */
	LM4_UART_ICR(0) = 0x70;


	/* Read input FIFO until empty, then fill output FIFO */
	uart_process();
}
DECLARE_IRQ(LM4_IRQ_UART0, uart_0_interrupt, 1);


/* Interrupt handler for UART1 */
static void uart_1_interrupt(void)
{
	/* Clear transmit and receive interrupt status */
	LM4_UART_ICR(1) = 0x70;

	/* TODO: (crosbug.com/p/7488) handle input */

	/* If we have space in our FIFO and a character is pending in LPC,
	 * handle that character. */
	if (!(LM4_UART_FR(1) & 0x20) && lpc_comx_has_char()) {
		/* Copy the next byte then disable transmit interrupt */
		LM4_UART_DR(1) = lpc_comx_get_char();
		LM4_UART_IM(1) &= ~0x20;
	}
}
/* Must be same prio as LPC interrupt handler so they don't preempt */
DECLARE_IRQ(LM4_IRQ_UART1, uart_1_interrupt, 2);


/* Configure GPIOs for the UART module. */
static void configure_gpio(void)
{
#ifdef BOARD_link
	/* UART0 RX and TX are GPIO PA0:1 alternate function 1 */
	gpio_set_alternate_function(LM4_GPIO_A, 0x03, 1);
	/* UART1 RX and TX are GPIO PC4:5 alternate function 2 */
	gpio_set_alternate_function(LM4_GPIO_C, 0x30, 2);
#else
	/* UART0 RX and TX are GPIO PA0:1 alternate function 1 */
	gpio_set_alternate_function(LM4_GPIO_A, 0x03, 1);
	/* UART1 RX and TX are GPIO PB0:1 alternate function 1*/
	gpio_set_alternate_function(LM4_GPIO_B, 0x03, 1);
#endif
}


int uart_init(void)
{
	volatile uint32_t scratch  __attribute__((unused));
	int ch;

	/*
	 * Check that the UART parameters used for panic/watchdog are matching
	 * the UART0 parameters.
	*/
	BUILD_ASSERT(LM4_UART_CH0_BASE == CONFIG_UART_ADDRESS);

	/* Enable UART0 and UART1 and delay a few clocks */
	LM4_SYSTEM_RCGCUART |= 0x03;
	scratch = LM4_SYSTEM_RCGCUART;

	/* Configure GPIOs */
	configure_gpio();

	/* Configure UART0 and UART1 (identically) */
	for (ch = 0; ch < 2; ch++) {
		/* Disable the port */
		LM4_UART_CTL(ch) = 0x0300;
		/* Set the baud rate divisor */
		LM4_UART_IBRD(ch) = (CPU_CLOCK / 16) / BAUD_RATE;
		LM4_UART_FBRD(ch) =
			(((CPU_CLOCK / 16) % BAUD_RATE) * 64 + BAUD_RATE / 2) /
			BAUD_RATE;
		/* 8-N-1, FIFO enabled.  Must be done after setting
		 * the divisor for the new divisor to take effect. */
		LM4_UART_LCRH(ch) = 0x70;
		/* Interrupt when RX fifo at minimum (>= 1/8 full), and TX fifo
		 * when <= 1/4 full */
		LM4_UART_IFLS(ch) = 0x01;
		/* Unmask receive-FIFO, receive-timeout.  We need
		 * receive-timeout because the minimum RX FIFO depth is 1/8 = 2
		 * bytes; without the receive-timeout we'd never be notified
		 * about single received characters. */
		LM4_UART_IM(ch) = 0x50;
		/* Enable the port */
		LM4_UART_CTL(ch) |= 0x0001;
	}

	/* Enable interrupts */
	task_enable_irq(LM4_IRQ_UART0);
	task_enable_irq(LM4_IRQ_UART1);

	/* Print hello on UART1 for debugging */
	/* TODO: remove in production */
	{
		const char *c = "Hello on UART1\r\n";
		while (*c)
			uart_comx_putc(*c++);
	}

	return EC_SUCCESS;
}


/*****************************************************************************/
/* COMx functions */


int uart_comx_putc_ok(void)
{
	if (LM4_UART_FR(1) & 0x20) {
		/* FIFO is full, so enable transmit interrupt to let us know
		 * when it empties. */
		LM4_UART_IM(1) |= 0x20;
		return 0;
	} else {
		return 1;
	}
}


void uart_comx_putc(int c)
{
	LM4_UART_DR(1) = c;
}
