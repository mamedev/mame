/***************************************************************************

                      -= Touch Master / Galaxy Games =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU:    68000
Video:  Blitter, double framebuffer
Sound:  OKI6295

[Touch Master]

Input:  Pressure sensitive touch screen
Other:  Dallas NVRAM + optional RTC
To Do:

- Proper touchscreen controller emulation. Currently it's flakey and
  tends to stop registering user input (try coining up in that case)
- Protection in tm4k
- Coin optics
- RTC emulation (there's code to check the upper bytes of NVRAM to see if
  the real time clock is present, and to only use it in that case)

[Galaxy Games]

Input:  Trackballs and buttons
Other:  EEPROM
To Do:

- Coin optics

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/okim6295.h"
#include "machine/eeprom.h"

/***************************************************************************

                                   Sound

***************************************************************************/

static int okibank;
static WRITE16_HANDLER( tmaster_oki_bank_w )
{
	if (ACCESSING_MSB)
	{
		// data & 0x0800?
		okibank = ((data >> 8) & 3);
		OKIM6295_set_bank_base(0, okibank * 0x40000);
	}

	if (ACCESSING_LSB)
	{
		// data & 0x0002?
	}
}

/***************************************************************************

                     Touch Screen Controller - PRELIMINARY

***************************************************************************/

static int touchscreen;

static void show_touchscreen(void)
{
#ifdef MAME_DEBUG
	popmessage("% d] %03x %03x - %d",touchscreen,readinputportbytag("TSCREEN_X")&0x1ff,readinputportbytag("TSCREEN_Y"),okibank);
#endif
}

static WRITE16_HANDLER( tmaster_tscreen_reset_w )
{
	if (ACCESSING_LSB && data == 0x05)
	{
		touchscreen = 0;
		show_touchscreen();
	}
}

static READ16_HANDLER( tmaster_tscreen_next_r )
{
	if (touchscreen != -1)
		touchscreen++;
	if (touchscreen == 6)
		touchscreen = -1;
	show_touchscreen();

	return 0;
}

static READ16_HANDLER( tmaster_tscreen_x_hi_r )
{
	switch (touchscreen)
	{
		case -1:	return 0xf1;
	}
	return 0x01;
}

static READ16_HANDLER( tmaster_tscreen_x_lo_r )
{
	UINT16 val = 0;

	int press1 = readinputportbytag("TSCREEN_X") & 0x4000;
	int press2 = readinputportbytag("TSCREEN_X") & 0x8000;
	if (press1)	press2 = 1;

	switch (touchscreen)
	{
		case 1:	val = press1 ? 0 : 1<<6;	break;	// press
		case 2:	val = readinputportbytag("TSCREEN_X") & 0x003;	break;
		case 3:	val = (readinputportbytag("TSCREEN_X") >> 2) & 0x7f;	break;
		case 4:	val = 0;	break;
		case 5:	val = ((readinputportbytag("TSCREEN_Y")^0xff) >> 1) & 0x7f;	break;

		default:
			return 0;
	}
	return val | (press2 ? 0x80 : 0);	// away : hover
}

static READ16_HANDLER( tmaster_tscreen_y_hi_r )	{	return 0x01;	}
static READ16_HANDLER( tmaster_tscreen_y_lo_r )	{	return 0x00;	}



/***************************************************************************

                                Video & Blitter


    Offset:     Bits:           Value:

        02      
                fedc ba-- ---- ----
                ---- --9- ---- ----       Layer 1 Buffer To Display
                ---- ---8 ---- ----       Layer 0 Buffer To Display
                ---- ---- 7654 3210

        04                                Width
        06                                X

        08                                Height - 1
        0A                                Y

        0C                                Source Address (low)
        0E                                Source Address (mid)

        10      fedc ba98 ---- ---- 
                ---- ---- 7--- ----       Layer
                ---- ---- -6-- ----       Buffer
                ---- ---- --5- ----       Solid Fill
                ---- ---- ---4 ----       flipped by lev.3 interrupt routine
                ---- ---- ---- 3---       flipped by lev.2 interrupt routine
                ---- ---- ---- -2--       flipped by lev.1 interrupt routine
                ---- ---- ---- --1-       Flip Y
                ---- ---- ---- ---0       Flip X

        12      fedc ba98 ---- ----       Solid Fill Pen
                ---- ---- 7654 3210       Source Address (high)

    A write to the source address (high) triggers the blit.
    A the end of the blit, a level 2 IRQ is issued.

***************************************************************************/

static mame_bitmap *tmaster_bitmap[2][2];	// 2 layers, 2 buffers per layer
static UINT16 *tmaster_regs;
static UINT16 tmaster_color;
static UINT16 tmaster_addr;
static int (*compute_addr) (UINT16 reg_low, UINT16 reg_mid, UINT16 reg_high);

int tmaster_compute_addr(UINT16 reg_low, UINT16 reg_mid, UINT16 reg_high)
{
	return (reg_low & 0xff) | ((reg_mid & 0x1ff) << 8) | (reg_high << 17);
}

int galgames_compute_addr(UINT16 reg_low, UINT16 reg_mid, UINT16 reg_high)
{
	return reg_low | (reg_mid << 16);
}

static VIDEO_START( tmaster )
{
	int layer, buffer;
	for (layer = 0; layer < 2; layer++)
	{
		for (buffer = 0; buffer < 2; buffer++)
		{
			tmaster_bitmap[layer][buffer] = auto_bitmap_alloc(machine->screen[0].width,machine->screen[0].height,machine->screen[0].format);
			bitmap_fill(tmaster_bitmap[layer][buffer], NULL, 0xff);
		}
	}

	compute_addr = tmaster_compute_addr;
}

static VIDEO_START( galgames )
{
	VIDEO_START_CALL( tmaster );
	compute_addr = galgames_compute_addr;
}

static VIDEO_UPDATE( tmaster )
{
	int layers_ctrl = -1;

#if MAME_DEBUG
	if (input_code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (input_code_pressed(KEYCODE_Q))	mask |= 1;
		if (input_code_pressed(KEYCODE_W))	mask |= 2;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	
	if (layers_ctrl & 1)	copybitmap			(bitmap,tmaster_bitmap[0][(tmaster_regs[0x02/2]>>8)&1],0,0,0,0,&machine->screen[0].visarea);
	else					fillbitmap(bitmap,get_black_pen(machine),cliprect);

	if (layers_ctrl & 2)	copybitmap_trans	(bitmap,tmaster_bitmap[1][(tmaster_regs[0x02/2]>>9)&1],0,0,0,0,&machine->screen[0].visarea,0xff);

	return 0;
}

static WRITE16_HANDLER( tmaster_color_w )
{
	COMBINE_DATA( &tmaster_color );
#if 0
	if (tmaster_color & ~7)
		logerror("%06x: color %04x\n", activecpu_get_pc(), tmaster_color);
#endif
}

static WRITE16_HANDLER( tmaster_addr_w )
{
	COMBINE_DATA( &tmaster_addr );
}

static void tmaster_draw(void)
{
	int x,y,x0,x1,y0,y1,dx,dy,flipx,flipy,sx,sy,sw,sh, addr, mode, layer,buffer, color;

	UINT8 *gfxdata	=	memory_region( REGION_GFX1 );
	size_t size		=	memory_region_length( REGION_GFX1 );

	UINT16 data;

	mame_bitmap *bitmap;

	buffer	=	(tmaster_regs[0x02/2] >> 8) & 3;	// 1 bit per layer, selects the currently displayed buffer
 	sw		=	 tmaster_regs[0x04/2];
	sx		=	 tmaster_regs[0x06/2];
	sh		=	 tmaster_regs[0x08/2] + 1;
	sy		=	 tmaster_regs[0x0a/2];
	addr	=	compute_addr(
				 tmaster_regs[0x0c/2],
				 tmaster_regs[0x0e/2], tmaster_addr);
	mode	=	 tmaster_regs[0x10/2];

	layer	=	(mode >> 7) & 1;	// layer to draw to
	buffer	=	((mode >> 6) & 1) ^ ((buffer >> layer) & 1);	// bit 6 selects whether to use the opposite buffer to that displayed
	bitmap	=	tmaster_bitmap[layer][buffer];

	addr <<= 1;

#if 0
	logerror("%06x: blit w %03x, h %02x, x %03x, y %02x, addr %06x, mode %02x\n", activecpu_get_pc(),
			sw,sh,sx,sy, addr, mode
	);
#endif

	flipx = mode & 1;
	flipy = mode & 2;

	if (flipx)	{ x0 = sw-1;	x1 = -1;	dx = -1;	sx -= sw-1;	}
	else		{ x0 = 0;		x1 = sw;	dx = +1;	}

	if (flipy)	{ y0 = sh-1;	y1 = -1;	dy = -1;	sy -= sh-1;	}
	else		{ y0 = 0;		y1 = sh;	dy = +1;	}

	sx = (sx & 0x7fff) - (sx & 0x8000);
	sy = (sy & 0x7fff) - (sy & 0x8000);

	color = (tmaster_color & 7) << 8;

	switch (mode & 0x20)
	{
		case 0x00:							// blit with transparency
			if (addr > size - sw*sh)
			{
				logerror("%06x: blit error, addr %06x out of bounds\n", activecpu_get_pc(),addr);
				addr = size - sw*sh;
			}

			for (y = y0; y != y1; y += dy)
			{
				for (x = x0; x != x1; x += dx)
				{
					data = gfxdata[addr++];

					if ((data != 0xff) && (sx + x >= 0) && (sx + x < 400) && (sy + y >= 0) && (sy + y < 256))
						*BITMAP_ADDR16(bitmap, sy + y, sx + x) = data + color;
				}
			}
			break;

		case 0x20:							// solid fill
			data = ((tmaster_addr >> 8) & 0xff) + color;
			for (y = y0; y != y1; y += dy)
			{
				for (x = x0; x != x1; x += dx)
				{
					if ((sx + x >= 0) && (sx + x < 400) && (sy + y >= 0) && (sy + y < 256))
						*BITMAP_ADDR16(bitmap, sy + y, sx + x) = data;
				}
			}
			break;

	}
}

static WRITE16_HANDLER( tmaster_blitter_w )
{
	COMBINE_DATA( tmaster_regs + offset );
	switch (offset*2)
	{
		case 0x0e:
			tmaster_draw();
			cpunum_set_input_line(Machine, 0, 2, HOLD_LINE);
			break;
	}
}

static READ16_HANDLER( tmaster_blitter_r )
{
	return 0x0000;	// bit 7 = 1 -> blitter busy
}

/***************************************************************************

                                Memory Maps

***************************************************************************/

/***************************************************************************
                                Touch Master
***************************************************************************/

static READ16_HANDLER( tmaster_coins_r )
{
	return readinputportbytag("COIN")|(mame_rand(Machine)&0x0800);
}

static ADDRESS_MAP_START( tmaster_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x1fffff ) AM_ROM
	AM_RANGE( 0x200000, 0x27ffff ) AM_RAM
	AM_RANGE( 0x280000, 0x28ffff ) AM_RAM AM_BASE(&generic_nvram16) AM_SIZE(&generic_nvram_size)

	AM_RANGE( 0x300010, 0x300011 ) AM_READ( tmaster_coins_r )

	AM_RANGE( 0x300022, 0x300023 ) AM_READ ( tmaster_tscreen_x_hi_r )
	AM_RANGE( 0x300024, 0x300025 ) AM_WRITE( tmaster_tscreen_reset_w )
	AM_RANGE( 0x300026, 0x300027 ) AM_READ ( tmaster_tscreen_x_lo_r )
	AM_RANGE( 0x300028, 0x300029 ) AM_READ ( tmaster_tscreen_next_r )
	AM_RANGE( 0x300032, 0x300033 ) AM_READ ( tmaster_tscreen_y_hi_r )
	AM_RANGE( 0x300036, 0x300037 ) AM_READ ( tmaster_tscreen_y_lo_r )

	AM_RANGE( 0x300040, 0x300041 ) AM_WRITE( tmaster_oki_bank_w )

	AM_RANGE( 0x300070, 0x300071 ) AM_WRITE( tmaster_addr_w )

	AM_RANGE( 0x500000, 0x500011 ) AM_WRITE( tmaster_blitter_w ) AM_BASE( &tmaster_regs )
	AM_RANGE( 0x500010, 0x500011 ) AM_READ ( tmaster_blitter_r )

	AM_RANGE( 0x580000, 0x580001 ) AM_WRITE(MWA16_NOP) // often

	AM_RANGE( 0x600000, 0x600fff ) AM_READWRITE( MRA16_RAM, paletteram16_xBBBBBGGGGGRRRRR_word_w ) AM_BASE(&paletteram16) // looks like palettes, maybe

	AM_RANGE( 0x800000, 0x800001 ) AM_READWRITE( OKIM6295_status_0_lsb_r, OKIM6295_data_0_lsb_w )

	AM_RANGE( 0x800010, 0x800011 ) AM_WRITE( tmaster_color_w )
ADDRESS_MAP_END


/***************************************************************************
                                Galaxy Games
***************************************************************************/

// NVRAM

static const struct EEPROM_interface galgames_eeprom_interface =
{
	10,					// address bits 10
	8,					// data bits    8
	"*1100",			// read         110 0aaaaaaaaaa
	"*1010",			// write        101 0aaaaaaaaaa dddddddd
	"*1110",			// erase        111 0aaaaaaaaaa
	"*10000xxxxxxxxx",	// lock         100 00xxxxxxxxx
	"*10011xxxxxxxxx",	// unlock       100 11xxxxxxxxx
	0,					// multi_read
	1					// reset_delay
};

static READ16_HANDLER( galgames_eeprom_r )
{
	return EEPROM_read_bit() ? 0x80 : 0x00;
}

static WRITE16_HANDLER( galgames_eeprom_w )
{
	if (data & ~0x0003)
		logerror("CPU #0 PC: %06X - Unknown EEPROM bit written %04X\n",activecpu_get_pc(),data);

	if ( ACCESSING_LSB )
	{
		// latch the bit
		EEPROM_write_bit(data & 0x0001);

		// clock line asserted: write latch or select next bit to read
		EEPROM_set_clock_line((data & 0x0002) ? ASSERT_LINE : CLEAR_LINE );
	}
}

static NVRAM_HANDLER( galgames )
{
	if (read_or_write)
		EEPROM_save(file);
	else
	{
		EEPROM_init(&galgames_eeprom_interface);
		if (file)	EEPROM_load(file);
	}
}

// BT481A Palette RAMDAC
static UINT32 palette_offset;
static UINT8 palette_index;
static UINT8 palette_data[3];

static WRITE16_HANDLER( galgames_palette_offset_w )
{
	if (ACCESSING_LSB)
	{
		palette_offset = data & 0xff;
		palette_index = 0;
	}
}
static WRITE16_HANDLER( galgames_palette_data_w )
{
	if (ACCESSING_LSB)
	{
		palette_data[palette_index] = data & 0xff;
		if (++palette_index == 3)
		{
			palette_set_color(Machine, palette_offset, MAKE_RGB(palette_data[0], palette_data[1], palette_data[2]));
			palette_index = 0;
			palette_offset++;
		}
	}
}

// Sound
static READ16_HANDLER( galgames_okiram_r )
{
	return memory_region(REGION_SOUND1)[offset] | 0xff00;
}
static WRITE16_HANDLER( galgames_okiram_w )
{
	if (ACCESSING_LSB)
		memory_region(REGION_SOUND1)[offset] = data & 0xff;
}


// Carts communication (preliminary, no cart is dumped yet)

static WRITE16_HANDLER( galgames_cart_sel_w )
{
	// cart selection (0 1 2 3 4 7)

	// 7 resets the eeprom
	if (ACCESSING_LSB)
		EEPROM_set_cs_line(((data&0xff) == 0x07) ? ASSERT_LINE : CLEAR_LINE);
}

static READ16_HANDLER( galgames_cart_clock_r )
{
	return 0x0080;
}

static WRITE16_HANDLER( galgames_cart_clock_w )
{
	if (ACCESSING_LSB)
	{
		// bit 3 = clock

		// ROM/RAM banking
		if ((data & 0xf7) == 0x05)
		{
			memory_set_bank(1, 1);	// ram
			memory_set_bank(3, 0);	// rom
			logerror("%06x: romram bank = %04x\n", activecpu_get_pc(), data);
		}
		else
		{
			memory_set_bank(1, 0);	// rom
			logerror("%06x: unknown romram bank = %04x\n", activecpu_get_pc(), data);
		}
	}
}

static READ16_HANDLER( galgames_cart_data_r )
{
	return 0;
}
static WRITE16_HANDLER( galgames_cart_data_w )
{
}


static READ16_HANDLER( dummy_read_01 )
{
	return 0x3;	// Pass the check at PC = 0xfae & a later one
}

static ADDRESS_MAP_START( galgames_map, ADDRESS_SPACE_PROGRAM, 16 )

	AM_RANGE( 0x000000, 0x03ffff ) AM_READWRITE(MRA16_BANK1, MWA16_BANK2)
	AM_RANGE( 0x040000, 0x1fffff ) AM_ROM AM_REGION( REGION_CPU1, 0 )
	AM_RANGE( 0x200000, 0x23ffff ) AM_READWRITE(MRA16_BANK3, MWA16_BANK4)
	AM_RANGE( 0x240000, 0x3fffff ) AM_ROM AM_REGION( REGION_CPU1, 0 )

	AM_RANGE( 0x400000, 0x400011 ) AM_WRITE( tmaster_blitter_w ) AM_BASE( &tmaster_regs )
	AM_RANGE( 0x400012, 0x400013 ) AM_WRITE( tmaster_addr_w )
	AM_RANGE( 0x400014, 0x400015 ) AM_WRITE( tmaster_color_w )
	AM_RANGE( 0x400020, 0x400021 ) AM_READ ( tmaster_blitter_r )

	AM_RANGE( 0x600000, 0x600001 ) AM_READWRITE( dummy_read_01, MWA16_NOP )
	AM_RANGE( 0x700000, 0x700001 ) AM_READWRITE( dummy_read_01, MWA16_NOP )
	AM_RANGE( 0x800020, 0x80003f ) AM_NOP	// ?
	AM_RANGE( 0x900000, 0x900001 ) AM_WRITE( watchdog_reset16_w )

	AM_RANGE( 0xa00000, 0xa00001 ) AM_READWRITE( OKIM6295_status_0_lsb_r, OKIM6295_data_0_lsb_w )
	AM_RANGE( 0xb00000, 0xb7ffff ) AM_READWRITE( galgames_okiram_r, galgames_okiram_w ) // (only low bytes tested) 4x N341024SJ-15

	AM_RANGE( 0xc00000, 0xc00001 ) AM_WRITE( galgames_palette_offset_w )
	AM_RANGE( 0xc00002, 0xc00003 ) AM_WRITE( galgames_palette_data_w )

	AM_RANGE( 0xd00000, 0xd00001 ) AM_READ ( input_port_0_word_r )	// trackball p1 x
	AM_RANGE( 0xd00000, 0xd00001 ) AM_WRITE( MWA16_NOP )
	AM_RANGE( 0xd00002, 0xd00003 ) AM_READ ( input_port_1_word_r )	// trackball p1 y
	AM_RANGE( 0xd00004, 0xd00005 ) AM_READ ( input_port_2_word_r )	// trackball p2 x
	AM_RANGE( 0xd00006, 0xd00007 ) AM_READ ( input_port_3_word_r )	// trackball p2 y
	AM_RANGE( 0xd00008, 0xd00009 ) AM_READ ( input_port_4_word_r )
	AM_RANGE( 0xd0000a, 0xd0000b ) AM_READ ( input_port_5_word_r )
	AM_RANGE( 0xd0000c, 0xd0000d ) AM_READWRITE( input_port_6_word_r, MWA16_NOP )

	AM_RANGE( 0xd0000e, 0xd0000f ) AM_WRITE ( galgames_cart_sel_w )
	AM_RANGE( 0xd00010, 0xd00011 ) AM_READWRITE( galgames_eeprom_r, galgames_eeprom_w )
	AM_RANGE( 0xd00012, 0xd00013 ) AM_READWRITE( galgames_cart_data_r, galgames_cart_data_w )
	AM_RANGE( 0xd00014, 0xd00015 ) AM_READWRITE( galgames_cart_clock_r, galgames_cart_clock_w )

ADDRESS_MAP_END


/***************************************************************************

                                Input Ports

***************************************************************************/

static INPUT_PORTS_START( tmaster )
	PORT_START_TAG("TSCREEN_X")
	PORT_BIT( 0x01ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(3) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON1 )	PORT_IMPULSE(5)	// press
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_BUTTON2 )					// hover

	PORT_START_TAG("TSCREEN_Y")
	PORT_BIT( 0x0ff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(3) PORT_PLAYER(1)

	PORT_START_TAG("COIN") // IN3
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_COIN1		)	PORT_IMPULSE(2)	// m. coin 1 (coin optics?)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_COIN1		)	PORT_IMPULSE(4)	// m. coin 2 (coin optics?)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_COIN1		)	PORT_IMPULSE(6)	// dbv input (coin optics?)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_COIN1		)	// (service coin?)
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW	)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_SERVICE1	)	// calibrate
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_UNKNOWN	)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL	)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_COIN1		)	// e. coin 1
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_COIN2		)	// e. coin 2
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_COIN3		)	// e. coin 3
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN4		)	// e. coin 4
INPUT_PORTS_END

static INPUT_PORTS_START( galgames )
	PORT_START_TAG("TRACKBALL_1_X")
    PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_RESET
	PORT_START_TAG("TRACKBALL_1_Y")
    PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_RESET

	PORT_START_TAG("TRACKBALL_2_X")
    PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_RESET
	PORT_START_TAG("TRACKBALL_2_Y")
    PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_RESET

	PORT_START_TAG("IN4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	// Button A (right)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	// Button B (left)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	// Button A (right)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	// Button B (left)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1)					// DBA (coin)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN6")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )	// CS 1 (coin)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )	// CS 2 (coin)
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )	// System Check
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************

                               Machine Drivers

***************************************************************************/

static MACHINE_RESET( tmaster )
{
	touchscreen = -1;
}

static INTERRUPT_GEN( tm3k_interrupt )
{
	switch (cpu_getiloops())
	{
		case 0:		cpunum_set_input_line(machine, 0, 2, HOLD_LINE);	break;
		case 1:		cpunum_set_input_line(machine, 0, 3, HOLD_LINE);	break;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:		cpunum_set_input_line_and_vector(machine, 0, 4, HOLD_LINE, 0x100/4);	break;	// touch screen controller

		default:	cpunum_set_input_line(machine, 0, 1, HOLD_LINE);	break;
	}
}

static MACHINE_DRIVER_START( tm3k )
	MDRV_CPU_ADD_TAG("main", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(tmaster_map,0)
	MDRV_CPU_VBLANK_INT(tm3k_interrupt,2+5+20) // ??

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_MACHINE_RESET(tmaster)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(400, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 256-1)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(tmaster)
	MDRV_VIDEO_UPDATE(tmaster)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD_TAG("OKI",OKIM6295, 2000000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static INTERRUPT_GEN( tm_interrupt )
{
	switch (cpu_getiloops())
	{
		case 0:		cpunum_set_input_line(machine, 0, 2, HOLD_LINE);	break;
		case 1:		cpunum_set_input_line(machine, 0, 3, HOLD_LINE);	break;
		case 2:		cpunum_set_input_line_and_vector(machine, 0, 4, HOLD_LINE, 0x100/4);	break;	// touch screen controller
		default:	cpunum_set_input_line(machine, 0, 1, HOLD_LINE);	break;
	}
}

static MACHINE_DRIVER_START( tm )
	MDRV_IMPORT_FROM(tm3k)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_VBLANK_INT(tm_interrupt,3+20) // ??

	MDRV_SOUND_REPLACE("OKI",OKIM6295, 1122000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static INTERRUPT_GEN( galgames_interrupt )
{
	switch (cpu_getiloops())
	{
		case 0:		cpunum_set_input_line(machine, 0, 3, HOLD_LINE);	break;
					// lev 2 triggered at the end of a blit
		default:	cpunum_set_input_line(machine, 0, 1, HOLD_LINE);	break;
	}
}

static MACHINE_RESET( galgames )
{
	memory_set_bank(1, 0);	// rom
	memory_set_bank(3, 1);	// ram

	memory_set_bank(2, 0);	// ram
	memory_set_bank(4, 0);	// ram
}

static MACHINE_DRIVER_START( galgames )
	MDRV_CPU_ADD_TAG("main", M68000, XTAL_24MHz / 2)
	MDRV_CPU_PROGRAM_MAP(galgames_map,0)
	MDRV_CPU_VBLANK_INT(galgames_interrupt, 1+20)	// ??

	MDRV_NVRAM_HANDLER( galgames )
	MDRV_MACHINE_RESET( galgames )

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	MDRV_SCREEN_SIZE(400, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 256-1)
	MDRV_PALETTE_LENGTH(0x800)	// only 0x100 used

	MDRV_VIDEO_START(galgames)
	MDRV_VIDEO_UPDATE(tmaster)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, XTAL_24MHz / 8)	// ??
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7low) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

                               ROMs Loading

***************************************************************************/

/***************************************************************************

Touch Master
1996, Midway

68000 @ 12MHz or 6MHz
u51 - u52 program code
u36 -> u39 gfx
u8 sound
OKI6295
NVSRAM DS1225a
Philips SCN68681
Xlinx XC3042a

Dumped by ANY

***************************************************************************/

ROM_START( tm )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tmaster.u51", 0x000000, 0x080000, CRC(edaa5874) SHA1(48b99bc7f5a6453def265967ca7d8eefdf9dc97b) )
	ROM_LOAD16_BYTE( "tmaster.u52", 0x000001, 0x080000, CRC(e9fd30fc) SHA1(d91ea05d5f574603883336729fb9df705688945d) )

	ROM_REGION( 0x400000, REGION_GFX1, ROMREGION_ERASE )	// Blitter gfx
	ROM_LOAD16_BYTE( "tmaster.u38", 0x100000, 0x080000, CRC(68885ef6) SHA1(010602b59c33c3e490491a296ddaf8952e315b83) )
	ROM_LOAD16_BYTE( "tmaster.u36", 0x100001, 0x080000, CRC(204096ec) SHA1(9239923b7eedb6003c63ef2e8ff224edee657bbc) )
	// unused gap
	ROM_LOAD16_BYTE( "tmaster.u39", 0x300000, 0x080000, CRC(cbb716cb) SHA1(4e8d8f6cbfb25a8161ff8fe7505d6b209650dd2b) )
	ROM_LOAD16_BYTE( "tmaster.u37", 0x300001, 0x080000, CRC(e0b6a9f7) SHA1(7e057ca87833c682e5be03668469259bbdefbf20) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) // Samples
	ROM_LOAD( "tmaster.u8", 0x40000, 0x040000, CRC(f39ad4cf) SHA1(9bcb9a5dd3636d6541eeb3e737c7253ab0ed4e8d) )
	ROM_CONTINUE(           0xc0000, 0x040000 )
ROM_END

/***************************************************************************

Touchmaster 3000
by Midway
touchscreen game
Dumped BY: N?Z!

All chips are SGS 27C801
---------------------------

Name_Board Location        Version               Use                      Checksum
-----------------------------------------------------------------------------------
TM3K_u8.bin                5.0  Audio Program & sounds          64d5
TM3K_u51.bin               5.01 Game Program & Cpu instructions 0c6c
TM3K_u52.bin               5.01 Game Program & Cpu instructions b2d8
TM3K_u36.bin               5.0  Video Images & Graphics         54f1
TM3K_u37.bin               5.0  Video Images & Graphics         4856
TM3K_u38.bin               5.0  Video Images & Graphics         5493
TM3K_u39.bin               5.0  Video Images & Graphics         6029
TM3K_u40.bin               5.0  Video Images & Graphics         ccb4
TM3K_u41.bin               5.0  Video Images & Graphics         54a7
u62 (NOT INCLUDED)         N/A  Battery Memory Module           N/A
J12 (NOT INCLUDED)         N/A  Security Key(not required for this Version)
-----------------------------------------------------------------------------------

SCN68681c1n40
xc3042A      www.xilinx.com

***************************************************************************/

ROM_START( tm3k )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm3k_u52.bin", 0x000001, 0x100000, CRC(8c6a0db7) SHA1(6b0eae60ea471cd8c4001749ac2677d8d4532567) )
	ROM_LOAD16_BYTE( "tm3k_u51.bin", 0x000000, 0x100000, CRC(c9522279) SHA1(e613b791f831271722f05b7e96c35519fa9fc174) )

	ROM_REGION( 0x600000, REGION_GFX1, 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm3k_u38.bin", 0x000000, 0x100000, CRC(a6683899) SHA1(d05024390917cdb1871d030996da8e1eb6460918) )
	ROM_LOAD16_BYTE( "tm3k_u36.bin", 0x000001, 0x100000, CRC(7bde520d) SHA1(77750b689e2f0d47804042456e54bbd9c28deeac) )
	ROM_LOAD16_BYTE( "tm3k_u39.bin", 0x200000, 0x100000, CRC(206b56a6) SHA1(09e5e05bffd0a09abd24d668e2c59b56f2c79134) )
	ROM_LOAD16_BYTE( "tm3k_u37.bin", 0x200001, 0x100000, CRC(18f50eb3) SHA1(a7c9d3b24b5fd110380ec87d9200d55cad473efc) )
	ROM_LOAD16_BYTE( "tm3k_u41.bin", 0x400000, 0x100000, BAD_DUMP CRC(74a36bca) SHA1(7ad594daa156dea40a25b390f26c2fd0550e66ff) )
	ROM_LOAD16_BYTE( "tm3k_u40.bin", 0x400001, 0x100000, CRC(353df7ca) SHA1(d6c5d5449af6b6a3acee219778583904c5b554b4) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) // Samples
	ROM_LOAD( "tm3k_u8.bin", 0x00000, 0x100000, CRC(d0ae33c1) SHA1(a079def9a086a091fcc4493a44fec756d2470415) )
ROM_END

/***************************************************************************

Touchmaster 4000
by Midway
touchscreen game
Dumped BY: N?Z!

All chips are SGS 27C801
---------------------------

Name_Board Location        Version               Use                      Checksum
-----------------------------------------------------------------------------------
TM4K_u8.bin                6.0  Audio Program & sounds          DE0B
TM4K_u51.bin               6.02 Game Program & Cpu instructions FEA0
TM4K_u52.bin               6.02 Game Program & Cpu instructions 9A71
TM4K_u36.bin               6.0  Video Images & Graphics         54f1
TM4K_u37.bin               6.0  Video Images & Graphics         609E
TM4K_u38.bin               6.0  Video Images & Graphics         5493
TM4K_u39.bin               6.0  Video Images & Graphics         CB90
TM4K_u40.bin               6.0  Video Images & Graphics         208A
TM4K_u41.bin               6.0  Video Images & Graphics         385D
u62 (NOT INCLUDED)         N/A  Battery Memory Module           N/A
J12 (NOT INCLUDED)         N/A  Security Key(required for this Version)
-----------------------------------------------------------------------------------

SCN68681c1n40
xc3042A      www.xilinx.com

***************************************************************************/

ROM_START( tm4k )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm4k_u51.bin", 0x000000, 0x100000, CRC(3d8d7848) SHA1(31638f23cdd5e6cfbb2270e953f84fe1bd437950) )
	ROM_LOAD16_BYTE( "tm4k_u52.bin", 0x000001, 0x100000, CRC(6d412871) SHA1(ae27c7723b292daf6682c53bafac22e4a3cd1ece) )

	ROM_REGION( 0x600000, REGION_GFX1, 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm4k_u38.bin", 0x000000, 0x100000, CRC(a6683899) SHA1(d05024390917cdb1871d030996da8e1eb6460918) )
	ROM_LOAD16_BYTE( "tm4k_u36.bin", 0x000001, 0x100000, CRC(7bde520d) SHA1(77750b689e2f0d47804042456e54bbd9c28deeac) )
	ROM_LOAD16_BYTE( "tm4k_u39.bin", 0x200000, 0x100000, CRC(bac88cfb) SHA1(26ed169296b890c5f5b50c418c15299355a6592f) )
	ROM_LOAD16_BYTE( "tm4k_u37.bin", 0x200001, 0x100000, CRC(bf49fafa) SHA1(b400667bf654dc9cd01a85c8b99670459400fd60) )
	ROM_LOAD16_BYTE( "tm4k_u41.bin", 0x400000, 0x100000, CRC(e97edb1e) SHA1(75510676cf1692ad03efd4ccd57d25af1cc8ef2a) )
	ROM_LOAD16_BYTE( "tm4k_u40.bin", 0x400001, 0x100000, CRC(f6771a09) SHA1(74f71d5e910006c83a38170f24aa811c38a3e020) )

	ROM_REGION( 0x100000, REGION_SOUND1, 0 ) // Samples
	ROM_LOAD( "tm4k_u8.bin", 0x00000, 0x100000, CRC(48c3782b) SHA1(bfe105ddbde8bbbd84665dfdd565d6d41926834a) )
ROM_END

/***************************************************************************

Galaxy Games BIOS v. 1.90

This is a multi-game cocktail cabinet released in 1998.  Namco seems to have
made some cartridges for it (or at least licensed their IP).

Trackball-based. 'BIOS' has 7 built-in games. There are two LEDs on the PCB.

More information here : http://www.cesgames.com/museum/galaxy/index.html

----

Board silkscreend  237-0211-00
                   REV.-D

Cartridge based mother board
Holds up to 4 cartridges
Chips labeled 
    GALAXY U1 V1.90 12/1/98
    GALAXY U2 V1.90 12/1/98

NAMCO 307 Cartridge has surface mount Flash chips in it (not dumped).

Motorola MC68HC000FN12
24 MHz oscillator
Xilinx XC5206
Xilinx XC5202
BT481AKPJ110 (Palette RAMDAC)
NKK N341024SJ-15	x8  (128kB RAM)
OKI M6295 8092352-2

PAL16V8H-15 @ U24	Blue dot on it
PAL16V8H-15 @ U25	Yellow dot on it
PAL16V8H-15 @ U26	Red dot on it
PAL16V8H-15 @ U27	Green dot on it
PAL16V8H-15 @ U45	red dot on it

***************************************************************************/

ROM_START( galgbios )
	ROM_REGION( 0x200000 + 0x40000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "galaxy.u2", 0x1c0000, 0x020000, CRC(e51ff184) SHA1(aaa795f2c15ec29b3ceeb5c917b643db0dbb7083) )
	ROM_CONTINUE(                 0x000000, 0x0e0000 )
	ROM_LOAD16_BYTE( "galaxy.u1", 0x1c0001, 0x020000, CRC(c6d7bc6d) SHA1(93c032f9aa38cbbdda59a8a25ff9f38f7ad9c760) )
	ROM_CONTINUE(                 0x000001, 0x0e0000 )
	ROM_FILL(                     0x200000, 0x040000, 0)	// RAM

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD16_BYTE( "galaxy.u2", 0x000000, 0x100000, CRC(e51ff184) SHA1(aaa795f2c15ec29b3ceeb5c917b643db0dbb7083) )
	ROM_LOAD16_BYTE( "galaxy.u1", 0x000001, 0x100000, CRC(c6d7bc6d) SHA1(93c032f9aa38cbbdda59a8a25ff9f38f7ad9c760) )

	ROM_REGION( 0x40000, REGION_SOUND1, ROMREGION_ERASE )
	// RAM, filled by the 68000 and fed to the OKI
ROM_END

static DRIVER_INIT( tm3k )
{
	// try this if you need to calibrate
#if 0
	UINT16 *ROM = (UINT16 *)memory_region( REGION_CPU1 );
	// tscreen test
	ROM[0x75e3c/2] = 0x4ef9;
	ROM[0x75e3e/2] = 0x0007;
	ROM[0x75e40/2] = 0x5e4a;

	// tscreen test
	ROM[0x765ca/2] = 0x7001;
#endif
}

static DRIVER_INIT( tm4k )
{
	UINT16 *ROM = (UINT16 *)memory_region( REGION_CPU1 );

	// protection
	ROM[0x83476/2] = 0x4e75;

	ROM[0x8342C/2] = 0x601a;
	ROM[0x8346C/2] = 0x6002;
}

static DRIVER_INIT( galgames )
{
	// configure memory banks
	memory_configure_bank(1, 0, 2, memory_region(REGION_CPU1)+0x1c0000, 0x40000);
	memory_configure_bank(3, 0, 2, memory_region(REGION_CPU1)+0x1c0000, 0x40000);

	memory_configure_bank(2, 0, 1, memory_region(REGION_CPU1)+0x200000, 0x40000);
	memory_configure_bank(4, 0, 1, memory_region(REGION_CPU1)+0x200000, 0x40000);
}

GAME( 1996, tm,       0, tm,       tmaster,  0,        ROT0, "Midway",                         "Touchmaster",               GAME_NOT_WORKING )
GAME( 1997, tm3k,     0, tm3k,     tmaster,  tm3k,     ROT0, "Midway",                         "Touchmaster 3000 (v5.01)",  GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS)	// imp. graphics due to bad dump
GAME( 1998, tm4k,     0, tm3k,     tmaster,  tm4k,     ROT0, "Midway",                         "Touchmaster 4000 (v6.02)",  GAME_NOT_WORKING )
GAME( 1998, galgbios, 0, galgames, galgames, galgames, ROT0, "Creative Electonics & Software", "Galaxy Games (BIOS v1.90)", GAME_IS_BIOS_ROOT )
