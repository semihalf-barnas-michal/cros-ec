/* Copyright 2022 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Kingler board-specific USB-C configuration */

#include "charger.h"
#include "console.h"
#include "driver/bc12/pi3usb9201_public.h"
#include "driver/charger/isl923x_public.h"
#include "driver/ppc/nx20p348x.h"
#include "driver/ppc/rt1718s.h"
#include "driver/tcpm/anx7447.h"
#include "driver/tcpm/rt1718s.h"
#include "driver/usb_mux/ps8743.h"
#include "gpio/gpio_int.h"
#include "hooks.h"
#include "timer.h"
#include "usb_charge.h"
#include "usb_mux.h"
#include "usb_pd_tcpm.h"
#include "usbc_ppc.h"

#include "baseboard_usbc_config.h"
#include "variant_db_detection.h"

#define CPRINTS(format, args...) cprints(CC_USBPD, format, ## args)
#define CPRINTF(format, args...) cprintf(CC_USBPD, format, ## args)

struct tcpc_config_t tcpc_config[CONFIG_USB_PD_PORT_MAX_COUNT] = {
	[USBC_PORT_C0] = {
		.bus_type = EC_BUS_TYPE_I2C,
		.i2c_info = {
			.port = I2C_PORT_USB_C0,
			.addr_flags = AN7447_TCPC0_I2C_ADDR_FLAGS,
		},
		.drv = &anx7447_tcpm_drv,
		/* Alert is active-low, push-pull */
		.flags = 0,
	},
	[USBC_PORT_C1] = {
		.bus_type = EC_BUS_TYPE_I2C,
		.i2c_info = {
			.port = I2C_PORT_USB_C1,
			.addr_flags = RT1718S_I2C_ADDR2_FLAGS,
		},
		.drv = &rt1718s_tcpm_drv,
	}
};


struct ppc_config_t ppc_chips[CONFIG_USB_PD_PORT_MAX_COUNT] = {
	[USBC_PORT_C0] = {
		.i2c_port = I2C_PORT_USB_C0,
		.i2c_addr_flags = NX20P3483_ADDR2_FLAGS,
		.drv = &nx20p348x_drv
	},
	[USBC_PORT_C1] = {
		.i2c_port = I2C_PORT_USB_C1,
		.i2c_addr_flags = NX20P3483_ADDR2_FLAGS,
		.drv = &nx20p348x_drv
	}
};
unsigned int ppc_cnt = ARRAY_SIZE(ppc_chips);

const struct charger_config_t chg_chips[] = {
	{
		.i2c_port = I2C_PORT_POWER,
		.i2c_addr_flags = ISL923X_ADDR_FLAGS,
		.drv = &isl923x_drv,
	}
};

/* USB Mux */

/* USB Mux C1 : board_init of PS8743 */
static int ps8743_tune_mux(const struct usb_mux *me)
{
	ps8743_tune_usb_eq(me,
			PS8743_USB_EQ_TX_3_6_DB,
			PS8743_USB_EQ_RX_16_0_DB);

	return EC_SUCCESS;
}

struct usb_mux usb_muxes[CONFIG_USB_PD_PORT_MAX_COUNT] = {
	[USBC_PORT_C0] = {
		.usb_port = USBC_PORT_C0,
		.driver = &anx7447_usb_mux_driver,
		.hpd_update = &anx7447_tcpc_update_hpd_status,
	},
	[USBC_PORT_C1] = {
		.usb_port = USBC_PORT_C1,
		.i2c_port = I2C_PORT_USB_C1,
		.i2c_addr_flags = PS8743_I2C_ADDR0_FLAG,
		.driver = &ps8743_usb_mux_driver,
		.board_init = &ps8743_tune_mux,
	},
};

struct bc12_config bc12_ports[CONFIG_USB_PD_PORT_MAX_COUNT] = {
	[USBC_PORT_C0] = {
		.drv = &pi3usb9201_drv,
	},
	[USBC_PORT_C1] = {
		.drv = &rt1718s_bc12_drv,
	}
};

const struct pi3usb9201_config_t
		pi3usb9201_bc12_chips[CONFIG_USB_PD_PORT_MAX_COUNT] = {
	[USBC_PORT_C0] = {
		.i2c_port = I2C_PORT_USB_C0,
		.i2c_addr_flags = PI3USB9201_I2C_ADDR_3_FLAGS,
	},
	[USBC_PORT_C1] = { /* unused */ }
};

void board_tcpc_init(void)
{
	/* Only reset TCPC if not sysjump */
	if (!system_jumped_late()) {
		/* TODO(crosbug.com/p/61098): How long do we need to wait? */
		board_reset_pd_mcu();
	}

	/* Enable PPC interrupts */
	/*
	 * The original code says GPIO_USB_C0_TCPC_INT_ODL but
	 * the comments say PPC interrupt, so perhaps
	 * this should be int_usb_c0_ppc.
	 * Since int_usb_c0_tcpc is enabled further down, this
	 * does look like a typo.
	 */
	gpio_enable_dt_interrupt(GPIO_INT_FROM_NODELABEL(int_usb_c0_tcpc));
	if (corsola_get_db_type() == CORSOLA_DB_TYPEC)
		gpio_enable_dt_interrupt(
			GPIO_INT_FROM_NODELABEL(int_x_ec_gpio2));

	/* Enable TCPC interrupts */
	gpio_enable_dt_interrupt(GPIO_INT_FROM_NODELABEL(int_usb_c0_tcpc));
	if (corsola_get_db_type() == CORSOLA_DB_TYPEC)
		gpio_enable_dt_interrupt(
			GPIO_INT_FROM_NODELABEL(int_usb_c1_tcpc));

	/* Enable BC1.2 interrupts. */
	gpio_enable_dt_interrupt(GPIO_INT_FROM_NODELABEL(int_usb_c0_bc12));

	/*
	 * Initialize HPD to low; after sysjump SOC needs to see
	 * HPD pulse to enable video path
	 */
	for (int port = 0; port < CONFIG_USB_PD_PORT_MAX_COUNT; ++port)
		usb_mux_hpd_update(port, USB_PD_MUX_HPD_LVL_DEASSERTED |
					 USB_PD_MUX_HPD_IRQ_DEASSERTED);
}
DECLARE_HOOK(HOOK_INIT, board_tcpc_init, HOOK_PRIO_INIT_I2C + 1);

__override int board_rt1718s_init(int port)
{
	static bool gpio_initialized;

	if (!system_jumped_late() && !gpio_initialized) {
		/* set GPIO 1~3 as push pull, as output, output low. */
		rt1718s_gpio_set_flags(port, RT1718S_GPIO1, GPIO_OUT_LOW);
		rt1718s_gpio_set_flags(port, RT1718S_GPIO2, GPIO_OUT_LOW);
		rt1718s_gpio_set_flags(port, RT1718S_GPIO3, GPIO_OUT_LOW);
		gpio_initialized = true;
	}

	/* gpio 1/2 output high when receiving frs signal */
	RETURN_ERROR(rt1718s_update_bits8(port, RT1718S_GPIO1_VBUS_CTRL,
			RT1718S_GPIO1_VBUS_CTRL_FRS_RX_VBUS, 0xFF));
	RETURN_ERROR(rt1718s_update_bits8(port, RT1718S_GPIO2_VBUS_CTRL,
			RT1718S_GPIO2_VBUS_CTRL_FRS_RX_VBUS, 0xFF));

	/* Turn on SBU switch */
	RETURN_ERROR(rt1718s_update_bits8(port, RT1718S_RT2_SBU_CTRL_01,
				RT1718S_RT2_SBU_CTRL_01_SBU_VIEN |
				RT1718S_RT2_SBU_CTRL_01_SBU2_SWEN |
				RT1718S_RT2_SBU_CTRL_01_SBU1_SWEN,
				0xFF));
	/* Trigger GPIO 1/2 change when FRS signal received */
	RETURN_ERROR(rt1718s_update_bits8(port, RT1718S_FRS_CTRL3,
			RT1718S_FRS_CTRL3_FRS_RX_WAIT_GPIO2 |
			RT1718S_FRS_CTRL3_FRS_RX_WAIT_GPIO1,
			RT1718S_FRS_CTRL3_FRS_RX_WAIT_GPIO2 |
			RT1718S_FRS_CTRL3_FRS_RX_WAIT_GPIO1));
	/* Set FRS signal detect time to 46.875us */
	RETURN_ERROR(rt1718s_update_bits8(port, RT1718S_FRS_CTRL1,
			RT1718S_FRS_CTRL1_FRSWAPRX_MASK,
			0xFF));

	return EC_SUCCESS;
}

void board_reset_pd_mcu(void)
{

	/* reset C0 ANX3447 */
	/* Assert reset */
	gpio_set_level(GPIO_USB_C0_TCPC_RST, 1);
	msleep(1);
	gpio_set_level(GPIO_USB_C0_TCPC_RST, 0);
	/* After TEST_R release, anx7447/3447 needs 2ms to finish eFuse
	 * loading.
	 */
	msleep(2);

	/* reset C1 RT1718s */
	rt1718s_sw_reset(USBC_PORT_C1);
}

/* Used by Vbus discharge common code with CONFIG_USB_PD_DISCHARGE */
int board_vbus_source_enabled(int port)
{
	return tcpm_get_src_ctrl(port);
}

/* Used by USB charger task with CONFIG_USB_PD_5V_EN_CUSTOM */
int board_is_sourcing_vbus(int port)
{
	return board_vbus_source_enabled(port);
}

__override int board_rt1718s_set_snk_enable(int port, int enable)
{
	if (port == USBC_PORT_C1)
		rt1718s_gpio_set_level(port, GPIO_EN_USB_C1_SINK, enable);

	return EC_SUCCESS;
}

int board_set_active_charge_port(int port)
{
	int i;
	bool is_valid_port =
		(port >= 0 && port < board_get_usb_pd_port_count());

	if (!is_valid_port && port != CHARGE_PORT_NONE)
		return EC_ERROR_INVAL;

	if (port == CHARGE_PORT_NONE) {
		CPRINTS("Disabling all charger ports");

		/* Disable all ports. */
		for (i = 0; i < board_get_usb_pd_port_count(); i++) {
			/*
			 * Do not return early if one fails otherwise we can
			 * get into a boot loop assertion failure.
			 */
			if (ppc_vbus_sink_enable(i, 0))
				CPRINTS("Disabling C%d as sink failed.", i);
		}

		return EC_SUCCESS;
	}

	/* Check if the port is sourcing VBUS. */
	if (ppc_is_sourcing_vbus(port)) {
		CPRINTS("Skip enable C%d", port);
		return EC_ERROR_INVAL;
	}

	CPRINTS("New charge port: C%d", port);

	/*
	 * Turn off the other ports' sink path FETs, before enabling the
	 * requested charge port.
	 */
	for (i = 0; i < board_get_usb_pd_port_count(); i++) {
		if (i == port)
			continue;

		if (ppc_vbus_sink_enable(i, 0))
			CPRINTS("C%d: sink path disable failed.", i);
	}

	/* Enable requested charge port. */
	if (ppc_vbus_sink_enable(port, 1)) {
		CPRINTS("C%d: sink path enable failed.", port);
		return EC_ERROR_UNKNOWN;
	}

	return EC_SUCCESS;
}

uint16_t tcpc_get_alert_status(void)
{
	uint16_t status = 0;

	if (!gpio_get_level(GPIO_USB_C0_TCPC_INT_ODL)) {
		if (!gpio_get_level(GPIO_USB_C0_TCPC_RST))
			status |= PD_STATUS_TCPC_ALERT_0;
	}

	if (!gpio_get_level(GPIO_USB_C1_TCPC_INT_ODL))
		return status |= PD_STATUS_TCPC_ALERT_1;
	return status;
}

void tcpc_alert_event(enum gpio_signal signal)
{
	int port;

	switch (signal) {
	case GPIO_USB_C0_TCPC_INT_ODL:
		port = 0;
		break;
	case GPIO_USB_C1_TCPC_INT_ODL:
		port = 1;
		break;
	default:
		return;
	}

	schedule_deferred_pd_interrupt(port);
}

void ppc_interrupt(enum gpio_signal signal)
{
	switch (signal) {
	case GPIO_USB_C0_PPC_INT_ODL:
		ppc_chips[0].drv->interrupt(0);
		break;
	case GPIO_USB_C1_PPC_INT_ODL:
		ppc_chips[1].drv->interrupt(1);
		break;
	default:
		break;
	}
}

void bc12_interrupt(enum gpio_signal signal)
{
	task_set_event(TASK_ID_USB_CHG_P0, USB_CHG_EVENT_BC12);
}
