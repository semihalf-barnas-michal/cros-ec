/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "console.h"
#include "ec_commands.h"
#include "test/drivers/stubs.h"
#include "test/drivers/test_state.h"
#include "test/drivers/utils.h"

#include <zephyr/fff.h>
#include <zephyr/shell/shell.h>
#include <zephyr/ztest.h>

#define TEST_PORT USBC_PORT_C0
#define BAD_PORT 82

ZTEST_USER(host_cmd_pd_control, test_bad_index)
{
	struct ec_params_pd_control params = { .chip = BAD_PORT,
					       .subcmd = PD_RESET };

	zassert_true(board_get_usb_pd_port_count() < BAD_PORT,
		     "Intended bad port exists");
	zassert_equal(ec_cmd_pd_control(NULL, &params), EC_RES_INVALID_PARAM,
		      "Failed to fail pd_control for port %d", params.chip);
}

ZTEST_USER(host_cmd_pd_control, test_unimplemented_command)
{
	struct ec_params_pd_control params = { .chip = TEST_PORT,
					       .subcmd = PD_CHIP_ON };

	zassert_equal(ec_cmd_pd_control(NULL, &params), EC_RES_INVALID_COMMAND,
		      "Failed to fail pd_control for port %d", params.chip);
}

ZTEST_USER(host_cmd_pd_control, test_pd_reset_resume)
{
	/*
	 * Note: this would ideally be a host command interface check, but
	 * the only HC return which would cover this is a state string, which
	 * could be brittle.
	 */
	zassert_true(pd_is_port_enabled(TEST_PORT), "Port not up at beginning");

	host_cmd_pd_control(TEST_PORT, PD_RESET);

	zassert_equal(1, board_reset_pd_mcu_fake.call_count,
		      "Failed to see board reset");

	/* Give some PD task processing time */
	k_sleep(K_SECONDS(1));

	zassert_false(pd_is_port_enabled(TEST_PORT), "Port failed to suspend");

	host_cmd_pd_control(TEST_PORT, PD_RESUME);

	/* Give some PD task processing time */
	k_sleep(K_SECONDS(1));

	zassert_true(pd_is_port_enabled(TEST_PORT), "Port failed to resume");

	RESET_FAKE(board_reset_pd_mcu);
}

ZTEST_USER(host_cmd_pd_control, test_suspend_resume)
{
	/*
	 * Note: this would ideally be a host command interface check, but
	 * the only HC return which would cover this is a state string, which
	 * could be brittle.
	 */
	zassert_true(pd_is_port_enabled(TEST_PORT), "Port not up at beginning");

	host_cmd_pd_control(TEST_PORT, PD_SUSPEND);

	/* Give some PD task processing time */
	k_sleep(K_SECONDS(1));

	zassert_false(pd_is_port_enabled(TEST_PORT), "Port failed to suspend");

	host_cmd_pd_control(TEST_PORT, PD_RESUME);

	/* Give some PD task processing time */
	k_sleep(K_SECONDS(1));

	zassert_true(pd_is_port_enabled(TEST_PORT), "Port failed to resume");
}

ZTEST_USER(host_cmd_pd_control, test_suspend_low_battery)
{
	struct ec_params_pd_control params = { .chip = TEST_PORT,
					       .subcmd = PD_SUSPEND };

	/* Suspending the port for firmware update should fail at critical low
	 * battery.
	 */
	test_set_battery_level(1);
	zassert_equal(ec_cmd_pd_control(NULL, &params), EC_RES_BUSY);
}

ZTEST_USER(host_cmd_pd_control, test_control_disable)
{
	struct ec_params_pd_control params = { .chip = TEST_PORT,
					       .subcmd = PD_RESET };

	host_cmd_pd_control(TEST_PORT, PD_CONTROL_DISABLE);

	zassert_equal(ec_cmd_pd_control(NULL, &params), EC_RES_ACCESS_DENIED,
		      "Access was not denied for port %d", params.chip);

	/*
	 * Disable lasts as long as the EC is booted.  Use a test hook to
	 * restore our state to a normal one
	 */
	pd_control_port_enable(TEST_PORT);
}

static void host_cmd_pd_control_begin(void *data)
{
	ARG_UNUSED(data);

	/* Assume we have at least one USB-C port */
	zassert_true(board_get_usb_pd_port_count() > 0,
		     "Insufficient TCPCs found");

	/* Set the system into S0, since the AP would drive these commands */
	test_set_chipset_to_s0();
	k_sleep(K_SECONDS(1));
}

ZTEST_SUITE(host_cmd_pd_control, drivers_predicate_post_main, NULL,
	    host_cmd_pd_control_begin, NULL, NULL);
