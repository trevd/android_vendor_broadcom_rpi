#define LOG_TAG	"BRCM_EGL"
#include <utils/Log.h>
#include <system/window.h>

#include <khronos/common/khrn_client.h>
#include <khronos/common/khrn_client_rpc.h>
#include <khronos/egl/egl_client_config.h>
#include "brcm_egl.h"

#define IS_ANDROID_WINDOW(w) ((w) && (w)->common.magic == ANDROID_NATIVE_WINDOW_MAGIC)


/*               surface = egl_surface_create((EGLSurface)(size_t)process->next_surface, 
                                linear ? LINEAR : SRGB,
                                premult ? PRE : NONPRE,
                               3,
                                width, height,
                                config,
                                win,
                                handle,
                                false,
                                false,
                                false);*/
/*EGLSurface *brcm_egl_window_surface_create( uint32_t buffers,  uint32_t width,  uint32_t height,   EGLConfig config,
   EGLNativeWindowType win)
{
   KHRN_IMAGE_FORMAT_T color;
   KHRN_IMAGE_FORMAT_T depth;
   KHRN_IMAGE_FORMAT_T mask;
   KHRN_IMAGE_FORMAT_T multi;
   uint32_t configid;
   uint32_t sem_name;
   EGLint   config_depth_bits;
   EGLint   config_stencil_bits;
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   EGL_SURFACE_T *surface = egl_surface_pool_alloc();

   //vcos_assert(width > 0 && height > 0);
   vcos_assert((width <= EGL_CONFIG_MAX_WIDTH && height <= EGL_CONFIG_MAX_HEIGHT) || largest_pbuffer);

   if (!surface) {
      return 0;
   }

   // TODO: respect largest_pbuffer? 

   surface->name = name;
   surface->type = WINDOW;
       
	egl_surface_check_attribs(WINDOW, attrib_list, surface->colorspace, surface->alphaformat , 0, 0, 0, 0, 0, 0, 0)


   surface->config = config;
   surface->win = win;
   surface->width = width;
   surface->height = height;
   surface->swap_interval = 1;

   surface->base_width = width;
   surface->base_height = height;

   surface->internal_handle = serverwin;

   surface->largest_pbuffer = false;
   surface->mipmap_texture = false;
   surface->mipmap_level = 0;
   surface->texture_format = EGL_NO_TEXTURE;
   surface->texture_target = EGL_NO_TEXTURE;
   surface->pixmap = 0;
   surface->pixmap_server_handle[0] = 0;
   surface->pixmap_server_handle[1] = (uint32_t)-1;
   surface->server_owned = false;
   surface->swap_behavior =  EGL_BUFFER_DESTROYED;
   surface->multisample_resolve = EGL_MULTISAMPLE_RESOLVE_DEFAULT;

   surface->context_binding_count = 0;
   surface->is_destroyed = false;


   surface->is_locked = false;
   surface->mapped_buffer = 0;


   configid = egl_config_to_id(config);
   color = egl_config_get_color_format(configid);
   
   if (surface->alphaformat == PRE) { color = (KHRN_IMAGE_FORMAT_T)(color | IMAGE_FORMAT_PRE); }
   if (surface->colorspace == LINEAR) { color = (KHRN_IMAGE_FORMAT_T)(color | IMAGE_FORMAT_LIN); }
   depth = egl_config_get_depth_format(configid);
   mask = egl_config_get_mask_format(configid);
   multi = egl_config_get_multisample_format(configid);

   // Find depth and stencil bits from chosen config (these may NOT be the same as the underlying format!) 
   egl_config_get_attrib(configid, EGL_DEPTH_SIZE, &config_depth_bits);
   egl_config_get_attrib(configid, EGL_STENCIL_SIZE, &config_stencil_bits);

 



   surface->buffers = buffers;

    surface->avail_buffers_valid = false;

      sem_name = KHRN_NO_SEMAPHORE;
      if (surface->buffers > 1) {
         uint64_t pid = khronos_platform_get_process_id();
         int sem[3];
         sem[0] = (int)pid; sem[1] = (int)(pid >> 32); sem[2] = (int)name;

         sem_name = (int)name;
         if (khronos_platform_semaphore_create(&surface->avail_buffers, sem, surface->buffers) == KHR_SUCCESS)
            surface->avail_buffers_valid = true;
      }
      if (sem_name == KHRN_NO_SEMAPHORE || surface->avail_buffers_valid) {

         uint32_t results[3];

         RPC_CALL15_OUT_CTRL(eglIntCreateSurface_impl,
                             thread,
                             EGLINTCREATESURFACE_ID,
                             RPC_UINT(serverwin),
                             RPC_UINT(buffers),
                             RPC_UINT(width),
                             RPC_UINT(height),
                             RPC_UINT(color),
                             RPC_UINT(depth),
                             RPC_UINT(mask),
                             RPC_UINT(multi),
                             RPC_UINT(false),
                             RPC_UINT(false),
                             RPC_UINT(config_depth_bits),
                             RPC_UINT(config_stencil_bits),
                             RPC_UINT(sem_name),
                             RPC_UINT(WINDOW),
                             results);
         surface->width = results[0];
         surface->height = results[1];
         surface->serverbuffer = results[2];
      } else {
         surface->serverbuffer = 0;
      }

   

   if (surface->serverbuffer)
      return surface;
   else {
      // Server failed to create a surface due to out-of-memory or
      //   we failed to create the named semaphore object. 
      egl_surface_pool_free(surface);
      return 0;
   }
}*/
#define EGL_CONFIG_MAX_WIDTH 2048
#define EGL_CONFIG_MAX_HEIGHT 2160
#define EGL_MAX_CONFIGS 28
EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface_Int(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list,int width, int height)
{
   CLIENT_THREAD_STATE_T *thread;
   CLIENT_PROCESS_STATE_T *process;
   EGLSurface result;
	
   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &process))
   {
      uint32_t handle = platform_get_handle(dpy, win);
		//ALOGI("%s handle=%d[0x%x] dpy=%d win=%p",__FUNCTION__,handle,handle,dpy, win);
      if ((int)(size_t)config < 1 || (int)(size_t)config > EGL_MAX_CONFIGS) {
		  ALOGI("%s EGL_BAD_CONFIG result=EGL_NO_SURFACE",__FUNCTION__);
         thread->error = EGL_BAD_CONFIG;
         result = EGL_NO_SURFACE;
      } else if (handle == PLATFORM_WIN_NONE) {
         // The platform reports that this is an invalid window handle
         ALOGI("%s EGL_BAD_NATIVE_WINDOW result=EGL_NO_SURFACE",__FUNCTION__);
         thread->error = EGL_BAD_NATIVE_WINDOW;
         result = EGL_NO_SURFACE;
      } else {
         bool linear = false;
         bool premult = false;
         bool single = false;

         if (!egl_surface_check_attribs(WINDOW, attrib_list, &linear, &premult, &single, 0, 0, 0, 0, 0, 0)) {
			 ALOGI("%s EGL_BAD_ATTRIBUTE result=EGL_NO_SURFACE",__FUNCTION__);
            thread->error = EGL_BAD_ATTRIBUTE;
            result = EGL_NO_SURFACE;
         } else {
            EGL_SURFACE_T *surface;

            uint32_t num_buffers = 3;
			//ALOGI("%s dpy=%d width=%d , height=%d EGL_CONFIG_MAX_HEIGHT=%d",__FUNCTION__,dpy,width,height,EGL_CONFIG_MAX_HEIGHT);
            if (width <= 0 || width > EGL_CONFIG_MAX_WIDTH || height <= 0 || height > EGL_CONFIG_MAX_HEIGHT) {
               /* TODO: Maybe EGL_BAD_ALLOC might be more appropriate? */
               
               ALOGE("%s EGL_BAD_NATIVE_WINDOW result=EGL_NO_SURFACE",__FUNCTION__);
               thread->error = EGL_BAD_NATIVE_WINDOW;
               result = EGL_NO_SURFACE;
            } else {
               surface = egl_surface_create(
                                (EGLSurface)(size_t)process->next_surface,
                                WINDOW,
                                linear ? LINEAR : SRGB,
                                premult ? PRE : NONPRE,
                               3,
                                width, height,
                                config,
                                win,
                                handle,
                                false,
                                false,
                                false,
                                EGL_NO_TEXTURE,
                                EGL_NO_TEXTURE,
                                0, 0);

               if (surface) {
                  if (khrn_pointer_map_insert(&process->surfaces, process->next_surface, surface)) {
                     thread->error = EGL_SUCCESS;
                     result = (EGLSurface)(size_t)process->next_surface++;
                  } else {
                     thread->error = EGL_BAD_ALLOC;
                     result = EGL_NO_SURFACE;
                     egl_surface_free(surface);
                  }
               } else {
                  thread->error = EGL_BAD_ALLOC;
                  result = EGL_NO_SURFACE;
               }
            }
         }
      }
      CLIENT_UNLOCK();
   }
   else
      result = EGL_NO_SURFACE;

   

   return result;
}
extern EGLSurface _glCreateWindowSurface(  EGLDisplay display, EGLConfig config, EGLNativeWindowType win,  const EGLint *attrib_list);
EGLSurface eglCreateWindowSurface(  EGLDisplay display, EGLConfig config, EGLNativeWindowType win,  const EGLint *attrib_list)
{


	ALOGD("%s display=%p config=%p win=%p",__FUNCTION__,display,config,win);
	//EGLSurface surface = _glCreateWindowSurface(display,config,win,NULL);
	//ALOGD("%s surface=%p",__FUNCTION__,surface);
	EGLSurface surface = EGL_NO_SURFACE;
	ANativeWindow *window = (ANativeWindow*)win;
	int native_format;
	window->query(window, NATIVE_WINDOW_FORMAT, &native_format);
	ALOGD("%s: format = %d",__FUNCTION__,native_format);
	if(native_format == 0){
		window->perform(window, NATIVE_WINDOW_SET_BUFFERS_FORMAT, HAL_PIXEL_FORMAT_YCbCr_422_I);
	}
	if(IS_ANDROID_WINDOW(window)){
		ALOGD("%s: Window is an Android Window",__FUNCTION__);
		struct surface *surface_ptr = (struct surface *)malloc(sizeof(struct surface));
	  
		int value = eglGetConfigAttrib(display,config,EGL_SAMPLE_BUFFERS,&value);
		ALOGD("%s: EGL_SAMPLE_BUFFERS=%d",__FUNCTION__,value);
		if(value){
			surface_ptr->isTiled = 0;
		}else{
			surface_ptr->isTiled = 1;
		}
		surface_ptr->isTiled = 0;

		int width, height;

		window->query(window, NATIVE_WINDOW_WIDTH, &width);
		ALOGD("%s: NATIVE_WINDOW_WIDTH=%d",__FUNCTION__,width);
		
		window->query(window, NATIVE_WINDOW_HEIGHT, &height);
		ALOGD("%s: NATIVE_WINDOW_HEIGHT=%d",__FUNCTION__,height);
		
		//Don't care about EGLNativeWindowType
		EGLSurface surface_int = eglCreateWindowSurface_Int(display, config,win, attrib_list,width,height);
		ALOGD("%s: surface_int=%p",__FUNCTION__,surface_int);
		
		//the above function will create a single buffered 'container' which currently not populated with a buffer pointer at creation
		if(surface_int != EGL_NO_SURFACE){
			surface_ptr->surface = surface_int;
			surface_ptr->window = window;
			surface_ptr->buffer = 0;

			//send back the surface
			surface = (EGLSurface)surface_ptr;

			ALOGD("%s:  surface: %p, VC surface: %d, Thread: %d",__FUNCTION__, surface, (int)surface_int, gettid());
		}else{
			ALOGE("%s:   FAILED",__FUNCTION__);
			free(surface_ptr);
		}

	}else{
		ALOGD("%s: surface: %p, Thread: %d",__FUNCTION__, surface, gettid());
	}

	return surface;
}

static pthread_key_t tls_key;
static pthread_once_t tls_key_once = PTHREAD_ONCE_INIT;

static void init_tls_key(void)
{
    pthread_key_create(&tls_key, NULL);
}

struct context *brcm_egl_get_current()
{
    pthread_once(&tls_key_once, init_tls_key);
    return (struct context *)pthread_getspecific(tls_key);
}

static void brcm_set_current(struct context *context)
{
    pthread_once(&tls_key_once, init_tls_key);
    pthread_setspecific(tls_key, context);
}


extern void egl_gl_flush_callback(bool wait);

extern void egl_vg_flush_callback(bool wait);
extern void egl_gl_render_callback(void);
extern void egl_vg_render_callback(void);
static void brcm_egl_current_release(CLIENT_PROCESS_STATE_T *process, EGL_CURRENT_T *current)
{
	ALOGI("%s",__FUNCTION__);
   if (current->context) {
      EGL_CONTEXT_T *context = current->context;
      vcos_assert(context->is_current);
      context->is_current = false;
      context->renderbuffer = EGL_NONE;
      egl_context_set_callbacks(context, NULL, NULL, NULL, NULL);

      current->context = 0;

      egl_context_maybe_free(context);

      vcos_assert(process->context_current_count > 0);
      process->context_current_count--;
   }
   if (current->draw) {
      EGL_SURFACE_T *draw = current->draw;

      vcos_assert(draw->context_binding_count > 0);
      draw->context_binding_count--;

      current->draw = 0;

      egl_surface_maybe_free(draw);
   }
   if (current->read) {
      EGL_SURFACE_T *read = current->read;

      vcos_assert(read->context_binding_count > 0);
      read->context_binding_count--;

      current->read = 0;

      egl_surface_maybe_free(read);
   }
}

static void brcm_flush_current_api(CLIENT_THREAD_STATE_T *thread)
{
   RPC_CALL2(eglIntFlush_impl,
                 thread,
                 EGLINTFLUSH_ID,
                 RPC_UINT(thread->bound_api == EGL_OPENGL_ES_API),
                 RPC_UINT(thread->bound_api == EGL_OPENVG_API));
   RPC_FLUSH(thread);

   if (thread->bound_api == EGL_OPENGL_ES_API)
      egl_gl_flush_callback(false);
   else
      egl_vg_flush_callback(false);
}

static bool brcm_context_and_surface_are_compatible(EGL_CONTEXT_T *context, EGL_SURFACE_T *surface)
{
	ALOGI("%s",__FUNCTION__);
   /*
      from section 2.2 of the (1.3) spec, a context and surface are compatible
      if:
      1) they support the same type of color buffer (rgb or luminance). this is
         trivially true for us as we only support rgb color buffers
      2) they have color buffers and ancillary buffers of the same depth
      3) the surface was created with respect to an EGLConfig supporting client
         api rendering of the same type as the api type of the context
      4) they were created with respect to the same EGLDisplay. this is
         trivially true for us as we only have one EGLDisplay
   */

   uint32_t api_type = 0;
   switch (context->type) {
   case OPENGL_ES_11: api_type = EGL_OPENGL_ES_BIT; break;
   case OPENGL_ES_20: api_type = EGL_OPENGL_ES2_BIT; break;
   case OPENVG:       api_type = EGL_OPENVG_BIT; break;
   default:           UNREACHABLE();
   }

   return
      egl_config_bpps_match((int)(intptr_t)context->configname - 1, (int)(intptr_t)surface->config - 1) && /* (2) */
      (egl_config_get_api_support((int)(intptr_t)surface->config - 1) & api_type); /* (3) */
}

static bool brcm_egl_current_set(CLIENT_PROCESS_STATE_T *process, CLIENT_THREAD_STATE_T *thread, EGL_CURRENT_T *current, EGL_CONTEXT_T *context, EGL_SURFACE_T *draw, EGL_SURFACE_T *read)
{
   bool result = false;

   UNUSED(process);

   if (context->is_current && context->thread != thread) {
		ALOGE("%s: ctx=%p dr=%p rd=%p  Fail - context is current to some other thread",__FUNCTION__, current, draw, read );
      // Fail - context is current to some other thread
      thread->error = EGL_BAD_ACCESS;
   } else if (draw->context_binding_count && draw->thread != thread) {
	   ALOGE("%s: ctx=%p dr=%p rd=%p  Fail - draw surface is bound to context which is current to another thread",__FUNCTION__, current, draw, read );
      // Fail - draw surface is bound to context which is current to another thread
      thread->error = EGL_BAD_ACCESS;
   } else if (read->context_binding_count && read->thread != thread) {
	   ALOGE("%s: ctx=%p dr=%p rd=%p  Fail - read surface is bound to context which is current to another thread",__FUNCTION__, current, draw, read );
      // Fail - read surface is bound to context which is current to another thread
      thread->error = EGL_BAD_ACCESS;
   } else if (!brcm_context_and_surface_are_compatible(context, draw)) {
	   ALOGE("%s: ctx=%p dr=%p rd=%p Fail - draw surface is not compatible with context",__FUNCTION__, current, draw, read );
      // Fail - draw surface is not compatible with context
      thread->error = EGL_BAD_MATCH;
   } else if (!brcm_context_and_surface_are_compatible(context, read)) {
	   ALOGE("%s: ctx=%p dr=%p rd=%p Fail - read surface is not compatible with context",__FUNCTION__, current, draw, read );
      // Fail - read surface is not compatible with context
      thread->error = EGL_BAD_MATCH;
   } else {
      brcm_egl_current_release(process, current);

      context->is_current = true;
      context->thread = thread;

      /* TODO: GLES supposedly doesn't support single-buffered rendering. Should we take this into account? */
      context->renderbuffer = egl_surface_get_render_buffer(draw);

      // Check surfaces are not bound to a different thread, and increase their reference count

      draw->thread = thread;
      draw->context_binding_count++;

      read->thread = thread;
      read->context_binding_count++;

      current->context = context;
      current->draw = draw;
      current->read = read;

      process->context_current_count++;

      result = true;
   }
   if (draw->type == PIXMAP) {
      egl_context_set_callbacks(context, egl_gl_render_callback, egl_gl_flush_callback, egl_vg_render_callback, egl_vg_flush_callback);
   } else {
      egl_context_set_callbacks(context, NULL,NULL, NULL, NULL);
   }

   return result;
}


extern EGLBoolean _glMakeCurrent(EGLDisplay display, EGLSurface draw, EGLSurface read, EGLContext context);
EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface dr, EGLSurface rd, EGLContext ctx)
{
    struct context *next_context = (struct context *)ctx;
    struct surface *next_draw_surface = (struct surface *)dr;
    struct surface *next_read_surface = (struct surface *)rd;

	ALOGD("%s: next_context: %p, next_draw_surface: %p next_read_surface: %p ",__FUNCTION__, next_context, next_draw_surface,next_read_surface);
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;
   CLIENT_PROCESS_STATE_T *process = NULL; /* init to avoid warnings */

   CLIENT_LOCK();

   ALOGD("%s: ctx=%p dr=%p rd=%p ",__FUNCTION__, ctx, dr, rd );

   /*
      check whether we are trying to release the current context
      Note that we can do this even if the display isn't initted.
   */

   if (dr == EGL_NO_SURFACE && rd == EGL_NO_SURFACE && ctx == EGL_NO_CONTEXT) {
      process = client_egl_get_process_state(thread, dpy, EGL_FALSE);

      if (process) {
         /* spec says we should flush in this case */
         brcm_flush_current_api(thread);

         brcm_egl_current_release(process,
            (thread->bound_api == EGL_OPENVG_API) ? &thread->openvg : &thread->opengl);

         client_send_make_current(thread);

         client_try_unload_server(process);

         thread->error = EGL_SUCCESS;
         result = EGL_TRUE;
      } else {
         result = EGL_FALSE;
      }
   } else  if (dr == EGL_NO_SURFACE || rd == EGL_NO_SURFACE || ctx == EGL_NO_CONTEXT) {
      thread->error = EGL_BAD_MATCH;
      result = EGL_FALSE;
   } else {
      /*
         get display
      */

      process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (!process)
         result = EGL_FALSE;
      else {
         /*
            get context
         */

         EGL_CONTEXT_T *context = client_egl_get_context(thread, process, ctx);

         if (!context) {
            result = EGL_FALSE;
         }  else {

            /*
               get surfaces
            */

            EGL_SURFACE_T *draw = client_egl_get_surface(thread, process, dr);
            EGL_SURFACE_T *read = client_egl_get_surface(thread, process, rd);
			ALOGD("%s: 1 - context=%p draw=%p read=%p ",__FUNCTION__, (*context), draw),read );
            if (!draw || !read) {
               result = EGL_FALSE;
               ALOGD("%s: 2 - context=%p draw=%p read=%p ",__FUNCTION__, (*context), draw,read );
            } else if (context->type == OPENVG && dr != rd) {
				ALOGD("%s: context->type %d=%d context=%p draw=%p read=%p ",__FUNCTION__, context->type ,OPENVG,(*context), draw,read );
               thread->error = EGL_BAD_MATCH;   //TODO: what error are we supposed to return here?
               result = EGL_FALSE;
            } else {
               EGL_CURRENT_T *current;

               if (context->type == OPENVG)
                  current = &thread->openvg;
               else
                  current = &thread->opengl;
				
				ALOGD("%s: context=%p draw=%p read=%p ",__FUNCTION__, (*context), draw,read );
               if (!brcm_egl_current_set(process, thread, current, context, draw, read)){
				   ALOGD("%s: brcm_egl_current_set failed context=%p draw=%p read=%p ",__FUNCTION__, (*context), draw,read );
                  result = EGL_FALSE;
               }else {
                  client_send_make_current(thread);

                  thread->error = EGL_SUCCESS;
                  result = EGL_TRUE;
               }
            }
         }
      }
   }

   CLIENT_UNLOCK();

	ALOGD("%s: ctx=%p dr=%p rd=%p ",__FUNCTION__, ctx, dr, rd );
	if(result == EGL_FALSE)
		ALOGD("%s: ret = EGL_FALSE",__FUNCTION__);
	else
		ALOGD("%s: ret = EGL_TRUE",__FUNCTION__);
   return result;
}

EGLBoolean _eglMakeCurrent(EGLDisplay display, EGLSurface draw, EGLSurface read, EGLContext context)
{
    struct context *next_context = (struct context *)context;
    struct surface *next_draw_surface = (struct surface *)draw;
    struct surface *next_read_surface = (struct surface *)read;
    struct context *current = brcm_egl_get_current();
	ALOGD("%s: next_context: %p, next_draw_surface: %p next_read_surface: %p current: %p",__FUNCTION__, next_context, next_draw_surface,next_read_surface,current);
	EGLBoolean ret = EGL_FALSE;

    //Are we getting rid of the context?
    if(draw == EGL_NO_SURFACE && read == EGL_NO_SURFACE && context == EGL_NO_CONTEXT)
    {
     

        //if there is a context current, get rid of it
        brcm_set_current(NULL);

        //call the V3D function
        //eglMakeCurrent_Int(display, draw, read, context);

        current->made_current = 0;
      ALOGD("%s: Thread: %d",__FUNCTION__, gettid());
        return EGL_TRUE;
    }
    //is the context being setup? check all fields
    else if(draw != EGL_NO_SURFACE && read != EGL_NO_SURFACE && context != EGL_NO_CONTEXT)
    {
          ALOGD("%s: Thread: %d",__FUNCTION__, gettid());
        //make the call to v3d
      //  ret = eglMakeCurrent_Int(display, draw, read, context);

        //This should not fail - maybe the context was deleted however by memory management
        if(ret == EGL_FALSE)
        {
			 ALOGD("%s: ret = EGL_FALSE Thread: %d",__FUNCTION__, gettid());
            /*CLIENT_THREAD_STATE_T *thread = platform_get_thread_state();

            if(thread->error == EGL_CONTEXT_LOST)
            {
                LOGE("eglMakeCurrent(%p, %p, %p) error: EGL_CONTEXT_LOST, Thread: %d", context, draw, read, gettid());
            }
            else
            {
                LOGE("eglMakeCurrent(%p, %p, %p) error: 0x%x, Thread: %d", context, draw, read, thread->error, gettid());
            }*/
        }
        else
        {
            ALOGD("%s ret = EGL_TRUE context %p draw %p read %p Thread: %d",__FUNCTION__, context, draw, read, gettid());
        }

        //TODO - we ignore the context error code and go ahead with the following anyway

        //set the current context up
			next_context = (struct context *)context;
			next_draw_surface = (struct surface *)draw;
			next_read_surface = (struct surface *)read;
			current = brcm_egl_get_current();
        brcm_set_current(next_context);
        current = next_context;
        current->read = next_read_surface;
        current->draw = next_draw_surface;
		ALOGD("%s current %p  current->draw %p current->read  %p Thread: %d",__FUNCTION__, current, current->draw, current->read, gettid());
       
        //connect a current context
       // attach_current_context(display, current);

		glFlush();
    }
    else
    {
        ret = EGL_FALSE;
    }
	if(ret == EGL_FALSE)
		ALOGD("%s: ret = EGL_FALSE",__FUNCTION__);
	else
		ALOGD("%s: ret = EGL_TRUE",__FUNCTION__);
		
	return ret;
}
