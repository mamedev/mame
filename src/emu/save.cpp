// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    save.cpp

    Save state management functions.

****************************************************************************

    Save state file format:

    00..07  'MAMESAVE'
    08      Format version (this is format 2)
    09      Flags
    0A..1B  Game name padded with \0
    1C..1F  Signature
    20..end Save game data (compressed)

    Data is always written as native-endian.
    Data is converted from the endiannness it was written upon load.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "coreutil.h"
#include "unzip.h"

#include <iomanip>
#include <zlib.h>


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) machine().logerror x; } while (0)



//**************************************************************************
//  ZLIB WRITE STREAMER
//**************************************************************************

// this class wraps the logic needed to stream compressed (deflated) data to
// a file in a .ZIP-compatible format
class zlib_write_streamer
{
public:
	// construction
	zlib_write_streamer(emu_file &output);

	// simple getters
	util::crc32_t crc() const { return m_crc_accum.finish(); }
	u32 uncompressed_bytes() const { return m_uncompressed_bytes; }
	u32 compressed_bytes() const { return m_compressed_bytes; }

	// initialize compression
	bool begin();

	// add more compressed data
	bool write(void const *data, u32 count);

	// finish compression
	bool end();

private:
	// internal state
	emu_file &m_output;                    // the file to spill data to
	z_stream m_stream;                     // the current zlib stream
	util::crc32_creator m_crc_accum;       // accumulated CRC value
	u32 m_uncompressed_bytes;              // accumulated uncompressed bytes
	u32 m_compressed_bytes;                // accumulated compressed bytes
	u8 m_buffer[4096];                     // temporary buffer to accumulate
};


//-------------------------------------------------
//  zlib_write_streamer - constuctor
//-------------------------------------------------

zlib_write_streamer::zlib_write_streamer(emu_file &output) :
	m_output(output)
{
	m_stream.zalloc = Z_NULL;
	m_stream.zfree = Z_NULL;
	m_stream.opaque = Z_NULL;
	m_stream.avail_in = m_stream.avail_out = 0;
}


//-------------------------------------------------
//  begin - initialize compression
//-------------------------------------------------

bool zlib_write_streamer::begin()
{
	// reset the output buffer
	m_stream.next_out = &m_buffer[0];
	m_stream.avail_out = sizeof(m_buffer);

	// reset our accumulators
	m_crc_accum.reset();
	m_uncompressed_bytes = 0;
	m_compressed_bytes = 0;

	// initialize the zlib engine; the negative window size means
	// no headers, which is what a .ZIP file wants
	return (deflateInit2(&m_stream, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) == Z_OK);
}


//-------------------------------------------------
//  write - add more compressed data
//-------------------------------------------------

bool zlib_write_streamer::write(void const *data, u32 count)
{
	// point the input buffer to the data
	m_stream.next_in = const_cast<Bytef *>(reinterpret_cast<Bytef const *>(data));
	m_stream.avail_in = count;

	// loop until all consumed
	while (m_stream.avail_in != 0)
	{
		// deflate as much as possible
		if (deflate(&m_stream, Z_NO_FLUSH) != Z_OK)
		{
			deflateEnd(&m_stream);
			return false;
		}

		// if we ran out of output space, flush to the file and reset
		if (m_stream.avail_out == 0)
		{
			if (m_output.write(&m_buffer[0], sizeof(m_buffer)) != sizeof(m_buffer))
			{
				deflateEnd(&m_stream);
				return false;
			}
			m_compressed_bytes += sizeof(m_buffer);
			m_stream.next_out = &m_buffer[0];
			m_stream.avail_out = sizeof(m_buffer);
		}
	}

	// update accumulators
	m_uncompressed_bytes += count;
	m_crc_accum.append(data, count);
	return true;
}


//-------------------------------------------------
//  end - finish cmopression
//-------------------------------------------------

bool zlib_write_streamer::end()
{
	// loop until all data processed
	int zerr = Z_OK;
	while (zerr != Z_STREAM_END)
	{
		// deflate and attempt to finish
		zerr = deflate(&m_stream, Z_FINISH);
		if (zerr != Z_OK && zerr != Z_STREAM_END)
		{
			deflateEnd(&m_stream);
			return false;
		}

		// if there's any output data, flush it to the file and reset
		if (m_stream.avail_out != sizeof(m_buffer))
		{
			u32 bytes = sizeof(m_buffer) - m_stream.avail_out;
			if (m_output.write(&m_buffer[0], bytes) != bytes)
			{
				deflateEnd(&m_stream);
				return false;
			}
			m_compressed_bytes += bytes;
			m_stream.next_out = &m_buffer[0];
			m_stream.avail_out = sizeof(m_buffer);
		}
	}

	// finalize the CRC
	m_crc_accum.finish();
	return (deflateEnd(&m_stream) == Z_OK);
}



//**************************************************************************
//  SAVE ZIP STATE
//**************************************************************************

// this class manages the creation of a ZIP file containing a JSON with most of
// the save data, plus various binary files containing larger chunks of data
class save_zip_state
{
	// internal constants
	static constexpr u32 JSON_EXPAND_CHUNK = 1024 * 1024;
	static constexpr u32 JSON_EXPAND_THRESH = 1024;

public:
	// the size threshold in bytes above which we will write an external file
	static constexpr u32 JSON_EXTERNAL_BINARY_THRESHOLD = 16 * 1024;

	// construction
	save_zip_state();

	// simple getters
	char const *json_string() { m_json[m_json_offset] = 0; return &m_json[0]; }
	int json_length() const { return m_json_offset; }

	// append a character to the JSON stream
	save_zip_state &json_append(char ch) { 	m_json[m_json_offset++] = ch; return *this; }

	// append an end-of-line sequence to the JSON stream
	save_zip_state &json_append_eol() { return json_append(13).json_append(10); }

	// additional JSON output helpers
	save_zip_state &json_append(char const *buffer);
	save_zip_state &json_append_indent(int count);
	save_zip_state &json_append_name(char const *name);
	save_zip_state &json_append_signed(s64 value);
	save_zip_state &json_append_unsigned(u64 value);
	save_zip_state &json_append_float(double value);

	// stage an item to be output as raw data
	char const *add_data_file(char const *proposed_name, save_registered_item &item, uintptr_t base);

	// commit the results to the given file
	bool commit(emu_file &output);

private:
	// check the reserve; if we're getting close, expand out one more chunk
	void json_check_reserve()
	{
		if (m_json_reserved - m_json_offset < JSON_EXPAND_THRESH)
		{
			m_json_reserved += JSON_EXPAND_CHUNK;
			m_json.resize(m_json_reserved);
		}
	}

	// other internal helpers
	void create_end_of_central_directory(std::vector<u8> &header, u32 central_dir_entries, u64 central_dir_offset, u32 central_dir_size);
	void create_zip_file_header(std::vector<u8> &local, std::vector<u8> &central, char const *filename, u64 local_offset);
	void create_zip_file_footer(std::vector<u8> &local, std::vector<u8> &central, u32 filesize, u32 compressed, u32 crc);
	bool write_data_recursive(zlib_write_streamer &zlib, save_registered_item &item, uintptr_t base);

	// file_entry represents a single raw data file that will be written
	struct file_entry
	{
		file_entry(char const *name, save_registered_item &item, uintptr_t base) :
			m_item(item), m_name(name), m_base(base) { }

		save_registered_item &m_item;
		std::string m_name;
		uintptr_t m_base;
		std::vector<u8> m_central_directory;
	};

	// internal state
	std::list<file_entry> m_file_list;     // list of files to be output
	std::vector<char> m_json;              // accumulated JSON data
	u32 m_json_offset;                     // current output offset in JSON stream
	u32 m_json_reserved;                   // current total reserved size for JSON stream
	u16 m_archive_date;                    // precomputed archive date, in MS-DOS format
	u16 m_archive_time;                    // precomputed archive time, in MS-DOS format
};


//-------------------------------------------------
//  save_zip_state - constuctor
//-------------------------------------------------

save_zip_state::save_zip_state() :
	m_json_offset(0),
	m_json_reserved(0)
{
	json_check_reserve();
}


//-------------------------------------------------
//  json_append - append a string to the JSON
//  stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append(char const *buffer)
{
	json_check_reserve();
	while (*buffer != 0)
		json_append(*buffer++);
	return *this;
}


//-------------------------------------------------
//  json_append_indent - append an indentation of
//  the given depth to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_indent(int count)
{
	for (int index = 0; index < count; index++)
		json_append('\t');
	return *this;
}


//-------------------------------------------------
//  json_append_name - append a string-ified name
//  to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_name(char const *name)
{
	if (name == nullptr || name[0] == 0)
		return *this;
	return json_append('"').json_append(name).json_append('"').json_append(':');
}


//-------------------------------------------------
//  json_append_signed - append a signed integer
//  value to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_signed(s64 value)
{
	json_check_reserve();

	// quote values that don't fit into a double
	bool quote = (s64(double(value)) != value);
	if (quote)
		json_append('"');

	// just use sprintf -- is there a faster way?
	char buffer[20];
	sprintf(buffer, "%lld", value);
	json_append(buffer);

	// end quotes
	if (quote)
		json_append('"');
	return *this;
}


//-------------------------------------------------
//  json_append_unsigned - append an unsigned
//  integer value to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_unsigned(u64 value)
{
	json_check_reserve();

	// quote values that don't fit into a double
	bool quote = (u64(double(value)) != value);
	if (quote)
		json_append('"');

	// just use sprintf -- is there a faster way?
	char buffer[20];
	sprintf(buffer, "%llu", value);
	json_append(buffer);

	// end quotes
	if (quote)
		json_append('"');
	return *this;
}


//-------------------------------------------------
//  json_append_float - append a floating-point
//  value to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_float(double value)
{
	json_check_reserve();
	char buffer[32];
	sprintf(buffer, "%1.17g", value);
	return json_append(buffer);
}


//-------------------------------------------------
//  add_data_file - add a data file to the ZIP
//  file, creating a clean, unique filename for it
//-------------------------------------------------

char const *save_zip_state::add_data_file(char const *proposed_name, save_registered_item &item, uintptr_t base)
{
	// first sanitize the filename
	std::string base_filename = proposed_name;
	for (int index = 0; index < base_filename.length(); )
	{
		if (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-.", base_filename[index]) == nullptr)
		{
			if (index != 0 && base_filename[index - 1] != '.')
				base_filename[index++] = '.';
			else
				base_filename.erase(index, 1);
		}
		else
			index++;
	}

	// now ensure it is unique
	std::string filename;
	bool retry = true;
	for (int index = 1; retry; index++)
	{
		if (index == 1)
			filename = string_format("%s.bin", base_filename.c_str());
		else
			filename = string_format("%s.%d.bin", base_filename.c_str(), index);

		// see if anyone else has this name; if so, retry it
		retry = false;
		for (auto &file : m_file_list)
			if (filename == file.m_name)
			{
				retry = true;
				break;
			}
	}

	// add to the list
	m_file_list.emplace_back(filename.c_str(), item, base);
	return m_file_list.back().m_name.c_str();
}


//-------------------------------------------------
//  commit - assemble all the files into their
//  final forms and write the ZIP data to the
//  output file
//-------------------------------------------------

bool save_zip_state::commit(emu_file &output)
{
	zlib_write_streamer zlib(output);
	std::vector<u8> local_header;
	std::vector<u8> local_footer;

	// determine the MS-DOS formatted time
	time_t rawtime;
	::time(&rawtime);
	struct tm &timeinfo = *localtime(&rawtime);
	m_archive_date = timeinfo.tm_mday | ((timeinfo.tm_mon + 1) << 5) | ((timeinfo.tm_year - 1980) << 9);
	m_archive_time = (timeinfo.tm_sec / 2) | (timeinfo.tm_min << 5) | (timeinfo.tm_hour << 11);

	// write the local header (and create the central directory entry) for the JSON itself
	std::vector<u8> json_central_directory;
	u64 local_header_offset = output.tell();
	create_zip_file_header(local_header, json_central_directory, "save.json", local_header_offset);
	output.write(&local_header[0], local_header.size());

	// stream the JSON and compress it
	if (!zlib.begin() || !zlib.write(&m_json[0], m_json_offset) || !zlib.end())
		return false;

	// write the local footer and update the central directory entry
	create_zip_file_footer(local_footer, json_central_directory, zlib.uncompressed_bytes(), zlib.compressed_bytes(), zlib.crc());
	output.seek(local_header_offset + 0xe, SEEK_SET);
	output.write(&local_footer[0], local_footer.size());
	output.seek(0, SEEK_END);

	// then write out the other files
	for (auto &file : m_file_list)
	{
		// write the local header (and create the central directory entry) for the file
		u64 local_header_offset = output.tell();
		create_zip_file_header(local_header, file.m_central_directory, file.m_name.c_str(), local_header_offset);
		output.write(&local_header[0], local_header.size());

		// write the file header and compress it
		if (!zlib.begin() || !write_data_recursive(zlib, file.m_item, file.m_base) || !zlib.end())
			return false;

		// write the local footer and update the central directory entry
		create_zip_file_footer(local_footer, file.m_central_directory, zlib.uncompressed_bytes(), zlib.compressed_bytes(), zlib.crc());
		output.seek(local_header_offset + 0xe, SEEK_SET);
		output.write(&local_footer[0], local_footer.size());
		output.seek(0, SEEK_END);
	}

	// remember the base of the central directory, then write it
	u64 central_dir_offset = output.tell();
	output.write(&json_central_directory[0], json_central_directory.size());
	for (auto &file : m_file_list)
		output.write(&file.m_central_directory[0], file.m_central_directory.size());

	// now create the
	std::vector<u8> eocd;
	create_end_of_central_directory(eocd, m_file_list.size() + 1, central_dir_offset, output.tell() - central_dir_offset);
	output.write(&eocd[0], eocd.size());
	return true;
}


//-------------------------------------------------
//  create_zip_file_header - create both the local
//  and central file headers; the CRC and size
//  information is stored as 0 at this stage
//-------------------------------------------------

void save_zip_state::create_zip_file_header(std::vector<u8> &local, std::vector<u8> &central, char const *filename, u64 local_offset)
{
	// reset the headers
	local.clear();
	central.clear();

	// write the standard headers
	local.push_back(0x50);	central.push_back(0x50);
	local.push_back(0x4b);	central.push_back(0x4b);
	local.push_back(0x03);	central.push_back(0x01);
	local.push_back(0x04);	central.push_back(0x02);

	// version created by = 3.0 / 0 (MS-DOS) (central directory only)
							central.push_back(0x1e);
							central.push_back(0x00);

	// version to extract = 2.0
	local.push_back(0x14);	central.push_back(0x14);
	local.push_back(0x00);	central.push_back(0x00);

	// general purpose bit flag = 0x02 (2=max compression)
	local.push_back(0x02);	central.push_back(0x02);
	local.push_back(0x00);	central.push_back(0x00);

	// compression method = 8 (deflate)
	local.push_back(0x08);	central.push_back(0x08);
	local.push_back(0x00);	central.push_back(0x00);

	// last mod file time
	local.push_back(BIT(m_archive_time, 0, 8));	central.push_back(BIT(m_archive_time, 0, 8));
	local.push_back(BIT(m_archive_time, 8, 8));	central.push_back(BIT(m_archive_time, 8, 8));

	// last mod file date
	local.push_back(BIT(m_archive_date, 0, 8));	central.push_back(BIT(m_archive_date, 0, 8));
	local.push_back(BIT(m_archive_date, 8, 8));	central.push_back(BIT(m_archive_date, 8, 8));

	// crc-32 -- to be written later
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);

	// compressed size -- to be written later
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);

	// uncompressed size -- to be written later
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);

	// file name length
	u16 len = strlen(filename);
	local.push_back(BIT(len, 0, 8)); central.push_back(BIT(len, 0, 8));
	local.push_back(BIT(len, 8, 8)); central.push_back(BIT(len, 8, 8));

	// extra field length
	local.push_back(0x00);	central.push_back(0x00);
	local.push_back(0x00);	central.push_back(0x00);

	// file comment length (central directory only)
							central.push_back(0x00);
							central.push_back(0x00);

	// disk number start (central directory only)
							central.push_back(0x00);
							central.push_back(0x00);

	// internal file attributes (central directory only)
							central.push_back(0x00);
							central.push_back(0x00);

	// external file attributes (central directory only)
							central.push_back(0x00);
							central.push_back(0x00);
							central.push_back(0x00);
							central.push_back(0x00);

	// relative offset of local header (central directory only)
							central.push_back(BIT(local_offset, 0, 8));
							central.push_back(BIT(local_offset, 8, 8));
							central.push_back(BIT(local_offset, 16, 8));
							central.push_back(BIT(local_offset, 24, 8));

	// filename
	for ( ; *filename != 0; filename++)
	{
		local.push_back(*filename);
		central.push_back(*filename);
	}
}


//-------------------------------------------------
//  create_zip_file_footer - create the CRC and
//  size information, and update the central
//  directory entry with the data
//-------------------------------------------------

void save_zip_state::create_zip_file_footer(std::vector<u8> &local, std::vector<u8> &central, u32 filesize, u32 compressed, u32 crc)
{
	// reset the local footer data
	local.clear();

	// crc-32 -- to be written later
	local.push_back(central[16] = BIT(crc, 0, 8));
	local.push_back(central[17] = BIT(crc, 8, 8));
	local.push_back(central[18] = BIT(crc, 16, 8));
	local.push_back(central[19] = BIT(crc, 24, 8));

	// compressed size -- to be written later
	local.push_back(central[20] = BIT(compressed, 0, 8));
	local.push_back(central[21] = BIT(compressed, 8, 8));
	local.push_back(central[22] = BIT(compressed, 16, 8));
	local.push_back(central[23] = BIT(compressed, 24, 8));

	// uncompressed size -- to be written later
	local.push_back(central[24] = BIT(filesize, 0, 8));
	local.push_back(central[25] = BIT(filesize, 8, 8));
	local.push_back(central[26] = BIT(filesize, 16, 8));
	local.push_back(central[27] = BIT(filesize, 24, 8));
}


//-------------------------------------------------
//  write_data_recursive - write potentially
//  multi-dimensional arrays to the compressed
//  output, computing size and CRC
//-------------------------------------------------

bool save_zip_state::write_data_recursive(zlib_write_streamer &zlib, save_registered_item &item, uintptr_t base)
{
	save_registered_item &inner = item.subitems().front();
	if (inner.is_array())
	{
		for (int index = 0; index < item.count(); index++)
		{
			if (!write_data_recursive(zlib, inner, base))
				return false;
			base += item.native_size();
		}
	}
	else
	{
		u32 size = item.count() * item.native_size();
		if (!zlib.write(reinterpret_cast<void *>(base), size))
			return false;
	}
	return true;
}


//-------------------------------------------------
//  create_end_of_central_directory - create a
//  buffer containing the end of central directory
//  record
//-------------------------------------------------

void save_zip_state::create_end_of_central_directory(std::vector<u8> &header, u32 central_dir_entries, u64 central_dir_offset, u32 central_dir_size)
{
	// end of central directory header
	header.push_back(0x50);
	header.push_back(0x4b);
	header.push_back(0x05);
	header.push_back(0x06);

	// number of this disk
	header.push_back(0x00);
	header.push_back(0x00);

	// number of disk with start of central directory
	header.push_back(0x00);
	header.push_back(0x00);

	// total central directory entries on this disk
	header.push_back(BIT(central_dir_entries, 0, 8));
	header.push_back(BIT(central_dir_entries, 8, 8));

	// total central directory entries
	header.push_back(BIT(central_dir_entries, 0, 8));
	header.push_back(BIT(central_dir_entries, 8, 8));

	// size of the central directory
	header.push_back(BIT(central_dir_size, 0, 8));
	header.push_back(BIT(central_dir_size, 8, 8));
	header.push_back(BIT(central_dir_size, 16, 8));
	header.push_back(BIT(central_dir_size, 24, 8));

	// offset of central directory
	header.push_back(BIT(central_dir_offset, 0, 8));
	header.push_back(BIT(central_dir_offset, 8, 8));
	header.push_back(BIT(central_dir_offset, 16, 8));
	header.push_back(BIT(central_dir_offset, 24, 8));

	// ZIP comment length
	header.push_back(0x00);
	header.push_back(0x00);
}



//**************************************************************************
//  ZLIB READ STREAMER
//**************************************************************************

class zlib_read_streamer
{
public:
	// construction
	zlib_read_streamer(emu_file &input);

	// simple getters
	util::crc32_t crc() const { return m_crc_accum.finish(); }
	util::crc32_t expected_crc() const { return m_expected_crc; }
	u32 uncompressed_bytes() const { return m_uncompressed_bytes; }
	u32 compressed_bytes() const { return m_compressed_bytes; }

	// initialize decompression
	bool begin(u64 offset);

	// read more compressed data
	bool read(void *data, u32 count);

	// finish decompression
	bool end();

private:
	// internal state
	emu_file &m_input;                     // the file to read from
	z_stream m_stream;                     // the current zlib stream
	util::crc32_creator m_crc_accum;       // accumulated CRC value
	u32 m_uncompressed_bytes;              // accumulated uncompressed bytes
	u32 m_compressed_bytes;                // accumulated compressed bytes
	u32 m_input_remaining;                 // number of input bytes remaining
	u32 m_expected_crc;                    // expected CRC value
	u8 m_buffer[4096];                     // temporary buffer to accumulate
};


//-------------------------------------------------
//  zlib_read_streamer - constuctor
//-------------------------------------------------

zlib_read_streamer::zlib_read_streamer(emu_file &input) :
	m_input(input)
{
	m_stream.zalloc = Z_NULL;
	m_stream.zfree = Z_NULL;
	m_stream.opaque = Z_NULL;
	m_stream.avail_in = m_stream.avail_out = 0;
}


//-------------------------------------------------
//  begin - initialize decompression
//-------------------------------------------------

bool zlib_read_streamer::begin(u64 offset)
{
	// read the local file header
	u8 local[30];
	m_input.seek(offset, SEEK_SET);
	if (m_input.read(&local[0], sizeof(local)) != sizeof(local))
		return false;

	// validate header
	if (local[0] != 0x50 || local[1] != 0x4b || local[2] != 0x03 || local[3] != 0x04)
		return false;

	// only deflate is supported
	if (local[8] != 0x08)
		return false;

	// parse data from the header
	m_expected_crc = local[14] | (local[15] << 8) | (local[16] << 16) | (local[17] << 24);
	m_compressed_bytes = local[18] | (local[19] << 8) | (local[20] << 16) | (local[21] << 24);
	m_uncompressed_bytes = local[22] | (local[23] << 8) | (local[24] << 16) | (local[25] << 24);
	u32 name_len = local[26] | (local[27] << 8);
	u32 extra_len = local[28] | (local[29] << 8);

	// advance past the header to the actual start of data
	offset += 30 + name_len + extra_len;
	m_input.seek(offset, SEEK_SET);

	// reset the input buffer
	m_stream.avail_in = 0;

	// reset our accumulators
	m_crc_accum.reset();
	m_input_remaining = m_compressed_bytes;

	// initialize the zlib engine; the negative window size means
	// no headers, which is what a .ZIP file wants
	return (inflateInit2(&m_stream, -MAX_WBITS) == Z_OK);
}


//-------------------------------------------------
//  read - read more compressed data
//-------------------------------------------------

bool zlib_read_streamer::read(void *data, u32 count)
{
	// point the output buffer to the target buffer
	m_stream.next_out = reinterpret_cast<Bytef *>(data);
	m_stream.avail_out = count;

	// loop until all consumed
	while (m_stream.avail_out != 0)
	{
		// if we need more data, fetch it
		if (m_stream.avail_in == 0)
		{
			m_stream.next_in = &m_buffer[0];
			m_stream.avail_in = std::min<u32>(sizeof(m_buffer), m_input_remaining);
			m_input_remaining -= m_stream.avail_in;
			if (m_input.read(&m_buffer[0], m_stream.avail_in) != m_stream.avail_in)
			{
				inflateEnd(&m_stream);
				return false;
			}
		}

		// deflate as much as possible
		auto zerr = inflate(&m_stream, Z_NO_FLUSH);
		if (zerr != Z_OK && (zerr != Z_STREAM_END || m_stream.avail_out != 0))
		{
			inflateEnd(&m_stream);
			return false;
		}
	}

	// update accumulators
	m_crc_accum.append(data, count);
	return true;
}


//-------------------------------------------------
//  end - finish cmopression
//-------------------------------------------------

bool zlib_read_streamer::end()
{
	// fail if CRCs didn't match
	return (inflateEnd(&m_stream) == Z_OK && crc() == expected_crc());
}



//**************************************************************************
//  LOAD ZIP STATE
//**************************************************************************

// this class manages loading from a ZIP file containing a JSON with most of
// the save data, plus various binary files containing larger chunks of data
class load_zip_state
{
public:
	// load_error is the exception we throw if anything bad happens
	class load_error : public std::exception
	{
	public:
		load_error(save_error err) :
			m_error(err) { }
		save_error error() const { return m_error; }

	private:
		save_error m_error;
	};

	// construction
	load_zip_state(emu_file &file);

	// simple getters
	emu_file &file() const { return m_file; }
	char const *warnings() const { return (m_warnings.length() == 0) ? nullptr : m_warnings.c_str(); }
	char const *errors() const { return (m_errors.length() == 0) ? nullptr : m_errors.c_str(); }
	char const *json_position() const { return m_json_ptr; }

	// simple setters
	void json_set_position(char const *pos) { m_json_ptr = pos; }

	// return the next character in the buffer
	char json_peek() const { return *m_json_ptr; }

	// advance to the next non-whitespace character
	void json_skip_whitespace() { while (isspace(*m_json_ptr)) m_json_ptr++; }

	// various JSON parsing helpers
	bool json_matches(char target);
	bool json_matches(char const *target);
	bool json_parse_number(double &result);
	bool json_parse_int_string(s64 &result);
	bool json_parse_uint_string(u64 &result);
	bool json_parse_string(std::string &result);

	// initialize, checking the input file basic validity
	save_error init();

	// find a file in the ZIP, returning its uncompressed size and offset to local header
	bool find_file(char const *name, u64 &offset, u32 &size);

	// recursively read data using
	bool read_data_recursive(zlib_read_streamer &zlib, save_registered_item &item, bool flip, u32 &remaining, uintptr_t base);

	// report a warning
	template<typename... Params>
	void report_warning(char const *format, Params &&... args)
	{
		m_warnings.append(string_format(format, std::forward<Params>(args)...));
		m_warnings += "\n";
	}

	// report an error; this implicitly throws to exit
	template<typename... Params>
	void report_error(save_error type, char const *format, Params &&... args)
	{
		m_errors.append(string_format(format, std::forward<Params>(args)...));
		m_errors += "\n";
		throw load_error(type);
	}

private:
	// file_entry represents a single file within the ZIP
	struct file_entry
	{
		file_entry(char const *name, u64 offset, u32 compsize, u32 uncompsize) :
			m_name(name), m_offset(offset), m_compsize(compsize), m_uncompsize(uncompsize) { }

		std::string m_name;
		u64 m_offset;
		u32 m_compsize;
		u32 m_uncompsize;
	};

	// internal state
	emu_file &m_file;                      // input file
	std::vector<u8> m_json_data;           // buffered JSON file
	char const *m_json_ptr;                // current input pointer to data
	std::list<file_entry> m_file_list;     // list of files to be output
	std::string m_warnings;                // accumulated warnings string
	std::string m_errors;                  // accumulated errors string
};


//-------------------------------------------------
//  load_zip_state - constructor
//-------------------------------------------------

load_zip_state::load_zip_state(emu_file &file) :
	m_file(file),
	m_json_ptr(nullptr)
{
}


//-------------------------------------------------
//  json_matches - return true and advance if the
//  next character matches the target
//-------------------------------------------------

bool load_zip_state::json_matches(char target)
{
	json_skip_whitespace();
	if (*m_json_ptr == target)
	{
		m_json_ptr++;
		return true;
	}
	return false;
}


//-------------------------------------------------
//  json_matches - return true and advance if the
//  next characters match the target string
//-------------------------------------------------

bool load_zip_state::json_matches(char const *target)
{
	json_skip_whitespace();
	char const *start = m_json_ptr;
	for ( ; *target != 0; target++)
		if (!json_matches(*target))
			break;
	if (*target == 0)
		return true;
	m_json_ptr = start;
	return false;
}


//-------------------------------------------------
//  json_parse_number - parse a floating-point
//  number from the JSON
//-------------------------------------------------

bool load_zip_state::json_parse_number(double &result)
{
	json_skip_whitespace();
	char const *start = m_json_ptr;
	result = strtod(start, const_cast<char **>(&m_json_ptr));
	return (start != m_json_ptr);
}


//-------------------------------------------------
//  json_parse_int_string - parse a 64-bit signed
//  integer from a string
//-------------------------------------------------

bool load_zip_state::json_parse_int_string(s64 &result)
{
	if (!json_matches('"'))
		return false;
	char const *start = m_json_ptr;
	result = strtoll(start, const_cast<char **>(&m_json_ptr), 10);
	return (start != m_json_ptr && json_matches('"'));
}


//-------------------------------------------------
//  json_parse_int_string - parse a 64-bit unsigned
//  integer from a string
//-------------------------------------------------

bool load_zip_state::json_parse_uint_string(u64 &result)
{
	if (!json_matches('"'))
		return false;
	char const *start = m_json_ptr;
	result = strtoull(start, const_cast<char **>(&m_json_ptr), 10);
	return (start != m_json_ptr && json_matches('"'));
}


//-------------------------------------------------
//  json_parse_int_string - parse a 64-bit unsigned
//  integer from a string
//-------------------------------------------------

bool load_zip_state::json_parse_string(std::string &result)
{
	if (!json_matches('"'))
		return false;
	char const *start = m_json_ptr;
	char ch;
	bool found_controls = false;
	while ((ch = *m_json_ptr) != 0)
	{
		m_json_ptr++;
		if (ch == '\\')
		{
			m_json_ptr++;
			found_controls = true;
		}
		else if (ch == '"')
			break;
	}
	result = std::string(start, m_json_ptr - 1 - start);

	// if we saw any control characters, go back and fix them up
	if (found_controls)
		for (int index = 0; index < result.length(); index++)
			if (result[index] == '\\')
			{
				char ch = result[index + 1];
				result.erase(index + 1, 1);
				switch (ch)
				{
					case '/':	result[index] = ch;	break;
					case '\\':	result[index] = ch;	break;
					case '"':	result[index] = ch;	break;
					case 'b':	result[index] = 8;	break;
					case 'f':	result[index] = 12;	break;
					case 'n':	result[index] = 10;	break;
					case 'r':	result[index] = 13;	break;
					case 't':	result[index] = 9;	break;
					case 'u':	result[index] = '?'; result.erase(index + 1, 4); break;
				}
			}
	return true;
}


//-------------------------------------------------
//  init - initialize by parsing the ZIP structure
//  and loading the JSON data
//-------------------------------------------------

save_error load_zip_state::init()
{
	// read the last 1k of the file to find the end of central directory record
	u8 buffer[1024];
	u64 filesize = m_file.size();
	int bufread = std::min(filesize, sizeof(buffer));
	m_file.seek(-bufread, SEEK_END);
	if (m_file.read(&buffer[0], bufread) != bufread)
		return STATERR_READ_ERROR;

	// scan backwards to find it
	u32 dir_offset = 0;
	u32 dir_size = 0;
	u32 dir_entries = 0;
	for (int scan = bufread - 20; scan >= 0; scan--)
		if (buffer[scan + 0] == 0x50 && buffer[scan + 1] == 0x4b && buffer[scan + 2] == 0x05 && buffer[scan + 3] == 0x06)
		{
			u8 *eocd = &buffer[scan];
			dir_entries = eocd[10] | (eocd[11] << 8);
			dir_size = eocd[12] | (eocd[13] << 8) | (eocd[14] << 16) | (eocd[15] << 24);
			dir_offset = eocd[16] | (eocd[17] << 8) | (eocd[18] << 16) | (eocd[19] << 24);
			break;
		}

	// if nothing found, it's an error
	if (dir_entries == 0)
		return STATERR_INVALID_FILE;

	// read the central directory
	std::vector<u8> central(dir_size);
	m_file.seek(dir_offset, SEEK_SET);
	if (m_file.read(&central[0], dir_size) != dir_size)
		return STATERR_READ_ERROR;

	// parse through the entries
	u32 offset = 0;
	for ( ; dir_entries != 0 && offset < central.size() - 46; dir_entries--)
	{
		// find the start of entry
		if (offset + 46 >= dir_size)
			return STATERR_INVALID_FILE;
		if (central[offset] != 0x50 && central[offset + 1] != 0x4b && central[offset + 2] != 0x01 && central[offset + 3] != 0x02)
			return STATERR_INVALID_FILE;

		// only deflate is supported; anything else will be an error
		if (central[offset + 10] != 8)
			return STATERR_INVALID_FILE;

		// pull out all the interesting data
		u32 compsize = central[offset + 20] | (central[offset + 21] << 8) | (central[offset + 22] << 16) | (central[offset + 23] << 24);
		u32 uncompsize = central[offset + 24] | (central[offset + 25] << 8) | (central[offset + 26] << 16) | (central[offset + 27] << 24);
		u32 namelen = central[offset + 28] | (central[offset + 29] << 8);
		u32 extralen = central[offset + 30] | (central[offset + 31] << 8);
		u32 commentlen = central[offset + 32] | (central[offset + 33] << 8);
		u32 header_offs = central[offset + 42] | (central[offset + 43] << 8) | (central[offset + 44] << 16) | (central[offset + 45] << 24);
		std::string filename(reinterpret_cast<char *>(&central[offset + 46]), namelen);
		offset += 46 + namelen + extralen + commentlen;

		// add a file entry
		m_file_list.emplace_back(filename.c_str(), header_offs, compsize, uncompsize);
	}

	// now find the json file
	u64 json_offset;
	u32 json_size;
	if (!find_file("save.json", json_offset, json_size))
		return STATERR_READ_ERROR;
	m_json_data.resize(json_size + 1);

	// read the data
	zlib_read_streamer reader(m_file);
	if (!reader.begin(json_offset) || !reader.read(&m_json_data[0], json_size) || !reader.end())
		return STATERR_READ_ERROR;
	m_json_data[json_size] = 0;
	m_json_ptr = reinterpret_cast<char *>(&m_json_data[0]);

	return STATERR_NONE;
}


//-------------------------------------------------
//  find_file - find a file by name and return the
//  uncompressed size plus the file offset
//-------------------------------------------------

bool load_zip_state::find_file(char const *name, u64 &offset, u32 &uncompsize)
{
	// just scan the list for a filename match and return the data
	for (auto &file : m_file_list)
		if (file.m_name == name)
		{
			offset = file.m_offset;
			uncompsize = file.m_uncompsize;
			return true;
		}

	return false;
}


//-------------------------------------------------
//  read_data_recursive - write potentially
//  multi-dimensional arrays to the compressed
//  output, computing size and CRC
//-------------------------------------------------

bool load_zip_state::read_data_recursive(zlib_read_streamer &zlib, save_registered_item &item, bool flip, u32 &remaining, uintptr_t base)
{
	save_registered_item &inner = item.subitems().front();
	if (inner.is_array())
	{
		for (int index = 0; index < item.count(); index++)
		{
			if (!read_data_recursive(zlib, inner, flip, remaining, base))
				return false;
			base += item.native_size();
		}
	}
	else
	{
		u32 size = item.count() * item.native_size();
		size = std::min(size, remaining);
		if (!zlib.read(reinterpret_cast<void *>(base), size))
			return false;
		remaining -= size;
		if (flip)
			switch (item.native_size())
			{
				case 2:
				{
					u16 *data = reinterpret_cast<u16 *>(base);
					for (int index = 0; index < item.count(); index++)
						data[index] = swapendian_int16(data[index]);
					break;
				}
				case 4:
				{
					u32 *data = reinterpret_cast<u32 *>(base);
					for (int index = 0; index < item.count(); index++)
						data[index] = swapendian_int32(data[index]);
					break;
				}
				case 8:
				{
					u64 *data = reinterpret_cast<u64 *>(base);
					for (int index = 0; index < item.count(); index++)
						data[index] = swapendian_int64(data[index]);
					break;
				}
			}
	}
	return true;
}



//**************************************************************************
//  SAVE REGISTERED ITEM
//**************************************************************************

//-------------------------------------------------
//  save_registered_item - constructor
//-------------------------------------------------

save_registered_item::save_registered_item() :
	m_ptr_offset(0),
	m_type_count(TYPE_CONTAINER),
	m_native_size(0)
{
}

// constructor for a new item
save_registered_item::save_registered_item(uintptr_t ptr_offset, save_type type, u32 native_size, char const *name, u32 count) :
	m_ptr_offset(ptr_offset),
	m_type_count(u32(type) | (count << 4)),
	m_native_size(native_size),
	m_name(name)
{
	// cleanup names a bit
	if (m_name[0] == '*')
		m_name.erase(0, 1);
	if (m_name[0] == 'm' && m_name[1] == '_')
		m_name.erase(0, 2);
}


//-------------------------------------------------
//  append - append a new item to the current one
//-------------------------------------------------

std::string type_string(save_registered_item::save_type type, u32 native_size)
{
	switch (type)
	{
	case save_registered_item::TYPE_CONTAINER:	return "CONTAINER";
	case save_registered_item::TYPE_UNIQUE:		return "UNIQUE";
	case save_registered_item::TYPE_VECTOR:		return "VECTOR";
	case save_registered_item::TYPE_STRUCT:		return "STRUCT";
	case save_registered_item::TYPE_BOOL:		return "BOOL";
	case save_registered_item::TYPE_INT:		return string_format("INT%d", 8 * native_size);
	case save_registered_item::TYPE_UINT:		return string_format("UINT%d", 8 * native_size);
	case save_registered_item::TYPE_FLOAT:		return string_format("FLOAT%d", 8 * native_size);
	default:				return string_format("ARRAY[%d]", int(type));
	}
}

save_registered_item &save_registered_item::append(uintptr_t ptr_offset, save_type type, u32 native_size, char const *name, u32 count)
{
	// make sure there are no duplicates
	if (find(name) != nullptr)
		throw emu_fatalerror("Duplicate save state registration '%s'\n", name);

//printf("%s '%s': adding %s '%s' @ %llX, size %d\n", type_string(m_type, m_native_size).c_str(), m_name.c_str(), type_string(type, native_size).c_str(), name, ptr_offset, native_size);

	// add the item to the back of the list
	m_items.emplace_back(ptr_offset, type, native_size, name, count);
	return m_items.back();
}


//-------------------------------------------------
//  find - find a subitem by name
//-------------------------------------------------

save_registered_item *save_registered_item::find(char const *name)
{
	// blank names can't be found this way
	if (name[0] == 0)
		return nullptr;

	// make sure there are no duplicates
	for (auto &item : m_items)
		if (strcmp(item.name(), name) == 0)
			return &item;
	return nullptr;
}


//-------------------------------------------------
//  is_replicatable - can this item be replicated
//  across an array?
//-------------------------------------------------

bool save_registered_item::is_replicatable(bool parent_is_array) const
{
	switch (type())
	{
		// numeric types are always replicatable
		case TYPE_BOOL:
		case TYPE_INT:
		case TYPE_UINT:
		case TYPE_FLOAT:
			return true;

		// unique pointers, vectors, and runtime arrays are non-replicatable
		case TYPE_UNIQUE:
		case TYPE_VECTOR:
		case TYPE_VECTOR_ARRAY:
		case TYPE_RAW_ARRAY:
			return false;

		// structs, containers, and static arrays are replicatable if all their owned items are
		case TYPE_STATIC_ARRAY:
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			for (auto &item : m_items)
				if (!item.is_replicatable(false))
					return false;
			return true;

		// when it doubt, no
		default:
			return false;
	}
}


//-------------------------------------------------
//  sort_and_prune - prune empty subitems and
//  sort them by name
//-------------------------------------------------

bool save_registered_item::sort_and_prune()
{
	// only applies to arrays, structs, and containers; don't prune anything else
	if (!is_array() && !is_struct_or_container())
		return false;

	// first prune any empty items
	for (auto it = m_items.begin(); it != m_items.end(); )
	{
		if (it->sort_and_prune())
			it = m_items.erase(it);
		else
			++it;
	}

	// then sort the rest if we have more than 1
	if (is_struct_or_container() && m_items.size() > 1)
		m_items.sort([] (auto const &x, auto const &y) { return (std::strcmp(x.name(), y.name()) < 0); });

	// return true if we have nothing
	return (m_items.size() == 0);
}


//-------------------------------------------------
//  unwrap_and_update_base - unwrap special
//  types and update the object base
//-------------------------------------------------

bool save_registered_item::unwrap_and_update_base(uintptr_t &objbase) const
{
	// update the base pointer with our local base/offset
	objbase += m_ptr_offset;

	// switch off the type
	switch (type())
	{
		// unique ptrs retrieve the pointer from their container
		case TYPE_UNIQUE:
			objbase = reinterpret_cast<uintptr_t>(reinterpret_cast<generic_unique *>(objbase)->get());
			return true;

		// vectors retrieve the pointer from their container
		case TYPE_VECTOR:
			objbase = reinterpret_cast<uintptr_t>(&(*reinterpret_cast<generic_vector *>(objbase))[0]);
			return true;

		// containers are always based at 0
		case TYPE_CONTAINER:
			objbase = 0;
			return false;

		// everything else is as-is
		default:
			return false;
	}
}


//-------------------------------------------------
//  save_binary - save this item and all owned
//  items into a binary form
//-------------------------------------------------

u64 save_registered_item::save_binary(u8 *ptr, u64 length, uintptr_t objbase) const
{
	// update the base pointer and forward if a special unwrap
	if (unwrap_and_update_base(objbase))
		return m_items.front().save_binary(ptr, length, objbase);

	// switch off the type
	u64 offset = 0;
	switch (type())
	{
		// boolean types save as a single byte
		case TYPE_BOOL:
			if (offset + 1 <= length)
				ptr[offset] = read_bool(objbase) ? 1 : 0;
			offset++;
			break;

		// integral/float types save as their native size
		case TYPE_INT:
		case TYPE_UINT:
		case TYPE_FLOAT:
			if (offset + m_native_size <= length)
				memcpy(&ptr[offset], reinterpret_cast<void const *>(objbase), m_native_size);
			offset += m_native_size;
			break;

		// structs and containers iterate over owned items
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			for (auto &item : m_items)
				offset += item.save_binary(&ptr[offset], (offset < length) ? length - offset : 0, objbase);
			break;

		// arrays are multiples of a single item
		case TYPE_STATIC_ARRAY:
		case TYPE_VECTOR_ARRAY:
		case TYPE_RAW_ARRAY:
		{
			auto item = m_items.begin();
			auto last = std::prev(m_items.end());
			for (u32 rep = 0; rep < count(); rep++)
			{
				offset += item->save_binary(&ptr[offset], (offset < length) ? length - offset : 0, objbase + rep * m_native_size);
				if (item != last)
					++item;
			}
			break;
		}
	}
	return offset;
}


//-------------------------------------------------
//  restore_binary - restore this item and all
//  owned items from binary form
//-------------------------------------------------

u64 save_registered_item::restore_binary(u8 const *ptr, u64 length, uintptr_t objbase) const
{
	// update the base pointer and forward if a trivial unwrap
	if (unwrap_and_update_base(objbase))
		return m_items.front().restore_binary(ptr, length, objbase);

	// switch off the type
	u64 offset = 0;
	switch (type())
	{
		// boolean types save as a single byte
		case TYPE_BOOL:
			if (offset + 1 <= length)
				write_bool(objbase, (ptr[offset] != 0));
			offset++;
			break;

		// integral/float types save as their native size
		case TYPE_INT:
		case TYPE_UINT:
		case TYPE_FLOAT:
			if (offset + m_native_size <= length)
				memcpy(reinterpret_cast<void *>(objbase), &ptr[offset], m_native_size);
			offset += m_native_size;
			break;

		// structs and containers iterate over owned items
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			for (auto &item : m_items)
				offset += item.restore_binary(&ptr[offset], (offset < length) ? length - offset : 0, objbase);
			break;

		// arrays are multiples of a single item
		case TYPE_STATIC_ARRAY:
		case TYPE_VECTOR_ARRAY:
		case TYPE_RAW_ARRAY:
		{
			auto item = m_items.begin();
			auto last = std::prev(m_items.end());
			for (u32 rep = 0; rep < count(); rep++)
			{
				offset += item->restore_binary(&ptr[offset], (offset < length) ? length - offset : 0, objbase + rep * m_native_size);
				if (item != last)
					++item;
			}
			break;
		}
	}
	return offset;
}


//-------------------------------------------------
//  save_json - save this item into a JSON stream
//-------------------------------------------------

void save_registered_item::save_json(save_zip_state &zipstate, char const *nameprefix, int indent, bool inline_form, uintptr_t objbase)
{
	// update the base pointer and forward if a trivial unwrap
	if (unwrap_and_update_base(objbase))
		return m_items.front().save_json(zipstate, nameprefix, indent, inline_form, objbase);

	// update the name prefix
	std::string localname = nameprefix;
	if (m_name.length() != 0)
	{
		if (localname.length() != 0)
			localname += ".";
		localname += m_name;
	}

	// output the name if present
	zipstate.json_append_name(m_name.c_str());

	// switch off the type
	switch (type())
	{
		// boolean types
		case TYPE_BOOL:
			zipstate.json_append(read_bool(objbase) ? "true" : "false");
			break;

		// signed integral types
		case TYPE_INT:
			zipstate.json_append_signed(read_int_signed(objbase, m_native_size));
			break;

		// unsigned integral types
		case TYPE_UINT:
			zipstate.json_append_unsigned(read_int_unsigned(objbase, m_native_size));
			break;

		// float types
		case TYPE_FLOAT:
			zipstate.json_append_float(read_float(objbase, m_native_size));
			break;

		// structs and containers iterate over owned items
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			if (inline_form || compute_binary_size(objbase - m_ptr_offset) <= 16)
			{
				// inline form outputs everything on a single line
				zipstate.json_append('{');
				for (auto &item : m_items)
				{
					item.save_json(zipstate, localname.c_str(), indent, true, objbase);
					if (&item != &m_items.back())
						zipstate.json_append(',');
				}
				zipstate.json_append('}');
			}
			else
			{
				// normal form outputs each item on its own line, indented
				zipstate.json_append('{').json_append_eol();
				for (auto &item : m_items)
				{
					zipstate.json_append_indent(indent + 1);
					item.save_json(zipstate, localname.c_str(), indent + 1, false, objbase);
					if (&item != &m_items.back())
						zipstate.json_append(',');
					zipstate.json_append_eol();
				}
				zipstate.json_append_indent(indent).json_append('}');
			}
			break;

		// arrays are multiples of a single item
		case TYPE_STATIC_ARRAY:
		case TYPE_VECTOR_ARRAY:
		case TYPE_RAW_ARRAY:
		{
			// look for large arrays of ints/floats
			u32 total, unitsize;
			if (is_endpoint_array(total, unitsize) && total * unitsize >= save_zip_state::JSON_EXTERNAL_BINARY_THRESHOLD)
			{
				char const *filename = zipstate.add_data_file(localname.c_str(), *this, objbase);

				zipstate.json_append('[').json_append('{');
				zipstate.json_append_name("external_file");
				zipstate.json_append('"').json_append(filename).json_append('"').json_append(',');
				zipstate.json_append_name("unit");
				zipstate.json_append_signed(unitsize).json_append(',');
				zipstate.json_append_name("count");
				zipstate.json_append_signed(total).json_append(',');
				zipstate.json_append_name("little_endian");
				zipstate.json_append((ENDIANNESS_NATIVE == ENDIANNESS_LITTLE) ? "true" : "false");
				zipstate.json_append('}').json_append(']');
			}
			else
			{
				auto item = m_items.begin();
				auto last = std::prev(m_items.end());

				// compute the size of an item to determine if we show it inline
				u32 item_size = item->compute_binary_size(objbase);
				if (inline_form || count() * item_size <= 16)
				{
					// strictly inline form outputs everything on a single line
					zipstate.json_append('[');
					for (u32 rep = 0; rep < count(); rep++)
					{
						item->save_json(zipstate, localname.c_str(), 0, true, objbase + rep * m_native_size);
						if (rep != count() - 1)
							zipstate.json_append(',');
						if (item != last)
							++item;
					}
					zipstate.json_append(']');
				}
				else
				{
					// normal form outputs a certain number of items per row
					zipstate.json_append('[').json_append_eol();
					u32 items_per_row = 1;
					if (item->is_int_or_float())
						items_per_row = (item_size <= 2) ? 32 : 16;

					// iterate over the items
					for (u32 rep = 0; rep < count(); rep++)
					{
						if (rep % items_per_row == 0)
							zipstate.json_append_indent(indent + 1);
						item->save_json(zipstate, localname.c_str(), indent + 1, false, objbase + rep * m_native_size);
						if (rep != count() - 1)
							zipstate.json_append(',');
						if (rep % items_per_row == items_per_row - 1)
							zipstate.json_append_eol();
						if (item != last)
							++item;
					}
					if (count() % items_per_row != 0)
						zipstate.json_append_eol();
					zipstate.json_append_indent(indent).json_append(']');
				}
			}
			break;
		}
	}
}


//-------------------------------------------------
//  restore_json - read data from a JSON file into
//  the target containers
//-------------------------------------------------

void save_registered_item::restore_json(load_zip_state &input, char const *nameprefix, json_restore_mode mode, uintptr_t objbase)
{
	// update the base pointer and forward if a trivial unwrap
	if (unwrap_and_update_base(objbase))
		return m_items.front().restore_json(input, nameprefix, mode, objbase);

	// update the name prefix
	std::string localname = nameprefix;
	if (m_name.length() != 0)
	{
		if (localname.length() != 0)
			localname += ".";
		localname += m_name;
	}

	// switch off the type
	switch (type())
	{
		// boolean types
		case TYPE_BOOL:
		{
			bool value = false;
			if (input.json_matches("true"))
				value = true;
			else if (input.json_matches("false"))
				value = false;
			else
				input.report_error(STATERR_MALFORMED_JSON, "%s: Unknown boolean value", localname.c_str());
			if (mode == RESTORE_DATA)
				write_bool(objbase, value);
			else if (mode == COMPARE_DATA && read_bool(objbase) != value)
				input.report_warning("%s: Compare failed: JSON says %d, data says %d", localname.c_str(), value, read_bool(objbase));
			break;
		}

		// signed integral types
		case TYPE_INT:
		{
			double dvalue;
			s64 ivalue;
			if (input.json_parse_number(dvalue))
			{
				if (mode == RESTORE_DATA && !write_int_signed(objbase, m_native_size, dvalue))
					input.report_warning("%s: Value of out range: %1.17g", localname.c_str(), dvalue);
				else if (mode == COMPARE_DATA && read_int_signed(objbase, m_native_size) != s64(dvalue))
					input.report_warning("%s: Compare failed: JSON says %I64d, data says %1.17g", localname.c_str(), dvalue, read_int_signed(objbase, m_native_size));
			}
			else if (input.json_parse_int_string(ivalue))
			{
				if (mode == RESTORE_DATA && !write_int_signed(objbase, m_native_size, ivalue))
					input.report_warning("%s: Value of out range: %I64d", localname.c_str(), ivalue);
				else if (mode == COMPARE_DATA && read_int_signed(objbase, m_native_size) != ivalue)
					input.report_warning("%s: Compare failed: JSON says %I64d, data says %I64d", localname.c_str(), ivalue, read_int_signed(objbase, m_native_size));
			}
			else
				input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: Expected integer value", localname.c_str());
			break;
		}

		// unsigned integral types
		case TYPE_UINT:
		{
			double dvalue;
			u64 ivalue;
			if (input.json_parse_number(dvalue))
			{
				if (mode == RESTORE_DATA && !write_int_unsigned(objbase, m_native_size, dvalue))
					input.report_warning("%s: Value of out range: %1.17g", localname.c_str(), dvalue);
				else if (mode == COMPARE_DATA && read_int_unsigned(objbase, m_native_size) != u64(dvalue))
					input.report_warning("%s: Compare failed: JSON says %I64d, data says %1.17g", localname.c_str(), dvalue, read_int_unsigned(objbase, m_native_size));
			}
			else if (input.json_parse_uint_string(ivalue))
			{
				if (mode == RESTORE_DATA && !write_int_unsigned(objbase, m_native_size, ivalue))
					input.report_warning("%s: Value of out range: %I64d", localname.c_str(), ivalue);
				else if (mode == COMPARE_DATA && read_int_unsigned(objbase, m_native_size) != ivalue)
					input.report_warning("%s: Compare failed: JSON says %I64d, data says %I64d", localname.c_str(), ivalue, read_int_unsigned(objbase, m_native_size));
			}
			else
				input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: Expected integer value", localname.c_str());
			break;
		}

		// float types
		case TYPE_FLOAT:
		{
			double value;
			if (!input.json_parse_number(value))
				input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: Expected number", localname.c_str());
			if (mode == RESTORE_DATA && !write_float(objbase, m_native_size, value))
				input.report_warning("%s: Value of out range: %1.17g", localname.c_str(), value);
			else if (mode == COMPARE_DATA && read_float(objbase, m_native_size) != value)
				input.report_warning("%s: Compare failed: JSON says %1.17g, data says %1.17g", localname.c_str(), value, read_float(objbase, m_native_size));
			break;
		}

		// structs and containers iterate over owned items
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			if (!input.json_matches('{'))
				input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: Expected '{'", localname.c_str());
			if (!input.json_matches('}'))
				while (1)
				{
					std::string name;
					if (!input.json_parse_string(name) || name == "")
						input.report_error(STATERR_MALFORMED_JSON, "%s: Expected name within struct", localname.c_str());
					if (!input.json_matches(':'))
						input.report_error(STATERR_MALFORMED_JSON, "%s: Expected ':'", localname.c_str());
					save_registered_item *target = find(name.c_str());
					if (target == nullptr)
						input.report_warning("%s: Found extraneous item '%s'", localname.c_str(), name.c_str());
					target->restore_json(input, localname.c_str(), (target == nullptr) ? PARSE_ONLY : mode, objbase);
					if (input.json_matches('}'))
						break;
					if (!input.json_matches(','))
						input.report_error(STATERR_MALFORMED_JSON, "%s: Expected ','", localname.c_str());
				}
			break;

		// arrays are multiples of a single item
		case TYPE_STATIC_ARRAY:
		case TYPE_VECTOR_ARRAY:
		case TYPE_RAW_ARRAY:
		{
			auto item = m_items.begin();
			auto last = std::prev(m_items.end());
			if (!input.json_matches('['))
				input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: Expected '['", localname.c_str());
			if (parse_external_data(input, *this, localname.c_str(), mode, objbase))
			{
				if (!input.json_matches(']'))
					input.report_error(STATERR_MALFORMED_JSON, "%s: Expected ']'", localname.c_str());
			}
			else
			{
				u32 rep = 0;
				if (!input.json_matches(']'))
					while (1)
					{
						item->restore_json(input, localname.c_str(), (rep >= count()) ? PARSE_ONLY : mode, objbase + rep * m_native_size);
						rep++;
						if (item != last)
							++item;
						if (input.json_matches(']'))
							break;
						if (!input.json_matches(','))
							input.report_error(STATERR_MALFORMED_JSON, "%s: Expected ','", localname.c_str());
					}
				if (rep != count())
					input.report_warning("%s: Found %s array items than expected", localname.c_str(), (rep < count()) ? "fewer" : "more");
			}
			break;
		}
	}
}


//-------------------------------------------------
//  parse_external_data - attempt to parse an
//  external file spec from a JSON file and load
//  it; returns false if not an external file spec
//-------------------------------------------------

bool save_registered_item::parse_external_data(load_zip_state &input, save_registered_item &baseitem, char const *localname, bool parseonly, uintptr_t objbase)
{
	const bool native_little_endian = (ENDIANNESS_NATIVE == ENDIANNESS_LITTLE);

	char const *pos = input.json_position();
	if (!input.json_matches('{'))
		return false;

	std::string parsed_filename;
	double parsed_unit = 0;
	double parsed_count = 0;
	bool parsed_little_endian = native_little_endian;
	bool valid = false;
	while (1)
	{
		std::string name;
		if (!input.json_parse_string(name) || name == "")
			input.report_error(STATERR_MALFORMED_JSON, "%s: Expected name within struct", localname);
		if (!input.json_matches(':'))
			input.report_error(STATERR_MALFORMED_JSON, "%s: Expected ':'", localname);
		valid = false;
		if (name == "external_file" && parsed_filename == "" && input.json_parse_string(parsed_filename) && parsed_filename != "")
			valid = true;
		else if (name == "unit" && parsed_unit == 0 && input.json_parse_number(parsed_unit) && parsed_unit != 0)
			valid = true;
		else if (name == "count" && parsed_count == 0 && input.json_parse_number(parsed_count) && parsed_count != 0)
			valid = true;
		else if (name == "little_endian")
		{
			if (input.json_matches("true"))
				parsed_little_endian = true, valid = true;
			else if (input.json_matches("false"))
				parsed_little_endian = false, valid = true;
		}
		if (!valid)
			break;
		if (input.json_matches('}'))
			break;
		if (!input.json_matches(','))
			input.report_error(STATERR_MALFORMED_JSON, "%s: Expected ','", localname);
	}
	if (!valid)
	{
		input.json_set_position(pos);
		return false;
	}

	// validate the data we found
	u64 offset;
	u32 size;
	if (!input.find_file(parsed_filename.c_str(), offset, size))
		input.report_error(STATERR_MISSING_FILE, "%s: Unable to find file '%s' in archive", localname, parsed_filename.c_str());
	if (parsed_unit != 1 && parsed_unit != 2 && parsed_unit != 4 && parsed_unit != 8)
		input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: Invalid unit size for external file, expected 1, 2, 4, or 8", localname);
	if (parsed_count == 0)
		input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: Invalid count for external file", localname);

	// look for the innermost registered item
	save_registered_item *inner = &baseitem.subitems().front();
	u32 total = count();
	while (inner->is_array())
	{
		total *= inner->count();
		inner = &inner->subitems().front();
	}
	if (!inner->is_int_or_float())
		input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: External file specified, but not valid for this type", localname);
	if (parsed_unit != inner->native_size())
		input.report_error(STATERR_INCOMPATIBLE_DATA, "%s: External file has mismatched unit size", localname);

	// if parseonly, we're done
	if (parseonly)
		return true;

	// ok time to stream the data out
	bool flip = (parsed_little_endian != native_little_endian);
	zlib_read_streamer reader(input.file());
	if (!reader.begin(offset) || !input.read_data_recursive(reader, *this, flip, size, objbase) || !reader.end())
		input.report_error(STATERR_READ_ERROR, "%s: Error reading file '%s' in archive", localname, parsed_filename.c_str());

	return true;
}


//-------------------------------------------------
//  read_int_unsigned - read an unsigned integer
//  of the given size
//-------------------------------------------------

u64 save_registered_item::read_int_unsigned(uintptr_t objbase, int size) const
{
	switch (size)
	{
		case 1:	return *reinterpret_cast<u8 const *>(objbase);
		case 2:	return *reinterpret_cast<u16 const *>(objbase);
		case 4:	return *reinterpret_cast<u32 const *>(objbase);
		case 8:	return *reinterpret_cast<u64 const *>(objbase);
	}
	return 0;
}


//-------------------------------------------------
//  read_int_signed - read a signed integer of the
//  given size
//-------------------------------------------------

s64 save_registered_item::read_int_signed(uintptr_t objbase, int size) const
{
	switch (size)
	{
		case 1:	return *reinterpret_cast<s8 const *>(objbase);
		case 2:	return *reinterpret_cast<s16 const *>(objbase);
		case 4:	return *reinterpret_cast<s32 const *>(objbase);
		case 8:	return *reinterpret_cast<s64 const *>(objbase);
	}
	return 0;
}


//-------------------------------------------------
//  read_float - read a floating-point value of the
//  given size
//-------------------------------------------------

double save_registered_item::read_float(uintptr_t objbase, int size) const
{
	switch (size)
	{
		case 4:	return *reinterpret_cast<float const *>(objbase);
		case 8:	return *reinterpret_cast<double const *>(objbase);
	}
	return 0;
}


//-------------------------------------------------
//  write_int_signed - write a signed integer of
//  the given size
//-------------------------------------------------

bool save_registered_item::write_int_signed(uintptr_t objbase, int size, s64 data) const
{
	switch (size)
	{
		case 1:	*reinterpret_cast<s8 *>(objbase) = s8(data); return (data >= -0x80 && data <= 0x7f);
		case 2:	*reinterpret_cast<s16 *>(objbase) = s16(data); return (data >= -0x8000 && data <= 0x7fff);
		case 4:	*reinterpret_cast<s32 *>(objbase) = s32(data); return (data >= -0x80000000ll && data <= 0x7fffffffll);
		case 8:	*reinterpret_cast<s64 *>(objbase) = s64(data); return true;
	}
	return false;
}

bool save_registered_item::write_int_signed(uintptr_t objbase, int size, double data) const
{
	s64 converted = s64(data);
	bool ok = (double(converted) == data);
	return write_int_signed(objbase, size, converted) && ok;
}


//-------------------------------------------------
//  write_int_unsigned - write an unsigned integer
//  of the given size
//-------------------------------------------------

bool save_registered_item::write_int_unsigned(uintptr_t objbase, int size, u64 data) const
{
	switch (size)
	{
		case 1:	*reinterpret_cast<u8 *>(objbase) = u8(data); return (data <= 0xff);
		case 2:	*reinterpret_cast<u16 *>(objbase) = u16(data); return (data <= 0xffff);
		case 4:	*reinterpret_cast<u32 *>(objbase) = u32(data); return (data <= 0xffffffffull);
		case 8:	*reinterpret_cast<u64 *>(objbase) = u64(data); return true;
	}
	return false;
}

bool save_registered_item::write_int_unsigned(uintptr_t objbase, int size, double data) const
{
	u64 converted = u64(data);
	bool ok = (data >= 0 && double(converted) == data);
	return write_int_unsigned(objbase, size, converted) && ok;
}


//-------------------------------------------------
//  write_float - write a floating-point value of
//  the given size
//-------------------------------------------------

bool save_registered_item::write_float(uintptr_t objbase, int size, double data) const
{
	switch (size)
	{
		case 4:	*reinterpret_cast<float *>(objbase) = float(data); return true;
		case 8:	*reinterpret_cast<double *>(objbase) = double(data); return true;
	}
	return false;
}


//-------------------------------------------------
//  is_endpoint_array - return true if this item
//  is a (multi-dimensional) array of an endpoint
//  type
//-------------------------------------------------

bool save_registered_item::is_endpoint_array(u32 &total, u32 &unitsize) const
{
	// we must be an array ourselves
	if (!is_array())
		return false;

	// scan downward through simple arrays
	save_registered_item const *current = this;
	total = 1;
	while (current->is_array() && current->m_items.size() == 1)
	{
		total *= current->count();
		current = &current->m_items.front();
	}

	// if we didn't end at an endpoint type, fail
	if (!current->is_int_or_float())
		return false;

	// set the unit size and return true
	unitsize = current->native_size();
	return true;
}



//**************************************************************************
//  SAVE REGISTRAR
//**************************************************************************

//-------------------------------------------------
//  save_registrar - minimal internal constructor
//-------------------------------------------------

save_registrar::save_registrar(save_registered_item &item, void *baseptr) :
	m_item(item),
	m_regcontainerbase(uintptr_t(baseptr)),
	m_regcontainersize(0)
{
}


//-------------------------------------------------
//  save_registrar - extended internal constructor
//-------------------------------------------------

save_registrar::save_registrar(save_registrar &parent, void *baseptr, save_registered_item::save_type type, u32 size, char const *name, u32 count, void *regcontainerbase, u32 regcontainersize) :
	m_item(parent.item().append(parent.ptr_to_offset(baseptr, size, type), type, size, name, count)),
	m_regcontainerbase(uintptr_t(regcontainerbase)),
	m_regcontainersize(regcontainersize)
{
}


//-------------------------------------------------
//  reg - append all items from the source 
//  registrar into a new container; note that the 
//  items are stolen, not copied
//-------------------------------------------------

save_registrar &save_registrar::reg(save_registrar &src, char const *name)
{
	save_registrar container(*this, name);
	auto &srcitems = src.item().subitems();
	auto &dstitems = container.item().subitems();
	dstitems.splice(dstitems.end(), srcitems);
	return *this;
}


//-------------------------------------------------
//  ptr_to_offset - given a pointer and size,
//  make sure the item fits within its container,
//  applying rules based on the container's type
//  and the new item's type
//-------------------------------------------------

uintptr_t save_registrar::ptr_to_offset(void *ptr, u32 size, save_registered_item::save_type type)
{
	save_registered_item::save_type parent_type = m_item.type();

	// anything you add to a container is at an absolute offset, so just return
	// the unadjusted value
	if (parent_type == save_registered_item::TYPE_CONTAINER)
		return uintptr_t(ptr);

	// if you are adding a container, the base pointer doesn't matter, so treat it as 0;
	// if you are adding a unique or vector, the pointer for owned objects comes from the
	// object itself, so also treat those as 0
	else if (type == save_registered_item::TYPE_CONTAINER || 
		parent_type == save_registered_item::TYPE_UNIQUE || 
		parent_type == save_registered_item::TYPE_VECTOR)
		return 0;

	// compute the offset relative to the container base
	uintptr_t offset = uintptr_t(ptr) - m_regcontainerbase;

	// raw arrays get a pass (though they should be rare)
	if (type == save_registered_item::TYPE_RAW_ARRAY)
	{
		osd_printf_warning("Raw array registered for saving; should be moved to a container\n");
		return offset;
	}

	// everyone else gets a full check
	if (m_regcontainersize != 0 && offset + size > m_regcontainersize)
		throw emu_fatalerror("Attempted to register item outside of parent element's bounds");
	return offset;
}



//**************************************************************************
//  SAVE MANAGER
//**************************************************************************

//-------------------------------------------------
//  save_manager - constructor
//-------------------------------------------------

save_manager::save_manager(running_machine &machine) :
	m_machine(machine),
	m_reg_allowed(true),
	m_root_registrar(m_root_item)
{
	m_rewind = std::make_unique<rewinder>(*this);
}


//-------------------------------------------------
//  allow_registration - allow/disallow
//  registrations to happen
//-------------------------------------------------

void save_manager::allow_registration(bool allowed)
{
	// allow/deny registration
	m_reg_allowed = allowed;
	if (!allowed)
	{
		// prune and sort
		m_root_item.sort_and_prune();

		// dump out a sample JSON
		{
			save_zip_state state;
			m_root_item.save_json(state);
			printf("%s\n", state.json_string());
		}

		// everything is registered by now, evaluate the savestate size
		m_rewind->clamp_capacity();
	}
}


//-------------------------------------------------
//  register_presave - register a pre-save
//  function callback
//-------------------------------------------------

void save_manager::register_presave(save_prepost_delegate func)
{
	// check for invalid timing
	if (!m_reg_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!\n");

	// scan for duplicates and push through to the end
	for (auto &cb : m_presave_list)
		if (cb->m_func == func)
			fatalerror("Duplicate save state function (%s/%s)\n", cb->m_func.name(), func.name());

	// allocate a new entry
	m_presave_list.push_back(std::make_unique<state_callback>(func));
}


//-------------------------------------------------
//  state_save_register_postload -
//  register a post-load function callback
//-------------------------------------------------

void save_manager::register_postload(save_prepost_delegate func)
{
	// check for invalid timing
	if (!m_reg_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!\n");

	// scan for duplicates and push through to the end
	for (auto &cb : m_postload_list)
		if (cb->m_func == func)
			fatalerror("Duplicate save state function (%s/%s)\n", cb->m_func.name(), func.name());

	// allocate a new entry
	m_postload_list.push_back(std::make_unique<state_callback>(func));
}


//-------------------------------------------------
//  dispatch_postload - invoke all registered
//  postload callbacks for updates
//-------------------------------------------------

void save_manager::dispatch_postload()
{
	for (auto &func : m_postload_list)
		func->m_func();
}


//-------------------------------------------------
//  dispatch_presave - invoke all registered
//  presave callbacks for updates
//-------------------------------------------------

void save_manager::dispatch_presave()
{
	for (auto &func : m_presave_list)
		func->m_func();
}


//-------------------------------------------------
//  save_binary - invoke all registered presave
//  callbacks for updates and then generate the
//  data in binary form
//-------------------------------------------------

save_error save_manager::save_binary(void *buf, size_t size)
{
	// call the pre-save functions
	dispatch_presave();

	// write the output
	u64 finalsize = m_root_item.save_binary(reinterpret_cast<u8 *>(buf), size);
	if (finalsize != size)
		return STATERR_WRITE_ERROR;

	return STATERR_NONE;
}


//-------------------------------------------------
//  load_binary - restore all data and then call
//  the postload callbacks
//-------------------------------------------------

save_error save_manager::load_binary(void *buf, size_t size)
{
	// read the input
	u64 finalsize = m_root_item.restore_binary(reinterpret_cast<u8 *>(buf), size);
	if (finalsize != size)
		return STATERR_READ_ERROR;

	// call the post-load functions
	dispatch_postload();
	return STATERR_NONE;
}


//-------------------------------------------------
//  save_file - invoke all registered presave
//  callbacks for updates and then generate the
//  data in JSON/ZIP form
//-------------------------------------------------

save_error save_manager::save_file(emu_file &file)
{
	// call the pre-save functions
	dispatch_presave();

	// create the JSON and target all the output files
	save_zip_state state;
	m_root_item.save_json(state);

	// then commit the state to the file
	return state.commit(file) ? STATERR_NONE : STATERR_WRITE_ERROR;
}


//-------------------------------------------------
//  load_file - restore all data and then call
//  the postload callbacks
//-------------------------------------------------

save_error save_manager::load_file(emu_file &file)
{
	// create the JSON and target all the output files
	load_zip_state state(file);
	save_error err = state.init();
	if (err != STATERR_NONE)
		return err;

	// restore_json will throw on parse errors and the like
	try
	{
		m_root_item.restore_json(state);
		char const *warnings = state.warnings();
		if (warnings != nullptr)
		{
			osd_printf_warning("WARNINGS during state load:\n%s", warnings);
			err = STATERR_MISMATCH_WARNING;
		}
	}
	catch (load_zip_state::load_error &loaderr)
	{
		char const *errors = state.errors();
		if (errors != nullptr)
			osd_printf_error("ERRORS during state load:\n%s", errors);
		return loaderr.error();
	}

	// call the post-load functions
	dispatch_postload();
	return err;
}


//-------------------------------------------------
//  compare_file - act as if we're loading the
//  file, but just compare data instead
//-------------------------------------------------

save_error save_manager::compare_file(emu_file &file)
{
	// create the JSON and target all the output files
	load_zip_state state(file);
	save_error err = state.init();
	if (err != STATERR_NONE)
		return err;

	// restore_json will throw on parse errors and the like
	try
	{
		m_root_item.restore_json(state, "", save_registered_item::COMPARE_DATA);
		char const *warnings = state.warnings();
		if (warnings != nullptr)
		{
			osd_printf_warning("WARNINGS during state compare:\n%s", warnings);
			err = STATERR_MISMATCH_WARNING;
		}
	}
	catch (load_zip_state::load_error &loaderr)
	{
		char const *errors = state.errors();
		if (errors != nullptr)
			osd_printf_error("ERRORS during state compare:\n%s", errors);
		return loaderr.error();
	}
	return err;
}



//**************************************************************************
//  RAM STATE
//**************************************************************************

//-------------------------------------------------
//  ram_state - constructor
//-------------------------------------------------

ram_state::ram_state(save_manager &save) :
	m_valid(false),
	m_time(m_save.machine().time()),
	m_save(save)
{
}


//-------------------------------------------------
//  save - write the current machine state to the
//  allocated stream
//-------------------------------------------------

save_error ram_state::save()
{
	// initialize
	m_valid = false;

	// get the save manager to write state
	const save_error err = m_save.save_binary(m_data);
	if (err != STATERR_NONE)
		return err;

	// final confirmation
	m_valid = true;
	m_time = m_save.machine().time();

	return STATERR_NONE;
}


//-------------------------------------------------
//  load - restore the machine state from the
//  stream
//-------------------------------------------------

save_error ram_state::load()
{
	// get the save manager to load state
	return m_save.load_binary(m_data);
}



//**************************************************************************
//  REWINDER
//**************************************************************************

//-------------------------------------------------
//  rewinder - constuctor
//-------------------------------------------------

rewinder::rewinder(save_manager &save) :
	m_save(save),
	m_enabled(save.machine().options().rewind()),
	m_capacity(save.machine().options().rewind_capacity()),
	m_current_index(REWIND_INDEX_NONE),
	m_first_invalid_index(REWIND_INDEX_NONE),
	m_first_time_warning(true),
	m_first_time_note(true)
{
}


//-------------------------------------------------
//  clamp_capacity - safety checks for commandline
//  override
//-------------------------------------------------

void rewinder::clamp_capacity()
{
	if (!m_enabled)
		return;

	const size_t total = m_capacity * 1024 * 1024;
	const size_t single = m_save.binary_size();

	// can't set below zero, but allow commandline to override options' upper limit
	if (total < 0)
		m_capacity = 0;

	// if capacity is below savestate size, can't save anything
	if (total < single)
	{
		m_enabled = false;
		m_save.machine().logerror("Rewind has been disabled, because rewind capacity is smaller than savestate size.\n");
		m_save.machine().logerror("Rewind buffer size: %d bytes. Savestate size: %d bytes.\n", total, single);
		m_save.machine().popmessage("Rewind has been disabled. See error.log for details");
	}
}


//-------------------------------------------------
//  invalidate - mark all the future states as
//  invalid to prevent loading them, as the
//  current input might have changed
//-------------------------------------------------

void rewinder::invalidate()
{
	if (!m_enabled)
		return;

	// is there anything to invalidate?
	if (!current_index_is_last())
	{
		// all states starting from the current one will be invalid
		m_first_invalid_index = m_current_index;

		// actually invalidate
		for (auto it = m_state_list.begin() + m_first_invalid_index; it < m_state_list.end(); ++it)
			it->get()->m_valid = false;
	}
}


//-------------------------------------------------
//  capture - record a single state, returns true
//  on success
//-------------------------------------------------

bool rewinder::capture()
{
	if (!m_enabled)
	{
		report_error(STATERR_DISABLED, rewind_operation::SAVE);
		return false;
	}

	if (current_index_is_last())
	{
		// we need to create a new state
		std::unique_ptr<ram_state> state = std::make_unique<ram_state>(m_save);
		const save_error error = state->save();

		// validate the state
		if (error == STATERR_NONE)
			// it's safe to append
			m_state_list.push_back(std::move(state));
		else
		{
			// internal error, complain and evacuate
			report_error(error, rewind_operation::SAVE);
			return false;
		}
	}
	else
	{
		// invalidate the future states
		invalidate();

		// update the existing state
		ram_state *state = m_state_list.at(m_current_index).get();
		const save_error error = state->save();

		// validate the state
		if (error != STATERR_NONE)
		{
			// internal error, complain and evacuate
			report_error(error, rewind_operation::SAVE);
			return false;
		}
	}

	// make sure we will fit in
	if (!check_size())
		// the list keeps growing
		m_current_index++;

	// update first invalid index
	if (current_index_is_last())
		m_first_invalid_index = REWIND_INDEX_NONE;
	else
		m_first_invalid_index = m_current_index + 1;

	// success
	report_error(STATERR_NONE, rewind_operation::SAVE);
	return true;
}


//-------------------------------------------------
//  step - single step back in time, returns true
//  on success
//-------------------------------------------------

bool rewinder::step()
{
	if (!m_enabled)
	{
		report_error(STATERR_DISABLED, rewind_operation::LOAD);
		return false;
	}

	// do we have states to load?
	if (m_current_index <= REWIND_INDEX_FIRST || m_first_invalid_index == REWIND_INDEX_FIRST)
	{
		// no valid states, complain and evacuate
		report_error(STATERR_NOT_FOUND, rewind_operation::LOAD);
		return false;
	}

	// prepare to load the last valid index if we're too far ahead
	if (m_first_invalid_index > REWIND_INDEX_NONE && m_current_index > m_first_invalid_index)
		m_current_index = m_first_invalid_index;

	// step back and obtain the state pointer
	ram_state *state = m_state_list.at(--m_current_index).get();

	// try to load and report the result
	const save_error error = state->load();
	report_error(error, rewind_operation::LOAD);

	if (error == save_error::STATERR_NONE)
		return true;

	return false;
}


//-------------------------------------------------
//  check_size - shrink the state list if it is
//  about to hit the capacity. returns true if
//  the list got shrank
//-------------------------------------------------

bool rewinder::check_size()
{
	if (!m_enabled)
		return false;

	// state sizes in bytes
	const size_t singlesize = m_save.binary_size();
	size_t totalsize = m_state_list.size() * singlesize;

	// convert our limit from megabytes
	const size_t capsize = m_capacity * 1024 * 1024;

	// safety check that shouldn't be allowed to trigger
	if (totalsize > capsize)
	{
		// states to remove
		const u32 count = (totalsize - capsize) / singlesize;

		// drop everything that's beyond capacity
		m_state_list.erase(m_state_list.begin(), m_state_list.begin() + count);
	}

	// update before new check
	totalsize = m_state_list.size() * singlesize;

	// check if capacity will be hit by the newly captured state
	if (totalsize + singlesize >= capsize)
	{
		// check if we have spare states ahead
		if (!current_index_is_last())
			// no need to move states around
			return false;

		// we can now get the first state and invalidate it
		std::unique_ptr<ram_state> first(std::move(m_state_list.front()));
		first->m_valid = false;

		// move it to the end for future use
		m_state_list.push_back(std::move(first));
		m_state_list.erase(m_state_list.begin());

		if (m_first_time_note)
		{
			m_save.machine().logerror("Rewind note: Capacity has been reached. Old savestates will be erased.\n");
			m_save.machine().logerror("Capacity: %d bytes. Savestate size: %d bytes. Savestate count: %d.\n",
				totalsize, singlesize, m_state_list.size());
			m_first_time_note = false;
		}

		return true;
	}

	return false;
}


//-------------------------------------------------
//  report_error - report rewind results
//-------------------------------------------------

void rewinder::report_error(save_error error, rewind_operation operation)
{
	const char *const opname = (operation == rewind_operation::LOAD) ? "load" : "save";
	switch (error)
	{
	// internal saveload failures
	case STATERR_INVALID_FILE:
		m_save.machine().logerror("Rewind error: Unable to %s state due to an invalid file. "
			"Make sure the save state is correct for this machine.\n", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;

	case STATERR_READ_ERROR:
		m_save.machine().logerror("Rewind error: Unable to %s state due to a read error.\n", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;

	case STATERR_WRITE_ERROR:
		m_save.machine().logerror("Rewind error: Unable to %s state due to a write error.\n", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;

	// external saveload failures
	case STATERR_NOT_FOUND:
		if (operation == rewind_operation::LOAD)
		{
			m_save.machine().logerror("Rewind error: No rewind state to load.\n");
			m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		}
		break;

	case STATERR_DISABLED:
		if (operation == rewind_operation::LOAD)
		{
			m_save.machine().logerror("Rewind error: Rewind is disabled.\n");
			m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		}
		break;

	// success
	case STATERR_NONE:
		{
			const u64 supported = m_save.machine().system().flags & MACHINE_SUPPORTS_SAVE;
			const char *const warning = supported || !m_first_time_warning ? "" :
				"Rewind warning: Save states are not officially supported for this machine.\n";
			const char *const opnamed = (operation == rewind_operation::LOAD) ? "loaded" : "captured";

			// for rewinding outside of debugger, give some indication that rewind has worked, as screen doesn't update
			m_save.machine().popmessage("Rewind state %i %s.\n%s", m_current_index + 1, opnamed, warning);
			if (m_first_time_warning && operation == rewind_operation::LOAD && !supported)
			{
				m_save.machine().logerror(warning);
				m_first_time_warning = false;
			}
		}
		break;

	// something that shouldn't be allowed to happen
	default:
		m_save.machine().logerror("Error: Unknown error during state %s.\n", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;
	}
}
