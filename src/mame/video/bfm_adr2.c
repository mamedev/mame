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
#include "cpu/m6809/m6809.h"
#include "machine/bfm_bd1.h"  // vfd
#include "video/bfm_adr2.h"
#include "rendlay.h"

#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_CTRL(x) do { if (VERBOSE) logerror x; } while (0)

// local vars /////////////////////////////////////////////////////////////

#define SL_DISPLAY    0x02	// displayed Adder screen,  1=screen1 0=screen0
#define SL_ACCESS     0x01	// accessable Adder screen, 1=screen1 0=screen0

static int adder2_screen_page_reg;		  // access/display select
static int adder2_c101;
static int adder2_rx;
static int adder_vbl_triggered;			  // flag <>0, VBL IRQ triggered
static int adder2_acia_triggered;		  // flag <>0, ACIA receive IRQ

static UINT8 adder_ram[0xE80];				// normal RAM
static UINT8 adder_screen_ram[2][0x1180];	// paged  display RAM

static tilemap_t *tilemap0;  // tilemap screen0
static tilemap_t *tilemap1;  // timemap screen1

static UINT8 adder2_data_from_sc2;
static UINT8 adder2_data_to_sc2;

static UINT8 adder2_data;
static UINT8 adder2_sc2data;

///////////////////////////////////////////////////////////////////////////

static TILE_GET_INFO( get_tile0_info )
{
	short data;
	int  code,  color, flags,x,y;

	y = tile_index / 50;
	x = tile_index - (y*50);

	tile_index = y * 128 + (x * 2);

	data =  adder_screen_ram[0][tile_index    ]<<8;
	data |= adder_screen_ram[0][tile_index + 1];

	code  = data & 0x1FFF;
	color = 0;
	flags = ((data & 0x4000)?TILE_FLIPY:0) |
			((data & 0x2000)?TILE_FLIPX:0);

	SET_TILE_INFO(0, code, color, flags);
}

///////////////////////////////////////////////////////////////////////////

static TILE_GET_INFO( get_tile1_info )
{
	short data;
	int  code,  color, flags,x,y;

	y = tile_index / 50;
	x = tile_index - (y*50);

	tile_index = y * 128 + (x * 2);

	data =  adder_screen_ram[1][tile_index    ]<<8;
	data |= adder_screen_ram[1][tile_index + 1];

	code  = data & 0x1FFF;
	color = 0;
	flags = ((data & 0x4000)?TILE_FLIPY:0) |
			((data & 0x2000)?TILE_FLIPX:0);

	SET_TILE_INFO(0, code, color, flags);
}

// video initialisation ///////////////////////////////////////////////////

VIDEO_RESET( adder2 )
{
	adder2_screen_page_reg   = 0;
	adder2_c101              = 0;
	adder2_rx                = 0;
	adder_vbl_triggered      = 0;
	adder2_acia_triggered    = 0;
	adder2_data_from_sc2     = 0;
	adder2_data_to_sc2       = 0;

	{
		UINT8 *rom = machine.root_device().memregion("adder2")->base();

		machine.root_device().membank("bank2")->configure_entries(0, 4, &rom[0x00000], 0x08000);

		machine.root_device().membank("bank2")->set_entry(0&0x03);
	}
}

VIDEO_START( adder2 )
{
	state_save_register_global(machine, adder2_screen_page_reg);
	state_save_register_global(machine, adder2_c101);
	state_save_register_global(machine, adder2_rx);
	state_save_register_global(machine, adder_vbl_triggered);
	state_save_register_global(machine, adder2_acia_triggered);

	state_save_register_global(machine, adder2_data_from_sc2);
	state_save_register_global(machine, adder2_data_to_sc2);

	state_save_register_item_array(machine, "Adder", NULL, 0, adder_ram);
	state_save_register_item_2d_array(machine, "Adder", NULL, 0, adder_screen_ram);

	tilemap0 = tilemap_create(machine, get_tile0_info, TILEMAP_SCAN_ROWS,  8, 8, 50, 35);

	tilemap1 = tilemap_create(machine, get_tile1_info, TILEMAP_SCAN_ROWS,  8, 8, 50, 35);
}

// video update ///////////////////////////////////////////////////////////
SCREEN_UPDATE_IND16( adder2 )
{
	const rectangle visible1(0, 400-1,  0,  280-1);  //minx,maxx, miny,maxy

	if (adder2_screen_page_reg & SL_DISPLAY) tilemap1->draw(bitmap, visible1, 0, 0);
	else                                     tilemap0->draw(bitmap, visible1, 0, 0);

	return 0;
}

// adder2 palette initialisation //////////////////////////////////////////

PALETTE_INIT( adder2 )
{
	palette_set_color(machine, 0,MAKE_RGB(0x00,0x00,0x00));
	palette_set_color(machine, 1,MAKE_RGB(0x00,0x00,0xFF));
	palette_set_color(machine, 2,MAKE_RGB(0x00,0xFF,0x00));
	palette_set_color(machine, 3,MAKE_RGB(0x00,0xFF,0xFF));
	palette_set_color(machine, 4,MAKE_RGB(0xFF,0x00,0x00));
	palette_set_color(machine, 5,MAKE_RGB(0xFF,0x00,0xFF));
	palette_set_color(machine, 6,MAKE_RGB(0xFF,0xFF,0x00));
	palette_set_color(machine, 7,MAKE_RGB(0xFF,0xFF,0xFF));
	palette_set_color(machine, 8,MAKE_RGB(0x80,0x80,0x80));
	palette_set_color(machine, 9,MAKE_RGB(0x00,0x00,0x80));
	palette_set_color(machine,10,MAKE_RGB(0x00,0x80,0x00));
	palette_set_color(machine,11,MAKE_RGB(0x00,0x80,0x80));
	palette_set_color(machine,12,MAKE_RGB(0x80,0x00,0x00));
	palette_set_color(machine,13,MAKE_RGB(0x80,0x00,0x80));
	palette_set_color(machine,14,MAKE_RGB(0x80,0x80,0x00));
	palette_set_color(machine,15,MAKE_RGB(0x80,0x80,0x80));
}

///////////////////////////////////////////////////////////////////////////

INTERRUPT_GEN( adder2_vbl )
{
	if ( adder2_c101 & 0x01 )
	{
		adder_vbl_triggered = 1;
		device_set_input_line(device, M6809_IRQ_LINE, HOLD_LINE );
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( screen_ram_r )
{
	return adder2_screen_page_reg & SL_ACCESS ? adder_screen_ram[1][offset]:adder_screen_ram[0][offset];
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( screen_ram_w )
{
	int dirty_off = (offset>>7)*50 + ((offset & 0x7F)>>1);

	if ( offset > 102 && offset < 102+1+16 )
	{ // format xxxrrggb ////////////////////////////////////////////////////
		int pal;
		UINT8 r,g,b;

		pal = offset-102-1;

		r = ((data & 0x18)>>3) *  85;  // 00011000b = 0x18
		g = ((data & 0x06)>>1) *  85;  // 00000110b = 0x06
		b = ((data & 0x01)   ) * 255;
		palette_set_color(space->machine(), pal, MAKE_RGB(r,g,b));
	}

	if ( adder2_screen_page_reg & SL_ACCESS )
	{
		adder_screen_ram[1][offset] = data;
		tilemap1->mark_tile_dirty(dirty_off);
	}

	else
	{
		adder_screen_ram[0][offset] = data;
		tilemap0->mark_tile_dirty(dirty_off);
	}
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( normal_ram_r )
{
	return adder_ram[offset];
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( normal_ram_w )
{
	adder_ram[offset] = data;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( adder2_rom_page_w )
{
	space->machine().root_device().membank("bank2")->set_entry(data&0x03);
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( adder2_c001_w )
{
	logerror("c101 = %02X\n",data);

	//adder2_screen_page_reg = 0;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( adder2_screen_page_w )
{
	adder2_screen_page_reg = data;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( adder2_vbl_ctrl_r )
{
	adder_vbl_triggered = 0;	// clear VBL start IRQ

	return adder2_c101;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( adder2_vbl_ctrl_w )
{
	adder2_c101 = data;
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( adder2_uart_ctrl_r )
{
	int status = 0;

	if ( adder2_data_from_sc2 ) status |= 0x01; // receive  buffer full
	if ( !adder2_data_to_sc2 ) status |= 0x02; // transmit buffer empty

	return status;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( adder2_uart_ctrl_w )
{
	adder2_data_from_sc2 = 0;	// data available for adder from sc2
	adder2_sc2data       = 0;	// data
	adder2_data_to_sc2   = 0;	// data available for sc2 from adder
	adder2_data          = 0;	// data

	LOG_CTRL(("adder2 uart ctrl:%02X\n", data));
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( adder2_uart_rx_r )
{
	int data = adder2_sc2data;
	adder2_data_from_sc2 = 0;		// clr flag, data from scorpion2 board available

	LOG_CTRL(("rsc2:%02X  (%c)\n",data, data ));

	return data;
}

///////////////////////////////////////////////////////////////////////////

static WRITE8_HANDLER( adder2_uart_tx_w )
{
	adder2_data_to_sc2 = 1;		// set flag, data from adder available
	adder2_data       = data;	// store data

	LOG_CTRL(("ssc2    %02X(%c)\n",data, data ));
}

///////////////////////////////////////////////////////////////////////////

static READ8_HANDLER( adder2_irq_r )
{
	int status = 0;

	if ( adder_vbl_triggered )  status |= 0x02;
	if ( adder2_acia_triggered ) status |= 0x08;

	return status;
}

void adder2_send(int data)
{
	adder2_data_from_sc2 = 1;		// set flag, data from scorpion2 board available
	adder2_sc2data       = data;	// store data

	adder2_acia_triggered = 1;		// set flag, acia IRQ triggered
}

int adder2_receive(void)
{
	UINT8 data = adder2_data;
	adder2_data_to_sc2 = 0;	  // clr flag, data from adder available

	return data;
}

int adder2_status()
{
	int status = 0;

	if ( adder2_data_to_sc2  ) status |= 0x01; // receive  buffer full
	if ( !adder2_data_from_sc2) status |= 0x02; // transmit buffer empty

	return status;
}

////////////////////////////////////////////////////////////////////
//                                                                //
// decode character data to a format which can be decoded by MAME //
//                                                                //
////////////////////////////////////////////////////////////////////

void adder2_decode_char_roms(running_machine &machine)
{
	UINT8 *p;

	p = machine.root_device().memregion("gfx1")->base();

	if ( p )
	{
		UINT8 *s;

		s = auto_alloc_array(machine, UINT8, 0x40000 );
		{
			int x, y;

			memcpy(s, p, 0x40000);

			y = 0;

			while ( y < 128 )
			{
				x = 0;
				while ( x < 64 )
				{
					UINT8 *src = s + (y*256*8)+(x*4);

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
			auto_free(machine, s);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// adder2 board memorymap /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

ADDRESS_MAP_START( adder2_memmap, AS_PROGRAM, 8, driver_device )

	AM_RANGE(0x0000, 0x0000) AM_WRITE_LEGACY(adder2_screen_page_w)		// screen access/display select
	AM_RANGE(0x0000, 0x7FFF) AM_ROMBANK("bank2")				// 8k  paged ROM (4 pages)
	AM_RANGE(0x8000, 0x917F) AM_READWRITE_LEGACY(screen_ram_r, screen_ram_w)
	AM_RANGE(0x9180, 0x9FFF) AM_READWRITE_LEGACY(normal_ram_r, normal_ram_w)

	AM_RANGE(0xC000, 0xC000) AM_WRITE_LEGACY(adder2_rom_page_w)		// ROM page select
	AM_RANGE(0xC001, 0xC001) AM_WRITE_LEGACY(adder2_c001_w)			// ??

	AM_RANGE(0xC101, 0xC101) AM_READWRITE_LEGACY(adder2_vbl_ctrl_r, adder2_vbl_ctrl_w)
	AM_RANGE(0xC103, 0xC103) AM_READ_LEGACY(adder2_irq_r)				// IRQ latch read

	// MC6850 compatible uart connected to main (scorpion2) board ///////////////////////////////////////

	AM_RANGE(0xC200, 0xC200) AM_READWRITE_LEGACY(adder2_uart_ctrl_r, adder2_uart_ctrl_w )	// 6850 compatible uart control reg
	AM_RANGE(0xC201, 0xC201) AM_READWRITE_LEGACY(adder2_uart_rx_r, adder2_uart_tx_w )	// 6850 compatible uart data reg

	AM_RANGE(0xE000, 0xFFFF) AM_ROM								// 8k  ROM
ADDRESS_MAP_END

static const gfx_layout charlayout =
{
	8,8,		  // 8 * 8 characters
	8192,		  // 8192  characters
	4,		  // 4     bits per pixel
	{ 0,1,2,3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },
	8*8*4
};

// this is a strange beast !!!!
//
// characters are grouped by 64 (512 pixels)
// there are max 128 of these groups

GFXDECODE_START( adder2 )
	GFXDECODE_ENTRY( "gfx1",  0, charlayout, 0, 16 )
GFXDECODE_END

///////////////////////////////////////////////////////////////////////////
