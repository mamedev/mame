// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    softlist.cpp

    Software list construction helpers.

***************************************************************************/

#include "emu.h"
#include "softlist.h"

#include "hash.h"

#include "expat.h"

#include <array>
#include <cstring>
#include <regex>



//**************************************************************************
//  STATIC VARIABLES
//**************************************************************************

static std::regex s_potential_softlist_regex("\\w+(\\:\\w+)*");


//**************************************************************************
//  FEATURE LIST ITEM
//**************************************************************************

//-------------------------------------------------
//  software_info_item - constructor
//-------------------------------------------------

software_info_item::software_info_item(const std::string &name, const std::string &value) :
	m_name(name),
	m_value(value)
{
}


//-------------------------------------------------
//  software_info_item - constructor
//-------------------------------------------------

software_info_item::software_info_item(std::string &&name, std::string &&value) :
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

software_part::software_part(software_info &info, std::string &&name, std::string &&interface) :
	m_info(info),
	m_name(std::move(name)),
	m_interface(std::move(interface))
{
}


//-------------------------------------------------
//  feature - return the value of the given
//  feature, if specified
//-------------------------------------------------

const char *software_part::feature(std::string_view feature_name) const noexcept
{
	// scan the feature list for an entry matching feature_name and return the value
	auto const iter = m_features.find(feature_name);
	return (iter != m_features.end()) ? iter->value().c_str() : nullptr;
}


//-------------------------------------------------
//  matches_interface - determine if we match
//  an interface in the provided list
//-------------------------------------------------

bool software_part::matches_interface(const char *interface_list) const noexcept
{
	// if we have no interface, then we match by default
	if (m_interface.empty())
		return true;

	// find our interface at the beginning of the list or immediately following a comma
	while (true)
	{
		char const *const found(std::strstr(interface_list, m_interface.c_str()));
		if (!found)
			return false;
		if (((found == interface_list) || (',' == found[-1])) && ((',' == found[m_interface.size()]) || !found[m_interface.size()]))
			return true;
		interface_list = std::strchr(interface_list, ',');
		if (!interface_list)
			return false;
		++interface_list;
	}
}


//**************************************************************************
//  SOFTWARE INFO
//**************************************************************************

//-------------------------------------------------
//  software_info - constructor
//-------------------------------------------------

software_info::software_info(std::string &&name, std::string &&parent, std::string_view supported) :
	m_supported(software_support::SUPPORTED),
	m_shortname(std::move(name)),
	m_parentname(std::move(parent))
{
	// handle the supported flag if provided
	if (supported == "partial")
		m_supported = software_support::PARTIALLY_SUPPORTED;
	else if (supported == "no")
		m_supported = software_support::UNSUPPORTED;
}


//-------------------------------------------------
//  find_part - find a part by name with an
//  optional interface match
//-------------------------------------------------

const software_part *software_info::find_part(std::string_view part_name, const char *interface) const
{
	// look for the part by name and match against the interface if provided
	auto iter = std::find_if(
		m_partdata.begin(),
		m_partdata.end(),
		[&part_name, interface] (const software_part &part)
		{
			// try to match the part_name (or all parts if part_name is empty), and then try
			// to match the interface (or all interfaces if interface is nullptr)
			return (part_name.empty() || part_name == part.name())
				&& (interface == nullptr || part.matches_interface(interface));
		});

	return iter != m_partdata.end()
		? &*iter
		: nullptr;
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


namespace detail {

//**************************************************************************
//  SOFTWARE LIST PARSER
//**************************************************************************

class softlist_parser
{
public:
	// construction (== execution)
	softlist_parser(
			util::read_stream &file,
			std::string_view filename,
			std::string &listname,
			std::string &description,
			std::list<software_info> &infolist,
			std::ostream &errors);

private:
	enum parse_position
	{
		POS_ROOT,
		POS_MAIN,
		POS_SOFT,
		POS_PART,
		POS_DATA
	};

	// internal parsing helpers
	const char *infoname() const { return m_current_info ? m_current_info->shortname().c_str() : "???"; }
	int line() const { return XML_GetCurrentLineNumber(m_parser); }
	int column() const { return XML_GetCurrentColumnNumber(m_parser); }
	const char *parser_error() const { return XML_ErrorString(XML_GetErrorCode(m_parser)); }

	// internal error helpers
	template <typename Format, typename... Params> void parse_error(Format &&fmt, Params &&... args);
	void unknown_tag(const char *tagname) { parse_error("Unknown tag: %s", tagname); }
	void unknown_attribute(const char *attrname) { parse_error("Unknown attribute: %s", attrname); }

	// internal helpers
	template <size_t N> std::array<std::string_view, N> parse_attributes(const char **attributes, const char *const (&attrlist)[N]);
	bool parse_name_and_value(const char **attributes, std::string_view &name, std::string_view &value);
	void add_rom_entry(std::string &&name, std::string &&hashdata, u32 offset, u32 length, u32 flags);

	// expat callbacks
	static void start_handler(void *data, const char *tagname, const char **attributes);
	static void data_handler(void *data, const char *s, int len);
	static void end_handler(void *data, const char *name);

	// internal parsing
	void parse_root_start(const char *tagname, const char **attributes);
	void parse_main_start(const char *tagname, const char **attributes);
	void parse_soft_start(const char *tagname, const char **attributes);
	void parse_part_start(const char *tagname, const char **attributes);
	void parse_data_start(const char *tagname, const char **attributes);
	void parse_main_end(const char *tagname);
	void parse_soft_end(const char *name);

	// internal parsing state
	const std::string_view      m_filename;
	std::list<software_info> &  m_infolist;
	std::ostream &              m_errors;
	struct XML_ParserStruct *   m_parser;
	std::string &               m_listname;
	std::string &               m_description;
	bool                        m_data_accum_expected;
	bool                        m_ignore_cdata;
	std::string                 m_data_accum;
	software_info *             m_current_info;
	software_part *             m_current_part;
	parse_position              m_pos;
};


//-------------------------------------------------
//  softlist_parser - constructor
//-------------------------------------------------

softlist_parser::softlist_parser(
		util::read_stream &file,
		std::string_view filename,
		std::string &listname,
		std::string &description,
		std::list<software_info> &infolist,
		std::ostream &errors) :
	m_filename(filename),
	m_infolist(infolist),
	m_errors(errors),
	m_listname(listname),
	m_description(description),
	m_data_accum_expected(false),
	m_ignore_cdata(false),
	m_current_info(nullptr),
	m_current_part(nullptr),
	m_pos(POS_ROOT)
{
	// create the parser
	m_parser = XML_ParserCreate_MM(nullptr, nullptr, nullptr);
	if (!m_parser)
		throw std::bad_alloc();

	// set the handlers
	XML_SetUserData(m_parser, this);
	XML_SetElementHandler(m_parser, &softlist_parser::start_handler, &softlist_parser::end_handler);
	XML_SetCharacterDataHandler(m_parser, &softlist_parser::data_handler);

	// parse the file contents
	char buffer[1024];
	for (bool done = false; !done; )
	{
		auto const [err, length] = read(file, buffer, sizeof(buffer)); // TODO: better error handling
		if (!length)
			done = true;
		if (XML_Parse(m_parser, buffer, length, done) == XML_STATUS_ERROR)
		{
			parse_error("%s", parser_error());
			break;
		}
	}

	// free the parser
	XML_ParserFree(m_parser);
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

template <size_t N>
std::array<std::string_view, N> softlist_parser::parse_attributes(const char **attributes, const char *const (&attrlist)[N])
{
	std::array<std::string_view, N> outlist;

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

bool softlist_parser::parse_name_and_value(const char **attributes, std::string_view &name, std::string_view &value)
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

void softlist_parser::add_rom_entry(std::string &&name, std::string &&hashdata, u32 offset, u32 length, u32 flags)
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
			state->parse_main_end(name);
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
	state->m_ignore_cdata = false;
	state->m_data_accum.clear();
}


//-------------------------------------------------
//  data_handler - expat data handler
//-------------------------------------------------

void softlist_parser::data_handler(void *data, const char *s, int len)
{
	softlist_parser *state = reinterpret_cast<softlist_parser *>(data);

	if (state->m_ignore_cdata)
	{
		// allowed, but we don't use it
	}
	else if (state->m_data_accum_expected)
	{
		// if we have an std::string to accumulate data in, do it
		state->m_data_accum.append(s, len);
	}
	else
	{
		// otherwise, report an error if the data is non-blank
		for (int i = 0; i < len; i++)
			if (!isspace(s[i]))
			{
				state->parse_error("Unexpected content");
				break;
			}
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
		static char const *const attrnames[] = { "name", "description" };
		const auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty())
			m_listname = attrvalues[0];

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
	if (strcmp(tagname, "software") == 0)
	{
		// <software name='' cloneof='' supported=''>
		static char const *const attrnames[] = { "name", "cloneof", "supported" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty())
		{
			m_infolist.emplace_back(std::string(attrvalues[0]), std::string(attrvalues[1]), attrvalues[2]);
			m_current_info = &m_infolist.back();
		}
		else
			parse_error("No name defined for item");
	}
	else if (strcmp(tagname, "notes") == 0)
	{
		// <notes>
		m_ignore_cdata = true;
	}
	else
		unknown_tag(tagname);
}


void softlist_parser::parse_main_end(const char *tagname)
{
}


//-------------------------------------------------
//  parse_soft_start - handle tag start within
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

	// <notes>
	else if (strcmp(tagname, "notes") == 0)
		m_ignore_cdata = true;

	// <info name='' value=''>
	else if (strcmp(tagname, "info") == 0)
	{
		std::string_view infoname, infovalue;

		if (parse_name_and_value(attributes, infoname, infovalue))
			m_current_info->m_info.emplace_back(std::string(infoname), std::string(infovalue));
		else
			parse_error("Incomplete other_info definition");
	}

	// <sharedfeat name='' value=''>
	else if (strcmp(tagname, "sharedfeat") == 0)
	{
		std::string_view featname, featvalue;

		if (parse_name_and_value(attributes, featname, featvalue))
		{
			if (!m_current_info->m_shared_features.emplace(std::string(featname), std::string(featvalue)).second)
				parse_error("Duplicate sharedfeat name");
		}
		else
		{
			parse_error("Incomplete sharedfeat definition");
		}
	}

	// <part name='' interface=''>
	else if (strcmp(tagname, "part" ) == 0)
	{
		static char const *const attrnames[] = { "name", "interface" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty() && !attrvalues[1].empty())
		{
			m_current_info->m_partdata.emplace_back(*m_current_info, std::string(attrvalues[0]), std::string(attrvalues[1]));
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
		static char const *const attrnames[] = { "name", "size", "width", "endianness" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty() && !attrvalues[1].empty())
		{
			// handle region attributes
			const auto &width = attrvalues[2];
			const auto &endianness = attrvalues[3];
			u32 regionflags = ROMENTRYTYPE_REGION;

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

			add_rom_entry(std::string(attrvalues[0]), "", 0, strtol(attrvalues[1].data(), nullptr, 0), regionflags);
		}
		else
			parse_error("Incomplete dataarea definition");
	}

	// <diskarea name=''>
	else if (strcmp(tagname, "diskarea") == 0)
	{
		static char const *const attrnames[] = { "name" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		if (!attrvalues[0].empty())
			add_rom_entry(std::string(attrvalues[0]), "", 0, 1, ROMENTRYTYPE_REGION | ROMREGION_DATATYPEDISK);
		else
			parse_error("Incomplete diskarea definition");
	}

	// <feature name='' value=''>
	else if (strcmp(tagname, "feature") == 0)
	{
		std::string_view featname, featvalue;

		if (parse_name_and_value(attributes, featname, featvalue))
		{
			if (!m_current_part->m_features.emplace(std::string(featname), std::string(featvalue)).second)
				parse_error("Duplicate feature name");
		}
		else
		{
			parse_error("Incomplete feature definition");
		}
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
		static char const *const attrnames[] = { "name", "size", "crc", "sha1", "offset", "value", "status", "loadflag" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		const std::string_view &name = attrvalues[0];
		const std::string_view &sizestr = attrvalues[1];
		const std::string_view &crc = attrvalues[2];
		const std::string_view &sha1 = attrvalues[3];
		const std::string_view &offsetstr = attrvalues[4];
		const std::string_view &value = attrvalues[5];
		const std::string_view &status = attrvalues[6];
		const std::string_view &loadflag = attrvalues[7];
		if (!sizestr.empty())
		{
			u32 length = strtol(sizestr.data(), nullptr, 0);
			u32 offset = offsetstr.empty() ? 0 : strtol(offsetstr.data(), nullptr, 0);

			if (loadflag == "reload")
				add_rom_entry("", "", offset, length, ROMENTRYTYPE_RELOAD | ROM_INHERITFLAGS);
			else if (loadflag == "reload_plain")
				add_rom_entry("", "", offset, length, ROMENTRYTYPE_RELOAD);
			else if (loadflag == "continue")
				add_rom_entry("", "", offset, length, ROMENTRYTYPE_CONTINUE | ROM_INHERITFLAGS);
			else if (loadflag == "fill")
				add_rom_entry("", std::string(value), offset, length, ROMENTRYTYPE_FILL);
			else if (loadflag == "ignore")
				add_rom_entry("", "", 0, length, ROMENTRYTYPE_IGNORE | ROM_INHERITFLAGS);
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

				add_rom_entry(std::string(name), std::move(hashdata), offset, length, ROMENTRYTYPE_ROM | romflags);
			}
			else
				parse_error("Rom name missing");
		}
		else
			parse_error("Incomplete rom definition");
	}

	// <rom name='' sha1='' status='' writeable=''>
	else if (strcmp(tagname, "disk") == 0)
	{
		static char const *const attrnames[] = { "name", "sha1", "status", "writeable" };
		auto attrvalues = parse_attributes(attributes, attrnames);

		const std::string_view &name = attrvalues[0];
		const std::string_view &sha1 = attrvalues[1];
		const std::string_view &status = attrvalues[2];
		const std::string_view &writeablestr = attrvalues[3];
		if (!name.empty() && !sha1.empty())
		{
			const bool baddump = (status == "baddump");
			const bool nodump = (status == "nodump" );
			const bool writeable = (writeablestr == "yes");
			std::string hashdata = string_format("%c%s%s", util::hash_collection::HASH_SHA1, sha1, (nodump ? NO_DUMP : (baddump ? BAD_DUMP : "")));

			add_rom_entry(std::string(name), std::move(hashdata), 0, 0, ROMENTRYTYPE_ROM | (writeable ? DISK_READWRITE : DISK_READONLY));
		}
		else if (status != "nodump") // a no_dump chd is not an incomplete entry
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
		m_current_info->m_longname = std::move(m_data_accum);

	// <year>
	else if (strcmp(tagname, "year") == 0)
		m_current_info->m_year = std::move(m_data_accum);

	// <publisher>
	else if (strcmp(tagname, "publisher") == 0)
		m_current_info->m_publisher = std::move(m_data_accum);

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

		// get the info; if present, copy shared data
		if (m_current_info != nullptr)
			for (const software_info_item &item : m_current_info->shared_features())
				m_current_part->m_features.emplace(item.name(), item.value());
	}
}

} // namespace detail


void parse_software_list(
		util::read_stream &file,
		std::string_view filename,
		std::string &listname,
		std::string &description,
		std::list<software_info> &infolist,
		std::ostream &errors)
{
	detail::softlist_parser(file, filename, listname, description, infolist, errors);
}


//-------------------------------------------------
//  software_name_parse - helper that splits a
//  software identifier (software_list:software:part)
//  string into separate software_list, software, and part
//  strings.
//
//  str1:str2:str3  => swlist_name - str1, swname - str2, swpart - str3
//  str1:str2       => swlist_name - nullptr, swname - str1, swpart - str2
//  str1            => swlist_name - nullptr, swname - str1, swpart - nullptr
//
//  Notice however that we could also have been
//  passed a string swlist_name:swname, and thus
//  some special check has to be performed in this
//  case.
//-------------------------------------------------

bool software_name_parse(std::string_view identifier, std::string *list_name, std::string *software_name, std::string *part_name)
{
	// first, sanity check the arguments
	if (!std::regex_match(identifier.begin(), identifier.end(), s_potential_softlist_regex))
		return false;

	// reset all output parameters (if specified of course)
	if (list_name != nullptr)
		list_name->clear();
	if (software_name != nullptr)
		software_name->clear();
	if (part_name != nullptr)
		part_name->clear();

	// if no colon, this is the swname by itself
	auto split1 = identifier.find_first_of(':');
	if (split1 == std::string_view::npos)
	{
		if (software_name != nullptr)
			*software_name = identifier;
		return true;
	}

	// if one colon, it is the swname and swpart alone
	auto split2 = identifier.find_first_of(':', split1 + 1);
	if (split2 == std::string_view::npos)
	{
		if (software_name != nullptr)
			*software_name = identifier.substr(0, split1);
		if (part_name != nullptr)
			*part_name = identifier.substr(split1 + 1);
		return true;
	}

	// if two colons present, split into 3 parts
	if (list_name != nullptr)
		*list_name = identifier.substr(0, split1);
	if (software_name != nullptr)
		*software_name = identifier.substr(split1 + 1, split2 - (split1 + 1));
	if (part_name != nullptr)
		*part_name = identifier.substr(split2 + 1);
	return true;
}
