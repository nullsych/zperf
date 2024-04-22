#ifndef __COMMON_ERROR_CODES_H__
#define __COMMON_ERROR_CODES_H__

#define NRF_OK      0u
#define NRF_ERROR   0u

/* Bluetooth error codes */
typedef enum
{
    NRF_BT_OK  = 0,
    NRF_BT_NOK = 1
} Ble_Error_t;

/* Dev tree error codes */
typedef enum
{
    NRF_LED_OK  = 0,
    NRF_LED_NOK = 1
} Led_Error_t;

#endif /* __COMMON_ERROR_CODES_H__ */