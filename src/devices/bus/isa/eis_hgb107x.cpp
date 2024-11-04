// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
/*****************************************************************************

  Ericsson PC Monochrome HR Graphics Board 1070

******************************************************************************/

/* PCB layouts and assembly years from online pictures and physical unit.
 Ericsson   -  marked SPVT02 8301 60 53-10, assembled in 1985 indicated by chip dates
 +--------------------------------------------------------------------------------------+ ___
 |  IC1  IC2   IC3   IC4   IC5 +-IC15--EPROM-+   IC6      IC7      IC8    S1        ||
 |                                 |8363 65 14-80|                                      ||
 | IC9  IC10 IC11  IC12  IC13  IC14|CG 50821 A64 |+------------------++-IC24 EPROM--+   ||
 |                                 +-------------+| CRTC HD46505SP-1 ||10-40VP      |   ||
 | IC16 IC17 IC18  IC19  IC20  IC21     IC22      | IC23 HD68A45SP   ||402 28 A19   | J4|| not
 |                                                +------------------++-------------+   || mounted
 | IC25 IC26 IC27  IC28  IC29  IC30       IC31       IC32      IC33      IC34           ||
 |                                                                                     O-|__
 | IC35 IC36 IC37  IC38  IC39  IC40       IC41       IC42      IC43      IC44           ||  |
 |                                                                                      ||DB15
 | IC45 IC46 IC47  IC48  IC49  IC50       IC51       IC52      IC53      IC54           ||  |
 |                                                                                      ||__|
 | IC55 IC56 IC57  IC58  IC59  IC60       IC61       IC62      IC63      IC64          O-|
 |                                                                               J1A    ||
 | IC65 IC66 IC67 IC68 IC69 IC70 IC71 IC72 +--------------------------------------------+|
 +-----------------------------------------+    |||||||||  |||||||||||||||||||||||||     |
   I85565  A85571 (labels)                                                               |
                                                                                         |

 IC's (from photos)
 ------------------------------------------------------------------------------
 IC1  74F109                              IC26 74F86                                IC51 TMS4416-15NL 4 x 16Kbits DRAM
 IC2  74LS393                             IC27 74LS08                               IC52 74ALS574
 IC3  74F64                               IC28 74F153                               IC53 74LS138
 IC4  74ALS299                            IC29 74LS174                              IC54 74F86
 IC5  74LS375                             IC30 74LS374                              IC55 74F109
 IC6  74LS151                             IC31 74LS374                              IC56 74F32
 IC7  74LS153                             IC32 74ALS574                             IC57 74F109
 IC8  74LS389?                            IC33 74LS08                               IC58 74F00?
 IC9  74F02                               IC34 74LS245                              IC59 74LS244
 IC10 74ALS109                            IC35 74F10?                               IC60 TMS4416-15NL 4 x 16Kbits DRAM
 IC11 Crystal 17.040MHz                   IC36 74LS02                               IC61 TMS4416-15NL 4 x 16Kbits DRAM
 IC12 74F64                               IC37 74LS00                               IC62 74ALS574
 IC13 74ALS299                            IC38 74F374                               IC63 74LS138
 IC14 PAL? 10-70ART40101                  IC39 74LS125                              IC64 74LS245
 IC15 EPROM 8363 65 14-80 CG 50821 A64    IC40 74LS244                              IC65 74LS00
 IC16 Crystal 19.170MHz                   IC41 74LS244                              IC66 74LS02
 IC17 74LS10                              IC42 74LS574                              IC67 74LS51
 IC18 74F08                               IC43 74LS32                               IC68 74LS04
 IC19 74ALS574                            IC44 MC10124 - TTL to MECL converter      IC69 74LS153
 IC20 74LS299                             IC45 74LS109                              IC70 74LS109
 IC21 74LS273                             IC46 74LS00                               IC71 74LS138
 IC22 74ALS574                            IC47 74F194                               IC72 74LS139
 IC23 CRTC HD46505SP,HD68A45SP            IC48 74F04
 IC24 EPROM 2764, 10-40 VP 402 28 A19     IC49 74LS174
 IC25 74ALS109                            IC50 TMS4416-15NL 4 x 16Kbits DRAM

 General description
 -------------------
 The PCB has a 2 bit DIP switch S1 and a DB15 non standard video connector. There is also an unsoldered J4 connector
 above the DB15 but no hole prepared for a connector in the plate. Above the J4 connector there is a two pin PCB connector
 that probably receives the power for the monitor for the DB15 from the PSU.

 Just below IC65 and IC66 there are two labels saying "I 85565" and "A E85571" respectively

 Video cable, card DB15 <---> monitor DB25
 ---------------------------------------------------
  Ericsson       2  +VS             4  Ericsson
  Monochrome     3  VS return       2  Monochrome HR
  HR Graphics   10  +VS            17  Monitors 3111 (Amber) or
  Board 1070    11  VS return      15  3712/3715 (Black & White)
         4  VSYNC           6
        12  VSYNC          19
         5  HSYNC           7
        13  HSYNC          20
         6  High intensity  8
        14  High intensity 21
         7  Video           9
        15  Video          22
                 8  GND            11

  This board is normaly used with an Ericsson monitor due to the non standard connector.
  Trivia: https://www.pinterest.se/pin/203084264425177097/
 */

#include "emu.h"
#include "eis_hgb107x.h"

#include "screen.h"

#define LOG_READ    (1U << 1)
#define LOG_SETUP   (1U << 2)
#define LOG_ROW     (1U << 3)
#define LOG_MODE    (1U << 4)
#define LOG_CHRG    (1U << 5)
#define LOG_STAT    (1U << 6)

//#define VERBOSE (LOG_MODE|LOG_SETUP|LOG_ROW)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGROW(...)   LOGMASKED(LOG_ROW,   __VA_ARGS__)
#define LOGMODE(...)  LOGMASKED(LOG_MODE,  __VA_ARGS__)
#define LOGCHRG(...)  LOGMASKED(LOG_CHRG,  __VA_ARGS__)
#define LOGSTAT(...)  LOGMASKED(LOG_STAT,  __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define MC6845_NAME "mc6845"

enum
{
	MDA_TEXT_INTEN = 0,
	MDA_TEXT_BLINK,
	MDA_LOWRES_TEXT_INTEN,
	MDA_LOWRES_TEXT_BLINK
};

#define EPC_MDA_SCREEN "epc_mda_screen" // TODO: use a device finder reference instead

ROM_START( epc )
	ROM_REGION(0x2000,"chargen", 0)
	ROM_LOAD("8363_65_14_80_cg_50821_a64.bin",  0x00000, 0x2000, CRC(be709786) SHA1(38ab26224bbe66bbe2bb2ccac29b41cbf78bdbf8))
	//ROM_LOAD("10_40_vp_402_28_ic_24_a19.bin",  0x00000, 0x2000, CRC(2aa53b92) SHA1(87051a037249eb631d7d2191bc0e925125c60f39))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
DEFINE_DEVICE_TYPE(ISA8_EPC_MDA, isa8_epc_mda_device, "isa_epc_mda", "Ericsson PC Monochrome HR Graphics Board 1070")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
/* There are two crystals on the board: 19.170Mhz and 17.040MHz  TODO: verify use */
/* Text modes uses 720x400 base resolution and the Graphics modes 320/640x200/400 */
/* This matches the difference between the crystals so we assume this for now     */
void isa8_epc_mda_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, EPC_MDA_SCREEN, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(19'170'000) / 4, 720, 0, 720, 400, 0, 400);
	screen.set_screen_update(MC6845_NAME, FUNC(mc6845_device::screen_update));

	HD6845S(config, m_crtc, XTAL(19'170'000) / 16); // clock and divider are guesswork
	m_crtc->set_screen(EPC_MDA_SCREEN);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	m_crtc->set_update_row_callback(FUNC(isa8_epc_mda_device::crtc_update_row));
	m_crtc->out_hsync_callback().set(FUNC(isa8_epc_mda_device::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(isa8_epc_mda_device::vsync_changed));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------
const tiny_rom_entry *isa8_epc_mda_device::device_rom_region() const
{
	return ROM_NAME( epc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_epc_mda_device - constructor
//-------------------------------------------------
isa8_epc_mda_device::isa8_epc_mda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_epc_mda_device(mconfig, ISA8_EPC_MDA, tag, owner, clock)
{
}

isa8_epc_mda_device::isa8_epc_mda_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_crtc(*this, MC6845_NAME),
	m_soft_chr_gen(nullptr),
	m_s1(*this, "S1"),
	m_color_mode(0),
	m_mode_control2(0),
	m_screen(*this, EPC_MDA_SCREEN),
	m_io_monitor(*this, "MONITOR"),
	m_chargen(*this, "chargen"),
	m_installed(false),
	m_framecnt(0),
	m_mode_control(0),
	m_update_row_type(-1),
	m_chr_gen(nullptr),
	m_vsync(0),
	m_hsync(0),
	m_pixel(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_epc_mda_device::device_start()
{
	/* Palette for use with the Ericsson Amber Monochrome HR CRT monitor 3111, P3 phospor 602nm 255,183, 0 */
	m_3111_pal[0] = rgb_t(  0,   0,     0); // black
	m_3111_pal[1] = rgb_t(  143, 103,   0); // dim
	m_3111_pal[2] = rgb_t(  191, 137,   0); // normal
	m_3111_pal[3] = rgb_t(  255, 183,   0); // bright

	/* Palette for use with the Ericsson B&W Monochrome HR CRT monitor 3712/3715 */
	m_371x_pal[0] = rgb_t(    0,   0,   0); // black
	m_371x_pal[1] = rgb_t(  143, 143, 143); // dim
	m_371x_pal[2] = rgb_t(  191, 191, 191); // normal
	m_371x_pal[3] = rgb_t(  255, 255, 255); // bright

	/* Init a default palette */
	m_pal = &m_3111_pal; // In case screen starts rendering before device_reset where we read the settings
	m_videoram.resize(0x8000);
	set_isa_device();
	m_installed = false;
	m_hd6845s = subdevice<hd6845s_device>(MC6845_NAME);
}

void isa8_epc_mda_device::device_reset()
{
	m_framecnt = 0;
	m_mode_control = 0;
	m_vsync = 0;
	m_hsync = 0;
	m_pixel = 0;

	m_color_mode = m_s1->read();
	LOGSETUP("%s: m_color_mode:%02x\n", FUNCNAME, m_color_mode);
	m_pal = (m_io_monitor-> read() & 1) == 1 ? &m_371x_pal : &m_3111_pal;
	m_vmode = 0;

	if (m_installed == false)
	{
		m_isa->install_device(0x3b0, 0x3bf, read8sm_delegate(*this, FUNC(isa8_epc_mda_device::io_read)), write8sm_delegate(*this, FUNC(isa8_epc_mda_device::io_write)));
		m_isa->install_bank(0xb0000, 0xb7fff, &m_videoram[0]); // Monochrome emulation mode VRAM address

		// This check allows a color monitor adapter to be installed at this address range if color emulation is disabled
		if (m_color_mode & 1)
		{
			m_isa->install_device(0x3d0, 0x3df, read8sm_delegate(*this, FUNC(isa8_epc_mda_device::io_read)), write8sm_delegate(*this, FUNC(isa8_epc_mda_device::io_write)));
			m_isa->install_bank(0xb8000, 0xbffff, &m_videoram[0]); // Color emulation mode VRAM address, but same 32KB areas as there are only this amount on the board
		}
		m_installed = true;
	}
}

/*
 * Register Address table from the manual
 * Ericsson name          MDA mode   CGA mode  Standard name
 *-------------------------------------------------------------------------------
 * 6845 Address Registers 0x3b4      0x3d4     wo CRT Index reg
 * 6845 Data Registers    0x3b5      0x3d5     wo CRT Data reg
 * Mode Register 1        0x3b8      0x3d8     rw MDA/CGA mode reg (bit 0,1 & 4 incompatible)
 * Mode Register 2        0x3bf      0x3df     rw CRT/CPU page reg (incompatible w PCjr only)
 * Status Register        0x3ba      0x3da     r  CGA/MDA status reg (incompatible)
 *                                              w EGA/VGA feature ccontrol reg (not used by this board)
 */
void isa8_epc_mda_device::io_write(offs_t offset, uint8_t data)
{
	LOG("%s: %04x <- %02x\n", FUNCNAME, offset, data);
	switch( offset )
	{
		case 0x04:
			//LOGSETUP(" - HD6845S address write\n");
			m_hd6845s->address_w( data );
			break;
		case 0x05:
			//LOGSETUP(" - HD6845S register write\n");
			m_hd6845s->register_w( data );
			break;
		case 0x08: // Mode 1 reg
			LOGMODE(" - Mode register 1 write: %02x\n", data);
			LOGMODE("   MSB attribute: %s\n", (data & 0x20) == 0 ? "intensity" : "blink");
			LOGMODE("   Horizontal px: %s\n", (data & 0x10) == 0 ? "320/LR" : "640/HR");
			LOGMODE("   Video        : %s\n", (data & 0x08) == 0 ? "Disabled" : "Enabled");
			LOGMODE("   Mode         : %s\n", (data & 0x02) == 0 ? "Text" : "Graphics");
			LOGMODE("   Text columns : %d\n", (data & 0x01) == 0 ? 40 : 80);
			m_mode_control = data;
			m_vmode &= ~(VM_GRAPH | VM_COLS80 | VM_HOR640);
			m_vmode |= ((m_mode_control & 0x01) ? VM_COLS80 : 0);
			m_vmode |= ((m_mode_control & 0x02) ? VM_GRAPH  : 0);
			m_vmode |= ((m_mode_control & 0x10) ? VM_HOR640 : 0);
			m_update_row_type = ((data & 0x20) == 0 ? MDA_LOWRES_TEXT_INTEN : MDA_LOWRES_TEXT_BLINK);
			{
				rectangle rect(0, get_xres() - 1, 0, get_yres() -1);
				m_screen->configure(get_xres(), get_yres(), rect, HZ_TO_ATTOSECONDS(50));
			}
			LOGMODE("Video Mode:%02x\n\n", m_vmode);
			break;
		case 0x0f: // Mode 2 reg
			LOGMODE(" - Mode register 2 write: %02x\n", data);
			LOGMODE("   Vertical px  : %s\n", (data & MR2_VER400) == 0 ? "200" : "400");
			LOGMODE("   Character set: %s\n", (data & MR2_CHRSET) == 0 ? "0" : "1");
			LOGMODE("   Emulated     : %s\n", (data & MR2_COLEMU) == 0 ? "Color" : "Monochrome");
			m_mode_control2 = data;
			m_vmode &= ~(VM_MONO | VM_VER400);
			m_vmode |= ((m_mode_control2 & 0x04) ? VM_MONO   : 0);
			m_vmode |= ((m_mode_control2 & 0x80) ? VM_VER400 : 0);
			{
				rectangle rect(0, get_xres() - 1, 0, get_yres() -1);
				m_screen->configure(get_xres(), get_yres(), rect, HZ_TO_ATTOSECONDS(50));
			}
			LOGMODE("Video Mode:%02x\n\n", m_vmode);
			break;
		default:
			LOG("EPC MDA: io_write at wrong offset:%02x\n", offset);
	}
}

uint8_t isa8_epc_mda_device::io_read(offs_t offset)
{
	LOG("%s: %04x <- ???\n", FUNCNAME, offset);
	int data = 0xff;
	switch( offset )
	{
		case 0x04:
			LOGR(" - hd6845s address read\n");
			break;
		case 0x05:
			LOGR(" - hd6845s register read\n");
			data = m_hd6845s->register_r();
			break;
		case 0x08: // Mode 1 reg
			data = m_mode_control;
			LOGMODE(" - Mode register 1 read: %02x\n", data);
			break;
		case 0x0a: // Status reg: b7-6=00 board ID; b3 vert retrace; b0 horiz retrace; b5,4,2,1 unused
			data = (m_vsync != 0 ? 0x08 : 0x00) | (m_hsync != 0 ? 0x01 : 0x00);
			LOGSTAT(" - Status register read: %02x\n", data);
			break;
		case 0x0f: // Mode 2 reg
			data = m_mode_control2;
			LOGMODE(" - Mode register 2 read: %02x\n", data);
			break;
		default:
			LOG("EPC MDA: io_read at wrong offset:%02x\n", offset);
			logerror("EPC MDA: io_read at wrong offset:%02x\n", offset);
	}
	LOG(" !!!: %04x <- %02x\n", offset, data);
	return data;
}

void isa8_epc_mda_device::hsync_changed(int state)
{
	m_hsync = state ? 1 : 0;
}


void isa8_epc_mda_device::vsync_changed(int state)
{
	m_vsync = state ? 0x80 : 0;
	if ( state )
	{
		m_framecnt++;
	}
}


/*
 *  rW  MDA mode control register (see #P138)
 */
void isa8_epc_mda_device::mode_control_w(uint8_t data)
{
	m_mode_control = data;

	switch( m_mode_control & 0x2a )
	{
	case 0x08:
		m_update_row_type = MDA_TEXT_INTEN;
		break;
	case 0x28:
		m_update_row_type = MDA_TEXT_BLINK;
		break;
	default:
		m_update_row_type = -1;
	}
}


/*  R-  CRT status register (see #P139)
 *      (EGA/VGA) input status 1 register
 *      7    HGC vertical sync in progress
 *      6-4  adapter 000  hercules
 *                   001  hercules+
 *                   101  hercules InColor
 *                   else unknown
 *      3    pixel stream (0 black, 1 white)
 *      2-1  reserved
 *      0    horizontal drive enable
 */
uint8_t isa8_epc_mda_device::status_r()
{
	// Faking pixel stream here
	if (!machine().side_effects_disabled())
		m_pixel++;

	return 0xF0 | (m_pixel & 0x08) | m_hsync;
}

inline int isa8_epc_mda_device::get_xres()
{
	return (m_vmode & VM_GRAPH) ? ( (m_vmode & VM_HOR640) ? 640 : 320 ) : 720;
}

inline int isa8_epc_mda_device::get_yres()
{
	return (m_vmode & VM_GRAPH) ? ( (m_vmode & VM_VER400) ? 400 : 200 ) : 400;
}

MC6845_UPDATE_ROW(isa8_epc_mda_device::crtc_update_row)
{
	// Get som debug data from a couple of rows now and then
	if ( y < (16 * 0 + 0x20) && (m_framecnt & 0xff) == 0 )
	{
		LOGROW("%11.6f %s\n - y:%d chr_base:%d ra:%d ma:%d x_count:%d\n", machine().time().as_double(), FUNCNAME,
			   y, y % 16, ra, ma, x_count);
	}

	// Video Off handling
	if ((m_mode_control & MR1_VIDEO) == 0)
	{
		for (int i = 0; i < get_xres(); i++)
		{
			bitmap.pix(y, i) = rgb_t::black();
		}
	}

	// Graphic modes using only pixeldata, soft fonts are 8x8 or 8x16 but this is transparant to the code
	else if ((m_vmode & VM_GRAPH) != 0)
	{
		logerror("EPC MDA: graphic modes not supported yet\n");
	}

	// Text modes using one of two 9x16 fonts in character rom
	else
	{
		uint32_t  *p = &bitmap.pix(y);
		uint16_t  chr_base = ra;

		// Adjust row pointer if in monochrome text mode as we insert two scanlines per row of characters (see below)
		if (m_vmode & VM_MONO)
		{
			p = &bitmap.pix((y / 14) * 16 + y % 14);
		}

		// Loop over each character in a row
		for ( int i = 0; i < x_count; i++ )
		{
			uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
			uint8_t chr = m_videoram[ offset ];
			uint8_t attr = m_videoram[ offset + 1 ];
			uint8_t data = m_chargen[ ((m_mode_control2 & MR2_CHRSET) ? 0x1000 : 0) + chr_base + chr * 16];

			// Default to light text on dark background
			uint8_t fg = 2;
			uint8_t bg = 0;

			if (y == 0 && i == 0) LOGCHRG(" - Offset: %04x Chr: '%c'[%02x] Attr: %02x Chr_base: %04x\n", offset, chr, chr, attr, chr_base);

			// Prepare some special monochrome emulation cases
			if ( m_vmode & VM_MONO)
			{
				// Handle invisible characters
				if ( (attr & (ATTR_FOREG | ATTR_BACKG)) == 0 )
				{
					data = 0x00;
				}
				// Handle reversed characters
				else if ( (attr & (ATTR_BACKG)) == ATTR_BACKG )
				{
					fg = 0;
					bg = 2;
				}
			}
			else // prepare some special color emulation cases
			{
				// Handle invisible characters
				if ( (attr & (ATTR_FOREG)) == ((attr & ATTR_BACKG) >> 4))
				{
					data = 0x00;
				}
				// Handle reversed characters
				else if ( (attr & ATTR_BACKG) == ATTR_BACKG ||
					  (attr & ATTR_FOREG) == 0 )
				{
					fg = 0;
					bg = 2;
				}
			}

			// Handle intense foreground
			if ((attr & ATTR_INTEN) != 0 && fg == 2)
			{
				fg = 3;
			}

			// Handle intense background if blinking is disabled
			if ((m_mode_control & MR1_BLINK) == 0 &&
				(attr & ATTR_BLINK) != 0 && bg == 2)
			{
				bg = 3;
			}

			// Handle cursor and blinks
			if ( i == (cursor_x))
			{
				if ( m_framecnt & 0x08 )
				{
					data = 0xFF;
				}
			}
			else
			{
				if ( (m_mode_control & MR1_BLINK) &&
					 ( attr & ATTR_BLINK ) && ( m_framecnt & 0x10 ) )
				{
					data = 0x00;
				}
			}

			*p = (*m_pal)[( data & 0x80 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x40 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x20 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x10 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x08 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x04 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x02 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x01 ) ? fg : bg]; p++;
			if (chr >= 0xc0 && chr <= 0xdf)
				*p = (*m_pal)[( data & 0x01 ) ? fg : bg]; // 9th pixel col is a copy of col 8
			else
				*p = (*m_pal)[bg];                        // 9th pixel col is just background
			p++;

			// Insert two extra scanlines in monochrome text mode to get 400 lines and support underline, needs verification on actual hardware.
			// The technical manual says that the character box is 9x16 pixels in 80x25 character mode which equals 720x400 resolution but the
			// CRTC calls back for only 350 lines. Assumption is that there is hardware adding these lines and that handles underlining. In color
			// emulation text mode all 400 lines are called for in 80x25 and this mode does not support underlining according to the technical manual
			if ( ra == 13 && (m_vmode & VM_MONO) )
			{
				uint16_t row = ra + (y / 14) * 16; // Calculate correct row number including the extra 2 lines per each row of characters
				for ( int j = 0; j < 9; j++)
				{
					if (chr >= 0xb3 && chr <= 0xdf) // Handle the meta graphics characters
					{
						bitmap.pix(row + 1, j + i * 9) = (*m_pal)[( data & (0x80 >> j) ) || (j == 8 && (data & 0x01)) ? fg : bg];
						bitmap.pix(row + 2, j + i * 9) = (*m_pal)[( data & (0x80 >> j) ) || (j == 8 && (data & 0x01)) ? fg : bg];
					}
					else
					{
						// Handle underline
						bitmap.pix(row + 1, j + i * 9) =(*m_pal)[( attr & ATTR_FOREG ) == ATTR_ULINE ? fg : bg];
						bitmap.pix(row + 2, j + i * 9) = (*m_pal)[bg];
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------
//  Port definitions
//--------------------------------------------------------------------
static INPUT_PORTS_START( epc_mda )
	PORT_START( "S1" )
	PORT_DIPNAME( 0x01, 0x00, "Color emulation") PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING( 0x00, "Disabled" )
	PORT_DIPSETTING( 0x01, "Enabled" )
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "S1:2")

	PORT_START( "MONITOR" )
	PORT_CONFNAME( 0x01, 0x00, "Ericsson Monochrome HR Monitors") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(isa8_epc_mda_device::monitor_changed), 0)
	PORT_CONFSETTING(    0x00, "Amber 3111")
	PORT_CONFSETTING(    0x01, "B&W 3712/3715")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( isa8_epc_mda_device::monitor_changed )
{
	if ((m_io_monitor->read() & 1) == 1)
	{
		m_pal = &m_371x_pal;
	}
	else
	{
		m_pal = &m_3111_pal;
	}
}

ioport_constructor isa8_epc_mda_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epc_mda );
}
