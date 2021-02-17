package com.github.urho3d;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemClock;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.view.animation.BounceInterpolator;
import android.view.animation.Interpolator;
import android.widget.TextView;

import com.amap.api.location.AMapLocation;
import com.amap.api.location.AMapLocationClient;
import com.amap.api.location.AMapLocationClientOption;
import com.amap.api.location.AMapLocationListener;
import com.amap.api.maps.AMap;
import com.amap.api.maps.CameraUpdateFactory;
import com.amap.api.maps.LocationSource;
import com.amap.api.maps.MapView;
import com.amap.api.maps.Projection;
import com.amap.api.maps.model.BitmapDescriptorFactory;
import com.amap.api.maps.model.CameraPosition;
import com.amap.api.maps.model.LatLng;
import com.amap.api.maps.model.Marker;
import com.amap.api.maps.model.MarkerOptions;
import com.amap.api.maps.model.MyLocationStyle;
import com.amap.api.services.core.AMapException;
import com.amap.api.services.core.LatLonPoint;
import com.amap.api.services.core.PoiItem;
import com.amap.api.services.geocoder.GeocodeResult;
import com.amap.api.services.geocoder.GeocodeSearch;
import com.amap.api.services.geocoder.RegeocodeQuery;
import com.amap.api.services.geocoder.RegeocodeResult;
import com.amap.api.services.help.Tip;
import com.amap.api.services.poisearch.PoiResult;
import com.amap.api.services.poisearch.PoiSearch;
import com.zhy.adapter.recyclerview.CommonAdapter;
import com.zhy.adapter.recyclerview.base.ViewHolder;

import org.greenrobot.eventbus.EventBus;

import java.util.ArrayList;
import java.util.List;

public class PiclocationActivity extends Activity implements LocationSource, AMapLocationListener, PoiSearch.OnPoiSearchListener, AMap.OnCameraChangeListener, GeocodeSearch.OnGeocodeSearchListener , View.OnClickListener {
    private static final String TAG = "PiclocationActivity";
    private MapView mMapView = null;
    private AMap amap;
    private Marker mEndMarker;
    private OnLocationChangedListener mListener;
    private AMapLocationClient mlocationClient;
    private AMapLocationClientOption mLocationOption;
    /**
     * 周边搜索条件
     */
    private PoiSearch.Query  query;
    /**
     * 周边搜索的业务执行
     */
    private PoiSearch poiSearch;
    /**
     * 逆地理编码业务类
     */
    private GeocodeSearch geocoderSearch;
    /**
     * 第一次定位的标志位
     */
    private boolean isFirstTime = true;
    private Context mContext;
    private List<ListViewHoldier> data = new ArrayList<>();
    //第一个位置数据，设为成员变量是因为有多个地方需要使用
    private ListViewHoldier lvHolder;
    private RecyclerView mRecyclerView;
    private CommonAdapter mAdapter;
    private View progressDialogView;
    private TextView tvHint;
    private View progressbar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_piclocation);
        ActionBar actionBar = getActionBar();
        if(actionBar!=null)actionBar.hide();
        initView();
        mMapView.onCreate(savedInstanceState);// 此方法必须重写
        initMap();

    }

    private void initView() {
        mMapView = (MapView) findViewById(R.id.map);
        mRecyclerView = (RecyclerView) findViewById(R.id.ll_rl_locations);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        mAdapter = getAdapter();
        mRecyclerView.setAdapter(mAdapter);
        progressDialogView = findViewById(R.id.ll_ll_holderview);
        tvHint = (TextView) findViewById(R.id.ll_tv_hint);
        progressbar = findViewById(R.id.ll_progressbar);
        findViewById(R.id.pick_loc_back).setOnClickListener(this);
    }


    /**
     * 地图实例化
     */
    private void initMap() {
        if (amap == null) {
            amap = mMapView.getMap();
            amap.setLocationSource(this);//设置了定位的监听,这里要实现LocationSource接口
            amap.getUiSettings().setMyLocationButtonEnabled(true); // 是否显示定位按钮
            amap.setMyLocationEnabled(true);//显示定位层并且可以触发定位,默认是flase
            amap.moveCamera(CameraUpdateFactory.zoomTo(15));//设置地图缩放级别
            MyLocationStyle myLocationStyle = new MyLocationStyle();//初始化定位蓝点样式类
            myLocationStyle.myLocationType(MyLocationStyle.LOCATION_TYPE_LOCATE);//连续定位、且将视角移动到地图中心点，定位点依照设备方向旋转，并且会跟随设备移动。（1秒1次定位）如果不设置myLocationType，默认也会执行此种模式。
            myLocationStyle.strokeColor(Color.TRANSPARENT);//设置定位蓝点精度圆圈的边框颜色
            myLocationStyle.radiusFillColor(Color.TRANSPARENT);//设置定位蓝点精度圆圈的填充颜色
            amap.setMyLocationStyle(myLocationStyle);//设置定位蓝点的Style
            lvHolder = new ListViewHoldier();
            //天添加屏幕移动的监听
            amap.setOnCameraChangeListener(this);
            // 初始化Marker添加到地图
            mEndMarker = amap.addMarker(new MarkerOptions().icon(BitmapDescriptorFactory.fromBitmap(BitmapFactory.decodeResource(getResources(), R.mipmap.end))));
            //初始化 geocoderSearch
            geocoderSearch = new GeocodeSearch(this);
            //注册 逆地理编码异步处理回调接口
            geocoderSearch.setOnGeocodeSearchListener(this);

        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        //在activity执行onDestroy时执行mMapView.onDestroy()，销毁地图
        mMapView.onDestroy();
    }
    @Override
    protected void onResume() {
        super.onResume();
        //在activity执行onResume时执行mMapView.onResume ()，重新绘制加载地图
        mMapView.onResume();
    }
    @Override
    protected void onPause() {
        super.onPause();
        //在activity执行onPause时执行mMapView.onPause ()，暂停地图的绘制
        mMapView.onPause();
    }
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        //在activity执行onSaveInstanceState时执行mMapView.onSaveInstanceState (outState)，保存地图当前的状态
        mMapView.onSaveInstanceState(outState);
    }

    /**
     * 激活定位
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
            //设置为定位一次
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
     * 实现定位
     * @param amapLocation
     */
    @Override
    public void onLocationChanged(AMapLocation amapLocation) {
        if (mListener != null && amapLocation != null) {
            if (amapLocation != null
                    &&amapLocation.getErrorCode() == 0) {

                if(isFirstTime){//只要第一次的数据，当然，也可以在这里关闭定位
//                    mlocationClient.stopLocation();//停止定位
                    mListener.onLocationChanged(amapLocation);// 显示系统小蓝点
                    lvHolder.title = "[位置]";
                    lvHolder.address = amapLocation.getProvider()+amapLocation.getCity()+amapLocation.getStreet()+amapLocation.getStreetNum();
                    lvHolder.lp = new LatLonPoint(amapLocation.getLatitude(),amapLocation.getLongitude());
                    mEndMarker.setPosition(new LatLng(amapLocation.getLatitude(),amapLocation.getLongitude()));
                    data.add(0,lvHolder);
                    doSearchQuery();

                }

            } else {
                String errText = "定位失败," + amapLocation.getErrorCode()+ ": " + amapLocation.getErrorInfo();
                Log.e("AmapErr",errText);
            }
        }
    }

    /**
     * 搜查周边数据
     */
    private void doSearchQuery() {
        //搜索类型
        String type = "汽车服务|汽车销售|" +
                "汽车维修|摩托车服务|餐饮服务|购物服务|生活服务|体育休闲服务|医疗保健服务|" +
                "住宿服务|风景名胜|商务住宅|政府机构及社会团体|科教文化服务|交通设施服务|" +
                "金融保险服务|公司企业|道路附属设施|地名地址信息|公共设施";
        query = new PoiSearch.Query("", type, "");// 第一个参数表示搜索字符串，第二个参数表示poi搜索类型，第三个参数表示poi搜索区域（空字符串代表全国）
        query.setPageSize(20);// 设置每页最多返回多少条poiitem
        query.setPageNum(0);// 设置查第一页

        poiSearch = new PoiSearch(this, query);
        //搜索回调
        poiSearch.setOnPoiSearchListener(this);
        //搜索位置及范围
        poiSearch.setBound(new PoiSearch.SearchBound(lvHolder.lp, 1000));
        //同步搜索
//        poiSearch.searchPOI();//不能在主线程实现耗时操作
        //异步搜索
        poiSearch.searchPOIAsyn();
    }

    /**
     * 返回POI搜索异步处理的结果。
     * @param result
     * @param rcode
     */
    @Override
    public void onPoiSearched(PoiResult result, int rcode) {
        if (rcode == AMapException.CODE_AMAP_SUCCESS) {
            if (result != null && result.getQuery() != null) {// 搜索poi的结果
                if (result.getQuery().equals(query)) {// 是否是同一条
                    // 取得搜索到的poiitems有多少页
                    List<PoiItem> poiItems = result.getPois();// 取得第一页的poiitem数据，页数从数字0开始
                    if (poiItems != null && poiItems.size() > 0) {
                        for (int i = 0;i<poiItems.size();i++){
                            PoiItem poiitem = poiItems.get(i);
                            ListViewHoldier holder = new ListViewHoldier();
                            holder.address = poiitem.getSnippet();
                            holder.title = poiitem.getTitle();
                            holder.lp = poiitem.getLatLonPoint();
                            if(data.size()>i+1){
                                data.remove(i+1);
                            }
                            data.add(i+1,holder);

                        }
                    mAdapter.notifyDataSetChanged();
                    mRecyclerView.setVisibility(View.VISIBLE);
                    progressDialogView.setVisibility(View.GONE);
                    } else {
                        progressbar.setVisibility(View.GONE);
                        tvHint.setText(R.string.no_location);
                    }
                }
            } else {
                progressbar.setVisibility(View.GONE);
                tvHint.setText(R.string.no_location);
            }
        } else {
            progressbar.setVisibility(View.GONE);
            tvHint.setText(R.string.no_location);
        }
    }

    /**
     * poi id搜索的结果回调
     * @param poiItem
     * @param i
     */
    @Override
    public void onPoiItemSearched(PoiItem poiItem, int i) {

    }

    /**
     * 在地图状态改变过程中回调此方法。
     * @param cameraPosition
     */
    @Override
    public void onCameraChange(CameraPosition cameraPosition) {
        mEndMarker.setPosition(cameraPosition.target);
    }


    /**
     * 在地图状态改变完成时回调此方法。
     * @param cameraPosition
     */
    @Override
    public void onCameraChangeFinish(CameraPosition cameraPosition) {
        //当地图定位成功的时候该方法也会回调，为了避免不必要的搜索，因此此处增加一个判断
        if(isFirstTime){
            isFirstTime = false;
            return;
        }
        //隐藏数据
        mRecyclerView.setVisibility(View.GONE);
        //展示dialogView
        progressDialogView.setVisibility(View.VISIBLE);
        findViewById(R.id.ll_progressbar).setVisibility(View.VISIBLE);
        tvHint.setText(R.string.loading);
        //marker 动画
        jumpPoint(mEndMarker);
        lvHolder.lp = new LatLonPoint(cameraPosition.target.latitude,cameraPosition.target.longitude);
        RegeocodeQuery query = new RegeocodeQuery(lvHolder.lp, 200,
                GeocodeSearch.AMAP);// 第一个参数表示一个Latlng，第二参数表示范围多少米，第三个参数表示是火系坐标系还是GPS原生坐标系
        geocoderSearch.getFromLocationAsyn(query);// 设置异步逆地理编码请求
        doSearchQuery();

    }

    /**
     * marker点击时跳动一下
     */
    public void jumpPoint(final Marker marker) {
        final Handler handler = new Handler();
        final long start = SystemClock.uptimeMillis();
        //获取地图投影坐标转换器
        Projection proj = amap.getProjection();
        final LatLng markerLatlng = marker.getPosition();
        Point markerPoint = proj.toScreenLocation(markerLatlng);
        markerPoint.offset(0, -50);
        final LatLng startLatLng = proj.fromScreenLocation(markerPoint);
        final long duration = 500;

        final Interpolator interpolator = new BounceInterpolator();
        handler.post(new Runnable() {
            @Override
            public void run() {
                long elapsed = SystemClock.uptimeMillis() - start;
                float t = interpolator.getInterpolation((float) elapsed
                        / duration);
                double lng = t * markerLatlng.longitude + (1 - t)
                        * startLatLng.longitude;
                double lat = t * markerLatlng.latitude + (1 - t)
                        * startLatLng.latitude;
                marker.setPosition(new LatLng(lat, lng));
                if (t < 1.0) {
                    handler.postDelayed(this, 16);
                }
            }
        });
    }

    /**
     * 逆地理编码回调方法
     * 经纬度转位置
     * @param result
     * @param rCode
     */
    @Override
    public void onRegeocodeSearched(RegeocodeResult result, int rCode) {
            if (rCode == AMapException.CODE_AMAP_SUCCESS) {
                if (result != null && result.getRegeocodeAddress() != null
                        && result.getRegeocodeAddress().getFormatAddress() != null) {
                    lvHolder.address = result.getRegeocodeAddress().getFormatAddress();
                    data.remove(0);
                    data.add(0,lvHolder);
                } else {
        //                ToastUtil.show(ReGeocoderActivity.this, R.string.no_result);
                }
        } else {
//            ToastUtil.showerror(this, rCode);
        }
    }

    /**
     * 地理编码回调方法
     * 位置转经纬度
     * @param geocodeResult
     * @param i
     */
    @Override
    public void onGeocodeSearched(GeocodeResult geocodeResult, int i) {

    }

    private CommonAdapter getAdapter() {
        return new CommonAdapter<ListViewHoldier>(this,R.layout.item_listview_location,data) {

            @Override
            protected void convert(ViewHolder holder, final ListViewHoldier listViewHoldier, int position) {
                holder.setText(R.id.rl_tv_name,listViewHoldier.title);
                holder.setText(R.id.rl_tv_location,listViewHoldier.address);
                holder.getView(R.id.rl_tv_subit).setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
    //                    Intent intent = new Intent(mContext,NavigationActivity.class);
    //                    intent.putExtra("value",listViewHoldier.lp);
    //                    intent.putExtra("address",listViewHoldier.address);
    //                    setResult(RESULT_OK,intent);
                        Tip tip = new Tip();
                        tip.setDistrict(listViewHoldier.address);
                        tip.setPostion(listViewHoldier.lp);
                        EventBus.getDefault().postSticky(tip);

                        finish();
                    }
                });
            }
        };
    }

    /**
     * 周边数据实体封装
     */
    private class ListViewHoldier{
        String title;
        String address;
        LatLonPoint lp;
    }

    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.pick_loc_back)
        {
            finish();
        }
    }
}

