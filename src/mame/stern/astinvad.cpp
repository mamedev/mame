// license:BSD-3-Clause
// copyright-holders:Lee Taylor
/***************************************************************************

Misc early Z80 games with simple color bitmap graphics

    - Space King 2    (c) Konami
    - Kosmo Killer    bootleg
    - Kamikaze        (c) Leijac Corporation
    - Astro Invader   (c) Stern Electronics
    - Space Intruder  (c) Shoei

Space Intruder emulation by Lee Taylor (lee@defender.demon.co.uk),
    December 1998.

DIP locations verified for:
    - astinvad (manual)

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_samples(*this, "samples")
		, m_screen(*this, "screen")
		, m_color_prom(*this, "proms")
		, m_cabinet(*this, "CABINET")
	{ }

protected:
	void plot_byte(bitmap_rgb32 &bitmap, uint8_t y, uint8_t x, uint8_t data, uint8_t color);

	uint8_t m_sound_state[2]{};
	uint8_t m_screen_flip = 0;

	// sample sound IDs - must match sample file name table below
	enum
	{
		SND_UFO = 0,
		SND_SHOT,
		SND_BASEHIT,
		SND_INVADERHIT,
		SND_FLEET1,
		SND_FLEET2,
		SND_FLEET3,
		SND_FLEET4,
		SND_UFOHIT,
		SND_BONUS
	};

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<samples_device> m_samples;
	required_device<screen_device> m_screen;
	required_region_ptr<uint8_t> m_color_prom;

	required_ioport m_cabinet;
};

class kamikaze_state : public base_state
{
public:
	kamikaze_state(const machine_config &mconfig, device_type type, const char *tag)
		: base_state(mconfig, type, tag)
		, m_ppi8255(*this, "ppi8255_%u", 0U)
	{ }

	void kamikaze(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device_array<i8255_device, 2>  m_ppi8255;
	uint8_t m_flip_yoffs = 32; // the flip screen logic adds 32 to the Y after flipping
	uint8_t m_screen_red = 0;

private:
	uint8_t ppi_r(offs_t offset);
	void ppi_w(offs_t offset, uint8_t data);
	void sound1_w(uint8_t data);
	void sound2_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(int_off);
	TIMER_CALLBACK_MEMBER(int_gen);

	void prg_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;

	emu_timer *m_int_timer = nullptr;
	emu_timer *m_int_off_timer = nullptr;
};

class spcking2_state : public kamikaze_state
{
public:
	spcking2_state(const machine_config &mconfig, device_type type, const char *tag)
		: kamikaze_state(mconfig, type, tag)
	{ m_flip_yoffs = 0; } // don't have the schematics, but the blanking must center the screen here

	void spcking2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void sound1_w(uint8_t data);
	void sound2_w(uint8_t data);
	void sound3_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	bool m_player = false;
};

class spaceint_state : public base_state
{
public:
	spaceint_state(const machine_config &mconfig, device_type type, const char *tag)
		: base_state(mconfig, type, tag)
		, m_colorram(*this, "colorram", 0x2000, ENDIANNESS_LITTLE)
	{ }

	void spaceint(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void color_latch_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void sound1_w(uint8_t data);
	void sound2_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;

	memory_share_creator<uint8_t> m_colorram;
	uint8_t m_color_latch = 0;
};

/*************************************
 *
 *  Spaceint color RAM handling
 *
 *************************************/

void spaceint_state::color_latch_w(uint8_t data)
{
	m_color_latch = data & 0x0f;
}


void spaceint_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_colorram[offset] = m_color_latch;
}


void base_state::plot_byte(bitmap_rgb32 &bitmap, uint8_t y, uint8_t x, uint8_t data, uint8_t color)
{
	uint8_t flip_xor = m_screen_flip & 7;

	for (int i = 0; i < 8; i++)
		bitmap.pix(y, x + (i ^ flip_xor)) = BIT(data, i) ? m_palette->pen_color(color) : rgb_t::black();
}


uint32_t kamikaze_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t yoffs = m_flip_yoffs & m_screen_flip;

	// render the visible pixels
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (int x = cliprect.min_x & ~7; x <= cliprect.max_x; x += 8)
		{
			uint8_t color = m_color_prom[((y & 0xf8) << 2) | (x >> 3)] >> (m_screen_flip ? 0 : 4);
			uint8_t data = m_videoram[(((y ^ m_screen_flip) + yoffs) << 5) | ((x ^ m_screen_flip) >> 3)];
			plot_byte(bitmap, y, x, data, m_screen_red ? 1 : color & 0x07);
		}

	return 0;
}


uint32_t spcking2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t yoffs = m_flip_yoffs & m_screen_flip;

	/* render the visible pixels */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (int x = cliprect.min_x & ~7; x <= cliprect.max_x; x += 8)
		{
			uint8_t color = m_color_prom[(((y & 0xf8) << 2) | (x >> 3)) ^ (m_screen_flip ? 0x3ff : m_player ? 0 : 0x3ff)] >> (m_player ? 4 : 0);
			uint8_t data = m_videoram[(((y ^ m_screen_flip) + yoffs) << 5) | ((x ^ m_screen_flip) >> 3)];
			plot_byte(bitmap, y, x, data, m_screen_red ? 1 : color & 0x07);
		}

	return 0;
}


uint32_t spaceint_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t data = m_videoram[offs];
		uint8_t color = m_colorram[offs];
		uint8_t x, y;

		if (m_screen_flip)
		{
			y = offs;
			x = ~offs >> 8 << 3;
		}
		else
		{
			y = ~offs;
			x = offs >> 8 << 3;
		}

		// this is almost certainly wrong
		offs_t n = ((offs >> 5) & 0xf0) | color;
		color = m_color_prom[n] & 0x07;

		plot_byte(bitmap, y, x, data, color);
	}

	return 0;
}



/*************************************
 *
 *  Interrupts
 *
 *************************************/

TIMER_CALLBACK_MEMBER(kamikaze_state::int_off)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(kamikaze_state::int_gen)
{
	// interrupts are asserted on every state change of the 128V line
	m_maincpu->set_input_line(0, ASSERT_LINE);
	param ^= 128;
	m_int_timer->adjust(m_screen->time_until_pos(param), param);

	// an RC circuit turns the interrupt off after a short amount of time
	m_int_off_timer->adjust(attotime::from_double(300 * 0.1e-6));
}


void kamikaze_state::machine_start()
{
	base_state::machine_start();

	m_int_timer = timer_alloc(FUNC(kamikaze_state::int_gen), this);
	m_int_timer->adjust(m_screen->time_until_pos(128), 128);
	m_int_off_timer = timer_alloc(FUNC(kamikaze_state::int_off), this);

	save_item(NAME(m_screen_flip));
	save_item(NAME(m_screen_red));
	save_item(NAME(m_sound_state));
}

void kamikaze_state::machine_reset()
{
	base_state::machine_reset();

	m_screen_flip = 0;
	m_screen_red = 0;
	m_sound_state[0] = 0;
	m_sound_state[1] = 0;
}


void spaceint_state::machine_start()
{
	base_state::machine_start();

	save_item(NAME(m_screen_flip));
	save_item(NAME(m_sound_state));
	save_item(NAME(m_color_latch));
}

void spaceint_state::machine_reset()
{
	base_state::machine_reset();

	m_screen_flip = 0;
	m_sound_state[0] = 0;
	m_sound_state[1] = 0;
	m_color_latch = 0;
}


void spcking2_state::machine_start()
{
	kamikaze_state::machine_start();

	save_item(NAME(m_player));
}


INPUT_CHANGED_MEMBER(spaceint_state::coin_inserted)
{
	// coin insertion causes an NMI
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  8255 PPI handlers
 *
 *************************************/

uint8_t kamikaze_state::ppi_r(offs_t offset)
{
	uint8_t result = 0xff;

	// the address lines are used for /CS; yes, they can overlap!
	if (!(offset & 4))
		result &= m_ppi8255[0]->read(offset);
	if (!(offset & 8))
		result &= m_ppi8255[1]->read(offset);
	return result;
}


void kamikaze_state::ppi_w(offs_t offset, uint8_t data)
{
	// the address lines are used for /CS; yes, they can overlap!
	if (!(offset & 4))
		m_ppi8255[0]->write(offset, data);
	if (!(offset & 8))
		m_ppi8255[1]->write(offset, data);
}



/*************************************
 *
 *  Sound and I/O port handlers
 *
 *************************************/

// Kamikaze
void kamikaze_state::sound1_w(uint8_t data)
{
	// d0: UFO sound generator
	// d1: fire sound generator
	// d2: tank explosion sound generator
	// d3: invader destroyed sound generator
	// d4: bonus sound generator
	// d5: sound enabled
	// other bits: unused

	int bits_gone_hi = data & ~m_sound_state[0];
	m_sound_state[0] = data;

	if (bits_gone_hi & 0x01) m_samples->start(0, SND_UFO, true);
	if (!(data & 0x01))      m_samples->stop(0);
	if (bits_gone_hi & 0x02) m_samples->start(1, SND_SHOT);
	if (bits_gone_hi & 0x04) m_samples->start(2, SND_BASEHIT);
	if (bits_gone_hi & 0x08) m_samples->start(3, SND_INVADERHIT);
	if (bits_gone_hi & 0x10) m_samples->start(2, SND_BONUS);

	machine().sound().system_mute(!BIT(data, 5));
}

void kamikaze_state::sound2_w(uint8_t data)
{
	// d0: red screen -> to video board
	// d1: invaders advancing sound generator
	// d4: UFO destroyed sound generator
	// d5: flip screen -> to video board
	// other bits: unused

	int bits_gone_hi = data & ~m_sound_state[1];
	m_sound_state[1] = data;

	if (bits_gone_hi & 0x02) m_samples->start(5, SND_FLEET1);
	if (bits_gone_hi & 0x10) m_samples->start(4, SND_UFOHIT);

	m_screen_flip = (m_cabinet->read() & data & 0x20) ? 0xff : 0x00;
	m_screen_red = data & 0x01;
}

// Space King 2
void spcking2_state::sound1_w(uint8_t data)
{
	int bits_gone_hi = data & ~m_sound_state[0];
	m_sound_state[0] = data;

	if (bits_gone_hi & 0x01) m_samples->start(0, SND_UFO, true);
	if (!(data & 0x01))      m_samples->stop(0);
	if (bits_gone_hi & 0x02) m_samples->start(1, SND_SHOT);
	if (bits_gone_hi & 0x04) m_samples->start(2, SND_BASEHIT);
	if (bits_gone_hi & 0x08) m_samples->start(3, SND_INVADERHIT);
	if (bits_gone_hi & 0x10) m_samples->start(2, SND_BONUS);
	machine().sound().system_mute(!BIT(data, 5));
	m_screen_red = data & 0x04; // ?
}

void spcking2_state::sound2_w(uint8_t data)
{
	int bits_gone_hi = data & ~m_sound_state[1];
	m_sound_state[1] = data;

	if (bits_gone_hi & 0x01) m_samples->start(5, SND_FLEET1);
	if (bits_gone_hi & 0x02) m_samples->start(5, SND_FLEET2);
	if (bits_gone_hi & 0x04) m_samples->start(5, SND_FLEET3);
	if (bits_gone_hi & 0x08) m_samples->start(5, SND_FLEET4);
	if (bits_gone_hi & 0x10) m_samples->start(4, SND_UFOHIT);

	m_screen_flip = (m_cabinet->read() & data & 0x20) ? 0xff : 0x00;
	m_player = BIT(data, 5);
}

void spcking2_state::sound3_w(uint8_t data)
{
	// ?
}

// Space Intruder
void spaceint_state::sound1_w(uint8_t data)
{
	int bits_gone_hi = data & ~m_sound_state[0];
	m_sound_state[0] = data;

	if (bits_gone_hi & 0x01) m_samples->start(1, SND_SHOT);
	if (bits_gone_hi & 0x02) m_samples->start(2, SND_BASEHIT);
	if (bits_gone_hi & 0x04) m_samples->start(4, SND_UFOHIT);
	if (bits_gone_hi & 0x08) m_samples->start(0, SND_UFO, true);
	if (!(data & 0x08))      m_samples->stop(0);

	if (bits_gone_hi & 0x10) m_samples->start(5, SND_FLEET1);
	if (bits_gone_hi & 0x20) m_samples->start(5, SND_FLEET2);
	if (bits_gone_hi & 0x40) m_samples->start(5, SND_FLEET3);
	if (bits_gone_hi & 0x80) m_samples->start(5, SND_FLEET4);
}

void spaceint_state::sound2_w(uint8_t data)
{
	int bits_gone_hi = data & ~m_sound_state[1];
	m_sound_state[1] = data;

	machine().sound().system_mute(!BIT(data, 1));

	if (bits_gone_hi & 0x04) m_samples->start(3, SND_INVADERHIT);

	m_screen_flip = (m_cabinet->read() & data & 0x80) ? 0xff : 0x00;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void kamikaze_state::prg_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x1bff).rom();
	map(0x1c00, 0x1fff).ram();
	map(0x2000, 0x3fff).ram().share(m_videoram);
}


void spaceint_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x4000, 0x5fff).ram().w(FUNC(spaceint_state::videoram_w)).share(m_videoram);
}


void kamikaze_state::port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(kamikaze_state::ppi_r), FUNC(kamikaze_state::ppi_w));
}


void spaceint_state::port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).w(FUNC(spaceint_state::sound1_w));
	map(0x03, 0x03).w(FUNC(spaceint_state::color_latch_w));
	map(0x04, 0x04).w(FUNC(spaceint_state::sound2_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( kamikaze )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x88, 0x88, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x88, "5000" )
	PORT_DIPSETTING(    0x80, "10000" )
	PORT_DIPSETTING(    0x08, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("CABINET")
	PORT_DIPNAME( 0xff, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0xff, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( astinvad )
	PORT_INCLUDE(kamikaze)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x88, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x88, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( spcking2 )
	PORT_INCLUDE(kamikaze)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Info" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( spaceint )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )            // code at 0x0d4a
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x06, "5" )                         // duplicate settings
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(spaceint_state::coin_inserted), 0)

	PORT_START("CABINET")
	PORT_DIPNAME( 0xff, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0xff, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( spaceintj )
	PORT_INCLUDE( spaceint )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )            // code at 0x0d37
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x04, "5" )                         // duplicate settings
	PORT_DIPSETTING(    0x06, "5" )                         // duplicate settings
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const char *const astinvad_sample_names[] =
{
	"*invaders",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	nullptr
};


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void kamikaze_state::kamikaze(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &kamikaze_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &kamikaze_state::port_map);

	I8255A(config, m_ppi8255[0]);
	m_ppi8255[0]->in_pa_callback().set_ioport("IN0");
	m_ppi8255[0]->in_pb_callback().set_ioport("IN1");
	m_ppi8255[0]->in_pc_callback().set_ioport("IN2");

	I8255A(config, m_ppi8255[1]);
	m_ppi8255[1]->out_pa_callback().set(FUNC(kamikaze_state::sound1_w));
	m_ppi8255[1]->out_pb_callback().set(FUNC(kamikaze_state::sound2_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(4'915'200), 320, 0, 256, 256, 32, 256);
	m_screen->set_screen_update(FUNC(kamikaze_state::screen_update));

	PALETTE(config, m_palette, palette_device::RBG_3BIT);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(astinvad_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void spcking2_state::spcking2(machine_config &config)
{
	kamikaze(config);

	// basic machine hardware
	m_ppi8255[1]->out_pa_callback().set(FUNC(spcking2_state::sound1_w));
	m_ppi8255[1]->out_pb_callback().set(FUNC(spcking2_state::sound2_w));
	m_ppi8255[1]->out_pc_callback().set(FUNC(spcking2_state::sound3_w));

	// video hardware
	m_screen->set_raw(XTAL(4'915'200), 320, 0, 256, 256, 16, 240);
	m_screen->set_screen_update(FUNC(spcking2_state::screen_update));
}

void spaceint_state::spaceint(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 2'000'000);        // a guess
	m_maincpu->set_addrmap(AS_PROGRAM, &spaceint_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &spaceint_state::port_map);
	m_maincpu->set_vblank_int("screen", FUNC(spaceint_state::irq0_line_hold));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 1*8, 31*8-1);
	m_screen->set_refresh_hz(60);
	m_screen->set_screen_update(FUNC(spaceint_state::screen_update));

	PALETTE(config, m_palette, palette_device::RBG_3BIT);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(astinvad_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( kamikaze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "km01",         0x0000, 0x0800, CRC(8aae7414) SHA1(91cb5c268a03960d50401000903d70dc29f904fb) )
	ROM_LOAD( "km02",         0x0800, 0x0800, CRC(6c7a2beb) SHA1(86447d077a58e8c1fc096d0d32b02d18523019a6) )
	ROM_LOAD( "km03",         0x1000, 0x0800, CRC(3e8dedb6) SHA1(19679d0e8ebe2d19dc766b12a07335b1220fb568) )
	ROM_LOAD( "km04",         0x1800, 0x0800, CRC(494e1f6d) SHA1(f9626072d80897a977c10fe9523a8b608f1f7b7c) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ai_vid_c.rom", 0x0000, 0x0400, BAD_DUMP CRC(b45287ff) SHA1(7e558eaf402641d7ff60171f854030219fbf9a59) )
ROM_END

ROM_START( astinvad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ai_cpu_1.rom", 0x0000, 0x0400, CRC(20e3ec41) SHA1(7e77fa3c51d1e83ce91a24808301d9f1e0bed18e) )
	ROM_LOAD( "ai_cpu_2.rom", 0x0400, 0x0400, CRC(e8f1ab55) SHA1(b3e38f2d6bdb65ee7c53c8d5dd3951a3fd43c51c) )
	ROM_LOAD( "ai_cpu_3.rom", 0x0800, 0x0400, CRC(a0092553) SHA1(34fced8ce06d912980ba45fad8d80d2a2e3357b9) )
	ROM_LOAD( "ai_cpu_4.rom", 0x0c00, 0x0400, CRC(be14185c) SHA1(59ecf450682dab9840c891c18ccda1d5ec4cc954) )
	ROM_LOAD( "ai_cpu_5.rom", 0x1000, 0x0400, CRC(fee681ec) SHA1(b4b94f62e598030e6a432a0bb83d18d0e342aed9) )
	ROM_LOAD( "ai_cpu_6.rom", 0x1400, 0x0400, CRC(eb338863) SHA1(e841c6c5903dd6dee9ec2fedaff431f4a31d738a) )
	ROM_LOAD( "ai_cpu_7.rom", 0x1800, 0x0400, CRC(16dcfea4) SHA1(b6a0e206a604297f548ac4658664e98b2d04f75f) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ai_vid_c.rom", 0x0000, 0x0400, CRC(b45287ff) SHA1(7e558eaf402641d7ff60171f854030219fbf9a59) )
ROM_END

ROM_START( astinvadb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "killer2_1.bin", 0x0000, 0x0400, CRC(20e3ec41) SHA1(7e77fa3c51d1e83ce91a24808301d9f1e0bed18e) )
	ROM_LOAD( "killer2_2.bin", 0x0400, 0x0400, CRC(581625cf) SHA1(adac0c1f27c3f3c02ec14c1db8dc73febe01545f) )
	ROM_LOAD( "killer2_3.bin", 0x0800, 0x0400, CRC(7ea9b6d6) SHA1(d9f9a3a0e0c68e022dec6c3c9a8266cdce06cb64) )
	ROM_LOAD( "killer2_4.bin", 0x0c00, 0x0400, CRC(0d305d5f) SHA1(1581717d6c0472b5adb36f3d35cccb63dc4ba209) )
	ROM_LOAD( "killer2_5.bin", 0x1000, 0x0400, CRC(fee681ec) SHA1(b4b94f62e598030e6a432a0bb83d18d0e342aed9) )
	ROM_LOAD( "killer2_6.bin", 0x1400, 0x0400, CRC(eb338863) SHA1(e841c6c5903dd6dee9ec2fedaff431f4a31d738a) )
	ROM_LOAD( "killer2_7.bin", 0x1800, 0x0400, CRC(9e2d279d) SHA1(357835761974ace956c965c0dd920a0692a5a2ea) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "killer2_1-8.bin", 0x0000, 0x0400, CRC(d62a3e62) SHA1(00d42988203fbf167791cf5b887f06d1d015e942) )
ROM_END

ROM_START( kosmokil )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",     0x0000, 0x0400, CRC(8d851fb2) SHA1(06b3816ecb45c5d034447a875669a96f443339f5) )
	ROM_LOAD( "us1-2.bin", 0x0400, 0x0400, CRC(786599d2) SHA1(70db8dae052c3556948d75b741ef4346aa947479) )
	ROM_LOAD( "si1-3.bin", 0x0800, 0x0400, CRC(12621222) SHA1(062b1dff3e129dff23e55bef0d29c72ac5f212c4) )
	ROM_LOAD( "ib1-4.bin", 0x0c00, 0x0400, CRC(a5c56156) SHA1(557f5fbb5e9fe4d7450f3cf97ed3c935b9dd5c1a) )
	ROM_LOAD( "bi1-5.bin", 0x1000, 0x0400, CRC(5e8b2b6f) SHA1(ec8499325d5a3dcb0d10e9f12b9d3a03f629bbfd) )
	ROM_LOAD( "il1-6.bin", 0x1400, 0x0400, CRC(a076de05) SHA1(bdb076b89795d69824c99a27473089f203690055) )
	ROM_LOAD( "li1-7.bin", 0x1800, 0x0400, CRC(ee9adb63) SHA1(038e6cadfdfe05c45a79ed9a54cff428de33d24c) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "40.bin", 0x0000, 0x0400,  CRC(d62a3e62) SHA1(00d42988203fbf167791cf5b887f06d1d015e942) )
ROM_END

ROM_START( betafrce ) // 3 ROMs are identical to the kosmokil ones
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1", 0x0000, 0x0400, CRC(d5e4ce6b) SHA1(19d8b9df8cbfe674ea45b0137d98951117a4b890) )
	ROM_LOAD( "2k", 0x0400, 0x0400, CRC(786599d2) SHA1(70db8dae052c3556948d75b741ef4346aa947479) )
	ROM_LOAD( "3k", 0x0800, 0x0400, CRC(12621222) SHA1(062b1dff3e129dff23e55bef0d29c72ac5f212c4) )
	ROM_LOAD( "4k", 0x0c00, 0x0400, CRC(0c5b8988) SHA1(a77fd2c58ee640973653b8af34b7ed3d81cae935) )
	ROM_LOAD( "5k", 0x1000, 0x0400, CRC(5e8b2b6f) SHA1(ec8499325d5a3dcb0d10e9f12b9d3a03f629bbfd) )
	ROM_LOAD( "6k", 0x1400, 0x0400, CRC(20cd429b) SHA1(a7d3216c005a7905b3549a4eee6c47fc5a9621fc) )
	ROM_LOAD( "7k", 0x1800, 0x0400, CRC(637bf292) SHA1(baf40f5a04331c4cf2b83830f3203bd65a2bb271) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "40.bin", 0x0000, 0x0400, BAD_DUMP CRC(d62a3e62) SHA1(00d42988203fbf167791cf5b887f06d1d015e942) ) // not dumped for this set, taken from kosmokil
ROM_END

ROM_START( spcking2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0400, CRC(716fe9e0) SHA1(d5131abf6e3e6650ff9f649a999bf1d8ae8afb78) )
	ROM_LOAD( "2.bin",        0x0400, 0x0400, CRC(6f6d4e5c) SHA1(0269c3b9da2723411c16ee13ff53e2140e49e7ff) )
	ROM_LOAD( "3.bin",        0x0800, 0x0400, CRC(2ab1c280) SHA1(62cb2445b3f859bddd5617e4ebfb37eedf8bd11e) )
	ROM_LOAD( "4.bin",        0x0c00, 0x0400, CRC(07ba1f21) SHA1(26468e142edef3475e71320292bd1817552a9218) )
	ROM_LOAD( "5.bin",        0x1000, 0x0400, CRC(b084c074) SHA1(1c7e86ae35cd69679712cd8a209b4a70a2075163) )
	ROM_LOAD( "6.bin",        0x1400, 0x0400, CRC(b53d7791) SHA1(45415bcccb03a9c61cea611df807b011e8cc0d2d) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "c.bin",        0x0000, 0x0400, CRC(d27fe595) SHA1(1781281110b57ab3a5eef7a3dbaa93f11c013554) )
ROM_END

ROM_START( spaceint )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",            0x0000, 0x0400, CRC(184314d2) SHA1(76789780c46e19c73904b229d23c865819915558) )
	ROM_LOAD( "2",            0x0400, 0x0400, CRC(55459aa1) SHA1(5631d8de4e41682962cde65002b0fe86f2b189f9) )
	ROM_LOAD( "3",            0x0800, 0x0400, CRC(9d6819be) SHA1(da061b908ca6a9f3312d6adc4395a138eed473c8) )
	ROM_LOAD( "4",            0x0c00, 0x0400, CRC(432052d4) SHA1(0c944c91cc7b1f03cd817250af13238eb62539ec) )
	ROM_LOAD( "5",            0x1000, 0x0400, CRC(c6cfa650) SHA1(afdfaedddf6703101856944bb49ba13fc40ede39) )
	ROM_LOAD( "6",            0x1400, 0x0400, CRC(c7ccf40f) SHA1(10efe05a4e0625ce427871fbb6e55df112fdd783) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "clr",          0x0000, 0x0100, CRC(13c1803f) SHA1(da59bf63d9e84aca32904c107674bc89974648eb) )
ROM_END

ROM_START( spaceintj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3j",           0x0000, 0x0800, CRC(b26c57a1) SHA1(456330c09130f910e847ef4bfe773421615d1448) )
	ROM_LOAD( "3f",           0x0800, 0x0800, CRC(bac8b96c) SHA1(5a7b24402c7a1a08e69cf15eb31c93d411a7e929) )
	ROM_LOAD( "3e",           0x1000, 0x0800, CRC(346125f3) SHA1(59c120ac3b120fa28acef3b9041c03939f2981f8) )
	ROM_LOAD( "3d",           0x1800, 0x0800, CRC(3a3a261f) SHA1(0604ec621180016acab804b57ac405e434d6f0c0) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "clr",          0x0000, 0x0100, BAD_DUMP CRC(13c1803f) SHA1(da59bf63d9e84aca32904c107674bc89974648eb) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980,  kamikaze,  0,        kamikaze, kamikaze,  kamikaze_state, empty_init, ROT270, "Konami (Leijac Corporation license)", "Kamikaze", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980,  astinvad,  kamikaze, kamikaze, astinvad,  kamikaze_state, empty_init, ROT270, "Konami (Stern Electronics license)", "Astro Invader", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980?, astinvadb, kamikaze, kamikaze, astinvad,  kamikaze_state, empty_init, ROT270, "bootleg", "Astro Invader (bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980?, kosmokil,  kamikaze, kamikaze, kamikaze,  kamikaze_state, empty_init, ROT270, "bootleg (BEM)", "Kosmo Killer", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // says >BEM< Mi Italy but it looks hacked in, different revision of game tho.
GAME( 1980?, betafrce,  kamikaze, kamikaze, kamikaze,  kamikaze_state, empty_init, ROT270, "bootleg (Omni)", "Beta Force", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979,  spcking2,  0,        spcking2, spcking2,  spcking2_state, empty_init, ROT270, "Konami", "Space King 2", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980,  spaceint,  0,        spaceint, spaceint,  spaceint_state, empty_init, ROT90,  "Shoei", "Space Intruder", MACHINE_IMPERFECT_SOUND | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1980,  spaceintj, spaceint, spaceint, spaceintj, spaceint_state, empty_init, ROT90,  "Shoei", "Space Intruder (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
