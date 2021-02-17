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

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.content.res.AssetManager;
import android.util.Log;
import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

import java.io.IOException;
import java.util.*;
import android.os.*;

import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.LinearLayout;

import android.content.IntentFilter;

import android.content.Context;
import android.view.View;
import android.widget.Toast;
import android.view.KeyEvent;

import java.util.Date;
import android.view.*;

import android.app.Activity;
import android.content.SharedPreferences;

public class EntryUI extends Activity implements View.OnClickListener{

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_entry);
        findViewById(R.id.map_button).setOnClickListener(this);
        findViewById(R.id.nav_button).setOnClickListener(this);
        findViewById(R.id.sim_button).setOnClickListener(this);
        findViewById(R.id.test_button).setOnClickListener(this);

        SharedPreferences pref = getPreferences(Context.MODE_PRIVATE);
        EditText text = findViewById(R.id.ip_text);
        String ip_address = pref.getString("IP", "192.168.");
        text.setText(ip_address);
    }

    @Override
    public void onClick(View v) {

        EditText text = findViewById(R.id.ip_text);
        String ip_text = text.getText().toString();
        Urho3D.IP_ADDRESS = ip_text;
        Urho3D_Map.IP_ADDRESS = ip_text;
        Urho3D_Test.IP_ADDRESS = ip_text;
        Urho3D_MapNew.IP_ADDRESS = ip_text;

        SharedPreferences pref = getPreferences(Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        editor.putString("IP", ip_text);
        editor.commit();

        switch (v.getId()) {
            case R.id.map_button:
                {
                    Urho3D.run_type = Urho3D.RUN_MAP;
                    Intent intent = new Intent(this, Urho3D_MapNew.class);
                    startActivity(intent);
                    break;
                }
//            case R.id.map_button:
//                {
//                    Urho3D.run_type = Urho3D.RUN_MAP;
//                    // Intent intent = new Intent(this, Urho3D.class);
//                    Intent intent = new Intent(this, Urho3D_Map.class);
//                    startActivity(intent);
//                    break;
//                }

            case R.id.nav_button:
                {
                    Urho3D.run_type = Urho3D.RUN_NAVI;
                    Intent intent = new Intent(this, NavigationActivity.class);
                    startActivity(intent);
                    break;
                }
            case R.id.sim_button:
                {
                    Urho3D.run_type = Urho3D.RUN_SIM;
                    Intent intent = new Intent(this, NavigationActivity.class);
                    startActivity(intent);
                    break;
                }
            case R.id.test_button:
                {
                    Intent intent = new Intent(this, Urho3D_Test.class);
                    startActivity(intent);
                    break;
                }
        }
    }
}
