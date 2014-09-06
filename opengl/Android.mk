#
# Copyright (C) 2014 Trevor Drake <trevd1234@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License v2.0 Only as 
# published by the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


LOCAL_PATH := $(call my-dir)

opengl_cflags := \
		-DANDROID -DEGL_EGLEXT_ANDROID_STRUCT_HEADER \
		-DEGL_SERVER_DISPMANX \
		-DHAVE_VMCS_CONFIG -DOMX_SKIP64BIT -DOpenVG_EXPORTS \
		-DTV_SUPPORTED_MODE_NO_DEPRECATED -DUSE_VCHIQ_ARM \
		-DVCHI_BULK_ALIGN=1 -DVCHI_BULK_GRANULARITY=1 \
		-D_FILE_OFFSET_BITS=64 -D_HAVE_SBRK -D_LARGEFILE64_SOURCE \
		-D_LARGEFILE_SOURCE -D_REENTRANT -D__VIDEOCORE4__

opengl_c_includes := \
		$(LOCAL_PATH)/host_applications/framework \
		$(LOCAL_PATH)/host_applications/linux/libs/bcm_host/include \
		$(LOCAL_PATH)/interface/vcos \
		$(LOCAL_PATH)/interface/vcos/pthreads \
		$(LOCAL_PATH)/interface/vmcs_host/linux \
		$(LOCAL_PATH)/interface/vmcs_host \
		$(LOCAL_PATH)/interface/vmcs_host/khronos \
		$(LOCAL_PATH)/interface/khronos/include \
		$(LOCAL_PATH)/interface/vchiq_arm \
		$(LOCAL_PATH)/host_support/include \
		hardware/broadcom/bcm2708/gralloc \
		system/core/include \

egl_src_files := \
		interface/khronos/egl/egl_client_config.c \
		interface/khronos/egl/egl_client_context.c \
		interface/khronos/egl/egl_client.c \
		interface/khronos/egl/egl_client_get_proc.c \
		interface/khronos/egl/egl_client_surface.c \
		interface/khronos/ext/egl_brcm_driver_monitor_client.c \
		interface/khronos/ext/egl_brcm_perf_monitor_client.c \
		interface/khronos/ext/egl_brcm_global_image_client.c \
		interface/khronos/ext/egl_brcm_flush_client.c \
		interface/khronos/ext/egl_khr_image_client.c \
		interface/khronos/ext/egl_khr_sync_client.c \
		interface/khronos/ext/gl_oes_egl_image_client.c \
		interface/khronos/ext/egl_khr_lock_surface_client.c \
		interface/khronos/ext/ext_gl_debug_marker.c \
		interface/khronos/common/khrn_int_image.c \
		interface/khronos/common/khrn_int_util.c \
		interface/khronos/common/khrn_options.c \
		interface/khronos/common/khrn_client_global_image_map.c \
		interface/khronos/common/linux/khrn_client_rpc_linux.c \
		interface/khronos/common/linux/khrn_client_platform_linux.c \
		interface/khronos/common/khrn_client.c \
		interface/khronos/ext/egl_openmaxil_client.c \
		interface/khronos/ext/gl_oes_draw_texture_client.c \
		interface/khronos/ext/gl_oes_query_matrix_client.c \
		interface/khronos/ext/gl_oes_framebuffer_object.c \
		interface/khronos/ext/gl_oes_map_buffer.c \
		interface/khronos/ext/gl_oes_matrix_palette_client.c \

gles_src_files := interface/khronos/glxx/glxx_client.c

vg_src_files := \
		interface/khronos/vg/vg_client.c \
		interface/khronos/vg/vg_int_mat3x3.c

client_src_files := \
		interface/khronos/common/khrn_client_pointermap.c \
		interface/khronos/common/khrn_client_vector.c \
		interface/khronos/common/khrn_int_hash.c \
		interface/khronos/common/khrn_int_hash_asm.s \
		interface/khronos/common/khrn_client_cache.c

vchostif_src_files := \
		interface/vmcs_host/linux/vcfilesys.c \
		interface/vmcs_host/linux/vcmisc.c \
		interface/vmcs_host/vc_vchi_gencmd.c \
		interface/vmcs_host/vc_vchi_filesys.c \
		interface/vmcs_host/vc_vchi_tvservice.c \
		interface/vmcs_host/vc_vchi_cecservice.c \
		interface/vmcs_host/vc_vchi_dispmanx.c \
		interface/vmcs_host/vc_service_common.c

bcm_host_src_files := host_applications/linux/libs/bcm_host/bcm_host.c

vchiq_arm_src_files := \
		interface/vchiq_arm/vchiq_lib.c \
		interface/vchiq_arm/vchiq_util.c

vcos_src_files := \
		interface/vcos/pthreads/vcos_pthreads.c \
		interface/vcos/pthreads/vcos_dlfcn.c \
		interface/vcos/generic/vcos_generic_event_flags.c \
		interface/vcos/generic/vcos_mem_from_malloc.c \
		interface/vcos/generic/vcos_generic_named_sem.c \
		interface/vcos/generic/vcos_generic_safe_string.c \
		interface/vcos/generic/vcos_generic_reentrant_mtx.c \
		interface/vcos/generic/vcos_abort.c \
		interface/vcos/generic/vcos_cmd.c \
		interface/vcos/generic/vcos_init.c \
		interface/vcos/generic/vcos_msgqueue.c \
		interface/vcos/generic/vcos_logcat.c \
		interface/vcos/generic/vcos_generic_blockpool.c

include $(CLEAR_VARS)
LOCAL_MODULE := libvc4
LOCAL_CFLAGS := $(opengl_cflags)
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE_PATH:= $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
LOCAL_SRC_FILES := \
			$(vchostif_src_files) \
			$(bcm_host_src_files) \
			$(vchiq_arm_src_files) \
			$(vcos_src_files)
LOCAL_C_INCLUDES := $(opengl_c_includes)
LOCAL_SHARED_LIBRARIES := liblog libdl
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libGLES_bcm2708
LOCAL_CFLAGS := $(opengl_cflags)
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE_PATH:= $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/egl
LOCAL_SRC_FILES := \
			$(client_src_files) \
			$(vg_src_files) \
			$(gles_src_files) \
			$(egl_src_files)
LOCAL_C_INCLUDES := $(opengl_c_includes)
LOCAL_SHARED_LIBRARIES := liblog libdl libvc4 libhardware
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := libvcsm
LOCAL_CFLAGS := $(opengl_cflags)
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE_PATH:= $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
LOCAL_SRC_FILES := \
			host_applications/linux/libs/sm/user-vcsm.c
LOCAL_C_INCLUDES := $(opengl_c_includes) \
					$(LOCAL_PATH)/host_applications/linux/libs/sm \
					$(LOCAL_PATH)/host_applications/linux/kernel_headers/
LOCAL_SHARED_LIBRARIES := liblog libdl libvc4
include $(BUILD_SHARED_LIBRARY)
