/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define VCOS_LOG_CATEGORY (&egl_client_log_cat)

#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/common/khrn_client_rpc.h"
#include "interface/khronos/common/khrn_int_ids.h"
#include "host_applications/linux/libs/bcm_host/include/bcm_host.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef WANT_X
#include "X11/Xlib.h"
#endif

#ifdef ANDROID
#include <hardware/gralloc.h>
#include <hardware/fb.h>
#include <hardware/hardware.h>
#include <system/graphics.h>
#include <gralloc_priv.h>

#endif
extern VCOS_LOG_CAT_T egl_client_log_cat;

extern void vc_vchi_khronos_init();

static void send_bound_pixmaps(void);
#ifdef WANT_X
static void dump_hierarchy(Window w, Window thisw, Window look, int level);
static void dump_ancestors(Window w);
#endif

//see helpers\scalerlib\scalerlib_misc.c
//int32_t scalerlib_convert_vcimage_to_display_element()
//dark blue, 1<<3 in 888
#define CHROMA_KEY_565 0x0001
//

#ifdef WANT_X
static Display *hacky_display = 0;

static XErrorHandler old_handler = (XErrorHandler) 0 ;
static int application_error_handler(Display *display, XErrorEvent *theEvent)
{
   vcos_log_trace(
   		"Ignoring Xlib error: error code %d request code %d\n",
   		theEvent->error_code,
   		theEvent->request_code) ;
   return 0 ;
}
#endif


VCOS_STATUS_T khronos_platform_semaphore_create(PLATFORM_SEMAPHORE_T *sem, int name[3], int count)
{
   char buf[64];
   vcos_snprintf(buf,sizeof(buf),"KhanSemaphore%08x%08x%08x", name[0], name[1], name[2]);
   return vcos_named_semaphore_create(sem, buf, count);
}

uint64_t khronos_platform_get_process_id()
{
   return vcos_process_id_current();
}

static bool process_attached = false;

void *platform_tls_get(PLATFORM_TLS_T tls)
{
   void *ret;

   if (!process_attached)
      /* TODO: this isn't thread safe */
   {
      vcos_log_trace("Attaching process");
      client_process_attach();
      process_attached = true;
      tls = client_tls;

      vc_vchi_khronos_init();
   }

   ret = vcos_tls_get(tls);
   if (!ret)
   {
     /* The problem here is that on VCFW, the first notification we get that a thread
       * exists at all is when it calls an arbitrary EGL function. We need to detect this
       * case and initiliase the per-thread state.
       *
       * On Windows this gets done in DllMain.
       */
      client_thread_attach();
      vcos_thread_at_exit(client_thread_detach, NULL);
      ret = vcos_tls_get(tls);
   }
   return ret;
}

void *platform_tls_get_check(PLATFORM_TLS_T tls)
{
   return platform_tls_get(tls);
}

/* ----------------------------------------------------------------------
 * workaround for broken platforms which don't detect threads exiting
 * -------------------------------------------------------------------- */
void platform_hint_thread_finished()
{
   /*
      todo: should we do this:

      vcos_thread_deregister_at_exit(client_thread_detach);
      client_thread_detach();

      here?
   */
}

#ifndef KHRN_PLATFORM_VCOS_NO_MALLOC

/**
   Allocate memory

   @param size Size in bytes of memory block to allocate
   @return pointer to memory block
**/
void *khrn_platform_malloc(size_t size, const char * name)
{
   return vcos_malloc(size, name);
}

/**
   Free memory

   @param v Pointer to  memory area to free
**/
void khrn_platform_free(void *v)
{
   if (v)
   {
      vcos_free(v);
   }
}

#endif


#ifdef WANT_X
static XImage *current_ximage = NULL;

static KHRN_IMAGE_FORMAT_T ximage_to_image_format(int bits_per_pixel, unsigned long red_mask, unsigned long green_mask, unsigned long blue_mask)
{
   if (bits_per_pixel == 16 /*&& red_mask == 0xf800 && green_mask == 0x07e0 && blue_mask == 0x001f*/)
      return RGB_565_RSO;
   //else if (bits_per_pixel == 24 && red_mask == 0xff0000 && green_mask == 0x00ff00 && blue_mask == 0x0000ff)
   //   return RGB_888_RSO;
   else if (bits_per_pixel == 24 && red_mask == 0x0000ff && green_mask == 0x00ff00 && blue_mask == 0xff0000)
      return BGR_888_RSO;
   else if (bits_per_pixel == 32 /*&& red_mask == 0x0000ff && green_mask == 0x00ff00 && blue_mask == 0xff0000*/)
      return ABGR_8888_RSO; //meego uses alpha channel
   else if (bits_per_pixel == 32 && red_mask == 0xff0000 && green_mask == 0x00ff00 && blue_mask == 0x0000ff)
      return ARGB_8888_RSO;
   else
   {
      vcos_log_warn("platform_get_pixmap_info unknown image format\n");
      return IMAGE_FORMAT_INVALID;
   }
}

bool platform_get_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image)
{
   Window r;
   int x, y;
   unsigned int w, h, b, d;
   KHRN_IMAGE_FORMAT_T format;
   XImage *xi;
   XWindowAttributes attr;
   Status rc;

   vcos_log_trace("platform_get_pixmap_info !!!");

   if (!XGetGeometry(hacky_display, (Drawable)pixmap, &r, &x, &y, &w, &h, &b, &d))
      return false;

   vcos_log_trace("platform_get_pixmap_info %d geometry = %d %d %d %d",(int)pixmap,
              x, y, w, h);

   xi = XGetImage(hacky_display, (Drawable)pixmap, 0, 0, w, h, 0xffffffff, ZPixmap);
   if (xi == NULL)
      return false;

   vcos_log_trace("platform_get_pixmap_info ximage = %d %d %d 0x%08x %d %x %x %x",
              xi->width, xi->height, xi->bytes_per_line, (uint32_t)xi->data,
              xi->bits_per_pixel, (uint32_t)xi->red_mask,
              (uint32_t)xi->green_mask, (uint32_t)xi->blue_mask);

   format = ximage_to_image_format(xi->bits_per_pixel, xi->red_mask, xi->green_mask, xi->blue_mask);
   if (format == IMAGE_FORMAT_INVALID)
   {
      XDestroyImage(xi);
      return false;
   }

   image->format = format;
   image->width = xi->width;
   image->height = xi->height;
   image->stride = xi->bytes_per_line;
   image->aux = NULL;
   image->storage = xi->data;

//hacking to see if this pixmap is actually the offscreen pixmap for the window that is our current surface
   {
      int xw, yw;
      unsigned int ww, hw, bw, dw;
      unsigned long pixel;
      Window rw,win  = (Window)CLIENT_GET_THREAD_STATE()->opengl.draw->win;
      vcos_log_trace("current EGL surface win %d ", (int)win);
      if(win!=0)
      {
         /* Install our error handler to override Xlib's termination behavior */
         old_handler = XSetErrorHandler(application_error_handler) ;

         XGetGeometry(hacky_display, (Drawable)win, &rw, &xw, &yw, &ww, &hw, &bw, &dw);
         vcos_log_trace("%dx%d", ww, hw);
         if(ww==w && hw==h)
         {
            //this pixmap is the same size as our current window
            pixel = XGetPixel(xi,w/2,h/2);
            vcos_log_trace("Pixmap centre pixel 0x%lx%s",pixel,pixel==CHROMA_KEY_565 ? "- chroma key!!" : "");
            if(pixel == CHROMA_KEY_565)//the pixmap is also full of our magic chroma key colour, we want to copy the server side EGL surface.
               image->aux = (void *)CLIENT_GET_THREAD_STATE()->opengl.draw->serverbuffer ;
         }

         (void) XSetErrorHandler(old_handler) ;
      }
   }
//

   current_ximage = xi;
   return true;
}

void khrn_platform_release_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image)
{
   XDestroyImage(current_ximage);
   current_ximage = NULL;
}
#else
static KHRN_IMAGE_FORMAT_T convert_format(uint32_t format)
{
   switch (format & ~EGL_PIXEL_FORMAT_USAGE_MASK_BRCM) {
      case EGL_PIXEL_FORMAT_ARGB_8888_PRE_BRCM: return (KHRN_IMAGE_FORMAT_T)(ABGR_8888 | IMAGE_FORMAT_PRE);
      case EGL_PIXEL_FORMAT_ARGB_8888_BRCM:     return ABGR_8888;
      case EGL_PIXEL_FORMAT_XRGB_8888_BRCM:     return XBGR_8888;
      case EGL_PIXEL_FORMAT_RGB_565_BRCM:       return RGB_565;
      case EGL_PIXEL_FORMAT_A_8_BRCM:           return A_8;
      default:                                  vcos_verify(0); return (KHRN_IMAGE_FORMAT_T)0;
   }
}

bool platform_get_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image)
{
   image->format = convert_format(((uint32_t *)pixmap)[4]);
   image->width = ((uint32_t *)pixmap)[2];
   image->height = ((uint32_t *)pixmap)[3];

   /* can't actually access data */
   image->stride = 0;
   image->aux = 0;
   image->storage = 0;

   return image->format != 0;
}
void khrn_platform_release_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image)
{
   /* Nothing to do */
}
#endif

void platform_get_pixmap_server_handle(EGLNativePixmapType pixmap, uint32_t *handle)
{
   handle[0] = ((uint32_t *)pixmap)[0];
   handle[1] = ((uint32_t *)pixmap)[1];
}

bool platform_match_pixmap_api_support(EGLNativePixmapType pixmap, uint32_t api_support)
{
   return
      (!(api_support & EGL_OPENGL_BIT) || (((uint32_t *)pixmap)[4] & EGL_PIXEL_FORMAT_RENDER_GL_BRCM)) &&
      (!(api_support & EGL_OPENGL_ES_BIT) || (((uint32_t *)pixmap)[4] & EGL_PIXEL_FORMAT_RENDER_GLES_BRCM)) &&
      (!(api_support & EGL_OPENGL_ES2_BIT) || (((uint32_t *)pixmap)[4] & EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM)) &&
      (!(api_support & EGL_OPENVG_BIT) || (((uint32_t *)pixmap)[4] & EGL_PIXEL_FORMAT_RENDER_VG_BRCM));
}

#if EGL_BRCM_global_image && EGL_KHR_image

bool platform_use_global_image_as_egl_image(uint32_t id_0, uint32_t id_1, EGLNativePixmapType pixmap, EGLint *error)
{
   return true;
}

void platform_acquire_global_image(uint32_t id_0, uint32_t id_1)
{
}

void platform_release_global_image(uint32_t id_0, uint32_t id_1)
{
}

void platform_get_global_image_info(uint32_t id_0, uint32_t id_1,
   uint32_t *pixel_format, uint32_t *width, uint32_t *height)
{
   EGLint id[2] = {id_0, id_1};
   EGLint width_height_pixel_format[3];
   verify(eglQueryGlobalImageBRCM(id, width_height_pixel_format));
   width_height_pixel_format[2] |=
      /* this isn't right (the flags should be those passed in to
       * eglCreateGlobalImageBRCM), but this stuff is just for basic testing, so
       * it doesn't really matter */
      EGL_PIXEL_FORMAT_RENDER_GLES_BRCM | EGL_PIXEL_FORMAT_RENDER_GLES2_BRCM |
      EGL_PIXEL_FORMAT_RENDER_VG_BRCM | EGL_PIXEL_FORMAT_VG_IMAGE_BRCM |
      EGL_PIXEL_FORMAT_GLES_TEXTURE_BRCM | EGL_PIXEL_FORMAT_GLES2_TEXTURE_BRCM;
   if (pixel_format) { *pixel_format = width_height_pixel_format[2]; }
   if (width) { *width = width_height_pixel_format[0]; }
   if (height) { *height = width_height_pixel_format[1]; }
}

#endif

void platform_client_lock(void)
{
   platform_mutex_acquire(&client_mutex);
}

void platform_client_release(void)
{
   platform_mutex_release(&client_mutex);
}

void platform_init_rpc(struct CLIENT_THREAD_STATE *state)
{
   assert(1);
}

void platform_term_rpc(struct CLIENT_THREAD_STATE *state)
{
   assert(1);
}

void platform_maybe_free_process(void)
{
   assert(1);
}

void platform_destroy_winhandle(void *a, uint32_t b)
{
   assert(1);
}

void platform_surface_update(uint32_t handle)
{
   /*
   XXX This seems as good a place as any to do the client side pixmap hack.
   (called from eglSwapBuffers)
   */
   send_bound_pixmaps();
}

void egl_gce_win_change_image(void)
{
   assert(0);
}

void platform_retrieve_pixmap_completed(EGLNativePixmapType pixmap)
{
   assert(0);
}

void platform_send_pixmap_completed(EGLNativePixmapType pixmap)
{
   assert(0);
}

uint32_t platform_memcmp(const void * aLeft, const void * aRight, size_t aLen)
{
   return memcmp(aLeft, aRight, aLen);
}

void platform_memcpy(void * aTrg, const void * aSrc, size_t aLength)
{
   memcpy(aTrg, aSrc, aLength);
}


#ifdef WANT_X
uint32_t platform_get_handle(EGLNativeWindowType win)
{
   return (uint32_t)win;
}

void platform_get_dimensions(EGLDisplay dpy, EGLNativeWindowType win,
      uint32_t *width, uint32_t *height, uint32_t *swapchain_count)
{
   Window w = (Window) win;
   XWindowAttributes attr;
   GC gc;
   Status rc = XGetWindowAttributes(hacky_display, w, &attr);

   // check rc is OK and if it is (vcos_assert(rc == 0);?????)
   *width = attr.width;
   *height = attr.height;
   *swapchain_count = 0;

	 /* Hackily assume if this function is called then they want to fill with GL stuff. So fill window with chromakey. */
   vcos_log_trace("Calling XCreateGC %d",(int)w);

	 gc = XCreateGC(hacky_display, w, 0, NULL);
	 XSetForeground(hacky_display, gc, CHROMA_KEY_565);

   vcos_log_trace("Calling XFillRectangle %d %dx%d",(int)w,attr.width, attr.height);

	 XFillRectangle(hacky_display, w, gc, 0, 0, attr.width, attr.height);

   vcos_log_trace("Calling XFreeGC");

	 XFreeGC(hacky_display, gc);

   vcos_log_trace("Done platform_get_dimensions");
    //debugging
    dump_hierarchy(attr.root, w, 0, 0);
}
#endif

#ifdef WANT_X
EGLDisplay khrn_platform_set_display_id(EGLNativeDisplayType display_id)
{
   if(hacky_display==0)
   {
	   hacky_display = (Display *)display_id;
	   return (EGLDisplay)1;
	}
	else
	   return EGL_NO_DISPLAY;
}
#else
EGLDisplay khrn_platform_set_display_id(EGLNativeDisplayType display_id)
{
   if (display_id == EGL_DEFAULT_DISPLAY)
      return (EGLDisplay)1;
   else
      return EGL_NO_DISPLAY;
}
#endif

#ifdef WANT_X
static void dump_hierarchy(Window w, Window thisw, Window look, int level)
{
   Window root_dummy, parent_dummy, *children;
   unsigned int i, nchildren;
   XWindowAttributes attr;

   XGetWindowAttributes(hacky_display, w, &attr);
   XQueryTree(hacky_display, w, &root_dummy, &parent_dummy, &children, &nchildren);

   for (i = 0; i < level; i++)
   {
         vcos_log_trace(" ");
   }
   vcos_log_trace( "%d %d%s%s",
              attr.map_state, (int)w,
              (w==look)?" <-- LOOK FOR ME!":((w==thisw)?" <-- THIS WINDOW":""),
              children?"":" no children");

   if (children)
   {
      for (i = 0; i < nchildren; i++)
      {
         dump_hierarchy(children[i], thisw, look, level + 1);
      }
      XFree(children);
   }
}

static void dump_ancestors(Window w)
{
   Window root_dummy, *children;
   unsigned int i, nchildren;

   Window grandparent,parent = w, child = 0;
   unsigned int rlayer = ~0;
   bool bidirectional;
   vcos_log_trace("walking back up heirarchy");
   while(parent)
   {
      bidirectional = false;
      if(!XQueryTree(hacky_display, parent, &root_dummy, &grandparent, &children, &nchildren))
         break;
      if (children)
      {
         for (i = 0; i < nchildren; i++)
         {
            if (children[i] == child)
            {
               bidirectional = true;
               rlayer = i;
            }
         }
         XFree(children);
      }
      vcos_log_trace("%s%s%d", bidirectional ? "<" : "", (child>0) ? "->" : "", (int)parent);

      child = parent;
      parent = grandparent;

   }
   vcos_log_trace("->end");
}


uint32_t khrn_platform_get_window_position(EGLNativeWindowType win)
{
   Window w = (Window) win;
   Window dummy;
   XWindowAttributes attr;
   Window look_for_me, root_dummy, root_dummy2, parent_dummy, *children;
   int x, y;
   unsigned int layer, i, nchildren;

   //the assumption is that windows are at the 2nd level i.e. in the below
   //root_dummy/attr.root -> look_for_me -> w
   vcos_log_trace("Start khrn_platform_get_window_position");

   XGetWindowAttributes(hacky_display, w, &attr);

   vcos_log_trace("XGetWindowAttributes");

   if (attr.map_state == IsViewable)
   {
      XTranslateCoordinates(hacky_display, w, attr.root, 0, 0, &x, &y, &dummy);

      vcos_log_trace("XTranslateCoordinates");

      XQueryTree(hacky_display, w, &root_dummy, &look_for_me, &children, &nchildren);
      if (children) XFree(children);
      XQueryTree(hacky_display, attr.root, &root_dummy2, &parent_dummy, &children, &nchildren);

      vcos_log_trace("XQueryTree");

      layer = ~0;

      vcos_log_trace("Dumping hierarchy %d %d (%d)", (int)w, (int)look_for_me, (int)root_dummy);
      dump_hierarchy(attr.root, w, look_for_me, 0);

      if (children)
      {
         for (i = 0; i < nchildren; i++)
         {
            if (children[i] == look_for_me)
               layer = i;
         }
         XFree(children);
      }

      vcos_log_trace("XFree");

      if (layer == ~0)
      {
         vcos_log_error("EGL window isn't child of root", i);

         //to try and find out where this window has gone, let us walk back up the heirarchy
         dump_ancestors(w);
         return ~0;
      }
      else
      {
         vcos_log_trace("End khrn_platform_get_window_position - visible");
         return x | y << 12 | layer << 24;
      }
   }
   else
   {
      vcos_log_trace("End khrn_platform_get_window_position - invisible");

      return ~0;      /* Window is invisible */
   }
}
#else
static int xxx_position = 0;
uint32_t khrn_platform_get_window_position(EGLNativeWindowType win)
{
   return xxx_position;
}
#endif

#define NUM_PIXMAP_BINDINGS 16
static struct
{
   bool used;
   bool send;
   EGLNativePixmapType pixmap;
   EGLImageKHR egl_image;
} pixmap_binding[NUM_PIXMAP_BINDINGS];

static void set_egl_image_color_data(EGLImageKHR egl_image, KHRN_IMAGE_WRAP_T *image)
{
   int line_size = (image->stride < 0) ? -image->stride : image->stride;
   int lines = KHDISPATCH_WORKSPACE_SIZE / line_size;
   int offset = 0;
   int height = image->height;

   if (khrn_image_is_brcm1(image->format))
      lines &= ~63;

   assert(lines > 0);

   while (height > 0) {
      int batch = _min(lines, height);
      uint32_t len = batch * line_size;

      CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
      int adjusted_offset = (image->stride < 0) ? (offset + (batch - 1)) : offset;

      RPC_CALL8_IN_BULK(eglIntImageSetColorData_impl,
         thread,
         EGLINTIMAGESETCOLORDATA_ID,
         RPC_EGLID(egl_image),
         RPC_UINT(image->format),
         RPC_UINT(0),
         RPC_INT(offset),
         RPC_UINT(image->width),
         RPC_INT(batch),
         RPC_UINT(image->stride),
         (const char *)image->storage + adjusted_offset * image->stride,
         len);

      offset += batch;
      height -= batch;
   }
}

static void send_bound_pixmap(int i)
{
   KHRN_IMAGE_WRAP_T image;

   vcos_log_trace("send_bound_pixmap %d %d", i, (int)pixmap_binding[i].egl_image);

   vcos_assert(i >= 0 && i < NUM_PIXMAP_BINDINGS);
   vcos_assert(pixmap_binding[i].used);

   platform_get_pixmap_info(pixmap_binding[i].pixmap, &image);
   set_egl_image_color_data(pixmap_binding[i].egl_image, &image);
   khrn_platform_release_pixmap_info(pixmap_binding[i].pixmap, &image);
}

static void send_bound_pixmaps(void)
{
   int i;
   for (i = 0; i < NUM_PIXMAP_BINDINGS; i++)
   {
      if (pixmap_binding[i].used && pixmap_binding[i].send)
      {
         send_bound_pixmap(i);
      }
   }
}

void khrn_platform_bind_pixmap_to_egl_image(EGLNativePixmapType pixmap, EGLImageKHR egl_image, bool send)
{
   int i;
   for (i = 0; i < NUM_PIXMAP_BINDINGS; i++)
   {
      if (!pixmap_binding[i].used)
      {

         vcos_log_trace("khrn_platform_bind_pixmap_to_egl_image %d", i);

         pixmap_binding[i].used = true;
         pixmap_binding[i].pixmap = pixmap;
         pixmap_binding[i].egl_image = egl_image;
         pixmap_binding[i].send = send;
         if(send)
            send_bound_pixmap(i);
         return;
      }
   }
   vcos_assert(0);  /* Not enough NUM_PIXMAP_BINDINGS? */
}

void khrn_platform_unbind_pixmap_from_egl_image(EGLImageKHR egl_image)
{
   int i;
   for (i = 0; i < NUM_PIXMAP_BINDINGS; i++)
   {
      if (pixmap_binding[i].used && pixmap_binding[i].egl_image == egl_image)
      {
         pixmap_binding[i].used = false;
      }
   }
}


#ifdef EGL_SERVER_DISPMANX
#define NUM_WIN 6

static bool have_default_dwin[NUM_WIN];
static EGL_DISPMANX_WINDOW_T default_dwin[NUM_WIN];

static EGL_DISPMANX_WINDOW_T *check_default(EGLNativeWindowType win)
{
   int wid = (int)win;
   vcos_log_trace("%s check_default win=%p",__FUNCTION__,win);
   if(wid>-NUM_WIN && wid <=0) {
      /*
       * Special identifiers indicating the default windows. Either use the
       * one we've got or create a new one
       * simple hack for VMCSX_VC4_1.0 release to demonstrate concurrent running of apps under linux

       * win ==  0 => full screen window on display 0
       * win == -1 => 1/4 screen top left window on display 0
       * win == -2 => 1/4 screen top right window on display 0
       * win == -3 => 1/4 screen bottom left window on display 0
       * win == -4 => 1/4 screen bottom right window on display 0
       * win == -5 => full screen window on display 2

       * it is expected that Open WFC will provide a proper mechanism in the near future
       */
       int success =0;
	wid = -wid;
      
	bcm_host_init();
	 
	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;
   

	int display_width;
	int display_height;

	// create an EGL window surface, passing context width/height
	success = graphics_get_display_size(0 /* LCD */, &display_width, &display_height);
	if ( success < 0 )
	{
	    return EGL_FALSE;
	}
   
	// You can hardcode the resolution here:
	//display_width = 1920;
	//display_height = 1080;

       dst_rect.x = 0;
       dst_rect.y = 0;
       dst_rect.width = display_width;
       dst_rect.height = display_height;
	  
       src_rect.x = 0;
       src_rect.y = 0;
       src_rect.width = display_width << 16;
       src_rect.height = display_height << 16;   

	dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
	dispman_update = vc_dispmanx_update_start( 0 );
         
	dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
						0/*layer*/, &dst_rect, 0/*src*/,
						&src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
      
	nativewindow.element = dispman_element;
	nativewindow.width = display_width;
	nativewindow.height = display_height;
	vc_dispmanx_update_submit_sync( dispman_update );
	return &nativewindow;
   } else
      return (EGL_DISPMANX_WINDOW_T*)win;
}

#ifdef ANDROID
uint32_t android_platform_get_handle(EGLDisplay dpy,EGLNativeWindowType window)
{
 
		errno = 0 ;
		// Do we have a window?
		if(window == NULL){
			errno = EINVAL;
			vcos_log_error("%s Error Window %p %d : %s ",__FUNCTION__,window,errno,strerror(errno));
			return NULL;	
		}	
		void *window_ptr = window;
		uint32_t* window_magic_ptr = window_ptr;
		uint32_t window_magic = (*window_magic_ptr);
		uint32_t return_value = 0;
		if(window_magic != ANDROID_NATIVE_WINDOW_MAGIC){
			vcos_log_trace("%s win=%p is not a ANativeWindow 0x%x\n",__FUNCTION__,window , window_magic);
			vcos_log_trace("%s Trying EGL_DISPMANX_WINDOW_T \n",__FUNCTION__);
			EGL_DISPMANX_WINDOW_T* native_window = window_ptr;
			//return_value = (uint32_t) android_get_dispmanx_element();
			return_value= (uint32_t)window;
		}else if(window_magic == ANDROID_NATIVE_WINDOW_MAGIC){
			vcos_log_trace("%s ANativeWindow Found win=%p 0x%x\n",__FUNCTION__,window , window_magic);
			//return_value = (uint32_t) android_get_dispmanx_element();
			return_value= (uint32_t)window;
		}
		return return_value;
}

static void android_native_window_get_dimensions(ANativeWindow *native_window,int32_t *width, int32_t *height){



	// Query ALL THE THINGS!
	native_window->query(native_window, NATIVE_WINDOW_WIDTH, width);
	vcos_log_trace("%s NATIVE_WINDOW_WIDTH = %d ",__FUNCTION__,(*width));

	native_window->query(native_window, NATIVE_WINDOW_HEIGHT, height);
	vcos_log_trace("%s NATIVE_WINDOW_HEIGHT = %d ",__FUNCTION__,(*height));

	int minbuffers = 0 ;
	native_window->query(native_window, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, &minbuffers);
	vcos_log_trace("%s  NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS = %d ",__FUNCTION__,minbuffers);

	int composer_queues = 0 ;
	native_window->query(native_window,  NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER, &composer_queues);
	vcos_log_trace("%s   NATIVE_WINDOW_QUEUES_TO_WINDOW_COMPOSER = %d ",__FUNCTION__,composer_queues);


	vcos_log_trace("%s native_window->common=%p\n",__FUNCTION__,native_window->common);
	if(native_window->common.magic == NULL){
		vcos_log_error("%s native_window->common.magic not set\n",__FUNCTION__);
	}else{
		vcos_log_trace("%s native_window->common.magic=%p\n",__FUNCTION__,native_window->common.magic);
	}


}

void android_platform_get_dimensions(EGLDisplay display, EGLNativeWindowType window,
      int32_t *width, int32_t *height, uint32_t *swapchain_count)
{
	errno = 0;

	// Do some sanity checks	

	// Make sure we have a valid pointer to store
	// the width in
	if(width == NULL){
		errno = EINVAL;
		vcos_log_error("%s Error Width %d : %s ",__FUNCTION__,errno,strerror(errno));
		return ;
	}
	// set width to zero now we know the
	// pointer is valid
	(*width) = 0;

	// Make sure we have a valid pointer to store the height in
	if(height == NULL){
		errno = EINVAL;
		vcos_log_error("%s Error Height %d : %s ",__FUNCTION__,errno,strerror(errno));
		return ;
	}

	// set height to zero now we know the
	// pointer is valid
	(*height) = 0;

	// Make sure we have a valid pointer to store the height in
	if(swapchain_count == NULL){
		errno = EINVAL;
		vcos_log_error("%s Error swapchain_count %d : %s ",__FUNCTION__,errno,strerror(errno));
		return ;
	}

	// set swapchain_count to zero now we know the
	// pointer is valid
	(*swapchain_count) = 0;


	// Do we have a window?
	if(window == NULL){
		errno = EINVAL;
		vcos_log_error("%s Error Window %p %d : %s ",__FUNCTION__,window,errno,strerror(errno));
		return ;	
	}	

	// We need to figure out what the concrete type backing
	// the EGLNativeWindow is. ANDROID_NATIVE_WINDOW_MAGIC is
	// "_wnd" [ 0x5f776e64 ] which fits nicely into a 4 byte integer
	// Extra void* variable to avoid unneccessary casting drama!
	// it compiler will abstract it away so there's no extra overhead.
	void *window_ptr = window;
	uint32_t* window_magic_ptr = window_ptr;
	uint32_t window_magic = (*window_magic_ptr);
	if(window_magic != ANDROID_NATIVE_WINDOW_MAGIC){
		vcos_log_trace("%s win=%p is not a ANativeWindow 0x%x\n",__FUNCTION__,window , window_magic);
		vcos_log_trace("%s Trying EGL_DISPMANX_WINDOW_T \n",__FUNCTION__);

		EGL_DISPMANX_WINDOW_T* native_window = NULL ;//android_get_dispmanx();
		(*height) = native_window->height;
		(*width) = native_window->width;


	}else if(window_magic == ANDROID_NATIVE_WINDOW_MAGIC){

		vcos_log_trace("%s ANDROID_NATIVE_WINDOW_MAGIC FOUND window=%p 0x%x\n",__FUNCTION__,window,window_magic);
		ANativeWindow *native_window = window;
		android_native_window_get_dimensions(native_window,height,width);
	}
	vcos_log_trace("%s returning height=%d width=%d",__FUNCTION__,(*height),(*width));
	return ;


}


#endif
EGLint is_anativewindow( EGLNativeWindowType window)
{
    errno = 0 ;
    // Do we have a window?
    if(window == NULL){
	    errno = EINVAL;
	    vcos_log_error("%s Error Window %p %d : %s ",__FUNCTION__,window,errno,strerror(errno));
	    return NULL;	
    }	
    void *window_ptr = window;
    uint32_t* window_magic_ptr = window_ptr;
    uint32_t window_magic = (*window_magic_ptr);
    uint32_t return_value = EGL_FALSE;
    if(window_magic == ANDROID_NATIVE_WINDOW_MAGIC){
	return_value = EGL_TRUE;
    }
    return return_value;
}

EGL_DISPMANX_WINDOW_T* platform_get_dispmanx_handle_from_anativewindow(EGLDisplay display, EGLNativeWindowType window)
{
        // If we are working with an ANativeWindow we need to pull the EGL_DISPMANX_WINDOW_T
    // out of the process state windows pointer map. 
    // If it's our first time here then we need to create an EGL_DISPMANX_WINDOW_T
    // and associate it with this ANative window
    CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
    CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, display, EGL_TRUE);
    
	// client_egl_get_window perform an khrn_pointer_map_lookup on the win argument internally
	EGL_DISPMANX_WINDOW_T *dwin = (EGL_DISPMANX_WINDOW_T*)client_egl_get_window(thread,process,0);
	vcos_log_trace("%s win %p dwin %p ",__FUNCTION__,window,dwin);

	if ( !is_anativewindow(window) ) {
	    dwin =  (EGL_DISPMANX_WINDOW_T*)window;
	} else if ( is_anativewindow(window) && (dwin==NULL) ) {
	    vcos_log_trace("%s Android Window Found Setting up dispmanx window win=%p dwin=%p ",__FUNCTION__,window,dwin);
	    dwin = check_default(0);
	    khrn_pointer_map_insert(&process->windows, 0, dwin);
	
	}
	vcos_assert(dwin);
	
	return dwin;
  
}
void platform_get_dimensions(EGLDisplay dpy, EGLNativeWindowType win,
      uint32_t *width, uint32_t *height, uint32_t *swapchain_count)
{
   
       vcos_log_trace("%s win=%p", __FUNCTION__,win);
    const hw_module_t *mod;
    int fd = -1, err;
    err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &mod);
    
        struct private_module_t* pmod =  (struct private_module_t*) mod;
    vcos_log_trace("%s mod=%p mod->window=%p", __FUNCTION__,pmod, pmod->window);
    if(pmod->window == NULL ){
	    alloc_device_t *gr;
	    int err = gralloc_open(mod, &gr);
	    if (err) {
		vcos_log_trace("couldn't open gralloc HAL (%s)", strerror(-err));
		return -ENODEV;
	    }
    }
    EGL_DISPMANX_WINDOW_T *dwin = pmod->window ;
    //EGL_DISPMANX_WINDOW_T *dwin = platform_get_dispmanx_handle_from_anativewindow(dpy,win);
   vcos_assert(dwin->width < 1<<16); // sanity check
   vcos_assert(dwin->height < 1<<16); // sanity check
   *width = dwin->width;
   *height = dwin->height;
   vcos_log_trace("%s win=%p dpy=%p dwin=%p dwin->height=%d,dwin->width=%d", __FUNCTION__,win,dpy,dwin,dwin->height,dwin->width);
   *swapchain_count = 0;
}
uint32_t platform_get_handle(EGLDisplay dpy, EGLNativeWindowType win)
{
    vcos_log_trace("%s win=%p", __FUNCTION__,win);
    hw_module_t *mod;
    int fd = -1, err;
    err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &mod);
    
    struct private_module_t* pmod =  (struct private_module_t*) mod;
    vcos_log_trace("%s mod=%p mod->window=%p", __FUNCTION__,pmod, pmod->window);
    if(pmod->window == NULL ){
	    alloc_device_t *gr;
	    int err = gralloc_open(mod, &gr);
	    if (err) {
		vcos_log_trace("couldn't open gralloc HAL (%s)", strerror(-err));
		return -ENODEV;
	    }
    }
    EGL_DISPMANX_WINDOW_T *dwin = pmod->window ; //platform_get_dispmanx_handle_from_anativewindow(dpy,win);
     vcos_log_trace("%s win=%p dpy=%p dwin=%p dwin->height=%d,dwin->width=%d", __FUNCTION__,win,dpy,dwin,dwin->height,dwin->width);
    return (uint32_t)dwin->element;
}


#endif

// Android Platform Color Format
uint32_t platform_get_color_format ( uint32_t format ) { 
    
    uint32_t return_value = 0 ; 
    format--;
    /*vcos_log_trace("%s format=%d", __FUNCTION__,format);
    vcos_log_trace("%s ABGR_8888=%d", __FUNCTION__,ABGR_8888);
    vcos_log_trace("%s RGB_565=%d", __FUNCTION__,RGB_565);
    vcos_log_trace("%s XBGR_8888=%d", __FUNCTION__,XBGR_8888);*/
    switch(format) {
	case ABGR_8888:
	    return_value = HAL_PIXEL_FORMAT_RGBA_8888;
	    break;
	case RGB_565:
	    return_value = HAL_PIXEL_FORMAT_RGB_565;
	    break;
	case XBGR_8888:
	    return_value = HAL_PIXEL_FORMAT_RGBX_8888;
	    break;
	default:
	    break;
	
    }
    //vcos_log_trace("%s return_value=%d", __FUNCTION__,return_value);
    return return_value; 
    
}
void platform_dequeue(EGLDisplay dpy, EGLNativeWindowType window) {}
