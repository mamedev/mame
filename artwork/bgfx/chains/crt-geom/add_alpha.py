"""
Assumes the input RGB image subpixels are all either 0 or 255.
Counts the bright subpixels in the whole image. This should be divisible by 3.
Creates a uniform alpha channel containing 255 minus 1/3 of
the number of bright subpixels.
Writes the RGBA image to the output file.
"""
import sys
import numpy
import PIL
import PIL.Image

if len(sys.argv) != 3:
    print("usage: add_alpha.py in.png out.png")
    sys.exit(1)

img = PIL.Image.open(sys.argv[1])

arr = numpy.asarray(img)
count = (arr==255).sum()
assert count%3 == 0
count //= 3

alpha = arr[:,:,0,None]*0 + (255-count)

arr = numpy.concatenate((arr,alpha),axis=2)

# DEBUG
#print(arr)

out = PIL.Image.fromarray(arr)
out.save(sys.argv[2])

