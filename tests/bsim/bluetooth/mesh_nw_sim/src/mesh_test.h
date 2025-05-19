/** @file
 *  @brief Common functionality for Bluetooth Mesh BabbleSim tests.
 */

/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_TESTS_BLUETOOTH_BSIM_BT_BSIM_TEST_MESH_MESH_TEST_H_
#define ZEPHYR_TESTS_BLUETOOTH_BSIM_BT_BSIM_TEST_MESH_MESH_TEST_H_
#include <zephyr/kernel.h>

#include "bs_types.h"
#include "bs_tracing.h"
#include "time_machine.h"
#include "bstests.h"

#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh.h>
#include <mesh/adv.h>

#define TEST_MOD_ID 0x8888
#define TEST_MSG_OP_1  BT_MESH_MODEL_OP_1(0x0f)
#define TEST_MSG_OP_2  BT_MESH_MODEL_OP_1(0x10)

#define TEST_VND_COMPANY_ID 0x1234
#define TEST_VND_MOD_ID   0x5678

#define MODEL_LIST(...) ((const struct bt_mesh_model[]){ __VA_ARGS__ })

#define FAIL(msg, ...)                                                         \
	do {                                                                   \
		bst_result = Failed;                                           \
		bs_trace_error_time_line(msg "\n", ##__VA_ARGS__);             \
	} while (0)

#define PASS()                                                                 \
	do {                                                                   \
		bst_result = Passed;                                           \
		bs_trace_info_time(1, "%s PASSED\n", __func__);                \
	} while (0)

#define ASSERT_OK(cond)                                                        \
	do {                                                                   \
		int _err = (cond);                                             \
		if (_err) {                                                    \
			bst_result = Failed;                                   \
			bs_trace_error_time_line(                              \
				#cond " failed with error %d\n", _err);        \
		}                                                              \
	} while (0)

#define ASSERT_OK_MSG(cond, fmt, ...)                                          \
	do {                                                                   \
		int _err = (cond);                                             \
		if (_err) {                                                    \
			bst_result = Failed;                                   \
			bs_trace_error_time_line(                              \
				#cond " failed with error %d\n" fmt, _err,     \
				##__VA_ARGS__);                                \
		}                                                              \
	} while (0)

#define ASSERT_TRUE(cond)                                                      \
	do {                                                                   \
		if (!(cond)) {                                                 \
			bst_result = Failed;                                   \
			bs_trace_error_time_line(                              \
				#cond " is false.\n");                         \
		}                                                              \
	} while (0)

#define ASSERT_TRUE_MSG(cond, fmt, ...)                                        \
	do {                                                                   \
		if (!(cond)) {                                                 \
			bst_result = Failed;                                   \
			bs_trace_error_time_line(                              \
				#cond " is false. " fmt, ##__VA_ARGS__);       \
		}                                                              \
	} while (0)


#define ASSERT_FALSE(cond)                                                     \
	do {                                                                   \
		if (cond) {                                                    \
			bst_result = Failed;                                   \
			bs_trace_error_time_line(                              \
				#cond " is true.\n");                          \
		}                                                              \
	} while (0)

#define ASSERT_FALSE_MSG(cond, fmt, ...)                                       \
	do {                                                                   \
		if (cond) {                                                    \
			bst_result = Failed;                                   \
			bs_trace_error_time_line(                              \
				#cond " is true. " fmt, ##__VA_ARGS__);        \
		}                                                              \
	} while (0)

#define ASSERT_EQUAL(expected, got)                                            \
	do {                                                                   \
		if ((expected) != (got)) {                                     \
			bst_result = Failed;                                   \
			bs_trace_error_time_line(                              \
				#expected " not equal to " #got ": %d != %d\n",\
				(expected), (got));                            \
		}                                                              \
	} while (0)

#define ASSERT_IN_RANGE(got, min, max)                                                             \
	do {                                                                                       \
		if (!IN_RANGE(got, min, max)) {                                            \
			bst_result = Failed;                                                       \
			bs_trace_error_time_line(#got " not in range %d <-> %d, " #got " = %d\n",  \
						 (min), (max), (got));                             \
		}                                                                                  \
	} while (0)

#define WAIT_FOR_COND(cond, wait)                                                                  \
	do {                                                                                       \
		bool _err = false;                                                                 \
		for (uint8_t sec = (wait); !(cond); sec--) {                                       \
			if (!sec) {                                                                \
				_err = true;                                                       \
				break;                                                             \
			}                                                                          \
			k_sleep(K_SECONDS(1));                                                     \
		}                                                                                  \
                                                                                                   \
		if (_err) {                                                                        \
			bst_result = Failed;                                                       \
			bs_trace_error_time_line("Waiting for " #cond " timed out\n");             \
		}                                                                                  \
	} while (0)

struct bt_mesh_test_cfg {
	uint16_t addr;
	uint8_t dev_key[16];
};

extern enum bst_result_t bst_result;
void bt_mesh_test_timeout(bs_time_t HW_device_time);

#endif /* ZEPHYR_TESTS_BLUETOOTH_BSIM_BT_BSIM_TEST_MESH_MESH_TEST_H_ */
