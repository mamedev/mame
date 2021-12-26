// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem metadata management

#include "fsmeta.h"

#include "strformat.h"

const char *fs_meta_data::entry_name(fs_meta_name name)
{
	switch(name) {
	case fs_meta_name::basic: return "basic";
	case fs_meta_name::creation_date: return "creation_date";
	case fs_meta_name::length: return "length";
	case fs_meta_name::loading_address: return "loading_address";
	case fs_meta_name::locked: return "locked";
	case fs_meta_name::modification_date: return "modification_date";
	case fs_meta_name::name: return "name";
	case fs_meta_name::os_minimum_version: return "os_minimum_version";
	case fs_meta_name::os_version: return "os_version";
	case fs_meta_name::rsrc_length: return "rsrc_length";
	case fs_meta_name::sequential: return "sequential";
	case fs_meta_name::size_in_blocks: return "size_in_blocks";
	}
	return "";
}

std::string fs_meta::to_string(fs_meta_type type, const fs_meta &m)
{
	switch(type) {
	case fs_meta_type::string: return m.as_string();
	case fs_meta_type::number: return util::string_format("0x%x", m.as_number());
	case fs_meta_type::flag:   return m.as_flag() ? "t" : "f";
	case fs_meta_type::date:   {
		auto dt = m.as_date();
		return util::string_format("%04d-%02d-%02d %02d:%02d:%02d",
								   dt.year, dt.month, dt.day_of_month,
								   dt.hour, dt.minute, dt.second);
	}
	}
	return std::string("");
}
