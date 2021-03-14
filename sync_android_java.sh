#!/usr/bin/env bash

CUR_DIR=$(pwd)
TARGET_DIR=${CUR_DIR}/android_app_build
URHO_DIR=${CUR_DIR}
SRC_DIR=${URHO_DIR}/build_android

cp ${URHO_DIR}/Android/AndroidManifest.xml ${TARGET_DIR}/app/src/main/
cp ${URHO_DIR}/Android/src/org/libsdl/app/* ${TARGET_DIR}/app/src/main/java/org/libsdl/app/
cp ${URHO_DIR}/Android/src/com/github/urho3d/* ${TARGET_DIR}/app/src/main/java/com/github/urho3d/
cp -rf ${URHO_DIR}/Android/res ${TARGET_DIR}/app/src/main/

cp ${URHO_DIR}/Android/build.gradle-project ${TARGET_DIR}/build.gradle
cp ${URHO_DIR}/Android/build.gradle-app ${TARGET_DIR}/app/build.gradle

sync