/**
 * @file   BleTask.c
 * @brief  Contain main implementation for BleTask.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 * The application in the task specifically exposes the BAS (Battery Service) GATT Service.
 * Once a device connects it will read battery level value and return the percentage of battery level.
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#include "common/TasksApi.h"
#include "common/TasksCfg.h"
#include "common/ErrorCodes.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(BleTask, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(BLE_Task_stack_area, BLE_TASK_STACKSIZE);
TASK_LOCAL struct k_thread Ble_zthread_data;
TASK_LOCAL k_tid_t Ble_zid;

TASK_LOCAL uint8_t Ble_taskId = BLE_TASKID;

TASK_LOCAL const struct bt_data Ble_advertisingData[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
                  BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
                  BT_UUID_16_ENCODE(BT_UUID_DIS_VAL))};

TASK_LOCAL void Ble_connected(struct bt_conn *conn, uint8_t errCode)
{
    if (errCode)
    {
        LOG_ERR("Connection failed, errCode=0x%02x", errCode);
    }
    else
    {
        LOG_INF("Connected!");
    }
}

TASK_LOCAL void Ble_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected, reason=0x%02x", reason);
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

    if (!batteryLevel)
    {
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

    errCode = bt_le_adv_start(BT_LE_ADV_CONN_NAME, Ble_advertisingData, ARRAY_SIZE(Ble_advertisingData), NULL, 0);
    if (errCode)
    {
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

    if (errCode)
    {
        LOG_ERR("Bluetooth initialization failed, errCode=%d", errCode);
        // TODO: add handle code
    }

    errCode = Ble_ready();
    if (errCode)
    {
        LOG_ERR("Bluetooth not ready, errCode=%d", errCode);
        // TODO: add handle code
    }

    errCode = bt_conn_auth_cb_register(&Ble_authenticatedCallback);
    if (errCode)
    {
        LOG_ERR("Authenticated callback register failed, errCode=%d", errCode);
        // TODO: add handle code
    }

    for (;;)
    {
        k_msleep(LED_TASK_PERIOD);

        /* Battery level simulation */
        Ble_notifyBatteryLevel();
    }
}

TASK_LOCAL void Task_BLE(void *unused_arg1, void *unused_arg2, void *unused_arg3)
{
    ARG_UNUSED(unused_arg1);
    ARG_UNUSED(unused_arg2);
    ARG_UNUSED(unused_arg3);

    k_thread_custom_data_set((void *)Ble_taskId);

    Ble_main();
}

void Ble_createTask(void)
{
    Ble_zid = k_thread_create(&Ble_zthread_data, BLE_Task_stack_area,
                              K_THREAD_STACK_SIZEOF(BLE_Task_stack_area),
                              Task_BLE, NULL, NULL, NULL,
                              BLE_TASK_PRIORITY, 0, K_FOREVER);

    k_thread_name_set(Ble_zid, "Task_BLE");

    k_thread_start(Ble_zid);
}

/* EOF */