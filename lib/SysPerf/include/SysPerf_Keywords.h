/**
 * @file   SysPerf_Keywords.h
 * @brief
 * @author kvant6ubl
 * @date   Nov 16, 2024
 *
 */

#ifndef SYS_PERF_KEYWORDS_H
#define SYS_PERF_KEYWORDS_H

/*******************************************************************************************************************
*                                            SYSPERF KEYWORDS                                                      *
*******************************************************************************************************************/
#define SPK_LOCAL                     static
#define SPK_INLINE                    inline
#define SPK_GLOBAL                    extern
#define SPK_LOCAL_INLINE              static inline
#define SPK_GLOBAL_INLINE             extern inline
#define SPK_PACKED                    __packed
#define SPK_ALIGNED(x)                __aligned(x)

#define RTOS_CALLED                   /* Called within RTOS hooks (Pre, Post, Idle, Shutdown etc.)     */
#define USER_CALLED                   /* Called within user application code                           */
#define HOOK_CALLED                   /* Called within SysPerf's calculation logic                     */
#define RTOS_WRAPPER                  /* Called within RTOS API (e.g. callbacks)                       */

#endif /* SYS_PERF_KEYWORDS_H */