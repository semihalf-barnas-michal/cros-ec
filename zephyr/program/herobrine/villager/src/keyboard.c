/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "keyboard_raw.h"
#include "keyboard_scan.h"

/*
 * We have total 30 pins for keyboard connecter {-1, -1} mean
 * the N/A pin that don't consider it and reserve index 0 area
 * that we don't have pin 0.
 */
const int keyboard_factory_scan_pins[][2] = {
	{ -1, -1 }, { 0, 5 },	{ 1, 1 },   { 1, 0 },	{ 0, 6 },   { 0, 7 },
	{ -1, -1 }, { -1, -1 }, { 1, 4 },   { 1, 3 },	{ -1, -1 }, { 1, 6 },
	{ 1, 7 },   { 3, 1 },	{ 2, 0 },   { 1, 5 },	{ 2, 6 },   { 2, 7 },
	{ 2, 1 },   { 2, 4 },	{ 2, 5 },   { 1, 2 },	{ 2, 3 },   { 2, 2 },
	{ 3, 0 },   { -1, -1 }, { -1, -1 }, { -1, -1 },
};
const int keyboard_factory_scan_pins_used =
	ARRAY_SIZE(keyboard_factory_scan_pins);
