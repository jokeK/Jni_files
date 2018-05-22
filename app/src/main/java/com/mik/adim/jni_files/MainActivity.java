package com.mik.adim.jni_files;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("my_jni");
    }
    private static final String SD_CARD_PATH = Environment.getExternalStorageDirectory()
            .getPath();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //高版本的别忘记了动态申请权限
        setJniEnv();
    }

    public void diff(View view) {
       String path = SD_CARD_PATH + File.separatorChar + "video.mp4";
        String pattern_path = SD_CARD_PATH + File.separatorChar + "video_%d.mp4";
        FileUtils.diff(path, pattern_path, 4);
    }
    public void merge(View view) {
        String path = SD_CARD_PATH + File.separatorChar + "video_merge.mp4";
        String pattern_path = SD_CARD_PATH + File.separatorChar + "video_%d.mp4";
        FileUtils.merge(path, pattern_path, 4);
    }

    public void jniThread(View view) {
        newJniThread();
    }

    private native void newJniThread();

    private native void setJniEnv();

    private static void fromJni(int i){
        Log.d("miK","from jni:"+i);
    }

    private void fromJniAgain( int a){
        Log.d("miK","from jni again:" +a);
    }

}
