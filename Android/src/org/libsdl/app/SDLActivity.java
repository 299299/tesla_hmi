// Modified by Lasse Oorni and Yao Wei Tjong for Urho3D

package org.libsdl.app;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.lang.reflect.Method;
import java.util.Objects;

import android.app.*;
import android.content.*;
import android.text.InputType;
import android.view.*;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.RelativeLayout;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.os.*;
import android.util.Log;
import android.util.SparseArray;
import android.graphics.*;
import android.graphics.drawable.Drawable;
import android.hardware.*;
import android.content.pm.ActivityInfo;


class CallbackMsg
{
    double data1;
    double data2;
    double data3;
    double data4;
    double data5;
}

/**
    SDL Activity
*/
public class SDLActivity extends Activity {
    private static final String TAG = "SDL";

    public static boolean mIsResumedCalled, mIsSurfaceReady, mHasFocus;

    // Handle the state of the native layer
    public enum NativeState {
           INIT, RESUMED, PAUSED
    }

    public static NativeState mNextNativeState;
    public static NativeState mCurrentNativeState;

    public static boolean mExitCalledFromJava;

    // If we want to separate mouse and touch events.
    //  This is only toggled in native code when a hint is set!
    public static boolean mSeparateMouseAndTouch;

    // Main components
    protected static SDLActivity mSingleton;
    protected static SDLSurface mSurface;
    protected static View mTextEdit;
    protected static boolean mScreenKeyboardShown;
    protected static ViewGroup mLayout;
    protected static SDLClipboardHandler mClipboardHandler;
    protected static boolean mInited = false;


    // This is what SDL runs in. It invokes SDL_main(), eventually
    protected static Thread mSDLThread;

    // Urho3D: Default implementation returns the last shared lib being loaded
    private static String mMainSharedLib;

    /**
     * This method returns the name of the shared object with the application entry point
     * It can be overridden by derived classes.
     * Urho3D: Should not be called before the library is loaded.
     */
    protected String getMainSharedObject() {
        return mMainSharedLib;
    }

    /**
     * This method returns the name of the application entry point
     * It can be overridden by derived classes.
     */
    protected String getMainFunction() {
        return "SDL_main";
    }

    // Urho3D: flag to load the .so and a new method load them
    private static boolean mIsSharedLibraryLoaded = false;

    // Urho3D: Avoid hardcoding of the library list
    protected boolean onLoadLibrary(ArrayList<String> libraryNames) {
        for (final String name : libraryNames) {
            System.loadLibrary(name);
        }
        return true;
    }

    /**
     * This method is called by SDL before starting the native application thread.
     * It can be overridden to provide the arguments after the application name.
     * The default implementation returns an empty array. It never returns null.
     * @return arguments for the native application.
     */
    protected String[] getArguments() {
        // Urho3D: the default implementation returns the "app_process" as the first argument instead of empty array
        return new String[]{"app_process"};
    }

    public static void initialize() {
        // The static nature of the singleton and Android quirkyness force us to initialize everything here
        // Otherwise, when exiting the app and returning to it, these variables *keep* their pre exit values
        mSingleton = null;
        mSurface = null;
        mTextEdit = null;
        mLayout = null;
        mClipboardHandler = null;
        mSDLThread = null;
        mExitCalledFromJava = false;
        mIsResumedCalled = false;
        mIsSurfaceReady = false;
        mHasFocus = true;
        mNextNativeState = NativeState.INIT;
        mCurrentNativeState = NativeState.INIT;
        mInited = false;
    }

    // Setup
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.v(TAG, "Device: " + android.os.Build.DEVICE);
        Log.v(TAG, "Model: " + android.os.Build.MODEL);
        Log.v(TAG, "onCreate()");
        super.onCreate(savedInstanceState);

        // Urho3D: auto load all the shared libraries available in the library path
        if (!mIsSharedLibraryLoaded) {
            String libraryPath = getApplicationInfo().nativeLibraryDir;
            String libraryPath1 = getApplicationInfo().dataDir;

            File[] all_files = new File(libraryPath).listFiles();
            File so_file = null;

            File[] data_files = new File(libraryPath1).listFiles();

            for (File file : all_files)
            {
                if (file.getName().matches("libGame.so"))
                {
                    so_file = file;
                }
            }


            String errorMsgBrokenLib = "";
            if (so_file == null) {
                errorMsgBrokenLib = "no libraries found in path \"" + libraryPath + "\"";
            } else {
                ArrayList<String> libraryNames = new ArrayList<>(1);
                libraryNames.add("Game");

                // Load shared libraries
                try {
                    if (onLoadLibrary(libraryNames)) {
                        mIsSharedLibraryLoaded = true;
                        mMainSharedLib = "lib" + libraryNames.get(libraryNames.size() - 1) + ".so";
                    }
                } catch(Exception e) {
                    errorMsgBrokenLib = e.getMessage();
                }
            }

            if (!errorMsgBrokenLib.isEmpty())
            {
                AlertDialog.Builder dlgAlert  = new AlertDialog.Builder(this);
                dlgAlert.setMessage("An error occurred while trying to start the application. Please try again and/or reinstall."
                      + System.getProperty("line.separator")
                      + System.getProperty("line.separator")
                      + "Error: " + errorMsgBrokenLib);
                dlgAlert.setTitle("SDL Error");
                dlgAlert.setPositiveButton("Exit",
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog,int id) {
                            // if this button is clicked, close current activity
                            SDLActivity.mSingleton.finish();
                        }
                    });
               dlgAlert.setCancelable(false);
               dlgAlert.create().show();

               return;
            }
        }

        // Urho3D: onLoadLibrary() may return false
        if (SDLActivity.mIsSharedLibraryLoaded && !SDLActivity.mInited) {

            Log.i(TAG, "SDL.initialize()");

            // Set up JNI
            SDL.setupJNI();

            // Initialize state
            SDL.initialize();

            // So we can call stuff from static callbacks
            mSingleton = this;
            SDL.setContext(this);

            if (Build.VERSION.SDK_INT >= 11) {
                mClipboardHandler = new SDLClipboardHandler_API11();
            } else {
                /* Before API 11, no clipboard notification (eg no SDL_CLIPBOARDUPDATE) */
                mClipboardHandler = new SDLClipboardHandler_Old();
            }

            SDLActivity.mInited = true;
        }

        onInitLayout();

        // Get filename from "Open with" of another application
        Intent intent = getIntent();
        if (intent != null && intent.getData() != null) {
            String filename = intent.getData().getPath();
            if (filename != null) {
                Log.v(TAG, "Got filename: " + filename);
                SDLActivity.onNativeDropFile(filename);
            }
        }
    }

    protected void onInitLayout()
    {
        // Set up the surface
        mSurface = new SDLSurface(getApplication());
        mLayout = new RelativeLayout(this);
        mLayout.addView(mSurface);
        setContentView(mLayout);
    }

    // Events
    @Override
    protected void onPause() {
        Log.v(TAG, "onPause()");
        super.onPause();
        mNextNativeState = NativeState.PAUSED;
        mIsResumedCalled = false;

        if (!SDLActivity.mIsSharedLibraryLoaded) {
           return;
        }

        SDLActivity.handleNativeState();
    }

    @Override
    protected void onResume() {
        Log.v(TAG, "onResume()");
        super.onResume();
        mNextNativeState = NativeState.RESUMED;
        mIsResumedCalled = true;

        if (!SDLActivity.mIsSharedLibraryLoaded) {
           return;
        }

        SDLActivity.handleNativeState();
    }


    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        Log.v(TAG, "onWindowFocusChanged(): " + hasFocus);

        if (!SDLActivity.mIsSharedLibraryLoaded) {
           return;
        }

        SDLActivity.mHasFocus = hasFocus;
        if (hasFocus) {
           mNextNativeState = NativeState.RESUMED;
        } else {
           mNextNativeState = NativeState.PAUSED;
        }

        SDLActivity.handleNativeState();
    }

    @Override
    public void onLowMemory() {
        Log.v(TAG, "onLowMemory()");
        super.onLowMemory();

        if (!SDLActivity.mIsSharedLibraryLoaded) {
           return;
        }

        SDLActivity.nativeLowMemory();
    }

    @Override
    protected void onDestroy() {
        Log.v(TAG, "onDestroy()");

        if (!SDLActivity.mIsSharedLibraryLoaded) {
           super.onDestroy();
           // Reset everything in case the user re opens the app
           if (mSingleton != null)
               SDLActivity.initialize();
           return;
        }

        mNextNativeState = NativeState.PAUSED;
        SDLActivity.handleNativeState();

        // Send a quit message to the application
        SDLActivity.mExitCalledFromJava = true;
        SDLActivity.nativeQuit();

        // Now wait for the SDL thread to quit
        if (SDLActivity.mSDLThread != null) {
            try {
                SDLActivity.mSDLThread.join();
            } catch(Exception e) {
                Log.v(TAG, "Problem stopping thread: " + e);
            }
            SDLActivity.mSDLThread = null;

            //Log.v(TAG, "Finished waiting for SDL thread");
        }

        super.onDestroy();

        // Reset everything in case the user re opens the app
        if (mSingleton != null)
            SDLActivity.initialize();
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {

        if (!SDLActivity.mIsSharedLibraryLoaded) {
           return false;
        }

        int keyCode = event.getKeyCode();
        // Ignore certain special keys so they're handled by Android
        // Urho3D: also ignore the Home key
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN ||
            keyCode == KeyEvent.KEYCODE_VOLUME_UP ||
            keyCode == KeyEvent.KEYCODE_HOME ||
            keyCode == KeyEvent.KEYCODE_CAMERA ||
            keyCode == KeyEvent.KEYCODE_ZOOM_IN || /* API 11 */
            keyCode == KeyEvent.KEYCODE_ZOOM_OUT /* API 11 */
            ) {
            return false;
        }
        return super.dispatchKeyEvent(event);
    }

    /* Transition to next state */
    public static void handleNativeState() {

        if (mNextNativeState == mCurrentNativeState) {
            // Already in same state, discard.
            return;
        }

        // Try a transition to init state
        if (mNextNativeState == NativeState.INIT) {

            mCurrentNativeState = mNextNativeState;
            return;
        }

        // Try a transition to paused state
        if (mNextNativeState == NativeState.PAUSED) {
            nativePause();
            mSurface.handlePause();
            mCurrentNativeState = mNextNativeState;
            return;
        }

        // Try a transition to resumed state
        if (mNextNativeState == NativeState.RESUMED) {
            if (mIsSurfaceReady && mHasFocus && mIsResumedCalled) {
                if (mSDLThread == null) {
                    // This is the entry point to the C app.
                    // Start up the C app thread and enable sensor input for the first time
                    // FIXME: Why aren't we enabling sensor input at start?

                    final Thread sdlThread = new Thread(new SDLMain(), "SDLThread");
                    mSurface.enableSensor(Sensor.TYPE_ACCELEROMETER, true);
                    sdlThread.start();

                    // Set up a listener thread to catch when the native thread ends
                    mSDLThread = new Thread(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                sdlThread.join();
                            } catch (Exception e) {
                                // Ignore any exception
                            } finally {
                                // Native thread has finished
                                if (!mExitCalledFromJava) {
                                    handleNativeExit();
                                }
                            }
                        }
                    }, "SDLThreadListener");

                    mSDLThread.start();
                }

                nativeResume();
                mSurface.handleResume();
                mCurrentNativeState = mNextNativeState;
            }
        }
    }

    /* The native thread has finished */
    public static void handleNativeExit() {
        SDLActivity.mSDLThread = null;
        mSingleton.finish();
    }


    // Messages from the SDLMain thread
    static final int COMMAND_CHANGE_TITLE = 1;
    static final int COMMAND_UNUSED = 2;
    static final int COMMAND_TEXTEDIT_HIDE = 3;
    static final int COMMAND_SET_KEEP_SCREEN_ON = 5;

    protected static final int COMMAND_USER = 0x8000;

    /**
     * This method is called by SDL if SDL did not handle a message itself.
     * This happens if a received message contains an unsupported command.
     * Method can be overwritten to handle Messages in a different class.
     * @param command the command of the message.
     * @param param the parameter of the message. May be null.
     * @return if the message was handled in overridden method.
     */
    protected boolean onUnhandledMessage(int command, Object param) {
        return false;
    }

    protected void onMessage2(int cmd, double data1, double data2, double data3, double data4, double data5) {}

    /**
     * A Handler class for Messages from native SDL applications.
     * It uses current Activities as target (e.g. for the title).
     * static to prevent implicit references to enclosing object.
     */
    protected static class SDLCommandHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            Context context = SDL.getContext();
            if (context == null) {
                Log.e(TAG, "error handling message, getContext() returned null");
                return;
            }

            if (msg.obj instanceof CallbackMsg)
            {
                CallbackMsg msg2 = (CallbackMsg)msg.obj;
                ((SDLActivity) context).onMessage2(msg.arg1, msg2.data1, msg2.data2, msg2.data3, msg2.data4, msg2.data5);
                return;
            }


            switch (msg.arg1) {
            case COMMAND_CHANGE_TITLE:
                if (context instanceof Activity) {
                    ((Activity) context).setTitle((String)msg.obj);
                } else {
                    Log.e(TAG, "error handling message, getContext() returned no Activity");
                }
                break;
            case COMMAND_TEXTEDIT_HIDE:
                if (mTextEdit != null) {
                    // Note: On some devices setting view to GONE creates a flicker in landscape.
                    // Setting the View's sizes to 0 is similar to GONE but without the flicker.
                    // The sizes will be set to useful values when the keyboard is shown again.
                    mTextEdit.setLayoutParams(new RelativeLayout.LayoutParams(0, 0));

                    InputMethodManager imm = (InputMethodManager) context.getSystemService(Context.INPUT_METHOD_SERVICE);
                    imm.hideSoftInputFromWindow(mTextEdit.getWindowToken(), 0);

                    mScreenKeyboardShown = false;
                }
                break;
            case COMMAND_SET_KEEP_SCREEN_ON:
            {
                if (context instanceof Activity) {
                    Window window = ((Activity) context).getWindow();
                    if (window != null) {
                        if ((msg.obj instanceof Integer) && (((Integer) msg.obj).intValue() != 0)) {
                            window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                        } else {
                            window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                        }
                    }
                }
                break;
            }
            default:
                if ((context instanceof SDLActivity) && !((SDLActivity) context).onUnhandledMessage(msg.arg1, msg.obj)) {
                    Log.e(TAG, "error handling message, command is " + msg.arg1);
                }
            }
        }
    }

    // Handler for the messages
    Handler commandHandler = new SDLCommandHandler();

    // Send a message from the SDLMain thread
    boolean sendCommand(int command, Object data) {
        Message msg = commandHandler.obtainMessage();
        msg.arg1 = command;
        msg.obj = data;
        return commandHandler.sendMessage(msg);
    }

    // C functions we call
    public static native int nativeSetupJNI();
    public static native int nativeRunMain(String library, String function, Object arguments);
    public static native void nativeLowMemory();
    public static native void nativeQuit();
    public static native void nativePause();
    public static native void nativeResume();
    public static native void onNativeDropFile(String filename);
    public static native void onNativeResize(int x, int y, int format, float rate);
    public static native void onNativeKeyDown(int keycode);
    public static native void onNativeKeyUp(int keycode);
    public static native void onNativeKeyboardFocusLost();
    public static native void onNativeMouse(int button, int action, float x, float y);
    public static native void onNativeTouch(int touchDevId, int pointerFingerId,
                                            int action, float x,
                                            float y, float p);
    public static native void onNativeAccel(float x, float y, float z);
    public static native void onNativeClipboardChanged();
    public static native void onNativeSurfaceChanged();
    public static native void onNativeSurfaceDestroyed();
    public static native String nativeGetHint(String name);
    public static native void onNativeMessage(int msg_code, double data1, double data2, double data3, String data4);

    /**
     * This method is called by SDL using JNI.
     */
    public static boolean setActivityTitle(String title) {
        // Called from SDLMain() thread and can't directly affect the view
        return mSingleton.sendCommand(COMMAND_CHANGE_TITLE, title);
    }

    /**
     * This method is called by SDL using JNI.
     * This is a static method for JNI convenience, it calls a non-static method
     * so that is can be overridden
     */
    public static void setOrientation(int w, int h, boolean resizable, String hint)
    {
        if (mSingleton != null) {
            mSingleton.setOrientationBis(w, h, resizable, hint);
        }
    }

    /**
     * This can be overridden
     */
    public void setOrientationBis(int w, int h, boolean resizable, String hint)
    {
      int orientation = -1;

      if (!Objects.equals(hint, "")) {
         if (hint.contains("LandscapeRight") && hint.contains("LandscapeLeft")) {
            orientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE;
         } else if (hint.contains("LandscapeRight")) {
            orientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
         } else if (hint.contains("LandscapeLeft")) {
            orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
         } else if (hint.contains("Portrait") && hint.contains("PortraitUpsideDown")) {
            orientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT;
         } else if (hint.contains("Portrait")) {
            orientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
         } else if (hint.contains("PortraitUpsideDown")) {
            orientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
         }
      }

      /* no valid hint */
      if (orientation == -1) {
         if (resizable) {
            /* no fixed orientation */
         } else {
            if (w > h) {
               orientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE;
            } else {
               orientation = ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT;
            }
         }
      }

      Log.v("SDL", "setOrientation() orientation=" + orientation + " width=" + w +" height="+ h +" resizable=" + resizable + " hint=" + hint);
      if (orientation != -1) {
         mSingleton.setRequestedOrientation(orientation);
      }
    }


    /**
     * This method is called by SDL using JNI.
     */
    public static boolean isScreenKeyboardShown()
    {
        if (mTextEdit == null) {
            return false;
        }

        if (!mScreenKeyboardShown) {
            return false;
        }

        InputMethodManager imm = (InputMethodManager) SDL.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        return imm.isAcceptingText();

    }

    /**
     * This method is called by SDL using JNI.
     */
    public static boolean sendMessage(int command, int param) {
        if (mSingleton == null) {
            return false;
        }
        return mSingleton.sendCommand(command, Integer.valueOf(param));
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void sendMessage2(int command, double data1, double data2, double data3, double data4, double data5) {
        if (mSingleton != null) {
            CallbackMsg msg = new CallbackMsg();
            msg.data1 = data1;
            msg.data2 = data2;
            msg.data3 = data3;
            msg.data4 = data4;
            msg.data5 = data5;
            mSingleton.sendCommand(command, msg);
        }
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static Context getContext() {
        return SDL.getContext();
    }

    static class ShowTextInputTask implements Runnable {
        /*
         * This is used to regulate the pan&scan method to have some offset from
         * the bottom edge of the input region and the top edge of an input
         * method (soft keyboard)
         */
        static final int HEIGHT_PADDING = 15;

        public int x, y, w, h;

        public ShowTextInputTask(int x, int y, int w, int h) {
            this.x = x;
            this.y = y;
            this.w = w;
            this.h = h;
        }

        @Override
        public void run() {
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(w, h + HEIGHT_PADDING);
            params.leftMargin = x;
            params.topMargin = y;

            if (mTextEdit == null) {
                mTextEdit = new DummyEdit(SDL.getContext());

                mLayout.addView(mTextEdit, params);
            } else {
                mTextEdit.setLayoutParams(params);
            }

            mTextEdit.setVisibility(View.VISIBLE);
            mTextEdit.requestFocus();

            InputMethodManager imm = (InputMethodManager) SDL.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.showSoftInput(mTextEdit, 0);

            mScreenKeyboardShown = true;
        }
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static boolean showTextInput(int x, int y, int w, int h) {
        // Transfer the task to the main thread as a Runnable
        return mSingleton.commandHandler.post(new ShowTextInputTask(x, y, w, h));
    }

    public static boolean isTextInputEvent(KeyEvent event) {

        // Key pressed with Ctrl should be sent as SDL_KEYDOWN/SDL_KEYUP and not SDL_TEXTINPUT
        if (android.os.Build.VERSION.SDK_INT >= 11) {
            if (event.isCtrlPressed()) {
                return false;
            }
        }

        return event.isPrintingKey() || event.getKeyCode() == KeyEvent.KEYCODE_SPACE;
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static Surface getNativeSurface() {
        if (SDLActivity.mSurface == null) {
            return null;
        }
        return SDLActivity.mSurface.getNativeSurface();
    }

    // Input

    /**
     * This method is called by SDL using JNI.
     * @return an array which may be empty but is never null.
     */
    public static int[] inputGetInputDeviceIds(int sources) {
        int[] ids = InputDevice.getDeviceIds();
        int[] filtered = new int[ids.length];
        int used = 0;
        for (int i = 0; i < ids.length; ++i) {
            InputDevice device = InputDevice.getDevice(ids[i]);
            if ((device != null) && ((device.getSources() & sources) != 0)) {
                filtered[used++] = device.getId();
            }
        }
        return Arrays.copyOf(filtered, used);
    }

    // APK expansion files support

    /** com.android.vending.expansion.zipfile.ZipResourceFile object or null. */
    private static Object expansionFile;

    /** com.android.vending.expansion.zipfile.ZipResourceFile's getInputStream() or null. */
    private static Method expansionFileMethod;

    /**
     * This method is called by SDL using JNI.
     * @return an InputStream on success or null if no expansion file was used.
     * @throws IOException on errors. Message is set for the SDL error message.
     */
    public static InputStream openAPKExpansionInputStream(String fileName) throws IOException {
        // Get a ZipResourceFile representing a merger of both the main and patch files
        if (expansionFile == null) {
            String mainHint = nativeGetHint("SDL_ANDROID_APK_EXPANSION_MAIN_FILE_VERSION");
            if (mainHint == null) {
                return null; // no expansion use if no main version was set
            }
            String patchHint = nativeGetHint("SDL_ANDROID_APK_EXPANSION_PATCH_FILE_VERSION");
            if (patchHint == null) {
                return null; // no expansion use if no patch version was set
            }

            Integer mainVersion;
            Integer patchVersion;
            try {
                mainVersion = Integer.valueOf(mainHint);
                patchVersion = Integer.valueOf(patchHint);
            } catch (NumberFormatException ex) {
                ex.printStackTrace();
                throw new IOException("No valid file versions set for APK expansion files", ex);
            }

            try {
                // To avoid direct dependency on Google APK expansion library that is
                // not a part of Android SDK we access it using reflection
                expansionFile = Class.forName("com.android.vending.expansion.zipfile.APKExpansionSupport")
                    .getMethod("getAPKExpansionZipFile", Context.class, int.class, int.class)
                    .invoke(null, SDL.getContext(), mainVersion, patchVersion);

                expansionFileMethod = expansionFile.getClass()
                    .getMethod("getInputStream", String.class);
            } catch (Exception ex) {
                ex.printStackTrace();
                expansionFile = null;
                expansionFileMethod = null;
                throw new IOException("Could not access APK expansion support library", ex);
            }
        }

        // Get an input stream for a known file inside the expansion file ZIPs
        InputStream fileStream;
        try {
            fileStream = (InputStream)expansionFileMethod.invoke(expansionFile, fileName);
        } catch (Exception ex) {
            // calling "getInputStream" failed
            ex.printStackTrace();
            throw new IOException("Could not open stream from APK expansion file", ex);
        }

        if (fileStream == null) {
            // calling "getInputStream" was successful but null was returned
            throw new IOException("Could not find path in APK expansion file");
        }

        return fileStream;
    }

    // Messagebox

    /** Result of current messagebox. Also used for blocking the calling thread. */
    protected final int[] messageboxSelection = new int[1];

    /** Id of current dialog. */
    protected int dialogs = 0;

    /**
     * This method is called by SDL using JNI.
     * Shows the messagebox from UI thread and block calling thread.
     * buttonFlags, buttonIds and buttonTexts must have same length.
     * @param buttonFlags array containing flags for every button.
     * @param buttonIds array containing id for every button.
     * @param buttonTexts array containing text for every button.
     * @param colors null for default or array of length 5 containing colors.
     * @return button id or -1.
     */
    public int messageboxShowMessageBox(
            final int flags,
            final String title,
            final String message,
            final int[] buttonFlags,
            final int[] buttonIds,
            final String[] buttonTexts,
            final int[] colors) {

        messageboxSelection[0] = -1;

        // sanity checks

        if ((buttonFlags.length != buttonIds.length) && (buttonIds.length != buttonTexts.length)) {
            return -1; // implementation broken
        }

        // collect arguments for Dialog

        final Bundle args = new Bundle();
        args.putInt("flags", flags);
        args.putString("title", title);
        args.putString("message", message);
        args.putIntArray("buttonFlags", buttonFlags);
        args.putIntArray("buttonIds", buttonIds);
        args.putStringArray("buttonTexts", buttonTexts);
        args.putIntArray("colors", colors);

        // trigger Dialog creation on UI thread

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                showDialog(dialogs++, args);
            }
        });

        // block the calling thread

        synchronized (messageboxSelection) {
            try {
                messageboxSelection.wait();
            } catch (InterruptedException ex) {
                ex.printStackTrace();
                return -1;
            }
        }

        // return selected value

        return messageboxSelection[0];
    }

    @Override
    protected Dialog onCreateDialog(int ignore, Bundle args) {

        // TODO set values from "flags" to messagebox dialog

        // get colors

        int[] colors = args.getIntArray("colors");
        int backgroundColor;
        int textColor;
        int buttonBorderColor;
        int buttonBackgroundColor;
        int buttonSelectedColor;
        if (colors != null) {
            int i = -1;
            backgroundColor = colors[++i];
            textColor = colors[++i];
            buttonBorderColor = colors[++i];
            buttonBackgroundColor = colors[++i];
            buttonSelectedColor = colors[++i];
        } else {
            backgroundColor = Color.TRANSPARENT;
            textColor = Color.TRANSPARENT;
            buttonBorderColor = Color.TRANSPARENT;
            buttonBackgroundColor = Color.TRANSPARENT;
            buttonSelectedColor = Color.TRANSPARENT;
        }

        // create dialog with title and a listener to wake up calling thread

        final Dialog dialog = new Dialog(this);
        dialog.setTitle(args.getString("title"));
        dialog.setCancelable(false);
        dialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface unused) {
                synchronized (messageboxSelection) {
                    messageboxSelection.notify();
                }
            }
        });

        // create text

        TextView message = new TextView(this);
        message.setGravity(Gravity.CENTER);
        message.setText(args.getString("message"));
        if (textColor != Color.TRANSPARENT) {
            message.setTextColor(textColor);
        }

        // create buttons

        int[] buttonFlags = args.getIntArray("buttonFlags");
        int[] buttonIds = args.getIntArray("buttonIds");
        String[] buttonTexts = args.getStringArray("buttonTexts");

        final SparseArray<Button> mapping = new SparseArray<Button>();

        LinearLayout buttons = new LinearLayout(this);
        buttons.setOrientation(LinearLayout.HORIZONTAL);
        buttons.setGravity(Gravity.CENTER);
        for (int i = 0; i < buttonTexts.length; ++i) {
            Button button = new Button(this);
            final int id = buttonIds[i];
            button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    messageboxSelection[0] = id;
                    dialog.dismiss();
                }
            });
            if (buttonFlags[i] != 0) {
                // see SDL_messagebox.h
                if ((buttonFlags[i] & 0x00000001) != 0) {
                    mapping.put(KeyEvent.KEYCODE_ENTER, button);
                }
                if ((buttonFlags[i] & 0x00000002) != 0) {
                    mapping.put(KeyEvent.KEYCODE_ESCAPE, button); /* API 11 */
                }
            }
            button.setText(buttonTexts[i]);
            if (textColor != Color.TRANSPARENT) {
                button.setTextColor(textColor);
            }
            if (buttonBorderColor != Color.TRANSPARENT) {
                // TODO set color for border of messagebox button
            }
            if (buttonBackgroundColor != Color.TRANSPARENT) {
                Drawable drawable = button.getBackground();
                if (drawable == null) {
                    // setting the color this way removes the style
                    button.setBackgroundColor(buttonBackgroundColor);
                } else {
                    // setting the color this way keeps the style (gradient, padding, etc.)
                    drawable.setColorFilter(buttonBackgroundColor, PorterDuff.Mode.MULTIPLY);
                }
            }
            if (buttonSelectedColor != Color.TRANSPARENT) {
                // TODO set color for selected messagebox button
            }
            buttons.addView(button);
        }

        // create content

        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        content.addView(message);
        content.addView(buttons);
        if (backgroundColor != Color.TRANSPARENT) {
            content.setBackgroundColor(backgroundColor);
        }

        // add content to dialog and return

        dialog.setContentView(content);
        dialog.setOnKeyListener(new Dialog.OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface d, int keyCode, KeyEvent event) {
                Button button = mapping.get(keyCode);
                if (button != null) {
                    if (event.getAction() == KeyEvent.ACTION_UP) {
                        button.performClick();
                    }
                    return true; // also for ignored actions
                }
                return false;
            }
        });

        return dialog;
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static boolean clipboardHasText() {
        return mClipboardHandler.clipboardHasText();
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static String clipboardGetText() {
        return mClipboardHandler.clipboardGetText();
    }

    /**
     * This method is called by SDL using JNI.
     */
    public static void clipboardSetText(String string) {
        mClipboardHandler.clipboardSetText(string);
    }

}

/**
    Simple runnable to start the SDL application
*/
class SDLMain implements Runnable {
    @Override
    public void run() {
        // Runs SDL_main()
        String library = SDLActivity.mSingleton.getMainSharedObject();
        String function = SDLActivity.mSingleton.getMainFunction();
        String[] arguments = SDLActivity.mSingleton.getArguments();

        Log.v("SDL", "Running main function " + function + " from library " + library);
        SDLActivity.nativeRunMain(library, function, arguments);

        Log.v("SDL", "Finished main function");
    }
}

/* This is a fake invisible editor view that receives the input and defines the
 * pan&scan region
 */
class DummyEdit extends View implements View.OnKeyListener {
    InputConnection ic;

    public DummyEdit(Context context) {
        super(context);
        setFocusableInTouchMode(true);
        setFocusable(true);
        setOnKeyListener(this);
    }

    @Override
    public boolean onCheckIsTextEditor() {
        return true;
    }

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        /*
         * This handles the hardware keyboard input
         */
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            if (SDLActivity.isTextInputEvent(event)) {
                ic.commitText(String.valueOf((char) event.getUnicodeChar()), 1);
            }
            SDLActivity.onNativeKeyDown(keyCode);
            return true;
        } else if (event.getAction() == KeyEvent.ACTION_UP) {
            SDLActivity.onNativeKeyUp(keyCode);
            return true;
        }
        return false;
    }

    //
    @Override
    public boolean onKeyPreIme (int keyCode, KeyEvent event) {
        // As seen on StackOverflow: http://stackoverflow.com/questions/7634346/keyboard-hide-event
        // FIXME: Discussion at http://bugzilla.libsdl.org/show_bug.cgi?id=1639
        // FIXME: This is not a 100% effective solution to the problem of detecting if the keyboard is showing or not
        // FIXME: A more effective solution would be to assume our Layout to be RelativeLayout or LinearLayout
        // FIXME: And determine the keyboard presence doing this: http://stackoverflow.com/questions/2150078/how-to-check-visibility-of-software-keyboard-in-android
        // FIXME: An even more effective way would be if Android provided this out of the box, but where would the fun be in that :)
        if (event.getAction()==KeyEvent.ACTION_UP && keyCode == KeyEvent.KEYCODE_BACK) {
            if (SDLActivity.mTextEdit != null && SDLActivity.mTextEdit.getVisibility() == View.VISIBLE) {
                SDLActivity.onNativeKeyboardFocusLost();
            }
        }
        return super.onKeyPreIme(keyCode, event);
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        ic = new SDLInputConnection(this, true);

        outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD;
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI
                | EditorInfo.IME_FLAG_NO_FULLSCREEN /* API 11 */;

        return ic;
    }
}

class SDLInputConnection extends BaseInputConnection {

    public SDLInputConnection(View targetView, boolean fullEditor) {
        super(targetView, fullEditor);

    }

    @Override
    public boolean sendKeyEvent(KeyEvent event) {
        /*
         * This handles the keycodes from soft keyboard (and IME-translated input from hardkeyboard)
         */
        int keyCode = event.getKeyCode();
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            if (SDLActivity.isTextInputEvent(event)) {
                commitText(String.valueOf((char) event.getUnicodeChar()), 1);
            }
            SDLActivity.onNativeKeyDown(keyCode);
            return true;
        } else if (event.getAction() == KeyEvent.ACTION_UP) {
            SDLActivity.onNativeKeyUp(keyCode);
            return true;
        }
        return super.sendKeyEvent(event);
    }

    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {

        nativeCommitText(text.toString(), newCursorPosition);

        return super.commitText(text, newCursorPosition);
    }

    @Override
    public boolean setComposingText(CharSequence text, int newCursorPosition) {

        nativeSetComposingText(text.toString(), newCursorPosition);

        return super.setComposingText(text, newCursorPosition);
    }

    public native void nativeCommitText(String text, int newCursorPosition);

    public native void nativeSetComposingText(String text, int newCursorPosition);

    @Override
    public boolean deleteSurroundingText(int beforeLength, int afterLength) {
        // Workaround to capture backspace key. Ref: http://stackoverflow.com/questions/14560344/android-backspace-in-webview-baseinputconnection
        // and https://bugzilla.libsdl.org/show_bug.cgi?id=2265
        if (beforeLength > 0 && afterLength == 0) {
            boolean ret = true;
            // backspace(s)
            while (beforeLength-- > 0) {
               boolean ret_key = sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL))
                              && sendKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL));
               ret = ret && ret_key;
            }
            return ret;
        }

        return super.deleteSurroundingText(beforeLength, afterLength);
    }
}

interface SDLClipboardHandler {

    public boolean clipboardHasText();
    public String clipboardGetText();
    public void clipboardSetText(String string);

}


class SDLClipboardHandler_API11 implements
    SDLClipboardHandler,
    android.content.ClipboardManager.OnPrimaryClipChangedListener {

    protected android.content.ClipboardManager mClipMgr;

    SDLClipboardHandler_API11() {
       mClipMgr = (android.content.ClipboardManager) SDL.getContext().getSystemService(Context.CLIPBOARD_SERVICE);
       mClipMgr.addPrimaryClipChangedListener(this);
    }

    @Override
    public boolean clipboardHasText() {
       return mClipMgr.hasText();
    }

    @Override
    public String clipboardGetText() {
        CharSequence text;
        text = mClipMgr.getText();
        if (text != null) {
           return text.toString();
        }
        return null;
    }

    @Override
    public void clipboardSetText(String string) {
       mClipMgr.removePrimaryClipChangedListener(this);
       mClipMgr.setText(string);
       mClipMgr.addPrimaryClipChangedListener(this);
    }

    @Override
    public void onPrimaryClipChanged() {
        SDLActivity.onNativeClipboardChanged();
    }

}

class SDLClipboardHandler_Old implements
    SDLClipboardHandler {

    protected android.text.ClipboardManager mClipMgrOld;

    SDLClipboardHandler_Old() {
       mClipMgrOld = (android.text.ClipboardManager) SDL.getContext().getSystemService(Context.CLIPBOARD_SERVICE);
    }

    @Override
    public boolean clipboardHasText() {
       return mClipMgrOld.hasText();
    }

    @Override
    public String clipboardGetText() {
       CharSequence text;
       text = mClipMgrOld.getText();
       if (text != null) {
          return text.toString();
       }
       return null;
    }

    @Override
    public void clipboardSetText(String string) {
       mClipMgrOld.setText(string);
    }
}
