// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h83002.h"
#include "machine/nvram.h"
#include "sound/swp00.h"
#include "video/t6963c.h"

#include "screen.h"
#include "speaker.h"

#include "utf8.h"

namespace {

class qs300_state : public driver_device {
public:
	qs300_state(const machine_config &mconfig, device_type type, const char *tag) :
		qs300_state(mconfig, type, tag, false)
	{
	}

	void qs300(machine_config &config);

protected:
	qs300_state(const machine_config &mconfig, device_type type, const char *tag, bool is_eos) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_swp00(*this, "swp00"),
		m_lcdc(*this, "vs254300"),
		m_nvram(*this, "ram"),
		m_inputs(*this, "DR%u", 0U),
		m_is_eos(is_eos)
	{
	}

	required_device<h83002_device> m_maincpu;
	required_device<h83002_device> m_subcpu;
	required_device<swp00_device> m_swp00;
	required_device<t6963c_device> m_lcdc;
	required_device<nvram_device> m_nvram;
	required_ioport_array<7> m_inputs;
	//required_ioport m_sustain;
	//required_ioport m_pitch_bend;

	bool const m_is_eos;

	u8 m_mlatch, m_slatch;
	bool m_mlatch_full, m_slatch_full;

	u8 m_mpb;

	void mainmap(address_map &map) ATTR_COLD;
	void submap(address_map &map) ATTR_COLD;
	void lcdmap(address_map &map) ATTR_COLD;

	u8 mlatch_r();
	void mlatch_w(u8 data);
	u8 slatch_r();
	void slatch_w(u8 data);

	u8 mp6_r();
	void mp6_w(u8 data);
	u8 mp7_r();
	u8 mpa_r();
	u8 mpb_r();
	void mpb_w(u8 data);

	u8 sp9_r();
	u8 spa_r();

	void lcd_palette(palette_device &palette) const;

	void machine_start() override ATTR_COLD;
};

class eos_b900_state : public qs300_state
{
public:
	eos_b900_state(const machine_config &mconfig, device_type type, const char *tag) :
		qs300_state(mconfig, type, tag, true)
	{
	}
};

void qs300_state::machine_start()
{
	save_item(NAME(m_mlatch));
	save_item(NAME(m_mlatch_full));
	save_item(NAME(m_slatch));
	save_item(NAME(m_slatch_full));
	save_item(NAME(m_mpb));

	m_mlatch = 0;
	m_mlatch_full = false;
	m_slatch = 0;
	m_slatch_full = false;
	m_mpb = 0;
}

u8 qs300_state::mlatch_r()
{
	if(!machine().side_effects_disabled()) {
		m_mlatch_full = false;
		m_maincpu->set_input_line(4, CLEAR_LINE);
	}
	return m_mlatch;
}

void qs300_state::mlatch_w(u8 data)
{
	m_mlatch = data;
	m_mlatch_full = true;
	m_maincpu->set_input_line(4, ASSERT_LINE);
}

u8 qs300_state::mpa_r()
{
	return (m_slatch_full ? 0x40 : 0x00) | (m_is_eos ? 0x00 : 0x20);
}

u8 qs300_state::slatch_r()
{
	if(!machine().side_effects_disabled()) {
		m_slatch_full = false;
		m_subcpu->set_input_line(4, CLEAR_LINE);
	}
	return m_slatch;
}

void qs300_state::slatch_w(u8 data)
{
	m_slatch = data;
	m_slatch_full = true;
	m_subcpu->set_input_line(4, ASSERT_LINE);
}

u8 qs300_state::spa_r()
{
	return m_mlatch_full ? 0x40 : 0x00;
}

u8 qs300_state::mpb_r()
{
	return m_mpb;
}

void qs300_state::mpb_w(u8 data)
{
	m_mpb = data;
}

u8 qs300_state::mp6_r()
{
	return 0xff;
}

void qs300_state::mp6_w(u8 data)
{
	// Bits 1-2 are related to leds
}

u8 qs300_state::mp7_r()
{
	// Some bits are inverted with transistors for led driving reasons.
	u8 mask = m_mpb ^ 0x1b;
	u8 res = 0;
	for(u32 i=0; i != 7; i++)
		if(BIT(mask, i))
			res |= m_inputs[i]->read();

	return res;
}

u8 qs300_state::sp9_r()
{
	// 0x20 = suspend pedal
	return 0x00;
}

void qs300_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(2, 176, 219));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}

void qs300_state::qs300(machine_config &config)
{
	H83002(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &qs300_state::mainmap);
	m_maincpu->read_port6().set(FUNC(qs300_state::mp6_r));
	m_maincpu->write_port6().set(FUNC(qs300_state::mp6_w));
	m_maincpu->read_port7().set(FUNC(qs300_state::mp7_r));
	m_maincpu->read_porta().set(FUNC(qs300_state::mpa_r));
	m_maincpu->read_portb().set(FUNC(qs300_state::mpb_r));
	m_maincpu->write_portb().set(FUNC(qs300_state::mpb_w));

	H83002(config, m_subcpu, 16_MHz_XTAL);
	m_subcpu->read_adc<0>().set_constant(0);     // Aftertouch
	m_subcpu->read_adc<1>().set_constant(0);     // Pitch bend
	m_subcpu->read_adc<2>().set_constant(0);     // Modulation wheel
	m_subcpu->read_adc<3>().set_constant(0x3ff); // Generic continuous controller, wired to +5V
	m_subcpu->read_adc<4>().set_constant(0);     // Foot control
	m_subcpu->read_adc<5>().set_constant(0);     // Foot volume
	m_subcpu->read_adc<6>().set_constant(0x3ff); // Unconnected
	m_subcpu->read_adc<7>().set_constant(0x276); // Battery (3V)
	m_subcpu->set_addrmap(AS_PROGRAM, &qs300_state::submap);
	m_subcpu->read_port9().set(FUNC(qs300_state::sp9_r));
	m_subcpu->read_porta().set(FUNC(qs300_state::spa_r));

	SWP00(config, m_swp00);
	m_swp00->add_route(0, "lspeaker", 1.0);
	m_swp00->add_route(1, "rspeaker", 1.0);

	T6963C(config, m_lcdc, 270000);
	m_lcdc->set_addrmap(0, &qs300_state::lcdmap);
	m_lcdc->set_fs(2);
	m_lcdc->set_md(0x13);

	PALETTE(config, "palette", FUNC(qs300_state::lcd_palette), 2);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(240, 80);
	screen.set_visarea(0, 239, 0, 63);
	screen.set_screen_update("vs254300", FUNC(t6963c_device::screen_update));
	screen.set_palette("palette");

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_maincpu, FUNC(h83002_device::sci_rx_w<0>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));

	// Annoying but required for faster inter-cpu communication
	config.set_maximum_quantum(attotime::from_usec(10));
}

void qs300_state::mainmap(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x207fff).ram().share("ram");
	map(0x420000, 0x420000).rw(FUNC(qs300_state::mlatch_r), FUNC(qs300_state::slatch_w));
	map(0x440000, 0x440001).rw(m_lcdc, FUNC(t6963c_device::read), FUNC(t6963c_device::write));

	// 480000: fdc

	map(0x600000, 0x67ffff).ram().mirror(0x180000);
}

void qs300_state::submap(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("subcpu", 0);
	map(0x200000, 0x2007ff).m(m_swp00, FUNC(swp00_device::map));
	map(0x400000, 0x400000).rw(FUNC(qs300_state::slatch_r), FUNC(qs300_state::mlatch_w));
	map(0x600000, 0x61ffff).ram();
}

void qs300_state::lcdmap(address_map &map)
{
	map(0x0000, 0x7fff).ram();
}

static INPUT_PORTS_START( qs300 )
	PORT_START("DR0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Song")     PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Voice")    PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Shift")    PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DR1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Pattern")  PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Phrase")   PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("F1")       PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("F2")       PORT_CODE(KEYCODE_W)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7")        PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("8")        PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("9")        PORT_CODE(KEYCODE_9)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DR2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Utility")  PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Disk")     PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("F3")       PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("F4")       PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4")        PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5")        PORT_CODE(KEYCODE_5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6")        PORT_CODE(KEYCODE_6)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DR3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Record")   PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Top")      PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("F5")       PORT_CODE(KEYCODE_T)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("F6")       PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1")        PORT_CODE(KEYCODE_1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2")        PORT_CODE(KEYCODE_2)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3")        PORT_CODE(KEYCODE_3)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DR4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Stop")     PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Run")      PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("F7")       PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("F8")       PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("0")        PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("-")        PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter")    PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DR5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("<<")       PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME(">>")       PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Exit")     PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Dec/No")   PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME(UTF8_UP)    PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Inc/Yes")  PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("DR6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Edit")     PORT_CODE(KEYCODE_C)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Job")      PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Store")    PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME(UTF8_LEFT)  PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME(UTF8_DOWN)  PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

ROM_START( qs300 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "xq055e0.ic04", 0x000000, 0x100000, CRC(abdbcc1f) SHA1(998c3cd5b14d407cb0e1fcdea9a9f585cf73fe5a))
	ROM_LOAD16_WORD_SWAP( "xq320e0.ic24", 0x100000, 0x100000, CRC(5fe1a151) SHA1(7e06716795fdd24e33a922a1f3fe77e6082b2abb))

	ROM_REGION( 0x80000, "subcpu", 0 )
	ROM_LOAD16_WORD_SWAP( "xq056e0.ic09", 0, 0x80000, CRC(ff0ed0f9) SHA1(be80b5e7c701db708e435c2c825d562cf20a284e))

	ROM_REGION16_LE( 0x400000, "swp00", 0)
	// Identical to the mu50 roms
	ROM_LOAD( "xq057c0.ic10", 0x000000, 0x200000, CRC(d4adbc7e) SHA1(32f653c7644d060f5a6d63a435ae3a7412386d92) )
	ROM_LOAD( "xq058c0.ic11", 0x200000, 0x200000, CRC(7b68f475) SHA1(adf68689b4842ec5bc9b0ea1bb99cf66d2dec4de) )

	ROM_REGION(0x400, "vs254300:cgrom", 0)
	ROM_LOAD("t6963c_0101.bin", 0x000, 0x400, CRC(547d118b) SHA1(0dd3e3acd3d47e6ece644c98c390fc86587373e9))
	// This t6963c_0101 internal CG ROM is similar to lm24014w_0101.bin which may be used as a replacement
ROM_END

ROM_START( eosb900 )
	// Identical afaict. The firmware can almost but not quite handle the sdx 4000 too.
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "xq055e0.ic04", 0x000000, 0x100000, CRC(abdbcc1f) SHA1(998c3cd5b14d407cb0e1fcdea9a9f585cf73fe5a))
	ROM_LOAD16_WORD_SWAP( "xq320e0.ic24", 0x100000, 0x100000, CRC(5fe1a151) SHA1(7e06716795fdd24e33a922a1f3fe77e6082b2abb))

	ROM_REGION( 0x80000, "subcpu", 0 )
	ROM_LOAD16_WORD_SWAP( "xq056e0.ic09", 0, 0x80000, CRC(ff0ed0f9) SHA1(be80b5e7c701db708e435c2c825d562cf20a284e))

	ROM_REGION16_LE( 0x400000, "swp00", 0)
	// Identical to the mu50 roms
	ROM_LOAD( "xq057c0.ic10", 0x000000, 0x200000, CRC(d4adbc7e) SHA1(32f653c7644d060f5a6d63a435ae3a7412386d92) )
	ROM_LOAD( "xq058c0.ic11", 0x200000, 0x200000, CRC(7b68f475) SHA1(adf68689b4842ec5bc9b0ea1bb99cf66d2dec4de) )

	ROM_REGION(0x400, "vs254300:cgrom", 0)
	ROM_LOAD("t6963c_0101.bin", 0x000, 0x400, CRC(547d118b) SHA1(0dd3e3acd3d47e6ece644c98c390fc86587373e9))
	// This t6963c_0101 internal CG ROM is similar to lm24014w_0101.bin which may be used as a replacement
ROM_END

} // anonymous namespace

SYST( 1999, qs300,       0, 0, qs300, qs300, qs300_state,    empty_init, "Yamaha", "QS300",    MACHINE_NOT_WORKING )
SYST( 1999, eosb900, qs300, 0, qs300, qs300, eos_b900_state, empty_init, "Yamaha", "EOS B900", MACHINE_NOT_WORKING )
