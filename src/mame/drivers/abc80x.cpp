// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Luxor ABC 800C/800M

Main PCB Layout
---------------

|-----------------------|-----------------------------------|
|   4116    4116        |                                   |
|   4116    4116        |                                   |
|   4116    4116        |                                   |
|   4116    4116        |           Z80         Z80CTC      |
|   4116    4116        |CN1                                |
|   4116    4116        |                                   |
|   4116    4116        |                       Z80DART     |
|                       |                       CN6         |
|   ROM3    ROM7        |                                   |
|                       |                       Z80SIO2     |
|   ROM2    ROM6        |-----------------------------------|
|                                                           |
|   ROM1    ROM5                                            |
|                                                       CN2 |
|   ROM0    ROM4                                            |
|                       CN7                                 |
|-------------------------------------------|           CN3 |
|                                           |               |
|                                           |               |
|                                           |           CN4 |
|                                           |               |
|               VIDEO BOARD                 |               |
|                                           |           CN5 |
|                                           |               |
|                                           |               |
|                                           |               |
|-------------------------------------------|---------------|

Notes:
    Relevant IC's shown.

    Z80     - ? Z80A CPU (labeled "Z80A (800) KASS.")
    Z80CTC  - Sharp LH0082A Z80A-CTC
    Z80DART - SGS Z8470AB1 Z80A-DART
    Z8OSIO2 - SGS Z8442AB1 Z80A-SIO/2
    4116    - Toshiba TMS4116-20NL 1Kx8 RAM
    ROM0    - NEC D2732D 4Kx8 EPROM "ABC M-12"
    ROM1    - NEC D2732D 4Kx8 EPROM "ABC 1-12"
    ROM2    - NEC D2732D 4Kx8 EPROM "ABC 2-12"
    ROM3    - NEC D2732D 4Kx8 EPROM "ABC 3-12"
    ROM4    - NEC D2732D 4Kx8 EPROM "ABC 4-12"
    ROM5    - NEC D2732D 4Kx8 EPROM "ABC 5-12"
    ROM6    - Hitachi HN462732G 4Kx8 EPROM "ABC 6-52"
    ROM7    - NEC D2732D 4Kx8 EPROM "ABC 7-22"
    CN1     - ABC bus connector
    CN2     - tape connector
    CN3     - RS-232 channel B connector
    CN4     - RS-232 channel A connector
    CN5     - video connector
    CN6     - keyboard connector
    CN7     - video board connector


Video PCB Layout (Color)
------------------------

55 10789-01

|-------------------------------------------|
|                                           |
|                                           |
|               2114                        |
|                                           |
|               2114        TROM            |
|                                           |
|       12MHz               TIC             |
|                                           |
|                                           |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    2114    - GTE Microcircuits 2114-2CB 1Kx4 RAM
    TIC     - Philips SAA5020 Teletext Timing Chain
    TROM    - Philips SAA5052 Teletext Character Generator w/Swedish Character Set


Video PCB Layout (Monochrome)
-----------------------------

55 10790-02

|-------------------------------------------|
|                                           |
|               2114                        |
|               2114                        |
|                                      12MHz|
|               2114                        |
|               2114                        |
|                                           |
|               CRTC            ROM0        |
|                                           |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    2114    - Toshiba TMM314AP-1 1Kx4 RAM
    CRTC    - Hitachi HD46505SP CRTC
    ROM0    - NEC D2716D 2Kx8 EPROM "VUM SE"


Video PCB Layout (High Resolution)
----------------------------------

55 10791-01

|-------------------------------------------|
|                                           |
|   4116        PROM1                       |
|   4116                                    |
|   4116                                    |
|   4116                                    |
|   4116                                    |
|   4116                                    |
|   4116                                    |
|   4116                PROM0               |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    4116    - Motorola MCM4116AC25 1Kx8 RAM
    PROM0   - Philips 82S123 32x8 Bipolar PROM "HRU I"
    PROM1   - Philips 82S131 512x4 Bipolar PROM "HRU II"

*/

/*

    TODO:

    - cassette
    - abc806 RTC
    - abc806 disks except ufd631 won't boot

*/

#include "includes/abc80x.h"
#include "softlist.h"


//**************************************************************************
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  DISCRETE_SOUND( abc800 )
//-------------------------------------------------

static DISCRETE_SOUND_START( abc800 )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_OUTPUT(NODE_01, 5000)
DISCRETE_SOUND_END


//-------------------------------------------------
//  pling_r - speaker read
//-------------------------------------------------

READ8_MEMBER( abc800_state::pling_r )
{
	m_pling = !m_pling;

	m_discrete->write(space, NODE_01, m_pling);

	return 0xff;
}


//-------------------------------------------------
//  pling_r - speaker read
//-------------------------------------------------

READ8_MEMBER( abc802_state::pling_r )
{
	m_pling = !m_pling;

	m_discrete->write(space, NODE_01, m_pling);

	return 0xff;
}



//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

//-------------------------------------------------
//  bankswitch
//-------------------------------------------------

void abc800_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (m_fetch_charram)
	{
		// HR video RAM selected
		program.install_ram(0x0000, 0x3fff, m_video_ram);
	}
	else
	{
		// BASIC ROM selected
		program.install_rom(0x0000, 0x3fff, m_rom->base());
	}
}


//-------------------------------------------------
//  bankswitch
//-------------------------------------------------

void abc802_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	if (m_lrs)
	{
		// ROM and video RAM selected
		program.install_rom(0x0000, 0x77ff, m_rom->base());
		program.install_ram(0x7800, 0x7fff, m_char_ram);
	}
	else
	{
		// low RAM selected
		program.install_ram(0x0000, 0x7fff, m_ram->pointer());
	}
}


//-------------------------------------------------
//  bankswitch
//-------------------------------------------------

void abc806_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT32 videoram_mask = m_ram->size() - (32 * 1024) - 1;
	int bank;
	char bank_name[10];

	if (!m_keydtr)
	{
		// 32K block mapping

		UINT32 videoram_start = (m_hrs & 0xf0) << 11;

		for (bank = 1; bank <= 8; bank++)
		{
			// 0x0000-0x7FFF is video RAM

			UINT16 start_addr = 0x1000 * (bank - 1);
			UINT16 end_addr = start_addr + 0xfff;
			UINT32 videoram_offset = (videoram_start + start_addr) & videoram_mask;
			sprintf(bank_name,"bank%d",bank);
			//logerror("%04x-%04x: Video RAM %04x (32K)\n", start_addr, end_addr, videoram_offset);

			program.install_readwrite_bank(start_addr, end_addr, bank_name);
			membank(bank_name)->configure_entry(1, m_video_ram + videoram_offset);
			membank(bank_name)->set_entry(1);
		}

		for (bank = 9; bank <= 16; bank++)
		{
			// 0x8000-0xFFFF is main RAM

			UINT16 start_addr = 0x1000 * (bank - 1);
			UINT16 end_addr = start_addr + 0xfff;
			sprintf(bank_name,"bank%d",bank);
			//logerror("%04x-%04x: Work RAM (32K)\n", start_addr, end_addr);

			program.install_readwrite_bank(start_addr, end_addr, bank_name);
			membank(bank_name)->set_entry(0);
		}
	}
	else
	{
		// 4K block mapping

		for (bank = 1; bank <= 16; bank++)
		{
			UINT16 start_addr = 0x1000 * (bank - 1);
			UINT16 end_addr = start_addr + 0xfff;
			UINT8 map = m_map[bank - 1];
			UINT32 videoram_offset = ((map & 0x7f) << 12) & videoram_mask;
			sprintf(bank_name,"bank%d",bank);
			if (BIT(map, 7) && m_eme)
			{
				// map to video RAM
				//logerror("%04x-%04x: Video RAM %04x (4K)\n", start_addr, end_addr, videoram_offset);

				program.install_readwrite_bank(start_addr, end_addr, bank_name);
				membank(bank_name)->configure_entry(1, m_video_ram + videoram_offset);
				membank(bank_name)->set_entry(1);
			}
			else
			{
				// map to ROM/RAM

				switch (bank)
				{
				case 1: case 2: case 3: case 4: case 5: case 6: case 7:
					// ROM
					//logerror("%04x-%04x: ROM (4K)\n", start_addr, end_addr);

					program.install_read_bank(start_addr, end_addr, bank_name);
					program.unmap_write(start_addr, end_addr);
					membank(bank_name)->set_entry(0);
					break;

				case 8:
					// ROM/char RAM
					//logerror("%04x-%04x: ROM (4K)\n", start_addr, end_addr);

					program.install_read_bank(0x7000, 0x77ff, bank_name);
					program.unmap_write(0x7000, 0x77ff);
					program.install_readwrite_handler(0x7800, 0x7fff, 0, 0, READ8_DELEGATE(abc806_state, charram_r), WRITE8_DELEGATE(abc806_state, charram_w));
					membank(bank_name)->set_entry(0);
					break;

				default:
					// work RAM
					//logerror("%04x-%04x: Work RAM (4K)\n", start_addr, end_addr);

					program.install_readwrite_bank(start_addr, end_addr, bank_name);
					membank(bank_name)->set_entry(0);
					break;
				}
			}
		}
	}

	if (m_fetch_charram)
	{
		// 30K block mapping

		UINT32 videoram_start = (m_hrs & 0xf0) << 11;

		for (bank = 1; bank <= 8; bank++)
		{
			// 0x0000-0x77FF is video RAM

			UINT16 start_addr = 0x1000 * (bank - 1);
			UINT16 end_addr = start_addr + 0xfff;
			UINT32 videoram_offset = (videoram_start + start_addr) & videoram_mask;
			sprintf(bank_name,"bank%d",bank);
			//logerror("%04x-%04x: Video RAM %04x (30K)\n", start_addr, end_addr, videoram_offset);

			if (start_addr == 0x7000)
			{
				program.install_readwrite_bank(0x7000, 0x77ff, bank_name);
				program.install_readwrite_handler(0x7800, 0x7fff, 0, 0, READ8_DELEGATE(abc806_state, charram_r), WRITE8_DELEGATE(abc806_state, charram_w));
			}
			else
			{
				program.install_readwrite_bank(start_addr, end_addr, bank_name);
			}

			membank(bank_name)->configure_entry(1, m_video_ram + videoram_offset);
			membank(bank_name)->set_entry(1);
		}
	}
}


//-------------------------------------------------
//  mai_r - memory bank map read
//-------------------------------------------------

READ8_MEMBER( abc806_state::mai_r )
{
	int bank = offset >> 12;

	return m_map[bank];
}


//-------------------------------------------------
//  mao_w - memory bank map write
//-------------------------------------------------

WRITE8_MEMBER( abc806_state::mao_w )
{
	/*

	    bit     description

	    0       physical block address bit 0
	    1       physical block address bit 1
	    2       physical block address bit 2
	    3       physical block address bit 3
	    4       physical block address bit 4
	    5
	    6
	    7       allocate block

	*/

	int bank = offset >> 12;

	m_map[bank] = data;

	bankswitch();
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( abc800c_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc800c_mem, AS_PROGRAM, 8, abc800c_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0x4000, 0x7bff) AM_ROM
	AM_RANGE(0x7c00, 0x7fff) AM_RAM AM_SHARE("char_ram")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc800c_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc800c_io, AS_IO, 8, abc800_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x18) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_t, inp_r, out_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x18) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_t, stat_r, cs_w)
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c1_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c2_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0x18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c3_w)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0x18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c4_w)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0x18) AM_READ(pling_r)
	AM_RANGE(0x06, 0x06) AM_MIRROR(0x18) AM_WRITE(hrs_w)
	AM_RANGE(0x07, 0x07) AM_MIRROR(0x18) AM_DEVREAD(ABCBUS_TAG, abcbus_slot_t, rst_r) AM_WRITE(hrc_w)
	AM_RANGE(0x20, 0x23) AM_MIRROR(0x0c) AM_DEVREADWRITE(Z80DART_TAG, z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x1c) AM_DEVREADWRITE(Z80SIO_TAG, z80sio2_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x60, 0x63) AM_MIRROR(0x1c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc800m_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc800m_mem, AS_PROGRAM, 8, abc800_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("video_ram")
	AM_RANGE(0x4000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("char_ram")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc800m_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc800m_io, AS_IO, 8, abc800_state )
	AM_IMPORT_FROM( abc800c_io )
	AM_RANGE(0x31, 0x31) AM_MIRROR(0x06) AM_DEVREAD(MC6845_TAG, mc6845_device, register_r)
	AM_RANGE(0x38, 0x38) AM_MIRROR(0x06) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0x39, 0x39) AM_MIRROR(0x06) AM_DEVWRITE(MC6845_TAG, mc6845_device, register_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc802_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc802_mem, AS_PROGRAM, 8, abc802_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("char_ram")
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc802_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc802_io, AS_IO, 8, abc802_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x18) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_t, inp_r, out_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x18) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_t, stat_r, cs_w)
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c1_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c2_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0x18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c3_w)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0x18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c4_w)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0x08) AM_READ(pling_r)
	AM_RANGE(0x07, 0x07) AM_MIRROR(0x18) AM_DEVREAD(ABCBUS_TAG, abcbus_slot_t, rst_r)
	AM_RANGE(0x20, 0x23) AM_MIRROR(0x0c) AM_DEVREADWRITE(Z80DART_TAG, z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x31, 0x31) AM_MIRROR(0x06) AM_DEVREAD(MC6845_TAG, mc6845_device, register_r)
	AM_RANGE(0x38, 0x38) AM_MIRROR(0x06) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0x39, 0x39) AM_MIRROR(0x06) AM_DEVWRITE(MC6845_TAG, mc6845_device, register_w)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x1c) AM_DEVREADWRITE(Z80SIO_TAG, z80sio2_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x60, 0x63) AM_MIRROR(0x1c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc806_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc806_mem, AS_PROGRAM, 8, abc806_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_RAMBANK("bank1")
	AM_RANGE(0x1000, 0x1fff) AM_RAMBANK("bank2")
	AM_RANGE(0x2000, 0x2fff) AM_RAMBANK("bank3")
	AM_RANGE(0x3000, 0x3fff) AM_RAMBANK("bank4")
	AM_RANGE(0x4000, 0x4fff) AM_RAMBANK("bank5")
	AM_RANGE(0x5000, 0x5fff) AM_RAMBANK("bank6")
	AM_RANGE(0x6000, 0x6fff) AM_RAMBANK("bank7")
	AM_RANGE(0x7000, 0x7fff) AM_RAMBANK("bank8")
	AM_RANGE(0x8000, 0x8fff) AM_RAMBANK("bank9")
	AM_RANGE(0x9000, 0x9fff) AM_RAMBANK("bank10")
	AM_RANGE(0xa000, 0xafff) AM_RAMBANK("bank11")
	AM_RANGE(0xb000, 0xbfff) AM_RAMBANK("bank12")
	AM_RANGE(0xc000, 0xcfff) AM_RAMBANK("bank13")
	AM_RANGE(0xd000, 0xdfff) AM_RAMBANK("bank14")
	AM_RANGE(0xe000, 0xefff) AM_RAMBANK("bank15")
	AM_RANGE(0xf000, 0xffff) AM_RAMBANK("bank16")
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc806_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc806_io, AS_IO, 8, abc806_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff18) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_t, inp_r, out_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0xff18) AM_DEVREADWRITE(ABCBUS_TAG, abcbus_slot_t, stat_r, cs_w)
	AM_RANGE(0x02, 0x02) AM_MIRROR(0xff18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c1_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0xff18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c2_w)
	AM_RANGE(0x04, 0x04) AM_MIRROR(0xff18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c3_w)
	AM_RANGE(0x05, 0x05) AM_MIRROR(0xff18) AM_DEVWRITE(ABCBUS_TAG, abcbus_slot_t, c4_w)
	AM_RANGE(0x06, 0x06) AM_MIRROR(0xff18) AM_WRITE(hrs_w)
	AM_RANGE(0x07, 0x07) AM_MIRROR(0xff18) AM_DEVREAD(ABCBUS_TAG, abcbus_slot_t, rst_r) AM_WRITE(hrc_w)
	AM_RANGE(0x20, 0x23) AM_MIRROR(0xff0c) AM_DEVREADWRITE(Z80DART_TAG, z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x31, 0x31) AM_MIRROR(0xff00) AM_DEVREAD(MC6845_TAG, mc6845_device, register_r)
	AM_RANGE(0x34, 0x34) AM_MIRROR(0xff00) AM_MASK(0xff00) AM_READWRITE(mai_r, mao_w)
	AM_RANGE(0x35, 0x35) AM_MIRROR(0xff00) AM_READWRITE(ami_r, amo_w)
	AM_RANGE(0x36, 0x36) AM_MIRROR(0xff00) AM_READWRITE(sti_r, sto_w)
	AM_RANGE(0x37, 0x37) AM_MIRROR(0xff00) AM_MASK(0xff00) AM_READWRITE(cli_r, sso_w)
	AM_RANGE(0x38, 0x38) AM_MIRROR(0xff00) AM_DEVWRITE(MC6845_TAG, mc6845_device, address_w)
	AM_RANGE(0x39, 0x39) AM_MIRROR(0xff00) AM_DEVWRITE(MC6845_TAG, mc6845_device, register_w)
	AM_RANGE(0x40, 0x43) AM_MIRROR(0xff1c) AM_DEVREADWRITE(Z80SIO_TAG, z80sio2_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x60, 0x63) AM_MIRROR(0xff1c) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( abc800 )
//-------------------------------------------------

INPUT_PORTS_START( abc800 )
	PORT_START("SB")
	PORT_DIPNAME( 0xff, 0xaa, "Serial Communications" ) PORT_DIPLOCATION("SB:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0xaa, "Asynchronous, Single Speed" )
	PORT_DIPSETTING(    0x2e, "Asynchronous, Split Speed" )
	PORT_DIPSETTING(    0x50, "Synchronous" )
	PORT_DIPSETTING(    0x8b, "ABC NET" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc802 )
//-------------------------------------------------

static INPUT_PORTS_START( abc802 )
	PORT_START("SB")
	PORT_DIPNAME( 0xff, 0xaa, "Serial Communications" ) PORT_DIPLOCATION("SB:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0xaa, "Asynchronous, Single Speed" )
	PORT_DIPSETTING(    0x2e, "Asynchronous, Split Speed" )
	PORT_DIPSETTING(    0x50, "Synchronous" )
	PORT_DIPSETTING(    0x8b, "ABC NET" )

	PORT_START("CONFIG")
	PORT_DIPNAME( 0x01, 0x00, "Clear Screen Time Out" ) PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("S2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Characters Per Line" ) PORT_DIPLOCATION("S3:1")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPNAME( 0x08, 0x08, "Frame Frequency" )
	PORT_DIPSETTING(    0x00, "60 Hz" )
	PORT_DIPSETTING(    0x08, "50 Hz" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc806 )
//-------------------------------------------------

static INPUT_PORTS_START( abc806 )
	PORT_START("SB")
	PORT_DIPNAME( 0xff, 0xaa, "Serial Communications" ) PORT_DIPLOCATION("SB:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0xaa, "Asynchronous, Single Speed" )
	PORT_DIPSETTING(    0x2e, "Asynchronous, Split Speed" )
	PORT_DIPSETTING(    0x50, "Synchronous" )
	PORT_DIPSETTING(    0x8b, "ABC NET" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

WRITE_LINE_MEMBER( abc800_state::ctc_z0_w )
{
	if (BIT(m_sb, 2))
	{
		m_sio->txca_w(state);
		m_ctc->trg3(state);
	}

	clock_cassette(state);
}

WRITE_LINE_MEMBER( abc800_state::ctc_z1_w )
{
	if (BIT(m_sb, 3))
	{
		m_sio->rxca_w(state);
	}

	if (BIT(m_sb, 4))
	{
		m_sio->txca_w(state);
		m_ctc->trg3(state);
	}
}

WRITE_LINE_MEMBER( abc800_state::ctc_z2_w )
{
	m_dart->rxca_w(state);
	m_dart->txca_w(state);
}

//-------------------------------------------------
//  Z80SIO
//-------------------------------------------------

void abc800_state::clock_cassette(int state)
{
	if (m_cassette == nullptr) return;

	if (m_ctc_z0 && !state)
	{
		m_sio_txcb = !m_sio_txcb;
		m_sio->txcb_w(!m_sio_txcb);

		if (m_sio_txdb || m_sio_txcb)
		{
			m_dfd_out = !m_dfd_out;
			m_cassette->output((m_dfd_out & m_sio_rtsb) ? +1.0 : -1.0);
		}

		if (m_tape_ctr < 15)
		{
			m_tape_ctr++;
		}

		m_sio->rxcb_w(m_tape_ctr == 15);
	}

	m_ctc_z0 = state;
}

WRITE_LINE_MEMBER( abc800_state::sio_txdb_w )
{
	m_sio_txdb = state;
}

WRITE_LINE_MEMBER( abc800_state::sio_dtrb_w )
{
	if (m_cassette == nullptr) return;

	if (state)
	{
		m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		m_cassette_timer->enable(true);
	}
	else
	{
		m_cassette->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_cassette_timer->enable(false);
	}
}

WRITE_LINE_MEMBER( abc800_state::sio_rtsb_w )
{
	if (m_cassette == nullptr) return;

	m_sio_rtsb = state;

	if (!m_sio_rtsb)
	{
		m_cassette->output(-1.0);
	}
}

//-------------------------------------------------
//  Z80DART abc802
//-------------------------------------------------

WRITE_LINE_MEMBER( abc802_state::lrs_w )
{
	m_lrs = state;

	bankswitch();
}

WRITE_LINE_MEMBER( abc802_state::mux80_40_w )
{
	m_80_40_mux = state;
}

//-------------------------------------------------
//  Z80DART abc806
//-------------------------------------------------

WRITE_LINE_MEMBER( abc806_state::keydtr_w )
{
	m_keydtr = state;

	bankswitch();
}

//-------------------------------------------------
//  z80_daisy_config abc800_daisy_chain
//-------------------------------------------------

static const z80_daisy_config abc800_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80SIO_TAG },
	{ Z80DART_TAG },
	{ nullptr }
};


//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void abc800_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_CTC:
		m_ctc->trg0(1);
		m_ctc->trg0(0);

		m_ctc->trg1(1);
		m_ctc->trg1(0);

		m_ctc->trg2(1);
		m_ctc->trg2(0);
		break;

	case TIMER_ID_CASSETTE:
		{
			int dfd_in = m_cassette->input() > 0;

			if (m_dfd_in && !dfd_in)
			{
				m_sio->rxb_w(!(m_tape_ctr == 15));
			}

			if (!dfd_in && (m_tape_ctr == 15))
			{
				m_tape_ctr = 4;
			}

			m_dfd_in = dfd_in;
		}
		break;
	}
}


//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void abc800_state::machine_start()
{
	// start timers
	m_ctc_timer = timer_alloc(TIMER_ID_CTC);
	m_ctc_timer->adjust(attotime::from_hz(ABC800_X01/2/2/2), 0, attotime::from_hz(ABC800_X01/2/2/2));

	m_cassette_timer = timer_alloc(TIMER_ID_CASSETTE);
	m_cassette_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));
	m_cassette_timer->enable(false);

	// register for state saving
	save_item(NAME(m_fetch_charram));
	save_item(NAME(m_pling));
	save_item(NAME(m_sb));
	save_item(NAME(m_ctc_z0));
	save_item(NAME(m_sio_txcb));
	save_item(NAME(m_sio_txdb));
	save_item(NAME(m_sio_rtsb));
	save_item(NAME(m_dfd_out));
	save_item(NAME(m_dfd_in));
	save_item(NAME(m_tape_ctr));
}


//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void abc800_state::machine_reset()
{
	m_sb = m_io_sb->read();

	m_fetch_charram = 0;
	bankswitch();

	m_dart->ria_w(1);

	// 50/60 Hz
	m_dart->ctsb_w(0); // 0 = 50Hz, 1 = 60Hz

	m_dfd_in = 0;
}


//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void abc802_state::machine_start()
{
	// start timers
	m_ctc_timer = timer_alloc(TIMER_ID_CTC);
	m_ctc_timer->adjust(attotime::from_hz(ABC800_X01/2/2/2), 0, attotime::from_hz(ABC800_X01/2/2/2));

	m_cassette_timer = timer_alloc(TIMER_ID_CASSETTE);
	m_cassette_timer->adjust(attotime::from_hz(44100), 0, attotime::from_hz(44100));

	// register for state saving
	save_item(NAME(m_pling));
	save_item(NAME(m_sb));
	save_item(NAME(m_ctc_z0));
	save_item(NAME(m_sio_txcb));
	save_item(NAME(m_sio_txdb));
	save_item(NAME(m_sio_rtsb));
	save_item(NAME(m_dfd_out));
	save_item(NAME(m_dfd_in));
	save_item(NAME(m_tape_ctr));
	save_item(NAME(m_lrs));
}


//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void abc802_state::machine_reset()
{
	UINT8 config = m_config->read();
	m_sb = m_io_sb->read();

	// memory banking
	m_lrs = 1;
	bankswitch();

	// clear screen time out (S1)
	m_sio->dcdb_w(BIT(config, 0));

	// unknown (S2)
	m_sio->ctsb_w(BIT(config, 1));

	// 40/80 char (S3)
	m_dart->ria_w(BIT(config, 2)); // 0 = 40, 1 = 80

	// 50/60 Hz
	m_dart->ctsb_w(BIT(config, 3)); // 0 = 50Hz, 1 = 60Hz

	m_dfd_in = 0;
}


//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void abc806_state::machine_start()
{
	// start timers
	m_ctc_timer = timer_alloc(TIMER_ID_CTC);
	m_ctc_timer->adjust(attotime::from_hz(ABC800_X01/2/2/2), 0, attotime::from_hz(ABC800_X01/2/2/2));

	// setup memory banking
	UINT8 *mem = m_rom->base();
	UINT32 videoram_size = m_ram->size() - (32 * 1024);
	int bank;
	char bank_name[10];

	m_video_ram.allocate(videoram_size);

	for (bank = 1; bank <= 16; bank++)
	{
		sprintf(bank_name,"bank%d",bank);
		membank(bank_name)->configure_entry(0, mem + (0x1000 * (bank - 1)));
		membank(bank_name)->configure_entry(1, m_video_ram);
		membank(bank_name)->set_entry(0);
	}

	// register for state saving
	save_item(NAME(m_fetch_charram));
	save_item(NAME(m_pling));
	save_item(NAME(m_sb));
	save_item(NAME(m_ctc_z0));
	save_item(NAME(m_sio_txcb));
	save_item(NAME(m_sio_txdb));
	save_item(NAME(m_sio_rtsb));
	save_item(NAME(m_dfd_out));
	save_item(NAME(m_dfd_in));
	save_item(NAME(m_tape_ctr));
	save_item(NAME(m_keydtr));
	save_item(NAME(m_eme));
	save_item(NAME(m_map));
}


//-------------------------------------------------
//  machine_reset
//-------------------------------------------------

void abc806_state::machine_reset()
{
	m_sb = m_io_sb->read();

	// setup memory banking
	int bank;
	char bank_name[10];

	for (bank = 1; bank <= 16; bank++)
	{
		sprintf(bank_name,"bank%d",bank);
		membank(bank_name)->set_entry(0);
	}

	bankswitch();

	// clear STO lines
	m_eme = 0;
	m_40 = 0;
	m_hru2_a8 = 0;
	m_txoff = 0;
	m_rtc->cs_w(1);
	m_rtc->clk_w(1);
	m_rtc->dio_w(1);

	m_dart->ria_w(1);

	// 50/60 Hz
	m_dart->ctsb_w(0); // 0 = 50Hz, 1 = 60Hz

	m_dfd_in = 0;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( abc800c )
//-------------------------------------------------

static MACHINE_CONFIG_START( abc800c, abc800c_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, ABC800_X01/2/2)
	MCFG_CPU_CONFIG(abc800_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(abc800c_mem)
	MCFG_CPU_IO_MAP(abc800c_io)

	// video hardware
	MCFG_FRAGMENT_ADD(abc800c_video)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(DISCRETE_TAG, DISCRETE, 0)
	MCFG_DISCRETE_INTF(abc800)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	// peripheral hardware
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, ABC800_X01/2/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(abc800_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(abc800_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(abc800_state, ctc_z2_w))

	MCFG_Z80SIO2_ADD(Z80SIO_TAG, ABC800_X01/2/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_RTSB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_Z80DART_ADD(Z80DART_TAG, ABC800_X01/2/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(ABC_KEYBOARD_PORT_TAG, abc_keyboard_port_device, txd_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, ctsa_w))

	MCFG_ABC_KEYBOARD_PORT_ADD(ABC_KEYBOARD_PORT_TAG, "abc800")
	MCFG_ABC_KEYBOARD_OUT_RX_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxb_w))
	MCFG_ABC_KEYBOARD_OUT_TRXC_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxtxcb_w))
	MCFG_ABC_KEYBOARD_OUT_KEYDOWN_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, dcdb_w))

	// ABC bus
	MCFG_ABCBUS_SLOT_ADD(ABCBUS_TAG, abcbus_cards, "abc830")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "abc800")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "abc800_hdd")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( abc800m )
//-------------------------------------------------

static MACHINE_CONFIG_START( abc800m, abc800m_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, ABC800_X01/2/2)
	MCFG_CPU_CONFIG(abc800_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(abc800m_mem)
	MCFG_CPU_IO_MAP(abc800m_io)

	// video hardware
	MCFG_FRAGMENT_ADD(abc800m_video)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(DISCRETE_TAG, DISCRETE, 0)
	MCFG_DISCRETE_INTF(abc800)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	// peripheral hardware
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, ABC800_X01/2/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(abc800_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(abc800_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(abc800_state, ctc_z2_w))

	MCFG_Z80SIO2_ADD(Z80SIO_TAG, ABC800_X01/2/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_RTSB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_Z80DART_ADD(Z80DART_TAG, ABC800_X01/2/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(ABC_KEYBOARD_PORT_TAG, abc_keyboard_port_device, txd_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, ctsa_w))

	MCFG_ABC_KEYBOARD_PORT_ADD(ABC_KEYBOARD_PORT_TAG, "abc800")
	MCFG_ABC_KEYBOARD_OUT_RX_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxb_w))
	MCFG_ABC_KEYBOARD_OUT_TRXC_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxtxcb_w))
	MCFG_ABC_KEYBOARD_OUT_KEYDOWN_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, dcdb_w))

	// ABC bus
	MCFG_ABCBUS_SLOT_ADD(ABCBUS_TAG, abcbus_cards, "abc830")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "abc800")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "abc800_hdd")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( abc802 )
//-------------------------------------------------

static MACHINE_CONFIG_START( abc802, abc802_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, ABC800_X01/2/2)
	MCFG_CPU_CONFIG(abc800_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(abc802_mem)
	MCFG_CPU_IO_MAP(abc802_io)

	// video hardware
	MCFG_FRAGMENT_ADD(abc802_video)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(DISCRETE_TAG, DISCRETE, 0)
	MCFG_DISCRETE_INTF(abc800)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	// peripheral hardware
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, ABC800_X01/2/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(abc800_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(abc800_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(abc800_state, ctc_z2_w))

	MCFG_Z80SIO2_ADD(Z80SIO_TAG, ABC800_X01/2/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_RTSB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_Z80DART_ADD(Z80DART_TAG, ABC800_X01/2/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(ABC_KEYBOARD_PORT_TAG, abc_keyboard_port_device, txd_w))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE(abc802_state, lrs_w))
	MCFG_Z80DART_OUT_RTSB_CB(WRITELINE(abc802_state, mux80_40_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, ctsa_w))

	MCFG_ABC_KEYBOARD_PORT_ADD(ABC_KEYBOARD_PORT_TAG, "abc55")
	MCFG_ABC_KEYBOARD_OUT_RX_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxb_w))
	MCFG_ABC_KEYBOARD_OUT_TRXC_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxtxcb_w))
	MCFG_ABC_KEYBOARD_OUT_KEYDOWN_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, dcdb_w))

	// ABC bus
	MCFG_ABCBUS_SLOT_ADD(ABCBUS_TAG, abcbus_cards, "abc834")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// software list
	//MCFG_SOFTWARE_LIST_ADD("flop_list", "abc802")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "abc800_hdd")
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG( abc806 )
//-------------------------------------------------

static MACHINE_CONFIG_START( abc806, abc806_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, ABC800_X01/2/2)
	MCFG_CPU_CONFIG(abc800_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(abc806_mem)
	MCFG_CPU_IO_MAP(abc806_io)

	// video hardware
	MCFG_FRAGMENT_ADD(abc806_video)

	// peripheral hardware
	MCFG_E0516_ADD(E0516_TAG, ABC806_X02)

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, ABC800_X01/2/2)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(abc800_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(abc800_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(abc800_state, ctc_z2_w))

	MCFG_Z80SIO2_ADD(Z80SIO_TAG, ABC800_X01/2/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_RTSB_CB(WRITELINE(abc800_state, sio_txdb_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_Z80DART_ADD(Z80DART_TAG, ABC800_X01/2/2, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(ABC_KEYBOARD_PORT_TAG, abc_keyboard_port_device, txd_w))
	MCFG_Z80DART_OUT_DTRB_CB(WRITELINE(abc806_state, keydtr_w))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, ctsa_w))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80dart_device, ctsa_w))

	MCFG_ABC_KEYBOARD_PORT_ADD(ABC_KEYBOARD_PORT_TAG, "abc77")
	MCFG_ABC_KEYBOARD_OUT_RX_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxb_w))
	MCFG_ABC_KEYBOARD_OUT_TRXC_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, rxtxcb_w))
	MCFG_ABC_KEYBOARD_OUT_KEYDOWN_HANDLER(DEVWRITELINE(Z80DART_TAG, z80dart_device, dcdb_w))

	// ABC bus
	MCFG_ABCBUS_SLOT_ADD(ABCBUS_TAG, abcbus_cards, "abc832")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
	MCFG_RAM_EXTRA_OPTIONS("512K")

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "abc806")
	MCFG_SOFTWARE_LIST_ADD("hdd_list", "abc800_hdd")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

/*

    ABC800 DOS ROMs

    Label       Drive Type
    ----------------------------
    ABC 6-1X    ABC830,DD82,DD84
    800 8"      DD88
    ABC 6-2X    ABC832
    ABC 6-3X    ABC838
    ABC 6-5X    UFD dos
    ABC 6-52    UFD dos with HD
    UFD 6.XX    Winchester


    Floppy Controllers

    Art N/O
    --------
    55 10761-01     "old" controller
    55 10828-01     "old" controller
    55 20900-0x
    55 21046-11     Luxor Conkort   25 pin D-sub connector
    55 21046-21     Luxor Conkort   34 pin FDD connector
    55 21046-41     Luxor Conkort   both of the above

*/

//-------------------------------------------------
//  ROM( abc800c )
//-------------------------------------------------

ROM_START( abc800c )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_LOAD( "abc c-1.1m", 0x0000, 0x1000, CRC(37009d29) SHA1(a1acd817ff8c2ed93935a163c5cf3d83d8bd6fb5) )
	ROM_LOAD( "abc 1-1.1l", 0x1000, 0x1000, CRC(ff15a28d) SHA1(bb4523ab1d190bc787f1923567b86795f66c26e2) )
	ROM_LOAD( "abc 2-1.1k", 0x2000, 0x1000, CRC(6ff2f528) SHA1(32d1639769c4a20b7a45c44f4c6993f77397f7a1) )
	ROM_LOAD( "abc 3-1.1j", 0x3000, 0x1000, CRC(9a43c47a) SHA1(964eb28e3d9779d1b13e0e938495133bbdb3aed3) )
	ROM_LOAD( "abc 4-1.2m", 0x4000, 0x1000, CRC(ba18db9c) SHA1(0c4543766fe9b10bee333f3f832f6f2209e449f8) )
	ROM_LOAD( "abc 5-1.2l", 0x5000, 0x1000, CRC(abbeb026) SHA1(44ebe417e3fa8d7878c23b56782aac8b443f1bfc) )
	ROM_LOAD( "abc 6-1.2k", 0x6000, 0x1000, CRC(4bd5e808) SHA1(5ca0a60571de6cfa3d6d166e0cde3c78560569f3) ) // 1981-01-12
	ROM_LOAD( "abc 7-22.2j", 0x7000, 0x1000, CRC(774511ab) SHA1(5171e43213a402c2d96dee33453c8306ac1aafc8) )

	ROM_REGION( 0x20, "hru", 0 )
	ROM_LOAD( "hru i.4g", 0x00, 0x20, CRC(d970a972) SHA1(c47fdd61fccc68368d42f03a01c7af90ab1fe1ab) )

	ROM_REGION( 0x200, "hru2", 0 )
	ROM_LOAD( "hru ii.3a", 0x000, 0x200, CRC(8e9d7cdc) SHA1(4ad16dc0992e31cdb2e644c7be81d334a56f7ad6) )
ROM_END


//-------------------------------------------------
//  ROM( abc800m )
//-------------------------------------------------

ROM_START( abc800m )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("ufd20")
	ROM_LOAD( "abc m-12.1m", 0x0000, 0x1000, CRC(f85b274c) SHA1(7d0f5639a528d8d8130a22fe688d3218c77839dc) )
	ROM_LOAD( "abc 1-12.1l", 0x1000, 0x1000, CRC(1e99fbdc) SHA1(ec6210686dd9d03a5ed8c4a4e30e25834aeef71d) )
	ROM_LOAD( "abc 2-12.1k", 0x2000, 0x1000, CRC(ac196ba2) SHA1(64fcc0f03fbc78e4c8056e1fa22aee12b3084ef5) )
	ROM_LOAD( "abc 3-12.1j", 0x3000, 0x1000, CRC(3ea2b5ee) SHA1(5a51ac4a34443e14112a6bae16c92b5eb636603f) )
	ROM_LOAD( "abc 4-12.2m", 0x4000, 0x1000, CRC(695cb626) SHA1(9603ce2a7b2d7b1cbeb525f5493de7e5c1e5a803) )
	ROM_LOAD( "abc 5-12.2l", 0x5000, 0x1000, CRC(b4b02358) SHA1(95338efa3b64b2a602a03bffc79f9df297e9534a) )
	ROM_SYSTEM_BIOS( 0, "13", "ABC-DOS (1982-07-19)" )
	ROMX_LOAD( "abc 6-13.2k", 0x6000, 0x1000, CRC(6fa71fb6) SHA1(b037dfb3de7b65d244c6357cd146376d4237dab6), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "ufd19", "UFD-DOS v.19 (1983-05-31)" )
	ROMX_LOAD( "abc 6-11 ufd.2k", 0x6000, 0x1000, CRC(2a45be80) SHA1(bf08a18a74e8bdaee2656a3c8246c0122398b58f), ROM_BIOS(2) ) // is this "ABC 6-5" or "ABC 6-51" instead?
	ROM_SYSTEM_BIOS( 2, "ufd20", "UFD-DOS v.20 (1984-03-02)" )
	ROMX_LOAD( "abc 6-52.2k", 0x6000, 0x1000, CRC(c311b57a) SHA1(4bd2a541314e53955a0d53ef2f9822a202daa485), ROM_BIOS(3) )
	ROM_LOAD_OPTIONAL( "abc 7-21.2j", 0x7000, 0x1000, CRC(fd137866) SHA1(3ac914d90db1503f61397c0ea26914eb38725044) )
	ROM_LOAD( "abc 7-22.2j", 0x7000, 0x1000, CRC(774511ab) SHA1(5171e43213a402c2d96dee33453c8306ac1aafc8) )

	ROM_REGION( 0x800, MC6845_TAG, 0 )
	ROM_LOAD( "vum se.7c", 0x000, 0x800, CRC(f9152163) SHA1(997313781ddcbbb7121dbf9eb5f2c6b4551fc799) )

	ROM_REGION( 0x20, "hru", 0 )
	ROM_LOAD( "hru i.4g", 0x00, 0x20, CRC(d970a972) SHA1(c47fdd61fccc68368d42f03a01c7af90ab1fe1ab) )

	ROM_REGION( 0x200, "hru2", 0 )
	ROM_LOAD( "hru ii.3a", 0x000, 0x200, CRC(8e9d7cdc) SHA1(4ad16dc0992e31cdb2e644c7be81d334a56f7ad6) )
ROM_END


//-------------------------------------------------
//  ROM( abc802 )
//-------------------------------------------------

ROM_START( abc802 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("ufd20")
	ROM_LOAD( "abc 02-11.9f",  0x0000, 0x2000, CRC(b86537b2) SHA1(4b7731ef801f9a03de0b5acd955f1e4a1828355d) )
	ROM_LOAD( "abc 12-11.11f", 0x2000, 0x2000, CRC(3561c671) SHA1(f12a7c0fe5670ffed53c794d96eb8959c4d9f828) )
	ROM_LOAD( "abc 22-11.12f", 0x4000, 0x2000, CRC(8dcb1cc7) SHA1(535cfd66c84c0370fd022d6edf702d3d1ad1b113) )
	ROM_SYSTEM_BIOS( 0, "abc", "ABC-DOS (1983-05-31)" )
	ROMX_LOAD( "abc 32-12.14f", 0x6000, 0x2000, CRC(23cd0f43) SHA1(639daec4565dcdb4de408b808d0c6cd97baa35d2), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "ufd19", "UFD-DOS v.19 (1984-03-02)" )
	ROMX_LOAD( "abc 32-21.14f", 0x6000, 0x2000, CRC(57050b98) SHA1(b977e54d1426346a97c98febd8a193c3e8259574), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "ufd20", "UFD-DOS v.20 (1984-04-03)" )
	ROMX_LOAD( "abc 32-31.14f", 0x6000, 0x2000, CRC(fc8be7a8) SHA1(a1d4cb45cf5ae21e636dddfa70c99bfd2050ad60), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "mica620", "MICA DOS v.20 (1984-03-02)" )
	ROMX_LOAD( "mica820.14f",   0x6000, 0x2000, CRC(edf998af) SHA1(daae7e1ff6ef3e0ddb83e932f324c56f4a98f79b), ROM_BIOS(4) )

	ROM_REGION( 0x1000, MC6845_TAG, 0 )
	ROM_LOAD( "abc t02-1.3g", 0x0000, 0x1000, CRC(4d54eed8) SHA1(04cb5fc5f3d7ba9b9a5ae0ec94241d1fe83647f7) ) // 64 90191-01

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "abc p2-1.2g", 0x000, 0x400, NO_DUMP ) // PAL16R4
ROM_END


//-------------------------------------------------
//  ROM( abc806 )
//-------------------------------------------------

ROM_START( abc806 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("ufd20")
	ROM_LOAD( "abc 06-11.1m",  0x0000, 0x1000, CRC(27083191) SHA1(9b45592273a5673e4952c6fe7965fc9398c49827) ) // BASIC PROM ABC 06-11 "64 90231-02"
	ROM_LOAD( "abc 16-11.1l",  0x1000, 0x1000, CRC(eb0a08fd) SHA1(f0b82089c5c8191fbc6a3ee2c78ce730c7dd5145) ) // BASIC PROM ABC 16-11 "64 90232-02"
	ROM_LOAD( "abc 26-11.1k",  0x2000, 0x1000, CRC(97a95c59) SHA1(013bc0a2661f4630c39b340965872bf607c7bd45) ) // BASIC PROM ABC 26-11 "64 90233-02"
	ROM_LOAD( "abc 36-11.1j",  0x3000, 0x1000, CRC(b50e418e) SHA1(991a59ed7796bdcfed310012b2bec50f0b8df01c) ) // BASIC PROM ABC 36-11 "64 90234-02"
	ROM_LOAD( "abc 46-11.2m",  0x4000, 0x1000, CRC(17a87c7d) SHA1(49a7c33623642b49dea3d7397af5a8b9dde8185b) ) // BASIC PROM ABC 46-11 "64 90235-02"
	ROM_LOAD( "abc 56-11.2l",  0x5000, 0x1000, CRC(b4b02358) SHA1(95338efa3b64b2a602a03bffc79f9df297e9534a) ) // BASIC PROM ABC 56-11 "64 90236-02"
	ROM_SYSTEM_BIOS( 0, "ufd19", "UFD-DOS v.19 (1984-03-02)" )
	ROMX_LOAD( "abc 66-21.2k", 0x6000, 0x1000, CRC(c311b57a) SHA1(4bd2a541314e53955a0d53ef2f9822a202daa485), ROM_BIOS(1) ) // DOS PROM ABC 66-21 "64 90314-01"
	ROM_SYSTEM_BIOS( 1, "ufd20", "UFD-DOS v.20 (1984-04-03)" )
	ROMX_LOAD( "abc 66-31.2k", 0x6000, 0x1000, CRC(a2e38260) SHA1(0dad83088222cb076648e23f50fec2fddc968883), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "mica20", "MICA DOS v.20 (1984-04-03)" )
	ROMX_LOAD( "mica2006.2k",  0x6000, 0x1000, CRC(58bc2aa8) SHA1(0604bd2396f7d15fcf3d65888b4b673f554037c0), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "catnet", "CAT-NET" )
	ROMX_LOAD( "cmd8_5.2k",    0x6000, 0x1000, CRC(25430ef7) SHA1(03a36874c23c215a19b0be14ad2f6b3b5fb2c839), ROM_BIOS(4) )
	ROM_LOAD_OPTIONAL( "abc 76-11.2j",  0x7000, 0x1000, CRC(3eb5f6a1) SHA1(02d4e38009c71b84952eb3b8432ad32a98a7fe16) ) // Options-PROM ABC 76-11 "64 90238-02"
	ROM_LOAD( "abc 76-xx.2j",  0x7000, 0x1000, CRC(b364cc49) SHA1(9a2c373778856a31902cdbd2ae3362c200a38e24) ) // Enhanced Options-PROM

	ROM_REGION( 0x1000, MC6845_TAG, 0 )
	ROM_LOAD( "abc t6-11.7c", 0x0000, 0x1000, CRC(b17c51c5) SHA1(e466e80ec989fbd522c89a67d274b8f0bed1ff72) ) // 64 90243-01

	ROM_REGION( 0x200, "rad", 0 )
	ROM_LOAD( "60 90241-01.9b", 0x000, 0x200, CRC(ad549ebb) SHA1(4fe228ce3b84ed6f0ffd5431f2f33b94c3e5268b) ) // "RAD" 7621/7643 (82S131/82S137), character line address

	ROM_REGION( 0x20, "hru", 0 )
	ROM_LOAD( "60 90128-01.6e", 0x00, 0x20, CRC(d970a972) SHA1(c47fdd61fccc68368d42f03a01c7af90ab1fe1ab) ) // "HRU I" 7603 (82S123), HR horizontal timing and video memory access

	ROM_REGION( 0x200, "hru2", 0 )
	ROM_LOAD( "60 90127-01.12g", 0x000, 0x200, CRC(8e9d7cdc) SHA1(4ad16dc0992e31cdb2e644c7be81d334a56f7ad6) ) // "HRU II" 7621 (82S131), ABC800C HR compatibility mode palette

	ROM_REGION( 0x200, "v50", 0 )
	ROM_LOAD( "60 90242-01.7e", 0x000, 0x200, CRC(788a56d8) SHA1(d81e55cdddc36f5d41bf0a33104c75fac590b764) ) // "V50" 7621 (82S131), HR vertical timing 50Hz

	//ROM_REGION( 0x200, "v60", 0 )
	//ROM_LOAD( "60 90319-01.7e", 0x000, 0x200, NO_DUMP ) // "V60" 7621 (82S131), HR vertical timing 60Hz

	ROM_REGION( 0x400, "att_hand", 0 )
	/*
	    1   E6P (RAD A8)
	    2   THP (chargen A12)
	    3   CCLK
	    4   B0 (TX ATT 6)
	    5   B1 (TX ATT 7)
	    6   CONDP (40/80)
	    7   B2 (TX ATT 0)
	    8   B3 (TX ATT 1)
	    9   ULP (RAD A5)
	    10  FLP (RAD A6)
	    11  F2P (RTF)
	    12  GND
	    13  F3P (GTF)
	    14  F4P (BTF)
	    15  B5P (RTB)
	    16  B4 (TX ATT 2)
	    17  B5 (TX ATT 3)
	    18  B6 (TX ATT 4)
	    19  B7 (TX ATT 5)
	    20  LP (*DEN+3)
	    21  B6P (GTB)
	    22  B7P (BTB)
	    23  E5P (RAD A7)
	    24  Vcc
	*/
	ROM_LOAD( "60 90225-01.11c", 0x000, 0x400, NO_DUMP ) // "VIDEO ATTRIBUTE" 40033A (?)

	ROM_REGION( 0x104, "abc_p3", 0 )
	ROM_LOAD( "60 90239-01.1b",  0x000, 0x104, CRC(f3d0ba00) SHA1(bcc0ee26ecac0028aef6bf5cb308133b509bb360) ) // "ABC P3-11" PAL16R4, color encoder

	ROM_REGION( 0x104, "abc_p4", 0 )
	ROM_LOAD( "60 90240-01.2d",  0x000, 0x104, CRC(3cc5518d) SHA1(343cf951d01c9d361b695bb4e80eaadf0820b6bc) ) // "ABC P4-11" PAL16L8, memory mapper
ROM_END



//**************************************************************************
//  DRIVER INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  DRIVER_INIT( abc800c )
//-------------------------------------------------

DIRECT_UPDATE_MEMBER( abc800c_state::direct_update_handler )
{
	if (address >= 0x7c00 && address < 0x8000)
	{
		direct.explicit_configure(0x7c00, 0x7fff, 0x3ff, m_rom->base() + 0x7c00);

		if (!m_fetch_charram)
		{
			m_fetch_charram = 1;
			bankswitch();
		}

		return ~0;
	}

	if (m_fetch_charram)
	{
		m_fetch_charram = 0;
		bankswitch();
	}

	return address;
}

DRIVER_INIT_MEMBER(abc800c_state,driver_init)
{
	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(abc800c_state::direct_update_handler), this));
}


//-------------------------------------------------
//  DRIVER_INIT( abc800m )
//-------------------------------------------------

DIRECT_UPDATE_MEMBER( abc800m_state::direct_update_handler )
{
	if (address >= 0x7800 && address < 0x8000)
	{
		direct.explicit_configure(0x7800, 0x7fff, 0x7ff, m_rom->base() + 0x7800);

		if (!m_fetch_charram)
		{
			m_fetch_charram = 1;
			bankswitch();
		}

		return ~0;
	}

	if (m_fetch_charram)
	{
		m_fetch_charram = 0;
		bankswitch();
	}

	return address;
}

DRIVER_INIT_MEMBER(abc800m_state,driver_init)
{
	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(abc800m_state::direct_update_handler), this));
}


//-------------------------------------------------
//  DRIVER_INIT( abc802 )
//-------------------------------------------------

DIRECT_UPDATE_MEMBER( abc802_state::direct_update_handler )
{
	if (m_lrs)
	{
		if (address >= 0x7800 && address < 0x8000)
		{
			direct.explicit_configure(0x7800, 0x7fff, 0x7ff, m_rom->base() + 0x7800);
			return ~0;
		}
	}

	return address;
}

DRIVER_INIT_MEMBER(abc802_state,driver_init)
{
	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(abc802_state::direct_update_handler), this));
}


//-------------------------------------------------
//  DRIVER_INIT( abc806 )
//-------------------------------------------------

DIRECT_UPDATE_MEMBER( abc806_state::direct_update_handler )
{
	if (address >= 0x7800 && address < 0x8000)
	{
		direct.explicit_configure(0x7800, 0x7fff, 0x7ff, m_rom->base() + 0x7800);

		if (!m_fetch_charram)
		{
			m_fetch_charram = 1;
			bankswitch();
		}

		return ~0;
	}

	if (m_fetch_charram)
	{
		m_fetch_charram = 0;
		bankswitch();
	}

	return address;
}

DRIVER_INIT_MEMBER(abc806_state,driver_init)
{
	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(abc806_state::direct_update_handler), this));
}



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT                         COMPANY             FULLNAME        FLAGS
COMP( 1981, abc800c,    0,          0,      abc800c,    abc800, abc800c_state,  driver_init, "Luxor Datorer AB", "ABC 800 C/HR", MACHINE_SUPPORTS_SAVE )
COMP( 1981, abc800m,    abc800c,    0,      abc800m,    abc800, abc800m_state,  driver_init, "Luxor Datorer AB", "ABC 800 M/HR", MACHINE_SUPPORTS_SAVE )
COMP( 1983, abc802,     0,          0,      abc802,     abc802, abc802_state,   driver_init, "Luxor Datorer AB", "ABC 802",      MACHINE_SUPPORTS_SAVE )
COMP( 1983, abc806,     0,          0,      abc806,     abc806, abc806_state,   driver_init, "Luxor Datorer AB", "ABC 806",      MACHINE_SUPPORTS_SAVE )
