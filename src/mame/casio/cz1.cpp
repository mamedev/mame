// license: BSD-3-Clause
// copyright-holders: Devin Acker
/***************************************************************************

    Casio CZ-1 Digital Synthesizer

    Also includes support for the MZ-1, an unreleased rack mount version supported by
    the same ROM. This has no key/wheel/pedal inputs and never sends the corresponding
    MIDI messages, but otherwise works identically to the CZ-1.

    Misc. notes:

    Hold all three Line 1 envelope buttons (DCO/DCW/DCA) on boot to perform a RAM test.

    Afterwards, the firmware will attempt to load and run a program from cartridge
    if a valid 5-byte header is detected at the beginning:
    - bytes 0-1: ignored
    - bytes 2-3: program load address (valid within 0x8000-9fff, includes this header)
    - byte 4: constant 0xD1
    - bytes 5+: start of program

    TODO:

    Both machines have MACHINE_IMPERFECT_SOUND due to unemulated stereo chorus.

***************************************************************************/

#include "emu.h"

#include "ra3.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/msm6200.h"
#include "machine/nvram.h"
#include "sound/mixer.h"
#include "sound/upd933.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include <cmath>

#include "cz1.lh"
#include "mz1.lh"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cz1_state : public driver_device
{
public:
	cz1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_mcu(*this, "mcu"),
		m_hd44780(*this, "hd44780"),
		m_upd933(*this, "upd933_%u", 0U),
		m_cart(*this, "cart"),
		m_mixer(*this, "mixer%u", 0U),
		m_keys(*this, "KC%u", 0U),
		m_leds(*this, "led_%u.%u", 0U, 0U),
		m_led_env(*this, "led_env%u", 0U),
		m_led_bank(*this, "led_bank%u", 0U),
		m_led_tone(*this, "led_tone%u", 0U)
	{ }

	void mz1(machine_config &config);
	void cz1(machine_config &config);

	int cont_r();
	int sync_r();

	int cont49_r();
	int sync49_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void cz1_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void mz1_main_map(address_map &map) ATTR_COLD;
	void cz1_main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void mcu_map(address_map &map) ATTR_COLD;

	// main CPU r/w methods
	u8 keys_r();
	void led_w(offs_t offset, u8 data);
	void volume_w(u8 data);
	void stereo_w(u8 data);

	void cart_addr_w(u8 data);
	void cart_addr_hi_w(u8 data);

	void main_pa_w(u8 data);
	u8 main_pa_r();
	void main_pb_w(u8 data);
	void main_pc_w(u8 data);

	// sub CPU r/w methods
	void sound_w(u8 data);
	u8 sound_r();

	void sub_pa_w(u8 data);
	void sub_pb_w(u8 data);
	void sub_pc_w(u8 data);

	// main/sub CPU comm methods
	void main_to_sub_0_w(u8 data);
	TIMER_CALLBACK_MEMBER(main_to_sub_0_cb);
	u8 main_to_sub_0_r();
	void main_to_sub_1_w(u8 data);
	TIMER_CALLBACK_MEMBER(main_to_sub_1_cb);
	u8 main_to_sub_1_r();
	void sub_to_main_w(u8 data);
	TIMER_CALLBACK_MEMBER(sub_to_main_cb);
	u8 sub_to_main_r();

	void sync_clr_w(u8);
	TIMER_CALLBACK_MEMBER(sync_clr_cb);

	void main_irq_w(u8);
	void main_irq_ack_w(u8);

	// main CPU / key MCU comm methods
	u8 mcu_r();
	void mcu_p2_w(u8 data);

	required_device<upd7810_device> m_maincpu;
	required_device<upd7810_device> m_subcpu;
	optional_device<i8049_device> m_mcu;
	required_device<hd44780_device> m_hd44780;
	required_device_array<upd933_device, 2> m_upd933;
	required_device<casio_ram_cart_device> m_cart;
	required_device_array<mixer_device, 2> m_mixer;
	optional_ioport_array<16> m_keys;
	output_finder<5, 6> m_leds;
	output_finder<16> m_led_env;
	output_finder<8> m_led_bank, m_led_tone;

	float m_volume[0x40];

	u8 m_main_port[3];
	u8 m_mcu_p2;
	u8 m_midi_rx;

	u8 m_main_to_sub[2];
	u8 m_sub_to_main;
	u8 m_sync, m_sync49;

	u16 m_cart_addr;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void cz1_state::mz1_main_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram().share("mainram");
	map(0xb000, 0xbfff).w(FUNC(cz1_state::volume_w));
	map(0xc000, 0xc000).mirror(0x1ff0).w(FUNC(cz1_state::main_to_sub_0_w));
	map(0xc001, 0xc001).mirror(0x1ff0).w(FUNC(cz1_state::main_to_sub_1_w));
	map(0xc002, 0xc002).mirror(0x1ff0).r(FUNC(cz1_state::sub_to_main_r));
	map(0xc004, 0xc004).mirror(0x1ff0).r(FUNC(cz1_state::keys_r));
	map(0xc005, 0xc005).mirror(0x1ff0).w(FUNC(cz1_state::sync_clr_w));
	map(0xc006, 0xc006).mirror(0x1ff0).w(FUNC(cz1_state::main_irq_ack_w));
	map(0xc007, 0xc00b).mirror(0x1ff0).w(FUNC(cz1_state::led_w));
	map(0xc00c, 0xc00c).mirror(0x1ff0).w(FUNC(cz1_state::stereo_w));
	map(0xc00e, 0xc00e).mirror(0x1ff0).w(FUNC(cz1_state::cart_addr_w));
	map(0xc00f, 0xc00f).mirror(0x1ff0).w(FUNC(cz1_state::cart_addr_hi_w));
}

/**************************************************************************/
void cz1_state::cz1_main_map(address_map &map)
{
	mz1_main_map(map);
	map(0xc00d, 0xc00d).mirror(0x1ff0).r(FUNC(cz1_state::mcu_r));
}

/**************************************************************************/
void cz1_state::sub_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).ram().share("subram");
	map(0x8000, 0x9fff).rw(FUNC(cz1_state::sound_r), FUNC(cz1_state::sound_w));
	map(0xa000, 0xbfff).rw(FUNC(cz1_state::main_to_sub_1_r), FUNC(cz1_state::sub_to_main_w));
	map(0xc000, 0xdfff).rw(FUNC(cz1_state::main_to_sub_0_r), FUNC(cz1_state::main_irq_w));
}

/**************************************************************************/
void cz1_state::mcu_map(address_map &map)
{
	map(0x00, 0xff).rw("kbd", FUNC(msm6200_device::read), FUNC(msm6200_device::write));
}

//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( mz1 )

	PORT_START("KC0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Normal")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tone Mix")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Key Split")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Operation Memory")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Solo")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MIDI")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Portamento On/Off")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Glide On/Off")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Key Transpose")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Master Tune")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Exchange")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cartridge")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Bank A")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Bank B")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Bank C")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Bank D")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Bank E")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Bank F")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Memory 5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Memory 6")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Memory 7")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Memory 8")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Bank G")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Bank H")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT)  PORT_NAME("Cursor Left / No")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Cursor Right / Yes")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Memory 1")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Memory 2")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Memory 3")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Memory 4")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Page Down")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_PLUS_PAD)  PORT_NAME("Page Up")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Env. Point Sustain")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Env. Point End")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_UP)   PORT_NAME("Value Down / Save")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Value Up / Load")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Wheel / Aftertouch")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bend Range")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Glide")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Portamento")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Name")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Cartridge/MIDI Save/Load")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Initialize")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Octave")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Vibrato")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Line Select")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Ring")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Noise")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCO 1 Wave")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCO 1 Env")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCW 1 Key Follow")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCW 1 Env")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCA 1 Key Follow")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCA 1 Env")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCO 2 Wave")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCO 2 Env")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCW 2 Key Follow")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCW 2 Env")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCA 2 Key Follow")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCA 2 Env")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCA 1 Velocity")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCA 2 Velocity")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCA 1 Level")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("DCA 2 Level")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Parameter Copy")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Detune")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MIDI On/Off")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Compare/Recall")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Write")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Modulation On/Off")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER)  PORT_TOGGLE PORT_NAME("Memory Protect")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KC12")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("cart", casio_ram_cart_device, exists)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) // low = MZ-1, high = CZ-1
	PORT_BIT(0xfc, IP_ACTIVE_LOW,  IPT_UNUSED)

	PORT_START("MAIN_PB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(cz1_state, sync_r)
	PORT_BIT(0xfe, IP_ACTIVE_LOW,  IPT_UNUSED)

	PORT_START("SUB_PB")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("upd933_0", upd933_device, rq_r)
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("upd933_1", upd933_device, rq_r)
	PORT_BIT(0xfc, IP_ACTIVE_LOW,  IPT_UNUSED)

	PORT_START("SUB_PC")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(cz1_state, sync_r)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(cz1_state, cont_r)
	PORT_BIT(0xf8, IP_ACTIVE_LOW,  IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( cz1 )
	PORT_INCLUDE(mz1)

	PORT_START("kbd:KI8")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C6")

	PORT_START("kbd:KI9")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#5")

	PORT_START("kbd:KI10")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#5")

	PORT_START("kbd:KI11")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#5")

	PORT_START("kbd:KI12")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E5")

	PORT_START("kbd:KI13")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D5")

	PORT_START("kbd:KI14")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C5")

	PORT_START("kbd:KI15")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#4")

	PORT_START("kbd:KI16")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#4")

	PORT_START("kbd:KI17")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#4")

	PORT_START("kbd:KI18")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E4")

	PORT_START("kbd:KI19")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D4")

	PORT_START("kbd:KI20")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C4")

	PORT_START("kbd:KI21")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#3")

	PORT_START("kbd:KI22")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#3")

	PORT_START("kbd:KI23")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#3")

	PORT_START("kbd:KI24")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E3")

	PORT_START("kbd:KI25")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D3")

	PORT_START("kbd:KI26")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C3")

	PORT_START("kbd:KI27")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#2")

	PORT_START("kbd:KI28")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#2")

	PORT_START("kbd:KI29")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#2")

	PORT_START("kbd:KI30")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E2")

	PORT_START("kbd:KI31")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D2")

	PORT_START("kbd:KI32")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C2")

	PORT_START("kbd:KI33")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#1")

	PORT_START("kbd:KI34")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#1")

	PORT_START("kbd:KI35")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#1")

	PORT_START("kbd:KI36")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E1")

	PORT_START("kbd:KI37")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D1")

	PORT_START("kbd:KI38")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C1")

	PORT_START("kbd:VELOCITY")
	PORT_BIT(0x3f, 0x3f, IPT_POSITIONAL_V) PORT_NAME("Key Velocity") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("AN0")
	PORT_BIT(0xff, 0x7f, IPT_PADDLE)       PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_MINMAX(0x00, 0xff) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN1")
	PORT_BIT(0xff, 0x00, IPT_POSITIONAL_V) PORT_NAME("Modulation Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN2")
	PORT_BIT(0xff, 0xff, IPT_POSITIONAL_V) PORT_NAME("Aftertouch") PORT_REVERSE PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(3) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_MODIFY("KC11")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Sustain Pedal")
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("KC12")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_CUSTOM) // low = MZ-1, high = CZ-1

	PORT_START("MAIN_PC")
	PORT_BIT(0x0f, IP_ACTIVE_LOW,  IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(cz1_state, cont49_r)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(cz1_state, sync49_r)
	PORT_BIT(0xc0, IP_ACTIVE_LOW,  IPT_UNUSED)
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void cz1_state::cz1_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 63,  59,  62)); // LCD pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // LCD pixel off
}

/**************************************************************************/
HD44780_PIXEL_UPDATE( cz1_state::lcd_pixel_update )
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 16)
		bitmap.pix(1 + y + line*8 + line, 1 + pos*6 + x) = state ? 1 : 2;
}


/**************************************************************************/
u8 cz1_state::keys_r()
{
	return m_keys[m_main_port[0] & 0xf].read_safe(0xffff);
}

/**************************************************************************/
void cz1_state::led_w(offs_t offset, u8 data)
{
	for (int i = 0; i < 6; i++)
		m_leds[offset][i] = BIT(data, i);
}

/**************************************************************************/
void cz1_state::volume_w(u8 data)
{
	const float vol = m_volume[~data & 0x3f];
	m_mixer[0]->set_output_gain(ALL_OUTPUTS, vol);
	m_mixer[1]->set_output_gain(ALL_OUTPUTS, vol);
}

/**************************************************************************/
void cz1_state::stereo_w(u8 data)
{
	/*
	bit 0: sound chip #1 routing (0: center, 1: left)
	bit 1: sound chip #2 routing (0: center, 2: right)
	bit 2: center channel stereo chorus (0: on, 1: off)
	*/
	m_mixer[0]->set_input_gain(1, BIT(data, 1) ? 0.0 : 1.0);
	m_mixer[1]->set_input_gain(0, BIT(data, 0) ? 0.0 : 1.0);
}

/**************************************************************************/
void cz1_state::cart_addr_w(u8 data)
{
	m_cart_addr &= 0x3f00;
	m_cart_addr |= ~data;
}

/**************************************************************************/
void cz1_state::cart_addr_hi_w(u8 data)
{
	m_cart_addr &= 0xff;
	m_cart_addr |= (~data & 0x3f) << 8;
}

/**************************************************************************/
void cz1_state::main_pa_w(u8 data)
{
	m_hd44780->db_w(data);
	m_main_port[0] = data;
}

/**************************************************************************/
u8 cz1_state::main_pa_r()
{
	u8 data = m_hd44780->db_r();
	if (!BIT(m_main_port[1], 2))
		data &= m_cart->read(m_cart_addr);
	return data;
}

/**************************************************************************/
void cz1_state::main_pb_w(u8 data)
{
	if (BIT(data ^ m_main_port[1], 2))
		m_subcpu->set_input_line(UPD7810_INTF1, BIT(data, 2));

	if (BIT(data, 4) && !BIT(m_main_port[1], 4) && BIT(m_main_port[1], 6))
		m_cart->write(m_cart_addr, m_main_port[0]);

	m_hd44780->e_w(BIT(~data, 7));
	m_hd44780->rw_w(BIT(data, 6));
	m_hd44780->rs_w(BIT(data, 5));

	m_main_port[1] = data;
}

/**************************************************************************/
void cz1_state::main_pc_w(u8 data)
{
	m_main_port[2] = data;
}


/**************************************************************************/
void cz1_state::sound_w(u8 data)
{
	m_upd933[0]->write(data);
	m_upd933[1]->write(data);
}

/**************************************************************************/
u8 cz1_state::sound_r()
{
	return m_upd933[0]->read() & m_upd933[1]->read();
}

/**************************************************************************/
void cz1_state::sub_pa_w(u8 data)
{
	for (int i = 0; i < 15; i++)
		m_led_env[i] = (BIT(data, 0, 4) == i);
	for (int i = 0; i < 8; i++)
		m_led_tone[i] = !BIT(data, 7) && (BIT(data, 4, 3) == i);
}

/**************************************************************************/
void cz1_state::sub_pb_w(u8 data)
{
	for (int i = 0; i < 2; i++)
	{
		m_upd933[i]->id_w(BIT(data, 5));
		m_upd933[i]->cs_w(BIT(data, 2 + i));

		m_upd933[i]->set_output_gain(ALL_OUTPUTS, BIT(data, 6) ? 0.0 : 1.0);
	}
}

/**************************************************************************/
void cz1_state::sub_pc_w(u8 data)
{
	for (int i = 0; i < 8; i++)
		m_led_bank[i] = !BIT(data, 0) && (BIT(data, 5, 3) == i);
}

/**************************************************************************/
void cz1_state::main_to_sub_0_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(cz1_state::main_to_sub_0_cb), this), data);
}

/**************************************************************************/
TIMER_CALLBACK_MEMBER(cz1_state::main_to_sub_0_cb)
{
	m_main_to_sub[0] = param;
	m_sync = 1;
}

/**************************************************************************/
u8 cz1_state::main_to_sub_0_r()
{
	if (!machine().side_effects_disabled())
		m_sync = 0;
	return m_main_to_sub[0];
}

/**************************************************************************/
void cz1_state::main_to_sub_1_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(cz1_state::main_to_sub_1_cb), this), data);
}

/**************************************************************************/
TIMER_CALLBACK_MEMBER(cz1_state::main_to_sub_1_cb)
{
	m_main_to_sub[1] = param;
}

/**************************************************************************/
u8 cz1_state::main_to_sub_1_r()
{
	return m_main_to_sub[1];
}

/**************************************************************************/
void cz1_state::sub_to_main_w(u8 data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(cz1_state::sub_to_main_cb), this), data);
}

/**************************************************************************/
TIMER_CALLBACK_MEMBER(cz1_state::sub_to_main_cb)
{
	m_sub_to_main = param;
}

/**************************************************************************/
u8 cz1_state::sub_to_main_r()
{
	return m_sub_to_main;
}

/**************************************************************************/
int cz1_state::cont_r()
{
	return BIT(m_main_port[1], 3);
}

/**************************************************************************/
int cz1_state::sync_r()
{
	return m_sync;
}

/**************************************************************************/
void cz1_state::sync_clr_w(u8)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(cz1_state::sync_clr_cb), this), 0);
}

/**************************************************************************/
TIMER_CALLBACK_MEMBER(cz1_state::sync_clr_cb)
{
	m_sync = 0;
}

/**************************************************************************/
void cz1_state::main_irq_w(u8)
{
	m_maincpu->set_input_line(UPD7810_INTF1, ASSERT_LINE);
}

/**************************************************************************/
void cz1_state::main_irq_ack_w(u8)
{
	m_maincpu->set_input_line(UPD7810_INTF1, CLEAR_LINE);
}

/**************************************************************************/
u8 cz1_state::mcu_r()
{
	if (!machine().side_effects_disabled())
		m_sync49 = 0;

	return ~m_mcu->p1_r();
}

/**************************************************************************/
void cz1_state::mcu_p2_w(u8 data)
{
	if (BIT(data ^ m_mcu_p2, 6))
		m_maincpu->set_input_line(UPD7810_INTF2, BIT(~data, 6));

	if (BIT(~data & m_mcu_p2, 7))
		m_sync49 = 1;

	m_mcu_p2 = data;
}

/**************************************************************************/
int cz1_state::cont49_r()
{
	return BIT(m_mcu_p2, 5);
}

/**************************************************************************/
int cz1_state::sync49_r()
{
	return m_sync49;
}

/**************************************************************************/
void cz1_state::machine_start()
{
	m_leds.resolve();
	m_led_env.resolve();
	m_led_bank.resolve();
	m_led_tone.resolve();

	// aftertouch amp levels (TODO: are these correct?)
	for (int i = 0; i < 0x40; i++)
		m_volume[i] = pow(2, (float)i / 0x3f) - 1.0;

	m_main_port[0] = m_main_port[1] = m_main_port[2] = 0xff;
	m_mcu_p2 = 0xff;

	// register for save states
	save_item(NAME(m_main_port));
	save_item(NAME(m_mcu_p2));
	save_item(NAME(m_midi_rx));
	save_item(NAME(m_cart_addr));
	save_item(NAME(m_main_to_sub));
	save_item(NAME(m_sub_to_main));
	save_item(NAME(m_sync));
	save_item(NAME(m_sync49));
}

/**************************************************************************/
void cz1_state::machine_reset()
{
	m_main_to_sub[0] = m_main_to_sub[1] = 0;
	m_sub_to_main = 0;
	m_sync = 0;
	m_sync49 = 1;

	m_cart_addr = 0;
	m_midi_rx = 1;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void cz1_state::mz1(machine_config &config)
{
	UPD7810(config, m_maincpu, 15_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cz1_state::mz1_main_map);
	m_maincpu->pa_in_cb().set(FUNC(cz1_state::main_pa_r));
	m_maincpu->pa_out_cb().set(FUNC(cz1_state::main_pa_w));
	m_maincpu->pb_in_cb().set_ioport("MAIN_PB");
	m_maincpu->pb_out_cb().set(FUNC(cz1_state::main_pb_w));
	m_maincpu->pc_out_cb().set(FUNC(cz1_state::main_pc_w));

	CLOCK(config, "midi_clock", 2_MHz_XTAL).signal_handler().set(m_maincpu, FUNC(upd7810_device::sck_w));

	UPD7810(config, m_subcpu, 15_MHz_XTAL);
	m_subcpu->set_addrmap(AS_PROGRAM, &cz1_state::sub_map);
	m_subcpu->pa_out_cb().set(FUNC(cz1_state::sub_pa_w));
	m_subcpu->pb_in_cb().set_ioport("SUB_PB");
	m_subcpu->pb_out_cb().set(FUNC(cz1_state::sub_pb_w));
	m_subcpu->pc_in_cb().set_ioport("SUB_PC");
	m_subcpu->pc_out_cb().set(FUNC(cz1_state::sub_pc_w));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline(m_subcpu, UPD7810_INTF2);

	NVRAM(config, "mainram");
	NVRAM(config, "subram");
	CASIO_RA6(config, m_cart);
	SOFTWARE_LIST(config, "cart_list").set_original("cz1_cart");

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set([this] (int state) { m_midi_rx = state; });
	m_maincpu->rxd_func().set([this] () { return m_midi_rx; });

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	m_maincpu->txd_func().set("mdout", FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(6*16 + 1, 19);
	screen.set_visarea_full();
	screen.set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(cz1_state::cz1_palette), 3);

	HD44780(config, m_hd44780, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_hd44780->set_lcd_size(2, 16);
	m_hd44780->set_function_set_at_any_time();
	m_hd44780->set_pixel_update_cb(FUNC(cz1_state::lcd_pixel_update));

	config.set_default_layout(layout_mz1);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MIXER(config, m_mixer[0]).add_route(0, "lspeaker", 1.0);
	MIXER(config, m_mixer[1]).add_route(0, "rspeaker", 1.0);

	UPD933(config, m_upd933[0], 8.96_MHz_XTAL / 2);
	m_upd933[0]->irq_cb().set("irq",  FUNC(input_merger_any_high_device::in_w<0>));
	m_upd933[0]->add_route(0, m_mixer[0], 1.0);
	m_upd933[0]->add_route(0, m_mixer[1], 1.0);

	UPD933(config, m_upd933[1], 8.96_MHz_XTAL / 2);
	m_upd933[1]->irq_cb().set("irq",  FUNC(input_merger_any_high_device::in_w<1>));
	m_upd933[1]->add_route(0, m_mixer[0], 1.0);
	m_upd933[1]->add_route(0, m_mixer[1], 1.0);
}

/**************************************************************************/
void cz1_state::cz1(machine_config &config)
{
	mz1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &cz1_state::cz1_main_map);
	m_maincpu->pc_in_cb().set_ioport("MAIN_PC");
	m_maincpu->an0_func().set_ioport("AN0");
	m_maincpu->an1_func().set_ioport("AN1");
	m_maincpu->an2_func().set_ioport("AN2");

	I8049(config, m_mcu, 8.96_MHz_XTAL);
	m_mcu->set_addrmap(AS_IO, &cz1_state::mcu_map);
	m_mcu->p2_out_cb().set(FUNC(cz1_state::mcu_p2_w));
	m_mcu->t0_in_cb().set(FUNC(cz1_state::sync49_r));
	m_mcu->t1_in_cb().set([this] () { return BIT(m_main_port[2], 7); });

	MSM6200(config, "kbd").irq_cb().set_inputline(m_mcu, MCS48_INPUT_IRQ);

	config.set_default_layout(layout_cz1);
}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( cz1 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("upd27c256c-20a154.bin", 0x0000, 0x8000, CRC(a970ee65) SHA1(269f2e823ac6353eca9fdb682deebeb7d4d0f585))

	ROM_REGION(0x2000, "mainram", 0)
	ROM_LOAD("init_main.bin", 0x0000, 0x2000, CRC(25fbf88a) SHA1(b7eee5af1d3470ea951df3a019ba2e2a055e84c7))

	ROM_REGION(0x4000, "subcpu", 0)
	ROM_LOAD("upd23c128ec-036.bin", 0x0000, 0x4000, CRC(3cf23c4e) SHA1(b27ee664c31526058defd8e8666ec8e7828059a2))

	ROM_REGION(0x4000, "subram", 0)
	ROM_LOAD("init_sub.bin", 0x0000, 0x4000,  CRC(c0b498af) SHA1(73c48bf5df0d3660c50c370286559a8d4cdb6b99))

	ROM_REGION(0x800, "mcu", 0) // this dump is actually uPD80C49HC-187 from the HT-6000, though it appears functionally identical
	ROM_LOAD("upd8049hc-672.bin", 0x000, 0x800, BAD_DUMP CRC(47b47af7) SHA1(8f0515f95dcc6e224a8a59e0c2cd7ddb4796e34e))
ROM_END

#define rom_mz1 rom_cz1

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME            FLAGS
SYST( 1986, cz1,    0,      0,      cz1,     cz1,    cz1_state,    empty_init, "Casio", "CZ-1",             MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
SYST( 1986, mz1,    cz1,    0,      mz1,     mz1,    cz1_state,    empty_init, "Casio", "MZ-1 (prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
