/*
 * Copyright (c) 2021 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mesh_test.h"
#include <bluetooth/mesh.h>
#include <bluetooth/mesh/models.h>
#include <sys/reboot.h>

#define LOG_MODULE_NAME test_persistence

#include <logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define WAIT_TIME 60 /*seconds*/

static const struct bt_mesh_test_cfg scheduler_srv_model_cfg = {
	.addr = 0x0001,
	.dev_key = { 0x01 },
};

static const struct bt_mesh_test_cfg load_cfg = {
	.addr = 0x0ADD,
	.dev_key = { 0xD },
	.net_idx = 0x077
};

static void test_provisioning_sv_init(void)
{
	LOG_INF("%s", __func__);
}

static void test_provisioning_ld_init(void)
{
	LOG_INF("%s", __func__);
}

/* This function is used just for debug and understanding */
static int read_dir(char * dirpath)
{
	struct fs_dir_t dirp = {0};
	int err = fs_opendir(&dirp, dirpath);

	if (!err)
	{
		struct fs_dirent dentry;
		err = fs_readdir(&dirp, &dentry);

		while (!err) {
			if (dentry.name[0] != 0) {
				char innerpath[100] = {0};
				strcpy(innerpath, dirpath);
				strcat(innerpath, "/");
				strcat(innerpath, dentry.name);

				if (dentry.type == FS_DIR_ENTRY_DIR) {
					err = read_dir(innerpath);
				}
				else {
					LOG_INF("Size: %4d Entry: %s", dentry.size, innerpath);
					struct fs_file_t fp = {0};
					err = fs_open(&fp, innerpath, FS_O_READ);
					if (!err) {
						char filedata[1000];
						ssize_t n = fs_read(&fp, &filedata, ARRAY_SIZE(filedata));
						LOG_HEXDUMP_INF(filedata, n , "raw data");

						err = fs_close(&fp);
					}
				}
				err = fs_readdir(&dirp, &dentry);
			}
			else {
				break;
			}
		};

		return fs_closedir(&dirp);
	}
	return err;
}

struct key_checker_data {
	const char *key;
	const void *exp_value;
	int exp_value_len;
};


typedef int (*key_value_checker_cb)(const char *key, size_t keylen,
				    const uint8_t *data, ssize_t datalen);

static key_value_checker_cb key_test_cb;
static bool key_checked;

int set_load_direct_cb(const char *key,
		       size_t len,
		       settings_read_cb read_cb,
		       void *cb_arg,
		       void *param)
{
	uint8_t data[100];

	LOG_INF("key: %s", key);
	ssize_t bytes = read_cb(cb_arg, &data, sizeof(data));
	LOG_HEXDUMP_INF(data, bytes, "data:");

	struct key_checker_data *validate = (struct key_checker_data *)param;

	if(strcmp(key, validate->key) == 0) {
		key_checked = true;

		if (!(bytes == validate->exp_value_len &&
   	     	     memcmp(data, validate->exp_value, bytes) == 0)) {
			FAIL("Validation failed for key %s", key);
			return -1;
		}
	}

	return 0;
}

static int persistent_values_check(struct key_checker_data *validater)
{

	key_checked = false;
	settings_load_subtree_direct("bt", set_load_direct_cb, validater);
	if (!key_checked) {
		FAIL("Not found. Key %s", validater->key);
		return -1;
	}

	return 0;
}

static struct net_key_val {
	uint8_t kr_flag:1,
		kr_phase:7;
	uint8_t val[2][16];
} __packed;

static struct iv_val {
	uint32_t iv_index;
	uint8_t  iv_update:1,
	      iv_duration:7;
} __packed;

static struct net_val {
	uint16_t primary_addr;
	uint8_t  dev_key[16];
} __packed;


extern const uint8_t test_net_key[16];

static void test_provisioning_sv_check_save(void)
{
	struct key_checker_data validater;

	mount_settings_area();
	bt_mesh_test_cfg_set(&scheduler_srv_model_cfg, WAIT_TIME);
	bt_mesh_test_setup(false);

	/* print raw dump of settings file */
	char dir_path[100] = CONFIG_SETTINGS_FS_DIR;
	read_dir(dir_path);

	/** check stored provisioning data **/
	/* check stored netkey matches with expected value */
	struct net_key_val keyval = {0};
	keyval.kr_flag = 0;
	keyval.kr_phase = 0;
	memcpy(keyval.val[0], test_net_key, sizeof(test_net_key));

	validater.key = "mesh/NetKey/0";
	validater.exp_value = &keyval;
	validater.exp_value_len = sizeof(keyval);

	persistent_values_check(&validater);

	PASS();
}

static void test_provisioning_ld_check_load(void)
{
	int err;

	mount_settings_area();
	settings_subsys_init();

	/* Provision the device */
	/* add Netkey with index 0x07, key[16] = {0xAA}, kr=0 */
	char keypath[20];
	uint8_t net_idx = load_cfg.net_idx;
	struct net_key_val key = {0};
	key.kr_flag = 0;
	key.kr_phase = 0;
	memset(&key.val[0], 0xAA, 16);

	snprintk(keypath, sizeof(keypath), "bt/mesh/NetKey/%x", net_idx);
	err = settings_save_one(keypath, &key, sizeof(key));
	if (err) {
		FAIL("Settings save failed %d", err);
	}

	/* set IV */
	struct iv_val iv;
	iv.iv_index = 1111;
	iv.iv_update = 0;
	iv.iv_duration = 5;
	err = settings_save_one("bt/mesh/IV", &iv, sizeof(iv));
	if (err) {
		FAIL("Settings save failed %d", err);
	}

	/* set addr and devkey */
	struct net_val net;
	net.primary_addr = load_cfg.addr;
	memset(net.dev_key, load_cfg.dev_key, 16);
	err = settings_save_one("bt/mesh/Net", &net, sizeof(net));
	if (err) {
		FAIL("Settings save failed %d", err);
	}

	/* TODO: LATER: bt_mesh_provision() also does this `bt_mesh_lpn_group_add(BT_MESH_ADDR_ALL_NODES)`
	 * not sure why is this added there. Also, there seems to be a bug, this setting will not
	 * be restored upon bootup since this is non-permenent. */
	bt_mesh_test_cfg_set(&load_cfg, WAIT_TIME);

	/* This should work without fail, as it contains additional config client API calls that
	 * should work as is since device is booting up in already provisioned state. */
	bt_mesh_test_setup(false);

	PASS();
}

#define TEST_CASE(role, name, description)                                     \
	{                                                                      \
		.test_id = "persistence_" #role "_" #name,                     \
		.test_descr = description,                                     \
		.test_post_init_f = test_##role##_init,                        \
		.test_tick_f = bt_mesh_test_timeout,                           \
		.test_main_f = test_##role##_##name,                           \
	}

static const struct bst_test_instance test_persistence[] = {
	TEST_CASE(provisioning_sv, check_save, "Verify saved provisioning data - netkey"),
	TEST_CASE(provisioning_ld, check_load, "Preload provisioning data"),
	BSTEST_END_MARKER
};

struct bst_test_list *test_persistence_install(struct bst_test_list *tests)
{
	return bst_add_tests(tests, test_persistence);
}
