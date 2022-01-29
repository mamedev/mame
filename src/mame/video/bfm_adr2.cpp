// license:BSD-3-Clause
// copyright-holders:James Wallace
/***************************************************************************

  Bellfruit Adder2 video board driver, (under heavy construction !!!)

  30-12-2006: State save support added (J. Wallace)
  16-08-2005: Decoupled from AGEMAME by El Condor
  19-08-2005: Re-Animator


CPU memorymap:

   hex     |r/w| D D D D D D D D |
 location  |   | 7 6 5 4 3 2 1 0 | function
-----------+---+-----------------+-----------------------------------------
0000       | W | ? ? ? ? ? ? D D | Screen Page latch

                                   bit0 --> 0 = display screen0
                                            1 = display screen1

                                   bit1 --> 0 = CPU can access screen0
                                            1 = CPU can access screen1

-----------+---+-----------------+-----------------------------------------
0000-7FFF  | R | D D D D D D D D | Paged ROM (4 pages)
-----------+---+-----------------+-----------------------------------------
8000-917F  |R/W| D D D D D D D D | Paged Screen RAM (2 pages)
                                 | screen size 128 x 35 bytes
-----------+---+-----------------+-----------------------------------------
9180-9FFF  |R/W| D D D D D D D D | RAM (used for program + stack)
-----------+---+-----------------+-----------------------------------------
A000-BFFF  |R/W| D D D D D D D D | ?window into character RAM/ROM?
-----------+---+-----------------+-----------------------------------------
C000-DFFF  |?/W| D D D D D D D D | I/O registers
C000       | W | ? ? ? ? ? ? D D | program ROM page select
                                   controls what portion of the eprom is
                                   mapped at 0000 - 7FFFF

                                    ______________________________________
                                   |bit1 | bit0 | Address in eprom        |
                                   |-----+------+-------------------------+
                                   |0    | 0    | 00000 - 07FFF           |
                                   |-----+------+-------------------------+
                                   |0    | 1    | 08000 - 0FFFF (not used)|
                                   |-----+------+-------------------------+
                                   |1    | 0    | 10000 - 17FFF (not used)|
                                   |-----+------+-------------------------+
                                   |1    | 1    | 18000 - 1FFFF           |

-----------+---+-----------------+-----------------------------------------
C001       | W | ? ? ? ? ? ? ? D | Palette enable (seems to turn off red)
           |   |                 | 0 = palette disabled (red signal always 0)
           |   |                 | 1 = palette enabled
-----------+---+-----------------+-----------------------------------------
C002       | W | ? ? ? ? D D D D | Character page register (not used)
-----------+---+-----------------+-----------------------------------------
C100       |R/W| ? ? ? ? ? ? ? ? | Raster IRQ ? (not used in game software)
-----------+---+-----------------+-----------------------------------------
C101       |R/W| ? ? ? ? ? ? ? D | Vertical Blanking IRQ enable
           |   |                 |  bit0  0 = disabled
           |   |                 |        1 = enabled, generate IRQ
-----------+---+-----------------+-----------------------------------------
C102       |R/W| ? ? ? ? ? ? ? D | Pre Vertical Blanking IRQ enable
           |   |                 |  bit0  0 = disabled
           |   |                 |        1 = enabled,
           |   |                 |            generate IRQ 100 cycles
           |   |                 |            before  VBL
-----------+---+-----------------+-----------------------------------------
C103       | R | ? ? ? D D D D D | IRQ status
           |   |                 |
           |   |                 |   b0 = Raster IRQ status
           |   |                 |   b1 = VBL start
           |   |                 |   b2 = VBL end
           |   |                 |   b3 = UART IRQ
           |   |                 |   b4 = ???
           |   |                 |
C103       | W | D D D D D D D D | Raster IRQ line number
-----------+---+-----------------+-----------------------------------------
C200       |R/W| D D D D D D D D | UART control reg. (MC6850 compatible)
-----------+---+-----------------+-----------------------------------------
C201       |R/W| D D D D D D D D | UART data    reg. (MC6850 compatible)
-----------+---+-----------------+-----------------------------------------
C202       | W | ? ? ? ? ? ? ? ? | ??
-----------+---+-----------------+-----------------------------------------
C300-C301  |R/W| D D D D D D D D | ?external MC6850??
-----------+---+-----------------+-----------------------------------------
C302       |R/W| D D D D D D D D | board unlock? something something?
-----------+---+-----------------+-----------------------------------------
E000-FFFF  | R | D D D D D D D D | 8K ROM
-----------+---+-----------------+-----------------------------------------

***************************************************************************/

#include "emu.h"
#include "video/bfm_adr2.h"

#include "cpu/m6809/m6809.h"
#include "machine/bfm_bd1.h"  // vfd
#include "emupal.h"
#include "screen.h"

#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_CTRL(x) do { if (VERBOSE) logerror x; } while (0)

// local vars /////////////////////////////////////////////////////////////

#define SL_DISPLAY    0x02  // displayed Adder screen,  1=screen1 0=screen0
#define SL_ACCESS     0x01  // accessable Adder screen, 1=screen1 0=screen0

#define ADDER_CLOCK     (XTAL(8'000'000))



static const gfx_layout charlayout =
{
	8,8,          // 8 * 8 characters
	8192,         // 8192  characters
	4,        // 4     bits per pixel
	{ 0,1,2,3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
	8*8*4
};

// this is a strange beast !!!!
//
// characters are grouped by 64 (512 pixels)
// there are max 128 of these groups

static GFXDECODE_START( gfx_adder2 )
	GFXDECODE_ENTRY( ":gfx1",  0, charlayout, 0, 16 )
GFXDECODE_END

DEFINE_DEVICE_TYPE(BFM_ADDER2, bfm_adder2_device, "bfm_adder2", "BFM ADDER2")

bfm_adder2_device::bfm_adder2_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: device_t(mconfig, BFM_ADDER2, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfx_adder2, "palette")
	, m_cpu(*this, "adder2")
{
}

///////////////////////////////////////////////////////////////////////////

TILE_GET_INFO_MEMBER( bfm_adder2_device::get_tile0_info )
{
	short data;
	int  code,  color, flags,x,y;

	y = tile_index / 50;
	x = tile_index - (y*50);

	tile_index = y * 128 + (x * 2);

	data =  m_screen_ram[0][tile_index    ]<<8;
	data |= m_screen_ram[0][tile_index + 1];

	code  = data & 0x1FFF;
	color = 0;
	flags = ((data & 0x4000)?TILE_FLIPY:0) |
			((data & 0x2000)?TILE_FLIPX:0);

	tileinfo.set(0, code, color, flags);
}

///////////////////////////////////////////////////////////////////////////

TILE_GET_INFO_MEMBER( bfm_adder2_device::get_tile1_info )
{
	short data;
	int  code,  color, flags,x,y;

	y = tile_index / 50;
	x = tile_index - (y*50);

	tile_index = y * 128 + (x * 2);

	data =  m_screen_ram[1][tile_index    ]<<8;
	data |= m_screen_ram[1][tile_index + 1];

	code  = data & 0x1FFF;
	color = 0;
	flags = ((data & 0x4000)?TILE_FLIPY:0) |
			((data & 0x2000)?TILE_FLIPX:0);

	tileinfo.set(0, code, color, flags);
}

// video initialisation ///////////////////////////////////////////////////

void bfm_adder2_device::device_reset()
{
	m_screen_page_reg   = 0;
	m_c101              = 0;
	m_rx                = 0;
	m_vbl_triggered     = false;
	m_acia_triggered    = false;
	m_data_from_sc2     = 0;
	m_data_to_sc2       = 0;

	{
		uint8_t *rom = machine().root_device().memregion("adder2")->base();

		membank("bank2")->configure_entries(0, 4, &rom[0x00000], 0x08000);

		membank("bank2")->set_entry(0&0x03);
	}
}

void bfm_adder2_device::device_start()
{
	if (!palette().device().started())
		throw device_missing_dependencies();

	adder2_decode_char_roms();

	save_item(NAME(m_screen_page_reg));
	save_item(NAME(m_c101));
	save_item(NAME(m_rx));
	save_item(NAME(m_vbl_triggered));
	save_item(NAME(m_acia_triggered));

	save_item(NAME(m_data_from_sc2));
	save_item(NAME(m_data_to_sc2));

	save_item(NAME(m_adder_ram));
	save_item(NAME(m_screen_ram));

	m_tilemap0 = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(bfm_adder2_device::get_tile0_info)), TILEMAP_SCAN_ROWS,  8, 8, 50, 35);

	m_tilemap1 = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(bfm_adder2_device::get_tile1_info)), TILEMAP_SCAN_ROWS,  8, 8, 50, 35);

	palette().set_pen_color(0,rgb_t(0x00,0x00,0x00));
	palette().set_pen_color(1,rgb_t(0x00,0x00,0xFF));
	palette().set_pen_color(2,rgb_t(0x00,0xFF,0x00));
	palette().set_pen_color(3,rgb_t(0x00,0xFF,0xFF));
	palette().set_pen_color(4,rgb_t(0xFF,0x00,0x00));
	palette().set_pen_color(5,rgb_t(0xFF,0x00,0xFF));
	palette().set_pen_color(6,rgb_t(0xFF,0xFF,0x00));
	palette().set_pen_color(7,rgb_t(0xFF,0xFF,0xFF));
	palette().set_pen_color(8,rgb_t(0x80,0x80,0x80));
	palette().set_pen_color(9,rgb_t(0x00,0x00,0x80));
	palette().set_pen_color(10,rgb_t(0x00,0x80,0x00));
	palette().set_pen_color(11,rgb_t(0x00,0x80,0x80));
	palette().set_pen_color(12,rgb_t(0x80,0x00,0x00));
	palette().set_pen_color(13,rgb_t(0x80,0x00,0x80));
	palette().set_pen_color(14,rgb_t(0x80,0x80,0x00));
	palette().set_pen_color(15,rgb_t(0x80,0x80,0x80));
}

// video update ///////////////////////////////////////////////////////////
uint32_t bfm_adder2_device::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle visible1(0, 400-1,  0,  280-1);  //minx,maxx, miny,maxy

	if (m_screen_page_reg & SL_DISPLAY)
		m_tilemap1->draw(screen, bitmap, visible1, 0, 0);
	else
		m_tilemap0->draw(screen, bitmap, visible1, 0, 0);

	return 0;
}

///////////////////////////////////////////////////////////////////////////

WRITE_LINE_MEMBER(bfm_adder2_device::adder2_vbl_w)
{
	if (state && BIT(m_c101, 0))
	{
		m_vbl_triggered = true;
		m_cpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);
	}
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfm_adder2_device::screen_ram_r(offs_t offset)
{
	return m_screen_page_reg & SL_ACCESS ? m_screen_ram[1][offset]:m_screen_ram[0][offset];
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::screen_ram_w(offs_t offset, uint8_t data)
{
	int dirty_off = (offset>>7)*50 + ((offset & 0x7F)>>1);

	if ( offset > 102 && offset < 102+1+16 )
	{ // format xxxrrggb ////////////////////////////////////////////////////
		int pal;
		uint8_t r,g,b;

		pal = offset-102-1;

		r = ((data & 0x18)>>3) *  85;  // 00011000b = 0x18
		g = ((data & 0x06)>>1) *  85;  // 00000110b = 0x06
		b = ((data & 0x01)   ) * 255;
		palette().set_pen_color(pal, rgb_t(r,g,b));
	}

	if (m_screen_page_reg & SL_ACCESS)
	{
		m_screen_ram[1][offset] = data;
		m_tilemap1->mark_tile_dirty(dirty_off);
	}

	else
	{
		m_screen_ram[0][offset] = data;
		m_tilemap0->mark_tile_dirty(dirty_off);
	}
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfm_adder2_device::normal_ram_r(offs_t offset)
{
	return m_adder_ram[offset];
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::normal_ram_w(offs_t offset, uint8_t data)
{
	m_adder_ram[offset] = data;
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::adder2_rom_page_w(uint8_t data)
{
	membank("bank2")->set_entry(data&0x03);
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::adder2_c001_w(uint8_t data)
{
	logerror("c101 = %02X\n",data);

	//m_screen_page_reg = 0;
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::adder2_screen_page_w(uint8_t data)
{
	m_screen_page_reg = data;
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfm_adder2_device::adder2_vbl_ctrl_r()
{
	m_vbl_triggered = false;    // clear VBL start IRQ

	return m_c101;
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::adder2_vbl_ctrl_w(uint8_t data)
{
	m_c101 = data;
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfm_adder2_device::adder2_uart_ctrl_r()
{
	int status = 0;

	if (m_data_from_sc2) status |= 0x01; // receive  buffer full
	if (!m_data_to_sc2 ) status |= 0x02; // transmit buffer empty

	return status;
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::adder2_uart_ctrl_w(uint8_t data)
{
	m_data_from_sc2 = false;    // data available for adder from sc2
	m_sc2data       = 0;        // data
	m_data_to_sc2   = false;    // data available for sc2 from adder
	m_adder2_data   = 0;        // data

	LOG_CTRL(("adder2 uart ctrl:%02X\n", data));
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfm_adder2_device::adder2_uart_rx_r()
{
	int data = m_sc2data;
	m_data_from_sc2 = false;    // clr flag, data from scorpion2 board available

	LOG_CTRL(("rsc2:%02X  (%c)\n",data, data ));

	return data;
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::adder2_uart_tx_w(uint8_t data)
{
	m_data_to_sc2 = true;       // set flag, data from adder available
	m_adder2_data = data;       // store data

	LOG_CTRL(("ssc2    %02X(%c)\n",data, data ));
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfm_adder2_device::adder2_irq_r()
{
	int status = 0;

	if (m_vbl_triggered)  status |= 0x02;
	if (m_acia_triggered) status |= 0x08;

	return status;
}


///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::vid_uart_tx_w(uint8_t data)
{
	m_data_from_sc2  = true;    // set flag, data from scorpion2 board available
	m_sc2data        = data;    // store data

	m_acia_triggered = true;    // set flag, acia IRQ triggered

	m_cpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE );

	//LOG_SERIAL(("sadder  %02X  (%c)\n",data, data ));
}

///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::vid_uart_ctrl_w(uint8_t data)
{
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfm_adder2_device::vid_uart_rx_r()
{
	uint8_t data = m_adder2_data;
	m_data_to_sc2 = false;   // clr flag, data from adder available

	//LOG_SERIAL(("radder:  %02X(%c)\n",data, data ));

	return data;
}

///////////////////////////////////////////////////////////////////////////

uint8_t bfm_adder2_device::vid_uart_ctrl_r()
{
	int status = 0;

	if (m_data_to_sc2   ) status |= 0x01; // receive  buffer full
	if (!m_data_from_sc2) status |= 0x02; // transmit buffer empty

	return status;
}


////////////////////////////////////////////////////////////////////
//                                                                //
// decode character data to a format which can be decoded by MAME //
//                                                                //
////////////////////////////////////////////////////////////////////

void bfm_adder2_device::adder2_decode_char_roms()
{
	uint8_t *p = machine().root_device().memregion("gfx1")->base();

	if ( p )
	{
		std::vector<uint8_t> s( 0x40000 );
		{
			int x, y;

			memcpy(&s[0], p, 0x40000);

			y = 0;

			while ( y < 128 )
			{
				x = 0;
				while ( x < 64 )
				{
					uint8_t *src = &s[(y*256*8)+(x*4)];

					*p++ = src[0*256+0];*p++ = src[0*256+1];*p++ = src[0*256+2];*p++ = src[0*256+3];
					*p++ = src[1*256+0];*p++ = src[1*256+1];*p++ = src[1*256+2];*p++ = src[1*256+3];
					*p++ = src[2*256+0];*p++ = src[2*256+1];*p++ = src[2*256+2];*p++ = src[2*256+3];
					*p++ = src[3*256+0];*p++ = src[3*256+1];*p++ = src[3*256+2];*p++ = src[3*256+3];
					*p++ = src[4*256+0];*p++ = src[4*256+1];*p++ = src[4*256+2];*p++ = src[4*256+3];
					*p++ = src[5*256+0];*p++ = src[5*256+1];*p++ = src[5*256+2];*p++ = src[5*256+3];
					*p++ = src[6*256+0];*p++ = src[6*256+1];*p++ = src[6*256+2];*p++ = src[6*256+3];
					*p++ = src[7*256+0];*p++ = src[7*256+1];*p++ = src[7*256+2];*p++ = src[7*256+3];
					x++;
				}
				y++;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// adder2 board memorymap /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void bfm_adder2_device::adder2_memmap(address_map &map)
{

	map(0x0000, 0x0000).w(FUNC(bfm_adder2_device::adder2_screen_page_w));      // screen access/display select
	map(0x0000, 0x7FFF).bankr("bank2");                // 8k  paged ROM (4 pages)
	map(0x8000, 0x917F).rw(FUNC(bfm_adder2_device::screen_ram_r), FUNC(bfm_adder2_device::screen_ram_w));
	map(0x9180, 0x9FFF).rw(FUNC(bfm_adder2_device::normal_ram_r), FUNC(bfm_adder2_device::normal_ram_w));

	map(0xC000, 0xC000).w(FUNC(bfm_adder2_device::adder2_rom_page_w));     // ROM page select
	map(0xC001, 0xC001).w(FUNC(bfm_adder2_device::adder2_c001_w));         // ??

	map(0xC101, 0xC101).rw(FUNC(bfm_adder2_device::adder2_vbl_ctrl_r), FUNC(bfm_adder2_device::adder2_vbl_ctrl_w));
	map(0xC103, 0xC103).r(FUNC(bfm_adder2_device::adder2_irq_r));               // IRQ latch read

	// MC6850 compatible uart connected to main (scorpion2) board ///////////////////////////////////////

	map(0xC200, 0xC200).rw(FUNC(bfm_adder2_device::adder2_uart_ctrl_r), FUNC(bfm_adder2_device::adder2_uart_ctrl_w));   // 6850 compatible uart control reg
	map(0xC201, 0xC201).rw(FUNC(bfm_adder2_device::adder2_uart_rx_r), FUNC(bfm_adder2_device::adder2_uart_tx_w));   // 6850 compatible uart data reg

	map(0xE000, 0xFFFF).rom().region(":adder2", 0xE000);                         // 8k  ROM
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bfm_adder2_device::device_add_mconfig(machine_config &config)
{
	M6809(config, m_cpu, ADDER_CLOCK/4);  // adder2 board 6809 CPU at 2 Mhz
	m_cpu->set_addrmap(AS_PROGRAM, &bfm_adder2_device::adder2_memmap);             // setup adder2 board memorymap

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(400, 280);
	screen.set_visarea(0, 400-1, 0, 280-1);
	screen.set_refresh_hz(50);
	screen.set_palette("palette");
	screen.set_screen_update(FUNC(bfm_adder2_device::update_screen));
	screen.screen_vblank().set(FUNC(bfm_adder2_device::adder2_vbl_w));      // board has a VBL IRQ

	PALETTE(config, "palette").set_entries(16);
}
