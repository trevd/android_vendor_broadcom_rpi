ifeq ($(strip $(TARGET_EGL_PREBUILT)),true)

$(info TARGET_EGL_PREBUILT $(TARGET_EGL_PREBUILT))

# libkhrn_static.a - We use BUILD_MULTI_PREBUILT here to create 
# obj/STATIC_LIBRARIES/libkhrn_static_intermediates so we can 
# reference the library in other project 
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := libkhrn_static.a
LOCAL_MODULE_TAGS := optional

include $(BUILD_MULTI_PREBUILT)




include $(CLEAR_VARS)

include $(ANDROID_BUILD_TOP)/hardware/broadcom/$(TARGET_BOARD_PLATFORM)/cflags.mk

LOCAL_C_INCLUDES += hardware/broadcom/$(TARGET_BOARD_PLATFORM)/include

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/egl
LOCAL_MODULE := libEGL_bcm2708

LOCAL_SRC_FILES:= dummy.c \
					brcm_egl.c \
					../../../../hardware/broadcom/$(TARGET_BOARD_PLATFORM)/libEGL/common/khrn_options.c

LOCAL_SHARED_LIBRARIES:=libvcos libvchiq_arm  libvchostif liblog
LOCAL_WHOLE_STATIC_LIBRARIES :=  libkhrn_static 
LOCAL_LDFLAGS := -Wl,--no-warn-shared-textrel
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


endif
