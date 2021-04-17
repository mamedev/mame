// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    imgtool.h

    Main headers for Imgtool core

***************************************************************************/

#ifndef IMGTOOL_H
#define IMGTOOL_H

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <functional>

#include "corestr.h"
#include "formats/flopimg.h"
#include "opresolv.h"
#include "library.h"
#include "filter.h"
#include "osdcomm.h"

/* ----------------------------------------------------------------------- */

#define EOLN_CR     "\x0d"
#define EOLN_LF     "\x0a"
#define EOLN_CRLF   "\x0d\x0a"

#define FILENAME_BOOTBLOCK  ((const char *) 1)

enum
{
	OSD_FOPEN_READ,
	OSD_FOPEN_WRITE,
	OSD_FOPEN_RW,
	OSD_FOPEN_RW_CREATE
};

/* ---------------------------------------------------------------------------
 * Image calls
 *
 * These are the calls that front ends should use for manipulating images. You
 * should never call the module functions directly because they may not be
 * implemented (i.e. - the function pointers are NULL). The img_* functions are
 * aware of these issues and will make the appropriate checks as well as
 * marking up return codes with the source.  In addition, some of the img_*
 * calls are high level calls that simply image manipulation
 *
 * Calls that return 'int' that are not explicitly noted otherwise return
 * imgtool error codes
 * ---------------------------------------------------------------------------
 */

struct imgtool_module_features
{
	unsigned int supports_create : 1;
	unsigned int supports_open : 1;
	unsigned int supports_readsector : 1;
	unsigned int supports_writesector : 1;
	unsigned int is_read_only : 1;
};

struct imgtool_partition_features
{
	unsigned int supports_reading : 1;
	unsigned int supports_writing : 1;
	unsigned int supports_deletefile : 1;
	unsigned int supports_directories : 1;
	unsigned int supports_freespace : 1;
	unsigned int supports_createdir : 1;
	unsigned int supports_deletedir : 1;
	unsigned int supports_creation_time : 1;
	unsigned int supports_lastmodified_time : 1;
	unsigned int supports_forks : 1;
	unsigned int supports_geticoninfo : 1;
	unsigned int is_read_only : 1;
};

/* ----- initialization and basics ----- */
void imgtool_init(bool omit_untested, void (*warning)(const char *message));
void imgtool_exit(void);
const imgtool_module *imgtool_find_module(const std::string &modulename);
const imgtool::library::modulelist &imgtool_get_modules();
imgtool_module_features imgtool_get_module_features(const imgtool_module *module);
void imgtool_warn(const char *format, ...) ATTR_PRINTF(1,2);

// ----- image management -----
namespace imgtool
{
	class image
	{
	public:
		typedef std::unique_ptr<image> ptr;

		image(const imgtool_module &module, object_pool *pool, void *extra_bytes);
		~image();

		static imgtoolerr_t identify_file(const char *filename, imgtool_module **modules, size_t count);
		static imgtoolerr_t open(const imgtool_module *module, const std::string &filename, int read_or_write, ptr &outimg);
		static imgtoolerr_t open(const std::string &modulename, const std::string &filename, int read_or_write, ptr &outimg);
		static imgtoolerr_t create(const imgtool_module *module, const std::string &filename, util::option_resolution *opts, ptr &image);
		static imgtoolerr_t create(const std::string &modulename, const std::string &filename, util::option_resolution *opts, ptr &image);
		static imgtoolerr_t create(const imgtool_module *module, const std::string &filename, util::option_resolution *opts);
		static imgtoolerr_t create(const std::string &modulename, const std::string &filename, util::option_resolution *opts);

		std::string info();
		imgtoolerr_t get_geometry(uint32_t *tracks, uint32_t *heads, uint32_t *sectors);
		imgtoolerr_t read_sector(uint32_t track, uint32_t head, uint32_t sector, std::vector<uint8_t> &buffer);
		imgtoolerr_t write_sector(uint32_t track, uint32_t head, uint32_t sector, const void *buffer, size_t len);
		imgtoolerr_t get_block_size(uint32_t &length);
		imgtoolerr_t read_block(uint64_t block, void *buffer);
		imgtoolerr_t write_block(uint64_t block, const void *buffer);
		imgtoolerr_t clear_block(uint64_t block, uint8_t data);
		imgtoolerr_t list_partitions(std::vector<imgtool::partition_info> &partitions);
		void *malloc(size_t size);
		const imgtool_module &module() { return m_module; }
		void *extra_bytes() { return m_extra_bytes; }

	private:
		const imgtool_module &m_module;
		object_pool *m_pool;
		void *m_extra_bytes;

		// because of an idiosyncrasy of how imgtool::image::internal_open() works, we are only "okay to close"
		// by invoking the module's close function once internal_open() succeeds.  the long term solution is
		// better C++ adoption (e.g. - std::unique_ptr<>, std:move() etc)
		bool m_okay_to_close;

		static imgtoolerr_t internal_open(const imgtool_module *module, const std::string &filename,
			int read_or_write, util::option_resolution *createopts, imgtool::image::ptr &outimg);
	};
}

namespace imgtool
{
	// ----- partition management -----
	class partition
	{
		friend class directory;
	public:
		typedef std::unique_ptr<partition> ptr;

		// ctor/dtor
		partition(imgtool::image &image, const imgtool_class &imgclass, int partition_index, uint64_t base_block, uint64_t block_count);
		~partition();

		static imgtoolerr_t open(imgtool::image &image, int partition_index, ptr &partition);
		imgtool::image &image() { return m_image; }

		// ----- partition operations -----
		imgtoolerr_t get_directory_entry(const char *path, int index, imgtool_dirent &ent);
		imgtoolerr_t get_file_size(const char *filename, uint64_t &filesize);
		imgtoolerr_t get_free_space(uint64_t &sz);
		imgtoolerr_t read_file(const char *filename, const char *fork, imgtool::stream &destf, filter_getinfoproc filter);
		imgtoolerr_t write_file(const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *resolution, filter_getinfoproc filter);
		imgtoolerr_t get_file(const char *filename, const char *fork, const char *dest, filter_getinfoproc filter);
		imgtoolerr_t put_file(const char *newfname, const char *fork, const char *source, util::option_resolution *opts, filter_getinfoproc filter);
		imgtoolerr_t delete_file(const char *fname);
		imgtoolerr_t list_file_forks(const char *path, std::vector<imgtool::fork_entry> &forks);
		imgtoolerr_t create_directory(const char *path);
		imgtoolerr_t delete_directory(const char *path);
		imgtoolerr_t list_file_attributes(const char *path, uint32_t *attrs, size_t len);
		imgtoolerr_t get_file_attributes(const char *path, const uint32_t *attrs, imgtool_attribute *values);
		imgtoolerr_t put_file_attributes(const char *path, const uint32_t *attrs, const imgtool_attribute *values);
		imgtoolerr_t get_file_attribute(const char *path, uint32_t attr, imgtool_attribute &value);
		imgtoolerr_t put_file_attribute(const char *path, uint32_t attr, const imgtool_attribute &value);
		void         get_attribute_name(uint32_t attribute, const imgtool_attribute *attr_value, char *buffer, size_t buffer_len);
		imgtoolerr_t get_icon_info(const char *path, imgtool_iconinfo *iconinfo);
		imgtoolerr_t suggest_file_filters(const char *path, imgtool::stream *stream, imgtool_transfer_suggestion *suggestions, size_t suggestions_length);
		imgtoolerr_t get_block_size(uint32_t &length);
		imgtoolerr_t read_block(uint64_t block, void *buffer);
		imgtoolerr_t write_block(uint64_t block, const void *buffer);
		imgtoolerr_t get_chain(const char *path, imgtool_chainent *chain, size_t chain_size);
		imgtoolerr_t get_chain_string(const char *path, char *buffer, size_t buffer_len);
		imgtool_partition_features get_features() const;
		void *       get_info_ptr(uint32_t state);
		const char * get_info_string(uint32_t state);
		uint64_t       get_info_int(uint32_t state);
		void *       extra_bytes();

		// ----- path management -----
		const char * get_root_path();
		const char * path_concatenate(const char *path1, const char *path2);
		const char * get_base_name(const char *path);

	private:
		imgtool::image &m_image;
		//int m_partition_index;
		uint64_t m_base_block;
		uint64_t m_block_count;

		imgtool_class m_imgclass;
		size_t m_directory_extra_bytes;

		char m_path_separator;
		char m_alternate_path_separator;

		unsigned int m_prefer_ucase : 1;
		unsigned int m_supports_creation_time : 1;
		unsigned int m_supports_lastmodified_time : 1;
		unsigned int m_supports_bootblock : 1;            /* this module supports loading/storing the boot block */

		std::function<imgtoolerr_t(imgtool::directory &enumeration, const char *path)> m_begin_enum;
		std::function<imgtoolerr_t(imgtool::directory &enumeration, imgtool_dirent &ent)> m_next_enum;
		std::function<void(imgtool::directory &enumeration)> m_close_enum;
		std::function<imgtoolerr_t(imgtool::partition &partition, uint64_t *size)> m_free_space;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)> m_read_file;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)> m_write_file;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *filename)> m_delete_file;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path, std::vector<imgtool::fork_entry> &forks)> m_list_forks;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path)> m_create_dir;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path)> m_delete_dir;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path, uint32_t *attrs, size_t len)> m_list_attrs;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path, const uint32_t *attrs, imgtool_attribute *values)> m_get_attrs;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path, const uint32_t *attrs, const imgtool_attribute *values)> m_set_attrs;
		std::function<imgtoolerr_t(uint32_t attribute, const imgtool_attribute *attr, char *buffer, size_t buffer_len)> m_attr_name;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path, imgtool_iconinfo *iconinfo)> m_get_iconinfo;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)> m_suggest_transfer;
		std::function<imgtoolerr_t(imgtool::partition &partition, const char *path, imgtool_chainent *chain, size_t chain_size)> m_get_chain;

		const util::option_guide *m_writefile_optguide;
		std::string m_writefile_optspec;

		std::unique_ptr<uint8_t[]> m_extra_bytes;

		// methods
		imgtoolerr_t canonicalize_path(uint32_t flags, const char *path, std::string &result);
		imgtoolerr_t canonicalize_fork(const char **fork);
		imgtoolerr_t map_block_to_image_block(uint64_t partition_block, uint64_t &image_block) const;
	};

	// ----- directory management -----
	class directory
	{
	public:
		typedef std::unique_ptr<directory> ptr;

		// ctor/dtor
		directory(imgtool::partition &partition);
		~directory();

		// methods
		static imgtoolerr_t open(imgtool::partition &partition, const std::string &path, ptr &outenum);
		imgtoolerr_t get_next(imgtool_dirent &ent);

		// accessors
		imgtool::partition &partition() { return m_partition; }
		imgtool::image &image() { return partition().image(); }
		const imgtool_module &module() { return partition().image().module(); }
		void *extra_bytes() { assert(m_extra_bytes); return m_extra_bytes.get(); }

	private:
		imgtool::partition &m_partition;
		std::unique_ptr<uint8_t[]> m_extra_bytes;
		bool m_okay_to_close;   // similar wart as what is on imgtool::image
	};
};

/* ----- special ----- */
bool imgtool_validitychecks(void);
void unknown_partition_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info);

char *strncpyz(char *dest, const char *source, size_t len);
void rtrim(char *buf);
std::string extract_padded_filename(const char *source, size_t filename_length, size_t extension_length, char pad = ' ');

#endif /* IMGTOOL_H */
