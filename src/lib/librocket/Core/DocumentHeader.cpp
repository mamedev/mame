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
#include "DocumentHeader.h"
#include "XMLParseTools.h"
#include <Rocket/Core.h>

namespace Rocket {
namespace Core {

void DocumentHeader::MergeHeader(const DocumentHeader& header)
{
	// Copy the title across if ours is empty
	if (title.Empty())
		title = header.title;
	// Copy the url across if ours is empty
	if (source.Empty())
		source = header.source;

	// Combine internal data	
	rcss_inline.insert(rcss_inline.end(), header.rcss_inline.begin(), header.rcss_inline.end());	
	scripts_inline.insert(scripts_inline.end(), header.scripts_inline.begin(), header.scripts_inline.end());
	
	// Combine external data, keeping relative paths
	MergePaths(template_resources, header.template_resources, header.source);
	MergePaths(rcss_external, header.rcss_external, header.source);
	MergePaths(scripts_external, header.scripts_external, header.source);
}

void DocumentHeader::MergePaths(StringList& target, const StringList& source, const String& source_path)
{
	for (size_t i = 0; i < source.size(); i++)
	{
		String joined_path;
		Rocket::Core::GetSystemInterface()->JoinPath(joined_path, source_path.Replace("|", ":"), source[i].Replace("|", ":"));

		target.push_back(joined_path.Replace(":", "|"));
	}
}

}
}
