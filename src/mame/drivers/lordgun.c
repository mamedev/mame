/***************************************************************************

                      -= IGS Lord Of Gun =-

                    driver by   Luca Elia (l.elia@tin.it)
           skeleton driver by   David Haywood
            code decrypted by   unknown


CPU     :   68000
Custom  :   IGS005, IGS006, IGS007, IGS008
Sound   :   Z80 + M6295 [+ M6295] + YM3812
NVRAM   :   93C46

-----------------------------------------------------------------------------------
Year + Game           PCB    FM Sound  Chips                         Notes
-----------------------------------------------------------------------------------
1994  Lord Of Gun     T0076  YM3812    IGS005? IGS006 IGS007 IGS008  Lightguns
199?  Huang Fei Hong  ?      ?         ?                             Not encrypted
-----------------------------------------------------------------------------------

To do:

- Protection emulation instead of patching the roms
- Priorities

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/8255ppi.h"
#include "machine/eeprom.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "lordgun.h"


static UINT16 *lordgun_priority_ram, lordgun_priority;


/***************************************************************************

    Code Decryption

***************************************************************************/

static DRIVER_INIT( lordgun )
{
	int i;
	UINT16 *src = (UINT16 *)memory_region(machine, "maincpu");

	int rom_size = 0x100000;

	for(i=0; i<rom_size/2; i++) {
		UINT16 x = src[i];

		if((i & 0x0120) == 0x0100 || (i & 0x0a00) == 0x0800)
			x ^= 0x0010;

		src[i] = x;
	}

	// protection
	src[0x14832/2]	=	0x6000;		// 014832: 6700 0006  beq     $1483a (protection)
	src[0x1587e/2]	=	0x6010;		// 01587E: 6710       beq     $15890 (rom check)
}

/***************************************************************************

    Protection

***************************************************************************/

// to be done

/***************************************************************************

    Memory Maps - Main

***************************************************************************/

static WRITE8_DEVICE_HANDLER(fake_w)
{
}
static WRITE8_DEVICE_HANDLER(fake2_w)
{
//  popmessage("%02x",data);
}

static WRITE8_DEVICE_HANDLER( lordgun_eeprom_w )
{
	static UINT8 old;
	int i;

	if (data & ~0xfd)
	{
//      popmessage("EE: %02x", data);
		logerror("%s - Unknown EEPROM bit written %02X\n",cpuexec_describe_context(device->machine),data);
	}

	coin_counter_w(device->machine, 0, data & 0x01);

	// Update light guns positions
	for (i = 0; i < 2; i++)
		if ( (data & (0x04 << i)) && !(old & (0x04 << i)) )
			lordgun_update_gun(device->machine, i);

	// latch the bit
	eeprom_write_bit(data & 0x40);

	// reset line asserted: reset.
	eeprom_set_cs_line((data & 0x10) ? CLEAR_LINE : ASSERT_LINE );

	// clock line asserted: write latch or select next bit to read
	eeprom_set_clock_line((data & 0x20) ? ASSERT_LINE : CLEAR_LINE );

	lordgun_whitescreen = data & 0x80;

	old = data;
}

static WRITE16_HANDLER( lordgun_priority_w )
{
	COMBINE_DATA(&lordgun_priority);
//  popmessage("PR: %04x", data);
}

static READ16_DEVICE_HANDLER( lordgun_ppi8255_r )	{	return ppi8255_r(device, offset);	}

static WRITE16_DEVICE_HANDLER( lordgun_ppi8255_w )	{	ppi8255_w(device, offset, data & 0xff);	}

static READ16_HANDLER( lordgun_gun_0_x_r )		{ return lordgun_gun[0].hw_x; }
static READ16_HANDLER( lordgun_gun_0_y_r )		{ return lordgun_gun[0].hw_y; }
static READ16_HANDLER( lordgun_gun_1_x_r )		{ return lordgun_gun[1].hw_x; }
static READ16_HANDLER( lordgun_gun_1_y_r )		{ return lordgun_gun[1].hw_y; }


static WRITE16_HANDLER( lordgun_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)	soundlatch_w (space, 0, (data >> 0) & 0xff);
	if (ACCESSING_BITS_8_15)	soundlatch2_w(space, 0, (data >> 8) & 0xff);

	cputag_set_input_line(space->machine, "soundcpu", INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( lordgun_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM AM_BASE(&lordgun_priority_ram)	// PRIORITY
	AM_RANGE(0x300000, 0x30ffff) AM_RAM_WRITE(lordgun_vram_0_w) AM_BASE(&lordgun_vram_0)	// DISPLAY
	AM_RANGE(0x310000, 0x313fff) AM_RAM_WRITE(lordgun_vram_1_w) AM_BASE(&lordgun_vram_1)	// DISPLAY
	AM_RANGE(0x314000, 0x317fff) AM_RAM_WRITE(lordgun_vram_2_w) AM_BASE(&lordgun_vram_2)	// DISPLAY
	AM_RANGE(0x318000, 0x319fff) AM_RAM_WRITE(lordgun_vram_3_w) AM_BASE(&lordgun_vram_3)	// DISPLAY
	AM_RANGE(0x31c000, 0x31c7ff) AM_RAM AM_BASE(&lordgun_scrollram)		// LINE
	AM_RANGE(0x400000, 0x4007ff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)	// ANIMATOR
	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x502000, 0x502001) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_x_0)
	AM_RANGE(0x502200, 0x502201) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_x_1)
	AM_RANGE(0x502400, 0x502401) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_x_2)
	AM_RANGE(0x502600, 0x502601) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_x_3)
	AM_RANGE(0x502800, 0x502801) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_y_0)
	AM_RANGE(0x502a00, 0x502a01) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_y_1)
	AM_RANGE(0x502c00, 0x502c01) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_y_2)
	AM_RANGE(0x502e00, 0x502e01) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_y_3)
	AM_RANGE(0x503000, 0x503001) AM_WRITE(lordgun_priority_w)
	AM_RANGE(0x503800, 0x503801) AM_READ(lordgun_gun_0_x_r)
	AM_RANGE(0x503a00, 0x503a01) AM_READ(lordgun_gun_1_x_r)
	AM_RANGE(0x503c00, 0x503c01) AM_READ(lordgun_gun_0_y_r)
	AM_RANGE(0x503e00, 0x503e01) AM_READ(lordgun_gun_1_y_r)
	AM_RANGE(0x504000, 0x504001) AM_WRITE(lordgun_soundlatch_w)
	AM_RANGE(0x506000, 0x506007) AM_DEVREADWRITE("ppi8255_0", lordgun_ppi8255_r, lordgun_ppi8255_w)
	AM_RANGE(0x508000, 0x508007) AM_DEVREADWRITE("ppi8255_1", lordgun_ppi8255_r, lordgun_ppi8255_w)
	AM_RANGE(0x50a900, 0x50a9ff) AM_RAM	// protection
ADDRESS_MAP_END


static ADDRESS_MAP_START( hfh_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM AM_BASE(&lordgun_priority_ram)	// PRIORITY
	AM_RANGE(0x300000, 0x30ffff) AM_RAM_WRITE(lordgun_vram_0_w) AM_BASE(&lordgun_vram_0)	// DISPLAY
	AM_RANGE(0x310000, 0x313fff) AM_RAM_WRITE(lordgun_vram_1_w) AM_BASE(&lordgun_vram_1)	// DISPLAY
	AM_RANGE(0x314000, 0x317fff) AM_RAM_WRITE(lordgun_vram_2_w) AM_BASE(&lordgun_vram_2)	// DISPLAY
	AM_RANGE(0x318000, 0x319fff) AM_RAM_WRITE(lordgun_vram_3_w) AM_BASE(&lordgun_vram_3)	// DISPLAY
	AM_RANGE(0x31c000, 0x31c7ff) AM_RAM AM_BASE(&lordgun_scrollram)		// LINE
	AM_RANGE(0x400000, 0x4007ff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)	// ANIMATOR
	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x502000, 0x502001) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_x_0)
	AM_RANGE(0x502200, 0x502201) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_x_1)
	AM_RANGE(0x502400, 0x502401) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_x_2)
	AM_RANGE(0x502600, 0x502601) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_x_3)
	AM_RANGE(0x502800, 0x502801) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_y_0)
	AM_RANGE(0x502a00, 0x502a01) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_y_1)
	AM_RANGE(0x502c00, 0x502c01) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_y_2)
	AM_RANGE(0x502e00, 0x502e01) AM_WRITE(SMH_RAM) AM_BASE(&lordgun_scroll_y_3)
	AM_RANGE(0x503000, 0x503001) AM_WRITE(lordgun_priority_w)
	AM_RANGE(0x504000, 0x504001) AM_WRITE(lordgun_soundlatch_w)
	AM_RANGE(0x506000, 0x506007) AM_DEVREADWRITE("ppi8255_0", lordgun_ppi8255_r, lordgun_ppi8255_w)
	AM_RANGE(0x508000, 0x508007) AM_DEVREADWRITE("ppi8255_1", lordgun_ppi8255_r, lordgun_ppi8255_w)
	AM_RANGE(0x50b900, 0x50b9ff) AM_RAM	// protection
ADDRESS_MAP_END


/***************************************************************************

    Memory Maps - Sound

***************************************************************************/

static ADDRESS_MAP_START( lordgun_soundmem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static WRITE8_DEVICE_HANDLER( lordgun_okibank_w )
{
	okim6295_set_bank_base(device, (data & 2) ? 0x40000 : 0);
	if (data & ~3)	logerror("%s: unknown okibank bits %02x\n", cpuexec_describe_context(device->machine), data);
//  popmessage("OKI %x", data);
}

static ADDRESS_MAP_START( lordgun_soundio_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE( "ymsnd", ym3812_w )
	AM_RANGE(0x2000, 0x2000) AM_DEVREADWRITE( "oki", okim6295_r, okim6295_w )
	AM_RANGE(0x3000, 0x3000) AM_READ( soundlatch2_r )
	AM_RANGE(0x4000, 0x4000) AM_READ( soundlatch_r )
	AM_RANGE(0x5000, 0x5000) AM_READ( SMH_NOP )
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE( "oki", lordgun_okibank_w )
ADDRESS_MAP_END


static ADDRESS_MAP_START( hfh_soundio_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x3000, 0x3000) AM_READ( soundlatch2_r )
	AM_RANGE(0x4000, 0x4000) AM_READ( soundlatch_r )
	AM_RANGE(0x5000, 0x5000) AM_READ( SMH_NOP )
	AM_RANGE(0x7000, 0x7001) AM_DEVWRITE( "ymsnd", ym3812_w )
	AM_RANGE(0x7400, 0x7400) AM_DEVREADWRITE( "oki", okim6295_r, okim6295_w )
	AM_RANGE(0x7800, 0x7800) AM_DEVREADWRITE( "oki2", okim6295_r, okim6295_w )
ADDRESS_MAP_END


/***************************************************************************

    Graphics Layout

***************************************************************************/

static const gfx_layout lordgun_8x8x6_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{	RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	8*8*2
};

static const gfx_layout lordgun_16x16x6_layout =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{	RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1),STEP8(8*16*2,1) },
	{ STEP16(0,8*2) },
	16*16*2
};

static const gfx_layout lordgun_32x32x6_layout =
{
	32,32,
	RGN_FRAC(1,3),
	6,
	{	RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0 },
	{ STEP8(0,1),STEP8(8*32*2,1),STEP8(8*32*2*2,1),STEP8(8*32*2*3,1) },
	{ STEP16(0,8*2),STEP16(16*8*2,8*2) },
	32*32*2
};

static GFXDECODE_START( lordgun )
	GFXDECODE_ENTRY( "gfx1", 0, lordgun_16x16x6_layout,  0x000, 0x400/0x40  )	// [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, lordgun_8x8x6_layout,    0x500, 0x100/0x40  )	// [1] Tilemap 0
	GFXDECODE_ENTRY( "gfx3", 0, lordgun_16x16x6_layout,  0x600, 0x200/0x40  )	// [2] Tilemap 1
	GFXDECODE_ENTRY( "gfx3", 0, lordgun_32x32x6_layout,  0x700, 0x100/0x40  )	// [3] Tilemap 2
	GFXDECODE_ENTRY( "gfx2", 0, lordgun_8x8x6_layout,    0x400, 0x400/0x40  )	// [4] Tilemap 3
GFXDECODE_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( lordgun )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "Game Mode" )
	PORT_DIPSETTING(    0x01, "Arcade" )
	PORT_DIPSETTING(    0x00, "Street" )
	PORT_DIPNAME( 0x02, 0x02, "Guns" )
	PORT_DIPSETTING(    0x02, "IGS" )		// x table offset  = 0x25
	PORT_DIPSETTING(    0x00, "Konami" )	// "" = 0x2c
	PORT_DIPNAME( 0x04, 0x04, "Ranking Music" )
	PORT_DIPSETTING(    0x04, "Exciting" )
	PORT_DIPSETTING(    0x00, "Tender" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )
	PORT_DIPSETTING(    0x08, "Separate" )
	PORT_DIPSETTING(    0x00, "Common" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(eeprom_bit_r, NULL)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE2 ) // cheat: skip ahead

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(5)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2   ) PORT_IMPULSE(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,0x1ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0x1ff, 0x100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,0x1ff) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END



/***************************************************************************

    Machine Drivers

***************************************************************************/

static const ppi8255_interface ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("IN0"),		// Port A read
		DEVCB_NULL,						// Port B read
		DEVCB_INPUT_PORT("IN3"),		// Port C read
		DEVCB_HANDLER(fake_w),			// Port A write
		DEVCB_HANDLER(lordgun_eeprom_w),// Port B write
		DEVCB_HANDLER(fake2_w)			// Port C write
	},
	{
		DEVCB_INPUT_PORT("IN1"),		// Port A read
		DEVCB_INPUT_PORT("IN2"),		// Port B read
		DEVCB_INPUT_PORT("IN4"),		// Port C read
		DEVCB_HANDLER(fake_w),			// Port A write
		DEVCB_HANDLER(fake_w),			// Port B write
		DEVCB_HANDLER(fake_w)			// Port C write
	}
};

static void soundirq(const device_config *device, int state)
{
	cputag_set_input_line(device->machine, "soundcpu", 0, state);
}

static const ym3812_interface lordgun_ym3812_interface =
{
	soundirq
};

static MACHINE_DRIVER_START( lordgun )
	MDRV_CPU_ADD("maincpu", M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(lordgun_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80, 5000000)
	MDRV_CPU_PROGRAM_MAP(lordgun_soundmem_map)
	MDRV_CPU_IO_MAP(lordgun_soundio_map)

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )

	MDRV_NVRAM_HANDLER(93C46)

	MDRV_GFXDECODE(lordgun)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x200, 0x100)
	MDRV_SCREEN_VISIBLE_AREA(0,0x1c0-1, 0,0xe0-1)

	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(lordgun)
	MDRV_VIDEO_UPDATE(lordgun)

	// sound hardware
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM3812, 3579545)
	MDRV_SOUND_CONFIG(lordgun_ym3812_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_SOUND_ADD("oki", OKIM6295, 1000000)	// 5MHz can't be right!
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( hfh )
	MDRV_IMPORT_FROM(lordgun)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(hfh_map)

	MDRV_CPU_MODIFY("soundcpu")
	MDRV_CPU_IO_MAP(hfh_soundio_map)

	// sound hardware
	MDRV_SOUND_ADD("oki2", OKIM6295, 1000000)	// 5MHz can't be right!
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

    ROMs Loading

***************************************************************************/

/*
Lord of Gun
IGS, 1994

PCB Layout
----------

IGSPCB NO. T0076
--------------------------------------------------------
| YM3014           62256      IGS008  IGS006   IGST003 |
| YM3812      6295 62256                       IGST002 |
|       3.57945MHz 62256                       IGST001 |
|                  62256                               |
|6116 LORDGUN.100                              IGSB003 |
|     Z80               62256                  IGSB002 |
|LORDGUN.90                                    IGSB001 |
|J    PAL              6116                            |
|A    PAL              6116                       6116 |
|M                          IGS003                6116 |
|M   68000P10 PAL                                 6116 |
|A                          PAL     PAL           6116 |
|                           PAL     6116               |
|                           PAL     6116        IGS007 |
|                           PAL     6116         20MHz |
|       DSW1(4)                     6116 PAL           |
|             62256    62256          IGSA001 IGSA004  |
|      8255          LORDGUN.10       IGSA002 IGSA005  |
|93C46 8255          LORDGUN.4        IGSA003 IGSA006  |
--------------------------------------------------------

HW Notes:
      68k clock: 10.000MHz
      Z80 clock: 5.000MHz
          VSync: 60Hz
          HSync: 15.15kHz
   YM3812 clock: 3.57945MHz
 OKI 6295 clock: 5.000MHz
  OKI 6295 pin7: HI

  All frequencies are checked with my frequency counter (i.e. they are not guessed)

  IGST* are 8M devices
  IGSA* and IGSB* are 16M devices
  LORDGUN.90 is 27C512
  LORDGUN.100 \
  LORDGUN.10  | 27C040
  LORDGUN.4   /

-----

Lord of Gun (c) 1994 IGS

PCB: IGSPCB NO.T0076

  Main: MC68000P10 10MHz
   Sub: Zilog Z0840006PCS (Z80 6MHz)
 Sound: OKI M6295, Yamaha YM3812-F + Y3014B-F
   OSC: 20.000 MHz, Unmarked OSC for sound chips
EEPROM: NMC 9346N

1 Push Button - Test/Setup Mode

Custom chips:
IGS 005 (144 Pin PQFP)
IGS 006 (144 Pin PQFP)
IGS 007 (144 Pin PQFP)
IGS 008 (160 Pin PQFP)

lg_u122.m3 - Labelled as "LORD GUN U122-M3" MX 27C4000
lg_u144.m3 - Labelled as "LORD GUN U144-M3" MX 27C4000

lordgun.u90  - Labelled as "LORD GUN U90"  27C512
lordgunu.100 - Labelled as "LORD GUN U100" MX 27C4000

Surface mounted ROMs (42 pin DIP)

2 Unmarked ROM(?) chips

IGS A001
IGS A002
IGS A003
IGS A004
IGS A005
IGS A006

IGS B001
IGS B002
IGS B003

IGS T001
IGS T002
IGS T003

DIP Switch-1 (4 Position DIP)
--------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 |
--------------------------------------------------
       Game Mode      |  Arcade  |off|           |
                      |  Street  |on |           |
--------------------------------------------------
       Selection      |   IGS    |   |off|       |
        of Guns       |  Konami  |   |on |       |
--------------------------------------------------
       Ranking        | Exciting |       |off|   |
      Background      |  Tender  |       |on |   |
--------------------------------------------------
      Coin Slots      | Seperate |           |off|
                      |  Common  |           |on |
--------------------------------------------------
     Settings Upon Shipping      |off|off|off|off|
--------------------------------------------------

Game modes explained:
 In "Arcade Mode" players could play this game by entering each scene in a
  pre-defined order.
 In "Street Mode" this game now presents 10 selectable scenes for players,
  not 4 any more.  After all scenes are passed (except training courses),
  players can enter the last scene; the Headquarters

                       Lord of Gun JAMMA Pinout

                          Main Jamma Connector
            Solder Side            |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
                             | E | 5 |
             +12             | F | 6 |             +12
------------ KEY ------------| H | 7 |------------ KEY -----------
                             | J | 8 |       Coin Counter
                             | K | 9 |
        Speaker (-)          | L | 10|        Speaker (+)
                             | M | 11|
        Video Green          | N | 12|        Video Red
        Video Sync           | P | 13|        Video Blue
                             | R | 14|        Video GND
                             | S | 15|        Test Switch
        Coin Switch 2        | T | 16|        Coin Switch 1
        Start Player 2       | U | 17|        Start Player 1
                             | V | 18|
                             | W | 19|
                             | X | 20|
                             | Y | 21|
                             | Z | 22|
                             | a | 23|
                             | b | 24|
                             | c | 25|
                             | d | 26|
             GND             | e | 27|             GND
             GND             | f | 28|             GND


NOTE: Speakers should be connected serially to Speaker (+) and Speaker (-).
      You must avoid connecting speakers parallelly or connecting speakers
      to Speaker (+) and GND, to keep the amplifier from being damaged or
      from malfunctioning.

 JP1: Player 1 Gun Connector Pinout

   1| +5 Volts - RED Wire    (Manual says "VCC")
   2| Trigger  - White Wire
   3| Ground   - Black Wire
   4| Gun OPTO - Blue Wire   (Manual says "HIT")

 JP2: Player 2 Gun Connector Pinout

   1| +5 Volts - RED Wire    (Manual says "VCC")
   2| Trigger  - White Wire
   3| Ground   - Black Wire
   4| Gun OPTO - Blue Wire   (Manual says "HIT")
*/

ROM_START( lordgun )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "lordgun.10", 0x00000, 0x80000, CRC(acda77ef) SHA1(7cd8580419e2f62a3b5a1e4a6020a3ef978ff1e8) )
	ROM_LOAD16_BYTE( "lordgun.4",  0x00001, 0x80000, CRC(a1a61254) SHA1(b0c5aa656024cfb9be28a11061656159e7b72d00) )

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "lordgun.90", 0x00000, 0x10000, CRC(d59b5e28) SHA1(36696058684d69306f463ed543c8b0195bafa21e) )	// 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0xc00000, "gfx1", 0 )	// Sprites
	ROM_LOAD( "igsa001.14", 0x000000, 0x200000, CRC(400abe33) SHA1(20de1eb626424ea41bd55eb3cecd6b50be744ee0) )
	ROM_LOAD( "igsa004.13", 0x200000, 0x200000, CRC(52687264) SHA1(28444cf6b5662054e283992857e0827a2ca15b83) )
	ROM_LOAD( "igsa002.9",  0x400000, 0x200000, CRC(a4810e38) SHA1(c31fe641feab2c93795fc35bf71d4f37af1056d4) )
	ROM_LOAD( "igsa005.8",  0x600000, 0x200000, CRC(e32e79e3) SHA1(419f9b501e5a37d763ece9322271e61035b50217) )
	ROM_LOAD( "igsa003.3",  0x800000, 0x200000, CRC(649e48d9) SHA1(ce346154024cf13f3e40000ceeb4c2003cd35894) )
	ROM_LOAD( "igsa006.2",  0xa00000, 0x200000, CRC(39288eb6) SHA1(54d157f0e151f6665f4288b4d09bd65571005132) )

	ROM_REGION( 0x300000, "gfx2", 0 )	// Tilemaps 0 & 3
	ROM_LOAD( "igst001.108", 0x000000, 0x100000, CRC(36dd96f3) SHA1(4e70eb807160e7ed1b19d7f38df3a38021f42d9b) )
	ROM_LOAD( "igst002.114", 0x100000, 0x100000, CRC(816a7665) SHA1(f2f2624ab262c957f84c657cfc432d14c61b19e8) )
	ROM_LOAD( "igst003.119", 0x200000, 0x100000, CRC(cbfee543) SHA1(6fad8ef8d683f709f6ff2b16319447516c372fc8) )

	ROM_REGION( 0x600000, "gfx3", 0 )	// Tilemaps 1 & 2
	ROM_LOAD( "igsb001.82", 0x000000, 0x200000, CRC(3096de1c) SHA1(d010990d21cfda9cb8ab5b4bc0e329c23b7719f5) )
	ROM_LOAD( "igsb002.91", 0x200000, 0x200000, CRC(2234531e) SHA1(58a82e31a1c0c1a4dd026576319f4e7ecffd140e) )
	ROM_LOAD( "igsb003.97", 0x400000, 0x200000, CRC(6cbf21ac) SHA1(ad25090a00f291aa48929ffa01347cc53e0051f8) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples
	ROM_LOAD( "lordgun.100", 0x00000, 0x80000, CRC(b4e0fa07) SHA1(f5f33fe3f3a124f4737751fda3ea409fceeec0be) )
ROM_END



/*

    Huang Fei Hong (Alien Challenge?)

*/

ROM_START( hfh )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "hfh_p.u80", 0x00000, 0x80000, CRC(5175ebdc) SHA1(4a0bdda0f8291f895f888bfd45328b2b124b9051) )
	ROM_LOAD16_BYTE( "hfh_p.u79", 0x00001, 0x80000, CRC(42ad978c) SHA1(eccb96e7170902b37989c8f207e1a821f29b2475) )

	ROM_REGION( 0x010000, "soundcpu", 0 ) // Z80
	ROM_LOAD( "hfh_s.u86", 0x00000, 0x10000, CRC(5728a9ed) SHA1(e5a9e4a1a2cc6c848b08608bc8727bc739270873) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "hfh_g1", 0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "hfh_g2", 0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "hfh_g3", 0x000000, 0x100000, NO_DUMP )

	ROM_REGION( 0x40000, "oki", 0 ) // Samples
	ROM_LOAD( "hfh_g.u65", 0x00000, 0x40000, CRC(ec469b57) SHA1(ba1668078987ad51f47bcd3e61c51a0cf2545350) )

	ROM_REGION( 0x40000, "oki2", 0 ) // Samples
	ROM_LOAD( "hfh_g.u66", 0x00000, 0x40000, CRC(7cfcd98e) SHA1(3b03123160adfd3404a9e0c4c68420930e80ae48) )
ROM_END

/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1994, lordgun, 0, lordgun, lordgun, lordgun, ROT0, "IGS", "Lord of Gun (USA)",                 GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 199?, hfh,     0, hfh,     lordgun, 0,       ROT0, "IGS", "Huang Fei Hong (Alien Challenge?)", GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
