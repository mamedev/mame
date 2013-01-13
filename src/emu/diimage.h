/***************************************************************************

    diimage.h

    Device image interfaces.

****************************************************************************

    Copyright Miodrag Milanovic
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIIMAGE_H__
#define __DIIMAGE_H__

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

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
	IO_MIDIIN,		/* 17 - MIDI In port */
	IO_MIDIOUT,		/* 18 - MIDI Out port */
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

struct image_device_format
{
	image_device_format *m_next;
	int m_index;
	astring m_name;
	astring m_description;
	astring m_extensions;
	astring m_optspec;
};

class device_image_interface;
struct feature_list;
struct software_part;
struct software_info;

// device image interface function types
typedef int (*device_image_load_func)(device_image_interface &image);
typedef void (*device_image_unload_func)(device_image_interface &image);
typedef void (*device_image_partialhash_func)(hash_collection &, const unsigned char *, unsigned long, const char *);
typedef void (*device_image_display_info_func)(device_image_interface &image);

//**************************************************************************
//  MACROS
//**************************************************************************

#define IMAGE_INIT_PASS     FALSE
#define IMAGE_INIT_FAIL     TRUE
#define IMAGE_VERIFY_PASS   FALSE
#define IMAGE_VERIFY_FAIL   TRUE

#define DEVICE_IMAGE_LOAD_NAME(name)        device_load_##name
#define DEVICE_IMAGE_LOAD(name)             int DEVICE_IMAGE_LOAD_NAME(name)(device_image_interface &image)

#define DEVICE_IMAGE_UNLOAD_NAME(name)      device_unload_##name
#define DEVICE_IMAGE_UNLOAD(name)           void DEVICE_IMAGE_UNLOAD_NAME(name)(device_image_interface &image)

#define DEVICE_IMAGE_DISPLAY_INFO_NAME(name)     device_image_display_info_func##name
#define DEVICE_IMAGE_DISPLAY_INFO(name)          void DEVICE_IMAGE_DISPLAY_INFO_NAME(name)(device_image_interface &image)

// ======================> device_image_interface

// class representing interface-specific live image
class device_image_interface : public device_interface
{
public:
	// construction/destruction
	device_image_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_image_interface();

	static const char *device_typename(iodevice_t type);
	static const char *device_brieftypename(iodevice_t type);
	static iodevice_t device_typeid(const char *name);

	virtual void device_compute_hash(hash_collection &hashes, const void *data, size_t length, const char *types) const;

	virtual bool call_load() { return FALSE; }
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { return FALSE; }
	virtual bool call_create(int format_type, option_resolution *format_options) { return FALSE; }
	virtual void call_unload() { }
	virtual void call_display() { }
	virtual void call_display_info() { }
	virtual device_image_partialhash_func get_partial_hash() const { return NULL; }
	virtual bool core_opens_image_file() const { return TRUE; }
	virtual iodevice_t image_type()  const = 0;
	virtual bool is_readable()  const = 0;
	virtual bool is_writeable() const = 0;
	virtual bool is_creatable() const = 0;
	virtual bool must_be_loaded() const = 0;
	virtual bool is_reset_on_load() const = 0;
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const = 0;
	virtual const option_guide *create_option_guide() const = 0;

	virtual ui_menu *get_selection_menu(running_machine &machine, class render_container *container);

	const image_device_format *device_get_indexed_creatable_format(int index);
	const image_device_format *device_get_named_creatable_format(const char *format_name);
	const option_guide *device_get_creation_option_guide() { return create_option_guide(); }
	const image_device_format *device_get_creatable_formats() { return formatlist(); }

	const char *error();
	void seterror(image_error_t err, const char *message);
	void message(const char *format, ...);

	bool exists() { return m_image_name; }
	const char *filename() { if (!m_image_name) return NULL; else return m_image_name; }
	const char *basename() { if (!m_basename) return NULL; else return m_basename; }
	const char *basename_noext()  { if (!m_basename_noext) return NULL; else return m_basename_noext; }
	const char *filetype()  { if (!m_filetype) return NULL; else return m_filetype; }
	core_file *image_core_file() { return m_file; }
	UINT64 length() { check_for_file(); return core_fsize(m_file); }
	bool is_readonly() { return m_readonly; }
	bool has_been_created() { return m_created; }
	void make_readonly() { m_readonly = true; }
	UINT32 fread(void *buffer, UINT32 length) { check_for_file(); return core_fread(m_file, buffer, length); }
	UINT32 fwrite(const void *buffer, UINT32 length) { check_for_file(); return core_fwrite(m_file, buffer, length); }
	int fseek(INT64 offset, int whence) { check_for_file(); return core_fseek(m_file, offset, whence); }
	UINT64 ftell() { check_for_file(); return core_ftell(m_file); }
	int fgetc() { char ch; if (fread(&ch, 1) != 1) ch = '\0'; return ch; }
	char *fgets(char *buffer, UINT32 length) { check_for_file(); return core_fgets(buffer, length, m_file); }
	int image_feof() { check_for_file(); return core_feof(m_file); }
	void *ptr() {check_for_file(); return (void *) core_fbuffer(m_file); }
	// configuration access
	void set_init_phase() { m_init_phase = TRUE; }

	const char* longname() { return m_longname; }
	const char* manufacturer() { return m_manufacturer; }
	const char* year() { return m_year; }
	UINT32 supported() { return m_supported; }

	const software_info *software_entry() { return m_software_info_ptr; }
	const software_part *part_entry() { return m_software_part_ptr; }
	const char *software_list_name() { return m_software_list_name; }

	void set_working_directory(const char *working_directory) { m_working_directory = working_directory; }
	const char * working_directory();

	UINT8 *get_software_region(const char *tag);
	UINT32 get_software_region_length(const char *tag);
	const char *get_feature(const char *feature_name);

	UINT32 crc();
	hash_collection& hash() { return m_hash; }

	void battery_load(void *buffer, int length, int fill);
	void battery_save(const void *buffer, int length);

	const char *image_type_name()  const { return device_typename(image_type()); }



	const char *instance_name() const { return m_instance_name; }
	const char *brief_instance_name() const { return m_brief_instance_name; }
	bool uses_file_extension(const char *file_extension) const;
	image_device_format *formatlist() const { return m_formatlist; }

	bool load(const char *path);
	bool open_image_file(emu_options &options);
	bool finish_load();
	void unload();
	bool create(const char *path, const image_device_format *create_format, option_resolution *create_args);
	bool load_software(char *swlist, char *swname, rom_entry *entry);
	int reopen_for_write(const char *path);

protected:
	bool load_internal(const char *path, bool is_create, int create_format, option_resolution *create_args, bool just_load);
	void determine_open_plan(int is_create, UINT32 *open_plan);
	image_error_t load_image_by_path(UINT32 open_flags, const char *path);
	void clear();
	bool is_loaded();

	image_error_t set_image_filename(const char *filename);

	void clear_error();

	void check_for_file() { assert_always(m_file != NULL, "Illegal operation on unmounted image"); }

	void setup_working_directory();
	bool try_change_working_directory(const char *subdir);

	int read_hash_config(const char *sysname);
	void run_hash(void (*partialhash)(hash_collection &, const unsigned char *, unsigned long, const char *), hash_collection &hashes, const char *types);
	void image_checkhash();
	void update_names(const device_type device_type = NULL, const char *inst = NULL, const char *brief = NULL);
	// derived class overrides

	// configuration
	static const image_device_type_info *find_device_type(iodevice_t type);
	static const image_device_type_info m_device_info_array[];

	/* error related info */
	image_error_t m_err;
	astring m_err_message;

	/* variables that are only non-zero when an image is mounted */
	core_file *m_file;
	emu_file *m_mame_file;
	astring m_image_name;
	astring m_basename;
	astring m_basename_noext;
	astring m_filetype;

	/* working directory; persists across mounts */
	astring m_working_directory;

	/* Software information */
	char *m_full_software_name;
	software_info *m_software_info_ptr;
	software_part *m_software_part_ptr;
	char *m_software_list_name;

	/* info read from the hash file/software list */
	astring m_longname;
	astring m_manufacturer;
	astring m_year;
	UINT32  m_supported;

	/* flags */
	bool m_readonly;
	bool m_created;
	bool m_init_phase;
	bool m_from_swlist;

	/* special - used when creating */
	int m_create_format;
	option_resolution *m_create_args;

	hash_collection m_hash;

	astring m_brief_instance_name;
	astring m_instance_name;

	/* creation info */
	image_device_format *m_formatlist;

	bool m_is_loading;
};

// iterator
typedef device_interface_iterator<device_image_interface> image_interface_iterator;

class ui_menu_control_device_image : public ui_menu {
public:
	ui_menu_control_device_image(running_machine &machine, render_container *container, device_image_interface *image);
	virtual ~ui_menu_control_device_image();
	virtual void populate();
	virtual void handle();

protected:
	enum {
		START_FILE, START_OTHER_PART, START_SOFTLIST, SELECT_PARTLIST, SELECT_ONE_PART, SELECT_OTHER_PART, SELECT_FILE, CREATE_FILE, CREATE_CONFIRM, DO_CREATE, SELECT_SOFTLIST,
		LAST_ID
	};
	int state;

	device_image_interface *image;
	astring current_directory;
	astring current_file;
	int submenu_result;
	bool create_confirmed;
	bool softlist_done;
	const struct software_list *swl;
	const software_info *swi;
	const software_part *swp;
	const class software_list_device *sld;
	astring software_info_name;

	void test_create(bool &can_create, bool &need_confirm);
	void load_software_part();
	virtual void hook_load(astring filename, bool softlist);
};

#endif  /* __DIIMAGE_H__ */
