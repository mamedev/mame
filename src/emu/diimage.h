// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    diimage.h

    Device image interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DIIMAGE_H
#define MAME_EMU_DIIMAGE_H

#include <memory>
#include <string>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

extern struct io_procs image_ioprocs;

enum iodevice_t
{
	/* List of all supported devices.  Refer to the device by these names only */
	IO_UNKNOWN,
	IO_CARTSLOT,    /*  1 - Cartridge Port, as found on most console and on some computers */
	IO_FLOPPY,      /*  2 - Floppy Disk unit */
	IO_HARDDISK,    /*  3 - Hard Disk unit */
	IO_CYLINDER,    /*  4 - Magnetically-Coated Cylinder */
	IO_CASSETTE,    /*  5 - Cassette Recorder (common on early home computers) */
	IO_PUNCHCARD,   /*  6 - Card Puncher/Reader */
	IO_PUNCHTAPE,   /*  7 - Tape Puncher/Reader (reels instead of punchcards) */
	IO_PRINTER,     /*  8 - Printer device */
	IO_SERIAL,      /*  9 - Generic Serial Port */
	IO_PARALLEL,    /* 10 - Generic Parallel Port */
	IO_SNAPSHOT,    /* 11 - Complete 'snapshot' of the state of the computer */
	IO_QUICKLOAD,   /* 12 - Allow to load program/data into memory, without matching any actual device */
	IO_MEMCARD,     /* 13 - Memory card */
	IO_CDROM,       /* 14 - optical CD-ROM disc */
	IO_MAGTAPE,     /* 15 - Magnetic tape */
	IO_ROM,         /* 16 - Individual ROM image - the Amstrad CPC has a few applications that were sold on 16kB ROMs */
	IO_MIDIIN,      /* 17 - MIDI In port */
	IO_MIDIOUT,     /* 18 - MIDI Out port */
	IO_PICTURE,     /* 19 - A single-frame image */
	IO_COUNT        /* 20 - Total Number of IO_devices for searching */
};

enum image_error_t
{
	IMAGE_ERROR_SUCCESS,
	IMAGE_ERROR_INTERNAL,
	IMAGE_ERROR_UNSUPPORTED,
	IMAGE_ERROR_OUTOFMEMORY,
	IMAGE_ERROR_FILENOTFOUND,
	IMAGE_ERROR_INVALIDIMAGE,
	IMAGE_ERROR_ALREADYOPEN,
	IMAGE_ERROR_UNSPECIFIED
};

struct image_device_type_info
{
	iodevice_t m_type;
	const char *m_name;
	const char *m_shortname;
};

class image_device_format
{
public:
	image_device_format(const std::string &name, const std::string &description, const std::string &extensions, const std::string &optspec);
	~image_device_format();

	const std::string &name() const { return m_name; }
	const std::string &description() const { return m_description; }
	const std::vector<std::string> &extensions() const { return m_extensions; }
	const std::string &optspec() const { return m_optspec; }

private:
	std::string                 m_name;
	std::string                 m_description;
	std::vector<std::string>    m_extensions;
	std::string                 m_optspec;
};


enum class image_init_result { PASS, FAIL };
enum class image_verify_result { PASS, FAIL };

//**************************************************************************
//  MACROS
//**************************************************************************

#define DEVICE_IMAGE_LOAD_MEMBER(_name)             image_init_result _name(device_image_interface &image)
#define DECLARE_DEVICE_IMAGE_LOAD_MEMBER(_name)     DEVICE_IMAGE_LOAD_MEMBER(_name)

#define DEVICE_IMAGE_UNLOAD_MEMBER(_name)           void _name(device_image_interface &image)
#define DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(_name)   DEVICE_IMAGE_UNLOAD_MEMBER(_name)


// ======================> device_image_interface

// class representing interface-specific live image
class device_image_interface : public device_interface
{
public:
	typedef device_delegate<image_init_result (device_image_interface &)> load_delegate;
	typedef device_delegate<void (device_image_interface &)> unload_delegate;

	typedef std::vector<std::unique_ptr<image_device_format>> formatlist_type;

	// construction/destruction
	device_image_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_image_interface();

	static const char *device_typename(iodevice_t type);
	static const char *device_brieftypename(iodevice_t type);
	static iodevice_t device_typeid(const char *name);

	virtual image_init_result call_load() { return image_init_result::PASS; }
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) { return image_init_result::PASS; }
	virtual void call_unload() { }
	virtual std::string call_display() { return std::string(); }
	virtual u32 unhashed_header_length() const { return 0; }
	virtual bool core_opens_image_file() const { return true; }
	virtual iodevice_t image_type()  const = 0;
	virtual bool is_readable()  const = 0;
	virtual bool is_writeable() const = 0;
	virtual bool is_creatable() const = 0;
	virtual bool must_be_loaded() const = 0;
	virtual bool is_reset_on_load() const = 0;
	virtual bool support_command_line_image_creation() const;
	virtual const char *image_interface() const { return nullptr; }
	virtual const char *file_extensions() const = 0;
	virtual const util::option_guide &create_option_guide() const;
	virtual const char *custom_instance_name() const { return nullptr; }
	virtual const char *custom_brief_instance_name() const { return nullptr; }

	const image_device_format *device_get_indexed_creatable_format(int index) const { if (index < m_formatlist.size()) return m_formatlist.at(index).get(); else return nullptr;  }
	const image_device_format *device_get_named_creatable_format(const std::string &format_name);
	const util::option_guide &device_get_creation_option_guide() const { return create_option_guide(); }

	const char *error();
	void seterror(image_error_t err, const char *message);
	void message(const char *format, ...) ATTR_PRINTF(2,3);

	bool exists() const { return !m_image_name.empty(); }
	const char *filename() const { if (m_image_name.empty()) return nullptr; else return m_image_name.c_str(); }
	const char *basename() const { if (m_basename.empty()) return nullptr; else return m_basename.c_str(); }
	const char *basename_noext()  const { if (m_basename_noext.empty()) return nullptr; else return m_basename_noext.c_str(); }
	const std::string &filetype() const { return m_filetype; }
	bool is_filetype(const std::string &candidate_filetype) { return !core_stricmp(filetype().c_str(), candidate_filetype.c_str()); }
	bool is_open() const { return bool(m_file); }
	util::core_file &image_core_file() const { return *m_file; }
	u64 length() { check_for_file(); return m_file->size(); }
	bool is_readonly() const { return m_readonly; }
	u32 fread(void *buffer, u32 length) { check_for_file(); return m_file->read(buffer, length); }
	u32 fread(optional_shared_ptr<u8> &ptr, u32 length) { ptr.allocate(length); return fread(ptr.target(), length); }
	u32 fread(optional_shared_ptr<u8> &ptr, u32 length, offs_t offset) { ptr.allocate(length); return fread(ptr + offset, length - offset); }
	u32 fwrite(const void *buffer, u32 length) { check_for_file(); return m_file->write(buffer, length); }
	int fseek(s64 offset, int whence) { check_for_file(); return m_file->seek(offset, whence); }
	u64 ftell() { check_for_file(); return m_file->tell(); }
	int fgetc() { char ch; if (fread(&ch, 1) != 1) ch = '\0'; return ch; }
	char *fgets(char *buffer, u32 length) { check_for_file(); return m_file->gets(buffer, length); }
	int image_feof() { check_for_file(); return m_file->eof(); }
	void *ptr() {check_for_file(); return const_cast<void *>(m_file->buffer()); }
	// configuration access

	const std::string &longname() const { return m_longname; }
	const std::string &manufacturer() const { return m_manufacturer; }
	const std::string &year() const { return m_year; }
	u32 supported() const { return m_supported; }

	const software_info *software_entry() const;
	const software_part *part_entry() const { return m_software_part_ptr; }
	const char *software_list_name() const { return m_software_list_name.c_str(); }
	bool loaded_through_softlist() const { return m_software_part_ptr != nullptr; }

	void set_working_directory(const char *working_directory) { m_working_directory = working_directory; }
	const std::string &working_directory();

	u8 *get_software_region(const char *tag);
	u32 get_software_region_length(const char *tag);
	const char *get_feature(const char *feature_name) const;
	bool load_software_region(const char *tag, optional_shared_ptr<u8> &ptr);

	u32 crc();
	util::hash_collection& hash() { return m_hash; }
	util::hash_collection calculate_hash_on_file(util::core_file &file) const;

	void battery_load(void *buffer, int length, int fill);
	void battery_load(void *buffer, int length, void *def_buffer);
	void battery_save(const void *buffer, int length);

	const char *image_type_name()  const { return device_typename(image_type()); }

	const std::string &instance_name() const { return m_instance_name; }
	const std::string &brief_instance_name() const { return m_brief_instance_name; }
	const std::string &canonical_instance_name() const { return m_canonical_instance_name; }
	bool uses_file_extension(const char *file_extension) const;
	const formatlist_type &formatlist() const { return m_formatlist; }

	// loads an image file
	image_init_result load(const std::string &path);

	// loads a softlist item by name
	image_init_result load_software(const std::string &software_identifier);

	image_init_result finish_load();
	void unload();
	image_init_result create(const std::string &path, const image_device_format *create_format, util::option_resolution *create_args);
	image_init_result create(const std::string &path);
	bool load_software(software_list_device &swlist, const char *swname, const rom_entry *entry);
	int reopen_for_write(const std::string &path);

	void set_user_loadable(bool user_loadable) { m_user_loadable = user_loadable; }

	bool user_loadable() const { return m_user_loadable; }
	bool is_reset_and_loading() const { return m_is_reset_and_loading; }
	const std::string &full_software_name() const { return m_full_software_name; }

protected:
	// interface-level overrides
	virtual void interface_config_complete() override;

	virtual const software_list_loader &get_software_list_loader() const;
	virtual const bool use_software_list_file_extension_for_filetype() const { return false; }

	image_init_result load_internal(const std::string &path, bool is_create, int create_format, util::option_resolution *create_args);
	image_error_t load_image_by_path(u32 open_flags, const std::string &path);
	void clear();
	bool is_loaded();

	void set_image_filename(const std::string &filename);

	void clear_error();

	void check_for_file() const { assert_always(m_file, "Illegal operation on unmounted image"); }

	void setup_working_directory();
	bool try_change_working_directory(const std::string &subdir);

	void make_readonly() { m_readonly = true; }

	bool image_checkhash();

	const software_part *find_software_item(const std::string &identifier, bool restrict_to_interface, software_list_device **device = nullptr) const;
	std::string software_get_default_slot(const char *default_card_slot) const;

	void add_format(std::unique_ptr<image_device_format> &&format);
	void add_format(std::string &&name, std::string &&description, std::string &&extensions, std::string &&optspec);

	// derived class overrides

	// configuration
	static const image_device_type_info *find_device_type(iodevice_t type);
	static const image_device_type_info m_device_info_array[];

	// error related info
	image_error_t m_err;
	std::string m_err_message;

private:
	// variables that are only non-zero when an image is mounted
	util::core_file::ptr m_file;
	std::unique_ptr<emu_file> m_mame_file;
	std::string m_image_name;
	std::string m_basename;
	std::string m_basename_noext;
	std::string m_filetype;

	// Software information
	std::string m_full_software_name;
	const software_part *m_software_part_ptr;
	std::string m_software_list_name;

	static image_error_t image_error_from_file_error(osd_file::error filerr);
	std::vector<u32> determine_open_plan(bool is_create);
	void update_names();
	bool load_software_part(const std::string &identifier);

	bool init_phase() const;
	static bool run_hash(util::core_file &file, u32 skip_bytes, util::hash_collection &hashes, const char *types);

	// loads an image or software items and resets - called internally when we
	// load an is_reset_on_load() item
	void reset_and_load(const std::string &path);

	// creation info
	formatlist_type m_formatlist;

	// working directory; persists across mounts
	std::string m_working_directory;

	// info read from the hash file/software list
	std::string m_longname;
	std::string m_manufacturer;
	std::string m_year;
	u32 m_supported;

	// flags
	bool m_readonly;
	bool m_created;

	// special - used when creating
	int m_create_format;
	util::option_resolution *m_create_args;

	util::hash_collection m_hash;

	std::string m_instance_name;                // e.g. - "cartridge", "floppydisk2"
	std::string m_brief_instance_name;          // e.g. - "cart", "flop2"
	std::string m_canonical_instance_name;      // e.g. - "cartridge1", "floppydisk2" - only used internally in emuopts.cpp

	// in the case of arcade cabinet with fixed carts inserted,
	// we want to disable command line cart loading...
	bool m_user_loadable;

	bool m_is_loading;

	bool m_is_reset_and_loading;
};

// iterator
typedef device_interface_iterator<device_image_interface> image_interface_iterator;

#endif  /* MAME_EMU_DIIMAGE_H */
