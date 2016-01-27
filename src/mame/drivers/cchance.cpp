// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/***************************************************************************************************************************

Cherry Chance (c) 1987 Taito Corporation?

driver by David Haywood & Angelo Salese

A cherry-type game that uses the tnzs video chip,might be a modified board as well.

TODO:
-imperfect hopper emulation makes this game to not coin up and gives "hopper time out errors" when you win (put dsw 1
(0x01) and 8 (0x80) to enable hopper full-mode);
-Wrong colors,caused by missing color proms;

============================================================================================================================

Cherry Chance Readme

This one should be a simple project, it uses a Z80 and a Seta graphics chipset and a YM2149 for sound. It is a slot machine.

There are 3 banks of dipswitches, 4,8,8. Battery backup of a 4364 cpu ram. 2 6264 video rams. All 5 eproms are 27512

Chip    checksum
cpu $ba0d
cha0    $2ed7
cha1    $dc81
cha2    $cca8
cha3    $10d8

2 color proms for the output. will get those dumped as well.


*****************************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/tnzs.h"
#include "sound/ay8910.h"

class cchance_state : public tnzs_state
{
public:
	cchance_state(const machine_config &mconfig, device_type type, const char *tag)
		: tnzs_state(mconfig, type, tag) { }

	UINT8 m_hop_io;
	UINT8 m_bell_io;
	DECLARE_WRITE8_MEMBER(output_0_w);
	DECLARE_READ8_MEMBER(input_1_r);
	DECLARE_WRITE8_MEMBER(output_1_w);
	DECLARE_MACHINE_START(cchance);
	DECLARE_MACHINE_RESET(cchance);
};


WRITE8_MEMBER(cchance_state::output_0_w)
{
	//---- --x- divider?
	machine().bookkeeping().coin_lockout_w(0, ~data & 1);

//  machine().bookkeeping().coin_counter_w(0, ~data & 1);
}


READ8_MEMBER(cchance_state::input_1_r)
{
	return (m_hop_io) | (m_bell_io) | (ioport("SP")->read() & 0xff);
}

WRITE8_MEMBER(cchance_state::output_1_w)
{
	m_hop_io = (data & 0x40)>>4;
	m_bell_io = (data & 0x80)>>4;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, cchance_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM

	AM_RANGE(0xa000, 0xafff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spritecodelow_r8, spritecodelow_w8)
	AM_RANGE(0xb000, 0xbfff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spritecodehigh_r8, spritecodehigh_w8)

	AM_RANGE(0xc000, 0xdfff) AM_RAM

	AM_RANGE(0xe000, 0xe2ff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spriteylow_r8, spriteylow_w8)
	AM_RANGE(0xe300, 0xe303) AM_RAM AM_MIRROR(0xfc) AM_DEVWRITE("spritegen", seta001_device, spritectrl_w8)  /* control registers (0x80 mirror used by Arkanoid 2) */
	AM_RANGE(0xe800, 0xe800) AM_DEVWRITE("spritegen", seta001_device, spritebgflag_w8)   /* enable / disable background transparency */

	AM_RANGE(0xf000, 0xf000) AM_READNOP AM_WRITENOP //???
	AM_RANGE(0xf001, 0xf001) AM_READ(input_1_r) AM_WRITE(output_0_w)
	AM_RANGE(0xf002, 0xf002) AM_READ_PORT("IN0") AM_WRITE(output_1_w)
	AM_RANGE(0xf800, 0xf801) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xf801, 0xf801) AM_DEVREAD("aysnd", ay8910_device, data_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( cchance )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet SW") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout SW")  PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("SP")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Opt 1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Opt 2") PORT_CODE(KEYCODE_S)
//  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_D)
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hop Over") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Slottle") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Drop SW") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Key") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Last Key") PORT_CODE(KEYCODE_K)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("DSW2") //likely to be unused
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// These ports are required in tnzs_state
	PORT_START("IN1")
	PORT_START("IN2")
	PORT_START("DSWA")
	PORT_START("DSWB")
INPUT_PORTS_END

static const gfx_layout cchance_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};



static GFXDECODE_START( cchance )
	GFXDECODE_ENTRY( "gfx1", 0, cchance_layout,   0x0, 32  )
GFXDECODE_END

MACHINE_START_MEMBER(cchance_state,cchance)
{
	save_item(NAME(m_hop_io));
	save_item(NAME(m_bell_io));
}

MACHINE_RESET_MEMBER(cchance_state,cchance)
{
	m_mcu_type = -1;
	m_hop_io = 0;
	m_bell_io = 0;

}

static MACHINE_CONFIG_START( cchance, cchance_state )

	MCFG_CPU_ADD("maincpu", Z80,4000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cchance_state,  irq0_line_hold)

	MCFG_MACHINE_START_OVERRIDE(cchance_state,cchance)
	MCFG_MACHINE_RESET_OVERRIDE(cchance_state,cchance)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cchance)

	MCFG_DEVICE_ADD("spritegen", SETA001_SPRITE, 0)
	MCFG_SETA001_SPRITE_GFXDECODE("gfxdecode")
	MCFG_SETA001_SPRITE_PALETTE("palette")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.5)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cchance_state, screen_update_tnzs)
	MCFG_SCREEN_VBLANK_DRIVER(cchance_state, screen_eof_tnzs)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INIT_OWNER(cchance_state,arknoid2)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 1500000/2)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

ROM_START( cchance )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("chance-cccpu.bin", 0x00000, 0x10000, CRC(77531028) SHA1(6f647dea3f1c5884c32a35e04ab6c8a61688171a) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD("chance-cccha0.bin", 0x30000, 0x10000, CRC(df8403cf) SHA1(997a6e07079fcbcae2fb82bbd7af0db9b90a03e0))
	ROM_LOAD("chance-cccha1.bin", 0x20000, 0x10000, CRC(26fddc7d) SHA1(d89757c28f14dccdc7d898e19fea59f41f4fa903) )
	ROM_LOAD("chance-cccha2.bin", 0x10000, 0x10000, CRC(fa5ccf5b) SHA1(21957a6a7b88c315d1fbb82e98a924a637a28397) )
	ROM_LOAD("chance-cccha3.bin", 0x00000, 0x10000, CRC(2a2979c9) SHA1(5036313e219ec561fa6753f0db6bb28c6fc97963) )

	ROM_REGION( 0x0400, "proms", 0 )        /* color proms */
	ROM_LOAD( "prom1", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom2", 0x0200, 0x0200, NO_DUMP )
ROM_END

GAME( 1987?, cchance,  0,    cchance, cchance, driver_device,  0, ROT0, "<unknown>", "Cherry Chance", MACHINE_NOT_WORKING | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
