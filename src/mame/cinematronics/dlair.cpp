// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Cinematronics Dragon's Lair system

    Games supported:
        * Dragon's Lair
        * Dragon's Lair (European version)
        * Space Ace
        * Space Ace (European version)

**************************************************************************

    TODO:
    - No viable solution for reading configuration DIPs at init time,
      so only LD-V1000 versions are supported at this time.

**************************************************************************

    There are two revisions of the Cinematronics board used in the
    U.S. Rev A


    ROM Revisions
    -------------
    Revision A, B, and C EPROMs use the Pioneer PR-7820 only.
    Revision D EPROMs used the Pioneer LD-V1000 only.
    Revisions E, F, and F2 EPROMs used either player.

    Revisions A, B, C, and D are a five chip set.
    Revisions E, F, and F2 are a four chip set.


    Laserdisc Players Used
    ----------------------
    Pioneer PR-7820 (USA / Cinematronics)
    Pioneer LD-V1000 (USA / Cinematronics)
    Philips 22VP932 (Europe / Atari) and (Italian / Sidam)

*************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/ldv1000hle.h"
#include "machine/ldstub.h"
#include "machine/watchdog.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/ay8910.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "speaker.h"

#include "dlair.lh"


namespace {

class dlair_state : public driver_device
{
public:
	dlair_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ldv1000(*this, "ld_ldv1000"),
		m_pr7820(*this, "ld_pr7820"),
		m_22vp932(*this, "ld_22vp932"),
		m_videoram(*this, "videoram"),
		m_digits(*this, "digit%u", 0U)
	{
	}

	int laserdisc_status_r();
	int laserdisc_command_r();

	void dlair_base(machine_config &config);
	void dlair_pr7820(machine_config &config);
	void dleuro(machine_config &config);
	void dlair_ldv1000(machine_config &config);

private:
	void laserdisc_data_w(uint8_t data)
	{
		if (m_ldv1000 != nullptr) m_ldv1000->data_w(data);
		if (m_pr7820 != nullptr) m_pr7820->data_w(data);
		if (m_22vp932 != nullptr) m_22vp932->data_w(data);
	}

	void laserdisc_enter_w(uint8_t data)
	{
		if (m_pr7820 != nullptr) m_pr7820->enter_w(data);
		if (m_22vp932 != nullptr) m_22vp932->enter_w(data);
	}

	uint8_t laserdisc_data_r()
	{
		if (m_ldv1000 != nullptr) return m_ldv1000->data_r();
		if (m_pr7820 != nullptr) return m_pr7820->data_r();
		if (m_22vp932 != nullptr) return m_22vp932->data_r();
		return 0;
	}

	uint8_t laserdisc_data_available_r()
	{
		return CLEAR_LINE;
	}

	uint8_t laserdisc_status_strobe_r()
	{
		if (m_ldv1000 != nullptr) return m_ldv1000->status_strobe_r();
		return CLEAR_LINE;
	}

	uint8_t laserdisc_ready_r()
	{
		if (m_ldv1000 != nullptr) return m_ldv1000->ready_r();
		if (m_pr7820 != nullptr) return m_pr7820->ready_r();
		return CLEAR_LINE;
	}

	void misc_w(uint8_t data);
	void dleuro_misc_w(uint8_t data);
	void led_den1_w(offs_t offset, uint8_t data);
	void led_den2_w(offs_t offset, uint8_t data);
	uint8_t laserdisc_r();
	void laserdisc_w(uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void dleuro_palette(palette_device &palette) const;
	uint32_t screen_update_dleuro(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void write_speaker(int state);

	void dleuro_io_map(address_map &map) ATTR_COLD;
	void dleuro_map(address_map &map) ATTR_COLD;
	void dlus_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<gfxdecode_device> m_gfxdecode;
	optional_device<palette_device> m_palette;
	optional_device<pioneer_ldv1000hle_device> m_ldv1000;
	optional_device<pioneer_pr7820_device> m_pr7820;
	optional_device<philips_22vp932_device> m_22vp932;
	optional_shared_ptr<uint8_t> m_videoram;
	output_finder<16> m_digits;

	uint8_t m_last_misc = 0;
	uint8_t m_laserdisc_data = 0;
};




/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK_US             16000000
#define MASTER_CLOCK_EURO           14318180



/*************************************
 *
 *  Globals
 *
 *************************************/

static const uint8_t led_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x77,0x7c,0x39,0x5e,0x79,0x00 };



/*************************************
 *
 *  Z80 peripheral interfaces
 *
 *************************************/

void dlair_state::write_speaker(int state)
{
	m_speaker->level_w(state);
}


static const z80_daisy_config dleuro_daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};



/*************************************
 *
 *  Video startup/shutdown
 *
 *************************************/

void dlair_state::dleuro_palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(2 * i + 0, rgb_t(0, 0, 0));
		palette.set_pen_color(2 * i + 1, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t dlair_state::screen_update_dleuro(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// redraw the overlay
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			uint8_t const *const base = &m_videoram[y * 64 + x * 2 + 1];
			// TODO: opaque?
			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, base[0], base[1], 0, 0, 10 * x, 16 * y);
		}
	}

	return 0;
}



/*************************************
 *
 *  Machine startup/reset
 *
 *************************************/

void dlair_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_last_misc));
	save_item(NAME(m_laserdisc_data));
}


void dlair_state::machine_reset()
{
#if 0

	/* determine the laserdisc player from the DIP switches */
	if (m_laserdisc_type == LASERDISC_TYPE_VARIABLE)
	{
		int newtype = (ioport("DSW2")->read() & 0x08) ? LASERDISC_TYPE_PIONEER_LDV1000 : LASERDISC_TYPE_PIONEER_PR7820;
		laserdisc_set_type(m_laserdisc, newtype);
	}
#endif
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

void dlair_state::misc_w(uint8_t data)
{
	/*
	    D0-D3 = B0-B3
	       D4 = coin counter
	       D5 = OUT DISC DATA
	       D6 = ENTER
	       D7 = INT/EXT
	*/
	uint8_t diff = data ^ m_last_misc;
	m_last_misc = data;

	machine().bookkeeping().coin_counter_w(0, (~data >> 4) & 1);

	/* on bit 5 going low, push the data out to the laserdisc player */
	if ((diff & 0x20) && !(data & 0x20))
		laserdisc_data_w(m_laserdisc_data);

	/* on bit 6 going low, we need to signal enter */
	laserdisc_enter_w((data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
}


void dlair_state::dleuro_misc_w(uint8_t data)
{
	/*
	       D0 = CHAR GEN ON+
	       D1 = KILL VIDEO+
	       D2 = SEL CHAR GEN VIDEO+
	       D3 = counter 2
	       D4 = coin counter
	       D5 = OUT DISC DATA
	       D6 = ENTER
	       D7 = INT/EXT
	*/
	uint8_t diff = data ^ m_last_misc;
	m_last_misc = data;

	machine().bookkeeping().coin_counter_w(1, (~data >> 3) & 1);
	machine().bookkeeping().coin_counter_w(0, (~data >> 4) & 1);

	/* on bit 5 going low, push the data out to the laserdisc player */
	if ((diff & 0x20) && !(data & 0x20))
		laserdisc_data_w(m_laserdisc_data);

	/* on bit 6 going low, we need to signal enter */
	laserdisc_enter_w((data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
}


void dlair_state::led_den1_w(offs_t offset, uint8_t data)
{
	m_digits[0 | (offset & 7)] = led_map[data & 0x0f];
}


void dlair_state::led_den2_w(offs_t offset, uint8_t data)
{
	m_digits[8 | (offset & 7)] = led_map[data & 0x0f];
}



/*************************************
 *
 *  Laserdisc communication
 *
 *************************************/

int dlair_state::laserdisc_status_r()
{
	return laserdisc_status_strobe_r();
}


int dlair_state::laserdisc_command_r()
{
	return (laserdisc_ready_r() == ASSERT_LINE) ? 0 : 1;
}


uint8_t dlair_state::laserdisc_r()
{
	uint8_t result = laserdisc_data_r();
	osd_printf_debug("laserdisc_r = %02X\n", result);
	return result;
}


void dlair_state::laserdisc_w(uint8_t data)
{
	m_laserdisc_data = data;
}



/*************************************
 *
 *  U.S. version memory map
 *
 *************************************/

/* complete memory map derived from schematics */
void dlair_state::dlus_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xa7ff).mirror(0x1800).ram();
	map(0xc000, 0xc000).mirror(0x1fc7).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xc008, 0xc008).mirror(0x1fc7).portr("P1");
	map(0xc010, 0xc010).mirror(0x1fc7).portr("SYSTEM");
	map(0xc020, 0xc020).mirror(0x1fc7).r(FUNC(dlair_state::laserdisc_r));
	map(0xe000, 0xe000).mirror(0x1fc7).w("aysnd", FUNC(ay8910_device::data_w));
	map(0xe008, 0xe008).mirror(0x1fc7).w(FUNC(dlair_state::misc_w));
	map(0xe010, 0xe010).mirror(0x1fc7).w("aysnd", FUNC(ay8910_device::address_w));
	map(0xe020, 0xe020).mirror(0x1fc7).w(FUNC(dlair_state::laserdisc_w));
	map(0xe030, 0xe037).mirror(0x1fc0).w(FUNC(dlair_state::led_den2_w));
	map(0xe038, 0xe03f).mirror(0x1fc0).w(FUNC(dlair_state::led_den1_w));
}



/*************************************
 *
 *  European version memory map
 *
 *************************************/

/* complete memory map derived from schematics */
void dlair_state::dleuro_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xa7ff).mirror(0x1800).ram();
	map(0xc000, 0xc7ff).mirror(0x1800).ram().share("videoram");
	map(0xe000, 0xe000).mirror(0x1f47); // WT LED 1
	map(0xe008, 0xe008).mirror(0x1f47); // WT LED 2
	map(0xe010, 0xe010).mirror(0x1f47).w(FUNC(dlair_state::led_den1_w));         // WT EXT LED 1
	map(0xe018, 0xe018).mirror(0x1f47).w(FUNC(dlair_state::led_den2_w));         // WT EXT LED 2
	map(0xe020, 0xe020).mirror(0x1f47).w(FUNC(dlair_state::laserdisc_w));        // DISC WT
	map(0xe028, 0xe028).mirror(0x1f47).w(FUNC(dlair_state::dleuro_misc_w));      // WT MISC
	map(0xe030, 0xe030).mirror(0x1f47).w("watchdog", FUNC(watchdog_timer_device::reset_w));   // CLR WDOG
	map(0xe080, 0xe080).mirror(0x1f47).portr("P1");           // CP A
	map(0xe088, 0xe088).mirror(0x1f47).portr("SYSTEM");       // CP B
	map(0xe090, 0xe090).mirror(0x1f47).portr("DSW1");         // OPT SW A
	map(0xe098, 0xe098).mirror(0x1f47).portr("DSW2");         // OPT SW B
	map(0xe0a0, 0xe0a0).mirror(0x1f47).r(FUNC(dlair_state::laserdisc_r));         // RD DISC DATA
}


/* complete memory map derived from schematics */
void dlair_state::dleuro_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0x7c).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x80, 0x83).mirror(0x7c).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
}


/*
WT LED 1 -> LS273 -> DS1 = 7segment
WT LED 2 -> LS273 -> DS2 = 7segment

Character generator:
  decade counter LS90
  cleared on HSYNC
  clocked each pixel
  on 0 -> VOUTLD- -> load RGB color info and character bits (D0=red, D1=green, D2=blue)
  on 3 -> RADRA- -> advance RA0-RA5
  on 6 -> LCHAR- -> load next character into address lines on ROM
  on 7 -> RADRB- -> advance RA0-RA5
  on 9 -> 0

Serial in on the shift register is 0, so extra bits are 0

Video clock:
7.16MHz 0 1 0 1 0 1 0 1 0 1
ACLK+   0 0 1 1 0 0 1 1 0 0 = 3.58Mhz
ACLK-   1 1 0 0 1 1 0 0 1 1 = 3.58Mhz
BCLK+   0 0 0 1 1 0 0 1 1 0 = 3.58Mhz
BCLK-   1 1 1 0 0 1 1 0 0 1 = 3.58Mhz

Row clock:
Cleared on VSYNC
Clocked on HSYNC
ROWCLK- = Qa (i.e., row / 2)

Line clock:
Cleared on VSYNC
Clocked on ROWCLK-
LINECLK- = Qc (i.e., ROWCLK / 8 = row / 16)

Video RAM = 11-bit = 2048 bytes
If CHAR GEN ON+, RAM address = RA0-RA10
If not CHAR GEN ON+, RAM address = BA0-BA10

RA0-RA5 = counters (64 across)
  Cleared to 0 on HSYNC
  Clocked on !(RADRA- & RADRB-)

RA6-RA10 = counters (32 up & down)
  Cleared to 0 on VSYNC
  Clocked on LINECLK- (every 16 rows)
  RA6 = Qb (i.e., counts by 2, not 1)

MA0-MA3 = counters
  Cleared to 0 on VSYNC
  Clocked by ROWCLK- (every 2 rows)

Address in ROM:
  MA0-MA3     = A0-A3
  D0-D7       = A4-A11 (data from VRAM)
  KILL VIDEO+ = A12
*/



/*************************************
 *
 *  Port definitions
 *
 *************************************/

	// TODO: DIPs still needs work
static INPUT_PORTS_START( dlair )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("A:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_HIGH, "A:2")
	PORT_DIPNAME( 0x04, 0x00, "Difficulty Mode" ) PORT_DIPLOCATION("A:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x00, "Engineering Mode" ) PORT_DIPLOCATION("A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "2 Credits/Free play" ) PORT_DIPLOCATION("A:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("A:6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Pay as you go" ) PORT_DIPLOCATION("A:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "A:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Sound Every 8 Attracts" ) PORT_DIPLOCATION("B:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("B:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Unlimited Dirks" ) PORT_DIPLOCATION("B:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Joystick Feedback Sound" ) PORT_DIPLOCATION("B:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Pay as you go options" ) PORT_DIPLOCATION("B:7,6")
	PORT_DIPSETTING(    0x00, "PAYG1" )
	PORT_DIPSETTING(    0x20, "PAYG2" )
	PORT_DIPSETTING(    0x40, "PAYG3" )
	PORT_DIPSETTING(    0x60, "PAYG4" )
	PORT_DIPNAME( 0x90, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("B:8,5")
	PORT_DIPSETTING(    0x00, "Increase after 5" ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, "Increase after 9" ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x90, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x90, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x04)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(dlair_state, laserdisc_status_r)     /* status strobe */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(dlair_state, laserdisc_command_r)    /* command strobe */
INPUT_PORTS_END


static INPUT_PORTS_START( dlaire )
	PORT_INCLUDE(dlair)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, "LD Player" )  PORT_DIPLOCATION("B:3")    /* In Rev F, F2 and so on... before it was Joystick Sound Feedback */
	PORT_DIPSETTING(    0x08, "LD-V1000" )
	PORT_DIPSETTING(    0x00, "LD-PR7820" )
INPUT_PORTS_END


static INPUT_PORTS_START( dleuro )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(dlair_state, laserdisc_status_r)     /* status strobe */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(dlair_state, laserdisc_command_r)    /* command strobe */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("A:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_HIGH, "A:2")
	PORT_DIPNAME( 0x04, 0x00, "Difficulty Mode" ) PORT_DIPLOCATION("A:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x00, "Engineering mode" ) PORT_DIPLOCATION("A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "2 Credits/Free play" ) PORT_DIPLOCATION("A:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("A:6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Pay as you go" ) PORT_DIPLOCATION("A:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "A:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Sound Every 8 Attracts" ) PORT_DIPLOCATION("B:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("B:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Unlimited Dirks" ) PORT_DIPLOCATION("B:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Joystick Feedback Sound" ) PORT_DIPLOCATION("B:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Pay as you go options" ) PORT_DIPLOCATION("B:7,6")
	PORT_DIPSETTING(    0x00, "PAYG1" )
	PORT_DIPSETTING(    0x20, "PAYG2" )
	PORT_DIPSETTING(    0x40, "PAYG3" )
	PORT_DIPSETTING(    0x60, "PAYG4" )
	PORT_DIPNAME( 0x90, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("B:8,5s")
	PORT_DIPSETTING(    0x00, "Increase after 5" ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, "Increase after 9" ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x90, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x04)
	PORT_DIPSETTING(    0x90, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, EQUALS, 0x04)
INPUT_PORTS_END

static INPUT_PORTS_START( spaceace )
	PORT_INCLUDE(dlair)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("A:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("A:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("A:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) ) // 5 chapters without losing life
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) ) // 3
	PORT_DIPNAME( 0x08, 0x00, "Difficulty Rank Increase" ) PORT_DIPLOCATION("A:4")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("A:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound Every 8 Attracts" ) PORT_DIPLOCATION("A:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_HIGH, "A:7")
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_HIGH, "A:8")

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "LD Player" ) PORT_DIPLOCATION("B:1")
	PORT_DIPSETTING(    0x00, "LD-V1000" )
	PORT_DIPSETTING(    0x01, "LD-PR7820" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_HIGH, "B:2")
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_HIGH, "B:3")
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_HIGH, "B:4")
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("B:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Unlimited Lives" ) PORT_DIPLOCATION("B:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Enable Frame Display" ) PORT_DIPLOCATION("B:7") // ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "B:8" ) // "Diagnostic Mode", plays the whole disc from start to finish, start buttons makes the disc go back one chapter. Setting this to off makes it to execute host CPU tests before proceeding to game.

INPUT_PORTS_END

// TODO: dips for Space Ace euro, different than NTSC

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(7,-1) },
	{ STEP16(0,8) },
	16*8
};


static GFXDECODE_START( gfx_dlair )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 8 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void dlair_state::dlair_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK_US/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &dlair_state::dlus_map);
	m_maincpu->set_periodic_int(FUNC(dlair_state::irq0_line_hold),  attotime::from_hz((double)MASTER_CLOCK_US/8/16/16/16/16));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK_US/8));
	aysnd.port_a_read_callback().set_ioport("DSW1");
	aysnd.port_b_read_callback().set_ioport("DSW2");
	aysnd.add_route(ALL_OUTPUTS, "rspeaker", 0.33);
}


void dlair_state::dlair_pr7820(machine_config &config)
{
	dlair_base(config);
	PIONEER_PR7820(config, m_pr7820, 0);
	m_pr7820->add_route(0, "lspeaker", 1.0);
	m_pr7820->add_route(1, "rspeaker", 1.0);
	m_pr7820->add_ntsc_screen(config, "screen");
}


void dlair_state::dlair_ldv1000(machine_config &config)
{
	dlair_base(config);
	PIONEER_LDV1000HLE(config, m_ldv1000, 0);
	m_ldv1000->add_route(0, "lspeaker", 1.0);
	m_ldv1000->add_route(1, "rspeaker", 1.0);
	m_ldv1000->add_ntsc_screen(config, "screen");
}


void dlair_state::dleuro(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK_EURO/4);
	m_maincpu->set_daisy_config(dleuro_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &dlair_state::dleuro_map);
	m_maincpu->set_addrmap(AS_IO, &dlair_state::dleuro_io_map);

	z80ctc_device& ctc(Z80CTC(config, "ctc", MASTER_CLOCK_EURO/4 /* same as "maincpu" */));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set(FUNC(dlair_state::write_speaker));

	z80sio_device& sio(Z80SIO(config, "sio", MASTER_CLOCK_EURO/4 /* same as "maincpu" */));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	// TODO: hook up tx and rx callbacks

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_hz(MASTER_CLOCK_EURO/(16*16*16*16*16*8)));

	PHILIPS_22VP932(config, m_22vp932, 0);
	m_22vp932->set_overlay(256, 256, FUNC(dlair_state::screen_update_dleuro));
	m_22vp932->add_route(0, "lspeaker", 1.0);
	m_22vp932->add_route(1, "rspeaker", 1.0);

	/* video hardware */
	m_22vp932->add_pal_screen(config, "screen");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dlair);
	PALETTE(config, m_palette, FUNC(dlair_state::dleuro_palette), 16);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "lspeaker", 0.33);
	m_speaker->add_route(ALL_OUTPUTS, "rspeaker", 0.33);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( dlair )      /* revision F2 */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_f2_u1.bin", 0x0000, 0x2000,  CRC(f5ea3b9d) SHA1(c0cafff8b2982125fd3314ffc66681e47f027fc9) )
	ROM_LOAD( "dl_f2_u2.bin", 0x2000, 0x2000,  CRC(dcc1dff2) SHA1(614ca8f6c5b6fa1d590f6b80d731377faa3a65a9) )
	ROM_LOAD( "dl_f2_u3.bin", 0x4000, 0x2000,  CRC(ab514e5b) SHA1(29d1015b951f0f2d4e5257497f3bf007c5e2262c) )
	ROM_LOAD( "dl_f2_u4.bin", 0x6000, 0x2000,  CRC(f5ec23d2) SHA1(71149e2d359cc5944fbbb53dd7d0c2b42fbc9bb4) )

	DISK_REGION( "ld_ldv1000" )
	DISK_IMAGE_READONLY( "dlair", 0, SHA1(c3ee2e27aa4847bf3884fc0d18f26f315456aa9e) )
ROM_END

ROM_START( dlair_1 )     /* Serial #001, courtesy Jason Finn */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_n1_u1.bin", 0x0000, 0x2000,  CRC(a1856eac) SHA1(f5c1daeec3d4a3c14399f181dc4dfafdc43acc51) )
	ROM_LOAD( "dl_n1_u2.bin", 0x2000, 0x2000,  CRC(1b34406f) SHA1(f78b2e7558e28fa81fdea4d72d68348fc11d224c) )
	ROM_LOAD( "dl_n1_u3.bin", 0x4000, 0x2000,  CRC(cf3f4d3c) SHA1(af263d914902c74d4090dc213be5b9edbde9e1ae) )
	ROM_LOAD( "dl_n1_u4.bin", 0x6000, 0x2000,  CRC(a98880c5) SHA1(48eec445f6f40db18b0f66c777bff475cf33571a) )
	ROM_LOAD( "dl_n1_u5.bin", 0x8000, 0x2000,  CRC(17b7336b) SHA1(f5b7fc6b2f100a7cf7c6e7e31492bf08a82199f1) )

	DISK_REGION( "ld_pr7820" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlair_2 )
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dlu1.bin", 0x0000, 0x2000,  CRC(ea6d5498) SHA1(1d854c04e0074e693e791e22a4c9cc21d5175d95) )
	ROM_LOAD( "dlu2.bin", 0x2000, 0x2000,  CRC(ffe84a95) SHA1(675ce2e68e43beb1f389bc6ab1a55bee862a1440) )
	ROM_LOAD( "dlu3.bin", 0x4000, 0x2000,  CRC(6363fd84) SHA1(d865495337ed77952b60ca267ce52e1e9e01224b) )
	ROM_LOAD( "dlu4.bin", 0x6000, 0x2000,  CRC(84cabb86) SHA1(eac6ba4c5989ba67d914c9c84f91cf7a1e86accf) )
	ROM_LOAD( "dlu5.bin", 0x8000, 0x2000,  CRC(8cc8f073) SHA1(78bbd7992224f4f273672d2fc1d64661f9200a77) )

	DISK_REGION( "ld_pr7820" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlaira )     /* revision A */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_a_u1.bin", 0x0000, 0x2000,  CRC(d76e83ec) SHA1(fc7ff5d883de9b38a9e0532c35990f4b319ba1d3) )
	ROM_LOAD( "dl_a_u2.bin", 0x2000, 0x2000,  CRC(a6a723d8) SHA1(5c71cb0b6be7331083adaf6fac6bdfc8445cb485) )
	ROM_LOAD( "dl_a_u3.bin", 0x4000, 0x2000,  CRC(52c59014) SHA1(d4015046bf1c1f51c29d9d9f8e8d008519b61cd1) )
	ROM_LOAD( "dl_a_u4.bin", 0x6000, 0x2000,  CRC(924d12f2) SHA1(05b487e651a4817991dfc2308834b8f2fae918b4) )
	ROM_LOAD( "dl_a_u5.bin", 0x8000, 0x2000,  CRC(6ec2f9c1) SHA1(0b8026927697a99fe8fa0dd4bd643418779a1d45) )

	DISK_REGION( "ld_pr7820" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlairb )     /* revision B */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_b_u1.bin", 0x0000, 0x2000,  CRC(d76e83ec) SHA1(fc7ff5d883de9b38a9e0532c35990f4b319ba1d3) )
	ROM_LOAD( "dl_b_u2.bin", 0x2000, 0x2000,  CRC(6751103d) SHA1(e94e19f738e0eb69700e56c6069c7f3c0911303f) )
	ROM_LOAD( "dl_b_u3.bin", 0x4000, 0x2000,  CRC(52c59014) SHA1(d4015046bf1c1f51c29d9d9f8e8d008519b61cd1) )
	ROM_LOAD( "dl_b_u4.bin", 0x6000, 0x2000,  CRC(924d12f2) SHA1(05b487e651a4817991dfc2308834b8f2fae918b4) )
	ROM_LOAD( "dl_b_u5.bin", 0x8000, 0x2000,  CRC(6ec2f9c1) SHA1(0b8026927697a99fe8fa0dd4bd643418779a1d45) )

	DISK_REGION( "ld_pr7820" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlairc )     /* revision C */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_c_u1.bin", 0x0000, 0x2000,  CRC(cebfe26a) SHA1(1c808de5c92fef67d8088621fbd743c1a0a3bb5e) )
	ROM_LOAD( "dl_c_u2.bin", 0x2000, 0x2000,  CRC(6751103d) SHA1(e94e19f738e0eb69700e56c6069c7f3c0911303f) )
	ROM_LOAD( "dl_c_u3.bin", 0x4000, 0x2000,  CRC(52c59014) SHA1(d4015046bf1c1f51c29d9d9f8e8d008519b61cd1) )
	ROM_LOAD( "dl_c_u4.bin", 0x6000, 0x2000,  CRC(924d12f2) SHA1(05b487e651a4817991dfc2308834b8f2fae918b4) )
	ROM_LOAD( "dl_c_u5.bin", 0x8000, 0x2000,  CRC(6ec2f9c1) SHA1(0b8026927697a99fe8fa0dd4bd643418779a1d45) )

	DISK_REGION( "ld_pr7820" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlaird )     /* revision D */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_d_u1.bin", 0x0000, 0x2000,  CRC(0b5ab120) SHA1(6ec59d6aaa27994d8de4f5635935fd6c1d42d2f6) )
	ROM_LOAD( "dl_d_u2.bin", 0x2000, 0x2000,  CRC(93ebfffb) SHA1(2a8f6d7ab18845e22a2ba238b44d7c636908a125) )
	ROM_LOAD( "dl_d_u3.bin", 0x4000, 0x2000,  CRC(22e6591f) SHA1(3176c07af6d942496c9ae338e3b93e28e2ce7982) )
	ROM_LOAD( "dl_d_u4.bin", 0x6000, 0x2000,  CRC(5f7212cb) SHA1(69c34de1bb44b6cd2adc2947d00d8823d3e87130) )
	ROM_LOAD( "dl_d_u5.bin", 0x8000, 0x2000,  CRC(2b469c89) SHA1(646394b51325ca9163221a43b5af64a8067eb80b) )

	DISK_REGION( "ld_ldv1000" )
	DISK_IMAGE_READONLY( "dlair", 0, SHA1(c3ee2e27aa4847bf3884fc0d18f26f315456aa9e) )
ROM_END

ROM_START( dlaire )     /* revision E */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_e_u1.bin", 0x0000, 0x2000,  CRC(02980426) SHA1(409de05045adbd054bc1fda24d4a9672832e2fae) )
	ROM_LOAD( "dl_e_u2.bin", 0x2000, 0x2000,  CRC(979d4c97) SHA1(5da6ceab5029ac5f5846bf52841675c5c70b17af) )
	ROM_LOAD( "dl_e_u3.bin", 0x4000, 0x2000,  CRC(897bf075) SHA1(d2ff9c2fec37544cfe8fb60273524c6610488502) )
	ROM_LOAD( "dl_e_u4.bin", 0x6000, 0x2000,  CRC(4ebffba5) SHA1(d04711247ffa88e371ec461465dd75a8158d90bc) )

	DISK_REGION( "ld_ldv1000" )
	DISK_IMAGE_READONLY( "dlair", 0, SHA1(c3ee2e27aa4847bf3884fc0d18f26f315456aa9e) )
ROM_END

ROM_START( dlairf )     /* revision F */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_f_u1.bin", 0x0000, 0x2000,  CRC(06fc6941) SHA1(ea8cf6d370f89d60721ab00ec58ff24027b5252f) )
	ROM_LOAD( "dl_f_u2.bin", 0x2000, 0x2000,  CRC(dcc1dff2) SHA1(614ca8f6c5b6fa1d590f6b80d731377faa3a65a9) )
	ROM_LOAD( "dl_f_u3.bin", 0x4000, 0x2000,  CRC(ab514e5b) SHA1(29d1015b951f0f2d4e5257497f3bf007c5e2262c) )
	ROM_LOAD( "dl_f_u4.bin", 0x6000, 0x2000,  CRC(a817324e) SHA1(1299c83342fc70932f67bda8ae60bace91d66429) )

	DISK_REGION( "ld_ldv1000" )
	DISK_IMAGE_READONLY( "dlair", 0, SHA1(c3ee2e27aa4847bf3884fc0d18f26f315456aa9e) )
ROM_END

ROM_START( dleuro )     /* European Atari version */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "elu45.bin", 0x0000, 0x2000, CRC(4d3a9eac) SHA1(e6cd274b4a0f92b1fb1f013f80f6fd2db3212431) )
	ROM_LOAD( "elu46.bin", 0x2000, 0x2000, CRC(8479612b) SHA1(b5543a06928274bde0e1bdda0747d936feaff177) )
	ROM_LOAD( "elu47.bin", 0x4000, 0x2000, CRC(6a66f6b4) SHA1(2bee981870e61977565439c34568952043656cfa) )
	ROM_LOAD( "elu48.bin", 0x6000, 0x2000, CRC(36575106) SHA1(178e26e7d5c7f879bc55c2fb170f3bb47a709610) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "elu33.bin", 0x0000, 0x2000, CRC(e7506d96) SHA1(610ae25bd8db13b18b9e681e855ffa978043255b) )

	DISK_REGION( "ld_22vp932" )
	DISK_IMAGE_READONLY( "dleuro", 0, NO_DUMP )
ROM_END

ROM_START( dleuroalt )     /* European Atari version */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "elu45_d2.bin",0x0000, 0x2000, CRC(329b354a) SHA1(54bbc5aa647d3c20166a57f9d3aa5569e7289af8) )
	ROM_LOAD( "elu46.bin",   0x2000, 0x2000, CRC(8479612b) SHA1(b5543a06928274bde0e1bdda0747d936feaff177) )
	ROM_LOAD( "elu47.bin",   0x4000, 0x2000, CRC(6a66f6b4) SHA1(2bee981870e61977565439c34568952043656cfa) )
	ROM_LOAD( "elu48.bin",   0x6000, 0x2000, CRC(36575106) SHA1(178e26e7d5c7f879bc55c2fb170f3bb47a709610) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "elu33.bin", 0x0000, 0x2000, CRC(e7506d96) SHA1(610ae25bd8db13b18b9e681e855ffa978043255b) )

	DISK_REGION( "ld_22vp932" )
	DISK_IMAGE_READONLY( "dleuro", 0, NO_DUMP )
ROM_END

ROM_START( dlital )     /* Italian Sidam version */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dlita45.bin", 0x0000, 0x2000, CRC(2ed85958) SHA1(9651989775d215b279716b8c3e30d8e799d91b37) ) /* Label: ELU 45 SID */
	ROM_LOAD( "dlita46.bin", 0x2000, 0x2000, CRC(8479612b) SHA1(b5543a06928274bde0e1bdda0747d936feaff177) ) /* Label: ELU 46 REV.B */
	ROM_LOAD( "dlita47.bin", 0x4000, 0x2000, CRC(6a66f6b4) SHA1(2bee981870e61977565439c34568952043656cfa) ) /* Label: ELU 47 REV.B */
	ROM_LOAD( "dlita48.bin", 0x6000, 0x2000, CRC(0c0b3011) SHA1(c3ac7bf870dd4ef12609c9cf25a7da68204b2889) ) /* Label: ELU 48 SID */

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dlita33.bin", 0x0000, 0x2000, CRC(e7506d96) SHA1(610ae25bd8db13b18b9e681e855ffa978043255b) ) /* Label: ELU 33 REV.B */

	DISK_REGION( "ld_22vp932" )
	DISK_IMAGE_READONLY( "dleuro", 0, NO_DUMP )
ROM_END


ROM_START( spaceace )       /* revision A3 */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "sa_a3_u1.bin", 0x0000, 0x2000,  CRC(427522d0) SHA1(de4d5353af0be3e60afe1ed13d1d531c425cdb4d) )
	ROM_LOAD( "sa_a3_u2.bin", 0x2000, 0x2000,  CRC(18d0262d) SHA1(c3920e3cabfe2b2add51881e262f090c5018e508) )
	ROM_LOAD( "sa_a3_u3.bin", 0x4000, 0x2000,  CRC(4646832d) SHA1(9f1370b13cca9857b0ed13f58641ef4ba3c7326d) )
	ROM_LOAD( "sa_a3_u4.bin", 0x6000, 0x2000,  CRC(57db2a79) SHA1(5286905d9bde697845a98bd77f31f2a96a8874fc) )
	ROM_LOAD( "sa_a3_u5.bin", 0x8000, 0x2000,  CRC(85cbcdc4) SHA1(97e01e96c885ab7af4c3a3b586eb40374d54f12f) )

	DISK_REGION( "ld_ldv1000" )
	DISK_IMAGE_READONLY( "space_ace_ver2", 0, NO_DUMP )
ROM_END

ROM_START( spaceacea2 )     /* revision A2 */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "sa_a2_u1.bin", 0x0000, 0x2000,  CRC(71b39e27) SHA1(15a34eee9d541b186761a78b5c97449c7b496e4f) )
	ROM_LOAD( "sa_a2_u2.bin", 0x2000, 0x2000,  CRC(18d0262d) SHA1(c3920e3cabfe2b2add51881e262f090c5018e508) )
	ROM_LOAD( "sa_a2_u3.bin", 0x4000, 0x2000,  CRC(4646832d) SHA1(9f1370b13cca9857b0ed13f58641ef4ba3c7326d) )
	ROM_LOAD( "sa_a2_u4.bin", 0x6000, 0x2000,  CRC(57db2a79) SHA1(5286905d9bde697845a98bd77f31f2a96a8874fc) )
	ROM_LOAD( "sa_a2_u5.bin", 0x8000, 0x2000,  CRC(85cbcdc4) SHA1(97e01e96c885ab7af4c3a3b586eb40374d54f12f) )

	DISK_REGION( "ld_ldv1000" )
	DISK_IMAGE_READONLY( "space_ace_ver2", 0, NO_DUMP )
ROM_END

ROM_START( spaceacea )      /* revision A */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "sa_a_u1.bin", 0x0000, 0x2000,  CRC(8eb1889e) SHA1(bfa2c5fc139c448b7b6b5c5757d4f2f74e610b85) )
	ROM_LOAD( "sa_a_u2.bin", 0x2000, 0x2000,  CRC(18d0262d) SHA1(c3920e3cabfe2b2add51881e262f090c5018e508) )
	ROM_LOAD( "sa_a_u3.bin", 0x4000, 0x2000,  CRC(4646832d) SHA1(9f1370b13cca9857b0ed13f58641ef4ba3c7326d) )
	ROM_LOAD( "sa_a_u4.bin", 0x6000, 0x2000,  CRC(57db2a79) SHA1(5286905d9bde697845a98bd77f31f2a96a8874fc) )
	ROM_LOAD( "sa_a_u5.bin", 0x8000, 0x2000,  CRC(85cbcdc4) SHA1(97e01e96c885ab7af4c3a3b586eb40374d54f12f) )

	DISK_REGION( "ld_ldv1000" )
	DISK_IMAGE_READONLY( "space_ace_ver2", 0, NO_DUMP )
ROM_END

ROM_START( spaceaceeuro )       /* Italian Sidam version */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sa_u45a.bin", 0x0000, 0x2000, CRC(41264d46) SHA1(3e0ecfb3249f857a29fe58a3853a55d31cbd63d6) )
	ROM_LOAD( "sa_u46a.bin", 0x2000, 0x2000, CRC(bc1c70cf) SHA1(cd6d2456ac2fbbfb86e1f31bd7cbd0cec0d31b45) )
	ROM_LOAD( "sa_u47a.bin", 0x4000, 0x2000, CRC(ff3f77c7) SHA1(d10ffd14ab9853cef8085c70aedfabea4059657e) )
	ROM_LOAD( "sa_u48a.bin", 0x6000, 0x2000, CRC(8c83ac81) SHA1(12818ee51ae8028a84bbbf3e43904b62942c76e3) )
	ROM_LOAD( "sa_u49a.bin", 0x6000, 0x2000, CRC(03b58fc3) SHA1(25e4c1df74e2d7cbb9e252e34f007bc1c9f015b2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sa_u33a.bin", 0x0000, 0x2000, CRC(a8c14612) SHA1(dbcf90b929e714f328bdcb0d8cd7c9e7d08a8be7) )

	DISK_REGION( "ld_22vp932" )
	DISK_IMAGE_READONLY( "saeuro", 0, NO_DUMP )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1983, dlair,        0,        dlair_ldv1000, dlaire,   dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Rev. F2)", MACHINE_SUPPORTS_SAVE, layout_dlair )
GAMEL( 1983, dlairf,       dlair,    dlair_ldv1000, dlaire,   dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Rev. F)",  MACHINE_SUPPORTS_SAVE, layout_dlair )
GAMEL( 1983, dlaire,       dlair,    dlair_ldv1000, dlaire,   dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Rev. E)",  MACHINE_SUPPORTS_SAVE, layout_dlair )
GAMEL( 1983, dlaird,       dlair,    dlair_ldv1000, dlair,    dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Rev. D, Pioneer LD-V1000)",  MACHINE_SUPPORTS_SAVE, layout_dlair )
GAMEL( 1983, dlairc,       dlair,    dlair_pr7820,  dlair,    dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Rev. C, Pioneer PR-7820)",  MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlairb,       dlair,    dlair_pr7820,  dlair,    dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Rev. B, Pioneer PR-7820)",  MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlaira,       dlair,    dlair_pr7820,  dlair,    dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Rev. A, Pioneer PR-7820)",  MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlair_2,      dlair,    dlair_pr7820,  dlair,    dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Beta 2?, Pioneer PR-7820)",  MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlair_1,      dlair,    dlair_pr7820,  dlair,    dlair_state, empty_init, ROT0, "Cinematronics", "Dragon's Lair (US Beta 1, Pioneer PR-7820)",  MACHINE_NOT_WORKING, layout_dlair )

GAMEL( 1983, dleuro,       dlair,    dleuro,        dleuro,   dlair_state, empty_init, ROT0, "Cinematronics (Atari license)", "Dragon's Lair (European)",  MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, dleuroalt,    dlair,    dleuro,        dleuro,   dlair_state, empty_init, ROT0, "Cinematronics (Atari license)", "Dragon's Lair (European, alternate)",  MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlital,       dlair,    dleuro,        dleuro,   dlair_state, empty_init, ROT0, "Cinematronics (Sidam license?)","Dragon's Lair (Italian)",  MACHINE_NOT_WORKING, layout_dlair )

GAMEL( 1983, spaceace,     0,        dlair_ldv1000, spaceace, dlair_state, empty_init, ROT0, "Cinematronics", "Space Ace (US Rev. A3)", MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, spaceacea2,   spaceace, dlair_ldv1000, spaceace, dlair_state, empty_init, ROT0, "Cinematronics", "Space Ace (US Rev. A2)", MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, spaceacea,    spaceace, dlair_ldv1000, spaceace, dlair_state, empty_init, ROT0, "Cinematronics", "Space Ace (US Rev. A)", MACHINE_NOT_WORKING, layout_dlair )
GAMEL( 1983, spaceaceeuro, spaceace, dleuro,        spaceace, dlair_state, empty_init, ROT0, "Cinematronics (Atari license)", "Space Ace (European)",  MACHINE_NOT_WORKING, layout_dlair )
