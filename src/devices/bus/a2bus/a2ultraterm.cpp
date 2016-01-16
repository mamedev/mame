// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2ultraterm.c

    Implementation of the Videx UltraTerm 80/132/160-column card

    Notes:

    C0nX: C0n0 is 6845 register address,
          C0n1 is 6845 register data.
          C0n2 is control 1: b7 = 0 to read RAM at cc00, 1 for ROM (writes always to RAM)
                             b6 = 0 for Apple II video, 1 for 6845
                             b5 = 0 for 17.430 MHz 6845 clock, 1 for 28.7595 MHz 6845 clock
                             b4 = 0 for 512 byte RAM block addressing (VideoTerm emulation), 1 for 256-byte RAM page addressing
                             b3-b0 = page select
          C0n3 is control 2: b7 = 0 for attributes software controllable, 1 for DIP switches control attributes
                             b5 = 0 for normal video if bit 7 set, 1 for inverse if bit 7 set
                             b4 = 0 for lowlight if bit 7 set, 1 for highlight if bit 7 set
                             b2 = 0 for high-density character set, 1 for low-density character set
                             b1 = same as b5
                             b0 = same as b4

    C800-CBFF: ROM page 1
    CC00-CFEF: VRAM window or ROM page 2

*********************************************************************/

#include "a2ultraterm.h"
#include "includes/apple2.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_ULTRATERM = &device_creator<a2bus_ultraterm_device>;
const device_type A2BUS_ULTRATERMENH = &device_creator<a2bus_ultratermenh_device>;

#define ULTRATERM_ROM_REGION  "uterm_rom"
#define ULTRATERM_GFX_REGION  "uterm_gfx"
#define ULTRATERM_SCREEN_NAME "uterm_screen"
#define ULTRATERM_MC6845_NAME "mc6845_uterm"

#define CLOCK_LOW   17430000
#define CLOCK_HIGH  28759500

#define CT1_MEMSEL  (0x80)  // 0 for read RAM at cc00, 1 for read ROM
#define CT1_VIDSEL  (0x40)  // 0 for Apple video passthrough, 1 for 6845 video
#define CT1_CLKSEL  (0x20)  // 0 for Videoterm clock, 1 for faster clock
#define CT1_VTEMU   (0x10)  // Videoterm emulation mode if 0
#define CT1_PAGEMASK (0x0f)

#define CT2_USEDIPS (0x80)  // 0 to use the rest of ctrl2's bits, 1 to use DIPs
#define CT2_INVBIT7H (0x20)
#define CT2_HLBIT7H (0x10)
#define CT2_HIDENSITY (0x04)
#define CT2_INVBIT7L (0x02)
#define CT2_HLBIT7L (0x01)


static const rgb_t ultraterm_palette[4] =
{
	rgb_t(0x00,0x00,0x00),
	rgb_t(0x55,0x55,0x55),
	rgb_t(0xaa,0xaa,0xaa),
	rgb_t(0xff,0xff,0xff)
};

MACHINE_CONFIG_FRAGMENT( a2ultraterm )
	MCFG_SCREEN_ADD( ULTRATERM_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(CLOCK_LOW, 882, 0, 720, 370, 0, 350 )
	MCFG_SCREEN_UPDATE_DEVICE( ULTRATERM_MC6845_NAME, mc6845_device, screen_update )

	MCFG_MC6845_ADD(ULTRATERM_MC6845_NAME, MC6845, ULTRATERM_SCREEN_NAME, CLOCK_LOW/9)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(a2bus_videx160_device, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(a2bus_videx160_device, vsync_changed))
MACHINE_CONFIG_END

ROM_START( a2ultraterm )
	ROM_REGION(0x1000, ULTRATERM_ROM_REGION, 0)
	ROM_LOAD( "frm_b537.bin", 0x000000, 0x001000, CRC(1e85a93e) SHA1(b4acd1775c08ae43996ab4edf6d8e28f4736346b) )

	ROM_REGION(0x1000, ULTRATERM_GFX_REGION, 0)
	ROM_LOAD( "chs_7859.bin", 0x000000, 0x001000, CRC(ebe8f333) SHA1(3517fa9e7a39573f1cb159b3161d6939dec199ba) )

	ROM_REGION(0x400, "pal", 0)
	ROM_LOAD( "ult_2a313.jed", 0x000000, 0x000305, CRC(dcd51dea) SHA1(0ad0c5e802e48495da27f7bd26ee3ab1c92d74dd) )
ROM_END

ROM_START( a2ultratermenh )
	ROM_REGION(0x1000, ULTRATERM_ROM_REGION, 0)
	ROM_LOAD( "frm_b5c9.bin", 0x000000, 0x001000, CRC(b71e05e0) SHA1(092e3eda4644d4f465809864a7f023ac7d1d1542) )

	ROM_REGION(0x1000, ULTRATERM_GFX_REGION, 0)
	ROM_LOAD( "chs_5604.bin", 0x000000, 0x001000, CRC(3fb4e90a) SHA1(94ff75199232a9b613585c22f88470f73fb7dd09) )

	ROM_REGION(0x400, "pal", 0)
	ROM_LOAD( "ult_251c.jed", 0x000000, 0x000305, CRC(12fabb0d) SHA1(d4a36837cb98bb65f7ddef7455eb5a7f8e648a82) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_videx160_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a2ultraterm );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_ultraterm_device::device_rom_region() const
{
	return ROM_NAME( a2ultraterm );
}

const rom_entry *a2bus_ultratermenh_device::device_rom_region() const
{
	return ROM_NAME( a2ultratermenh );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_videx160_device::a2bus_videx160_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this), m_rom(nullptr), m_chrrom(nullptr), m_framecnt(0), m_ctrl1(0), m_ctrl2(0),
	m_crtc(*this, ULTRATERM_MC6845_NAME), m_rambank(0)
{
}

a2bus_ultraterm_device::a2bus_ultraterm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_videx160_device(mconfig, A2BUS_ULTRATERM, "Videx UltraTerm (original)", tag, owner, clock, "a2ulttrm", __FILE__)
{
}

a2bus_ultratermenh_device::a2bus_ultratermenh_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	a2bus_videx160_device(mconfig, A2BUS_ULTRATERMENH, "Videx UltraTerm (enhanced //e)", tag, owner, clock, "a2ultrme", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_videx160_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(ULTRATERM_ROM_REGION).c_str())->base();

	m_chrrom = device().machine().root_device().memregion(this->subtag(ULTRATERM_GFX_REGION).c_str())->base();

	memset(m_ram, 0, 256*16);

	save_item(NAME(m_ram));
	save_item(NAME(m_framecnt));
	save_item(NAME(m_rambank));
	save_item(NAME(m_ctrl1));
	save_item(NAME(m_ctrl2));
}

void a2bus_videx160_device::device_reset()
{
	m_rambank = 0;
	m_framecnt = 0;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_videx160_device::read_c0nx(address_space &space, UINT8 offset)
{
//    printf("Read c0n%x (PC=%x)\n", offset, space.device().safe_pc());

	if (!(m_ctrl1 & CT1_VTEMU))
	{
		m_rambank = ((offset>>2) & 3) * 512;
	}

	switch (offset)
	{
		case 1:
			return m_crtc->register_r(space, offset);   // status_r?

		case 2:
			return m_ctrl1;

		case 3:
			return m_ctrl2;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_videx160_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
//    printf("Write %02x to c0n%x (PC=%x)\n", data, offset, space.device().safe_pc());

	switch (offset)
	{
		case 0:
			m_crtc->address_w(space, offset, data);
			break;

		case 1:
			m_crtc->register_w(space, offset, data);
			break;

		case 2:
			m_ctrl1 = data;
//          printf("%02x to ctrl1\n", data);

			// if disabling Videoterm emulation, change RAM banking
			if (data & CT1_VTEMU)
			{
				m_rambank = (data & CT1_PAGEMASK) * 256;
			}
			break;

		case 3:
			m_ctrl2 = data;
//          printf("%02x to ctrl2\n", data);
			break;
	}

	if (!(m_ctrl1 & CT1_VTEMU))
	{
		m_rambank = ((offset >> 2) & 3) * 512;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_videx160_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset+(m_slot * 0x100)];
}

/*-------------------------------------------------
    write_cnxx - called for writes to this card's cnxx space
    the firmware writes here to switch in our $C800 a lot
-------------------------------------------------*/
void a2bus_videx160_device::write_cnxx(address_space &space, UINT8 offset, UINT8 data)
{
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_videx160_device::read_c800(address_space &space, UINT16 offset)
{
	// ROM at c800-cbff
	// bankswitched RAM at cc00-cdff
	if (offset < 0x400)
	{
//        printf("Read VRAM at %x = %02x\n", offset+m_rambank, m_ram[offset + m_rambank]);
		return m_rom[offset + 0x800];
	}
	else
	{
		if (m_ctrl1 & CT1_MEMSEL)   // read ROM?
		{
			return m_rom[offset + 0x800];
		}

		return m_ram[(offset - 0x400) + m_rambank];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_videx160_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
	if (offset >= 0x400)
	{
//        printf("%02x to VRAM at %x\n", data, offset-0x400+m_rambank);
		m_ram[(offset-0x400) + m_rambank] = data;
	}
}

MC6845_UPDATE_ROW( a2bus_videx160_device::crtc_update_row )
{
	UINT32  *p = &bitmap.pix32(y);
	UINT16  chr_base = ra;
	int i;

	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ma + i );
		UINT8 chr = m_ram[ offset ];
		UINT8 data = m_chrrom[ chr_base + (chr * 16) ];
		UINT8 fg = 2;
		UINT8 bg = 0;
		UINT8 tmp;

		// apply attributes
		if (!(m_ctrl2 & CT2_USEDIPS))
		{
			// this set and bit 7 in char, highlight
			if ((m_ctrl2 & CT2_HLBIT7H) && (chr & 0x80))
			{
				fg = 3;
				bg = 0;
			}

			// this set and NOT bit 7 in char, highlight
			if ((m_ctrl2 & CT2_HLBIT7L) && (!(chr & 0x80)))
			{
				fg = 3;
				bg = 0;
			}

			// this clear and bit 7 in char, lowlight
			if (!(m_ctrl2 & CT2_HLBIT7H) && (chr & 0x80))
			{
				fg = 1;
				bg = 0;
			}

			// this clear and NOT bit 7 in char, lowlight
			if (!(m_ctrl2 & CT2_HLBIT7L) && (!(chr & 0x80)))
			{
				fg = 1;
				bg = 0;
			}

			// invert last so invert + hilight/invert + lowlight are possible
			// invert if char bit 7 is set
			if ((m_ctrl2 & CT2_INVBIT7H) && (chr & 0x80))
			{
				tmp = fg;
				fg = bg;
				bg = tmp;
			}

			// invert if char bit 7 is clear
			if ((m_ctrl2 & CT2_INVBIT7L) && (!(chr & 0x80)))
			{
				tmp = fg;
				fg = bg;
				bg = tmp;
			}
		}

		if ( i == cursor_x )
		{
			if ( m_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}

		*p = ultraterm_palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

WRITE_LINE_MEMBER( a2bus_videx160_device::vsync_changed )
{
	if ( state )
	{
		m_framecnt++;
	}
}
