/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <inttypes.h>
#include <stdint.h>

#include "mesh_test.h"

#define WAIT_TIME 20 /* Seconds */

#define PASS_THRESHOLD 100 /* Audio packets */

extern enum bst_result_t bst_result;

#define FAIL(...)					\
	do {						\
		bst_result = Failed;			\
		bs_trace_error_time_line(__VA_ARGS__);	\
	} while (0)

#define PASS(...)					\
	do {						\
		bst_result = Passed;			\
		bs_trace_info_time(1, __VA_ARGS__);	\
	} while (0)

static void test_node_device(void)
{
	bst_result = In_progress;
	LOG_INF("Hello :simid %s nbr %d", bsim_args_get_simid(), bsim_args_get_global_device_nbr());
}

void bt_mesh_test_timeout(bs_time_t HW_device_time)
{
	if (bst_result != Passed) {
		FAIL("Test timeout (not passed after %i seconds)",
		     HW_device_time / USEC_PER_SEC);
	}

	bs_trace_silent_exit(0);
}

#define TEST_CASE(role, name, description)                     \
	{                                                      \
		.test_id = "check_" #role "_" #name,           \
		.test_descr = description,                     \
		.test_tick_f = bt_mesh_test_timeout,           \
		.test_main_f = test_##role##_##name,           \
	}

static const struct bst_test_instance test_network[] = {
	TEST_CASE(node, device, "Network simulation test"),
	BSTEST_END_MARKER
};

struct bst_test_list *test_network_tst_install(struct bst_test_list *tests)
{
	tests = bst_add_tests(tests, test_network);
	return tests;
}
