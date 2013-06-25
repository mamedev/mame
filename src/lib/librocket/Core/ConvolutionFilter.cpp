/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "precompiled.h"
#include <Rocket/Core/ConvolutionFilter.h>

namespace Rocket {
namespace Core {

ConvolutionFilter::ConvolutionFilter()
{
	kernel_size = 0;
	kernel = NULL;

	operation = MEDIAN;
}

ConvolutionFilter::~ConvolutionFilter()
{
	delete[] kernel;
}

// Initialises the filter. A filter must be initialised and populated with values before use.
bool ConvolutionFilter::Initialise(int _kernel_size, FilterOperation _operation)
{
	if (_kernel_size <= 0)
		return false;

	kernel_size = Math::Max(_kernel_size, 1);
	kernel_size = kernel_size * 2 + 1;

	kernel = new float[kernel_size * kernel_size];
	memset(kernel, 0, kernel_size * kernel_size * sizeof(float));

	operation = _operation;
	return true;
}

// Returns a reference to one of the rows of the filter kernel.
float* ConvolutionFilter::operator[](int index)
{
	ROCKET_ASSERT(kernel != NULL);

	index = Math::Max(index, 0);
	index = Math::Min(index, kernel_size - 1);

	return kernel + kernel_size * index;
}

// Runs the convolution filter.
void ConvolutionFilter::Run(byte* destination, const Vector2i& destination_dimensions, int destination_stride, const byte* source, const Vector2i& source_dimensions, const Vector2i& source_offset) const
{
	for (int y = 0; y < destination_dimensions.y; ++y)
	{
		for (int x = 0; x < destination_dimensions.x; ++x)
		{
			int num_pixels = 0;
			int opacity = 0;

			for (int kernel_y = 0; kernel_y < kernel_size; ++kernel_y)
			{
				int source_y = y - source_offset.y - ((kernel_size - 1) / 2) + kernel_y;

				for (int kernel_x = 0; kernel_x < kernel_size; ++kernel_x)
				{
					int pixel_opacity;

					int source_x = x - source_offset.x - ((kernel_size - 1) / 2) + kernel_x;
					if (source_y >= 0 &&
						source_y < source_dimensions.y &&
						source_x >= 0 &&
						source_x < source_dimensions.x)
					{
						pixel_opacity = Math::RealToInteger(source[source_y * source_dimensions.x + source_x] * kernel[kernel_y * kernel_size + kernel_x]);
					}
					else
						pixel_opacity = 0;

					switch (operation)
					{
						case MEDIAN:	opacity += pixel_opacity; break;
						case DILATION:	opacity = Math::Max(opacity, pixel_opacity); break;
						case EROSION:	opacity = num_pixels == 0 ? pixel_opacity : Math::Min(opacity, pixel_opacity); break;
					}

					++num_pixels;
				}
			}

			if (operation == MEDIAN)
				opacity /= num_pixels;

			opacity = Math::Min(255, opacity);
			destination[x * 4 + 3] = (byte) opacity;
		}

		destination += destination_stride;
	}
}

}
}
