// license:BSD-3-Clause
// copyright-holders: Stefan Jokisch

/***************************************************************************

Taito / Midway Super Speed Race driver

TODO:
- add Japan version(s), it has different graphics

***************************************************************************/

#include "emu.h"

#include "nl_sspeedr.h"

#include "cpu/z80/z80.h"
#include "machine/netlist.h"
#include "machine/watchdog.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "sspeedr.lh"


namespace {

class sspeedr_state : public driver_device
{
public:
	sspeedr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_pedal_bit(*this, "sound_nl:pedal_bit_%u", 0U)
		, m_hi_shift(*this, "sound_nl:hi_shift")
		, m_lo_shift(*this, "sound_nl:lo_shift")
		, m_boom(*this, "sound_nl:boom")
		, m_engine_sound_off(*this, "sound_nl:engine_sound_off")
		, m_noise_cr(*this, "sound_nl:noise_cr%u", 1U)
		, m_silence(*this, "sound_nl:silence")
		, m_track(*this, "track")
		, m_digits(*this, "digit%u", 0U)
		, m_lampgo(*this, "lampGO")
		, m_lampep(*this, "lampEP")
	{ }

	void sspeedr(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<netlist_mame_logic_input_device, 4> m_pedal_bit;
	required_device<netlist_mame_logic_input_device> m_hi_shift;
	required_device<netlist_mame_logic_input_device> m_lo_shift;
	required_device<netlist_mame_logic_input_device> m_boom;
	required_device<netlist_mame_logic_input_device> m_engine_sound_off;
	required_device_array<netlist_mame_logic_input_device, 2> m_noise_cr;
	required_device<netlist_mame_logic_input_device> m_silence;
	required_region_ptr<uint8_t> m_track;
	output_finder<26> m_digits;
	output_finder<> m_lampgo;
	output_finder<> m_lampep;


	uint8_t m_toggle = 0;
	uint16_t m_driver_horz = 0;
	uint8_t m_driver_vert = 0;
	uint8_t m_driver_pic = 0;
	uint16_t m_drones_horz = 0;
	uint8_t m_drones_vert[3]{};
	uint8_t m_drones_mask = 0;
	uint16_t m_track_horz = 0;
	uint8_t m_track_vert[2]{};
	uint8_t m_track_ice = 0;

	void int_ack_w(uint8_t data);
	void lamp_w(uint8_t data);
	void time_w(offs_t offset, uint8_t data);
	void score_w(offs_t offset, uint8_t data);
	void sound1_w(uint8_t data);
	void sound2_w(uint8_t data);
	void driver_horz_w(uint8_t data);
	void driver_horz_2_w(uint8_t data);
	void driver_vert_w(uint8_t data);
	void driver_pic_w(uint8_t data);
	void drones_horz_w(uint8_t data);
	void drones_horz_2_w(uint8_t data);
	void drones_mask_w(uint8_t data);
	void drones_vert_w(offs_t offset, uint8_t data);
	void track_horz_w(uint8_t data);
	void track_horz_2_w(uint8_t data);
	void track_vert_w(offs_t offset, uint8_t data);
	void track_ice_w(uint8_t data);
	void palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void draw_track(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_drones(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_driver(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
};


void sspeedr_state::driver_horz_w(uint8_t data)
{
	m_driver_horz = (m_driver_horz & 0x100) | data;
}


void sspeedr_state::driver_horz_2_w(uint8_t data)
{
	m_driver_horz = (m_driver_horz & 0xff) | ((data & 1) << 8);
}


void sspeedr_state::driver_vert_w(uint8_t data)
{
	m_driver_vert = data;
}


void sspeedr_state::driver_pic_w(uint8_t data)
{
	m_driver_pic = data & 0x1f;
}


void sspeedr_state::drones_horz_w(uint8_t data)
{
	m_drones_horz = (m_drones_horz & 0x100) | data;
}


void sspeedr_state::drones_horz_2_w(uint8_t data)
{
	m_drones_horz = (m_drones_horz & 0xff) | ((data & 1) << 8);
}


void sspeedr_state::drones_mask_w(uint8_t data)
{
	m_drones_mask = data & 0x3f;
}


void sspeedr_state::drones_vert_w(offs_t offset, uint8_t data)
{
	m_drones_vert[offset] = data;
}


void sspeedr_state::track_horz_w(uint8_t data)
{
	m_track_horz = (m_track_horz & 0x100) | data;
}


void sspeedr_state::track_horz_2_w(uint8_t data)
{
	m_track_horz = (m_track_horz & 0xff) | ((data & 1) << 8);
}


void sspeedr_state::track_vert_w(offs_t offset, uint8_t data)
{
	m_track_vert[offset] = data & 0x7f;
}


void sspeedr_state::track_ice_w(uint8_t data)
{
	m_track_ice = data & 0x07;
}


void sspeedr_state::draw_track(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
	{
		unsigned counter_x = x + m_track_horz + 0x50;

		int flag = 0;

		if (m_track_ice & 2)
		{
			flag = 1;
		}
		else if (m_track_ice & 4)
		{
			if (m_track_ice & 1)
			{
				flag = (counter_x <= 0x1ff);
			}
			else
			{
				flag = (counter_x >= 0x200);
			}
		}

		if (counter_x >= 0x200)
		{
			counter_x -= 0x1c8;
		}

		int y = cliprect.min_y;

		// upper landscape
		for (; y < m_track_vert[0] && y <= cliprect.max_y; y++)
		{
			unsigned const counter_y = y - m_track_vert[0];

			int const offset =
				((counter_y & 0x1f) << 3) |
				((counter_x & 0x1c) >> 2) |
				((counter_x & 0xe0) << 3);

			if (counter_x & 2)
			{
				bitmap.pix(y, x) = m_track[offset] >> 4;
			}
			else
			{
				bitmap.pix(y, x) = m_track[offset] & 0xf;
			}
		}

		// street
		for (; y < 128 + m_track_vert[1] && y <= cliprect.max_y; y++)
		{
			bitmap.pix(y, x) = flag ? 15 : 0;
		}

		// lower landscape
		for (; y <= cliprect.max_y; y++)
		{
			unsigned const counter_y = y - m_track_vert[1];

			int const offset =
				((counter_y & 0x1f) << 3) |
				((counter_x & 0x1c) >> 2) |
				((counter_x & 0xe0) << 3);

			if (counter_x & 2)
			{
				bitmap.pix(y, x) = m_track[offset] >> 4;
			}
			else
			{
				bitmap.pix(y, x) = m_track[offset] & 0xf;
			}
		}
	}
}


void sspeedr_state::draw_drones(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const uint8_t code[6] =
	{
		0xf, 0x4, 0x3, 0x9, 0x7, 0xc
	};

	for (int i = 0; i < 6; i++)
	{
		if ((m_drones_mask >> i) & 1)
		{
			continue;
		}

		int x = (code[i] << 5) - m_drones_horz - 0x50;

		if (x <= -32)
		{
			x += 0x1c8;
		}

		int const y = 0xf0 - m_drones_vert[i >> 1];

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code[i] ^ m_toggle,
				0,
				0, 0,
				x,
				y, 0);
	}
}


void sspeedr_state::draw_driver(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!(m_driver_pic & 0x10))
	{
		return;
	}

	int x = 0x1e0 - m_driver_horz - 0x50;

	if (x <= -32)
	{
		x += 0x1c8;
	}

	int const y = 0xf0 - m_driver_vert;

	m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
			m_driver_pic,
			0,
			0, 0,
			x,
			y, 0);
}


void sspeedr_state::video_start()
{
	m_toggle = 0;

	save_item(NAME(m_toggle));
	save_item(NAME(m_driver_horz));
	save_item(NAME(m_driver_vert));
	save_item(NAME(m_driver_pic));
	save_item(NAME(m_drones_horz));
	save_item(NAME(m_drones_vert));
	save_item(NAME(m_drones_mask));
	save_item(NAME(m_track_horz));
	save_item(NAME(m_track_vert));
	save_item(NAME(m_track_ice));
}


uint32_t sspeedr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_track(bitmap, cliprect);
	draw_drones(bitmap, cliprect);
	draw_driver(bitmap, cliprect);
	return 0;
}


void sspeedr_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_toggle ^= 1;
	}
}


void sspeedr_state::machine_start()
{
	m_digits.resolve();
	m_lampgo.resolve();
	m_lampep.resolve();
}

void sspeedr_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		int r = BIT(i, 0) ? 0xb0 : 0x20;
		int g = BIT(i, 1) ? 0xb0 : 0x20;
		int b = BIT(i, 2) ? 0xb0 : 0x20;

		if (BIT(i, 3))
		{
			r += 0x4f;
			g += 0x4f;
			b += 0x4f;
		}

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void sspeedr_state::int_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


void sspeedr_state::lamp_w(uint8_t data)
{
	m_lampgo = BIT(data, 0);
	m_lampep = BIT(data, 1);
	machine().bookkeeping().coin_counter_w(0, BIT(data, 3));
}


// uses a 7447A, which is equivalent to an LS47/48
constexpr uint8_t ls48_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

void sspeedr_state::time_w(offs_t offset, uint8_t data)
{
	m_digits[24 + offset] = ls48_map[data & 0xf];
}

void sspeedr_state::score_w(offs_t offset, uint8_t data)
{
	m_digits[offset] = ls48_map[~data & 0xf];
}


void sspeedr_state::sound1_w(uint8_t data)
{
	// **** Output pins from 74174 latch at C2 ****

	// Bits 0-3 (PEDAL_BIT0 to PEDAL_BIT3): accelerator pedal position
	// Sets the frequency and volume of the engine sound oscillators.
	for (int i = 0; i < 4; i++)
		m_pedal_bit[i]->write_line(BIT(data, i));

	// Bit 4 (HI SHIFT): set when gearshift is in high gear
	// Modifies the engine sound to be lower pitched at a given speed and
	// to change more slowly.
	m_hi_shift->write_line(BIT(data, 4));

	// Bit 5 (LO SHIFT): set when gearshift is in low gear
	// Modifies the engine sound to be higher pitched at a given speed and
	// to change faster.
	m_lo_shift->write_line(BIT(data, 5));

	// Bits 6-7 (D6, D7): not connected.
}


void sspeedr_state::sound2_w(uint8_t data)
{
	// **** Output pins from 74174 latch at D2 ****

	// Bit 0 (BOOM): Set to activate boom sound for a crash. Cleared to
	// terminate boom.
	m_boom->write_line(BIT(data, 0));

	// Bit 1 (ENGINE SOUND OFF): Set to turn *off* engine sound.
	// Used in a crash.
	m_engine_sound_off->write_line(BIT(data, 1));

	// Bit 2 (NOISE CR 1): tire squealing sound
	// Set to activate "tire squeal" noise from noise generator.
	m_noise_cr[0]->write_line(BIT(data, 2));

	// Bit 3 (NOISE CR 2): secondary crash noise
	// Set to activate high-pitched screeching hiss that accompanies BOOM
	// when the the car crashes. In Super Speed Race, the BOOM and NOISE
	// CR 2 effects play simultaneously.
	m_noise_cr[1]->write_line(BIT(data, 3));

	// Bit 4 (SILENCE): mute all sound when game is not running.
	m_silence->write_line(BIT(data, 4));

	// Bits 5-7 (D5, D6, D7): not connected.
}


void sspeedr_state::prg_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x21ff).ram();
	map(0x7f00, 0x7f17).w(FUNC(sspeedr_state::score_w));
}


void sspeedr_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w(FUNC(sspeedr_state::sound1_w));
	map(0x01, 0x01).portr("IN1").w(FUNC(sspeedr_state::sound2_w));
	map(0x02, 0x02).w(FUNC(sspeedr_state::lamp_w));
	map(0x03, 0x03).portr("DSW");
	map(0x04, 0x04).portr("IN2");
	map(0x04, 0x05).w(FUNC(sspeedr_state::time_w));
	map(0x06, 0x06).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x10, 0x10).w(FUNC(sspeedr_state::driver_horz_w));
	map(0x11, 0x11).w(FUNC(sspeedr_state::driver_pic_w));
	map(0x12, 0x12).w(FUNC(sspeedr_state::driver_horz_2_w));
	map(0x13, 0x13).w(FUNC(sspeedr_state::drones_horz_w));
	map(0x14, 0x14).w(FUNC(sspeedr_state::drones_horz_2_w));
	map(0x15, 0x15).w(FUNC(sspeedr_state::drones_mask_w));
	map(0x16, 0x16).w(FUNC(sspeedr_state::driver_vert_w));
	map(0x17, 0x18).w(FUNC(sspeedr_state::track_vert_w));
	map(0x19, 0x19).w(FUNC(sspeedr_state::track_horz_w));
	map(0x1a, 0x1a).w(FUNC(sspeedr_state::track_horz_2_w));
	map(0x1b, 0x1b).w(FUNC(sspeedr_state::track_ice_w));
	map(0x1c, 0x1e).w(FUNC(sspeedr_state::drones_vert_w));
	map(0x1f, 0x1f).w(FUNC(sspeedr_state::int_ack_w));
}


static const ioport_value sspeedr_controller_table[] =
{
	0x3f, 0x3e, 0x3c, 0x3d, 0x39, 0x38, 0x3a, 0x3b,
	0x33, 0x32, 0x30, 0x31, 0x35, 0x34, 0x36, 0x37,
	0x27, 0x26, 0x24, 0x25, 0x21, 0x20, 0x22, 0x23,
	0x2b, 0x2a, 0x28, 0x29, 0x2d, 0x2c, 0x2e, 0x2f,
	0x0f, 0x0e, 0x0c, 0x0d, 0x09, 0x08, 0x0a, 0x0b,
	0x03, 0x02, 0x00, 0x01, 0x05, 0x04, 0x06, 0x07,
	0x17, 0x16, 0x14, 0x15, 0x11, 0x10, 0x12, 0x13,
	0x1b, 0x1a, 0x18, 0x19, 0x1d, 0x1c, 0x1e, 0x1f
};


static INPUT_PORTS_START( sspeedr )

	PORT_START("IN0")
	PORT_BIT( 0x3f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(64) PORT_REMAP_TABLE(sspeedr_controller_table) PORT_WRAPS PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("IN1")
	// The gas pedal is adjusted physically so the encoder is at position 2 when the pedal is not pressed.
	// It also only uses half of the encoder.
	PORT_BIT( 0x1f, 0x00, IPT_POSITIONAL_V ) PORT_POSITIONS(30) PORT_REMAP_TABLE(sspeedr_controller_table + 2) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x08, "Play Time" )              PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "60 seconds")
	PORT_DIPSETTING(    0x04, "70 seconds")
	PORT_DIPSETTING(    0x08, "80 seconds")
	PORT_DIPSETTING(    0x0c, "90 seconds")
	PORT_DIPNAME( 0x10, 0x00, "Extended Play" )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "20 seconds" )
	PORT_DIPSETTING(    0x10, "30 seconds" )
	PORT_DIPNAME( 0xe0, 0x20, DEF_STR( Service_Mode ) )  PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x20, "Play Mode" )
	PORT_DIPSETTING(    0xa0, "RAM/ROM Test" )
	PORT_DIPSETTING(    0xe0, "Accelerator Adjustment" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("POT_MASTER_VOL")
	PORT_ADJUSTER( 50, "Pot: Master Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "pot_master_vol")
INPUT_PORTS_END


static const gfx_layout car_layout =
{
	32, 16,
	16,
	4,
	{ 0, 1, 2, 3 },
	{
		0x04, 0x04, 0x00, 0x00, 0x0c, 0x0c, 0x08, 0x08,
		0x14, 0x14, 0x10, 0x10, 0x1c, 0x1c, 0x18, 0x18,
		0x24, 0x24, 0x20, 0x20, 0x2c, 0x2c, 0x28, 0x28,
		0x34, 0x34, 0x30, 0x30, 0x3c, 0x3c, 0x38, 0x38
	},
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0
	},
	0x400
};


static GFXDECODE_START( gfx_sspeedr )
	GFXDECODE_ENTRY( "driver", 0, car_layout, 0, 1 )
	GFXDECODE_ENTRY( "drone", 0, car_layout, 0, 1 )
GFXDECODE_END


void sspeedr_state::sspeedr(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 19.968_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &sspeedr_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &sspeedr_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(sspeedr_state::irq0_line_assert));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.318181_MHz_XTAL / 2, 456, 0, 376, 264, 0, 248);
	screen.set_screen_update(FUNC(sspeedr_state::screen_update));
	screen.screen_vblank().set(FUNC(sspeedr_state::screen_vblank));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sspeedr);
	PALETTE(config, m_palette, FUNC(sspeedr_state::palette), 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(sspeedr))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, m_pedal_bit[0], "I_PEDAL_BIT0", 0);
	NETLIST_LOGIC_INPUT(config, m_pedal_bit[1], "I_PEDAL_BIT1", 0);
	NETLIST_LOGIC_INPUT(config, m_pedal_bit[2], "I_PEDAL_BIT2", 0);
	NETLIST_LOGIC_INPUT(config, m_pedal_bit[3], "I_PEDAL_BIT3", 0);
	NETLIST_LOGIC_INPUT(config, m_hi_shift, "I_HI_SHIFT", 0);
	NETLIST_LOGIC_INPUT(config, m_lo_shift, "I_LO_SHIFT", 0);
	NETLIST_LOGIC_INPUT(config, m_boom, "I_BOOM", 0);
	NETLIST_LOGIC_INPUT(config, m_engine_sound_off, "I_ENGINE_SOUND_OFF", 0);
	NETLIST_LOGIC_INPUT(config, m_noise_cr[0], "I_NOISE_CR_1", 0);
	NETLIST_LOGIC_INPUT(config, m_noise_cr[1], "I_NOISE_CR_2", 0);
	NETLIST_LOGIC_INPUT(config, m_silence, "I_SILENCE", 0);

	// Audio output is from an LM3900 op-amp whose output has a
	// peak-to-peak range of about 12 volts, centered on 6 volts.
	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(1.0 / 6.0, -1.0);

	// Netlist volume-potentiometer interface
	NETLIST_ANALOG_INPUT(config, "sound_nl:pot_master_vol", "R70.DIAL");
}


ROM_START( sspeedr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ssr0000.pgm", 0x0000, 0x0800, CRC(bfc7069a) SHA1(2f7aa3d3c7cfd804ba4b625c6a8338534a204855) )
	ROM_LOAD( "ssr0800.pgm", 0x0800, 0x0800, CRC(ec46b59a) SHA1(d5727efecb32ad3d034b885e4a57d7373368ca9e) )

	ROM_REGION( 0x0800, "driver", 0 )
	ROM_LOAD( "ssrm762a.f3", 0x0000, 0x0800, CRC(de4653a9) SHA1(a6bbffb7eb60581eee43c74d20ca00b50c9a6e07) )

	ROM_REGION( 0x0800, "drone", 0 )
	ROM_LOAD( "ssrm762b.j3", 0x0000, 0x0800, CRC(ef6a1cd6) SHA1(77c31f14783e5ba90849bdc930b099c8360aeba7) )

	ROM_REGION( 0x0800, "track", 0 )
	ROM_LOAD( "ssrm762c.l3", 0x0000, 0x0800, CRC(ebaad3ee) SHA1(54ac994b505d20c75cf07a4f68da12360ee00153) )
ROM_END

} // anonymous namespace


GAMEL( 1979, sspeedr, 0, sspeedr, sspeedr, sspeedr_state, empty_init, ROT270, "Taito (Midway license)", "Super Speed Race", MACHINE_SUPPORTS_SAVE, layout_sspeedr )
