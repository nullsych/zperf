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
#define SP_LOCAL                      static
#define SP_EXTERN                     extern
#define SP_LOCAL_INLINE               static inline
#define SP_PACKED                     __packed
#define SP_ALIGNED(x)                 __aligned(x)

#define RTOS_CALLED                   /* Called within RTOS hooks (Pre, Post, Idle, Shutdown etc.)     */
#define USER_CALLED                   /* Called within user application code                           */
#define HOOK_CALLED                   /* Called within SysPerf's calculation logic                     */

#endif /* SYS_PERF_KEYWORDS_H */