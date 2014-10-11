// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef __SG1000__
#define __SG1000__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "formats/sc3000_bit.h"
#include "formats/sf7000_dsk.h"
#include "imagedev/cassette.h"
#include "imagedev/printer.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "bus/sega8/sega8_slot.h"
#include "machine/upd765.h"
#include "sound/sn76496.h"
#include "video/tms9928a.h"
#include "crsshair.h"

#define SCREEN_TAG      "screen"
#define Z80_TAG         "z80"
#define SN76489AN_TAG   "sn76489an"
#define UPD765_TAG      "upd765"
#define UPD8251_TAG     "upd8251"
#define UPD9255_TAG     "upd9255"
#define UPD9255_0_TAG   "upd9255_0"
#define UPD9255_1_TAG   "upd9255_1"
#define CENTRONICS_TAG  "centronics"
#define TMS9918A_TAG    "tms9918a"
#define RS232_TAG       "rs232"
#define CARTSLOT_TAG    "slot"


INPUT_PORTS_EXTERN( sk1100 );

class sg1000_state : public driver_device
{
public:
	enum
	{
		TIMER_LIGHTGUN_TICK
	};

	sg1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, Z80_TAG),
			m_ram(*this, RAM_TAG),
			m_rom(*this, Z80_TAG),
			m_cart(*this, CARTSLOT_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	optional_device<sega8_cart_slot_device> m_cart;

	virtual void machine_start();

	DECLARE_READ8_MEMBER( joysel_r );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi );

	DECLARE_READ8_MEMBER( omv_r );
	DECLARE_WRITE8_MEMBER( omv_w );

	/* keyboard state */
	UINT8 m_keylatch;

	DECLARE_WRITE_LINE_MEMBER(sg1000_vdp_interrupt);
};

class sc3000_state : public sg1000_state
{
public:
	sc3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: sg1000_state(mconfig, type, tag),
			m_cassette(*this, "cassette"),
			m_pa0(*this, "PA0"),
			m_pa1(*this, "PA1"),
			m_pa2(*this, "PA2"),
			m_pa3(*this, "PA3"),
			m_pa4(*this, "PA4"),
			m_pa5(*this, "PA5"),
			m_pa6(*this, "PA6"),
			m_pa7(*this, "PA7"),
			m_pb0(*this, "PB0"),
			m_pb1(*this, "PB1"),
			m_pb2(*this, "PB2"),
			m_pb3(*this, "PB3"),
			m_pb4(*this, "PB4"),
			m_pb5(*this, "PB5"),
			m_pb6(*this, "PB6"),
			m_pb7(*this, "PB7")
	{ }

	required_device<cassette_image_device> m_cassette;
	required_ioport m_pa0;
	required_ioport m_pa1;
	required_ioport m_pa2;
	required_ioport m_pa3;
	required_ioport m_pa4;
	required_ioport m_pa5;
	required_ioport m_pa6;
	required_ioport m_pa7;
	required_ioport m_pb0;
	required_ioport m_pb1;
	required_ioport m_pb2;
	required_ioport m_pb3;
	required_ioport m_pb4;
	required_ioport m_pb5;
	required_ioport m_pb6;
	required_ioport m_pb7;

	virtual void machine_start();

	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_READ8_MEMBER( ppi_pb_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );

	ioport_port* m_key_row[16];
};

class sf7000_state : public sc3000_state
{
public:
	sf7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: sc3000_state(mconfig, type, tag),
			m_fdc(*this, UPD765_TAG),
			m_centronics(*this, CENTRONICS_TAG),
			m_floppy0(*this, UPD765_TAG ":0:3ssdd")
	{ }

	required_device<upd765a_device> m_fdc;
	required_device<centronics_device> m_centronics;
	required_device<floppy_image_device> m_floppy0;

	virtual void machine_start();
	virtual void machine_reset();

	int m_centronics_busy;
	DECLARE_WRITE_LINE_MEMBER( write_centronics_busy );
	DECLARE_READ8_MEMBER( ppi_pa_r );
	DECLARE_WRITE8_MEMBER( ppi_pc_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );
};

#endif
