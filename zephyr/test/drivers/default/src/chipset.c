/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "chipset.h"
#include "console.h"
#include "test/drivers/test_state.h"

#include <stdint.h>

#include <zephyr/fff.h>
#include <zephyr/shell/shell.h>
#include <zephyr/ztest.h>

#include <ap_power/ap_power.h>
#include <ap_power/ap_power_events.h>

ZTEST(chipset, test_get_ap_reset_stats__bad_pointers)
{
	zassert_equal(EC_ERROR_INVAL, get_ap_reset_stats(NULL, 0, NULL));
}

ZTEST(chipset, test_get_ap_reset_stats__happy_path)
{
	struct ap_reset_log_entry reset_log_entries[4];
	uint32_t actual_reset_count, reset_count;

	memset(reset_log_entries, 0, sizeof(reset_log_entries));

	/* Report two AP resets */
	report_ap_reset(CHIPSET_RESET_AP_WATCHDOG);
	report_ap_reset(CHIPSET_RESET_HANG_REBOOT);

	zassert_equal(EC_SUCCESS,
		      get_ap_reset_stats(reset_log_entries,
					 ARRAY_SIZE(reset_log_entries),
					 &reset_count));

	/* Check the reset causes. The reset entry log is not a FIFO, so we get
	 * the last two empty slots followed by the two we triggered above.
	 */
	zassert_equal(0, reset_log_entries[0].reset_cause);
	zassert_equal(0, reset_log_entries[1].reset_cause);
	zassert_equal(CHIPSET_RESET_AP_WATCHDOG,
		      reset_log_entries[2].reset_cause);
	zassert_equal(CHIPSET_RESET_HANG_REBOOT,
		      reset_log_entries[3].reset_cause);

	/* Check reset count */
	actual_reset_count = test_chipset_get_ap_resets_since_ec_boot();
	zassert_equal(actual_reset_count, reset_count,
		      "Found %d resets, expected %d", reset_count,
		      actual_reset_count);
}

ZTEST(chipset, test_console_cmd_apreset)
{
	struct ap_reset_log_entry reset_log_entries[4];
	uint32_t reset_count;

	zassert_ok(shell_execute_cmd(get_ec_shell(), "apreset"));

	/* Make sure an AP reset happened. The expected reset log entry is at
	 * index 3 because we read out 3 empty slots first.
	 */
	zassert_ok(get_ap_reset_stats(reset_log_entries,
				      ARRAY_SIZE(reset_log_entries),
				      &reset_count));

	zassert_equal(CHIPSET_RESET_CONSOLE_CMD,
		      reset_log_entries[3].reset_cause);
}

ZTEST(chipset, test_console_cmd_apshutdown)
{
	struct ap_reset_log_entry reset_log_entries[4];
	uint32_t reset_count;

	zassert_ok(shell_execute_cmd(get_ec_shell(), "apshutdown"));

	/* Make sure an AP reset happened. The expected reset log entry is at
	 * index 3 because we read out 3 empty slots first.
	 */
	zassert_ok(get_ap_reset_stats(reset_log_entries,
				      ARRAY_SIZE(reset_log_entries),
				      &reset_count));

	zassert_equal(CHIPSET_SHUTDOWN_CONSOLE_CMD,
		      reset_log_entries[3].reset_cause);
}

static void reset(void *arg)
{
	ARG_UNUSED(arg);

	test_chipset_corrupt_reset_log_checksum();
	init_reset_log();
}

ZTEST_SUITE(chipset, drivers_predicate_post_main, NULL, reset, reset, NULL);
