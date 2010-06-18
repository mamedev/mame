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
    IO_COUNT        /* 16 - Total Number of IO_devices for searching */
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

// device image interface function types
typedef int (*device_image_load_func)(device_image_interface *image);
typedef int (*device_image_create_func)(device_image_interface *image, int format_type, option_resolution *format_options);
typedef void (*device_image_unload_func)(device_image_interface *image);
typedef void (*device_image_display_func)(device_image_interface *image);
typedef void (*device_image_partialhash_func)(char *, const unsigned char *, unsigned long, unsigned int);
typedef void (*device_image_get_devices_func)(device_image_interface *device);


//**************************************************************************
//  MACROS
//**************************************************************************

#define DEVICE_IMAGE_LOAD_NAME(name)        device_load_##name
#define DEVICE_IMAGE_LOAD(name)             int DEVICE_IMAGE_LOAD_NAME(name)(device_image_interface *image)

#define DEVICE_IMAGE_CREATE_NAME(name)      device_create_##name
#define DEVICE_IMAGE_CREATE(name)           int DEVICE_IMAGE_CREATE_NAME(name)(device_image_interface *image, int create_format, option_resolution *create_args)

#define DEVICE_IMAGE_UNLOAD_NAME(name)      device_unload_##name
#define DEVICE_IMAGE_UNLOAD(name)           void DEVICE_IMAGE_UNLOAD_NAME(name)(device_image_interface *image)

#define DEVICE_IMAGE_DISPLAY_NAME(name)     device_image_display_func##name
#define DEVICE_IMAGE_DISPLAY(name)          void DEVICE_IMAGE_DISPLAY_NAME(name)(device_image_interface *image)

#define DEVICE_IMAGE_GET_DEVICES_NAME(name) device_image_get_devices_##name
#define DEVICE_IMAGE_GET_DEVICES(name)      void DEVICE_IMAGE_GET_DEVICES_NAME(name)(device_image_interface *device)


// ======================> device_config_image_interface

// class representing interface-specific configuration image
class device_config_image_interface : public device_config_interface
{
	friend class device_image_interface;
public:
	// construction/destruction
	device_config_image_interface(const machine_config &mconfig, device_config &device);
	virtual ~device_config_image_interface();

	// public accessors... for now
	virtual const char *name() const = 0;	
	virtual iodevice_t image_type()  const = 0;
	virtual const char *image_type_name()  const = 0;
	virtual iodevice_t image_type_direct() const = 0;
	virtual bool is_readable()  const = 0;
	virtual bool is_writeable() const = 0;
	virtual bool is_creatable() const = 0;
	virtual bool must_be_loaded() const = 0;
	virtual bool is_reset_on_load() const = 0;
	virtual bool has_partial_hash() const = 0;
	virtual const char *image_interface() const = 0;
	virtual const char *file_extensions() const = 0;
	virtual const char *instance_name() const = 0;
	virtual const char *brief_instance_name() const = 0;
	virtual bool uses_file_extension(const char *file_extension) const = 0;
	virtual const option_guide *create_option_guide() const = 0;
    virtual image_device_format *formatlist() const = 0;
	
	static const char *device_typename(iodevice_t type);
	static const char *device_brieftypename(iodevice_t type);	
	
	virtual device_image_load_func		load_func() const  = 0;
	virtual device_image_create_func	create_func() const = 0;
	virtual device_image_unload_func	unload_func() const = 0;
	
protected:
	static const image_device_type_info *find_device_type(iodevice_t type);
	static const image_device_type_info m_device_info_array[];
};



// ======================> device_image_interface

// class representing interface-specific live image
class device_image_interface : public device_interface
{
	friend class device_config_image_interface;
public:
	// construction/destruction
	device_image_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_image_interface();

	virtual void set_working_directory(const char *working_directory) = 0;
	virtual const char * working_directory() = 0;
	virtual bool load(const char *path) = 0;
	virtual bool finish_load() = 0;
	virtual void unload() = 0;
	
	virtual const image_device_format *device_get_indexed_creatable_format(int index);
	virtual const image_device_format *device_get_named_creatable_format(const char *format_name);
	const option_guide *image_device_get_creation_option_guide() { return m_image_config.create_option_guide(); }
	const image_device_format *device_get_creatable_formats() { return m_image_config.formatlist(); }	

	virtual bool create(const char *path, const image_device_format *create_format, option_resolution *create_args) = 0;
	
	const char *error();
	void seterror(image_error_t err, const char *message);
	void message(const char *format, ...);

	bool exists() { return m_name.len() != 0; }
	const char *filename();
	const char *basename();
	const char *basename_noext();
	const char *filetype();
	core_file *image_core_file() { return m_file; }
	UINT64 length() { check_for_file(); return core_fsize(m_file); }
	bool is_writable() { return m_writeable; }
	bool has_been_created() { return m_created; }
	void make_readonly() { m_writeable = 0; }
	UINT32 fread(void *buffer, UINT32 length) { check_for_file(); return core_fread(m_file, buffer, length); }
	UINT32 fwrite(const void *buffer, UINT32 length) { check_for_file(); return core_fwrite(m_file, buffer, length); }
	int fseek(running_device *image, INT64 offset, int whence) { check_for_file(); return core_fseek(m_file, offset, whence); }
	UINT64 ftell() { check_for_file(); return core_ftell(m_file); }
	int fgetc() { char ch; if (fread(&ch, 1) != 1) ch = '\0'; return ch; }
	char *fgets(char *buffer, UINT32 length) { check_for_file(); return core_fgets(buffer, length, m_file); }
	int feof() { check_for_file(); return core_feof(m_file); }	
	void *ptr() {check_for_file(); return (void *) core_fbuffer(m_file); }	
	// configuration access
	const device_config_image_interface &image_config() const { return m_image_config; }
	
	void set_init_phase() { m_init_phase = TRUE; }
protected:
	void clear_error();
	
	void check_for_file() { assert_always(m_file != NULL, "Illegal operation on unmounted image"); }
	// derived class overrides

	// configuration
	const device_config_image_interface &m_image_config;	// reference to our device_config_execute_interface	

    /* error related info */
    image_error_t m_err;
    astring m_err_message;	
	
    /* variables that are only non-zero when an image is mounted */
	core_file *m_file;	
	astring m_name;	
    /* flags */
    bool m_writeable;
    bool m_created;	
	bool m_init_phase;
	
    /* special - used when creating */
    int m_create_format;
    option_resolution *m_create_args;
};


#endif	/* __DIIMAGE_H__ */
