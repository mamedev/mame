// license:BSD-3-Clause
// copyright-holders:Uki
/*****************************************************************************

Strength & Skill (c) 1984 Sun Electronics

    Driver by Uki

    19/Jun/2001 -

TODO:
- needs merging with Ikki driver;

Notes:
 Banbam has a Fujitsu MB8841 4-Bit MCU for protection labeled SUN 8212.
 Its internal ROM has been imaged, manually typed, and decoded as sun-8212.ic3.
 Pettan Pyuu is a clone of Banbam although with different levels / play fields.

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mb88xx/mb88xx.h"
#include "sound/sn76496.h"
#include "includes/strnskil.h"



void strnskil_state::machine_start()
{
	save_item(NAME(m_scrl_ctrl));
	save_item(NAME(m_irq_source));
}

/****************************************************************************/

READ8_MEMBER(strnskil_state::strnskil_d800_r)
{
/* bit0: interrupt type?, bit1: CPU2 busack? */

	return (m_irq_source);
}

/****************************************************************************/

READ8_MEMBER(strnskil_state::pettanp_protection_r)
{
	int res;

	switch (space.device().safe_pc())
	{
		case 0x6066:    res = 0xa5; break;
		case 0x60dc:    res = 0x20; break;  /* bits 0-3 unknown */
		case 0x615d:    res = 0x30; break;  /* bits 0-3 unknown */
		case 0x61b9:    res = 0x60|(machine().rand()&0x0f); break;  /* bits 0-3 unknown */
		case 0x6219:    res = 0x77; break;
		case 0x626c:    res = 0xb4; break;
		default:        res = 0xff; break;
	}

	logerror("%04x: protection_r -> %02x\n",space.device().safe_pc(),res);
	return res;
}

READ8_MEMBER(strnskil_state::banbam_protection_r)
{
	int res;

	switch (space.device().safe_pc())
	{
		case 0x6094:    res = 0xa5; break;
		case 0x6118:    res = 0x20; break;  /* bits 0-3 unknown */
		case 0x6199:    res = 0x30; break;  /* bits 0-3 unknown */
		case 0x61f5:    res = 0x60|(machine().rand()&0x0f); break;  /* bits 0-3 unknown */
		case 0x6255:    res = 0x77; break;
		case 0x62a8:    res = 0xb4; break;
		default:        res = 0xff; break;
	}

	logerror("%04x: protection_r -> %02x\n",space.device().safe_pc(),res);
	return res;
}

WRITE8_MEMBER(strnskil_state::protection_w)
{
	logerror("%04x: protection_w %02x\n",space.device().safe_pc(),data);
}

/****************************************************************************/

static ADDRESS_MAP_START( strnskil_map1, AS_PROGRAM, 8, strnskil_state )
	AM_RANGE(0x0000, 0x9fff) AM_ROM

	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(strnskil_videoram_w) AM_SHARE("videoram")

	AM_RANGE(0xd800, 0xd800) AM_READ(strnskil_d800_r)
	AM_RANGE(0xd801, 0xd801) AM_READ_PORT("DSW1")
	AM_RANGE(0xd802, 0xd802) AM_READ_PORT("DSW2")
	AM_RANGE(0xd803, 0xd803) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xd804, 0xd804) AM_READ_PORT("P1")
	AM_RANGE(0xd805, 0xd805) AM_READ_PORT("P2")

	AM_RANGE(0xd808, 0xd808) AM_WRITE(strnskil_scrl_ctrl_w)
	AM_RANGE(0xd809, 0xd809) AM_WRITENOP /* coin counter? */
	AM_RANGE(0xd80a, 0xd80b) AM_WRITEONLY AM_SHARE("xscroll")
ADDRESS_MAP_END

static ADDRESS_MAP_START( strnskil_map2, AS_PROGRAM, 8, strnskil_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share1")

	AM_RANGE(0xd801, 0xd801) AM_DEVWRITE("sn1", sn76496_device, write)
	AM_RANGE(0xd802, 0xd802) AM_DEVWRITE("sn2", sn76496_device, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mcu_io_map, AS_IO, 8, strnskil_state )
//  AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(mcu_portk_r)
//  AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READWRITE(mcu_portr0_r, mcu_portr0_w)
ADDRESS_MAP_END

/****************************************************************************/

static INPUT_PORTS_START( strnskil )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Unknown 1-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Unknown 1-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0x00, "Coin1 / Coin2" )
	PORT_DIPSETTING(    0x00, "1C 1C / 1C 1C" )
	PORT_DIPSETTING(    0x10, "2C 1C / 2C 1C" )
	PORT_DIPSETTING(    0x20, "2C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x30, "1C 1C / 1C 2C" )
	PORT_DIPSETTING(    0x40, "1C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x50, "1C 1C / 1C 4C" )
	PORT_DIPSETTING(    0x60, "1C 1C / 1C 5C" )
	PORT_DIPSETTING(    0x70, "1C 1C / 1C 6C" )
	PORT_DIPSETTING(    0x80, "1C 2C / 1C 2C" )
	PORT_DIPSETTING(    0x90, "1C 2C / 1C 4C" )
	PORT_DIPSETTING(    0xa0, "1C 2C / 1C 5C" )
	PORT_DIPSETTING(    0xb0, "1C 2C / 1C 10C" )
	PORT_DIPSETTING(    0xc0, "1C 2C / 1C 11C" )
	PORT_DIPSETTING(    0xd0, "1C 2C / 1C 12C" )
	PORT_DIPSETTING(    0xe0, "1C 2C / 1C 6C" )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, "Unknown 2-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Unknown 2-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Unknown 2-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Unknown 2-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Unknown 2-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Unknown 2-7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* d804 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY

	PORT_START("P2")        /* d805 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SYSTEM")    /* d803 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( banbam )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPNAME( 0xf0, 0x00, "Coin1 / Coin2" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x00, "1C 1C / 1C 1C" )
	PORT_DIPSETTING(    0x10, "2C 1C / 2C 1C" )
	PORT_DIPSETTING(    0x20, "2C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x30, "1C 1C / 1C 2C" )
	PORT_DIPSETTING(    0x40, "1C 1C / 1C 3C" )
	PORT_DIPSETTING(    0x50, "1C 1C / 1C 4C" )
	PORT_DIPSETTING(    0x60, "1C 1C / 1C 5C" )
	PORT_DIPSETTING(    0x70, "1C 1C / 1C 6C" )
	PORT_DIPSETTING(    0x80, "1C 2C / 1C 2C" )
	PORT_DIPSETTING(    0x90, "1C 2C / 1C 4C" )
	PORT_DIPSETTING(    0xa0, "1C 2C / 1C 5C" )
	PORT_DIPSETTING(    0xb0, "1C 2C / 1C 10C" )
	PORT_DIPSETTING(    0xc0, "1C 2C / 1C 11C" )
	PORT_DIPSETTING(    0xd0, "1C 2C / 1C 12C" )
	PORT_DIPSETTING(    0xe0, "1C 2C / 1C 6C" )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x00, "20000 50000" )
	PORT_DIPSETTING(    0x02, "20000 80000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x06, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, "Second Practice" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )    /*  These four dips are unused according to the manual */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" ) PORT_DIPLOCATION("SW2:8") //game stands in a tight loop at $14-$16 -> $866 if this is putted off
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")        /* d804 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_START("P2")        /* d805 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("SYSTEM")    /* d803 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END


/****************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	3,      /* 3 bits per pixel */
	{0,8192*8,8192*8*2},
	{7,6,5,4,3,2,1,0},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7},
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 characters */
	256,    /* 256 characters */
	3,      /* 3 bits per pixel */
	{8192*8*2,8192*8,0},
	{7,6,5,4,3,2,1,0,
		8*16+7,8*16+6,8*16+5,8*16+4,8*16+3,8*16+2,8*16+1,8*16+0},
	{8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7,
		8*8,8*9,8*10,8*11,8*12,8*13,8*14,8*15},
	8*8*4
};

static GFXDECODE_START( strnskil )
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout,   512, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0,   64 )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(strnskil_state::strnskil_irq)
{
	int scanline = param;

	if(scanline == 240 || scanline == 96)
	{
		m_maincpu->set_input_line(0,HOLD_LINE);

		m_irq_source = (scanline != 240);
	}
}


static MACHINE_CONFIG_START( strnskil, strnskil_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000/2) /* 4.000MHz */
	MCFG_CPU_PROGRAM_MAP(strnskil_map1)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", strnskil_state, strnskil_irq, "screen", 0, 1)

	MCFG_CPU_ADD("sub", Z80,8000000/2) /* 4.000MHz */
	MCFG_CPU_PROGRAM_MAP(strnskil_map2)
	MCFG_CPU_PERIODIC_INT_DRIVER(strnskil_state, irq0_line_hold, 2*60)

//  MCFG_QUANTUM_PERFECT_CPU("maincpu")
	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8+3*8, 32*8+3*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(strnskil_state, screen_update_strnskil)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", strnskil)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(strnskil_state, strnskil)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76496, 8000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_SOUND_ADD("sn2", SN76496, 8000000/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( banbam, strnskil )
	MCFG_CPU_ADD("mcu", MB8841, 8000000/2)
	MCFG_CPU_IO_MAP(mcu_io_map)
MACHINE_CONFIG_END

/****************************************************************************/

ROM_START( strnskil )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "tvg3.7",  0x0000,  0x2000, CRC(31fd793a) SHA1(b86efe8ea60edf414a23fb6abc09db691c085fe9) )
	ROM_CONTINUE(        0x8000,  0x2000 )
	ROM_LOAD( "tvg4.8",  0x2000,  0x2000, CRC(c58315b5) SHA1(2039cd89ef59d05f353f6c367fa851c0f60cdc4a) )
	ROM_LOAD( "tvg5.9",  0x4000,  0x2000, CRC(29e7ded5) SHA1(6eae5988139f22c3ff166192e4fda77db38a79bc) )
	ROM_LOAD( "tvg6.10", 0x6000,  0x2000, CRC(8b126a4b) SHA1(68b617c5dc120c777e152919cba9daeaf3ceac5f) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sub CPU */
	ROM_LOAD( "tvg1.2",  0x0000,  0x2000, CRC(b586b753) SHA1(7c9891fb279b1323c059ffdcf7c009bf971037be) )
	ROM_LOAD( "tvg2.3",  0x2000,  0x2000, CRC(8bd71bb6) SHA1(cc35e1e4cbb893ab04f1b6ceef0a050243e3b462) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "tvg7.90",   0x0000,  0x2000, CRC(ee3bd593) SHA1(398e426e53695cc184d5a2750fd32a1c2c68bf30) )
	ROM_LOAD( "tvg8.92",   0x2000,  0x2000, CRC(1b265360) SHA1(fbc64c504639106c1813bf91bd31bda1ce4c7ffe) )
	ROM_LOAD( "tvg9.94",   0x4000,  0x2000, CRC(776c7ca6) SHA1(23fd1ac15395822b318db4435e48dd4e0e3e61de) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "tvg12.102", 0x0000,  0x2000, CRC(68b9d888) SHA1(7a4071fe882c1949979f97a020d7c6e95643ef42) )
	ROM_LOAD( "tvg11.101", 0x2000,  0x2000, CRC(7f2179ff) SHA1(24fab1f4430ae883bc1f477d3df7643e06c67349) )
	ROM_LOAD( "tvg10.100", 0x4000,  0x2000, CRC(321ad963) SHA1(9b50fbf0c3b4ce7ce3c68339b99a2ccadef4646f) )

	ROM_REGION( 0x0800, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "15-3.prm", 0x0000,  0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) /* R */
	ROM_LOAD( "15-4.prm", 0x0100,  0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) /* G */
	ROM_LOAD( "15-5.prm", 0x0200,  0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) /* B */
	ROM_LOAD( "15-1.prm", 0x0300,  0x0200, CRC(d4f5b3d7) SHA1(9a244c77a752df655ff756e063d56c2c767e37d9) ) /* sprite */
	ROM_LOAD( "15-2.prm", 0x0500,  0x0200, CRC(cdffede9) SHA1(3ecdf91e3f78eb6cdd3a6f58d1a89d448a676c52) ) /* bg */

	ROM_REGION( 0x0100, "user1", 0 ) /* scroll control PROM */
	ROM_LOAD( "15-6.prm", 0x0000,  0x0100, CRC(ec4faf5b) SHA1(7ebbf50807d04105ebadec91bded069408e399ba) )
ROM_END

ROM_START( guiness )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "tvg3.15", 0x0000,  0x2000, CRC(3a605ad8) SHA1(f6e2dd4989fdb68bc55857f5a8f06601416139d5) )
	ROM_CONTINUE(        0x8000,  0x2000 )
	ROM_LOAD( "tvg4.8",  0x2000,  0x2000, CRC(c58315b5) SHA1(2039cd89ef59d05f353f6c367fa851c0f60cdc4a) )
	ROM_LOAD( "tvg5.9",  0x4000,  0x2000, CRC(29e7ded5) SHA1(6eae5988139f22c3ff166192e4fda77db38a79bc) )
	ROM_LOAD( "tvg6.10", 0x6000,  0x2000, CRC(8b126a4b) SHA1(68b617c5dc120c777e152919cba9daeaf3ceac5f) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sub CPU */
	ROM_LOAD( "tvg1.2",  0x0000,  0x2000, CRC(b586b753) SHA1(7c9891fb279b1323c059ffdcf7c009bf971037be) )
	ROM_LOAD( "tvg2.3",  0x2000,  0x2000, CRC(8bd71bb6) SHA1(cc35e1e4cbb893ab04f1b6ceef0a050243e3b462) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "tvg7.90",   0x0000,  0x2000, CRC(ee3bd593) SHA1(398e426e53695cc184d5a2750fd32a1c2c68bf30) )
	ROM_LOAD( "tvg8.92",   0x2000,  0x2000, CRC(1b265360) SHA1(fbc64c504639106c1813bf91bd31bda1ce4c7ffe) )
	ROM_LOAD( "tvg9.94",   0x4000,  0x2000, CRC(776c7ca6) SHA1(23fd1ac15395822b318db4435e48dd4e0e3e61de) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "tvg12.15", 0x0000,  0x2000, CRC(a82c923d) SHA1(2bd2b028d782fac18f2fe9c9ef73ce0af67db347) )
	ROM_LOAD( "tvg11.15", 0x2000,  0x2000, CRC(d432c96f) SHA1(0d4b3af778dbd40bc26bad4c673a9ce1ef537c04) )
	ROM_LOAD( "tvg10.15", 0x4000,  0x2000, CRC(a53959d6) SHA1(cdf7acf1a75d83b259948c482f06543624a695a3) )

	ROM_REGION( 0x0800, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "15-3.prm", 0x0000,  0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) /* R */
	ROM_LOAD( "15-4.prm", 0x0100,  0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) /* G */
	ROM_LOAD( "15-5.prm", 0x0200,  0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) /* B */
	ROM_LOAD( "15-1.prm", 0x0300,  0x0200, CRC(d4f5b3d7) SHA1(9a244c77a752df655ff756e063d56c2c767e37d9) ) /* sprite */
	ROM_LOAD( "15-2.prm", 0x0500,  0x0200, CRC(cdffede9) SHA1(3ecdf91e3f78eb6cdd3a6f58d1a89d448a676c52) ) /* bg */

	ROM_REGION( 0x0100, "user1", 0 ) /* scroll control PROM */
	ROM_LOAD( "15-6.prm", 0x0000,  0x0100, CRC(ec4faf5b) SHA1(7ebbf50807d04105ebadec91bded069408e399ba) )
ROM_END

ROM_START( pettanp )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "tvg2-16a.7",  0x0000,  0x2000, CRC(4cbbbd01) SHA1(3905cf9e9d324bb23688ab29c98d71529d3dbf0c) )
	ROM_CONTINUE(            0x8000,  0x2000 )
	ROM_LOAD( "tvg3-16a.8",  0x2000,  0x2000, CRC(aaa0420f) SHA1(aa7ead51002f8b1bbefd07ff23b9064804fc31b3) )
	ROM_LOAD( "tvg4-16a.9",  0x4000,  0x2000, CRC(43306369) SHA1(1eadebd3d962da49fd204eff8692f1e1a1e3cc98) )
	ROM_LOAD( "tvg5-16a.10", 0x6000,  0x2000, CRC(da9c635f) SHA1(3c084ad159dbabfd02a9772489c3193852d135b7) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sub CPU */
	ROM_LOAD( "tvg1-16.2",   0x0000,  0x2000, CRC(e36009f6) SHA1(72c485e8c19fbfc9c850094cfd87f1055154c0c5) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "tvg6-16.90",  0x0000,  0x2000, CRC(6905d9d5) SHA1(586bf72bab5ab6e3e319c925decc16d7f3711af1) )
	ROM_LOAD( "tvg7-16.92",  0x2000,  0x2000, CRC(40d02bfd) SHA1(2f6ca8197048318f7900b56169aba4c9fdf48693) )
	ROM_LOAD( "tvg8-16.94",  0x4000,  0x2000, CRC(b18a2244) SHA1(168061e050530e6a5bc78c14a64e635370256dfd) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "tvg11-16.102",0x0000,  0x2000, CRC(327b7a29) SHA1(4b8d57607c4a1e84c630c38eba3fa90b5496dcde) )
	ROM_LOAD( "tvg10-16.101",0x2000,  0x2000, CRC(624ac061) SHA1(9d479a8a256a8ff37c00bc7449b11357f9fe6cdc) )
	ROM_LOAD( "tvg9-16.100", 0x4000,  0x2000, CRC(c477e74c) SHA1(864eddcd9c817aeecb09423071f87d3b39eb5fc4) )

	ROM_REGION( 0x0700, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "16-3.66",  0x0000,  0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) /* R       Prom type 24s10 */
	ROM_LOAD( "16-4.67",  0x0100,  0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) /* G       Prom type 24s10 */
	ROM_LOAD( "16-5.68",  0x0200,  0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) /* B       Prom type 24s10 */
	ROM_LOAD( "16-1.148", 0x0300,  0x0200, CRC(777e2770) SHA1(7f4ef42ab4e0546c2932d498cf573bd4f4296db7) ) /* sprite  Prom type mb7124h */
	ROM_LOAD( "16-2.97",  0x0500,  0x0200, CRC(7f95d4b2) SHA1(68dc311739a4d5d72f4cfbace27f3a82f05316ff) ) /* bg      Prom type mb7124h */

	ROM_REGION( 0x0100, "user1", 0 ) /* scroll control PROM */
	ROM_LOAD( "16-6.59",  0x0000,  0x0100, CRC(ec4faf5b) SHA1(7ebbf50807d04105ebadec91bded069408e399ba) ) /* Prom type 24s10 */

	ROM_REGION( 0x1000, "user2", 0 ) /* protection data used with Fujitsu MB8841 4-Bit MCU */
	ROM_LOAD( "tvg12-16.2", 0x0000,  0x1000, CRC(3abc6ba8) SHA1(15e0b0f9d068f6094e2be4f4f1dea0ff6e85686b) )
ROM_END

ROM_START( banbam )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "ban-rom2.ic7",  0x0000,  0x2000, CRC(a5aeef6e) SHA1(92c1ad6ccb96b723e122899150e3c1855c5017b8) )
	ROM_CONTINUE(              0x8000,  0x2000 )
	ROM_LOAD( "ban-rom3.ic8",  0x2000,  0x2000, CRC(f91472bf) SHA1(94988b7bf2be3d3704a802db544070251d6c6a9c) )
	ROM_LOAD( "ban-rom4.ic9",  0x4000,  0x2000, CRC(436a09ef) SHA1(7287f741fbf3ac43d5c46b2f43ec4439cb3d0d56) )
	ROM_LOAD( "ban-rom5.ic10", 0x6000,  0x2000, CRC(45205f86) SHA1(a11d10beb21519f797c902b8a18775c8e2aa0fae) )

	ROM_REGION( 0x10000, "sub", 0 ) /* sub CPU */
	ROM_LOAD( "ban-rom1.ic2",   0x0000,  0x2000, CRC(e36009f6) SHA1(72c485e8c19fbfc9c850094cfd87f1055154c0c5) )

	ROM_REGION( 0x6000, "gfx1", 0 ) /* sprite */
	ROM_LOAD( "ban-rom6.ic90",  0x0000,  0x2000, CRC(41fc44df) SHA1(1c4f21cdc423078fab58370d5245a13292bf7fe6) )
	ROM_LOAD( "ban-rom7.ic92",  0x2000,  0x2000, CRC(8b429c5b) SHA1(505796eac2c8dd84f9ed29a6227b3243f81ec072) )
	ROM_LOAD( "ban-rom8.ic94",  0x4000,  0x2000, CRC(76c02d6b) SHA1(9ef7585da5376cdd604afee281594e24c69addf2) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* bg */
	ROM_LOAD( "ban-rom11.ic102", 0x0000,  0x2000, CRC(aa827c57) SHA1(87d0c5e7df6ce40b0b2fc8f4b9dd43d4b7b0bc2e) )
	ROM_LOAD( "ban-rom10.ic101", 0x2000,  0x2000, CRC(51bd1c5c) SHA1(f714a3168d59cb4f8512440f7a2e27b6e0726a39) )
	ROM_LOAD( "ban-rom9.ic100",  0x4000,  0x2000, CRC(c0a5a4c8) SHA1(7f0978669218982d379a5d72c6198a33a8213ab5) )

	ROM_REGION( 0x0700, "proms", 0 ) /* color PROMs */
	ROM_LOAD( "16-3.66",  0x0000,  0x0100, CRC(dbcd3bec) SHA1(1baeec277b16c82b67e10da9d4c84cf383ef4a82) ) /* R       Prom type 24s10 */
	ROM_LOAD( "16-4.67",  0x0100,  0x0100, CRC(9eb7b6cf) SHA1(86451e8a510f8cfbc0be7d4e7bb1ee7dfd67f1f4) ) /* G       Prom type 24s10 */
	ROM_LOAD( "16-5.68",  0x0200,  0x0100, CRC(9b30a7f3) SHA1(a0aefc2c8325b95ea227e404583d14622b04a3b9) ) /* B       Prom type 24s10 */
	ROM_LOAD( "16-1.148", 0x0300,  0x0200, CRC(777e2770) SHA1(7f4ef42ab4e0546c2932d498cf573bd4f4296db7) ) /* sprite  Prom type mb7124h */
	ROM_LOAD( "16-2.97",  0x0500,  0x0200, CRC(7f95d4b2) SHA1(68dc311739a4d5d72f4cfbace27f3a82f05316ff) ) /* bg      Prom type mb7124h */

	ROM_REGION( 0x0100, "user1", 0 ) /* scroll control PROM */
	ROM_LOAD( "16-6.59",  0x0000,  0x0100, CRC(ec4faf5b) SHA1(7ebbf50807d04105ebadec91bded069408e399ba) ) /* Prom type 24s10 */

	ROM_REGION( 0x2000, "user2", 0 ) /* protection, data used with Fujitsu MB8841 4-Bit MCU */
	ROM_LOAD( "ban-rom12.ic2", 0x0000,  0x2000, CRC(044bb2f6) SHA1(829b2152740061e0506c7504885d8404fb8fe360) )

	ROM_REGION( 0x800, "mcu", 0 ) /* Fujitsu MB8841 4-Bit MCU internal ROM */
	ROM_LOAD( "sun-8212.ic3", 0x000,  0x800, BAD_DUMP CRC(8869611e) SHA1(c6443f3bcb0cdb4d7b1b19afcbfe339c300f36aa) )
ROM_END

DRIVER_INIT_MEMBER(strnskil_state,pettanp)
{
//  AM_RANGE(0xd80c, 0xd80c) AM_WRITENOP     /* protection reset? */
//  AM_RANGE(0xd80d, 0xd80d) AM_WRITE(protection_w) /* protection data write (pettanp) */
//  AM_RANGE(0xd806, 0xd806) AM_READ(protection_r) /* protection data read (pettanp) */

	/* Fujitsu MB8841 4-Bit MCU */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd806, 0xd806, read8_delegate(FUNC(strnskil_state::pettanp_protection_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd80d, 0xd80d, write8_delegate(FUNC(strnskil_state::protection_w),this));

}

DRIVER_INIT_MEMBER(strnskil_state,banbam)
{
	/* Fujitsu MB8841 4-Bit MCU */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd806, 0xd806, read8_delegate(FUNC(strnskil_state::banbam_protection_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd80d, 0xd80d, write8_delegate(FUNC(strnskil_state::protection_w),this));
}

GAME( 1984, strnskil, 0,        strnskil, strnskil, driver_device, 0,       ROT0, "Sun Electronics", "Strength & Skill", MACHINE_SUPPORTS_SAVE )
GAME( 1984, guiness,  strnskil, strnskil, strnskil, driver_device, 0,       ROT0, "Sun Electronics", "The Guiness (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, banbam,   0,        banbam,   banbam, strnskil_state,   banbam,  ROT0, "Sun Electronics", "BanBam", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1984, pettanp,  banbam,   strnskil, banbam, strnskil_state,   pettanp, ROT0, "Sun Electronics", "Pettan Pyuu (Japan)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
