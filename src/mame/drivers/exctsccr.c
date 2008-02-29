/***************************************************************************

Exciting Soccer    - (c) 1983 Alpha Denshi Co.
Exciting Soccer II - (c) 1984 Alpha Denshi Co.

Supported sets:
Exciting Soccer - Alpha Denshi
Exciting Soccer (bootleg) - Kazutomi
Exciting Soccer II - Alpha Denshi


Preliminary driver by:
Ernesto Corvi
ernesto@imagina.com

Jarek Parchanski
jpdev@friko6.onet.pl
ALPHA 8301 MCU handling by Tatsuyuki satoh

NOTES:
The game supports Coin 2, but the dip switches used for it are the same
as Coin 1. Basically, this allowed to select an alternative coin table
based on wich Coin input was connected.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "sound/ay8910.h"
#include "sound/dac.h"

/* MCU hacking switch */
/*  when set 1, old raw patch */
/*  when set 0, uses MCU core emulation and some MCU patch */
#define MCU_HACK 0

/* from video */
extern WRITE8_HANDLER( exctsccr_videoram_w );
extern WRITE8_HANDLER( exctsccr_colorram_w );
extern WRITE8_HANDLER( exctsccr_gfx_bank_w );
extern WRITE8_HANDLER( exctsccr_flipscreen_w );

extern PALETTE_INIT( exctsccr );
extern VIDEO_START( exctsccr );
extern VIDEO_UPDATE( exctsccr );

/* from machine */
#if MCU_HACK
extern UINT8 *exctsccr_mcu_ram;
extern WRITE8_HANDLER( exctsccr_mcu_w );
extern WRITE8_HANDLER( exctsccr_mcu_control_w );
extern WRITE8_HANDLER( exctscc2_mcu_control_w );
#endif


static WRITE8_HANDLER( exctsccr_DAC_data_w )
{
	DAC_signed_data_w(offset,data << 2);
}

/*
exctsccr MCU program patch data
*/
static const int exctsccr_mcu_patch_data[] =
{
/*
bypass ROM sum error because some opcodes are unknown yet
3B7: F6      db   f6
3BA: F6      db   f6
3BC: F9      db   f9
3BD: F6      db   f6
3C1: F6      db   f6
3C7: F8      db   f8 ; check SUM to ZF
*/
  (0x3c8<<16) | (0xd1<<8) | 0xd2, // JZ -> JMP
  -1
};

/*
exctscc2 MCU program patch data
*/
static const int exctscc2_mcu_patch_data[] =
{
/*
bypass initislize check

307: D5 01 unknown
309: D4 00 unknown
A reg 2 to 5

30F: D4 01 unknown
A reg 5 to 2

*/
  (0x30d<<16) | (0xcf<<8) | 0xda, // JNZ -> CMP
  (0x313<<16) | (0xcf<<8) | 0xda, // JNZ -> CMP
/*
bypass ROM check sum
3B7: F6      db   f6
3BA: F6      db   f6
3BC: F9      db   f9
3BD: F6      db   f6
3C1: F6      db   f6
3C7: F8      db   f8
*/
  (0x3C8<<16) | (0xd1<<8) | 0xd2, // JZ -> JMP
  -1
};

static const int *mcu_patch_data = NULL;
static UINT8 *mcu_shared_ram;
static WRITE8_HANDLER( cexctsccr_mcu_halt_w )
{
#if MCU_HACK
	exctsccr_mcu_control_w(offset,data&1);
#else
	const int *p;

	data &= 1;
	cpunum_set_input_line(Machine, 2, INPUT_LINE_HALT, data ? ASSERT_LINE : CLEAR_LINE);
	if( (p=mcu_patch_data) != NULL)
	{
		/* patch MCU program */
		while(*p != -1)
		{
			int a = (*p)>>16;
			int c = ((*p)>>8) & 0xff;
			int d = (*p)&0xff;
			if(mcu_shared_ram[a] == c)
				mcu_shared_ram[a] = d;
			p++;
		}
	}
#endif
}

/***************************************************************************

    Memory definition(s)

***************************************************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
#if MCU_HACK
	AM_RANGE(0x6000, 0x63ff) AM_WRITE(exctsccr_mcu_w) AM_READ(MRA8_RAM) AM_BASE(&exctsccr_mcu_ram) /* Alpha mcu (protection) */
#else
	AM_RANGE(0x6000, 0x63ff) AM_RAM AM_SHARE(1) AM_BASE(&mcu_shared_ram)
#endif
	AM_RANGE(0x7c00, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(exctsccr_videoram_w) AM_READ(MRA8_RAM) AM_BASE(&videoram)
	AM_RANGE(0x8400, 0x87ff) AM_WRITE(exctsccr_colorram_w) AM_READ(MRA8_RAM) AM_BASE(&colorram)
	AM_RANGE(0x8800, 0x8bff) AM_WRITE(MWA8_RAM) AM_READ(MRA8_RAM) AM_BASE(&spriteram_2) /* ??? */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(MWA8_NOP) AM_READ(input_port_0_r)
	AM_RANGE(0xa001, 0xa001) AM_WRITE(MWA8_NOP) /* ??? */
	AM_RANGE(0xa002, 0xa002) AM_WRITE(exctsccr_gfx_bank_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(exctsccr_flipscreen_w) /* Cocktail mode ( 0xff = flip screen, 0x00 = normal ) */
	/* 0xa006 MCU control */
	AM_RANGE(0xa006, 0xa006) AM_WRITE(cexctsccr_mcu_halt_w)
	AM_RANGE(0xa007, 0xa007) AM_WRITE(MWA8_NOP) /* This is also MCU control, but i dont need it */
	AM_RANGE(0xa040, 0xa06f) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) /* Sprite pos */
	AM_RANGE(0xa080, 0xa080) AM_WRITE(soundlatch_w)
	AM_RANGE(0xa0c0, 0xa0c0) AM_WRITE(watchdog_reset_w)

	AM_RANGE(0xa040, 0xa040) AM_READ(input_port_1_r)
	AM_RANGE(0xa080, 0xa080) AM_READ(input_port_3_r)
	AM_RANGE(0xa0c0, 0xa0c0) AM_READ(input_port_2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x8fff) AM_ROM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM
	AM_RANGE(0xc008, 0xc009) AM_WRITE(exctsccr_DAC_data_w)
	AM_RANGE(0xc00c, 0xc00c) AM_WRITE(soundlatch_w) /* used to clear the latch */
	AM_RANGE(0xc00d, 0xc00d) AM_READ(soundlatch_r)
	AM_RANGE(0xc00f, 0xc00f) AM_WRITE(MWA8_NOP) /* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writeport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x82, 0x82) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x83, 0x83) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x86, 0x86) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x87, 0x87) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x8a, 0x8a) AM_WRITE(AY8910_write_port_2_w)
	AM_RANGE(0x8b, 0x8b) AM_WRITE(AY8910_control_port_2_w)
	AM_RANGE(0x8e, 0x8e) AM_WRITE(AY8910_write_port_3_w)
	AM_RANGE(0x8f, 0x8f) AM_WRITE(AY8910_control_port_3_w)
ADDRESS_MAP_END

/* Bootleg */
static ADDRESS_MAP_START( bl_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0x83ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x8400, 0x87ff) AM_READ(MRA8_RAM)
	AM_RANGE(0x8800, 0x8fff) AM_READ(MRA8_RAM) /* ??? */
	AM_RANGE(0xa000, 0xa000) AM_READ(input_port_0_r)
	AM_RANGE(0xa040, 0xa040) AM_READ(input_port_1_r)
	AM_RANGE(0xa080, 0xa080) AM_READ(input_port_3_r)
	AM_RANGE(0xa0c0, 0xa0c0) AM_READ(input_port_2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bl_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x7001, 0x7001) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(exctsccr_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x8400, 0x87ff) AM_WRITE(exctsccr_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x8800, 0x8fff) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram_2) /* ??? */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(MWA8_NOP) /* ??? */
	AM_RANGE(0xa001, 0xa001) AM_WRITE(MWA8_NOP) /* ??? */
	AM_RANGE(0xa002, 0xa002) AM_WRITE(exctsccr_gfx_bank_w) /* ??? */
	AM_RANGE(0xa003, 0xa003) AM_WRITE(exctsccr_flipscreen_w) /* Cocktail mode ( 0xff = flip screen, 0x00 = normal ) */
	AM_RANGE(0xa006, 0xa006) AM_WRITE(MWA8_NOP) /* no MCU, but some leftover code still writes here */
	AM_RANGE(0xa007, 0xa007) AM_WRITE(MWA8_NOP) /* no MCU, but some leftover code still writes here */
	AM_RANGE(0xa040, 0xa06f) AM_WRITE(MWA8_RAM) AM_BASE(&spriteram) /* Sprite Pos */
	AM_RANGE(0xa080, 0xa080) AM_WRITE(soundlatch_w)
	AM_RANGE(0xa0c0, 0xa0c0) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bl_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0xe000, 0xe3ff) AM_READ(MRA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bl_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0x8000, 0x8000) AM_WRITE(MWA8_NOP) /* 0 = DAC sound off, 1 = DAC sound on */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(soundlatch_w) /* used to clear the latch */
	AM_RANGE(0xc000, 0xc000) AM_WRITE(exctsccr_DAC_data_w)
	AM_RANGE(0xe000, 0xe3ff) AM_WRITE(MWA8_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE(1) /* main CPU shared RAM */
ADDRESS_MAP_END

/***************************************************************************

    Input port(s)

***************************************************************************/

static INPUT_PORTS_START( exctsccr )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, "A 1C/1C B 3C/1C" )
	PORT_DIPSETTING(    0x01, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(    0x00, "A 1C/3C B 1C/6C" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(    0x20, "1 Min." )
	PORT_DIPSETTING(    0x00, "2 Min." )
	PORT_DIPSETTING(    0x60, "3 Min." )
	PORT_DIPSETTING(    0x40, "4 Min." )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Has to be 0 */
INPUT_PORTS_END

/***************************************************************************

    Graphic(s) decoding

***************************************************************************/

static const gfx_layout charlayout1 =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	3,		/* 3 bits per pixel */
	{ 0x4000*8+4, 0, 4 },	/* plane offset */
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 consecutive bytes */
};

static const gfx_layout charlayout2 =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	3,		/* 3 bits per pixel */
	{ 0x2000*8, 0, 4 },	/* plane offset */
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 consecutive bytes */
};

static const gfx_layout spritelayout1 =
{
	16,16,	    /* 16*16 sprites */
	64,	        /* 64 sprites */
	3,	        /* 3 bits per pixel */
	{ 0x4000*8+4, 0, 4 },	/* plane offset */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3  },
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8,
			32 * 8, 33 * 8, 34 * 8, 35 * 8, 36 * 8, 37 * 8, 38 * 8, 39 * 8 },
	64*8	/* every sprite takes 64 bytes */
};

static const gfx_layout spritelayout2 =
{
	16,16,	    /* 16*16 sprites */
	64,         /* 64 sprites */
	3,	        /* 3 bits per pixel */
	{ 0x2000*8, 0, 4 },	/* plane offset */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3  },
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8,
			32 * 8, 33 * 8, 34 * 8, 35 * 8, 36 * 8, 37 * 8, 38 * 8, 39 * 8 },
	64*8	/* every sprite takes 64 bytes */
};

static const gfx_layout spritelayout =
{
	16,16,		/* 16*16 sprites */
	64,	    	/* 64 sprites */
	3,	    	/* 2 bits per pixel */
	{ 0x1000*8+4, 0, 4 },	/* plane offset */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3  },
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8,
			32 * 8, 33 * 8, 34 * 8, 35 * 8, 36 * 8, 37 * 8, 38 * 8, 39 * 8 },
	64*8	/* every sprite takes 64 bytes */
};

static GFXDECODE_START( exctsccr )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout1,      0, 16 ) /* chars */
	GFXDECODE_ENTRY( REGION_GFX1, 0x2000, charlayout2,      0, 16 ) /* chars */
	GFXDECODE_ENTRY( REGION_GFX1, 0x1000, spritelayout1, 16*8, 32 ) /* sprites */
	GFXDECODE_ENTRY( REGION_GFX1, 0x3000, spritelayout2, 16*8, 32 ) /* sprites */
	GFXDECODE_ENTRY( REGION_GFX2, 0x0000, spritelayout,  16*8, 32 ) /* sprites */
GFXDECODE_END


/***************************************************************************

    Machine driver(s)

***************************************************************************/

static MACHINE_DRIVER_START( exctsccr )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 4000000)	/* 4.0 MHz (?) */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80, 4123456)	/* ??? with 4 MHz, nested NMIs might happen */
	MDRV_CPU_PROGRAM_MAP(sub_map,0)
	MDRV_CPU_IO_MAP(0,sound_writeport)
	MDRV_CPU_PERIODIC_INT(nmi_line_pulse, 4000) /* 4 kHz, updates the dac */

	/* MCU */
#if MCU_HACK
#else
	MDRV_CPU_ADD(ALPHA8301, 4000000/8)
	MDRV_CPU_PROGRAM_MAP(mcu_map,0)
#endif

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(exctsccr)
	MDRV_PALETTE_LENGTH(48*8)

	MDRV_PALETTE_INIT(exctsccr)
	MDRV_VIDEO_START(exctsccr)
	MDRV_VIDEO_UPDATE(exctsccr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/* Bootleg */
static MACHINE_DRIVER_START( exctsccb )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 4000000)	/* 4.0 MHz (?) */
	MDRV_CPU_PROGRAM_MAP(bl_readmem,bl_writemem)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD(Z80, 3072000)	/* 3.072 MHz ? */
	MDRV_CPU_PROGRAM_MAP(bl_sound_readmem,bl_sound_writemem)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(exctsccr)
	MDRV_PALETTE_LENGTH(48*8)

	MDRV_PALETTE_INIT(exctsccr)
	MDRV_VIDEO_START(exctsccr)
	MDRV_VIDEO_UPDATE(exctsccr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( exctsccr )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1_g10.bin",    0x0000, 0x2000, CRC(aa68df66) SHA1(f10cac5a4c5aad1e1eb8835174dc8d517bb2921a) )
	ROM_LOAD( "2_h10.bin",    0x2000, 0x2000, CRC(2d8f8326) SHA1(8809e7b081fa2a1966cb51ac969fd7b468d35be0) )
	ROM_LOAD( "3_j10.bin",    0x4000, 0x2000, CRC(dce4a04d) SHA1(9c015e4597ec8921bea213d9841fc69c776a4e6d) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "0_h6.bin",     0x0000, 0x2000, CRC(3babbd6b) SHA1(b81bd47c4449f4f21f2d55d01eb9cb6db10664c7) )
	ROM_LOAD( "9_f6.bin",     0x2000, 0x2000, CRC(639998f5) SHA1(c4ff5e5e75d53dea38449f323186d08d5b57bf90) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "4_a5.bin",     0x0000, 0x2000, CRC(c342229b) SHA1(a989d6c12521c77882a7e17d4d80afe7eae05906) )
	ROM_LOAD( "5_b5.bin",     0x2000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) )
	ROM_LOAD( "6_c5.bin",     0x4000, 0x2000, CRC(eda40e32) SHA1(6c08fd4f4fb35fd354d02e04548e960c545f6a88) )

	ROM_REGION( 0x02000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "2_k5.bin",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3_l5.bin",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom2.8r",     0x0020, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
	ROM_LOAD( "prom3.k5",     0x0120, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
ROM_END

ROM_START( exctscca )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1_g10.bin",    0x0000, 0x2000, CRC(aa68df66) SHA1(f10cac5a4c5aad1e1eb8835174dc8d517bb2921a) )
	ROM_LOAD( "2_h10.bin",    0x2000, 0x2000, CRC(2d8f8326) SHA1(8809e7b081fa2a1966cb51ac969fd7b468d35be0) )
	ROM_LOAD( "3_j10.bin",    0x4000, 0x2000, CRC(dce4a04d) SHA1(9c015e4597ec8921bea213d9841fc69c776a4e6d) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "exctsccc.000", 0x0000, 0x2000, CRC(642fc42f) SHA1(cfc849d18e347e3e23fc31c1ce7f2580d5d9b2b0) )
	ROM_LOAD( "exctsccc.009", 0x2000, 0x2000, CRC(d88b3236) SHA1(80f083fb15243e9e68978677caed8aee8e3109a0) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "4_a5.bin",     0x0000, 0x2000, CRC(c342229b) SHA1(a989d6c12521c77882a7e17d4d80afe7eae05906) )
	ROM_LOAD( "5_b5.bin",     0x2000, 0x2000, CRC(35f4f8c9) SHA1(cdf5bbfea9abdd338938e5f4499d2d71ce3c6237) )
	ROM_LOAD( "6_c5.bin",     0x4000, 0x2000, CRC(eda40e32) SHA1(6c08fd4f4fb35fd354d02e04548e960c545f6a88) )

	ROM_REGION( 0x02000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "2_k5.bin",     0x0000, 0x1000, CRC(7f9cace2) SHA1(bf05a31716f3ca1c2fd1034cd1f39e2d21cdaed3) )
	ROM_LOAD( "3_l5.bin",     0x1000, 0x1000, CRC(db2d9e0d) SHA1(6ec09a47f7aea6bf31eb0ee78f44012f4d92de8a) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom2.8r",     0x0020, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
	ROM_LOAD( "prom3.k5",     0x0120, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
ROM_END

/* Bootleg */
ROM_START( exctsccb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "es-1.e2",      0x0000, 0x2000, CRC(997c6a82) SHA1(60fe27a12eedd22c775b7e65c5ba692cfcf5ac74) )
	ROM_LOAD( "es-2.g2",      0x2000, 0x2000, CRC(5c66e792) SHA1(f7a7f32806965fa926261217cee3159ccd198d49) )
	ROM_LOAD( "es-3.h2",      0x4000, 0x2000, CRC(e0d504c0) SHA1(d9a9f37b3a44a05a3f3389aa9617c419a2cee661) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* sound */
	ROM_LOAD( "es-a.k2",      0x0000, 0x2000, CRC(99e87b78) SHA1(f12006ff3f6f3c706e06288c97a1446141373432) )
	ROM_LOAD( "es-b.l2",      0x2000, 0x2000, CRC(8b3db794) SHA1(dbfed2357c7631bfca6bbd63a23617bc3abf6ca3) )
	ROM_LOAD( "es-c.m2",      0x4000, 0x2000, CRC(7bed2f81) SHA1(cbbb0480519cc04a99e8983228b18c9e49a9985d) )

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	/* I'm using the ROMs from exctscc2, national flags are wrong (ITA replaces USA) */
	ROM_LOAD( "vr.5a",        0x0000, 0x2000, BAD_DUMP CRC(4ff1783d) SHA1(c45074864c3a4bcbf3a87d164027ae16dca53d9c)  )
	ROM_LOAD( "vr.5b",        0x2000, 0x2000, BAD_DUMP CRC(5605b60b) SHA1(19d5909896ae4a3d7552225c369d30475c56793b)  )
	ROM_LOAD( "vr.5c",        0x4000, 0x2000, BAD_DUMP CRC(1fb84ee6) SHA1(56ceb86c509be783f806403ac21e7c9684760d5f)  )

	ROM_REGION( 0x02000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "vr.5k",        0x0000, 0x1000, BAD_DUMP CRC(1d37edfa) SHA1(184fa6dd7b1b3fff4c5fc19b42301ccb7979ac84)  )
	ROM_LOAD( "vr.5l",        0x1000, 0x1000, BAD_DUMP CRC(b97f396c) SHA1(4ffe512acf047230bd593911a615fc0ef66b481d)  )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom2.8r",     0x0020, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
	ROM_LOAD( "prom3.k5",     0x0120, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
ROM_END

ROM_START( exctscc2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "vr.3j",        0x0000, 0x2000, CRC(c6115362) SHA1(6a258631abd72ef6b8d7968bb4b2bc88e89e597d) )
	ROM_LOAD( "vr.3k",        0x2000, 0x2000, CRC(de36ba00) SHA1(0a0d92e710b8c749f145571bc8a204609456d19d) )
	ROM_LOAD( "vr.3l",        0x4000, 0x2000, CRC(1ddfdf65) SHA1(313d0a7f13fc2de15aa32492c38a59fbafad9f01) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "vr.7d",        0x0000, 0x2000, CRC(2c675a43) SHA1(aa0a8dbcae955e3da92c435202f2a1ed238c377e) )
	ROM_LOAD( "vr.7e",        0x2000, 0x2000, CRC(e571873d) SHA1(2dfff24f5dac86e92612f40cf3642005c7f36ad3) )
	ROM_LOAD( "8_d6.bin",     0x4000, 0x2000, CRC(88651ee1) SHA1(2052e1b3f9784439369f464e31f4a2b0d1bb0565) )	/* vr.7f */
	ROM_LOAD( "7_c6.bin",     0x6000, 0x2000, CRC(6d51521e) SHA1(2809bd2e61f40dcd31d43c62520982bdcfb0a865) )	/* vr.7h */
	ROM_LOAD( "1_a6.bin",     0x8000, 0x1000, CRC(20f2207e) SHA1(b1ed2237d0bd50ddbe593fd2fbff9f1d67c1eb11) )	/* vr.7k */

	ROM_REGION( 0x06000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "vr.5a",        0x0000, 0x2000, CRC(4ff1783d) SHA1(c45074864c3a4bcbf3a87d164027ae16dca53d9c) )
	ROM_LOAD( "vr.5b",        0x2000, 0x2000, CRC(5605b60b) SHA1(19d5909896ae4a3d7552225c369d30475c56793b) )
	ROM_LOAD( "vr.5c",        0x4000, 0x2000, CRC(1fb84ee6) SHA1(56ceb86c509be783f806403ac21e7c9684760d5f) )

	ROM_REGION( 0x02000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "vr.5k",        0x0000, 0x1000, CRC(1d37edfa) SHA1(184fa6dd7b1b3fff4c5fc19b42301ccb7979ac84) )
	ROM_LOAD( "vr.5l",        0x1000, 0x1000, CRC(b97f396c) SHA1(4ffe512acf047230bd593911a615fc0ef66b481d) )

	ROM_REGION( 0x0220, REGION_PROMS, 0 )
	ROM_LOAD( "prom1.e1",     0x0000, 0x0020, CRC(d9b10bf0) SHA1(bc1263331968f4bf37eb70ec4f56a8cb763c29d2) ) /* palette */
	ROM_LOAD( "prom2.8r",     0x0020, 0x0100, CRC(8a9c0edf) SHA1(8aad387e9409cff0eeb42eeb57e9ea88770a8c9a) ) /* lookup table */
	ROM_LOAD( "prom3.k5",     0x0120, 0x0100, CRC(b5db1c2c) SHA1(900aaaac6b674a9c5c7b7804a4b0c3d5cce761aa) ) /* lookup table */
ROM_END

/* The games need a different MCU control */

static DRIVER_INIT( exctsccr )
{
#if MCU_HACK
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa006, 0xa006, 0, 0, exctsccr_mcu_control_w);
#else
	mcu_patch_data = exctsccr_mcu_patch_data;
#endif
}

static DRIVER_INIT( exctscc2 )
{
#if MCU_HACK
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xa006, 0xa006, 0, 0, exctscc2_mcu_control_w);
#else
	mcu_patch_data = exctscc2_mcu_patch_data;
#endif
}

GAME( 1983, exctsccr, 0,        exctsccr, exctsccr, exctsccr, ROT90, "Alpha Denshi Co.", "Exciting Soccer", 0 )
GAME( 1983, exctscca, exctsccr, exctsccr, exctsccr, exctsccr, ROT90, "Alpha Denshi Co.", "Exciting Soccer (alternate music)", 0 )
GAME( 1983, exctsccb, exctsccr, exctsccb, exctsccr, 0,        ROT90, "bootleg",          "Exciting Soccer (bootleg)", 0 )
GAME( 1984, exctscc2, 0       , exctsccr, exctsccr, exctscc2, ROT90, "Alpha Denshi Co.", "Exciting Soccer II", GAME_IMPERFECT_GRAPHICS )
