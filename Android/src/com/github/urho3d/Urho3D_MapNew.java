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
import com.amap.api.maps.MapView;
import android.widget.RelativeLayout;
import android.widget.LinearLayout;

import android.content.IntentFilter;

import android.content.Context;
import android.view.View;
import android.widget.Toast;
import android.view.KeyEvent;

import java.util.Date;
import android.view.*;

import com.amap.api.maps.model.LatLng;
import com.amap.api.maps.model.Poi;
import com.amap.api.navi.*;
import com.amap.api.navi.enums.AMapNaviOnlineCarHailingType;
import com.amap.api.navi.model.AMapCarInfo;
import com.amap.api.navi.model.AMapNaviLocation;
import com.amap.api.navi.model.NaviInfo;
import com.amap.api.navi.model.NaviLatLng;

import com.amap.api.maps.CameraUpdateFactory;

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
import com.amap.api.navi.AimlessModeListener;
import com.amap.api.navi.model.NaviInfo;
import com.amap.api.navi.model.NaviLatLng;
import com.amap.api.navi.enums.NaviType;
import com.amap.api.navi.model.AMapExitDirectionInfo;
import android.location.Location;
import com.amap.api.navi.enums.AimLessMode;

import java.time.LocalTime;
import com.amap.api.maps.AMap;

import com.amap.api.maps.model.BitmapDescriptorFactory;
import com.amap.api.maps.model.LatLng;
import com.amap.api.maps.model.Marker;
import com.amap.api.maps.model.MarkerOptions;
import com.amap.api.maps.model.MyLocationStyle;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Point;

public class Urho3D_MapNew extends SDLActivity  implements AMapNaviViewListener, AMapNaviListener, AMap.OnMapLongClickListener, AimlessModeListener {

    static final String SCRIPTS = "scripts";
    static final String PICKED_SCRIPT = "pickedScript";
    private static final String TAG = "Urho3D";
    private static final int OBTAINING_SCRIPT = 1;
    private static String[] mArguments = new String[0];

    protected AMapNavi mAMapNavi;
    protected MapView mAmapView;
    protected AMap mAmap;

    // 是否需要跟随定位
    private boolean isNeedFollow = true;

    private static final int SET_IP = 2;
    private static final int NAVI_INIT = 4;
    private static final int NAVI_ROUTE_NOTIFY = 5;
    private static final int NAVI_ARRIVED = 6;
    private static final int NAVI_TEXT = 7;
    private static final int NAVI_INFO = 8;
    private static final int NAVI_MAP_TYPE = 9;
    private static final int NAVI_CAMERA_INFO = 10;
    private static final int NAVI_INFO2 = 11;
    private static final int NAVI_FACILITY = 12;

    private boolean init_called = false;

    public static final int RUN_NAVI = 0;
    public static final int RUN_SIM = 1;
    public static final int RUN_MAP = 2;
    public static int run_type = RUN_NAVI;

    private static final int FROM_NATIVE_GPS = 100;

    public static String IP_ADDRESS;

    private Marker myLocationMarker;
    // 处理静止后跟随的timer
    private Timer needFollowTimer;
    // 屏幕静止DELAY_TIME之后，再次跟随
    private long DELAY_TIME = 5000;

    private int BASE_ZOOM = 17;

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

        mAMapNavi.addAMapNaviListener(this);

        mAmapView.onCreate(savedInstanceState);
        mAmap = mAmapView.getMap();
        mAmap.setOnMapLongClickListener(this);

        Log.i(TAG, "startAimlessMode");
        mAMapNavi.startAimlessMode(AimLessMode.CAMERA_AND_SPECIALROAD_DETECTED);
        mAMapNavi.addAimlessModeListener(this);
        mAmap.setTrafficEnabled(true);

        //mAmap.setLocationSource(this);//设置了定位的监听,这里要实现LocationSource接口
        //mAmap.getUiSettings().setMyLocationButtonEnabled(true); // 是否显示定位按钮
        //mAmap.setMyLocationEnabled(true);//显示定位层并且可以触发定位,默认是flase
        mAmap.moveCamera(CameraUpdateFactory.zoomTo(BASE_ZOOM));//设置地图缩放级别
//        MyLocationStyle myLocationStyle = new MyLocationStyle();//初始化定位蓝点样式类
//        myLocationStyle.myLocationType(MyLocationStyle.LOCATION_TYPE_LOCATE);//连续定位、且将视角移动到地图中心点，定位点依照设备方向旋转，并且会跟随设备移动。（1秒1次定位）如果不设置myLocationType，默认也会执行此种模式。
//        myLocationStyle.strokeColor(Color.TRANSPARENT);//设置定位蓝点精度圆圈的边框颜色
//        myLocationStyle.radiusFillColor(Color.TRANSPARENT);//设置定位蓝点精度圆圈的填充颜色
//        mAmap.setMyLocationStyle(myLocationStyle);//设置定位蓝点的Style

        LocalTime time = LocalTime.now();
        if (time.getHour() >= 17 || time.getHour() <= 5)
        {
            mAmap.setMapType(AMap.MAP_TYPE_NIGHT);
        }
        //

        init_called = false;

        // 初始化 显示我的位置的Marker
        myLocationMarker = mAmap.addMarker(new MarkerOptions()
                .icon(BitmapDescriptorFactory.fromBitmap(BitmapFactory
                        .decodeResource(getResources(), R.drawable.car))));

        setMapInteractiveListener();

        // Log.i(TAG, "WTF !!!");
        SDLActivity.onNativeMessage(NAVI_INIT, (double)mAMapNavi.getNaviType(), 0, 0, "Init NAVI");
        SDLActivity.onNativeMessage(SET_IP, 0, 0, 0, IP_ADDRESS);
        SDLActivity.onNativeMessage(9, (double) mAmap.getMapType(), 0, 0, "");
    }

    @Override
    protected void onInitLayout() {
        setContentView(R.layout.activity_basic_map);
        mSurface = findViewById(R.id.sdl_view);
        mAmapView = findViewById(R.id.map_view);
        mAMapNavi = AMapNavi.getInstance(getApplicationContext());
        mLayout = findViewById(R.id.basic_map);
        Log.i(TAG, "layout init OK");
    }

    /**
     * 方法必须重写
     */
    @Override
    protected void onResume() {
        super.onResume();
        mAmapView.onResume();
    }

    /**
     * 方法必须重写
     */
    @Override
    protected void onPause() {
        super.onPause();
        mAmapView.onPause();
    }

    /**
     * 方法必须重写
     */
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mAmapView.onSaveInstanceState(outState);
    }

    @Override
    protected void onDestroy() {
        Log.i(TAG, "onDestroy !!!");
        mAMapNavi.stopAimlessMode();
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
    public void onInitNaviFailure() {
    }

    @Override
    public void onInitNaviSuccess() {
    }

    @Override
    public void onStartNavi(int type) {
        //开始导航回调
        SDLActivity.onNativeMessage(0, (double) type, 0, 0, "");
    }

    @Override
    public void onTrafficStatusUpdate() {
        //
    }

    @Override
    public void onLocationChange(AMapNaviLocation location) {
        //当前位置回调

        if (location != null) {
            LatLng latLng = new LatLng(location.getCoord().getLatitude(),
                    location.getCoord().getLongitude());
            // 显示定位小图标，初始化时已经创建过了，这里修改位置即可
            myLocationMarker.setPosition(latLng);
            if (isNeedFollow) {
                // 跟随
                mAmap.animateCamera(CameraUpdateFactory.changeLatLng(latLng));
                mAmap.moveCamera(CameraUpdateFactory.zoomTo(BASE_ZOOM));//设置地图缩放级别
            }
        } else {
//            Toast.makeText(IntelligentBroadcastActivity.this, "定位出现异常",
//                    Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onGetNavigationText(int type, String text) {
        //播报类型和播报文字回调
        SDLActivity.onNativeMessage(NAVI_TEXT, 0, 0, 0, text);
    }

    @Override
    public void onGetNavigationText(String s) {
        SDLActivity.onNativeMessage(NAVI_TEXT, 0, 0, 0, s);
    }

    @Override
    public void onEndEmulatorNavi() {
        //结束模拟导航
    }

    @Override
    public void onArriveDestination() {
        //到达目的地
        SDLActivity.onNativeMessage(NAVI_ARRIVED, 0, 0, 0, "");
    }

    @Override
    public void onCalculateRouteFailure(int errorInfo) {
        //路线计算失败
        //Log.e("dm", "--------------------------------------------");
        //Log.i("dm", "路线计算失败：错误码=" + errorInfo + ",Error Message= " + ErrorInfo.getError(errorInfo));
        //Log.i("dm", "错误码详细链接见：http://lbs.amap.com/api/android-navi-sdk/guide/tools/errorcode/");
        //Log.e("dm", "--------------------------------------------");
        //Toast.makeText(this, "errorInfo：" + errorInfo + ",Message：" + ErrorInfo.getError(errorInfo), Toast.LENGTH_LONG).show();
    }

    @Override
    public void onReCalculateRouteForYaw() {
        //偏航后重新计算路线回调
    }

    @Override
    public void onReCalculateRouteForTrafficJam() {
        //拥堵后重新计算路线回调
    }

    @Override
    public void onArrivedWayPoint(int wayID) {
        //到达途径点
    }

    @Override
    public void onGpsOpenStatus(boolean enabled) {
        //GPS开关状态回调
    }

    @Override
    public void onNaviSetting() {
        //底部导航设置点击回调
    }

    @Override
    public void onNaviMapMode(int naviMode) {
        //导航态车头模式，0:车头朝上状态；1:正北朝上模式。
    }

    @Override
    public void onNaviCancel() {
        finish();
    }


    @Override
    public void onNaviTurnClick() {
        //转弯view的点击回调
    }

    @Override
    public void onNextRoadClick() {
        //下一个道路View点击回调
    }


    @Override
    public void onScanViewButtonClick() {
        //全览按钮点击回调

    }

    @Override
    public void updateCameraInfo(AMapNaviCameraInfo[] aMapCameraInfos) {
        try {
            for (int i = 0; i < aMapCameraInfos.length; ++i) {
                SDLActivity.onNativeMessage(NAVI_CAMERA_INFO,
                        (double) aMapCameraInfos[i].getCameraType(),
                        (double) aMapCameraInfos[i].getCameraSpeed(),
                        (double) aMapCameraInfos[i].getCameraDistance(),
                        "");
            }
        }
        catch(Exception e) {
            Log.w(TAG, e.getMessage());
        }
    }

    @Override
    public void onServiceAreaUpdate(AMapServiceAreaInfo[] amapServiceAreaInfos) {

    }

    @Override
    public void onNaviInfoUpdate(NaviInfo info) {

        if (!init_called) {
            init_called = true;
            SDLActivity.onNativeMessage(NAVI_INIT, (double) mAMapNavi.getNaviType(), 0, 0, "Init NAVI");
            SDLActivity.onNativeMessage(NAVI_MAP_TYPE, (double) mAmap.getMapType(), 0, 0, "");
        }

        //导航过程中的信息更新，请看NaviInfo的具体说明
        String info_text = "";

        try {
            AMapExitDirectionInfo exit = info.getExitDirectionInfo();
            if (exit.getExitNameInfo().length > 0) {
                info_text = exit.getExitNameInfo()[0];
            }
        }
        catch(Exception e) {
            Log.w(TAG, e.getMessage());
        }

        SDLActivity.onNativeMessage(NAVI_INFO,
                (double) info.getCurStepRetainDistance(),
                (double) info.getCurStepRetainTime(),
                (double) info.getIconType(),
                info_text);

        SDLActivity.onNativeMessage(NAVI_INFO2,
                (double) info.getPathRetainDistance(),
                (double) info.getPathRetainTime(),
                (double) info.getCurStep(),
                info.getNextRoadName());

//        Log.d(TAG, info.getCurrentRoadName() + " retain_dist:" + info.getCurStepRetainDistance());
//
//        AMapExitDirectionInfo exit = info.getExitDirectionInfo();
//        for (int i=0; i<exit.getDirectionInfo().length; ++i)
//        {
//            Log.d(TAG, "exit dir: " + exit.getDirectionInfo()[i]);
//        }
//
//        for (int i=0; i<exit.getExitNameInfo().length; ++i)
//        {
//            Log.d(TAG, "exit name: " + exit.getExitNameInfo()[i]);
//        }
    }

    @Override
    public void showCross(AMapNaviCross aMapNaviCross) {
        //显示转弯回调
    }

    @Override
    public void hideCross() {
        //隐藏转弯回调
    }

    @Override
    public void showLaneInfo(AMapLaneInfo[] laneInfos, byte[] laneBackgroundInfo, byte[] laneRecommendedInfo) {
        //显示车道信息

    }

    @Override
    public void hideLaneInfo() {
        //隐藏车道信息
    }

    @Override
    public void onCalculateRouteSuccess(int[] ints) {
        //多路径算路成功回调
        SDLActivity.onNativeMessage(2, 0, 0, 0, "onCalculateRouteSuccess");
    }

    @Override
    public void notifyParallelRoad(int i) {
    }

    @Override
    public void updateAimlessModeStatistics(AimLessModeStat aimLessModeStat) {
        //更新巡航模式的统计信息
    }


    @Override
    public void updateAimlessModeCongestionInfo(AimLessModeCongestionInfo aimLessModeCongestionInfo) {
        //更新巡航模式的拥堵信息
    }

    @Override
    public void onPlayRing(int i) {

    }

    @Override
    public void onLockMap(boolean isLock) {
        //锁地图状态发生变化时回调
    }

    @Override
    public void onNaviViewLoaded() {
        Log.d(TAG, "导航页面加载成功");
        Log.d(TAG, "请不要使用AMapNaviView.getMap().setOnMapLoadedListener();会overwrite导航SDK内部画线逻辑");
    }

    @Override
    public void onMapTypeChanged(int i) {
        SDLActivity.onNativeMessage(NAVI_MAP_TYPE, (double) i, 0, 0, "");
    }

    @Override
    public void onNaviViewShowMode(int i) {

    }

    @Override
    public boolean onNaviBackClick() {
        return false;
    }


    @Override
    public void showModeCross(AMapModelCross aMapModelCross) {

    }

    @Override
    public void hideModeCross() {

    }

    @Override
    public void updateIntervalCameraInfo(AMapNaviCameraInfo aMapNaviCameraInfo, AMapNaviCameraInfo aMapNaviCameraInfo1, int i) {

    }

    @Override
    public void showLaneInfo(AMapLaneInfo aMapLaneInfo) {

    }

    @Override
    public void onCalculateRouteSuccess(AMapCalcRouteResult aMapCalcRouteResult) {

    }

    @Override
    public void onCalculateRouteFailure(AMapCalcRouteResult aMapCalcRouteResult) {

    }

    @Override
    public void onNaviRouteNotify(AMapNaviRouteNotifyData aData) {
        Log.i(TAG, "onNaviRouteNotify" + aData.getReason());
        SDLActivity.onNativeMessage(NAVI_ROUTE_NOTIFY,
                (double) aData.getDistance(),
                aData.getLatitude(),
                aData.getLongitude(),
                aData.getReason());
    }

    @Override
    protected void onMessage2(int cmd, double data1, double data2, double data3, double data4, double data5)
    {
        Log.i(TAG, "onMessage2 " + cmd + " " + data1 + " " + data2 + " " + data3 + " " + data4 + " " + data5);
        if (cmd == FROM_NATIVE_GPS)
        {
            Location location = new Location("GPS");
            location.setLongitude(data1);
            location.setLatitude(data2);
            location.setSpeed((float)data3);
            location.setAccuracy((float)data4);
            location.setBearing((float)data5);
            location.setTime(System.currentTimeMillis());
            //以上6项数据缺一不可
            Log.i(TAG, "setExtraGPSData");
            mAMapNavi.setExtraGPSData(1,location);
            //type字段传1时代表WGS84坐标；
        }
    }

    //@Override
    public void onGpsSignalWeak(boolean var1) {

    }

    @Override
    public void onMapLongClick(LatLng var1)
    {
        Log.i(TAG, "onMapLongClick!!!");
        int map_type = mAmap.getMapType();
        if (map_type == AMap.MAP_TYPE_SATELLITE)
        {
            mAmap.setMapType(AMap.MAP_TYPE_NORMAL);
            LocalTime time = LocalTime.now();
            if (time.getHour() >= 17 || time.getHour() <= 5)
            {
                mAmap.setMapType(AMap.MAP_TYPE_NIGHT);
            }
        }
        else
        {
            mAmap.setMapType(AMap.MAP_TYPE_SATELLITE);
        }
    }

    @Override
    public void OnUpdateTrafficFacility(AMapNaviTrafficFacilityInfo var1) {
        try {
            SDLActivity.onNativeMessage(NAVI_FACILITY,
                    (double) var1.distance,
                    (double) var1.type,
                    (double) var1.limitSpeed,
                    "");
        }
        catch(Exception e) {
            Log.w(TAG, e.getMessage());
        }
    }

    @Override
    public void OnUpdateTrafficFacility(AMapNaviTrafficFacilityInfo[] var1) {
        try {
            for (int i = 0; i < var1.length; ++i) {
                SDLActivity.onNativeMessage(NAVI_FACILITY,
                        (double) var1[i].distance,
                        (double) var1[i].type,
                        (double) var1[i].limitSpeed,
                        "");
            }
        }
        catch(Exception e) {
            Log.w(TAG, e.getMessage());
        }
    }


    @Override
    public void onUpdateTrafficFacility(AMapNaviTrafficFacilityInfo[] var1)
    {
        try {
            for (int i = 0; i < var1.length; ++i) {
                SDLActivity.onNativeMessage(NAVI_FACILITY,
                        (double) var1[i].distance,
                        (double) var1[i].type,
                        (double) var1[i].limitSpeed,
                        "");
            }
        }
        catch(Exception e) {
            Log.w(TAG, e.getMessage());
        }
    }

    @Override
    public void onUpdateAimlessModeElecCameraInfo(AMapNaviTrafficFacilityInfo[] var1)
    {
        try {
            for (int i = 0; i < var1.length; ++i) {
                SDLActivity.onNativeMessage(NAVI_FACILITY,
                        (double) var1[i].distance,
                        (double) var1[i].type,
                        (double) var1[i].limitSpeed,
                        "");
            }
        }
        catch(Exception e) {
            Log.w(TAG, e.getMessage());
        }
    }

    /**
     * 设置导航监听
     */
    private void setMapInteractiveListener() {

        mAmap.setOnMapTouchListener(new AMap.OnMapTouchListener() {

            @Override
            public void onTouch(MotionEvent event) {

                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        // 按下屏幕
                        // 如果timer在执行，关掉它
                        clearTimer();
                        // 改变跟随状态
                        isNeedFollow = false;
                        break;

                    case MotionEvent.ACTION_UP:
                        // 离开屏幕
                        startTimerSomeTimeLater();
                        break;

                    default:
                        break;
                }
            }
        });

    }

    /**
     * 取消timer任务
     */
    private void clearTimer() {
        if (needFollowTimer != null) {
            needFollowTimer.cancel();
            needFollowTimer = null;
        }
    }

    /**
     * 如果地图在静止的情况下
     */
    private void startTimerSomeTimeLater() {
        // 首先关闭上一个timer
        clearTimer();
        needFollowTimer = new Timer();
        // 开启一个延时任务，改变跟随状态
        needFollowTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                isNeedFollow = true;
            }
        }, DELAY_TIME);
    }
}
