/**
 * @file   nrf_perf_keywords.h
 * @brief
 * @author nullsych
 * @date   Nov 16, 2024
 *
 */

#ifndef NRF_PERF_KEYWORDS_H
#define NRF_PERF_KEYWORDS_H

/*******************************************************************************************************************
*                                            nrf_perf KEYWORDS                                                      *
*******************************************************************************************************************/
#define NP_LOCAL                     static
#define NP_INLINE                    inline
#define NP_GLOBAL                    extern
#define NP_LOCAL_INLINE              static inline
#define NP_GLOBAL_INLINE             extern inline
#define NP_PACKED                    __packed
#define NP_ALIGNED(x)                __aligned(x)

#define NP_RTOS_CALLED                   /* Called within RTOS hooks (Pre, Post, Idle, Shutdown etc.)     */
#define NP_USER_CALLED                   /* Called within user application code                           */
#define NP_HOOK_CALLED                   /* Called within nrf_perf's calculation logic                     */
#define NP_RTOS_WRAPPER                  /* Called within RTOS API (e.g. callbacks)                       */

#endif /* NRF_PERF_KEYWORDS_H */