// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

  Nintendo 8080 hardware

- Space Fever / Color Space Fever
- SF-HiSplitter
- Space Launcher
- Sheriff / Bandido / Western Gun 2
- HeliFire

Space Fever was initially produced with B&W video hardware. It was later sold
(maybe upgraded too) featuring color graphics, known as "Color Space Fever".
The other Space Fever hardware games were only sold with color graphics, but
they run fine on the older B&W video hardware. For example, doing a ROM swap
on the older Space Fever.

TODO:
- spacefev sound pitch for laser fire and enemy explosion is wrong

----------------------------------------------------------------------------

Space Fever (3 sets, Space Fever?, High Splitter?, Space Launcher?)
Nintendo, 1979

Note: These are all simple ROM swaps on a standard b/w Space Fever PCB.


PCB Layouts
-----------

Top Board (Sound PCB)

TSF-SOU
|----------------------------------------------------|
|                                 VR3    VR2    VR1  |
|  8035                    74123                     |
|                 74275                              |
|  6MHz                                              |
|                          74123         SN76477     |
|  SF_SOUND.IC2   74275                              |
|                                                    |
|                          7405                      |
|                                                    |
|----------------------------------------------------|
Notes:
      All IC's shown.
      There is no AMP on the PCB, sound amplification is done via a small external AMP board.
      ROM IC2 is a 2708 EPROM.
      VR1: master volume
      VR2: shoot volume
      VR3: music volume
      8035 clocks: pins 2 and 3 measure 6.000MHz
                   pin 9 measures 399.256kHz
                   pin 12 measures 200.0kHz
                   pin 13 measures 105.0kHz
                   pin 21 measures 399.4Khz
                   pin 22 measures 400.0kHz
                   pin 23 measures 399.3kHz
                   pin 24 measures 399.3kHz
                   pin 39 measures 61.5627Hz


Middle board
------------

TSF-I/O  PI-500803
|----------------------------------------------------|
|                                                    |
|     VR1                                            |
|                     20.160MHz                      |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                     DSW1(8)                        |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|----------------------------------------------------|
Notes:
      VR1: adjusts brightness
      Board contains mostly logic ICs (not shown)
      Video output is b/w, the harness is wired to a JAMMA fingerboard but only blue is used.


Bottom board
------------

TSF-CPU  PI-500802
|----------------------------------------------------|
|                                                    |
|                                                    |
|                                                    |
|             SF_F1.F1  SF_G1.G1  SF_H1.H1  SF_I1.I1 |
|                                                    |
|   8080                                             |
|                                                    |
|             SF_F2.F2  SF_G2.G2  SF_H2.H2  SF_I2.I2 |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|                                                    |
|     4116  4116  4116  4116  4116  4116  4116  4116 |
|----------------------------------------------------|
Notes:
      All ROMs are 2708, 1K x8
      4116: 2K x8 DRAM
      8080 clock: 2.0160MHz (20.160 / 10)
      Sync: no V reading, H is 15.57kHz

      Set 1 is on the PCB and is complete.
      Some ROMs in set1 match the current sfeverbw set.

      The other two sets were supplied as just EPROMs.
      Set2 (maybe High Splitter) is missing the ROM at location I2. Might be missing, or maybe
      just the program is smaller and the extra ROM was not required.

***************************************************************************/

#include "emu.h"

#include "n8080_a.h"

#include "cpu/i8085/i8085.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"


namespace {

class n8080_state : public driver_device
{
public:
	n8080_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_prom(*this, "proms"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_sound(*this, "soundboard")
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void delayed_sound_1(u8 data);
	virtual void delayed_sound_2(u8 data);

	void n8080_shift_bits_w(u8 data);
	void n8080_shift_data_w(u8 data);
	u8 n8080_shift_r();
	void n8080_video_control_w(u8 data);
	void n8080_sound_1_w(u8 data);
	void n8080_sound_2_w(u8 data);
	void n8080_inte_callback(int state);
	void n8080_status_callback(u8 data);
	void n8080_palette(palette_device &palette) const ATTR_COLD;
	TIMER_DEVICE_CALLBACK_MEMBER(rst1_tick);
	TIMER_DEVICE_CALLBACK_MEMBER(rst2_tick);
	TIMER_CALLBACK_MEMBER(stop_mono_flop_callback);
	TIMER_CALLBACK_MEMBER(delayed_sound_1_callback);
	TIMER_CALLBACK_MEMBER(delayed_sound_2_callback);

	void main_cpu_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;

	/* memory pointers */
	required_shared_ptr<u8> m_videoram;
	optional_memory_region m_prom;

	/* video-related */
	u8 m_sheriff_color_mode;
	u8 m_sheriff_color_data;

	/* other */
	unsigned m_shift_data;
	u8 m_shift_bits;
	bool m_inte;

	/* devices */
	required_device<i8080a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<n8080_sound_device_base> m_sound;
};

class spacefev_state : public n8080_state
{
public:
	spacefev_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD :
		n8080_state(mconfig, type, tag),
		m_video_conf(*this, "VIDEO")
	{ }

	void spacefev(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void delayed_sound_1(u8 data) override;
	virtual void delayed_sound_2(u8 data) override;

private:
	TIMER_CALLBACK_MEMBER(stop_red_cannon);
	void start_red_cannon();
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_ioport m_video_conf;

	emu_timer *m_cannon_timer;

	u8 m_prev_snd_data = 0;
	bool m_red_screen = false;
	bool m_red_cannon = false;
};

class sheriff_state : public n8080_state
{
public:
	sheriff_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD :
		n8080_state(mconfig, type, tag)
	{ }

	void sheriff(machine_config &config) ATTR_COLD;
	void westgun2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

class helifire_state : public n8080_state
{
public:
	helifire_state(const machine_config &mconfig, device_type type, const char *tag) ATTR_COLD :
		n8080_state(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_pot(*this, "POT%u", 0)
	{ }

	void helifire(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void delayed_sound_2(u8 data) override;

private:
	void helifire_palette(palette_device &palette) const ATTR_COLD;
	void next_line();
	void screen_vblank(int state);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_cpu_map(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_colorram;
	required_ioport_array<2> m_pot;

	bool m_flash = false;
	u8 m_LSFR[63];
	unsigned m_mv = 0;
	unsigned m_sc = 0; // IC56
};


// Shifter circuit done with TTL

void n8080_state::n8080_shift_bits_w(u8 data)
{
	m_shift_bits = data & 7;
}

void n8080_state::n8080_shift_data_w(u8 data)
{
	m_shift_data = (m_shift_data >> 8) | (data << 8);
}

u8 n8080_state::n8080_shift_r()
{
	return m_shift_data >> (8 - m_shift_bits);
}


void n8080_state::delayed_sound_1(u8 data)
{
	m_sound->sound1_w(data);
}

void spacefev_state::delayed_sound_1(u8 data)
{
	if (BIT(data & ~m_prev_snd_data, 4))
		start_red_cannon();

	m_red_screen = BIT(data, 3);

	m_prev_snd_data = data;

	n8080_state::delayed_sound_1(data);
}


void n8080_state::delayed_sound_2(u8 data)
{
	m_sound->sound2_w(data);
}

void spacefev_state::delayed_sound_2(u8 data)
{
	flip_screen_set(BIT(data, 5));

	n8080_state::delayed_sound_2(data);
}

void helifire_state::delayed_sound_2(u8 data)
{
	m_flash = BIT(data, 5);

	n8080_state::delayed_sound_2(data);
}


void n8080_state::n8080_video_control_w(u8 data)
{
	m_sheriff_color_mode = (data >> 3) & 3;
	m_sheriff_color_data = (data >> 0) & 7;
	flip_screen_set(data & 0x20);
}


void n8080_state::n8080_palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
}


void helifire_state::helifire_palette(palette_device &palette) const
{
	n8080_palette(palette);

	for (int i = 0; i < 0x100; i++)
	{
		int const level = 0xff * exp(-3 * i / 255.); // capacitor discharge

		palette.set_pen_color(0x000 + 8 + i, rgb_t(0x00, 0x00, level)); // shades of blue
		palette.set_pen_color(0x100 + 8 + i, rgb_t(0x00, 0xc0, level)); // shades of blue w/ green star

		palette.set_pen_color(0x200 + 8 + i, rgb_t(level, 0x00, 0x00)); // shades of red
		palette.set_pen_color(0x300 + 8 + i, rgb_t(level, 0xc0, 0x00)); // shades of red w/ green star
	}
}


void spacefev_state::start_red_cannon()
{
	m_red_cannon = true;
	m_cannon_timer->adjust(attotime::from_usec(550 * 68 * 10));
}


TIMER_CALLBACK_MEMBER(spacefev_state::stop_red_cannon)
{
	m_red_cannon = false;
	m_cannon_timer->adjust(attotime::never);
}


void helifire_state::next_line()
{
	m_mv++;

	if (m_sc % 4 == 2)
	{
		m_mv %= 256;
	}
	else
	{
		if (flip_screen())
			m_mv %= 255;
		else
			m_mv %= 257;
	}

	if (m_mv == 128)
	{
		m_sc++;
	}
}


u32 spacefev_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const bool mono = bool(m_video_conf->read());
	u8 mask = flip_screen() ? 0xff : 0x00;

	u8 const *pRAM = m_videoram;
	u8 const *const pPROM = m_prom->base();

	for (int y = 0; y < 256; y++)
	{
		u16 *const pLine = &bitmap.pix(y ^ mask);

		for (int x = 0; x < 256; x += 8)
		{
			u8 color = 0;

			if (m_red_screen)
				color = 1;
			else
			{
				u8 val = pPROM[x >> 3];

				if ((x >> 3) == 0x06)
				{
					color = m_red_cannon ? 1 : 7;
				}

				if ((x >> 3) == 0x1b)
				{
					static const u8 ufo_color[] =
					{
						1, // red
						2, // green
						7, // white
						3, // yellow
						5, // magenta
						6, // cyan
					};

					int cycle = screen.frame_number() / 32;

					color = ufo_color[cycle % 6];
				}

				for (int n = color + 1; n < 8; n++)
				{
					if (~val & (1 << n))
					{
						color = n;
					}
				}
			}

			if (mono)
				color = 7; // force B&W here

			for (int n = 0; n < 8; n++)
			{
				pLine[(x + n) ^ mask] = (pRAM[x >> 3] & (1 << n)) ? color : 0;
			}
		}

		pRAM += 32;
	}
	return 0;
}


u32 sheriff_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 mask = flip_screen() ? 0xff : 0x00;

	u8 const *pRAM = m_videoram;
	u8 const *const pPROM = m_prom->base();

	for (int y = 0; y < 256; y++)
	{
		u16 *const pLine = &bitmap.pix(y ^ mask);

		for (int x = 0; x < 256; x += 8)
		{
			u8 color = pPROM[32 * (y >> 3) + (x >> 3)];

			if (m_sheriff_color_mode == 1 && !(color & 8))
				color = m_sheriff_color_data ^ 7;

			if (m_sheriff_color_mode == 2)
				color = m_sheriff_color_data ^ 7;

			if (m_sheriff_color_mode == 3)
				color = 7;

			for (int n = 0; n < 8; n++)
			{
				pLine[(x + n) ^ mask] = ((pRAM[x >> 3] >> n) & 1) ? (color & 7) : 0;
			}
		}

		pRAM += 32;
	}
	return 0;
}


u32 helifire_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int SUN_BRIGHTNESS = m_pot[0]->read();
	const int SEA_BRIGHTNESS = m_pot[1]->read();

	static const int wave[8] = { 0, 1, 2, 2, 2, 1, 0, 0 };

	unsigned saved_mv = m_mv;
	unsigned saved_sc = m_sc;

	for (int y = 0; y < 256; y++)
	{
		u16 *const pLine = &bitmap.pix(y);

		int level = 120 + wave[m_mv & 7];

		// draw sky
		for (int x = level; x < 256; x++)
		{
			pLine[x] = 0x200 + 8 + SUN_BRIGHTNESS + x - level;
		}

		// draw stars
		if (m_mv % 8 == 4) // upper half
		{
			int step = (320 * (m_mv - 0)) % sizeof m_LSFR;

			int data =
				((m_LSFR[step] & 1) << 6) |
				((m_LSFR[step] & 2) << 4) |
				((m_LSFR[step] & 4) << 2) |
				((m_LSFR[step] & 8) << 0);

			pLine[0x80 + data] |= 0x100;
		}

		if (m_mv % 8 == 5) // lower half
		{
			int step = (320 * (m_mv - 1)) % sizeof m_LSFR;

			int data =
				((m_LSFR[step] & 1) << 6) |
				((m_LSFR[step] & 2) << 4) |
				((m_LSFR[step] & 4) << 2) |
				((m_LSFR[step] & 8) << 0);

			pLine[0x00 + data] |= 0x100;
		}

		// draw sea
		for (int x = 0; x < level; x++)
		{
			pLine[x] = 8 + SEA_BRIGHTNESS + x;
		}

		// draw foreground
		for (int x = 0; x < 256; x += 8)
		{
			int offset = 32 * y + (x >> 3);

			for (int n = 0; n < 8; n++)
			{
				if (flip_screen())
				{
					if ((m_videoram[offset ^ 0x1fff] << n) & 0x80)
					{
						pLine[x + n] = m_colorram[offset ^ 0x1fff] & 7;
					}
				}
				else
				{
					if ((m_videoram[offset] >> n) & 1)
					{
						pLine[x + n] = m_colorram[offset] & 7;
					}
				}
			}
		}

		// next line
		next_line();
	}

	m_mv = saved_mv;
	m_sc = saved_sc;
	return 0;
}


void helifire_state::screen_vblank(int state)
{
	// falling edge
	if (!state)
	{
		int n = (m_screen->frame_number() >> 1) % sizeof m_LSFR;

		int i;

		for (i = 0; i < 8; i++)
		{
			int R = (i & 1);
			int G = (i & 2);
			int B = (i & 4);

			if (m_flash)
			{
				if (m_LSFR[n] & 0x20)
				{
					G |= B;
				}

				if (m_screen->frame_number() & 0x04)
				{
					R |= G;
				}
			}

			m_palette->set_pen_color(i,
				R ? 255 : 0,
				G ? 255 : 0,
				B ? 255 : 0);
		}

		for (i = 0; i < 256; i++)
		{
			next_line();
		}
	}
}


void n8080_state::n8080_sound_1_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(n8080_state::delayed_sound_1_callback), this), data); // force CPUs to sync
}

void n8080_state::n8080_sound_2_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(n8080_state::delayed_sound_2_callback), this), data); // force CPUs to sync
}

TIMER_CALLBACK_MEMBER(n8080_state::delayed_sound_1_callback)
{
	delayed_sound_1(param);
}

TIMER_CALLBACK_MEMBER(n8080_state::delayed_sound_2_callback)
{
	delayed_sound_2(param);
}


// Memory maps

void n8080_state::main_cpu_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).ram().share("videoram");
}

void helifire_state::main_cpu_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).ram().share("videoram");
	map(0xc000, 0xdfff).ram().share("colorram");
}

void n8080_state::main_io_map(address_map &map)
{
	map.global_mask(0x7);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).r(FUNC(n8080_state::n8080_shift_r));
	map(0x04, 0x04).portr("IN3");

	map(0x02, 0x02).w(FUNC(n8080_state::n8080_shift_bits_w));
	map(0x03, 0x03).w(FUNC(n8080_state::n8080_shift_data_w));
	map(0x04, 0x04).w(FUNC(n8080_state::n8080_sound_1_w));
	map(0x05, 0x05).w(FUNC(n8080_state::n8080_sound_2_w));
	map(0x06, 0x06).w(FUNC(n8080_state::n8080_video_control_w));
}


// Input ports

INPUT_PORTS_START( spacefev )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Game A")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Game B")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Game C")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // enables diagnostic ROM at $1c00 ($2000 for highsplt/spacelnc)

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("VIDEO")
	PORT_CONFNAME( 0x01, 0x00, "Video Hardware" )
	PORT_CONFSETTING(    0x01, "Monochrome" )
	PORT_CONFSETTING(    0x00, "Color" )
INPUT_PORTS_END


INPUT_PORTS_START( highsplt )
	PORT_INCLUDE( spacefev )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x08, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
INPUT_PORTS_END


INPUT_PORTS_START( spacelnc )
	PORT_INCLUDE( highsplt )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "3000" )
	PORT_DIPSETTING(    0x08, "5000" )
	PORT_DIPSETTING(    0x0c, "8000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
INPUT_PORTS_END


INPUT_PORTS_START( sheriff )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP2
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP3 enables diagnostic ROM at $2400
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )    // Switches 3-7 are UNUSED
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ))   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
INPUT_PORTS_END


INPUT_PORTS_START( bandido )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP2
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP3 enables diagnostic ROM at $2400
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown )) // don't know if this is used
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
INPUT_PORTS_END


INPUT_PORTS_START( westgun2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) // enables diagnostic ROM at $2400
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( helifire )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP1
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP2
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) // EXP3
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x04, "6000" )
	PORT_DIPSETTING(    0x08, "8000" )
	PORT_DIPSETTING(    0x0c, "10000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ))

	/* potentiometers */
	PORT_START("POT0")
	PORT_DIPNAME( 0xff, 0x50, "VR1 sun brightness" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x30, "30" )
	PORT_DIPSETTING(    0x40, "40" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x60, "60" )
	PORT_DIPSETTING(    0x70, "70" )

	PORT_START("POT1")
	PORT_DIPNAME( 0xff, 0x00, "VR2 sea brightness" )
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x30, "30" )
	PORT_DIPSETTING(    0x40, "40" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x60, "60" )
	PORT_DIPSETTING(    0x70, "70" )
INPUT_PORTS_END


// Interrupts

TIMER_DEVICE_CALLBACK_MEMBER(n8080_state::rst1_tick)
{
	int state = m_inte ? ASSERT_LINE : CLEAR_LINE;

	// V7 = 1, V6 = 0
	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, state, 0xcf); // I8080
}

TIMER_DEVICE_CALLBACK_MEMBER(n8080_state::rst2_tick)
{
	int state = m_inte ? ASSERT_LINE : CLEAR_LINE;

	// vblank
	m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, state, 0xd7); // I8080
}

void n8080_state::n8080_inte_callback(int state)
{
	m_inte = state != 0;
}

void n8080_state::n8080_status_callback(u8 data)
{
	if (data & i8080a_cpu_device::STATUS_INTA)
	{
		// interrupt acknowledge
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}


// Machine start/reset

void n8080_state::machine_start()
{
	save_item(NAME(m_shift_data));
	save_item(NAME(m_shift_bits));
	save_item(NAME(m_inte));
}

void n8080_state::machine_reset()
{
	m_shift_data = 0;
	m_shift_bits = 0;
	m_inte = false;

	delayed_sound_1(0);
	delayed_sound_2(0);
}

void spacefev_state::machine_start()
{
	n8080_state::machine_start();

	m_cannon_timer = timer_alloc(FUNC(spacefev_state::stop_red_cannon), this);

	flip_screen_set(0);

	save_item(NAME(m_prev_snd_data));
	save_item(NAME(m_red_screen));
	save_item(NAME(m_red_cannon));
}

void spacefev_state::machine_reset()
{
	m_prev_snd_data = 0;

	n8080_state::machine_reset();

	m_red_screen = false;
	m_red_cannon = false;
}

void sheriff_state::machine_start()
{
	n8080_state::machine_start();

	flip_screen_set(0);

	save_item(NAME(m_sheriff_color_mode));
	save_item(NAME(m_sheriff_color_data));
}

void sheriff_state::machine_reset()
{
	n8080_state::machine_reset();

	m_sheriff_color_mode = 0;
	m_sheriff_color_data = 0;
}

void helifire_state::machine_start()
{
	n8080_state::machine_start();

	u8 data = 0;
	for (int i = 0; i < 63; i++)
	{
		u8 const bit = (data >> 6) ^ (data >> 7) ^ 1;
		data = (data << 1) | (bit & 1);
		m_LSFR[i] = data;
	}

	flip_screen_set(0);

	save_item(NAME(m_mv));
	save_item(NAME(m_sc));
	save_item(NAME(m_flash));
	save_item(NAME(m_LSFR));
}

void helifire_state::machine_reset()
{
	n8080_state::machine_reset();

	m_mv = 0;
	m_sc = 0;
	m_flash = false;
}


// Machine configs

void spacefev_state::spacefev(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 20.160_MHz_XTAL / 10);
	m_maincpu->out_status_func().set(FUNC(spacefev_state::n8080_status_callback));
	m_maincpu->out_inte_func().set(FUNC(spacefev_state::n8080_inte_callback));
	m_maincpu->set_addrmap(AS_PROGRAM, &spacefev_state::main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &spacefev_state::main_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(20.160_MHz_XTAL / 4, 320, 0, 256, 256, 16, 240);
	m_screen->set_screen_update(FUNC(spacefev_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(spacefev_state::n8080_palette), 8);

	TIMER(config, "rst1").configure_scanline(FUNC(spacefev_state::rst1_tick), "screen", 128, 256);
	TIMER(config, "rst2").configure_scanline(FUNC(spacefev_state::rst2_tick), "screen", 240, 256);

	/* sound hardware */
	SPACEFEV_SOUND(config, m_sound, u32(0));
}

void sheriff_state::sheriff(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 20.160_MHz_XTAL / 10);
	m_maincpu->out_status_func().set(FUNC(sheriff_state::n8080_status_callback));
	m_maincpu->out_inte_func().set(FUNC(sheriff_state::n8080_inte_callback));
	m_maincpu->set_addrmap(AS_PROGRAM, &sheriff_state::main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &sheriff_state::main_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(20.160_MHz_XTAL / 4, 320, 0, 256, 256, 16, 240);
	m_screen->set_screen_update(FUNC(sheriff_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(sheriff_state::n8080_palette), 8);

	TIMER(config, "rst1").configure_scanline(FUNC(sheriff_state::rst1_tick), "screen", 128, 256);
	TIMER(config, "rst2").configure_scanline(FUNC(sheriff_state::rst2_tick), "screen", 240, 256);

	/* sound hardware */
	SHERIFF_SOUND(config, m_sound, u32(0));
}

void sheriff_state::westgun2(machine_config &config)
{
	sheriff(config);

	/* basic machine hardware */
	m_maincpu->set_clock(19.968_MHz_XTAL / 10);
	m_screen->set_raw(19.968_MHz_XTAL / 4, 320, 0, 256, 262, 16, 240); // presumably more like other 8080bw.cpp games
}

void helifire_state::helifire(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 20.160_MHz_XTAL / 10);
	m_maincpu->out_status_func().set(FUNC(helifire_state::n8080_status_callback));
	m_maincpu->out_inte_func().set(FUNC(helifire_state::n8080_inte_callback));
	m_maincpu->set_addrmap(AS_PROGRAM, &helifire_state::main_cpu_map);
	m_maincpu->set_addrmap(AS_IO, &helifire_state::main_io_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(20.160_MHz_XTAL / 4, 320, 0, 256, 256, 16, 240);
	m_screen->set_screen_update(FUNC(helifire_state::screen_update));
	m_screen->screen_vblank().set(FUNC(helifire_state::screen_vblank));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(helifire_state::helifire_palette), 8 + 0x400);

	TIMER(config, "rst1").configure_scanline(FUNC(helifire_state::rst1_tick), "screen", 128, 256);
	TIMER(config, "rst2").configure_scanline(FUNC(helifire_state::rst2_tick), "screen", 240, 256);

	/* sound hardware */
	HELIFIRE_SOUND(config, m_sound, u32(0));
}


// ROM definitions

ROM_START( spacefev )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1-ro-.bin",  0x0000, 0x0400, CRC(35f295bd) SHA1(34d1df25fcdea598ca1191cecc2125e6f63dbce3) ) // "F1??"
	ROM_LOAD( "f2-ro-.bin",  0x0400, 0x0400, CRC(0c633f4c) SHA1(a551ddbf21670fb1f000404b92da87a97f7ba157) ) // "F2??"
	ROM_LOAD( "g1-ro-.bin",  0x0800, 0x0400, CRC(f3d851cb) SHA1(535c52a56e54a064aa3d1c48a129f714234a1007) ) // "G1??"
	ROM_LOAD( "g2-ro-.bin",  0x0c00, 0x0400, CRC(1faef63a) SHA1(68e1bfc45587bfb1ee2eb477b60efd4f69dffd2c) ) // "G2??"
	ROM_LOAD( "h1-ro-.bin",  0x1000, 0x0400, CRC(b365389d) SHA1(e681f2c5e37cc07912915ef74184ff9336309de3) ) // "H1??"
	ROM_LOAD( "h2-ro-.bin",  0x1400, 0x0400, CRC(a163e800) SHA1(e8817f3e17f099a0dc66213d2d3d3fdeb117b10e) ) // "H2??"
	ROM_LOAD( "i1-ro-p.bin", 0x1800, 0x0400, CRC(756b5582) SHA1(b7f3d218b7f4267ce6128624306396bcacb9b44e) ) // "I1??P"

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "ss3.ic2",     0x0000, 0x0400, CRC(95c2c1ee) SHA1(42a3a382fc7d2782052372d71f6d0e8a153e74d0) )

	ROM_REGION( 0x0020, "proms", 0 ) // for color video hw
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( spacefevo )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1-ro-.bin",  0x0000, 0x0400, CRC(35f295bd) SHA1(34d1df25fcdea598ca1191cecc2125e6f63dbce3) ) // "F1??"
	ROM_LOAD( "f2-ro-.bin",  0x0400, 0x0400, CRC(0c633f4c) SHA1(a551ddbf21670fb1f000404b92da87a97f7ba157) ) // "F2??"
	ROM_LOAD( "g1-ro-.bin",  0x0800, 0x0400, CRC(f3d851cb) SHA1(535c52a56e54a064aa3d1c48a129f714234a1007) ) // "G1??"
	ROM_LOAD( "g2-ro-.bin",  0x0c00, 0x0400, CRC(1faef63a) SHA1(68e1bfc45587bfb1ee2eb477b60efd4f69dffd2c) ) // "G2??"
	ROM_LOAD( "h1-ro-.bin",  0x1000, 0x0400, CRC(b365389d) SHA1(e681f2c5e37cc07912915ef74184ff9336309de3) ) // "H1??"
	ROM_LOAD( "h2-ro-.bin",  0x1400, 0x0400, CRC(a163e800) SHA1(e8817f3e17f099a0dc66213d2d3d3fdeb117b10e) ) // "H2??"
	ROM_LOAD( "i1-ro-.bin",  0x1800, 0x0400, CRC(00027be2) SHA1(551a779a2e5a6455b7a348d246731c094e0ec709) ) // "I1??"

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "ss3.ic2",     0x0000, 0x0400, CRC(95c2c1ee) SHA1(42a3a382fc7d2782052372d71f6d0e8a153e74d0) )

	ROM_REGION( 0x0020, "proms", 0 ) // for color video hw
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( spacefevo2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1-i-.bin",   0x0000, 0x0400, CRC(7fa305e8) SHA1(cda9fc9c76f57800de25ddf65f69fef19fd28481) ) // "F1?C"
	ROM_LOAD( "f2-i-.bin",   0x0400, 0x0400, CRC(7c1429aa) SHA1(8d8e0a4fc09fb1ecbfb86c67c20000ef30ab3fac) ) // "F2?C"
	ROM_LOAD( "g1-i-.bin",   0x0800, 0x0400, CRC(75f6efc1) SHA1(286bc75e35e8ad6277e9db7377e90731b9c2ec97) ) // "G1?C"
	ROM_LOAD( "g2-i-.bin",   0x0c00, 0x0400, CRC(fb6bcf4a) SHA1(3edea04d67c2f3b1a6a73adadea83ddda0be3842) ) // "G2?C"
	ROM_LOAD( "h1-i-.bin",   0x1000, 0x0400, CRC(3beef037) SHA1(4bcc157e7d721b3a9e16e7a2efa807303d4be8ac) ) // "H1?C"
	ROM_LOAD( "h2-i-.bin",   0x1400, 0x0400, CRC(bddbc94f) SHA1(f90cbc3cd0f695cbb9ae03b608f4bf5a4a000c64) ) // "H2?C"
	ROM_LOAD( "i1-i-.bin",   0x1800, 0x0400, CRC(437786c5) SHA1(2ccdb0d48dbbfe47ae82e970ca37970602405cf6) ) // "I1?C"

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "ss3.ic2",     0x0000, 0x0400, CRC(95c2c1ee) SHA1(42a3a382fc7d2782052372d71f6d0e8a153e74d0) )

	ROM_REGION( 0x0020, "proms", 0 ) // for color video hw
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( highsplt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f1-ha-.bin",  0x0000, 0x0400, CRC(b8887351) SHA1(ccd49937f1cbd7a157b3715474ccc3e8fdcea2b2) ) // "F1?n"
	ROM_LOAD( "f2-ha-.bin",  0x0400, 0x0400, CRC(cda933a7) SHA1(a0447c8c98e24674081c9bf4b1ef07dc186c6e2b) ) // "F2?n"
	ROM_LOAD( "g1-ha-.bin",  0x0800, 0x0400, CRC(de17578a) SHA1(d9d5dbf38331f212d2a566c60756a788e169104d) ) // "G1?n"
	ROM_LOAD( "g2-ha-.bin",  0x0c00, 0x0400, CRC(f1a90948) SHA1(850f27b42ca12bcba4aa95a1ad3e66206fa63554) ) // "G2?n"
	ROM_LOAD( "hs.h1",       0x1000, 0x0400, CRC(eefb4273) SHA1(853a62976a406516f10ac68dc2859399b8b7aae8) )
	ROM_LOAD( "h2-ha-.bin",  0x1400, 0x0400, CRC(e91703e8) SHA1(f58606b0c7d945e94c3fccc7ebe17ca25675e6a0) ) // "H2?n"
	ROM_LOAD( "hs.i1",       0x1800, 0x0400, CRC(41e18df9) SHA1(2212c836313775e7c507a875672c0b3635825e02) )
	ROM_LOAD( "i2-ha-.bin",  0x1c00, 0x0400, CRC(eff9f82d) SHA1(5004e52dfa652ceefca9ed4210c0fa8f0591dc08) ) // "I2?n"

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "ss4.bin",     0x0000, 0x0400, CRC(939e01d4) SHA1(7c9ccd24e5da03831cd0aa821da17e3b81cd8381) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( highsplta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "f1-ha-.bin",  0x0000, 0x0400, CRC(b8887351) SHA1(ccd49937f1cbd7a157b3715474ccc3e8fdcea2b2) ) // "F1?n"
	ROM_LOAD( "f2-ha-.bin",  0x0400, 0x0400, CRC(cda933a7) SHA1(a0447c8c98e24674081c9bf4b1ef07dc186c6e2b) ) // "F2?n"
	ROM_LOAD( "g1-ha-.bin",  0x0800, 0x0400, CRC(de17578a) SHA1(d9d5dbf38331f212d2a566c60756a788e169104d) ) // "G1?n"
	ROM_LOAD( "g2-ha-.bin",  0x0c00, 0x0400, CRC(f1a90948) SHA1(850f27b42ca12bcba4aa95a1ad3e66206fa63554) ) // "G2?n"
	ROM_LOAD( "h1-ha-.bin",  0x1000, 0x0400, CRC(b0505da3) SHA1(f7b1f3a6dd06ff0cdeb6b13c948b7a262592514a) ) // "H1?n"
	ROM_LOAD( "h2-ha-.bin",  0x1400, 0x0400, CRC(e91703e8) SHA1(f58606b0c7d945e94c3fccc7ebe17ca25675e6a0) ) // "H2?n"
	ROM_LOAD( "i1-ha-.bin",  0x1800, 0x0400, CRC(aa36b25d) SHA1(28f555aab27b206a8c6f550b6caa938cece6e204) ) // "I1?n"
	ROM_LOAD( "i2-ha-.bin",  0x1c00, 0x0400, CRC(eff9f82d) SHA1(5004e52dfa652ceefca9ed4210c0fa8f0591dc08) ) // "I2?n"

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "ss4.bin",     0x0000, 0x0400, CRC(939e01d4) SHA1(7c9ccd24e5da03831cd0aa821da17e3b81cd8381) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( highspltb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "f1-ha-.bin",  0x0000, 0x0400, CRC(b8887351) SHA1(ccd49937f1cbd7a157b3715474ccc3e8fdcea2b2) ) // "F1?n"
	ROM_LOAD( "f2-ha-.bin",  0x0400, 0x0400, CRC(cda933a7) SHA1(a0447c8c98e24674081c9bf4b1ef07dc186c6e2b) ) // "F2?n"
	ROM_LOAD( "g1-ha-.bin",  0x0800, 0x0400, CRC(de17578a) SHA1(d9d5dbf38331f212d2a566c60756a788e169104d) ) // "G1?n"
	ROM_LOAD( "g2-ha-.bin",  0x0c00, 0x0400, CRC(f1a90948) SHA1(850f27b42ca12bcba4aa95a1ad3e66206fa63554) ) // "G2?n"
	ROM_LOAD( "h1-ha-.bin",  0x1000, 0x0400, CRC(b0505da3) SHA1(f7b1f3a6dd06ff0cdeb6b13c948b7a262592514a) ) // "H1?n"
	ROM_LOAD( "h2-ha-.bin",  0x1400, 0x0400, CRC(e91703e8) SHA1(f58606b0c7d945e94c3fccc7ebe17ca25675e6a0) ) // "H2?n"
	ROM_LOAD( "i1-ha-.bin",  0x1800, 0x0400, CRC(aa36b25d) SHA1(28f555aab27b206a8c6f550b6caa938cece6e204) ) // "I1?n"
	ROM_LOAD( "i2-ha-.bin",  0x1c00, 0x0400, CRC(eff9f82d) SHA1(5004e52dfa652ceefca9ed4210c0fa8f0591dc08) ) // "I2?n"

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "ss4.ic2",     0x0000, 0x0400, CRC(ce95dc5f) SHA1(20f7b8c565c408439dcfae240b7d1aa42c29651b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "f5-i-.bin",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) ) // "F5?C"
ROM_END

ROM_START( spacelnc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sl.f1",    0x0000, 0x0400, CRC(6ad59e40) SHA1(d416f7e6f5f55178df5c390548cd299650853022) )
	ROM_LOAD( "sl.f2",    0x0400, 0x0400, CRC(2de568e2) SHA1(f13740d3d9bf7434b7760e9286ef6e2ede40845f) )
	ROM_LOAD( "sl.g1",    0x0800, 0x0400, CRC(06d0ab36) SHA1(bf063100b065dbf511d6f32da169fb461568d15d) )
	ROM_LOAD( "sl.g2",    0x0c00, 0x0400, CRC(73ac4fe6) SHA1(7fa8c09692446bdf804900158e040f0b875a2e32) )
	ROM_LOAD( "sl.h1",    0x1000, 0x0400, CRC(7f42a94b) SHA1(ad85706de5e3f952b12756275be1ea1276a10666) )
	ROM_LOAD( "sl.h2",    0x1400, 0x0400, CRC(04b7a5f9) SHA1(589b0a0c8dcb1300623fe8478f1d7173b2bc575f) )
	ROM_LOAD( "sl.i1",    0x1800, 0x0400, CRC(d30007a3) SHA1(9e5905df8f7822385daef159a07f0e8257cb862a) )
	ROM_LOAD( "sl.i2",    0x1c00, 0x0400, CRC(640ffd2f) SHA1(65c21396c39dc99ec263f66f400a8e4c7712b20a) )

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "sl.snd",   0x0000, 0x0400, CRC(8e1ff929) SHA1(5c7da97b05fb8fff242158978199f5d35b234426) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sf.prm",   0x0000, 0x0020, CRC(c5914ec1) SHA1(198875fcab36d09c8726bb21e2fdff9882f6721a) )
ROM_END

ROM_START( sheriff )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sh.f1",    0x0000, 0x0400, CRC(e79df6e8) SHA1(908176de9bfc3d48e2da9af6ba7ebdee698ec2de) )
	ROM_LOAD( "sh.f2",    0x0400, 0x0400, CRC(da67721a) SHA1(ee6a5fb98da1d1fcfad0ef27af300473a637f578) )
	ROM_LOAD( "sh.g1",    0x0800, 0x0400, CRC(3fb7888e) SHA1(2c2d6b27d577d5ccf759e451e53c2e3314af40f6) )
	ROM_LOAD( "sh.g2",    0x0c00, 0x0400, CRC(585fcfee) SHA1(82f2abc14f893c092b80da45fc297fa5fb0890b5) )
	ROM_LOAD( "sh.h1",    0x1000, 0x0400, CRC(e59eab52) SHA1(aa87710237dd48d1831f1b307d547b1b0707cd4e) )
	ROM_LOAD( "sh.h2",    0x1400, 0x0400, CRC(79e69a6a) SHA1(1780ce77d7d9ddbf4aceabe0fcf079339837bbe1) )
	ROM_LOAD( "sh.i1",    0x1800, 0x0400, CRC(dda7d1e8) SHA1(bd2a7388e81c71922b2e97d68be71359a75e8d37) )
	ROM_LOAD( "sh.i2",    0x1c00, 0x0400, CRC(5c5f3f86) SHA1(25c64ccb7d0e136f67d6e1da7927ae6d89e0ceb9) )
	ROM_LOAD( "sh.j1",    0x2000, 0x0400, CRC(0aa8b79a) SHA1(aed139e8c8ba912823c57fe4cc7231b2d638f479) )

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "sh.snd",   0x0000, 0x0400, CRC(75731745) SHA1(538a63c9c60f1886fca4caf3eb1e0bada2d3f162) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s137.3l", 0x0000, 0x0400, CRC(820f8cdd) SHA1(197eeb008c140558e7c1ab2b2bd0f6a27096877c) )
ROM_END

ROM_START( bandido )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sh-a.f1",  0x0000, 0x0400, CRC(aec94829) SHA1(aa6d241670ea061bac4a71dff82dfa832095eae6) )
	ROM_LOAD( "sh.f2",    0x0400, 0x0400, CRC(da67721a) SHA1(ee6a5fb98da1d1fcfad0ef27af300473a637f578) )
	ROM_LOAD( "sh.g1",    0x0800, 0x0400, CRC(3fb7888e) SHA1(2c2d6b27d577d5ccf759e451e53c2e3314af40f6) )
	ROM_LOAD( "sh.g2",    0x0c00, 0x0400, CRC(585fcfee) SHA1(82f2abc14f893c092b80da45fc297fa5fb0890b5) )
	ROM_LOAD( "sh-a.h1",  0x1000, 0x0400, CRC(5cb63677) SHA1(59a8e5f8b134bf44d3e5a1105a9346f0c5f9378e) )
	ROM_LOAD( "sh.h2",    0x1400, 0x0400, CRC(79e69a6a) SHA1(1780ce77d7d9ddbf4aceabe0fcf079339837bbe1) )
	ROM_LOAD( "sh.i1",    0x1800, 0x0400, CRC(dda7d1e8) SHA1(bd2a7388e81c71922b2e97d68be71359a75e8d37) )
	ROM_LOAD( "sh.i2",    0x1c00, 0x0400, CRC(5c5f3f86) SHA1(25c64ccb7d0e136f67d6e1da7927ae6d89e0ceb9) )
	ROM_LOAD( "sh.j1",    0x2000, 0x0400, CRC(0aa8b79a) SHA1(aed139e8c8ba912823c57fe4cc7231b2d638f479) )
	ROM_LOAD( "sh-a.j2",  0x2400, 0x0400, CRC(a10b848a) SHA1(c045f1f6a11cbf49a1bae06c701b659d587292a3) )

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "sh.snd",   0x0000, 0x0400, CRC(75731745) SHA1(538a63c9c60f1886fca4caf3eb1e0bada2d3f162) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s137.3l", 0x0000, 0x0400, CRC(820f8cdd) SHA1(197eeb008c140558e7c1ab2b2bd0f6a27096877c) )
ROM_END

ROM_START( westgun2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "rf01.ic36",    0x0000, 0x0800, CRC(7eabc538) SHA1(8f0540a5cb391b83aafc0365b44686628eae5c77) )
	ROM_LOAD( "rf02.ic35",    0x0800, 0x0800, CRC(3344d6a5) SHA1(ea2a8413401b53c9d1b9c653ac3a98855a35cce6) )
	ROM_LOAD( "rf03.ic34",    0x1000, 0x0800, CRC(d4bb08fd) SHA1(92c0821f259037b193658997289b6b41c6f67215) )
	ROM_LOAD( "rf04.ic33",    0x1800, 0x0800, CRC(60b71f0d) SHA1(10650426972afb0ccb964548a52879ed3f0b316a) )
	ROM_LOAD( "rf05.ic32",    0x2000, 0x0800, CRC(81e650fb) SHA1(e600567125294d1411fcad3a015edb98cee36ff8) )

	ROM_REGION( 0x0800, "soundboard:cpu", 0 )
	ROM_LOAD( "rf06.ic35",   0x0000, 0x0800, CRC(4eafe957) SHA1(78e03402219c0ad814f63ae507eadc636d95f755) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s137.3l", 0x0000, 0x0400, CRC(820f8cdd) SHA1(197eeb008c140558e7c1ab2b2bd0f6a27096877c) ) // rf07 not dumped, taken from parent
ROM_END

ROM_START( helifire )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tub_f1_b",  0x0000, 0x0400, CRC(032f89ca) SHA1(63b0310875ed78a6385e44eea781ddcc4a63557c) )
	ROM_LOAD( "tub_f2_b",  0x0400, 0x0400, CRC(2774e70f) SHA1(98d845e80db61799493dbebe8db801567277432c) )
	ROM_LOAD( "tub_g1_b",  0x0800, 0x0400, CRC(b5ad6e8a) SHA1(1eb4931e85bd6a559e85a2b978d383216d3988a7) )
	ROM_LOAD( "tub_g2_b",  0x0c00, 0x0400, CRC(5e015bf4) SHA1(60f5a9707c8655e54a8381afd764856fb25c29f1) )
	ROM_LOAD( "tub_h1_b",  0x1000, 0x0400, CRC(23bb4e5a) SHA1(b59bc0adff3635aca1def2b1997f7edc6ca7e8ee) )
	ROM_LOAD( "tub_h2_b",  0x1400, 0x0400, CRC(358227c6) SHA1(d7bd678ef1737edc6aa609e43e3ae96a8d61dc15) )
	ROM_LOAD( "tub_i1_b",  0x1800, 0x0400, CRC(0c679f44) SHA1(cbe31dbe5f2c5f11a637cb3bde4e059c310d0e76) )
	ROM_LOAD( "tub_i2_b",  0x1c00, 0x0400, CRC(d8b7a398) SHA1(3ddfeac39147d5df6096f525f7ef67abef32a28b) )
	ROM_LOAD( "tub_j1_b",  0x2000, 0x0400, CRC(98ef24db) SHA1(70ad8dd6e1e8f4bf4ce431737ca1856eecc03d53) )
	ROM_LOAD( "tub_j2_b",  0x2400, 0x0400, CRC(5e2b5877) SHA1(f7c747e8a1d9fe2dda71ee6304636cf3cdf727a7) )

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "tub-e_ic5-a", 0x0000, 0x0400, CRC(9d77a31f) SHA1(36db9b5087b6661de88042854874bc247c92d985) )
ROM_END

ROM_START( helifirea )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "hf-a.f1",  0x0000, 0x0400, CRC(92c9d6c1) SHA1(860a7b3980e9e11d48769fad347c965e04ed3f89) )
	ROM_LOAD( "hf-a.f2",  0x0400, 0x0400, CRC(a264dde8) SHA1(48f972ad5af6c2ab61117f60d9244df6df6d313c) )
	ROM_LOAD( "hf.g1",    0x0800, 0x0400, CRC(b5ad6e8a) SHA1(1eb4931e85bd6a559e85a2b978d383216d3988a7) )
	ROM_LOAD( "hf-a.g2",  0x0c00, 0x0400, CRC(a987ebcd) SHA1(46726293c308c18b28941809419ba4c2ffc8084f) )
	ROM_LOAD( "hf-a.h1",  0x1000, 0x0400, CRC(25abcaf0) SHA1(a14c795de1fc283405f71bb83f4ac5c98fd406cb) )
	ROM_LOAD( "hf.h2",    0x1400, 0x0400, CRC(358227c6) SHA1(d7bd678ef1737edc6aa609e43e3ae96a8d61dc15) )
	ROM_LOAD( "hf.i1",    0x1800, 0x0400, CRC(0c679f44) SHA1(cbe31dbe5f2c5f11a637cb3bde4e059c310d0e76) )
	ROM_LOAD( "hf-a.i2",  0x1c00, 0x0400, CRC(296610fd) SHA1(f1ab379983e45f3cd718dd82962c609297b4dcb8) )
	ROM_LOAD( "hf.j1",    0x2000, 0x0400, CRC(98ef24db) SHA1(70ad8dd6e1e8f4bf4ce431737ca1856eecc03d53) )
	ROM_LOAD( "hf.j2",    0x2400, 0x0400, CRC(5e2b5877) SHA1(f7c747e8a1d9fe2dda71ee6304636cf3cdf727a7) )

	ROM_REGION( 0x0400, "soundboard:cpu", 0 )
	ROM_LOAD( "hf.snd",   0x0000, 0x0400, CRC(9d77a31f) SHA1(36db9b5087b6661de88042854874bc247c92d985) )
ROM_END

} // anonymous namespace


//    YEAR, NAME,       PARENT,   MACHINE,  INPUT,    CLASS,          INIT,       MONITOR, COMPANY, FULLNAME, FLAGS
GAME( 1979, spacefev,   0,        spacefev, spacefev, spacefev_state, empty_init, ROT270, "Nintendo", "Space Fever (new version)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, spacefevo,  spacefev, spacefev, spacefev, spacefev_state, empty_init, ROT270, "Nintendo", "Space Fever (old version)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, spacefevo2, spacefev, spacefev, spacefev, spacefev_state, empty_init, ROT270, "Nintendo", "Space Fever (older version)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, highsplt,   0,        spacefev, highsplt, spacefev_state, empty_init, ROT270, "Nintendo", "SF-HiSplitter (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, highsplta,  highsplt, spacefev, highsplt, spacefev_state, empty_init, ROT270, "Nintendo", "SF-HiSplitter (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, highspltb,  highsplt, spacefev, highsplt, spacefev_state, empty_init, ROT270, "Nintendo", "SF-HiSplitter (alt sound)", MACHINE_SUPPORTS_SAVE )
GAME( 1979, spacelnc,   0,        spacefev, spacelnc, spacefev_state, empty_init, ROT270, "Nintendo", "Space Launcher", MACHINE_SUPPORTS_SAVE )

GAME( 1979, sheriff,    0,        sheriff,  sheriff,  sheriff_state,  empty_init, ROT270, "Nintendo", "Sheriff", MACHINE_SUPPORTS_SAVE )
GAME( 1980, bandido,    sheriff,  sheriff,  bandido,  sheriff_state,  empty_init, ROT270, "Nintendo (Exidy license)", "Bandido", MACHINE_SUPPORTS_SAVE )
GAME( 1980, westgun2,   sheriff,  westgun2, westgun2, sheriff_state,  empty_init, ROT270, "Nintendo (Taito Corporation license)", "Western Gun Part II", MACHINE_SUPPORTS_SAVE ) // official Taito PCBs, but title/copyright not shown

GAME( 1980, helifire,   0,        helifire, helifire, helifire_state, empty_init, ROT270, "Nintendo", "HeliFire (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1980, helifirea,  helifire, helifire, helifire, helifire_state, empty_init, ROT270, "Nintendo", "HeliFire (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
