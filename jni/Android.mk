LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := gifdrawable
LOCAL_SRC_FILES := libgifdrawable.c dgif_lib.c gif_err.c gifalloc.c
LOCAL_LDLIBS    := -lm -llog -ljnigraphics -landroid

include $(BUILD_SHARED_LIBRARY)
