/*************************************************************************************************************

                                              -= Sigma B-98 Hardware =-

                                                 driver by Luca Elia

CPU     :   Z80
Custom  :   TAXAN KY-3211, TAXAN KY-80 (Yamaha)
Sound   :   YMZ280B
NVRAM   :   93C46, Battery

Graphics are made of sprites only.
Each sprite is composed of X x Y tiles and can be zoomed / shrunk.
Tiles can be 16x16x4 or 16x16x8.

To Do:

- Remove ROM patches from gegege
- gegege checks the EEPROM output after reset, and wants a timed 0->1 transition or locks up while
  saving setting in service mode. Using a reset_delay of 7 works, unless when "play style" is set
  to "coin" (it probably changes the number of reads from port $C0).
  I guess the reset_delay mechanism should be implemented with a timer in eeprom.c.

*************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ymz280b.h"
#include "machine/eeprom.h"
#include "machine/ticket.h"

/***************************************************************************

    Video

***************************************************************************/

/***************************************************************************

    Sprites

    Offset:     Bits:         Value:

    0           7654 ----
                ---- 3210     Color
    1           7--- ----
                -6-- ----     256 Color Sprite
                --5- ----
                ---4 ----     ?
                ---- 3---
                ---- -2--     Draw Sprite
                ---- --10
    2                         Tile Code (High)
    3                         Tile Code (Low)
    4           7654 3---     Number of X Tiles - 1
                ---- -210     X (High)
    5                         X (Low)
    6           7654 3---     Number of Y Tiles - 1
                ---- -210     Y (High)
    7                         Y (Low)
    8                         Zoom Factor (<< 8, High)
    9                         Zoom Factor (<< 8, Low)
    a
    b
    c           7654 3---
                ---- -210     Delta X (High)
    d                         Delta X (Low)
    e           7654 3---
                ---- -210     Delta Y (High)
    f                         Delta Y (Low)

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *end		=	machine->generic.spriteram.u8 - 0x10;
	UINT8 *s		=	end + machine->generic.spriteram_size;

	for ( ; s != end; s -= 0x10 )
	{
		int gfx, code, color, zoom, dim, scale;
		int sx, nx, x, x0, x1, dx, flipx;
		int sy, ny, y, y0, y1, dy, flipy;

		if ( (s[ 0x01 ] & 0x04) == 0)
			continue;

		color	=	s[ 0x00 ] & 0xf;

		gfx		=	(s[ 0x01 ] & 0x40 ) ? 1 : 0;

		code	=	s[ 0x02 ] * 256 + s[ 0x03 ];

		nx		=	((s[ 0x04 ] & 0xf8) >> 3) + 1;

		sx		=	(s[ 0x04 ] & 0x03) * 256 + s[ 0x05 ];

		ny		=	((s[ 0x06 ] & 0xf8) >> 3) + 1;

		sy		=	(s[ 0x06 ] & 0x03) * 256 + s[ 0x07 ];

		zoom	=	(s[ 0x08 ] & 0xff) * 256 + s[ 0x09 ];

		dx		=	(s[ 0x0c ] & 0x03) * 256 + s[ 0x0d ];
		dy		=	(s[ 0x0e ] & 0x03) * 256 + s[ 0x0f ];

		// Sign extend the position
		sx		=	(sx & 0x1ff) - (sx & 0x200);
		sy		=	(sy & 0x1ff) - (sy & 0x200);
		dx		=	(dx & 0x1ff) - (dx & 0x200);
		dy		=	(dy & 0x1ff) - (dy & 0x200);

		// Add shift
		sx		+=	dx;
		sy		+=	dy;

		// Use fixed point values (16.16), for accuracy
		sx		<<=	16;
		sy		<<=	16;

		dim		=	(0x10 << 8) * zoom;
		scale	=	dim / 0x10;

		// Let's approximate to the nearest greater integer value
        // to avoid holes in between tiles
		if (scale & 0xffff)	scale += (1<<16) / 0x10;

		flipx	=	0;	// ?
		flipy	=	0;	// ?

		if ( flipx )	{	x0 = nx - 1;	x1 = -1;	dx = -1;	}
		else			{	x0 = 0;			x1 = nx;	dx = +1;	}

		if ( flipy )	{	y0 = ny - 1;	y1 = -1;	dy = -1;	}
		else			{	y0 = 0;			y1 = ny;	dy = +1;	}

		for (y = y0; y != y1; y += dy)
		{
			for (x = x0; x != x1; x += dx)
			{
				drawgfxzoom_transpen(	bitmap,	cliprect, machine->gfx[gfx],
										code++, color,
										flipx, flipy,
										(sx + x * dim) / 0x10000, (sy + y * dim) / 0x10000,
										scale, scale, 0	);
			}
		}
	}
}

static VIDEO_UPDATE(sigmab98)
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}


/***************************************************************************

    Memory Maps

***************************************************************************/

static UINT8 reg, rombank;
static UINT8 reg2, rambank;

// rombank
static WRITE8_HANDLER( regs_w )
{
	if (offset == 0)
	{
		reg = data;
		return;
	}

	switch ( reg )
	{
		case 0x1f:
			rombank = data;
			if (data >= 0x18)
				logerror("%s: unknown rom bank = %02x\n", cpuexec_describe_context(space->machine), data);
			else
				memory_set_bank(space->machine, "rombank", data);
			break;

		default:
			logerror("%s: unknown reg written: %02x = %02x\n", cpuexec_describe_context(space->machine), reg, data);
	}
}
static READ8_HANDLER( regs_r )
{
	if (offset == 0)
		return reg;

	switch ( reg )
	{
		case 0x1f:
			return rombank;

		default:
			logerror("%s: unknown reg read: %02x\n", cpuexec_describe_context(space->machine), reg);
			return 0x00;
	}
}

// rambank
static WRITE8_HANDLER( regs2_w )
{
	if (offset == 0)
	{
		reg2 = data;
		return;
	}

	switch ( reg2 )
	{
		case 0xb5:
			rambank = data;
			switch (data)
			{
				case 0x32:
					memory_set_bank(space->machine, "rambank", 0);
					break;
				case 0x36:
					memory_set_bank(space->machine, "rambank", 1);
					break;
				default:
					logerror("%s: unknown ram bank = %02x\n", cpuexec_describe_context(space->machine), data);
			}
			break;

		default:
			logerror("%s: unknown reg2 written: %02x = %02x\n", cpuexec_describe_context(space->machine), reg2, data);
	}
}
static READ8_HANDLER( regs2_r )
{
	if (offset == 0)
		return reg2;

	switch ( reg2 )
	{
		case 0xb5:
			return rambank;

		default:
			logerror("%s: unknown reg2 read: %02x\n", cpuexec_describe_context(space->machine), reg2);
			return 0x00;
	}
}


// Outputs

static UINT8 c0,c4,c6,c8;
static void show_outputs()
{
#ifdef MAME_DEBUG
	popmessage("0: %02X  4: %02X  6: %02X  8: %02X",c0,c4,c6,c8);
#endif
}

// Port c0
static WRITE8_DEVICE_HANDLER( eeprom_w )
{
	// latch the bit
	eeprom_write_bit(device, data & 0x40);

	// reset line asserted: reset.
//  if ((c0 ^ data) & 0x20)
		eeprom_set_cs_line(device, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);

	// clock line asserted: write latch or select next bit to read
	eeprom_set_clock_line(device, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	c0 = data;
	show_outputs();
}

// Port c4
// 10 led?
static WRITE8_HANDLER( c4_w )
{
	set_led_status(space->machine, 0, (data & 0x10));

	c4 = data;
	show_outputs();
}

// Port c6
// 03 lockout (active low, 02 is cleared when reaching 99 credits)
// 04 pulsed on coin in
// 08 always blinks
// 10 led?
// 20 blinks after coin up
static WRITE8_HANDLER( c6_w )
{
	coin_lockout_w(space->machine, 0, (~data) & 0x02);

	coin_counter_w(space->machine, 0,   data  & 0x04);

	set_led_status(space->machine, 1,   data  & 0x08);
	set_led_status(space->machine, 2,   data  & 0x10);
	set_led_status(space->machine, 3,   data  & 0x20);	//

	c6 = data;
	show_outputs();
}

// Port c8
// 01 hopper enable?
// 02 hopper motor on (active low)?
static WRITE8_HANDLER( c8_w )
{
	ticket_dispenser_w(space->machine->device("hopper"), 0, (!(data & 0x02) && (data & 0x01)) ? 0x00 : 0x80);

	c8 = data;
	show_outputs();
}

static ADDRESS_MAP_START( gegege_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x7fff ) AM_ROM
	AM_RANGE( 0x8000, 0x9fff ) AM_ROMBANK("rombank")

	AM_RANGE( 0xa000, 0xafff ) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)

	AM_RANGE( 0xc000, 0xc1ff ) AM_RAM_WRITE(paletteram_xRRRRRGGGGGBBBBB_be_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE( 0xc800, 0xc87f ) AM_RAM

//  AM_RANGE( 0xd001, 0xd021 ) AM_RAM
	AM_RANGE( 0xd800, 0xdfff ) AM_RAMBANK("rambank")

	AM_RANGE( 0xe000, 0xefff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)	// battery

	AM_RANGE( 0xf000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( gegege_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)

	AM_RANGE( 0x00, 0x01 ) AM_DEVWRITE( "ymz", ymz280b_w )

	AM_RANGE( 0xa0, 0xa1 ) AM_READWRITE( regs_r,  regs_w )
//  AM_RANGE( 0xa2, 0xa3 )
	AM_RANGE( 0xa4, 0xa5 ) AM_READWRITE( regs2_r, regs2_w )

	AM_RANGE( 0xc0, 0xc0 ) AM_READ_PORT( "EEPROM" )
	AM_RANGE( 0xc0, 0xc0 ) AM_DEVWRITE("eeprom", eeprom_w)

	AM_RANGE( 0xc2, 0xc2 ) AM_READ_PORT( "IN1" )

	AM_RANGE( 0xc4, 0xc4 ) AM_READ_PORT( "IN2" )
	AM_RANGE( 0xc4, 0xc4 ) AM_WRITE( c4_w )

	AM_RANGE( 0xc6, 0xc6 ) AM_WRITE( c6_w )

	AM_RANGE( 0xc8, 0xc8 ) AM_WRITE( c8_w )

	AM_RANGE( 0xe5, 0xe5 ) AM_READNOP	// during irq
ADDRESS_MAP_END


/***************************************************************************

    Graphics Layout

***************************************************************************/

static const gfx_layout sigmab98_16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 4*1,4*0, 4*3,4*2, 4*5,4*4, 4*7,4*6, 4*9,4*8, 4*11,4*10, 4*13,4*12, 4*15,4*14 },
	{ STEP16(0,16*4) },
	16*16*4
};

static const gfx_layout sigmab98_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};

static GFXDECODE_START( sigmab98 )
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x4_layout, 0, 0x100/16  )
	GFXDECODE_ENTRY( "sprites", 0, sigmab98_16x16x8_layout, 0, 0x100/256  )
GFXDECODE_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( gegege )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )	// protection? checks. Must be 0
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL )	// protection? checks. Must be 0
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2   ) PORT_IMPULSE(5)	// ? (coin error, pulses mask 4 of port c6)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN1   ) PORT_IMPULSE(5)	// coin/medal in (coin error)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("hopper", ticket_dispenser_line_r)
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_NAME("Bet")	// bet / select in test menu
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME("Play")	// play game / select in test menu
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_NAME("Pay Out")	// pay out / change option in test menu
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

const eeprom_interface eeprom_intf =
{
	6,				// address bits 6
	16,				// data bits    16
	"*110",			// read         1 10 aaaaaa
	"*101",			// write        1 01 aaaaaa dddddddddddddddd
	"*111",			// erase        1 11 aaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	0,				// enable_multi_read
	7				// reset_delay (otherwise gegege will hang when saving settings)
};

static INTERRUPT_GEN( gegege_vblank_interrupt )
{
	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x5a);
}

static MACHINE_DRIVER_START( gegege )
	MDRV_CPU_ADD("maincpu", Z80, XTAL_27MHz / 4)	// ?
	MDRV_CPU_PROGRAM_MAP(gegege_mem_map)
	MDRV_CPU_IO_MAP(gegege_io_map)
	MDRV_CPU_VBLANK_INT("screen", gegege_vblank_interrupt)

	MDRV_NVRAM_HANDLER(generic_0fill)
	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	MDRV_TICKET_DISPENSER_ADD("hopper", 200, TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)					// ?
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x200, 0x200)
	MDRV_SCREEN_VISIBLE_AREA(0,0x140-1, 0,0xf0-1)

	MDRV_GFXDECODE(sigmab98)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_UPDATE(sigmab98)

	// sound hardware
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_27MHz / 2)	// ?
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

  GeGeGe no Kitarou Youkai Slot

  (C) 1997 Banpresto, Sigma

  PCB B-98-1 / 970703 (c) 1997 Sigma:

  XTAL 27MHz

  Battery

  93C46 EEPROM

  YMZ280B

  TAXAN Japan
  KY-3211
  9722 AZGC
  QFP(PULL), ASIC for TFT-LCD

  TAXAN
  KY-80
  YAMAHA
  9650 AZGC
  QFP(PULL) 50, Video IC for LCD

***************************************************************************/

ROM_START( gegege )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b9804-1.ic7", 0x00000, 0x20000, CRC(f8b4f855) SHA1(598bd9f91123e9ab539ce3f33779bff2d072e731) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "b9804-2.ic12", 0x00000, 0x80000, CRC(4211079d) SHA1(d601c623fb909f1346fd02b8fb37b67956e2cd4e) )
	ROM_LOAD( "b9804-3.ic13", 0x80000, 0x80000, CRC(54aeb2aa) SHA1(ccf939111f6288a889846d51bab47ff4e992c542) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "b9804-5.ic16", 0x00000, 0x80000, CRC(ddd7984c) SHA1(3558c495776671ffd3cd5c665b87827b3959b360) )
ROM_END

static DRIVER_INIT( gegege )
{
	UINT8 *rom = memory_region(machine, "maincpu");

	// Protection?
	rom[0xbd3] = 0x18;
	rom[0xbd4] = 0x14;

	rom[0xbef] = 0x18;
	rom[0xbf0] = 0x14;

	rom[0xdec] = 0x00;
	rom[0xded] = 0x00;

	// EEPROM timing checks
	rom[0x8138] = 0x00;
	rom[0x8139] = 0x00;

	rom[0x8164] = 0x00;
	rom[0x8165] = 0x00;

	// ROM banks
	memory_configure_bank(machine, "rombank", 0, 0x18, rom + 0x8000, 0x1000);
	memory_set_bank(machine, "rombank", 0);

	// RAM banks
	UINT8 *bankedram = auto_alloc_array(machine, UINT8, 0x800 * 2);

	memory_configure_bank(machine, "rambank", 0, 2, bankedram, 0x800);
	memory_set_bank(machine, "rambank", 0);
}


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1997, gegege, 0, gegege, gegege, gegege, ROT0, "Banpresto / Sigma", "GeGeGe no Kitarou Youkai Slot", 0 )
