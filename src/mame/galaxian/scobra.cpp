// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

 Super Cobra hardware

NOTE:  Eventually to be merged into galaxian.cpp

TODO:
----

- Dark Planet background graphics

- Explosion sound in Scramble/Super Cobra repeats

- Armored Car probably has some other effect(s) during an explosion.
  It uses both POUT1 and POUT2.


Notes/Tidbits:
-------------

- Armored Car sets Port C as well, but it's input only and the games uses other
  bits for the 2nd player controls.  Maybe the games was meant to use 2 joysticks
  at one time.

- Video Hustler and its two bootlegs all have identical code, the only
  differences are the title, copyright removed, different encryptions or
  no encryption, plus hustlerb has a different memory map.

- In Tazmania and clones, when set to Upright mode, player 2 left skips the current
  level

***************************************************************************/

#include "emu.h"
#include "scramble.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "speaker.h"


namespace {

class scobra_state : public scramble_state
{
public:
	scobra_state(const machine_config &mconfig, device_type type, const char *tag)
		: scramble_state(mconfig, type, tag)
		, m_soundram(*this, "soundram")
	{ }

	void mimonkeyug(machine_config &config);
	void stratgyx(machine_config &config);
	void type1(machine_config &config);
	void type2(machine_config &config);
	void rescueb(machine_config &config);
	void minefldfe(machine_config &config);
	void hustlerb4(machine_config &config);
	void hustlerb6(machine_config &config);
	void minefld(machine_config &config);
	void hustler(machine_config &config);
	void rescue(machine_config &config);
	void darkplnt(machine_config &config);
	void tazmani3(machine_config &config);
	void hustlerb(machine_config &config);
	void rescuefe(machine_config &config);

	template <int Mask> int stratgyx_coinage_r();

private:
	uint8_t scobra_soundram_r(offs_t offset);
	void scobra_soundram_w(offs_t offset, uint8_t data);
	uint8_t scobra_type2_ppi8255_0_r(offs_t offset);
	uint8_t scobra_type2_ppi8255_1_r(offs_t offset);
	uint8_t hustler_ppi8255_0_r(offs_t offset);
	uint8_t hustler_ppi8255_1_r(offs_t offset);
	void scobra_type2_ppi8255_0_w(offs_t offset, uint8_t data);
	void scobra_type2_ppi8255_1_w(offs_t offset, uint8_t data);
	void hustler_ppi8255_0_w(offs_t offset, uint8_t data);
	void hustler_ppi8255_1_w(offs_t offset, uint8_t data);

	void hustler_map(address_map &map) ATTR_COLD;
	void hustler_sound_io_map(address_map &map) ATTR_COLD;
	void hustler_sound_map(address_map &map) ATTR_COLD;
	void hustlerb_map(address_map &map) ATTR_COLD;
	void hustlerb_sound_io_map(address_map &map) ATTR_COLD;
	void hustlerb_sound_map(address_map &map) ATTR_COLD;
	void hustlerb6_map(address_map &map) ATTR_COLD;
	void mimonkeyug_map(address_map &map) ATTR_COLD;
	void minefldfe_map(address_map &map) ATTR_COLD;
	void rescuefe_map(address_map &map) ATTR_COLD;
	void scobra_sound_io_map(address_map &map) ATTR_COLD;
	void scobra_sound_map(address_map &map) ATTR_COLD;
	void tazmani3_map(address_map &map) ATTR_COLD;
	void type1_map(address_map &map) ATTR_COLD;
	void type2_map(address_map &map) ATTR_COLD;

	optional_shared_ptr<uint8_t> m_soundram;
};


static const gfx_layout scobra_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout scobra_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};


static GFXDECODE_START( gfx_scobra )
	GFXDECODE_ENTRY( "gfx1", 0x0000, scobra_charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, scobra_spritelayout, 0, 8 )
GFXDECODE_END


uint8_t scobra_state::scobra_type2_ppi8255_0_r(offs_t offset) { return m_ppi8255_0->read(offset >> 2); }
uint8_t scobra_state::scobra_type2_ppi8255_1_r(offs_t offset) { return m_ppi8255_1->read(offset >> 2); }
void scobra_state::scobra_type2_ppi8255_0_w(offs_t offset, uint8_t data) { m_ppi8255_0->write(offset >> 2, data); }
void scobra_state::scobra_type2_ppi8255_1_w(offs_t offset, uint8_t data) { m_ppi8255_1->write(offset >> 2, data); }

uint8_t scobra_state::hustler_ppi8255_0_r(offs_t offset) { return m_ppi8255_0->read(offset >> 3); }
uint8_t scobra_state::hustler_ppi8255_1_r(offs_t offset) { return m_ppi8255_1->read(offset >> 3); }
void scobra_state::hustler_ppi8255_0_w(offs_t offset, uint8_t data) { m_ppi8255_0->write(offset >> 3, data); }
void scobra_state::hustler_ppi8255_1_w(offs_t offset, uint8_t data) { m_ppi8255_1->write(offset >> 3, data); }

void scobra_state::type1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8bff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram").mirror(0x0400);
	map(0x9000, 0x903f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram");
	map(0x9040, 0x905f).ram().share("spriteram");
	map(0x9060, 0x907f).ram().share("bulletsram");
	map(0x9080, 0x90ff).ram();
	map(0x9800, 0x9803).rw(m_ppi8255_0, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa000, 0xa003).rw(m_ppi8255_1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa801, 0xa801).w(FUNC(scobra_state::galaxold_nmi_enable_w));
	map(0xa802, 0xa802).w(FUNC(scobra_state::galaxold_coin_counter_w));
	map(0xa804, 0xa804).w(FUNC(scobra_state::galaxold_stars_enable_w));
	map(0xa806, 0xa806).w(FUNC(scobra_state::galaxold_flip_screen_x_w));
	map(0xa807, 0xa807).w(FUNC(scobra_state::galaxold_flip_screen_y_w));
	map(0xb000, 0xb000).r("watchdog", FUNC(watchdog_timer_device::reset_r));
}

void scobra_state::type2_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x883f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram");
	map(0x8840, 0x885f).ram().share("spriteram");
	map(0x8860, 0x887f).ram().share("bulletsram");
	map(0x8880, 0x88ff).ram();
	map(0x9000, 0x93ff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram").mirror(0x0400);
	map(0x9800, 0x9800).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xa000, 0xa00f).rw(FUNC(scobra_state::scobra_type2_ppi8255_0_r), FUNC(scobra_state::scobra_type2_ppi8255_0_w));
	map(0xa800, 0xa80f).rw(FUNC(scobra_state::scobra_type2_ppi8255_1_r), FUNC(scobra_state::scobra_type2_ppi8255_1_w));
	map(0xb000, 0xb000).w(FUNC(scobra_state::galaxold_stars_enable_w));
	map(0xb004, 0xb004).w(FUNC(scobra_state::galaxold_nmi_enable_w));
	map(0xb006, 0xb006).w(FUNC(scobra_state::galaxold_coin_counter_0_w));
	map(0xb008, 0xb008).w(FUNC(scobra_state::galaxold_coin_counter_1_w));
	map(0xb00c, 0xb00c).w(FUNC(scobra_state::galaxold_flip_screen_y_w));
	map(0xb00e, 0xb00e).w(FUNC(scobra_state::galaxold_flip_screen_x_w));
}

void scobra_state::tazmani3_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x883f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram");
	map(0x8840, 0x885f).ram().share("spriteram");
	map(0x8860, 0x887f).ram().share("bulletsram");
	map(0x8880, 0x88ff).ram();
	map(0x9000, 0x93ff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram").mirror(0x0400);
	map(0x9800, 0x9800).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xa000, 0xa003).rw(m_ppi8255_0, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa800, 0xa803).rw(m_ppi8255_1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb000, 0xb000).w(FUNC(scobra_state::galaxold_stars_enable_w));
	map(0xb001, 0xb001).w(FUNC(scobra_state::galaxold_nmi_enable_w));
	map(0xb006, 0xb006).w(FUNC(scobra_state::galaxold_coin_counter_0_w));
	map(0xb008, 0xb008).w(FUNC(scobra_state::galaxold_coin_counter_1_w));
	map(0xb00c, 0xb00c).w(FUNC(scobra_state::galaxold_flip_screen_y_w));
	map(0xb00e, 0xb00e).w(FUNC(scobra_state::galaxold_flip_screen_x_w));
}

void scobra_state::hustler_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8bff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram");
	map(0x9000, 0x903f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram");
	map(0x9040, 0x905f).ram().share("spriteram");
	map(0x9060, 0x907f).ram().share("bulletsram");
	map(0x9080, 0x90ff).ram();
	map(0xa802, 0xa802).w(FUNC(scobra_state::galaxold_flip_screen_x_w));
	map(0xa804, 0xa804).w(FUNC(scobra_state::galaxold_nmi_enable_w));
	map(0xa806, 0xa806).w(FUNC(scobra_state::galaxold_flip_screen_y_w));
	map(0xa80e, 0xa80e).nopw();    /* coin counters */
	map(0xb800, 0xb800).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xd000, 0xd01f).rw(FUNC(scobra_state::hustler_ppi8255_0_r), FUNC(scobra_state::hustler_ppi8255_0_w));
	map(0xe000, 0xe01f).rw(FUNC(scobra_state::hustler_ppi8255_1_r), FUNC(scobra_state::hustler_ppi8255_1_w));
}

void scobra_state::hustlerb_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8bff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram");
	map(0x9000, 0x903f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram");
	map(0x9040, 0x905f).ram().share("spriteram");
	map(0x9060, 0x907f).ram().share("bulletsram");
	map(0x9080, 0x90ff).ram();
	map(0xa801, 0xa801).w(FUNC(scobra_state::galaxold_nmi_enable_w));
	map(0xa802, 0xa802).nopw();    /* coin counters */
	map(0xa806, 0xa806).w(FUNC(scobra_state::galaxold_flip_screen_y_w));
	map(0xa807, 0xa807).w(FUNC(scobra_state::galaxold_flip_screen_x_w));
	map(0xb000, 0xb000).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xc100, 0xc103).rw(m_ppi8255_0, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc200, 0xc203).rw(m_ppi8255_1, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void scobra_state::hustlerb6_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4800, 0x4800).portr("IN0").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x4801, 0x4801).portr("IN1");
	map(0x4803, 0x4803).portr("IN2");
	map(0x5000, 0x5000).w(FUNC(scramble_state::scramble_sh_irqtrigger_w));
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8bff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram");
	map(0x9000, 0x903f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram");
	map(0x9040, 0x905f).ram().share("spriteram");
	map(0x9060, 0x907f).ram().share("bulletsram");
	map(0x9080, 0x90ff).ram();
	map(0xa801, 0xa801).w(FUNC(scobra_state::galaxold_nmi_enable_w));
	map(0xa802, 0xa802).w(FUNC(scobra_state::galaxold_flip_screen_x_w));
	map(0xa806, 0xa806).w(FUNC(scobra_state::galaxold_flip_screen_y_w));
	map(0xa808, 0xa808).nopw();    /* coin counters */
	map(0xb800, 0xb800).lr8(NAME([] () -> uint8_t { return 0xff; })); // doesn't boot with 0x00
}

void scobra_state::mimonkeyug_map(address_map &map)
{
	map(0x0000, 0x47ff).rom();
	map(0x6000, 0x6000).portr("IN0"); // ok
	map(0x6800, 0x6800).portr("IN1");
	map(0x7000, 0x7000).portr("IN2");
	map(0x8000, 0x87ff).ram(); // ok
	map(0x8800, 0x8bff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram").mirror(0x0400);
	map(0x9000, 0x903f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram");
	map(0x9040, 0x905f).ram().share("spriteram");
	map(0x9060, 0x907f).ram().share("bulletsram");
	map(0x9080, 0x90ff).ram();
	map(0xa800, 0xa802).w(FUNC(scobra_state::galaxold_gfxbank_w)); // ok
	map(0xa801, 0xa801).w(FUNC(scobra_state::galaxold_nmi_enable_w)); // ok
	map(0xa804, 0xa804).w(FUNC(scobra_state::scrambold_background_enable_w));
	map(0xa806, 0xa806).w(FUNC(scobra_state::galaxold_flip_screen_x_w)); // ok
	map(0xa807, 0xa807).w(FUNC(scobra_state::galaxold_flip_screen_y_w)); // ok
	map(0xb000, 0xb000).r("watchdog", FUNC(watchdog_timer_device::reset_r)); // ok
	map(0xc000, 0xffff).rom();
}

// weird address map like anteateruk in galaxian.cpp (also a free enterprise set)
void scobra_state::rescuefe_map(address_map &map)
{
	map(0x0000, 0x05ff).rom();
	map(0x0600, 0x0fff).ram(); // sets stack here
	map(0x1000, 0x13ff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram");
	map(0x1400, 0x143f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram");
	map(0x1440, 0x145f).ram().share("spriteram");
	map(0x1460, 0x147f).ram().share("bulletsram");
	map(0x1480, 0x14ff).ram();

	map(0x4400, 0x5dff).rom();
	map(0x5e00, 0x6bff).rom(); // some right?

	map(0x8200, 0x93ff).rom(); // wrong? (maybe some correct?)

	map(0xa600, 0xa6ff).rom(); // wrong (taunt string) jumps to around a600 so something must map in this area?

	map(0xEA01, 0xEA01).w(FUNC(scobra_state::galaxold_nmi_enable_w));
	map(0xEA93, 0xEA93).w(FUNC(scobra_state::scrambold_background_enable_w));
	map(0xEA86, 0xEA86).w(FUNC(scobra_state::galaxold_flip_screen_x_w));
	map(0xEA87, 0xEA87).w(FUNC(scobra_state::galaxold_flip_screen_y_w));

	// does it have a real ppi8255?
	map(0xC190, 0xC193).r(m_ppi8255_0, FUNC(i8255_device::read)); // ?
	map(0xC160, 0xC163).r(m_ppi8255_0, FUNC(i8255_device::read)); // ?

	map(0xC4Ac, 0xC4AF).r(m_ppi8255_1, FUNC(i8255_device::read)); // ?

	map(0xc180, 0xc183).w(m_ppi8255_0, FUNC(i8255_device::write)); // correct based on main set?
	map(0xc220, 0xc223).w(m_ppi8255_1, FUNC(i8255_device::write)); // ^

	// addresses below are WRONG, just moved to keep things out the way while the rom mapping is figured out
//  map(0xf802, 0xf802).w(FUNC(scobra_state::galaxold_coin_counter_w));
//  map(0xf000, 0xf000).r("watchdog", FUNC(watchdog_timer_device::reset_r));
}

void scobra_state::minefldfe_map(address_map &map)
{
	map(0x0000, 0x0bff).rom(); // ok
	map(0x0c00, 0x0c3f).ram().w(FUNC(scobra_state::galaxold_attributesram_w)).share("attributesram"); // ok
	map(0x0c40, 0x0c5f).ram().share("spriteram"); // ok
	map(0x0c60, 0x0c7f).ram().share("bulletsram"); // ok
	map(0x0c80, 0x17ff).ram();

	map(0x2200, 0x3fff).rom(); // ok
	map(0x5800, 0x6dff).rom();
	map(0x8a00, 0x91ff).rom();

	map(0x4200, 0x45ff).ram().w(FUNC(scobra_state::galaxold_videoram_w)).share("videoram"); // ok

	map(0x7621, 0x7621).w(FUNC(scobra_state::galaxold_nmi_enable_w)); // a801
	map(0x7673, 0x7673).w(FUNC(scobra_state::scrambold_background_enable_w)); // a083
	map(0x7704, 0x7704).w(FUNC(scobra_state::galaxold_stars_enable_w)); // a804
	map(0x7616, 0x7616).w(FUNC(scobra_state::galaxold_flip_screen_x_w)); // a806
	map(0x7617, 0x7617).w(FUNC(scobra_state::galaxold_flip_screen_y_w)); // a807

	map(0x4C00, 0x4C03).mirror(0x00fc).rw(m_ppi8255_1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x4B00, 0x4B03).mirror(0x00fc).rw(m_ppi8255_0, FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0x1D98, 0x1D98).r("watchdog", FUNC(watchdog_timer_device::reset_r)); // 0xb000

	// addresses below are WRONG, just moved to keep things out the way while the rom mapping is figured out
	map(0xf802, 0xf802).w(FUNC(scobra_state::galaxold_coin_counter_w));
}


uint8_t scobra_state::scobra_soundram_r(offs_t offset)
{
	return m_soundram[offset & 0x03ff];
}

void scobra_state::scobra_soundram_w(offs_t offset, uint8_t data)
{
	m_soundram[offset & 0x03ff] = data;
}

void scobra_state::scobra_sound_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x8000, 0x8fff).rw(FUNC(scobra_state::scobra_soundram_r), FUNC(scobra_state::scobra_soundram_w)).share("soundram");
	map(0x9000, 0x9fff).nopw(); // w(FUNC(scobra_state::scramble_filter_w)); - scobra doesn't instantiate any RC filters. This is dead code!
}


void scobra_state::scobra_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).w("ay1", FUNC(ay8910_device::address_w));
	map(0x20, 0x20).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x40, 0x40).w("ay2", FUNC(ay8910_device::address_w));
	map(0x80, 0x80).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
}

void scobra_state::hustler_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x6000, 0x6fff).nopw(); // w(FUNC(scobra_state::frogger_filter_w)); - hustler doesn't instantiate any RC filters. This is dead code!
}

void scobra_state::hustler_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x80, 0x80).w("aysnd", FUNC(ay8910_device::address_w));
}


void scobra_state::hustlerb_sound_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x6000, 0x6fff).nopw(); // w(FUNC(scobra_state::frogger_filter_w)); - hustlerb doesn't instantiate any RC filters. This is dead code!
	map(0x8000, 0x8fff).ram().r(FUNC(scobra_state::scobra_soundram_r)).share("soundram");  /* only here to initialize pointer */
}

void scobra_state::hustlerb_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x80, 0x80).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
}


/* stratgyx coinage DIPs are spread across two input ports */
template <int Mask>
int scobra_state::stratgyx_coinage_r()
{
	return (ioport("IN4")->read() & Mask) ? 1 : 0;
}


static INPUT_PORTS_START( stratgyx )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(scobra_state, stratgyx_coinage_r<0x01>) /* lower 2 coinage DIPs */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(scobra_state, stratgyx_coinage_r<0x02>) /* lower 2 coinage DIPs */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(scobra_state, stratgyx_coinage_r<0x04>) /* upper 2 coinage DIPs */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(scobra_state, stratgyx_coinage_r<0x08>) /* upper 2 coinage DIPs */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  /* none of these appear to be used */
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

	PORT_START("IN4")      /* fake coinage DIPs.  read via IN2 and IN3 */
	PORT_DIPNAME( 0x0f, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0a, "A 2/1  B 1/3" )
	/*PORT_DIPSETTING(    0x02, "A 2/1  B 1/3" )*/
	PORT_DIPSETTING(    0x09, "A 2/1  B 1/6" )
	PORT_DIPSETTING(    0x03, "A 4/3  B 1/3" )
	PORT_DIPSETTING(    0x0c, "A 1/1  B 2/1" )
	PORT_DIPSETTING(    0x07, "A 1/1  B 1/2" )
	PORT_DIPSETTING(    0x01, "A 1/1  B 1/3" )
	PORT_DIPSETTING(    0x06, "A 1/1  B 1/5" )
	/*PORT_DIPSETTING(    0x0b, "A 1/1  B 1/5" )*/
	PORT_DIPSETTING(    0x05, "A 1/1  B 1/7" )
	PORT_DIPSETTING(    0x0d, "A 1/2  B 1/1" )
	PORT_DIPSETTING(    0x0e, "A 1/3  B 3/1" )
	PORT_DIPSETTING(    0x0f, "A 1/4  B 4/1" )
	PORT_DIPSETTING(    0x04, "A 1/99  B 2/1" )
	/*PORT_DIPSETTING(    0x08, "A 1/99  B 2/1" )*/
	PORT_DIPSETTING(    0x00, "A 1/99  B 1/3" )
INPUT_PORTS_END

/* cocktail mode is N/A */
static INPUT_PORTS_START( darkplnt )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )    // and START1
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )    // and START2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Bonus Occurrence" )
	PORT_DIPSETTING(    0x01, "Once" )
	PORT_DIPSETTING(    0x00, "Every" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_BIT( 0xfc, 0x00, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(scobra_state, darkplnt_dial_r) // scrambled dial

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPSETTING(    0x04, "Coin A 1/3 Coin B 3/1" )
	PORT_DIPSETTING(    0x06, "Coin A 1/4 Coin B 4/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPSETTING(    0x08, "200k" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAL")
	PORT_BIT( 0xfc, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) /* scrambled dial */
INPUT_PORTS_END

static INPUT_PORTS_START( tazmani2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPSETTING(    0x04, "Coin A 1/3 Coin B 3/1" )
	PORT_DIPSETTING(    0x06, "Coin A 1/4 Coin B 4/1" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( tazmani3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPSETTING(    0x04, "Coin A 1/3 Coin B 3/1" )
	PORT_DIPSETTING(    0x06, "Coin A 1/4 Coin B 4/1" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* cocktail mode is N/A */
static INPUT_PORTS_START( rescue )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x02, 0x02, "Starting Level" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x06, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPSETTING(    0x04, "Coin A 1/3 Coin B 3/1" )
	PORT_DIPSETTING(    0x06, "Coin A 1/4 Coin B 4/1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* cocktail mode is N/A */
static INPUT_PORTS_START( minefld )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x02, 0x02, "Starting Level" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/2 Coin B 2/1" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

// doesn't seem to have a 'starting level' dipswitch, probably has difficult tho?
static INPUT_PORTS_START( minefldfe )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY // fire up
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // bomb
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) // also credit sound!
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY // fire down
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "Coin A 1/1 Coin B 1/10" )
	PORT_DIPSETTING(    0x02, "Coin A 2/1 Coin B 1/3" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY // move up
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY // move down
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( hustler )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( hustlerb6 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) // 0x01 also controls cocktail / upright, leftover not connected in this bootleg?
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mimonkeyug )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x08, 0x08, "Infinite Lives (Cheat)")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) // actually only 2 of the following should be dips
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
INPUT_PORTS_END


void scobra_state::type1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 18432000/6);    /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::type1_map);

	Z80(config, m_audiocpu, 14318000/8);    /* 1.78975 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &scobra_state::scobra_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &scobra_state::scobra_sound_io_map);
	m_audiocpu->set_irq_acknowledge_callback(FUNC(scramble_state::scramble_sh_irq_callback));

	ttl7474_device &konami_7474(TTL7474(config, "konami_7474", 0));
	konami_7474.comp_output_cb().set(FUNC(scobra_state::scramble_sh_7474_q_callback));

	MCFG_MACHINE_RESET_OVERRIDE(scobra_state,scramble)

	I8255A(config, m_ppi8255_0);
	m_ppi8255_0->in_pa_callback().set_ioport("IN0");
	m_ppi8255_0->in_pb_callback().set_ioport("IN1");
	m_ppi8255_0->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi8255_1);
	m_ppi8255_1->out_pa_callback().set(m_soundlatch, FUNC(generic_latch_8_device::write));
	m_ppi8255_1->out_pb_callback().set(FUNC(scramble_state::scramble_sh_irqtrigger_w));

	ttl7474_device &ttl7474_9m_1(TTL7474(config, "7474_9m_1", 0));
	ttl7474_9m_1.output_cb().set(FUNC(scobra_state::galaxold_7474_9m_1_callback));

	ttl7474_device &ttl7474_9m_2(TTL7474(config, "7474_9m_2", 0));
	ttl7474_9m_2.comp_output_cb().set(FUNC(scobra_state::galaxold_7474_9m_2_q_callback));

	TIMER(config, "int_timer").configure_generic(FUNC(scobra_state::galaxold_interrupt_timer));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(16000.0/132/2);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(scobra_state::screen_update_galaxold));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_scobra);
	PALETTE(config, m_palette, FUNC(scobra_state::scrambold_palette), 32+64+2+1); // 32 for characters, 64 for stars, 2 for bullets, 1 for background

	MCFG_VIDEO_START_OVERRIDE(scobra_state,scrambold)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, "ay1", 14318000/8).add_route(ALL_OUTPUTS, "mono", 0.16);

	ay8910_device &ay2(AY8910(config, "ay2", 14318000/8));
	ay2.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	ay2.port_b_read_callback().set(FUNC(scramble_state::scramble_portB_r));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.16);
}



/* Rescue, Minefield and Strategy X have extra colors, and custom video initialise */
/* routines to set up the graduated color backgound they use */
void scobra_state::rescue(machine_config &config)
{
	type1(config);

	/* basic machine hardware */

	/* video hardware */
	m_palette->set_entries(32+64+2+128); // 32 for characters, 64 for stars, 2 for bullets, 128 for background
	m_palette->set_init(FUNC(scobra_state::rescue_palette));

	MCFG_VIDEO_START_OVERRIDE(scobra_state,rescue)
}

void scobra_state::rescuefe(machine_config &config)
{
	rescue(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::rescuefe_map);
}

void scobra_state::rescueb(machine_config &config)
{
	rescue(config);
	m_ppi8255_1->in_pc_callback().set(FUNC(scobra_state::rescueb_a002_r)); // protection? must return 0xfc or the game jumps to 0x00
}


void scobra_state::minefld(machine_config &config)
{
	type1(config);

	/* video hardware */
	m_palette->set_entries(32+64+2+256); // 32 for characters, 64 for stars, 2 for bullets, 256 for background
	m_palette->set_init(FUNC(scobra_state::minefld_palette));

	MCFG_VIDEO_START_OVERRIDE(scobra_state,minefld)
}

void scobra_state::minefldfe(machine_config &config)
{
	minefld(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::minefldfe_map);
}

void scobra_state::mimonkeyug(machine_config &config)
{
	Z80(config, m_maincpu, 18432000/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::mimonkeyug_map);

	MCFG_MACHINE_RESET_OVERRIDE(scobra_state,scramble)

	ttl7474_device &ttl7474_9m_1(TTL7474(config, "7474_9m_1", 0));
	ttl7474_9m_1.output_cb().set(FUNC(scobra_state::galaxold_7474_9m_1_callback));

	ttl7474_device &ttl7474_9m_2(TTL7474(config, "7474_9m_2", 0));
	ttl7474_9m_2.comp_output_cb().set(FUNC(scobra_state::galaxold_7474_9m_2_q_callback));

	TIMER(config, "int_timer").configure_generic(FUNC(scobra_state::galaxold_interrupt_timer));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(16000.0/132/2);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(scobra_state::screen_update_galaxold));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_scobra);
	PALETTE(config, m_palette, FUNC(scobra_state::scrambold_palette), 32+64+2+1); // 32 for characters, 64 for stars, 2 for bullets, 1 for background

	MCFG_VIDEO_START_OVERRIDE(scobra_state,mimonkey)

	SPEAKER(config, "mono").front_center();

	// discrete audio
}

void scobra_state::type2(machine_config &config)
{
	type1(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::type2_map);
}

void scobra_state::tazmani3(machine_config &config)
{
	type2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::tazmani3_map);
}


void scobra_state::stratgyx(machine_config &config)
{
	type2(config);

	/* basic machine hardware */

	m_ppi8255_1->out_pa_callback().set(m_soundlatch, FUNC(generic_latch_8_device::write));
	m_ppi8255_1->out_pb_callback().set(FUNC(scramble_state::scramble_sh_irqtrigger_w));
	m_ppi8255_1->in_pc_callback().set_ioport("IN3");

	/* video hardware */
	m_palette->set_entries(32+64+2+8); // 32 for characters, 64 for stars, 2 for bullets, 8 for background
	m_palette->set_init(FUNC(scobra_state::stratgyx_palette));

	MCFG_VIDEO_START_OVERRIDE(scobra_state,stratgyx)
}


void scobra_state::darkplnt(machine_config &config)
{
	type2(config);

	/* video hardware */
	m_palette->set_entries(32+64+2); // 32 for characters, 64 (buffer) for stars, 2 for bullets
	m_palette->set_init(FUNC(scobra_state::darkplnt_palette));

	MCFG_VIDEO_START_OVERRIDE(scobra_state,darkplnt)
}


void scobra_state::hustler(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 18432000/6);    /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::hustler_map);

	Z80(config, m_audiocpu, 14318000/8); /* 1.78975 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &scobra_state::hustler_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &scobra_state::hustler_sound_io_map);
	m_audiocpu->set_irq_acknowledge_callback(FUNC(scramble_state::scramble_sh_irq_callback));

	ttl7474_device &konami_7474(TTL7474(config, "konami_7474", 0));
	konami_7474.comp_output_cb().set(FUNC(scobra_state::scramble_sh_7474_q_callback));

	MCFG_MACHINE_RESET_OVERRIDE(scobra_state,scramble)

	ttl7474_device &ttl7474_9m_1(TTL7474(config, "7474_9m_1", 0));
	ttl7474_9m_1.output_cb().set(FUNC(scobra_state::galaxold_7474_9m_1_callback));

	ttl7474_device &ttl7474_9m_2(TTL7474(config, "7474_9m_2", 0));
	ttl7474_9m_2.comp_output_cb().set(FUNC(scobra_state::galaxold_7474_9m_2_q_callback));

	TIMER(config, "int_timer").configure_generic(FUNC(scobra_state::galaxold_interrupt_timer));

	WATCHDOG_TIMER(config, "watchdog");

	I8255A(config, m_ppi8255_0);
	m_ppi8255_0->in_pa_callback().set_ioport("IN0");
	m_ppi8255_0->in_pb_callback().set_ioport("IN1");
	m_ppi8255_0->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi8255_1);
	m_ppi8255_1->out_pa_callback().set(m_soundlatch, FUNC(generic_latch_8_device::write));
	m_ppi8255_1->out_pb_callback().set(FUNC(scramble_state::scramble_sh_irqtrigger_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(16000.0/132/2);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(scobra_state::screen_update_galaxold));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_scobra);
	PALETTE(config, m_palette, FUNC(scobra_state::galaxold_palette), 32+64+2); // 32 for characters, 64 for stars, 2 for bullets

	MCFG_VIDEO_START_OVERRIDE(scobra_state,scrambold)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &aysnd(AY8910(config, "aysnd", 14318000/8));
	aysnd.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	aysnd.port_b_read_callback().set(FUNC(scramble_state::hustler_portB_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.33);
}

void scobra_state::hustlerb(machine_config &config)
{
	hustler(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::hustlerb_map);

	m_audiocpu->set_addrmap(AS_PROGRAM, &scobra_state::hustlerb_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &scobra_state::hustlerb_sound_io_map);
}

void scobra_state::hustlerb4(machine_config &config)
{
	hustler(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::hustlerb_map);
}

// this bootleg has no 8255s and it redirects reads and writes to other addresses
void scobra_state::hustlerb6(machine_config &config)
{
	hustler(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &scobra_state::hustlerb6_map);

	config.device_remove("ppi8255_0");
	config.device_remove("ppi8255_1");
	config.device_remove("watchdog");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( stratgyx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c_1.bin",     0x0000, 0x1000, CRC(eec01237) SHA1(619aaf6379064395a1166575f207d61c90731bb9) )
	ROM_LOAD( "2e_2.bin",     0x1000, 0x1000, CRC(926cb2d5) SHA1(241e7b0f0d7e20a79299696be28963fd01269e86) )
	ROM_LOAD( "2f_3.bin",     0x2000, 0x1000, CRC(849e2504) SHA1(0ec00a5c77e9d81d69f8b847a50f36af2dacc8d0) )
	ROM_LOAD( "2h_4.bin",     0x3000, 0x1000, CRC(8a64069b) SHA1(ccc0e0441360e3ed93f2a0dab70c3a66edad969f) )
	ROM_LOAD( "2j_5.bin",     0x4000, 0x1000, CRC(78b9b898) SHA1(158e11352d68ed9e4277efbcb7e927e9bdc662f8) )
	ROM_LOAD( "2l_6.bin",     0x5000, 0x1000, CRC(20bae414) SHA1(cd3c03d3b6a3abb64cb86cffa733d5494cb226f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "s1.bin",       0x0000, 0x1000, CRC(713a5db8) SHA1(8ec41cb93cfd856dc5aecace6238240a5d114ce1) )
	ROM_LOAD( "s2.bin",       0x1000, 0x1000, CRC(46079411) SHA1(72bfc39979818309ac5a49654a825f9e4bd0236c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f_c2.bin",    0x0000, 0x0800, CRC(7121b679) SHA1(bf76c5b777d1f1468b31524bfe250a1bc4911b09) )
	ROM_LOAD( "5h_c1.bin",    0x0800, 0x0800, CRC(d105ad91) SHA1(fd30d001764cf37ae52bab010db27acdc35fece6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "strategy.6e",  0x0000, 0x0020, CRC(51a629e1) SHA1(f9826202c91a4a3deb8d815cbaa107b29dce5835) )

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "strategy.10k", 0x0000, 0x0020, CRC(d95c0318) SHA1(83e5355fdca7b4e1fb5a0e6eeaacfbf9561e2c36) )    /* background color map */
ROM_END

ROM_START( stratgys )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2c.cpu",       0x0000, 0x1000, CRC(f2aaaf2b) SHA1(1aa5d0696961bcd43a4659db0a53f443cd3d3985) )
	ROM_LOAD( "2e.cpu",       0x1000, 0x1000, CRC(5873fdc8) SHA1(9707c6f140cd7d51f8bd1e53f3c138491aa302d1) )
	ROM_LOAD( "2f.cpu",       0x2000, 0x1000, CRC(532d604f) SHA1(eaf314d98005caacde8d2ea6240043ebe10650a5) )
	ROM_LOAD( "2h.cpu",       0x3000, 0x1000, CRC(82b1d95e) SHA1(dc5e79e159990e76353dea7b54e52fadfc9f59ab) )
	ROM_LOAD( "2j.cpu",       0x4000, 0x1000, CRC(66e84cde) SHA1(c1e6b9fe0052fe1f521c943a69d60f65fa392147) )
	ROM_LOAD( "2l.cpu",       0x5000, 0x1000, CRC(62b032d0) SHA1(592866f801839550e60a515978a2d97dd414c87a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "s1.bin",       0x0000, 0x1000, CRC(713a5db8) SHA1(8ec41cb93cfd856dc5aecace6238240a5d114ce1) )
	ROM_LOAD( "s2.bin",       0x1000, 0x1000, CRC(46079411) SHA1(72bfc39979818309ac5a49654a825f9e4bd0236c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.cpu",       0x0000, 0x0800, CRC(f4aa5ddd) SHA1(2be9c8d8b94c599d4ff12864cbbddd7301be9fb2) )
	ROM_LOAD( "5h.cpu",       0x0800, 0x0800, CRC(548e4635) SHA1(fb8d4d498a98b9d19504f1b6c86dfa0110dcc5a7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "strategy.6e",  0x0000, 0x0020, CRC(51a629e1) SHA1(f9826202c91a4a3deb8d815cbaa107b29dce5835) )

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "strategy.10k", 0x0000, 0x0020, CRC(d95c0318) SHA1(83e5355fdca7b4e1fb5a0e6eeaacfbf9561e2c36) )
ROM_END

ROM_START( strongx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "strongx.2d",   0x0000, 0x1000, CRC(c2f7268c) SHA1(2943198d82d4bc7990b327c44bbce7e7eb011886) )
	ROM_LOAD( "strongx.2e",   0x1000, 0x1000, CRC(91059422) SHA1(c73b50bc4035bd7268628cf92b21acbafef0456e) )
	ROM_LOAD( "2f_3.bin",     0x2000, 0x1000, CRC(849e2504) SHA1(0ec00a5c77e9d81d69f8b847a50f36af2dacc8d0) )
	ROM_LOAD( "2h_4.bin",     0x3000, 0x1000, CRC(8a64069b) SHA1(ccc0e0441360e3ed93f2a0dab70c3a66edad969f) )
	ROM_LOAD( "2j_5.bin",     0x4000, 0x1000, CRC(78b9b898) SHA1(158e11352d68ed9e4277efbcb7e927e9bdc662f8) )
	ROM_LOAD( "2l_6.bin",     0x5000, 0x1000, CRC(20bae414) SHA1(cd3c03d3b6a3abb64cb86cffa733d5494cb226f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound code */
	ROM_LOAD( "s1.bin",       0x0000, 0x1000, CRC(713a5db8) SHA1(8ec41cb93cfd856dc5aecace6238240a5d114ce1) )
	ROM_LOAD( "s2.bin",       0x1000, 0x1000, CRC(46079411) SHA1(72bfc39979818309ac5a49654a825f9e4bd0236c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f_c2.bin",    0x0000, 0x0800, CRC(7121b679) SHA1(bf76c5b777d1f1468b31524bfe250a1bc4911b09) )
	ROM_LOAD( "5h_c1.bin",    0x0800, 0x0800, CRC(d105ad91) SHA1(fd30d001764cf37ae52bab010db27acdc35fece6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "strategy.6e",  0x0000, 0x0020, CRC(51a629e1) SHA1(f9826202c91a4a3deb8d815cbaa107b29dce5835) )

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "strategy.10k", 0x0000, 0x0020, CRC(d95c0318) SHA1(83e5355fdca7b4e1fb5a0e6eeaacfbf9561e2c36) )    /* background color map */
ROM_END

ROM_START( darkplnt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "drkplt2c.dat", 0x0000, 0x1000, CRC(5a0ca559) SHA1(cf77bda30bb08633d274c22734a8f186520ddf7d) )
	ROM_LOAD( "drkplt2e.dat", 0x1000, 0x1000, CRC(52e2117d) SHA1(774e8ee84316135313c4b66c51d21c1661f345b3) )
	ROM_LOAD( "drkplt2g.dat", 0x2000, 0x1000, CRC(4093219c) SHA1(cc92ec4ec8661993173daedfa521252a6e34073d) )
	ROM_LOAD( "drkplt2j.dat", 0x3000, 0x1000, CRC(b974c78d) SHA1(af4c1b21ca8651bfc343a24a106db4c82f0052e7) )
	ROM_LOAD( "drkplt2k.dat", 0x4000, 0x1000, CRC(71a37385) SHA1(a6a358b2f7daa2eaa5bdf96aa903619376473b20) )
	ROM_LOAD( "drkplt2l.dat", 0x5000, 0x1000, CRC(5ad25154) SHA1(20b1347ed5fca108303132d28ae73fc4d71b4e5a) )
	ROM_LOAD( "drkplt2m.dat", 0x6000, 0x1000, CRC(8d2f0122) SHA1(59280abca22fd3d7af53aebf56a9e37eeb7cb11c) )
	ROM_LOAD( "drkplt2p.dat", 0x7000, 0x1000, CRC(2d66253b) SHA1(445c00a74551732fd02df2d5879688f9984931b5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "5c.snd",       0x0000, 0x1000, CRC(672b9454) SHA1(2b2ec06d62ff5f19a1c802152c798940631fb1d6) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "drkplt5f.dat", 0x0000, 0x0800, CRC(2af0ee66) SHA1(a4f32c256c393a18afde5e3f3c67d7df1223999a) )
	ROM_LOAD( "drkplt5h.dat", 0x0800, 0x0800, CRC(66ef3225) SHA1(87f36a853c4a8ae40d1e3f0482b6e31c86f834f3) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6e.cpu",       0x0000, 0x0020, CRC(86b6e124) SHA1(76cc422aeb53a4970f91bd5eb00a3e24a76c09d1) )
ROM_END

ROM_START( tazmani2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2ck.cpu",      0x0000, 0x1000, CRC(bf0492bf) SHA1(e665be8b36eb00388daca1bc3a4e4b93b943e2bd) )
	ROM_LOAD( "2ek.cpu",      0x1000, 0x1000, CRC(6636c4d0) SHA1(a0470950111fd9ef3ec71ee9f4b337276db35887) )
	ROM_LOAD( "2fk.cpu",      0x2000, 0x1000, CRC(ce59a57b) SHA1(1ee9a1535f9e06b08db3133a29cc700d1b487e82) )
	ROM_LOAD( "2hk.cpu",      0x3000, 0x1000, CRC(8bda3380) SHA1(eb31dd180da5cb8f8dcdaa777a27ed5514cf38cf) )
	ROM_LOAD( "2jk.cpu",      0x4000, 0x1000, CRC(a4095e35) SHA1(5ccab4cb9c44870852b22511daeda10d54b60822) )
	ROM_LOAD( "2kk.cpu",      0x5000, 0x1000, CRC(f308ca36) SHA1(af360c4ba88b3a3365a2d81575886fcec25f19bf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom0.snd",     0x0000, 0x0800, CRC(b8d741f1) SHA1(a1bb8a1e0d6b34111f05c539c8e92fffacf5aa5c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.cpu",       0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) )
	ROM_LOAD( "5h.cpu",       0x0800, 0x0800, CRC(3f5ff3ac) SHA1(bc70eef54a45b52c14e35464e5f06b5eec554eb6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colr6f.cpu",   0x0000, 0x0020, CRC(fce333c7) SHA1(f63a214dc47c5e7c80db000b0b6a261ca8da6629) )
ROM_END

// PCBs: RODMAR 6920-00-01 P1 and 6920-01-01 P1 but Arfyc copyright
ROM_START( tazmani3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2ck.cpu",      0x0000, 0x1000, CRC(e8b6f9c3) SHA1(121f83274b3fdb4b2cb4bd0160d61886825c8793) ) // sldh
	ROM_LOAD( "2ek.cpu",      0x1000, 0x1000, CRC(c2f92fc0) SHA1(d95746d82e213a523dcdc921454994c6f57056a4) ) // sldh
	ROM_LOAD( "2fk.cpu",      0x2000, 0x1000, CRC(4362182f) SHA1(84f33578c49380e208f89c679e115fe2b13f646d) ) // sldh
	ROM_LOAD( "2hk.cpu",      0x3000, 0x1000, CRC(3d9925ab) SHA1(f188cb44c91a1f117eb1adc5a0901b073e390fd4) ) // sldh
	ROM_LOAD( "2jk.cpu",      0x4000, 0x1000, CRC(207c0a63) SHA1(5168ad793cf4a26d72bddd736d9776d315e66365) ) // sldh
	ROM_LOAD( "2kk.cpu",      0x5000, 0x1000, CRC(c6253504) SHA1(d1c643a65ac6e69afb3ed1dac169e6de5f049c93) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom0.snd",     0x0000, 0x0800, CRC(b8d741f1) SHA1(a1bb8a1e0d6b34111f05c539c8e92fffacf5aa5c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5f.cpu",       0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) )
	ROM_LOAD( "5h.cpu",       0x0800, 0x0800, CRC(e726b559) SHA1(090a99dd52adba379011c26e119a5d816e8f669f) ) // sldh

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "colr6f.cpu",   0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) ) // sldh
ROM_END

ROM_START( tazmaniet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "13200.bin",      0x0000, 0x1000, CRC(cea9df87) SHA1(e4ecaee74134580978c0fbb209a704151ea013b2) )
	ROM_LOAD( "13201.bin",      0x1000, 0x1000, CRC(21cfe0b3) SHA1(fb5a8b239c06c93483112558a0e1f6269a5d14b2) )
	ROM_LOAD( "13202.bin",      0x2000, 0x1000, CRC(d204732e) SHA1(069dc57e35a653fec65d1fdc19f716e7adea9911) )
	ROM_LOAD( "13203.bin",      0x3000, 0x1000, CRC(5d0651bd) SHA1(436f5f00619a2a05db1b33ba0775367af0c637b3) )
	ROM_LOAD( "13204.bin",      0x4000, 0x1000, CRC(9082ac06) SHA1(cfedfac8a1172ffc2c44e8a87aa527148d24befc) )
	ROM_LOAD( "13205.bin",      0x5000, 0x1000, CRC(9c2a08a2) SHA1(65cbea5db287863ced20c75b3133647ed4b83894) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "13208.bin",     0x0000, 0x0800, CRC(b8d741f1) SHA1(a1bb8a1e0d6b34111f05c539c8e92fffacf5aa5c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "13206.bin",       0x0000, 0x0800, CRC(2c5b612b) SHA1(32e3a41a9a4a8b1285b6a195213ff0d98012360a) )
	ROM_LOAD( "13207.bin",       0x0800, 0x0800, CRC(3f5ff3ac) SHA1(bc70eef54a45b52c14e35464e5f06b5eec554eb6) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "prom.bin",   0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) ) // same as hustler?! Gives wrong colors

	ROM_REGION( 0x200, "unused_proms", 0) // TODO: are these the correct color PROMs?
	ROM_LOAD( "13209.prom", 0x0000, 0x0100, CRC(da1708c9) SHA1(b2633eba050866c6f555ed0fd9be369d69f13683) )
	ROM_LOAD( "13210.prom", 0x0100, 0x0100, CRC(e54d042a) SHA1(e3259b5c178fc5ab43eda27f07107b65555512d3) )
ROM_END

/*
    Rescue

    CPU/Video Board: A969 (Has various wire mods)
    Sound Board:     ?
    Interface Card:  A2372
*/

ROM_START( rescue )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rb15acpu.bin", 0x0000, 0x1000, CRC(d7e654ba) SHA1(6bcf7a6d86bc2de9a304f0aa9542fdcb58ca73dd) )
	ROM_LOAD( "rb15bcpu.bin", 0x1000, 0x1000, CRC(a93ea158) SHA1(c5550881d2a44f82eeb56d90487137d959c8c6ae) )
	ROM_LOAD( "rb15ccpu.bin", 0x2000, 0x1000, CRC(058cd3d0) SHA1(3f8962be982952adab23c1884cd3a9be5ddd4108) )
	ROM_LOAD( "rb15dcpu.bin", 0x3000, 0x1000, CRC(d6505742) SHA1(804df84ea507bf41e9376a7ce63a278c3701f0cd) )
	ROM_LOAD( "rb15ecpu.bin", 0x4000, 0x1000, CRC(604df3a4) SHA1(15790fa442538632f232280c096ac788d9bf8117) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rb15csnd.bin", 0x0000, 0x0800, CRC(8b24bf17) SHA1(cc49fef3c629c12f1a7eb9886fdc2df4b08f4b37) )
	ROM_LOAD( "rb15dsnd.bin", 0x0800, 0x0800, CRC(d96e4fb3) SHA1(8bb023c7c668f93d2333d648fc3cefdbd66f92db) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rb15fcpu.bin", 0x0000, 0x0800, CRC(4489d20c) SHA1(c7a2afbd2e5645a1a25dec6147d61a38ba12380f) )
	ROM_LOAD( "rb15hcpu.bin", 0x0800, 0x0800, CRC(5512c547) SHA1(e0f1c994daaa8933230cbc4bb88d459a698e0d8e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "rescue.clr",   0x0000, 0x0020, CRC(40c6bcbd) SHA1(cb0c058eadc37eba4b1a99be095da81a14099d8d) )
ROM_END

ROM_START( rescueb )  /* VIDEL GAMES @ ($079A-$07A4) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.2c",        0x0000, 0x1000, CRC(d3661258) SHA1(41782097f6acb1d76067e7d727837a5a5297e43f) )
	ROM_LOAD( "p2.2e",        0x1000, 0x1000, CRC(47464506) SHA1(53a2a3ff307ad49ead8a7fcf897c8650f56eeb19) )
	ROM_LOAD( "p3.2f",        0x2000, 0x1000, CRC(7dc03ec3) SHA1(9aa5fda48a40c977d15e51bbcfe9b155705a6941) )
	ROM_LOAD( "p4.2h",        0x3000, 0x1000, CRC(a11eaf78) SHA1(dfbc6ea5894493fd74fe3f75cd1793bd31673319) )
	ROM_LOAD( "rb15ecpu.bin", 0x4000, 0x1000, CRC(604df3a4) SHA1(15790fa442538632f232280c096ac788d9bf8117) ) // p5.2j

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rb15csnd.bin", 0x0000, 0x0800, CRC(8b24bf17) SHA1(cc49fef3c629c12f1a7eb9886fdc2df4b08f4b37) ) // s1.5c
	ROM_LOAD( "rb15dsnd.bin", 0x0800, 0x0800, CRC(d96e4fb3) SHA1(8bb023c7c668f93d2333d648fc3cefdbd66f92db) ) // s1.5d

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rb15fcpu.bin", 0x0000, 0x0800, CRC(4489d20c) SHA1(c7a2afbd2e5645a1a25dec6147d61a38ba12380f) ) // c1.5f
	ROM_LOAD( "rb15hcpu.bin", 0x0800, 0x0800, CRC(5512c547) SHA1(e0f1c994daaa8933230cbc4bb88d459a698e0d8e) ) // c2.5h

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "rescue.clr",   0x0000, 0x0020, CRC(40c6bcbd) SHA1(cb0c058eadc37eba4b1a99be095da81a14099d8d) )
ROM_END

ROM_START( aponow )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aponow01.rom", 0x0000, 0x1000, CRC(33011579) SHA1(62bd821f6e0968a5ed9cbab26a8bff31e707f07a) )
	ROM_LOAD( "aponow02.rom", 0x1000, 0x1000, CRC(d477573e) SHA1(e5deb62f2763ff81e9ca24df67b0539cca8ba714) )
	ROM_LOAD( "aponow03.rom", 0x2000, 0x1000, CRC(46c41898) SHA1(c501f1d75762cfeda68e178e70cdf7eb423005fe) )
	ROM_LOAD( "rb15dcpu.bin", 0x3000, 0x1000, CRC(d6505742) SHA1(804df84ea507bf41e9376a7ce63a278c3701f0cd) ) // aponow04.rom
	ROM_LOAD( "rb15ecpu.bin", 0x4000, 0x1000, CRC(604df3a4) SHA1(15790fa442538632f232280c096ac788d9bf8117) ) // aponow05.rom

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rb15csnd.bin", 0x0000, 0x0800, CRC(8b24bf17) SHA1(cc49fef3c629c12f1a7eb9886fdc2df4b08f4b37) )
	ROM_LOAD( "rb15dsnd.bin", 0x0800, 0x0800, CRC(d96e4fb3) SHA1(8bb023c7c668f93d2333d648fc3cefdbd66f92db) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "rb15fcpu.bin", 0x0000, 0x0800, CRC(4489d20c) SHA1(c7a2afbd2e5645a1a25dec6147d61a38ba12380f) )
	ROM_LOAD( "rb15hcpu.bin", 0x0800, 0x0800, CRC(5512c547) SHA1(e0f1c994daaa8933230cbc4bb88d459a698e0d8e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "rescue.clr",   0x0000, 0x0020, CRC(40c6bcbd) SHA1(cb0c058eadc37eba4b1a99be095da81a14099d8d) )
ROM_END

ROM_START( minefld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ma22c",        0x0000, 0x1000, CRC(1367a035) SHA1(25e415743a4cbe573a00b81155f6292592e32cdd) )
	ROM_LOAD( "ma22e",        0x1000, 0x1000, CRC(68946d21) SHA1(62f4501c6d2e8615b1f176e6cea9a75c912de23e) )
	ROM_LOAD( "ma22f",        0x2000, 0x1000, CRC(7663aee5) SHA1(eacb2283e0555176629a96e9b607f0477927d84c) )
	ROM_LOAD( "ma22h",        0x3000, 0x1000, CRC(9787475d) SHA1(161c6f35cc2ce2a88bcc2e9626ccfe1f5e099f28) )
	ROM_LOAD( "ma22j",        0x4000, 0x1000, CRC(2ceceb54) SHA1(c7385ce876582515f6b9a529e0101032056e14e7) )
	ROM_LOAD( "ma22l",        0x5000, 0x1000, CRC(85138fc9) SHA1(b3f2e483150583d2b4daf6a8a308a7f47a0e4f1b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ma15c",        0x0000, 0x0800, CRC(8bef736b) SHA1(c5c7ce9c40e6fe60a4914b6bbd646ba6853c9043) )
	ROM_LOAD( "ma15d",        0x0800, 0x0800, CRC(f67b3f97) SHA1(bb15f95eab6594508b5c02af60ed9fff3abd23ee) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "ma15f",        0x0000, 0x0800, CRC(9f703006) SHA1(95c1368b6a3e0cea6fa9406155d3b8d1eb9477f4) )
	ROM_LOAD( "ma15h",        0x0800, 0x0800, CRC(ed0dccb1) SHA1(f1554aa5a4d3a70936645e784a94305f642670df) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "minefld.clr",  0x0000, 0x0020, CRC(1877368e) SHA1(5850e03debe572f72abd91c756c0f8613018a962) )
ROM_END

/*

This is a bootleg by Free Enterprise Games.
The host board is common Scramble hardware.
There is a small sub-board containing a Z80, the 3 'sub' eproms, a 2kx8 SRAM and an unknown
DIP24 chip by AMD(C)1979. I'm sure there is some obfuscation going on with this sub board.
There's a taunt in one of the EPROMs telling people they won't be able to copy it and get
it working ;-)
There are 2 GFX ROMs on the main board and 2 program ROMs and a color PROM.
There's also another small sub board plugged in at position 5L and 5M replacing 2 logic chips
with 8 logic chips and it's also wired to various logic chips on the main board.
The games are possibly Minefield and Scramble together on one board although both boards could
be running different games.
One board is missing the 2 sound ROMs (possibly on purpose, don't know)
Both are missing the top sound PCB.

*/

/* the 'Minefield' shop shows a Logicshop copyright, and a phone number with a Great Yarmouth area code
   which seems to indicate the following manufacturer / operator.

    The Logicshop (Great Yarmouth)
    Caesar's Arcade
    Beach Rd
    Hemsby
    Great Yarmouth
    Norfolk
    NR29 4HS

*/


ROM_START( minefldfe )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sub-1.bin",        0x0000, 0x0c00, CRC(0511df9f) SHA1(e4b93e70a070a8f42442c8329478b98cba387971) ) // all used
	ROM_CONTINUE(0x2200, 0x1400)
	ROM_LOAD( "sub-2.bin",        0x3600, 0x0a00, CRC(00205e04) SHA1(f8e8399a1aaa548ae2448ebc96fb3e68625f9260) ) //
	ROM_CONTINUE(0x5800, 0x1600)
	ROM_LOAD( "sub-3.bin",        0x8a00, 0x0800, CRC(98c05d8d) SHA1(9c89504421df754bb0228ca08c7bfae5f6dc3e91) ) // all used

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASEFF )
	ROM_LOAD( "snd.2l",        0x0000, 0x0800, CRC(8bef736b) SHA1(c5c7ce9c40e6fe60a4914b6bbd646ba6853c9043) )
	ROM_LOAD( "snd.2m",        0x0800, 0x0800, CRC(f67b3f97) SHA1(bb15f95eab6594508b5c02af60ed9fff3abd23ee) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gfx.5f",        0x0000, 0x0800, CRC(8b5e187a) SHA1(3afd348a2ade9fc01fd1fc6a5ce9b148e0a1ead1) )
	ROM_LOAD( "gfx.5h",        0x0800, 0x0800, CRC(4b7e1fbc) SHA1(334c09f380a3750874c4bef8e987112e7a50fdbd) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",  0x0000, 0x0020, CRC(1877368e) SHA1(5850e03debe572f72abd91c756c0f8613018a962) )
ROM_END

/*

notes about missing code in rescuefe - where is this??

the first part of 0xa600 - 0xa643 should be a set of jumps
based on the replaced code, setting 'a' them jumping
elsewhere (probably directly after the table)

547C: CD 00 A6      call $A600
is
1E34: 3E 00         ld   a,$00
1E36: CD 50 09      call $0950

4FA2: CD 03 A6      call $A603
is
18D7: 3E 01         ld   a,$01
18D9: CD 50 09      call $0950

48CF: CD 07 A6      call $A607
is
11F6: 3E 02         ld   a,$02
11F8: CD 50 09      call $0950

640B: CD 0B A6      call $A60B
is
2EBD: 3E 03         ld   a,$03
2EBF: CD 50 09      call $0950

// there's no ld   a,0x04? anywhere in the original code
// so next entry is 0x05

58CB: CD 0F A6      call $A60F
is
2296: 3E 05         ld   a,$05
2298: CD 50 09      call $0950

49DB: CD 13 A6      call $A613
is
1304: 3E 06         ld   a,$06
1306: CD 50 09      call $0950

5FA9: CD 17 A6      call $A617
is
2A35: 3E 07         ld   a,$07
2A37: CD 50 09      call $0950


5834: F5            push af
5835: CD 1B A6      call $A61B
5838: F1            pop  af
is inserted, replaces nothing?!
elsewhere, as expected we have

5DA9: CD 1B A6      call $A61B
is
281C: 3E 08         ld   a,$08
281E: CD 50 09      call $0950

5E45: CD 1F A6      call $A61F
is
28CB: 3E 09         ld   a,$09
28CD: CD 50 09      call $0950


51B8: CD 23 A6      call $A623
is
1B0A: 3E 0A         ld   a,$0A
1B0C: CD 50 09      call $0950

4FE4: CD 27 A6      call $A627
is
1930: 3E 0B         ld   a,$0B
1932: CD 50 09      call $0950

4A51: CD 2B A6      call $A62B
is
137C: 3E 0C         ld   a,$0C
137E: CD 50 09      call $0950

4648: CD 2F A6      call $A62F
is
0E6D: 3E 0D         ld   a,$0D
0E6F: CD 50 09      call $0950

516D: CD 33 A6      call $A633
is
1ABC: 3E 0E         ld   a,$0E
1ABE: CD 50 09      call $0950

5476: CD 37 A6      call $A637
is
1E2C: 3E 0F         ld   a,$0F
1E2E: CD 50 09      call $0950

4DD3: CD 3B A6      call $A63B
is
1704: 3E 10         ld   a,$10
1706: CD 50 09      call $0950

4AF0: CD 3F A6      call $A63F
is
141D: 3E 11         ld   a,$11
141F: CD 50 09      call $0950

I imagine the a643 - A664 area is the code that was originally at 0x950

479F: CD 65 A6      call $A665 << call to bigger function
is
0FE2: CD EB 0F      call $0FEB << call to bigger function

so $a665 - $a69a is probably the code from 0x0feb


44EC: CD 9B A6      call $A69B << call to bigger function
is
0D0D: CD D2 10      call $10D2 << call to bigger function

and 0xa69b - 0xa??? is the code from 0x10d2

*/

ROM_START( rescuefe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sub-1.bin",        0x0000, 0x0600, CRC(7a6e813e) SHA1(5f9fff576f6a8d8e09f11404ca49b1011d17fa36) ) // correct?
	ROM_CONTINUE( 0x4400, 0x1a00 ) // seems to be correct at least for the majority of blocks
	ROM_LOAD( "sub-2.bin",        0x5e00, 0x0e00, CRC(28fcec83) SHA1(f2b2b2c8f65450b627587cf5f16620b95d7fe4e2) ) // correct at least mapping of 6903?
	ROM_CONTINUE(0x8200, 0x1200) // some of this seems to be in the right place
	ROM_LOAD( "sub-3.bin",        0xa000, 0x0800, CRC(6fbeb239) SHA1(cb81719e8bb6b99ad4e46d9404c61c09db8475c3) ) // strange size, contains nothing except a taunt message which seems to be cut off at the start??

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASEFF ) // audio CPU code was missing, assuming the same as regular Rescue due to sound code on the minefield bootleg matching.
	ROM_LOAD( "rb15csnd.2l", 0x0000, 0x0800, CRC(8b24bf17) SHA1(cc49fef3c629c12f1a7eb9886fdc2df4b08f4b37) )
	ROM_LOAD( "rb15dsnd.2m", 0x0800, 0x0800, CRC(d96e4fb3) SHA1(8bb023c7c668f93d2333d648fc3cefdbd66f92db) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "gfx.5f",        0x0000, 0x0800, CRC(60f49e88) SHA1(92ea21715b2d4d28a0509a31b35b43354be7b75f) )
	ROM_LOAD( "gfx.5h",        0x0800, 0x0800, CRC(9486e09e) SHA1(a8c2b48a3a2633c2eae19bb40299c4da1499c15c) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "82s123.6e",  0x0000, 0x0020, CRC(40c6bcbd) SHA1(cb0c058eadc37eba4b1a99be095da81a14099d8d) )
ROM_END

ROM_START( hustler )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hustler.1",    0x0000, 0x1000, CRC(94479a3e) SHA1(ea3a1a3f6bee3d35a6a0fb0ba689a25f6b919e5d) )
	ROM_LOAD( "hustler.2",    0x1000, 0x1000, CRC(3cc67bcc) SHA1(bc36eaab68f48c837d0d3d3bc1e7de961fa24a21) )
	ROM_LOAD( "hustler.3",    0x2000, 0x1000, CRC(9422226a) SHA1(445daa94f7236f0776f4692de6cb213f67684f70) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hustler.6",    0x0000, 0x0800, CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) )
	ROM_LOAD( "hustler.7",    0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hustler.5f",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "hustler.5h",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( hustlerd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vh1.ic5",    0x0000, 0x1000, CRC(7cb6a940) SHA1(8b51072563f7a63aab5d5ee4e835dc7275a2b98a) )
	ROM_LOAD( "vh2.ic6",    0x1000, 0x1000, CRC(4ca45239) SHA1(729ec16c0a192f957ba454d4acbe873a71030a22) )
	ROM_LOAD( "vh3.ic7",    0x2000, 0x1000, CRC(4c752453) SHA1(467ffd6e3ec13a27fc2979883678a1e7531d98ac) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ho1.ic32",    0x0000, 0x0800, CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) )
	ROM_LOAD( "ho2.ic33",    0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hc2",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "hc1",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( billiard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a",            0x0000, 0x1000, CRC(b7eb50c0) SHA1(213d177d2b2af648a18d196b83e96d804947fd40) )
	ROM_LOAD( "b",            0x1000, 0x1000, CRC(988fe1c5) SHA1(0aaa13006e1832d7c25984b38c2e52fb6a5fdbbc) )
	ROM_LOAD( "c",            0x2000, 0x1000, CRC(7b8de793) SHA1(f54aa0bd558b156c2511a36518963df0e493dbb4) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hustler.6",    0x0000, 0x0800, CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) )
	ROM_LOAD( "hustler.7",    0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hustler.5f",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "hustler.5h",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

/* this is identical to billiard, but with a different memory map */
ROM_START( hustlerb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hustler.2c",   0x0000, 0x1000, CRC(3a1ac6a9) SHA1(f9a6137bab78ce64ec119f199b2bd5ed37e61099) )
	ROM_LOAD( "hustler.2f",   0x1000, 0x1000, CRC(dc6752ec) SHA1(b103021079646286156e4141fe34dd92ccfd34bd) )
	ROM_LOAD( "hustler.2j",   0x2000, 0x1000, CRC(27c1e0f8) SHA1(9713e84b3aa4ed4829de2f62059229564d2b9f72) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hustler.11d",  0x0000, 0x0800, CRC(b559bfde) SHA1(f7733fbc5cabb441ba039b9d7202aaf0cebb9a85) )
	ROM_LOAD( "hustler.10d",  0x0800, 0x0800, CRC(6ef96cfb) SHA1(eba0bdc8bc1652ff2f62594371ded711dbfcce86) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hustler.5f",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "hustler.5h",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "hustler.clr",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( hustlerb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1.1",   0x0000, 0x1000, CRC(2ce5e7b9) SHA1(e47e89bd085b40d7dd38be51fd69f69b143daf1b) )
	ROM_LOAD( "b1.2",   0x1000, 0x1000, CRC(dc6752ec) SHA1(b103021079646286156e4141fe34dd92ccfd34bd) )
	ROM_LOAD( "b3.3",   0x2000, 0x1000, CRC(23092f67) SHA1(3c04544c636eab8998752e1f521e34b1345e39ff) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "b6.bin",    0x0000, 0x0800, CRC(7a946544) SHA1(7ee2ad3fdf996f08534fb87fc02b619c168f420c) ) // sldh
	ROM_LOAD( "b7.bin",    0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "b4.r2",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "b5",      0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) ) // broken rom, assumed to be the same

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "27s19.bin",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END



ROM_START( hustlerb4 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // identical to hustlerb but in 6 roms instead of 3
	ROM_LOAD( "1.d2",   0x0000, 0x0800, CRC(f87d75c7) SHA1(48f0f5289366a3c4dec86f5bb48d16a33745844d) )
	ROM_LOAD( "2.e2",   0x0800, 0x0800, CRC(a639d4e1) SHA1(2130de635f41f915db26a5fcba9654ae140d0ecc) )
	ROM_LOAD( "3.f2",   0x1000, 0x0800, CRC(1b75520e) SHA1(b4ebb69c0f17fde7a527d54ec8406b1b80798e0c) )
	ROM_LOAD( "4.h2",   0x1800, 0x0800, CRC(fdea3165) SHA1(6120919445599ec9116d14d0baf4fb4e4720e473) )
	ROM_LOAD( "5.j2",   0x2000, 0x0800, CRC(730100e1) SHA1(81e44d768ca4e654981c14660e12e355fe720636) )
	ROM_LOAD( "6.l2",   0x2800, 0x0800, CRC(68dff552) SHA1(5dad38db45afbd79b5627a75b295fc920ad68856) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // closer to original sound system
	ROM_LOAD( "topa.c5",  0x0000, 0x0800, CRC(9fc1d60f) SHA1(93386098199fd7655da8137a79857ffffebc62eb) )
	ROM_LOAD( "topb.d5",  0x0800, 0x0800, CRC(3db57351) SHA1(e5075a7130a80d2bf24f0556c2589dff0625ee60) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "7.f5",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "8.h5",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mni6331.e6",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )

	ROM_REGION( 0x0800, "unk", 0 )
	ROM_LOAD( "top.c5",  0x0000, 0x0800, CRC(88226086) SHA1(fe2da172313063e5b056fc8c8d8b2a5c64db5179) )
ROM_END

// the following romset came from a blister, so no PCB infos. It's a mix between hustlerb4's main CPU and GFX ROMs and hustlerb's audio CPU ROMs.

ROM_START( hustlerb5 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // identical to hustlerb but in 6 roms instead of 3
	ROM_LOAD( "b1.bin",   0x0000, 0x0800, CRC(f87d75c7) SHA1(48f0f5289366a3c4dec86f5bb48d16a33745844d) )
	ROM_LOAD( "b2.bin",   0x0800, 0x0800, CRC(a639d4e1) SHA1(2130de635f41f915db26a5fcba9654ae140d0ecc) )
	ROM_LOAD( "b3.bin",   0x1000, 0x0800, CRC(1b75520e) SHA1(b4ebb69c0f17fde7a527d54ec8406b1b80798e0c) )
	ROM_LOAD( "b4.bin",   0x1800, 0x0800, CRC(fdea3165) SHA1(6120919445599ec9116d14d0baf4fb4e4720e473) )
	ROM_LOAD( "b5.bin",   0x2000, 0x0800, CRC(730100e1) SHA1(81e44d768ca4e654981c14660e12e355fe720636) )
	ROM_LOAD( "b6.bin",   0x2800, 0x0800, CRC(68dff552) SHA1(5dad38db45afbd79b5627a75b295fc920ad68856) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bs1.bin",  0x0000, 0x0800, CRC(b559bfde) SHA1(f7733fbc5cabb441ba039b9d7202aaf0cebb9a85) )
	ROM_LOAD( "bs2.bin",  0x0800, 0x0800, CRC(6ef96cfb) SHA1(eba0bdc8bc1652ff2f62594371ded711dbfcce86) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "c1.bin",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "c2.bin",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 ) // not dumped for this set, but the same in all other original and bootleg sets
	ROM_LOAD( "mni6331.e6",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( hustlerb6 ) // dump confirmed from two different PCBs, seems most similar to hustlerb, but for the absence of 8255s
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "h0.10l",  0x0000, 0x1000, CRC(15748377) SHA1(9f4a88cbef0b3860bb35cc8d208c47db16924837) )
	ROM_LOAD( "h1.9l",   0x1000, 0x1000, CRC(dc6752ec) SHA1(b103021079646286156e4141fe34dd92ccfd34bd) )
	ROM_LOAD( "h2.8l",   0x2000, 0x1000, CRC(58268f2b) SHA1(96e3f95c93f1199afaeffc6bbb1cc65e1c0b3de1) )
	/* 3000-3fff space for diagnostics ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "h5.3b",  0x0000, 0x1000, CRC(7d4085eb) SHA1(4f7a7860223ab823cf5b697e17c2e83783442697) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "h3.5h",   0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "h4.3h",   0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331-1j.1k",  0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( hustlerb7 ) // bootleg from Marti Colls (Spain)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mcbi.1.10l",     0x0000, 0x1000, CRC(15748377) SHA1(9f4a88cbef0b3860bb35cc8d208c47db16924837) )
	ROM_LOAD( "b1.2.10l",       0x1000, 0x1000, CRC(dc6752ec) SHA1(b103021079646286156e4141fe34dd92ccfd34bd) )
	ROM_LOAD( "mcbi.3.8l",      0x2000, 0x1000, CRC(73a011f0) SHA1(b7948ba93a77c9a7bf469a1ae38afa81e231ac90) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "mcbi.7_2732.3b", 0x0000, 0x1000, CRC(7d4085eb) SHA1(4f7a7860223ab823cf5b697e17c2e83783442697) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "mcbi.5f.5h",     0x0000, 0x0800, CRC(0bdfad0e) SHA1(8e6f1737604f3801c03fa2e9a5e6a2778b54bae8) )
	ROM_LOAD( "mcbi.5h.3h",     0x0800, 0x0800, CRC(8e062177) SHA1(7e52a1669804b6c2f694cfc64b04abc8246bb0c2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mcbi_prom.1k",   0x0000, 0x0020, CRC(aa1f7f5e) SHA1(311dd17aa11490a1173c76223e4ccccf8ea29850) )
ROM_END

ROM_START( mimonkeyug ) // this bootleg has significant hardware changes: no audio CPU or sound chips, no 8255s. Only discrete sound. Also 0x800 more ROM.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mig.m.1-g.7l-bottom",    0x0000, 0x1000, CRC(5667c124) SHA1(49e5393bc0d2e54c3466e6567a934ed048624dfb) )
	ROM_LOAD( "mig.m.2-g.7f-bottom",    0x1000, 0x1000, CRC(0ddd41d4) SHA1(dfd879738fa9dfbd6335e20291087270329a49dc) )
	ROM_LOAD( "mig.m.3-g.8l-bottom",    0x2000, 0x1000, CRC(6c421e34) SHA1(f5038160617e7e24dcea973f02df84134458abc7) )
	ROM_LOAD( "mig.m.4-g.8i-bottom",    0x3000, 0x1000, CRC(5dec8f11) SHA1(7182b0eef9ea60d5336810e151ce5f8bbfc3b4fc) )
	ROM_LOAD( "mig.m.9-g.8f",           0x4000, 0x0800, CRC(14bb1708) SHA1(724292f1537340e0182087ff5c604f5ecb7b73fb) )
	ROM_LOAD( "mig.m.5-g.7l-top",       0xc000, 0x1000, CRC(b4e5c32d) SHA1(18e53519e8f4e813109cfaf45f2f66444e6fa1a2) )
	ROM_LOAD( "mig.m.6-g.7f-top",       0xd000, 0x1000, CRC(0ad81502) SHA1(f2b46bf645a405a2e7a427592c918cbf553a5aa7) )
	ROM_LOAD( "mig.m.7-g.8l-top",       0xe000, 0x1000, CRC(5093b483) SHA1(ab91b9a8bd36344aae8a7100d0b0effeef3c618c) )
	ROM_LOAD( "mig.m.8-g.8i-top",       0xf000, 0x1000, CRC(6a51c513) SHA1(b570f5ea8d8cf8375e8bd61b1c0998ed2896f46a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "mig.m.10-g.1h-bottom",   0x0000, 0x1000, CRC(f73a8412) SHA1(9baf4336cceb9b039372b0a1c733910aeab5ec6d) )
	ROM_LOAD( "mig.m.11-g.1h-top",      0x1000, 0x1000, CRC(3828c9db) SHA1(eaf9e81c803ad2be6c2db3104f07f80788378286) )
	ROM_LOAD( "mig.m.12-g.1k-bottom",   0x2000, 0x1000, CRC(9e0e9289) SHA1(79d412dbceb364bc798feda658b15792feb63338) )
	ROM_LOAD( "mig.m.13-g.1k-top",      0x3000, 0x1000, CRC(92085b0c) SHA1(a791703fa9f17e42450c871d902430fc3c6b10ef) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sn74s288n.6l",    0x0000, 0x0020, CRC(4e3caeab) SHA1(a25083c3e36d28afdefe4af6e6d4f3155e303625) )
ROM_END

} // anonymous namespace


GAME( 1981, stratgyx,   0,        stratgyx,   stratgyx,   scobra_state,  init_stratgyx, ROT0,   "Konami",                             "Strategy X",                     MACHINE_SUPPORTS_SAVE )
GAME( 1981, stratgys,   stratgyx, stratgyx,   stratgyx,   scobra_state,  init_stratgyx, ROT0,   "Konami (Stern Electronics license)", "Strategy X (Stern Electronics)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, strongx,    stratgyx, stratgyx,   stratgyx,   scobra_state,  init_stratgyx, ROT0,   "bootleg",                            "Strong X",                       MACHINE_SUPPORTS_SAVE )

GAME( 1982, darkplnt,   0,        darkplnt,   darkplnt,   scobra_state,  init_darkplnt, ROT180, "Stern Electronics",                  "Dark Planet", MACHINE_SUPPORTS_SAVE )

GAME( 1982, tazmani2,   tazmania, type2,      tazmani2,   scobra_state,  init_tazmani2, ROT90,  "Stern Electronics",                  "Tazz-Mania (set 2, alt hardware)",              MACHINE_SUPPORTS_SAVE )
GAME( 1982, tazmani3,   tazmania, tazmani3,   tazmani3,   scobra_state,  empty_init,    ROT90,  "bootleg (Arfyc / Rodmar)",           "Tazz-Mania (Arfyc / Rodmar bootleg)",           MACHINE_SUPPORTS_SAVE )
GAME( 1982, tazmaniet,  tazmania, tazmani3,   tazmani3,   scobra_state,  init_tazmaniet,ROT90,  "bootleg (U.R.V. BBCPE)",             "Tazz-Mania - El Trompa (U.R.V. BBCPE bootleg)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1982, rescue,     0,        rescue,     rescue,     scobra_state,  init_rescue,   ROT90,  "Stern Electronics",                  "Rescue",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1982, rescueb,    rescue,   rescueb,    rescue,     scobra_state,  init_rescue,   ROT90,  "bootleg (Videl Games)",              "Tuono Blu (bootleg of Rescue)",           MACHINE_SUPPORTS_SAVE )
GAME( 1982, aponow,     rescue,   rescue,     rescue,     scobra_state,  init_rescue,   ROT90,  "bootleg",                            "Apocaljpse Now (bootleg of Rescue)",      MACHINE_SUPPORTS_SAVE )
GAME( 1982, rescuefe,   rescue,   rescuefe,   rescue,     scobra_state,  empty_init,    ROT90,  "bootleg (Free Enterprise Games)",    "Rescue (Free Enterprise Games, bootleg)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

GAME( 1983, minefld,    0,        minefld,    minefld,    scobra_state,  init_minefld,  ROT90,  "Stern Electronics",                  "Minefield",                          MACHINE_SUPPORTS_SAVE )
GAME( 1983, minefldfe,  minefld,  minefldfe,  minefldfe,  scobra_state,  empty_init,    ROT90,  "bootleg (The Logicshop)",            "Minefield (The Logicshop, bootleg)", MACHINE_SUPPORTS_SAVE ) // The Logicshop ('licensed' from Free Enterprise Games?

GAME( 1981, hustler,    0,        hustler,    hustler,    scobra_state,  init_hustler,  ROT90,  "Konami",                             "Video Hustler",                            MACHINE_SUPPORTS_SAVE )
GAME( 1981, hustlerd,   hustler,  hustler,    hustler,    scobra_state,  init_hustlerd, ROT90,  "Konami (Dynamo Games license)",      "Video Hustler (Dynamo Games)",             MACHINE_SUPPORTS_SAVE )
GAME( 1981, billiard,   hustler,  hustler,    hustler,    scobra_state,  init_billiard, ROT90,  "bootleg",                            "The Billiards (bootleg of Video Hustler)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, hustlerb,   hustler,  hustlerb,   hustler,    scobra_state,  empty_init,    ROT90,  "bootleg (Digimatic)",                "Video Hustler (bootleg, set 1)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, hustlerb2,  hustler,  hustler,    hustler,    scobra_state,  init_hustlerd, ROT90,  "bootleg",                            "Fatsy Gambler (bootleg of Video Hustler)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, hustlerb4,  hustler,  hustlerb4,  hustler,    scobra_state,  empty_init,    ROT90,  "bootleg",                            "Video Hustler (bootleg, set 2)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, hustlerb5,  hustler,  hustlerb,   hustler,    scobra_state,  empty_init,    ROT90,  "bootleg",                            "Video Hustler (bootleg, set 3)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, hustlerb6,  hustler,  hustlerb6,  hustlerb6,  scobra_state,  empty_init,    ROT90,  "bootleg",                            "Video Hustler (bootleg, set 4)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, hustlerb7,  hustler,  hustlerb6,  hustlerb6,  scobra_state,  empty_init,    ROT90,  "bootleg (Marti Colls)",              "Video Hustler (bootleg, set 5)",           MACHINE_SUPPORTS_SAVE )

GAME( 1983, mimonkeyug, mimonkey, mimonkeyug, mimonkeyug, scobra_state,  empty_init,    ROT90,  "bootleg (U.Games)",                  "Mighty Monkey (U.Games bootleg)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE ) // missing discrete sound components emulation
