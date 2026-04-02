#!/usr/bin/env python3
# /// script
# dependencies = [
#   "Pillow",
# ]
# ///

from PIL import Image
from os import listdir
from os.path import isfile, join

files = []

DIRNAME = "output"
onlyFiles = [f for f in listdir(DIRNAME) if isfile(join(DIRNAME, f))]

for file in onlyFiles:
    if file.endswith(".converted"):
        files.append(DIRNAME + "/" + file)

for file in files:
    im = Image.open(file)

    rgb_im = im.convert('RGB')
    width, height = rgb_im.size
    
    with open(file.replace(".converted", ".pharma"), "w+") as f:
        for y in range(height):
            for x in range(width):
                r, g, b = rgb_im.getpixel((x, y))

                if r == 0 and g == 0 and b == 0:
                    f.write(".")
                else:
                    f.write("x")
            f.write("\n")
