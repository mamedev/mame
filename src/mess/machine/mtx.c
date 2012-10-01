/*************************************************************************

    machine/mtx.c

    Memotech MTX 500, MTX 512 and RS 128

**************************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "includes/mtx.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"
#include "machine/ctronics.h"
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

READ8_DEVICE_HANDLER( mtx_strobe_r )
{
	centronics_device *centronics = space.machine().device<centronics_device>(CENTRONICS_TAG);
	/* set STROBE low */
	centronics->strobe_w(FALSE);

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

static void bankswitch(running_machine &machine, UINT8 data)
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
	mtx_state *state = machine.driver_data<mtx_state>();
	address_space &program = machine.device(Z80_TAG)->memory().space(AS_PROGRAM);
	ram_device *messram = machine.device<ram_device>(RAM_TAG);

//  UINT8 cbm_mode = data >> 7 & 0x01;
	UINT8 rom_page = data >> 4 & 0x07;
	UINT8 ram_page = data >> 0 & 0x0f;

	/* set rom bank (switches between basic and assembler rom or cartridges) */
	state->membank("bank2")->set_entry(rom_page);

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
		state->membank("bank4")->set_entry(ram_page);
	}
	else
	{
		program.install_readwrite_bank(0x4000, 0x7fff, "bank3");
		program.install_readwrite_bank(0x8000, 0xbfff, "bank4");
		state->membank("bank3")->set_entry(ram_page);
		state->membank("bank4")->set_entry(ram_page);
	}
}

WRITE8_MEMBER(mtx_state::mtx_bankswitch_w)
{
	bankswitch(machine(), data);
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

WRITE8_DEVICE_HANDLER( mtx_cst_w )
{
	dynamic_cast<cassette_image_device *>(device)->output( BIT(data, 0) ? -1 : 1);
}

/*-------------------------------------------------
    mtx_prt_r - centronics status
-------------------------------------------------*/

READ8_DEVICE_HANDLER( mtx_prt_r )
{
	centronics_device *centronics = space.machine().device<centronics_device>(CENTRONICS_TAG);

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
	centronics->strobe_w( TRUE);

	/* busy */
	data |= centronics->busy_r() << 0;

	/* fault */
	data |= centronics->fault_r() << 1;

	/* paper empty */
	data |= !centronics->pe_r() << 2;

	/* select */
	data |= centronics->vcc_r() << 3;

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

SNAPSHOT_LOAD( mtx )
{
	address_space &program = image.device().machine().device(Z80_TAG)->memory().space(AS_PROGRAM);

	UINT8 header[18];
	UINT16 addr;

	/* get the header */
	image.fread( &header, sizeof(header));

	if (header[0] == 0xff)
	{
		/* long header */
		addr = pick_integer_le(header, 16, 2);
		void *ptr = program.get_write_ptr(addr);
		image.fread( ptr, 599);
		ptr = program.get_write_ptr(0xc000);
		image.fread( ptr, snapshot_size - 599 - 18);
	}
	else
	{
		/* short header */
		addr = pick_integer_le(header, 0, 2);
		image.fseek(4, SEEK_SET);
		void *ptr = program.get_write_ptr(addr);
		image.fread( ptr, 599);
		ptr = program.get_write_ptr(0xc000);
		image.fread( ptr, snapshot_size - 599 - 4);
	}

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
	ram_device *messram = machine().device<ram_device>(RAM_TAG);

	/* find devices */
	m_z80ctc = machine().device<z80ctc_device>(Z80CTC_TAG);
	m_z80dart = machine().device(Z80DART_TAG);
	m_cassette = machine().device<cassette_image_device>(CASSETTE_TAG);

	/* configure memory */
	membank("bank1")->set_base(machine().root_device().memregion("user1")->base());
	membank("bank2")->configure_entries(0, 8, memregion("user2")->base(), 0x2000);
	membank("bank3")->configure_entries(0, messram->size()/0x4000/2, messram->pointer(), 0x4000);
	membank("bank4")->configure_entries(0, messram->size()/0x4000/2, messram->pointer() + messram->size()/2, 0x4000);
}

/*-------------------------------------------------
    MACHINE_RESET( mtx512 )
-------------------------------------------------*/

MACHINE_RESET_MEMBER(mtx_state,mtx512)
{
	/* bank switching */
	bankswitch(machine(), 0);
}
