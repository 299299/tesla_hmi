#!/usr/bin/env bash

CUR_DIR=$(pwd)
TARGET_DIR=${CUR_DIR}/android_app_build
URHO_DIR=${CUR_DIR}
SRC_DIR=${URHO_DIR}/build_android

SO_1=*.so

./code_format.sh

cd build_android
make -j8
cd -

#mkdir -p ${TARGET_DIR}/app/src/main/jniLibs/armeabi-v7a/
mkdir -p ${TARGET_DIR}/app/libs/
mkdir -p ${TARGET_DIR}/app/libs/armeabi-v7a/

rm ${TARGET_DIR}/app/build/outputs/apk/debug/app-debug.apk
#rm ${TARGET_DIR}/app/src/main/jniLibs/armeabi-v7a/${SO_1}
#rm ${TARGET_DIR}/app/src/main/jniLibs/x86/${SO_1}

#cp ${SRC_DIR}/libs/armeabi-v7a/${SO_1} ${TARGET_DIR}/app/src/main/jniLibs/armeabi-v7a/
cp ${SRC_DIR}/libs/armeabi-v7a/${SO_1} ${TARGET_DIR}/app/libs/armeabi-v7a/
#cp ${SRC_DIR}/libs/x86/${SO_1} ${TARGET_DIR}/app/src/main/jniLibs/x86/

cp ${URHO_DIR}/Android/navi_sdk/*.jar ${TARGET_DIR}/app/libs/

cp ${URHO_DIR}/Android/navi_sdk/*.so ${TARGET_DIR}/app/libs/armeabi-v7a/
#cp ${URHO_DIR}/Android/navi_sdk/*.so ${TARGET_DIR}/app/src/main/jniLibs/armeabi-v7a/

cp -rf ${URHO_DIR}/bin/Data/MY ${TARGET_DIR}/app/src/main/assets/Data/

#cp ${URHO_DIR}/Android/AndroidManifest.xml ${TARGET_DIR}/app/src/main/
#cp ${URHO_DIR}/Android/src/org/libsdl/app/* ${TARGET_DIR}/app/src/main/java/org/libsdl/app/
#cp ${URHO_DIR}/Android/src/com/github/urho3d/* ${TARGET_DIR}/app/src/main/java/com/github/urho3d/

cp -rf ${URHO_DIR}/Android/res ${TARGET_DIR}/app/src/main/

#cp ${URHO_DIR}/Android/build.gradle-project ${TARGET_DIR}/build.gradle
#cp ${URHO_DIR}/Android/build.gradle-app ${TARGET_DIR}/app/build.gradle

sync