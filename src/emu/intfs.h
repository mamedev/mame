// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Embedded virtual filesystem manager

#ifndef MAME_EMU_INTFS_H
#define MAME_EMU_INTFS_H

namespace intfs {
    struct file_entry {
	const char *file_name;
	const unsigned char *data;
	unsigned int compressed_size;
	unsigned int uncompressed_size;
    };

    struct dir_entry {
	const char *dir_name;
	const struct dir_entry *parent;
	const struct dir_entry *dirs;
	const struct file_entry *files;
	unsigned int dir_count;
	unsigned int file_count;
    };

    using root_entry = dir_entry;

    std::pair<std::unique_ptr<u8[]>, size_t> get_file(const root_entry &root, std::string filename);
};

#endif
