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

#ifndef ROCKETCOREDOCUMENTHEADER_H
#define ROCKETCOREDOCUMENTHEADER_H

#include <Rocket/Core/String.h>

namespace Rocket {
namespace Core {

/**
	The document header struct contains the
	header details gathered from an XML document parse.

	@author Lloyd Weehuizen
 */

class DocumentHeader
{
public:
	/// Path and filename this document was loaded from
	String source;
	/// The title of the document
	String title;	
	/// A list of template resources that can used while parsing the document
	StringList template_resources;

	/// Inline RCSS definitions
	StringList rcss_inline;
	/// External RCSS definitions that should be loaded
	StringList rcss_external;

	/// Inline script source
	StringList scripts_inline;
	/// External scripts that should be loaded
	StringList scripts_external;

	/// Merges the specified header with this one
	/// @param header Header to merge
	void MergeHeader(const DocumentHeader& header);

	/// Merges paths from one string list to another, preserving the base_path
	void MergePaths(StringList& target, const StringList& source, const String& base_path);
};

}
}

#endif
