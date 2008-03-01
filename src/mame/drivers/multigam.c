/*
    Multi Game

    driver by Mariusz Wojcieszek
    uses NES emulation by Brad Olivier

    Hardware is based around Nintendo NES. After coin up, it allows to play one of
    35 NES games (choosen from text menu). Manufacturer is unknown, code for title
    screen and game selection contains string:
    "Programed by SANG  HO  ENG. K. B. KIM 1986/1/ 1"

    Notes:
    - rom at 0x6000-0x7fff contains control program, which handles coins and timer
    - each game rom has NMI (VBLANK) vector changed to 0x6100 (control program). Then,
      game NMI is called through IRQ vector
    - there is additional RAM (0x800-0xfff) used by control program
    - ROM banked at 0x5000-0x5ffe seems to contain patches for games. At least
      Ninja code jumps into this area
    - game roms are banked in 16KB units. Bank switching of game rom is handled by
      writing to 0x6fff. If bank select has 7th bit turned on, then 32KB rom is
      switched into $8000 - $ffff. Otherwise, 16KB rom is switched on.
    - PPU VROMS are banked in 8KB units. Write to 0x7fff controls PPU VROM banking.
    - some games use additional banking on PPU VROMS. This is enabled by writing
      value with bit 7 on to 0x7fff and then selecting a vrom by writing into
      0x8000 - 0xffff range.
    - there is a register at 0x5002. 0xff is written there when game is played,
      0x00 otherwise. Its purpose is unknown.


    Notes by Roberto Fresca:

    - Added multigame (set2).
    - Main differences are:
         - Control program: Offset 0x79b4 = JMP $c4c4 was changed to JMP $c4c6.
         - ROM at 0x80000:  Several things patched and/or relocated.
                            Most original NOPs were replaced by proper code.

*/

#include "driver.h"
#include "deprecat.h"
#include "video/ppu2c0x.h"
#include "sound/nes_apu.h"
#include "cpu/m6502/m6502.h"

static WRITE8_HANDLER( sprite_dma_w )
{
	int source = ( data & 7 );
	ppu2c0x_spriteram_dma( 0, source );
}

static READ8_HANDLER( psg_4015_r )
{
	return NESPSG_0_r(0x15);
}

static WRITE8_HANDLER( psg_4015_w )
{
	NESPSG_0_w(0x15, data);
}

static WRITE8_HANDLER( psg_4017_w )
{
	NESPSG_0_w(0x17, data);
}

/* Inputs */

static UINT32 in_0;
static UINT32 in_1;
static UINT32 in_0_shift;
static UINT32 in_1_shift;
static UINT32 multigam_in_dsw;
static UINT32 multigam_in_dsw_shift;

static READ8_HANDLER( multigam_IN0_r )
{
	return ((in_0 >> in_0_shift++) & 0x01) | 0x40;
}

static WRITE8_HANDLER( multigam_IN0_w )
{
	if ( data & 0x01 )
	{
		return;
	}

	in_0_shift = 0;
	in_1_shift = 0;

	in_0 = readinputport(0);
	in_1 = readinputport(1);

	multigam_in_dsw_shift = 0;
	multigam_in_dsw = readinputport(3);
}

static READ8_HANDLER( multigam_IN1_r )
{
	return ((in_1 >> in_1_shift++) & 0x01) | 0x40;
}

static READ8_HANDLER( multigam_inputs_r )
{
	/* bit 0: serial input (dsw)
       bit 1: coin */
	return ((multigam_in_dsw >> multigam_in_dsw_shift++) & 0x01) |
			readinputport(2);
}


/*PRG and VROM Banking */

static int multigam_game_gfx_bank = 0;

static WRITE8_HANDLER( multigam_mapper_w )
{
	if ( offset == 0x0fff )
	{
		/* switch PRG rom */
		UINT8* dst = memory_region( REGION_CPU1 );
		UINT8* src = memory_region( REGION_USER1 );

		if ( data & 0x80 )
		{
			if ( data & 0x01 )
			{
				data &= ~0x01;
			}
			memcpy( &dst[0x8000], &src[(data & 0x7f) * 0x4000], 0x8000 );
		}
		else
		{
			memcpy( &dst[0x8000], &src[data*0x4000], 0x4000 );
			memcpy( &dst[0xc000], &src[data*0x4000], 0x4000 );
		}
	}
	else if ( offset == 0x1fff )
	{
		ppu2c0x_set_videorom_bank( 0, 0, 8, data /*& 0x3f*/, 512 );
		ppu2c0x_set_mirroring( 0, data & 0x40 ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT );
		multigam_game_gfx_bank = data;
	}
	else
	{
		logerror( "Unmapped multigam_mapper_w: offset = %04X, data = %02X\n", offset, data );
	}
}

static WRITE8_HANDLER(multigam_mapper2_w)
{
	if ( multigam_game_gfx_bank & 0x80 )
	{
		ppu2c0x_set_videorom_bank( 0, 0, 8, multigam_game_gfx_bank + (data & 0xf), 512 );
	}
	else
	{
		logerror( "Unmapped multigam_mapper2_w: offset = %04X, data = %02X\n", offset, data );
	}
}

static ADDRESS_MAP_START( multigam_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM	/* NES RAM */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* additional RAM */
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(ppu2c0x_0_r, ppu2c0x_0_w)
	AM_RANGE(0x4000, 0x4013) AM_READWRITE(NESPSG_0_r, NESPSG_0_w)			/* PSG primary registers */
	AM_RANGE(0x4014, 0x4014) AM_WRITE(sprite_dma_w)
	AM_RANGE(0x4015, 0x4015) AM_READWRITE(psg_4015_r, psg_4015_w)			/* PSG status / first control register */
	AM_RANGE(0x4016, 0x4016) AM_READWRITE(multigam_IN0_r,        multigam_IN0_w)			/* IN0 - input port 1 */
	AM_RANGE(0x4017, 0x4017) AM_READWRITE(multigam_IN1_r,        psg_4017_w)		/* IN1 - input port 2 / PSG second control register */
	AM_RANGE(0x5002, 0x5002) AM_WRITENOP
	AM_RANGE(0x5000, 0x5ffe) AM_ROM
	AM_RANGE(0x5fff, 0x5fff) AM_READ(multigam_inputs_r)
	AM_RANGE(0x6000, 0x7fff) AM_ROM AM_WRITE(multigam_mapper_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_WRITE(multigam_mapper2_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( multigam )
	PORT_START /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)

	PORT_START /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)	/* Select */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START /* IN2 */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START /* IN3 */
	PORT_DIPNAME( 0x06, 0x00, "Coin/Time" )
	PORT_DIPSETTING( 0x00, "3 min" )
	PORT_DIPSETTING( 0x04, "5 min" )
	PORT_DIPSETTING( 0x02, "7 min" )
	PORT_DIPSETTING( 0x06, "10 min" )
INPUT_PORTS_END

static const struct NESinterface multigam_interface_1 =
{
	REGION_CPU1
};

static MACHINE_RESET( multigam )
{
	/* reset the ppu */
	ppu2c0x_reset( 0, 1 );
}

static PALETTE_INIT( multigam )
{
	ppu2c0x_init_palette(machine, 0 );
}

static void ppu_irq( int num, int *ppu_regs )
{
	cpunum_set_input_line(Machine, num, INPUT_LINE_NMI, PULSE_LINE );
}

/* our ppu interface                                            */
static const ppu2c0x_interface ppu_interface =
{
	PPU_2C04,				/* type */
	1,						/* num */
	{ REGION_GFX1 },		/* vrom gfx region */
	{ 0 },					/* gfxlayout num */
	{ 0 },					/* color base */
	{ PPU_MIRROR_NONE },	/* mirroring */
	{ ppu_irq }				/* irq */
};

static VIDEO_START( multigam )
{
	ppu2c0x_init(machine, &ppu_interface );
}

static VIDEO_UPDATE( multigam )
{
	/* render the ppu */
	ppu2c0x_render( 0, bitmap, 0, 0, 0, 0 );
	return 0;
}

static DRIVER_INIT( multigam )
{
	multigam_mapper_w( 0x0fff, 0x01 );
}

static GFXDECODE_START( multigam )
	/* none, the ppu generates one */
GFXDECODE_END

static MACHINE_DRIVER_START( multigam )
	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", N2A03, N2A03_DEFAULTCLOCK)
	MDRV_CPU_PROGRAM_MAP(multigam_map, 0)

	MDRV_MACHINE_RESET( multigam )

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 262)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(multigam)
	MDRV_PALETTE_LENGTH(8*4*16)

	MDRV_PALETTE_INIT(multigam)
	MDRV_VIDEO_START(multigam)
	MDRV_VIDEO_UPDATE(multigam)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(NES, N2A03_DEFAULTCLOCK)
	MDRV_SOUND_CONFIG(multigam_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

ROM_START( multigam )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "7.bin", 0x0000, 0x8000, CRC(f0fa7cf2) SHA1(7f3b3dca796b964893197aef7f0f31dfd7a2c1a4) )

	ROM_REGION( 0xC0000, REGION_USER1, 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x20000, CRC(e0bb14a5) SHA1(74026f59dfb08456183adaaf381bb28830212a1c) )
	ROM_LOAD( "2.bin", 0x20000, 0x20000, CRC(f52c07ad) SHA1(51be288bcf5aeab5bdd95ee93a6d807867e30e97) )
	ROM_LOAD( "3.bin", 0x40000, 0x20000, CRC(92e8303e) SHA1(4a735fce22cdea65801aa7e4e00fa8c15a022ea4) )
	ROM_LOAD( "4.bin", 0x60000, 0x20000, CRC(f5c50f29) SHA1(6f3bd969d9d82a0d92aa6119375a607ccdb5d8b9) )
	ROM_LOAD( "5.bin", 0x80000, 0x20000, CRC(e1232243) SHA1(fa864b255b7f3cb195d3789f8a2a7b3848b255a2) )
	ROM_LOAD( "6.bin", 0xa0000, 0x20000, CRC(fb129b09) SHA1(b677937d6cf7800359dc6d35dd2de3ec27919d71) )

	ROM_REGION( 0x80000, REGION_GFX1, 0 )
	ROM_LOAD( "8.bin", 0x00000, 0x20000, CRC(509c0e94) SHA1(9e87c74e76afe6c3a7ba194439434f54e2c879eb) )
	ROM_LOAD( "9.bin", 0x20000, 0x20000, CRC(48416f20) SHA1(f461fcbff6d2fa4774d64c26475089d1aeea7fb5) )
	ROM_LOAD( "10.bin", 0x40000, 0x20000, CRC(869d1661) SHA1(ac155bd24ebfea8e064859e1a05317001286f9ae) )
	ROM_FILL( 0x60000, 0x20000, 0x00 )
ROM_END

ROM_START( multigmb )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "mg_4.bin", 0x0000, 0x8000, CRC(079226c1) SHA1(f4ceedd9b6cc3454550be7421dc75904ad664545) )

	ROM_REGION( 0xC0000, REGION_USER1, 0 )
	ROM_LOAD( "1.bin",    0x00000, 0x20000, CRC(e0bb14a5) SHA1(74026f59dfb08456183adaaf381bb28830212a1c) )
	ROM_LOAD( "2.bin",    0x20000, 0x20000, CRC(f52c07ad) SHA1(51be288bcf5aeab5bdd95ee93a6d807867e30e97) )
	ROM_LOAD( "3.bin",    0x40000, 0x20000, CRC(92e8303e) SHA1(4a735fce22cdea65801aa7e4e00fa8c15a022ea4) )
	ROM_LOAD( "4.bin",    0x60000, 0x20000, CRC(f5c50f29) SHA1(6f3bd969d9d82a0d92aa6119375a607ccdb5d8b9) )
	ROM_LOAD( "mg_9.bin", 0x80000, 0x20000, CRC(47f968af) SHA1(ed60bda060984e5acc2c6b6cac5b36eafbb2b631) )
	ROM_LOAD( "6.bin",    0xa0000, 0x20000, CRC(fb129b09) SHA1(b677937d6cf7800359dc6d35dd2de3ec27919d71) )

	ROM_REGION( 0x80000, REGION_GFX1, 0 )
	ROM_LOAD( "8.bin", 0x00000, 0x20000, CRC(509c0e94) SHA1(9e87c74e76afe6c3a7ba194439434f54e2c879eb) )
	ROM_LOAD( "9.bin", 0x20000, 0x20000, CRC(48416f20) SHA1(f461fcbff6d2fa4774d64c26475089d1aeea7fb5) )
	ROM_LOAD( "10.bin", 0x40000, 0x20000, CRC(869d1661) SHA1(ac155bd24ebfea8e064859e1a05317001286f9ae) )
	ROM_FILL( 0x60000, 0x20000, 0x00 )
ROM_END


GAME( 1992, multigam, 0,        multigam, multigam, multigam, ROT0, "unknown", "Multi Game (set 1)", 0 )
GAME( 1992, multigmb, multigam, multigam, multigam, multigam, ROT0, "unknown", "Multi Game (set 2)", 0 )
