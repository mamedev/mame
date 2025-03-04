// license:BSD-3-Clause
// copyright-holders:David Broman
/***************************************************************************

    srcdbgdump.cpp

    MAME source-level debugging info dump utility

***************************************************************************/

#include "srcdbg_format_reader.h"
#include "srcdbg_api.h"

#include "corefile.h"

#include <cstdio>
#include <string>

// --------------------------------------------------------------------------
// Subclass of srcdbg_format_reader_callback to receive data as it is read
// --------------------------------------------------------------------------

class srcdbg_dump : public srcdbg_format_reader_callback
{
public:
	srcdbg_dump()
		: m_printed_source_file_paths_title(false)
		, m_printed_line_mapping_title(false)
		, m_printed_symbol_names_title(false)
		, m_printed_global_fixed_symbol_value_title(false)
		, m_printed_local_fixed_symbol_value_title(false)
		, m_printed_local_relative_symbol_value_title(false)
		{}
	virtual bool on_read_header_base(const mame_debug_info_header_base & header_base) override;
	virtual bool on_read_simp_header(const mame_debug_info_simple_header & simp_header) override;
	virtual bool on_read_source_path(u32 source_path_index, std::string && source_path) override;
	virtual bool on_read_line_mapping(const srcdbg_line_mapping & line_map) override;
	virtual bool on_read_symbol_name(u32 symbol_name_index, std::string && symbol_name) override;
	virtual bool on_read_global_fixed_symbol_value(const global_fixed_symbol_value & value) override;
	virtual bool on_read_local_fixed_symbol_value(const local_fixed_symbol_value & value) override;
	virtual bool on_read_local_relative_symbol_value(const local_relative_symbol_value & value) override;

private:
	bool m_printed_source_file_paths_title;
	bool m_printed_line_mapping_title;
	bool m_printed_symbol_names_title;
	bool m_printed_global_fixed_symbol_value_title;
	bool m_printed_local_fixed_symbol_value_title;
	bool m_printed_local_relative_symbol_value_title;
};


bool srcdbg_dump::on_read_header_base(const mame_debug_info_header_base & header_base)
{
	printf("magic:\t'%.4s'\n", header_base.magic);
	printf("type:\t'%.4s'\n", header_base.type);
	printf("version:\t%d\n", u32(header_base.version));
	return true;
}

bool srcdbg_dump::on_read_simp_header(const mame_debug_info_simple_header & simp_header)
{
	printf("\nReading simple format...\n\n");
	printf("source_file_paths_size:\t\t\t%u\n", simp_header.source_file_paths_size);
	printf("num_line_mappings:\t\t\t%u\n", simp_header.num_line_mappings);
	printf("symbol_names_size:\t\t\t%u\n", simp_header.symbol_names_size);
	printf("num_global_fixed_symbol_values:\t\t%u\n", simp_header.num_global_fixed_symbol_values);
	printf("local_fixed_symbol_values_size:\t\t%u\n", simp_header.local_fixed_symbol_values_size);
	printf("local_relative_symbol_values_size:\t%u\n", simp_header.local_relative_symbol_values_size);
	return true;
}

bool srcdbg_dump::on_read_source_path(u32 source_path_index, std::string && source_path)
{
	if (!m_printed_source_file_paths_title)
	{
		printf("\n**** Source file paths: ****\n");
		m_printed_source_file_paths_title = true;
	}
	printf("(%u)\t%s\n", u32(source_path_index), source_path.c_str());
	return true;
}

bool srcdbg_dump::on_read_line_mapping(const srcdbg_line_mapping & line_map)
{
	if (!m_printed_line_mapping_title)
	{
		printf("\n**** Line mappings: ****\n");
		m_printed_line_mapping_title = true;
	}
	printf("address_first: $%04X\taddress_last: $%04X\tsource_file_index: %u\tline_number: %u\n",
		u32(line_map.range.address_first),
		u32(line_map.range.address_last),
		u32(line_map.source_file_index),
		line_map.line_number);
	return true;
}

bool srcdbg_dump::on_read_symbol_name(u32 symbol_name_index, std::string && symbol_name)
{
	if (!m_printed_symbol_names_title)
	{
		printf("\n**** Symbol names: ****\n");
		m_printed_symbol_names_title = true;
	}
	printf("(%u)\t%s\n", u32(symbol_name_index), symbol_name.c_str());
	return true;
}

bool srcdbg_dump::on_read_global_fixed_symbol_value(const global_fixed_symbol_value & value)
{
	if (!m_printed_global_fixed_symbol_value_title)
	{
		printf("\n**** Global fixed symbol values: ****\n");
		m_printed_global_fixed_symbol_value_title = true;
	}
	printf("Symbol name index: %u, symbol value: $%08X, symbol flags: $%08X\n", value.symbol_name_index, value.symbol_value, value.symbol_flags);
	return true;
}

bool srcdbg_dump::on_read_local_fixed_symbol_value(const local_fixed_symbol_value & value)
{
	if (!m_printed_local_fixed_symbol_value_title)
	{
		printf("\n**** Local fixed symbol values: ****\n");
		m_printed_local_fixed_symbol_value_title = true;
	}
	printf("Symbol name index: %u, symbol value: $%08X\n", value.symbol_name_index, value.symbol_value);
	for (u32 i = 0; i < value.num_address_ranges; i++)
	{
		printf("\taddress range: $%04X-$%04X\n", value.ranges[i].address_first, value.ranges[i].address_last);
	}
	return true;
}

bool srcdbg_dump::on_read_local_relative_symbol_value(const local_relative_symbol_value & value)
{
	if (!m_printed_local_relative_symbol_value_title)
	{
		printf("\n**** Local relative symbol values: ****\n");
		m_printed_local_relative_symbol_value_title = true;
	}
	printf("Symbol name index: %u\n", value.symbol_name_index);
	for (u32 i = 0; i < value.num_local_relative_eval_rules; i++)
	{
		printf(
			"\tvalue: (reg idx %d) + %d\taddress range: $%04X-$%04X\n",
			value.local_relative_eval_rules[i].reg,
			value.local_relative_eval_rules[i].reg_offset,
			value.local_relative_eval_rules[i].range.address_first,
			value.local_relative_eval_rules[i].range.address_last);
	}
	return true;
}


// --------------------------------------------------------------------------
// main
// --------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage:\nsrcdbgdump <path to MAME source debugging information file>\n");
		return 1;
	}

	int srcdbg_lib_major, srcdbg_lib_minor;
	const char * mame_build;
	int ret = mame_srcdbg_get_version_info(&srcdbg_lib_major, &srcdbg_lib_minor, &mame_build);
	if (ret != MAME_SRCDBG_E_SUCCESS)
	{
		fprintf(stderr, "Failed to get mame_srcdbg library version information: Error code %d\n", ret);
		return 1;
	}

	printf("%s\n", mame_build);
	printf("Using mame_srcdbg library %d.%d\n", srcdbg_lib_major, srcdbg_lib_minor);
	printf("Dumping '%s'...\n", argv[1]);

	std::string error;
	srcdbg_format format;
	if (!srcdbg_format_header_read(argv[1], format, error))
	{
		fprintf(stderr, "Error reading source-level debugging information file\n%s\n\n%s", argv[1], error.c_str());
		return 1;
	}

	switch (format)
	{
	case SRCDBG_FORMAT_SIMPLE:
	{
		srcdbg_dump dumper;
		if (!srcdbg_format_simp_read(argv[1], dumper, error))
		{
			if (!error.empty())
			{
				fprintf(stderr, "Error reading source-level debugging information file\n%s\n\n%s", argv[1], error.c_str());
			}
			return 1;
		}
		return 0;
	}

	// FUTURE: If more file formats are invented, add cases for them here to read them

	default:
		assert(!"Unexpected source-level debugging information file format");
		return 1;
	}

	// return 0;
}
