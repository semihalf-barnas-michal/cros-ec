/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ec_commands.h"
#include "host_command.h"
#include "test/drivers/test_state.h"
#include "test/drivers/utils.h"

#include <zephyr/drivers/emul.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/shell/shell_dummy.h>
#include <zephyr/ztest.h>

#include <console.h>
#include <lid_switch.h>

#define LID_GPIO_PATH NAMED_GPIOS_GPIO_NODE(lid_open_ec)
#define LID_GPIO_PIN DT_GPIO_PIN(LID_GPIO_PATH, gpios)

int emul_lid_open(void)
{
	const struct device *lid_gpio_dev =
		DEVICE_DT_GET(DT_GPIO_CTLR(LID_GPIO_PATH, gpios));

	return gpio_emul_input_set(lid_gpio_dev, LID_GPIO_PIN, 1);
}

int emul_lid_close(void)
{
	const struct device *lid_gpio_dev =
		DEVICE_DT_GET(DT_GPIO_CTLR(LID_GPIO_PATH, gpios));

	return gpio_emul_input_set(lid_gpio_dev, LID_GPIO_PIN, 0);
}

static void *lid_switch_setup(void)
{
	/**
	 * Set chipset to S0 as chipset power on after opening lid may disturb
	 * test
	 */
	test_set_chipset_to_s0();

	return NULL;
}

static void lid_switch_before(void *unused)
{
	/* Make sure that interrupt fire at the next lid open/close */
	zassert_ok(emul_lid_close());
	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(100));
}

static void lid_switch_after(void *unused)
{
	struct ec_params_force_lid_open params = {
		.enabled = 0,
	};
	int res;

	res = ec_cmd_force_lid_open(NULL, &params);
	if (res)
		TC_ERROR("host_command_process() failed (%d)\n", res);

	res = emul_lid_open();
	if (res)
		TC_ERROR("emul_lid_open() failed (%d)\n", res);
	k_sleep(K_MSEC(100));
}

ZTEST(lid_switch, test_lid_open)
{
	/* Start closed. */
	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 0);

	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 1);
}

ZTEST(lid_switch, test_lid_debounce)
{
	/* Start closed. */
	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 0);

	/* Create interrupts quickly before they can be handled. */
	zassert_ok(emul_lid_open());
	zassert_ok(emul_lid_close());
	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 1);
}

ZTEST(lid_switch, test_lid_close)
{
	/* Start open. */
	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(100));

	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(200));
	zassert_equal(lid_is_open(), 0);
}

ZTEST(lid_switch, test_enable_lid_detect)
{
	/* Start open. */
	zassert_ok(emul_lid_open(), NULL);
	k_sleep(K_MSEC(500));
	zassert_equal(lid_is_open(), 1, NULL);

	/* Disable lid detect interrupts */
	enable_lid_detect(false);
	k_sleep(K_MSEC(100));

	/* Close lid but check if still indicates open as interrupt is
	 * disabled
	 */
	zassert_ok(emul_lid_close(), NULL);
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 1, NULL);
	zassert_ok(emul_lid_open(), NULL);
	k_sleep(K_MSEC(100));

	/* Restore lid detect interrupts, confirm interrupt is firing again */
	enable_lid_detect(true);
	zassert_ok(emul_lid_close(), NULL);
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 0, NULL);
}

ZTEST(lid_switch, test_cmd_lidopen)
{
	/* Start closed. */
	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 0);

	/* Forced override lid open. */
	zassert_equal(EC_SUCCESS, shell_execute_cmd(get_ec_shell(), "lidopen"),
		      NULL);
	zassert_equal(lid_is_open(), 1);
	k_sleep(K_MSEC(100));

	printk("GPIO lid open/close\n");
	/* Open & close with gpio. */
	zassert_ok(emul_lid_open());
	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(500));
	zassert_equal(lid_is_open(), 0);
}

ZTEST(lid_switch, test_cmd_lidopen_bounce)
{
	/* Start closed. */
	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 0);

	printk("Console lid open\n");
	/* Forced override lid open. */
	zassert_equal(EC_SUCCESS, shell_execute_cmd(get_ec_shell(), "lidopen"),
		      NULL);
	zassert_equal(lid_is_open(), 1);
	k_sleep(K_MSEC(100));

	printk("Console lid open\n");
	/* Forced override lid open. */
	zassert_equal(EC_SUCCESS, shell_execute_cmd(get_ec_shell(), "lidopen"),
		      NULL);
	zassert_equal(lid_is_open(), 1);
	k_sleep(K_MSEC(100));

	printk("GPIO lid open/close\n");
	/* Open & close with gpio. */
	zassert_ok(emul_lid_open());
	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(500));
	zassert_equal(lid_is_open(), 0);
}

ZTEST(lid_switch, test_cmd_lidclose)
{
	/* Start open. */
	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 1);

	/* Forced override lid close. */
	zassert_equal(EC_SUCCESS, shell_execute_cmd(get_ec_shell(), "lidclose"),
		      NULL);
	zassert_equal(lid_is_open(), 0);
	k_sleep(K_MSEC(100));

	printk("GPIO lid close/open\n");
	/* Close & open with gpio. */
	zassert_ok(emul_lid_close());
	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(500));
	zassert_equal(lid_is_open(), 1);
}

ZTEST(lid_switch, test_cmd_lidclose_bounce)
{
	/* Start open. */
	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 1);

	/* Forced override lid close. */
	zassert_equal(EC_SUCCESS, shell_execute_cmd(get_ec_shell(), "lidclose"),
		      NULL);
	zassert_equal(lid_is_open(), 0);
	k_sleep(K_MSEC(100));

	/* Forced override lid close. */
	zassert_equal(EC_SUCCESS, shell_execute_cmd(get_ec_shell(), "lidclose"),
		      NULL);
	zassert_equal(lid_is_open(), 0);
	k_sleep(K_MSEC(100));

	printk("GPIO lid close/open\n");
	/* Close & open with gpio. */
	zassert_ok(emul_lid_close());
	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(500));
	zassert_equal(lid_is_open(), 1);
}

#if defined(CONFIG_SHELL_BACKEND_DUMMY)
ZTEST(lid_switch, test_cmd_lidstate_open)
{
	const char *buffer;
	size_t buffer_size;

	/* Start open. */
	zassert_ok(emul_lid_open());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 1);

	/* Read the state with console. */
	shell_backend_dummy_clear_output(get_ec_shell());
	zassert_equal(EC_SUCCESS, shell_execute_cmd(get_ec_shell(), "lidstate"),
		      NULL);
	buffer = shell_backend_dummy_get_output(get_ec_shell(), &buffer_size);
	zassert_true(strcmp(buffer, "\r\nlid state: open\r\n") == 0,
		     "Invalid console output %s", buffer);
}

ZTEST(lid_switch, test_cmd_lidstate_close)
{
	const char *buffer;
	size_t buffer_size;

	/* Start closed. */
	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 0);

	/* Read the state with console. */
	shell_backend_dummy_clear_output(get_ec_shell());
	zassert_equal(EC_SUCCESS, shell_execute_cmd(get_ec_shell(), "lidstate"),
		      NULL);
	buffer = shell_backend_dummy_get_output(get_ec_shell(), &buffer_size);
	zassert_true(strcmp(buffer, "\r\nlid state: closed\r\n") == 0,
		     "Invalid console output %s", buffer);
}
#else
#error This test requires CONFIG_SHELL_BACKEND_DUMMY
#endif

ZTEST(lid_switch, test_hc_force_lid_open)
{
	struct ec_params_force_lid_open params = {
		.enabled = 1,
	};

	/* Start closed. */
	zassert_ok(emul_lid_close());
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 0);

	zassert_ok(ec_cmd_force_lid_open(NULL, &params));
	k_sleep(K_MSEC(100));
	zassert_equal(lid_is_open(), 1);
}

ZTEST_SUITE(lid_switch, drivers_predicate_post_main, lid_switch_setup,
	    lid_switch_before, lid_switch_after, NULL);
