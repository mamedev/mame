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
#include "StyleSheetParser.h"
#include <algorithm>
#include "StyleSheetFactory.h"
#include "StyleSheetNode.h"
#include <Rocket/Core/Log.h>
#include <Rocket/Core/StreamMemory.h>
#include <Rocket/Core/StyleSheet.h>
#include <Rocket/Core/StyleSheetSpecification.h>

namespace Rocket {
namespace Core {

StyleSheetParser::StyleSheetParser()
{
	line_number = 0;
	stream = NULL;
	parse_buffer_pos = 0;
}

StyleSheetParser::~StyleSheetParser()
{
}

int StyleSheetParser::Parse(StyleSheetNode* node, Stream* _stream)
{
	int rule_count = 0;
	line_number = 0;
	stream = _stream;
	stream_file_name = stream->GetSourceURL().GetURL().Replace("|", ":");

	// Look for more styles while data is available
	while (FillBuffer())
	{
		String style_names;
		
		while (FindToken(style_names, "{", true))
		{
			// Read the attributes
			PropertyDictionary properties;
			if (!ReadProperties(properties))
			{
				continue;
			}

			StringList style_name_list;
			StringUtilities::ExpandString(style_name_list, style_names);

			// Add style nodes to the root of the tree
			for (size_t i = 0; i < style_name_list.size(); i++)
				ImportProperties(node, style_name_list[i], properties, rule_count);

			rule_count++;
		}
	}	

	return rule_count;
}

bool StyleSheetParser::ParseProperties(PropertyDictionary& parsed_properties, const String& properties)
{
	stream = new StreamMemory((const byte*)properties.CString(), properties.Length());
	bool success = ReadProperties(parsed_properties);
	stream->RemoveReference();
	stream = NULL;
	return success;
}

bool StyleSheetParser::ReadProperties(PropertyDictionary& properties)
{
	int rule_line_number = line_number;
	String name;
	String value;

	enum ParseState { NAME, VALUE, QUOTE };
	ParseState state = NAME;

	char character;
	char previous_character = 0;
	while (ReadCharacter(character))
	{
		parse_buffer_pos++;

		switch (state)
		{
			case NAME:
			{
				if (character == ';')
				{
					name = StringUtilities::StripWhitespace(name);
					if (!name.Empty())
					{
						Log::Message(Log::LT_WARNING, "Found name with no value parsing property declaration '%s' at %s:%d", name.CString(), stream_file_name.CString(), line_number);
						name.Clear();
					}
				}
				else if (character == '}')
				{
					name = StringUtilities::StripWhitespace(name);
					if (!StringUtilities::StripWhitespace(name).Empty())
						Log::Message(Log::LT_WARNING, "End of rule encountered while parsing property declaration '%s' at %s:%d", name.CString(), stream_file_name.CString(), line_number);
					return true;
				}
				else if (character == ':')
				{
					name = StringUtilities::StripWhitespace(name);
					state = VALUE;
				}
				else
					name.Append(character);
			}
			break;
			
			case VALUE:
			{
				if (character == ';')
				{
					value = StringUtilities::StripWhitespace(value);

					if (!StyleSheetSpecification::ParsePropertyDeclaration(properties, name, value, stream_file_name, rule_line_number))
						Log::Message(Log::LT_WARNING, "Syntax error parsing property declaration '%s: %s;' in %s: %d.", name.CString(), value.CString(), stream_file_name.CString(), line_number);

					name.Clear();
					value.Clear();
					state = NAME;
				}
				else if (character == '}')
				{
					Log::Message(Log::LT_WARNING, "End of rule encountered while parsing property declaration '%s: %s;' in %s: %d.", name.CString(), value.CString(), stream_file_name.CString(), line_number);
					return true;
				}
				else
				{
					value.Append(character);
					if (character == '"')
						state = QUOTE;
				}
			}
			break;

			case QUOTE:
			{
				value.Append(character);
				if (character == '"' && previous_character != '/')
					state = VALUE;
			}
			break;
		}

		previous_character = character;
	}

	if (!name.Empty() || !value.Empty())
		Log::Message(Log::LT_WARNING, "Invalid property declaration at %s:%d", stream_file_name.CString(), line_number);
	
	return true;
}

// Updates the StyleNode tree, creating new nodes as necessary, setting the definition index
bool StyleSheetParser::ImportProperties(StyleSheetNode* node, const String& names, const PropertyDictionary& properties, int rule_specificity)
{
	StyleSheetNode* tag_node = NULL;
	StyleSheetNode* leaf_node = node;

	StringList nodes;
	StringUtilities::ExpandString(nodes, names, ' ');

	// Create each node going down the tree
	for (size_t i = 0; i < nodes.size(); i++)
	{
		String name = nodes[i];

		String tag;
		String id;
		StringList classes;
		StringList pseudo_classes;
		StringList structural_pseudo_classes;

		size_t index = 0;
		while (index < name.Length())
		{
			size_t start_index = index;
			size_t end_index = index + 1;

			// Read until we hit the next identifier.
			while (end_index < name.Length() &&
				   name[end_index] != '#' &&
				   name[end_index] != '.' &&
				   name[end_index] != ':')
				end_index++;

			String identifier = name.Substring(start_index, end_index - start_index);
			if (!identifier.Empty())
			{
				switch (identifier[0])
				{
					case '#':	id = identifier.Substring(1); break;
					case '.':	classes.push_back(identifier.Substring(1)); break;
					case ':':
					{
						String pseudo_class_name = identifier.Substring(1);
						if (StyleSheetFactory::GetSelector(pseudo_class_name) != NULL)
							structural_pseudo_classes.push_back(pseudo_class_name);
						else
							pseudo_classes.push_back(pseudo_class_name);
					}
					break;

					default:	tag = identifier;
				}
			}

			index = end_index;
		}

		// Sort the classes and pseudo-classes so they are consistent across equivalent declarations that shuffle the
		// order around.
		std::sort(classes.begin(), classes.end());
		std::sort(pseudo_classes.begin(), pseudo_classes.end());
		std::sort(structural_pseudo_classes.begin(), structural_pseudo_classes.end());

		// Get the named child node.
		leaf_node = leaf_node->GetChildNode(tag, StyleSheetNode::TAG);
		tag_node = leaf_node;

		if (!id.Empty())
			leaf_node = leaf_node->GetChildNode(id, StyleSheetNode::ID);

		for (size_t j = 0; j < classes.size(); ++j)
			leaf_node = leaf_node->GetChildNode(classes[j], StyleSheetNode::CLASS);

		for (size_t j = 0; j < structural_pseudo_classes.size(); ++j)
			leaf_node = leaf_node->GetChildNode(structural_pseudo_classes[j], StyleSheetNode::STRUCTURAL_PSEUDO_CLASS);

		for (size_t j = 0; j < pseudo_classes.size(); ++j)
			leaf_node = leaf_node->GetChildNode(pseudo_classes[j], StyleSheetNode::PSEUDO_CLASS);
		
		(void)tag_node;
	}

	// Merge the new properties with those already on the leaf node.
	leaf_node->ImportProperties(properties, rule_specificity);

	return true;
}

bool StyleSheetParser::FindToken(String& buffer, const char* tokens, bool remove_token)
{
	buffer.Clear();
	char character;
	while (ReadCharacter(character))
	{
		if (strchr(tokens, character) != NULL)
		{
			if (remove_token)
				parse_buffer_pos++;
			return true;
		}
		else
		{
			buffer.Append(character);
			parse_buffer_pos++;
		}
	}

	return false;
}

// Attempts to find the next character in the active stream.
bool StyleSheetParser::ReadCharacter(char& buffer)
{
	bool comment = false;

	// Continuously fill the buffer until either we run out of
	// stream or we find the requested token
	do
	{
		while (parse_buffer_pos < parse_buffer.Length())
		{
			if (parse_buffer[parse_buffer_pos] == '\n')
				line_number++;
			else if (comment)
			{
				// Check for closing comment
				if (parse_buffer[parse_buffer_pos] == '*')
				{
					parse_buffer_pos++;
					if (parse_buffer_pos >= parse_buffer.Length())
					{
						if (!FillBuffer())
							return false;
					}

					if (parse_buffer[parse_buffer_pos] == '/')
						comment = false;
				}
			}
			else
			{
				// Check for an opening comment
				if (parse_buffer[parse_buffer_pos] == '/')
				{
					parse_buffer_pos++;
					if (parse_buffer_pos >= parse_buffer.Length())
					{
						if (!FillBuffer())
						{
							buffer = '/';
							parse_buffer = "/";
							return true;
						}
					}
					
					if (parse_buffer[parse_buffer_pos] == '*')
						comment = true;
					else
					{
						buffer = '/';
						if (parse_buffer_pos == 0)
							parse_buffer.Insert(parse_buffer_pos, '/');
						else
							parse_buffer_pos--;
						return true;
					}
				}

				if (!comment)
				{
					// If we find a character, return it
					buffer = parse_buffer[parse_buffer_pos];
					return true;
				}
			}

			parse_buffer_pos++;
		}
	}
	while (FillBuffer());

	return false;
}

// Fills the internal buffer with more content
bool StyleSheetParser::FillBuffer()
{
	// If theres no data to process, abort
	if (stream->IsEOS())
		return false;

	// Read in some data (4092 instead of 4096 to avoid the buffer growing when we have to add back
	// a character after a failed comment parse.)
	parse_buffer.Clear();
	bool read = stream->Read(parse_buffer, 4092) > 0;
	parse_buffer_pos = 0;

	return read;
}

}
}
