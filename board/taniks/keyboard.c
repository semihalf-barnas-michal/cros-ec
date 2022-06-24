/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "aw20198.h"
#include "common.h"
#include "ec_commands.h"
#include "keyboard_scan.h"
#include "rgb_keyboard.h"
#include "timer.h"

/* Keyboard scan setting */
__override struct keyboard_scan_config keyscan_config = {
	/* Increase from 50 us, because KSO_02 passes through the H1. */
	.output_settle_us = 80,
	/* Other values should be the same as the default configuration. */
	.debounce_down_us = 9 * MSEC,
	.debounce_up_us = 30 * MSEC,
	.scan_period_us = 3 * MSEC,
	.min_post_scan_delay_us = 1000,
	.poll_timeout_us = 100 * MSEC,
	.actual_key_mask = {
		0x1c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xa4, 0xff, 0xff, 0x55, 0xff, 0xff, 0xff, 0xff,  /* full set */
	},
	.ksi_threshold_mv = 250,
};

static const struct ec_response_keybd_config taniks_kb = {
	.num_top_row_keys = 14,
	.action_keys = {
		TK_BACK,		/* T1 */
		TK_REFRESH,		/* T2 */
		TK_FULLSCREEN,		/* T3 */
		TK_OVERVIEW,		/* T4 */
		TK_SNAPSHOT,		/* T5 */
		TK_BRIGHTNESS_DOWN,	/* T6 */
		TK_BRIGHTNESS_UP,	/* T7 */
		TK_ABSENT,		/* T8 */
		TK_ABSENT,		/* T9 */
		TK_ABSENT,		/* T10 */
		TK_MICMUTE,		/* T11 */
		TK_VOL_MUTE,		/* T12 */
		TK_VOL_DOWN,		/* T13 */
		TK_VOL_UP,		/* T14 */
	},
	.capabilities = KEYBD_CAP_SCRNLOCK_KEY | KEYBD_CAP_NUMERIC_KEYPAD,
};

static struct rgb_s grid0[RGB_GRID0_COL * RGB_GRID0_ROW];

struct rgbkbd rgbkbds[] = {
	[0] = {
		.cfg = &(const struct rgbkbd_cfg) {
			.drv = &aw20198_drv,
			.i2c = I2C_PORT_KBMCU,
			.col_len = RGB_GRID0_COL,
			.row_len = RGB_GRID0_ROW,
		},
		.buf = grid0,
	},
};
const uint8_t rgbkbd_count = ARRAY_SIZE(rgbkbds);

const uint8_t rgbkbd_hsize = RGB_GRID0_COL;
const uint8_t rgbkbd_vsize = RGB_GRID0_ROW;

#define LED(x, y)	RGBKBD_COORD((x), (y))
#define DELM		RGBKBD_DELM
const uint8_t rgbkbd_map[] = {
	DELM,
	LED( 0, 0), DELM,
	LED( 1, 0), DELM,
	LED( 2, 0), DELM,
	LED( 3, 0), DELM,
	LED( 4, 0), DELM,
	LED( 5, 0), DELM,
	LED( 6, 0), DELM,
	LED( 7, 0), DELM,
	LED( 0, 1), DELM,
	LED( 1, 1), DELM,
	LED( 2, 1), DELM,
	LED( 3, 1), DELM,
	LED( 4, 1), DELM,
	LED( 5, 1), DELM,
	LED( 6, 1), DELM,
	LED( 7, 1), DELM,
	LED( 0, 2), DELM,
	LED( 1, 2), DELM,
	LED( 2, 2), DELM,
	LED( 3, 2), DELM,
	LED( 4, 2), DELM,
	LED( 5, 2), DELM,
	LED( 6, 2), DELM,
	LED( 7, 2), DELM,
	LED( 0, 3), DELM,
	LED( 1, 3), DELM,
	LED( 2, 3), DELM,
	LED( 3, 3), DELM,
	LED( 4, 3), DELM,
	LED( 5, 3), DELM,
	LED( 6, 3), DELM,
	LED( 7, 3), DELM,
	LED( 0, 4), DELM,
	LED( 1, 4), DELM,
	LED( 2, 4), DELM,
	LED( 3, 4), DELM,
	LED( 4, 4), DELM,
	LED( 5, 4), DELM,
	LED( 6, 4), DELM,
	LED( 7, 4), DELM,
	DELM,
};
#undef LED
#undef DELM
const size_t rgbkbd_map_size = ARRAY_SIZE(rgbkbd_map);

__override const struct ec_response_keybd_config
*board_vivaldi_keybd_config(void)
{
	return &taniks_kb;
}
