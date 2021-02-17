package com.github.urho3d;

import android.Manifest;
import android.content.Context;
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
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.util.SparseArray;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;


import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationClientOption;
import com.amap.api.location.AMapLocationListener;
import com.amap.api.maps.AMap;
import com.amap.api.maps.CameraUpdateFactory;
import com.amap.api.maps.LocationSource;
import com.amap.api.maps.MapView;
import com.amap.api.maps.UiSettings;
import com.amap.api.navi.AMapNavi;
import com.amap.api.navi.AMapNaviListener;
import com.amap.api.navi.enums.NaviType;
import com.amap.api.navi.model.AMapCalcRouteResult;
import com.amap.api.navi.model.AMapCarInfo;
import com.amap.api.navi.model.AMapLaneInfo;
import com.amap.api.navi.model.AMapModelCross;
import com.amap.api.navi.model.AMapNaviCameraInfo;
import com.amap.api.navi.model.AMapNaviCross;
import com.amap.api.navi.model.AMapNaviLocation;
import com.amap.api.navi.model.AMapNaviPath;
import com.amap.api.navi.model.AMapNaviRouteNotifyData;
import com.amap.api.navi.model.AMapNaviTrafficFacilityInfo;
import com.amap.api.navi.model.AMapServiceAreaInfo;
import com.amap.api.navi.model.AimLessModeCongestionInfo;
import com.amap.api.navi.model.AimLessModeStat;
import com.amap.api.navi.model.NaviInfo;
import com.amap.api.navi.model.NaviLatLng;
import com.amap.api.navi.view.RouteOverLay;
import com.amap.api.services.core.LatLonPoint;
import com.amap.api.services.help.Tip;
import com.zhy.adapter.recyclerview.CommonAdapter;
import com.zhy.adapter.recyclerview.base.ViewHolder;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

import static com.github.urho3d.R.id.ll_itemview;

public class NavigationActivity extends AppCompatActivity implements View.OnClickListener, LocationSource, AMapLocationListener, AMapNaviListener {
    private static final String TAG = "Urho3D";
    private TabLayout mTabLayout;
    private TextView tvStart, tvEnd;
    private TextView tvNavi;
    private RelativeLayout oneWay;
    private TextView tvTime, tvLength;
    private AMap amap;
    private MapView mapview;
    private OnLocationChangedListener mListener;
    private AMapLocationClient mlocationClient;
    private AMapLocationClientOption mLocationOption;
    private RecyclerView mRecyclerView;
    private CommonAdapter mAdapter;
    private int currentPosition, lastPosition = -1;
    private int BASE_ZOOM = 15;
    /**************************************************导航相关************************************** ********************/
    private AMapNavi mAMapNavi;
    /**
     * 起点坐标集合[由于需要确定方向，建议设置多个起点]
     */
    private List<NaviLatLng> startList = new ArrayList<NaviLatLng>();
    /**
     * 途径点坐标集合
     */
    private List<NaviLatLng> wayList = new ArrayList<NaviLatLng>();
    /**
     * 终点坐标集合［建议就一个终点］
     */
    private List<NaviLatLng> endList = new ArrayList<NaviLatLng>();
    /**
     * 保存当前算好的路线
     */
    private SparseArray<RouteOverLay> routeOverlays = new SparseArray<RouteOverLay>();
    private List<AMapNaviPath> ways = new ArrayList<>();
    private boolean calculateSuccess;
    private int routeIndex = 0;
    private int zindex = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_navigation);
        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null)
            actionBar.hide();
        EventBus.getDefault().register(this);
        initView();
        mapview.onCreate(savedInstanceState);// 此方法必须重写
        checkPermission();
        //int screenWidth = getWindowManager().getDefaultDisplay().getWidth(); // 屏幕宽（像素，如：480px）
        //int screenHeight = getWindowManager().getDefaultDisplay().getHeight(); // 屏幕高（像素，如：800p）
        amap.setTrafficEnabled(true);
        // amap.setMapType(AMap.MAP_TYPE_SATELLITE);
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
                        new AlertDialog.Builder(NavigationActivity.this).setTitle("温馨提示")
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
                                        XPermissionUtils.requestPermissionsAgain(NavigationActivity.this, deniedPermissions,
                                                100);
                                    }
                                })
                                .show();
                    }
                });
    }


    private void initView() {
        mapview = (MapView) findViewById(R.id.navi_view);
        mRecyclerView = (RecyclerView) findViewById(R.id.rl_rlv_ways);
        mRecyclerView.setLayoutManager(new GridLayoutManager(this, 3));
        mAdapter = getAdapter();
        mRecyclerView.setAdapter(mAdapter);
        oneWay = (RelativeLayout) findViewById(R.id.ll_rl_1way);
        tvTime = (TextView) findViewById(R.id.rl_tv_time);
        tvLength = (TextView) findViewById(R.id.rl_tv_length);
        tvStart = (TextView) findViewById(R.id.rl_tv_start);
        tvEnd = (TextView) findViewById(R.id.rl_tv_end);
        tvNavi = (TextView) findViewById(R.id.rl_tv_navistart);
        tvNavi.setOnClickListener(this);
        tvEnd.setOnClickListener(this);

        findViewById(R.id.rl_iv_back).setOnClickListener(this);

        mTabLayout = (TabLayout) findViewById(R.id.tabs);
        //tab的字体选择器,默认灰色,选择时白色
        mTabLayout.setTabTextColors(Color.LTGRAY, Color.WHITE);
        //设置tab的下划线颜色,默认是粉红色
        mTabLayout.setSelectedTabIndicatorColor(Color.WHITE);
        mTabLayout.addTab(mTabLayout.newTab().setText("白天"));
        mTabLayout.addTab(mTabLayout.newTab().setText("黑夜"));
        mTabLayout.addTab(mTabLayout.newTab().setText("卫星"));
        mTabLayout.addTab(mTabLayout.newTab().setText("导航"));

        //添加Tab点击事件
        mTabLayout.addOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                String tabName = tab.getText().toString();
                int mapType = AMap.MAP_TYPE_NORMAL;
                if (tabName.equals("卫星")) {
                    mapType = AMap.MAP_TYPE_SATELLITE;
                } else if (tabName.equals("黑夜")) {
                    mapType = AMap.MAP_TYPE_NIGHT;
                }
                else if (tabName.equals("导航")) {
                    mapType = AMap.MAP_TYPE_NAVI;
                }
                amap.setMapType(mapType);
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {

            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {

            }
        });
    }

    /**
     * 初始化AMap对象
     */
    private void initMap() {
        if (amap == null) {
            amap = mapview.getMap();
            //设置显示定位按钮 并且可以点击
            UiSettings settings = amap.getUiSettings();
            amap.setLocationSource(this);//设置了定位的监听,这里要实现LocationSource接口
            // 是否显示定位按钮
            settings.setMyLocationButtonEnabled(true);
            amap.setMyLocationEnabled(true);//显示定位层并且可以触发定位,默认是flase
            mAMapNavi = AMapNavi.getInstance(getApplicationContext());
            mAMapNavi.addAMapNaviListener(this);
            amap.moveCamera(CameraUpdateFactory.zoomTo(BASE_ZOOM));
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.rl_tv_end:
                Intent intent = new Intent(this, PoiSearchActivity.class);//拾取坐标点
                startActivity(intent);
                endList.clear();
                break;
            case R.id.rl_tv_navistart:
                clickNavigation();
                break;
            case R.id.rl_iv_back:
                finish();
                break;
        }
    }

    /**
     * 导航按钮点击事件实现方法
     */
    private void clickNavigation() {
        if (startList.size() == 0) {
            Snackbar.make(tvEnd, "未获取到当前位置，不能导航", Snackbar.LENGTH_SHORT).show();
        } else if (endList.size() == 0) {
            Snackbar.make(tvEnd, "未获取到终点，不能导航", Snackbar.LENGTH_SHORT).show();
        } else {
            if (!calculateSuccess) {
                Snackbar.make(tvEnd, "请先计算路线", Snackbar.LENGTH_SHORT).show();
                return;
            } else {//实时导航
                if (routeIndex > ways.size()) {
                    routeIndex = 0;
                }
                mAMapNavi.selectRouteId(routeOverlays.keyAt(routeIndex));
                //Intent gpsintent = new Intent(this, GPSNaviActivity.class);
                Intent gpsintent = new Intent(this, Urho3D.class);
                startActivity(gpsintent);

            }
        }
    }

    /**
     * 绘制路线
     *
     * @param routeId
     * @param path
     */
    private void drawRoutes(int routeId, AMapNaviPath path) {
        calculateSuccess = true;
        amap.moveCamera(CameraUpdateFactory.changeTilt(0));
        RouteOverLay routeOverLay = new RouteOverLay(amap, path, this);
        // routeOverLay.setTrafficLine(false);
        routeOverLay.addToMap();
        routeOverlays.put(routeId, routeOverLay);
    }


    /**
     * 获取终点信息
     *
     * @param tip
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onMessageEvent(Tip tip) {
        tvEnd.setText("到     " + tip.getName());
        LatLonPoint endLp = tip.getPoint();
        endList.clear();

        // tip.getName();

        SharedPreferences sharedPref = getSharedPreferences("com.op.urho3D", Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPref.edit();

        int num_of_history = 3;
        boolean update = false;
        for (int i=1; i<=num_of_history; ++i)
        {
            String keyName = "H_0" + Integer.toString(i);
            // Log.i("Urho3D", keyName);
            if (!sharedPref.contains(keyName))
            {
                Log.i("Urho3D", "put " + tip.getName());
                editor.putString(keyName, tip.getName());
                update = true;
                break;
            }
        }

        if (!update)
        {
            int index = sharedPref.getInt("H_index", 1);
            editor.putString("H_0" + index, tip.getName());
            ++ index;
            if (index > num_of_history)
            {
                index = 1;
            }
            editor.putInt("H_index", index);
        }

        editor.commit();


        endList.add(new NaviLatLng(endLp.getLatitude(), endLp.getLongitude()));
    }

    ;

    /**
     * 多条路线计算结果回调2
     *
     * @param ints
     */
//    @Override
    private void onCalculateMultipleRoutesSuccessOld(int[] ints) {
        //清空上次计算的路径列表。
        routeOverlays.clear();
        ways.clear();
        HashMap<Integer, AMapNaviPath> paths = mAMapNavi.getNaviPaths();
        for (int i = 0; i < ints.length; i++) {
            AMapNaviPath path = paths.get(ints[i]);
            if (path != null) {
                drawRoutes(ints[i], path);
                ways.add(path);
            }
        }
        if (ways.size() > 0) {
            currentPosition = 0;
            lastPosition = -1;
            mAdapter.notifyDataSetChanged();
            mRecyclerView.setVisibility(View.VISIBLE);
            oneWay.setVisibility(View.GONE);
            tvNavi.setText("开始导航");
        } else if (ways.size() == 1) {
            mRecyclerView.setVisibility(View.GONE);
            oneWay.setVisibility(View.VISIBLE);
            tvTime.setText(getTime(ways.get(0).getAllTime()));
            tvLength.setText(getLength(ways.get(0).getAllLength()));
            tvNavi.setText("开始导航");
        } else {
            mRecyclerView.setVisibility(View.GONE);
            tvNavi.setText("准备导航");
        }
        changeRoute();
    }

    /**
     * 单条路线计算结果回调2
     */
//    @Override
    private void onCalculateRouteSuccessOld() {
        /**
         * 清空上次计算的路径列表。
         */
        routeOverlays.clear();
        ways.clear();
        AMapNaviPath path = mAMapNavi.getNaviPath();
        /**
         * 单路径不需要进行路径选择，直接传入－1即可
         */
        drawRoutes(-1, path);
        mRecyclerView.setVisibility(View.GONE);
        oneWay.setVisibility(View.VISIBLE);
        tvTime.setText(getTime(path.getAllTime()));
        tvLength.setText(getLength(path.getAllLength()));
        tvNavi.setText("开始导航");
    }

    /**
     * 选择路线
     */
    public void changeRoute() {
        if (!calculateSuccess) {
            Toast.makeText(this, "请先算路", Toast.LENGTH_SHORT).show();
            return;
        }
        /**
         * 计算出来的路径只有一条
         */
        if (routeOverlays.size() == 1) {
            //必须告诉AMapNavi 你最后选择的哪条路
            mAMapNavi.selectRouteId(routeOverlays.keyAt(0));
            return;
        }

        Log.i(TAG, "ChangeRoute: " + routeIndex + " size=" + routeOverlays.size());

        if (routeIndex >= routeOverlays.size())
            routeIndex = 0;

        int routeID = routeOverlays.keyAt(routeIndex);
        //突出选择的那条路
        for (int i = 0; i < routeOverlays.size(); i++) {
            int key = routeOverlays.keyAt(i);
            routeOverlays.get(key).setTransparency(0.1f);
        }
        RouteOverLay routeOverlay = routeOverlays.get(routeID);
        if(routeOverlay != null) {
            routeOverlay.setTransparency(1);
            /**把用户选择的那条路的权值弄高，使路线高亮显示的同时，重合路段不会变的透明**/
            routeOverlay.setZindex(zindex++);
        }

        //必须告诉AMapNavi 你最后选择的哪条路
        mAMapNavi.selectRouteId(routeID);
        routeIndex++;

        // here is the bug that some time amap do not redraw if selected route changed
        // so I had to tell amp to redraw
        amap.moveCamera(CameraUpdateFactory.zoomBy(0.0f));
    }

    /**
     * 方法必须重写
     */
    @Override
    protected void onResume() {
        super.onResume();
        mapview.onResume();
        planRoute();//路线规划
    }

    /**
     * 清除当前地图上算好的路线
     */
    private void clearRoute() {
        for (int i = 0; i < routeOverlays.size(); i++) {
            RouteOverLay routeOverlay = routeOverlays.valueAt(i);
            routeOverlay.removeFromMap();
        }
        routeOverlays.clear();
        ways.clear();
    }

    /**
     * 路线规划
     */
    private void planRoute() {
        mRecyclerView.setVisibility(View.GONE);//多条路线规划结果
        oneWay.setVisibility(View.GONE);//一条路线规划结果
        if (startList.size() > 0 && endList.size() > 0) {
            int strategy = 0;
            try {
                /**
                 * 方法:
                 *   int strategy=mAMapNavi.strategyConvert(congestion, avoidhightspeed, cost, hightspeed, multipleroute);
                 * 参数:
                 * @congestion 躲避拥堵
                 * @avoidhightspeed 不走高速
                 * @cost 避免收费
                 * @hightspeed 高速优先
                 * @multipleroute 多路径
                 *
                 * 说明:
                 *      以上参数都是boolean类型，其中multipleroute参数表示是否多条路线，如果为true则此策略会算出多条路线。
                 * 注意:
                 *      不走高速与高速优先不能同时为true
                 *      高速优先与避免收费不能同时为true
                 */
                strategy = mAMapNavi.strategyConvert(false, false, false, true, true);
            } catch (Exception e) {
                e.printStackTrace();
            }
            mAMapNavi.calculateDriveRoute(startList, endList, wayList, strategy);
        }
    }


    @Override
    public void onCalculateRouteFailure(int i) {
        calculateSuccess = false;
        Snackbar.make(tvEnd, "计算路线失败", Snackbar.LENGTH_SHORT).show();

    }


    private CommonAdapter getAdapter() {

        return new CommonAdapter<AMapNaviPath>(this, R.layout.item_recycleview_naviways, ways) {
            private float maxWidth = 0;
            Handler handler = new Handler();

            /**
             * 初始化Item样式
             */
            private void initItemBackground(ViewHolder holder) {
                holder.getView(ll_itemview).setBackgroundResource(R.drawable.item_naviway_normal_bg);
                TextView tvTitle = holder.getView(R.id.ll_tv_labels);
                TextView tvTime = holder.getView(R.id.ll_tv_time);
                TextView tvLength = holder.getView(R.id.ll_tv_length);
                tvTitle.setTextColor(getResources().getColor(R.color.item_text_title_color));
                tvLength.setTextColor(getResources().getColor(R.color.item_text_title_color));
                tvTime.setTextColor(getResources().getColor(R.color.black));
                tvTitle.setBackgroundResource(R.drawable.item_naviway_title_normal);
            }

            /**
             * 选中的背景色修改
             */
            private void selectedBackground(ViewHolder holder) {
                holder.getView(ll_itemview).setBackgroundResource(R.drawable.item_naviway_selected_bg);
                TextView tvTitle = holder.getView(R.id.ll_tv_labels);
                TextView tvTime = holder.getView(R.id.ll_tv_time);
                TextView tvLength = holder.getView(R.id.ll_tv_length);
                tvTitle.setTextColor(Color.WHITE);
                tvTime.setTextColor(getResources().getColor(R.color.blue));
                tvLength.setTextColor(getResources().getColor(R.color.blue));
                tvTitle.setBackgroundResource(R.drawable.item_naviway_title_selected);
            }

            /**
             * 清除选中的样式
             */
            private void cleanSelector() {
                if (lastPosition != -1) {
                    View view = mRecyclerView.getChildAt(lastPosition);
                    view.setBackgroundResource(R.drawable.item_naviway_normal_bg);
                    TextView tvTitle = (TextView) view.findViewById(R.id.ll_tv_labels);
                    TextView tvTime = (TextView) view.findViewById(R.id.ll_tv_time);
                    TextView tvLength = (TextView) view.findViewById(R.id.ll_tv_length);
                    tvTitle.setTextColor(getResources().getColor(R.color.item_text_title_color));
                    tvLength.setTextColor(getResources().getColor(R.color.item_text_title_color));
                    tvTime.setTextColor(getResources().getColor(R.color.black));
                    tvTitle.setBackgroundResource(R.drawable.item_naviway_title_normal);
                }

            }

            /**
             * 固定宽度文字自适应大小(小屏幕手机换行效果需要)
             * @param textView
             * @param text
             */
            private void reSizeTextView(TextView textView, String text) {

                Paint paint = textView.getPaint();
                float textWidth = paint.measureText(text);
                float textSizeInDp = textView.getTextSize();

                if (textWidth > maxWidth) {
                    for (; textSizeInDp > 0; textSizeInDp -= 1) {
                        textView.setTextSize(textSizeInDp);
                        paint = textView.getPaint();
                        textWidth = paint.measureText(text);
                        if (textWidth <= maxWidth) {
                            break;
                        }
                    }
                }
                textView.invalidate();
                textView.setText(text);
            }

            @Override
            protected void convert(final ViewHolder holder, final AMapNaviPath aMapNaviPath, final int position) {
                final TextView tvTitle = holder.getView(R.id.ll_tv_labels);
                final TextView tvTime = holder.getView(R.id.ll_tv_time);
                final TextView tvLength = holder.getView(R.id.ll_tv_length);
                if (maxWidth == 0) {
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            maxWidth = tvTitle.getWidth() - tvTitle.getPaddingLeft() - tvTitle.getPaddingRight();
                        }
                    });
                }
                String title = aMapNaviPath.getLabels();
                int len = title.split(",").length;
                if (len >= 3)
                    title = "推荐";

                final String text = title;
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        reSizeTextView(tvTitle, text);
                        reSizeTextView(tvTime, getTime(aMapNaviPath.getAllTime()));
                        reSizeTextView(tvLength, getLength(aMapNaviPath.getAllLength()));
                    }
                });


                holder.getView(ll_itemview).setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        currentPosition = position;
                        //已经选中，再次选中直接返回
                        if (lastPosition == currentPosition) {
                            return;
                        } else {
                            //当前的下标值赋值给当前选择的线路下标值
                            routeIndex = position;
                            changeRoute();
                            selectedBackground(holder);
                            cleanSelector();
                        }
                        lastPosition = position;
                    }

                });
                if (position == 0) {
                    currentPosition = position;
                    if (lastPosition == currentPosition) {
                        return;
                    } else {
                        routeIndex = position;
                        changeRoute();
                        selectedBackground(holder);
                        cleanSelector();
                    }
                    lastPosition = position;
                } else {
                    initItemBackground(holder);
                }
            }
        };
    }

    /**
     * 计算路程
     *
     * @param allLength
     * @return
     */
    private String getLength(int allLength) {
        if (allLength > 1000) {
            int remainder = allLength % 1000;
            String m = remainder > 0 ? remainder + "米" : "";
            return allLength / 1000 + "公里" + m;
        } else {
            return allLength + "米";
        }
    }

    /**
     * 计算时间
     *
     * @param allTime
     * @return
     */
    private String getTime(int allTime) {
        if (allTime > 3600) {//1小时以上
            int minute = allTime % 3600;
            String min = minute / 60 != 0 ? minute / 60 + "分钟" : "";
            return allTime / 3600 + "小时" + min;
        } else {
            int minute = allTime % 3600;
            return minute / 60 + "分钟";
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
            EventBus.getDefault().post(amapLocation);
            if (amapLocation != null
                    && amapLocation.getErrorCode() == 0) {
                if (startList.size() == 0)
                    startList.add(new NaviLatLng(amapLocation.getLatitude(), amapLocation.getLongitude()));
                if (!calculateSuccess) {
                    mListener.onLocationChanged(amapLocation);// 显示系统小蓝点
                }

            } else {
                String errText = "定位失败," + amapLocation.getErrorCode() + ": " + amapLocation.getErrorInfo();
                Log.e("AmapErr", errText);
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

    /**
     * 方法必须重写
     */
    @Override
    protected void onPause() {
        super.onPause();
        mapview.onPause();
        clearRoute();
    }

    /**
     * 方法必须重写
     */
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mapview.onSaveInstanceState(outState);
    }

    /**
     * 方法必须重写
     */
    @Override
    protected void onDestroy() {
        super.onDestroy();
        mapview.onDestroy();
        if (null != mlocationClient) {
            mlocationClient.onDestroy();
        }
        EventBus.getDefault().unregister(this);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        mAMapNavi.destroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        XPermissionUtils.onRequestPermissionsResult(this, requestCode, permissions, grantResults);
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

    }

    /**
     * ************************************************** 在算路页面，以下接口全不需要处理，在以后的版本中SDK会进行优化***********************************************************************************************
     **/

    @Override
    public void onReCalculateRouteForYaw() {

    }

    @Override
    public void onInitNaviSuccess() {

    }

    @Override
    public void onGetNavigationText(int i, String s) {

    }

    @Override
    public void onGetNavigationText(String s) {

    }

    @Override
    public void onInitNaviFailure() {

    }


    @Override
    public void onStartNavi(int i) {

    }

    @Override
    public void onTrafficStatusUpdate() {

    }

    @Override
    public void onLocationChange(AMapNaviLocation aMapNaviLocation) {

    }


    @Override
    public void onEndEmulatorNavi() {

    }

    @Override
    public void onArriveDestination() {

    }

    @Override
    public void onReCalculateRouteForTrafficJam() {

    }

    @Override
    public void onArrivedWayPoint(int i) {

    }

    @Override
    public void onGpsOpenStatus(boolean b) {

    }

    @Override
    public void onNaviInfoUpdate(NaviInfo naviInfo) {

    }

    @Override
    public void updateCameraInfo(AMapNaviCameraInfo[] aMapNaviCameraInfos) {

    }



    @Override
    public void onServiceAreaUpdate(AMapServiceAreaInfo[] aMapServiceAreaInfos) {

    }

    @Override
    public void showCross(AMapNaviCross aMapNaviCross) {

    }

    @Override
    public void hideCross() {

    }



    @Override
    public void showLaneInfo(AMapLaneInfo[] aMapLaneInfos, byte[] bytes, byte[] bytes1) {

    }


    @Override
    public void hideLaneInfo() {

    }

    @Override
    public void onCalculateRouteSuccess(int[] ints) {
        if(ints.length == 1){
            onCalculateRouteSuccessOld();
        }else {
            onCalculateMultipleRoutesSuccessOld(ints);
        }
    }


    @Override
    public void notifyParallelRoad(int i) {

    }

    @Override
    public void OnUpdateTrafficFacility(AMapNaviTrafficFacilityInfo aMapNaviTrafficFacilityInfo) {

    }

    @Override
    public void OnUpdateTrafficFacility(AMapNaviTrafficFacilityInfo[] aMapNaviTrafficFacilityInfos) {

    }

    @Override
    public void updateAimlessModeStatistics(AimLessModeStat aimLessModeStat) {

    }

    @Override
    public void updateAimlessModeCongestionInfo(AimLessModeCongestionInfo aimLessModeCongestionInfo) {

    }

    @Override
    public void onPlayRing(int i) {

    }

    @Override
    public void updateIntervalCameraInfo(AMapNaviCameraInfo aMapNaviCameraInfo, AMapNaviCameraInfo aMapNaviCameraInfo1, int i) {

    }

    @Override
    public void hideModeCross() {

    }

    @Override
    public void showModeCross(AMapModelCross aMapModelCross) {

    }

    @Override
    public void showLaneInfo(AMapLaneInfo aMapLaneInfo) {

    }

    @Override
    public void onNaviRouteNotify(AMapNaviRouteNotifyData aMapNaviRouteNotifyData) {

    }

    @Override
    public void onCalculateRouteFailure(AMapCalcRouteResult aMapCalcRouteResult) {

    }

    @Override
    public void onCalculateRouteSuccess(AMapCalcRouteResult aMapCalcRouteResult) {
        // super.onCalculateRouteSuccess(ids);
        // mAMapNavi.startNavi(NaviType.EMULATOR);
    }

    //@Override
    public void onGpsSignalWeak(boolean var1) {

    }
}
