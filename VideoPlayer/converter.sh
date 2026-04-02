#!/usr/bin/env bash

convert_bw_image() {
    convert $1 -resize 24x24 -monochrome $1.converted
}

convert_all_bw_images() {
    FILES=./output/*.png

    for f in $FILES; do
        convert_bw_image $f
    done
}

if [ -z $1 ]; then
    echo "No video given"
    exit 1
fi

rm -rf output/
mkdir -p output/

ffmpeg -i $1 -vf fps=10 output/out%d.png

convert_all_bw_images

uv run converter.py
