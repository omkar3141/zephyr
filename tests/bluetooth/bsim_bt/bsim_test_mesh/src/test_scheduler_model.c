/*
 * Copyright (c) 2021 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mesh_test.h"
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>
#include <sys/reboot.h>
#ifndef REMOVE_SCHED
#include <fs/fs.h>
#include <ff.h>
#endif

#define LOG_MODULE_NAME test_scheduler

#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define WAIT_TIME 60 /*seconds*/

#ifndef REMOVE_SCHED
static FATFS fat_fs;

/* mounting info */
static struct fs_mount_t fatfs_mnt_srv = {
	.type = FS_FATFS,
	.mnt_point = "/RAM:",
	.fs_data = &fat_fs,
};

static struct fs_mount_t fatfs_mnt_cli = {
	.type = FS_FATFS,
	.mnt_point = "/RAM:",
	.fs_data = &fat_fs,
};

void scheduler_action_set_cb(struct bt_mesh_scheduler_srv *srv,
			     struct bt_mesh_msg_ctx *ctx,
			     uint8_t idx,
			     struct bt_mesh_schedule_entry *entry)
{
}
#endif

static const struct bt_mesh_test_cfg scheduler_srv_model_cfg = {
	.addr = 0x0001,
	.dev_key = { 0x01 },
};

static const struct bt_mesh_test_cfg scheduler_cli_model_cfg = {
	.addr = 0x0002,
	.dev_key = { 0x02 },
};

static void mount_settings_area(struct fs_mount_t * fs_mnt)
{
#ifndef REMOVE_SCHED
	int err = fs_mount(fs_mnt);
	LOG_INF("Mount point creation status: %i", err);
#endif
}

static void test_server_init(void)
{
	LOG_INF("%s", __func__);
}

static void test_client_init(void)
{
	LOG_INF("%s", __func__);
}

static void test_server_check_init(void)
{
	mount_settings_area(&fatfs_mnt_srv);
	bt_mesh_test_cfg_set(&scheduler_srv_model_cfg, WAIT_TIME);
	bt_mesh_test_setup();

	LOG_INF("sleeping for 5 seconds");
	k_sleep(K_SECONDS(5));

	/* This does not really work, as reboot support is not
	 * available in native posix execution environment.
	 * After this call test will terminate. */
	sys_reboot(SYS_REBOOT_COLD);
	LOG_INF("sleeping for 5 seconds");
	k_sleep(K_SECONDS(5));

	PASS();
}

static void test_client_check_init(void)
{
	mount_settings_area(&fatfs_mnt_cli);
	bt_mesh_test_cfg_set(&scheduler_cli_model_cfg, WAIT_TIME);
	bt_mesh_test_setup();

	LOG_INF("sleeping for 10 seconds");
	k_sleep(K_SECONDS(10));

	PASS();
}

#define TEST_CASE(role, name, description)                                     \
	{                                                                      \
		.test_id = "scheduler_model_" #role "_" #name,                 \
		.test_descr = description,                                     \
		.test_post_init_f = test_##role##_init,                        \
		.test_tick_f = bt_mesh_test_timeout,                           \
		.test_main_f = test_##role##_##name,                           \
	}

static const struct bst_test_instance test_scheduler[] = {
	TEST_CASE(server, check_init,        "Initialization of the scheduler server model"),
	TEST_CASE(client, check_init,        "Initialization of the scheduler client model"),
	BSTEST_END_MARKER
};

struct bst_test_list *test_scheduler_model_install(struct bst_test_list *tests)
{
	return bst_add_tests(tests, test_scheduler);
}
