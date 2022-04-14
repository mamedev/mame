// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Filesystem metadata management

#include "fsmeta.h"

#include "strformat.h"

#include <optional>

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

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

meta_type meta_value::type() const
{
	std::optional<meta_type> result;
	std::visit(overloaded
	{
		[&result](const std::string &)				{ result = meta_type::string; },
		[&result](std::uint64_t)					{ result = meta_type::number; },
		[&result](bool)								{ result = meta_type::flag; },
		[&result](const util::arbitrary_datetime &)	{ result = meta_type::date; }
	}, value);
	return *result;
}

std::string meta_value::to_string() const
{
	std::string result;

	switch (type())
	{
	case meta_type::string:
		result = as_string();
		break;
	case meta_type::number:
		result = util::string_format("0x%x", as_number());
		break;
	case meta_type::flag:
		result = as_flag() ? "t" : "f";
		break;
	case meta_type::date:
		{
			auto dt = as_date();
			result = util::string_format("%04d-%02d-%02d %02d:%02d:%02d",
				dt.year, dt.month, dt.day_of_month,
				dt.hour, dt.minute, dt.second);
		}
		break;
	default:
		throw false;
	}
	return result;
}

} // namespace fs
