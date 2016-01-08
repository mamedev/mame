#!/usr/bin/python

import sys, Image, array, os

def main(argv):
	x = 0
	y = 0
	wimg = 0
	h = 8
	w = 256
	if len(sys.argv) == 1:
		print "fnt2gif file height width "
		exit(0)
	elif len(sys.argv) > 1:
		f = open(sys.argv[1], "rb")
		length = os.stat(sys.argv[1]).st_size
	if len(sys.argv) > 2:
		h = int(sys.argv[2])
	if len(sys.argv) > 3:
		w = int(sys.argv[3]) 

	wimg = length/w*8
	print wimg, length
	img = Image.new('RGB', (w, wimg), 'black')
	pixels = img.load()
	arr = array.array('B')
	arr.read(f, length)
	l = 0
	y1 = 0
	for i in arr:
		l = l + 1
		x1 = 0
#		print l, int(i), x, ",", y
		for j in range(0, 8):
			pixels[x + x1, y + y1] = (255,255,255) if i & (0x80 >> j) else (0,0,0)
			x1 = x1 + 1
		y1 = y1 + 1
		if y1 >= h:
			y1 = 0
			x = x + 8
		if x >=  w:
			x = 0
			y = y + h
			
	img.save(sys.argv[1] + ".gif", 'GIF')

if __name__ == "__main__":
   main(sys.argv[1:])