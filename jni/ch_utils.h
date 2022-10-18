#ifndef CH_UTILS_H
#define CH_UTILS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef MAX_LENGTH
#define MAX_LENGTH 1024
#endif //MAX_LENGTH

#ifdef __cplusplus
extern "C" {
#endif

void* ch_utils_get_module_base(pid_t pid, const char* module_name);

#ifdef __cplusplus
}
#endif



#include <android/log.h>

#define CH_DEBUG
#ifdef CH_DEBUG
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "CH_HOOK", __VA_ARGS__)
#else
#define LOGD(...)
#endif // CH_DEBUG

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "CH_HOOK", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  "CH_HOOK", __VA_ARGS__)

#endif //CH_UTILS_H