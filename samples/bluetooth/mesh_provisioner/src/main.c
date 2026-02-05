/*
 * Copyright (c) 2019 Tobias Svehagen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/settings/settings.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh.h>
#include <zephyr/drivers/gpio.h>

#define SW0_NODE	DT_ALIAS(sw0)
#define SW3_NODE	DT_ALIAS(sw3)

#define SCAN_UUID_LIST_MAX  60
#define SCAN_DURATION_MS    (300 * 1000)  /* 5 minutes or until 60 UUIDs */

static const uint16_t net_idx;
static const uint16_t app_idx;
static uint16_t self_addr = 1, node_addr;
static const uint8_t dev_uuid[16] = { 0xdd, 0xdd };
static uint8_t node_uuid[16];

#ifdef CONFIG_MESH_PROVISIONER_SCAN_ON_SW3
static uint8_t scan_uuid_list[SCAN_UUID_LIST_MAX][16];
static uint16_t scan_uuid_count;
static K_MUTEX_DEFINE(scan_list_mutex);
static atomic_t scan_list_mode;
#endif

K_SEM_DEFINE(sem_unprov_beacon, 0, 1);
K_SEM_DEFINE(sem_node_added, 0, 1);
#ifdef CONFIG_MESH_PROVISIONER_USE_SW0
K_SEM_DEFINE(sem_button_pressed, 0, 1);
#endif
#ifdef CONFIG_MESH_PROVISIONER_SCAN_ON_SW3
K_SEM_DEFINE(sem_scan_request, 0, 1);
K_SEM_DEFINE(sem_scan_uuid, 0, 1);
#endif

static struct bt_mesh_cfg_cli cfg_cli = {
};

static void health_current_status(struct bt_mesh_health_cli *cli, uint16_t addr,
				  uint8_t test_id, uint16_t cid, uint8_t *faults,
				  size_t fault_count)
{
	size_t i;

	printk("Health Current Status from 0x%04x\n", addr);

	if (!fault_count) {
		printk("Health Test ID 0x%02x Company ID 0x%04x: no faults\n",
		       test_id, cid);
		return;
	}

	printk("Health Test ID 0x%02x Company ID 0x%04x Fault Count %zu:\n",
	       test_id, cid, fault_count);

	for (i = 0; i < fault_count; i++) {
		printk("\t0x%02x\n", faults[i]);
	}
}

static struct bt_mesh_health_cli health_cli = {
	.current_status = health_current_status,
};

static const struct bt_mesh_model root_models[] = {
	BT_MESH_MODEL_CFG_SRV,
	BT_MESH_MODEL_CFG_CLI(&cfg_cli),
	BT_MESH_MODEL_HEALTH_CLI(&health_cli),
};

static const struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp mesh_comp = {
	.cid = BT_COMP_ID_LF,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

static void setup_cdb(void)
{
	struct bt_mesh_cdb_app_key *key;
	uint8_t app_key[16];
	int err;

	key = bt_mesh_cdb_app_key_alloc(net_idx, app_idx);
	if (key == NULL) {
		printk("Failed to allocate app-key 0x%04x\n", app_idx);
		return;
	}

	bt_rand(app_key, 16);

	err = bt_mesh_cdb_app_key_import(key, 0, app_key);
	if (err) {
		printk("Failed to import appkey into cdb. Err:%d\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		bt_mesh_cdb_app_key_store(key);
	}
}

static void configure_self(struct bt_mesh_cdb_node *self)
{
	struct bt_mesh_cdb_app_key *key;
	uint8_t app_key[16];
	uint8_t status = 0;
	int err;

	printk("Configuring self...\n");

	key = bt_mesh_cdb_app_key_get(app_idx);
	if (key == NULL) {
		printk("No app-key 0x%04x\n", app_idx);
		return;
	}

	err = bt_mesh_cdb_app_key_export(key, 0, app_key);
	if (err) {
		printk("Failed to export appkey from cdb. Err:%d\n", err);
		return;
	}

	/* Add Application Key */
	err = bt_mesh_cfg_cli_app_key_add(self->net_idx, self->addr, self->net_idx, app_idx,
					  app_key, &status);
	if (err || status) {
		printk("Failed to add app-key (err %d, status %d)\n", err,
		       status);
		return;
	}

	err = bt_mesh_cfg_cli_mod_app_bind(self->net_idx, self->addr, self->addr, app_idx,
					   BT_MESH_MODEL_ID_HEALTH_CLI, &status);
	if (err || status) {
		printk("Failed to bind app-key (err %d, status %d)\n", err,
		       status);
		return;
	}

	atomic_set_bit(self->flags, BT_MESH_CDB_NODE_CONFIGURED);

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		bt_mesh_cdb_node_store(self);
	}

	printk("Configuration complete\n");
}

static void configure_node(struct bt_mesh_cdb_node *node)
{
	NET_BUF_SIMPLE_DEFINE(buf, BT_MESH_RX_SDU_MAX);
	struct bt_mesh_comp_p0_elem elem;
	struct bt_mesh_cdb_app_key *key;
	uint8_t app_key[16];
	struct bt_mesh_comp_p0 comp;
	uint8_t status;
	int err, elem_addr;

	printk("Configuring node 0x%04x...\n", node->addr);

	key = bt_mesh_cdb_app_key_get(app_idx);
	if (key == NULL) {
		printk("No app-key 0x%04x\n", app_idx);
		return;
	}

	err = bt_mesh_cdb_app_key_export(key, 0, app_key);
	if (err) {
		printk("Failed to export appkey from cdb. Err:%d\n", err);
		return;
	}

	/* Add Application Key */
	err = bt_mesh_cfg_cli_app_key_add(net_idx, node->addr, net_idx, app_idx, app_key, &status);
	if (err || status) {
		printk("Failed to add app-key (err %d status %d)\n", err, status);
		return;
	}

	/* Get the node's composition data and bind all models to the appkey */
	err = bt_mesh_cfg_cli_comp_data_get(net_idx, node->addr, 0, &status, &buf);
	if (err || status) {
		printk("Failed to get Composition data (err %d, status: %d)\n",
		       err, status);
		return;
	}

	err = bt_mesh_comp_p0_get(&comp, &buf);
	if (err) {
		printk("Unable to parse composition data (err: %d)\n", err);
		return;
	}

	elem_addr = node->addr;
	while (bt_mesh_comp_p0_elem_pull(&comp, &elem)) {
		printk("Element @ 0x%04x: %u + %u models\n", elem_addr,
		       elem.nsig, elem.nvnd);
		for (int i = 0; i < elem.nsig; i++) {
			uint16_t id = bt_mesh_comp_p0_elem_mod(&elem, i);

			if (id == BT_MESH_MODEL_ID_CFG_CLI ||
			    id == BT_MESH_MODEL_ID_CFG_SRV) {
				continue;
			}
			printk("Binding AppKey to model 0x%03x:%04x\n",
			       elem_addr, id);

			err = bt_mesh_cfg_cli_mod_app_bind(net_idx, node->addr, elem_addr, app_idx,
							   id, &status);
			if (err || status) {
				printk("Failed (err: %d, status: %d)\n", err,
				       status);
			}
		}

		for (int i = 0; i < elem.nvnd; i++) {
			struct bt_mesh_mod_id_vnd id =
				bt_mesh_comp_p0_elem_mod_vnd(&elem, i);

			printk("Binding AppKey to model 0x%03x:%04x:%04x\n",
			       elem_addr, id.company, id.id);

			err = bt_mesh_cfg_cli_mod_app_bind_vnd(net_idx, node->addr, elem_addr,
							       app_idx, id.id, id.company, &status);
			if (err || status) {
				printk("Failed (err: %d, status: %d)\n", err,
				       status);
			}
		}

		elem_addr++;
	}

	atomic_set_bit(node->flags, BT_MESH_CDB_NODE_CONFIGURED);

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		bt_mesh_cdb_node_store(node);
	}

	printk("Configuration complete\n");
}

static void unprovisioned_beacon(uint8_t uuid[16],
				 bt_mesh_prov_oob_info_t oob_info,
				 uint32_t *uri_hash)
{
#ifdef CONFIG_MESH_PROVISIONER_SCAN_ON_SW3
	if (atomic_get(&scan_list_mode)) {
		bool added = false;

		k_mutex_lock(&scan_list_mutex, K_FOREVER);
		if (scan_uuid_count < SCAN_UUID_LIST_MAX) {
			for (uint16_t i = 0; i < scan_uuid_count; i++) {
				if (memcmp(scan_uuid_list[i], uuid, 16) == 0) {
					added = false;
					goto release;
				}
			}
			memcpy(scan_uuid_list[scan_uuid_count], uuid, 16);
			scan_uuid_count++;
			added = true;
		}
release:
		k_mutex_unlock(&scan_list_mutex);
		if (added) {
			k_sem_give(&sem_scan_uuid);
		}
		return;
	}
#endif
	memcpy(node_uuid, uuid, 16);
	k_sem_give(&sem_unprov_beacon);
}

static void node_added(uint16_t idx, uint8_t uuid[16], uint16_t addr, uint8_t num_elem)
{
	node_addr = addr;
	k_sem_give(&sem_node_added);
}

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.unprovisioned_beacon = unprovisioned_beacon,
	.node_added = node_added,
};

static int bt_ready(void)
{
	uint8_t net_key[16], dev_key[16];
	int err;

	err = bt_mesh_init(&prov, &mesh_comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return err;
	}

	printk("Mesh initialized\n");

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		printk("Loading stored settings\n");
		settings_load();
	}

	bt_rand(net_key, 16);

	err = bt_mesh_cdb_create(net_key);
	if (err == -EALREADY) {
		printk("Using stored CDB\n");
	} else if (err) {
		printk("Failed to create CDB (err %d)\n", err);
		return err;
	} else {
		printk("Created CDB\n");
		setup_cdb();
	}

	bt_rand(dev_key, 16);

	err = bt_mesh_provision(net_key, BT_MESH_NET_PRIMARY, 0, 0, self_addr,
				dev_key);
	if (err == -EALREADY) {
		printk("Using stored settings\n");
	} else if (err) {
		printk("Provisioning failed (err %d)\n", err);
		return err;
	} else {
		printk("Provisioning completed\n");
	}

	return 0;
}

static uint8_t check_unconfigured(struct bt_mesh_cdb_node *node, void *data)
{
	if (!atomic_test_bit(node->flags, BT_MESH_CDB_NODE_CONFIGURED)) {
		if (node->addr == self_addr) {
			configure_self(node);
		} else {
			configure_node(node);
		}
	}

	return BT_MESH_CDB_ITER_CONTINUE;
}

#ifdef CONFIG_MESH_PROVISIONER_USE_SW0
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	k_sem_give(&sem_button_pressed);
}

static void button_init(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button)) {
		printk("Error: button device %s is not ready\n", button.port->name);
		return;
	}
	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n", ret, button.port->name,
		       button.pin);
		return;
	}
	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n", ret,
		       button.port->name, button.pin);
		return;
	}
	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
}
#endif

#ifdef CONFIG_MESH_PROVISIONER_SCAN_ON_SW3
#define SCAN_THREAD_STACK_SIZE 1024
static K_THREAD_STACK_DEFINE(scan_thread_stack, SCAN_THREAD_STACK_SIZE);
static struct k_thread scan_thread_data;

static const struct gpio_dt_spec button_sw3 = GPIO_DT_SPEC_GET_OR(SW3_NODE, gpios, {0});
static struct gpio_callback button_sw3_cb;

static void button_sw3_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	k_sem_give(&sem_scan_request);
}

static void scan_thread(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	char uuid_hex_str[32 + 1];
	uint16_t last_printed;
	int64_t start_uptime;

	while (true) {
		k_sem_take(&sem_scan_request, K_FOREVER);
		k_mutex_lock(&scan_list_mutex, K_FOREVER);
		scan_uuid_count = 0;
		k_mutex_unlock(&scan_list_mutex);
		last_printed = 0;
		atomic_set(&scan_list_mode, 1);
		start_uptime = k_uptime_get();
		printk("Scanning for unprovisioned beacons (max %d unique UUIDs, %d s)...\n",
		       SCAN_UUID_LIST_MAX, SCAN_DURATION_MS / 1000);

		while (true) {
			(void)k_sem_take(&sem_scan_uuid, K_MSEC(500));

			k_mutex_lock(&scan_list_mutex, K_FOREVER);
			for (uint16_t i = last_printed; i < scan_uuid_count; i++) {
				bin2hex(scan_uuid_list[i], 16, uuid_hex_str, sizeof(uuid_hex_str));
				for (char *c = uuid_hex_str; *c; c++) {
					if (*c >= 'a' && *c <= 'f') {
						*c -= 32;
					}
				}
				printk("  [%u] %s\n", (unsigned int)(i + 1), uuid_hex_str);
			}
			last_printed = scan_uuid_count;
			k_mutex_unlock(&scan_list_mutex);

			if (scan_uuid_count >= SCAN_UUID_LIST_MAX) {
				printk("Reached %d unique UUIDs.\n", SCAN_UUID_LIST_MAX);
				break;
			}
			if ((k_uptime_get() - start_uptime) >= SCAN_DURATION_MS) {
				printk("Scan duration reached.\n");
				break;
			}
		}

		atomic_set(&scan_list_mode, 0);
		printk("Scan stopped. Total unique UUIDs: %u\n", (unsigned int)scan_uuid_count);
	}
}

static void button_sw3_init(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button_sw3)) {
		printk("sw3 (button3) not ready\n");
		return;
	}
	ret = gpio_pin_configure_dt(&button_sw3, GPIO_INPUT);
	if (ret != 0) {
		printk("sw3 configure failed: %d\n", ret);
		return;
	}
	ret = gpio_pin_interrupt_configure_dt(&button_sw3, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("sw3 interrupt configure failed: %d\n", ret);
		return;
	}
	gpio_init_callback(&button_sw3_cb, button_sw3_pressed, BIT(button_sw3.pin));
	gpio_add_callback(button_sw3.port, &button_sw3_cb);
	printk("Press button3 to start scanning for unprovisioned beacons.\n");
}
#endif

int main(void)
{
	char uuid_hex_str[32 + 1];
	int err;

	printk("Initializing...\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	printk("Bluetooth initialized\n");
	bt_ready();

#ifdef CONFIG_MESH_PROVISIONER_USE_SW0
	button_init();
#endif
#ifdef CONFIG_MESH_PROVISIONER_SCAN_ON_SW3
	k_thread_create(&scan_thread_data, scan_thread_stack,
			K_THREAD_STACK_SIZEOF(scan_thread_stack),
			scan_thread, NULL, NULL, NULL,
			K_PRIO_PREEMPT(8), 0, K_NO_WAIT);
	button_sw3_init();
#endif

	while (1) {
#ifdef CONFIG_MESH_PROVISIONER_SCAN_ON_SW3
		/* Block provisioning flow while scan (button3) is running; only scan list is printed */
		while (atomic_get(&scan_list_mode)) {
			k_msleep(100);
		}
#endif
		k_sem_reset(&sem_unprov_beacon);
		k_sem_reset(&sem_node_added);
		bt_mesh_cdb_node_foreach(check_unconfigured, NULL);

		printk("Waiting for unprovisioned beacon...\n");
		err = k_sem_take(&sem_unprov_beacon, K_SECONDS(30));
		if (err == -EAGAIN) {
			continue;
		}

		bin2hex(node_uuid, 16, uuid_hex_str, sizeof(uuid_hex_str));
		for (char *c = uuid_hex_str; *c; c++) {
			if (*c >= 'a' && *c <= 'f') {
				*c -= 32;
			}
		}

#ifdef CONFIG_MESH_PROVISIONER_USE_SW0
		k_sem_reset(&sem_button_pressed);
		printk("Device %s detected, press button 1 to provision.\n", uuid_hex_str);
		err = k_sem_take(&sem_button_pressed, K_SECONDS(30));
		if (err == -EAGAIN) {
			printk("Timed out, button 1 wasn't pressed in time.\n");
			continue;
		}
#endif

		printk("Provisioning %s\n", uuid_hex_str);
		err = bt_mesh_provision_adv(node_uuid, net_idx, 0, 0);
		if (err < 0) {
			printk("Provisioning failed (err %d)\n", err);
			continue;
		}

		printk("Waiting for node to be added...\n");
		err = k_sem_take(&sem_node_added, K_SECONDS(10));
		if (err == -EAGAIN) {
			printk("Timeout waiting for node to be added\n");
			continue;
		}

		printk("Added node 0x%04x\n", node_addr);
	}
	return 0;
}
