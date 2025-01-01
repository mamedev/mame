// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    nbmj8991 - Nichibutsu Mahjong games for years 1989-1991

    Driver by Takahiro Nogi 1999/12/02 -

******************************************************************************/
/******************************************************************************

Notes:

- If "Game sound" is set to "OFF" in mjlstory, attract sound is not played
  even if "Attract sound" is set to "ON".

- The program of galkaika, tokyogal, and tokimbsj runs on Interrupt mode 2
  on real machine, but they don't run correctly in MAME so I changed to
  interrupt mode 1.

- Sound CPU of qmhayaku is running on 4MHz in real machine. But if I set
  it to 4MHz in MAME, sounds are not  played so I lowered the clock a bit.

- av2mj's VCR playback is not implemented.

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  in Debug build.

- Screen flip is not perfect.

******************************************************************************/

#include "emu.h"
#include "nbmj8991.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "speaker.h"


void nbmj8991_state::soundbank_w(uint8_t data)
{
	if (!(data & 0x80)) m_soundlatch->clear_w();
	membank("bank1")->set_entry(data & 0x03);
}

void nbmj8991_state::machine_reset()
{
	if (m_audiocpu)
	{
		membank("bank1")->configure_entries(0, 4, memregion("audiocpu")->base() + 0x8000, 0x8000);
		membank("bank1")->set_entry(0);
	}
}

void nbmj8991_state::init_finalbny()
{
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0xf800; i < 0x10000; i++) ROM[i] = 0x00;
}

void nbmj8991_state::init_galkaika()
{
#if 1
	uint8_t *ROM = memregion("maincpu")->base();

	// Patch to IM2 -> IM1
	ROM[0x0002] = 0x56;
#endif
}

void nbmj8991_state::init_tokyogal()
{
#if 1
	uint8_t *ROM = memregion("maincpu")->base();

	// Patch to IM2 -> IM1
	ROM[0x0002] = 0x56;
#endif
}

void nbmj8991_state::init_tokimbsj()
{
#if 1
	uint8_t *ROM = memregion("maincpu")->base();

	// Patch to IM2 -> IM1
	ROM[0x0002] = 0x56;
#endif
}


void nbmj8991_state::pstadium_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf00f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf200, 0xf3ff).ram().w(FUNC(nbmj8991_state::palette_type3_w)).share("paletteram");
	map(0xf800, 0xffff).ram().share("nvram");   // finalbny
}

void nbmj8991_state::triplew1_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf1ff).ram().w(FUNC(nbmj8991_state::palette_type3_w)).share("paletteram");
	map(0xf200, 0xf20f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf800, 0xffff).ram().share("nvram");   // mjgottub
}

void nbmj8991_state::triplew2_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf1ff).ram().w(FUNC(nbmj8991_state::palette_type3_w)).share("paletteram");
	map(0xf400, 0xf40f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf800, 0xffff).ram();
}

void nbmj8991_state::mjlstory_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf200, 0xf3ff).ram().w(FUNC(nbmj8991_state::palette_type3_w)).share("paletteram");
	map(0xf700, 0xf70f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf800, 0xffff).ram();
}

void nbmj8991_state::galkoku_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf00f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf400, 0xf5ff).ram().w(FUNC(nbmj8991_state::palette_type1_w)).share("paletteram");
	map(0xf800, 0xffff).ram().share("nvram");   // hyouban
}

void nbmj8991_state::galkaika_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf00f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf400, 0xf5ff).ram().w(FUNC(nbmj8991_state::palette_type2_w)).share("paletteram");
	map(0xf800, 0xffff).ram().share("nvram");   // tokimbsj
}

void nbmj8991_state::tokyogal_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf1ff).ram().w(FUNC(nbmj8991_state::palette_type2_w)).share("paletteram");
	map(0xf400, 0xf40f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf800, 0xffff).ram();
}

void nbmj8991_state::av2mj1bb_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf1ff).ram().w(FUNC(nbmj8991_state::palette_type3_w)).share("paletteram");
	map(0xf500, 0xf50f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf800, 0xffff).ram();
}

void nbmj8991_state::av2mj2rg_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xf00f).rw(FUNC(nbmj8991_state::clut_r), FUNC(nbmj8991_state::clut_w));
	map(0xf200, 0xf3ff).ram().w(FUNC(nbmj8991_state::palette_type3_w)).share("paletteram");
	map(0xf800, 0xffff).ram();
}

void nbmj8991_state::galkoku_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r)).w(FUNC(nbmj8991_state::blitter_w));
	map(0x80, 0x81).w("fmsnd", FUNC(ym3812_device::write));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xc0, 0xc0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport3_r), FUNC(nb1413m3_device::nmi_clock_w));
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
//  map(0xe0, 0xe0).nopw();
	map(0xf0, 0xf0).rw(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r), FUNC(nb1413m3_device::outcoin_w));
	map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}

void nbmj8991_state::hyouban_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).r(m_nb1413m3, FUNC(nb1413m3_device::sndrom_r)).w(FUNC(nbmj8991_state::blitter_w));
	map(0x81, 0x81).r("fmsnd", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("fmsnd", FUNC(ay8910_device::data_address_w));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::sndrombank1_w));
	map(0xc0, 0xc0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport3_r), FUNC(nb1413m3_device::nmi_clock_w));
	map(0xd0, 0xd0).w("dac", FUNC(dac_byte_interface::data_w));
//  map(0xe0, 0xe0).nopw();
	map(0xf0, 0xf0).rw(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r), FUNC(nb1413m3_device::outcoin_w));
	map(0xf1, 0xf1).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}

void nbmj8991_state::pstadium_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).w(FUNC(nbmj8991_state::blitter_w));
	map(0x80, 0x80).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).r(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r)); //.nopw();
	map(0xc0, 0xc0).r(m_nb1413m3, FUNC(nb1413m3_device::inputport3_r));
//  map(0xd0, 0xd0).nopw();
	map(0xf0, 0xf0).rw(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r), FUNC(nb1413m3_device::outcoin_w));
	map(0xf8, 0xf8).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}

void nbmj8991_state::av2mj1bb_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x7f).w(FUNC(nbmj8991_state::blitter_w));
	map(0x80, 0x80).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x90, 0x90).r(m_nb1413m3, FUNC(nb1413m3_device::inputport0_r));
	map(0xa0, 0xa0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport1_r), FUNC(nb1413m3_device::inputportsel_w));
	map(0xb0, 0xb0).rw(m_nb1413m3, FUNC(nb1413m3_device::inputport2_r), FUNC(nb1413m3_device::vcrctrl_w));
	map(0xc0, 0xc0).r(m_nb1413m3, FUNC(nb1413m3_device::inputport3_r));
//  map(0xd0, 0xd0).nopw();
	map(0xf0, 0xf0).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw1_r));
//  map(0xf0, 0xf0).nopw();
	map(0xf8, 0xf8).r(m_nb1413m3, FUNC(nb1413m3_device::dipsw2_r));
}


void nbmj8991_state::nbmj8991_sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr("bank1");
}

void nbmj8991_state::nbmj8991_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w("dac1", FUNC(dac_byte_interface::data_w));
	map(0x02, 0x02).w("dac2", FUNC(dac_byte_interface::data_w));
	map(0x04, 0x04).w(FUNC(nbmj8991_state::soundbank_w));
	map(0x06, 0x06).nopw();
	map(0x80, 0x81).w("fmsnd", FUNC(ym3812_device::write));
}

static INPUT_PORTS_START( pstadium )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Sounds" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( triplew1 )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Sounds" )       PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )    PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )      PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( ntopstar )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Sounds" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mjlstory )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Sounds" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( vanilla )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2")
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Sounds" )               PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )    PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )          PORT_DIPLOCATION("DSWA:8") // Marked as unused in manual
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "DSWB:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "DSWB:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "DSWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "DSWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "DSWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "DSWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSWB:8" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( finalbny )

	// I don't have manual for this game. (finalbny)
	// Lady Maker manual matches this, identical PCB
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out" )                  PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "90% (Easy)" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x00, "55% (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x00, "Last Chance" )               PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Last chance needs 1credit" ) PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Graphic ROM Test" )          PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Bet1 Only" )                 PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Bet Min" )                   PORT_DIPLOCATION("DSWB:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x60, 0x00, "Bet Max" )                   PORT_DIPLOCATION("DSWB:6,7")
	PORT_DIPSETTING(    0x60, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x80, 0x00, "Score Pool" )                PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( qmhayaku )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("DSWA:1,2")
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )          PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Sounds" )               PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" )    PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Graphic ROM Test" )          PORT_DIPLOCATION("DSWA:8")  // manual states this is unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* manual says this does not exist but it is physically present */
	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )           PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( galkoku )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )    PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )       PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	// not even physically present on PCBs
	PORT_START("DSWB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( hyouban )
	// see MT05554 for dip sheet
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, "Game Out" )               PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x07, "95% (Easy)" )
	PORT_DIPSETTING(    0x06, "90%" )
	PORT_DIPSETTING(    0x05, "85%" )
	PORT_DIPSETTING(    0x04, "80%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "60% (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, "Last Chance" )            PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "W Bet" )                  PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Girl Presentation" )      PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Character Display Test" ) PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )   PORT_DIPLOCATION("DSWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x06, 0x06, "Bet Min" )            PORT_DIPLOCATION("DSWB:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, "Bet Max" )            PORT_DIPLOCATION("DSWB:4,5")
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "12" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, "Bet1 Only" )          PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Score Pool" )         PORT_DIPLOCATION("DSWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Odds Number" )        PORT_DIPLOCATION("DSWB:8")
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x80, "B" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( galkaika )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( tokyogal )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( tokimbsj )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Character Display Test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mcontest )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Character Display Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( uchuuai )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Game Sounds" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Character Display Test" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( mjgottub )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DIPSW 1-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIPSW 1-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Character Display Test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 2-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )       // SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( av2mj1bb )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Video Playback Time" )
	PORT_DIPSETTING(    0xc0, "Type-A" )
	PORT_DIPSETTING(    0x80, "Type-B" )
	PORT_DIPSETTING(    0x40, "Type-C" )
	PORT_DIPSETTING(    0x00, "Type-D" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, "Attract mode" )
	PORT_DIPSETTING(    0x03, "No attract mode" )
	PORT_DIPSETTING(    0x02, "Once per 10min." )
	PORT_DIPSETTING(    0x01, "Once per 5min." )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Graphic ROM Test" )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

static INPUT_PORTS_START( av2mj2rg )

	// I don't have manual for this game.

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Video Playback Time" )
	PORT_DIPSETTING(    0xc0, "Type-A" )
	PORT_DIPSETTING(    0x80, "Type-B" )
	PORT_DIPSETTING(    0x40, "Type-C" )
	PORT_DIPSETTING(    0x00, "Type-D" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, "Attract mode" )
	PORT_DIPSETTING(    0x03, "No attract mode" )
	PORT_DIPSETTING(    0x02, "Once per 10min." )
	PORT_DIPSETTING(    0x01, "Once per 5min." )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Graphic ROM Test" )
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("nb1413m3", FUNC(nb1413m3_device::busyflag_r))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )         //
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )       // ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )                 // TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )          // COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Credit Clear") PORT_CODE(KEYCODE_4) // CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )          // COIN2

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END


void nbmj8991_state::nbmjdrv1(machine_config &config) // galkoku
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 25000000/5);        /* 5.00 MHz ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::galkoku_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8991_state::galkoku_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(nbmj8991_state::irq0_line_hold));

	NB1413M3(config, m_nb1413m3, 0);
	m_nb1413m3->set_blitter_rom_tag("gfx1");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(1024, 512);
	m_screen->set_visarea(0, 640-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(nbmj8991_state::screen_update_type1));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	YM3812(config, "fmsnd", 25000000/10).add_route(ALL_OUTPUTS, "speaker", 0.7);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}


void nbmj8991_state::nbmjdrv2(machine_config &config) // pstadium
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6000000/2); /* 3.00 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::pstadium_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8991_state::pstadium_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(nbmj8991_state::irq0_line_hold));

	NB1413M3(config, m_nb1413m3, 0);
	m_nb1413m3->set_blitter_rom_tag("gfx1");

	Z80(config, m_audiocpu, 4000000);                  /* 4.00 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::nbmj8991_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &nbmj8991_state::nbmj8991_sound_io_map);
	m_audiocpu->set_periodic_int(FUNC(nbmj8991_state::irq0_line_hold), attotime::from_hz(128*60)); // ?

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(1024, 512);
	m_screen->set_visarea(0, 640-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(nbmj8991_state::screen_update_type2));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(256);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM3812(config, "fmsnd", 25000000/6.25).add_route(ALL_OUTPUTS, "speaker", 0.7);

	DAC_8BIT_R2R(config, "dac1", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
	DAC_8BIT_R2R(config, "dac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
}


void nbmj8991_state::nbmjdrv3(machine_config &config)
{
	nbmjdrv1(config);

	/* sound hardware */
	ay8910_device &fmsnd(AY8910(config.replace(), "fmsnd", 1250000));
	fmsnd.port_a_read_callback().set_ioport("DSWA");
	fmsnd.port_b_read_callback().set_ioport("DSWB");
	fmsnd.add_route(ALL_OUTPUTS, "speaker", 0.35);
}


// ---------------------------------------------------------------------

void nbmj8991_state::galkoku(machine_config &config)
{
	nbmjdrv1(config);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_GALKOKU);
}


void nbmj8991_state::galkaika(machine_config &config)
{
	nbmjdrv1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::galkaika_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_GALKAIKA);
}


void nbmj8991_state::tokyogal(machine_config &config)
{
	nbmjdrv1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::tokyogal_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_TOKYOGAL);
}


void nbmj8991_state::tokimbsj(machine_config &config)
{
	nbmjdrv1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::galkaika_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_TOKIMBSJ);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


void nbmj8991_state::mcontest(machine_config &config)
{
	nbmjdrv1(config);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_MCONTEST);
}


void nbmj8991_state::uchuuai(machine_config &config)
{
	nbmjdrv1(config);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_UCHUUAI);
}


void nbmj8991_state::hyouban(machine_config &config)
{
	nbmjdrv3(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &nbmj8991_state::hyouban_io_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_HYOUBAN);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


void nbmj8991_state::pstadium(machine_config &config)
{
	nbmjdrv2(config);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_PSTADIUM);
}


void nbmj8991_state::triplew1(machine_config &config)
{
	nbmjdrv2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::triplew1_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_TRIPLEW1);
}


void nbmj8991_state::triplew2(machine_config &config)
{
	nbmjdrv2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::triplew2_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_TRIPLEW2);
}


void nbmj8991_state::ntopstar(machine_config &config)
{
	nbmjdrv2(config);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_NTOPSTAR);
}


void nbmj8991_state::mjlstory(machine_config &config)
{
	nbmjdrv2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::mjlstory_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_MJLSTORY);
}


void nbmj8991_state::vanilla(machine_config &config)
{
	nbmjdrv2(config);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_VANILLA);
}


void nbmj8991_state::finalbny(machine_config &config)
{
	nbmjdrv2(config);

	/* basic machine hardware */
	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_FINALBNY);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


void nbmj8991_state::qmhayaku(machine_config &config)
{
	nbmjdrv2(config);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_QMHAYAKU);
}


void nbmj8991_state::mjgottub(machine_config &config)
{
	nbmjdrv2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::triplew1_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_MJGOTTUB);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


void nbmj8991_state::av2mj1bb(machine_config &config)
{
	nbmjdrv2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::av2mj1bb_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8991_state::av2mj1bb_io_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_AV2MJ1BB);
}


void nbmj8991_state::av2mj2rg(machine_config &config)
{
	nbmjdrv2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &nbmj8991_state::av2mj2rg_map);
	m_maincpu->set_addrmap(AS_IO, &nbmj8991_state::av2mj1bb_io_map);

	m_nb1413m3->set_type(nb1413m3_device::NB1413M3_AV2MJ2RG);
}




ROM_START( pstadium )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "psdm_01.bin", 0x00000,  0x10000, CRC(4af81589) SHA1(d3fc618ecd7763facb465fcd4ba2def1760a99ff) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "psdm_03.bin", 0x00000,  0x10000, CRC(ac17cef2) SHA1(9160e8d98e0708a19b9cfbdd7c6815d241d0a837) )
	ROM_LOAD( "psdm_02.bin", 0x10000,  0x10000, CRC(efefe881) SHA1(b1e6dbb3f006b101aea479d910633c1f8cbe50d5) )

	ROM_REGION( 0x110000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "psdm_04.bin", 0x000000, 0x10000, CRC(01957a76) SHA1(048de386dcbbbcaea8809a409857e2decd33d21f) )
	ROM_LOAD( "psdm_05.bin", 0x010000, 0x10000, CRC(f5dc1d20) SHA1(15a909be0fd5854beda6fd1fe2570049924be1d2) )
	ROM_LOAD( "psdm_06.bin", 0x020000, 0x10000, CRC(6fc89b50) SHA1(de3711df98038df120f954bb4d4a620017dcc8ee) )
	ROM_LOAD( "psdm_07.bin", 0x030000, 0x10000, CRC(aec64ff4) SHA1(6da69021e972eb8643feea77648d9eb92e8c7f85) )
	ROM_LOAD( "psdm_08.bin", 0x040000, 0x10000, CRC(ebeaf64a) SHA1(aa377a9ba5e350ad981ab41abe9942ed34851ff9) )
	ROM_LOAD( "psdm_09.bin", 0x050000, 0x10000, CRC(854b2914) SHA1(c0b5274c31658e45301bf2ac0bc33caf018d698a) )
	ROM_LOAD( "psdm_10.bin", 0x060000, 0x10000, CRC(eca5cd5a) SHA1(9be3d265405a1193a9e0cae1b31293e3edc5582f) )
	ROM_LOAD( "psdm_11.bin", 0x070000, 0x10000, CRC(a2de166d) SHA1(51949242ebeb06dd2c4cd4a049698d95e512be83) )
	ROM_LOAD( "psdm_12.bin", 0x080000, 0x10000, CRC(2c99ec4d) SHA1(efc8381a8511f876ff57f70fc12c7fd15d55aef9) )
	ROM_LOAD( "psdm_13.bin", 0x090000, 0x10000, CRC(77b99a6e) SHA1(4be1abd236727b222e5942f66a47b3c17daf1e46) )
	ROM_LOAD( "psdm_14.bin", 0x0a0000, 0x10000, CRC(a3cf907b) SHA1(96fa5af2d6dea2816c3725fd44c79ec92bfa3d8a) )
	ROM_LOAD( "psdm_15.bin", 0x0b0000, 0x10000, CRC(b0da8d18) SHA1(f6b2e8ad1c6077fceb1ff1362346489571089bc0) )
	ROM_LOAD( "psdm_16.bin", 0x0c0000, 0x10000, CRC(9a2fd9c5) SHA1(7017007b58b9aa1e41aada542c694b14c5c324b2) )
	ROM_LOAD( "psdm_17.bin", 0x0d0000, 0x10000, CRC(e462d507) SHA1(eb751ed78ab093a40913cc0c4f328bcd3f1ab20c) )
	ROM_LOAD( "psdm_18.bin", 0x0e0000, 0x10000, CRC(e9ce8e02) SHA1(cba98c32811059e035cb34cbcbf23d69d25b5720) )
	ROM_LOAD( "psdm_19.bin", 0x0f0000, 0x10000, CRC(f23496c6) SHA1(988bbc891c1ae3a7bf1f3c2164a3e4e84ed2732c) )
	ROM_LOAD( "psdm_20.bin", 0x100000, 0x10000, CRC(c410ce4b) SHA1(117d556ba0ed219225975d1b800e300d33343db0) )
ROM_END

ROM_START( triplew1 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "tpw1_01.bin", 0x00000,  0x10000, CRC(2542958a) SHA1(3d904990ac4d51f80aefe3f84252603de87e791d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "tpw1_03.bin", 0x00000,  0x10000, CRC(d86cc7d2) SHA1(430f1d2bad2dbab8f39829d3353fff52308542cb) )
	ROM_LOAD( "tpw1_02.bin", 0x10000,  0x10000, CRC(857656a7) SHA1(9e35e4970b4a80cf98c86cd6384022d62a5e543d) )

	ROM_REGION( 0x160000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "tpw1_04.bin", 0x000000, 0x20000, CRC(ca26ccb3) SHA1(36c60dbef81119be9b39bc584a79d466ec45502b) )
	ROM_LOAD( "tpw1_05.bin", 0x020000, 0x20000, CRC(26501af0) SHA1(7e7447085b8387cfa1111ae402f2b1ff48f8753e) )
	ROM_LOAD( "tpw1_06.bin", 0x040000, 0x10000, CRC(789bbacd) SHA1(90dbe6ac4231ce6f6885f19f38a312a4d35cb4da) )
	ROM_LOAD( "tpw1_07.bin", 0x050000, 0x10000, CRC(38aaad61) SHA1(fae23d5b9d5fe2cad0cb1cc3b6c5f144d9e4d10e) )
	ROM_LOAD( "tpw1_08.bin", 0x060000, 0x10000, CRC(9f4042b4) SHA1(0bfdebea60230c5889e57beb27086cf819f56614) )
	ROM_LOAD( "tpw1_09.bin", 0x070000, 0x10000, CRC(388a78b9) SHA1(dc994f98fff70483437f622c0a729de4fcc285d6) )
	ROM_LOAD( "tpw1_10.bin", 0x080000, 0x10000, CRC(7a19730d) SHA1(a3dbb2880b15ba5bd4dc1830ebfa8c3ad1da6c2d) )
	ROM_LOAD( "tpw1_11.bin", 0x090000, 0x10000, CRC(1239a0c6) SHA1(bb76b72fbca75b447751e08b74c519a725a0df45) )
	ROM_LOAD( "tpw1_12.bin", 0x0a0000, 0x10000, CRC(ca469c52) SHA1(602251d149324ca0aa5160d8b90a9b83a2bd6109) )
	ROM_LOAD( "tpw1_13.bin", 0x0b0000, 0x10000, CRC(0ca520c0) SHA1(7515e135ddcd0dc75d330a499e5182382d1b2e30) )
	ROM_LOAD( "tpw1_14.bin", 0x0c0000, 0x10000, CRC(3880db99) SHA1(cf3826fc0da0a5f9d84ac1d6b079f524c3587a0c) )
	ROM_LOAD( "tpw1_15.bin", 0x0d0000, 0x10000, CRC(996ea3e8) SHA1(7f7b938c7952c0474fef6af137705661cc35e290) )
	ROM_LOAD( "tpw1_16.bin", 0x0e0000, 0x10000, CRC(415ae47c) SHA1(68b8386a1730e1d919a4ac0e592188313f9431aa) )
	ROM_LOAD( "tpw1_17.bin", 0x0f0000, 0x10000, CRC(b5c88f0e) SHA1(efc13818faed5a249631f41ca09fd17241aa8b26) )
	ROM_LOAD( "tpw1_18.bin", 0x100000, 0x10000, CRC(def06191) SHA1(ff1ba2a53c307706b8e676e0dd52e798e500c10c) )
	ROM_LOAD( "tpw1_19.bin", 0x110000, 0x10000, CRC(b293561b) SHA1(fc32c18a8d242953f1fb98af7e453f97f50a908c) )
	ROM_LOAD( "tpw1_20.bin", 0x120000, 0x10000, CRC(81bfa331) SHA1(5fef84db195aa4a670bb3edb17afcc131e79c412) )
	ROM_LOAD( "tpw1_21.bin", 0x130000, 0x10000, CRC(2dbb68e5) SHA1(1cc327362d69908189621595ce018c5d1761ba13) )
	ROM_LOAD( "tpw1_22.bin", 0x140000, 0x10000, CRC(9633278c) SHA1(279d059f731dc81230cf0038f53318cfdade6fae) )
	ROM_LOAD( "tpw1_23.bin", 0x150000, 0x10000, CRC(11580513) SHA1(8c2e9e91ffdc1f323290f3d67bf10e1f3275e8bd) )
ROM_END

ROM_START( triplew2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "tpw2_01.bin", 0x00000,  0x10000, CRC(2637f19d) SHA1(0f6372eda7586d6531664036aaaefc135d9ca522) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "tpw2_03.bin", 0x00000,  0x10000, CRC(8e7922c3) SHA1(f9d54545de48642e1ed7504d9fbbecf5459e3294) )
	ROM_LOAD( "tpw2_02.bin", 0x10000,  0x10000, CRC(5339692d) SHA1(eebe4fefbefe854eeb1d2856c650a53e7afd70c6) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "tpw2_04.bin", 0x000000, 0x20000, CRC(d4af2c04) SHA1(a901842c6203db55ca5aea318252721ba55c4f4b) )
	ROM_LOAD( "tpw2_05.bin", 0x020000, 0x20000, CRC(fff198c8) SHA1(606052f2233efda42bee47c8882f83f7caa076f3) )
	ROM_LOAD( "tpw2_06.bin", 0x040000, 0x20000, CRC(4966b15b) SHA1(9a72b5f631e86ad111c01eec51b4ac6aef025cba) )
	ROM_LOAD( "tpw2_07.bin", 0x060000, 0x20000, CRC(de1b8788) SHA1(2bf42234123be07ee2f65ea2c5c0517e84a004a5) )
	ROM_LOAD( "tpw2_08.bin", 0x080000, 0x20000, CRC(fb1b1ebc) SHA1(7877099c45c56c77642a88a69d4e6c28e151cf89) )
	ROM_LOAD( "tpw2_09.bin", 0x0a0000, 0x10000, CRC(d40cacfd) SHA1(1aed145b0bc6862d3a47ba00fa4f8435f4544715) )
	ROM_LOAD( "tpw2_10.bin", 0x0b0000, 0x10000, CRC(8fa96a92) SHA1(4f86c4608bbb7043cdfcf3507bd5ffc715bc0933) )
	ROM_LOAD( "tpw2_11.bin", 0x0c0000, 0x10000, CRC(a6a44edd) SHA1(d433cf3d1db1dca500b67531570cd8fb3905c002) )
	ROM_LOAD( "tpw2_12.bin", 0x0d0000, 0x10000, CRC(d01a3a6a) SHA1(4e5af588fb791d9ecbd2d5f21eebebf4a9b0fe83) )
	ROM_LOAD( "tpw2_13.bin", 0x0e0000, 0x10000, CRC(6b4ebd1f) SHA1(dfd9030cbd3e043984adcb7e8beb1c742c7944dc) )
	ROM_LOAD( "tpw2_14.bin", 0x0f0000, 0x10000, CRC(383d2735) SHA1(697b5db02a8a817019962e29741d141a159b9ee0) )
	ROM_LOAD( "tpw2_15.bin", 0x100000, 0x10000, CRC(682110f5) SHA1(ddabca37f98a4313479d48215d5edcc9332f62f9) )
	ROM_LOAD( "tpw2_16.bin", 0x110000, 0x10000, CRC(466eea24) SHA1(f65571f33a00861ad7e9ce4aa88ec4761ed79864) )
	ROM_LOAD( "tpw2_17.bin", 0x120000, 0x10000, CRC(a422ece3) SHA1(d78b4600fcb7eba3e2902a7af2bbb87172221122) )
	ROM_LOAD( "tpw2_18.bin", 0x130000, 0x10000, CRC(f65b699d) SHA1(04bebf8f0bbc5173140bd509fbfbca03717f7169) )
	ROM_LOAD( "tpw2_19.bin", 0x140000, 0x10000, CRC(8356beac) SHA1(4cc4eb1587859730e540e3813bff7d329b78d96b) )
	ROM_LOAD( "tpw2_20.bin", 0x150000, 0x10000, CRC(240c408e) SHA1(18e64bc193315fb8b257dd999b898c4c329f0de8) )
	ROM_LOAD( "mj_1802.bin", 0x180000, 0x80000, CRC(e6213f10) SHA1(377399e9cd20fc2055b680eb28d024824161b2ff) )
ROM_END

ROM_START( ntopstar )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "ntsr_01.bin", 0x00000,  0x10000, CRC(3a4325f2) SHA1(bfd1797d4a5acb83866438ae70f79b3d78aec003) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "ntsr_03.bin", 0x00000,  0x10000, CRC(747ba06a) SHA1(e9ed4e29edc10b95ee1bca775c9c647c4fee634e) )
	ROM_LOAD( "ntsr_02.bin", 0x10000,  0x10000, CRC(12334718) SHA1(607e4053cc76ec1b4284138959af295d982f522a) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "ntsr_04.bin", 0x000000, 0x20000, CRC(06edf3a4) SHA1(a6cdd7de870f2c38d91669352ba82dca19eb8edf) )
	ROM_LOAD( "ntsr_05.bin", 0x020000, 0x20000, CRC(b3f014fa) SHA1(942a3b66946d09a745daa4e2e1d6f092689ebef3) )
	ROM_LOAD( "ntsr_06.bin", 0x040000, 0x10000, CRC(9333ebcb) SHA1(72a103b1991b684ea81e37da7f47668bc40df1bc) )
	ROM_LOAD( "ntsr_07.bin", 0x050000, 0x10000, CRC(0948f999) SHA1(54401048af2c0f670aebf357f32cc4d643fe95e7) )
	ROM_LOAD( "ntsr_08.bin", 0x060000, 0x10000, CRC(abbd7494) SHA1(b04e10f711c7a83b6a036a99c8e3429880c2b790) )
	ROM_LOAD( "ntsr_09.bin", 0x070000, 0x10000, CRC(dd84badd) SHA1(e023580a457822f1ca02440e5364a2a1c3e32961) )
	ROM_LOAD( "ntsr_10.bin", 0x080000, 0x10000, CRC(7083a505) SHA1(89dfc327dcf375f295ebd148dfb91cd2a3abdbe7) )
	ROM_LOAD( "ntsr_11.bin", 0x090000, 0x10000, CRC(45ed0f6d) SHA1(bbb5524f5383688f82c179e22fc837e437e06ac7) )
	ROM_LOAD( "ntsr_12.bin", 0x0a0000, 0x10000, CRC(3d51ae82) SHA1(5f81da8e6ad7f7f38ebc34724c0f470e99290d56) )
	ROM_LOAD( "ntsr_13.bin", 0x0b0000, 0x10000, CRC(eccde427) SHA1(9e4aac4832211a5d58ab3f4012da7146c8365ab4) )
	ROM_LOAD( "ntsr_14.bin", 0x0c0000, 0x10000, CRC(dd21bbfb) SHA1(91a80fa3c53f9f5b1c08b265b5752dd091c425a5) )
	ROM_LOAD( "ntsr_15.bin", 0x0d0000, 0x10000, CRC(5556024b) SHA1(b9caf22394e58215df0ff9f8a8556f6f146ddcb3) )
	ROM_LOAD( "ntsr_16.bin", 0x0e0000, 0x10000, CRC(f1273c7f) SHA1(3a97860265e856f74e2de06e2cfdb2505f04f152) )
	ROM_LOAD( "ntsr_17.bin", 0x0f0000, 0x10000, CRC(d5574307) SHA1(e502f66ddf9d77703e1978410fe9abf7ac30519f) )
	ROM_LOAD( "ntsr_18.bin", 0x100000, 0x10000, CRC(71566140) SHA1(7f1755b03e87c0030144df1a744e2a1578653c3e) )
	ROM_LOAD( "ntsr_19.bin", 0x110000, 0x10000, CRC(6c880b9d) SHA1(7915505de353c508491f9ed52b389e10467f9310) )
	ROM_LOAD( "ntsr_20.bin", 0x120000, 0x10000, CRC(4b832d37) SHA1(b86dca689aea608ebf9cd77d383db591c1b8f6ab) )
	ROM_LOAD( "ntsr_21.bin", 0x130000, 0x10000, CRC(133183db) SHA1(9c01c8c4f3dd2eaaa93cad78387d478529494f21) )
ROM_END

ROM_START( mjlstory )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "mjls_01.bin", 0x00000,  0x10000, CRC(a9febe8b) SHA1(b29570434145885120304ab001e6dd15fb40d528) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "mjls_03.bin", 0x00000,  0x10000, CRC(15e54af0) SHA1(2553cf77e0be996ddf9bce901bbabe8dec4c7884) )
	ROM_LOAD( "mjls_02.bin", 0x10000,  0x10000, CRC(da976e4f) SHA1(da8a256ed6376f059fe3d4b9a3550e0c338c5a1c) )

	ROM_REGION( 0x190000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mjls_04.bin", 0x000000, 0x20000, CRC(d3e642ee) SHA1(62032d8b23431b85e722a39436e16f5021d44fca) )
	ROM_LOAD( "mjls_05.bin", 0x020000, 0x20000, CRC(dc888639) SHA1(1370a32f310192dcae56f472d7bc09bc2254cb05) )
	ROM_LOAD( "mjls_06.bin", 0x040000, 0x20000, CRC(8a191142) SHA1(911909518b1b911ec20e664d93eeda2941f5d6d7) )
	ROM_LOAD( "mjls_07.bin", 0x060000, 0x20000, CRC(384b9c40) SHA1(11761954083653450db194f488aeb9302fb1cc69) )
	ROM_LOAD( "mjls_08.bin", 0x080000, 0x20000, CRC(072ac9b6) SHA1(8a82b8bcc108a0fcffeea1ed123f87f683db50bc) )
	ROM_LOAD( "mjls_09.bin", 0x0a0000, 0x20000, CRC(f4dc5e77) SHA1(9624dcd303af56385d3fb1a57ecfef367c0984aa) )
	ROM_LOAD( "mjls_10.bin", 0x0c0000, 0x20000, CRC(aa5a165a) SHA1(4b801fce2254e10c695ea6281bcb30619f1d92f9) )
	ROM_LOAD( "mjls_11.bin", 0x0e0000, 0x20000, CRC(25a44a56) SHA1(40695d4b6b31fdaa52761ff8b4c60eaf3497c994) )
	ROM_LOAD( "mjls_12.bin", 0x100000, 0x20000, CRC(2e19183c) SHA1(14f436ce4b6966cd94e6b0d4ab40fcdf753402bd) )
	ROM_LOAD( "mjls_13.bin", 0x120000, 0x20000, CRC(cc08652c) SHA1(7b8b7f6035f64702c8e96d5c4598d1374d76f8fe) )
	ROM_LOAD( "mjls_14.bin", 0x140000, 0x20000, CRC(f469f3a5) SHA1(331f6aacd73a62c7756d88690e29b150ece917f2) )
	ROM_LOAD( "mjls_15.bin", 0x160000, 0x20000, CRC(815b187a) SHA1(467d58a89871024d07dcad35372aa10d7e65498f) )
	ROM_LOAD( "mjls_16.bin", 0x180000, 0x10000, CRC(53366690) SHA1(9155897a886d0a1ce3ab5bb5389dd8852a014244) )
ROM_END

ROM_START( ladymakr )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "1.e4", 0x00000,  0x10000, CRC(c5b5c37d) SHA1(a7a1d504c3045f9c3767ed3b76ce284a3feb9b28) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "3.t4", 0x00000,  0x10000, CRC(15e54af0) SHA1(2553cf77e0be996ddf9bce901bbabe8dec4c7884) )
	ROM_LOAD( "2.s4", 0x10000,  0x10000, CRC(da976e4f) SHA1(da8a256ed6376f059fe3d4b9a3550e0c338c5a1c) )

	ROM_REGION( 0x190000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.a8",  0x000000, 0x20000, CRC(79ff2c90) SHA1(4cffb9116090ec7b59367225330f98e7cc62d5f5) )
	ROM_LOAD( "5.b8",  0x020000, 0x20000, CRC(a275025d) SHA1(becaa9d79b53eddc2f9be8248ae13907d3ca126d) )
	ROM_LOAD( "6.c8",  0x040000, 0x20000, CRC(8a191142) SHA1(911909518b1b911ec20e664d93eeda2941f5d6d7) )
	ROM_LOAD( "7.e8",  0x060000, 0x20000, CRC(384b9c40) SHA1(11761954083653450db194f488aeb9302fb1cc69) )
	ROM_LOAD( "8.f8",  0x080000, 0x20000, CRC(072ac9b6) SHA1(8a82b8bcc108a0fcffeea1ed123f87f683db50bc) )
	ROM_LOAD( "9.j8",  0x0a0000, 0x20000, CRC(f4dc5e77) SHA1(9624dcd303af56385d3fb1a57ecfef367c0984aa) )
	ROM_LOAD( "10.k8", 0x0c0000, 0x20000, CRC(aa5a165a) SHA1(4b801fce2254e10c695ea6281bcb30619f1d92f9) )
	ROM_LOAD( "11.l8", 0x0e0000, 0x20000, CRC(25a44a56) SHA1(40695d4b6b31fdaa52761ff8b4c60eaf3497c994) )
	ROM_LOAD( "12.m8", 0x100000, 0x20000, CRC(2e19183c) SHA1(14f436ce4b6966cd94e6b0d4ab40fcdf753402bd) )
	ROM_LOAD( "13.p8", 0x120000, 0x20000, CRC(cc08652c) SHA1(7b8b7f6035f64702c8e96d5c4598d1374d76f8fe) )
	ROM_LOAD( "14.a7", 0x140000, 0x20000, CRC(f469f3a5) SHA1(331f6aacd73a62c7756d88690e29b150ece917f2) )
	ROM_LOAD( "15.b7", 0x160000, 0x20000, CRC(a85b22df) SHA1(7af2d90ab45720fa0f744a26cf7e572f40772ea9) )
	ROM_LOAD( "16.c7", 0x180000, 0x10000, CRC(c25889d1) SHA1(5cb526c56ebe6a34c537144574489625325955b7) )
ROM_END

ROM_START( vanilla )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "vanilla.01", 0x00000,  0x10000, CRC(2a3341a8) SHA1(d434adf3e2dd0c95b614a0208e874bdd6bc2ede7) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "vanilla.03", 0x00000,  0x10000, CRC(e035842f) SHA1(47e03d148458602d727fe58b86084d24111f4a37) )
	ROM_LOAD( "vanilla.02", 0x10000,  0x10000, CRC(93d8398a) SHA1(8e8a235c840546e6ff6dca174abcf68944536e27) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "vanilla.04", 0x000000, 0x20000, CRC(f21e1ff4) SHA1(22330b41e4f6f736309f8b29360934f62616f80f) )
	ROM_LOAD( "vanilla.05", 0x020000, 0x20000, CRC(15d6ff78) SHA1(c1b5f65fd30f0f0640083c2aed3926045206ca8b) )
	ROM_LOAD( "vanilla.06", 0x040000, 0x20000, CRC(90da7b35) SHA1(344052ea33c680cc03d1a056ee134ad309065f97) )
	ROM_LOAD( "vanilla.07", 0x060000, 0x20000, CRC(71b2896f) SHA1(60d23b68ccf84d1a8fdba913ef73719e86bea281) )
	ROM_LOAD( "vanilla.08", 0x080000, 0x20000, CRC(dd195233) SHA1(f3cdd3822021b019e24252394f0cb09165e115ff) )
	ROM_LOAD( "vanilla.09", 0x0a0000, 0x20000, CRC(5521c7a1) SHA1(d155ff601651b4f5f29ec2dcc6d417d5085fb68a) )
	ROM_LOAD( "vanilla.10", 0x0c0000, 0x20000, CRC(e7d781da) SHA1(70e507d82de6f09e159d73d56c080051012b42dd) )
	ROM_LOAD( "vanilla.11", 0x0e0000, 0x20000, CRC(ba7fbf3d) SHA1(8f088baff3ba9dc2bd35fe6c9dd9f26c740c635d) )
	ROM_LOAD( "vanilla.12", 0x100000, 0x20000, CRC(56fe9708) SHA1(b5abac2ced3f6310a25034325ba4e63d74cc50a0) )
	ROM_LOAD( "vanilla.13", 0x120000, 0x20000, CRC(91011a9e) SHA1(af541e9cb8e9f6477a890eeb0016ec9378ab1c0f) )
	ROM_LOAD( "vanilla.14", 0x140000, 0x20000, CRC(460db736) SHA1(9dcb155ac9eb0335556724602bc43099e0413f7d) )
	ROM_LOAD( "vanilla.15", 0x160000, 0x20000, CRC(f977655c) SHA1(22d38dcb85d64a1adf20c213d265a07f20621942) )
	ROM_LOAD( "vanilla.16", 0x180000, 0x10000, CRC(f286a9db) SHA1(b1f1e700fd6f809b0602112ffa219b92e1c40b01) )
	ROM_LOAD( "vanilla.17", 0x190000, 0x10000, CRC(9b0a7bb5) SHA1(4a04851988028f0a427759c4cf7ac7afba8119aa) )
	ROM_LOAD( "vanilla.18", 0x1a0000, 0x10000, CRC(54120c24) SHA1(f1466dae2b2c4f85e3f77af42105dde1433de10a) )
	ROM_LOAD( "vanilla.19", 0x1b0000, 0x10000, CRC(c1bb8643) SHA1(764b4f5b9c46b7c0ac53de61410cb979ec6f387e) )
	ROM_LOAD( "vanilla.20", 0x1c0000, 0x10000, CRC(26bb26a0) SHA1(3f54e7ed690d38b594f118328b7a35c71cb59045) )
	ROM_LOAD( "vanilla.21", 0x1d0000, 0x10000, CRC(61046b51) SHA1(25f2850bd0c0e05d44b888988436cc46ad003d25) )
	ROM_LOAD( "vanilla.22", 0x1e0000, 0x10000, CRC(66de02e6) SHA1(2081fcabd2ac9da2720fa64c5dbdbe97ade12187) )
	ROM_LOAD( "vanilla.23", 0x1f0000, 0x10000, CRC(64186e8a) SHA1(3dbcc4d79e2e28e67267e6cb76f42dadb2f974e9) )
ROM_END

ROM_START( finalbny )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "22.4e",      0x00000,  0x10000, CRC(ccb85d99) SHA1(1fb64fd2cd8bccd6bfd9496792d5e57f128d270e) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "3.4t",       0x00000,  0x10000, CRC(f5d60735) SHA1(6149889fb6646a9807b5394256b4f977d365e9e1) )
	ROM_LOAD( "vanilla.02", 0x10000,  0x10000, CRC(93d8398a) SHA1(8e8a235c840546e6ff6dca174abcf68944536e27) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "vanilla.04", 0x000000, 0x20000, CRC(f21e1ff4) SHA1(22330b41e4f6f736309f8b29360934f62616f80f) )
	ROM_LOAD( "vanilla.05", 0x020000, 0x20000, CRC(15d6ff78) SHA1(c1b5f65fd30f0f0640083c2aed3926045206ca8b) )
	ROM_LOAD( "vanilla.06", 0x040000, 0x20000, CRC(90da7b35) SHA1(344052ea33c680cc03d1a056ee134ad309065f97) )
	ROM_LOAD( "vanilla.07", 0x060000, 0x20000, CRC(71b2896f) SHA1(60d23b68ccf84d1a8fdba913ef73719e86bea281) )
	ROM_LOAD( "vanilla.08", 0x080000, 0x20000, CRC(dd195233) SHA1(f3cdd3822021b019e24252394f0cb09165e115ff) )
	ROM_LOAD( "vanilla.09", 0x0a0000, 0x20000, CRC(5521c7a1) SHA1(d155ff601651b4f5f29ec2dcc6d417d5085fb68a) )
	ROM_LOAD( "vanilla.10", 0x0c0000, 0x20000, CRC(e7d781da) SHA1(70e507d82de6f09e159d73d56c080051012b42dd) )
	ROM_LOAD( "vanilla.11", 0x0e0000, 0x20000, CRC(ba7fbf3d) SHA1(8f088baff3ba9dc2bd35fe6c9dd9f26c740c635d) )
	ROM_LOAD( "vanilla.12", 0x100000, 0x20000, CRC(56fe9708) SHA1(b5abac2ced3f6310a25034325ba4e63d74cc50a0) )
	ROM_LOAD( "vanilla.13", 0x120000, 0x20000, CRC(91011a9e) SHA1(af541e9cb8e9f6477a890eeb0016ec9378ab1c0f) )
	ROM_LOAD( "vanilla.14", 0x140000, 0x20000, CRC(460db736) SHA1(9dcb155ac9eb0335556724602bc43099e0413f7d) )
	ROM_LOAD( "vanilla.15", 0x160000, 0x20000, CRC(f977655c) SHA1(22d38dcb85d64a1adf20c213d265a07f20621942) )
	ROM_LOAD( "16.7d",      0x180000, 0x10000, CRC(7d122177) SHA1(d84fe29e7ded34d5c28629a440e2168637e332be) )
	ROM_LOAD( "17.7e",      0x190000, 0x10000, CRC(3cfb4265) SHA1(44d90d759fa6bf5a7ecaef36435b41edfb3ee27e) )
	ROM_LOAD( "18.7f",      0x1a0000, 0x10000, CRC(7b8ca753) SHA1(e0cdc71cec4db37adda0a67e748afd47e576a956) )
	ROM_LOAD( "19.7j",      0x1b0000, 0x10000, CRC(d7deca63) SHA1(eac27eb9cf4e26f72f26e11aac4edfa639ca9d84) )
ROM_END

ROM_START( qmhayaku )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "1.4e",  0x00000,  0x10000, CRC(5a73cdf8) SHA1(bec7764175b1cc142ed08b18e3ff240c4c996e36) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "3.4t",  0x00000,  0x10000, CRC(d420dac8) SHA1(d46c383a23d502b8b8988ca0267e09b6ad9faa73) )
	ROM_LOAD( "2.4s",  0x10000,  0x10000, CRC(f88cb623) SHA1(458fe4f9f3c4a8edf6c01c6e5bd39ab44a87e9a7) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.9b",  0x000000, 0x20000, CRC(2fba26fe) SHA1(02afaed3f1726eec6810eabe767a594f97fafdbf) )
	ROM_LOAD( "5.9d",  0x020000, 0x20000, CRC(105f9930) SHA1(1e3f17e0eef5f2d0f697d7ccecf429c89806687b) )
	ROM_LOAD( "6.9e",  0x040000, 0x20000, CRC(5e8f0177) SHA1(1bf797209296e1d689ef15b915151fc700d1253a) )
	ROM_LOAD( "7.9f",  0x060000, 0x20000, CRC(612803ba) SHA1(bb604224cc361f15b2e7c56e90159ce84ae28d13) )
	ROM_LOAD( "8.9j",  0x080000, 0x20000, CRC(874fe074) SHA1(91f79425a8951593ec9edba0f0b059ce917c9490) )
	ROM_LOAD( "9.9k",  0x0a0000, 0x20000, CRC(afa873d2) SHA1(6f9a1e4ab79d337414c068402e25ff9e19d742a1) )
	ROM_LOAD( "10.9l", 0x0c0000, 0x20000, CRC(17a4a609) SHA1(ac9cdbb0fcf96f7f40b50d22506b39cfb4093ee8) )
	ROM_LOAD( "11.9n", 0x0e0000, 0x20000, CRC(d2357c72) SHA1(8e370dd4f3c37a74140898996c78be871a192477) )
	ROM_LOAD( "12.9p", 0x100000, 0x20000, CRC(4b63c040) SHA1(6861143571fe690d43f7c2e2bcca16537a24a16e) )
	ROM_LOAD( "13.7a", 0x120000, 0x20000, CRC(a182d9cd) SHA1(d4fdb06c2941ef10d2b2660fe9de8fabdb13cf06) )
	ROM_LOAD( "14.7b", 0x140000, 0x20000, CRC(22b1f1fd) SHA1(e69a19df993cd75dd00eac6a7119d7f7ab1c2a2f) )
	ROM_LOAD( "15.7d", 0x160000, 0x20000, CRC(3db4df6c) SHA1(114f3853ddb61ac2d709e99fb9afd70e01dbca9f) )
	ROM_LOAD( "16.7e", 0x180000, 0x20000, CRC(c1283063) SHA1(4cecb2e46eaa7d8be8fcce02e3cad1114316ed6c) )
	ROM_LOAD( "17.7f", 0x1a0000, 0x10000, CRC(4ca71ef1) SHA1(5d5af252711e813a99299cfe26cf7d0385ec330d) )
	ROM_LOAD( "18.7j", 0x1b0000, 0x10000, CRC(81190d74) SHA1(16b9bdb87e0f54f7a8cf29854cdcee6d344ca98a) )
	ROM_LOAD( "19.7k", 0x1c0000, 0x10000, CRC(cad37c2f) SHA1(6338d43e74400d8c2224b9aa520ba8b86bf85b99) )
	ROM_LOAD( "20.7l", 0x1d0000, 0x10000, CRC(18e18174) SHA1(e80f140f1e389569ea4db63e544df82afaf30791) )
ROM_END

ROM_START( galkoku )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "gkok_01.bin", 0x00000,  0x10000, CRC(254c526c) SHA1(644095cccf8812342ddc0eb452db4ec032965152) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "gkok_02.bin", 0x00000,  0x10000, CRC(3dec7469) SHA1(e464899e7f2e1a6bf88260c6bc746df94623ef0e) )
	ROM_LOAD( "gkok_03.bin", 0x10000,  0x10000, CRC(66f51b21) SHA1(bac5f9e20bd2eac63e5bbd72178e39b4892b0ba4) )

	ROM_REGION( 0x110000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "gkok_04.bin", 0x000000, 0x10000, CRC(741815a5) SHA1(5ddb61d88738d15e1f68df2deaa0b1612803a23e) )
	ROM_LOAD( "gkok_05.bin", 0x010000, 0x10000, CRC(28a17cd8) SHA1(dfb4066898bd66b8306252695e9f801c8e44f25e) )
	ROM_LOAD( "gkok_06.bin", 0x020000, 0x10000, CRC(8eac2143) SHA1(75cd75de761144b31dec387083f5a11846e6809f) )
	ROM_LOAD( "gkok_07.bin", 0x030000, 0x10000, CRC(de5f3f20) SHA1(dd0eab9e3a7422638836859c7d7317855d7dc62e) )
	ROM_LOAD( "gkok_08.bin", 0x040000, 0x10000, CRC(f3348126) SHA1(b2633e4ed2395f8978d88959632166d66bd29e0b) )
	ROM_LOAD( "gkok_09.bin", 0x050000, 0x10000, CRC(691f2521) SHA1(79eb7b8dab160f30af7cb81a565d5ecdce341930) )
	ROM_LOAD( "gkok_10.bin", 0x060000, 0x10000, CRC(f1b0b411) SHA1(b77b6026dd3daa42e00820c2a473ede3b7733c3e) )
	ROM_LOAD( "gkok_11.bin", 0x070000, 0x10000, CRC(ef42af9e) SHA1(c1972aa92ffdcb5f462256cee2a3e38df8922d64) )
	ROM_LOAD( "gkok_12.bin", 0x080000, 0x10000, CRC(e2b32195) SHA1(208ff04f951ba1cd5f96a484820e12005ed66e5f) )
	ROM_LOAD( "gkok_13.bin", 0x090000, 0x10000, CRC(83d913a1) SHA1(831dc254e1300b98c236dc934bbe3c91e42dea60) )
	ROM_LOAD( "gkok_14.bin", 0x0a0000, 0x10000, CRC(04c97de9) SHA1(03d5be851107e0e736c2ae3bb8cec0be0783174b) )
	ROM_LOAD( "gkok_15.bin", 0x0b0000, 0x10000, CRC(3845280d) SHA1(1b86a396c9f2affb9431bc878405d87cb30b4ac0) )
	ROM_LOAD( "gkok_16.bin", 0x0c0000, 0x10000, CRC(7472a7ce) SHA1(321ee7caeb76e6963f33ebed3db1d81bdff4de30) )
	ROM_LOAD( "gkok_17.bin", 0x0d0000, 0x10000, CRC(92b605a2) SHA1(3f84579a855a89361ae0449c4dd4cbfe9cbae06e) )
	ROM_LOAD( "gkok_18.bin", 0x0e0000, 0x10000, CRC(8bb7bdcc) SHA1(0d025f9b96a606fa777d11602863c118862450d5) )
	ROM_LOAD( "gkok_19.bin", 0x0f0000, 0x10000, CRC(b1b4643a) SHA1(d4b7f1cad1b544d2800d7358ff90b7fa19ff0ca2) )
	ROM_LOAD( "gkok_20.bin", 0x100000, 0x10000, CRC(36107e6f) SHA1(0872d0ae2add129bdd036754fd5d751627bc142e) )
ROM_END

ROM_START( hyouban ) // Medal Series No. 124N, according to dip sheet
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.3d",        0x00000,  0x10000, CRC(307b4f7e) SHA1(303e1818cb12ede15dadec165f18a6a33d564d5e) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "gkok_02.bin", 0x00000,  0x10000, CRC(3dec7469) SHA1(e464899e7f2e1a6bf88260c6bc746df94623ef0e) )
	ROM_LOAD( "gkok_03.bin", 0x10000,  0x10000, CRC(66f51b21) SHA1(bac5f9e20bd2eac63e5bbd72178e39b4892b0ba4) )

	ROM_REGION( 0x110000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "gkok_04.bin", 0x000000, 0x10000, CRC(741815a5) SHA1(5ddb61d88738d15e1f68df2deaa0b1612803a23e) )
	ROM_LOAD( "gkok_05.bin", 0x010000, 0x10000, CRC(28a17cd8) SHA1(dfb4066898bd66b8306252695e9f801c8e44f25e) )
	ROM_LOAD( "6.10d",       0x020000, 0x10000, CRC(2a941698) SHA1(25d7c1885850c9a3d9a4facb8ce42ef140ebb0e4) )
	ROM_LOAD( "gkok_07.bin", 0x030000, 0x10000, CRC(de5f3f20) SHA1(dd0eab9e3a7422638836859c7d7317855d7dc62e) )
	ROM_LOAD( "gkok_08.bin", 0x040000, 0x10000, CRC(f3348126) SHA1(b2633e4ed2395f8978d88959632166d66bd29e0b) )
	ROM_LOAD( "gkok_09.bin", 0x050000, 0x10000, CRC(691f2521) SHA1(79eb7b8dab160f30af7cb81a565d5ecdce341930) )
	ROM_LOAD( "gkok_10.bin", 0x060000, 0x10000, CRC(f1b0b411) SHA1(b77b6026dd3daa42e00820c2a473ede3b7733c3e) )
	ROM_LOAD( "gkok_11.bin", 0x070000, 0x10000, CRC(ef42af9e) SHA1(c1972aa92ffdcb5f462256cee2a3e38df8922d64) )
	ROM_LOAD( "gkok_12.bin", 0x080000, 0x10000, CRC(e2b32195) SHA1(208ff04f951ba1cd5f96a484820e12005ed66e5f) )
	ROM_LOAD( "gkok_13.bin", 0x090000, 0x10000, CRC(83d913a1) SHA1(831dc254e1300b98c236dc934bbe3c91e42dea60) )
	ROM_LOAD( "gkok_14.bin", 0x0a0000, 0x10000, CRC(04c97de9) SHA1(03d5be851107e0e736c2ae3bb8cec0be0783174b) )
	ROM_LOAD( "gkok_15.bin", 0x0b0000, 0x10000, CRC(3845280d) SHA1(1b86a396c9f2affb9431bc878405d87cb30b4ac0) )
	ROM_LOAD( "gkok_16.bin", 0x0c0000, 0x10000, CRC(7472a7ce) SHA1(321ee7caeb76e6963f33ebed3db1d81bdff4de30) )
	ROM_LOAD( "gkok_17.bin", 0x0d0000, 0x10000, CRC(92b605a2) SHA1(3f84579a855a89361ae0449c4dd4cbfe9cbae06e) )
	ROM_LOAD( "gkok_18.bin", 0x0e0000, 0x10000, CRC(8bb7bdcc) SHA1(0d025f9b96a606fa777d11602863c118862450d5) )
	ROM_LOAD( "gkok_19.bin", 0x0f0000, 0x10000, CRC(b1b4643a) SHA1(d4b7f1cad1b544d2800d7358ff90b7fa19ff0ca2) )
	ROM_LOAD( "gkok_20.bin", 0x100000, 0x10000, CRC(36107e6f) SHA1(0872d0ae2add129bdd036754fd5d751627bc142e) )
ROM_END

ROM_START( galkaika )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "gkai_01.bin", 0x00000,  0x10000, CRC(81b89559) SHA1(9c9136f9483a23eafdf69bc8a31659c772533d01) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "gkai_02.bin", 0x00000,  0x10000, CRC(db899dd5) SHA1(cd77ada2a8e1b0e03e66aa66c207aa3b07f51e19) )
	ROM_LOAD( "gkai_03.bin", 0x10000,  0x10000, CRC(a66a1c52) SHA1(171da75df9e34f4fb36892c3074a305ae843754f) )

	ROM_REGION( 0x120000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "gkai_04.bin", 0x000000, 0x20000, CRC(b1071e49) SHA1(ea31e866a8d912eacc5c9f90eda23373babe02e1) )
	ROM_LOAD( "gkai_05.bin", 0x020000, 0x20000, CRC(e5162326) SHA1(fafeffedc253bb02ad1a699bda8c2ea2d6c3b076) )
	ROM_LOAD( "gkai_06.bin", 0x040000, 0x20000, CRC(e0cebb15) SHA1(32c31ece2e7f9caa473ae1d4b965d422fa68ee5e) )
	ROM_LOAD( "gkai_07.bin", 0x060000, 0x20000, CRC(26915aa7) SHA1(f12657d3f3afd26eb232737f1dbaedc5f295e6f4) )
	ROM_LOAD( "gkai_08.bin", 0x080000, 0x20000, CRC(df009be3) SHA1(3490081a09e76e78188a117bf884ba4cfa2b7bc9) )
	ROM_LOAD( "gkai_09.bin", 0x0a0000, 0x20000, CRC(cebfb4f3) SHA1(5ad3504fb69732271265c1b0ad2e83511bfd0289) )
	ROM_LOAD( "gkai_10.bin", 0x0c0000, 0x20000, CRC(43ecb3c5) SHA1(79d11b7b9b9597605cd6d3b8cecd615b706708b8) )
	ROM_LOAD( "gkai_11.bin", 0x0e0000, 0x20000, CRC(66f4dbfa) SHA1(8bf1947a692116e6760e08ab95597e326518b6c9) )
	ROM_LOAD( "gkai_12.bin", 0x100000, 0x10000, CRC(dc35168a) SHA1(966faf6f673a2c9262c472e7bf19bc0aed42116b) )
	ROM_LOAD( "gkai_13.bin", 0x110000, 0x10000, CRC(d9f495f3) SHA1(06fc64088568564ac8ec7103585fff48962f3a2e) )
ROM_END

ROM_START( tokyogal )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "tgal_21.bin", 0x00000,  0x10000, CRC(ad4eecec) SHA1(76f7411d504de9d75b4004966c69655c0df3870d) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "tgal_22.bin", 0x00000,  0x10000, CRC(36be0868) SHA1(e061326ede61c2fc1a551bc6e29a38d5977dd563) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "tgal_01.bin", 0x000000, 0x10000, CRC(6a7a5c13) SHA1(ee5630e68fa00464b54e7cfd68a91c0bf2eac135) )
	ROM_LOAD( "tgal_02.bin", 0x010000, 0x10000, CRC(31e052e6) SHA1(b93be56425ac56e463515ec0226a166aac97cec0) )
	ROM_LOAD( "tgal_03.bin", 0x020000, 0x10000, CRC(d4bbf1e6) SHA1(1c4a364f545560c124db9a459bb3c6df2278cfe7) )
	ROM_LOAD( "tgal_04.bin", 0x030000, 0x10000, CRC(f2b30256) SHA1(c4a331ee52e5ce7c687545f8774dbbced192120a) )
	ROM_LOAD( "tgal_05.bin", 0x040000, 0x10000, CRC(af820677) SHA1(43cd9ee8b0af9d864617b6f26b8683bb30019ecd) )
	ROM_LOAD( "tgal_06.bin", 0x050000, 0x10000, CRC(d9ff9b76) SHA1(d8e84bd6690b3ab8ab90b2ec8477be9a8eb64199) )
	ROM_LOAD( "tgal_07.bin", 0x060000, 0x10000, CRC(d5288e37) SHA1(c8d3947e117869d4895b2c5056e2dd97819e50d5) )
	ROM_LOAD( "tgal_08.bin", 0x070000, 0x10000, CRC(824fa5cc) SHA1(2320cce871b705271fc95f3fc2d6124efe463b86) )
	ROM_LOAD( "tgal_09.bin", 0x080000, 0x10000, CRC(795b8f8c) SHA1(bd1bc4a602b03d3dfa11c646e0d3efb7a5b2f85c) )
	ROM_LOAD( "tgal_10.bin", 0x090000, 0x10000, CRC(f2c13f7a) SHA1(28c22072d9d85fd7f29820b3f9b36a8d51893658) )
	ROM_LOAD( "tgal_11.bin", 0x0a0000, 0x10000, CRC(551f6fb4) SHA1(508340982ab5456ec7b6e51c756da3ef5603e366) )
	ROM_LOAD( "tgal_12.bin", 0x0b0000, 0x10000, CRC(78db30a7) SHA1(66a57c0ca5a1a8cc88e882213ffbb9b52ec5b471) )
	ROM_LOAD( "tgal_13.bin", 0x0c0000, 0x10000, CRC(04a81e7a) SHA1(34b1255c3e44b7e88379723e87681324d0578962) )
	ROM_LOAD( "tgal_14.bin", 0x0d0000, 0x10000, CRC(12b43b21) SHA1(806662fb8b66ac89c3347060883eece8bc565cbe) )
	ROM_LOAD( "tgal_15.bin", 0x0e0000, 0x10000, CRC(af06f649) SHA1(384086412d6c009ae6134656fb3aa98c33395346) )
	ROM_LOAD( "tgal_16.bin", 0x0f0000, 0x10000, CRC(2996431a) SHA1(ebf925b8850ee9832d21aabf1898fdec619862fe) )
	ROM_LOAD( "tgal_17.bin", 0x100000, 0x10000, CRC(470dde3c) SHA1(a0439c8f9ec5bae95dd4b6b345c1bc9a9d8a20bc) )
	ROM_LOAD( "tgal_18.bin", 0x110000, 0x10000, CRC(0d04d3bc) SHA1(1f425f104723c7ce0a3d2f63ee63b19dbf739a4e) )
	ROM_LOAD( "tgal_19.bin", 0x120000, 0x10000, CRC(1c8fe0e8) SHA1(6f8c09b44e027b0c99ba2610092ed6a28aa45269) )
	ROM_LOAD( "tgal_20.bin", 0x130000, 0x10000, CRC(b8542eeb) SHA1(9d9897a7eb267fcf4cc578d4ab6d8ac4c13082e3) )
ROM_END

ROM_START( tokimbsj )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "tmbj_01.bin", 0x00000,  0x10000, CRC(b335c300) SHA1(4e35400285a062bfe204b9a82e676dd4d86691d5) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "tgal_22.bin", 0x00000,  0x10000, CRC(36be0868) SHA1(e061326ede61c2fc1a551bc6e29a38d5977dd563) )

	ROM_REGION( 0x140000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "tgal_01.bin", 0x000000, 0x10000, CRC(6a7a5c13) SHA1(ee5630e68fa00464b54e7cfd68a91c0bf2eac135) )
	ROM_LOAD( "tmbj_04.bin", 0x010000, 0x10000, CRC(09e3f23d) SHA1(e2d5775a0c71ac2470b63db897cd2e5e657c7837) )
	ROM_LOAD( "tgal_03.bin", 0x020000, 0x10000, CRC(d4bbf1e6) SHA1(1c4a364f545560c124db9a459bb3c6df2278cfe7) )
	ROM_LOAD( "tgal_04.bin", 0x030000, 0x10000, CRC(f2b30256) SHA1(c4a331ee52e5ce7c687545f8774dbbced192120a) )
	ROM_LOAD( "tgal_05.bin", 0x040000, 0x10000, CRC(af820677) SHA1(43cd9ee8b0af9d864617b6f26b8683bb30019ecd) )
	ROM_LOAD( "tgal_06.bin", 0x050000, 0x10000, CRC(d9ff9b76) SHA1(d8e84bd6690b3ab8ab90b2ec8477be9a8eb64199) )
	ROM_LOAD( "tgal_07.bin", 0x060000, 0x10000, CRC(d5288e37) SHA1(c8d3947e117869d4895b2c5056e2dd97819e50d5) )
	ROM_LOAD( "tgal_08.bin", 0x070000, 0x10000, CRC(824fa5cc) SHA1(2320cce871b705271fc95f3fc2d6124efe463b86) )
	ROM_LOAD( "tgal_09.bin", 0x080000, 0x10000, CRC(795b8f8c) SHA1(bd1bc4a602b03d3dfa11c646e0d3efb7a5b2f85c) )
	ROM_LOAD( "tgal_10.bin", 0x090000, 0x10000, CRC(f2c13f7a) SHA1(28c22072d9d85fd7f29820b3f9b36a8d51893658) )
	ROM_LOAD( "tgal_11.bin", 0x0a0000, 0x10000, CRC(551f6fb4) SHA1(508340982ab5456ec7b6e51c756da3ef5603e366) )
	ROM_LOAD( "tgal_12.bin", 0x0b0000, 0x10000, CRC(78db30a7) SHA1(66a57c0ca5a1a8cc88e882213ffbb9b52ec5b471) )
	ROM_LOAD( "tgal_13.bin", 0x0c0000, 0x10000, CRC(04a81e7a) SHA1(34b1255c3e44b7e88379723e87681324d0578962) )
	ROM_LOAD( "tgal_14.bin", 0x0d0000, 0x10000, CRC(12b43b21) SHA1(806662fb8b66ac89c3347060883eece8bc565cbe) )
	ROM_LOAD( "tgal_15.bin", 0x0e0000, 0x10000, CRC(af06f649) SHA1(384086412d6c009ae6134656fb3aa98c33395346) )
	ROM_LOAD( "tgal_16.bin", 0x0f0000, 0x10000, CRC(2996431a) SHA1(ebf925b8850ee9832d21aabf1898fdec619862fe) )
	ROM_LOAD( "tgal_17.bin", 0x100000, 0x10000, CRC(470dde3c) SHA1(a0439c8f9ec5bae95dd4b6b345c1bc9a9d8a20bc) )
	ROM_LOAD( "tgal_18.bin", 0x110000, 0x10000, CRC(0d04d3bc) SHA1(1f425f104723c7ce0a3d2f63ee63b19dbf739a4e) )
	ROM_LOAD( "tmbj_21.bin", 0x120000, 0x10000, CRC(b608d6b1) SHA1(cbb004d899e40e33829f7fbea6075aed2d0a47eb) )
	ROM_LOAD( "tmbj_22.bin", 0x130000, 0x10000, CRC(e706fc87) SHA1(fc6f72b1c7dbdc2644458a90861da5279cc46cfb) )
ROM_END

ROM_START( mcontest )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "mcon_01.bin", 0x00000, 0x10000, CRC(79a30028) SHA1(7255cf161010314ae4308455e7573190e42ba27e) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "mcon_02.bin", 0x00000, 0x10000, CRC(236b8fdc) SHA1(afeee7339c83c4e432961a6ddd7579670157b372) )
	ROM_LOAD( "mcon_03.bin", 0x10000, 0x10000, CRC(6d6bdefb) SHA1(077524c030622887455bbd23bcab9f3aac99ade4) )

	ROM_REGION( 0x160000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "mcon_04.bin", 0x000000, 0x20000, CRC(adb6e002) SHA1(f59b4f96c579a1dfe62c07a30bc2b2e7d6e3737a) )
	ROM_LOAD( "mcon_05.bin", 0x020000, 0x20000, CRC(ea8ceb49) SHA1(90b0a47c6fbb2798a845cac03af11a2647c4132c) )
	ROM_LOAD( "mcon_06.bin", 0x040000, 0x10000, CRC(d3fee691) SHA1(bdb13cb5eed5f6d092e06d1e539096faa506f98d) )
	ROM_LOAD( "mcon_07.bin", 0x050000, 0x10000, CRC(7685a1b1) SHA1(027183e55b8e372791d4a2333c3b020dc78c49f1) )
	ROM_LOAD( "mcon_08.bin", 0x060000, 0x10000, CRC(eee52454) SHA1(55f1b46bba1faabb48ca9969bcc6ca53fd0a4a47) )
	ROM_LOAD( "mcon_09.bin", 0x070000, 0x10000, CRC(2ad2d00f) SHA1(544dc6d6bec118ee672acbf0024e8c70ace11c0e) )
	ROM_LOAD( "mcon_10.bin", 0x080000, 0x10000, CRC(6ff32ed9) SHA1(3ab071a14a0ce72f17c281bebda393e530b3b67d) )
	ROM_LOAD( "mcon_11.bin", 0x090000, 0x10000, CRC(4f9c340f) SHA1(9486b75b6d0aa22170fe0c90b6f64f940537fc03) )
	ROM_LOAD( "mcon_12.bin", 0x0a0000, 0x10000, CRC(41cffdf0) SHA1(08201f913f29d5ad940d1afdd25267d0d7d8fea4) )
	ROM_LOAD( "mcon_13.bin", 0x0b0000, 0x10000, CRC(d494fdb7) SHA1(658a4e4d2a142faca7bc0e026786f80a48a2a5b6) )
	ROM_LOAD( "mcon_14.bin", 0x0c0000, 0x10000, CRC(9fe3f75d) SHA1(4722e8c4ffc9df742fa1092f915fb35555ec05c3) )
	ROM_LOAD( "mcon_15.bin", 0x0d0000, 0x10000, CRC(79fa427a) SHA1(f7449a95290dcd39537f7773aef6f521f6ac0f4e) )
	ROM_LOAD( "mcon_16.bin", 0x0e0000, 0x10000, CRC(f5ae3668) SHA1(78845a45bd31e1711a185325cd43d3ba6ff49bec) )
	ROM_LOAD( "mcon_17.bin", 0x0f0000, 0x10000, CRC(cb02f51d) SHA1(6532ac691c73a4f175fdfe04b6abeb6d3cde5c42) )
	ROM_LOAD( "mcon_18.bin", 0x100000, 0x10000, CRC(8e5fe1bc) SHA1(92e7a0a7548f491db451e2e1abb3290a10315a1e) )
	ROM_LOAD( "mcon_19.bin", 0x110000, 0x10000, CRC(5b382cf3) SHA1(5dc996ef15005b798ccfa8be7f0f14f3bb310de6) )
	ROM_LOAD( "mcon_20.bin", 0x120000, 0x10000, CRC(8ffbd8fe) SHA1(e45c7981a556b79f81eecb71a244af5aa74f35e0) )
	ROM_LOAD( "mcon_21.bin", 0x130000, 0x10000, CRC(9476d11d) SHA1(0130828fb9cdc72f7f5b6a649fa3f3285d6cc0ed) )
	ROM_LOAD( "mcon_22.bin", 0x140000, 0x10000, CRC(07d21863) SHA1(828bd86af6d5a61a2b0225920701d37811ba5447) )
	ROM_LOAD( "mcon_23.bin", 0x150000, 0x10000, CRC(979e0f93) SHA1(b1f4fedb7b64f79936d43e328e170dd38f712ca2) )
ROM_END

ROM_START( uchuuai )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.3h",   0x00000, 0x10000, CRC(6a6fd569) SHA1(4c3986f111c6db1d375d1ab35a85af0f250ffb4f) )

	ROM_REGION( 0x20000, "voice", 0 ) /* voice */
	ROM_LOAD( "2.3h",   0x00000, 0x10000, CRC(8673ba16) SHA1(6cb3cac1cfdffb8b9b705c5b7e9cd468b039992c) )

	ROM_REGION( 0x160000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "3.10a",  0x000000, 0x20000, CRC(67b8dcd9) SHA1(c063538dbb49c2c9ca313f494f3b50eba9960bb2) )
	ROM_LOAD( "4.10c",  0x020000, 0x20000, CRC(6a3b50ce) SHA1(4ef08220f8ed24d216ca80ec8ff9f01975b6644f) )
	ROM_LOAD( "5.10d",  0x040000, 0x10000, CRC(5334ed3c) SHA1(eaec91a814120abef792fff5baee67fde5e67a23) )
	ROM_LOAD( "6.10e",  0x050000, 0x10000, CRC(2871addf) SHA1(d1eaeba1dd703004b293037b2d3432720f555707) )
	ROM_LOAD( "7.10f",  0x060000, 0x10000, CRC(0a75383d) SHA1(0df9d9ef1775e9a4dede753ab4cd2eba0490dce7) )
	ROM_LOAD( "8.10j",  0x070000, 0x10000, CRC(4a45a098) SHA1(f62cf1454ae12001c6c8e98734a754bea3a903b2) )
	ROM_LOAD( "9.10k",  0x080000, 0x10000, CRC(36ec60f8) SHA1(0801cc6f418493cd7801b43e932426b06b5c34d1) )
	ROM_LOAD( "10.10m", 0x090000, 0x10000, CRC(4f17dce6) SHA1(90f50c704a63835d9a1aae6ec1e7b33d922e35ca) )
	ROM_LOAD( "11.10n", 0x0a0000, 0x10000, CRC(84c31068) SHA1(b6996fad0476936a14d72c8de36b97a84de68226) )
	ROM_LOAD( "12.10p", 0x0b0000, 0x10000, CRC(8a263dfb) SHA1(f06d6c159cf58185af2c461ec199fb2f9b60a90f) )
	ROM_LOAD( "13.11a", 0x0c0000, 0x10000, CRC(3f47bf0b) SHA1(607a8802a9cfea0cb8d3eede853cedca06d6c5f2) )
	ROM_LOAD( "14.11c", 0x0d0000, 0x10000, CRC(89f0143f) SHA1(90a37ba6bb82a5cac2c2b8dbc43b9d54e78c1c24) )
	ROM_LOAD( "15.11d", 0x0e0000, 0x10000, CRC(dc3d52ad) SHA1(5b7ad4c8545b92abebd90a15d71108c632dc45ee) )
	ROM_LOAD( "16.11e", 0x0f0000, 0x10000, CRC(aba3e0c5) SHA1(7dcad1f96d62d55972c5e36ce4c8bce9edbfb968) )
	ROM_LOAD( "17.11f", 0x100000, 0x10000, CRC(23a75436) SHA1(20d11c17f125c174093bba7ce1d05231e1cacc4f) )
	ROM_LOAD( "18.11j", 0x110000, 0x10000, CRC(3602af29) SHA1(d469faa0895fff7b96abae03d4e3556db6c91146) )
	ROM_LOAD( "19.11k", 0x120000, 0x10000, CRC(1c4a3b49) SHA1(cd0c55e584e73bbfcc8985050d845a509ce61900) )
	ROM_LOAD( "20.11m", 0x130000, 0x10000, CRC(cc491fa9) SHA1(8bd6ed061296985ac0ad0ded21ed65107836d664) )
	ROM_LOAD( "21.11n", 0x140000, 0x10000, CRC(ba4e42a1) SHA1(6c5bdcdc8ae9c3869e5cbc14aab92ad2a889519c) )
	ROM_LOAD( "22.11p", 0x150000, 0x10000, CRC(be5ebd80) SHA1(1f9a84339520c5653f89c638d2650cee8effd7fc) )
ROM_END

ROM_START( mjgottub )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "1.4e",  0x00000,  0x10000, CRC(a143c52d) SHA1(9e6b5b5c6f5aa92aa8a71b69a4533e1de3ca962d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sound program */
	ROM_LOAD( "3.4t",  0x00000,  0x10000, CRC(1a8970df) SHA1(f10cfde4ec1833a9945535275e90627ca39915f7) )
	ROM_LOAD( "2.4s",  0x10000,  0x10000, CRC(686450e3) SHA1(84d26ffcad622c6fb4fe1407c8d04c13bd65514e) )

	ROM_REGION( 0x280000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.9a",  0x000000, 0x20000, CRC(58528909) SHA1(ee481ded7a3dbae01e16fd12f9fe806976e87a7f) )
	ROM_LOAD( "5.9b",  0x020000, 0x20000, CRC(d09ad54d) SHA1(0489d9b0755356dd14a05469228cc44784dd7c91) )
	ROM_LOAD( "6.9d",  0x040000, 0x20000, CRC(40346785) SHA1(cb1e51bbd56c68990fee1ba61c04949a65a1ffd1) )
	ROM_LOAD( "7.9e",  0x060000, 0x20000, CRC(e96635e1) SHA1(47b297fcd24ed0efc6230b6a38ab20297edb0d87) )
	ROM_LOAD( "8.9f",  0x080000, 0x20000, CRC(174d7ad6) SHA1(894666c5038c88a6532f9a66686d13118aafa82f) )
	ROM_LOAD( "9.9j",  0x0a0000, 0x20000, CRC(65fd9c90) SHA1(a2fca559453ab34184b909db48cfede6ad8e5833) )
	ROM_LOAD( "10.9k", 0x0c0000, 0x20000, CRC(7d9a681a) SHA1(bd482cd1a13774f3f24299250abf69542483f71b) )
	ROM_LOAD( "11.9l", 0x0e0000, 0x20000, CRC(7a484783) SHA1(5219841db6c747ea0cd9634ecff3ad80d360c825) )
	ROM_LOAD( "12.9n", 0x100000, 0x20000, CRC(2ffd55be) SHA1(20d291102c0194d74faba76aba9bbf0836b86355) )
	ROM_LOAD( "13.9p", 0x120000, 0x20000, CRC(7a731fa9) SHA1(55266d1b62dd84c5ed4653f5d57d0980e97257a8) )
	ROM_LOAD( "14.7a", 0x140000, 0x20000, CRC(6d4e56f7) SHA1(c29d0ff6b8ccde7c76d70d267f95004f18b9ca1e) )
	ROM_LOAD( "15.7b", 0x160000, 0x20000, CRC(de3a675c) SHA1(c4c65feb5697a070184fa72be4136fdf404d07b8) )
	ROM_LOAD( "16.7d", 0x180000, 0x20000, CRC(e1d6d504) SHA1(b14a5d67318e2ef3f664db3b0a9a2778f675be15) )
	ROM_LOAD( "17.7e", 0x1a0000, 0x20000, CRC(ca1bca8d) SHA1(3d4138dc9ad4cd366585bc9ae75f612917e5447d) )
	ROM_LOAD( "18.7f", 0x1c0000, 0x20000, CRC(a69973ad) SHA1(fda9046256ef33b668aa8b7d0a697acc32df2a83) )
	ROM_LOAD( "19.7j", 0x1e0000, 0x20000, CRC(5048733d) SHA1(6aee991343ce6bb6ce34141a6daac8f770fac7f9) )
ROM_END

ROM_START( av2mj1bb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.bin",      0x00000, 0x10000, CRC(df0f03fb) SHA1(e6c2d6905b507611782bbd29f4d5fcd378a0a993) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "3.bin",      0x00000, 0x10000, CRC(0cdc9489) SHA1(dfcee760679a893fed98bdb7642889d7491f00a7) )
	ROM_LOAD( "2.bin",      0x10000, 0x10000, CRC(6283a444) SHA1(fb6bc100be2af9fca2e47607700aa33dba9b2d5e) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.bin",      0x000000, 0x20000, CRC(18fe29c3) SHA1(4da8bac188330d142aa55948dc236896a58d480c) )
	ROM_LOAD( "5.bin",      0x020000, 0x20000, CRC(0eff4bbf) SHA1(c1bfbab792ac4b8e26944d759f40059077326a57) )
	ROM_LOAD( "6.bin",      0x040000, 0x20000, CRC(ac351796) SHA1(23aaf79034227febe13defe86a3d5051733cfd3e) )
	ROM_LOAD( "mj-1802.9a", 0x180000, 0x80000, CRC(e6213f10) SHA1(377399e9cd20fc2055b680eb28d024824161b2ff) )

	DISK_REGION( "vhs" ) /* Video Home System tape */
	DISK_IMAGE_READONLY( "av2mj1bb", 0, NO_DUMP )
ROM_END

ROM_START( av2mj2rg )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "1.4e",       0x00000, 0x10000, CRC(2295b8df) SHA1(600b23c968dca480db59baf708ffa4b4e4fc7c81) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* sub program */
	ROM_LOAD( "3.4t",       0x00000, 0x10000, CRC(52be7b5e) SHA1(831eb33ba78e5f2eb76d803ef30157e7c4009baa) )
	ROM_LOAD( "2.4s",       0x10000, 0x10000, CRC(6283a444) SHA1(fb6bc100be2af9fca2e47607700aa33dba9b2d5e) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "4.9b",       0x000000, 0x20000, CRC(4d965b5c) SHA1(02ebaff608343e10bdf060609bedbed688386323) )
	ROM_LOAD( "5.9d",       0x020000, 0x20000, CRC(4f5bd948) SHA1(afe206be1f992f58dd58c8f19de3ff3bc6c672eb) )
	ROM_LOAD( "6.9e",       0x040000, 0x20000, CRC(1921dae4) SHA1(7cb7b45c8d93dc45e0a952a36c1ee1e7986503ac) )
	ROM_LOAD( "7.9f",       0x060000, 0x20000, CRC(fbd9d0b0) SHA1(6c3a0d9d4f574ffe7f26d03b30e7416a7ab491a3) )
	ROM_LOAD( "8.9j",       0x080000, 0x20000, CRC(637098a9) SHA1(f534308dd6300940df17b0b757a63b5861e43937) )
	ROM_LOAD( "9.9k",       0x0a0000, 0x20000, CRC(6c06ca0d) SHA1(dee7023a906ce2709c4ef5d3bd33b9798dd56777) )
	ROM_LOAD( "mj-1802.9a", 0x180000, 0x80000, CRC(e6213f10) SHA1(377399e9cd20fc2055b680eb28d024824161b2ff) )

	DISK_REGION( "vhs" ) /* Video Home System tape */
	DISK_IMAGE_READONLY( "av2mj2rg", 0, NO_DUMP )
ROM_END


GAME( 1989, galkoku,  0,        galkoku,  galkoku,  nbmj8991_state, empty_init,    ROT180, "Nichibutsu / T.R.Tec", "Mahjong Gal no Kokuhaku (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, hyouban,  galkoku,  hyouban,  hyouban,  nbmj8991_state, empty_init,    ROT180, "Nichibutsu / T.R.Tec", "Mahjong Hyouban Musume (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, galkaika, 0,        galkaika, galkaika, nbmj8991_state, init_galkaika, ROT180, "Nichibutsu / T.R.Tec", "Mahjong Gal no Kaika (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, tokyogal, 0,        tokyogal, tokyogal, nbmj8991_state, init_tokyogal, ROT180, "Nichibutsu", "Tokyo Gal Zukan (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, tokimbsj, tokyogal, tokimbsj, tokimbsj, nbmj8991_state, init_tokimbsj, ROT180, "Nichibutsu", "Tokimeki Bishoujo (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, mcontest, 0,        mcontest, mcontest, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Miss Mahjong Contest (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, uchuuai,  0,        uchuuai,  uchuuai,  nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Mahjong Uchuu yori Ai wo komete (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, triplew1, 0,        triplew1, triplew1, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Mahjong Triple Wars (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pstadium, 0,        pstadium, pstadium, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Mahjong Panic Stadium (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, triplew2, 0,        triplew2, triplew1, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Mahjong Triple Wars 2 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, ntopstar, 0,        ntopstar, ntopstar, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Mahjong Nerae! Top Star (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, mjlstory, 0,        mjlstory, mjlstory, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Mahjong Jikken Love Story (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, ladymakr, mjlstory, mjlstory, finalbny, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Lady Maker (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, vanilla,  0,        vanilla,  vanilla,  nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Mahjong Vanilla Syndrome (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, finalbny, vanilla,  finalbny, finalbny, nbmj8991_state, init_finalbny, ROT180, "Nichibutsu", "Mahjong Final Bunny (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, qmhayaku, 0,        qmhayaku, qmhayaku, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Quiz-Mahjong Hayaku Yatteyo! (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, mjgottub, 0,        mjgottub, mjgottub, nbmj8991_state, empty_init,    ROT180, "Nichibutsu", "Medal Mahjong Gottsu ee-kanji (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, av2mj1bb, 0,        av2mj1bb, av2mj1bb, nbmj8991_state, empty_init,    ROT0,   "Miki Syouji / AV Japan", "AV2Mahjong No.1 Bay Bridge no Seijo (Japan)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1991, av2mj2rg, 0,        av2mj2rg, av2mj2rg, nbmj8991_state, empty_init,    ROT0,   "Miki Syouji / AV Japan", "AV2Mahjong No.2 Rouge no Kaori (Japan)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
