#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#define LOG_TAG "dummy"
#include <utils/Log.h>

#ifndef EGL_ANDROID_framebuffer_target
#define EGL_ANDROID_framebuffer_target 1
#define EGL_FRAMEBUFFER_TARGET_ANDROID		0x3147
#endif

extern bool _gl_config_get_attrib(int id, EGLint attrib, EGLint *value);
extern bool _gl_config_check_attribs(const EGLint *attrib_list, bool *use_red, bool *use_green, bool *use_blue, bool *use_alpha);
extern bool _gl_config_filter(int id, const EGLint *attrib_list);

bool egl_config_get_attrib(int id, EGLint attrib, EGLint *value){
	
	
	if(attrib== EGL_FRAMEBUFFER_TARGET_ANDROID){
		*value = EGL_TRUE;
	  //ALOGD("%s ConfigID=%d attrib=EGL_FRAMEBUFFER_TARGET_ANDROID[0x%x] value=[0x%x]",__FUNCTION__,id,attrib,(*value));
      return true;
	}else{
		
		bool ret =  _gl_config_get_attrib(id,attrib,value);
		//ALOGD("%s ConfigID=%d attrib=[0x%x] value=[0x%x] ret=%d",__FUNCTION__,id,attrib,(*value),ret);
		return ret;
	}
}


bool egl_config_filter(int id, const EGLint *attrib_list){
	
	bool ret = _gl_config_filter(id,attrib_list);
	//ALOGD("%s _gl_config_filter returns %d %d",__FUNCTION__, ret,true);
	if(!ret){
		while (*attrib_list != EGL_NONE) {
			EGLint name = *attrib_list++;
			EGLint value = *attrib_list++;
			EGLint actual_value;
			//ALOGD("%s ConfigID=%d attrib=[0x%x] value=%d actual_value=%d",__FUNCTION__,id,name,value,actual_value);
			if(name == EGL_FRAMEBUFFER_TARGET_ANDROID){
				//ALOGD("%s ConfigID=%d attrib=EGL_FRAMEBUFFER_TARGET_ANDROID[0x%x] value=%d actual_value=%d",__FUNCTION__,id,name,value,actual_value);
				egl_config_get_attrib(id, name, &actual_value) ;
				ret = true;
				if (value != EGL_DONT_CARE && value != actual_value){
					ret= false;
				}
				break;
			}
		}
		
	}
	//ALOGD("%s  returns %d",__FUNCTION__, ret);
	return ret;
}

bool egl_config_check_attribs(const EGLint *attrib_list, bool *use_red, bool *use_green, bool *use_blue, bool *use_alpha){
	
	bool ret = _gl_config_check_attribs(attrib_list, use_red, use_green, use_blue, use_alpha);
	//ALOGD("%s _gl_config_check_attribs returns %d  attrib_list=%p" ,__FUNCTION__, ret,attrib_list);
	if(!ret){
		while (*attrib_list != EGL_NONE) {
			EGLint name = *attrib_list++;
			EGLint value = *attrib_list++;

			//ALOGD("%s attrib=[0x%x] value=%d",__FUNCTION__,name,value);
			if(name == EGL_FRAMEBUFFER_TARGET_ANDROID){
				//ALOGD("%s attrib=EGL_FRAMEBUFFER_TARGET_ANDROID[0x%x] value=%d",__FUNCTION__,name,value);
				switch (value) {
					case EGL_DONT_CARE:
					case EGL_TRUE:
					case EGL_FALSE:
						ret = true ;
						break;
					default:
						ret = false;
						break;
			
				}
				break; // break out the while if name == EGL_FRAMEBUFFER_TARGET_ANDROID
			}	
			
		}
	}
	//ALOGD("%s  returns %d",__FUNCTION__, ret);
	return ret;
	

}
volatile int *__errno_location(void)
{
    return &errno;
}
