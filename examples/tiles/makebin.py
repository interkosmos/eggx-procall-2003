#!/usr/bin/env python3
#
# Call with:
#
#   $ python3 ./makebin.py tiles.png tiles.bin
#
# Change TILE_WIDTH and TILE_HEIGHT to actual size.
#
import sys
from PIL import Image

TILE_WIDTH  = 32
TILE_HEIGHT = 32

image_path = str(sys.argv[1])
bin_path = str(sys.argv[2])

image = Image.open(image_path)
w, h = image.size
pixels = image.load()

if w % TILE_WIDTH != 0:
    print("Image width is not multiple of {}.".format(TILE_WIDTH))
    exit()

if h % TILE_HEIGHT != 0:
    print("Image height is not multiple of {].".format(TILE_HEIGHT))
    exit()

print("Width:  {}".format(w))
print("Height: {}".format(h))
print("Converting input file {} ...".format(image_path))

with open(bin_path, 'wb') as out:
    for y in range(0, h, TILE_HEIGHT):
        for x in range(0, w, TILE_WIDTH):
            print("Tile x: {} y: {}".format(x, y))
            for j in range(0, TILE_HEIGHT):
                for i in range(0, TILE_WIDTH):
                    r, g, b, a = pixels[x + i, y + j]
                    out.write(a.to_bytes(1, byteorder='big'))
                    out.write(r.to_bytes(1, byteorder='big'))
                    out.write(g.to_bytes(1, byteorder='big'))
                    out.write(b.to_bytes(1, byteorder='big'))

print("Finished.")
