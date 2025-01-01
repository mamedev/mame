// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Game Driver for Nichibutsu Mahjong series.

    Niyanpai
    (c)1996 Nihon Bussan Co.,Ltd.

    Musoubana
    (c)1995 Nihon Bussan Co.,Ltd. / Yubis Co.,Ltd.

    Mahjong 4P Shimasho
    (c)1994 SPHINX/AV JAPAN

    Mahjong Housoukyoku Honbanchuu
    (c)199? Nihon Bussan Co.,Ltd.

    Zoku Mahjong Housoukyoku Honbanchuu
    (c)199? Nihon Bussan Co.,Ltd.

    Driver by Takahiro Nogi 2000/12/23 -

******************************************************************************/
/******************************************************************************
Memo:

- niyanpai's 2p start does not mean 2p simultaneous or exchanging play.
  Simply uses controls for 2p side.

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  in Debug build.

- Screen flip is not perfect.

******************************************************************************/

#include "emu.h"
#include "niyanpai.h"

#include "machine/nvram.h"



void niyanpai_state::init_niyanpai()
{
	//uint8_t *SNDROM = memregion("nichisnd:audiorom")->base();

	// sound program patch
	//SNDROM[0x0213] = 0x00;          // DI -> NOP

	// initialize sound rom bank
	//membank("soundbank")->configure_entries(0, 3, memregion("nichisnd:audiorom")->base() + 0x8000, 0x8000);
	//membank("soundbank")->set_entry(0);

	// initialize out coin flag (musobana)
	m_musobana_outcoin_flag = 1;
}


uint16_t niyanpai_state::dipsw_r()
{
	uint8_t dipsw_a = ioport("DSWA")->read();
	uint8_t dipsw_b = ioport("DSWB")->read();

	dipsw_a = bitswap<8>(dipsw_a,0,1,2,3,4,5,6,7);
	dipsw_b = bitswap<8>(dipsw_b,0,1,2,3,4,5,6,7);

	return ((dipsw_a << 8) | dipsw_b);
}

MACHINE_START_MEMBER(niyanpai_state, musobana)
{
	save_item(NAME(m_motor_on));
	save_item(NAME(m_musobana_inputport));
	save_item(NAME(m_musobana_outcoin_flag));
}

uint16_t niyanpai_state::musobana_inputport_0_r()
{
	int portdata;

	switch ((m_musobana_inputport ^ 0xff00) >> 8)
	{
		case 0x01:  portdata = ioport("KEY0")->read(); break;
		case 0x02:  portdata = ioport("KEY1")->read(); break;
		case 0x04:  portdata = ioport("KEY2")->read(); break;
		case 0x08:  portdata = ioport("KEY3")->read(); break;
		case 0x10:  portdata = ioport("KEY4")->read(); break;
		default:    portdata = ioport("KEY0")->read() & ioport("KEY1")->read() & ioport("KEY2")->read()
								& ioport("KEY3")->read() & ioport("KEY4")->read(); break;
	}

	return (portdata);
}

void niyanpai_state::tmp68301_parallel_port_w(uint16_t data)
{
	// tmp68301_parallel_interface[0x05]
	//  bit 0   coin counter
	//  bit 2   motor on
	//  bit 3   coin lock
	//  bit 8-9 video page select?

	m_motor_on = data & 4;
	machine().bookkeeping().coin_counter_w(0,data & 1);
	machine().bookkeeping().coin_lockout_w(0,data & 0x08);
}

int niyanpai_state::musobana_outcoin_flag_r()
{
	if (m_motor_on) m_musobana_outcoin_flag ^= 1;
	else m_musobana_outcoin_flag = 1;

	return m_musobana_outcoin_flag & 0x01;
}

void niyanpai_state::musobana_inputport_w(uint16_t data)
{
	m_musobana_inputport = data;
}

void niyanpai_state::niyanpai_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x040fff).ram().share("nvram");

	map(0x0a0000, 0x0a08ff).rw(FUNC(niyanpai_state::palette_r), FUNC(niyanpai_state::palette_w));
	map(0x0a0900, 0x0a11ff).ram(); // palette work ram?

	map(0x0bf800, 0x0bffff).ram();

	map(0x200000, 0x200000).w("nichisnd", FUNC(nichisnd_device::sound_host_command_w));

	map(0x200200, 0x200201).nopw();            // unknown
	map(0x240000, 0x240009).nopw();            // unknown
	map(0x240200, 0x2403ff).nopw();            // unknown

	map(0x240400, 0x240403).r(FUNC(niyanpai_state::blitter_0_r)).umask16(0x00ff);
	map(0x240400, 0x24041f).w(FUNC(niyanpai_state::blitter_0_w)).umask16(0x00ff);
	map(0x240420, 0x24043f).w(FUNC(niyanpai_state::clut_0_w)).umask16(0x00ff);
	map(0x240600, 0x240603).r(FUNC(niyanpai_state::blitter_1_r)).umask16(0x00ff);
	map(0x240600, 0x24061f).w(FUNC(niyanpai_state::blitter_1_w)).umask16(0x00ff);
	map(0x240620, 0x24063f).w(FUNC(niyanpai_state::clut_1_w)).umask16(0x00ff);
	map(0x240800, 0x240803).r(FUNC(niyanpai_state::blitter_2_r)).umask16(0x00ff);
	map(0x240800, 0x24081f).w(FUNC(niyanpai_state::blitter_2_w)).umask16(0x00ff);
	map(0x240820, 0x24083f).w(FUNC(niyanpai_state::clut_2_w)).umask16(0x00ff);
	map(0x280000, 0x280001).r(FUNC(niyanpai_state::dipsw_r));

	map(0x280200, 0x280201).portr("P1_P2");
	map(0x280400, 0x280401).portr("SYSTEM");
	map(0x240a01, 0x240a01).w(FUNC(niyanpai_state::clutsel_0_w));
	map(0x240c01, 0x240c01).w(FUNC(niyanpai_state::clutsel_1_w));
	map(0x240e01, 0x240e01).w(FUNC(niyanpai_state::clutsel_2_w));
}

void niyanpai_state::musobana_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x040fff).ram();

	map(0x0a0000, 0x0a08ff).rw(FUNC(niyanpai_state::palette_r), FUNC(niyanpai_state::palette_w));
	map(0x0a0900, 0x0a11ff).ram();             // palette work ram?

	map(0x0a8000, 0x0a87ff).ram().share("nvram");
	map(0x0bf800, 0x0bffff).ram();

	map(0x200000, 0x200000).w("nichisnd", FUNC(nichisnd_device::sound_host_command_w));

	map(0x200200, 0x200201).w(FUNC(niyanpai_state::musobana_inputport_w)); // inputport select
	map(0x240000, 0x240009).nopw();            // unknown
	map(0x240200, 0x2403ff).nopw();            // unknown

	map(0x240400, 0x240403).r(FUNC(niyanpai_state::blitter_0_r)).umask16(0x00ff);
	map(0x240400, 0x24041f).w(FUNC(niyanpai_state::blitter_0_w)).umask16(0x00ff);
	map(0x240420, 0x24043f).w(FUNC(niyanpai_state::clut_0_w)).umask16(0x00ff);

	map(0x240600, 0x240603).r(FUNC(niyanpai_state::blitter_1_r)).umask16(0x00ff);
	map(0x240600, 0x24061f).w(FUNC(niyanpai_state::blitter_1_w)).umask16(0x00ff);
	map(0x240620, 0x24063f).w(FUNC(niyanpai_state::clut_1_w)).umask16(0x00ff);

	map(0x240800, 0x240803).r(FUNC(niyanpai_state::blitter_2_r)).umask16(0x00ff);
	map(0x240800, 0x24081f).w(FUNC(niyanpai_state::blitter_2_w)).umask16(0x00ff);
	map(0x240820, 0x24083f).w(FUNC(niyanpai_state::clut_2_w)).umask16(0x00ff);
	map(0x240a01, 0x240a01).w(FUNC(niyanpai_state::clutsel_0_w));
	map(0x240c01, 0x240c01).w(FUNC(niyanpai_state::clutsel_1_w));
	map(0x240e01, 0x240e01).w(FUNC(niyanpai_state::clutsel_2_w));

	map(0x280000, 0x280001).r(FUNC(niyanpai_state::dipsw_r));
	map(0x280200, 0x280201).r(FUNC(niyanpai_state::musobana_inputport_0_r));
	map(0x280400, 0x280401).portr("SYSTEM");
}

void niyanpai_state::mhhonban_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x040fff).ram();

	map(0x060000, 0x0608ff).rw(FUNC(niyanpai_state::palette_r), FUNC(niyanpai_state::palette_w));
	map(0x060900, 0x0611ff).ram();             // palette work ram?
	map(0x07f800, 0x07ffff).ram();

	map(0x0a8000, 0x0a87ff).ram().share("nvram");
	map(0x0bf000, 0x0bffff).ram();

	map(0x200000, 0x200000).w("nichisnd", FUNC(nichisnd_device::sound_host_command_w));

	map(0x200200, 0x200201).w(FUNC(niyanpai_state::musobana_inputport_w)); // inputport select
	map(0x240000, 0x240009).nopw();            // unknown
	map(0x240200, 0x2403ff).nopw();            // unknown

	map(0x240400, 0x240403).r(FUNC(niyanpai_state::blitter_0_r)).umask16(0x00ff);
	map(0x240400, 0x24041f).w(FUNC(niyanpai_state::blitter_0_w)).umask16(0x00ff);
	map(0x240420, 0x24043f).w(FUNC(niyanpai_state::clut_0_w)).umask16(0x00ff);

	map(0x240600, 0x240603).r(FUNC(niyanpai_state::blitter_1_r)).umask16(0x00ff);
	map(0x240600, 0x24061f).w(FUNC(niyanpai_state::blitter_1_w)).umask16(0x00ff);
	map(0x240620, 0x24063f).w(FUNC(niyanpai_state::clut_1_w)).umask16(0x00ff);

	map(0x240800, 0x240803).r(FUNC(niyanpai_state::blitter_2_r)).umask16(0x00ff);
	map(0x240800, 0x24081f).w(FUNC(niyanpai_state::blitter_2_w)).umask16(0x00ff);
	map(0x240820, 0x24083f).w(FUNC(niyanpai_state::clut_2_w)).umask16(0x00ff);

	map(0x240a01, 0x240a01).w(FUNC(niyanpai_state::clutsel_0_w));
	map(0x240c01, 0x240c01).w(FUNC(niyanpai_state::clutsel_1_w));
	map(0x240e01, 0x240e01).w(FUNC(niyanpai_state::clutsel_2_w));

	map(0x280000, 0x280001).r(FUNC(niyanpai_state::dipsw_r));
	map(0x280200, 0x280201).r(FUNC(niyanpai_state::musobana_inputport_0_r));
	map(0x280400, 0x280401).portr("SYSTEM");
}

void niyanpai_state::zokumahj_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x0ff000, 0x0fffff).ram();

	map(0x0e0000, 0x0e08ff).rw(FUNC(niyanpai_state::palette_r), FUNC(niyanpai_state::palette_w));
	map(0x0e0900, 0x0e11ff).ram();             // palette work ram?

	map(0x0a8000, 0x0a87ff).ram().share("nvram");
	map(0x0c0000, 0x0cffff).ram();

	map(0x200000, 0x200000).w("nichisnd", FUNC(nichisnd_device::sound_host_command_w));

	map(0x200200, 0x200201).w(FUNC(niyanpai_state::musobana_inputport_w)); // inputport select
	map(0x240000, 0x240009).nopw();            // unknown
	map(0x240200, 0x2403ff).nopw();            // unknown

	map(0x240400, 0x240403).r(FUNC(niyanpai_state::blitter_0_r)).umask16(0x00ff);
	map(0x240400, 0x24041f).w(FUNC(niyanpai_state::blitter_0_w)).umask16(0x00ff);
	map(0x240420, 0x24043f).w(FUNC(niyanpai_state::clut_0_w)).umask16(0x00ff);

	map(0x240600, 0x240603).r(FUNC(niyanpai_state::blitter_1_r)).umask16(0x00ff);
	map(0x240600, 0x24061f).w(FUNC(niyanpai_state::blitter_1_w)).umask16(0x00ff);
	map(0x240620, 0x24063f).w(FUNC(niyanpai_state::clut_1_w)).umask16(0x00ff);

	map(0x240800, 0x240803).r(FUNC(niyanpai_state::blitter_2_r)).umask16(0x00ff);
	map(0x240800, 0x24081f).w(FUNC(niyanpai_state::blitter_2_w)).umask16(0x00ff);
	map(0x240820, 0x24083f).w(FUNC(niyanpai_state::clut_2_w)).umask16(0x00ff);

	map(0x240a01, 0x240a01).w(FUNC(niyanpai_state::clutsel_0_w));
	map(0x240c01, 0x240c01).w(FUNC(niyanpai_state::clutsel_1_w));
	map(0x240e01, 0x240e01).w(FUNC(niyanpai_state::clutsel_2_w));

	map(0x280000, 0x280001).r(FUNC(niyanpai_state::dipsw_r));
	map(0x280200, 0x280201).r(FUNC(niyanpai_state::musobana_inputport_0_r));
	map(0x280400, 0x280401).portr("SYSTEM");
}


static INPUT_PORTS_START( niyanpai )
	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )           // ?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )            // COIN1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )            // COIN2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )           // CREDIT CLEAR
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )           // START2
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 )         // ANALYZER
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START1 )           // START1
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )          // ?
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )                   // TEST

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSWA:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSWA:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Game Sounds" )       PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, "Nudity" )            PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )      PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nbmjctrl_16 )
	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( musobana )    // I don't have manual for this game.
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, "Game Out" )          PORT_DIPLOCATION("DSWA:1,2")
	PORT_DIPSETTING(    0x03, "90% (Easy)" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "70%" )
	PORT_DIPSETTING(    0x00, "60% (Hard)" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSWA:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )         PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )         PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Game Type" )         PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, "Medal Type" )
	PORT_DIPSETTING(    0x00, "Credit Type" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, "Bet Min" )           PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x00, "Bet Max" )           PORT_DIPLOCATION("DSWB:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )         PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Score Pool" )        PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )         PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )         PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )            // COIN1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )            // COIN2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )           // CREDIT CLEAR
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MEMORY_RESET )     // MEMORY RESET
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 )         // ANALYZER
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(niyanpai_state::musobana_outcoin_flag_r))   // OUT COIN
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )                   // TEST

	PORT_INCLUDE( nbmjctrl_16 )
INPUT_PORTS_END

static INPUT_PORTS_START( 4psimasy )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x05, DEF_STR( Medium_Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x00, "Game Sounds" )               PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWA:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )

	/* this switch not mentioned by the manual */
	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "DSWB:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "DSWB:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "DSWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "DSWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "DSWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "DSWB:6" )
	PORT_DIPNAME( 0x40, 0x40, "Option Test" )               PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )          PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )            // COIN1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )            // COIN2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )           // CREDIT CLEAR
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MEMORY_RESET )     // MEMORY RESET
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 )         // ANALYZER
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(niyanpai_state::musobana_outcoin_flag_r))   // OUT COIN
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )                   // TEST

	PORT_INCLUDE( nbmjctrl_16 )
INPUT_PORTS_END

static INPUT_PORTS_START( mhhonban )    // I don't have manual for this game.
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )         PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )         PORT_DIPLOCATION("DSWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )         PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, "DIPSW 1-5" )         PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )         PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )         PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )         PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )         PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )         PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )         PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )         PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )         PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )         PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Option Test" )       PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )            // COIN1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )            // COIN2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )           // CREDIT CLEAR
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MEMORY_RESET )     // MEMORY RESET
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 )         // ANALYZER
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(niyanpai_state::musobana_outcoin_flag_r))   // OUT COIN
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )                   // TEST

	PORT_INCLUDE( nbmjctrl_16 )
INPUT_PORTS_END

static INPUT_PORTS_START( zokumahj )    // I don't have manual for this game.
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )         PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )         PORT_DIPLOCATION("DSWA:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )         PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )         PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )         PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )         PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )         PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )         PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )         PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )         PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )         PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )         PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )         PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Option Test" )       PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )            // COIN1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )            // COIN2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )           // CREDIT CLEAR
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_MEMORY_RESET )     // MEMORY RESET
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 )         // ANALYZER
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(niyanpai_state::musobana_outcoin_flag_r))   // OUT COIN
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )                   // TEST

	PORT_INCLUDE( nbmjctrl_16 )
INPUT_PORTS_END


void niyanpai_state::niyanpai(machine_config &config)
{
	/* basic machine hardware */
	TMP68301(config, m_maincpu, 12288000/2); /* TMP68301, 6.144 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &niyanpai_state::niyanpai_map);
	m_maincpu->parallel_w_cb().set(FUNC(niyanpai_state::tmp68301_parallel_port_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(1024, 512);
	m_screen->set_visarea(0, 640-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(niyanpai_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0);

	PALETTE(config, m_palette).set_entries(256*3);

	/* sound hardware */
	NICHISND(config, "nichisnd", 0);
}

void niyanpai_state::musobana(machine_config &config)
{
	niyanpai(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &niyanpai_state::musobana_map);

	MCFG_MACHINE_START_OVERRIDE(niyanpai_state, musobana)
}

void niyanpai_state::mhhonban(machine_config &config)
{
	musobana(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &niyanpai_state::mhhonban_map);
}

void niyanpai_state::zokumahj(machine_config &config)
{
	musobana(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &niyanpai_state::zokumahj_map);
}


ROM_START( niyanpai )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "npai_01.bin", 0x00000, 0x20000, CRC(a904e8a1) SHA1(77865d7b48cac96af1e3cac4a702f7de4b5ee82b) )
	ROM_LOAD16_BYTE( "npai_02.bin", 0x00001, 0x20000, CRC(244f9d6f) SHA1(afde18f32c4879a66c0707671d783c21c54cffa4) )

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "npai_03.bin", 0x000000, 0x20000, CRC(d154306b) SHA1(3375568a6d387d850b8996b8bad3d0220de13993) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "npai_04.bin", 0x000000, 0x80000, CRC(bec845b5) SHA1(2b00b4fd0bdda84cdc08933e593afdd91dde8d07) )
	ROM_LOAD( "npai_05.bin", 0x080000, 0x80000, CRC(3300ce07) SHA1(dc2eeb804aaf0aeb6cfee1844260ea24c3164bd9) )
	ROM_LOAD( "npai_06.bin", 0x100000, 0x80000, CRC(448e4e39) SHA1(63ca27f76a23235d3538d7f6c18dcc309e0f1f1c) )
	ROM_LOAD( "npai_07.bin", 0x180000, 0x80000, CRC(2ad47e55) SHA1(dbda82e654a85b0d5303bffa3005aaf78bdf0d28) )
	ROM_LOAD( "npai_08.bin", 0x200000, 0x80000, CRC(2ff980a0) SHA1(055addac657a5f7ec37ba85385834805c7aa0402) )
	ROM_LOAD( "npai_09.bin", 0x280000, 0x80000, CRC(74037ee3) SHA1(d975e6af962b9c62304ac15adab46c0ce972194b) )
	ROM_LOAD( "npai_10.bin", 0x300000, 0x80000, CRC(d35a9af6) SHA1(9a41aeea84c59b194bd122e2f102476834303302) )
	ROM_LOAD( "npai_11.bin", 0x380000, 0x80000, CRC(0748eb73) SHA1(63849f6625928646238a76748fd7903cee3ece2e) )
ROM_END

ROM_START( musobana )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "1.209", 0x00000, 0x20000, CRC(574929a1) SHA1(70ea96c3aa8a3512176b719de0928470541d85cb) )
	ROM_LOAD16_BYTE( "2.208", 0x00001, 0x20000, CRC(12734fda) SHA1(46241efe4266ad6426eb31db757ae4852c70c25d) )

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "3.804",  0x000000, 0x20000, CRC(0be8f2ce) SHA1(c1ee8907c03f615fbc42654a3c37387714761560) )

	ROM_REGION( 0x500000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.102",  0x000000, 0x80000, CRC(1b5dcff8) SHA1(afc44d8a381e1f6059e8e29d415799f863ba8528) )
	ROM_LOAD( "5.103",  0x080000, 0x80000, CRC(dd69b24a) SHA1(2d1986f2b24877cfb4df9c32d76e4c4aada11420) )
	ROM_LOAD( "6.104",  0x100000, 0x80000, CRC(e898f3a2) SHA1(4d5002105b3a20f962a0f31c7703e16fcd4970aa) )
	ROM_LOAD( "7.105",  0x180000, 0x80000, CRC(812cb79a) SHA1(f905663f8656270c4cdda4a8547c57e9f3e1093b) )
	ROM_LOAD( "8.106",  0x200000, 0x80000, CRC(20285661) SHA1(503650148e07af9c34b22ae60b4a10c253f694aa) )
	ROM_LOAD( "9.107",  0x280000, 0x80000, CRC(91dfb28b) SHA1(4d673957533e6ef155765b71e3b0010455f2968b) )
	ROM_LOAD( "10.108", 0x300000, 0x80000, CRC(5c8e2300) SHA1(0e36d32d679f80d76a4c947e65d56ae449b03966) )
	ROM_LOAD( "11.109", 0x380000, 0x80000, CRC(12894ba4) SHA1(dcaef30283dfb5b97cc8a48247888434ffa7ec81) )
	ROM_LOAD( "12.202", 0x400000, 0x80000, CRC(d02957e0) SHA1(54f5e4724cc31030bc2185ac9d00fa57cc5c2255) )
	ROM_LOAD( "13.203", 0x480000, 0x80000, CRC(240ffce3) SHA1(8b357e962af8081bd01603ce1a3be6d2a7a4ed2a) )
ROM_END

ROM_START( 4psimasy )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "1.209", 0x00000, 0x20000, CRC(28dda353) SHA1(3d4738189a7b8b8b0434b3e58550572c3ce74b42) )
	ROM_LOAD16_BYTE( "2.208", 0x00001, 0x20000, CRC(3679c9fb) SHA1(74a940c3c95723680a63a281f194ef4bbe3dc58a) )

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "3.804",  0x000000, 0x20000, CRC(bd644726) SHA1(1f8e12a081657d6e1dd9c896056d1ffd977dfe95) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.102",  0x000000, 0x80000, CRC(66c96d20) SHA1(2e8c6876c52fdc9afda4c29c84568942e3fe7fb8) )
	ROM_LOAD( "5.103",  0x080000, 0x80000, CRC(d8787e7d) SHA1(85a69f69da25159e0f7f75370ffaa3b8cb754eb0) )
	ROM_LOAD( "6.104",  0x100000, 0x80000, CRC(ad68defc) SHA1(fe6e0fd88dfbb20e13efb8ab80bc41c19963e6d7) )
	ROM_LOAD( "7.105",  0x180000, 0x80000, CRC(daceaa7b) SHA1(6b8ab028653f2546ab56860d81a3f5a8bbd6dede) )
	ROM_LOAD( "8.106",  0x200000, 0x80000, CRC(469032ce) SHA1(8ea743646acf0e2e0860ca092b0a45a43d19333f) )
	ROM_LOAD( "9.107",  0x280000, 0x80000, CRC(fa87a29e) SHA1(14c33f66efcbd4cbb8de16ace609711cc87e9ece) )
	ROM_LOAD( "10.108", 0x300000, 0x80000, CRC(f5608b85) SHA1(96f817ead285fefd5d1ebb1d4dd1f79125b110be) )
	ROM_LOAD( "11.109", 0x380000, 0x80000, CRC(1659f8d0) SHA1(daa5a2c5c7b5dc362581268fe98897164ccaa316) )
ROM_END

ROM_START( mhhonban )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "u209.bin", 0x00000, 0x20000, CRC(121c861f) SHA1(70a6b695998904dccb8791ea5d9acbf7484bd812) )
	ROM_LOAD16_BYTE( "u208.bin", 0x00001, 0x20000, CRC(d6712d0b) SHA1(a384c8f508ec6885bccb989d150cfd7f36a6898d) )

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "u804.bin",  0x000000, 0x20000, CRC(48407507) SHA1(afd24d16d487fd2b6548d967e2f1ae122e2633a2) )

	ROM_REGION( 0x300000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "u102.bin",  0x000000, 0x80000, CRC(385b51aa) SHA1(445e365e762e60d6189d84608459f7d18fff859f) )
	ROM_LOAD( "u103.bin",  0x080000, 0x80000, CRC(1b85c6f4) SHA1(f8417b2526a8b51e52117d7d2690ce70af5e90fa) )
	ROM_LOAD( "u104.bin",  0x100000, 0x80000, CRC(0f091b1d) SHA1(f53425524a22ab0be241dc4303be7e1403989f3a) )
	ROM_LOAD( "u105.bin",  0x180000, 0x80000, CRC(20b39ac9) SHA1(b32b56c52cc6b79000588ad2cc8bfa533d7203f6) )
	ROM_LOAD( "u106.bin",  0x200000, 0x80000, CRC(11f42938) SHA1(f7cdc21cdefa8476090cc6e5b87b220b001fbeb1) )
	ROM_LOAD( "u107.bin",  0x280000, 0x80000, CRC(efc85912) SHA1(b9d523fd5f9ce879e2ed6916c89940be1d738a1c) )
ROM_END

ROM_START( zokumahj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* TMP68301 main program */
	ROM_LOAD16_BYTE( "1.bin", 0x00000, 0x20000, CRC(53ca34a9) SHA1(5a1e2660442665efd529ec6c98ffc994c6103419) )
	ROM_LOAD16_BYTE( "2.bin", 0x00001, 0x20000, CRC(474f9303) SHA1(4b03e3b6b6ee864dfcce3978f19bf329e39b3fe7) )

	ROM_REGION( 0x20000, "nichisnd:audiorom", 0 ) /* TMPZ84C011 sound program */
	ROM_LOAD( "3.bin",  0x000000, 0x20000, CRC(48407507) SHA1(afd24d16d487fd2b6548d967e2f1ae122e2633a2) )

	ROM_REGION( 0x300000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.bin",  0x000000, 0x80000, CRC(2b3e88b4) SHA1(d49a81f1bc6ac5b56df36caf7ee7188a917d947f) )
	ROM_LOAD( "5.bin",  0x080000, 0x80000, CRC(cfe9477a) SHA1(9d08f5b1d638cef7d8dc97bdd9d98627f9af1ef9) )
	ROM_LOAD( "6.bin",  0x100000, 0x80000, CRC(2d62df76) SHA1(83fe8e0a853c0137e7c77ecde762617c082774e5) )
	ROM_LOAD( "7.bin",  0x180000, 0x80000, CRC(75922e76) SHA1(4ad23e241a1a223e0fcd4fd26bd695b1a68c258a) )
	ROM_LOAD( "8.bin",  0x200000, 0x80000, CRC(22b3befa) SHA1(e44635e021962ce29b4e129ae394c794038aef39) )
	ROM_LOAD( "9.bin",  0x280000, 0x80000, CRC(f72d83af) SHA1(4b897b40765084fd9483065fd0eb0e29cbcfa5ac) )
ROM_END


GAME( 1996, niyanpai, 0,        niyanpai, niyanpai, niyanpai_state, init_niyanpai, ROT0, "Nichibutsu",         "Niyanpai (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, musobana, 0,        musobana, musobana, niyanpai_state, init_niyanpai, ROT0, "Nichibutsu / Yubis", "Musoubana (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, 4psimasy, 0,        musobana, 4psimasy, niyanpai_state, init_niyanpai, ROT0, "Sphinx / AV Japan",  "Mahjong 4P Shimasho (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, mhhonban, 0,        mhhonban, mhhonban, niyanpai_state, init_niyanpai, ROT0, "Nichibutsu",         "Mahjong Housoukyoku Honbanchuu (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 199?, zokumahj, mhhonban, zokumahj, zokumahj, niyanpai_state, init_niyanpai, ROT0, "Nichibutsu?",        "Zoku Mahjong Housoukyoku (Japan)", MACHINE_SUPPORTS_SAVE )
