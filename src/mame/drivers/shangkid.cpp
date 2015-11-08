// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*
Dynamic Ski
(c)1984 Taiyo

Dynamic Ski runs on a single Z80.  It has the same graphics format as the
newer Taiyo games.

The game has some minor priority glitches.

---------------------------------------------------------------------------

Chinese Hero (developed by Taiyo)
(c)1984 Taiyo

Chinese Hero hardware differs only slightly from Shanghai Kid:
- sprites have 3 bitplanes instead of 2
- videoram attributes for the tilemap don't include xflip
- no protection

---------------------------------------------------------------------------

Shanghai Kid / (Hokuha Syourin) Hiryu no Ken
(c)1985 Nihon Game (distributed by Taito)

    3 Z-80A CPU
    1 AY-3-8910
    1 XTAL 18.432MHz

Also distributed with Data East and Memetron license.

Two board set CPU/sound & video.

There is a 1.5" by 2" by 4" black epoxy block that has an external battery.
The block is connected to the PCB by a 40 pin DIP socket labeled IC30.
There is a small smt IC on the video board with the numbers ground off.

---------------------------------------------------------------------------

Some company history:

Nihon Game changed their name to Culture Brain.

Games by Nihon Game/Culture Brain:
    1982 Monster Zero
    1983 Space Hunter
    1984 Chinese Hero
    1985 Hokuha Syourin Hiryuu no Ken / Shanghai Kid
    1986 Super Chinese (Nintendo Vs. System)
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "includes/shangkid.h"

/***************************************************************************************/

WRITE8_MEMBER(shangkid_state::maincpu_bank_w)
{
	membank("bank1")->set_entry(data & 1);
}

WRITE8_MEMBER(shangkid_state::bbx_enable_w)
{
	m_bbx->set_input_line(INPUT_LINE_HALT, data?0:1 );
}

WRITE8_MEMBER(shangkid_state::cpu_reset_w)
{
	if( data == 0 )
	{
		m_bbx->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	}
	else if( data == 1 )
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	}
}

WRITE8_MEMBER(shangkid_state::sound_enable_w)
{
	m_bbx_sound_enable = data;
}

WRITE8_MEMBER(shangkid_state::chinhero_ay8910_porta_w)
{
	if( m_bbx_sound_enable )
	{
		if( data == 0x01 )
			/* 0->1 transition triggers interrupt on Sound CPU */
			m_audiocpu->set_input_line(0, HOLD_LINE );
	}
}

WRITE8_MEMBER(shangkid_state::shangkid_ay8910_porta_w)
{
	if( m_bbx_sound_enable )
	{
		if( data == 0x01 )
			/* 0->1 transition triggers interrupt on Sound CPU */
			m_audiocpu->set_input_line(0, HOLD_LINE );
	}
	else
		membank("bank2")->set_entry(data ? 0 : 1);
}

WRITE8_MEMBER(shangkid_state::ay8910_portb_w)
{
	m_sound_latch = data;
}

/***************************************************************************************/

READ8_MEMBER(shangkid_state::soundlatch_r)
{
	return m_sound_latch;
}

/***************************************************************************************/

DRIVER_INIT_MEMBER(shangkid_state,chinhero)
{
	m_gfx_type = 0;

	save_item(NAME(m_bbx_sound_enable));
	save_item(NAME(m_sound_latch));
}

DRIVER_INIT_MEMBER(shangkid_state,shangkid)
{
	m_gfx_type = 1;

	/* set up banking */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x8000, 0x8000);
	membank("bank2")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x0000, 0x10000);

	save_item(NAME(m_bbx_sound_enable));
	save_item(NAME(m_sound_latch));
}

/***************************************************************************************/

MACHINE_RESET_MEMBER(shangkid_state,chinhero)
{
	m_bbx->set_input_line(INPUT_LINE_HALT, 1 );
}

MACHINE_RESET_MEMBER(shangkid_state,shangkid)
{
	m_bbx->set_input_line(INPUT_LINE_HALT, 1 );

	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
}

/***************************************************************************************/

static const gfx_layout shangkid_char_layout = {
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16 },
	8*16
};

static const gfx_layout shangkid_sprite_layout = {
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0,4 },
	{
		0,1,2,3,8,9,10,11,
		128+0,128+1,128+2,128+3,128+8,128+9,128+10,128+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		256+0*16,256+1*16,256+2*16,256+3*16,256+4*16,256+5*16,256+6*16,256+7*16
	},
	8*0x40
};

static const gfx_layout chinhero_sprite_layout1 = {
	16,16,
	0x80,
	3,
	{ 0x4000*8+4,0,4 },
	{
		0,1,2,3,8,9,10,11,
		128+0,128+1,128+2,128+3,128+8,128+9,128+10,128+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		256+0*16,256+1*16,256+2*16,256+3*16,256+4*16,256+5*16,256+6*16,256+7*16
	},
	8*0x40
};

static const gfx_layout chinhero_sprite_layout2 = {
	16,16,
	0x80,
	3,
	{ 0x4000*8,0x2000*8+0,0x2000*8+4 },
	{
		0,1,2,3,8,9,10,11,
		128+0,128+1,128+2,128+3,128+8,128+9,128+10,128+11
	},
	{
		0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
		256+0*16,256+1*16,256+2*16,256+3*16,256+4*16,256+5*16,256+6*16,256+7*16
	},
	8*0x40
};

static GFXDECODE_START( chinhero )
	GFXDECODE_ENTRY( "gfx1", 0, shangkid_char_layout,   0, 0x40 )
	GFXDECODE_ENTRY( "gfx2", 0, chinhero_sprite_layout1,    0, 0x20 )
	GFXDECODE_ENTRY( "gfx2", 0, chinhero_sprite_layout2,    0, 0x20 )
	GFXDECODE_ENTRY( "gfx3", 0, chinhero_sprite_layout1,    0, 0x20 )
	GFXDECODE_ENTRY( "gfx3", 0, chinhero_sprite_layout2,    0, 0x20 )
GFXDECODE_END

static GFXDECODE_START( shangkid )
	GFXDECODE_ENTRY( "gfx1", 0, shangkid_char_layout,   0, 0x40 )
	GFXDECODE_ENTRY( "gfx2", 0, shangkid_sprite_layout, 0, 0x40 )
GFXDECODE_END

static GFXDECODE_START( dynamski )
	GFXDECODE_ENTRY( "gfx1", 0, shangkid_char_layout,      0, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, shangkid_sprite_layout, 0x40, 0x10 )
GFXDECODE_END

/***************************************************************************************/

static ADDRESS_MAP_START( chinhero_main_map, AS_PROGRAM, 8, shangkid_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP /* ? */
	AM_RANGE(0xb000, 0xb000) AM_WRITE(bbx_enable_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(sound_enable_w)
	AM_RANGE(0xb002, 0xb002) AM_WRITENOP        /* main CPU interrupt-related */
	AM_RANGE(0xb003, 0xb003) AM_WRITENOP        /* BBX interrupt-related */
	AM_RANGE(0xb004, 0xb004) AM_WRITE(cpu_reset_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITENOP        /* coin counter */
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("DSW")
	AM_RANGE(0xb801, 0xb801) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb802, 0xb802) AM_READ_PORT("P2")
	AM_RANGE(0xb803, 0xb803) AM_READ_PORT("P1")
	AM_RANGE(0xc000, 0xc002) AM_WRITEONLY AM_SHARE("videoreg")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xfdff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( shangkid_main_map, AS_PROGRAM, 8, shangkid_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP /* ? */
	AM_RANGE(0xb000, 0xb000) AM_WRITE(bbx_enable_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(sound_enable_w)
	AM_RANGE(0xb002, 0xb002) AM_WRITENOP        /* main CPU interrupt-related */
	AM_RANGE(0xb003, 0xb003) AM_WRITENOP        /* BBX interrupt-related */
	AM_RANGE(0xb004, 0xb004) AM_WRITE(cpu_reset_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITENOP        /* coin counter */
	AM_RANGE(0xb007, 0xb007) AM_WRITE(maincpu_bank_w)
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("DSW")
	AM_RANGE(0xb801, 0xb801) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb802, 0xb802) AM_READ_PORT("P2")
	AM_RANGE(0xb803, 0xb803) AM_READ_PORT("P1")
	AM_RANGE(0xc000, 0xc002) AM_WRITEONLY AM_SHARE("videoreg")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xfdff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

/***************************************************************************************/

static ADDRESS_MAP_START( chinhero_bbx_map, AS_PROGRAM, 8, shangkid_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP /* ? */
	AM_RANGE(0xb000, 0xb000) AM_WRITE(bbx_enable_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(sound_enable_w)
	AM_RANGE(0xb002, 0xb002) AM_WRITENOP        /* main CPU interrupt-related */
	AM_RANGE(0xb003, 0xb003) AM_WRITENOP        /* BBX interrupt-related */
	AM_RANGE(0xb004, 0xb004) AM_WRITE(cpu_reset_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITENOP        /* coin counter */
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("DSW")
	AM_RANGE(0xb801, 0xb801) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb802, 0xb802) AM_READ_PORT("P2")
	AM_RANGE(0xb803, 0xb803) AM_READ_PORT("P1")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xfdff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( shangkid_bbx_map, AS_PROGRAM, 8, shangkid_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP /* ? */
	AM_RANGE(0xb000, 0xb000) AM_WRITE(bbx_enable_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(sound_enable_w)
	AM_RANGE(0xb002, 0xb002) AM_WRITENOP        /* main CPU interrupt-related */
	AM_RANGE(0xb003, 0xb003) AM_WRITENOP        /* BBX interrupt-related */
	AM_RANGE(0xb004, 0xb004) AM_WRITE(cpu_reset_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITENOP        /* coin counter */
	AM_RANGE(0xb007, 0xb007) AM_WRITE(maincpu_bank_w)
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("DSW")
	AM_RANGE(0xb801, 0xb801) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb802, 0xb802) AM_READ_PORT("P2")
	AM_RANGE(0xb803, 0xb803) AM_READ_PORT("P1")
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xfdff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( chinhero_bbx_portmap, AS_IO, 8, shangkid_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( shangkid_bbx_portmap, AS_IO, 8, shangkid_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
ADDRESS_MAP_END

/***************************************************************************************/

static ADDRESS_MAP_START( chinhero_sound_map, AS_PROGRAM, 8, shangkid_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( shangkid_sound_map, AS_PROGRAM, 8, shangkid_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROMBANK("bank2") /* sample player writes to ROM area */
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, shangkid_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_r) AM_DEVWRITE("dac", dac_device, write_unsigned8)
ADDRESS_MAP_END

/***************************************************************************************/

static MACHINE_CONFIG_START( chinhero, shangkid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(chinhero_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", shangkid_state,  irq0_line_hold)

	MCFG_CPU_ADD("bbx", Z80, XTAL_18_432MHz/6) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(chinhero_bbx_map)
	MCFG_CPU_IO_MAP(chinhero_bbx_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", shangkid_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_18_432MHz/6) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(chinhero_sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	MCFG_MACHINE_RESET_OVERRIDE(shangkid_state,chinhero)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 28*8)
	MCFG_SCREEN_VISIBLE_AREA(16, 319-16, 0, 223)
	MCFG_SCREEN_UPDATE_DRIVER(shangkid_state, screen_update_shangkid)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", chinhero)
	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)
	MCFG_VIDEO_START_OVERRIDE(shangkid_state,shangkid)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_18_432MHz/12) /* verified on pcb */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(shangkid_state, chinhero_ay8910_porta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(shangkid_state, ay8910_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( shangkid, chinhero )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(shangkid_main_map)

	MCFG_CPU_MODIFY("bbx")
	MCFG_CPU_PROGRAM_MAP(shangkid_bbx_map)
	MCFG_CPU_IO_MAP(shangkid_bbx_portmap)

	MCFG_CPU_MODIFY("audiocpu")
	MCFG_CPU_PROGRAM_MAP(shangkid_sound_map)

	MCFG_MACHINE_RESET_OVERRIDE(shangkid_state,shangkid)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", shangkid)

	MCFG_SOUND_MODIFY("aysnd")
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(shangkid_state, shangkid_ay8910_porta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(shangkid_state, ay8910_portb_w))
MACHINE_CONFIG_END



static ADDRESS_MAP_START( dynamski_map, AS_PROGRAM, 8, shangkid_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("videoram") /* tilemap */
	AM_RANGE(0xc800, 0xcbff) AM_RAM
	AM_RANGE(0xd000, 0xd3ff) AM_RAM
	AM_RANGE(0xd800, 0xdbff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_WRITENOP /* IRQ disable */
	AM_RANGE(0xe001, 0xe002) AM_RAM /* screen flip */
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe801, 0xe801) AM_READ_PORT("P1")
	AM_RANGE(0xe802, 0xe802) AM_READ_PORT("P2")
	AM_RANGE(0xe803, 0xe803) AM_READ_PORT("DSW")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM /* work ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dynamski_portmap, AS_IO, 8, shangkid_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	/* ports are reversed */
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( dynamski, shangkid_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3000000) /* ? */
	MCFG_CPU_PROGRAM_MAP(dynamski_map)
	MCFG_CPU_IO_MAP(dynamski_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", shangkid_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256+32, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255+32, 16, 255-16)
	MCFG_SCREEN_UPDATE_DRIVER(shangkid_state, screen_update_dynamski)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dynamski)
	MCFG_PALETTE_ADD("palette", 16*4+16*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(shangkid_state,dynamski)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END

/***************************************************************************************/

static INPUT_PORTS_START( dynamski )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) /* service */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Unknown ) )
	/* what's 00 ? */
	PORT_DIPSETTING(    0x01, "A" )
	PORT_DIPSETTING(    0x02, "B" )
	PORT_DIPSETTING(    0x03, "C" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) /* unused? */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) /* unused? */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chinhero )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) /* service */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) ) /* not verified */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( shangkid )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) /* service */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) /* busy flag? */

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) /* kick */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) /* punch */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) /* kick */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) /* punch */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)

	PORT_START("DSW")
	/*  There are also two potentiometers on the PCB for volume:
	**  RV1 - Music
	**  RV2 - Sound Effects
	*/
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x1c, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Unknown ) ) /* 1C_1C; no coin counter */
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END

/***************************************************************************************/

ROM_START( chinhero )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code (main) */
	ROM_LOAD( "ic2.1",        0x0000, 0x2000, CRC(8974bac4) SHA1(932a677d0928f4146201f206b71ac2bcc0f6735c) )
	ROM_LOAD( "ic3.2",        0x2000, 0x2000, CRC(9b7a02fe) SHA1(b17593262ec24b999d66634b84eee95c1088f7eb) )
	ROM_LOAD( "ic4.3",        0x4000, 0x2000, CRC(e86d4195) SHA1(5081500e0a6d4fd19690134efd6f35b6047e6727) )
	ROM_LOAD( "ic5.4",        0x6000, 0x2000, CRC(2b629d2c) SHA1(7e92e2c2d09d3501ddbf79a14228cf273f4a17df) )
	ROM_LOAD( "ic6.5",        0x8000, 0x2000, CRC(35bf4a4f) SHA1(2600c57d40355775847eed8a9592c67f5d11f1f1) )

	ROM_REGION( 0x10000, "bbx", 0 ) /* Z80 code (coprocessor) */
	ROM_LOAD( "ic31.6",       0x0000, 0x2000, CRC(7c56927b) SHA1(565a10b39f2d5d38cb5415aadd7fbdb90dcf13cb) )
	ROM_LOAD( "ic32.7",       0x2000, 0x2000, CRC(d67b8045) SHA1(0374cafb8d4828e195791784ac187772c49c18f9) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "ic47.8",       0x0000, 0x2000, CRC(3c396062) SHA1(2c1540eb123b3124e1679ba09e063e80f2423022) )
	ROM_LOAD( "ic48.9",       0x2000, 0x2000, CRC(b14f2bab) SHA1(3643b430e3b464b0bc9aca81122b07fb8eb0fb9f) )
	ROM_LOAD( "ic49.10",      0x4000, 0x2000, CRC(8c0e43d1) SHA1(acaead801b4782875c8b6092e987b73f9973f8b0) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "ic21.11",      0x0000, 0x2000, CRC(3a37fb45) SHA1(4c631cf924f1e1dfea6db3f014ab7d9cb9f4b0c4) )
	ROM_LOAD( "ic22.12",      0x2000, 0x2000, CRC(bc21c002) SHA1(4fc5e4dfe8331a3feb1c370a8aca9c8303eb7b4e) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "ic114.18",     0x0000, 0x2000, CRC(fc4183a8) SHA1(4bc891a9e16cd84ce353180705cc8fcadf414a49) )
	ROM_LOAD( "ic113.17",     0x2000, 0x2000, CRC(d713d7fe) SHA1(8dd97f96a1190c5be5e19721227dd80adf060b4d) )
	ROM_LOAD(  "ic99.13",     0x4000, 0x2000, CRC(a8e2a3f4) SHA1(db9f954d4b46660f5f1cb4122838e6418f92d0a3) )

	ROM_REGION( 0x6000, "gfx3", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "ic112.16",     0x0000, 0x2000, CRC(dd5170ca) SHA1(e0a9d1dbc021a8ad84dd7d1bd7e390e51e6328b7) )
	ROM_LOAD( "ic111.15",     0x2000, 0x2000, CRC(20f6052e) SHA1(e22ddb3fb90ff8df5ce7fda6a26c1b9fce2f59ab) )
	ROM_LOAD( "ic110.14",     0x4000, 0x2000, CRC(9bc2d568) SHA1(a4ee8822709645b0dc088635c0a9c263fb5a2245) )

	ROM_REGION( 0xa80, "proms", 0 )
	ROM_LOAD( "v_ic36_r",     0x000, 0x100, CRC(16ae1692) SHA1(e287b96890da4815350af72e9f2189d0c72313b6) ) /* red */
	ROM_LOAD( "v_ic35_g",     0x100, 0x100, CRC(b3d0a074) SHA1(e955fda8cb8df389507e17b7b4609e845e5ef0c4) ) /* green */
	ROM_LOAD( "v_ic27_b",     0x200, 0x100, CRC(353a2d11) SHA1(76f21e3e092024592d9ccd33ae69c438254c5755) ) /* blue */

	ROM_LOAD( "v_ic28_m",     0x300, 0x100, CRC(7ca273c1) SHA1(20d85547d96bea8b310c943c45e4978a7e5b5585) ) /* unknown */
	ROM_LOAD( "v_ic69",       0x400, 0x200, CRC(410d6f86) SHA1(3cfaef3702dbda3e7c7eb84a93561e36778aec3e) ) /* zoom */
	ROM_LOAD( "v_ic108",      0x600, 0x200, CRC(d33c02ae) SHA1(1a2146ae404a5e8a701e1d547a8409a376d4bee4) ) /* zoom */

	ROM_LOAD( "v_ic12",       0x800, 0x100, CRC(0de07e89) SHA1(5655bce6ff3abad63f5b31add402cdbb51c323f0) ) /* tile pen priority */
	ROM_LOAD( "v_ic15_p",     0x900, 0x100, CRC(7e0a0581) SHA1(e355a6ef21a65a1e828d7bd5b0f2224b06438b4a) ) /* sprite pen transparency */
	ROM_LOAD( "v_ic8",        0xa00, 0x020, CRC(4c62974d) SHA1(fd5970b5ba1d9e986515ae06c2e83f8bf20b3cdc) )

	ROM_LOAD( "ic8",          0xa20, 0x020, CRC(84bcd9af) SHA1(5a5afeb6aedb8ac6ac49fb8da62df57fbd8b1780) ) /* main CPU banking */
	ROM_LOAD( "ic22",         0xa40, 0x020, CRC(84bcd9af) SHA1(5a5afeb6aedb8ac6ac49fb8da62df57fbd8b1780) ) /* coprocessor banking */
	ROM_LOAD( "ic42",         0xa60, 0x020, CRC(2ccfe10a) SHA1(d89ea91e5da436805fca9ded9b33609f4a862724) ) /* sound cpu banking */
ROM_END

ROM_START( chinhero2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code (main) */
	ROM_LOAD( "1.128",        0x0000, 0x4000, CRC(68e247aa) SHA1(27c2b864e482ba10c81337ed7c03a58b395e52bb) )
	ROM_LOAD( "2.128",        0x4000, 0x4000, CRC(0346d8c9) SHA1(458b9a37b0ad0cafecdb0348f7d93508531bc310) )
	ROM_LOAD( "3.128",        0x8000, 0x4000, CRC(a78b8d78) SHA1(c2b7b2d56e2fdb7a2a11bb8b1aab35a841331b96) )

	ROM_REGION( 0x10000, "bbx", 0 ) /* Z80 code (coprocessor) */
	ROM_LOAD( "4.128",        0x0000, 0x4000, CRC(6ab2e836) SHA1(61c84c0b685e29bac8020a0051586267ecd20166) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "5.128",        0x0000, 0x4000, CRC(4e4f3f92) SHA1(57d0485f8a0110f5448b554d2fab1caba52551fd) )
	ROM_LOAD( "ic49.10",      0x4000, 0x2000, CRC(8c0e43d1) SHA1(acaead801b4782875c8b6092e987b73f9973f8b0) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "ic21.11",      0x0000, 0x2000, CRC(3a37fb45) SHA1(4c631cf924f1e1dfea6db3f014ab7d9cb9f4b0c4) )
	ROM_LOAD( "ic22.12",      0x2000, 0x2000, CRC(bc21c002) SHA1(4fc5e4dfe8331a3feb1c370a8aca9c8303eb7b4e) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "ic114.18",     0x0000, 0x2000, CRC(fc4183a8) SHA1(4bc891a9e16cd84ce353180705cc8fcadf414a49) )
	ROM_LOAD( "ic113.17",     0x2000, 0x2000, CRC(d713d7fe) SHA1(8dd97f96a1190c5be5e19721227dd80adf060b4d) )
	ROM_LOAD(  "ic99.13",     0x4000, 0x2000, CRC(a8e2a3f4) SHA1(db9f954d4b46660f5f1cb4122838e6418f92d0a3) )

	ROM_REGION( 0x6000, "gfx3", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "ic112.16",     0x0000, 0x2000, CRC(dd5170ca) SHA1(e0a9d1dbc021a8ad84dd7d1bd7e390e51e6328b7) )
	ROM_LOAD( "ic111.15",     0x2000, 0x2000, CRC(20f6052e) SHA1(e22ddb3fb90ff8df5ce7fda6a26c1b9fce2f59ab) )
	ROM_LOAD( "ic110.14",     0x4000, 0x2000, CRC(9bc2d568) SHA1(a4ee8822709645b0dc088635c0a9c263fb5a2245) )

	ROM_REGION( 0xa80, "proms", 0 )
	ROM_LOAD( "v_ic36_r",     0x000, 0x100, CRC(16ae1692) SHA1(e287b96890da4815350af72e9f2189d0c72313b6) ) /* red */
	ROM_LOAD( "v_ic35_g",     0x100, 0x100, CRC(b3d0a074) SHA1(e955fda8cb8df389507e17b7b4609e845e5ef0c4) ) /* green */
	ROM_LOAD( "v_ic27_b",     0x200, 0x100, CRC(353a2d11) SHA1(76f21e3e092024592d9ccd33ae69c438254c5755) ) /* blue */

	ROM_LOAD( "v_ic28_m",     0x300, 0x100, CRC(7ca273c1) SHA1(20d85547d96bea8b310c943c45e4978a7e5b5585) ) /* unknown */
	ROM_LOAD( "v_ic69",       0x400, 0x200, CRC(410d6f86) SHA1(3cfaef3702dbda3e7c7eb84a93561e36778aec3e) ) /* zoom */
	ROM_LOAD( "v_ic108",      0x600, 0x200, CRC(d33c02ae) SHA1(1a2146ae404a5e8a701e1d547a8409a376d4bee4) ) /* zoom */

	ROM_LOAD( "v_ic12",       0x800, 0x100, CRC(0de07e89) SHA1(5655bce6ff3abad63f5b31add402cdbb51c323f0) ) /* tile pen priority */
	ROM_LOAD( "v_ic15_p",     0x900, 0x100, CRC(7e0a0581) SHA1(e355a6ef21a65a1e828d7bd5b0f2224b06438b4a) ) /* sprite pen transparency */
	ROM_LOAD( "v_ic8",        0xa00, 0x020, CRC(4c62974d) SHA1(fd5970b5ba1d9e986515ae06c2e83f8bf20b3cdc) )

	ROM_LOAD( "ic8",          0xa20, 0x020, CRC(84bcd9af) SHA1(5a5afeb6aedb8ac6ac49fb8da62df57fbd8b1780) ) /* main CPU banking */
	ROM_LOAD( "ic22",         0xa40, 0x020, CRC(84bcd9af) SHA1(5a5afeb6aedb8ac6ac49fb8da62df57fbd8b1780) ) /* coprocessor banking */
	ROM_LOAD( "ic42",         0xa60, 0x020, CRC(2ccfe10a) SHA1(d89ea91e5da436805fca9ded9b33609f4a862724) ) /* sound cpu banking */
ROM_END

ROM_START( chinhero3 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code (main) */
	ROM_LOAD( "1-11-22.ic2",        0x0000, 0x4000, CRC(9b24f886) SHA1(ac34c353bae68d2fa716b0dd9f5e41edcf59ad2a) )
	ROM_LOAD( "2-11-22.ic3",        0x4000, 0x4000, CRC(39c66686) SHA1(e19ef3116c848f76e516f1de08bfe4be306753ca) )
	ROM_LOAD( "3-11-22.ic4",        0x8000, 0x2000, CRC(2d51135e) SHA1(58892495ee51a43fa02f2421b1d6fe4c48d7c8a7) ) // other boards use a rom with both halves identical here instead of a smaller one.

	ROM_REGION( 0x10000, "bbx", 0 ) /* Z80 code (coprocessor) */
	ROM_LOAD( "4-11-22.ic31",        0x0000, 0x4000, CRC(6ab2e836) SHA1(61c84c0b685e29bac8020a0051586267ecd20166) )

	// nothing below was dumped on this set, assuming to be the same.

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "5.128",        0x0000, 0x4000, CRC(4e4f3f92) SHA1(57d0485f8a0110f5448b554d2fab1caba52551fd) )
	ROM_LOAD( "ic49.10",      0x4000, 0x2000, CRC(8c0e43d1) SHA1(acaead801b4782875c8b6092e987b73f9973f8b0) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "ic21.11",      0x0000, 0x2000, CRC(3a37fb45) SHA1(4c631cf924f1e1dfea6db3f014ab7d9cb9f4b0c4) )
	ROM_LOAD( "ic22.12",      0x2000, 0x2000, CRC(bc21c002) SHA1(4fc5e4dfe8331a3feb1c370a8aca9c8303eb7b4e) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "ic114.18",     0x0000, 0x2000, CRC(fc4183a8) SHA1(4bc891a9e16cd84ce353180705cc8fcadf414a49) )
	ROM_LOAD( "ic113.17",     0x2000, 0x2000, CRC(d713d7fe) SHA1(8dd97f96a1190c5be5e19721227dd80adf060b4d) )
	ROM_LOAD(  "ic99.13",     0x4000, 0x2000, CRC(a8e2a3f4) SHA1(db9f954d4b46660f5f1cb4122838e6418f92d0a3) )

	ROM_REGION( 0x6000, "gfx3", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "ic112.16",     0x0000, 0x2000, CRC(dd5170ca) SHA1(e0a9d1dbc021a8ad84dd7d1bd7e390e51e6328b7) )
	ROM_LOAD( "ic111.15",     0x2000, 0x2000, CRC(20f6052e) SHA1(e22ddb3fb90ff8df5ce7fda6a26c1b9fce2f59ab) )
	ROM_LOAD( "ic110.14",     0x4000, 0x2000, CRC(9bc2d568) SHA1(a4ee8822709645b0dc088635c0a9c263fb5a2245) )

	ROM_REGION( 0xa80, "proms", 0 )
	ROM_LOAD( "v_ic36_r",     0x000, 0x100, CRC(16ae1692) SHA1(e287b96890da4815350af72e9f2189d0c72313b6) ) /* red */
	ROM_LOAD( "v_ic35_g",     0x100, 0x100, CRC(b3d0a074) SHA1(e955fda8cb8df389507e17b7b4609e845e5ef0c4) ) /* green */
	ROM_LOAD( "v_ic27_b",     0x200, 0x100, CRC(353a2d11) SHA1(76f21e3e092024592d9ccd33ae69c438254c5755) ) /* blue */

	ROM_LOAD( "v_ic28_m",     0x300, 0x100, CRC(7ca273c1) SHA1(20d85547d96bea8b310c943c45e4978a7e5b5585) ) /* unknown */
	ROM_LOAD( "v_ic69",       0x400, 0x200, CRC(410d6f86) SHA1(3cfaef3702dbda3e7c7eb84a93561e36778aec3e) ) /* zoom */
	ROM_LOAD( "v_ic108",      0x600, 0x200, CRC(d33c02ae) SHA1(1a2146ae404a5e8a701e1d547a8409a376d4bee4) ) /* zoom */

	ROM_LOAD( "v_ic12",       0x800, 0x100, CRC(0de07e89) SHA1(5655bce6ff3abad63f5b31add402cdbb51c323f0) ) /* tile pen priority */
	ROM_LOAD( "v_ic15_p",     0x900, 0x100, CRC(7e0a0581) SHA1(e355a6ef21a65a1e828d7bd5b0f2224b06438b4a) ) /* sprite pen transparency */
	ROM_LOAD( "v_ic8",        0xa00, 0x020, CRC(4c62974d) SHA1(fd5970b5ba1d9e986515ae06c2e83f8bf20b3cdc) )

	ROM_LOAD( "ic8",          0xa20, 0x020, CRC(84bcd9af) SHA1(5a5afeb6aedb8ac6ac49fb8da62df57fbd8b1780) ) /* main CPU banking */
	ROM_LOAD( "ic22",         0xa40, 0x020, CRC(84bcd9af) SHA1(5a5afeb6aedb8ac6ac49fb8da62df57fbd8b1780) ) /* coprocessor banking */
	ROM_LOAD( "ic42",         0xa60, 0x020, CRC(2ccfe10a) SHA1(d89ea91e5da436805fca9ded9b33609f4a862724) ) /* sound cpu banking */
ROM_END

/*

CPU
Main cpu z80A x2
Sound cpu z80A
Sound chip AY-3-8910
Other ics: on lower video board,one flat-pack and two smd types.I cannot read id code on them because it was been erased.
Osc 18,432 Mhz

ROMs
chtaito1 to 4 main program
chtaito5-6 sound program
chtaito11 to 18 graphics

All eproms are 27128,2764

There are present 12 bipolar proms but i cannot dump them,but i think that code is identical to Taiyo set.
 */

ROM_START( chinherot )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code (main) */
	ROM_LOAD( "chtaito1.bin",        0x0000, 0x4000, CRC(2bd64de0) SHA1(cce14f9779a830ed83fa185918d22de0658e40ea) )
	ROM_LOAD( "chtaito2.bin",        0x4000, 0x4000, CRC(8fd04da9) SHA1(17f868c33ad01f0df96fe50e78dbde35ac8e2fe1) )
	ROM_LOAD( "chtaito3.bin",        0x8000, 0x2000, CRC(cd6908e7) SHA1(0a9b1d91bf64e8722a4e9c89658f5d4ea96c07a4) )

	ROM_REGION( 0x10000, "bbx", 0 ) /* Z80 code (coprocessor) */
	ROM_LOAD( "chtaito4.bin",        0x0000, 0x4000, CRC(2a012614) SHA1(384575197f927a6acc8b77c3b3516e0d93e7784d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code (sound) */
	ROM_LOAD( "chtaito5.bin",        0x0000, 0x4000, CRC(4e4f3f92) SHA1(57d0485f8a0110f5448b554d2fab1caba52551fd) )
	ROM_LOAD( "chtaito6.bin",        0x4000, 0x2000, CRC(8c0e43d1) SHA1(acaead801b4782875c8b6092e987b73f9973f8b0) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT ) /* tiles */
	ROM_LOAD( "chtaito11.bin",      0x0000, 0x2000, CRC(3a37fb45) SHA1(4c631cf924f1e1dfea6db3f014ab7d9cb9f4b0c4) )
	ROM_LOAD( "chtaito12.bin",      0x2000, 0x2000, CRC(bc21c002) SHA1(4fc5e4dfe8331a3feb1c370a8aca9c8303eb7b4e) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "chtaito18.bin",     0x0000, 0x2000, CRC(fc4183a8) SHA1(4bc891a9e16cd84ce353180705cc8fcadf414a49) )
	ROM_LOAD( "chtaito17.bin",     0x2000, 0x2000, CRC(d713d7fe) SHA1(8dd97f96a1190c5be5e19721227dd80adf060b4d) )
	ROM_LOAD( "chtaito13.bin",     0x4000, 0x2000, CRC(a8e2a3f4) SHA1(db9f954d4b46660f5f1cb4122838e6418f92d0a3) )

	ROM_REGION( 0x6000, "gfx3", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "chtaito16.bin",     0x0000, 0x2000, CRC(dd5170ca) SHA1(e0a9d1dbc021a8ad84dd7d1bd7e390e51e6328b7) )
	ROM_LOAD( "chtaito15.bin",     0x2000, 0x2000, CRC(20f6052e) SHA1(e22ddb3fb90ff8df5ce7fda6a26c1b9fce2f59ab) )
	ROM_LOAD( "chtaito14.bin",     0x4000, 0x2000, CRC(9bc2d568) SHA1(a4ee8822709645b0dc088635c0a9c263fb5a2245) )

	/* these were not dumped from this pcb */
	ROM_REGION( 0xa80, "proms", 0 )
	ROM_LOAD( "v_ic36_r",     0x000, 0x100, CRC(16ae1692) SHA1(e287b96890da4815350af72e9f2189d0c72313b6) ) /* red */
	ROM_LOAD( "v_ic35_g",     0x100, 0x100, CRC(b3d0a074) SHA1(e955fda8cb8df389507e17b7b4609e845e5ef0c4) ) /* green */
	ROM_LOAD( "v_ic27_b",     0x200, 0x100, CRC(353a2d11) SHA1(76f21e3e092024592d9ccd33ae69c438254c5755) ) /* blue */

	ROM_LOAD( "v_ic28_m",     0x300, 0x100, CRC(7ca273c1) SHA1(20d85547d96bea8b310c943c45e4978a7e5b5585) ) /* unknown */
	ROM_LOAD( "v_ic69",       0x400, 0x200, CRC(410d6f86) SHA1(3cfaef3702dbda3e7c7eb84a93561e36778aec3e) ) /* zoom */
	ROM_LOAD( "v_ic108",      0x600, 0x200, CRC(d33c02ae) SHA1(1a2146ae404a5e8a701e1d547a8409a376d4bee4) ) /* zoom */

	ROM_LOAD( "v_ic12",       0x800, 0x100, CRC(0de07e89) SHA1(5655bce6ff3abad63f5b31add402cdbb51c323f0) ) /* tile pen priority */
	ROM_LOAD( "v_ic15_p",     0x900, 0x100, CRC(7e0a0581) SHA1(e355a6ef21a65a1e828d7bd5b0f2224b06438b4a) ) /* sprite pen transparency */
	ROM_LOAD( "v_ic8",        0xa00, 0x020, CRC(4c62974d) SHA1(fd5970b5ba1d9e986515ae06c2e83f8bf20b3cdc) )

	ROM_LOAD( "ic8",          0xa20, 0x020, CRC(84bcd9af) SHA1(5a5afeb6aedb8ac6ac49fb8da62df57fbd8b1780) ) /* main CPU banking */
	ROM_LOAD( "ic22",         0xa40, 0x020, CRC(84bcd9af) SHA1(5a5afeb6aedb8ac6ac49fb8da62df57fbd8b1780) ) /* coprocessor banking */
	ROM_LOAD( "ic42",         0xa60, 0x020, CRC(2ccfe10a) SHA1(d89ea91e5da436805fca9ded9b33609f4a862724) ) /* sound cpu banking */
ROM_END

ROM_START( shangkid )
	/* Main CPU - handles game logic */
	ROM_REGION( 0x12000, "maincpu", 0 ) /* Z80 (NEC D780C-1) code */
	ROM_LOAD( "cr00ic02.bin", 0x00000, 0x4000, CRC(2e420377) SHA1(73eb916b1693ffab8049ea0d8d3503629fa27948) )
	ROM_LOAD( "cr01ic03.bin", 0x04000, 0x4000, CRC(161cd358) SHA1(2cc1c30b3d3215ddc7c7f96a3358ed50e0f850e3) )
	ROM_LOAD( "cr02ic04.bin", 0x08000, 0x2000, CRC(85b6e455) SHA1(3b2cd1e55355d24c014c5afe0212c6c9f0899a28) )   /* banked at 0x8000 */
	ROM_LOAD( "cr03ic05.bin", 0x10000, 0x2000, CRC(3b383863) SHA1(3fb10a7f89cf2387d70b0337916063fd4ec5f754) )   /* banked at 0x8000 */

	/* The BBX coprocessor is buried in an epoxy block.  It contains:
	**  -   a surface-mounted Z80 (TMPZ84C00P)
	**  -   LS245 logic IC
	**  -   battery backed ram chip Fujitsu MB8464
	**
	**  The BBX coprocessor receives graphics and sound-related commands from
	**  the main CPU via shared RAM.  It directly manages an AY8910, is
	**  responsible for populating spriteram, and forwards appropriate sound
	**  commands to the sample-playing CPU.
	*/
	ROM_REGION( 0x10000, "bbx", 0 ) /* Z80: bbx module */
	ROM_LOAD( "bbx.bin",      0x0000, 0x2000, CRC(560c0abd) SHA1(165beadd55bc29195cc680825f71f3f7f60fa0f6) ) /* battery-backed RAM */
	ROM_LOAD( "cr04ic31.bin", 0x2000, 0x2000, CRC(cb207885) SHA1(b73458c959a4ebceb4c88931f8e3d1aff01dbaff) )
	ROM_LOAD( "cr05ic32.bin", 0x4000, 0x4000, CRC(cf3b8d55) SHA1(c2e196e2762dd7884f461fc2be37698b9ed1deef) )
	ROM_LOAD( "cr06ic33.bin", 0x8000, 0x2000, CRC(0f3bdbd8) SHA1(2e0e81425e4e5592d3e2c8395075720c2ad3f79a) )

	/*  The Sound CPU is a dedicated Sample Player */
	ROM_REGION( 0x1e000, "audiocpu", 0 ) /* Z80 (NEC D780C-1) */
	ROM_LOAD( "cr11ic51.bin", 0x00000, 0x4000, CRC(2e2d6afe) SHA1(1414a06b6cf14dfd69ca6cf35e4eb7d75af3f219) )
	ROM_LOAD( "cr12ic43.bin", 0x04000, 0x4000, CRC(dd29a0c8) SHA1(8411c31fefdce8c9233fe531b5bf3b6c43c03cba) )
	ROM_LOAD( "cr13ic44.bin", 0x08000, 0x4000, CRC(879d0de0) SHA1(b1422cf239381ac949867c42ca8101fa8dcac9d6) )
	ROM_LOAD( "cr07ic47.bin", 0x10000, 0x4000, CRC(20540f7c) SHA1(85c0b913948a67a34b25f0974fdd22e1dbb63166) )
	ROM_LOAD( "cr08ic48.bin", 0x14000, 0x2000, CRC(392f24db) SHA1(5bd68a4105717e18e79afba4c00733ad74b39875) )
	ROM_LOAD( "cr09ic49.bin", 0x18000, 0x4000, CRC(d50c96a8) SHA1(7fcf798b49b0827349366475dbbca1554df25cc4) )
	ROM_LOAD( "cr10ic50.bin", 0x1c000, 0x2000, CRC(873a5f2d) SHA1(32f806da319807bef68b5e810815ef2aba6ea0a7) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT ) /* 8x8 tiles */
	ROM_LOAD( "cr20ic21.bin", 0x0000, 0x2000, CRC(eb3cbb11) SHA1(8d36d6f328263eb0b956c0bd752d2cac84795c1a) )
	ROM_LOAD( "cr21ic22.bin", 0x2000, 0x2000, CRC(7c6e75f4) SHA1(2a4a7971777136a476b8ca0b888e65a31a032a9e) )

	ROM_REGION( 0x18000, "gfx2", ROMREGION_INVERT ) /* 16x16 sprites */
	ROM_LOAD( "cr14i114.bin", 0x00000, 0x4000, CRC(ee1f348f) SHA1(7bfcdf645a2f406130444bb7b641a91351761c83) )
	ROM_LOAD( "cr15i113.bin", 0x04000, 0x4000, CRC(a46398bd) SHA1(2f968eb95f3406110b4b503fe4da735bb64b548b) )
	ROM_LOAD( "cr16i112.bin", 0x08000, 0x4000, CRC(cbed446c) SHA1(cc36ab32b42f6ec8ce574f040f7fb034b1351467) )
	ROM_LOAD( "cr17i111.bin", 0x0c000, 0x4000, CRC(b0a44330) SHA1(9d52856243e21ab906ee1701b6485411f2933707) )
	ROM_LOAD( "cr18ic99.bin", 0x10000, 0x4000, CRC(ff7efd7c) SHA1(95f83a9aa2f0845efe2c9c72d29e7a08c78d9b1f) )
	ROM_LOAD( "cr19i100.bin", 0x14000, 0x4000, CRC(f948f829) SHA1(c4305d4a04213af39413e1575eaee3905344c788) )

	ROM_REGION( 0xa80, "proms", 0 )
	ROM_LOAD( "cr31ic36.bin", 0x000, 0x100, CRC(9439590b) SHA1(cd07526d6373358bae6bfce8dbcab7d44472041f) )  /* 82S129 - red */
	ROM_LOAD( "cr30ic35.bin", 0x100, 0x100, CRC(324e295e) SHA1(9076e3da2edc8889bd635191e7687676b6fb4cec) )  /* 82S129 - green */
	ROM_LOAD( "cr28ic27.bin", 0x200, 0x100, CRC(375cba96) SHA1(fd3ba36588147a3252b800f1f86b2897e9605b8d) )  /* 82S129 - blue */

	ROM_LOAD( "cr29ic28.bin", 0x300, 0x100, CRC(7ca273c1) SHA1(20d85547d96bea8b310c943c45e4978a7e5b5585) )  /* 82S129 - unknown */
	ROM_LOAD( "cr32ic69.bin", 0x400, 0x200, CRC(410d6f86) SHA1(3cfaef3702dbda3e7c7eb84a93561e36778aec3e) )  /* 82S147 - sprite-related (zoom?) */
	ROM_LOAD( "cr33-108.bin", 0x600, 0x200, CRC(d33c02ae) SHA1(1a2146ae404a5e8a701e1d547a8409a376d4bee4) )  /* 82S147 - sprite-related (zoom?) */

	ROM_LOAD( "cr26ic12.bin", 0x800, 0x100, CRC(85b5e958) SHA1(f211b5122fccf84e4aa1556c0290b5cb83935386) )  /* 82S129 - tile pen priority? */
	ROM_LOAD( "cr27ic15.bin", 0x900, 0x100, CRC(f7a19fe2) SHA1(d88e0743aa858b132f636fcd5d493ccb1af82224) )  /* 82S129 - sprite pen transparency */

	ROM_LOAD( "cr25ic8.bin",  0xa00, 0x020, CRC(c85e09ad) SHA1(f42e3840ec0e4720067eda7c536b6dcc540e63ff) )  /* 82S123 */
	ROM_LOAD( "cr22ic8.bin",  0xa20, 0x020, CRC(1a7e0b06) SHA1(648d58a4ad14f4b242e492cf302d6678d899cf4f) )  /* 82S123 - main CPU banking */
	ROM_LOAD( "cr23ic22.bin", 0xa40, 0x020, CRC(efb5f265) SHA1(3de15e03cb12956d34074abb48236537f2b47dba) )  /* 82S123 - coprocessor banking */
	ROM_LOAD( "cr24ic42.bin", 0xa60, 0x020, CRC(823878aa) SHA1(eb5026270890e5af9193e354b7e814f32238a9bf) )  /* 82S123 - sample player banking */
ROM_END

ROM_START( hiryuken )
	/* Main CPU - handles game logic */
	ROM_REGION( 0x12000, "maincpu", 0 ) /* Z80 (NEC D780C-1) code */
	ROM_LOAD( "1.2", 0x00000, 0x4000, CRC(c7af7f2e) SHA1(b035a4230e10bcf0891e41423a51fb6169087b8e) )
	ROM_LOAD( "2.3", 0x04000, 0x4000, CRC(639afdb3) SHA1(50bd1deffb66049f101faceb108ee95eb3fe8ae6) )
	ROM_LOAD( "3.4", 0x08000, 0x2000, CRC(ad210482) SHA1(9a32bbaf601d3b00f0a79ce90bb9a32e8e608977) ) /* banked at 0x8000 */
	ROM_LOAD( "4.5", 0x10000, 0x2000, CRC(6518943a) SHA1(b5e78267d5a58c466c9ae20ba4f9c5e14e252287) ) /* banked at 0x8000 */

	/* The BBX coprocessor is buried in an epoxy block.  It contains:
	** - a surface-mounted Z80 (TMPZ84C00P)
	** - LS245 logic IC
	** - battery backed ram chip Fujitsu MB8464
	**
	** The BBX coprocessor receives graphics and sound-related commands from
	** the main CPU via shared RAM.  It directly manages an AY8910, is
	** responsible for populating spriteram, and forwards appropriate sound
	** commands to the sample-playing CPU.
	*/
	ROM_REGION( 0x10000, "bbx", 0 ) /* Z80: bbx module */
	ROM_LOAD( "bbxj.bin",     0x0000, 0x2000, CRC(8def4aaf) SHA1(bfb9b2874294499c1026f2a4cd20f9cd0efd30f9) ) /* battery-backed RAM */
	ROM_LOAD( "5.31",         0x2000, 0x2000, CRC(8ae37ce7) SHA1(6299b0cd4e7348d4599126d61192924f19ae5401) )
	ROM_LOAD( "6.32",         0x4000, 0x4000, CRC(e835bb7f) SHA1(4ed8033994fe6ca268e20e30382dbe61eb8d2cf6) )
	ROM_LOAD( "7.33",         0x8000, 0x2000, CRC(3745ed36) SHA1(29a462a7d6e994cd2a917ce0b79fe342cfcc2417) )

	/* The Sound CPU is a dedicated Sample Player */
	ROM_REGION( 0x1e000, "audiocpu", 0 ) /* Z80 (NEC D780C-1) */
	ROM_LOAD( "cr11ic51.bin", 0x00000, 0x4000, CRC(2e2d6afe) SHA1(1414a06b6cf14dfd69ca6cf35e4eb7d75af3f219) )   // 12.51
//  ROM_LOAD( "cr12ic43.bin", 0x04000, 0x4000, CRC(dd29a0c8) SHA1(8411c31fefdce8c9233fe531b5bf3b6c43c03cba) )   // not present in this set
//  ROM_LOAD( "cr13ic44.bin", 0x08000, 0x4000, CRC(879d0de0) SHA1(b1422cf239381ac949867c42ca8101fa8dcac9d6) )   // not present in this set
	ROM_LOAD( "cr07ic47.bin", 0x10000, 0x4000, CRC(20540f7c) SHA1(85c0b913948a67a34b25f0974fdd22e1dbb63166) )   // 8.47
	ROM_LOAD( "9.48",         0x14000, 0x4000, CRC(8da23cad) SHA1(051459d7b5395336d698b03f1cc8566f33b62c8b) )
	ROM_LOAD( "10.49",        0x18000, 0x4000, CRC(52b82fee) SHA1(99a2952af6493586745463c33b11ab69251af063) )
	ROM_LOAD( "cr10ic50.bin", 0x1c000, 0x2000, CRC(873a5f2d) SHA1(32f806da319807bef68b5e810815ef2aba6ea0a7) )   // 11.50

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT ) /* 8x8 tiles */
	ROM_LOAD( "21.21",        0x0000, 0x2000, CRC(ce20a1d4) SHA1(4d5708a1b8b3ac81afeebd82eb6a3d9763ef3d39) )
	ROM_LOAD( "22.22",        0x2000, 0x2000, CRC(26fc88bf) SHA1(663c88510b4df8153b10d1b0cc20b332aab5ca2e) )

	ROM_REGION( 0x18000, "gfx2", ROMREGION_INVERT ) /* 16x16 sprites */
	ROM_LOAD( "15.114",       0x00000, 0x4000, CRC(ed07854e) SHA1(caebd227c458717a5fc58daa6e774b1a09e68d34) )
	ROM_LOAD( "16.113",       0x04000, 0x4000, CRC(85cf1939) SHA1(8bf410a91aba40b0336e3c6c5b2c2b353aeea420) )
	ROM_LOAD( "cr16i112.bin", 0x08000, 0x4000, CRC(cbed446c) SHA1(cc36ab32b42f6ec8ce574f040f7fb034b1351467) )   // 17.112
	ROM_LOAD( "cr17i111.bin", 0x0c000, 0x4000, CRC(b0a44330) SHA1(9d52856243e21ab906ee1701b6485411f2933707) )   // 18.111
	ROM_LOAD( "cr18ic99.bin", 0x10000, 0x4000, CRC(ff7efd7c) SHA1(95f83a9aa2f0845efe2c9c72d29e7a08c78d9b1f) )   // 19.99
	ROM_LOAD( "20.100",       0x14000, 0x4000, CRC(4bc77ca0) SHA1(22a057c3c29ff9feb0afab0cb76b37c4a1363cb1) )

	ROM_REGION( 0xa80, "proms", 0 )
	ROM_LOAD( "r.36",         0x000, 0x100, CRC(65dec63d) SHA1(b481151687311f8d732f8c313f8af183a53dbc2f) ) /* 82S129 - red */
	ROM_LOAD( "g.35",         0x100, 0x100, CRC(e79de8cf) SHA1(65cc626c91830eafbf5a7e4ce7571cbb0ada91c1) ) /* 82S129 - green */
	ROM_LOAD( "b.27",         0x200, 0x100, CRC(d6ab3448) SHA1(efb879e7c5dd50ea70fb5ed5e4d3b87ab2c1b8b9) ) /* 82S129 - blue */

	ROM_LOAD( "cr29ic28.bin", 0x300, 0x100, CRC(7ca273c1) SHA1(20d85547d96bea8b310c943c45e4978a7e5b5585) ) /* 82S129 - unknown */                   // m.28
	ROM_LOAD( "cr32ic69.bin", 0x400, 0x200, CRC(410d6f86) SHA1(3cfaef3702dbda3e7c7eb84a93561e36778aec3e) ) /* 82S147 - sprite-related (zoom?) */    // ic69
	ROM_LOAD( "cr33-108.bin", 0x600, 0x200, CRC(d33c02ae) SHA1(1a2146ae404a5e8a701e1d547a8409a376d4bee4) ) /* 82S147 - sprite-related (zoom?) */    // ic108

	ROM_LOAD( "cr26ic12.bin", 0x800, 0x100, CRC(85b5e958) SHA1(f211b5122fccf84e4aa1556c0290b5cb83935386) ) /* 82S129 - tile pen priority? */        // sc.12
	ROM_LOAD( "cr27ic15.bin", 0x900, 0x100, CRC(f7a19fe2) SHA1(d88e0743aa858b132f636fcd5d493ccb1af82224) ) /* 82S129 - sprite pen transparency */   // sp.15

	ROM_LOAD( "cr25ic8.bin",  0xa00, 0x020, CRC(c85e09ad) SHA1(f42e3840ec0e4720067eda7c536b6dcc540e63ff) ) /* 82S123 */                         // a.8
	ROM_LOAD( "cr22ic8.bin",  0xa20, 0x020, CRC(1a7e0b06) SHA1(648d58a4ad14f4b242e492cf302d6678d899cf4f) ) /* 82S123 - main CPU banking */      // 1.8
	ROM_LOAD( "cr23ic22.bin", 0xa40, 0x020, CRC(efb5f265) SHA1(3de15e03cb12956d34074abb48236537f2b47dba) ) /* 82S123 - coprocessor banking */       // 2.22
	ROM_LOAD( "cr24ic42.bin", 0xa60, 0x020, CRC(823878aa) SHA1(eb5026270890e5af9193e354b7e814f32238a9bf) ) /* 82S123 - sample player banking */ // 3.42
ROM_END

ROM_START( dynamski )
	ROM_REGION( 0x12000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "dynski.1",     0x00000, 0x1000, CRC(30191160) SHA1(5ffa3355f53e4be65bd96101088d2d7b66490141) ) /* code */
	ROM_LOAD( "dynski.2",     0x01000, 0x1000, CRC(5e08a0b0) SHA1(89398752e8ea1ffd8ec8392f5c8e20f25cf8fdfb) )
	ROM_LOAD( "dynski.3",     0x02000, 0x1000, CRC(29cfd740) SHA1(a5d6b7b59e631f387788f29e8f029eaf00d1ea3f) )
	ROM_LOAD( "dynski.4",     0x03000, 0x1000, CRC(e1d47776) SHA1(d08985a5b523706bc2b3e090373a72b781116a80) )
	ROM_LOAD( "dynski.5",     0x04000, 0x1000, CRC(e39aba1b) SHA1(133d6280abccdca248b553e80dedcc4682ae8d70) )
	ROM_LOAD( "dynski.6",     0x05000, 0x1000, CRC(95780608) SHA1(16b28e014ec5602df48e382e2b63d14acb60b9ba) )
	ROM_LOAD( "dynski.7",     0x06000, 0x1000, CRC(b88d328b) SHA1(5229fcb0ed1067770fcd47bec674a6fd7b999484) )
	ROM_LOAD( "dynski.8",     0x07000, 0x1000, CRC(8db5e691) SHA1(ccacfb7cd30f03de95690bbd32ab46e39d084244) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT ) /* 8x8 tiles */
	ROM_LOAD( "dynski8.3e",   0x0000, 0x2000, CRC(32c354dc) SHA1(6f73c9fc3802ec5f349a6088d7eaee5876c901de) )
	ROM_LOAD( "dynski9.2e",   0x2000, 0x2000, CRC(80a6290c) SHA1(3a1581451802bf2e822fba1084800e6de9bd0f7a) )

	ROM_REGION( 0x6000, "gfx2", ROMREGION_INVERT ) /* 16x16 sprites */
	ROM_LOAD( "dynski5.14b",  0x0000, 0x2000, CRC(aa4ac6e2) SHA1(b6f7cb7310be2a12ca17dfd0ee3526e0522eb85b) )
	ROM_LOAD( "dynski6.15b",  0x2000, 0x2000, CRC(47e76886) SHA1(c42bd3d973cf17ec265852527085085bcc674b18) )
	ROM_LOAD( "dynski7.14d",  0x4000, 0x2000, CRC(a153dfa9) SHA1(c81dbdce2e11e9d6d8465f400a048d7688745232) )

	ROM_REGION( 0x240, "proms", 0 )
	ROM_LOAD( "dynskic.15g",  0x000, 0x020, CRC(9333a5e4) SHA1(11025f53c98c2ae21e9d4f95da86bba4703a52bc) )  /* palette */
	ROM_LOAD( "dynskic.15f",  0x020, 0x020, CRC(3869514b) SHA1(1ef062284e52777ec6e269368a16b9b357a2647a) )  /* palette */
	ROM_LOAD( "dynski.4g",    0x040, 0x100, CRC(761fe465) SHA1(87741a6e4d14440073aaed3a8a15bc5e30b8fcfc) )  /* lookup table */
	ROM_LOAD( "dynski.11e",   0x140, 0x100, CRC(e625aa09) SHA1(c10371f1adf7245815c0bdcb24458c7b04edd5b9) )  /* lookup table */
ROM_END


GAME( 1984, dynamski, 0,        dynamski, dynamski, driver_device, 0,        ROT90, "Taiyo", "Dynamic Ski", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1984, chinhero, 0,        chinhero, chinhero, shangkid_state, chinhero, ROT90, "Taiyo", "Chinese Hero", MACHINE_SUPPORTS_SAVE ) // by Nihon Game?
GAME( 1984, chinhero2,chinhero, chinhero, chinhero, shangkid_state, chinhero, ROT90, "Taiyo", "Chinese Hero (older, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, chinhero3,chinhero, chinhero, chinhero, shangkid_state, chinhero, ROT90, "Taiyo", "Chinese Hero (older, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, chinherot,chinhero, chinhero, chinhero, shangkid_state, chinhero, ROT90, "Taiyo (Taito license)", "Chinese Heroe (Taito)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, shangkid, 0,        shangkid, shangkid, shangkid_state, shangkid, ROT0,  "Taiyo (Data East license)", "Shanghai Kid", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1985, hiryuken, shangkid, shangkid, shangkid, shangkid_state, shangkid, ROT0,  "Taiyo (Taito license)", "Hokuha Syourin Hiryu no Ken", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
