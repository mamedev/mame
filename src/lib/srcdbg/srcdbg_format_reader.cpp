// license:BSD-3-Clause
// copyright-holders:David Broman
/*********************************************************************

    srcdbg_format_reader.cpp

    Internal implementation of portions of srcdbg_api.h.
    Helper for reading MAME source-level debugging info files

    WARNING: Tools external to MAME should only use functionality
    declared in srcdg_format.h and srcdbg_api.h.

***************************************************************************/


#include "srcdbg_format_reader.h"
#include "srcdbg_util.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#include <vector>
#include <system_error>


// Helper to read some bytes and advance the index.
// Returns true iff read succeeds
template <typename T>
static bool read_field(
	T * & var,                      // [out] Next value to read
	const std::vector<u8> & data,   // [in] Reference to beginning of data
	u32 & i,                        // [in/out] Current index into data to read from
	std::string & error)            // [out] Error text if there was a problem reading
{
	if (data.size() < i + sizeof(T))
	{
		srcdbg_sprintf(error, "File ended prematurely while reading next field of size %d", sizeof(T));
		return false;
	}

	var = (T *)(&data[i]);
	i += sizeof(T);
	return true;
}


// Reads the source-level debugging information file format and base
// header fields.
// [in] srcdbg_path - Path to source-level debugging information file
// [out] format - On success, the format reported by the header
// [out] error - On failure, explanatory error text
// Returns true on success, false on failure.
bool srcdbg_format_header_read(const char * srcdbg_path, srcdbg_format & format, std::string & error)
{
	FILE * file = fopen(srcdbg_path, "rb");
	if (file == nullptr)
	{
		srcdbg_sprintf(error, "Error opening file\n%s\n\nError code: %d", srcdbg_path, errno);
		return false;
	}

	mame_debug_info_header_base header;
	if (fread(&header, sizeof(header), 1, file) != 1)
	{
		if (feof(file))
		{
			srcdbg_sprintf(error, "Error reading from file\n%s\n\nFile too small to contain header", srcdbg_path);
			fclose(file);
			return false;
		}
		srcdbg_sprintf(error, "Error reading from file\n%s\n\nError code: %d", srcdbg_path, errno);
		fclose(file);
		return false;
	}

	fclose(file);
	file = nullptr;

	if (strncmp(header.type, "simp", 4) != 0)
	{
		srcdbg_sprintf(error, "Error while reading header from file\n%s\n\nOnly type 'simp' is currently supported", srcdbg_path);
		return false;
	};

	if (header.version != 1)
	{
		srcdbg_sprintf(error, "Error while reading header from file\n%s\n\nOnly version 1 is currently supported", srcdbg_path);
		return false;
	};

	format = SRCDBG_FORMAT_SIMPLE;

	// FUTURE: If new formats are created (for emulated machines where "simp" is unsuitable),
	// add code here to read the format identifier and return the appropriate format enum

	return true;
}


// Reads the source-level debugging information file already
// determined to be in the "simple" format
// [in] srcdbg_path - Path to source-level debugging information file
// [in] callback - An implementation of a subclass of srcdbg_format_reader_callback
//      to receive callbacks while reading the file
// [in/out] error - On success, untouched
//                  If a caller-supplied callback returns false, untouched
//                  On failure, explanatory error text
// Returns true on success, false on failure or if a caller-supplied callback returns false
bool srcdbg_format_simp_read(const char * srcdbg_path, srcdbg_format_reader_callback & callback, std::string & error)
{
	// ** Load full binary contents into memory **

	std::vector<u8> data;
	FILE * file = fopen(srcdbg_path, "rb");
	if (file == nullptr)
	{
		srcdbg_sprintf(error, "Error opening file\n%s\n\nError code: %d", srcdbg_path, errno);
		return false;
	}

	s32 fgetcRet;
	while ((fgetcRet = fgetc(file)) != EOF)
	{
		data.push_back((u8) fgetcRet);
	}

	if (ferror(file))
	{
		srcdbg_sprintf(error, "Error reading from file\n%s\n\nError code: %d", srcdbg_path, errno);
		fclose(file);
		return false;
	}

	fclose(file);
	file = nullptr;

	// ** Simple format header **

	u32 i = 0;
	mame_debug_info_simple_header * header;
	if (!read_field<mame_debug_info_simple_header>(header, data, i, error) ||
		!callback.on_read_simp_header(*header))
	{
		return false;
	}

	header->source_file_paths_size = from_little_endian32(header->source_file_paths_size);
	header->num_line_mappings = from_little_endian32(header->num_line_mappings);
	header->symbol_names_size = from_little_endian32(header->symbol_names_size);
	header->num_global_fixed_symbol_values = from_little_endian32(header->num_global_fixed_symbol_values);
	header->local_fixed_symbol_values_size = from_little_endian32(header->local_fixed_symbol_values_size);
	header->local_relative_symbol_values_size = from_little_endian32(header->local_relative_symbol_values_size);

	// ** Validate some sizes and null terminators **

	u32 first_line_mapping = i + header->source_file_paths_size;
	if (data.size() <= first_line_mapping - 1)
	{
		srcdbg_sprintf(error, "Error reading file\n%s\n\nFile too small to contain reported source_file_paths_size=%d", srcdbg_path, header->source_file_paths_size);
		return false;
	}
	if (data[first_line_mapping - 1] != '\0')
	{
		srcdbg_sprintf(error, "Error reading file\n%s\n\nnull terminator missing at end of last source file", srcdbg_path);
		return false;
	}

	u32 first_symbol_name = first_line_mapping + (header->num_line_mappings * sizeof(srcdbg_line_mapping));
	u32 after_symbol_names = first_symbol_name + header->symbol_names_size;
	if (header->symbol_names_size > 0)
	{
		if (data.size() <= after_symbol_names)
		{
			srcdbg_sprintf(error, "Error reading file\n%s\n\nFile too small to contain reported symbol_names_size=%d", srcdbg_path, header->symbol_names_size);
			return false;
		}

		if (data[after_symbol_names - 1] != '\0')
		{
			srcdbg_sprintf(error, "Error reading file\n%s\n\nnull terminator missing at end of last symbol name", srcdbg_path);
			return false;
		}
	}

	u32 after_global_fixed_symbol_values =
		after_symbol_names + header->num_global_fixed_symbol_values * sizeof(global_fixed_symbol_value);
	if (data.size() < after_global_fixed_symbol_values)
	{
		srcdbg_sprintf(error, "Error reading file\n%s\n\nFile too small to contain reported num_global_fixed_symbol_values=%d", srcdbg_path, header->num_global_fixed_symbol_values);
		return false;
	}

	u32 after_local_fixed_symbol_values = after_global_fixed_symbol_values + header->local_fixed_symbol_values_size;
	if (data.size() < after_local_fixed_symbol_values)
	{
		srcdbg_sprintf(error, "Error reading file\n%s\n\nFile too small to contain reported local_fixed_symbol_values_size=%d", srcdbg_path, header->local_fixed_symbol_values_size);
		return false;
	}

	u32 after_local_relative_symbol_values = after_local_fixed_symbol_values + header->local_relative_symbol_values_size;
	if (data.size() != after_local_relative_symbol_values)
	{
		srcdbg_sprintf(error, "Error reading file\n%s\n\nFile size (%d) not an exact match to the sum of section sizes reported in header", srcdbg_path, data.size());
		return false;
	}

	// ** Source file paths **

	std::string str;
	u32 source_index = 0;
	for (; i < first_line_mapping; i++)
	{
		if (data[i] == '\0')
		{
			if (!callback.on_read_source_path(source_index++, std::move(str)))
			{
				return false;
			}
			str.clear();
		}
		else
		{
			str += char(data[i]);
		}
	}

	if (!callback.end_read_source_paths())
	{
		return false;
	}

	// ** Line mappings **

	for (u32 line_map_idx = 0; line_map_idx < header->num_line_mappings; line_map_idx++)
	{
		srcdbg_line_mapping * line_map;
		if (!read_field<srcdbg_line_mapping>(line_map, data, i, error))
		{
			return false;
		}

		line_map->range.address_first = from_little_endian16(line_map->range.address_first);
		line_map->range.address_last = from_little_endian16(line_map->range.address_last);
		line_map->source_file_index = from_little_endian32(line_map->source_file_index);
		line_map->line_number = from_little_endian32(line_map->line_number);

		if (line_map->source_file_index >= source_index)
		{
			srcdbg_sprintf(error, "Error reading file\n%s\n\nInvalid source_file_index encountered in line map: %d", srcdbg_path, line_map->source_file_index);
			return false;
		}

		if (!callback.on_read_line_mapping(*line_map))
		{
			return false;
		}
	}

	if (!callback.end_read_line_mappings())
	{
		return false;
	}

	// ** Symbol names **

	u32 symbol_index = 0;
	str.clear();
	for (; i < after_symbol_names; i++)
	{
		if (data[i] == '\0')
		{
			if (!callback.on_read_symbol_name(symbol_index++, std::move(str)))
			{
				return false;
			}
			str.clear();
		}
		else
		{
			str += char(data[i]);
		}
	}

	if (!callback.end_read_symbol_names())
	{
		return false;
	}

	// ** Global fixed symbols **

	for (u32 global_fixed_idx = 0; global_fixed_idx < header->num_global_fixed_symbol_values; global_fixed_idx++)
	{
		global_fixed_symbol_value * value;
		if (!read_field<global_fixed_symbol_value>(value, data, i, error))
		{
			return false;
		}

		value->symbol_name_index = from_little_endian32(value->symbol_name_index);
		value->symbol_value = from_little_endian32(value->symbol_value);
		value->symbol_flags = from_little_endian32(value->symbol_flags);

		if (value->symbol_name_index >= symbol_index)
		{
			srcdbg_sprintf(error, "Error reading file\n%s\n\nInvalid symbol_name_index encountered in global_fixed_symbol_value: %d", srcdbg_path, value->symbol_name_index);
			return false;
		}

		if (!callback.on_read_global_fixed_symbol_value(*value))
		{
			return false;
		}
	}

	if (!callback.end_read_global_fixed_symbol_values())
	{
		return false;
	}

	// ** Local fixed symbols **

	while (i < after_local_fixed_symbol_values)
	{
		local_fixed_symbol_value * value;
		u32 value_start_idx = i;
		if (!read_field<local_fixed_symbol_value>(value, data, i, error))
		{
			return false;
		}

		value->symbol_name_index = from_little_endian32(value->symbol_name_index);
		value->symbol_value = from_little_endian32(value->symbol_value);
		value->num_address_ranges = from_little_endian32(value->num_address_ranges);

		if (value->symbol_name_index >= symbol_index)
		{
			srcdbg_sprintf(error, "Error reading file\n%s\n\nInvalid symbol_name_index encountered in local_fixed_symbol_value: %d", srcdbg_path, value->symbol_name_index);
			return false;
		}

		for (u32 idx = 0; idx < value->num_address_ranges; idx++)
		{
			address_range * range;
			if (!read_field<address_range>(range, data, i, error))
			{
				return false;
			}
			range->address_first = from_little_endian16(range->address_first);
			range->address_last = from_little_endian16(range->address_last);
		}

		if (!callback.on_read_local_fixed_symbol_value(*(const local_fixed_symbol_value *) &data[value_start_idx]))
		{
			return false;
		}
	}

	if (!callback.end_read_local_fixed_symbol_values())
	{
		return false;
	}

	// ** Local relative symbols **

	while (i < after_local_relative_symbol_values)
	{
		local_relative_symbol_value * value;
		u32 value_start_idx = i;
		if (!read_field<local_relative_symbol_value>(value, data, i, error))
		{
			return false;
		}

		value->symbol_name_index = from_little_endian32(value->symbol_name_index);
		value->num_local_relative_eval_rules = from_little_endian32(value->num_local_relative_eval_rules);

		if (value->symbol_name_index >= symbol_index)
		{
			srcdbg_sprintf(error, "Error reading file\n%s\n\nInvalid symbol_name_index encountered in local_relative_symbol_value: %d", srcdbg_path, value->symbol_name_index);
			return false;
		}

		for (u32 idx = 0; idx < value->num_local_relative_eval_rules; idx++)
		{
			local_relative_eval_rule * rule;
			if (!read_field<local_relative_eval_rule>(rule, data, i, error))
			{
				return false;
			}

			rule->range.address_first = from_little_endian16(rule->range.address_first);
			rule->range.address_last = from_little_endian16(rule->range.address_last);
			rule->reg_offset = from_little_endian32(rule->reg_offset);
		}

		if (!callback.on_read_local_relative_symbol_value(*(const local_relative_symbol_value *) &data[value_start_idx]))
		{
			return false;
		}
	}

	if (!callback.end_read_local_relative_symbol_values())
	{
		return false;
	}

	return true;
}
