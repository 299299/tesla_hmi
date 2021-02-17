package com.github.urho3d;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AutoCompleteTextView;
import android.widget.ListView;

import com.amap.api.location.AMapLocation;
import com.amap.api.services.core.AMapException;
import com.amap.api.services.help.Inputtips;
import com.amap.api.services.help.InputtipsQuery;
import com.amap.api.services.help.Tip;
import com.zhy.adapter.recyclerview.CommonAdapter;
import com.zhy.adapter.recyclerview.base.ViewHolder;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.List;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.content.SharedPreferences;
import android.content.Context;

public class PoiSearchActivity extends AppCompatActivity implements TextWatcher, Inputtips.InputtipsListener, View.OnClickListener  {

    private CommonAdapter mAdapter;
    private RecyclerView mRecyclerView;
    private List<Tip> tips = new ArrayList<>();
    private String city = null;
    private AutoCompleteTextView mKeywordText = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_poi_search);
        ActionBar actionbar = getSupportActionBar();
        if(actionbar!=null){
            actionbar.hide();
        }
        EventBus.getDefault().register(this);
        mKeywordText = findViewById(R.id.input_edittext);
        mKeywordText.addTextChangedListener(this);
        findViewById(R.id.rl_tv_map_pick).setOnClickListener(this);
        mRecyclerView = findViewById(R.id.ll_rv_inputlist);
        mRecyclerView.setLayoutManager(new LinearLayoutManager(this));
        mAdapter = getAdapter();
        mRecyclerView.setAdapter(mAdapter);

        SharedPreferences sharedPref = getSharedPreferences("com.op.urho3D", Context.MODE_PRIVATE);
        TextView view1 = findViewById(R.id.history_text1);
        view1.setOnClickListener(this);
        TextView view2 = findViewById(R.id.history_text2);
        view2.setOnClickListener(this);
        TextView view3 = findViewById(R.id.history_text3);
        view3.setOnClickListener(this);

        view1.setText(sharedPref.getString("H_01", "..."));
        view2.setText(sharedPref.getString("H_02", "..."));
        view3.setText(sharedPref.getString("H_03", "..."));

        findViewById(R.id.poi_back).setOnClickListener(this);
    }

    /**
     * 跳转到地图地图选点Activity
     * @param v
     */
    @Override
    public void onClick(View v) {
        if (v.getId() == R.id.rl_tv_map_pick)
        {
            startActivity(new Intent(this,PiclocationActivity.class));
        }
        else if (v.getId() == R.id.history_text1)
        {
            TextView view = findViewById(R.id.history_text1);
            mKeywordText.setText(view.getText());
        }
        else if (v.getId() == R.id.history_text2)
        {
            TextView view = findViewById(R.id.history_text2);
            mKeywordText.setText(view.getText());
        }
        else if (v.getId() == R.id.history_text3)
        {
            TextView view = findViewById(R.id.history_text3);
            mKeywordText.setText(view.getText());
        }
        else if (v.getId() == R.id.poi_back)
        {
            finish();
        }
    }

    /**
     * 选择了地点，关闭当前Activity
     * @param tip
     */
    @Subscribe(threadMode = ThreadMode.MAIN,sticky = false,priority = 2)
    public void close(Tip tip)
    {
        finish();
    };

    /**
     * 设置当前城市
     * @param amapLocation
     */
    @Subscribe(threadMode = ThreadMode.BACKGROUND)
    public void setCity(AMapLocation amapLocation) {
        if(amapLocation!=null){
            this.city = amapLocation.getCity();
        }
    };
    /**
     * 文本变化监听事件
     * @param s
     * @param start
     * @param before
    * @param count
     */
    @Override
    public void onTextChanged(CharSequence s, int start, int before, int count) {
        String newText = s.toString().trim();
        InputtipsQuery inputquery = new InputtipsQuery(newText, city);
        inputquery.setCityLimit(true);
        Inputtips inputTips = new Inputtips(this, inputquery);
        inputTips.setInputtipsListener(this);
        inputTips.requestInputtipsAsyn();
    }

    /**
     * 输入自动提示结果
     * @param list
     * @param i
     */
    @Override
    public void onGetInputtips(List<Tip> list, int i) {
        if(i == AMapException.CODE_AMAP_SUCCESS){
            if(tips.size()>0)
                tips.clear();
            for (Tip tip:list) {
                if(tip.getPoint()!=null){
                    tips.add(tip);
                }
            }
//            tips.addAll(list);
            mAdapter.notifyDataSetChanged();
        }
    }
    private CommonAdapter getAdapter() {
        return new CommonAdapter<Tip>(this,R.layout.item_layout,tips) {

            @Override
            protected void convert(ViewHolder holder, final Tip tip, int position) {

                holder.setText(R.id.poi_field_id,tip.getName());
                holder.setText(R.id.poi_value_id,tip.getDistrict());
                holder.getView(R.id.item_layout).setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        EventBus.getDefault().post(tip);
                        finish();
                    }
                });
            }
        };
    }

    @Override
    public void beforeTextChanged(CharSequence s, int start, int count, int after) {

    }
    @Override
    public void afterTextChanged(Editable s) {

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);
    }
}
