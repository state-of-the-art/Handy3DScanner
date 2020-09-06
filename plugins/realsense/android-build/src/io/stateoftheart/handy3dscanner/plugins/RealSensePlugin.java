package io.stateoftheart.handy3dscanner.plugins;

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
import android.app.Service;
import android.os.IBinder;

public class RealSensePlugin extends Service
{
    private String TAG = "Handy3DScanner::RealSensePlugin";

    private static final String ACTION_USB_PERMISSION = "io.stateoftheart.handy3dscanner.plugins.RealSensePlugin.USB_PERMISSION";
    private PendingIntent mPermissionIntent;
    private UsbManager manager;
    private UsbDeviceConnection connection;
    private HashMap<Integer, Integer> connectedDevices = new HashMap<Integer, Integer>();

    public RealSensePlugin() {
        super();
        Log.d(TAG, "Creating plugin java object");
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        super.onCreate();

        manager = (UsbManager) getSystemService(Context.USB_SERVICE);

        registerReceiver(usbManagerBroadcastReceiver, new IntentFilter(UsbManager.ACTION_USB_DEVICE_ATTACHED));
        registerReceiver(usbManagerBroadcastReceiver, new IntentFilter(UsbManager.ACTION_USB_DEVICE_DETACHED));

        mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
        registerReceiver(usbManagerBroadcastReceiver, new IntentFilter(ACTION_USB_PERMISSION));

        final Handler handler = new Handler();

        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                checkForDevices();
            }
        }, 1000);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");

        // Do not receive new devices anymore
        unregisterReceiver(usbManagerBroadcastReceiver);

        // Detach the connected devices
        for( HashMap.Entry<Integer, Integer> pair: connectedDevices.entrySet() ) {
            notifyDeviceDetached(pair.getValue());
            Log.d(TAG, "device: " + pair.getKey() + " disconnected. fd: " + pair.getValue());
        }

        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);

        Log.d(TAG, "onStartCommand");

        return ret;
    }

    private static native void notifyDeviceAttached(String name, int fd);
    private static native void notifyDeviceDetached(int fd);
    private static native void appNotice(String message);
    private static native void appWarning(String message);
    private static native void appError(String message);

    private final BroadcastReceiver usbManagerBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            try {
                String action = intent.getAction();
                if( ACTION_USB_PERMISSION.equals(action) ) {
                    Log.d(TAG, "onUsbPermission");

                    synchronized(this) {
                        UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                        if( intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false) ) {
                            if( device != null ) {
                                connection = manager.openDevice(device);
                                Log.d(TAG, "inserting device with id: " + device.getDeviceId() + " and file descriptor: " + connection.getFileDescriptor());
                                connectedDevices.put(device.getDeviceId(), connection.getFileDescriptor());
                                notifyDeviceAttached(device.getDeviceName(), connection.getFileDescriptor());
                                Log.d(TAG, "device attached: " + device.getProductName() + " - " + device);
                                appNotice("device attached: " + device.getProductName());
                            }
                        } else {
                            Log.d(TAG, "permission denied for device " + device);
                            appWarning("permission denied for device " + device.getProductName());
                            appError("If you did not saw the request to give permission - it's probably the Android 10 bug: https://github.com/state-of-the-art/Handy3DScanner/issues/68");
                        }
                    }
                }

                if( UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action) ) {
                    Log.d(TAG, "onDeviceConnected");

                    synchronized(this) {
                        UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                        if( device != null ) {
                            appNotice("device connected: " + device.getProductName());
                            manager.requestPermission(device, mPermissionIntent);
                        }
                    }
                }

                if( UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action) ) {
                    Log.d(TAG, "onDeviceDisconnected");

                    synchronized(this) {
                        UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                        int fd = connectedDevices.get(device.getDeviceId());
                        Log.d(TAG, "device: " + device.getDeviceId() + " disconnected. fd: " + fd);
                        appNotice("device detached: " + device.getProductName());
                        notifyDeviceDetached(fd);
                        connectedDevices.remove(device.getDeviceId());
                    }
                }
            }
            catch(Exception e) {
                Log.d(TAG, "Exception: " + e);
            }
        }
    };

    private void checkForDevices() {
        Log.d(TAG, "checkForDevices()");
        HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
        Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();

        while( deviceIterator.hasNext() ) {
            UsbDevice device = deviceIterator.next();
            Log.d(TAG, "Found a device: " + device);
            manager.requestPermission(device, mPermissionIntent);
        }
    }
}
