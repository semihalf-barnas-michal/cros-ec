/* Copyright 2023 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "cbi.h"
#include "common.h"
#include "compile_time_macros.h"
#include "console.h"
#include "cros_board_info.h"
#include "fw_config.h"

#define CPRINTS(format, args...) cprints(CC_SYSTEM, format, ##args)

static union aurash_cbi_fw_config fw_config;
BUILD_ASSERT(sizeof(fw_config) == sizeof(uint32_t));

/*
 * FW_CONFIG defaults for aurash if the CBI.FW_CONFIG data is not
 * initialized.
 */
static const union aurash_cbi_fw_config fw_config_defaults = {
	.bj_power = BJ_90W,
	.po_mon = POWER_ON_MONITOR_ENABLE,
};

/*
 * Barrel-jack power adapter ratings.
 */
static const struct {
	int voltage;
	int current;
} bj_power[] = {
	[BJ_90W] = { /* 0 - 90W (also default) */
	  .voltage = 19000,
	  .current = 4740 },
	[BJ_135W] = { /* 1 - 135W */
	  .voltage = 19500,
	  .current = 6920 },
};

/****************************************************************************
 * Aurash FW_CONFIG access
 */
void board_init_fw_config(void)
{
	if (cbi_get_fw_config(&fw_config.raw_value)) {
		CPRINTS("CBI: Read FW_CONFIG failed, using board defaults");
		fw_config = fw_config_defaults;
	}
}

union aurash_cbi_fw_config get_fw_config(void)
{
	return fw_config;
}

void ec_bj_power(uint32_t *voltage, uint32_t *current)
{
	unsigned int bj;

	bj = fw_config.bj_power;
	/* Out of range value defaults to 0 */
	if (bj >= ARRAY_SIZE(bj_power))
		bj = 0;
	*voltage = bj_power[bj].voltage;
	*current = bj_power[bj].current;
}

enum ec_cfg_power_on_monitor ec_cfg_power_on_monitor(void)
{
	return fw_config.po_mon;
}
