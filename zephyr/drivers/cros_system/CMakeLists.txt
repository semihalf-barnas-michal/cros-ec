# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

zephyr_library_sources_ifdef(CONFIG_CROS_SYSTEM_IT8XXX2	cros_system_it8xxx2.c)
zephyr_library_sources_ifdef(CONFIG_CROS_SYSTEM_NPCX	cros_system_npcx.c)
zephyr_library_sources_ifdef(CONFIG_CROS_SYSTEM_XEC	cros_system_xec.c)
zephyr_library_sources_ifdef(CONFIG_CROS_SYSTEM_NATIVE_POSIX	cros_system_native_posix.c)
zephyr_library_sources_ifdef(CONFIG_CROS_SYSTEM_STM32	cros_system_stm32.c)
