/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_SUBSYS_BLUETOOTH_MESH_SBT_BT_H_
#define ZEPHYR_SUBSYS_BLUETOOTH_MESH_SBT_BT_H_

#include <stdint.h>
#include <sys/types.h>
#include <zephyr/kernel.h>

enum sbr_bt_direction {
    SBR_BT_DIRECTION_PROHIBITED = 0,
    SBR_BT_DIRECTION_ADDR1_TO_ADDR2 = 1,
    SBR_BT_DIRECTION_ADDR1_TOFR_ADDR2 = 2,
    SBR_BT_DIRECTION_RFU_MAX = 3,
};

bool sbr_bt_enable_get(void);

int sbr_bt_enable_set(bool enable);

int sbr_bt_add(enum sbr_bt_direction direction, uint16_t netidx1, uint16_t netidx2,
                        uint16_t addr1, uint16_t addr2);

int sbr_bt_check(uint16_t src, uint16_t dst, uint16_t netidx, uint16_t *new_netidx);







#endif /* ZEPHYR_SUBSYS_BLUETOOTH_MESH_SBT_BT_H_ */
