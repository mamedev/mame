// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    softlist.cpp

    Software list construction helpers.

***************************************************************************/

#include "softlist.h"
#include "hash.h"
#include "expat.h"

//**************************************************************************
//  FEATURE LIST ITEM
//**************************************************************************

//-------------------------------------------------
//  feature_list_item - constructor
//-------------------------------------------------

feature_list_item::feature_list_item(const std::string &name, const std::string &value)
	: m_next(nullptr),
	m_name(name),
	m_value(value)
{
}


//-------------------------------------------------
//  feature_list_item - constructor
//-------------------------------------------------

feature_list_item::feature_list_item(std::string &&name, std::string &&value)
	: m_next(nullptr),
	m_name(std::move(name)),
	m_value(std::move(value))
{
}


//**************************************************************************
//  SOFTWARE PART
//**************************************************************************

//-------------------------------------------------
//  software_part - constructor
//-------------------------------------------------

software_part::software_part(software_info &info, std::string &&name, std::string &&interface)
	: m_next(nullptr),
		m_info(info),
		m_name(std::move(name)),
		m_interface(std::move(interface))
{
}


//-------------------------------------------------
//  feature - return the value of the given
//  feature, if specified
//-------------------------------------------------

const char *software_part::feature(const std::string &feature_name) const
{
	// scan the feature list for an entry matching feature_name and return the value
	for (const feature_list_item &feature : m_featurelist)
		if (feature.name() == feature_name)
			return feature.value().c_str();
	return nullptr;

}


//-------------------------------------------------
//  matches_interface - determine if we match
//  an interface in the provided list
//-------------------------------------------------

bool software_part::matches_interface(const char *interface_list) const
{
	// if we have no interface, then we match by default
	if (m_interface.empty())
		return true;

	// copy the comma-delimited interface list and ensure it ends with a final comma
	std::string interfaces = std::string(interface_list).append(",");

	// then add a comma to the end of our interface and return true if we find it in the list string
	std::string our_interface = std::string(m_interface).append(",");
	return (interfaces.find(our_interface) != -1);
}


//**************************************************************************
//  SOFTWARE INFO
//**************************************************************************

//-------------------------------------------------
//  software_info - constructor
//-------------------------------------------------

software_info::software_info(std::string &&name, std::string &&parent, const std::string &supported)
	: m_supported(SOFTWARE_SUPPORTED_YES),
		m_shortname(std::move(name)),
		m_parentname(std::move(parent))
{
	// handle the supported flag if provided
	if (supported == "partial")
		m_supported = SOFTWARE_SUPPORTED_PARTIAL;
	else if (supported == "no")
		m_supported = SOFTWARE_SUPPORTED_NO;
}


//-------------------------------------------------
//  find_part - find a part by name with an
//  optional interface match
//-------------------------------------------------

const software_part *software_info::find_part(const std::string &partname, const char *interface) const
{
	// if neither partname nor interface supplied, then we just return the first entry
	if (partname.empty() && interface == nullptr)
		return &m_partdata.front();

	// look for the part by name and match against the interface if provided
	for (const software_part &part : m_partdata)
		if (!partname.empty() && (partname == part.name()))
		{
			if (interface == nullptr || part.matches_interface(interface))
				return &part;
		}
		else if (partname.empty() && part.matches_interface(interface))
				return &part;
	return nullptr;
}


//-------------------------------------------------
//  has_multiple_parts - return true if we have
//  more than one part matching the given
//  interface
//-------------------------------------------------

bool software_info::has_multiple_parts(const char *interface) const
{
	int count = 0;

	// increment the count for each match and stop if we hit more than 1
	for (const software_part &part : m_partdata)
		if (part.matches_interface(interface))
			if (++count > 1)
				return true;

	return false;
}


//**************************************************************************
//  SOFTWARE LIST PARSER
//**************************************************************************

//-------------------------------------------------
//  softlist_parser - constructor
//-------------------------------------------------

softlist_parser::softlist_parser(util::core_file &file, const std::string &filename, std::string &description, std::list<software_info> &infolist, std::ostringstream &errors)
		: m_file(file),
		m_filename(filename),
		m_infolist(infolist),
		m_errors(errors),
		m_done(false),
		m_description(description),
		m_data_accum_expected(false),
		m_current_info(nullptr),
		m_current_part(nullptr),
		m_pos(POS_ROOT)
{
	// create the parser
	m_parser = XML_ParserCreate_MM(nullptr, nullptr, nullptr);
	if (m_parser == nullptr)
		throw std::bad_alloc();

	// set the handlers
	XML_SetUserData(m_parser, this);
	XML_SetElementHandler(m_parser, &softlist_parser::start_handler, &softlist_parser::end_handler);
	XML_SetCharacterDataHandler(m_parser, &softlist_parser::data_handler);

	// parse the file contents
	m_file.seek(0, SEEK_SET);
	char buffer[1024];
	while (!m_done)
	{
		UINT32 length = m_file.read(buffer, sizeof(buffer));
		m_done = m_file.eof();
		if (XML_Parse(m_parser, buffer, length, m_done) == XML_STATUS_ERROR)
		{
			parse_error("%s", parser_error());
			break;
		}
	}

	// free the parser
	XML_ParserFree(m_parser);
}


//-------------------------------------------------
//	line
//-------------------------------------------------

int softlist_parser::line() const
{
	return XML_GetCurrentLineNumber(m_parser);
}


//-------------------------------------------------
//	column
//-------------------------------------------------

int softlist_parser::column() const
{
	return XML_GetCurrentColumnNumber(m_parser);
}


//-------------------------------------------------
//	parser_error
//-------------------------------------------------

const char *softlist_parser::parser_error() const
{
	return XML_ErrorString(XML_GetErrorCode(m_parser));
}


//-------------------------------------------------
//  parse_error - append a parsing error with
//  filename, line and column information
//-------------------------------------------------

template <typename Format, typename... Params>
inline void softlist_parser::parse_error(Format &&fmt, Params &&... args)
{
	// always start with filename(line.column):
	util::stream_format(m_errors, "%s(%d.%d): ", m_filename, line(), column());

	// append the remainder of the string
	util::stream_format(m_errors, std::forward<Format>(fmt), std::forward<Params>(args)...);

	// append a newline at the end
	m_errors.put('\n');
}


//-------------------------------------------------
//  parse_attributes - helper to parse a set of
//  attributes into a list of strings
//-------------------------------------------------

template <typename T>
std::vector<std::string> softlist_parser::parse_attributes(const char **attributes, const T &attrlist)
{
	std::vector<std::string> outlist(std::distance(std::begin(attrlist), std::end(attrlist)));

	// iterate over attribute/value pairs
	for( ; attributes[0]; attributes += 2)
	{
		auto iter = std::begin(attrlist);

		// look for a match among the attributes provided
		for (std::size_t index = 0; iter != std::end(attrlist); ++index, ++iter)
		{
			if (strcmp(attributes[0], *iter) == 0)
			{
				// if found, set the corresponding output entry to the value
				outlist[index] = attributes[1];
				break;
			}
		}

		// if not found, report an unknown attribute
		if (iter == std::end(attrlist))
			unknown_attribute(attributes[0]);
	}

	return outlist;
}


//-------------------------------------------------
//  parse_name_and_value - helper to parse "name"
//  and "value" attribute pairs (allowing the
//  latter to be defined as an empty string)
//-------------------------------------------------

bool softlist_parser::parse_name_and_value(const char **attributes, std::string &name, std::string &value)
{
	bool found_value = false;

	// iterate over attribute/value pairs
	for( ; attributes[0]; attributes += 2)
	{
		// if found, set the corresponding output entry to the value
		if (strcmp(attributes[0], "name") == 0)
		{
			name = attributes[1];
		}

		else if (strcmp(attributes[0], "value") == 0)
		{
			value = attributes[1];
			found_value = true;
		}

		// if not found, report an unknown attribute
		else
			unknown_attribute(attributes[0]);
	}

	return !name.empty() && found_value;
}


//-------------------------------------------------
//  add_rom_entry - append a new ROM entry to the
//  current part's list
//-------------------------------------------------

void softlist_parser::add_rom_entry(std::string &&name, std::string &&hashdata, UINT32 offset, UINT32 length, UINT32 flags)
{
	// get the current part
	if (m_current_part == nullptr)
	{
		parse_error("ROM entry added in invalid context");
		return;
	}

	// make sure we don't add duplicate regions
	if (!name.empty() && (flags & ROMENTRY_TYPEMASK) == ROMENTRYTYPE_REGION)
	{
		for (auto &elem : m_current_part->m_romdata)
			if (elem.name() == name)
				parse_error("Duplicated dataarea %s in software %s", name, infoname());
	}

	m_current_part->m_romdata.emplace_back(std::move(name), std::move(hashdata), offset, length, flags);
}


//-------------------------------------------------
//  start_handler - expat handler for tag start
//-------------------------------------------------

void softlist_parser::start_handler(void *data, const char *tagname, const char **attributes)
{
	// switch off the current state
	softlist_parser *state = reinterpret_cast<softlist_parser *>(data);
	switch (state->m_pos)
	{
		case POS_ROOT:
			state->parse_root_start(tagname, attributes);
			break;

		case POS_MAIN:
			state->parse_main_start(tagname, attributes);
			break;

		case POS_SOFT:
			state->parse_soft_start(tagname, attributes);
			break;

		case POS_PART:
			state->parse_part_start(tagname, attributes);
			break;

		case POS_DATA:
			state->parse_data_start(tagname, attributes);
			break;
	}

	// increment the state since this is a tag start
	state->m_pos = parse_position(state->m_pos + 1);
}


//-------------------------------------------------
//  end_handler - handle end-of-tag post-processing
//-------------------------------------------------

void softlist_parser::end_handler(void *data, const char *name)
{
	// reset the text destination and bump the position down
	softlist_parser *state = reinterpret_cast<softlist_parser *>(data);
	state->m_pos = parse_position(state->m_pos - 1);

	// switch off of the new position
	switch (state->m_pos)
	{
		case POS_ROOT:
			break;

		case POS_MAIN:
			state->m_current_info = nullptr;
			break;

		case POS_SOFT:
			state->parse_soft_end(name);
			state->m_current_part = nullptr;
			break;

		case POS_PART:
			break;

		case POS_DATA:
			break;
	}

	// stop accumulating
	state->m_data_accum_expected = false;
	state->m_data_accum.clear();
}


//-------------------------------------------------
//  data_handler - expat data handler
//-------------------------------------------------

void softlist_parser::data_handler(void *data, const char *s, int len)
{
	softlist_parser *state = reinterpret_cast<softlist_parser *>(data);

	// if we have an std::string to accumulate data in, do it
	if (state->m_data_accum_expected)
		state->m_data_accum.append(s, len);

	// otherwise, report an error if the data is non-blank
	else
		for (int i = 0; i < len; i++)
			if (!isspace(s[i]))
			{
				state->parse_error("Unexpected content");
				break;
			}
}


//-------------------------------------------------
//  parse_root_start - handle tag start at the root
//-------------------------------------------------

void softlist_parser::parse_root_start(const char *tagname, const char **attributes)
{
	// <softwarelist name='' description=''>
	if (strcmp(tagname, "softwarelist") == 0)
	{
		static const char *attrnames[] = { "name", "description" };
		const auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[1].empty())
			m_description = attrvalues[1];
	}
	else
		unknown_tag(tagname);
}


//-------------------------------------------------
//  parse_main_start - handle tag start within
//  a softwarelist tag
//-------------------------------------------------

void softlist_parser::parse_main_start(const char *tagname, const char **attributes)
{
	// <software name='' cloneof='' supported=''>
	if (strcmp(tagname, "software") == 0)
	{
		static const char *attrnames[] = { "name", "cloneof", "supported" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty())
		{
			m_infolist.emplace_back(std::move(attrvalues[0]), std::move(attrvalues[1]), attrvalues[2].c_str());
			m_current_info = &m_infolist.back();
		}
		else
			parse_error("No name defined for item");
	}
	else
		unknown_tag(tagname);
}


//-------------------------------------------------
//  parse_main_start - handle tag start within
//  a software tag
//-------------------------------------------------

void softlist_parser::parse_soft_start(const char *tagname, const char **attributes)
{
	// get the current info; error if none
	if (m_current_info == nullptr)
	{
		parse_error("Tag %s found outside of software context", tagname);
		return;
	}

	// <description>
	if (strcmp(tagname, "description") == 0)
		m_data_accum_expected = true;

	// <year>
	else if (strcmp(tagname, "year") == 0)
		m_data_accum_expected = true;

	// <publisher>
	else if (strcmp(tagname, "publisher") == 0)
		m_data_accum_expected = true;

	// <info name='' value=''>
	else if (strcmp(tagname, "info") == 0)
	{
		std::string infoname, infovalue;

		if (parse_name_and_value(attributes, infoname, infovalue))
			m_current_info->m_other_info.emplace_back(std::move(infoname), std::move(infovalue));
		else
			parse_error("Incomplete other_info definition");
	}

	// <sharedfeat name='' value=''>
	else if (strcmp(tagname, "sharedfeat") == 0)
	{
		std::string featname, featvalue;

		if (parse_name_and_value(attributes, featname, featvalue))
			m_current_info->m_shared_info.emplace_back(std::move(featname), std::move(featvalue));
		else
			parse_error("Incomplete sharedfeat definition");
	}

	// <part name='' interface=''>
	else if (strcmp(tagname, "part" ) == 0)
	{
		static const char *attrnames[] = { "name", "interface" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty() && !attrvalues[1].empty())
		{
			m_current_info->m_partdata.emplace_back(*m_current_info, std::move(attrvalues[0]), std::move(attrvalues[1]));
			m_current_part = &m_current_info->m_partdata.back();
		}
		else
			parse_error("Incomplete part definition");
	}
	else
		unknown_tag(tagname);
}


//-------------------------------------------------
//  parse_part_start - handle tag start within
//  a part tag
//-------------------------------------------------

void softlist_parser::parse_part_start(const char *tagname, const char **attributes)
{
	// get the current part; error if none
	if (m_current_part == nullptr)
	{
		parse_error("Tag %s found outside of part context", tagname);
		return;
	}

	// <dataarea name='' size=''>
	if (strcmp(tagname, "dataarea") == 0)
	{
		static const char *attrnames[] = { "name", "size", "width", "endianness" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty() && !attrvalues[1].empty())
		{
			// handle region attributes
			const std::string &width = attrvalues[2];
			const std::string &endianness = attrvalues[3];
			UINT32 regionflags = ROMENTRYTYPE_REGION;

			if (!width.empty())
			{
				if (width == "8")
					regionflags |= ROMREGION_8BIT;
				else if (width == "16")
					regionflags |= ROMREGION_16BIT;
				else if (width == "32")
					regionflags |= ROMREGION_32BIT;
				else if (width == "64")
					regionflags |= ROMREGION_64BIT;
				else
					parse_error("Invalid dataarea width");
			}
			if (!endianness.empty())
			{
				if (endianness == "little")
					regionflags |= ROMREGION_LE;
				else if (endianness == "big")
					regionflags |= ROMREGION_BE;
				else
					parse_error("Invalid dataarea endianness");
			}

			add_rom_entry(std::move(attrvalues[0]), "", 0, strtol(attrvalues[1].c_str(), nullptr, 0), regionflags);
		}
		else
			parse_error("Incomplete dataarea definition");
	}

	// <diskarea name=''>
	else if (strcmp(tagname, "diskarea") == 0)
	{
		static const char *attrnames[] = { "name" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty())
			add_rom_entry(std::move(attrvalues[0]), "", 0, 1, ROMENTRYTYPE_REGION | ROMREGION_DATATYPEDISK);
		else
			parse_error("Incomplete diskarea definition");
	}

	// <feature name='' value=''>
	else if (strcmp(tagname, "feature") == 0)
	{
		std::string featname, featvalue;

		if (parse_name_and_value(attributes, featname, featvalue))
			m_current_part->m_featurelist.emplace_back(std::move(featname), std::move(featvalue));
		else
			parse_error("Incomplete feature definition");
	}

	// <dipswitch>
	else if (strcmp(tagname, "dipswitch") == 0)
		;
	else
		unknown_tag(tagname);
}


//-------------------------------------------------
//  parse_data_start - handle tag start within a
//  dataarea or diskarea tag
//-------------------------------------------------

void softlist_parser::parse_data_start(const char *tagname, const char **attributes)
{
	// get the current part; error if none
	if (m_current_part == nullptr)
	{
		parse_error("Tag %s found outside of part context", tagname);
		return;
	}

	// <rom name='' size='' crc='' sha1='' offset='' value='' status='' loadflag=''>
	if (strcmp(tagname, "rom") == 0)
	{
		static const char *attrnames[] = { "name", "size", "crc", "sha1", "offset", "value", "status", "loadflag" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		std::string &name = attrvalues[0];
		const std::string &sizestr = attrvalues[1];
		const std::string &crc = attrvalues[2];
		const std::string &sha1 = attrvalues[3];
		const std::string &offsetstr = attrvalues[4];
		std::string &value = attrvalues[5];
		const std::string &status = attrvalues[6];
		const std::string &loadflag = attrvalues[7];
		if (!sizestr.empty() && !offsetstr.empty())
		{
			UINT32 length = strtol(sizestr.c_str(), nullptr, 0);
			UINT32 offset = strtol(offsetstr.c_str(), nullptr, 0);

			if (loadflag == "reload")
				add_rom_entry("", "", offset, length, ROMENTRYTYPE_RELOAD | ROM_INHERITFLAGS);
			else if (loadflag == "reload_plain")
				add_rom_entry("", "", offset, length, ROMENTRYTYPE_RELOAD);
			else if (loadflag == "continue")
				add_rom_entry("", "", offset, length, ROMENTRYTYPE_CONTINUE | ROM_INHERITFLAGS);
			else if (loadflag == "fill")
				add_rom_entry("", std::move(value), offset, length, ROMENTRYTYPE_FILL);
			else if (!name.empty())
			{
				bool baddump = (status == "baddump");
				bool nodump = (status == "nodump");

				std::string hashdata;
				if (nodump)
				{
					hashdata = string_format("%s", NO_DUMP);
					if (!crc.empty() && !sha1.empty())
						parse_error("No need for hash definition");
				}
				else
				{
					if (!crc.empty() && !sha1.empty())
						hashdata = string_format("%c%s%c%s%s", util::hash_collection::HASH_CRC, crc, util::hash_collection::HASH_SHA1, sha1, (baddump ? BAD_DUMP : ""));
					else
						parse_error("Incomplete rom hash definition");
				}

				// Handle loadflag attribute
				int romflags = 0;
				if (loadflag == "load16_word_swap")
					romflags = ROM_GROUPWORD | ROM_REVERSE;
				else if (loadflag == "load16_byte")
					romflags = ROM_SKIP(1);
				else if (loadflag == "load32_word_swap")
					romflags = ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2);
				else if (loadflag == "load32_word")
					romflags = ROM_GROUPWORD | ROM_SKIP(2);
				else if (loadflag == "load32_byte")
					romflags = ROM_SKIP(3);

				add_rom_entry(std::move(name), std::move(hashdata), offset, length, ROMENTRYTYPE_ROM | romflags);
			}
			else
				parse_error("Rom name missing");
		}
		else if (!sizestr.empty() && !loadflag.empty() && loadflag == "ignore")
		{
			UINT32 length = strtol(sizestr.c_str(), nullptr, 0);
			add_rom_entry("", "", 0, length, ROMENTRYTYPE_IGNORE | ROM_INHERITFLAGS);
		}
		else
			parse_error("Incomplete rom definition");
	}

	// <rom name='' sha1='' status='' writeable=''>
	else if (strcmp(tagname, "disk") == 0)
	{
		static const char *attrnames[] = { "name", "sha1", "status", "writeable" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		std::string &name = attrvalues[0];
		const std::string &sha1 = attrvalues[1];
		const std::string &status = attrvalues[2];
		const std::string &writeablestr = attrvalues[3];
		if (!name.empty() && !sha1.empty())
		{
			const bool baddump = (status == "baddump");
			const bool nodump = (status == "nodump" );
			const bool writeable = (writeablestr == "yes");
			std::string hashdata = string_format("%c%s%s", util::hash_collection::HASH_SHA1, sha1, (nodump ? NO_DUMP : (baddump ? BAD_DUMP : "")));

			add_rom_entry(std::move(name), std::move(hashdata), 0, 0, ROMENTRYTYPE_ROM | (writeable ? DISK_READWRITE : DISK_READONLY));
		}
		else if (status.empty() || (status == "nodump")) // a no_dump chd is not an incomplete entry
			parse_error("Incomplete disk definition");
	}

	// <dipvalue>
	else if (strcmp(tagname, "dipvalue") == 0)
		;
	else
		unknown_tag(tagname);
}


//-------------------------------------------------
//  parse_soft_end - handle end-of-tag post-
//  processing within the <software> tag
//-------------------------------------------------

void softlist_parser::parse_soft_end(const char *tagname)
{
	assert(m_current_info != nullptr);

	// <description>
	if (strcmp(tagname, "description") == 0)
		m_current_info->m_longname = m_data_accum;

	// <year>
	else if (strcmp(tagname, "year") == 0)
		m_current_info->m_year = m_data_accum;

	// <publisher>
	else if (strcmp(tagname, "publisher") == 0)
		m_current_info->m_publisher = m_data_accum;

	// </part>
	else if (strcmp(tagname, "part") == 0)
	{
		// get the last part
		assert(m_current_part != nullptr);
		if (m_current_part == nullptr)
			return;

		// was any dataarea/rom information encountered? if so, add a terminator
		if (!m_current_part->m_romdata.empty())
			add_rom_entry("", "", 0, 0, ROMENTRYTYPE_END);

		// get the info; if present, copy shared data (we assume name/value strings live
		// in the string pool and don't need to be reallocated)
		if (m_current_info != nullptr)
			for (const feature_list_item &item : m_current_info->shared_info())
				m_current_part->m_featurelist.emplace_back(item.name(), item.value());
	}
}

