
# Khronos
#
# Note: in Android the linker is unable or unwilling to resolve
# dynamically using already loaded symbols. The libraries here
# have quite a few circular dependencies, and so the only way
# to make it work seems to be to have everything static.

set(EGL_SOURCE
   egl/egl_client_config.c
   egl/egl_client_context.c
   egl/egl_client.c
   egl/egl_client_get_proc.c
   egl/egl_client_surface.c
   ext/egl_brcm_driver_monitor_client.c
   ext/egl_brcm_perf_monitor_client.c
   ext/egl_brcm_global_image_client.c
   ext/egl_brcm_flush_client.c
   ext/egl_khr_image_client.c
   ext/egl_khr_sync_client.c
   ext/gl_oes_egl_image_client.c
   ext/egl_khr_lock_surface_client.c
   ext/ext_gl_debug_marker.c
   common/khrn_int_image.c
   common/khrn_int_util.c
   common/khrn_options.c
   common/khrn_client_global_image_map.c
   common/linux/khrn_client_rpc_linux.c
   common/linux/khrn_client_platform_linux.c
   vg/vg_int_mat3x3.c
   vg/vg_client.c
   common/khrn_client.c
   ext/egl_openmaxil_client.c
#   ../vmcs_host/vc_vchi_dispmanx.c
   ext/gl_oes_draw_texture_client.c
   ext/gl_oes_query_matrix_client.c
   ext/gl_oes_framebuffer_object.c
   ext/gl_oes_map_buffer.c
   ext/gl_oes_matrix_palette_client.c)
set(GLES_SOURCE
   glxx/glxx_client.c)
set(VG_SOURCE
   vg/vg_client.c
   vg/vg_int_mat3x3.c)
set(WFC_SOURCE
   wf/wfc_client_stream.c
   wf/wfc_client.c
   wf/wfc_client_server_api.c
   wf/wfc_client_ipc.c
   common/openwfc/khrn_client_platform_openwfc.c)
set(CLIENT_SOURCE
   common/khrn_client_pointermap.c
   common/khrn_client_vector.c
   common/khrn_int_hash.c
   common/khrn_int_hash_asm.s
   common/khrn_client_cache.c)
   
set(VCHOSTIF_SOURCE
            ../vmcs_host/linux/vcfilesys.c ../vmcs_host/linux/vcmisc.c
            ../vmcs_host/vc_vchi_gencmd.c ../vmcs_host/vc_vchi_filesys.c
            ../vmcs_host/vc_vchi_tvservice.c ../vmcs_host/vc_vchi_cecservice.c
            ../vmcs_host/vc_vchi_dispmanx.c ../vmcs_host/vc_service_common.c)

set(BCMHOST_SOURCE ../../host_applications/linux/libs/bcm_host/bcm_host.c)

set(VCHQ_ARM_SOURCE ../vchiq_arm/vchiq_lib.c ../vchiq_arm/vchiq_util.c)

set (VCOS_SOURCE
   ../vcos/pthreads/vcos_pthreads.c
   ../vcos/pthreads/vcos_dlfcn.c
   ../vcos/generic/vcos_generic_event_flags.c
   ../vcos/generic/vcos_mem_from_malloc.c
   ../vcos/generic/vcos_generic_named_sem.c
   ../vcos/generic/vcos_generic_safe_string.c
   ../vcos/generic/vcos_generic_reentrant_mtx.c
   ../vcos/generic/vcos_abort.c
   ../vcos/generic/vcos_cmd.c
   ../vcos/generic/vcos_init.c
   ../vcos/generic/vcos_msgqueue.c
   ../vcos/generic/vcos_logcat.c
   ../vcos/generic/vcos_generic_blockpool.c )

set (GRALLOC_SOURCE
   ../../gralloc/gralloc_brcm.cpp
   )


add_library(EGL ${SHARED} ${EGL_SOURCE})
add_library(GLESv2 ${SHARED} ${GLES_SOURCE})
add_library(OpenVG ${SHARED} ${VG_SOURCE})
add_library(WFC ${SHARED} ${WFC_SOURCE})
add_library(khrn_client ${CLIENT_SOURCE})

# TODO do we need EGL_static and GLESv2_static now that khrn_static exists?
add_library(EGL_static STATIC ${EGL_SOURCE})
add_library(GLESv2_static STATIC ${GLES_SOURCE})
add_library(khrn_static STATIC
   ${EGL_SOURCE} ${GLES_SOURCE} ${VG_SOURCE} ${CLIENT_SOURCE} ${VCHOSTIF_SOURCE} ${BCMHOST_SOURCE} ${VCHQ_ARM_SOURCE} ${VCOS_SOURCE} ${GRALLOC_SOURCE})
   
add_library(bcm_host_static STATIC
   ${VCHOSTIF_SOURCE} ${BCMHOST_SOURCE} ${VCHQ_ARM_SOURCE} ${VCOS_SOURCE} ${GRALLOC_SOURCE})

target_link_libraries(EGL khrn_client vchiq_arm vcos bcm_host -lm)
target_link_libraries(GLESv2 EGL khrn_client vcos)
target_link_libraries(WFC EGL)
target_link_libraries(OpenVG EGL)

install(TARGETS EGL GLESv2 OpenVG WFC khrn_client DESTINATION lib)
install(TARGETS EGL_static GLESv2_static khrn_static DESTINATION lib)
