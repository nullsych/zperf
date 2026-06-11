/**
 * @file   zperf_keywords.h
 * @brief
 * @author nullsych
 * @date   Nov 16, 2024
 *
 */

#ifndef ZPERF_KEYWORDS_H
#define ZPERF_KEYWORDS_H

/*******************************************************************************************************************
 *                                            KEYWORDS
 *******************************************************************************************************************/

#define ZP_LOCAL         static
#define ZP_INLINE        inline
#define ZP_GLOBAL        extern
#define ZP_LOCAL_INLINE  static inline
#define ZP_GLOBAL_INLINE extern inline
#define ZP_PACKED        __packed
#define ZP_ALIGNED(x)    __aligned(x)

#define ZP_RTOS_CALLED  /* Called within RTOS hooks (Pre, Post, Idle, Shutdown etc.)     */
#define ZP_USER_CALLED  /* Called within user application code                           */
#define ZP_HOOK_CALLED  /* Called within zperf's calculation logic                       */
#define ZP_RTOS_WRAPPER /* Called within RTOS API (e.g. callbacks)                       */

#endif /* ZPERF_KEYWORDS_H */
