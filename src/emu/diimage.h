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

#ifndef __DIIMAGE_H__
#define __DIIMAGE_H__

#include <memory>
#include <string>
#include <vector>

#include "softlist.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

extern struct io_procs image_ioprocs;

class software_list;

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
	IO_MAGTAPE,     /* 15 - Magentic tape */
	IO_ROM,         /* 16 - Individual ROM image - the Amstrad CPC has a few applications that were sold on 16kB ROMs */
	IO_MIDIIN,      /* 17 - MIDI In port */
	IO_MIDIOUT,     /* 18 - MIDI Out port */
	IO_COUNT        /* 19 - Total Number of IO_devices for searching */
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


class device_image_interface;
struct feature_list;
class software_part;
class software_info;

// device image interface function types
typedef delegate<bool (device_image_interface &)> device_image_load_delegate;
typedef delegate<void (device_image_interface &)> device_image_func_delegate;
// legacy
typedef void (*device_image_partialhash_func)(util::hash_collection &, const unsigned char *, unsigned long, const char *);

//**************************************************************************
//  MACROS
//**************************************************************************

#define IMAGE_INIT_PASS     false
#define IMAGE_INIT_FAIL     true

#define DEVICE_IMAGE_LOAD_MEMBER_NAME(_name)           device_image_load_##_name
#define DEVICE_IMAGE_LOAD_NAME(_class,_name)           _class::DEVICE_IMAGE_LOAD_MEMBER_NAME(_name)
#define DECLARE_DEVICE_IMAGE_LOAD_MEMBER(_name)        bool DEVICE_IMAGE_LOAD_MEMBER_NAME(_name)(device_image_interface &image)
#define DEVICE_IMAGE_LOAD_MEMBER(_class,_name)         bool DEVICE_IMAGE_LOAD_NAME(_class,_name)(device_image_interface &image)
#define DEVICE_IMAGE_LOAD_DELEGATE(_class,_name)       device_image_load_delegate(&DEVICE_IMAGE_LOAD_NAME(_class,_name),#_class "::device_image_load_" #_name, downcast<_class *>(device->owner()))

#define DEVICE_IMAGE_UNLOAD_MEMBER_NAME(_name)          device_image_unload_##_name
#define DEVICE_IMAGE_UNLOAD_NAME(_class,_name)          _class::DEVICE_IMAGE_UNLOAD_MEMBER_NAME(_name)
#define DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(_name)       void DEVICE_IMAGE_UNLOAD_MEMBER_NAME(_name)(device_image_interface &image)
#define DEVICE_IMAGE_UNLOAD_MEMBER(_class,_name)        void DEVICE_IMAGE_UNLOAD_NAME(_class,_name)(device_image_interface &image)
#define DEVICE_IMAGE_UNLOAD_DELEGATE(_class,_name)      device_image_func_delegate(&DEVICE_IMAGE_UNLOAD_NAME(_class,_name),#_class "::device_image_unload_" #_name, downcast<_class *>(device->owner()))

#define MCFG_SET_IMAGE_LOADABLE(_usrload) \
	device_image_interface::static_set_user_loadable(*device, _usrload);


// ======================> device_image_interface

// class representing interface-specific live image
class device_image_interface : public device_interface
{
public:
	typedef std::vector<std::unique_ptr<image_device_format>> formatlist_type;

	// construction/destruction
	device_image_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_image_interface();

	static const char *device_typename(iodevice_t type);
	static const char *device_brieftypename(iodevice_t type);
	static iodevice_t device_typeid(const char *name);

	virtual void device_compute_hash(util::hash_collection &hashes, const void *data, size_t length, const char *types) const;

	virtual bool call_load() { return IMAGE_INIT_PASS; }
	virtual bool call_create(int format_type, util::option_resolution *format_options) { return IMAGE_INIT_PASS; }
	virtual void call_unload() { }
	virtual std::string call_display() { return std::string(); }
	virtual device_image_partialhash_func get_partial_hash() const { return nullptr; }
	virtual bool core_opens_image_file() const { return true; }
	virtual iodevice_t image_type()  const = 0;
	virtual bool is_readable()  const = 0;
	virtual bool is_writeable() const = 0;
	virtual bool is_creatable() const = 0;
	virtual bool must_be_loaded() const = 0;
	virtual bool is_reset_on_load() const = 0;
	virtual const char *image_interface() const { return nullptr; }
	virtual const char *file_extensions() const = 0;
	virtual const option_guide *create_option_guide() const { return nullptr; }

	const image_device_format *device_get_indexed_creatable_format(int index) const { if (index < m_formatlist.size()) return m_formatlist.at(index).get(); else return nullptr;  }
	const image_device_format *device_get_named_creatable_format(const std::string &format_name);
	const option_guide *device_get_creation_option_guide() const { return create_option_guide(); }

	const char *error();
	void seterror(image_error_t err, const char *message);
	void message(const char *format, ...) ATTR_PRINTF(2,3);

	bool exists() { return !m_image_name.empty(); }
	const char *filename() const { if (m_image_name.empty()) return nullptr; else return m_image_name.c_str(); }
	const char *basename() const { if (m_basename.empty()) return nullptr; else return m_basename.c_str(); }
	const char *basename_noext()  const { if (m_basename_noext.empty()) return nullptr; else return m_basename_noext.c_str(); }
	const char *filetype() const { if (m_filetype.empty()) return nullptr; else return m_filetype.c_str(); }
	bool is_open() const { return bool(m_file); }
	util::core_file &image_core_file() const { return *m_file; }
	UINT64 length() { check_for_file(); return m_file->size(); }
	bool is_readonly() const { return m_readonly; }
	void make_readonly() { m_readonly = true; }
	UINT32 fread(void *buffer, UINT32 length) { check_for_file(); return m_file->read(buffer, length); }
	UINT32 fread(optional_shared_ptr<UINT8> &ptr, UINT32 length) { ptr.allocate(length); return fread(ptr.target(), length); }
	UINT32 fread(optional_shared_ptr<UINT8> &ptr, UINT32 length, offs_t offset) { ptr.allocate(length); return fread(ptr + offset, length - offset); }
	UINT32 fwrite(const void *buffer, UINT32 length) { check_for_file(); return m_file->write(buffer, length); }
	int fseek(INT64 offset, int whence) { check_for_file(); return m_file->seek(offset, whence); }
	UINT64 ftell() { check_for_file(); return m_file->tell(); }
	int fgetc() { char ch; if (fread(&ch, 1) != 1) ch = '\0'; return ch; }
	char *fgets(char *buffer, UINT32 length) { check_for_file(); return m_file->gets(buffer, length); }
	int image_feof() { check_for_file(); return m_file->eof(); }
	void *ptr() {check_for_file(); return const_cast<void *>(m_file->buffer()); }
	// configuration access
	void set_init_phase() { m_init_phase = true; }

	const char* longname() const { return m_longname.c_str(); }
	const char* manufacturer() const { return m_manufacturer.c_str(); }
	const char* year() const { return m_year.c_str(); }
	UINT32 supported() const { return m_supported; }

	const software_info *software_entry() const { return m_software_info_ptr; }
	const software_part *part_entry() const { return m_software_part_ptr; }
	const char *software_list_name() const { return m_software_list_name.c_str(); }
	bool loaded_through_softlist() const { return m_software_info_ptr != nullptr; }

	void set_working_directory(const char *working_directory) { m_working_directory = working_directory; }
	const char * working_directory();

	UINT8 *get_software_region(const char *tag);
	UINT32 get_software_region_length(const char *tag);
	const char *get_feature(const char *feature_name);
	bool load_software_region(const char *tag, optional_shared_ptr<UINT8> &ptr);

	UINT32 crc();
	util::hash_collection& hash() { return m_hash; }

	void battery_load(void *buffer, int length, int fill);
	void battery_load(void *buffer, int length, void *def_buffer);
	void battery_save(const void *buffer, int length);

	const char *image_type_name()  const { return device_typename(image_type()); }

	const char *instance_name() const { return m_instance_name.c_str(); }
	const char *brief_instance_name() const { return m_brief_instance_name.c_str(); }
	bool uses_file_extension(const char *file_extension) const;
	const formatlist_type &formatlist() const { return m_formatlist; }

	bool load(const char *path);
	bool open_image_file(emu_options &options);
	bool finish_load();
	void unload();
	bool create(const char *path, const image_device_format *create_format, util::option_resolution *create_args);
	bool load_software(software_list_device &swlist, const char *swname, const rom_entry *entry);
	int reopen_for_write(const char *path);

	static void software_name_split(const char *swlist_swname, std::string &swlist_name, std::string &swname, std::string &swpart);
	static void static_set_user_loadable(device_t &device, bool user_loadable) {
		device_image_interface *img;
		if (!device.interface(img))
			throw emu_fatalerror("MCFG_SET_IMAGE_LOADABLE called on device '%s' with no image interface\n", device.tag());

		img->m_user_loadable = user_loadable;
	}

	bool user_loadable() const { return m_user_loadable; }

protected:
	virtual const software_list_loader &get_software_list_loader() const { return false_software_list_loader::instance(); }

	bool load_internal(const std::string &path, bool is_create, int create_format, util::option_resolution *create_args, bool just_load);
	void determine_open_plan(int is_create, UINT32 *open_plan);
	image_error_t load_image_by_path(UINT32 open_flags, const std::string &path);
	void clear();
	bool is_loaded();

	void set_image_filename(const std::string &filename);

	void clear_error();

	void check_for_file() const { assert_always(m_file, "Illegal operation on unmounted image"); }

	void setup_working_directory();
	bool try_change_working_directory(const char *subdir);

	void run_hash(void (*partialhash)(util::hash_collection &, const unsigned char *, unsigned long, const char *), util::hash_collection &hashes, const char *types);
	void image_checkhash();
	void update_names(const device_type device_type = nullptr, const char *inst = nullptr, const char *brief = nullptr);

	const software_part *find_software_item(const char *path, bool restrict_to_interface) const;
	bool load_software_part(const char *path, const software_part *&swpart);
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

	// variables that are only non-zero when an image is mounted
	util::core_file::ptr m_file;
	std::unique_ptr<emu_file> m_mame_file;
	std::string m_image_name;
	std::string m_basename;
	std::string m_basename_noext;
	std::string m_filetype;

	// Software information
	std::string m_full_software_name;
	const software_info *m_software_info_ptr;
	const software_part *m_software_part_ptr;
	std::string m_software_list_name;

private:
	static image_error_t image_error_from_file_error(osd_file::error filerr);

	// creation info
	formatlist_type m_formatlist;

	// working directory; persists across mounts
	std::string m_working_directory;

	// info read from the hash file/software list
	std::string m_longname;
	std::string m_manufacturer;
	std::string m_year;
	UINT32  m_supported;

	// flags
	bool m_readonly;
	bool m_created;
	bool m_init_phase;

	// special - used when creating
	int m_create_format;
	util::option_resolution *m_create_args;

	util::hash_collection m_hash;

	std::string m_brief_instance_name;
	std::string m_instance_name;

	// in the case of arcade cabinet with fixed carts inserted,
	// we want to disable command line cart loading...
	bool m_user_loadable;

	bool m_is_loading;
};

// iterator
typedef device_interface_iterator<device_image_interface> image_interface_iterator;

#endif  /* __DIIMAGE_H__ */
