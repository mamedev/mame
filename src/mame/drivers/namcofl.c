/*
    Namco System FL
    Driver by R. Belmont and ElSemi

    IMPORTANT
    ---------
    To calibrate the steering, do the following:
    1) Delete the .nv file
    2) Start the game.  Speed Racer will guide you through the calibration
       (the "jump button" is the same as player 1 start).
       For Final Lap R, hold down service (9) and tap test (F2).  If you do
       not get an "initializing" message followed by the input test, keep doing
       it until you do.
    3) Exit MAME and restart the game, it's now calibrated.


PCB Layout
----------

SYSTEM-FL MAIN PCB 8636960100 (8636970100)
       |--------------------------------------------------------------------------------------------|
       |                                                                       |-----------------|  |
       |                                                                       |       4Z IC3 IC1| Z|
       |                21Y 20Y 19Y 18Y     16Y     14Y 13Y 12Y                |       4Y        | Y|
       |                    20X                                                |       4X        | X|
    |--|                                                                       |          IC4 IC2| W|
    |               22U 21U         18U     16U     14U 13U 12U                |    Sub Board    | U|
    |               22T             18T                                        |-----------------| T|
    |                                                                                               |
    |J          23S                                         12S                        4S    2S    S|
    |                                                                                               |
    |A          23R                                     13R                                        R|
    |                                                                                               |
    |M                  21P 20P 19P 18P                             10P    8P 7P    5P             P|
    |                       20N                                                               OSC1 N|
    |M                                                                                    OSC2      |
    |           23L     21L                             OSC3                              3L       L|
    |A                                              14K         11K                                K|
    |                                                                                               |
    |                           19J 18J 17J 16J                                                    J|
    |--|        23F     21F                                                                        F|
       |                                                                                            |
    |--|        23E                                     13E                                        E|
    |N          23D             19D                     13D                               3D       D|
    |A                                                                                              |
    |M          23C                                                                                C|
    |C  25B             21B                                                                        B|
    |O                                                                                              |
    |4              22A 21A     19A 18A     16A 15A 14A 13A 12A 11A 10A 9A 8A 7A 6A 5A    3A       A|
    |4                                                                                              |
    |--|                                                                                            |
       |25  24  23  22  21  20  19  18  17  16  15  14  13  12  11  10  9  8  7  6  5  4  3  2  1   |
       |--------------------------------------------------------------------------------------------|

RAM
---
4Y : CY7C199-25PC
4Z : CY7C199-25PC
5A : TC514256BZ-60
5P : LH528256N-70LL
6A : TC514256BZ-60
7A : TC514256BZ-60
7P : LH528256N-70LL
8A : TC514256BZ-60
8P : LH528256N-70LL
9A : TC514256BZ-60
10A: TC514256BZ-60
10P: LH528256N-70LL
11A: TC514256BZ-60
12A: TC514256BZ-60
12U: TC511632FL-70
12Y: TC511632FL-70
13U: TC511632FL-70
13Y: TC511632FL-70
14U: TC511632FL-70
14Y: TC511632FL-70
16U: TC511632FL-70
16Y: TC511632FL-70
19Y: M5M5178AFP-25
20Y: M5M5178AFP-25
21A: 65256BLFP-10T
21Y: M5M5178AFP-25
22A: 65256BLFP-10T
22T: LH528256N-70LL
22U: LH528256N-70LL
23E: M5M5256BFP-70LL
23F: M5M5256BFP-70LL

CUSTOM
------
4X : NAMCO C355 (sprite chip)
18Y: NAMCO 156
20X: NAMCO C116
21U: NAMCO 145
18T: NAMCO 123
23R: NAMCO C352
3D : NAMCO C380
4S : NAMCO 187
23L: NAMCO 75 (M37702 MCU)
11K: NAMCO 137
21B: NAMCO C345  9348EV  333791  VY06436A  NX25024K JAPAN
21F: NAMCO 195


ROMs - Main Board
-----------------
23S: MASK ROM - SE1_VOI.23S (PCB LABEL 'VOICE'), mounted on a small plug-in PCB
     labelled MEMEXT 32M MROM PCB 8635909200 (8635909300). This chip is programmed in BYTE mode.
18U: MB834000 MASK ROM - SE1_SSH.18U (PCB LABEL 'SSHAPE')
21P: MB838000 MASK ROM - SE1_SCH0.21P (PCB LABEL 'SCHA0')
20P: MB838000 MASK ROM - SE1_SCH1.20P (PCB LABEL 'SCHA1')
19P: MB838000 MASK ROM - SE1_SCH2.19P (PCB LABEL 'SCHA2')
18P: MB838000 MASK ROM - SE1_SCH3.18P (PCB LABEL 'SCHA3')
21L: M27C4002 EPROM - SE1_SPR.21L (PCB LABEL 'SPROG')
14K: MB834000 MASK ROM - SE1_RSH.14K (PCB LABEL 'RSHAPE')
19J: MB838000 MASK ROM - SE1_RCH0.19J (PCB LABEL 'RCHA0')
18J: MB838000 MASK ROM - SE1_RCH1.18J (PCB LABEL 'RCHA1')
17J, 16J: RCH2, RCH3 but sockets not populated
19A: D27C4096 EPROM - SE2MPEA4.19A (PCB LABEL 'PROGE')
18A: D27C4096 EPROM - SE2MPOA4.18A (PCB LABEL 'PROGO')
16A: AM27C040 EPROM - SE1_DAT3.16A (PCB LABEL 'DATA3')
15A: AM27C040 EPROM - SE1_DAT2.15A (PCB LABEL 'DATA2')
14A: AM27C040 EPROM - SE1_DAT1.14A (PCB LABEL 'DATA1')
13A: AM27C040 EPROM - SE1_DAT0.13A (PCB LABEL 'DATA0')


ROMs - Sub Board
----------------
IC1: MB8316200 SOP44 MASK ROM - SE1OBJ0L.IC1 (PCB LABEL 'OBJ0L')
IC2: MB8316200 SOP44 MASK ROM - SE1OBJ0U.IC2 (PCB LABEL 'OBJ0U')
IC3: MB8316200 SOP44 MASK ROM - SE1OBJ1L.IC3 (PCB LABEL 'OBJ1L')
IC4: MB8316200 SOP44 MASK ROM - SE1OBJ1U.IC4 (PCB LABEL 'OBJ1U')


PALs
----
2S : PAL16L8BCN (PCB LABEL 'SYSFL-1')
3L : PAL16L8BCN (PCB LABEL 'SYSFL-2')
12S: PALCE16V8H (PCB LABEL 'SYSFL-3')
20N: PAL20L8BCN (PCB LABEL 'SYSFL-4')
19D: PALCE16V8H (PCB LABEL 'SYSFL-5')


OTHER
-----
OSC1: 80.000MHz
OSC2: 27.800MHz
OSC3: 48.384MHz
13D : KEYCUS2 (not populated)
3A  : Intel NG80960KA-20 (i960 CPU)
25B : Sharp PC9D10
23C : IR2C24AN
23D : IR2C24AN
13E : HN58C65 EEPROM

*/

#include "driver.h"
#include "deprecat.h"
#include "machine/eeprom.h"
#include "namconb1.h"
#include "namcos2.h"
#include "namcoic.h"
#include "cpu/i960/i960.h"
#include "cpu/m37710/m37710.h"
#include "sound/c352.h"


static emu_timer *raster_interrupt_timer;

VIDEO_START( namcofl );
VIDEO_UPDATE( namcofl );

extern WRITE32_HANDLER(namcofl_spritebank_w);

static UINT32 *namcofl_workram;
static UINT16 *namcofl_shareram;
static UINT8 mcu_port6;


static READ32_HANDLER( fl_unk1_r )
{
	return 0xffffffff;
}

static READ32_HANDLER( fl_network_r )
{
	return 0xffffffff;
}

static READ32_HANDLER( namcofl_sysreg_r )
{
	return 0;
}

static WRITE32_HANDLER( namcofl_sysreg_w )
{
	if ((offset == 2) && ACCESSING_BITS_0_7)  // address space configuration
	{
		if (data == 0)	// RAM at 00000000, ROM at 10000000
		{
			memory_set_bankptr(space->machine,  "bank1", namcofl_workram );
			memory_set_bankptr(space->machine,  "bank2", memory_region(space->machine, "maincpu") );
		}
		else		// ROM at 00000000, RAM at 10000000
		{
			memory_set_bankptr(space->machine,  "bank1", memory_region(space->machine, "maincpu") );
			memory_set_bankptr(space->machine,  "bank2", namcofl_workram );
		}
	}
}

static WRITE32_HANDLER( namcofl_paletteram_w )
{
	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);

	if ((offset == 0x1808/4) && ACCESSING_BITS_16_31)
	{
		UINT16 v = space->machine->generic.paletteram.u32[offset] >> 16;
		UINT16 triggerscanline=(((v>>8)&0xff)|((v&0xff)<<8))-(32+1);

		timer_adjust_oneshot(raster_interrupt_timer, video_screen_get_time_until_pos(space->machine->primary_screen, triggerscanline, 0), 0);
	}
}

static READ32_HANDLER( namcofl_share_r )
{
	return (namcofl_shareram[offset*2+1] << 16) | namcofl_shareram[offset*2];
}

static WRITE32_HANDLER( namcofl_share_w )
{
	COMBINE_DATA(namcofl_shareram+offset*2);
	data >>= 16;
	mem_mask >>= 16;
	COMBINE_DATA(namcofl_shareram+offset*2+1);
}

static ADDRESS_MAP_START( namcofl_mem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_RAMBANK("bank1")
	AM_RANGE(0x10000000, 0x100fffff) AM_RAMBANK("bank2")
	AM_RANGE(0x20000000, 0x201fffff) AM_ROM AM_REGION("user1", 0)	/* data */
	AM_RANGE(0x30000000, 0x30001fff) AM_RAM	AM_BASE_SIZE_GENERIC(nvram) /* nvram */
	AM_RANGE(0x30100000, 0x30100003) AM_WRITE(namcofl_spritebank_w)
	AM_RANGE(0x30284000, 0x3028bfff) AM_READWRITE(namcofl_share_r, namcofl_share_w)
	AM_RANGE(0x30300000, 0x30303fff) AM_RAM /* COMRAM */
	AM_RANGE(0x30380000, 0x303800ff) AM_READ( fl_network_r )	/* network registers */
	AM_RANGE(0x30400000, 0x3040ffff) AM_RAM_WRITE(namcofl_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x30800000, 0x3080ffff) AM_READWRITE(namco_tilemapvideoram32_le_r, namco_tilemapvideoram32_le_w )
	AM_RANGE(0x30a00000, 0x30a0003f) AM_READWRITE(namco_tilemapcontrol32_le_r, namco_tilemapcontrol32_le_w )
	AM_RANGE(0x30c00000, 0x30c1ffff) AM_READWRITE(namco_rozvideoram32_le_r,namco_rozvideoram32_le_w)
	AM_RANGE(0x30d00000, 0x30d0001f) AM_READWRITE(namco_rozcontrol32_le_r,namco_rozcontrol32_le_w)
	AM_RANGE(0x30e00000, 0x30e1ffff) AM_READWRITE(namco_obj32_le_r, namco_obj32_le_w)
	AM_RANGE(0x30f00000, 0x30f0000f) AM_RAM /* NebulaM2 code says this is int enable at 0000, int request at 0004, but doesn't do much about it */
	AM_RANGE(0x40000000, 0x4000005f) AM_READWRITE( namcofl_sysreg_r, namcofl_sysreg_w )
	AM_RANGE(0xfffffffc, 0xffffffff) AM_READ( fl_unk1_r )
ADDRESS_MAP_END


static WRITE16_HANDLER( mcu_shared_w )
{
	// HACK!  Many games data ROM routines redirect the vector from the sound command read to an RTS.
	// This needs more investigation.  nebulray and vshoot do NOT do this.
	// Timers A2 and A3 are set up in "external input counter" mode, this may be related.
#if 0
	if ((offset == 0x647c/2) && (data != 0))
	{
		data = 0xd2f6;
	}
#endif

	COMBINE_DATA(&namcofl_shareram[offset]);

	// C75 BIOS has a very short window on the CPU sync signal, so immediately let the i960 at it
	if ((offset == 0x6000/2) && (data & 0x80))
	{
		cpu_yield(space->cpu);
	}
}


static READ8_HANDLER( port6_r )
{
	return mcu_port6;
}

static WRITE8_HANDLER( port6_w )
{
	mcu_port6 = data;
}

static READ8_HANDLER( port7_r )
{
	switch (mcu_port6 & 0xf0)
	{
		case 0x00:
			return input_port_read(space->machine, "IN0");

		case 0x20:
			return input_port_read(space->machine, "MISC");

		case 0x40:
			return input_port_read(space->machine, "IN1");

		case 0x60:
			return input_port_read(space->machine, "IN2");

		default:
			break;
	}

	return 0xff;
}

static READ8_HANDLER(dac7_r)
{
	return input_port_read_safe(space->machine, "ACCEL", 0xff);
}

static READ8_HANDLER(dac6_r)
{
	return input_port_read_safe(space->machine, "BRAKE", 0xff);
}

static READ8_HANDLER(dac5_r)
{
	return input_port_read_safe(space->machine, "WHEEL", 0xff);
}

static READ8_HANDLER(dac4_r) { return 0xff; }
static READ8_HANDLER(dac3_r) { return 0xff; }
static READ8_HANDLER(dac2_r) { return 0xff; }
static READ8_HANDLER(dac1_r) { return 0xff; }
static READ8_HANDLER(dac0_r) { return 0xff; }

static ADDRESS_MAP_START( namcoc75_am, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x002000, 0x002fff) AM_DEVREADWRITE("c352", c352_r, c352_w)
	AM_RANGE(0x004000, 0x00bfff) AM_RAM_WRITE(mcu_shared_w) AM_BASE(&namcofl_shareram)
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM AM_REGION("c75", 0)
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("c75data", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( namcoc75_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M37710_PORT6, M37710_PORT6) AM_READWRITE(port6_r, port6_w)
	AM_RANGE(M37710_PORT7, M37710_PORT7) AM_READ(port7_r)
	AM_RANGE(M37710_ADC7_L, M37710_ADC7_L) AM_READ(dac7_r)
	AM_RANGE(M37710_ADC6_L, M37710_ADC6_L) AM_READ(dac6_r)
	AM_RANGE(M37710_ADC5_L, M37710_ADC5_L) AM_READ(dac5_r)
	AM_RANGE(M37710_ADC4_L, M37710_ADC4_L) AM_READ(dac4_r)
	AM_RANGE(M37710_ADC3_L, M37710_ADC3_L) AM_READ(dac3_r)
	AM_RANGE(M37710_ADC2_L, M37710_ADC2_L) AM_READ(dac2_r)
	AM_RANGE(M37710_ADC1_L, M37710_ADC1_L) AM_READ(dac1_r)
	AM_RANGE(M37710_ADC0_L, M37710_ADC0_L) AM_READ(dac0_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( namcofl )
	PORT_START("MISC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_TOGGLE
	PORT_DIPNAME( 0x20, 0x20, "Freeze Screen" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("BRAKE")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
INPUT_PORTS_END

static const gfx_layout obj_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8, /* bits per pixel */
	{
		/* plane offsets */
		0,1,2,3,4,5,6,7,
	},
	{
		0*16+8,1*16+8,0*16,1*16,
		2*16+8,3*16+8,2*16,3*16,
		4*16+8,5*16+8,4*16,5*16,
		6*16+8,7*16+8,6*16,7*16
	},
	{
		0x0*128,0x1*128,0x2*128,0x3*128,0x4*128,0x5*128,0x6*128,0x7*128,
		0x8*128,0x9*128,0xa*128,0xb*128,0xc*128,0xd*128,0xe*128,0xf*128
	},
	16*128
};

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	{ 0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64 },
	8*64
};

static const gfx_layout roz_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	{
		0*128,1*128,2*128,3*128,4*128,5*128,6*128,7*128,8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128
	},
	16*128
};

static GFXDECODE_START( 2 )
	GFXDECODE_ENTRY( NAMCONB1_TILEGFXREGION,	0, tile_layout,	0x1000, 0x08 )
	GFXDECODE_ENTRY( NAMCONB1_SPRITEGFXREGION,	0, obj_layout,		0x0000, 0x10 )
	GFXDECODE_ENTRY( NAMCONB1_ROTGFXREGION,	    0, roz_layout,		0x1800, 0x08 )
GFXDECODE_END


static TIMER_CALLBACK( network_interrupt_callback )
{
	cputag_set_input_line(machine, "maincpu", I960_IRQ0, ASSERT_LINE);
	timer_set(machine, video_screen_get_frame_period(machine->primary_screen), NULL, 0, network_interrupt_callback);
}


static TIMER_CALLBACK( vblank_interrupt_callback )
{
	cputag_set_input_line(machine, "maincpu", I960_IRQ2, ASSERT_LINE);
	timer_set(machine, video_screen_get_frame_period(machine->primary_screen), NULL, 0, vblank_interrupt_callback);
}


static TIMER_CALLBACK( raster_interrupt_callback )
{
	video_screen_update_partial(machine->primary_screen, video_screen_get_vpos(machine->primary_screen));
	cputag_set_input_line(machine, "maincpu", I960_IRQ1, ASSERT_LINE);
	timer_adjust_oneshot(raster_interrupt_timer, video_screen_get_frame_period(machine->primary_screen), 0);
}

static INTERRUPT_GEN( mcu_interrupt )
{
	if (cpu_getiloops(device) == 0)
	{
		cpu_set_input_line(device, M37710_LINE_IRQ0, HOLD_LINE);
	}
	else if (cpu_getiloops(device) == 1)
	{
		cpu_set_input_line(device, M37710_LINE_IRQ2, HOLD_LINE);
	}
	else
	{
		cpu_set_input_line(device, M37710_LINE_ADC, HOLD_LINE);
	}
}

static MACHINE_START( namcofl )
{
	raster_interrupt_timer = timer_alloc(machine, raster_interrupt_callback, NULL);
}


static MACHINE_RESET( namcofl )
{
	timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, video_screen_get_visible_area(machine->primary_screen)->max_y + 3, 0), NULL, 0, network_interrupt_callback);
	timer_set(machine, video_screen_get_time_until_pos(machine->primary_screen, video_screen_get_visible_area(machine->primary_screen)->max_y + 1, 0), NULL, 0, vblank_interrupt_callback);
}


static MACHINE_DRIVER_START( namcofl )
	MDRV_CPU_ADD("maincpu", I960, 20000000)	// i80960KA-20 == 20 MHz part
	MDRV_CPU_PROGRAM_MAP(namcofl_mem)

	MDRV_CPU_ADD("mcu", M37702, 48384000/3)
	MDRV_CPU_PROGRAM_MAP(namcoc75_am)
	MDRV_CPU_IO_MAP(namcoc75_io)
	MDRV_CPU_VBLANK_INT_HACK(mcu_interrupt, 3)

	MDRV_MACHINE_START(namcofl)
	MDRV_MACHINE_RESET(namcofl)
	MDRV_NVRAM_HANDLER(generic_1fill)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(NAMCONB1_HTOTAL, NAMCONB1_VTOTAL)
	MDRV_SCREEN_VISIBLE_AREA(0, NAMCONB1_HBSTART-1, 0, NAMCONB1_VBSTART-1)

	MDRV_PALETTE_LENGTH(8192)

	MDRV_GFXDECODE(2)

	MDRV_VIDEO_START(namcofl)
	MDRV_VIDEO_UPDATE(namcofl)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MDRV_SOUND_ADD("c352", C352, 48384000/3)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_DRIVER_END

ROM_START( speedrcr )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("se2mpea4.19a",   0x000000, 0x080000, CRC(95ab3fd7) SHA1(273a536f8512f3c55260ac1b78533bc35b8390ed) )
	ROM_LOAD32_WORD("se2mpoa4.18a",   0x000002, 0x080000, CRC(5b5ef1eb) SHA1(3e9e4abb1a32269baef772079de825dfe1ea230c) )

	ROM_REGION( 0x200000, "user1", 0 ) // Data
	ROM_LOAD32_BYTE("se1_dat0.13a",   0x000000, 0x080000, CRC(cc5d6ff5) SHA1(6fad40a1fac75bc64d3b7a7562cf7ce2a3abd36a) )
	ROM_LOAD32_BYTE("se1_dat1.14a",   0x000001, 0x080000, CRC(ddc8b306) SHA1(f169d521b800c108deffdef9fc6b0058621ee909) )
	ROM_LOAD32_BYTE("se1_dat2.15a",   0x000002, 0x080000, CRC(2a29abbb) SHA1(945419ed61e9a656a340214a63a01818396fbe98) )
	ROM_LOAD32_BYTE("se1_dat3.16a",   0x000003, 0x080000, CRC(49849aff) SHA1(b7c7eea1d56304e40e996ee998c971313ff03614) )

	ROM_REGION16_LE( 0x4000, "c75", 0 ) // C75 program
	ROM_LOAD( "c75.bin", 0, 0x4000, CRC(42f539a5) SHA1(3103e5a0a2867620309fd4fe478a2be0effbeff8) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) // C75 data
	ROM_LOAD("se1_spr.21l",   0x000000,  0x80000, CRC(850a27ac) SHA1(7d5db840ec67659a1f2e69a62cdb03ce6ee0b47b) )

	ROM_REGION( 0x200000, NAMCONB1_ROTGFXREGION, 0 )	// "RCHAR" (roz characters)
	ROM_LOAD("se1_rch0.19j",   0x000000, 0x100000, CRC(a0827288) SHA1(13691ef4d402a6dc91851de4f82cfbdf96d417cb) )
	ROM_LOAD("se1_rch1.18j",   0x100000, 0x100000, CRC(af7609ad) SHA1(b16041f0eb47d7566011d9d762a3083411dc422e) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("se1_sch0.21p",   0x000000, 0x100000, CRC(7b5cfad0) SHA1(5a0355e37eb191bc0cf8b6b7c3d0274560b9bbd5) )
	ROM_LOAD("se1_sch1.20p",   0x100000, 0x100000, CRC(5086e0d3) SHA1(0aa7d11f4f9a75117e69cc77f1b73a68d9007aef) )
	ROM_LOAD("se1_sch2.19p",   0x200000, 0x100000, CRC(e59a731e) SHA1(3fed72e9bb485d4d689ab51490360c4c6f1dc5cb) )
	ROM_LOAD("se1_sch3.18p",   0x300000, 0x100000, CRC(f817027a) SHA1(71745476f496c60d89c8563b3e46bc85eebc79ce) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )	// OBJ
	ROM_LOAD16_BYTE("se1obj0l.ic1", 0x000001, 0x200000, CRC(17585218) SHA1(3332afa9bd194ac37b8d6f352507c523a0f2e2b3) )
	ROM_LOAD16_BYTE("se1obj0u.ic2", 0x000000, 0x200000, CRC(d14b1236) SHA1(e5447732ef3acec88fb7a00e0deca3e71a40ae65) )
	ROM_LOAD16_BYTE("se1obj1l.ic3", 0x400001, 0x200000, CRC(c4809fd5) SHA1(e0b80fccc17c83fb9d08f7f1cf2cd2f0f3a510b4) )
	ROM_LOAD16_BYTE("se1obj1u.ic4", 0x400000, 0x200000, CRC(0beefa56) SHA1(012fb7b330dbf851ab2217da0a0e7136ddc3d23f) )

	ROM_REGION( 0x100000, NAMCONB1_ROTMASKREGION, 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("se1_rsh.14k",    0x000000, 0x100000, CRC(7aa5a962) SHA1(ff936dfcfcc4ee1f5f2232df62def76ff99e671e) )

	ROM_REGION( 0x100000, NAMCONB1_TILEMASKREGION, 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("se1_ssh.18u",    0x000000, 0x100000, CRC(7a8e0bda) SHA1(f6a508d90274d0205fec0c46f5f783a2715c0c6e) )

	ROM_REGION( 0x400000, "c352", 0 ) // Samples
	ROM_LOAD("se1_voi.23s",   0x000000, 0x400000, CRC(b95e2ffb) SHA1(7669232d772caa9afa4c7593d018e8b6e534114a) )

    ROM_REGION( 0x000005, "pals", 0) /* PAL's */
    ROM_LOAD( "sysfl-1.bin",  0x000000, 0x000001, NO_DUMP ) /* PAL16L8BCN at 2S */
    ROM_LOAD( "sysfl-2.bin",  0x000000, 0x000001, NO_DUMP ) /* PAL16L8BCN at 3L */
    ROM_LOAD( "sysfl-3.bin",  0x000000, 0x000001, NO_DUMP ) /* PALCE16V8H-15PC/4 at 12S */
    ROM_LOAD( "sysfl-4.bin",  0x000000, 0x000001, NO_DUMP ) /* PAL20L8BCNS at 20N */
    ROM_LOAD( "sysfl-5.bin",  0x000000, 0x000001, NO_DUMP ) /* PALCE16V8H-15PC/4 at 19D */
ROM_END

ROM_START( finalaprb )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("flr2mpeb.19a",   0x000000, 0x080000, CRC(8bfe615f) SHA1(7b867eb261268a83177f1f873689f77d1b6c47ca) )
	ROM_LOAD32_WORD("flr2mpob.18a",   0x000002, 0x080000, CRC(91c14e4f) SHA1(934a86daaef0e3e2c2b3066f4677ccb3aaab6eaf) )

	ROM_REGION( 0x200000, "user1", ROMREGION_ERASEFF ) // Data

	ROM_REGION16_LE( 0x4000, "c75", 0 ) // C75 program
	ROM_LOAD( "c75.bin", 0, 0x4000, CRC(42f539a5) SHA1(3103e5a0a2867620309fd4fe478a2be0effbeff8) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) // C75 data
	ROM_LOAD("flr1spr.21l",   0x000000,  0x20000, CRC(69bb0f5e) SHA1(6831d618de42a165e508ad37db594d3aa290c530) )

	ROM_REGION( 0x200000, NAMCONB1_ROTGFXREGION, 0 )	// "RCHAR" (roz characters)
	ROM_LOAD("flr1rch0.19j",   0x000000, 0x100000, CRC(f413f50d) SHA1(cdd8073dda4feaea78e3b94520cf20a9799fd04d) )
	ROM_LOAD("flr1rch1.18j",   0x100000, 0x100000, CRC(4654d519) SHA1(f8bb473013cdca48dd98df0de2f78c300c156e91) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("flr1sch0.21p",   0x000000, 0x100000, CRC(7169efca) SHA1(66c7aa1b50b236b4700b07be0dca7aebdabedb8c) )
	ROM_LOAD("flr1sch1.20p",   0x100000, 0x100000, CRC(aa233a02) SHA1(0011329f585658d90f820daf0ba08ce2735bddfc) )
	ROM_LOAD("flr1sch2.19p",   0x200000, 0x100000, CRC(9b6b7abd) SHA1(5cdec70db1b46bc5d0866ca155b520157fef3adf) )
	ROM_LOAD("flr1sch3.18p",   0x300000, 0x100000, CRC(50a14f54) SHA1(ab9c2f2e11f006a9dc7e5aedd5788d7d67166d36) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )	// OBJ
	ROM_LOAD16_BYTE("flr1obj0l.ic1", 0x000001, 0x200000, CRC(364a902c) SHA1(4a1ea48eee86d410e36096cc100b4c9a5a645034) )
	ROM_LOAD16_BYTE("flr1obj0u.ic2", 0x000000, 0x200000, CRC(a5c7b80e) SHA1(4e0e863cfdd8c051c3c4594bb21e11fb93c28f0c) )
	ROM_LOAD16_BYTE("flr1obj1l.ic3", 0x400001, 0x200000, CRC(51fd8de7) SHA1(b1571c45e8c33d746716fd790c704a3361d02bdc) )
	ROM_LOAD16_BYTE("flr1obj1u.ic4", 0x400000, 0x200000, CRC(1737aa3c) SHA1(8eaf0dc5d60a270d2c1626f54f5edbddbb0a59c8) )

	ROM_REGION( 0x80000, NAMCONB1_ROTMASKREGION, 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("flr1rsh.14k",    0x000000, 0x080000, CRC(037c0983) SHA1(c48574a8ad125cedfaf2538c5ff824e121204629) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("flr1ssh.18u",    0x000000, 0x080000, CRC(f70cb2bf) SHA1(dbddda822287783a43415172b81d0382a8ac43d8) )

	ROM_REGION( 0x200000, "c352", 0 ) // Samples
	ROM_LOAD("flr1voi.23s",   0x000000, 0x200000, CRC(ff6077cd) SHA1(73c289125ddeae3e43153e4c570549ca04501262) )
ROM_END

ROM_START( finalapro )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("flr2mpe.19a",   0x000000, 0x080000, CRC(cc8961ae) SHA1(08ce4d27a723101370d1c536b26256ce0d8a1b6c) )
	ROM_LOAD32_WORD("flr2mpo.18a",   0x000002, 0x080000, CRC(8118f465) SHA1(c4b79878a82fd36b5707e92aa893f69c2b942d57) )

	ROM_REGION( 0x200000, "user1", ROMREGION_ERASEFF ) // Data

	ROM_REGION16_LE( 0x4000, "c75", 0 ) // C75 program
	ROM_LOAD( "c75.bin", 0, 0x4000, CRC(42f539a5) SHA1(3103e5a0a2867620309fd4fe478a2be0effbeff8) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) // C75 data
	ROM_LOAD("flr1spr.21l",   0x000000,  0x20000, CRC(69bb0f5e) SHA1(6831d618de42a165e508ad37db594d3aa290c530) )

	ROM_REGION( 0x200000, NAMCONB1_ROTGFXREGION, 0 )	// "RCHAR" (roz characters)
	ROM_LOAD("flr1rch0.19j",   0x000000, 0x100000, CRC(f413f50d) SHA1(cdd8073dda4feaea78e3b94520cf20a9799fd04d) )
	ROM_LOAD("flr1rch1.18j",   0x100000, 0x100000, CRC(4654d519) SHA1(f8bb473013cdca48dd98df0de2f78c300c156e91) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("flr1sch0.21p",   0x000000, 0x100000, CRC(7169efca) SHA1(66c7aa1b50b236b4700b07be0dca7aebdabedb8c) )
	ROM_LOAD("flr1sch1.20p",   0x100000, 0x100000, CRC(aa233a02) SHA1(0011329f585658d90f820daf0ba08ce2735bddfc) )
	ROM_LOAD("flr1sch2.19p",   0x200000, 0x100000, CRC(9b6b7abd) SHA1(5cdec70db1b46bc5d0866ca155b520157fef3adf) )
	ROM_LOAD("flr1sch3.18p",   0x300000, 0x100000, CRC(50a14f54) SHA1(ab9c2f2e11f006a9dc7e5aedd5788d7d67166d36) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )	// OBJ
	ROM_LOAD16_BYTE("flr1obj0l.ic1", 0x000001, 0x200000, CRC(364a902c) SHA1(4a1ea48eee86d410e36096cc100b4c9a5a645034) )
	ROM_LOAD16_BYTE("flr1obj0u.ic2", 0x000000, 0x200000, CRC(a5c7b80e) SHA1(4e0e863cfdd8c051c3c4594bb21e11fb93c28f0c) )
	ROM_LOAD16_BYTE("flr1obj1l.ic3", 0x400001, 0x200000, CRC(51fd8de7) SHA1(b1571c45e8c33d746716fd790c704a3361d02bdc) )
	ROM_LOAD16_BYTE("flr1obj1u.ic4", 0x400000, 0x200000, CRC(1737aa3c) SHA1(8eaf0dc5d60a270d2c1626f54f5edbddbb0a59c8) )

	ROM_REGION( 0x80000, NAMCONB1_ROTMASKREGION, 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("flr1rsh.14k",    0x000000, 0x080000, CRC(037c0983) SHA1(c48574a8ad125cedfaf2538c5ff824e121204629) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("flr1ssh.18u",    0x000000, 0x080000, CRC(f70cb2bf) SHA1(dbddda822287783a43415172b81d0382a8ac43d8) )

	ROM_REGION( 0x200000, "c352", 0 ) // Samples
	ROM_LOAD("flr1voi.23s",   0x000000, 0x200000, CRC(ff6077cd) SHA1(73c289125ddeae3e43153e4c570549ca04501262) )
ROM_END

ROM_START( finalapr )
	ROM_REGION( 0x200000, "maincpu", 0 ) // i960 program
	ROM_LOAD32_WORD("flr1_mpec.19a", 0x000000, 0x080000, CRC(52735494) SHA1(db9873cb39bcfdd3dbe2e5079249fecac2c46df9) )
	ROM_LOAD32_WORD("flr1_mpoc.18a", 0x000002, 0x080000, CRC(b11fe577) SHA1(70b51a1e66a3bb92f027aad7ba0f358c0e139b3c) )

	ROM_REGION( 0x200000, "user1", ROMREGION_ERASEFF ) // Data

	ROM_REGION16_LE( 0x4000, "c75", 0 ) // C75 program
	ROM_LOAD( "c75.bin", 0, 0x4000, CRC(42f539a5) SHA1(3103e5a0a2867620309fd4fe478a2be0effbeff8) )

	ROM_REGION16_LE( 0x80000, "c75data", 0 ) // C75 data
	ROM_LOAD("flr1spr.21l",   0x000000,  0x20000, CRC(69bb0f5e) SHA1(6831d618de42a165e508ad37db594d3aa290c530) )

	ROM_REGION( 0x200000, NAMCONB1_ROTGFXREGION, 0 )	// "RCHAR" (roz characters)
	ROM_LOAD("flr1rch0.19j",   0x000000, 0x100000, CRC(f413f50d) SHA1(cdd8073dda4feaea78e3b94520cf20a9799fd04d) )
	ROM_LOAD("flr1rch1.18j",   0x100000, 0x100000, CRC(4654d519) SHA1(f8bb473013cdca48dd98df0de2f78c300c156e91) )

	ROM_REGION( 0x400000, NAMCONB1_TILEGFXREGION, 0 ) // "SCHAR" (regular BG characters)
	ROM_LOAD("flr1sch0.21p",   0x000000, 0x100000, CRC(7169efca) SHA1(66c7aa1b50b236b4700b07be0dca7aebdabedb8c) )
	ROM_LOAD("flr1sch1.20p",   0x100000, 0x100000, CRC(aa233a02) SHA1(0011329f585658d90f820daf0ba08ce2735bddfc) )
	ROM_LOAD("flr1sch2.19p",   0x200000, 0x100000, CRC(9b6b7abd) SHA1(5cdec70db1b46bc5d0866ca155b520157fef3adf) )
	ROM_LOAD("flr1sch3.18p",   0x300000, 0x100000, CRC(50a14f54) SHA1(ab9c2f2e11f006a9dc7e5aedd5788d7d67166d36) )

	ROM_REGION( 0x800000, NAMCONB1_SPRITEGFXREGION, 0 )	// OBJ
	ROM_LOAD16_BYTE("flr1obj0l.ic1", 0x000001, 0x200000, CRC(364a902c) SHA1(4a1ea48eee86d410e36096cc100b4c9a5a645034) )
	ROM_LOAD16_BYTE("flr1obj0u.ic2", 0x000000, 0x200000, CRC(a5c7b80e) SHA1(4e0e863cfdd8c051c3c4594bb21e11fb93c28f0c) )
	ROM_LOAD16_BYTE("flr1obj1l.ic3", 0x400001, 0x200000, CRC(51fd8de7) SHA1(b1571c45e8c33d746716fd790c704a3361d02bdc) )
	ROM_LOAD16_BYTE("flr1obj1u.ic4", 0x400000, 0x200000, CRC(1737aa3c) SHA1(8eaf0dc5d60a270d2c1626f54f5edbddbb0a59c8) )

	ROM_REGION( 0x80000, NAMCONB1_ROTMASKREGION, 0 ) // "RSHAPE" (roz mask like NB-1?)
	ROM_LOAD("flr1rsh.14k",    0x000000, 0x080000, CRC(037c0983) SHA1(c48574a8ad125cedfaf2538c5ff824e121204629) )

	ROM_REGION( 0x80000, NAMCONB1_TILEMASKREGION, 0 ) // "SSHAPE" (mask for other tiles?)
	ROM_LOAD("flr1ssh.18u",    0x000000, 0x080000, CRC(f70cb2bf) SHA1(dbddda822287783a43415172b81d0382a8ac43d8) )

	ROM_REGION( 0x200000, "c352", 0 ) // Samples
	ROM_LOAD("flr1voi.23s",   0x000000, 0x200000, CRC(ff6077cd) SHA1(73c289125ddeae3e43153e4c570549ca04501262) )
ROM_END

static void namcofl_common_init(running_machine *machine)
{
	namcofl_workram = auto_alloc_array(machine, UINT32, 0x100000/4);

	memory_set_bankptr(machine,  "bank1", memory_region(machine, "maincpu") );
	memory_set_bankptr(machine,  "bank2", namcofl_workram );
}

static DRIVER_INIT(speedrcr)
{
	namcofl_common_init(machine);
	namcos2_gametype = NAMCOFL_SPEED_RACER;
}

static DRIVER_INIT(finalapr)
{
	namcofl_common_init(machine);
	namcos2_gametype = NAMCOFL_FINAL_LAP_R;
}

GAME( 1995, speedrcr,         0, namcofl, namcofl, speedrcr, ROT0, "Namco", "Speed Racer", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1995, finalapr,         0, namcofl, namcofl, finalapr, ROT0, "Namco", "Final Lap R (Japan Rev. C)", GAME_IMPERFECT_SOUND )
GAME( 1995, finalaprb, finalapr, namcofl, namcofl, finalapr, ROT0, "Namco", "Final Lap R (Rev. B)", GAME_IMPERFECT_SOUND )
GAME( 1995, finalapro, finalapr, namcofl, namcofl, finalapr, ROT0, "Namco", "Final Lap R", GAME_IMPERFECT_SOUND )
