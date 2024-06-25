
/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Implimentation for Bridging Table state of Subnet Bridge feature
 * in Bluetooth Mesh Protocol v1.1 specification
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/net/buf.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/mesh.h>

#include "mesh.h"
#include "net.h"
#include "subnet.h"
#include "settings.h"
#include "sbr_bt.h"

#define LOG_LEVEL CONFIG_BT_MESH_NET_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(bt_mesh_sbr_bt);

struct cfg_sbr_val {
    bool enabled;
    // ...
    // bridging_table [CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX]
};

/* One row of the bridging table */
struct sbr_bt_row {
	/* Direction of the entry in the bridging table
	 * 0 - no entry,
	 * 1 - bridge messages with src as addr1 and dst as addr2
	 * 2 - bridge messages with src as addr1 and dst as addr2 and vice-versa
	 */
	// uint8_t direction;
	// uint16_t netidx1;
	// uint16_t netidx2;
	// uint16_t addr1;
	// uint16_t addr2;

	uint32_t direction:8;
	uint32_t netidx1:12;
	uint32_t netidx2:12;
	uint16_t addr1;
	uint16_t addr2;
};

/* Bridging table state. Each item is a slist node. */
static struct sbr_bt {
	sys_snode_t node;
	struct sbr_bt_row row;
} sbr_bt[CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX];

static sys_slist_t sbr_bt_slist;

static uint16_t dst_addrs[CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX*2];
static uint16_t dst_addrcnt;

static bool sbr_enabled;

static void add_to_dst_list(uint16_t dst_addr)
{
	/* Check if addr1 already exist */
	for (int i = 0; i < dst_addrcnt; i++) {
		if (dst_addrs[i] == dst_addr) {
			return;
		}
	}

	dst_addrs[dst_addrcnt++] = dst_addr;
}

static void dst_list_sort(void)
{
	/* sort this list */
	for (int i = 0; i < dst_addrcnt - 1; i++) {
		for (int j = i + 1; j < dst_addrcnt; j++) {
			if (dst_addrs[i] > dst_addrs[j]) {
				uint16_t tmp = dst_addrs[i];
				dst_addrs[i] = dst_addrs[j];
				dst_addrs[j] = tmp;
			}
		}
	}

	// for (int i = 0; i < dst_addrcnt; i++) {
	// 	printk("%d: %d\n", i, dst_addrs[i]);
	// }
}

static bool dst_list_check(uint16_t dst_addr)
{
	int left = 0;
	int right = dst_addrcnt - 1;

	while (left <= right) {
		int md = left + (right - left) / 2;

		if (dst_addrs[md] < dst_addr) {
			left = md + 1;
		} else if (dst_addrs[md] > dst_addr) {
			right = md - 1;
		} else {
			return true;
		}
	}

	return false;
}

static void dst_list_refresh(void)
{
	memset(dst_addrs, 0, sizeof(dst_addrs));
	dst_addrcnt = 0;

	sys_snode_t *node;
	SYS_SLIST_FOR_EACH_NODE(&sbr_bt_slist, node) {
		struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);
		if (br->row.direction == 1 || br->row.direction == 2) {
			add_to_dst_list(br->row.addr2);
		}
		if (br->row.direction == 2) {
			add_to_dst_list(br->row.addr1);
		}
	}

	dst_list_sort();
}

static void dst_list_refresh3(void)
{
	memset(dst_addrs, 0, sizeof(dst_addrs));
	dst_addrcnt = 0;

	for (int i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		if (sbr_bt[i].row.direction == 1 || sbr_bt[i].row.direction == 2) {
			add_to_dst_list(sbr_bt[i].row.addr2);
		}
		if (sbr_bt[i].row.direction == 2) {
			add_to_dst_list(sbr_bt[i].row.addr1);
		}
	}

	dst_list_sort();
}

static int sbren_set(const char *name, size_t len_rd, settings_read_cb read_cb, void *cb_arg)
{
	bool sbren;
	int err;

	if (len_rd == 0) {
		LOG_DBG("Cleared configuration state");
		return 0;
	}

	err = bt_mesh_settings_set(read_cb, cb_arg, &sbren, sizeof(sbren));
	if (err) {
		LOG_ERR("Failed to set \'sbren\'");
		return err;
	}

	sbr_enabled = sbren;

	LOG_DBG("Restored sbren state");

	return 0;
}

/* Define a setting for storing enable state */
BT_MESH_SETTINGS_DEFINE(sbren, "sbren", sbren_set);

/* Set function for initializing bridging table rows from sbr_bt slist */
static int sbrbt_set(const char *name, size_t len_rd, settings_read_cb read_cb, void *cb_arg)
{
	if (len_rd == 0) {
		LOG_DBG("Cleared configuration state");
		return 0;
	}

	/* TODO: based on 'name' initialize sbr_bt rows */



	LOG_DBG("Restored sbrbt state");

	return 0;
}

/* Define a setting for storing briging table rows */
BT_MESH_SETTINGS_DEFINE(sbrbt, "sbrbt", sbrbt_set);

bool sbr_bt_enable_get(void)
{
	return sbr_enabled;
}

int sbr_bt_enable_set(bool enable)
{
	if (sbr_enabled == enable) {
		return 0;
	}

	sbr_enabled = enable;

	bt_mesh_settings_store_schedule(BT_MESH_SETTINGS_SBR_PENDING);

	return 0;
}

void bt_mesh_sbr_en_pending_store(void)
{
#if CONFIG_BT_MESH_SBC_SRV
	char *path = "bt/mesh/sbren";
	int err;

	err = settings_save_one(path, &sbr_enabled, sizeof(sbr_enabled));

	if (err) {
		LOG_ERR("Failed to %s SSeq %s value", (sbr_enabled == 0 ? "delete" : "store"), path);
	} else {
		LOG_DBG("%s %s value", (sbr_enabled == 0 ? "Deleted" : "Stored"), path);
	}
#endif
}

void bt_mesh_sbr_bt_pending_store(void)
{
#if CONFIG_BT_MESH_SBC_SRV
	char *path = "bt/mesh/sbrbt";
	char path_bt[20];
	int err;

	for (int i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		snprintk(path_bt, sizeof(path_bt), "%s/%d", path, i);
		err = settings_save_one(path_bt, &sbr_bt[i].row, sizeof(sbr_bt[i].row));

		if (err) {
			LOG_ERR("Failed to store %s value", path_bt);
		}
	}
#endif
}


/* Add entry to the bridging table */
int sbr_bt_add(enum sbr_bt_direction direction, uint16_t netidx1, uint16_t netidx2,
			uint16_t addr1, uint16_t addr2)
{
	struct sbr_bt_row *row = NULL;
	sys_snode_t *node;

	/* Check if entry already exists, if yes, then it is success. */
	SYS_SLIST_FOR_EACH_NODE(&sbr_bt_slist, node) {
		struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);
		if (br->row.direction == direction && br->row.netidx1 == netidx1 &&
		    br->row.netidx2 == netidx2 && br->row.addr1 == addr1 &&
		    br->row.addr2 == addr2) {
			return 0;
		}
	}

	/* Find empty row */
	SYS_SLIST_FOR_EACH_NODE(&sbr_bt_slist, node) {
		struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);
		if (br->row.direction == 0) {
			row = &br->row;
			break;
		}
	}

	if (!row) {
		return -ENOMEM;
	}

	row->direction = direction;
	row->netidx1 = netidx1;
	row->netidx2 = netidx2;
	row->addr1 = addr1;
	row->addr2 = addr2;

	// dst_list_refresh();

	/* TODO: trigger bridging table scheduled save */

	return 0;
}

/* Remove entry from the bridging table */
int sbr_bt_remove(uint16_t netidx1, uint16_t netidx2, uint16_t addr1, uint16_t addr2)
{
	sys_snode_t *node;

	SYS_SLIST_FOR_EACH_NODE(&sbr_bt_slist, node) {
		struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);
		if (br->row.direction && br->row.netidx1 == netidx1 &&
		    br->row.netidx2 == netidx2 && br->row.addr1 == addr1 &&
		    br->row.addr2 == addr2) {
			memset(&br->row, 0, sizeof(br->row));
			// dst_list_refresh();
			return 0;
		}
	}
	return 0;
}

/* Check if entry corresponding to given src, dst or netidx matches with the
 * any of the briging table entries as per the direction. If yes, populate
 * the new_netidx with the netidx of the entry.
 */
int sbr_bt_check(uint16_t src, uint16_t dst, uint16_t netidx, uint16_t *new_netidx)
{
	// if (!dst_list_check(dst)) {
	// 	return -ENOENT;
	// }

	sys_snode_t *node;

	SYS_SLIST_FOR_EACH_NODE(&sbr_bt_slist, node) {
		struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);
		if ((br->row.direction ==  1 || br->row.direction == 2) &&
		     br->row.netidx1 == netidx && br->row.addr1 == src && br->row.addr2 == dst) {
			*new_netidx = br->row.netidx2;
			return 0;
		}
		if (br->row.direction == 2 &&
		    br->row.netidx2 == netidx && br->row.addr2 == src && br->row.addr1 == dst) {
			*new_netidx = br->row.netidx1;
			return 0;
		}
	}
	return -ENOENT;
}

int sbr_bt_reset(void)
{
	int err;

	sbr_enabled = false;
	sys_slist_init(&sbr_bt_slist);
	memset(sbr_bt, 0, sizeof(sbr_bt));
	for (int i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		sys_slist_append(&sbr_bt_slist, &sbr_bt[i].node);
	}

	err = settings_delete("bt/mesh/sbren");
	if (err) {
		return err;
	}
	err = settings_delete("bt/mesh/sbrbt");

	printk("size: %d\n", sizeof(struct sbr_bt_row));
	printk("size: %d\n", sizeof(sbr_bt));
	return err;
}

int sbr_bt_init(void)
{
	sys_slist_init(&sbr_bt_slist);
	memset(sbr_bt, 0, sizeof(sbr_bt));
	for (int i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		sys_slist_append(&sbr_bt_slist, &sbr_bt[i].node);
	}

	// dst_list_refresh();
	return 0;
}

/* Remove the entry from the bridging table that corresponds with the netkey index
 * of the removed subnet.
 */
static void sbr_netkey_removed_evt(struct bt_mesh_subnet *sub, enum bt_mesh_key_evt evt)
{
	sys_snode_t *node;

	SYS_SLIST_FOR_EACH_NODE(&sbr_bt_slist, node) {
		struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);
		if (br->row.netidx1 == sub->net_idx || br->row.netidx2 == sub->net_idx) {
			memset(&br->row, 0, sizeof(br->row));
		}
	}
}

/* Add event hook for key deletion event */
BT_MESH_SUBNET_CB_DEFINE(sbr) = {
	.evt_handler = sbr_netkey_removed_evt,
};


/** Alternate 2: Remove slist entries and add them when needed */
int sbr_bt_init2(void)
{
	sys_slist_init(&sbr_bt_slist);
	memset(sbr_bt, 0, sizeof(sbr_bt));
	// dst_list_refresh();
	return 0;
}


int sbr_bt_add2(enum sbr_bt_direction direction, uint16_t netidx1, uint16_t netidx2,
			uint16_t addr1, uint16_t addr2)
{
	 sys_snode_t *node = NULL;

	/* Check if entry already exists, if yes, then it is success. */
	SYS_SLIST_FOR_EACH_NODE(&sbr_bt_slist, node) {
		struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);
		if (br->row.direction == direction && br->row.netidx1 == netidx1 &&
		    br->row.netidx2 == netidx2 && br->row.addr1 == addr1 &&
		    br->row.addr2 == addr2) {
			return 0;
		}
	}

	/* Find empty node */
	for (int i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		if (sbr_bt[i].row.direction == 0) {
			node = &sbr_bt[i].node;
		}
	}

	if (!node) {
		return -ENOMEM;
	}

	/* Append to slist and populate the row */
	sys_slist_append(&sbr_bt_slist, node);
	struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);

	br->row.direction = direction;
	br->row.netidx1 = netidx1;
	br->row.netidx2 = netidx2;
	br->row.addr1 = addr1;
	br->row.addr2 = addr2;

	// dst_list_refresh();

	/* TODO: trigger bridging table scheduled save */

	return 0;
}


int sbr_bt_remove2(uint16_t netidx1, uint16_t netidx2, uint16_t addr1, uint16_t addr2)
{
	sys_snode_t *node;

	SYS_SLIST_FOR_EACH_NODE(&sbr_bt_slist, node) {
		struct sbr_bt *br = CONTAINER_OF(node, struct sbr_bt, node);
		if (br->row.direction && br->row.netidx1 == netidx1 &&
		    br->row.netidx2 == netidx2 && br->row.addr1 == addr1 &&
		    br->row.addr2 == addr2) {
			sys_slist_find_and_remove(&sbr_bt_slist, node);
			memset(br, 0, sizeof(struct sbr_bt));
			// dst_list_refresh();
			return 0;
		}
	}
	return 0;
}

/** Alternate 3: Directly use array, no linked-list */
int sbr_bt_init3(void)
{
	memset(sbr_bt, 0, sizeof(sbr_bt));
	dst_list_refresh();
	return 0;
}


int sbr_bt_add3(enum sbr_bt_direction direction, uint16_t netidx1, uint16_t netidx2,
			uint16_t addr1, uint16_t addr2)
{

	/* Check if entry already exists, if yes, then it is success. */
	int i;
	for (i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		if (sbr_bt[i].row.direction == direction && sbr_bt[i].row.netidx1 == netidx1 &&
		    sbr_bt[i].row.netidx2 == netidx2 && sbr_bt[i].row.addr1 == addr1 &&
		    sbr_bt[i].row.addr2 == addr2) {
			return 0;
		}
	}

	/* Find empty element in the array */
	for (i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		if (sbr_bt[i].row.direction == 0) {
			break;
		}
	}

	if (i == CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX) {
		return -ENOMEM;
	}

	/* Update the row */
	sbr_bt[i].row.direction = direction;
	sbr_bt[i].row.netidx1 = netidx1;
	sbr_bt[i].row.netidx2 = netidx2;
	sbr_bt[i].row.addr1 = addr1;
	sbr_bt[i].row.addr2 = addr2;

	// dst_list_refresh3();

	/* TODO: trigger bridging table scheduled save */

	return 0;
}

int sbr_bt_check3(uint16_t src, uint16_t dst, uint16_t netidx, uint16_t *new_netidx)
{
	// if (!dst_list_check(dst)) {
	// 	return -ENOENT;
	// }

	/* Iterate over items */
	for (int i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		if ((sbr_bt[i].row.direction ==  1 || sbr_bt[i].row.direction == 2) &&
		     sbr_bt[i].row.netidx1 == netidx && sbr_bt[i].row.addr1 == src &&
		     sbr_bt[i].row.addr2 == dst) {
			*new_netidx = sbr_bt[i].row.netidx2;
			return 0;
		}

		if (sbr_bt[i].row.direction == 2 &&
		    sbr_bt[i].row.netidx2 == netidx && sbr_bt[i].row.addr2 == src &&
		    sbr_bt[i].row.addr1 == dst) {
			*new_netidx = sbr_bt[i].row.netidx1;
			return 0;
		}
	}

	return -ENOENT;
}

int sbr_bt_remove3(uint16_t netidx1, uint16_t netidx2, uint16_t addr1, uint16_t addr2)
{
	/* Iterate over items and set matching row to 0 */
	for (int i = 0; i < CONFIG_BT_MESH_SBC_BRIDGING_TABLE_ITEMS_MAX; i++) {
		if (sbr_bt[i].row.direction && sbr_bt[i].row.netidx1 == netidx1 &&
		    sbr_bt[i].row.netidx2 == netidx2 && sbr_bt[i].row.addr1 == addr1 &&
		    sbr_bt[i].row.addr2 == addr2) {
			memset(&sbr_bt[i].row, 0, sizeof(sbr_bt[i].row));
			// dst_list_refresh3();
			return 0;
		}
	}

	return 0;
}
