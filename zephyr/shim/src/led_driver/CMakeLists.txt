# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_LED_DT      led.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_LED_DT_GPIO led_gpio.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_LED_DT_PWM  led_pwm.c)
