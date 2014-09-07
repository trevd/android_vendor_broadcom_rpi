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


ifeq ($(strip $(TARGET_BOARD_OPENGL_CFLAGS)),)
	$(error TARGET_BOARD_OPENGL_CFLAGS no set)
endif

ifeq ($(strip $(TARGET_BOARD_OPENGL_C_INCLUDES)),)
	$(error TARGET_BOARD_OPENGL_C_INCLUDES no set)
endif

					
		
####################   libvc4    ######################################



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

vcsm_src_files := host_applications/linux/libs/sm/user-vcsm.c

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
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE_PATH:= $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)
LOCAL_CFLAGS := $(TARGET_BOARD_OPENGL_CFLAGS)
LOCAL_C_INCLUDES := $(TARGET_BOARD_OPENGL_C_INCLUDES)
LOCAL_SRC_FILES := \
			$(vchostif_src_files) \
			$(bcm_host_src_files) \
			$(vcsm_src_files) \
			$(vchiq_arm_src_files) \
			$(vcos_src_files)
LOCAL_SHARED_LIBRARIES := liblog libdl
include $(BUILD_SHARED_LIBRARY)

########################################################################


##################  libGLES_2708 #######################################

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


include $(CLEAR_VARS)
LOCAL_MODULE := libGLES_bcm2708
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE_PATH:= $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/egl
LOCAL_CFLAGS := $(TARGET_BOARD_OPENGL_CFLAGS)
LOCAL_C_INCLUDES := $(TARGET_BOARD_OPENGL_C_INCLUDES)
LOCAL_SRC_FILES := \
			$(client_src_files) \
			$(vg_src_files) \
			$(gles_src_files) \
			$(egl_src_files)
LOCAL_SHARED_LIBRARIES := liblog libvc4 libhardware
include $(BUILD_SHARED_LIBRARY)

########################################################################
