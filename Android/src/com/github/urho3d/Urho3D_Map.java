//
// Copyright (c) 2008-2018 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

package com.github.urho3d;

import android.content.Intent;
import android.content.res.AssetManager;
import android.util.Log;
import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

import java.io.IOException;
import java.util.*;
import android.os.*;

import android.widget.RelativeLayout;
import android.widget.LinearLayout;

import android.content.IntentFilter;

import android.content.Context;
import android.view.View;
import android.widget.Toast;
import android.view.KeyEvent;

import java.util.Date;
import android.view.*;

import com.amap.api.maps.AMap;
import com.amap.api.maps.MapView;
import com.amap.api.maps.model.LatLng;
import com.amap.api.maps.model.MyLocationStyle;
import com.amap.api.maps.model.MyTrafficStyle;
import com.amap.api.maps.model.Poi;
import com.amap.api.navi.*;
import com.amap.api.navi.model.AMapNaviLocation;
import com.amap.api.navi.model.NaviInfo;
import com.amap.api.navi.model.NaviLatLng;

import com.amap.api.navi.AMapNavi;
import com.amap.api.navi.AMapNaviListener;
import com.amap.api.navi.AMapNaviView;
import com.amap.api.navi.AMapNaviViewListener;
import com.amap.api.navi.model.AMapCalcRouteResult;
import com.amap.api.navi.model.AMapLaneInfo;
import com.amap.api.navi.model.AMapModelCross;
import com.amap.api.navi.model.AMapNaviCameraInfo;
import com.amap.api.navi.model.AMapNaviCross;
import com.amap.api.navi.model.AMapNaviLocation;
import com.amap.api.navi.model.AMapNaviRouteNotifyData;
import com.amap.api.navi.model.AMapNaviTrafficFacilityInfo;
import com.amap.api.navi.model.AMapServiceAreaInfo;
import com.amap.api.navi.model.AimLessModeCongestionInfo;
import com.amap.api.navi.model.AimLessModeStat;
import com.amap.api.navi.model.NaviInfo;
import com.amap.api.navi.model.NaviLatLng;
import com.amap.api.navi.enums.NaviType;
import com.amap.api.navi.model.AMapExitDirectionInfo;

import android.Manifest;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.support.design.widget.Snackbar;
import android.support.design.widget.TabLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AlertDialog;

import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationClientOption;
import com.amap.api.location.AMapLocationListener;
import com.amap.api.maps.AMap;
import com.amap.api.maps.CameraUpdateFactory;
import com.amap.api.maps.LocationSource;
import com.amap.api.maps.MapView;
import com.amap.api.maps.UiSettings;

import java.time.LocalTime;
import com.amap.api.maps.model.MyLocationStyle;

public class Urho3D_Map extends SDLActivity implements  LocationSource, AMapLocationListener, AMap.OnMapLongClickListener {

    static final String SCRIPTS = "scripts";
    static final String PICKED_SCRIPT = "pickedScript";
    private static final String TAG = "Urho3D";
    private static final int OBTAINING_SCRIPT = 1;
    private static String[] mArguments = new String[0];

    private AMap amap = null;
    private MapView mapview = null;

    private OnLocationChangedListener mListener;
    private AMapLocationClient mlocationClient;
    private AMapLocationClientOption mLocationOption;

    public static String IP_ADDRESS;

    @Override
    protected String[] getArguments() {
        return mArguments;
    }

    @Override
    protected boolean onLoadLibrary(ArrayList<String> libraryNames) {
        // Ensure "Urho3D" shared library (if any) and "Urho3DPlayer" are being sorted to the top of the list
        // Also ensure STL runtime shared library (if any) is sorted to the top most entry
        Collections.sort(libraryNames, new Comparator<String>() {
            private String sortName(String name) {
                return name.matches("^\\d+_.+$") ? name : (name.matches("^.+_shared$") ? "0000_" : "000_") + name;
            }

            @Override
            public int compare(String lhs, String rhs) {
                return sortName(lhs).compareTo(sortName(rhs));
            }
        });

        // All shared shared libraries must always be loaded if available, so exclude it from return result and all list operations below
        int startIndex = libraryNames.indexOf("Game");

        // Determine the intention
        Intent intent = getIntent();
        String pickedLibrary = intent.getStringExtra(SampleLauncher.PICKED_LIBRARY);
        if (pickedLibrary == null) {
            // Intention for obtaining library names
            String[] array = libraryNames.subList(startIndex, libraryNames.size()).toArray(new String[libraryNames.size() - startIndex]);
            if (array.length > 1) {
                setResult(RESULT_OK, intent.putExtra(SampleLauncher.LIBRARY_NAMES, array));

                // End Urho3D activity lifecycle
                finish();

                // Return false to indicate no library is being loaded yet
                return false;
            } else {
                // There is only one library available, so cancel the intention for obtaining the library name and by not returning any result
                // However, since we have already started Urho3D activity, let's the activity runs its whole lifecycle by falling through to call the super implementation
                setResult(RESULT_CANCELED);
            }
        } else {
            // Intention for loading a picked library name (and remove all others)
            libraryNames.subList(startIndex, libraryNames.size()).clear();
            mArguments = pickedLibrary.split(":");
            libraryNames.add(mArguments[0]);
            if ("Urho3DPlayer".equals(mArguments[0]) && mArguments.length == 1) {
                // Urho3DPlayer needs a script name to play
                try {
                    final AssetManager assetManager = getAssets();
                    HashMap<String, ArrayList<String>> scripts = new HashMap<String, ArrayList<String>>(2) {{
                        put("AngelScript", new ArrayList<>(Arrays.asList(assetManager.list("Data/Scripts"))));
                        put("Lua", new ArrayList<>(Arrays.asList(assetManager.list("Data/LuaScripts"))));
                    }};
                    startActivityForResult(new Intent(this, ScriptPicker.class).putExtra(SCRIPTS, scripts), OBTAINING_SCRIPT);
                } catch (IOException e) {
                    Log.e(TAG, "Could not scan assets directory for playable scripts", e);
                }
            }
        }

        return super.onLoadLibrary(libraryNames);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        Log.i(TAG, "onCreate !!!");
        super.onCreate(savedInstanceState);
        mapview.onCreate(savedInstanceState);// 此方法必须重写
        checkPermission();
        SDLActivity.onNativeMessage(2, 1, 0, 0, IP_ADDRESS);
        Log.i(TAG, "onCreate 222 !!!");
    }

    @Override
    protected void onInitLayout()
    {
        setContentView(R.layout.activity_basic_map);
        mSurface = findViewById(R.id.sdl_view);
        mapview = findViewById(R.id.map_view);
        mLayout = findViewById(R.id.basic_map);
        Log.i(TAG, "layout init OK");
    }

    @Override
    protected void onDestroy() {

        Log.i(TAG, "onDestroy !!!");
        mapview.onDestroy();
        super.onDestroy();
    }

    @Override
    protected void onStop() {
        Log.i(TAG, "onStop !!!");
        super.onStop();
        finish();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (OBTAINING_SCRIPT != requestCode || RESULT_CANCELED == resultCode)
            return;
        String script = data.getStringExtra(PICKED_SCRIPT);
        script = (script.endsWith(".as") ? "Scripts/" : "LuaScripts/") + script;
        mArguments = new String[]{mArguments[0], script};
    }

    @Override
    protected void onResume() {
        Log.i(TAG, "onResume !!!");
        super.onResume();
        mapview.onResume();
    }

    @Override
    protected void onPause() {
        Log.i(TAG, "onResume !!!");
        super.onPause();
        mapview.onPause();
    }


    private void checkPermission() {
        XPermissionUtils.requestPermissions(this, 100, new String[]{
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.ACCESS_COARSE_LOCATION,
                        Manifest.permission.ACCESS_FINE_LOCATION,
                        Manifest.permission.READ_PHONE_STATE},
                new XPermissionUtils.OnPermissionListener() {
                    @Override
                    public void onPermissionGranted() {
                        initMap();
                    }

                    @Override
                    public void onPermissionDenied(final String[] deniedPermissions, boolean alwaysDenied) {
                        new AlertDialog.Builder(Urho3D_Map.this).setTitle("温馨提示")
                                .setMessage("此处需要定位等权限才能正常使用")
                                .setNegativeButton("取消", new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        dialog.dismiss();
                                        finish();
                                    }
                                })
                                .setPositiveButton("验证权限", new DialogInterface.OnClickListener() {
                                    @RequiresApi(api = Build.VERSION_CODES.M)
                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        XPermissionUtils.requestPermissionsAgain(Urho3D_Map.this, deniedPermissions,
                                                100);
                                    }
                                })
                                .show();
                    }
                });
    }

    /**
     * 初始化AMap对象
     */
    private void initMap() {
        if (amap == null) {
            amap = mapview.getMap();
            amap.setTrafficEnabled(true);
            //MyLocationStyle style = new MyLocationStyle();
            //style.myLocationType(MyLocationStyle.LOCATION_TYPE_FOLLOW);
            //amap.setMyLocationStyle(style);

            amap.setOnMapLongClickListener(this);

            //设置显示定位按钮 并且可以点击
            UiSettings settings = amap.getUiSettings();
            amap.setLocationSource(this);//设置了定位的监听,这里要实现LocationSource接口
            // 是否显示定位按钮
            settings.setMyLocationButtonEnabled(true);
            settings.setCompassEnabled(true);

            amap.setMyLocationEnabled(true);//显示定位层并且可以触发定位,默认是flase
            amap.moveCamera(CameraUpdateFactory.zoomTo(15));

            LocalTime time = LocalTime.now();
            if (time.getHour() >= 17 || time.getHour() <= 5)
            {
                amap.setMapType(AMap.MAP_TYPE_NIGHT);
            }
            SDLActivity.onNativeMessage(9, (double) amap.getMapType(), 0, 0, "");
        }
    }


    /**
     * 定位地点
     *
     * @param amapLocation
     */
    @Override
    public void onLocationChanged(AMapLocation amapLocation) {
        if (mListener != null && amapLocation != null) {
            if (amapLocation != null
                    && amapLocation.getErrorCode() == 0) {
                mListener.onLocationChanged(amapLocation);// 显示系统小蓝点

            } else {
                String errText = "定位失败," + amapLocation.getErrorCode() + ": " + amapLocation.getErrorInfo();
                Log.e(TAG, errText);
            }
        }
    }

    /**
     * 激活定位
     *
     * @param listener
     */
    @Override
    public void activate(OnLocationChangedListener listener) {
        mListener = listener;
        if (mlocationClient == null) {
            //初始化定位
            mlocationClient = new AMapLocationClient(this);
            //初始化定位参数
            mLocationOption = new AMapLocationClientOption();
            //设置定位回调监听
            mlocationClient.setLocationListener(this);
            //设置为高精度定位模式
            mLocationOption.setLocationMode(AMapLocationClientOption.AMapLocationMode.Hight_Accuracy);
//            mLocationOption.setOnceLocation(true);
            //设置定位参数
            mlocationClient.setLocationOption(mLocationOption);
            // 此方法为每隔固定时间会发起一次定位请求，为了减少电量消耗或网络流量消耗，
            // 注意设置合适的定位时间的间隔（最小间隔支持为2000ms），并且在合适时间调用stopLocation()方法来取消定位请求
            // 在定位结束后，在合适的生命周期调用onDestroy()方法
            // 在单次定位情况下，定位无论成功与否，都无需调用stopLocation()方法移除请求，定位sdk内部会移除
            mlocationClient.startLocation();//启动定位
        }
    }

    /**
     * 注销定位
     */
    @Override
    public void deactivate() {
        mListener = null;
        if (mlocationClient != null) {
            mlocationClient.stopLocation();
            mlocationClient.onDestroy();
        }
        mlocationClient = null;
    }

    @Override
    protected void onMessage2(int cmd, double data1, double data2, double data3, double data4, double data5)
    {
        Log.i(TAG, "onMessage2 " + cmd + " " + data1 + " " + data2 + " " + data3);
    }

    @Override
    public void onMapLongClick(LatLng var1)
    {
        Log.i(TAG, "onMapLongClick!!!");
        int map_type = amap.getMapType();
        if (map_type == AMap.MAP_TYPE_SATELLITE)
        {
            amap.setMapType(AMap.MAP_TYPE_NORMAL);
            LocalTime time = LocalTime.now();
            if (time.getHour() >= 17 || time.getHour() <= 5)
            {
                amap.setMapType(AMap.MAP_TYPE_NIGHT);
            }
        }
        else
        {
            amap.setMapType(AMap.MAP_TYPE_SATELLITE);
        }
    }
}
