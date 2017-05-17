// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    xmlfile.h

    XML file parsing code.

***************************************************************************/

#pragma once

#ifndef MAME_LIB_UTIL_XMLFILE_H
#define MAME_LIB_UTIL_XMLFILE_H

#include "osdcore.h"
#include "corefile.h"
#define PUGIXML_HEADER_ONLY
#include "pugixml/src/pugixml.hpp"

namespace util { namespace xml {

/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	PARSE_FLAG_WHITESPACE_SIGNIFICANT = 1
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

enum class int_format
{
	DECIMAL,
	DECIMAL_HASH,
	HEX_DOLLAR,
	HEX_C
};

class data_node : public pugi::xml_node
{
public:
	pugi::xml_attribute get_or_append_attribute(const pugi::char_t* name)
	{
		pugi::xml_attribute attr = attribute(name);
		return attr ? attr : append_attribute(name);
	}
	data_node() : pugi::xml_node()
	{
	}
	data_node(pugi::xml_node p) : pugi::xml_node(p.internal_object())
	{
	}
};

class document_node : public pugi::xml_document
{
	struct xml_corefile_writer : pugi::xml_writer
	{
		xml_corefile_writer(util::core_file &file) : savefile(file)
		{
		}

		virtual void write(const void* data, size_t size) override
		{
			savefile.write(data, size);
		}
		util::core_file &savefile;
	};

public:

	// write an XML tree to a file
	void file_write(util::core_file &file);

	void load_string(const pugi::char_t* contents, unsigned int options = 0);

	void load_file(util::core_file &file, unsigned int options = 0);

	const char* err_desc() { return res.description(); }
	int err_offset() { return res.offset; }

	document_node() : pugi::xml_document()
	{
	}

	document_node(const char *string) : pugi::xml_document()
	{
		load_string(string);
	}

	document_node(util::core_file &file) : pugi::xml_document()
	{
		load_file(file);
	}
private:
	pugi::xml_parse_result res;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- miscellaneous interfaces ----- */

/* normalize a string into something that can be written to an XML file */
const char *normalize_string(const char *string);
// return the format of the given integer attribute
int_format get_attribute_int_format(const char *name);

} } // namespace util::xml

#endif  /* MAME_LIB_UTIL_XMLFILE_H */
