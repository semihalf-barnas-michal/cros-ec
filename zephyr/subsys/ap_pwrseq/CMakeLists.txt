# SPDX-License-Identifier: Apache-2.0

zephyr_library_include_directories(include)

zephyr_library_sources(ap_events.c)

zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ
	ap_power_interface.c
	power_host_sleep.c
	power_signals.c
	)

zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ_SIGNAL_GPIO signal_gpio.c)
zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ_SIGNAL_VW signal_vw.c)
zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ_SIGNAL_ADC signal_adc.c)

zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ_DRIVER ap_pwrseq.c)
zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ_DRIVER ap_pwrseq_sm.c)

zephyr_library_sources_ifdef(CONFIG_X86_NON_DSX_PWRSEQ
	x86_non_dsx_common_pwrseq_sm_handler.c
	x86_non_dsx_chipset_power_state.c)
zephyr_library_sources_ifdef(CONFIG_AP_PWRSEQ_S0IX_ERROR_RECOVERY
	x86_non_dsx_common_pwrseq_host_sleep.c)
zephyr_library_sources_ifdef(CONFIG_X86_NON_DSX_PWRSEQ_CONSOLE
	x86_non_dsx_common_pwrseq_console.c)
zephyr_library_sources_ifdef(CONFIG_X86_NON_DSX_PWRSEQ_HOST_CMD
	x86_non_dsx_common_pwrseq_host_command.c)
zephyr_library_sources_ifdef(CONFIG_X86_NON_DSX_PWRSEQ_ADL
	x86_non_dsx_adlp_pwrseq_sm.c)
zephyr_library_sources_ifdef(CONFIG_X86_NON_DSX_PWRSEQ_MTL
	x86_non_dsx_mtl_pwrseq_sm.c)
