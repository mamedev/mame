// license:BSD-3-Clause
// copyright-holders:Lee Ward, Dirk Best, Curt Coder
/*************************************************************************

    Memotech MTX 500, MTX 512 and RS 128

**************************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "includes/mtx.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"
#include "bus/centronics/ctronics.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "video/tms9928a.h"
#include "sound/sn76496.h"

/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    mtx_strobe_r - centronics strobe
-------------------------------------------------*/

READ8_MEMBER(mtx_state::mtx_strobe_r)
{
	/* set STROBE low */
	m_centronics->write_strobe(FALSE);

	return 0xff;
}

/*-------------------------------------------------
    mtx_bankswitch_w - bankswitch
-------------------------------------------------*/

/*
    There are two memory models on the MTX, the standard one and a
    CBM mode. In standard mode, the memory map is defined as:

    0x0000 - 0x1fff  OSROM
    0x2000 - 0x3fff  Paged ROM
    0x4000 - 0x7fff  Paged RAM
    0x8000 - 0xbfff  Paged RAM
    0xc000 - 0xffff  RAM

    Banks are selected by output port 0. Bits 0-3 define the RAM page
    and bits 4-6 the ROM page.

    CBM mode is selected by bit 7 of output port 0. ROM is replaced
    by RAM in this mode.
*/

void mtx_state::bankswitch(UINT8 data)
{
	/*

	    bit     description

	    0       P0
	    1       P1
	    2       P2
	    3       P3
	    4       R0
	    5       R1
	    6       R2
	    7       RELCPMH

	*/
	address_space &program = m_maincpu->space(AS_PROGRAM);
	ram_device *messram = m_ram;

//  UINT8 cbm_mode = data >> 7 & 0x01;
	UINT8 rom_page = data >> 4 & 0x07;
	UINT8 ram_page = data >> 0 & 0x0f;

	/* set rom bank (switches between basic and assembler rom or cartridges) */
	membank("bank2")->set_entry(rom_page);

	/* set ram bank, for invalid pages a nop-handler will be installed */
	if (ram_page >= messram->size()/0x8000)
	{
		program.nop_readwrite(0x4000, 0x7fff);
		program.nop_readwrite(0x8000, 0xbfff);
	}
	else if (ram_page + 1 == messram->size()/0x8000)
	{
		program.nop_readwrite(0x4000, 0x7fff);
		program.install_readwrite_bank(0x8000, 0xbfff, "bank4");
		membank("bank4")->set_entry(ram_page);
	}
	else
	{
		program.install_readwrite_bank(0x4000, 0x7fff, "bank3");
		program.install_readwrite_bank(0x8000, 0xbfff, "bank4");
		membank("bank3")->set_entry(ram_page);
		membank("bank4")->set_entry(ram_page);
	}
}

WRITE8_MEMBER(mtx_state::mtx_bankswitch_w)
{
	bankswitch(data);
}

/*-------------------------------------------------
    mtx_sound_strobe_r - sound strobe
-------------------------------------------------*/

READ8_MEMBER(mtx_state::mtx_sound_strobe_r)
{
	m_sn->write(space, 0, m_sound_latch);
	return 0xff;
}

/*-------------------------------------------------
    mtx_sound_latch_w - sound latch write
-------------------------------------------------*/

WRITE8_MEMBER(mtx_state::mtx_sound_latch_w)
{
	m_sound_latch = data;
}

/*-------------------------------------------------
    mtx_cst_w - cassette write
-------------------------------------------------*/

WRITE8_MEMBER(mtx_state::mtx_cst_w)
{
	m_cassette->output( BIT(data, 0) ? -1 : 1);
}

/*-------------------------------------------------
    mtx_prt_r - centronics status
-------------------------------------------------*/

WRITE_LINE_MEMBER(mtx_state::write_centronics_busy)
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER(mtx_state::write_centronics_fault)
{
	m_centronics_fault = state;
}

WRITE_LINE_MEMBER(mtx_state::write_centronics_perror)
{
	m_centronics_perror = state;
}

WRITE_LINE_MEMBER(mtx_state::write_centronics_select)
{
	m_centronics_select = state;
}

READ8_MEMBER(mtx_state::mtx_prt_r)
{
	/*

	    bit     description

	    0       BUSY
	    1       ERROR
	    2       PE
	    3       SLCT
	    4
	    5
	    6
	    7

	*/

	UINT8 data = 0;

	/* reset STROBE to high */
	m_centronics->write_strobe( TRUE);

	data |= m_centronics_busy << 0;
	data |= m_centronics_fault << 1;
	data |= m_centronics_perror << 2;
	data |= m_centronics_select << 3;

	return data;
}

/*-------------------------------------------------
    mtx_sense_w - keyboard sense write
-------------------------------------------------*/

WRITE8_MEMBER(mtx_state::mtx_sense_w)
{
	m_key_sense = data;
}

/*-------------------------------------------------
    mtx_key_lo_r - keyboard low read
-------------------------------------------------*/

READ8_MEMBER(mtx_state::mtx_key_lo_r)
{
	UINT8 data = 0xff;

	if (!(m_key_sense & 0x01)) data &= ioport("ROW0")->read();
	if (!(m_key_sense & 0x02)) data &= ioport("ROW1")->read();
	if (!(m_key_sense & 0x04)) data &= ioport("ROW2")->read();
	if (!(m_key_sense & 0x08)) data &= ioport("ROW3")->read();
	if (!(m_key_sense & 0x10)) data &= ioport("ROW4")->read();
	if (!(m_key_sense & 0x20)) data &= ioport("ROW5")->read();
	if (!(m_key_sense & 0x40)) data &= ioport("ROW6")->read();
	if (!(m_key_sense & 0x80)) data &= ioport("ROW7")->read();

	return data;
}

/*-------------------------------------------------
    mtx_key_lo_r - keyboard high read
-------------------------------------------------*/

READ8_MEMBER(mtx_state::mtx_key_hi_r)
{
	UINT8 data = ioport("country_code")->read();

	if (!(m_key_sense & 0x01)) data &= ioport("ROW0")->read() >> 8;
	if (!(m_key_sense & 0x02)) data &= ioport("ROW1")->read() >> 8;
	if (!(m_key_sense & 0x04)) data &= ioport("ROW2")->read() >> 8;
	if (!(m_key_sense & 0x08)) data &= ioport("ROW3")->read() >> 8;
	if (!(m_key_sense & 0x10)) data &= ioport("ROW4")->read() >> 8;
	if (!(m_key_sense & 0x20)) data &= ioport("ROW5")->read() >> 8;
	if (!(m_key_sense & 0x40)) data &= ioport("ROW6")->read() >> 8;
	if (!(m_key_sense & 0x80)) data &= ioport("ROW7")->read() >> 8;

	return data;
}

/*-------------------------------------------------
    hrx_address_w - HRX video RAM address
-------------------------------------------------*/

WRITE8_MEMBER(mtx_state::hrx_address_w)
{
	if (offset)
	{
		/*

		    bit     description

		    0       A8
		    1       A9
		    2       A10
		    3
		    4
		    5       attribute memory write enable
		    6       ASCII memory write enable
		    7       cycle (0=read/1=write)

		*/
	}
	else
	{
		/*

		    bit     description

		    0       A0
		    1       A1
		    2       A2
		    3       A3
		    4       A4
		    5       A5
		    6       A6
		    7       A7

		*/
	}
}

/*-------------------------------------------------
    hrx_data_r - HRX data read
-------------------------------------------------*/

READ8_MEMBER(mtx_state::hrx_data_r)
{
	return 0;
}

/*-------------------------------------------------
    hrx_data_w - HRX data write
-------------------------------------------------*/

WRITE8_MEMBER(mtx_state::hrx_data_w)
{
}

/*-------------------------------------------------
    hrx_attr_r - HRX attribute read
-------------------------------------------------*/

READ8_MEMBER(mtx_state::hrx_attr_r)
{
	return 0;
}

/*-------------------------------------------------
    hrx_attr_r - HRX attribute write
-------------------------------------------------*/

WRITE8_MEMBER(mtx_state::hrx_attr_w)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/
}

/***************************************************************************
    SNAPSHOT
***************************************************************************/

// this only works for some of the files, nothing which tries to load
// more data from tape. todo: tapes which autorun after loading
SNAPSHOT_LOAD_MEMBER( mtx_state, mtx )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	void *ptr;
	UINT8 header[18];

	// read header
	image.fread(&header, sizeof(header));

	// verify first byte
	if (header[0] != 0xff)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, nullptr);
		return IMAGE_INIT_FAIL;
	}

	// get tape name
	char tape_name[16];
	memcpy(&tape_name, &header[1], 15);
	tape_name[15] = '\0';
	image.message("Loading '%s'", tape_name);

	// start of system variables area
	UINT16 system_variables_base = pick_integer_le(header, 16, 2);

	// write system variables
	UINT16 system_variables_size = 0;

	if (system_variables_base != 0)
	{
		ptr = program.get_write_ptr(system_variables_base);
		system_variables_size = 0xfb4b - system_variables_base;
		image.fread(ptr, system_variables_size);
	}

	// write actual image data
	UINT16 data_size = snapshot_size - 18 - system_variables_size;

	ptr = program.get_write_ptr(0x4000);
	image.fread(ptr, 0x4000);

	// if we cross the page boundary, get a new write pointer and write the rest
	if (data_size > 0x4000)
	{
		ptr = program.get_write_ptr(0x8000);
		image.fread(ptr, 0x4000);
	}

	logerror("snapshot name = '%s', system_size = 0x%04x, data_size = 0x%04x\n", tape_name, system_variables_size, data_size);

	return IMAGE_INIT_PASS;
}

/***************************************************************************
    MACHINE INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    MACHINE_START( mtx512 )
-------------------------------------------------*/

MACHINE_START_MEMBER(mtx_state,mtx512)
{
	ram_device *messram = m_ram;

	/* configure memory */
	membank("bank1")->set_base(memregion("user1")->base());
	membank("bank2")->configure_entries(0, 8, memregion("user2")->base(), 0x2000);
	membank("bank3")->configure_entries(0, messram->size()/0x4000/2, messram->pointer(), 0x4000);
	membank("bank4")->configure_entries(0, messram->size()/0x4000/2, messram->pointer() + messram->size()/2, 0x4000);
}

MACHINE_RESET_MEMBER(mtx_state,mtx512)
{
	/* bank switching */
	bankswitch(0);
}
