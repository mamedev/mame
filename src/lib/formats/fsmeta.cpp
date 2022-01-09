// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem metadata management

#include "fsmeta.h"

#include "strformat.h"

namespace fs {

const char *meta_data::entry_name(meta_name name)
{
	switch(name) {
	case meta_name::basic: return "basic";
	case meta_name::creation_date: return "creation_date";
	case meta_name::length: return "length";
	case meta_name::loading_address: return "loading_address";
	case meta_name::locked: return "locked";
	case meta_name::modification_date: return "modification_date";
	case meta_name::name: return "name";
	case meta_name::os_minimum_version: return "os_minimum_version";
	case meta_name::os_version: return "os_version";
	case meta_name::rsrc_length: return "rsrc_length";
	case meta_name::sequential: return "sequential";
	case meta_name::size_in_blocks: return "size_in_blocks";
	case meta_name::file_type: return "file_type";
	case meta_name::ascii_flag: return "ascii_flag";
	case meta_name::owner_id: return "owner_id";
	case meta_name::attributes: return "attributes";
	}
	return "";
}

std::string meta_value::to_string(meta_type type, const meta_value &m)
{
	switch(type) {
	case meta_type::string: return m.as_string();
	case meta_type::number: return util::string_format("0x%x", m.as_number());
	case meta_type::flag:   return m.as_flag() ? "t" : "f";
	case meta_type::date:   {
		auto dt = m.as_date();
		return util::string_format("%04d-%02d-%02d %02d:%02d:%02d",
								   dt.year, dt.month, dt.day_of_month,
								   dt.hour, dt.minute, dt.second);
	}
	}
	return std::string("");
}

} // namespace fs
