/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "JniHelper.h"
#include <android/log.h>
#include <string.h>

#if 1
#define  LOG_TAG    "JniHelper"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#else
#define  LOGD(...) 
#endif

#define JAVAVM	cocos2d::JniHelper::getJavaVM()

extern "C"
{

    //////////////////////////////////////////////////////////////////////////
    // java vm helper function
    //////////////////////////////////////////////////////////////////////////

    jint JNI_OnLoad(JavaVM *vm, void *reserved)
    {
		cocos2d::JniHelper::setJavaVM(vm);
        return JNI_VERSION_1_4;
    }

	static bool getEnv(JNIEnv **env)
	{
		bool bRet = false;

		do 
		{
			if (JAVAVM->GetEnv((void**)env, JNI_VERSION_1_4) != JNI_OK)
			{
				LOGD("Failed to get the environment using GetEnv()");
				break;
			}

			if (JAVAVM->AttachCurrentThread(env, 0) < 0)
			{
				LOGD("Failed to get the environment using AttachCurrentThread()");
				break;
			}

			bRet = true;
		} while (0);		

		return bRet;
	}

	static jclass getClassID_(const char *className, JNIEnv *env)
	{
		JNIEnv *pEnv = env;
		jclass ret = 0;

		do 
		{
			if (! pEnv)
			{
				if (! getEnv(&pEnv))
				{
					break;
				}
			}
			
			ret = pEnv->FindClass(className);
			if (! ret)
			{
				 LOGD("Failed to find class of %s", className);
				break;
			}
		} while (0);

		return ret;
	}

	static bool getStaticMethodInfo_(cocos2d::JniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode)
    {
		jmethodID methodID = 0;
		JNIEnv *pEnv = 0;
		bool bRet = false;

        do 
        {
			if (! getEnv(&pEnv))
			{
				break;
			}

            jclass classID = getClassID_(className, pEnv);

            methodID = pEnv->GetStaticMethodID(classID, methodName, paramCode);
            if (! methodID)
            {
                LOGD("Failed to find static method id of %s", methodName);
                break;
            }

			methodinfo.classID = classID;
			methodinfo.env = pEnv;
			methodinfo.methodID = methodID;

			bRet = true;
        } while (0);

        return bRet;
    }

	static bool getMethodInfo_(cocos2d::JniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode)
	{
		jmethodID methodID = 0;
		JNIEnv *pEnv = 0;
		bool bRet = false;

		do 
		{
			if (! getEnv(&pEnv))
			{
				break;
			}

			jclass classID = getClassID_(className, pEnv);

			methodID = pEnv->GetMethodID(classID, methodName, paramCode);
			if (! methodID)
			{
				LOGD("Failed to find method id of %s", methodName);
				break;
			}

			methodinfo.classID = classID;
			methodinfo.env = pEnv;
			methodinfo.methodID = methodID;

			bRet = true;
		} while (0);

		return bRet;
	}

	static char* jstringToChar_(jstring jstr)
	{
		char* rtn = 0;
		JNIEnv *env = 0;

		if (! getEnv(&env))
		{
			return 0;
		}

		// convert jstring to byte array
		jclass clsstring = env->FindClass("java/lang/String");
		jstring strencode = env->NewStringUTF("utf-8");
		jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
		jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
		jsize alen =  env->GetArrayLength(barr);
		jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);

		// copy byte array into char[]
		if (alen > 0)
		{
			rtn = new char[alen + 1];
			memcpy(rtn, ba, alen);
			rtn[alen] = 0;
		}
		env->ReleaseByteArrayElements(barr, ba, 0);

		return rtn;
	}
}

namespace cocos2d {

	JavaVM* JniHelper::m_psJavaVM = NULL;

	JavaVM* JniHelper::getJavaVM()
	{
		return m_psJavaVM;
	}

	void JniHelper::setJavaVM(JavaVM *javaVM)
	{
		m_psJavaVM = javaVM;
	}

	jclass JniHelper::getClassID(const char *className, JNIEnv *env)
	{
		return getClassID_(className, env);
	}

	bool JniHelper::getStaticMethodInfo(JniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode)
	{
		return getStaticMethodInfo_(methodinfo, className, methodName, paramCode);
	}

	bool JniHelper::getMethodInfo(JniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode)
	{
		return getMethodInfo_(methodinfo, className, methodName, paramCode);
	}

	char* JniHelper::jstringToChar(jstring str)
	{
		return jstringToChar_(str);
	}
}