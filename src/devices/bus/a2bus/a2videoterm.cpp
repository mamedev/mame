// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2videoterm.c

    Implementation of the Videx VideoTerm 80-column card

    Notes (from Videoterm user's manual, which contains
           schematics and firmware source listings).

    C0nX: C0n0 is 6845 register address,
          C0n1 is 6845 register data.

          Bits 2 & 3 on any access to C0nX set the VRAM page at CC00.

    C800-CBFF: ROM
    CC00-CDFF: VRAM window

    TODO:
    Cursor is probably not completely right.
    Add font ROM select.

*********************************************************************/

#include "a2videoterm.h"
#include "includes/apple2.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_VIDEOTERM = &device_creator<a2bus_videoterm_device>;
const device_type A2BUS_IBSAP16 = &device_creator<a2bus_ap16_device>;
const device_type A2BUS_IBSAP16ALT = &device_creator<a2bus_ap16alt_device>;
const device_type A2BUS_VTC1 = &device_creator<a2bus_vtc1_device>;
const device_type A2BUS_VTC2 = &device_creator<a2bus_vtc2_device>;
const device_type A2BUS_AEVIEWMASTER80 = &device_creator<a2bus_aevm80_device>;

#define VIDEOTERM_ROM_REGION  "vterm_rom"
#define VIDEOTERM_GFX_REGION  "vterm_gfx"
#define VIDEOTERM_SCREEN_NAME "vterm_screen"
#define VIDEOTERM_MC6845_NAME "mc6845_vterm"

#define MDA_CLOCK   16257000

MACHINE_CONFIG_FRAGMENT( a2videoterm )
	MCFG_SCREEN_ADD( VIDEOTERM_SCREEN_NAME, RASTER) // 560x216?  (80x24 7x9 characters)
	MCFG_SCREEN_RAW_PARAMS(MDA_CLOCK, 882, 0, 720, 370, 0, 350 )
	MCFG_SCREEN_UPDATE_DEVICE( VIDEOTERM_MC6845_NAME, mc6845_device, screen_update )

	MCFG_MC6845_ADD(VIDEOTERM_MC6845_NAME, MC6845, VIDEOTERM_SCREEN_NAME, MDA_CLOCK/9)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(a2bus_videx80_device, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(a2bus_videx80_device, vsync_changed))
MACHINE_CONFIG_END

ROM_START( a2videoterm )
	ROM_REGION(0x400, VIDEOTERM_ROM_REGION, 0)
	ROM_LOAD( "videx videoterm rom 2.4.bin", 0x000000, 0x000400, CRC(bbe3bb28) SHA1(bb653836e84850ce3197f461d4e19355f738cfbf) )

	ROM_REGION(0x5000, VIDEOTERM_GFX_REGION, 0)
	ROM_LOAD( "videx videoterm character rom normal.bin", 0x000000, 0x000800, CRC(87f89f08) SHA1(410b54f33d13c82e3857f1be906d93a8c5b8d321) )
	ROM_LOAD( "videx videoterm character rom normal uppercase.bin", 0x000800, 0x000800, CRC(3d94a7a4) SHA1(5518254f24bc945aab13bc71ecc9526d6dd8e033) )
	ROM_LOAD( "videx videoterm character rom apl.bin", 0x001000, 0x000800, CRC(1adb704e) SHA1(a95df910eca33188cacee333b1325aa47edbcc25) )
	ROM_LOAD( "videx videoterm character rom epson.bin", 0x001800, 0x000800, CRC(0c6ef8d0) SHA1(db72c0c120086f1aa4a87120c5d7993c4a9d3a18) )
	ROM_LOAD( "videx videoterm character rom french.bin", 0x002000, 0x000800, CRC(266aa837) SHA1(2c6b4e9d342dbb2de8e278740f11925a9d8c6616) )
	ROM_LOAD( "videx videoterm character rom german.bin", 0x002800, 0x000800, CRC(df7324fa) SHA1(0ce58d2ffadbebc8db9f85bbb9a08a4f142af682) )
	ROM_LOAD( "videx videoterm character rom katakana.bin", 0x003000, 0x000800, CRC(b728690e) SHA1(e018fa66b0ff560313bb35757c9ce7adecae0c3a) )
	ROM_LOAD( "videx videoterm character rom spanish.bin", 0x003800, 0x000800, CRC(439eac08) SHA1(d6f9f8eb7702440d9ae39129ea4f480b80fc4608) )
	ROM_LOAD( "videx videoterm character rom super and subscript.bin", 0x004000, 0x000800, CRC(08b7c538) SHA1(7f4029d97be05680fe695debe07cea07666419e0) )
	ROM_LOAD( "videx videoterm character rom symbol.bin", 0x004800, 0x000800, CRC(82bce582) SHA1(29dfa8c5257dbf25651c6bffa9cdb453482aa70e) )
ROM_END

ROM_START( a2ap16 )
	ROM_REGION(0x2000, VIDEOTERM_ROM_REGION, 0)
	ROM_LOAD( "space 84 video ap16.bin", 0x000000, 0x002000, CRC(0e188da2) SHA1(9a29250b6cc7b576fdc67769944de35e6f54b9d5) )

	ROM_REGION(0x4000, VIDEOTERM_GFX_REGION, 0)
	ROM_LOAD( "space 84 video chargen ap16.bin", 0x000000, 0x002000, CRC(b9447088) SHA1(19c95f91a67b948fc00a14621d574d629479d451) )
ROM_END

ROM_START( a2ap16alt )
	ROM_REGION(0x1000, VIDEOTERM_ROM_REGION, 0)
	ROM_LOAD( "unknown apple ii clone video 3.bin", 0x000000, 0x001000, CRC(af1226d2) SHA1(18a569f417a47f54a17bd9046d306a54b46ed049) )

	ROM_REGION(0x4000, VIDEOTERM_GFX_REGION, 0)
	ROM_LOAD( "unknown apple ii clone video 1.bin", 0x000000, 0x001000, CRC(cf84811c) SHA1(135f4f35607dd74941f0a3cae813227bf8a8a020) )
ROM_END

ROM_START( vtc1 )
	ROM_REGION(0x800, VIDEOTERM_ROM_REGION, 0)
	ROM_LOAD( "10.ic10.bin",  0x000000, 0x000800, CRC(ddbbc2fb) SHA1(15d6142b177b47c016f2745e1d95767b440d77c7) )

	ROM_REGION(0x1000, VIDEOTERM_GFX_REGION, 0)
	ROM_LOAD( "9.ic9.bin",    0x000000, 0x000800, CRC(094670f1) SHA1(aefae76fb07740d042cf294f01424efa7cc7a199) )
	ROM_LOAD( "8.ic8.bin",    0x000800, 0x000800, CRC(fbd98d77) SHA1(0d9b1c3917e23ca35d5fbd405f05ff6e87122b92) )
ROM_END

ROM_START( vtc2 )
	ROM_REGION(0x800, VIDEOTERM_ROM_REGION, 0)
	ROM_LOAD( "6.ic6.bin",    0x000000, 0x000800, CRC(5776fa24) SHA1(19f69011ed7d2551c39d5c1cac1f5a2defc8f8fb) )

	ROM_REGION(0x1000, VIDEOTERM_GFX_REGION, 0)
	ROM_LOAD( "5.ic5.bin",    0x000000, 0x000800, CRC(aafa7085) SHA1(54d7c358f1927ba8f3b61145215a806d8cb6b673) )
	ROM_LOAD( "4.ic4.bin",    0x000800, 0x000800, CRC(8a497a48) SHA1(50c3df528109c65491a001ec74e50351a652c1fd) )
ROM_END

ROM_START( a2aevm80 )
	ROM_REGION(0x800, VIDEOTERM_ROM_REGION, 0)
	ROM_LOAD( "ae viewmaster 80 rom.bin", 0x000000, 0x000800, CRC(62a4b111) SHA1(159bf7c4add1435be215fddb648c0743fbcc49b5) )

	ROM_REGION(0x1000, VIDEOTERM_GFX_REGION, 0)
	ROM_LOAD( "ae viewmaster 80 video rom.bin", 0x000000, 0x000800, CRC(4801ab90) SHA1(f90658ffee7740f3cb30ecef2e151f7dc6098833) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_videx80_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2videoterm );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_videoterm_device::device_rom_region() const
{
	return ROM_NAME( a2videoterm );
}

const rom_entry *a2bus_ap16_device::device_rom_region() const
{
	return ROM_NAME( a2ap16 );
}

const rom_entry *a2bus_ap16alt_device::device_rom_region() const
{
	return ROM_NAME( a2ap16alt );
}

const rom_entry *a2bus_vtc1_device::device_rom_region() const
{
	return ROM_NAME( vtc1 );
}

const rom_entry *a2bus_vtc2_device::device_rom_region() const
{
	return ROM_NAME( vtc2 );
}

const rom_entry *a2bus_aevm80_device::device_rom_region() const
{
	return ROM_NAME( a2aevm80 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_videx80_device::a2bus_videx80_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this), m_rom(nullptr), m_chrrom(nullptr), m_framecnt(0),
	m_crtc(*this, VIDEOTERM_MC6845_NAME), m_rambank(0),
	m_palette(*this, ":palette")
{
}

a2bus_videoterm_device::a2bus_videoterm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_videx80_device(mconfig, A2BUS_VIDEOTERM, "Videx VideoTerm", tag, owner, clock, "a2vidtrm", __FILE__)
{
}

a2bus_ap16_device::a2bus_ap16_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_videx80_device(mconfig, A2BUS_IBSAP16, "IBS AP-16 80 column card", tag, owner, clock, "a2ap16", __FILE__)
{
}

a2bus_ap16alt_device::a2bus_ap16alt_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_videx80_device(mconfig, A2BUS_IBSAP16ALT, "IBS AP-16 80 column card (alt. version)", tag, owner, clock, "a2ap16a", __FILE__)
{
}

a2bus_vtc1_device::a2bus_vtc1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_videx80_device(mconfig, A2BUS_VTC1, "Unknown VideoTerm clone #1", tag, owner, clock, "a2vtc1", __FILE__)
{
}

a2bus_vtc2_device::a2bus_vtc2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_videx80_device(mconfig, A2BUS_VTC2, "Unknown VideoTerm clone #2", tag, owner, clock, "a2vtc2", __FILE__)
{
}

a2bus_aevm80_device::a2bus_aevm80_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_videx80_device(mconfig, A2BUS_AEVIEWMASTER80, "Applied Engineering Viewmaster 80", tag, owner, clock, "a2aevm80", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_videx80_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(VIDEOTERM_ROM_REGION).c_str())->base();

	m_chrrom = device().machine().root_device().memregion(this->subtag(VIDEOTERM_GFX_REGION).c_str())->base();

	memset(m_ram, 0, 4*512);

	save_item(NAME(m_ram));
	save_item(NAME(m_framecnt));
	save_item(NAME(m_rambank));
}

void a2bus_videx80_device::device_reset()
{
	m_rambank = 0;
	m_framecnt = 0;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_videx80_device::read_c0nx(address_space &space, UINT8 offset)
{
//    printf("Read c0n%x (PC=%x)\n", offset, space.device().safe_pc());

	m_rambank = ((offset>>2) & 3) * 512;

	if (offset == 1)
	{
		return m_crtc->register_r(space, offset);   // status_r?
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_videx80_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
//    printf("Write %02x to c0n%x (PC=%x)\n", data, offset, space.device().safe_pc());

	if (offset == 0)
	{
		m_crtc->address_w(space, offset, data);
	}
	else if (offset == 1)
	{
		m_crtc->register_w(space, offset, data);
	}

	m_rambank = ((offset>>2) & 3) * 512;
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_videx80_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset+0x300];
}

UINT8 a2bus_ap16_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset+0x1f00];
}

UINT8 a2bus_ap16alt_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset+0xb00];
}

/*-------------------------------------------------
    write_cnxx - called for writes to this card's cnxx space
    the firmware writes here to switch in our $C800 a lot
-------------------------------------------------*/
void a2bus_videx80_device::write_cnxx(address_space &space, UINT8 offset, UINT8 data)
{
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_videx80_device::read_c800(address_space &space, UINT16 offset)
{
	// ROM at c800-cbff
	// bankswitched RAM at cc00-cdff
	if (offset < 0x400)
	{
//        printf("Read VRAM at %x = %02x\n", offset+m_rambank, m_ram[offset + m_rambank]);
		return m_rom[offset];
	}
	else
	{
		return m_ram[(offset-0x400) + m_rambank];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_videx80_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
	if (offset >= 0x400)
	{
//        printf("%02x to VRAM at %x\n", data, offset-0x400+m_rambank);
		m_ram[(offset-0x400) + m_rambank] = data;
	}
}

MC6845_UPDATE_ROW( a2bus_videx80_device::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(y);
	UINT16  chr_base = ra; //( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ma + i ) & 0x7ff;
		UINT8 chr = m_ram[ offset ];
		UINT8 data = m_chrrom[ chr_base + chr * 16 ];
		UINT8 fg = 15;
		UINT8 bg = 0;

		if ( i == cursor_x )
		{
			if ( m_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

WRITE_LINE_MEMBER( a2bus_videx80_device::vsync_changed )
{
	if ( state )
	{
		m_framecnt++;
	}
}
