/* -*- mode:c -*-
 *
 * Copyright 2016 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Declare symbolic names for all the GPIOs that we care about.
 * Note: Those with interrupt handlers must be declared first. */

/* Outputs */
GPIO(PD11,			PIN(D, 11), GPIO_OUT_HIGH)

GPIO(I2C1_SCL,			PIN(B, 8), GPIO_INPUT)
GPIO(I2C1_SDA,			PIN(B, 9), GPIO_INPUT)
GPIO(FMPI2C_SCL,		PIN(C, 6), GPIO_INPUT)
GPIO(FMPI2C_SDA,		PIN(C, 7), GPIO_INPUT)

/* USART3 TX/RX */
GPIO(MCU_UART1_TX,		PIN(A, 9), GPIO_INPUT)
GPIO(MCU_UART1_RX,		PIN(A, 10), GPIO_INPUT)
GPIO(MCU_UART3_TX,		PIN(C, 10), GPIO_INPUT)
GPIO(MCU_UART3_RX,		PIN(C, 11), GPIO_INPUT)

GPIO(USB_FS_DM,			PIN(A, 11), GPIO_INPUT)
GPIO(USB_FS_DP,			PIN(A, 12), GPIO_INPUT)


GPIO(USB_HS_ULPI_NXT,		PIN(C, 3), GPIO_INPUT)
GPIO(USB_HS_ULPI_DIR,		PIN(C, 2), GPIO_INPUT)
GPIO(USB_HS_ULPI_STP,		PIN(C, 0), GPIO_INPUT)
GPIO(USB_HS_ULPI_CK,		PIN(A, 5), GPIO_INPUT)

GPIO(USB_HS_ULPI_D7,		PIN(B, 5), GPIO_INPUT)
GPIO(USB_HS_ULPI_D6,		PIN(B,13), GPIO_INPUT)
GPIO(USB_HS_ULPI_D5,		PIN(B,12), GPIO_INPUT)
GPIO(USB_HS_ULPI_D4,		PIN(B, 2), GPIO_INPUT)
GPIO(USB_HS_ULPI_D3,		PIN(B,10), GPIO_INPUT)
GPIO(USB_HS_ULPI_D2,		PIN(B, 1), GPIO_INPUT)
GPIO(USB_HS_ULPI_D1,		PIN(B, 0), GPIO_INPUT)
GPIO(USB_HS_ULPI_D0,		PIN(A, 3), GPIO_INPUT)



/* Unimplemented signals since this is a dev board */
UNIMPLEMENTED(ENTERING_RW)
UNIMPLEMENTED(WP_L)

ALTERNATE(PIN_MASK(A, 0x0600), 7, MODULE_UART, 0) /* USART1: PA9/PA10 - Console */
ALTERNATE(PIN_MASK(C, 0x0C00), 7, MODULE_USART, 0) /* USART3: PC10/PC11 - NOT Console */
ALTERNATE(PIN_MASK(A, 0x0100), 0, MODULE_MCO, 0) /* MCO1: PA8 */
ALTERNATE(PIN_MASK(C, 0x0200), 0, MODULE_MCO, 0) /* MCO2: PC9 */

ALTERNATE(PIN_MASK(B, 0x0300), 4, MODULE_I2C, GPIO_ODR_HIGH | GPIO_PULL_UP) /* I2C1: PB8-9 */
ALTERNATE(PIN_MASK(D, 0x3000), 4, MODULE_I2C, GPIO_ODR_HIGH | GPIO_PULL_UP) /* FMPI2C MASTER:PD12/13 */

ALTERNATE(PIN_MASK(A, 0x1800), 10, MODULE_USB, 0) /* DWC USB OTG: PA11/12 */

/* OTG HS */
ALTERNATE(PIN_MASK(A, 0x0028), 10, MODULE_USB, 0) /* DWC USB OTG HS */
ALTERNATE(PIN_MASK(B, 0x3427), 10, MODULE_USB, 0) /* DWC USB OTG HS */
ALTERNATE(PIN_MASK(C, 0x000d), 10, MODULE_USB, 0) /* DWC USB OTG HS */
