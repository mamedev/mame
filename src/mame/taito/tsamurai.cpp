// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino,Paul Swan
/****************************************************************************

    Samurai, Nunchackun, Yuke Yuke Yamaguchi-kun (c) Taito 1985

    TODO:
    - colors for this HW are a complete mystery and probably needs HW tests.
    - vsgongf sets 0 as player color in work RAM 0xc502 and it's never ever
      set up properly. Assume protection related issue.

    driver by Phil Stroffolino

Mission 660 extensions by Paul Swan (swan@easynet.co.uk)
--------------------------------------------------------
The game appears to use the same video board as Samurai et al. There is a
character column scroll feature that I have added. Its used to scroll in
the "660" logo on the title screen at the beginning. Not sure if Samurai
at al use it but it's likely their boards have the feature. Extra banking
of the foreground is done using an extra register. A bit in the background
video RAM selects the additional background character space.

The sound board is similar. There are 3 CPU's instead of the 2 of Samurai.
2 are still used for sample-like playback (as Samurai) and the other is used
to drive the AY-3-8910 (that was driven directly by the video CPU on Samurai).
The memory maps are different over Samurai, probably to allow larger capacity
ROMS though only the AY driver uses 27256 on Mission 660. A picture of the board
I have suggests that the AY CPU could have a DAC as well but the circuit is not
populated on Mission 660.

There is some kind of protection. There is code in there to do the same "unknown"
reads and writes as Samurai and the original M660 won't run with the Samurai value.
The bootleg M660 has the protection code patched out to use a fixed value. I've
used this same value on the original M660 and it seems to work.

I'm guessing the bootleg is of a "world" release and the original is from
the "America" release.

****************************************************************************/

#include "emu.h"
#include "tsamurai.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "screen.h"
#include "speaker.h"


void tsamurai_state::machine_start()
{
	m_textbank1 = 0;
	m_nmi_enabled = 0;

	m_sound_command1 = 0;
	m_sound_command2 = 0;

	save_item(NAME(m_sound_command1));
	save_item(NAME(m_sound_command2));
	save_item(NAME(m_nmi_enabled));
}

void m660_state::machine_start()
{
	tsamurai_state::machine_start();
	save_item(NAME(m_sound_command3));
}

void vsgongf_state::machine_start()
{
	tsamurai_state::machine_start();
	save_item(NAME(m_vsgongf_sound_nmi_enabled));
}

void tsamurai_state::nmi_enable_w(int state)
{
	m_nmi_enabled = state;
}

void tsamurai_state::vblank_irq(int state)
{
	if (state && m_nmi_enabled)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

uint8_t tsamurai_state::tsamurai_unknown_d803_r()
{
	return 0x6b;
}

uint8_t m660_state::m660_unknown_d803_r()
{
	return 0x53;     // this is what the bootleg patches in.
}

uint8_t tsamurai_state::unknown_d806_r()
{
	return 0x40;
}

uint8_t tsamurai_state::unknown_d900_r()
{
	return 0x6a;
}

uint8_t tsamurai_state::unknown_d938_r()
{
	return 0xfb;
}

void tsamurai_state::sound_command1_w(uint8_t data)
{
	m_sound_command1 = data;
	m_audiocpu->set_input_line(0, HOLD_LINE );
}

void tsamurai_state::sound_command2_w(uint8_t data)
{
	m_sound_command2 = data;
	m_audio2->set_input_line(0, HOLD_LINE );
}

void m660_state::m660_sound_command3_w(uint8_t data)
{
	m_sound_command3 = data;
	m_audio3->set_input_line(0, HOLD_LINE );
}

void tsamurai_state::coin1_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void tsamurai_state::coin2_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


void tsamurai_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();

	/* protection? - there are writes as well...*/
	map(0xd803, 0xd803).r(FUNC(tsamurai_state::tsamurai_unknown_d803_r));
	map(0xd806, 0xd806).r(FUNC(tsamurai_state::unknown_d806_r));
	map(0xd900, 0xd900).r(FUNC(tsamurai_state::unknown_d900_r));
	map(0xd938, 0xd938).r(FUNC(tsamurai_state::unknown_d938_r));

	map(0xe000, 0xe3ff).ram().w(FUNC(tsamurai_state::fg_videoram_w)).share("videoram");
	map(0xe400, 0xe43f).ram().w(FUNC(tsamurai_state::fg_colorram_w)).share("colorram");
	map(0xe440, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().w(FUNC(tsamurai_state::bg_videoram_w)).share("bg_videoram");
	map(0xf000, 0xf3ff).ram().share("spriteram");

	map(0xf400, 0xf400).nopw();
	map(0xf401, 0xf401).w(FUNC(tsamurai_state::sound_command1_w));
	map(0xf402, 0xf402).w(FUNC(tsamurai_state::sound_command2_w));

	map(0xf800, 0xf800).portr("P1");
	map(0xf801, 0xf801).portr("P2").w(FUNC(tsamurai_state::bgcolor_w));
	map(0xf802, 0xf802).portr("SYSTEM").w(FUNC(tsamurai_state::scrolly_w));
	map(0xf803, 0xf803).w(FUNC(tsamurai_state::scrollx_w));
	map(0xf804, 0xf804).portr("DSW1");
	map(0xf805, 0xf805).portr("DSW2");

	map(0xfc00, 0xfc07).w("mainlatch", FUNC(ls259_device::write_d0));
}

void m660_state::m660_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();

	/* protection? - there are writes as well...*/
	map(0xd803, 0xd803).r(FUNC(m660_state::m660_unknown_d803_r));
	map(0xd806, 0xd806).r(FUNC(m660_state::unknown_d806_r));
	map(0xd900, 0xd900).r(FUNC(m660_state::unknown_d900_r));
	map(0xd938, 0xd938).r(FUNC(m660_state::unknown_d938_r));

	map(0xe000, 0xe3ff).ram().w(FUNC(m660_state::fg_videoram_w)).share("videoram");
	map(0xe400, 0xe43f).ram().w(FUNC(m660_state::fg_colorram_w)).share("colorram");
	map(0xe440, 0xe7ff).ram();
	map(0xe800, 0xefff).ram().w(FUNC(m660_state::bg_videoram_w)).share("bg_videoram");
	map(0xf000, 0xf3ff).ram().share("spriteram");

	map(0xf400, 0xf400).nopw();/* This is always written with F401, F402 & F403 data */
	map(0xf401, 0xf401).w(FUNC(m660_state::m660_sound_command3_w));
	map(0xf402, 0xf402).w(FUNC(m660_state::sound_command2_w));
	map(0xf403, 0xf403).w(FUNC(m660_state::sound_command1_w));

	map(0xf800, 0xf800).portr("P1");
	map(0xf801, 0xf801).portr("P2").w(FUNC(m660_state::bgcolor_w));
	map(0xf802, 0xf802).portr("SYSTEM").w(FUNC(m660_state::scrolly_w));
	map(0xf803, 0xf803).w(FUNC(m660_state::scrollx_w));
	map(0xf804, 0xf804).portr("DSW1");
	map(0xf805, 0xf805).portr("DSW2");

	map(0xfc00, 0xfc07).w("mainlatch", FUNC(ls259_device::write_d0));
}

void tsamurai_state::z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
}

void m660_state::z80_m660_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw();        /* ? */
	map(0x01, 0x01).nopw();        /* Written continuously. Increments with level. */
	map(0x02, 0x02).nopw();        /* Always follows above with 0x01 data */
}

void vsgongf_state::vsgongf_audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
}

uint8_t tsamurai_state::sound_command1_r()
{
	return m_sound_command1;
}

uint8_t tsamurai_state::sound_command2_r()
{
	return m_sound_command2;
}

uint8_t m660_state::m660_sound_command3_r()
{
	return m_sound_command3;
}

/*******************************************************************************/
void tsamurai_state::sound1_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x6000).r(FUNC(tsamurai_state::sound_command1_r));
	map(0x6001, 0x6001).nopw(); /* ? - probably clear IRQ */
	map(0x6002, 0x6002).w("dac1", FUNC(dac_byte_interface::data_w));
	map(0x7f00, 0x7fff).ram();
}

/*******************************************************************************/

void tsamurai_state::sound2_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x6000).r(FUNC(tsamurai_state::sound_command2_r));
	map(0x6001, 0x6001).nopw(); /* ? - probably clear IRQ */
	map(0x6002, 0x6002).w("dac2", FUNC(dac_byte_interface::data_w));
	map(0x7f00, 0x7fff).ram();
}

/*******************************************************************************/

void m660_state::sound1_m660_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xc000, 0xc000).r(FUNC(m660_state::sound_command1_r));
	map(0xc001, 0xc001).nopw(); /* ? - probably clear IRQ */
	map(0xc002, 0xc002).w("dac1", FUNC(dac_byte_interface::data_w));
	map(0x8000, 0x87ff).ram();
}

/*******************************************************************************/

void m660_state::sound2_m660_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xc000, 0xc000).r(FUNC(m660_state::sound_command2_r));
	map(0xc001, 0xc001).nopw(); /* ? - probably clear IRQ */
	map(0xc002, 0xc002).w("dac2", FUNC(dac_byte_interface::data_w));
	map(0x8000, 0x87ff).ram();
}

/*******************************************************************************/

void m660_state::sound3_m660_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc000).r(FUNC(m660_state::m660_sound_command3_r));
	map(0xc001, 0xc001).nopw(); /* ? - probably clear IRQ */
	map(0x8000, 0x87ff).ram();
	map(0xfffc, 0xffff).ram(); /* CPU writes here - music data */
}

void m660_state::sound3_m660_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
}

/*******************************************************************************/

void vsgongf_state::vsgongf_sound_nmi_enable_w(uint8_t data)
{
	m_vsgongf_sound_nmi_enabled = data;
}

INTERRUPT_GEN_MEMBER(vsgongf_state::vsgongf_sound_interrupt)
{
	if (m_vsgongf_sound_nmi_enabled)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

/* what are these, protection of some kind? */

uint8_t vsgongf_state::vsgongf_a006_r()
{
	/* sound CPU busy? */
	if (!strcmp(machine().system().name,"vsgongf"))  return 0x80;
	if (!strcmp(machine().system().name,"ringfgt"))  return 0x80;
	if (!strcmp(machine().system().name,"ringfgt2")) return 0xc0;

	logerror ("unhandled read from a006\n");
	return 0x00;
}

uint8_t vsgongf_state::vsgongf_a100_r()
{
	/* protection? */
	if (!strcmp(machine().system().name,"vsgongf"))  return 0xaa;
	if (!strcmp(machine().system().name,"ringfgt"))  return 0x63;
	if (!strcmp(machine().system().name,"ringfgt2")) return 0x6a;

	logerror ("unhandled read from a100\n");
	return 0x00;
}

void vsgongf_state::vsgongf_sound_command_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void vsgongf_state::vsgongf_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xa003, 0xa003).readonly();
	map(0xa006, 0xa006).r(FUNC(vsgongf_state::vsgongf_a006_r)); /* protection */
	map(0xa100, 0xa100).r(FUNC(vsgongf_state::vsgongf_a100_r)); /* protection */
	map(0xc000, 0xc7ff).ram();                  /* work ram */
	map(0xe000, 0xe3ff).ram().w(FUNC(vsgongf_state::fg_videoram_w)).share("videoram");
	map(0xe400, 0xe43f).ram().share("spriteram");
	map(0xe440, 0xe47b).ram();
	map(0xe800, 0xe800).w(FUNC(vsgongf_state::vsgongf_sound_command_w));
	map(0xec00, 0xec06).nopw();
	map(0xf000, 0xf000).w(FUNC(vsgongf_state::vsgongf_color_w));
	map(0xf400, 0xf400).nopw(); /* vreg? always 0 */
	map(0xf800, 0xf800).portr("P1");
	map(0xf801, 0xf801).portr("P2");
	map(0xf802, 0xf802).portr("SYSTEM");
	map(0xf804, 0xf804).portr("DSW1");
	map(0xf805, 0xf805).portr("DSW2");
	map(0xf800, 0xf800).nopw();
	map(0xf801, 0xf801).nopw(); /* vreg? always 0 */
	map(0xf803, 0xf803).nopw(); /* vreg? always 0 */
	map(0xfc00, 0xfc07).w("mainlatch", FUNC(ls259_device::write_d0));
}

void vsgongf_state::sound_vsgongf_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x63ff).ram(); /* work RAM */
	map(0x8000, 0x8000).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w(FUNC(vsgongf_state::vsgongf_sound_nmi_enable_w)); /* NMI enable */
	map(0xa000, 0xa000).w("dac", FUNC(dac_byte_interface::data_w));
}

/*******************************************************************************/

static INPUT_PORTS_START( tsamurai )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("DSW1:!1,!2,!3")
	PORT_DIPSETTING(    0x07, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("DSW1:!4,!5,!6")
	PORT_DIPSETTING(    0x38, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x00, "Freeze" )                 PORT_DIPLOCATION("DSW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )       PORT_DIPLOCATION("DSW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )         PORT_DIPLOCATION("DSW2:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x03, "254 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, "DSW2 Unknown 1" )         PORT_DIPLOCATION("DSW2:!3,!4")
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x0c, "70" )
	PORT_DIPNAME( 0x30, 0x30, "DSW2 Unknown 2" )         PORT_DIPLOCATION("DSW2:!5,!6")
	PORT_DIPSETTING(    0x00, "0x00" )
	PORT_DIPSETTING(    0x10, "0x01" )
	PORT_DIPSETTING(    0x20, "0x02" )
	PORT_DIPSETTING(    0x30, "0x03" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("DSW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2 Unknown 3" )         PORT_DIPLOCATION("DSW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tsamuraih )
	PORT_INCLUDE( tsamurai )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )         PORT_DIPLOCATION("DSW2:!1,!2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "254 (Cheat)")
INPUT_PORTS_END

static INPUT_PORTS_START( ladymstr )
	PORT_INCLUDE( tsamurai )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( nunchaku )
	PORT_INCLUDE( ladymstr )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )         PORT_DIPLOCATION("DSW2:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, "DSW2 Unknown 1" )         PORT_DIPLOCATION("DSW2:!3,!4")
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x0c, "70" )
	PORT_DIPNAME( 0x30, 0x30, "DSW2 Unknown 2" )         PORT_DIPLOCATION("DSW2:!5,!6")
	PORT_DIPSETTING(    0x00, "0x00" )
	PORT_DIPSETTING(    0x10, "0x01" )
	PORT_DIPSETTING(    0x20, "0x02" )
	PORT_DIPSETTING(    0x30, "0x03" )
	PORT_DIPNAME( 0x40, 0x40, "DSW2 Unknown 3" )         PORT_DIPLOCATION("DSW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2 Unknown 4" )         PORT_DIPLOCATION("DSW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( vsgongf )
	PORT_INCLUDE( tsamurai )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DSW2:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DSW2:!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DSW2:!3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DSW2:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DSW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DSW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DSW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )       PORT_DIPLOCATION("DSW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( yamagchi )
	PORT_INCLUDE( tsamurai )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )         PORT_DIPLOCATION("DSW2:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "7" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, "DSW2 Unknown 1" )         PORT_DIPLOCATION("DSW2:!3,!4")
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x0c, "70" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Language ) )      PORT_DIPLOCATION("DSW2:!5")
	PORT_DIPSETTING(    0x10, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2 Unknown 2" )         PORT_DIPLOCATION("DSW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )   PORT_DIPLOCATION("DSW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2 Unknown 3" )         PORT_DIPLOCATION("DSW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( m660 )
	PORT_INCLUDE( tsamurai )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )        PORT_DIPLOCATION("DSW1:!1,!2,!3")
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )        PORT_DIPLOCATION("DSW1:!4,!5,!6")
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, "Freeze" )                 PORT_DIPLOCATION("DSW1:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Continues ) )     PORT_DIPLOCATION("DSW1:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )         PORT_DIPLOCATION("DSW2:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x0c, "Bonus" )                  PORT_DIPLOCATION("DSW2:!3,!4")
	PORT_DIPSETTING(    0x00, "10,30,50" )
	PORT_DIPSETTING(    0x04, "20,50,80" )
	PORT_DIPSETTING(    0x08, "30,70,110" )
	PORT_DIPSETTING(    0x0c, "50,100,150" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("DSW2:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )       PORT_DIPLOCATION("DSW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )        PORT_DIPLOCATION("DSW2:!8") // listed as screen flip (hardware)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout sprite_layout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{
		0,1,2,3,4,5,6,7,
		64+0,64+1,64+2,64+3,64+4,64+5,64+6,64+7,
		128+0,128+1,128+2,128+3,128+4,128+5,128+6,128+7,
		64*3+0,64*3+1,64*3+2,64*3+3,64*3+4,64*3+5,64*3+6,64*3+7
	},
	{
		0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		1*256+0*8,1*256+1*8,1*256+2*8,1*256+3*8,1*256+4*8,1*256+5*8,1*256+6*8,1*256+7*8,
		2*256+0*8,2*256+1*8,2*256+2*8,2*256+3*8,2*256+4*8,2*256+5*8,2*256+6*8,2*256+7*8,
		3*256+0*8,3*256+1*8,3*256+2*8,3*256+3*8,3*256+4*8,3*256+5*8,3*256+6*8,3*256+7*8
	},
	4*256
};

static GFXDECODE_START( gfx_tsamurai )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x3_planar, 0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x3_planar, 0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, sprite_layout,    0, 32 )
GFXDECODE_END


/*******************************************************************************/

void tsamurai_state::tsamurai(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(24'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &tsamurai_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &tsamurai_state::z80_io_map);

	Z80(config, m_audiocpu, XTAL(24'000'000)/8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &tsamurai_state::sound1_map);

	Z80(config, m_audio2, XTAL(24'000'000)/8);
	m_audio2->set_addrmap(AS_PROGRAM, &tsamurai_state::sound2_map);


	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(tsamurai_state::flip_screen_set));
	mainlatch.q_out_cb<1>().set(FUNC(tsamurai_state::nmi_enable_w));
	mainlatch.q_out_cb<2>().set(FUNC(tsamurai_state::textbank1_w));
	mainlatch.q_out_cb<3>().set(FUNC(tsamurai_state::coin1_counter_w));
	mainlatch.q_out_cb<4>().set(FUNC(tsamurai_state::coin2_counter_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0, 255, 16, 255-16);
	screen.set_screen_update(FUNC(tsamurai_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(tsamurai_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tsamurai);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	AY8910(config,"aysnd", XTAL(24'000'000)/8).add_route(ALL_OUTPUTS, "speaker", 0.1);

	DAC_8BIT_R2R(config, "dac1", 0).add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC
	DAC_8BIT_R2R(config, "dac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC
}


void vsgongf_state::vsgongf(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(24'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &vsgongf_state::vsgongf_map);

	Z80(config, m_audiocpu, XTAL(24'000'000)/8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &vsgongf_state::sound_vsgongf_map);
	m_audiocpu->set_addrmap(AS_IO, &vsgongf_state::vsgongf_audio_io_map);
	m_audiocpu->set_periodic_int(FUNC(vsgongf_state::vsgongf_sound_interrupt), attotime::from_hz(3*60));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 4L
	mainlatch.q_out_cb<0>().set_nop(); // vreg? always 0
	mainlatch.q_out_cb<1>().set(FUNC(vsgongf_state::nmi_enable_w));
	mainlatch.q_out_cb<2>().set(FUNC(vsgongf_state::coin1_counter_w));
	mainlatch.q_out_cb<3>().set(FUNC(vsgongf_state::coin2_counter_w));
	mainlatch.q_out_cb<4>().set(FUNC(vsgongf_state::textbank1_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0, 255, 16, 255-16);
	screen.set_screen_update(FUNC(vsgongf_state::screen_update_vsgongf));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(vsgongf_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tsamurai);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config,"aysnd", XTAL(24'000'000)/8).add_route(ALL_OUTPUTS, "speaker", 0.1);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC
}


void m660_state::m660(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(24'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &m660_state::m660_map);
	m_maincpu->set_addrmap(AS_IO, &m660_state::z80_m660_io_map);

	Z80(config, m_audiocpu, XTAL(24'000'000)/8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &m660_state::sound1_m660_map);

	Z80(config, m_audio2, XTAL(24'000'000)/8);
	m_audio2->set_addrmap(AS_PROGRAM, &m660_state::sound2_m660_map);

	Z80(config, m_audio3, XTAL(24'000'000)/8);
	m_audio3->set_addrmap(AS_PROGRAM, &m660_state::sound3_m660_map);
	m_audio3->set_addrmap(AS_IO, &m660_state::sound3_m660_io_map);


	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(m660_state::flip_screen_set));
	mainlatch.q_out_cb<1>().set(FUNC(m660_state::nmi_enable_w));
	mainlatch.q_out_cb<2>().set(FUNC(m660_state::textbank1_w));
	mainlatch.q_out_cb<3>().set(FUNC(m660_state::coin1_counter_w));
	mainlatch.q_out_cb<4>().set(FUNC(m660_state::coin2_counter_w));
	mainlatch.q_out_cb<7>().set(FUNC(m660_state::textbank2_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0, 255, 16, 255-16);
	screen.set_screen_update(FUNC(m660_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(m660_state::vblank_irq));
	screen.screen_vblank().append_inputline(m_audio3, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tsamurai);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	AY8910(config,"aysnd", XTAL(24'000'000)/8).add_route(ALL_OUTPUTS, "speaker", 0.1);

	DAC_8BIT_R2R(config, "dac1", 0).add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC
	DAC_8BIT_R2R(config, "dac2", 0).add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC
}


/*******************************************************************************/

ROM_START( tsamurai ) // there's a protection device labeled B5 at location l3 on the main board
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "a35-01-1.3r", 0x0000, 0x4000, CRC(d09c8609) SHA1(66b51897704250f520b4c58cb6f6f3aef8913459) )
	ROM_LOAD( "a35-02-1.3t", 0x4000, 0x4000, CRC(d0f2221c) SHA1(6cfa9a52b35d17776cfa3e14e679b1a6218d54fa) )
	ROM_LOAD( "a35.03-1.3v", 0x8000, 0x4000, CRC(eee8b0c9) SHA1(91dd47cdcd36d804e178b70d4338292ac36517f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player#1 */
	ROM_LOAD( "a35-14.4e",  0x0000, 0x2000, CRC(220e9c04) SHA1(660351c866995147d2ba69940707879d6cf11718) )
	ROM_LOAD( "a35-15.4c",  0x2000, 0x2000, CRC(1e0d1e33) SHA1(02612f10c264f06f59f61f0de4df0ef84249e963) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player#2 */
	ROM_LOAD( "a35-13.4j",      0x0000, 0x2000, CRC(73feb0e2) SHA1(7c650d0cdc517a60e14614083ab42aa934338556) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "a35-04.10a", 0x0000, 0x2000, CRC(b97ce9b1) SHA1(3993001bd98758fd5673d91786846ae019c64027) ) // tiles
	ROM_LOAD( "a35-05.10b", 0x2000, 0x2000, CRC(55a17b08) SHA1(29427b51b780fb622ac093ea4604caf77cb587ed) )
	ROM_LOAD( "a35-06.10d", 0x4000, 0x2000, CRC(f5ee6f8f) SHA1(0ec90c5edd4d8a9ef614d60bafb002be3daf34ee) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "a35-10.11n", 0x0000, 0x1000, CRC(0b5a0c45) SHA1(4b8ea5dd58c437e7d7929b93ff795803422cb505) ) // characters
	ROM_LOAD( "a35-11.11q", 0x2000, 0x1000, CRC(93346d75) SHA1(b7936ff6f08e4d01bf7d4d2f06ef6b5d84b5097d) )
	ROM_LOAD( "a35-12.11r", 0x4000, 0x1000, CRC(f4c69d8a) SHA1(fb308146d4f4b01bd1318c6986e5124725b2a98f) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "a35-07.12h", 0x0000, 0x4000, CRC(38fc349f) SHA1(d9fdb5bc84808d065e84e077f0e78d8f71b4f0ca) ) // sprites
	ROM_LOAD( "a35-08.12j", 0x4000, 0x4000, CRC(a07d6dc3) SHA1(2ab50ea462a63548d401f85627d85de2e4867303) )
	ROM_LOAD( "a35-09.12k", 0x8000, 0x4000, CRC(c0784a0e) SHA1(e8303bb274361910746eba5e3d61b6ea862a6416) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "a35-16.2j",  0x000, 0x0100, CRC(72d8b332) SHA1(623b1f3fd0008ad92fd1f6fc2b07215da0c26207) )
	ROM_LOAD( "a35-17.2l",  0x100, 0x0100, CRC(9bf1829e) SHA1(7ee47a1a0aa2e4592896d0fc5959343452c224a8) )
	ROM_LOAD( "a35-18.2m",  0x200, 0x0100, CRC(918e4732) SHA1(a38686b32d5ac0ebcba59fdba3201fe35c83d4d0) )
ROM_END

ROM_START( tsamurai2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "a35-01.3r",  0x0000, 0x4000, CRC(282d96ad) SHA1(c9d7a9b7acbe6431c061a9b50c05fab3ae664094) )
	ROM_LOAD( "a35-02.3t",  0x4000, 0x4000, CRC(e3fa0cfa) SHA1(3ed8a67789f666fe12d7597014d39deea3c12506) )
	ROM_LOAD( "a35-03.3v",  0x8000, 0x4000, CRC(2fff1e0a) SHA1(0d54b0c9c4760a02bfe5b5d77fff4c858b15dbd8) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player#1 */
	ROM_LOAD( "a35-14.4e",  0x0000, 0x2000, CRC(f10aee3b) SHA1(c4fa2bd626b15b9ea1d5d7e6eaab4f1674841b02) )
	ROM_LOAD( "a35-15.4c",  0x2000, 0x2000, CRC(1e0d1e33) SHA1(02612f10c264f06f59f61f0de4df0ef84249e963) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player#2 */
	ROM_LOAD( "a35-13.4j",  0x0000, 0x2000, CRC(3828f4d2) SHA1(646477c431b123c031ed452b65e96a33e78a2bac) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "a35-04.10a", 0x0000, 0x2000, CRC(b97ce9b1) SHA1(3993001bd98758fd5673d91786846ae019c64027) ) // tiles
	ROM_LOAD( "a35-05.10b", 0x2000, 0x2000, CRC(55a17b08) SHA1(29427b51b780fb622ac093ea4604caf77cb587ed) )
	ROM_LOAD( "a35-06.10d", 0x4000, 0x2000, CRC(f5ee6f8f) SHA1(0ec90c5edd4d8a9ef614d60bafb002be3daf34ee) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "a35-10.11n", 0x0000, 0x1000, CRC(0b5a0c45) SHA1(4b8ea5dd58c437e7d7929b93ff795803422cb505) ) // characters
	ROM_LOAD( "a35-11.11q", 0x2000, 0x1000, CRC(93346d75) SHA1(b7936ff6f08e4d01bf7d4d2f06ef6b5d84b5097d) )
	ROM_LOAD( "a35-12.11r", 0x4000, 0x1000, CRC(f4c69d8a) SHA1(fb308146d4f4b01bd1318c6986e5124725b2a98f) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "a35-07.12h", 0x0000, 0x4000, CRC(38fc349f) SHA1(d9fdb5bc84808d065e84e077f0e78d8f71b4f0ca) ) // sprites
	ROM_LOAD( "a35-08.12j", 0x4000, 0x4000, CRC(a07d6dc3) SHA1(2ab50ea462a63548d401f85627d85de2e4867303) )
	ROM_LOAD( "a35-09.12k", 0x8000, 0x4000, CRC(c0784a0e) SHA1(e8303bb274361910746eba5e3d61b6ea862a6416) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "a35-16.2j",  0x000, 0x0100, CRC(72d8b332) SHA1(623b1f3fd0008ad92fd1f6fc2b07215da0c26207) )
	ROM_LOAD( "a35-17.2l",  0x100, 0x0100, CRC(9bf1829e) SHA1(7ee47a1a0aa2e4592896d0fc5959343452c224a8) )
	ROM_LOAD( "a35-18.2m",  0x200, 0x0100, CRC(918e4732) SHA1(a38686b32d5ac0ebcba59fdba3201fe35c83d4d0) )
ROM_END

ROM_START( tsamuraih )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "a35-01h.3r", 0x0000, 0x4000, CRC(551e1fd1) SHA1(2f573977f6b66c45a9f10b4ec6727e6105c10469) )
	ROM_LOAD( "a35-02.3t",  0x4000, 0x4000, CRC(e3fa0cfa) SHA1(3ed8a67789f666fe12d7597014d39deea3c12506) )
	ROM_LOAD( "a35-03.3v",  0x8000, 0x4000, CRC(2fff1e0a) SHA1(0d54b0c9c4760a02bfe5b5d77fff4c858b15dbd8) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player#1 */
	ROM_LOAD( "a35-14.4e",  0x0000, 0x2000, CRC(f10aee3b) SHA1(c4fa2bd626b15b9ea1d5d7e6eaab4f1674841b02) )
	ROM_LOAD( "a35-15.4c",  0x2000, 0x2000, CRC(1e0d1e33) SHA1(02612f10c264f06f59f61f0de4df0ef84249e963) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player#2 */
	ROM_LOAD( "a35-13.4j",  0x0000, 0x2000, CRC(3828f4d2) SHA1(646477c431b123c031ed452b65e96a33e78a2bac) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "a35-04.10a", 0x0000, 0x2000, CRC(b97ce9b1) SHA1(3993001bd98758fd5673d91786846ae019c64027) ) // tiles
	ROM_LOAD( "a35-05.10b", 0x2000, 0x2000, CRC(55a17b08) SHA1(29427b51b780fb622ac093ea4604caf77cb587ed) )
	ROM_LOAD( "a35-06.10d", 0x4000, 0x2000, CRC(f5ee6f8f) SHA1(0ec90c5edd4d8a9ef614d60bafb002be3daf34ee) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "a35-10.11n", 0x0000, 0x1000, CRC(0b5a0c45) SHA1(4b8ea5dd58c437e7d7929b93ff795803422cb505) ) // characters
	ROM_LOAD( "a35-11.11q", 0x2000, 0x1000, CRC(93346d75) SHA1(b7936ff6f08e4d01bf7d4d2f06ef6b5d84b5097d) )
	ROM_LOAD( "a35-12.11r", 0x4000, 0x1000, CRC(f4c69d8a) SHA1(fb308146d4f4b01bd1318c6986e5124725b2a98f) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "a35-07.12h", 0x0000, 0x4000, CRC(38fc349f) SHA1(d9fdb5bc84808d065e84e077f0e78d8f71b4f0ca) ) // sprites
	ROM_LOAD( "a35-08.12j", 0x4000, 0x4000, CRC(a07d6dc3) SHA1(2ab50ea462a63548d401f85627d85de2e4867303) )
	ROM_LOAD( "a35-09.12k", 0x8000, 0x4000, CRC(c0784a0e) SHA1(e8303bb274361910746eba5e3d61b6ea862a6416) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "a35-16.2j",  0x000, 0x0100, CRC(72d8b332) SHA1(623b1f3fd0008ad92fd1f6fc2b07215da0c26207) )
	ROM_LOAD( "a35-17.2l",  0x100, 0x0100, CRC(9bf1829e) SHA1(7ee47a1a0aa2e4592896d0fc5959343452c224a8) )
	ROM_LOAD( "a35-18.2m",  0x200, 0x0100, CRC(918e4732) SHA1(a38686b32d5ac0ebcba59fdba3201fe35c83d4d0) )
ROM_END

ROM_START( nunchaku )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "nunchack.p1", 0x0000, 0x4000, CRC(4385aca6) SHA1(bf6b40340b773929189fb2a0a271040c79a405a1) )
	ROM_LOAD( "nunchack.p2", 0x4000, 0x4000, CRC(f9beb72c) SHA1(548dc9187f87d7a47958691391d2494c2306d767) )
	ROM_LOAD( "nunchack.p3", 0x8000, 0x4000, CRC(cde5d674) SHA1(0360fe81acb6fd77ef581a36a756db550de73732) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "nunchack.m3", 0x0000, 0x2000, CRC(9036c945) SHA1(8e7cb6313b32a78ca0a7fa8595fb872e0f27d8c7) )
	ROM_LOAD( "nunchack.m4", 0x2000, 0x2000, CRC(e7206724) SHA1(fb7f9bfe1e04e1f6af733fc3a79f88e942f3b0b1) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "nunchack.m1", 0x0000, 0x2000, CRC(b53d73f6) SHA1(20b333646a1374fa566b6d608723296e6ded7bc8) )
	ROM_LOAD( "nunchack.m2", 0x2000, 0x2000, CRC(f37d7c49) SHA1(e26d32bf1ecf23b55511260596daf676d9824d37) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "nunchack.b1", 0x0000, 0x2000, CRC(48c88fea) SHA1(2ab27fc69f060e8923f88f9e878e3911d670f5a8) ) // tiles
	ROM_LOAD( "nunchack.b2", 0x2000, 0x2000, CRC(eec818e4) SHA1(1d806dbf6589737e3a4fb52f17bc4c6766a6d6a1) )
	ROM_LOAD( "nunchack.b3", 0x4000, 0x2000, CRC(5f16473f) SHA1(32d8b4a0a7152d86f161d0f30496c25ceff46af3) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "nunchack.v1", 0x0000, 0x1000, CRC(358a3714) SHA1(bf54bf4059cd344d4d861e172b5df5b7763d49d8) ) // characters
	ROM_LOAD( "nunchack.v2", 0x2000, 0x1000, CRC(54c18d8e) SHA1(edcd0a6b5fb1efa2f3693e170cb398574280f7fa) )
	ROM_LOAD( "nunchack.v3", 0x4000, 0x1000, CRC(f7ac203a) SHA1(148a003b48d858eb33f0fee295350483cef42481) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "nunchack.c1", 0x0000, 0x4000, CRC(797cbc8a) SHA1(28ad936318d8b671d4031927c2ea666891a1a408) ) // sprites
	ROM_LOAD( "nunchack.c2", 0x4000, 0x4000, CRC(701a0cc3) SHA1(323ee1cba3da0ccb2c4d542c497de0e1c047f532) )
	ROM_LOAD( "nunchack.c3", 0x8000, 0x4000, CRC(ffb841fc) SHA1(c1285cf093360923307bc86f6a5473d689b16a2c) )

	ROM_REGION( 0x0300, "proms", 0 ) // no way these PROMs can give the colours seen in reference videos
	ROM_LOAD( "nunchack.016", 0x000, 0x100, BAD_DUMP CRC(a7b077d4) SHA1(48c3e68d67de067c0ead0dbd34769b755fb5952f) )
	ROM_LOAD( "nunchack.017", 0x100, 0x100, BAD_DUMP CRC(1c04c087) SHA1(7179edf96f59a469353d9652900b99fef25f4054) )
	ROM_LOAD( "nunchack.018", 0x200, 0x100, BAD_DUMP CRC(f5ce3c45) SHA1(f2dcdaf95b55b8fd713bdbb965731c064b4a0757) )
ROM_END

ROM_START( yamagchi )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "a38-01.3s", 0x0000, 0x4000, CRC(1a6c8498) SHA1(5c343ff09733507a1518e5a3cab315d9a51ae289) )
	ROM_LOAD( "a38-02.3t", 0x4000, 0x4000, CRC(fa66b396) SHA1(7594549d0c90f5937d11b7ffe80f229df2cea352) )
	ROM_LOAD( "a38-03.3v", 0x8000, 0x4000, CRC(6a4239cf) SHA1(7883f3a7ed18cc0dc0ebb3d929eb6f92df9200de) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "a38-14.4e", 0x0000, 0x2000, CRC(5a758992) SHA1(ee30037ffddf45e9374ab01867c9f6604dede952) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "a38-13.4j", 0x0000, 0x2000, CRC(a26445bb) SHA1(bf09e3f8cf36563bda9ef5a7d01b76155f505b98) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "a38-04.10a", 0x0000, 0x2000, CRC(6bc69d4d) SHA1(245303851ff1850eb68c73b033bb4253afc92f2e) ) // tiles
	ROM_LOAD( "a38-05.10b", 0x2000, 0x2000, CRC(047fb315) SHA1(3689c3cf4c005791fd76574171d14add2bc046a6) )
	ROM_LOAD( "a38-06.10d", 0x4000, 0x2000, CRC(a636afb2) SHA1(bff04d696e54a91b5e16c9d98769970af2184b13) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "a38-10.11n", 0x0000, 0x1000, CRC(51ab4671) SHA1(8e1bf5c995dbda3038f889a15282974b265d50ef) ) // characters
	ROM_LOAD( "a38-11.11p", 0x2000, 0x1000, CRC(27890169) SHA1(ded7dc8d2e738e337965b548c2289d2a7acdb663) )
	ROM_LOAD( "a38-12.11r", 0x4000, 0x1000, CRC(c98d5cf2) SHA1(e4fc8a46790a596de789ddb3ce1caa3400a84c74) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "a38-07.12h", 0x0000, 0x4000, CRC(a3a521b6) SHA1(267e718e4f288b264b814edde4f72bd917190203) ) // sprites
	ROM_LOAD( "a38-08.12j", 0x4000, 0x4000, CRC(553afc66) SHA1(3215be8642ae011b5a8f2ada5799eeb32a01cac1) )
	ROM_LOAD( "a38-09.12l", 0x8000, 0x4000, CRC(574156ae) SHA1(b82cca25fca7b64778bf461e61ff04fec846c6fa) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "mb7114e.2k", 0x000, 0x100, CRC(e7648110) SHA1(17327bc8dacbaa8f03e14c8b59bded117be46e68) )
	ROM_LOAD( "mb7114e.2l", 0x100, 0x100, CRC(7b874ee6) SHA1(d23bc6dfe882cfa9a2c84bf8037b47759427060f) )
	ROM_LOAD( "mb7114e.2m", 0x200, 0x100, CRC(938d0fce) SHA1(9aa14fdee23b1ed50300fe8f82525bb363914d93) )
ROM_END

ROM_START( ladymstr ) // there's a protection device labeled 6 at location l3 on the main board
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "a49-01-1.3r",0x0000, 0x4000, CRC(acbd0b64) SHA1(00a95ad28b6923dab808dd94af10cb1d70123d3e) ) // believed to be newer because of the -01 suffix
	ROM_LOAD( "a49-02.3t",  0x4000, 0x4000, CRC(b0a9020b) SHA1(78c777ffa6e9063fe4e816d9a58e394f45bd875b) )
	ROM_LOAD( "a49-03.3v",  0x8000, 0x4000, CRC(641c94ed) SHA1(494502d2478f9d8ad29be6c1815a5e4639d6ba3a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player#1 */
	ROM_LOAD( "a49-14.4e",  0x0000, 0x2000, CRC(d83a3c02) SHA1(43f49d8f57726a629533d37a11338fd071e2e2d7) )
	ROM_LOAD( "a49-15.4c",  0x2000, 0x2000, CRC(d24ee5fd) SHA1(f35a602cd24427687580f80d4c045c42557cdf06) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player#2 */
	ROM_LOAD( "a49-13.4j",  0x0000, 0x2000, CRC(7942bd7c) SHA1(71a10db610a4eaf2e30a7ace4473d9eeb3239dc9) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "a49-04.10a", 0x0000, 0x2000, CRC(1fed96e6) SHA1(47c44def272a94e11d06948841c414568acc832a) ) // tiles
	ROM_LOAD( "a49-05.10c", 0x2000, 0x2000, CRC(e0fce676) SHA1(f374ab8de8157327fdd48438d74c1bf7a5bf156d) )
	ROM_LOAD( "a49-06.10d", 0x4000, 0x2000, CRC(f895672e) SHA1(f6ee81663850db025484b39c683ff992a0c84877) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "a49-10.11n", 0x0000, 0x1000, CRC(a7a361ba) SHA1(f142f78ac2434b22240202a9763c921db2d42fdb) ) // characters
	ROM_LOAD( "a49-11.11q", 0x2000, 0x1000, CRC(801902e3) SHA1(abbcc2c6d805ab158f94ef60125eeea4d5041f71) )
	ROM_LOAD( "a49-12.11r", 0x4000, 0x1000, CRC(cef75565) SHA1(9325effa7bd920af4997858bf26ebb01eeb858cd) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "a49-07.12h", 0x0000, 0x4000, CRC(8c749828) SHA1(118057c1364d996f1445214c6aff749d6aa30a22) ) // sprites
	ROM_LOAD( "a49-08.12j", 0x4000, 0x4000, CRC(03c10aed) SHA1(65b28bd724d9812d58ee5e0a4d0b5b0f9c2723b4) )
	ROM_LOAD( "a49-09.12k", 0x8000, 0x4000, CRC(f61316d2) SHA1(b07eec0f4a2fe971ea3f4d56739981aef5ff4d0a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "a49-16.2j",  0x000, 0x0100, CRC(a7b077d4) SHA1(48c3e68d67de067c0ead0dbd34769b755fb5952f) )
	ROM_LOAD( "a49-17.2l",  0x100, 0x0100, CRC(1c04c087) SHA1(7179edf96f59a469353d9652900b99fef25f4054) )
	ROM_LOAD( "a49-18.2m",  0x200, 0x0100, CRC(f5ce3c45) SHA1(f2dcdaf95b55b8fd713bdbb965731c064b4a0757) )
ROM_END

ROM_START( ladymstr2 ) // there's a protection device labeled 6 at location l3 on the main board
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "a49-01.3r",  0x0000, 0x4000, CRC(8729e50e) SHA1(0b75dd6da26e71b32cfd1dfc1160e35f928286c4) )
	ROM_LOAD( "a49-02.3t",  0x4000, 0x4000, CRC(b0a9020b) SHA1(78c777ffa6e9063fe4e816d9a58e394f45bd875b) )
	ROM_LOAD( "a49-03.3v",  0x8000, 0x4000, CRC(641c94ed) SHA1(494502d2478f9d8ad29be6c1815a5e4639d6ba3a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player#1 */
	ROM_LOAD( "a49-14.4e",  0x0000, 0x2000, CRC(d83a3c02) SHA1(43f49d8f57726a629533d37a11338fd071e2e2d7) )
	ROM_LOAD( "a49-15.4c",  0x2000, 0x2000, CRC(d24ee5fd) SHA1(f35a602cd24427687580f80d4c045c42557cdf06) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player#2 */
	ROM_LOAD( "a49-13.4j",  0x0000, 0x2000, CRC(7942bd7c) SHA1(71a10db610a4eaf2e30a7ace4473d9eeb3239dc9) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "a49-04.10a", 0x0000, 0x2000, CRC(1fed96e6) SHA1(47c44def272a94e11d06948841c414568acc832a) ) // tiles
	ROM_LOAD( "a49-05.10c", 0x2000, 0x2000, CRC(e0fce676) SHA1(f374ab8de8157327fdd48438d74c1bf7a5bf156d) )
	ROM_LOAD( "a49-06.10d", 0x4000, 0x2000, CRC(f895672e) SHA1(f6ee81663850db025484b39c683ff992a0c84877) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "a49-10.11n", 0x0000, 0x1000, CRC(a7a361ba) SHA1(f142f78ac2434b22240202a9763c921db2d42fdb) ) // characters
	ROM_LOAD( "a49-11.11q", 0x2000, 0x1000, CRC(801902e3) SHA1(abbcc2c6d805ab158f94ef60125eeea4d5041f71) )
	ROM_LOAD( "a49-12.11r", 0x4000, 0x1000, CRC(cef75565) SHA1(9325effa7bd920af4997858bf26ebb01eeb858cd) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "a49-07.12h", 0x0000, 0x4000, CRC(8c749828) SHA1(118057c1364d996f1445214c6aff749d6aa30a22) ) // sprites
	ROM_LOAD( "a49-08.12j", 0x4000, 0x4000, CRC(03c10aed) SHA1(65b28bd724d9812d58ee5e0a4d0b5b0f9c2723b4) )
	ROM_LOAD( "a49-09.12k", 0x8000, 0x4000, CRC(f61316d2) SHA1(b07eec0f4a2fe971ea3f4d56739981aef5ff4d0a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "a49-16.2j",  0x000, 0x0100, CRC(a7b077d4) SHA1(48c3e68d67de067c0ead0dbd34769b755fb5952f) )
	ROM_LOAD( "a49-17.2l",  0x100, 0x0100, CRC(1c04c087) SHA1(7179edf96f59a469353d9652900b99fef25f4054) )
	ROM_LOAD( "a49-18.2m",  0x200, 0x0100, CRC(f5ce3c45) SHA1(f2dcdaf95b55b8fd713bdbb965731c064b4a0757) )
ROM_END

ROM_START( m660 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "660l.bin",    0x0000, 0x4000, CRC(57c0d1cc) SHA1(3d71d0554e445f27f5b57a185acddd58f70e95f4) )
	ROM_LOAD( "660m.bin",    0x4000, 0x4000, CRC(628c6686) SHA1(e695ccfb1251bc7571122de30e682b135e773f20) )
	ROM_LOAD( "660n.bin",    0x8000, 0x4000, CRC(1b418a97) SHA1(a9afa341c790e650fb91b6e9df4959c3bd7ab5c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "14.4n",       0x0000, 0x4000, CRC(5734db5a) SHA1(ff99bf618018be20a4b38fcfbe75d9c5bb6fd176) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "13.4j",       0x0000, 0x4000, CRC(fba51cf7) SHA1(b18571112dcbe3214a803d0898b0a21957dc5e5f) )

	ROM_REGION( 0x10000, "audio3", 0 ) /* Z80 code AY driver */
	ROM_LOAD( "660x.bin",    0x0000, 0x8000, CRC(b82f0cfa) SHA1(7c74f3d57fccc020d3a99cbd676480ea6625b2a1) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "4.7k",        0x0000, 0x4000, CRC(e24e431a) SHA1(b5558550782dc7452dcd50b72390408ffc17e5e7) ) // tiles
	ROM_LOAD( "5.6k",        0x4000, 0x4000, CRC(b2c93d46) SHA1(f81b5aadb2d4af8b3bb4e386fe408aaff7360225) )
	ROM_LOAD( "6.5k",        0x8000, 0x4000, CRC(763c5983) SHA1(d4a62dab71e88d90dbd16ed4e152e1a5a01cf64f) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "660u.bin",    0x0000, 0x2000, CRC(030af716) SHA1(a9c664274a3d6127934273bb5b45a4a374b244dd) ) // characters
	ROM_LOAD( "660v.bin",    0x2000, 0x2000, CRC(51a6e160) SHA1(26689b1de33dde2c6053dbdf44f004cbcc46d292) )
	ROM_LOAD( "660w.bin",    0x4000, 0x2000, CRC(8a45b469) SHA1(7c6d895b23846fcf16285ec542c263b6168a46c0) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "7.15e",       0x0000, 0x4000, CRC(990c0cee) SHA1(20aebf64f62c16069df752124da509359d5d1af2) ) // sprites
	ROM_LOAD( "8.15d",       0x4000, 0x4000, CRC(d9aa7834) SHA1(0921c16a195d9e2d77a1f1591c5e77d87ca65c04) )
	ROM_LOAD( "9.15b",       0x8000, 0x4000, CRC(27b26905) SHA1(58715e5792ec5388e68510b9cd8846fb4fb3caf8) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "4r.bpr",      0x000, 0x100, CRC(cd16d0f1) SHA1(4b0a68f28329fb86d252f4170edd2ab0488805e5) )
	ROM_LOAD( "4p.bpr",      0x100, 0x100, CRC(22e8b22c) SHA1(2934ca96495fca72a33fa2881dc65ab21342c410) )
	ROM_LOAD( "5r.bpr",      0x200, 0x100, CRC(b7d6fdb5) SHA1(67d3bb16470f5d4ec35164a391ad6b65850f824a) )
ROM_END

ROM_START( m660j )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "1.3c",        0x0000, 0x4000, CRC(4c8f96aa) SHA1(cc076bdc7ecc206f7e0b9c17fbba59507f515df1) )
	ROM_LOAD( "2.3d",        0x4000, 0x4000, CRC(e6661504) SHA1(4ec208d49f95f378f3dbeb375e6c220d02a35092) )
	ROM_LOAD( "3.3f",        0x8000, 0x4000, CRC(3a389ccd) SHA1(3d99312d7fa9d269fdd218917ceafdd61890617f) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "14.4n",       0x0000, 0x4000, CRC(5734db5a) SHA1(ff99bf618018be20a4b38fcfbe75d9c5bb6fd176) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "13.4j",       0x0000, 0x4000, CRC(fba51cf7) SHA1(b18571112dcbe3214a803d0898b0a21957dc5e5f) )

	ROM_REGION( 0x10000, "audio3", 0 ) /* Z80 code AY driver */
	ROM_LOAD( "d.4e",        0x0000, 0x4000, CRC(93f3d852) SHA1(074f6899ff9064e29002c830cfae000973d8b89e) )
	ROM_LOAD( "e.4d",        0x4000, 0x4000, CRC(12f5c077) SHA1(3bb3f9c1a84744b8849dd25591ac186e23803542) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "4.7k",        0x0000, 0x4000, CRC(e24e431a) SHA1(b5558550782dc7452dcd50b72390408ffc17e5e7) ) // tiles
	ROM_LOAD( "5.6k",        0x4000, 0x4000, CRC(b2c93d46) SHA1(f81b5aadb2d4af8b3bb4e386fe408aaff7360225) )
	ROM_LOAD( "6.5k",        0x8000, 0x4000, CRC(763c5983) SHA1(d4a62dab71e88d90dbd16ed4e152e1a5a01cf64f) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "a.16j",       0x0000, 0x2000, CRC(06f44c8c) SHA1(b4820a051fcf4f1cc17a4d94cce0e8ab04aaafb5) ) // characters
	ROM_LOAD( "b.16k",       0x2000, 0x2000, CRC(94b8b69f) SHA1(1c28d674023323bcec3ef752b3614b1581c0b551) )
	ROM_LOAD( "c.16m",       0x4000, 0x2000, CRC(d6768c68) SHA1(dadbece88e53cd80de2a9dba8ded694176f432c9) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "7.15e",       0x0000, 0x4000, CRC(990c0cee) SHA1(20aebf64f62c16069df752124da509359d5d1af2) ) // sprites
	ROM_LOAD( "8.15d",       0x4000, 0x4000, CRC(d9aa7834) SHA1(0921c16a195d9e2d77a1f1591c5e77d87ca65c04) )
	ROM_LOAD( "9.15b",       0x8000, 0x4000, CRC(27b26905) SHA1(58715e5792ec5388e68510b9cd8846fb4fb3caf8) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "4r.bpr",      0x000, 0x100, CRC(cd16d0f1) SHA1(4b0a68f28329fb86d252f4170edd2ab0488805e5) )
	ROM_LOAD( "4p.bpr",      0x100, 0x100, CRC(22e8b22c) SHA1(2934ca96495fca72a33fa2881dc65ab21342c410) )
	ROM_LOAD( "5r.bpr",      0x200, 0x100, CRC(b7d6fdb5) SHA1(67d3bb16470f5d4ec35164a391ad6b65850f824a) )
ROM_END

ROM_START( m660b )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "m660-1.bin",  0x0000, 0x4000, CRC(18f6c4be) SHA1(29a66be9216347b40ab0ccb95baf3e0a6207da8e) )
	ROM_LOAD( "2.3d",        0x4000, 0x4000, CRC(e6661504) SHA1(4ec208d49f95f378f3dbeb375e6c220d02a35092) )
	ROM_LOAD( "3.3f",        0x8000, 0x4000, CRC(3a389ccd) SHA1(3d99312d7fa9d269fdd218917ceafdd61890617f) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "14.4n",       0x0000, 0x4000, CRC(5734db5a) SHA1(ff99bf618018be20a4b38fcfbe75d9c5bb6fd176) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "13.4j",       0x0000, 0x4000, CRC(fba51cf7) SHA1(b18571112dcbe3214a803d0898b0a21957dc5e5f) )

	ROM_REGION( 0x10000, "audio3", 0 ) /* Z80 code AY driver */
	ROM_LOAD( "660x.bin",    0x0000, 0x8000, CRC(b82f0cfa) SHA1(7c74f3d57fccc020d3a99cbd676480ea6625b2a1) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "4.7k",        0x0000, 0x4000, CRC(e24e431a) SHA1(b5558550782dc7452dcd50b72390408ffc17e5e7) ) // tiles
	ROM_LOAD( "5.6k",        0x4000, 0x4000, CRC(b2c93d46) SHA1(f81b5aadb2d4af8b3bb4e386fe408aaff7360225) )
	ROM_LOAD( "6.5k",        0x8000, 0x4000, CRC(763c5983) SHA1(d4a62dab71e88d90dbd16ed4e152e1a5a01cf64f) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "m660-10.bin", 0x0000, 0x2000, CRC(b11405a6) SHA1(c093ba567ce95abc4f08c8449e366cd7d2abc8d2) ) // characters
	ROM_LOAD( "b.16k",       0x2000, 0x2000, CRC(94b8b69f) SHA1(1c28d674023323bcec3ef752b3614b1581c0b551) )
	ROM_LOAD( "c.16m",       0x4000, 0x2000, CRC(d6768c68) SHA1(dadbece88e53cd80de2a9dba8ded694176f432c9) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "7.15e",       0x0000, 0x4000, CRC(990c0cee) SHA1(20aebf64f62c16069df752124da509359d5d1af2) ) // sprites
	ROM_LOAD( "8.15d",       0x4000, 0x4000, CRC(d9aa7834) SHA1(0921c16a195d9e2d77a1f1591c5e77d87ca65c04) )
	ROM_LOAD( "9.15b",       0x8000, 0x4000, CRC(27b26905) SHA1(58715e5792ec5388e68510b9cd8846fb4fb3caf8) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "4r.bpr",      0x000, 0x100, CRC(cd16d0f1) SHA1(4b0a68f28329fb86d252f4170edd2ab0488805e5) )
	ROM_LOAD( "4p.bpr",      0x100, 0x100, CRC(22e8b22c) SHA1(2934ca96495fca72a33fa2881dc65ab21342c410) )
	ROM_LOAD( "5r.bpr",      0x200, 0x100, CRC(b7d6fdb5) SHA1(67d3bb16470f5d4ec35164a391ad6b65850f824a) )
ROM_END

ROM_START( alphaxz )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "az-01.bin",   0x0000, 0x4000, CRC(5336f842) SHA1(cf43c87fad9131120ac75dfd4e2aec260611af7b) )
	ROM_LOAD( "az-02.bin",   0x4000, 0x4000, CRC(a0779b6b) SHA1(146c967253031d2bbbdbc49b5854c0676e458af1) )
	ROM_LOAD( "az-03.bin",   0x8000, 0x4000, CRC(2797bc7b) SHA1(137ef537917a9f243208d2befee0c09a36782647) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "14.4n",       0x0000, 0x4000, CRC(5734db5a) SHA1(ff99bf618018be20a4b38fcfbe75d9c5bb6fd176) )

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "13.4j",       0x0000, 0x4000, CRC(fba51cf7) SHA1(b18571112dcbe3214a803d0898b0a21957dc5e5f) )

	ROM_REGION( 0x10000, "audio3", 0 ) /* Z80 code AY driver */
	ROM_LOAD( "660x.bin",    0x0000, 0x8000, CRC(b82f0cfa) SHA1(7c74f3d57fccc020d3a99cbd676480ea6625b2a1) )

	ROM_REGION( 0xc000, "gfx1", 0 )
	ROM_LOAD( "az-04.bin",   0x00000, 0x4000, CRC(23da4e3d) SHA1(fd48c86769360a778057a673d5c1dfdfe00f6c18) ) // tiles
	ROM_LOAD( "az-05.bin",   0x04000, 0x4000, CRC(8746ff69) SHA1(6b95f209d931601155ad492652e3016df3e5dfb0) )
	ROM_LOAD( "az-06.bin",   0x08000, 0x4000, CRC(6e494964) SHA1(9f42935281502a1e73f602cb93ca241aeaa03201) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "az-10.bin",   0x00000, 0x2000, CRC(10b499bb) SHA1(270702d2ba0313548b78ae3967edb10cf6b80610) ) // characters
	ROM_LOAD( "az-11.bin",   0x02000, 0x2000, CRC(d91993f6) SHA1(250e3ba1e694fedf4152c86f2ce88593866160f2) )
	ROM_LOAD( "az-12.bin",   0x04000, 0x2000, CRC(8ea48ef3) SHA1(f6a5477d28ab2d2bac655a02f96f31b8db9c44f8) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "az-07.bin",   0x00000, 0x4000, CRC(5f9cc65e) SHA1(235b4a8762ba429855cc9db07477efde2a62e03d) ) // sprites
	ROM_LOAD( "az-08.bin",   0x04000, 0x4000, CRC(23e3a6ba) SHA1(d6d035b7b7530a909669ac045aea51e297ba784e) )
	ROM_LOAD( "az-09.bin",   0x08000, 0x4000, CRC(7096fa71) SHA1(f9697b30d7eec5ee9122d783f9ee7b9cdbab9262) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "4r.bpr",      0x000, 0x100, CRC(cd16d0f1) SHA1(4b0a68f28329fb86d252f4170edd2ab0488805e5) )
	ROM_LOAD( "4p.bpr",      0x100, 0x100, CRC(22e8b22c) SHA1(2934ca96495fca72a33fa2881dc65ab21342c410) )
	ROM_LOAD( "5r.bpr",      0x200, 0x100, CRC(b7d6fdb5) SHA1(67d3bb16470f5d4ec35164a391ad6b65850f824a) )
ROM_END


/*
This version of the game is the original which was tested in small quantities in Taito owned arcades.
Taito lost the license to sell the title which Kaneko sold to Wood Place.
Same hardware as Samurai Nihon-ichi.
*/

ROM_START( the26thz ) // there's a protection device labeled 6 or 9 on the main board (seems to be the same as the tsamurai one)
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "a72_01.3r",   0x0000, 0x4000, CRC(2be77520) SHA1(26c65a9c59a0cf3609d084ec4e0af4e670a3069c) ) // 27128
	ROM_LOAD( "a72_02.3t",   0x4000, 0x4000, CRC(ef2646f2) SHA1(0b178793420bd5e5fb77cf55e049a73f0fbc2d0f) ) // 27128
	ROM_LOAD( "a72_03.3v",   0x8000, 0x4000, CRC(d83b7758) SHA1(ff025c67e7ace9b178fd46642f98d79e18076d28) ) // 27128

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "a72_16.4n",       0x0000, 0x4000, CRC(5734db5a) SHA1(ff99bf618018be20a4b38fcfbe75d9c5bb6fd176) ) // 27128

	ROM_REGION( 0x10000, "audio2", 0 ) /* Z80 code - sample player */
	ROM_LOAD( "a72_15.4j",       0x0000, 0x4000, CRC(fba51cf7) SHA1(b18571112dcbe3214a803d0898b0a21957dc5e5f) ) // 27128

	ROM_REGION( 0x10000, "audio3", 0 ) /* Z80 code AY driver, identical to the parent set but split in 2 ROMs */
	ROM_LOAD( "a72_14.4e",   0x0000, 0x4000, CRC(d078373e) SHA1(939c8c1eb651d34b3c873db3aeef4c59b4f70a9f) ) // 27128
	ROM_LOAD( "a72_13.4d",   0x4000, 0x4000, CRC(11980449) SHA1(e17611980043c152c518b334ffda8e93d368e4a9) ) // 27128

	ROM_REGION( 0xc000, "gfx1", 0 ) // tiles
	ROM_LOAD( "a72_04.10a",   0x00000, 0x4000, CRC(23da4e3d) SHA1(fd48c86769360a778057a673d5c1dfdfe00f6c18) ) // 27128
	ROM_LOAD( "a72_05.10b",   0x04000, 0x4000, CRC(8746ff69) SHA1(6b95f209d931601155ad492652e3016df3e5dfb0) ) // 27128
	ROM_LOAD( "a72_06.10d",   0x08000, 0x4000, CRC(6e494964) SHA1(9f42935281502a1e73f602cb93ca241aeaa03201) ) // 27128

	ROM_REGION( 0x6000, "gfx2", 0 ) // characters
	ROM_LOAD( "a72_10.11n",   0x00000, 0x1000, CRC(a23e4829) SHA1(329ff6bd21c804c103938e333b355c7387b5d0be) ) // 2732
	ROM_LOAD( "a72_11.11q",   0x02000, 0x1000, CRC(9717229f) SHA1(ecc1bbde29fbd28eebc2016ca5e04bc841ca061e) ) // 2732
	ROM_LOAD( "a72_12.11r",   0x04000, 0x1000, CRC(7a602979) SHA1(32c40ad1cb1b6bb49c6f37da443a4368fd24faf7) ) // 2732

	ROM_REGION( 0xc000, "gfx3", 0 ) // sprites
	ROM_LOAD( "a72_07.12h",   0x00000, 0x4000, CRC(5f9cc65e) SHA1(235b4a8762ba429855cc9db07477efde2a62e03d) ) // 27128
	ROM_LOAD( "a72_08.12j",   0x04000, 0x4000, CRC(23e3a6ba) SHA1(d6d035b7b7530a909669ac045aea51e297ba784e) ) // 27128
	ROM_LOAD( "a72_09.12k",   0x08000, 0x4000, CRC(7096fa71) SHA1(f9697b30d7eec5ee9122d783f9ee7b9cdbab9262) ) // 27128

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "a72_17.2k",      0x000, 0x100, CRC(cd16d0f1) SHA1(4b0a68f28329fb86d252f4170edd2ab0488805e5) ) // 82S129
	ROM_LOAD( "a72_18.2l",      0x100, 0x100, CRC(22e8b22c) SHA1(2934ca96495fca72a33fa2881dc65ab21342c410) ) // 82S129
	ROM_LOAD( "a72_19.2m",      0x200, 0x100, CRC(b7d6fdb5) SHA1(67d3bb16470f5d4ec35164a391ad6b65850f824a) ) // 82S129
ROM_END

ROM_START( vsgongf )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "1.5a",   0x0000, 0x2000, CRC(2c056dee) SHA1(f063fdd571949a1b7ac36f88e17feec7354ea894) ) /* good? */
	ROM_LOAD( "2",      0x2000, 0x2000, CRC(1a634daf) SHA1(d282fbb2ca2c8db70cbbbf640ce507d4c142cc39) ) /* good? */
	ROM_LOAD( "3.5d",   0x4000, 0x2000, CRC(5ac16861) SHA1(2af51811285fb2de44b023872e42aae37bfbf105) )
	ROM_LOAD( "4.5f",   0x6000, 0x2000, CRC(1d1baf7b) SHA1(b05d3d7bca299c219a02966b3af2ac517472d0a5) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sound CPU */
	ROM_LOAD( "6.5n",   0x0000, 0x2000, CRC(785b9000) SHA1(9eb32724b0611a93959485a7f9c806eb6d7ab013) )
	ROM_LOAD( "5.5l",   0x2000, 0x2000, CRC(76dbfde9) SHA1(fe6e02f4b5a0a5baa59506636226c8ea7b551ef6) )

	ROM_REGION( 0x6000, "gfx1", ROMREGION_ERASEFF ) /* tiles (N/A) */

	ROM_REGION( 0x3000, "gfx2", 0 ) /* characters */
	ROM_LOAD( "7.6f",   0x0000, 0x1000, CRC(6ec68692) SHA1(9c0742749ca71c888abbc7eb7ed8a538a9465ed2) )
	ROM_LOAD( "8.7f",   0x1000, 0x1000, CRC(afba16c8) SHA1(bfa03d95e8c4372efe2864b423bf32cda79760ce) )
	ROM_LOAD( "9.8f",   0x2000, 0x1000, CRC(536bf710) SHA1(43f653b21deac58b66b9df267ea44cbd99aff694) )

	ROM_REGION( 0x6000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "13.15j",  0x0000, 0x2000, CRC(a2451a31) SHA1(e416d8c5ae18596b2619618b4666fa306204ca71) )
	ROM_LOAD( "14.15h",  0x2000, 0x2000, CRC(b387403e) SHA1(7aff3d7c5a9861f3e30244706ac303fbc240b4bd) )
	ROM_LOAD( "15.15f",  0x4000, 0x2000, CRC(0e649334) SHA1(31f22b511c7b73056139f17a30012cdc0a0c7d52) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "clr.6s",  0x000, 0x0100, CRC(578bfbea) SHA1(12a97de0f8012ccce75e14abf743bcec6857684c) )
	ROM_LOAD( "clr.6r",  0x100, 0x0100, CRC(3ec00739) SHA1(614d1799fe197df389f6155f86fe113e1b0b018a) )
	ROM_LOAD( "clr.6p",  0x200, 0x0100, CRC(0e4fd17a) SHA1(d4e32bd9dd903177af61f77976a25c5db1467bba) )
ROM_END


ROM_START( ringfgt )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "rft_04-1.5a", 0x0000, 0x2000, CRC(11030866) SHA1(b95b231c241e5bdc002de3f6a732cd627c7dc145) )
	ROM_LOAD( "rft_03-1.5c", 0x2000, 0x2000, CRC(357a2085) SHA1(0534f6c1a876dacfcff09a547290354eeddb3126) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sound CPU */
	ROM_LOAD( "rft_01.5n", 0x0000, 0x2000, CRC(785b9000) SHA1(9eb32724b0611a93959485a7f9c806eb6d7ab013) )
	ROM_LOAD( "rft_02.5l", 0x2000, 0x2000, CRC(76dbfde9) SHA1(fe6e02f4b5a0a5baa59506636226c8ea7b551ef6) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_ERASEFF ) /* tiles (N/A) */

	ROM_REGION( 0x6000, "gfx2", 0 ) /* characters */
	ROM_LOAD( "rft_05.6f", 0x0000, 0x1000, CRC(a7b732fd) SHA1(2abe4b422a7cd32cd12c6d6acba1872afe4a2ecc) )
	ROM_LOAD( "rft_06.7f", 0x2000, 0x1000, CRC(ff2721f7) SHA1(ae75103a7663a190da36e0bb2d46a333f830eba5) )
	ROM_LOAD( "rft_07.8f", 0x4000, 0x1000, CRC(ec1d7ba4) SHA1(047aa3c6c92126ac623fddbe0adc50a450910d6e) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "rft_08.15j", 0x0000, 0x2000, CRC(80d67d28) SHA1(f4016159201abdfe0c0441f78ed6a1a12b7ba34b) )
	ROM_LOAD( "rft_09.15h", 0x4000, 0x2000, CRC(ea8f0656) SHA1(c4d2302d046c41bae9946a5ddc0087349a421b35) )
	ROM_LOAD( "rft_10.15f", 0x8000, 0x2000, CRC(833ca89f) SHA1(b43416e783e6109d19de81a124fae84f1afb8440) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "rft-11.6s", 0x000, 0x0100, CRC(578bfbea) SHA1(12a97de0f8012ccce75e14abf743bcec6857684c) )
	ROM_LOAD( "rft-12.6r", 0x100, 0x0100, CRC(3ec00739) SHA1(614d1799fe197df389f6155f86fe113e1b0b018a) )
	ROM_LOAD( "rft-13.6p", 0x200, 0x0100, CRC(0e4fd17a) SHA1(d4e32bd9dd903177af61f77976a25c5db1467bba) )
ROM_END

ROM_START( ringfgt2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code  - main CPU */
	ROM_LOAD( "rft_04.5a", 0x0000, 0x2000, CRC(6b9b3f3d) SHA1(ea75e77e0e3379a22381b1d0aae7f96b53cd7562) )
	ROM_LOAD( "rft_03.5c", 0x2000, 0x2000, CRC(1821974b) SHA1(1ce52f20bf49c111000f870bbe3416d27673b91d) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code - sound CPU */
	ROM_LOAD( "rft_01.5n", 0x0000, 0x2000, CRC(785b9000) SHA1(9eb32724b0611a93959485a7f9c806eb6d7ab013) )
	ROM_LOAD( "rft_02.5l", 0x2000, 0x2000, CRC(76dbfde9) SHA1(fe6e02f4b5a0a5baa59506636226c8ea7b551ef6) )

	ROM_REGION( 0xc000, "gfx1", ROMREGION_ERASEFF ) /* tiles (N/A) */

	ROM_REGION( 0x6000, "gfx2", 0 ) /* characters */
	ROM_LOAD( "rft_05.6f", 0x0000, 0x1000, CRC(a7b732fd) SHA1(2abe4b422a7cd32cd12c6d6acba1872afe4a2ecc) )
	ROM_LOAD( "rft_06.7f", 0x2000, 0x1000, CRC(ff2721f7) SHA1(ae75103a7663a190da36e0bb2d46a333f830eba5) )
	ROM_LOAD( "rft_07.8f", 0x4000, 0x1000, CRC(ec1d7ba4) SHA1(047aa3c6c92126ac623fddbe0adc50a450910d6e) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* sprites */
	ROM_LOAD( "rft_08.15j", 0x0000, 0x2000, CRC(80d67d28) SHA1(f4016159201abdfe0c0441f78ed6a1a12b7ba34b) )
	ROM_LOAD( "rft_09.15h", 0x4000, 0x2000, CRC(ea8f0656) SHA1(c4d2302d046c41bae9946a5ddc0087349a421b35) )
	ROM_LOAD( "rft_10.15f", 0x8000, 0x2000, CRC(833ca89f) SHA1(b43416e783e6109d19de81a124fae84f1afb8440) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "rft-11.6s", 0x000, 0x0100, CRC(578bfbea) SHA1(12a97de0f8012ccce75e14abf743bcec6857684c) )
	ROM_LOAD( "rft-12.6r", 0x100, 0x0100, CRC(3ec00739) SHA1(614d1799fe197df389f6155f86fe113e1b0b018a) )
	ROM_LOAD( "rft-13.6p", 0x200, 0x0100, CRC(0e4fd17a) SHA1(d4e32bd9dd903177af61f77976a25c5db1467bba) )
ROM_END

void m660_state::init_the26thz()
{
	m_maincpu->space(AS_PROGRAM).unmap_read(0xd803, 0xd803);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd803, 0xd803, read8smo_delegate(*this, FUNC(m660_state::tsamurai_unknown_d803_r)));
}

GAME( 1984, vsgongf,   0,        vsgongf,  vsgongf,   vsgongf_state, empty_init,    ROT90, "Kaneko", "VS Gong Fight", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION )
GAME( 1984, ringfgt,   vsgongf,  vsgongf,  vsgongf,   vsgongf_state, empty_init,    ROT90, "Kaneko (Taito license)", "Ring Fighter (set 1)", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, ringfgt2,  vsgongf,  vsgongf,  vsgongf,   vsgongf_state, empty_init,    ROT90, "Kaneko (Taito license)", "Ring Fighter (set 2)", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1985, tsamurai,  0,        tsamurai, tsamurai,  tsamurai_state, empty_init,    ROT90, "Kaneko / Taito", "Samurai Nihon-Ichi (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tsamurai2, tsamurai, tsamurai, tsamurai,  tsamurai_state, empty_init,    ROT90, "Kaneko / Taito", "Samurai Nihon-Ichi (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tsamuraih, tsamurai, tsamurai, tsamuraih, tsamurai_state, empty_init,    ROT90, "bootleg", "Samurai Nihon-Ichi (bootleg, harder)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, ladymstr,  0,        tsamurai, ladymstr,  tsamurai_state, empty_init,    ROT90, "Kaneko / Taito", "Lady Master of Kung Fu (set 1, newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ladymstr2, ladymstr, tsamurai, ladymstr,  tsamurai_state, empty_init,    ROT90, "Kaneko / Taito", "Lady Master of Kung Fu (set 2, older)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, nunchaku,  ladymstr, tsamurai, nunchaku,  tsamurai_state, empty_init,    ROT90, "Kaneko / Taito", "Nunchackun", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1985, yamagchi,  0,        tsamurai, yamagchi,  tsamurai_state, empty_init,    ROT90, "Kaneko / Taito", "Go Go Mr. Yamaguchi / Yuke Yuke Yamaguchi-kun", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1986, m660,      0,        m660,     m660,      m660_state, empty_init,    ROT90, "Wood Place Co. Ltd. (Taito America Corporation license)", "Mission 660 (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, m660j,     m660,     m660,     m660,      m660_state, empty_init,    ROT90, "Wood Place Co. Ltd. (Taito Corporation license)", "Mission 660 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, m660b,     m660,     m660,     m660,      m660_state, empty_init,    ROT90, "bootleg", "Mission 660 (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, alphaxz,   0,        m660,     m660,      m660_state, empty_init,    ROT90, "Ed Co., Ltd. (Wood Place Co., Ltd. license)", "The Alphax Z (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, the26thz,  alphaxz,  m660,     m660,      m660_state, init_the26thz, ROT90, "Ed Co., Ltd. (Taito license)", "The 26th Z (Japan, location test)", MACHINE_SUPPORTS_SAVE )
