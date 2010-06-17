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


// ======================> device_config_image_interface

// class representing interface-specific configuration image
class device_config_image_interface : public device_config_interface
{
public:
	// construction/destruction
	device_config_image_interface(const machine_config &mconfig, device_config &device);
	virtual ~device_config_image_interface();

	// public accessors... for now
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
};



// ======================> device_image_interface

// class representing interface-specific live image
class device_image_interface : public device_interface
{
public:
	// construction/destruction
	device_image_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_image_interface();

	// configuration access
	const device_config_image_interface &image_config() const { return m_image_config; }

protected:
	// derived class overrides

	// configuration
	const device_config_image_interface &m_image_config;	// reference to our device_config_execute_interface
};


#endif	/* __DIIMAGE_H__ */
