#!/usr/bin/env bash

CUR_DIR=$(pwd)
TARGET_DIR=${CUR_DIR}/android_app_build
URHO_DIR=${CUR_DIR}
SRC_DIR=${URHO_DIR}/build_android

cp ${TARGET_DIR}/app/src/main/AndroidManifest.xml ${URHO_DIR}/Android/
cp ${TARGET_DIR}/app/src/main/java/org/libsdl/app/*.java ${URHO_DIR}/Android/src/org/libsdl/app/
cp ${TARGET_DIR}/app/src/main/java/com/github/urho3d/*.java ${URHO_DIR}/Android/src/com/github/urho3d/

cp -rf ${TARGET_DIR}/app/src/main/res ${URHO_DIR}/Android/

cp ${TARGET_DIR}/build.gradle ${URHO_DIR}/Android/build.gradle-project
cp ${TARGET_DIR}/app/build.gradle ${URHO_DIR}/Android/build.gradle-app

sync