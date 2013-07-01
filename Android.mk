LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    Onload.c
LOCAL_C_INCLUDES := \
    $(JNI_H_INCLUDE)
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libbinder
    

    
LOCAL_MODULE_PATH:= $(LOCAL_PATH)/libmm_jni
#LOCAL_CFLAGS := -DMSOS_TYPE_LINUX
LOCAL_CFLAGS += -DUSE_ANDROID_OVERLAY
LOCAL_CFLAGS += -DAndroid_4
LOCAL_MODULE := libJLibCodeMapping
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)
