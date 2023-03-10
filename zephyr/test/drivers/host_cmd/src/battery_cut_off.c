/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "battery.h"
#include "emul/emul_common_i2c.h"
#include "emul/emul_smart_battery.h"
#include "hooks.h"
#include "host_command.h"
#include "test/drivers/test_state.h"
#include "test/drivers/utils.h"

#include <zephyr/drivers/emul.h>
#include <zephyr/ztest.h>

struct host_cmd_battery_cut_off_fixture {
	const struct emul *emul;
	struct i2c_common_emul_data *i2c_emul;
};

static void *host_cmd_battery_cut_off_setup(void)
{
	static struct host_cmd_battery_cut_off_fixture fixture = {
		.emul = EMUL_DT_GET(DT_NODELABEL(battery)),
	};

	fixture.i2c_emul = emul_smart_battery_get_i2c_common_data(fixture.emul);

	return &fixture;
}

static void host_cmd_battery_cut_off_before(void *f)
{
	ARG_UNUSED(f);
	test_set_battery_level(75);
}

static void host_cmd_battery_cut_off_after(void *f)
{
	struct host_cmd_battery_cut_off_fixture *fixture = f;

	i2c_common_emul_set_write_fail_reg(fixture->i2c_emul,
					   I2C_COMMON_EMUL_NO_FAIL_REG);
	set_ac_enabled(true);
	hook_notify(HOOK_AC_CHANGE);
	k_msleep(500);
}

ZTEST_SUITE(host_cmd_battery_cut_off, drivers_predicate_post_main,
	    host_cmd_battery_cut_off_setup, host_cmd_battery_cut_off_before,
	    host_cmd_battery_cut_off_after, NULL);

ZTEST_USER_F(host_cmd_battery_cut_off, test_fail_sb_write)
{
	int rv;

	/* Force a failure on the battery i2c write to 0x00 */
	i2c_common_emul_set_write_fail_reg(fixture->i2c_emul, 0);

	rv = ec_cmd_battery_cut_off(NULL);
	zassert_equal(EC_RES_ERROR, rv, "Expected 0, but got %d", rv);
}

ZTEST_USER(host_cmd_battery_cut_off, test_cutoff_battery)
{
	int rv;

	rv = ec_cmd_battery_cut_off(NULL);
	zassert_equal(EC_RES_SUCCESS, rv, "Expected 0, but got %d", rv);
	zassert_true(battery_is_cut_off(), NULL);
}

ZTEST_USER(host_cmd_battery_cut_off, test_cutoff_v1)
{
	int rv;
	struct ec_params_battery_cutoff params = {
		.flags = 0,
	};

	rv = ec_cmd_battery_cut_off_v1(NULL, &params);
	zassert_equal(EC_RES_SUCCESS, rv, "Expected 0, but got %d", rv);
	zassert_true(battery_is_cut_off(), NULL);
}

ZTEST_USER(host_cmd_battery_cut_off, test_cutoff_at_shutdown)
{
	int rv;
	struct ec_params_battery_cutoff params = {
		.flags = EC_BATTERY_CUTOFF_FLAG_AT_SHUTDOWN,
	};

	rv = ec_cmd_battery_cut_off_v1(NULL, &params);
	zassert_equal(EC_RES_SUCCESS, rv, "Expected 0, but got %d", rv);
	zassert_false(battery_is_cut_off(), NULL);
	hook_notify(HOOK_CHIPSET_SHUTDOWN);
	zassert_true(WAIT_FOR(battery_is_cut_off(), 1500000, k_msleep(250)),
		     NULL);
}
