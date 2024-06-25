
/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Implimentation for Bridging Table state of Subnet Bridge feature
 * in Bluetooth Mesh Protocol v1.1 specification
 */

#include <zephyr/kernel.h>
#include <stdlib.h>
#include <zephyr/sys/slist.h>

#include <zephyr/bluetooth/mesh.h>
#include <zephyr/bluetooth/bluetooth.h>
#include "subnet.h"
#include "settings.h"

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

static sys_slist_t sbr_bt;

static bool sbr_enabled;

static int sbren_set(const char *name, size_t len_rd,
		   settings_read_cb read_cb, void *cb_arg)
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
	int err;

	err = settings_save_one(path, &sseq_out, sizeof(sseq_out));

	if (err) {
		LOG_ERR("Failed to %s SSeq %s value", (sseq_out == 0 ? "delete" : "store"), path);
	} else {
		LOG_DBG("%s %s value", (sseq_out == 0 ? "Deleted" : "Stored"), path);
	}
#endif
}


/* Add entry to the bridging table */
int sbr_bt_add(enum sbr_bt_direction direction, uint16_t netidx1, uint16_t netidx2,
                        uint16_t addr1, uint16_t addr2)
{

}

/* Check if entry corresponding to given src, dst or netidx matches with the
 * any of the briging table entries as per the direction rule.
 */
int sbr_bt_check(uint16_t src, uint16_t dst, uint16_t netidx, uint16_t *new_netidx)
{

}

int sbt_bt_reset(void)
{
        int err;

        sbr_enabled = false;
        sys_slist_init(&sbr_bt);
        memset(sbr_bt, 0, sizeof(sbr_bt));

        char *path = "bt/mesh/sbren";
        err = settings_delete(path);
        char *path = "bt/mesh/sbrbt";
        err = settings_delete(path);

}

/* Remove the entry from the bridging table that corresponds with the netkey index
 * of the removed subnet
 */
static void sbr_netkey_removed_evt(struct bt_mesh_subnet *sub, enum bt_mesh_key_evt evt)
{

}

/* Add event hook for key deletion event */
BT_MESH_SUBNET_CB_DEFINE(sbr) = {
	.evt_handler = sbr_netkey_removed_evt,
};
