
#include "com_mik_adim_jni_files_FileUtils.h"
#include <android/log.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

#define TAG "Mik_JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define NELEM(x) ((int)(sizeof(x)/sizeof((x)[0])))

JavaVM *jvm = NULL;
jobject *j_obj = NULL;

//获取总文件的大小
long get_file_size(const char *file_path) {
    //rb 是打开一个文件，文件必须存在，只允许读
    FILE *fp = fopen(file_path, "rb");
    if (fp == NULL) {
        LOGI("文件不存在");
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size;
}

JNIEXPORT void JNICALL native_diff
        (JNIEnv *env, jclass clazz, jstring path, jstring pattern_Path, jint file_num) {

    const char *path_c = (*env)->GetStringUTFChars(env, path, NULL);
    const char *pattern_Path_c = (*env)->GetStringUTFChars(env, pattern_Path, NULL);
    //申请二维字符数组存放子文件名
    char **patches = (char **) malloc(sizeof(char *) * file_num);

    int i = 0;
    for (; i < file_num; ++i) {
        //为每个子文件名申请地址
        patches[i] = (char *) malloc(sizeof(char) * 100);
        //文件名 video_%d.mp4动态替换video_i.mp4
        sprintf(patches[i], pattern_Path_c, i);
        LOGI("patch path: %s", patches[i]);
    }
    int file_size = get_file_size(path_c);

    //打开总文件
    FILE *fpr = fopen(path_c, "rb");
    /**
     * 1.判断文件的大小能否被file_num整除
     * 如果能整除就平分file_num份，
     * 不能整除就先平分file_num-1份，然后把剩下的作为一份
     */
    if (file_size % file_num == 0) {
        //part每一个文件的大小
        int part = file_size / file_num;
        for (int i = 0; i < file_num; i++) {
            //wb 文件不存在就创建，只允许写
            FILE *fpw = fopen(patches[i], "wb");
            for (int j = 0; j < part; j++) {
                //把总文件中的part长度部分写入子文件
                fputc(fgetc(fpr), fpw);
            }
            fclose(fpw);
        }
    } else {
        int part = file_size / (file_num - 1);
        for (int i = 0; i < file_num - 1; i++) {
            //wb 文件不存在就创建，只允许写
            FILE *fpw = fopen(patches[i], "wb");
            for (int j = 0; j < part; j++) {
                //把总文件中的part长度部分写入子文件
                fputc(fgetc(fpr), fpw);
            }
            fclose(fpw);
        }
        FILE *fpw_last = fopen(patches[file_num - 1], "wb");
        for (int i = 0; i < file_size % (file_num - 1); i++) {
            fputc(fgetc(fpr), fpw_last);
        }
        fclose(fpw_last);
    }
    fclose(fpr);

    for (int i = 0; i < file_num; i++) {
        free(patches[i]);
    }
    free(patches);
    (*env)->ReleaseStringUTFChars(env, path, path_c);
    (*env)->ReleaseStringUTFChars(env, pattern_Path, pattern_Path_c);
}

JNIEXPORT void JNICALL native_merge
        (JNIEnv *env, jclass clazz, jstring path_merge, jstring pattern_Path, jint file_num) {

    const char *path_c = (*env)->GetStringUTFChars(env, path_merge, NULL);
    const char *pattern_Path_c = (*env)->GetStringUTFChars(env, pattern_Path, NULL);
    //申请二维字符数组存放子文件名
    char **patches_merge = (char **) malloc(sizeof(char *) * file_num);

    int i = 0;
    for (; i < file_num; ++i) {
        //为每个子文件名申请地址
        patches_merge[i] = (char *) malloc(sizeof(char) * 100);
        //文件名 video_%d.mp4动态替换video_i.mp4
        sprintf(patches_merge[i], pattern_Path_c, i);
        LOGI("merge path: %s", patches_merge[i]);
    }

    FILE *fpw = fopen(path_c, "wb");
    for (int i = 0; i < file_num; i++) {
        long file_size = get_file_size(patches_merge[i]);
        FILE *fp = fopen(patches_merge[i], "rb");
        for (int j = 0; j < file_size; j++) {
            int c = fgetc(fp);
            fputc(c, fpw);
        }
        fclose(fp);
    }
    fclose(fpw);

    for (int i = 0; i < file_num; i++) {
        free(patches_merge[i]);
    }
    free(patches_merge);

    (*env)->ReleaseStringUTFChars(env, path_merge, path_c);
    (*env)->ReleaseStringUTFChars(env, pattern_Path, pattern_Path_c);
}

void *thread_fun(void *arg) {

    JNIEnv *env;
    jclass cls;
    jmethodID mid, mid1;
    if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != JNI_OK) {
        LOGI("%s AttachCurrentThread error", __FUNCTION__);
        return NULL;
    }
    cls = (*env)->GetObjectClass(env, j_obj);
    if (cls == NULL) {
        LOGI("class is null");
        goto error;
    }
    LOGI("call back begin");
    mid = (*env)->GetStaticMethodID(env, cls, "fromJni", "(I)V");
    if (mid == NULL) {
        LOGI("mid is null");
        goto error;
    }
    (*env)->CallStaticVoidMethod(env,cls,mid,(int)arg);

    mid1 = (*env)->GetMethodID(env,cls,"fromJniAgain","(I)V");
    if (mid1 == NULL) {
        LOGI("mid is null");
        goto error;
    }
    (*env)->CallVoidMethod(env,j_obj,mid1,(int)arg);

    error:
    if ((*jvm)->DetachCurrentThread(jvm) != JNI_OK) {
        LOGI("%s DetachCurrentThread error", __FUNCTION__);
        return NULL;
    }
    pthread_exit(0);

}

JNIEXPORT void JNICALL new_thread
        (JNIEnv *env, jclass clazz) {
    LOGI("new thread");
    //线程的标识
    pthread_t pt[5];
    for (int i = 0; i < 5; i++) {
        pthread_create(&pt[i],NULL,&thread_fun,(void*)i);
    }

}

JNIEXPORT void JNICALL native_setJniEnv
        (JNIEnv *env, jobject obj) {
    (*env)->GetJavaVM(env, &jvm);
    j_obj = (*env)->NewGlobalRef(env, obj);
}

static const JNINativeMethod gMethods[] = {
        {
                //1、java方法名 2、参数和返回值的签名3、C方法的指针
                "diff",  "(Ljava/lang/String;Ljava/lang/String;I)V", (void *) native_diff
        },
        {
                "merge", "(Ljava/lang/String;Ljava/lang/String;I)V", (void *) native_merge
        }
};
static const JNINativeMethod gMethods_Main[] = {
        {
                "newJniThread", "()V", (void *) new_thread
        },
        {
                "setJniEnv",    "()V", (void *) native_setJniEnv
        }
};

static int registerNative(JNIEnv *env) {
    jclass clz;
    clz = (*env)->FindClass(env, "com/mik/adim/jni_files/FileUtils");
    if (clz == NULL) {
        LOGI("clz null");
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, clz, gMethods, NELEM(gMethods)) < 0) {
        LOGI("registerNative error");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static int registerNative_Main(JNIEnv *env) {
    jclass clz;
    clz = (*env)->FindClass(env, "com/mik/adim/jni_files/MainActivity");
    if (clz == NULL) {
        LOGI("clz null");
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, clz, gMethods_Main, NELEM(gMethods_Main)) < 0) {
        LOGI("registerNative error");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGI("JNI_OnLoad");
    JNIEnv *env = NULL;
    jint result = -1;
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }
    assert(env != NULL);
    if (registerNative(env) < 0) {
        return result;
    }
    if (registerNative_Main(env) < 0) {
        return result;
    }
    return JNI_VERSION_1_4;
}
