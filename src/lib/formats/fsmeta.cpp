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

std::string meta_value::to_string() const
{
	std::string result;

	std::visit([this, &result](auto &&arg)
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, std::string>)
		{
			result = as_string();
		}
		else if constexpr (std::is_same_v<T, uint64_t>)
		{
			result = util::string_format("0x%x", as_number());
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			result = as_flag() ? "t" : "f";
		}
		else if constexpr (std::is_same_v<T, util::arbitrary_datetime>)
		{
			auto dt = as_date();
			result = util::string_format("%04d-%02d-%02d %02d:%02d:%02d",
				dt.year, dt.month, dt.day_of_month,
				dt.hour, dt.minute, dt.second);
		}
	}, value);
	return result;
}

} // namespace fs
