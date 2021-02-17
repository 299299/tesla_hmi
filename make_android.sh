#!/usr/bin/env bash


cd ../ndk
NDK=$(pwd)
cd -
API=android-26

#-DURHO3D_64BIT=0

./cmake_android.sh build_android -DURHO3D_PHYSICS=1 -DURHO3D_LUA=0 -DURHO3D_URHO2D=0 -DURHO3D_NETWORK=1 -DURHO3D_NAVIGATION=0 \
                                 -DURHO3D_TOOLS=1 -DURHO3D_SAMPLES=0 -DURHO3D_EXTRAS=0 -DURHO3D_FILEWATCHER=0 -DURHO3D_ANGELSCRIPT=0 \
                                 -DURHO3D_WEBP=0 -DURHO3D_IK=0 -DURHO3D_LIB_TYPE=SHARED \
                                 -DANDROID_NDK=$NDK -DANDROID_NATIVE_API_LEVEL=$API "$@"
