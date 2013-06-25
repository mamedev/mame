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

#ifndef ROCKETCORECONVOLUTIONFILTER_H
#define ROCKETCORECONVOLUTIONFILTER_H

namespace Rocket {
namespace Core {

/**
	A programmable convolution filter, designed to aid in the generation of texture data by custom
	FontEffect types.

	@author Peter Curry
 */

class ConvolutionFilter
{
public:
	enum FilterOperation
	{
		// The result is the median value of all the filtered pixels.
		MEDIAN,
		// The result is the smallest value of all filtered pixels.
		DILATION,
		// The result is the largest value of all the filtered pixels.
		EROSION
	};

	ConvolutionFilter();
	~ConvolutionFilter();

	/// Initialises the filter. A filter must be initialised and populated with values before use.
	/// @param[in] kernel_size The size of the filter's kernel each side of the origin. So, for example, a filter initialised with a size of 1 will store 9 values.
	/// @param[in] operation The operation the filter conducts to determine the result.
	bool Initialise(int kernel_size, FilterOperation operation = MEDIAN);

	/// Returns a reference to one of the rows of the filter kernel.
	/// @param[in] index The index of the desired row.
	/// @return The row of kernel values.
	float* operator[](int index);

	/// Runs the convolution filter. The filter will operate on each pixel in the destination
	/// surface, setting its opacity to the result the filter on the source opacity values. The
	/// colour values will remain unchanged.
	/// @param[in] destination The RGBA-encoded destination buffer.
	/// @param[in] destination_dimensions The size of the destination region (in pixels).
	/// @param[in] destination_stride The stride (in bytes) of the destination region.
	/// @param[in] source The opacity information for the source buffer.
	/// @param[in] source_dimensions The size of the source region (in pixels). The stride is assumed to be equivalent to the horizontal width.
	/// @param[in] source_offset The offset of the source region from the destination region. This is usually the same as the kernel size.
	void Run(byte* destination, const Vector2i& destination_dimensions, int destination_stride, const byte* source, const Vector2i& source_dimensions, const Vector2i& source_offset) const;

private:
	int kernel_size;
	float* kernel;

	FilterOperation operation;
};

}
}

#endif

