package io.stateoftheart.handy3dscanner;

import android.os.Bundle;
import android.util.Log;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import org.qtproject.qt5.android.bindings.QtActivity;

public class Handy3DScannerActivity extends QtActivity
{
    private String TAG = "Handy3DScanner";
    private static final int MY_PERMISSIONS_REQUEST_CAMERA = 10;

    public Handy3DScannerActivity()
    {
        super();
        Log.d(TAG, "Creating platform java object");
        Log.d(TAG, "ANDROID BASE_OS    : " + Build.VERSION.BASE_OS);
        Log.d(TAG, "ANDROID CODENAME   : " + Build.VERSION.CODENAME);
        Log.d(TAG, "ANDROID INCREMENTAL: " + Build.VERSION.INCREMENTAL);
        Log.d(TAG, "ANDROID RELEASE    : " + Build.VERSION.RELEASE);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        Log.d(TAG, "onRequestPermissionsResult");
        if( ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED ) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, MY_PERMISSIONS_REQUEST_CAMERA);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Android 9 also requires camera permissions
        if( ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED ) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, MY_PERMISSIONS_REQUEST_CAMERA);
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "Destroying platform java object");
    }
}
