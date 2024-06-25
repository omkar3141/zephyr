/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/net/buf.h>
#include <zephyr/bluetooth/mesh.h>

#include "settings.h"
#include "sbr_bt.h"

/* Used for cleaning RPL without checking it. */
static bool skip_delete;


/**** Helper functions ****/

static void prepare_rpl_and_start_reset(void)
{

}

/* Should be called after the reset operation is finished. */
static void check_entries_from_test_vector(void)
{

}

static void check_empty_entries(int cnt)
{

}

static void call_rpl_check_on(int func, int cnt, struct test_rpl_entry *entry)
{
	settings_func = func;
	settings_func_cnt = cnt;

	recv_msg.local_match = true;
	recv_msg.ctx.addr = entry->src;
	recv_msg.seq = entry->seq;
	recv_msg.old_iv = entry->old_iv;
}

static void expect_pending_store(void)
{
	/* Entries with old_iv == true should be removed, others should be stored. */
	for (int i = 0; i < ARRAY_SIZE(test_vector); i++) {
		if (test_vector[i].old_iv) {
			ztest_expect_value(settings_delete, name, test_vector[i].name);
		} else {
			ztest_expect_data(settings_save_one, name, test_vector[i].name);
		}
	}
}


static void setup(void *f)
{

}

/**** Mocked functions ****/

void bt_mesh_settings_store_schedule(enum bt_mesh_settings_flag flag)
{
	ztest_check_expected_value(flag);
}

void bt_mesh_settings_store_cancel(enum bt_mesh_settings_flag flag)
{
}

int settings_save_one(const char *name, const void *value, size_t val_len)
{
	ztest_check_expected_data(name, strlen(name));

	check_op(SETTINGS_SAVE_ONE);

	return 0;
}

int settings_delete(const char *name)
{
	if (skip_delete) {
		return 0;
	}

	ztest_check_expected_data(name, strlen(name));
	return 0;
}

/**** Tests ****/

ZTEST_SUITE(bt_mesh_sbr_bt, NULL, NULL, setup, NULL, NULL);

/** Test that entries with old_iv == true are removed after the reset operation finished. */
ZTEST(bt_mesh_sbr_bt, test_reset_normal)
{
	ztest_expect_data(settings_delete, name, "bt/mesh/sbren");
	ztest_expect_data(settings_delete, name, "bt/mesh/sbrbt");
	sbt_bt_reset();
}
