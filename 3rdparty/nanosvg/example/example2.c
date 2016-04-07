//
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <stdio.h>
#include <string.h>
#include <float.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

int main()
{
	NSVGimage *image = NULL;
	NSVGrasterizer *rast = NULL;
	unsigned char* img = NULL;
	int w, h;
	const char* filename = "../example/_test.svg";

	printf("parsing %s\n", filename);
	image = nsvgParseFromFile(filename, "px", 96.0f);
	if (image == NULL) {
		printf("Could not open SVG image.\n");
		goto error;
	}
	w = (int)image->width;
	h = (int)image->height;

	rast = nsvgCreateRasterizer();
	if (rast == NULL) {
		printf("Could not init rasterizer.\n");
		goto error;
	}

	img = malloc(w*h*4);
	if (img == NULL) {
		printf("Could not alloc image buffer.\n");
		goto error;
	}

	printf("rasterizing image %d x %d\n", w, h);
	nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);

	printf("writing svg.png\n");
 	stbi_write_png("svg.png", w, h, 4, img, w*4);

error:
	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);

	return 0;
}
