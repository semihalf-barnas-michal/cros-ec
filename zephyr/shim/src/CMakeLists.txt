# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

zephyr_library_sources(console.c)
zephyr_library_sources(crc.c)
zephyr_library_sources(gpio.c)
zephyr_library_sources(gpio_int.c)

add_subdirectory("buttons")
add_subdirectory("cbi")
add_subdirectory("led_driver")

if (DEFINED CONFIG_ARCH_POSIX)
  zephyr_library_sources(ztest_system.c)
else()
  zephyr_library_sources(system.c)
endif()

zephyr_library_sources_ifdef(no_libgcc                  libgcc_${ARCH}.S)

zephyr_library_sources_ifdef(CONFIG_NAMED_ADC_CHANNELS      adc.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_BATTERY
                                                            battery.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_CHARGER_RT9490
                                                            bc12_rt9490.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_CHARGER     charger.c)
zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ               chipset_api.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_HOST_INTERFACE_ESPI
                                                            espi.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_FAN         fan.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_FLASH_CROS  flash.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_GPIO_ID
							    gpio_id.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_HOOKS       hooks.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_HOSTCMD     host_command.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_HOSTCMD_CONSOLE
                                                            console_buffer.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_IOEX_CROS_DRV
                                                            ioex.c
                                                            ioex_drv.c)
if (NOT DEFINED CONFIG_PLATFORM_EC_KEYBOARD_DISCRETE)
  zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_KEYBOARD
                                                            keyboard_raw.c)
endif()
zephyr_library_sources_ifdef(CONFIG_CROS_EC_KEYSCAN         keyscan.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_LOG_BACKEND_CONSOLE_BUFFER
                             log_backend_console_buffer.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_MKBP_EVENT  mkbp_event.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_MOTIONSENSE
                                                            motionsense_sensors.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_PANIC       panic.c)
zephyr_library_sources_ifdef(CONFIG_CROS_EC_POWER_SIGNAL_LIST
							    power.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_PWM_HC      pwm_hc.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_LED_PWM     pwm_led.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_RTC         rtc.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_SWITCHCAP_GPIO
							    switchcap_gpio.c)
zephyr_library_sources_ifdef(CONFIG_CROS_EC_SWITCHCAP_LN9310
							    switchcap_ln9310.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_TEMP_SENSOR temp_sensors.c
                                                            thermal.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_TIMER       hwtimer.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_I2C         i2c.c)
zephyr_library_sources_ifdef(CONFIG_SHIMMED_TASKS           tasks.c)
zephyr_library_sources_ifdef(CONFIG_WATCHDOG                watchdog.c)
zephyr_library_sources_ifndef(CONFIG_PLATFORM_EC_BC12_SINGLE_DRIVER
                                                            bc12.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_BC12_DETECT_PI3USB9201
	                                                     bc12_pi3usb9201.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_USBC_PPC    ppc.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_USB_PD_TCPM_NCT38XX
                                                            tcpc_nct38xx.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_USB_PD_TCPM_TCPCI
                                                            tcpc.c)

zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_USBA        usba.c)
zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ power_host_sleep_api.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_USB_MUX     usb_muxes.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_USBC_RETIMER_INTEL_BB
                                                            bb_retimer_usb_mux.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_USBC_RETIMER_INTEL_HB
                                                            bb_retimer_usb_mux.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_SYSTEM_SAFE_MODE system_safe_mode.c)
zephyr_library_sources_ifdef(CONFIG_PLATFORM_EC_SHA256_HW_ZEPHYR
	                                                    sha256_hw.c)
