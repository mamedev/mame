// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Embedded virtual filesystem manager

#include "emu.h"
#include "intfs.h"

#include <zlib.h>

namespace intfs {
    template<typename S, typename L> const S *search(const S *array, unsigned int count, std::string filename, L get_name)
	{
		if(!count)
			return nullptr;
		unsigned int last_index = count - 1;
		unsigned int fill1  = last_index | (last_index >> 1);
		unsigned int fill2  = fill1 | (fill1 >> 2);
		unsigned int fill4  = fill2 | (fill2 >> 4);
		unsigned int fill8  = fill4 | (fill4 >> 8);
		unsigned int fill16 = fill8 | (fill8 >> 16);
		unsigned int ppow2  = fill16 - (fill16 >> 1);
		unsigned int slot = ppow2;
		unsigned int step = ppow2;
		while(step) {
			if(slot > last_index)
				slot = slot ^ (step | (step >> 1));
			else {
				int cmp = strcmp(filename.c_str(), get_name(array+slot));
				if(!cmp)
					return array + slot;
				if(cmp > 0)
					slot = slot | (step >> 1);
				else
					slot = slot ^ (step | (step >> 1));
			}
			step = step >> 1;
		}
		int cmp = strcmp(filename.c_str(), get_name(array+slot));
		if(!cmp)
			return array + slot;
		return nullptr;
    }

	const file_entry *find_file(const dir_entry *dir, std::string filename)
	{
		return search(dir->files, dir->file_count, filename, [](const file_entry *file) { return file->file_name; });
	}

	const dir_entry *find_dir(const dir_entry *dir, std::string dirname)
	{
		if(dirname.empty())
			return dir;
		return search(dir->dirs, dir->dir_count, dirname, [](const dir_entry *dir) { return dir->dir_name; });
	}

    std::unique_ptr<u8[]> unpack(const file_entry *file)
	{
		// +1 to ensure data is terminated for text/XML parsers
		auto tempout = make_unique_clear<u8 []>(file->uncompressed_size + 1);

		z_stream stream;
		int zerr;

		// initialize the stream
		memset(&stream, 0, sizeof(stream));
		stream.next_out = tempout.get();
		stream.avail_out = file->uncompressed_size;

		zerr = inflateInit(&stream);
		if(zerr != Z_OK) {
			fatalerror("could not inflateInit");
			return nullptr;
		}

		// decompress this chunk
		stream.next_in = const_cast<unsigned char *>(file->data);
		stream.avail_in = file->compressed_size;
		zerr = inflate(&stream, Z_NO_FLUSH);

		// stop at the end of the stream
		if(zerr != Z_STREAM_END && zerr != Z_OK) {
			fatalerror("decompression error\n");
			return nullptr;
		}

		// clean up
		zerr = inflateEnd(&stream);
		if (zerr != Z_OK) {
			fatalerror("inflateEnd error\n");
			return nullptr;
		}

		return tempout;
    }
}

std::pair<std::unique_ptr<u8[]>, size_t> intfs::get_file(const root_entry &root, std::string filename)
{
	const dir_entry *dir = &root;
	size_t pos = 0;
	if(filename[0] == '/')
		pos++;
	for(;;) {
		size_t pos2 = filename.find('/', pos);
		if(pos2 == std::string::npos)
			break;
		dir = find_dir(dir, filename.substr(pos, pos2 - pos));
		if(!dir)
			return std::pair<std::unique_ptr<u8[]>, size_t>(nullptr, 0);
		pos = pos2 + 1;
	}

	auto file = find_file(dir, filename.substr(pos));
	if(file)
		return std::make_pair(unpack(file), file->uncompressed_size);
	return std::pair<std::unique_ptr<u8[]>, size_t>(nullptr, 0);
}

