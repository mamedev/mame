// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*****************************************************************************

    Irem M90/M97 system games:

    Hasamu                                                  1991 M90
    Bomberman / Atomic Punk / Dynablaster                   1992 M90
    Bomberman World / New Atomic Punk / New Dyna Blaster    1992 M99 A
    Quiz F1 1-2 Finish                                      1992 M97
    Risky Challenge / Gussun Oyoyo                          1993 M97
    Match It II / Shisensho II                              1993 M97-A


    Uses M72 sound hardware.

    Emulation by Bryan McPhail, mish@tendril.co.uk, thanks to Chris Hardy!

*****************************************************************************/

#include "emu.h"
#include "includes/m90.h"
#include "includes/iremipt.h"

#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/irem_cpu.h"
#include "machine/rstbuf.h"
#include "sound/dac.h"
#include "sound/ym2151.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"


/***************************************************************************/

void m90_state::machine_start()
{
}

/***************************************************************************/

WRITE16_MEMBER(m90_state::coincounter_w)
{
	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x01);
		machine().bookkeeping().coin_counter_w(1, data & 0x02);

		if (data & 0xfc) logerror("Coin counter %02x\n",data);
	}
}

WRITE16_MEMBER(m90_state::quizf1_bankswitch_w)
{
	if (ACCESSING_BITS_0_7)
		m_mainbank->set_entry(data & 0xf);
}

#ifdef UNUSED_FUNCTION
WRITE16_MEMBER(m90_state::unknown_w)
{
	printf("%04x    ",data);
}
#endif

/***************************************************************************/

void m90_state::m90_main_cpu_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
	map(0xa0000, 0xa3fff).ram();
	map(0xd0000, 0xdffff).ram().w(FUNC(m90_state::m90_video_w)).share("video_data");
	map(0xe0000, 0xe03ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xffff0, 0xfffff).rom();
}

void m90_state::quizf1_main_cpu_map(address_map &map)
{
	m90_main_cpu_map(map);
	map(0x80000, 0x8ffff).bankr("mainbank");  /* Quiz F1 only */
}

void m90_state::dynablsb_main_cpu_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom();
	map(0x6000e, 0x60fff).ram().share("spriteram");
	map(0xa0000, 0xa3fff).ram();
	map(0xd0000, 0xdffff).ram().w(FUNC(m90_state::bootleg_video_w)).share("video_data");
	map(0xe0000, 0xe03ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xffff0, 0xfffff).rom();
}

void m90_state::bomblord_main_cpu_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
	map(0xa0000, 0xa3fff).ram();
	map(0xc000e, 0xc0fff).ram().share("spriteram");
	map(0xd0000, 0xdffff).ram().w(FUNC(m90_state::bootleg_video_w)).share("video_data");
	map(0xe0000, 0xe03ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xffff0, 0xfffff).rom();
}

void m90_state::m90_main_cpu_io_map(address_map &map)
{
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x00, 0x01).portr("P1_P2");
	map(0x02, 0x03).w(FUNC(m90_state::coincounter_w));
	map(0x02, 0x03).portr("SYSTEM");
	map(0x04, 0x05).portr("DSW");
	map(0x06, 0x07).portr("P3_P4");
	map(0x80, 0x8f).writeonly().share("video_control");
}

void m90_state::quizf1_main_cpu_io_map(address_map &map)
{
	m90_main_cpu_io_map(map);
	map(0x04, 0x05).w(FUNC(m90_state::quizf1_bankswitch_w));
}

void m90_state::dynablsb_main_cpu_io_map(address_map &map)
{
	map(0x00, 0x00).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x00, 0x01).portr("P1_P2");
	map(0x02, 0x03).w(FUNC(m90_state::coincounter_w));
	map(0x02, 0x03).portr("SYSTEM");
//  AM_RANGE(0x04, 0x05) AM_WRITE(unknown_w)      /* dynablsb: write continuously 0x6000 */
	map(0x04, 0x05).portr("DSW");
	map(0x06, 0x07).portr("P3_P4");
	map(0x80, 0x8f).writeonly().share("video_control");
//  AM_RANGE(0x90, 0x91) AM_WRITE(unknown_w)
}

/*****************************************************************************/

void m90_state::m90_sound_cpu_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}

void m90_state::m90_sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).w(m_audio, FUNC(m72_audio_device::rtype2_sample_addr_w));
	map(0x82, 0x82).w(m_audio, FUNC(m72_audio_device::sample_w));
	map(0x83, 0x83).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
	map(0x84, 0x84).r(m_audio, FUNC(m72_audio_device::sample_r));
}

void m90_state::dynablsb_sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x82, 0x82).w("dac", FUNC(dac_byte_interface::data_w));
}

void m90_state::m99_sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w(m_audio, FUNC(m72_audio_device::poundfor_sample_addr_w));
	map(0x40, 0x41).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x42, 0x42).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
}

/*****************************************************************************/


static INPUT_PORTS_START( hasamu )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_2_BUTTONS(1, 2)

	PORT_START("SYSTEM")
	IREM_COINS

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4") /* unused */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( dynablst )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_2_BUTTONS_4WAY(1, 2)

	PORT_START("SYSTEM")
	IREM_COINS

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Game Title" )    PORT_DIPLOCATION("SW1:5") /* Manual says "NOT USE" */
	PORT_DIPSETTING(      0x0010, "Dynablaster" )
	PORT_DIPSETTING(      0x0000, "Bomber Man" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, "2 Player Upright" )
	PORT_DIPSETTING(      0x0600, "4 Player Upright A" ) /* Separate Coin Slots */
	PORT_DIPSETTING(      0x0200, "4 Player Upright B" ) /* Shared Coin Slots */
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )  /* This setting shows screen with offset, no cocktail support :-( */
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4")
	IREM_INPUT_PLAYER_3_4WAY
	IREM_INPUT_PLAYER_4_4WAY
INPUT_PORTS_END

static INPUT_PORTS_START( dynablsb )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_2_BUTTONS_4WAY(2, 1)

	PORT_START("SYSTEM")
	IREM_COINS

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Game Title" )    PORT_DIPLOCATION("SW1:5") /* Manual says "NOT USE" */
	PORT_DIPSETTING(      0x0010, "Dynablaster" )
	PORT_DIPSETTING(      0x0000, "Bomber Man" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, "2 Player Upright" )
	PORT_DIPSETTING(      0x0600, "4 Player Upright A" ) /* Separate Coin Slots */
	PORT_DIPSETTING(      0x0200, "4 Player Upright B" ) /* Shared Coin Slots */
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )  /* This setting shows screen with offset, no cocktail support :-( */
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4")
	IREM_INPUT_PLAYER_3_4WAY
	IREM_INPUT_PLAYER_4_4WAY
INPUT_PORTS_END


static INPUT_PORTS_START( atompunk )
	PORT_INCLUDE(dynablst)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5") /* Manual says "NOT USE" */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bombrman ) /* Does not appear to support 4 players or cocktail mode */
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_2_BUTTONS_4WAY(1, 2)

	PORT_START("SYSTEM")
	IREM_COINS

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:5" )   /* Manual says "NOT USE" */
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0200, 0x0200, "SW2:2" )   /* Manual says "NOT USE" */
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )   /* Manual says "NOT USE" */
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4") /* unused */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( bbmanw )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_2_BUTTONS_4WAY(1, 2)

	PORT_START("SYSTEM")
	IREM_COINS

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Game Title" )    PORT_DIPLOCATION("SW1:5") /* Manual says "NOT USE" */
	PORT_DIPSETTING(      0x0010, "Bomber Man World" )
	PORT_DIPSETTING(      0x0000, "New Dyna Blaster Global Quest" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC ( 0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0400, "2 Player" )
	PORT_DIPSETTING(      0x0600, "4 Player Separate Coins" )       /* Each player has a separate Coin Slot */
	PORT_DIPSETTING(      0x0200, "4 Player Shared Coins" )     /* All 4 players Share coin 1&2 */
	PORT_DIPSETTING(      0x0000, "4 Player 1&2 3&4 Share Coins" )  /* Players 1&2 share coin 1&2, Players 3&4 share coin 3&4 */
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4")
	IREM_INPUT_PLAYER_3_4WAY
	IREM_INPUT_PLAYER_4_4WAY
INPUT_PORTS_END

static INPUT_PORTS_START( bbmanwj )
	PORT_INCLUDE(bbmanw)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easiest ) ) // Errata corrige sheet for the Japanese version shows "Very Easy" instead of "Very Hard"
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5") /* Manual says "NOT USE" - No Game Title Change */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0200, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,3") /* Cabinet type switched around */
	PORT_DIPSETTING(      0x0600, "2 Player" )
	PORT_DIPSETTING(      0x0200, "4 Player Separate Coins" )       /* Each player has a separate Coin Slot */
	PORT_DIPSETTING(      0x0400, "4 Player Shared Coins" )     /* All 4 players Share coin 1&2 */
	PORT_DIPSETTING(      0x0000, "4 Player 1&2 3&4 Share Coins" )  /* Players 1&2 share coin 1&2, Players 3&4 share coin 3&4 */
INPUT_PORTS_END

static INPUT_PORTS_START( newapunk )
	PORT_INCLUDE(bbmanw)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5") /* Manual says "NOT USE" - No Game Title Change */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( quizf1 )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_4_BUTTONS(1, 2)

	PORT_START("SYSTEM")
	IREM_COINS

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3") /* Probably difficulty */
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Joystick ) ) /* for quiz cabinets with buttons connected to the JAMMA "joystick" pins */
	PORT_DIPSETTING(      0x0000, "Buttons" )           /* for non-quiz cabinet conversions */
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4") /* unused */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( matchit2 )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_3_BUTTONS(1, 2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Girls Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "China Tiles" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, "Mahjong" )
	PORT_DIPSETTING(      0x0000, "Alpha-Numeric" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Timer Speed" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy )  )
	PORT_DIPNAME( 0x0040, 0x0040, "Title Screen" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, "Match It II" )
	PORT_DIPSETTING(      0x0000, "Shisensho II" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(      0x0600, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( German ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Korean ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4") /* unused */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( shisen2 )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_3_BUTTONS(1, 2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Girls Mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Timer Speed" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy )  )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4") /* unused */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( riskchal )
	PORT_START("P1_P2")
	IREM_GENERIC_JOYSTICKS_2_BUTTONS(1, 2)

	PORT_START("SYSTEM")
	IREM_COINS

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5") /* Manual says "NOT USE" */
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6") /* Manual says "NOT USE" */
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2") /* Manual says "NOT USE" */
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Coin Slots" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "Common" )
	PORT_DIPSETTING(      0x0000, "Separate" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	/* Coin Mode 1 */
	IREM_COIN_MODE_1_NEW_HIGH
	/* Coin Mode 2 */
	IREM_COIN_MODE_2_HIGH

	PORT_START("P3_P4") /* unused */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*****************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static GFXDECODE_START( gfx_m90 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 256, 16 )
GFXDECODE_END

/*****************************************************************************/

INTERRUPT_GEN_MEMBER(m90_state::fake_nmi)
{
	int sample = m_audio->sample_r();
	if (sample)
		m_audio->sample_w(sample);
}

INTERRUPT_GEN_MEMBER(m90_state::bomblord_fake_nmi)
{
	int sample = m_audio->sample_r();
	if (sample != 0x80)
		m_audio->sample_w(sample);
}

WRITE_LINE_MEMBER(m90_state::dynablsb_vblank_int_w)
{
	if (state)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x60/4); // V30
}

WRITE_LINE_MEMBER(m90_state::bomblord_vblank_int_w)
{
	if (state)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x50/4); // V30
}


/* Basic hardware -- no decryption table is setup for CPU */
void m90_state::m90(machine_config &config)
{
	/* basic machine hardware */
	V35(config, m_maincpu, XTAL(32'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &m90_state::m90_main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &m90_state::m90_main_cpu_io_map);

	Z80(config, m_soundcpu, XTAL(3'579'545)); /* verified on pcb */
	m_soundcpu->set_addrmap(AS_PROGRAM, &m90_state::m90_sound_cpu_map);
	m_soundcpu->set_addrmap(AS_IO, &m90_state::m90_sound_cpu_io_map);
	m_soundcpu->set_periodic_int(FUNC(m90_state::nmi_line_pulse), attotime::from_hz(128*60)); /* clocked by V1? (Vigilante) */
	/* IRQs are generated by main Z80 and YM2151 */
	m_soundcpu->set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 64*8);
	m_screen->set_visarea(6*8, 54*8-1, 17*8, 47*8-1);
	m_screen->set_screen_update(FUNC(m90_state::screen_update_m90));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline("maincpu", NEC_INPUT_LINE_INTP0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_m90);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 512);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq", 0).int_callback().set_inputline(m_soundcpu, 0);

	IREM_M72_AUDIO(config, m_audio);
	m_audio->set_dac_tag("dac");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(3'579'545))); /* verified on pcb */
	ymsnd.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ymsnd.add_route(0, "speaker", 0.15);
	ymsnd.add_route(1, "speaker", 0.15);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref", 0));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

void m90_state::hasamu(machine_config &config)
{
	m90(config);
	subdevice<v35_device>("maincpu")->set_decryption_table(gunforce_decryption_table);
}

void m90_state::quizf1(machine_config &config)
{
	m90(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &m90_state::quizf1_main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &m90_state::quizf1_main_cpu_io_map);
	subdevice<v35_device>("maincpu")->set_decryption_table(lethalth_decryption_table);

	m_screen->set_visarea(6*8, 54*8-1, 17*8-8, 47*8-1+8);
}

void m90_state::matchit2(machine_config &config)
{
	m90(config);
	subdevice<v35_device>("maincpu")->set_decryption_table(matchit2_decryption_table);
	m_screen->set_visarea(6*8, 54*8-1, 17*8-8, 47*8-1+8);
}

void m90_state::riskchal(machine_config &config)
{
	m90(config);
	subdevice<v35_device>("maincpu")->set_decryption_table(gussun_decryption_table);
	m_screen->set_visarea(10*8, 50*8-1, 17*8, 47*8-1);
}

void m90_state::bombrman(machine_config &config)
{
	m90(config);
	subdevice<v35_device>("maincpu")->set_decryption_table(bomberman_decryption_table);
	m_screen->set_visarea(10*8, 50*8-1, 17*8, 47*8-1);
}

void m90_state::bbmanwj(machine_config &config)
{
	m90(config);
	subdevice<v35_device>("maincpu")->set_decryption_table(dynablaster_decryption_table);
	m_screen->set_visarea(10*8, 50*8-1, 17*8, 47*8-1);
}

void m90_state::bbmanw(machine_config &config)
{
	bbmanwj(config);
	m_soundcpu->set_addrmap(AS_IO, &m90_state::m99_sound_cpu_io_map);
	m_soundcpu->set_periodic_int(FUNC(m90_state::fake_nmi), attotime::from_hz(128*60));
}

void m90_state::bomblord(machine_config &config)
{
	m90(config);
	V30(config.replace(), m_maincpu, 32000000/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &m90_state::bomblord_main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &m90_state::m90_main_cpu_io_map);

	m_soundcpu->set_addrmap(AS_IO, &m90_state::m99_sound_cpu_io_map);
	m_soundcpu->set_periodic_int(FUNC(m90_state::bomblord_fake_nmi), attotime::from_hz(128*60));

	m_screen->set_visarea(10*8, 50*8-1, 17*8, 47*8-1);
	m_screen->set_screen_update(FUNC(m90_state::screen_update_bomblord));
	m_screen->screen_vblank().set(FUNC(m90_state::bomblord_vblank_int_w));

	MCFG_VIDEO_START_OVERRIDE(m90_state,bomblord)
}

void m90_state::dynablsb(machine_config &config)
{
	m90(config);
	V30(config.replace(), m_maincpu, 32000000/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &m90_state::dynablsb_main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &m90_state::dynablsb_main_cpu_io_map);

	m_soundcpu->set_addrmap(AS_IO, &m90_state::dynablsb_sound_cpu_io_map);
	m_soundcpu->set_periodic_int(FUNC(m90_state::irq0_line_hold), attotime::from_hz(64*60)); /* half the sample rate of the original */
	m_soundcpu->set_irq_acknowledge_callback(device_irq_acknowledge_delegate());

	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(m90_state::screen_update_dynablsb));
	m_screen->screen_vblank().set(FUNC(m90_state::dynablsb_vblank_int_w));

	MCFG_VIDEO_START_OVERRIDE(m90_state,dynablsb)

	config.device_remove("m72");
	config.device_remove("soundirq");

	auto &soundlatch(*subdevice<generic_latch_8_device>("soundlatch"));
	soundlatch.data_pending_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);
	soundlatch.set_separate_acknowledge(false);

	subdevice<ym2151_device>("ymsnd")->irq_handler().set_nop(); /* this bootleg polls the YM2151 instead of taking interrupts from it */
}


/***************************************************************************/

#define CODE_SIZE 0x100000

ROM_START( hasamu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hasc-p1.bin",  0x00001, 0x20000, CRC(53df9834) SHA1(2e7e38157a497e3def69c4abcae5803f71a098da) )
	ROM_LOAD16_BYTE( "hasc-p0.bin",  0x00000, 0x20000, CRC(dff0ba6e) SHA1(83e20b3ae10b57c1e58d3d44bfca2ffd5f142056) )
	ROM_COPY( "maincpu", 0x3fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "hasc-sp.bin",    0x0000, 0x10000, CRC(259b1687) SHA1(39c3a89b1d0f5fec2a94a3048cc4639fe96820e2) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "hasc-c0.bin",    0x000000, 0x20000, CRC(dd5a2174) SHA1(c28499419f961d126a838dd1390db74c1475ee02) )
	ROM_LOAD( "hasc-c1.bin",    0x020000, 0x20000, CRC(76b8217c) SHA1(8b21562875d856a1ce4863f325d049090f5716ae) )
	ROM_LOAD( "hasc-c2.bin",    0x040000, 0x20000, CRC(d90f9a68) SHA1(c9eab3e87dd5d3eb88461be493d88f5482c9e257) )
	ROM_LOAD( "hasc-c3.bin",    0x060000, 0x20000, CRC(6cfe0d39) SHA1(104feeacbbc86b168c41cd37fc5797781d9b5a0f) )

	ROM_REGION( 0x20000, "m72", ROMREGION_ERASE00 ) /* samples */
	/* No samples */
ROM_END

ROM_START( dynablst )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbm-cp1e.62",   0x00001, 0x20000, CRC(27667681) SHA1(7d5f762026ea01817a65ea13b4b5793640e3e8fd) )
	ROM_LOAD16_BYTE( "bbm-cp0e.65",   0x00000, 0x20000, CRC(95db7a67) SHA1(1a224d73615a60530cbcc54fdbb526e8d5a6c555) )
	ROM_COPY( "maincpu", 0x3fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bbm-sp.23",    0x0000, 0x10000, CRC(251090cd) SHA1(9245072c1afbfa3e4a1d1549942765d58bd78ed3) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "bbm-c0.66",    0x000000, 0x40000, CRC(695d2019) SHA1(3537e9fb0e7dc13d6113b4af71cba3c73392335a) )
	ROM_LOAD( "bbm-c1.67",    0x040000, 0x40000, CRC(4c7c8bbc) SHA1(31ab5557d96c4184a9c02ed1c309f3070d148e25) )
	ROM_LOAD( "bbm-c2.68",    0x080000, 0x40000, CRC(0700d406) SHA1(0d43a31a726b0de0004beef41307de2508106b69) )
	ROM_LOAD( "bbm-c3.69",    0x0c0000, 0x40000, CRC(3c3613af) SHA1(f9554a73e95102333e449f6e81f2bb817ec00881) )

	ROM_REGION( 0x20000, "m72", 0 ) /* samples */
	ROM_LOAD( "bbm-v0.20",    0x0000, 0x20000, CRC(0fa803fe) SHA1(d2ac1e624de38bed385442ceae09a76f203fa084) )
ROM_END

ROM_START( bombrman )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbm-p1.62",   0x00001, 0x20000, CRC(982bd166) SHA1(ed67393ec319127616bff5fa3b7f84e8ac8e1d93) )
	ROM_LOAD16_BYTE( "bbm-p0.65",   0x00000, 0x20000, CRC(0a20afcc) SHA1(a42b7458938300b0c84c820c1ea627aed9080f1b) )
	ROM_COPY( "maincpu", 0x3fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bbm-sp.23",    0x0000, 0x10000, CRC(251090cd) SHA1(9245072c1afbfa3e4a1d1549942765d58bd78ed3) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "bbm-c0.66",    0x000000, 0x40000, CRC(695d2019) SHA1(3537e9fb0e7dc13d6113b4af71cba3c73392335a) )
	ROM_LOAD( "bbm-c1.67",    0x040000, 0x40000, CRC(4c7c8bbc) SHA1(31ab5557d96c4184a9c02ed1c309f3070d148e25) )
	ROM_LOAD( "bbm-c2.68",    0x080000, 0x40000, CRC(0700d406) SHA1(0d43a31a726b0de0004beef41307de2508106b69) )
	ROM_LOAD( "bbm-c3.69",    0x0c0000, 0x40000, CRC(3c3613af) SHA1(f9554a73e95102333e449f6e81f2bb817ec00881) )

	ROM_REGION( 0x20000, "m72", 0 ) /* samples */
	ROM_LOAD( "bbm-v0.20",    0x0000, 0x20000, CRC(0fa803fe) SHA1(d2ac1e624de38bed385442ceae09a76f203fa084) )
ROM_END

ROM_START( atompunk )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbm-cp0d.65",   0x00001, 0x20000, CRC(860c0479) SHA1(7556d62955d0d7a7100fbd9d9cb7356b96a4df78) )
	ROM_LOAD16_BYTE( "bbm-cp1d.62",   0x00000, 0x20000, CRC(be57bf74) SHA1(cd3f887f7ec8a5721551477ec2d4a7336f422c6f) )
	ROM_COPY( "maincpu", 0x3fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bbm-sp.23",    0x0000, 0x10000, CRC(251090cd) SHA1(9245072c1afbfa3e4a1d1549942765d58bd78ed3) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "bbm-c0.66",    0x000000, 0x40000, CRC(695d2019) SHA1(3537e9fb0e7dc13d6113b4af71cba3c73392335a) ) /* Labeled as 9134HD004 */
	ROM_LOAD( "bbm-c1.67",    0x040000, 0x40000, CRC(4c7c8bbc) SHA1(31ab5557d96c4184a9c02ed1c309f3070d148e25) ) /* Labeled as 9134HD001 */
	ROM_LOAD( "bbm-c2.68",    0x080000, 0x40000, CRC(0700d406) SHA1(0d43a31a726b0de0004beef41307de2508106b69) ) /* Labeled as 9134HD002 */
	ROM_LOAD( "bbm-c3.69",    0x0c0000, 0x40000, CRC(3c3613af) SHA1(f9554a73e95102333e449f6e81f2bb817ec00881) ) /* Labeled as 9134HD003 */

	ROM_REGION( 0x20000, "m72", 0 ) /* samples */
	ROM_LOAD( "bbm-v0.20",    0x0000, 0x20000, CRC(0fa803fe) SHA1(d2ac1e624de38bed385442ceae09a76f203fa084) ) /* Labeled as 9132E9001 */
ROM_END

ROM_START( dynablstb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "db2-26.bin",   0x00001, 0x20000, CRC(a78c72f8) SHA1(e3ed1bce0278bada6357b5d0823511fa0241f3cd) )
	ROM_LOAD16_BYTE( "db3-25.bin",   0x00000, 0x20000, CRC(bf3137c3) SHA1(64bbca4b3a509b552ee8a19b3b50fe6638fd90e2) )
	ROM_COPY( "maincpu", 0x3fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "db1-17.bin",    0x0000, 0x10000, CRC(e693c32f) SHA1(b6f228d26318718eedae765de9479706a3e4c38d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "bbm-c0.66",    0x000000, 0x40000, CRC(695d2019) SHA1(3537e9fb0e7dc13d6113b4af71cba3c73392335a) )
	ROM_LOAD( "bbm-c1.67",    0x040000, 0x40000, CRC(4c7c8bbc) SHA1(31ab5557d96c4184a9c02ed1c309f3070d148e25) )
	ROM_LOAD( "bbm-c2.68",    0x080000, 0x40000, CRC(0700d406) SHA1(0d43a31a726b0de0004beef41307de2508106b69) )
	ROM_LOAD( "bbm-c3.69",    0x0c0000, 0x40000, CRC(3c3613af) SHA1(f9554a73e95102333e449f6e81f2bb817ec00881) )

	ROM_REGION( 0x20000, "m72", ROMREGION_ERASE00 ) /* samples */
	/* the samples are in the Z80 ROM in this bootleg */
ROM_END

ROM_START( dynablstb2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.bin", 0x000001, 0x020000, CRC(9c3e996a) SHA1(9244f6a8b498d0f488bbfc68465916ea0560f3be) )
	ROM_LOAD16_BYTE( "3.bin", 0x000000, 0x020000, CRC(697a2ed9) SHA1(d329987f7a0321414347b8e6a6dfc4304fbe4987) )
	ROM_COPY( "maincpu", 0x3fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "db1-17.bin",    0x0000, 0x10000, CRC(e693c32f) SHA1(b6f228d26318718eedae765de9479706a3e4c38d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "bbm-c0.66",    0x000000, 0x40000, CRC(695d2019) SHA1(3537e9fb0e7dc13d6113b4af71cba3c73392335a) )
	ROM_LOAD( "bbm-c1.67",    0x040000, 0x40000, CRC(4c7c8bbc) SHA1(31ab5557d96c4184a9c02ed1c309f3070d148e25) )
	ROM_LOAD( "bbm-c2.68",    0x080000, 0x40000, CRC(0700d406) SHA1(0d43a31a726b0de0004beef41307de2508106b69) )
	ROM_LOAD( "bbm-c3.69",    0x0c0000, 0x40000, CRC(3c3613af) SHA1(f9554a73e95102333e449f6e81f2bb817ec00881) )

	ROM_REGION( 0x20000, "m72", ROMREGION_ERASE00 ) /* samples */
	/* the samples are in the Z80 ROM in this bootleg */
ROM_END

/*Roms 2 and 3 (code) are different

XTALS:

16mhz (x1)
&
30mhz (x2)

The ROMs came with Playmark labels but the copyright is still Seitu.
*/

ROM_START( dynablstb3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.ic26", 0x000001, 0x020000, CRC(08b1573e) SHA1(ef39177792b3263027e6eb88f61e761976e770ef) )
	ROM_LOAD16_BYTE( "3.ic25", 0x000000, 0x020000, CRC(bbf540ad) SHA1(8bec4e8da9cb2e7b9e22894dcc0e078ced028174) )
	ROM_COPY( "maincpu", 0x3fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "1.ic17",    0x0000, 0x10000, CRC(e693c32f) SHA1(b6f228d26318718eedae765de9479706a3e4c38d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "7.ic104",    0x000000, 0x40000, CRC(695d2019) SHA1(3537e9fb0e7dc13d6113b4af71cba3c73392335a) )
	ROM_LOAD( "6.ic105",    0x040000, 0x40000, CRC(4c7c8bbc) SHA1(31ab5557d96c4184a9c02ed1c309f3070d148e25) )
	ROM_LOAD( "5.ic108",    0x080000, 0x40000, CRC(0700d406) SHA1(0d43a31a726b0de0004beef41307de2508106b69) )
	ROM_LOAD( "4.ic107",    0x0c0000, 0x40000, CRC(3c3613af) SHA1(f9554a73e95102333e449f6e81f2bb817ec00881) )

	ROM_REGION( 0x20000, "m72", ROMREGION_ERASE00 ) /* samples */
	/* the samples are in the Z80 ROM in this bootleg */
ROM_END

/*
New Dyna Blaster Global Quest
Irem, 1992

PCB Layout
----------

M99-A-A  05C04369A1 MADE IN JAPAN
License Sticker - 'BOMBER MAN WORLD NEW DYNA BLASTER'
|--------------------------------------------------|
|                  |----------|                    |
|        YM3014    |IREM      |                    |
|VOL    3.579545MHZ|D9000001A1|  PAL2     4364     |
|        LM358     |----------|                    |
|                   BBM2-V0-              4364     |
|                            |----------|          |
|                   4364     |NANAO     | BBM2-H0-B|
|J                     M51953|08J27291A5|          |
|A                  BBM2-SP- |015       |          |
|M               D70008AC-6  |          | BBM2-L0-B|
|M                           |----------|          |
|A                 6116     32MHz         BBM2-C0- |
|                                                  |
|    DSW1          6116  26.6666MHz       BBM2-C1- |
|                                                  |
|        PAL1       43256   |-------|     BBM2-C2- |
|                           | NANAO |              |
|    DSW2           43256   | GA25  |     BBM2-C3- |
|                           |-------|              |
|--------------------------------------------------|
Notes:
      D70008AC-6  : NEC D70008AC-6 Z80 compatible CPU, clock 3.579545MHz
      YM2151 clock: 3.579545MHz
      PAL1        : PAL16L8 labelled 'M99 A-4S-'
      PAL2        : PAL16L8 labelled 'M99 A-8C-'
      HSync       : 15.42kHz
      VSync       : 60Hz
*/

ROM_START( bbmanw )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbm2-h0-b.77",  0x00001, 0x40000, CRC(567d3709) SHA1(1447fc68798589a8757ee2d133d053b80f052113) )
	ROM_LOAD16_BYTE( "bbm2-l0-b.79",  0x00000, 0x40000, CRC(e762c22b) SHA1(b389a65adf1348e6529a992d9b68178d7503238e) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bbm2-sp.33",    0x0000, 0x10000, CRC(6bc1689e) SHA1(099c275632965e19eb6131863f69d2afa9916e90) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "bbm2-c0.81",  0x000000, 0x40000, CRC(e7ce058a) SHA1(f2336718ecbce4771f27abcdc4d28fe91c702a9e) )
	ROM_LOAD( "bbm2-c1.82",  0x080000, 0x40000, CRC(636a78a9) SHA1(98562ea056e5bd36c1a094ae6f267367236d166f) )
	ROM_LOAD( "bbm2-c2.83",  0x100000, 0x40000, CRC(9ac2142f) SHA1(744fe1acae2fcba0051c303b644081546b4aed9e) )
	ROM_LOAD( "bbm2-c3.84",  0x180000, 0x40000, CRC(47af1750) SHA1(dce176a6ca95852208b6eba7fb88a0d96467c34b) )

	ROM_REGION( 0x20000, "m72", 0 )
	ROM_LOAD( "bbm2-v0.30",    0x0000, 0x20000, CRC(4ad889ed) SHA1(b685892a2348f17f89c6d6ce91216f6cf1e33751) )
ROM_END

ROM_START( bbmanwj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbm2-h0.77",  0x00001, 0x40000, CRC(e1407b91) SHA1(6c94afc6b1d2a469295890ee5dd9d9d5a02ae5c4) )
	ROM_LOAD16_BYTE( "bbm2-l0.79",  0x00000, 0x40000, CRC(20873b49) SHA1(30ae595f7961cd56f2506608ae76973b2d0e73ca) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bbm2-sp-a.33", 0x0000, 0x10000, CRC(a4b0a66e) SHA1(834c809089c7ad424796f3fd0cc6747601af9fb6) ) /* Newer sound CPU code, same sound hardware as bbnanw / newapunk */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "bbm2-c0.81",  0x000000, 0x40000, CRC(e7ce058a) SHA1(f2336718ecbce4771f27abcdc4d28fe91c702a9e) )
	ROM_LOAD( "bbm2-c1.82",  0x080000, 0x40000, CRC(636a78a9) SHA1(98562ea056e5bd36c1a094ae6f267367236d166f) )
	ROM_LOAD( "bbm2-c2.83",  0x100000, 0x40000, CRC(9ac2142f) SHA1(744fe1acae2fcba0051c303b644081546b4aed9e) )
	ROM_LOAD( "bbm2-c3.84",  0x180000, 0x40000, CRC(47af1750) SHA1(dce176a6ca95852208b6eba7fb88a0d96467c34b) )

	ROM_REGION( 0x20000, "m72", 0 ) /* samples */
	ROM_LOAD( "bbm2-v0-b.30",  0x0000, 0x20000, CRC(0ae655ff) SHA1(78752182662fd8f5b55bbbc2787c9f2b04096ea1) )
ROM_END

ROM_START( bbmanwja )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbm2-h0.77",  0x00001, 0x40000, CRC(e1407b91) SHA1(6c94afc6b1d2a469295890ee5dd9d9d5a02ae5c4) )
	ROM_LOAD16_BYTE( "bbm2-l0.79",  0x00000, 0x40000, CRC(20873b49) SHA1(30ae595f7961cd56f2506608ae76973b2d0e73ca) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bbm2-sp-b.33", 0x0000, 0x10000, CRC(b8d8108c) SHA1(ef4fb46d843819c273db2083754eb312f5abd44e) ) /* Differently mapped sound hardware */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "bbm2-c0.81",  0x000000, 0x40000, CRC(e7ce058a) SHA1(f2336718ecbce4771f27abcdc4d28fe91c702a9e) )
	ROM_LOAD( "bbm2-c1.82",  0x080000, 0x40000, CRC(636a78a9) SHA1(98562ea056e5bd36c1a094ae6f267367236d166f) )
	ROM_LOAD( "bbm2-c2.83",  0x100000, 0x40000, CRC(9ac2142f) SHA1(744fe1acae2fcba0051c303b644081546b4aed9e) )
	ROM_LOAD( "bbm2-c3.84",  0x180000, 0x40000, CRC(47af1750) SHA1(dce176a6ca95852208b6eba7fb88a0d96467c34b) )

	ROM_REGION( 0x20000, "m72", 0 ) /* samples */
	ROM_LOAD( "bbm2-v0-b.30",  0x0000, 0x20000, CRC(0ae655ff) SHA1(78752182662fd8f5b55bbbc2787c9f2b04096ea1) )
ROM_END

ROM_START( newapunk )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bbm2-h0-a.77",  0x00001, 0x40000, CRC(7d858682) SHA1(03580e2903becb69766023585c6ecffbb8e0b9c5) )
	ROM_LOAD16_BYTE( "bbm2-l0-a.79",  0x00000, 0x40000, CRC(c7568031) SHA1(ff4d0809260a088f530098a0173eec16fa6396f1) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bbm2-sp.33",    0x0000, 0x10000, CRC(6bc1689e) SHA1(099c275632965e19eb6131863f69d2afa9916e90) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "bbm2-c0.81",  0x000000, 0x40000, CRC(e7ce058a) SHA1(f2336718ecbce4771f27abcdc4d28fe91c702a9e) )
	ROM_LOAD( "bbm2-c1.82",  0x080000, 0x40000, CRC(636a78a9) SHA1(98562ea056e5bd36c1a094ae6f267367236d166f) )
	ROM_LOAD( "bbm2-c2.83",  0x100000, 0x40000, CRC(9ac2142f) SHA1(744fe1acae2fcba0051c303b644081546b4aed9e) )
	ROM_LOAD( "bbm2-c3.84",  0x180000, 0x40000, CRC(47af1750) SHA1(dce176a6ca95852208b6eba7fb88a0d96467c34b) )

	ROM_REGION( 0x20000, "m72", 0 ) /* samples */
	ROM_LOAD( "bbm2-v0.30",    0x0000, 0x20000, CRC(4ad889ed) SHA1(b685892a2348f17f89c6d6ce91216f6cf1e33751) )
ROM_END

//PCB is marked: "BOMBER LORD 030" and "lc" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
//PCB is marked: "BOMBER LORD 030" and "ls" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
ROM_START( bomblord )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "27c020_3.u6",  0x00001, 0x40000, CRC(65d5c54a) SHA1(f794a193d5927b5fb838ab2351c176d8cbd37236) )
	ROM_LOAD16_BYTE( "27c020_4.u5",  0x00000, 0x40000, CRC(cfe65f81) SHA1(8dae94abc67bc53f1c8dbe13243dc08a62fd5d22) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "27c512_1.u100",    0x0000, 0x10000, CRC(6bc1689e) SHA1(099c275632965e19eb6131863f69d2afa9916e90) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "27c020_5.u69",  0x000000, 0x40000, CRC(3ded3278) SHA1(2fec2f10d875e44d966b6f652e3b09308db9b343) )
	ROM_LOAD( "27c020_6.u70",  0x080000, 0x40000, CRC(1c489632) SHA1(f4412b138e4933c8d152ec51d05baa02abc7fc00) )
	ROM_LOAD( "27c020_7.u71",  0x100000, 0x40000, CRC(cc2b6237) SHA1(8450edac25817afdc393e2e6960fec4da382d2c9) )
	// previous 7.u71 dump had a single different byte at 0x86eb: 0x6f instead of 0x2f. Hashes left as a remainder:
	// ROM_LOAD( "bomblord.7",  0x100000, 0x40000, CRC(68935e94) SHA1(6725c7ad49bd0ee6ed1db22193852a11cdf95aaa) )
	ROM_LOAD( "27c020_8.u72",  0x180000, 0x40000, CRC(6a423b24) SHA1(d30dac90a7dc2a616714eae7450ae0edef566c31) )

	ROM_REGION( 0x20000, "m72", 0 )
	ROM_LOAD( "27c010_2.u104",    0x0000, 0x20000, CRC(37d356bd) SHA1(15f187954f94e2b1a4757e4a27ab7be9598972ff) )

	ROM_REGION( 0x700, "plds", ROMREGION_ERASE00 ) // all read protected
	ROM_LOAD( "palce16v8h.u45",   0x000, 0x117, NO_DUMP )
	ROM_LOAD( "palce16v8h.u119",  0x200, 0x117, NO_DUMP )
	ROM_LOAD( "palce22v10h.u105", 0x400, 0x2dd, NO_DUMP )
ROM_END

ROM_START( quizf1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "qf1-h0-.77",   0x000001, 0x40000, CRC(280e3049) SHA1(3b1f303d803f844fd260ed93e4d12a72876e4dbe) )
	ROM_LOAD16_BYTE( "qf1-l0-.79",   0x000000, 0x40000, CRC(94588a6f) SHA1(ee912739c7719fc2b099da0c63f7473eedcfc718) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION16_LE( 0x100000, "user1", 0 ) /* banked at 80000-8FFFF */
	ROM_LOAD16_BYTE( "qf1-h1-.78",   0x000001, 0x80000, CRC(c6c2eb2b) SHA1(83de08b0c72da8c3e4786063802d83cb1015032a) )
	ROM_LOAD16_BYTE( "qf1-l1-.80",   0x000000, 0x80000, CRC(3132c144) SHA1(de3ae35cdfbb1231cab343142ac700df00f9b77a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "qf1-sp-.33",   0x0000, 0x10000, CRC(0664fa9f) SHA1(db003beb4f8461bf4411efa8df9f700770fb153b) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "qf1-c0-.81",   0x000000, 0x80000, CRC(c26b521e) SHA1(eb5d33a21d1f82e361e0c0945abcf42562c32f03) )
	ROM_LOAD( "qf1-c1-.82",   0x080000, 0x80000, CRC(db9d7394) SHA1(06b41288c41df8ae0cafb53e77b519d0419cf1d9) )
	ROM_LOAD( "qf1-c2-.83",   0x100000, 0x80000, CRC(0b1460ae) SHA1(c6394e6bb2a4e3722c20d9f291cb6ba7aad5766d) )
	ROM_LOAD( "qf1-c3-.84",   0x180000, 0x80000, CRC(2d32ff37) SHA1(f414f6bad1ffc4396fd757155e602bdefdc99408) )

	ROM_REGION( 0x40000, "m72", 0 ) /* samples */
	ROM_LOAD( "qf1-v0-.30",   0x0000, 0x40000, CRC(b8d16e7c) SHA1(28a20afb171dc68848f9fe793f53571d4c7502dd) )
ROM_END

ROM_START( riskchal )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "l4-a-h0-b.ic77",    0x00001, 0x40000, CRC(4c9b5344) SHA1(61e26950a672c6404e2386acdd098536b61b9933) )
	ROM_LOAD16_BYTE( "l4-a-l0-b.ic79",    0x00000, 0x40000, CRC(0455895a) SHA1(1072b8d280f7ccc48cd8fbd81323e1f8c8d0db95) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "l4_a-sp.ic33",    0x0000, 0x10000, CRC(bb80094e) SHA1(1c62e702c395b7ebb666a79af1912b270d5f95aa) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "rc_c0.rom",    0x000000, 0x80000, CRC(84d0b907) SHA1(a686ccd67d068e5e4ba41bb8b73fdc1cad8eb5ee) )
	ROM_LOAD( "rc_c1.rom",    0x080000, 0x80000, CRC(cb3784ef) SHA1(51b8cdc35c8f3b452939ab6023a15f1c7e1a4423) )
	ROM_LOAD( "rc_c2.rom",    0x100000, 0x80000, CRC(687164d7) SHA1(0f0beb0a85ae5ae4434d1e45a27bbe67f5ee378a) )
	ROM_LOAD( "rc_c3.rom",    0x180000, 0x80000, CRC(c86be6af) SHA1(c8a66b8b38a62e3eebb4a0e65a85e20f91182097) )

	ROM_REGION( 0x40000, "m72", 0 ) /* samples */
	ROM_LOAD( "rc_v0.rom",    0x0000, 0x40000, CRC(cddac360) SHA1(a3b18325991473c6d54b778a02bed86180aad37c) )
ROM_END

ROM_START( gussun )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "l4_a-h0.ic77",    0x00001, 0x40000, CRC(9d585e61) SHA1(e108a9dc2dc1b75c1439271a2391f943c3a53fe1) )
	ROM_LOAD16_BYTE( "l4_a-l0.ic79",    0x00000, 0x40000, CRC(c7b4c519) SHA1(44887ccf54f5e507d2db4f09a7c2b7b9ea217058) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "l4_a-sp.ic33",    0x0000, 0x10000, CRC(bb80094e) SHA1(1c62e702c395b7ebb666a79af1912b270d5f95aa) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "rc_c0.rom",    0x000000, 0x80000, CRC(84d0b907) SHA1(a686ccd67d068e5e4ba41bb8b73fdc1cad8eb5ee) )
	ROM_LOAD( "rc_c1.rom",    0x080000, 0x80000, CRC(cb3784ef) SHA1(51b8cdc35c8f3b452939ab6023a15f1c7e1a4423) )
	ROM_LOAD( "rc_c2.rom",    0x100000, 0x80000, CRC(687164d7) SHA1(0f0beb0a85ae5ae4434d1e45a27bbe67f5ee378a) )
	ROM_LOAD( "rc_c3.rom",    0x180000, 0x80000, CRC(c86be6af) SHA1(c8a66b8b38a62e3eebb4a0e65a85e20f91182097) )

	ROM_REGION( 0x40000, "m72", 0 ) /* samples */
	ROM_LOAD( "rc_v0.rom",    0x0000, 0x40000, CRC(cddac360) SHA1(a3b18325991473c6d54b778a02bed86180aad37c) )
ROM_END

ROM_START( matchit2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sis2-h0-b.bin", 0x00001, 0x40000, CRC(9a2556ac) SHA1(3e4d5ac2869c703c5d5b769c2a09e501b5e6462e) ) /* Actually labeled as "SIS2-H0-B" */
	ROM_LOAD16_BYTE( "sis2-l0-b.bin", 0x00000, 0x40000, CRC(d35d948a) SHA1(e4f119fa00fd8ede2533323e14d94ad4d5fabbc5) ) /* Actually labeled as "SIS2-L0-B" */
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sis2-sp-.rom", 0x0000, 0x10000, CRC(6fc0ff3a) SHA1(2b8c648c1fb5d516552fc260b8f18ffd56bbe062) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ic81.rom",     0x000000, 0x80000, CRC(5a7cb88f) SHA1(ce3befcd956b803655b261c2ece911f444aa3a13) )
	ROM_LOAD( "ic82.rom",     0x080000, 0x80000, CRC(54a7852c) SHA1(887e7543f09d00323ce1986e72c5613dde1dc6cc) )
	ROM_LOAD( "ic83.rom",     0x100000, 0x80000, CRC(2bd65dc6) SHA1(b50dec707ea5a71972df0a8dc47141d75e8f874e) )
	ROM_LOAD( "ic84.rom",     0x180000, 0x80000, CRC(876d5fdb) SHA1(723c58268be60f4973e914df238b264708d3f1e3) )

	ROM_REGION( 0x20000, "m72", ROMREGION_ERASE00 ) /* samples */
	/* Does this have a sample rom? */
ROM_END

ROM_START( shisen2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sis2-h0-.rom", 0x00001, 0x40000, CRC(6fae0aea) SHA1(7ebecbfdb17e15b8c0ebd293cd42a618c596782e) )
	ROM_LOAD16_BYTE( "sis2-l0-.rom", 0x00000, 0x40000, CRC(2af25182) SHA1(ec6dcc3913e1b7e7a3958b78610e83f51c404e07) )
	ROM_COPY( "maincpu", 0x7fff0,  0xffff0, 0x10 )  /* start vector */

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sis2-sp-.rom", 0x0000, 0x10000, CRC(6fc0ff3a) SHA1(2b8c648c1fb5d516552fc260b8f18ffd56bbe062) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ic81.rom",     0x000000, 0x80000, CRC(5a7cb88f) SHA1(ce3befcd956b803655b261c2ece911f444aa3a13) )
	ROM_LOAD( "ic82.rom",     0x080000, 0x80000, CRC(54a7852c) SHA1(887e7543f09d00323ce1986e72c5613dde1dc6cc) )
	ROM_LOAD( "ic83.rom",     0x100000, 0x80000, CRC(2bd65dc6) SHA1(b50dec707ea5a71972df0a8dc47141d75e8f874e) )
	ROM_LOAD( "ic84.rom",     0x180000, 0x80000, CRC(876d5fdb) SHA1(723c58268be60f4973e914df238b264708d3f1e3) )

	ROM_REGION( 0x20000, "m72", ROMREGION_ERASE00 ) /* samples */
	/* Does this have a sample rom? */
ROM_END



void m90_state::init_quizf1()
{
	m_mainbank->configure_entries(0, 16, memregion("user1")->base(), 0x10000);
}



void m90_state::init_bomblord()
{
	uint16_t *ROM = (uint16_t *)(memregion("maincpu")->base());

	for (int i = 0; i < 0x100000 / 2; i += 4)
	{
		ROM[i+0]=bitswap<16>(ROM[i+0],0xc,0x8,0xd,0xe,0xf,0xb,0xa,0x9,0x6,0x4,0x7,0x3,0x1,0x2,0x0,0x5);
		ROM[i+1]=bitswap<16>(ROM[i+1],0xc,0xb,0xd,0xa,0xe,0x9,0xf,0x8,0x0,0x6,0x1,0x5,0x3,0x4,0x2,0x7);
		ROM[i+2]=bitswap<16>(ROM[i+2],0xd,0x9,0xc,0x8,0xe,0xf,0xa,0xb,0x4,0x7,0x3,0x2,0x5,0x6,0x1,0x0);
		ROM[i+3]=bitswap<16>(ROM[i+3],0xe,0xd,0xf,0x8,0xb,0xa,0x9,0xc,0x6,0x3,0x7,0x5,0x0,0x1,0x4,0x2);
	}
}



GAME( 1991, hasamu,   0,        hasamu,   hasamu,   m90_state, empty_init,    ROT0, "Irem", "Hasamu (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, dynablst, 0,        bombrman, dynablst, m90_state, empty_init,    ROT0, "Irem (licensed from Hudson Soft)", "Dynablaster / Bomber Man", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, bombrman, dynablst, bombrman, bombrman, m90_state, empty_init,    ROT0, "Irem (licensed from Hudson Soft)", "Bomber Man (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, atompunk, dynablst, bombrman, atompunk, m90_state, empty_init,    ROT0, "Irem America (licensed from Hudson Soft)", "Atomic Punk (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, dynablstb,dynablst, dynablsb, dynablsb, m90_state, empty_init,    ROT0, "bootleg (Seitu)", "Dynablaster / Bomber Man (bootleg, set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, dynablstb2,dynablst,dynablsb, dynablsb, m90_state, empty_init,    ROT0, "bootleg (Seitu)", "Dynablaster / Bomber Man (bootleg, set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, dynablstb3,dynablst,dynablsb, dynablsb, m90_state, empty_init,    ROT0, "bootleg (Seitu)", "Dynablaster / Bomber Man (bootleg, set 3)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // PCB has Playmark labels
GAME( 1992, bbmanw,   0,        bbmanw,   bbmanw,   m90_state, empty_init,    ROT0, "Irem", "Bomber Man World / New Dyna Blaster - Global Quest", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, bbmanwj,  bbmanw,   bbmanw,   bbmanwj,  m90_state, empty_init,    ROT0, "Irem", "Bomber Man World (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, bbmanwja, bbmanw,   bbmanwj,  bbmanwj,  m90_state, empty_init,    ROT0, "Irem", "Bomber Man World (Japan, revised sound hardware)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, newapunk, bbmanw,   bbmanw,   newapunk, m90_state, empty_init,    ROT0, "Irem America", "New Atomic Punk - Global Quest (US)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, bomblord, bbmanw,   bomblord, bbmanw,   m90_state, init_bomblord, ROT0, "bootleg", "Bomber Lord (bootleg)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, quizf1,   0,        quizf1,   quizf1,   m90_state, init_quizf1,   ROT0, "Irem", "Quiz F1 1-2 Finish (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, riskchal, 0,        riskchal, riskchal, m90_state, empty_init,    ROT0, "Irem", "Risky Challenge", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, gussun,   riskchal, riskchal, riskchal, m90_state, empty_init,    ROT0, "Irem", "Gussun Oyoyo (Japan)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, matchit2, 0,        matchit2, matchit2, m90_state, empty_init,    ROT0, "Tamtex", "Match It II", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, shisen2,  matchit2, matchit2, shisen2,  m90_state, empty_init,    ROT0, "Tamtex", "Shisensho II", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
