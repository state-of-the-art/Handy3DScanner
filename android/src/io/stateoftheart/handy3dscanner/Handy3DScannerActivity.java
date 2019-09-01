package io.stateoftheart.handy3dscanner;

import java.util.HashMap;
import java.util.Iterator;

import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Bundle;
import android.util.Log;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

import org.qtproject.qt5.android.bindings.QtActivity;

public class Handy3DScannerActivity extends QtActivity
{
    private static Handy3DScannerActivity m_instance;
    private String TAG = "Handy3DScanner";
    private static final int MY_PERMISSIONS_REQUEST_CAMERA = 0;

    private static final String ACTION_USB_PERMISSION = "com.android.example.USB_PERMISSION";
    private PendingIntent mPermissionIntent;
    private UsbManager manager;
    private UsbDeviceConnection connection;
    private HashMap<Integer, Integer> connectedDevices;

    public Handy3DScannerActivity()
    {
        m_instance = this;

        connectedDevices = new HashMap<Integer, Integer>();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, MY_PERMISSIONS_REQUEST_CAMERA);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Android 9 also requires camera permissions
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, MY_PERMISSIONS_REQUEST_CAMERA);
        }

        manager = (UsbManager) getSystemService(Context.USB_SERVICE);

        registerReceiver(usbManagerBroadcastReceiver, new IntentFilter(UsbManager.ACTION_USB_DEVICE_ATTACHED));
        registerReceiver(usbManagerBroadcastReceiver, new IntentFilter(UsbManager.ACTION_USB_DEVICE_DETACHED));
        registerReceiver(usbManagerBroadcastReceiver, new IntentFilter(ACTION_USB_PERMISSION));

        mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);

        final Handler handler = new Handler();

        handler.postDelayed(new Runnable()
        {
            @Override
            public void run()
            {
                checkForDevices();
            }
        }, 1000);
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
    }

    private static native void notifyDeviceAttached(String name, int fd);
    private static native void notifyDeviceDetached(int fd);

    private final BroadcastReceiver usbManagerBroadcastReceiver = new BroadcastReceiver()
    {
        public void onReceive(Context context, Intent intent)
        {
            try
            {
                String action = intent.getAction();
                if (ACTION_USB_PERMISSION.equals(action))
                {
                    Log.d(TAG, "onUsbPermission");

                    synchronized (this)
                    {
                        UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                        if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false))
                        {
                            if(device != null)
                            {
                                connection = manager.openDevice(device);
                                Log.d(TAG, "inserting device with id: " + device.getDeviceId() + " and file descriptor: " + connection.getFileDescriptor());
                                connectedDevices.put(device.getDeviceId(), connection.getFileDescriptor());
                                notifyDeviceAttached(device.getDeviceName(), connection.getFileDescriptor());
                                Log.d(TAG, "device attached: " + device.getDeviceName() + " - " + device);
                            }
                        }
                        else
                        {
                            Log.d(TAG, "permission denied for device " + device);
                        }
                    }
                }

                if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action))
                {
                    Log.d(TAG, "onDeviceConnected");

                    synchronized(this)
                    {
                        UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                        if (device != null)
                        {
                            manager.requestPermission(device, mPermissionIntent);
                        }
                    }
                }

                if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action))
                {
                    Log.d(TAG, "onDeviceDisconnected");

                    synchronized(this)
                    {
                        UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                        int fd = connectedDevices.get(device.getDeviceId());
                        Log.d(TAG, "device: " + device.getDeviceId() + " disconnected. fd: " + fd);
                        notifyDeviceDetached(fd);
                        connectedDevices.remove(device.getDeviceId());
                    }
                }
            }
            catch(Exception e)
            {
                Log.d(TAG, "Exception: " + e);
            }
        }
    };

    private void checkForDevices()
    {
        HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();

        while(deviceIterator.hasNext())
        {
            UsbDevice device = deviceIterator.next();
            Log.d(TAG, "Found a device: " + device);
            manager.requestPermission(device, mPermissionIntent);
        }
    }
}
