LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := test
LOCAL_SRC_FILES := main.c chook.c
LOCAL_LDLIBS := -lc -ldl
LOCAL_CFLAGS += -v

include $(BUILD_EXECUTABLE)