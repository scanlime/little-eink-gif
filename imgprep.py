#
# Quick utility to pre-process images (including animaged gifs)
# into a binary blob of compressed framebuffer data.
#

from glob import glob
from PIL import Image
import struct
import zlib
import os


def deflate(data, level=9):
    zData = zlib.compress(data, level)

    # Strip off the zlib header, and return the raw DEFLATE data stream.
    # See the zlib RFC: http://www.gzip.org/zlib/rfc-zlib.html

    cmf = ord(zData[0])
    flg = ord(zData[1])
    assert (cmf & 0x0F) == 8   # DEFLATE algorithm
    assert (flg & 0x20) == 0   # No preset dictionary

    # Strip off 2-byte header and 4-byte checksum
    return zData[2:len(zData)-4]


def encode_frame(im):
    # Convert to 2-bit per pixel grayscale

    bg = Image.new('L', im.size, color=255)
    rgba = im.convert('RGBA')
    bg.paste(rgba, mask=rgba.split()[3])
    gray = bg.tobytes()

    fb = ''.join(chr(
        ((ord(gray[i*4 + 0]) >> 6) << 6) |
        ((ord(gray[i*4 + 1]) >> 6) << 4) |
        ((ord(gray[i*4 + 2]) >> 6) << 2) |
        ((ord(gray[i*4 + 3]) >> 6) << 0) )
        for i in range(len(gray)/4))

    # Compress using the raw Deflate algorithm
    zfb = deflate(fb)

    # Pad to 4-byte boundary
    return zfb + chr(0) * (-len(zfb) & 3)

def encode_image(im):
    assert im.size == (72, 172)
    results = []
    while True:
        millis = im.info.get('duration') or 6000
        data = encode_frame(im)
        header = struct.pack("<LL", millis, len(data))
        results.append(header)
        results.append(data)
        try:
            im.seek(im.tell() + 1)
        except EOFError:
            break
    return ''.join(results)


def main():
    f = open('images.bin', 'wb')
    for file in glob('images/*'):
        if os.path.isfile(file):
            print('  IMAGE   %s' % file)
            f.write(encode_image(Image.open(file)))
    f.close()

if __name__ == '__main__':
    main()
