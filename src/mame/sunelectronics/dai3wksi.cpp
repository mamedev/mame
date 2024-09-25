// license:BSD-3-Clause
// copyright-holders:Hau, Derrick Renaud
/*

--------------------------
Galaxy Force
PCB: SIV-01-A
Runaway
PCB: SIV-01-B
--------------------------
Dai 3 Wakusei
(c)1979 Sun Electronics

PCB: SIV-01-B
TVG_13.6     [8e8b40b1]
TVG_14.7     [d48cbabe]
TVG_15.8     [cf44bd60]
TVG_16.9     [ae723f56]
--------------------------
Warp 1
(c)1979 Sun Electronics

PCB: SIV-01-C
TVG_23.6     [c956b3a8]
TVG_24.7     [6cca323d]
TVG_25.8     [5aa6eb1e]
TVG_26.9     [840c1a75]
--------------------------
Warp 1
PCB: SIV-01-C, Taito logo

Z80 @ 2.5MHz
1KB SRAM (2*MB8114)
16KB(nibbles) DRAM (4*UPD416C)
5*SN76477, dai3wksi has 6
--------------------------

TODO:
- Get rid of those big lookup tables, I see no proms on the PCB so it's hardcoded
  logic somehow? it's 3bpp colors, not b&w + color overlay.
  Note: warp1 colors match the ones of dai3wksi according to flyer and game photos.
  DIP switch for changing cyan to white does not make sense.
  PCB video of warp1 does not red-blink the 'fuel low' message.
- runaways colors are wrong (video is available).
- Two player games are automatically displayed in cocktail mode.
  Is this by design (a cocktail only romset)?
- Discrete audio needs adding to replace hardcoded samples
- Is warp1 sound same as dai3wksi?
- runaways uses less SN76477, it has different sound and should use different samples
  (until it's done via netlist).
- Dips need identifying
- warp1 service mode is started by booting with coin1 held down,
  the service switch can still be tested there but otherwise has no function?
  In warp1bl, it is a service coin input.

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/rescap.h"
#include "sound/samples.h"
#include "sound/sn76477.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "warp1bl.lh"


namespace {

#define USE_SAMPLES     (1)


class dai3wksi_state : public driver_device
{
public:
	dai3wksi_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_ic77(*this, "ic77"),
		m_ic78(*this, "ic78"),
		m_ic79(*this, "ic79"),
		m_ic80(*this, "ic80"),
		m_ic81(*this, "ic81"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_inputs(*this, "IN%u", 0)
	{ }

	void dai3wksi(machine_config &config);
	void warp1bl(machine_config &config);

	int warp1_protection_r() { return m_audio_data[0] & 1; }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<sn76477_device> m_ic77;
	optional_device<sn76477_device> m_ic78;
	optional_device<sn76477_device> m_ic79;
	optional_device<sn76477_device> m_ic80;
	optional_device<sn76477_device> m_ic81;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_videoram;
	required_ioport_array<3> m_inputs;

	void main_map(address_map &map) ATTR_COLD;

	// video
	u8 m_flipscreen = 0;
	u8 m_redscreen = 0;
	u8 m_redterop = 0;
	u32 screen_update_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// sound
	u8 m_audio_data[3] = { 0, 0, 0 };
	u8 m_enabled_sound = 0;
	u8 m_sound3_counter = 0;
	void audio_1_w(u8 data);
	void audio_2_w(u8 data);
	void audio_3_w(u8 data);
};

void dai3wksi_state::machine_start()
{
	// Set up save state
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_redscreen));
	save_item(NAME(m_redterop));
	save_item(NAME(m_audio_data));
	save_item(NAME(m_enabled_sound));
	save_item(NAME(m_sound3_counter));
}


/*************************************
 *
 *  Video system
 *
 *************************************/

static const u8 vr_prom1[64*8*2]={
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,

	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 5,5,5,5,5,5,5,5, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
};

static const u8 vr_prom2[64*8*2]={
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,6,6,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,

	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	6, 6,6,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
	3, 3,3,2,2,6,6,6,6, 6,6,6,6,6,6,6,6, 3,3,3,3,3,3,3,3, 7,7,7,7,7,7,7,7, 3,3,3,3,3,3,3,3, 2,2,2,2,2,2,2,2, 6,6,6,6,6,6,6,6, 4,4,4,4,4,4,4,
};

u32 dai3wksi_state::screen_update_color(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		u8 x = offs << 2;
		u8 y = offs >> 6;
		u8 data = m_videoram[offs];
		u8 color;
		int value = (x >> 2) + ((y >> 5) << 6) + 64 * 8 * (m_redterop ? 1 : 0);

		if (m_redscreen)
		{
			color = 0x02;
		}
		else
		{
			if (m_inputs[2]->read() & 0x03)
				color = vr_prom2[value];
			else
				color = vr_prom1[value];
		}

		for (int i = 0; i < 4; i++)
		{
			rgb_t pen = (data & (1 << i)) ? m_palette->pen_color(color) : rgb_t::black();

			if (m_flipscreen)
				bitmap.pix(255-y, 255-x) = pen;
			else
				bitmap.pix(y, x) = pen;

			x++;
		}
	}

	return 0;
}

u32 dai3wksi_state::screen_update_bw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		u8 x = offs << 2;
		u8 y = offs >> 6;
		u8 data = m_videoram[offs];

		for (int i = 0; i < 4; i++)
		{
			rgb_t pen = (data & (1 << i)) ? rgb_t::white() : rgb_t::black();

			if (m_flipscreen)
				bitmap.pix(255-y, 255-x) = pen;
			else
				bitmap.pix(y, x) = pen;

			x++;
		}
	}

	return 0;
}

/*************************************
 *
 *  Audio system
 *
 *************************************/

#if (USE_SAMPLES)

#define SAMPLE_SOUND1       0
#define SAMPLE_SOUND2       1
#define SAMPLE_SOUND3_1     2
#define SAMPLE_SOUND3_2     3
#define SAMPLE_SOUND4       4
#define SAMPLE_SOUND5       5
#define SAMPLE_SOUND6_1     6
#define SAMPLE_SOUND6_2     7

#define CHANNEL_SOUND1      0
#define CHANNEL_SOUND2      1
#define CHANNEL_SOUND3      2
#define CHANNEL_SOUND4      3
#define CHANNEL_SOUND5      4
#define CHANNEL_SOUND6      5

void dai3wksi_state::audio_1_w(u8 data)
{
	u8 rising_bits = data & ~m_audio_data[0];

	m_enabled_sound = data & 0x80;

	if ((rising_bits & 0x20) && m_enabled_sound)
	{
		if (data & 0x04)
			m_samples->start(CHANNEL_SOUND5, SAMPLE_SOUND5);
		else
			m_samples->start(CHANNEL_SOUND5, SAMPLE_SOUND5, true);
	}
	if (!(data & 0x20) && (m_audio_data[0] & 0x20))
		m_samples->stop(CHANNEL_SOUND5);

	m_audio_data[0] = data;
}

void dai3wksi_state::audio_2_w(u8 data)
{
	u8 rising_bits = data & ~m_audio_data[1];

	m_flipscreen = data & 0x10;
	m_redscreen  = ~data & 0x20;
	m_redterop   = data & 0x40;

	if (m_enabled_sound)
	{
		if (rising_bits & 0x01) m_samples->start(CHANNEL_SOUND1, SAMPLE_SOUND1);
		if (rising_bits & 0x02) m_samples->start(CHANNEL_SOUND2, SAMPLE_SOUND2);
		if (rising_bits & 0x08) m_samples->start(CHANNEL_SOUND4, SAMPLE_SOUND4);
		if (rising_bits & 0x04)
		{
			if (!m_sound3_counter)
				m_samples->start(CHANNEL_SOUND3, SAMPLE_SOUND3_1);
			else
				m_samples->start(CHANNEL_SOUND3, SAMPLE_SOUND3_2);

			m_sound3_counter ^= 1;
		}
	}

	m_audio_data[1] = data;
}

void dai3wksi_state::audio_3_w(u8 data)
{
	if (m_enabled_sound)
	{
		if (data & 0x40)
			m_samples->start(CHANNEL_SOUND6, SAMPLE_SOUND6_1);
		else if (data & 0x80)
			m_samples->start(CHANNEL_SOUND6, SAMPLE_SOUND6_2);
	}

	m_audio_data[2] = data;
}


static const char *const dai3wksi_sample_names[] =
{
	"*dai3wksi",
	"1",
	"2",
	"3",
	"3-2",
	"4",
	"5",
	"6",
	"6-2",
	nullptr
};


#else

void dai3wksi_state::audio_1_w(u8 data)
{
	machine().sound().system_mute(!BIT(data, 7));

	m_ic79->enable_w((~data >> 5) & 0x01);        // invader movement enable
	m_ic79->envelope_1_w((~data >> 2) & 0x01);    // invader movement envelope control

	m_audio_data[0] = data;
}

void dai3wksi_state::audio_2_w(u8 data)
{
	m_flipscreen =  data & 0x10;
	m_redscreen  = ~data & 0x20;
	m_redterop   =  data & 0x40;

	m_ic77->enable_w((~data >> 0) & 0x01);    // ship movement
	m_ic78->enable_w((~data >> 1) & 0x01);    // danger text
	// ic76 - invader hit  (~data >> 2) & 0x01
	m_ic80->enable_w((~data >> 3) & 0x01);    // planet explosion

	m_audio_data[1] = data;
}

void dai3wksi_state::audio_3_w(u8 data)
{
	m_ic81->enable_w((~data >> 2) & 0x01);    // player shoot enable
	m_ic81->vco_w((~data >> 3) & 0x01);       // player shoot vco control

	m_audio_data[2] = data;
}

#endif


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void dai3wksi_state::main_map(address_map &map)
{
	map(0x0000, 0x1bff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x2400, 0x2400).mirror(0x1ff).portr("IN0");
	map(0x2800, 0x2800).mirror(0x1ff).portr("IN1");
	map(0x3000, 0x3000).w(FUNC(dai3wksi_state::audio_1_w));
	map(0x3400, 0x3400).w(FUNC(dai3wksi_state::audio_2_w));
	map(0x3800, 0x3800).w(FUNC(dai3wksi_state::audio_3_w));
	map(0x8000, 0xbfff).ram().share(m_videoram);
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( dai3wksi )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) // hold down at boot for service mode
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x10, 0x00, "DIPSW #7" )                      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DIPSW #8" )                      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x00, "DIPSW #1" )                      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DIPSW #2" )                      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( warp1 )
	PORT_INCLUDE( dai3wksi )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(dai3wksi_state, warp1_protection_r)
	PORT_DIPNAME( 0x20, 0x20, "High Score Table" )              PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("IN1") // active-low
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_MODIFY("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( warp1bl ) // the bootleg seems to expect active low
	PORT_INCLUDE( warp1 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( runaways ) // only dips 7 and 8 tested in service mode, but 1 8 dip bank on PCB
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) // hold down at boot for service mode
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x10, 0x00, "DIPSW #7" )                      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void dai3wksi_state::dai3wksi(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(10'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &dai3wksi_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(dai3wksi_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 8, 247);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(dai3wksi_state::screen_update_color));

	PALETTE(config, m_palette, palette_device::BRG_3BIT);

	SPEAKER(config, "mono").front_center();

#if (USE_SAMPLES)
	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(dai3wksi_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
#else
	// Invader Hit
	sn76477_device &ic76(SN76477(config, "ic76"));
	ic76.set_noise_params(0, 0, 0);
	ic76.set_decay_res(RES_K(4.7));
	ic76.set_attack_params(CAP_U(0.1), RES_K(4.7));
	ic76.set_amp_res(RES_K(150));
	ic76.set_feedback_res(RES_K(47));
	ic76.set_vco_params(0, CAP_U(0.022), RES_K(33));
	ic76.set_pitch_voltage(5.0);
	ic76.set_slf_params(0, 0);
	ic76.set_oneshot_params(0, 0);
	ic76.set_vco_mode(0);
	ic76.set_mixer_params(0, 0, 0);
	ic76.set_envelope_params(0, 0);
	ic76.set_enable(0);
	ic76.add_route(ALL_OUTPUTS, "mono", 0.4);

	// Ship Movement
	SN76477(config, m_ic77);
	m_ic77->set_noise_params(0, 0, 0);
	m_ic77->set_decay_res(RES_K(4.7));
	m_ic77->set_attack_params(CAP_U(0.1), RES_K(4.7));
	m_ic77->set_amp_res(RES_K(150));
	m_ic77->set_feedback_res(RES_K(47));
	m_ic77->set_vco_params(0, 0, 0);
	m_ic77->set_pitch_voltage(0);
	m_ic77->set_slf_params(CAP_U(0.0022), RES_K(200));
	m_ic77->set_oneshot_params(CAP_U(10), RES_K(4.7));
	m_ic77->set_vco_mode(5);
	m_ic77->set_mixer_params(5, 0, 0);
	m_ic77->set_envelope_params(5, 0);
	m_ic77->set_enable(1);
	m_ic77->add_route(ALL_OUTPUTS, "mono", 0.4);

	// Danger
	SN76477(config, m_ic78);
	m_ic78->set_noise_params(RES_K(47), 0, 0);
	m_ic78->set_decay_res(RES_K(200));
	m_ic78->set_attack_params(CAP_U(0.1), RES_K(4.7));
	m_ic78->set_amp_res(RES_K(150));
	m_ic78->set_feedback_res(RES_K(47));
	m_ic78->set_vco_params(0, CAP_U(0.47), RES_K(75));
	m_ic78->set_pitch_voltage(5.0);
	m_ic78->set_slf_params(CAP_N(1), RES_K(47));
	m_ic78->set_oneshot_params(CAP_U(10), RES_K(22));
	m_ic78->set_vco_mode(5);
	m_ic78->set_mixer_params(0, 0, 0);
	m_ic78->set_envelope_params(5, 0);
	m_ic78->set_enable(1);
	m_ic78->add_route(ALL_OUTPUTS, "mono", 0.4);

	// Invader Marching Noise
	SN76477(config, m_ic79);
	m_ic79->set_noise_params(0, 0, 0);
	m_ic79->set_decay_res(RES_K(56));
	m_ic79->set_attack_params(CAP_U(0.1), RES_K(4.7));
	m_ic79->set_amp_res(RES_K(150));
	m_ic79->set_feedback_res(RES_K(47));
	m_ic79->set_vco_params(0, CAP_U(0.01), RES_K(100));
	m_ic79->set_pitch_voltage(5.0);
	m_ic79->set_slf_params(CAP_N(1), RES_K(150));
	m_ic79->set_oneshot_params(CAP_U(10), RES_K(22));
	m_ic79->set_vco_mode(5);
	m_ic79->set_mixer_params(0, 0, 0);
	m_ic79->set_envelope_params(5, 5);
	m_ic79->set_enable(1);
	m_ic79->add_route(ALL_OUTPUTS, "mono", 0.4);

	// Big Planet Explosion
	SN76477(config, m_ic80);
	m_ic80->set_noise_params(RES_K(47), RES_K(330), CAP_P(470));
	m_ic80->set_decay_res(RES_M(2));
	m_ic80->set_attack_params(CAP_U(1.0), RES_K(4.7));
	m_ic80->set_amp_res(RES_K(150));
	m_ic80->set_feedback_res(RES_K(47));
	m_ic80->set_vco_params(0, 0, 0);
	m_ic80->set_pitch_voltage(5.0);
	m_ic80->set_slf_params(0, 0);
	m_ic80->set_oneshot_params(CAP_U(10), RES_K(55));
	m_ic80->set_vco_mode(5);
	m_ic80->set_mixer_params(0, 5, 0);
	m_ic80->set_envelope_params(5, 0);
	m_ic80->set_enable(1);
	m_ic80->add_route(ALL_OUTPUTS, "mono", 0.4);

	// Plane Shoot noise
	SN76477(config, m_ic81);
	m_ic81->set_noise_params(0, 0, 0);
	m_ic81->set_decay_res(RES_K(200));
	m_ic81->set_attack_params(CAP_U(10), RES_K(4.7));
	m_ic81->set_amp_res(RES_K(150));
	m_ic81->set_feedback_res(RES_K(47));
	m_ic81->set_vco_params(2.5, CAP_U(0.01), RES_K(100));
	m_ic81->set_pitch_voltage(5.0);
	m_ic81->set_slf_params(CAP_N(0.47), RES_K(100));
	m_ic81->set_oneshot_params(CAP_U(10), RES_K(6.8));
	m_ic81->set_vco_mode(0);
	m_ic81->set_mixer_params(0, 5, 5);
	m_ic81->set_envelope_params(5, 0);
	m_ic81->set_enable(1);
	m_ic81->add_route(ALL_OUTPUTS, "mono", 0.4);
#endif
}

void dai3wksi_state::warp1bl(machine_config &config)
{
	dai3wksi(config);

	// the bootleg is in b&w
	subdevice<screen_device>("screen")->set_screen_update(FUNC(dai3wksi_state::screen_update_bw));
	PALETTE(config.replace(), m_palette, palette_device::MONOCHROME);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( dai3wksi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tvg_13.6",  0x0000, 0x0800, CRC(8e8b40b1) SHA1(25b9223486dd348ea302e8e8f1d47c804a88b142) )
	ROM_LOAD( "tvg_14.7",  0x0800, 0x0800, CRC(d48cbabe) SHA1(64b571cd778fc7d67a5fa998a0defd36c04f111f) )
	ROM_LOAD( "tvg_15.8",  0x1000, 0x0800, CRC(cf44bd60) SHA1(61e0b3f9c4a1f9da1de57fb8276d4fc9e43b8f24) )
	ROM_LOAD( "tvg_16.9",  0x1800, 0x0400, CRC(ae723f56) SHA1(c25c27d6144533b2b2a888bfa8dbf48ed8d8b09a) )
ROM_END

ROM_START( warp1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tvg_23.6",  0x0000, 0x0800, CRC(c956b3a8) SHA1(1e90d3480ac8ebb26eed5800a453b2b9484a8b05) )
	ROM_LOAD( "tvg_24.7",  0x0800, 0x0800, CRC(6cca323d) SHA1(e5dc4f09990c3f9e408e25e189a9355d7c021128) )
	ROM_LOAD( "tvg_25.8",  0x1000, 0x0800, CRC(5aa6eb1e) SHA1(ccf18d9618635c38747a57dad2214e6ca14c835d) )
	ROM_LOAD( "tvg_26.9",  0x1800, 0x0400, CRC(840c1a75) SHA1(63853e560c0fba5600a8ad0408cc9e7a55db6d93) )
ROM_END

ROM_START( warp1t )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tvg_23.6",  0x0000, 0x0800, CRC(2025a316) SHA1(704798ec1d0039cb977396330801a625c8b02cff) )
	ROM_LOAD( "tvg_24.7",  0x0800, 0x0800, CRC(6cca323d) SHA1(e5dc4f09990c3f9e408e25e189a9355d7c021128) )
	ROM_LOAD( "tvg_25.8",  0x1000, 0x0800, CRC(5aa6eb1e) SHA1(ccf18d9618635c38747a57dad2214e6ca14c835d) )
	ROM_LOAD( "tvg_26.9",  0x1800, 0x0400, CRC(6bf25327) SHA1(ee9f635d70a97628ab82efa12efdb39349b52e2b) )
ROM_END

ROM_START( warp1bl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0",  0x0000, 0x0400, CRC(d4383a9a) SHA1(eda0b388bd0adf22059f0848ce91fee889700f0c) )
	ROM_LOAD( "1",  0x0400, 0x0400, CRC(6005eddc) SHA1(a06dbdbe3356eb011dbce756e6c76adef4c50beb) )
	ROM_LOAD( "2",  0x0800, 0x0400, CRC(76ea7ebc) SHA1(60eb870f30aaf95589c7326ca6664974e39f3e27) )
	ROM_LOAD( "3",  0x0c00, 0x0400, CRC(0c34ef33) SHA1(513fe5a875746d94d2ea0b7eed81ca0cf869c457) )
	ROM_LOAD( "4",  0x1000, 0x0400, CRC(41ffad36) SHA1(479af4506f3637a95d2d41d04e1d66debb28da03) )
	ROM_LOAD( "5",  0x1400, 0x0400, CRC(f67a1e1d) SHA1(aeb7b018889de4bd32ee8f65d8c0c32bbee07440) )
	ROM_LOAD( "6",  0x1800, 0x0400, CRC(44e9327f) SHA1(aa217b0fbbef6a408231dc0dfedd94167c351c28) )
ROM_END

ROM_START( runaways ) // only 4 SN76477 are populated
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gf10",  0x0000, 0x0800, CRC(3d8ace0a) SHA1(e1f1393724e2f66d4ce9269dd982fdbfa96f8b6f) )
	ROM_LOAD( "gf11",  0x0800, 0x0800, CRC(a67b889f) SHA1(bfe1ebbcf2fb14b7557343229eaa0a42af70a3ad) )
	ROM_LOAD( "gf12",  0x1000, 0x0800, CRC(10350c4c) SHA1(32a505180d3212eb4c7abbe1459b3a3b6a55fe20) )
ROM_END

} // Anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, runaways, 0,     dai3wksi, runaways, dai3wksi_state, empty_init, ROT270, "Sun Electronics", "Runaway (Sun Electronics, Japan)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1979, dai3wksi, 0,     dai3wksi, dai3wksi, dai3wksi_state, empty_init, ROT270, "Sun Electronics", "Dai 3 Wakusei (Japan)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1979, warp1,    0,     dai3wksi, warp1,    dai3wksi_state, empty_init, ROT270, "Sun Electronics", "Warp-1 (Japan)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, warp1t,   warp1, dai3wksi, warp1,    dai3wksi_state, empty_init, ROT90,  "Sun Electronics (Taito license)", "Warp-1 (Japan, Taito license)", MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAMEL(1979, warp1bl,  warp1, warp1bl,  warp1bl,  dai3wksi_state, empty_init, ROT270, "bootleg (Igleck)", "Warp-1 (Japan, bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_warp1bl )
