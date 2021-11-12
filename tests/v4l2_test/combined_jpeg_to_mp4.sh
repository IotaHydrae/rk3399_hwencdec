#!/bin/bash
ffmpeg -f image2 -i ./abc_%04d.jpeg -vcodec libx264 -b:v 200k -r 25 -s 640x480 -y multi_jpg.mp4
