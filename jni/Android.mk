LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := test
LOCAL_SRC_FILES := main.c chook.c ch_elf.c
LOCAL_LDLIBS := -lc -ldl -llog
LOCAL_CFLAGS += -o0
include $(BUILD_EXECUTABLE)