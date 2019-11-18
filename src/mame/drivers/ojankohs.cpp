// license: BSD-3-Clause
// copyright-holders: Takahiro Nogi, Uki, Dirk Best
/***************************************************************************

    Video System Mahjong hardware

    Ojanko High School (お雀子ハイスクール)
    © 1988 V-System Co.

    Ojanko Yakata (勝ち抜き麻雀戦 お雀子館)
    © 1986 V-System Co.

    Ojanko Yakata 2bankan (勝ち抜き麻雀戦 お雀子館2番館)
    © 1987 V-System Co.

    Chinese Casino (チャイニーズカジノ)
    © 1987 V-System Co.

    Ojanko Club (お雀子クラブ)
    © 1986 V-System Co.

    TODO:
    - Figure out the rest of the dip switches
    - XTAL values/clocks
    - Raw screen params

***************************************************************************/

#include "emu.h"
#include "includes/ojankohs.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "video/vsystem_gga.h"
#include "speaker.h"


WRITE8_MEMBER(ojankohs_state::ojankohs_rombank_w)
{
	membank("bank1")->set_entry(data & 0x3f);
}

WRITE8_MEMBER(ojankohs_state::ojankoy_rombank_w)
{
	membank("bank1")->set_entry(data & 0x1f);

	m_adpcm_reset = BIT(data, 5);
	if (!m_adpcm_reset)
		m_vclk_left = 0;

	m_msm->reset_w(!m_adpcm_reset);
}

WRITE8_MEMBER(ojankohs_state::ojankohs_adpcm_reset_w)
{
	m_adpcm_reset = BIT(data, 0);
	m_vclk_left = 0;

	m_msm->reset_w(!m_adpcm_reset);
}

WRITE8_MEMBER(ojankohs_state::ojankohs_msm5205_w)
{
	m_adpcm_data = data;
	m_vclk_left = 2;
}

WRITE_LINE_MEMBER(ojankohs_state::ojankohs_adpcm_int)
{
	/* skip if we're reset */
	if (!m_adpcm_reset)
		return;

	/* clock the data through */
	if (m_vclk_left)
	{
		m_msm->write_data((m_adpcm_data >> 4));
		m_adpcm_data <<= 4;
		m_vclk_left--;
	}

	/* generate an NMI if we're out of data */
	if (!m_vclk_left)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE8_MEMBER(ojankohs_state::ojankoc_ctrl_w)
{
	membank("bank1")->set_entry(data & 0x0f);

	m_adpcm_reset = BIT(data, 4);
	m_msm->reset_w(!BIT(data, 4));
	ojankoc_flipscreen(space, data);
}


void ojankohs_state::ojankohs_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram().w(FUNC(ojankohs_state::ojankohs_videoram_w)).share("videoram");
	map(0x9000, 0x9fff).ram().w(FUNC(ojankohs_state::ojankohs_colorram_w)).share("colorram");
	map(0xa000, 0xb7ff).ram().share("nvram");
	map(0xb800, 0xbfff).ram().w(FUNC(ojankohs_state::ojankohs_palette_w)).share("paletteram");
	map(0xc000, 0xffff).bankr("bank1");
}


void ojankohs_state::ojankoy_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram().w(FUNC(ojankohs_state::ojankohs_videoram_w)).share("videoram");
	map(0xa000, 0xafff).ram().w(FUNC(ojankohs_state::ojankohs_colorram_w)).share("colorram");
	map(0xb000, 0xbfff).ram().share("nvram");
	map(0xc000, 0xffff).bankr("bank1");
}


void ojankohs_state::ojankoc_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).bankr("bank1").w(FUNC(ojankohs_state::ojankoc_videoram_w));
}


void ojankohs_state::ojankohs_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("system").w(FUNC(ojankohs_state::port_select_w));
	map(0x01, 0x01).rw(FUNC(ojankohs_state::keymatrix_p1_r), FUNC(ojankohs_state::ojankohs_rombank_w));
	map(0x02, 0x02).rw(FUNC(ojankohs_state::keymatrix_p2_r), FUNC(ojankohs_state::ojankohs_gfxreg_w));
	map(0x03, 0x03).w(FUNC(ojankohs_state::ojankohs_adpcm_reset_w));
	map(0x04, 0x04).w(FUNC(ojankohs_state::ojankohs_flipscreen_w));
	map(0x05, 0x05).w(FUNC(ojankohs_state::ojankohs_msm5205_w));
	map(0x06, 0x06).r("aysnd", FUNC(ym2149_device::data_r));
	map(0x06, 0x07).w("aysnd", FUNC(ym2149_device::data_address_w));
	map(0x10, 0x11).w("gga", FUNC(vsystem_gga_device::write));
}

void ojankohs_state::ojankoy_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("system").w(FUNC(ojankohs_state::port_select_w));
	map(0x01, 0x01).rw(FUNC(ojankohs_state::keymatrix_p1_r), FUNC(ojankohs_state::ojankoy_rombank_w));
	map(0x02, 0x02).rw(FUNC(ojankohs_state::keymatrix_p2_r), FUNC(ojankohs_state::ojankoy_coinctr_w));
	map(0x04, 0x04).w(FUNC(ojankohs_state::ojankohs_flipscreen_w));
	map(0x05, 0x05).w(FUNC(ojankohs_state::ojankohs_msm5205_w));
	map(0x06, 0x06).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x06, 0x07).w("aysnd", FUNC(ay8910_device::data_address_w));
}

void ojankohs_state::ccasino_io_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff00).portr("system").w(FUNC(ojankohs_state::port_select_w));
	map(0x01, 0x01).mirror(0xff00).rw(FUNC(ojankohs_state::keymatrix_p1_r), FUNC(ojankohs_state::ojankohs_rombank_w));
	map(0x02, 0x02).mirror(0xff00).rw(FUNC(ojankohs_state::keymatrix_p2_r), FUNC(ojankohs_state::ccasino_coinctr_w));
	map(0x03, 0x03).mirror(0xff00).r(FUNC(ojankohs_state::ccasino_dipsw3_r)).w(FUNC(ojankohs_state::ojankohs_adpcm_reset_w));
	map(0x04, 0x04).mirror(0xff00).r(FUNC(ojankohs_state::ccasino_dipsw4_r)).w(FUNC(ojankohs_state::ojankohs_flipscreen_w));
	map(0x05, 0x05).mirror(0xff00).w(FUNC(ojankohs_state::ojankohs_msm5205_w));
	map(0x06, 0x06).mirror(0xff00).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x06, 0x07).mirror(0xff00).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x08, 0x0f).select(0xff00).w(FUNC(ojankohs_state::ccasino_palette_w));     // 16bit address access
	map(0x10, 0x11).mirror(0xff00).w("gga", FUNC(vsystem_gga_device::write));
}

void ojankohs_state::ojankoc_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x1f).w(FUNC(ojankohs_state::ojankoc_palette_w));
	map(0xf9, 0xf9).w(FUNC(ojankohs_state::ojankohs_msm5205_w));
	map(0xfb, 0xfb).w(FUNC(ojankohs_state::ojankoc_ctrl_w));
	map(0xfc, 0xfc).r(FUNC(ojankohs_state::ojankoc_keymatrix_p1_r));
	map(0xfd, 0xfd).r(FUNC(ojankohs_state::ojankoc_keymatrix_p2_r));
	map(0xfd, 0xfd).w(FUNC(ojankohs_state::port_select_w));
	map(0xfe, 0xff).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0xff, 0xff).r("aysnd", FUNC(ay8910_device::data_r));
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( ojankohs )
	PORT_START("system")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_SERVICE2)     PORT_NAME("Freeze")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("coin")
	PORT_BIT(0x3f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p1_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p1_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p1_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p1_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p1_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p2_0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E)     PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I)     PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M)     PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN)   PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p2_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F)     PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J)     PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N)     PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET)   PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p2_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G)     PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K)     PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI)   PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON)   PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p2_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H)     PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L)     PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON)   PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("p2_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE)       PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP)   PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP)   PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG)         PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL)       PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("dsw1")
	PORT_DIPNAME(0x07, 0x07, DEF_STR( Difficulty ))       PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(   0x07, "1 (Easy)" )
	PORT_DIPSETTING(   0x03, "2" )
	PORT_DIPSETTING(   0x05, "3" )
	PORT_DIPSETTING(   0x01, "4" )
	PORT_DIPSETTING(   0x06, "5" )
	PORT_DIPSETTING(   0x02, "6" )
	PORT_DIPSETTING(   0x04, "7" )
	PORT_DIPSETTING(   0x00, "8 (Hard)" )
	PORT_DIPNAME(0x18, 0x18, DEF_STR( Coinage ))          PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(   0x18, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x10, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x00, DEF_STR( 3C_1C ))
	PORT_DIPNAME(0x20, 0x20, DEF_STR( Lives ))            PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(   0x20, "1")
	PORT_DIPSETTING(   0x00, "2")
	PORT_DIPNAME(0x40, 0x40, DEF_STR( Allow_Continue ))   PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(   0x00, DEF_STR( No ))
	PORT_DIPSETTING(   0x40, DEF_STR( Yes ))
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Flip_Screen ))      PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))

	PORT_START("dsw2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, IP_ACTIVE_LOW, "DSW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( ojankoy )
	PORT_INCLUDE(ojankohs)

	PORT_MODIFY("system")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Statistics")

	PORT_MODIFY("dsw1")
	PORT_DIPNAME(0x07, 0x07, DEF_STR( Difficulty ))
	PORT_DIPSETTING(   0x07, "1 (Easy)")
	PORT_DIPSETTING(   0x03, "2")
	PORT_DIPSETTING(   0x05, "3")
	PORT_DIPSETTING(   0x01, "4")
	PORT_DIPSETTING(   0x06, "5")
	PORT_DIPSETTING(   0x02, "6")
	PORT_DIPSETTING(   0x04, "7")
	PORT_DIPSETTING(   0x00, "8 (Hard)")
	PORT_DIPNAME(0x18, 0x18, "Player Initial Score")
	PORT_DIPSETTING(   0x18, "1000")
	PORT_DIPSETTING(   0x08, "2000")
	PORT_DIPSETTING(   0x10, "3000")
	PORT_DIPSETTING(   0x00, "5000")
	PORT_DIPNAME(0x60, 0x60, "Noten penalty after ryukyoku")
	PORT_DIPSETTING(   0x60, "1000")
	PORT_DIPSETTING(   0x20, "2000")
	PORT_DIPSETTING(   0x40, "3000")
	PORT_DIPSETTING(   0x00, "5000")
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Flip_Screen ))
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))

	PORT_MODIFY("dsw2")
	PORT_DIPNAME(0x03, 0x02, "Number of ending chance")
	PORT_DIPSETTING(   0x03, "0")
	PORT_DIPSETTING(   0x01, "3")
	PORT_DIPSETTING(   0x02, "5")
	PORT_DIPSETTING(   0x00, "10")
	PORT_DIPNAME(0x04, 0x04, "Ending chance requires fee")
	PORT_DIPSETTING(   0x04, DEF_STR( No ))
	PORT_DIPSETTING(   0x00, DEF_STR( Yes ))
	PORT_DIPNAME(0x18, 0x18, DEF_STR( Coinage ))
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x18, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x10, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x00, DEF_STR( Free_Play ))
	PORT_DIPNAME(0x60, 0x60, "Opponent Initial Score")
	PORT_DIPSETTING(   0x60, "3000 - 8000")
	PORT_DIPSETTING(   0x20, "5000 - 10000")
	PORT_DIPSETTING(   0x40, "8000")
	PORT_DIPSETTING(   0x00, "10000")
	PORT_DIPNAME(0x80, 0x00, "Gal select / Continue")
	PORT_DIPSETTING(   0x80, "Yes / No")
	PORT_DIPSETTING(   0x00, "No / Yes")
INPUT_PORTS_END

static INPUT_PORTS_START( ccasino )
	PORT_INCLUDE(ojankohs)

	PORT_MODIFY("system")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SERVICE1) PORT_NAME("Statistics")

	PORT_MODIFY("dsw1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")

	PORT_MODIFY("dsw2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPNAME(0x18, 0x18, "Girl Select")        PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(0x18, "A/B/C")
	PORT_DIPSETTING(0x10, "A")
	PORT_DIPSETTING(0x08, "B")
	PORT_DIPSETTING(0x00, "C")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")

	PORT_START("dsw3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW3:2")
	PORT_DIPNAME(0x04, 0x04, "Score Display" )     PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(   0x04, "Normal")
	PORT_DIPSETTING(   0x00, "* 1000")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW3:8")

	PORT_START("dsw4")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Coin_B ))    PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(   0x01, "1 Coin/10 Credits" )
	PORT_DIPSETTING(   0x00, "1 Coin/20 Credits" )
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW4:2")
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR( Coin_A ))    PORT_DIPLOCATION("DSW4:3,4")
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_1C ))
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW4:8")
INPUT_PORTS_END

static INPUT_PORTS_START( ojankoc )
	PORT_START("coin")
	PORT_BIT(0x3f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p1_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_E)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_I)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_M)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p1_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_B)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_F)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_J)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_N)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p1_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_C)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_G)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_K)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_RON)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p1_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_D)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_H)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_PON)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p2_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_A)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_E)     PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_I)     PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_M)     PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN)   PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p2_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_B)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_F)     PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_J)     PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_N)     PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p2_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_C)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_G)     PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_K)     PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI)   PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_MAHJONG_RON)   PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("p2_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MAHJONG_D)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_MAHJONG_H)     PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_MAHJONG_L)     PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_MAHJONG_PON)   PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("dsw1")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Flip_Screen ))  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x06, 0x06, "Player Initial Score")  PORT_DIPLOCATION("DSW1:2,3")
	PORT_DIPSETTING(   0x06, "1000")
	PORT_DIPSETTING(   0x04, "2000")
	PORT_DIPSETTING(   0x02, "3000")
	PORT_DIPSETTING(   0x00, "5000")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "DSW1:8")

	PORT_START("dsw2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")
INPUT_PORTS_END


//**************************************************************************
//  INPUT PORT HANDLING
//**************************************************************************

WRITE8_MEMBER( ojankohs_state::port_select_w )
{
	m_port_select = data;
}

READ8_MEMBER( ojankohs_state::keymatrix_p1_r )
{
	uint8_t data = 0xff;

	if (BIT(m_port_select, 0)) data &= m_inputs_p1[0]->read();
	if (BIT(m_port_select, 1)) data &= m_inputs_p1[1]->read();
	if (BIT(m_port_select, 2)) data &= m_inputs_p1[2]->read();
	if (BIT(m_port_select, 3)) data &= m_inputs_p1[3]->read();
	if (BIT(m_port_select, 4)) data &= m_inputs_p1_extra->read();

	data &= m_coin->read();

	return data;
}

READ8_MEMBER( ojankohs_state::keymatrix_p2_r )
{
	uint8_t data = 0xff;

	if (BIT(m_port_select, 0)) data &= m_inputs_p2[0]->read();
	if (BIT(m_port_select, 1)) data &= m_inputs_p2[1]->read();
	if (BIT(m_port_select, 2)) data &= m_inputs_p2[2]->read();
	if (BIT(m_port_select, 3)) data &= m_inputs_p2[3]->read();
	if (BIT(m_port_select, 4)) data &= m_inputs_p2_extra->read();

	data &= m_coin->read();

	return data;
}

READ8_MEMBER( ojankohs_state::ojankoc_keymatrix_p1_r )
{
	uint8_t data = 0x00;

	if (BIT(m_port_select, 0) == 0) data |= m_inputs_p1[0]->read();
	if (BIT(m_port_select, 1) == 0) data |= m_inputs_p1[1]->read();
	if (BIT(m_port_select, 2) == 0) data |= m_inputs_p1[2]->read();
	if (BIT(m_port_select, 3) == 0) data |= m_inputs_p1[3]->read();

	data |= m_coin->read();

	return data;
}

READ8_MEMBER( ojankohs_state::ojankoc_keymatrix_p2_r )
{
	uint8_t data = 0x00;

	if (BIT(m_port_select, 0) == 0) data |= m_inputs_p2[0]->read();
	if (BIT(m_port_select, 1) == 0) data |= m_inputs_p2[1]->read();
	if (BIT(m_port_select, 2) == 0) data |= m_inputs_p2[2]->read();
	if (BIT(m_port_select, 3) == 0) data |= m_inputs_p2[3]->read();

	data |= m_coin->read();

	return data;
}

READ8_MEMBER( ojankohs_state::ojankohs_dipsw1_r )
{
	uint8_t data = m_dsw1->read();
	return bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
}

READ8_MEMBER( ojankohs_state::ojankohs_dipsw2_r )
{
	uint8_t data = m_dsw2->read();
	return bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
}

READ8_MEMBER( ojankohs_state::ccasino_dipsw3_r )
{
	return m_dsw3->read() ^ 0xff;
}

READ8_MEMBER( ojankohs_state::ccasino_dipsw4_r )
{
	return m_dsw4->read() ^ 0xff;
}

WRITE8_MEMBER( ojankohs_state::ojankoy_coinctr_w )
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
}

WRITE8_MEMBER( ojankohs_state::ccasino_coinctr_w )
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1));
}


static const gfx_layout ojankohs_bglayout =
{
	8, 4,
	RGN_FRAC(1, 1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32 },
	16*8
};

static GFXDECODE_START( gfx_ojankohs )
	GFXDECODE_ENTRY( "gfx1", 0, ojankohs_bglayout,   0, 64 )
GFXDECODE_END

MACHINE_START_MEMBER(ojankohs_state,common)
{
	save_item(NAME(m_gfxreg));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_flipscreen_old));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_screen_refresh));
	save_item(NAME(m_port_select));
	save_item(NAME(m_adpcm_reset));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_vclk_left));
}

MACHINE_START_MEMBER(ojankohs_state,ojankohs)
{
	uint8_t *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 0x40, &ROM[0x10000], 0x4000);

	MACHINE_START_CALL_MEMBER(common);
}

MACHINE_START_MEMBER(ojankohs_state,ojankoy)
{
	uint8_t *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 0x20, &ROM[0x10000], 0x4000);

	MACHINE_START_CALL_MEMBER(common);
}

MACHINE_START_MEMBER(ojankohs_state,ojankoc)
{
	uint8_t *ROM = memregion("user1")->base();

	membank("bank1")->configure_entries(0, 0x10, &ROM[0x0000], 0x8000);

	MACHINE_START_CALL_MEMBER(common);
}

void ojankohs_state::machine_reset()
{
	m_port_select = 0;

	m_adpcm_reset = 0;
	m_adpcm_data = 0;
	m_vclk_left = 0;

	m_gfxreg = 0;
	m_flipscreen = 0;
	m_flipscreen_old = 0;
	m_scrollx = 0;
	m_scrolly = 0;
	m_screen_refresh = 0;
}

void ojankohs_state::ojankohs(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12000000/2);     /* 6.00 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &ojankohs_state::ojankohs_map);
	m_maincpu->set_addrmap(AS_IO, &ojankohs_state::ojankohs_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(ojankohs_state::irq0_line_hold));

	MCFG_MACHINE_START_OVERRIDE(ojankohs_state,ojankohs)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 512);
	m_screen->set_visarea(0, 288-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(ojankohs_state::screen_update_ojankohs));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ojankohs);
	PALETTE(config, m_palette).set_entries(1024);

	VSYSTEM_GGA(config, "gga", XTAL(13'333'000)/2); // divider not verified

	MCFG_VIDEO_START_OVERRIDE(ojankohs_state,ojankohs)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2149_device &aysnd(YM2149(config, "aysnd", 12000000/6));
	aysnd.port_a_read_callback().set(FUNC(ojankohs_state::ojankohs_dipsw1_r));
	aysnd.port_b_read_callback().set(FUNC(ojankohs_state::ojankohs_dipsw2_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.15);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(ojankohs_state::ojankohs_adpcm_int));     /* IRQ handler */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 KHz */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void ojankohs_state::ojankoy(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12000000/2);     /* 6.00 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &ojankohs_state::ojankoy_map);
	m_maincpu->set_addrmap(AS_IO, &ojankohs_state::ojankoy_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(ojankohs_state::irq0_line_hold));

	MCFG_MACHINE_START_OVERRIDE(ojankohs_state,ojankoy)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 512);
	m_screen->set_visarea(0, 288-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(ojankohs_state::screen_update_ojankohs));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ojankohs);
	PALETTE(config, m_palette, FUNC(ojankohs_state::ojankoy_palette), 1024);

	MCFG_VIDEO_START_OVERRIDE(ojankohs_state,ojankoy)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 12000000/8));
	aysnd.port_a_read_callback().set_ioport("dsw1");
	aysnd.port_b_read_callback().set_ioport("dsw2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.15);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(ojankohs_state::ojankohs_adpcm_int));     /* IRQ handler */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 KHz */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void ojankohs_state::ccasino(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12000000/2);     /* 6.00 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &ojankohs_state::ojankoy_map);
	m_maincpu->set_addrmap(AS_IO, &ojankohs_state::ccasino_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(ojankohs_state::irq0_line_hold));

	MCFG_MACHINE_START_OVERRIDE(ojankohs_state,ojankohs)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 512);
	m_screen->set_visarea(0, 288-1, 0, 224-1);
	m_screen->set_screen_update(FUNC(ojankohs_state::screen_update_ojankohs));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ojankohs);
	PALETTE(config, m_palette).set_entries(1024);

	VSYSTEM_GGA(config, "gga", XTAL(13'333'000)/2); // divider not verified

	MCFG_VIDEO_START_OVERRIDE(ojankohs_state,ccasino)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 12000000/8));
	aysnd.port_a_read_callback().set_ioport("dsw1");
	aysnd.port_b_read_callback().set_ioport("dsw2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.15);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_legacy_callback().set(FUNC(ojankohs_state::ojankohs_adpcm_int));     /* IRQ handler */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 KHz */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void ojankohs_state::ojankoc(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8000000/2);  /* 4.00 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &ojankohs_state::ojankoc_map);
	m_maincpu->set_addrmap(AS_IO, &ojankohs_state::ojankoc_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(ojankohs_state::irq0_line_hold));

	MCFG_MACHINE_START_OVERRIDE(ojankohs_state,ojankoc)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0, 256-1, 8, 248-1);
	m_screen->set_screen_update(FUNC(ojankohs_state::screen_update_ojankoc));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(16);

	MCFG_VIDEO_START_OVERRIDE(ojankohs_state,ojankoc)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 8000000/4));
	aysnd.port_a_read_callback().set_ioport("dsw1");
	aysnd.port_b_read_callback().set_ioport("dsw2");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.15);

	MSM5205(config, m_msm, 8000000/22);
	m_msm->vck_legacy_callback().set(FUNC(ojankohs_state::ojankohs_adpcm_int));     /* IRQ handler */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 KHz */
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( ojankohs )
	ROM_REGION( 0x98000, "maincpu", 0 )
	ROM_LOAD( "3.3c", 0x00000, 0x08000, CRC(f652db23) SHA1(7fcb4227804301f0404af4b007eb4accb0787c98) )
	ROM_LOAD( "5b",   0x10000, 0x80000, CRC(bd4fd0b6) SHA1(79e0937fdd34ec03b4b0a503efc1fa7c8f29e7cf) )
	ROM_LOAD( "6.6c", 0x90000, 0x08000, CRC(30772679) SHA1(8bc415da465faa70ec468a23b3528493849e83ee) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "13b",  0x00000, 0x80000, CRC(bda30bfa) SHA1(c412e573c40816735f7e2d0600dd0d78ebce91dc) )
ROM_END

ROM_START( ojankoy )
	ROM_REGION( 0x70000, "maincpu", 0 )
	ROM_LOAD( "p-ic17.bin", 0x00000, 0x08000, CRC(9f149c30) SHA1(e3a8407844c0bb2d2fda83b01a187c87b3b7767a) )
	ROM_LOAD( "ic30.bin",   0x10000, 0x20000, CRC(37be3f7c) SHA1(9ef19ef1e118d75ae719623b90188d68e6faa8f2) )
	ROM_LOAD( "ic29.bin",   0x30000, 0x20000, CRC(dab7c4d8) SHA1(812f56a15545e98eb67ac46ca1c006201d432b5d) )
	ROM_LOAD( "a-ic34.bin", 0x50000, 0x08000, CRC(93c20ea3) SHA1(f9b74813132fd9cef7803568daad5ea8e8e02a04) )
	ROM_LOAD( "b-ic33.bin", 0x58000, 0x08000, CRC(ef86d711) SHA1(922f4c29e8b5f7cf034e1ed623793aec57e799b6) )
	ROM_LOAD( "c-ic32.bin", 0x60000, 0x08000, CRC(d20de9b0) SHA1(bfec453a5e16bb3e1ffa454d6dad44e113a54968) )
	ROM_LOAD( "d-ic31.bin", 0x68000, 0x08000, CRC(b78e6913) SHA1(a0ebe0b29025beabe5609a5d1adecfd2565da623) )

	ROM_REGION( 0x70000, "gfx1", 0 )
	ROM_LOAD( "ic55.bin",   0x00000, 0x20000, CRC(586fb385) SHA1(cdf18f52ba8d25c740fc85a68505f102fe6ba208) )
	ROM_LOAD( "0-ic53.bin", 0x40000, 0x08000, CRC(db38c288) SHA1(8b98091eae9c22ade123a6f58c108f8e653d99c8) )
	ROM_LOAD( "1-ic52.bin", 0x48000, 0x08000, CRC(a8b4a10b) SHA1(fa44c52efd42a99e2d34c4785a09947523a8385a) )
	ROM_LOAD( "2-ic51.bin", 0x50000, 0x08000, CRC(5e2bb752) SHA1(39054cbb8f9cd99f815e2bce83bb82ec4a93b550) )
	ROM_LOAD( "3-ic50.bin", 0x58000, 0x08000, CRC(10c73a44) SHA1(e4ecfd0e1067eaec9e8f78f1cedac78599814556) )
	ROM_LOAD( "4-ic49.bin", 0x60000, 0x08000, CRC(31807d24) SHA1(9a2458386c1e970a47dd7bad85bbc2e28113759a) )
	ROM_LOAD( "5-ic48.bin", 0x68000, 0x08000, CRC(e116721d) SHA1(85e5b70fcdfc6ca92ce5aee8a17f1476b4f077d5) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "0-ic65.bin", 0x0000, 0x0400, CRC(28fde5ef) SHA1(81c645b5601ff33c6a5091e7debe99a8d6b6bd70) )
	ROM_LOAD( "1-ic64.bin", 0x0400, 0x0400, CRC(36c305c5) SHA1(43be6346e421f03a55bddb58a1570905321cf914) )
ROM_END

ROM_START( ojanko2 )
	ROM_REGION( 0x70000, "maincpu", 0 )
	ROM_LOAD( "p-ic17.bin", 0x00000, 0x08000, CRC(4b33bd54) SHA1(be235492cf3824ea740f401201ad821bb71c6d89) )
	ROM_LOAD( "ic30.bin",   0x10000, 0x20000, CRC(37be3f7c) SHA1(9ef19ef1e118d75ae719623b90188d68e6faa8f2) )
	ROM_LOAD( "ic29.bin",   0x30000, 0x20000, CRC(dab7c4d8) SHA1(812f56a15545e98eb67ac46ca1c006201d432b5d) )
	ROM_LOAD( "a-ic34.bin", 0x50000, 0x08000, CRC(93c20ea3) SHA1(f9b74813132fd9cef7803568daad5ea8e8e02a04) )
	ROM_LOAD( "b-ic33.bin", 0x58000, 0x08000, CRC(ef86d711) SHA1(922f4c29e8b5f7cf034e1ed623793aec57e799b6) )
	ROM_LOAD( "c-ic32.bin", 0x60000, 0x08000, CRC(5453b9de) SHA1(d9758c56cd65d65d0711368054fc0dfbb4b213ae) )
	ROM_LOAD( "d-ic31.bin", 0x68000, 0x08000, CRC(44cd5348) SHA1(a73a676fbca4678aef8066ad72ea22c6c4ca4b32) )

	ROM_REGION( 0x70000, "gfx1", 0 )
	ROM_LOAD( "ic55.bin",   0x00000, 0x20000, CRC(b058fb3d) SHA1(32b04405f218c1f9ca58f01dbadda3536df3d0b5) )
	ROM_LOAD( "0-ic53.bin", 0x40000, 0x08000, CRC(db38c288) SHA1(8b98091eae9c22ade123a6f58c108f8e653d99c8) )
	ROM_LOAD( "1-ic52.bin", 0x48000, 0x08000, CRC(49f2ca73) SHA1(387613fd886f3a4a569146aaec59ad15f13a8ea5) )
	ROM_LOAD( "2-ic51.bin", 0x50000, 0x08000, CRC(199a9bfb) SHA1(fa39aa5d97cf5b54327388d8f1668f24f2f420e4) )
	ROM_LOAD( "3-ic50.bin", 0x58000, 0x08000, CRC(f175510e) SHA1(9925d23b8cbd8bcadff1b37027899b63439ee734) )
	ROM_LOAD( "4-ic49.bin", 0x60000, 0x08000, CRC(3a6a9685) SHA1(756ed845f0b2f53b344a660961bd7e15df2a50f1) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "0-ic65.bin", 0x0000, 0x0400, CRC(86e19b01) SHA1(1facd72183d127aec1c5ad8f17f3450512698d94) )
	ROM_LOAD( "1-ic64.bin", 0x0400, 0x0400, CRC(e2f7093d) SHA1(428903e4fc9f05cf8dab01a5d4145a5b44faa311) )
ROM_END

ROM_START( ccasino )
	ROM_REGION( 0x68000, "maincpu", 0 )
	ROM_LOAD( "p5.bin", 0x00000, 0x08000, CRC(d6cf3387) SHA1(507a40a0ace0742a8fd205c641d27d22d80da948) )
	ROM_LOAD( "l5.bin", 0x10000, 0x20000, CRC(49c9ecfb) SHA1(96005904cef9b9e4434034c9d68978ff9c431457) )
	ROM_LOAD( "f5.bin", 0x50000, 0x08000, CRC(fa71c91c) SHA1(f693f6bb0a9433fbf3f272e43472f6a728ae35ef) )
	ROM_LOAD( "g5.bin", 0x58000, 0x08000, CRC(8cfd60aa) SHA1(203789c58a9cbfbf37ad2a3dfcd86eefe406b2c7) )
	ROM_LOAD( "h5.bin", 0x60000, 0x08000, CRC(d20dfcf9) SHA1(83ca36f2e02bbada5b03734b5d92c5c860292db2) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "r1.bin", 0x00000, 0x20000, CRC(407f77ca) SHA1(a65e5403fa84185d67d994acee6f32051991d546) )
	ROM_LOAD( "s1.bin", 0x20000, 0x20000, CRC(8572d156) SHA1(22f73bfb1419c3d467b4cd4ffaa6f1598f4ee4fa) )
	ROM_LOAD( "e1.bin", 0x40000, 0x08000, CRC(d78c3428) SHA1(b033a7aa3029b7a9ff836c5c737c07aaad5d7456) )
	ROM_LOAD( "f1.bin", 0x48000, 0x08000, CRC(799cc0e7) SHA1(51ca991a76945235375f1c7c4db2abfa1d7ebd15) )
	ROM_LOAD( "g1.bin", 0x50000, 0x08000, CRC(3ac8ae04) SHA1(7ac3095bb2ee6e86970464746fe4644eabc769ec) )
	ROM_LOAD( "h1.bin", 0x58000, 0x08000, CRC(f0af2d38) SHA1(14f29404a10633f5c4b574fc1f34139f9fb8a8bf) )
ROM_END

ROM_START( ojankoc )
	ROM_REGION( 0x10000, "maincpu", 0 )   /* CPU */
	ROM_LOAD( "c11.1p", 0x0000, 0x8000, CRC(cb3e900c) SHA1(95f0354f147e339a97368b5cc67200151cdfa0e9) )

	ROM_REGION( 0x80000, "user1", 0 )  /* BANK */
	ROM_LOAD( "1.1a", 0x00000, 0x8000, CRC(d40b17eb) SHA1(1e8c16e1562c112ca5150b3187a2d4aa22c1adf0) )
	ROM_LOAD( "2.1b", 0x08000, 0x8000, CRC(d181172a) SHA1(65d6710464a1f505df705c553558bbf22704359d) )
	ROM_LOAD( "3.1c", 0x10000, 0x8000, CRC(2e86d5bc) SHA1(0226eb81b31e43325f24b40ab51bce1729bf678c) )
	ROM_LOAD( "4.1e", 0x18000, 0x8000, CRC(00a780cb) SHA1(f0b4f6f0c58e9d069e0f6794243925679f220f35) )
	ROM_LOAD( "5.1f", 0x20000, 0x8000, CRC(f9885076) SHA1(ebf4c0769eab6545fd227eb9f4036af2472bcac3) )
	ROM_LOAD( "6.1h", 0x28000, 0x8000, CRC(42575d0c) SHA1(1f9c187b0c05179798cbdb28eb212202ffdc9fde) )
	ROM_LOAD( "7.1k", 0x30000, 0x8000, CRC(4d8d8928) SHA1(a5ccf4a1d84ef3a4966db01d66371de83e270701) )
	ROM_LOAD( "8.1l", 0x38000, 0x8000, CRC(534573b7) SHA1(ec53cad7d652c88508edd29c2412834920fe8ef6) )
	ROM_LOAD( "9.1m", 0x48000, 0x8000, CRC(2bf88eda) SHA1(55de96d057a0f35d9e74455444751f217aa4741e) )
	ROM_LOAD( "0.1n", 0x50000, 0x8000, CRC(5665016e) SHA1(0f7f0a8e55e93bcb3060c91d9704905a6e827250) )
ROM_END

ROM_START( ojankoca )
	ROM_REGION( 0x10000, "maincpu", 0 )   /* CPU */
	ROM_LOAD( "11.1p", 0x0000, 0x8000, CRC(0c552e32) SHA1(2a8714796b2c95a042d783aae79c135ba03d1958) )

	ROM_REGION( 0x80000, "user1", 0 )  /* BANK */
	ROM_LOAD( "1.1a", 0x00000, 0x8000, CRC(d40b17eb) SHA1(1e8c16e1562c112ca5150b3187a2d4aa22c1adf0) )
	ROM_LOAD( "2.1b", 0x08000, 0x8000, CRC(d181172a) SHA1(65d6710464a1f505df705c553558bbf22704359d) )
	ROM_LOAD( "3.1c", 0x10000, 0x8000, CRC(2e86d5bc) SHA1(0226eb81b31e43325f24b40ab51bce1729bf678c) )
	ROM_LOAD( "4.1e", 0x18000, 0x8000, CRC(00a780cb) SHA1(f0b4f6f0c58e9d069e0f6794243925679f220f35) )
	ROM_LOAD( "5.1f", 0x20000, 0x8000, CRC(f9885076) SHA1(ebf4c0769eab6545fd227eb9f4036af2472bcac3) )
	ROM_LOAD( "6.1h", 0x28000, 0x8000, CRC(42575d0c) SHA1(1f9c187b0c05179798cbdb28eb212202ffdc9fde) )
	ROM_LOAD( "7.1k", 0x30000, 0x8000, CRC(4d8d8928) SHA1(a5ccf4a1d84ef3a4966db01d66371de83e270701) )
	ROM_LOAD( "8.1l", 0x38000, 0x8000, CRC(534573b7) SHA1(ec53cad7d652c88508edd29c2412834920fe8ef6) )
	ROM_LOAD( "9.1m", 0x48000, 0x8000, CRC(2bf88eda) SHA1(55de96d057a0f35d9e74455444751f217aa4741e) )
	ROM_LOAD( "0.1n", 0x50000, 0x8000, CRC(5665016e) SHA1(0f7f0a8e55e93bcb3060c91d9704905a6e827250) )
ROM_END


GAME( 1986, ojankoc,  0,       ojankoc,  ojankoc,  ojankohs_state, empty_init, ROT0, "V-System Co.", "Ojanko Club (Japan, Program Ver. 1.3)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ojankoca, ojankoc, ojankoc,  ojankoc,  ojankohs_state, empty_init, ROT0, "V-System Co.", "Ojanko Club (Japan, Program Ver. 1.2)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, ojankoy,  0,       ojankoy,  ojankoy,  ojankohs_state, empty_init, ROT0, "V-System Co.", "Ojanko Yakata (Japan)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1987, ojanko2,  0,       ojankoy,  ojankoy,  ojankohs_state, empty_init, ROT0, "V-System Co.", "Ojanko Yakata 2bankan (Japan)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, ccasino,  0,       ccasino,  ccasino,  ojankohs_state, empty_init, ROT0, "V-System Co.", "Chinese Casino [BET] (Japan)",          MACHINE_SUPPORTS_SAVE )
GAME( 1988, ojankohs, 0,       ojankohs, ojankohs, ojankohs_state, empty_init, ROT0, "V-System Co.", "Ojanko High School (Japan)",            MACHINE_SUPPORTS_SAVE )
