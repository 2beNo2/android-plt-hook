#ifndef CH_UTILS_H
#define CH_UTILS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <inttypes.h>

#ifndef MAX_LENGTH
#define MAX_LENGTH 1024
#endif //MAX_LENGTH

#include <android/log.h>

#define CH_DEBUG
#define TAG "CH_HOOK"

#ifdef CH_DEBUG
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif // CH_DEBUG

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  TAG, __VA_ARGS__)


#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define PAGE_START(addr)	(~(PAGE_SIZE - 1) & (addr))
#define PAGE_END(addr)      (PAGE_START(addr + sizeof(uintptr_t) - 1) + PAGE_SIZE)
#define PAGE_COVER(addr)    (PAGE_END(addr) - PAGE_START(addr))

#define SET_BIT0(addr)		(addr | 1)
#define CLEAR_BIT0(addr)	(addr & 0xFFFFFFFE)
#define TEST_BIT0(addr)		(addr & 1)

#endif //CH_UTILS_H