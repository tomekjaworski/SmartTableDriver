#!/bin/bash

ffmpeg -i frame_%05d.png -c:v libx264 -vf fps=10 -pix_fmt yuv420p out.mp4


