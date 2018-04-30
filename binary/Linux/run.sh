#!/bin/bash

sudo apt-get install libavcodec-ffmpeg-extra56 libavformat-ffmpeg56 libavutil-ffmpeg54 libswscale-ffmpeg3 libudev-dev libdc1394-*

sudo cp include/opencv* -rf /usr/local/include/
sudo cp lib/libopencv_world* /usr/local/lib/
sudo cp lib/pkgconfig -rf /usr/local/lib/
