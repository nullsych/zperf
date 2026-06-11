/**
 * @file   BleTask.c
 * @brief  Contain main implementation for BleTask.
 * @author nullsych
 * @date   May 6, 2024
 *
 * The application exposes the Bluetooth Battery Service (BAS). Connected BLE clients can read the
 * Battery Level characteristic, which reports a simulated (!) battery percentage updated
 * periodically.
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/util.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>

#include "common/TasksApi.h"
#include "common/TasksCfg.h"
#include "common/ErrorCodes.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BleTask, LOG_LEVEL_DBG);

TASK_LOCAL const struct bt_data Ble_advertisingData[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_DIS_VAL))};

TASK_LOCAL const struct bt_data Ble_scanResponseData[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

TASK_LOCAL void Ble_connected(struct bt_conn *conn, uint8_t errCode)
{
	if (errCode) {
		LOG_ERR("Connection failed, errCode=0x%02x", errCode);
	} else {
		LOG_INF("Client connected!");
	}
}

TASK_LOCAL void Ble_disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Client disconnected, reason=0x%02x", reason);
}

TASK_LOCAL void Ble_authCancel(struct bt_conn *conn)
{
	char bleAddress[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), bleAddress, sizeof(bleAddress));

	LOG_INF("Pairing cancelled: %s", bleAddress);
}

TASK_LOCAL void Ble_notifyBatteryLevel(void)
{
	uint8_t batteryLevel = bt_bas_get_battery_level();

	batteryLevel--;

	if (!batteryLevel) {
		batteryLevel = 100U;
	}

	bt_bas_set_battery_level(batteryLevel);
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = Ble_connected,
	.disconnected = Ble_disconnected,
};

TASK_LOCAL struct bt_conn_auth_cb Ble_authenticatedCallback = {
	.cancel = Ble_authCancel,
};

TASK_LOCAL int Ble_ready(void)
{
	int errCode;

	LOG_INF("Begin advertising...");

	errCode = bt_le_adv_start(BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONN, BT_GAP_ADV_FAST_INT_MIN_2,
						  BT_GAP_ADV_FAST_INT_MAX_2, NULL),
				  Ble_advertisingData, ARRAY_SIZE(Ble_advertisingData),
				  Ble_scanResponseData, ARRAY_SIZE(Ble_scanResponseData));
	if (errCode) {
		LOG_ERR("Advertising failed to start, errCode=%d", errCode);
		return errCode;
	}

	LOG_INF("Advertising successfully started!");
	return errCode;
}

TASK_LOCAL void Ble_main(void)
{
	int errCode;

	errCode = bt_enable(NULL);

	if (errCode) {
		LOG_ERR("Bluetooth initialization failed, errCode=%d", errCode);
	}

	errCode = Ble_ready();
	if (errCode) {
		LOG_ERR("Bluetooth not ready, errCode=%d", errCode);
	}

	errCode = bt_conn_auth_cb_register(&Ble_authenticatedCallback);
	if (errCode) {
		LOG_ERR("Authenticated callback register failed, errCode=%d", errCode);
	}

	for (;;) {
		k_msleep(LED_TASK_PERIOD);

		/* Battery level simulation */
		Ble_notifyBatteryLevel();
	}
}

TASK_LOCAL TASK_ENTRY_POINT void Task_BLE_entry(void)
{
	Ble_main();
}

/* Define the thread using a macro */
K_THREAD_DEFINE(Task_BLE, BLE_TASK_STACKSIZE, Task_BLE_entry, NULL, NULL, NULL, BLE_TASK_PRIORITY,
		0, 0);

/* EOF */
