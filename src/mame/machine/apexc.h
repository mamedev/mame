// license:GPL-2.0+
// copyright-holders:Raphael Nabet, Robbbert
/*
    machine/apexc.c : APEXC machine

    By Raphael Nabet

    see cpu/apexc.c for background and tech info
*/

#ifndef MAME_MACHINE_APEXC
#define MAME_MACHINE_APEXC

#pragma once


/*
    APEXC RAM loading/saving from cylinder image

    Note that, in an actual APEXC, the RAM contents are not read from the cylinder :
    the cylinder IS the RAM.

    This feature is important : of course, the tape reader allows to enter programs, but you
    still need an object code loader in memory.  (Of course, the control panel enables
    the user to enter such a loader manually, but it would take hours...)
*/

class apexc_cylinder_image_device : public device_t, public device_image_interface
{
public:
	// construction/destruction
	apexc_cylinder_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// image-level overrides
	virtual iodevice_t image_type() const override { return IO_CYLINDER; }

	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return false; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return true; }
	virtual const char *file_extensions() const override { return "apc"; }

	virtual image_init_result call_load() override;
	virtual void call_unload() override;

private:
	virtual void device_start() override { }

	int m_writable;
};

/*
    APEXC tape support

    APEXC does all I/O on paper tape.  There are 5 punch rows on tape.

    Both a reader (read-only), and a puncher (write-only) are provided.

    Tape output can be fed into a teletyper, in order to have text output :

    code                    Symbol
    (binary)        Letters         Figures
    00000                           0
    00001           T               1
    00010           B               2
    00011           O               3
    00100           E               4
    00101           H               5
    00110           N               6
    00111           M               7
    01000           A               8
    01001           L               9
    01010           R               +
    01011           G               -
    01100           I               z
    01101           P               .
    01110           C               d
    01111           V               =
    10000                   Space
    10001           Z               y
    10010           D               theta (greek letter)
    10011                   Line Space (i.e. LF)
    10100           S               ,
    10101           Y               Sigma (greek letter)
    10110           F               x
    10111           X               /
    11000                   Carriage Return
    11001           W               phi (greek letter)
    11010           J               - (dash ?)
    11011                   Figures
    11100           U               pi (greek letter)
    11101           Q               )
    11110           K               (
    11111                   Letters
*/

class apexc_tape_puncher_image_device : public device_t, public device_image_interface
{
public:
	// construction/destruction
	apexc_tape_puncher_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// image-level overrides
	virtual iodevice_t image_type() const override { return IO_PUNCHTAPE; }

	virtual bool is_readable()  const override { return false; }
	virtual bool is_writeable() const override { return true; }
	virtual bool is_creatable() const override { return true; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override { return "tap"; }

	DECLARE_WRITE8_MEMBER(write);

private:
	virtual void device_start() override { }
};

class apexc_tape_reader_image_device :  public device_t, public device_image_interface
{
public:
	// construction/destruction
	apexc_tape_reader_image_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: apexc_tape_reader_image_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	apexc_tape_reader_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual iodevice_t image_type() const override { return IO_PUNCHTAPE; }

	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return false; }
	virtual bool is_creatable() const override { return false; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return false; }
	virtual const char *file_extensions() const override { return "tap"; }

	DECLARE_READ8_MEMBER(read);
private:
	virtual void device_start() override { }
};

DECLARE_DEVICE_TYPE(APEXC_CYLINDER, apexc_cylinder_image_device)
DECLARE_DEVICE_TYPE(APEXC_TAPE_PUNCHER, apexc_tape_puncher_image_device)
DECLARE_DEVICE_TYPE(APEXC_TAPE_READER, apexc_tape_reader_image_device)

#endif // MAME_MACHINE_APEXC
