#!/usr/bin/env bash

cur_dir=`pwd`

./compile_linux.sh
if [ $? -eq 0 ]; then
    echo 'BUILD OK'
    cd $cur_dir/build_linux/bin/
    ./Game "$@"
else
    echo 'BUILD FAIL'
fi
