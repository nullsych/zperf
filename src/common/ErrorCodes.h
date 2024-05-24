/**
 * @file   ErrorCodes.h
 * @brief  Error codes for common project usage.
 * @author kvant6ubl
 * @date   May 6, 2024
 *
 */

#ifndef SRC_COMMON_ERROR_CODES_H
#define SRC_COMMON_ERROR_CODES_H

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

#endif /* SRC_COMMON_ERROR_CODES_H */