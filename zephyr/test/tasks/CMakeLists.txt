# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS "${ZEPHYR_BASE}")
project(tasks)

# Include the local test directory for shimmed_test_tasks.h
zephyr_include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

target_sources_ifdef(CONFIG_HAS_TEST_TASKS app PRIVATE
	main.c
	"${CMAKE_CURRENT_SOURCE_DIR}/../../shim/src/tasks.c")

target_sources_ifdef(CONFIG_SHIMMED_TASKS app PRIVATE
	extra_tasks.c)
