package com.mik.adim.jni_files;

public class FileUtils {

    public static native void diff(String path, String pattern_path, int num);
    public static native void merge(String mergePath, String pattern_path, int num);
}
