/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/net/buf.h>
#include <zephyr/bluetooth/mesh.h>
#include <stdlib.h>

#include "settings.h"
#include "sbr_bt.h"

/* Used for cleaning RPL without checking it. */
static bool skip_delete;

#define TEST_VECT_SZ (CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX + 1)

static struct test_sbr_bt_row {
	uint8_t direction;
	uint16_t netidx1;
	uint16_t netidx2;
	uint16_t addr1;
	uint16_t addr2;
} test_vector[TEST_VECT_SZ];

#define ADDR1_BASE (1)
#define ADDR2_BASE (100)

/**** Helper functions ****/

/* Should be called after the reset operation is finished. */
// static void check_entries_from_test_vector(void)
// {

// }

// static void check_empty_entries(int cnt)
// {

// }

// static void expect_pending_store(void)
// {
// 	/* Entries with old_iv == true should be removed, others should be stored. */
// 	for (int i = 0; i < ARRAY_SIZE(test_vector); i++) {
// 		if (test_vector[i].old_iv) {
// 			ztest_expect_value(settings_delete, name, test_vector[i].name);
// 		} else {
// 			ztest_expect_data(settings_save_one, name, test_vector[i].name);
// 		}
// 	}
// }


static void setup(void *f)
{
	/* create test vector */
	for (int i = 0; i < TEST_VECT_SZ; i++) {
		test_vector[i].direction = (i % 2) + 1;
		test_vector[i].netidx1 = i;
		test_vector[i].netidx2 = i + 16;
		test_vector[i].addr1 = ADDR1_BASE + i;
		test_vector[i].addr2 = ADDR2_BASE + i;
	}

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

static void check_fill_all_bt_entries(void)
{
	int err;
	for (int i = 0; i < TEST_VECT_SZ; i++) {
		err = sbr_bt_add(test_vector[i].direction, test_vector[i].netidx1,
			test_vector[i].netidx2, test_vector[i].addr1, test_vector[i].addr2);
		if (i != CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX) {
			zassert_equal(err, 0);
		} else {
			zassert_equal(err, -ENOMEM);
		}
	}
}

static void check_delete_all_bt_entries(void)
{
	int err;
	for (int i = TEST_VECT_SZ; i >= 0; --i) {
		err = sbr_bt_remove(test_vector[i].netidx1, test_vector[i].netidx2,
			test_vector[i].addr1, test_vector[i].addr2);
		zassert_equal(err, 0);
	}
}

static void check_fill_all_bt_entries2(void)
{
	int err;
	for (int i = 0; i < TEST_VECT_SZ; i++) {
		err = sbr_bt_add2(test_vector[i].direction, test_vector[i].netidx1,
			test_vector[i].netidx2, test_vector[i].addr1, test_vector[i].addr2);
		if (i != CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX) {
			zassert_equal(err, 0);
		} else {
			zassert_equal(err, -ENOMEM);
		}
	}
}

static void check_delete_all_bt_entries2(void)
{
	int err;
	for (int i = TEST_VECT_SZ; i >= 0; --i) {
		err = sbr_bt_remove2(test_vector[i].netidx1, test_vector[i].netidx2,
			test_vector[i].addr1, test_vector[i].addr2);
		zassert_equal(err, 0);
	}
}


static void check_sbr_bt_reset(void)
{
	int err;

	ztest_expect_data(settings_delete, name, "bt/mesh/sbren");
	ztest_expect_data(settings_delete, name, "bt/mesh/sbrbt");
	err = sbr_bt_reset();
	zassert_equal(err, 0);
}

/**** Tests ****/

ZTEST_SUITE(bt_mesh_sbr_bt, NULL, NULL, setup, NULL, NULL);

ZTEST(bt_mesh_sbr_bt, test_reset_normal)
{
	check_sbr_bt_reset();

	/* Test add entries to bridging table. */
	check_fill_all_bt_entries();

	/* Test remove entries from bridging table, and then fill it again. */
	check_delete_all_bt_entries();
	check_fill_all_bt_entries();

	/* Test resetting of the table, and then fill it again. */
	check_sbr_bt_reset();
	check_fill_all_bt_entries();

}

ZTEST(bt_mesh_sbr_bt, test_reset_normal2)
{
	sbr_bt_init2();

	/* Test add entries to bridging table. */
	check_fill_all_bt_entries2();

	/* Test remove entries from bridging table, and then fill it again. */
	check_delete_all_bt_entries2();
	check_fill_all_bt_entries2();

	/* Test resetting of the table, and then fill it again. */
	sbr_bt_init2();
	check_fill_all_bt_entries2();
}

ZTEST(bt_mesh_sbr_bt, test_storage)
{
	/* TODO: Add test for storing restoring of the data */
}

ZTEST(bt_mesh_sbr_bt, test_entry_removal)
{
	/* TODO: Add test for removing table entries */
}

/** Test for performance check on physical devices only */
typedef int (* init_func_t)(void);
init_func_t init_func[2] = {sbr_bt_init, sbr_bt_init2};

typedef int (* add_func_t)(enum sbr_bt_direction direction, uint16_t netidx1, uint16_t netidx2,
                    uint16_t addr1, uint16_t addr2);
add_func_t add_func[2] = {sbr_bt_add, sbr_bt_add2};

typedef int (* check_func_t)(uint16_t src, uint16_t dst, uint16_t netidx, uint16_t *new_netidx);
check_func_t check_func[2] = {sbr_bt_check, sbr_bt_check2};

char * opt_str[2] = {"linked list", "array"};

#define LOOP_INC (CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX/8)
#define NUM_MSGS (1000)

ZTEST(bt_mesh_sbr_bt, test_zcheck_entry_randomly)
{
	int err, idx;
	uint16_t src, dst, netidx1, new_netidx;
	int entry_idx;
	zassert(LOOP_INC > 0 && (CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX % LOOP_INC) == 0,
		"inncorrect increment value");

	for (int f = 0; f < ARRAY_SIZE(init_func); f ++) {
		check_sbr_bt_reset();
		init_func[f]();

		printk("option: %d (%s), num msgs: %d\n", f + 1, opt_str[f], NUM_MSGS);
		for (int t = 0; t < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; t += LOOP_INC)
		{
			/* add one entry to the table */
			for (int k = 0; k < LOOP_INC; k++) {
				entry_idx = t + k;
				err = add_func[f](test_vector[entry_idx].direction,
					test_vector[entry_idx].netidx1,
					test_vector[entry_idx].netidx2,
					test_vector[entry_idx].addr1,
					test_vector[entry_idx].addr2);
				zassert_equal(err, 0);
			}

			uint32_t tick1 = k_uptime_ticks();
			for (int i = 0; i < NUM_MSGS; i++)
			{
				/* randomly pick an entry from the test vector */
				idx = rand() % TEST_VECT_SZ;
				// idx = i % CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX;

				src = test_vector[idx].addr1;
				dst = test_vector[idx].addr2;
				netidx1 = test_vector[idx].netidx1;

				err = check_func[f](src, dst, netidx1, &new_netidx);

				if (idx > (LOOP_INC - 1 + t)) {
					zassert_equal(err, -ENOENT);
				} else {

					zassert_equal(err, 0);
					zassert_equal(new_netidx, test_vector[idx].netidx2);

					if (test_vector[idx].direction == 2) {
						netidx1 = test_vector[idx].netidx2;
						err = check_func[f](dst, src, netidx1, &new_netidx);
						zassert_equal(err, 0);
						zassert_equal(new_netidx, test_vector[idx].netidx1);
					}
				}
			}
			uint32_t tick2 = k_uptime_ticks();
			uint32_t diff = tick2 - tick1;
			int bridged_traffic = ((entry_idx + 1) * 100) / TEST_VECT_SZ;
			printk("added entries: %3u, target traffic: %3u%%, tick2: %8u - tick1: %8u = diff: %8u \tus: %u\n",
				(entry_idx + 1), bridged_traffic, tick2, tick1, diff, k_ticks_to_us_floor32(diff));
		}
	}
}