// license: BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************
    Casio CZ-230S digital synthesizer and SZ-1 MIDI sequencer

    Misc. stuff:
    Both of these devices have a way of loading and running external code.
    - CZ-230S:
      Hold "portamento speed", "value up", and "value down" together on boot. This will cause the
      LCD to display "L-", as when loading from tape. At this point, the unit will try to load
      $700 bytes over MIDI/serial to address $3800 and then jump to it.
    - SZ-1:
      While not recording or playing, pressing the Rest + Dot + Triplet buttons at the same time will
      cause the firmware to check for a JMP instruction (54) at the first byte of cartridge memory
      ($e000), and execute it if there is one.

    TODO: auto power off. Even after activating this, both units still continue executing as normal
    (and the power switch itself is not connected to the CPU, unlike on the CZ-101/1000)

***************************************************************************/

#include "emu.h"

#include "ra3.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/upd7810/upd7810.h"
#include "imagedev/cassette.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "sound/beep.h"
#include "sound/upd933.h"
#include "sound/upd934g.h"
#include "video/mn1252.h"

#include "screen.h"
#include "speaker.h"

#include "cz230s.lh"
#include "sz1.lh"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cz230s_state : public driver_device
{
public:
	cz230s_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_lcdc(*this, "lcdc"),
		m_cassette(*this, "cassette"),
		m_pd(*this, "pd"),
		m_pcm(*this, "pcm"),
		m_keys(*this, "KC%u", 0U),
		m_lcd_seg(*this, "%u.%u", 0U, 0U),
		m_led(*this, "led%u.%u", 0U, 0U),
		m_rhythm(*this, "rhythm_pos"),
		m_mode(*this, "mode_pos")
	{ }

	void config_base(machine_config &config, u16 screen_w, u16 screen_h, bool midi_thru = true);
	void cz230s(machine_config &config);
	void sz1(machine_config &config);

	void keys_w(int state) { m_key_sel = state; }
	void keys_mux_w(int state) { m_key_mux = state; }
	template <int Row> ioport_value keys_row_r();
	template <int Row> u8 keys_analog_r();

	DECLARE_INPUT_CHANGED_MEMBER(rhythm_w);
	template <int Bit> ioport_value rhythm_r() { return m_rhythm >> Bit; }
	DECLARE_INPUT_CHANGED_MEMBER(mode_w);
	ioport_value mode_r() { return m_mode; }

	void cassette_w(int state);
	void cassette_motor_w(int state);
	ioport_value cassette_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void cz230s_map(address_map &map) ATTR_COLD;
	void cz230s_pcm_map(address_map &map) ATTR_COLD;
	void sz1_map(address_map &map) ATTR_COLD;

	void pcm_w(offs_t offset, u8 data);
	template <int Num> void led_w(u8 data);
	void port_a_w(u8 data);
	u8 keys_r();

	void render_w(int state);

	required_device<upd7811_device> m_maincpu;
	required_device<mn1252_device> m_lcdc;
	required_device<cassette_image_device> m_cassette;
	optional_device<upd933_device> m_pd;
	optional_device<upd934g_device> m_pcm;

	optional_ioport_array<12> m_keys;

	output_finder<6, 9> m_lcd_seg;
	output_finder<2, 8> m_led;
	output_finder<> m_rhythm;
	output_finder<> m_mode;

	u8 m_port_a;
	u8 m_key_sel;
	u8 m_key_mux;
	u8 m_midi_rx;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void cz230s_state::cz230s_map(address_map &map)
{
	map.unmap_value_high();

//  map(0x0000, 0x0fff).rom(); - internal
	map(0x1000, 0x1fff).w(FUNC(cz230s_state::pcm_w));
	map(0x2000, 0x3fff).ram().share("nvram");
	map(0x4000, 0x7fff).rw(m_pd, FUNC(upd933_device::read), FUNC(upd933_device::write));
	map(0x8000, 0xffff).rom().region("program", 0);
}

/**************************************************************************/
void cz230s_state::cz230s_pcm_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

/**************************************************************************/
void cz230s_state::sz1_map(address_map &map)
{
	map.unmap_value_high();

//  map(0x0000, 0x0fff).rom(); - internal
	map(0x4000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x9fff).mirror(0x2000).ram().share("nvram");
	map(0xc000, 0xcfff).w(FUNC(cz230s_state::led_w<0>));
	map(0xd000, 0xdfff).w(FUNC(cz230s_state::led_w<1>));
	map(0xe000, 0xffff).rw("cart", FUNC(casio_ram_cart_device::read), FUNC(casio_ram_cart_device::write));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( cz230s )
	PORT_START("KC0")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C2")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#2")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D2")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#2")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E2")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F2")
	PORT_BIT(0x1c0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Solo / Insert")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Portamento On/Off")
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Bend Range / Check")

	PORT_START("KC1")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#2")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G2")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#2")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A2")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#2")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B2")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Rhythm 1")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Rhythm 2")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Rhythm 3")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Rhythm 4")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Rhythm 5")
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Preset 1")

	PORT_START("KC2")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C3")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#3")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D3")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#3")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E3")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F3")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Rhythm 6")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Rhythm 7")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Rhythm 8")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Rhythm 9")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Rhythm 10")
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Preset 2")

	PORT_START("KC3")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#3")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G3")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#3")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A3")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#3")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B3")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Program")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Intro / Fill In")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("Tempo Down")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Tempo Up")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Start / Stop / Record")
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Synchro / Clear")

	PORT_START("KC4")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C4")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#4")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D4")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#4")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E4")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F4")
	PORT_BIT(0xfc0, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(cz230s_state, rhythm_r<0>)

	PORT_START("KC5")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#4")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G4")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#4")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A4")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#4")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B4")
	PORT_BIT(0xfc0, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(cz230s_state, rhythm_r<6>)

	PORT_START("KC6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C5")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#5")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D5")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#5")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E5")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F5")
	PORT_BIT(0x3c0, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(cz230s_state, rhythm_r<12>)
	PORT_BIT(0xc00, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#5")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G5")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#5")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A5")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#5")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B5")
	PORT_BIT(0x7c0, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(cz230s_state, mode_r)
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC8")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C6")
	PORT_BIT(0x03e, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("MT")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("MIDI Channel")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Portamento Speed")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Transpose")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Value Down / Save")
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_UP)   PORT_NAME("Value Up / Load")

	PORT_START("KC9")
	PORT_BIT(0x03f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Tone 4")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Tone 5")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Tone 6")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Tone 7")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Tone 8")
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Tone 9")

	PORT_START("KC10")
	PORT_BIT(0x03f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Tone 0")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Tone 1")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Tone 2")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Tone 3")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Cancel")
	PORT_BIT(0x800, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC11")
	PORT_BIT(0x03f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune Down")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tune Up")
	PORT_BIT(0x100, 0x000, IPT_OTHER ) PORT_TOGGLE PORT_NAME("MIDI Clock")
	PORT_DIPSETTING(0x100, "External")
	PORT_DIPSETTING(0x000, "Internal")
	PORT_BIT(0x200, 0x000, IPT_OTHER ) PORT_TOGGLE PORT_NAME("MIDI")
	PORT_DIPSETTING(0x200, DEF_STR(Off)) // this should be on by default
	PORT_DIPSETTING(0x000, DEF_STR(On))
	PORT_BIT(0x400, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Auto Power Off")

	PORT_START("RHYTHM")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (BD)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0001)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (SD)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0002)
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (LT)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0004)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (HT)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0008)
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (LB)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0010)
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (HB)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0020)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (CH)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0040)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (Rim)")   PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0080)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (OH)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0100)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (CB)")    PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0200)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (Ride)")  PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0400)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (Claps)") PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x0800)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (PD 1)")  PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x1000)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (PD 2)")  PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x2000)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (PD 3)")  PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x4000)
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Rhythm Sound (PD 4)")  PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, rhythm_w, 0x8000)

	PORT_START("MODE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Mode (Pattern Play)")       PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, mode_w, 0x01)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Mode (Pattern Memory 4/4)") PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, mode_w, 0x02)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Mode (Pattern Memory 3/4)") PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, mode_w, 0x04)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Mode (Song Play)")          PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, mode_w, 0x08)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Mode (Song Memory)")        PORT_CHANGED_MEMBER(DEVICE_SELF, cz230s_state, mode_w, 0x10)

	PORT_START("PB")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(cz230s_state, keys_w)
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("pd", upd933_device, rq_r)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("pd", upd933_device, cs_w)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OUTPUT) // TODO: auto power off

	PORT_START("PC")
	PORT_BIT(0x07, IP_ACTIVE_HIGH, IPT_UNUSED) // MIDI in/out/clock
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(cz230s_state, cassette_r)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(cz230s_state, cassette_w)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(cz230s_state, cassette_motor_w)
	PORT_BIT(0xc0, IP_ACTIVE_LOW,  IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(cz230s_state, keys_mux_w)

	PORT_START("AN1")
	PORT_BIT(0xff, 0x7f, IPT_PADDLE) PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN2")
	PORT_CONFNAME(0xff, 0xff, "Battery Level")
	PORT_CONFSETTING(   0x00, "Low")
	PORT_CONFSETTING(   0xff, "Normal")

	PORT_START("AN3")
	PORT_BIT(0xff, 0xff, IPT_POSITIONAL_H) PORT_NAME("PD Rhythm Volume") PORT_REVERSE PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

INPUT_PORTS_END

static INPUT_PORTS_START( sz1 )
	PORT_START("KC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Rest")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Dot")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Triplet")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Tie")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("Reverse / Save")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Forward / Load")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Play / Check")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Stop")

	PORT_START("KC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("8th Note")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("16th Note")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("32nd Note")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN ) PORT_NAME("Tempo Down")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Real Time")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Manual")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Record")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Reset")

	PORT_START("KC2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Quarter Note")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Half Note")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Whole Note")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("Tempo Up")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Track 1")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Track 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Track 3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Track 4")

	PORT_START("KC3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Copy")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_INSERT) PORT_NAME("Insert")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL)    PORT_NAME("Delete")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER)  PORT_NAME("Metronome / Enter")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Repeat")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("MIDI")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Auto Power Off")
	PORT_BIT(0x20, 0x00, IPT_OTHER ) PORT_TOGGLE PORT_NAME("MIDI Clock")
	PORT_DIPSETTING(0x20, "External")
	PORT_DIPSETTING(0x00, "Internal")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Touch Data")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Cartridge / MT")

	PORT_START("PA")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", mn1252_device, data_w)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", mn1252_device, ce_w)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("lcdc", mn1252_device, std_w)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(cz230s_state, keys_row_r<7>)

	PORT_START("PB")
	PORT_BIT(0x1f, IP_ACTIVE_LOW,  IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(cz230s_state, keys_w)
	PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OUTPUT) // TODO: auto power off

	PORT_START("PC")
	PORT_BIT(0x07, IP_ACTIVE_HIGH, IPT_UNUSED) // MIDI in/out/clock
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(cz230s_state, cassette_r)
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_OUTPUT) PORT_WRITE_LINE_DEVICE_MEMBER("beep", beep_device, set_state)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(cz230s_state, cassette_motor_w)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(cz230s_state, cassette_w)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME("Foot Switch")
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void cz230s_state::machine_start()
{
	m_lcd_seg.resolve();
	m_led.resolve();
	m_rhythm.resolve();
	m_mode.resolve();

	m_rhythm = 1;
	m_mode = 1;

	m_port_a = 0;

	save_item(NAME(m_port_a));
	save_item(NAME(m_key_sel));
	save_item(NAME(m_key_mux));
	save_item(NAME(m_midi_rx));
}

/**************************************************************************/
void cz230s_state::machine_reset()
{
	m_key_sel = m_key_mux = 0;
	m_midi_rx = 1;
}


/**************************************************************************/
void cz230s_state::pcm_w(offs_t offset, u8 data)
{
	data = (BIT(offset, 0, 6) << 2) | BIT(offset, 8, 2);
	m_pcm->write(offset >> 10, data);
}

/**************************************************************************/
template <int Num>
void cz230s_state::led_w(u8 data)
{
	for (int i = 0; i < 8; i++)
		m_led[Num][i] = BIT(data, i);
}

/**************************************************************************/
void cz230s_state::port_a_w(u8 data)
{
	m_lcdc->data_w(data & 0xf);
	m_lcdc->std_w(BIT(data, 5));
	m_lcdc->ce_w(BIT(data, 6));

	if (BIT(data, 7) && !BIT(m_port_a, 7))
		led_w<0>(~data & 0x3f);

	m_port_a = data;
}

/**************************************************************************/
u8 cz230s_state::keys_r()
{
	u8 data = 0x3f;

	if (m_key_sel < m_keys.size())
	{
		const u16 input = m_keys[m_key_sel].read_safe(0xfff);
		if (BIT(m_key_mux, 0))
			data &= (input & 0x3f);
		if (BIT(m_key_mux, 1))
			data &= (input >> 6);
	}

	return data;
}

/**************************************************************************/
template <int Row>
ioport_value cz230s_state::keys_row_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 5; i++)
		if (BIT(m_key_sel, i))
			data &= m_keys[i].read_safe(0xff);

	return BIT(data, Row);
}

/**************************************************************************/
template <int Row>
u8 cz230s_state::keys_analog_r()
{
	return keys_row_r<Row>() ? 0xff : 0x00;
}

/**************************************************************************/
INPUT_CHANGED_MEMBER(cz230s_state::rhythm_w)
{
	if (!oldval && newval)
		m_rhythm = param;
}

/**************************************************************************/
INPUT_CHANGED_MEMBER(cz230s_state::mode_w)
{
	if (!oldval && newval)
		m_mode = param;
}

/**************************************************************************/
void cz230s_state::cassette_w(int state)
{
	m_cassette->output(state ? -1.0 : 1.0);
}

/**************************************************************************/
void cz230s_state::cassette_motor_w(int state)
{
	m_cassette->change_state(state ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

/**************************************************************************/
ioport_value cz230s_state::cassette_r()
{
	return m_cassette->input() > 0 ? 0 : 1;
}

/**************************************************************************/
void cz230s_state::render_w(int state)
{
	if (!state)
		return;

	for (int digit = 0; digit < 6; digit++)
	{
		const u16 data = m_lcdc->output(digit);
		for (int seg = 0; seg < 9; seg++)
			m_lcd_seg[digit][seg] = BIT(data, seg);
	}
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void cz230s_state::config_base(machine_config &config, u16 screen_w, u16 screen_h, bool midi_thru)
{
	UPD7811(config, m_maincpu, 10_MHz_XTAL);

	CLOCK(config, "midi_clock", 2_MHz_XTAL).signal_handler().set(m_maincpu, FUNC(upd7810_device::sck_w));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set([this](int state) { m_midi_rx = state; });
	m_maincpu->rxd_func().set([this]() { return m_midi_rx; });

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	m_maincpu->txd_func().set("mdout", FUNC(midi_port_device::write_txd));

	if (midi_thru)
	{
		MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
		mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));
	}

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MN1252(config, m_lcdc);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_SVG);
	screen.set_refresh_hz(60);
	screen.set_size(screen_w, screen_h);
	screen.set_visarea_full();
	screen.screen_vblank().set(FUNC(cz230s_state::render_w));

	SPEAKER(config, "speaker").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "speaker", 0.05);
}

/**************************************************************************/
void cz230s_state::cz230s(machine_config &config)
{
	config_base(config, 975, 205);

	m_maincpu->set_addrmap(AS_PROGRAM, &cz230s_state::cz230s_map);
	m_maincpu->pa_in_cb().set(FUNC(cz230s_state::keys_r));
	m_maincpu->pa_out_cb().set(FUNC(cz230s_state::port_a_w));
	m_maincpu->pb_in_cb().set_ioport("PB");
	m_maincpu->pb_out_cb().set_ioport("PB");
	m_maincpu->pc_in_cb().set_ioport("PC");
	m_maincpu->pc_out_cb().set_ioport("PC");
	m_maincpu->an1_func().set_ioport("AN1");
	m_maincpu->an2_func().set_ioport("AN2");
	m_maincpu->an3_func().set_ioport("AN3");

	UPD933(config, m_pd, 8.96_MHz_XTAL / 2);
	m_pd->irq_cb().set_inputline(m_maincpu, UPD7810_INTF1);
	m_pd->add_route(0, "speaker", 1.0);

	UPD934G(config, m_pcm, 1'280'000);
	m_pcm->set_addrmap(0, &cz230s_state::cz230s_pcm_map);
	m_pcm->add_route(ALL_OUTPUTS, "speaker", 0.5);

	config.set_default_layout(layout_cz230s);
}

/**************************************************************************/
void cz230s_state::sz1(machine_config &config)
{
	config_base(config, 938, 205, false);

	m_maincpu->set_addrmap(AS_PROGRAM, &cz230s_state::sz1_map);
	m_maincpu->pa_in_cb().set_ioport("PA");
	m_maincpu->pa_out_cb().set_ioport("PA");
	m_maincpu->pb_out_cb().set_ioport("PB");
	m_maincpu->pc_in_cb().set_ioport("PC");
	m_maincpu->pc_out_cb().set_ioport("PC");
	m_maincpu->an0_func().set(FUNC(cz230s_state::keys_analog_r<0>));
	m_maincpu->an1_func().set(FUNC(cz230s_state::keys_analog_r<1>));
	m_maincpu->an2_func().set(FUNC(cz230s_state::keys_analog_r<2>));
	m_maincpu->an3_func().set(FUNC(cz230s_state::keys_analog_r<3>));
	m_maincpu->an4_func().set(FUNC(cz230s_state::keys_analog_r<4>));
	m_maincpu->an5_func().set(FUNC(cz230s_state::keys_analog_r<5>));
	m_maincpu->an6_func().set(FUNC(cz230s_state::keys_analog_r<6>));

	CASIO_RA5(config, "cart");

	BEEP(config, "beep", 2000).add_route(ALL_OUTPUTS, "speaker", 0.5); // TODO: verify freq

	config.set_default_layout(layout_sz1);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( cz230s )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7811g-301.bin", 0x0000, 0x1000, CRC(506b008c) SHA1(2d91d817bd0fa4688591160e53cbc6e14acd7014))

	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("hn613256pda4.bin", 0x0000, 0x8000, CRC(f58758ec) SHA1(11e5c95e51e1c77c89682ea3db85b9457f8b6cf6))

	ROM_REGION(0x8000, "pcm", 0)
	ROM_LOAD("hn613256pct1.bin", 0x0000, 0x8000, CRC(97b9805b) SHA1(f3502a26b6a9bccb60bea11ae940619ab9960e05))

	ROM_REGION(0x2000, "nvram", 0)
	ROM_LOAD("init_ram.bin", 0x0000, 0x2000, CRC(eb756425) SHA1(3a21b45269a00d27d5943de50825edc329062c60))

	ROM_REGION(0x7bb5, "screen", 0)
	ROM_LOAD("cz230s.svg", 0x0000, 0x7bb5, CRC(e35cc3d3) SHA1(36cb369414f1e65843cd0ea318ad27f536b582be))
ROM_END

ROM_START( sz1 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7811g-120.bin", 0x0000, 0x1000, CRC(597ac04a) SHA1(96451a764296eaa22aaad3cba121226dcba865f4))

	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("program.bin", 0x0000, 0x4000, CRC(15f83fa5) SHA1(cb0d8d8390266f247dc7718b95bc658d1719d105))

	ROM_REGION(0x6437, "screen", 0)
	ROM_LOAD("sz1.svg", 0x0000, 0x6437, CRC(fd14625b) SHA1(069790868b382725d309fcab0148147f76ff82cc))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS
SYST( 1985, cz230s,  0,      0,      cz230s,  cz230s, cz230s_state, empty_init, "Casio", "CZ-230S", MACHINE_SUPPORTS_SAVE )
SYST( 1985, sz1,     0,      0,      sz1,     sz1,    cz230s_state, empty_init, "Casio", "SZ-1",    MACHINE_SUPPORTS_SAVE )
