package com.example.ffmpegtest;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.File;
//创建显示预览页面的surface
public class MainActivity extends AppCompatActivity {
    private Button btnStartRecord=null;
    private Button btnCameraPreview = null;
    private Button btnVideRecord = null;
    private CameraRender cameraRender = null;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        cameraRender = findViewById(R.id.cameraView);
                //new CameraRender(this);
        btnCameraPreview = findViewById(R.id.cameraPreviewBt);
        btnCameraPreview.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                cameraRender.startCameraPreview();
            }
        });




        // Example of a call to a native method
//        TextView tv = findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());
        //cameraRender = findViewById(R.id.surfaceView)

        btnStartRecord = findViewById(R.id.audioRecordBt);
        btnStartRecord.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                File outFile = new File(getExternalFilesDir(null), "output.pcm");

                startRecord(outFile.getAbsolutePath());

            }
        });

        btnVideRecord = findViewById(R.id.vidoeRecordBt);
        btnVideRecord.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                cameraRender.startRecord();
            }
        });
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native int decode();

    public native void startRecord(String path);
}
