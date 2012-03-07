/***************************************************************************

Side Pocket - (c) 1986 Data East

The original board has an 8751 protection mcu

Ernesto Corvi
ernesto@imagina.com

Thanks must go to Mirko Buffoni for testing the music.

i8751 protection simluation and other fixes by Bryan McPhail, 15/10/00.

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "includes/sidepckt.h"


static WRITE8_HANDLER( sound_cpu_command_w )
{
	soundlatch_w(space, offset, data);
	cputag_set_input_line(space->machine(), "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}

static READ8_HANDLER( sidepckt_i8751_r )
{
	sidepckt_state *state = space->machine().driver_data<sidepckt_state>();
	return state->m_i8751_return;
}

static WRITE8_HANDLER( sidepckt_i8751_w )
{
	sidepckt_state *state = space->machine().driver_data<sidepckt_state>();
	static const int table_1[]={5,3,2};
	static const int table_2[]={0x8e,0x42,0xad,0x58,0xec,0x85,0xdd,0x4c,0xad,0x9f,0x00,0x4c,0x7e,0x42,0xa2,0xff};
	static const int table_3[]={0xbd,0x73,0x80,0xbd,0x73,0xa7,0xbd,0x73,0xe0,0x7e,0x72,0x56,0xff,0xff,0xff,0xff};

	cputag_set_input_line(space->machine(), "maincpu", M6809_FIRQ_LINE, HOLD_LINE); /* i8751 triggers FIRQ on main cpu */

	/* This function takes multiple parameters */
	if (state->m_in_math==1) {
		state->m_in_math=2;
		state->m_i8751_return=state->m_math_param=data;
	}
	else if (state->m_in_math==2) {
		state->m_in_math=0;
		state->m_i8751_return=state->m_math_param/data;
	}
	else switch (data) {
		case 1: /* ID Check */
			state->m_current_table=1; state->m_current_ptr=0; state->m_i8751_return=table_1[state->m_current_ptr++]; break;

		case 2: /* Protection data (executable code) */
			state->m_current_table=2; state->m_current_ptr=0; state->m_i8751_return=table_2[state->m_current_ptr++]; break;

		case 3: /* Protection data (executable code) */
			state->m_current_table=3; state->m_current_ptr=0; state->m_i8751_return=table_3[state->m_current_ptr++]; break;

		case 4: /* Divide function - multiple parameters */
			state->m_in_math=1;
			state->m_i8751_return=4;
			break;

		case 6: /* Read table data */
			if (state->m_current_table==1) state->m_i8751_return=table_1[state->m_current_ptr++];
			if (state->m_current_table==2) state->m_i8751_return=table_2[state->m_current_ptr++];
			if (state->m_current_table==3) state->m_i8751_return=table_3[state->m_current_ptr++];
			break;
	}
}

static WRITE8_HANDLER( sidepctj_i8751_w )
{
	sidepckt_state *state = space->machine().driver_data<sidepckt_state>();
	static const int table_1[]={5,3,0};
	static const int table_2[]={0x8e,0x42,0xb2,0x58,0xec,0x85,0xdd,0x4c,0xad,0x9f,0x00,0x4c,0x7e,0x42,0xa7,0xff};
	static const int table_3[]={0xbd,0x71,0xc8,0xbd,0x71,0xef,0xbd,0x72,0x28,0x7e,0x70,0x9e,0xff,0xff,0xff,0xff};

	cputag_set_input_line(space->machine(), "maincpu", M6809_FIRQ_LINE, HOLD_LINE); /* i8751 triggers FIRQ on main cpu */

	/* This function takes multiple parameters */
	if (state->m_in_math==1) {
		state->m_in_math=2;
		state->m_i8751_return=state->m_math_param=data;
	}
	else if (state->m_in_math==2) {
		state->m_in_math=0;
		state->m_i8751_return=state->m_math_param/data;
	}
	else switch (data) {
		case 1: /* ID Check */
			state->m_current_table=1; state->m_current_ptr=0; state->m_i8751_return=table_1[state->m_current_ptr++]; break;

		case 2: /* Protection data */
			state->m_current_table=2; state->m_current_ptr=0; state->m_i8751_return=table_2[state->m_current_ptr++]; break;

		case 3: /* Protection data (executable code) */
			state->m_current_table=3; state->m_current_ptr=0; state->m_i8751_return=table_3[state->m_current_ptr++]; break;

		case 4: /* Divide function - multiple parameters */
			state->m_in_math=1;
			state->m_i8751_return=4;
			break;

		case 6: /* Read table data */
			if (state->m_current_table==1) state->m_i8751_return=table_1[state->m_current_ptr++];
			if (state->m_current_table==2) state->m_i8751_return=table_2[state->m_current_ptr++];
			if (state->m_current_table==3) state->m_i8751_return=table_3[state->m_current_ptr++];
			break;
	}
}

/******************************************************************************/

static ADDRESS_MAP_START( sidepckt_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(sidepckt_videoram_w) AM_BASE_MEMBER(sidepckt_state,m_videoram) AM_SIZE_MEMBER(sidepckt_state,m_videoram_size)
	AM_RANGE(0x1400, 0x17ff) AM_RAM // ???
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(sidepckt_colorram_w) AM_BASE_MEMBER(sidepckt_state,m_colorram)
	AM_RANGE(0x1c00, 0x1fff) AM_RAM // ???
	AM_RANGE(0x2000, 0x20ff) AM_RAM AM_BASE_MEMBER(sidepckt_state,m_spriteram) AM_SIZE_MEMBER(sidepckt_state,m_spriteram_size)
	AM_RANGE(0x2100, 0x24ff) AM_RAM // ???
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("P1")
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("P2")
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("DSW1")
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("DSW2")
	AM_RANGE(0x3004, 0x3004) AM_WRITE(sound_cpu_command_w)
	AM_RANGE(0x300c, 0x300c) AM_READNOP AM_WRITE(sidepckt_flipscreen_w)
//  AM_RANGE(0x3014, 0x3014) //i8751 read
//  AM_RANGE(0x3018, 0x3018) //i8751 write
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ym1", ym2203_w)
	AM_RANGE(0x2000, 0x2001) AM_DEVWRITE("ym2", ym3526_w)
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/******************************************************************************/

static INPUT_PORTS_START( sidepckt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	/* I haven't found a way to make the game use the 2p controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, "Unused?" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unused?" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unused?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Timer Speed" )
	PORT_DIPSETTING(    0x00, "Stopped (Cheat)")
	PORT_DIPSETTING(    0x03, "Slow" )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, "Fast" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_DIPNAME( 0x80, 0x80, "Unused?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
    8,8,    /* 8*8 characters */
    2048,   /* 2048 characters */
    3,      /* 3 bits per pixel */
    { 0, 0x8000*8, 0x10000*8 },     /* the bitplanes are separated */
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
    16,16,  /* 16*16 sprites */
    1024,   /* 1024 sprites */
    3,      /* 3 bits per pixel */
    { 0, 0x8000*8, 0x10000*8 },     /* the bitplanes are separated */
    { 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
    32*8    /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( sidepckt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   128,  4 )	/* colors 128-159 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,   0, 16 )	/* colors   0-127 */
GFXDECODE_END


static const ym3526_interface ym3526_config =
{
	DEVCB_CPU_INPUT_LINE("audiocpu", M6502_IRQ_LINE)
};



static MACHINE_CONFIG_START( sidepckt, sidepckt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 2000000)        /* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(sidepckt_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_CPU_ADD("audiocpu", M6502, 1500000)        /* 1.5 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)
								/* NMIs are triggered by the main cpu */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */  /* VERIFY:  May be 55 or 56 */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(sidepckt)

	MCFG_GFXDECODE(sidepckt)
	MCFG_PALETTE_LENGTH(256)

	MCFG_PALETTE_INIT(sidepckt)
	MCFG_VIDEO_START(sidepckt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ym2", YM3526, 3000000)
	MCFG_SOUND_CONFIG(ym3526_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sidepckt )
    ROM_REGION( 0x10000, "maincpu", 0 )
    ROM_LOAD( "dh00",         0x00000, 0x10000, CRC(251b316e) SHA1(c777d87621b8fefe0e33156be03da8aed733db9a) )

    ROM_REGION( 0x10000, "audiocpu", 0 )
    ROM_LOAD( "dh04.bin",     0x08000, 0x8000, CRC(d076e62e) SHA1(720ff1a6a58697b4a9c7c4f31c24a2cf8a04900a) )

    ROM_REGION( 0x10000, "mcu", 0 ) //i8751 MCU
    ROM_LOAD( "i8751.mcu",     0x00000, 0x8000, NO_DUMP )

    ROM_REGION( 0x18000, "gfx1", 0 )
    ROM_LOAD( "sp_07.bin",    0x00000, 0x8000, CRC(9d6f7969) SHA1(583852be0861a89c63ce09eb39146ec379b9e12d) ) /* characters */
    ROM_LOAD( "sp_06.bin",    0x08000, 0x8000, CRC(580e4e43) SHA1(de152a5d4fbc52d80e3eb9af17835ecb6258d45e) )
    ROM_LOAD( "sp_05.bin",    0x10000, 0x8000, CRC(05ab71d2) SHA1(6f06d1d1440a5fb05c01f712457d0bb167e93099) )

    ROM_REGION( 0x18000, "gfx2", 0 )
    ROM_LOAD( "dh01.bin",     0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) ) /* sprites */
    ROM_LOAD( "dh02.bin",     0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
    ROM_LOAD( "dh03.bin",     0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

    ROM_REGION( 0x0200, "proms", 0 )	/* color PROMs */
    ROM_LOAD( "dh-09.bpr",    0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) )
    ROM_LOAD( "dh-08.bpr",    0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) )
ROM_END

ROM_START( sidepcktj )
    ROM_REGION( 0x10000, "maincpu", 0 )
    ROM_LOAD( "dh00.bin",     0x00000, 0x10000, CRC(a66bc28d) SHA1(cd62ce1dce6fe42d9745eec50d11e86b076d28e1) )

    ROM_REGION( 0x10000, "audiocpu", 0 )
    ROM_LOAD( "dh04.bin",     0x08000, 0x8000, CRC(d076e62e) SHA1(720ff1a6a58697b4a9c7c4f31c24a2cf8a04900a) )

    ROM_REGION( 0x10000, "mcu", 0 ) //i8751 MCU
    ROM_LOAD( "i8751.mcu",     0x00000, 0x8000, NO_DUMP )

    ROM_REGION( 0x18000, "gfx1", 0 )
    ROM_LOAD( "dh07.bin",     0x00000, 0x8000, CRC(7d0ce858) SHA1(3a158f218a762e6841d2611f41ace67a1afefb35) ) /* characters */
    ROM_LOAD( "dh06.bin",     0x08000, 0x8000, CRC(b86ddf72) SHA1(7596dd1b646971d8df1bc4fd157ccf161a712d59) )
    ROM_LOAD( "dh05.bin",     0x10000, 0x8000, CRC(df6f94f2) SHA1(605796191f37cb76d496aa459243655070bb90c0) )

    ROM_REGION( 0x18000, "gfx2", 0 )
    ROM_LOAD( "dh01.bin",     0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) ) /* sprites */
    ROM_LOAD( "dh02.bin",     0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
    ROM_LOAD( "dh03.bin",     0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

    ROM_REGION( 0x0200, "proms", 0 )	/* color PROMs */
    ROM_LOAD( "dh-09.bpr",    0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) )
    ROM_LOAD( "dh-08.bpr",    0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) )
ROM_END

ROM_START( sidepcktb )
    ROM_REGION( 0x10000, "maincpu", 0 )
    ROM_LOAD( "sp_09.bin",    0x04000, 0x4000, CRC(3c6fe54b) SHA1(4025ac48d75f171f4c979d3fcd6a2f8da18cef4f) )
    ROM_LOAD( "sp_08.bin",    0x08000, 0x8000, CRC(347f81cd) SHA1(5ab06130f35788e51a881cc0f387649532145bd6) )

    ROM_REGION( 0x10000, "audiocpu", 0 )
    ROM_LOAD( "dh04.bin",     0x08000, 0x8000, CRC(d076e62e) SHA1(720ff1a6a58697b4a9c7c4f31c24a2cf8a04900a) )

    ROM_REGION( 0x18000, "gfx1", 0 )
    ROM_LOAD( "sp_07.bin",    0x00000, 0x8000, CRC(9d6f7969) SHA1(583852be0861a89c63ce09eb39146ec379b9e12d) ) /* characters */
    ROM_LOAD( "sp_06.bin",    0x08000, 0x8000, CRC(580e4e43) SHA1(de152a5d4fbc52d80e3eb9af17835ecb6258d45e) )
    ROM_LOAD( "sp_05.bin",    0x10000, 0x8000, CRC(05ab71d2) SHA1(6f06d1d1440a5fb05c01f712457d0bb167e93099) )

    ROM_REGION( 0x18000, "gfx2", 0 )
    ROM_LOAD( "dh01.bin",     0x00000, 0x8000, CRC(a2cdfbea) SHA1(0721e538e3306d616f11008f784cf21e679f330d) ) /* sprites */
    ROM_LOAD( "dh02.bin",     0x08000, 0x8000, CRC(eeb5c3e7) SHA1(57eda1cc29124e04fe5025a904634d8ca52c0f12) )
    ROM_LOAD( "dh03.bin",     0x10000, 0x8000, CRC(8e18d21d) SHA1(74f0ddf1fcbed386332eba882b4136295b4f096d) )

    ROM_REGION( 0x0200, "proms", 0 )	/* color PROMs */
    ROM_LOAD( "dh-09.bpr",    0x0000, 0x0100, CRC(ce049b4f) SHA1(e4918cef7b319dd40cf1722eb8bf5e79be04fd6c) )
    ROM_LOAD( "dh-08.bpr",    0x0100, 0x0100, CRC(cdf2180f) SHA1(123215d096f88b66396d40d7a579380d0b5b2b89) )
ROM_END


static DRIVER_INIT( sidepckt )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x3014, 0x3014, FUNC(sidepckt_i8751_r) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x3018, 0x3018, FUNC(sidepckt_i8751_w)  );
}

static DRIVER_INIT( sidepctj )
{
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x3014, 0x3014, FUNC(sidepckt_i8751_r) );
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x3018, 0x3018, FUNC(sidepctj_i8751_w)  );
}


GAME( 1986, sidepckt, 0,        sidepckt, sidepckt, sidepckt, ROT0, "Data East Corporation", "Side Pocket (World)", 0 )
GAME( 1986, sidepcktj,sidepckt, sidepckt, sidepckt, sidepctj, ROT0, "Data East Corporation", "Side Pocket (Japan)", 0 )
GAME( 1986, sidepcktb,sidepckt, sidepckt, sidepckt, 0,        ROT0, "bootleg", "Side Pocket (bootleg)", 0 )
