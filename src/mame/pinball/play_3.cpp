// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*********************************************************************************************

PINBALL
Playmatic MPU 3,4,5

Note: The input lines INT, EF1-4 are inverted (not true voltage).

First time:
- The default settings are fine, so start with a clean slate
- Wait for it to say No Ball, then hold down X.
- Keep holding X, insert credits, and press start. When it says ball 1, let go immediately.
- If you hold down X longer than you should, it says Coil Error, and the game is ended. You
  should let go the instant the ball number increments.

Setting up:
The manual is not that clear, there's a lot we don't know, this *seems* to work...
- Start machine, wait for it to say No Ball, and it may play a tune
- Press num-1 (to simulate opening the front door)
- Press 1, it says BooP (Bookkeeping)
- Press num-1 and the credits number increments to 8, then 01 and on to 17 (2 pushes per number)
- When it gets to 01, the display says Adjust
- To adjust a setting, press 1 to choose a digit, then tap 9 until it is correct.
- The settings in the manual do not fully line up with what we see in the display.

Small FAQ:
- If game shows "Test" just after start, try cleaning the nvram etc. If it still does it, when
  the red screen shows, hold down num-1 until "No Ball" starts flashing, then release the key.
- Game says "No Ball". Press and hold X.
- Game says "Coil Error". You held down X too long.

Status:
- flashman, fldragon, ironball, kz26, nautilus, phntmshp, ridersrf, rock2500, starfirp, theraid,
   trailer, terrlake: playable
- sklflite: can insert coin, cannot start game
- spain82: see notes below
- megaaton: playable, ball number doesn't show. If it says Test, see FAQ above.
- ufo_x: playable. Make sure to release X after the last ball ends, or you get "Coil Error".
         If you can't seem to score any points, hit minus to kick it into gear.

ToDo:
- Sometimes the machine starts up in a non-responsive state. Exit and restart.
- Output lamps
- Mechanical sounds
- Skill Flight: not working
- Miss Disco: not working (no manual available). Uses different hardware. No sound rom.
- Spain82: can start a game but it's not really playable (no manual available).
   The test buttons are unknown. The ball number doesn't show.
   So, after starting the machine, make sure the displays cycle between blank and 700000. If it
   just sits there it has hung up, exit and restart. Insert a coin, hold X and press 1. Now,
   you need to jiggle F and G and perhaps other nearby keys until the 0 starts flashing. You
   can then score. At outhole time, need to press F,G,X to register the fact. If you hit X too
   many times the machine reboots itself. The machine has 2 capture holes and therefore multiball.
   At ball 2, the playfield becomes unresponsive.
- Meg Aaton: Ball number doesn't show. Can suddenly reboot in the middle of a game. Match digits
   are reversed.

***********************************************************************************************/


#include "emu.h"
#include "genpin.h"

#include "efo_sound3.h"
#include "efo_zsu.h"

#include "cpu/cosmac/cosmac.h"
#include "machine/clock.h"
#include "machine/ripple_counter.h"
#include "machine/7474.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "play_3.lh"

namespace {

class play_3_state : public genpin_class
{
public:
	play_3_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_4013a(*this, "4013a")
		, m_4013b(*this, "4013b")
		, m_4020(*this, "4020")
		, m_ay1(*this, "ay1")
		, m_ay2(*this, "ay2")
		, m_sound3(*this, "sound3")
		, m_zsu(*this, "zsu")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void play_3(machine_config &config);
	void spain82(machine_config &config);
	void flashman(machine_config &config);
	void megaaton(machine_config &config);
	void sklflite(machine_config &config);
	void terrlake(machine_config &config);

private:
	void port01_w(u8 data);
	void terrlake_port01_w(u8 data);
	void port02_w(u8 data);
	void spain82_port02_w(u8 data);
	void port03_w(u8 data);
	void spain82_port03_w(u8 data);
	void sklflite_port03_w(u8 data);
	u8 port04_r();
	u8 port05_r();
	void port06_w(u8 data);
	void flashman_port06_w(u8 data);
	void port07_w(u8 data);
	int clear_r();
	int ef1_r();
	int ef4_r();
	void clockcnt_w(u16 data);
	void clock_w(int state);
	void clock2_w(int state);
	void port01_a_w(u8 data);
	u8 port02_a_r();
	int clear_a_r();

	void terrlake_io(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
	void audio_mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void flashman_io(address_map &map) ATTR_COLD;
	void sklflite_io(address_map &map) ATTR_COLD;
	void spain82_io(address_map &map) ATTR_COLD;

	u8 m_resetcnt = 0U;
	u8 m_resetcnt_a = 0U;
	u8 m_soundlatch = 0U;
	u8 m_port03_old = 0U;
	u8 m_a_irqset = 0U;
	u16 m_a_irqcnt = 0U;
	u8 m_kbdrow = 0U;
	u8 m_segment[5]{};
	bool m_disp_sw = false;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<cosmac_device> m_maincpu;
	optional_device<cosmac_device> m_audiocpu;
	required_device<ttl7474_device> m_4013a;
	required_device<ttl7474_device> m_4013b;
	required_device<ripple_counter_device> m_4020;
	optional_device<ay8910_device> m_ay1;
	optional_device<ay8910_device> m_ay2;
	optional_device<efo_sound3_device> m_sound3;
	optional_device<efo_zsu_device> m_zsu;
	required_ioport_array<10> m_io_keyboard;
	output_finder<70> m_digits;
	output_finder<64> m_io_outputs;   // 16 solenoids + 48 lamps
};


void play_3_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x80ff).ram().share("nvram"); // pair of 5101, battery-backed
}

void play_3_state::io_map(address_map &map)
{
	map(0x01, 0x01).w(FUNC(play_3_state::port01_w)); // digits, scan-lines
	map(0x02, 0x02).w(FUNC(play_3_state::port02_w)); // sound code
	map(0x03, 0x03).w(FUNC(play_3_state::port03_w)); //
	map(0x04, 0x04).r(FUNC(play_3_state::port04_r)); // switches
	map(0x05, 0x05).r(FUNC(play_3_state::port05_r)); // more switches
	map(0x06, 0x06).w(FUNC(play_3_state::port06_w)); // segments
	map(0x07, 0x07).w(FUNC(play_3_state::port07_w)); // flipflop clear
}

void play_3_state::spain82_io(address_map &map)
{
	io_map(map);
	map(0x02, 0x02).w(FUNC(play_3_state::spain82_port02_w));
	map(0x03, 0x03).w(FUNC(play_3_state::spain82_port03_w));
}

void play_3_state::flashman_io(address_map &map)
{
	io_map(map);
	map(0x01, 0x01).w(FUNC(play_3_state::terrlake_port01_w)); // digits, scan-lines
	map(0x06, 0x06).w(FUNC(play_3_state::flashman_port06_w)); // segments
}

void play_3_state::sklflite_io(address_map &map)
{
	io_map(map);
	map(0x03, 0x03).w(FUNC(play_3_state::sklflite_port03_w)); //
}

void play_3_state::terrlake_io(address_map &map)
{
	sklflite_io(map);
	map(0x01, 0x01).w(FUNC(play_3_state::terrlake_port01_w)); // digits, scan-lines
}

void play_3_state::audio_mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4001).mirror(0x1ffe).rw(m_ay1, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x6000, 0x6001).mirror(0x1ffe).rw(m_ay2, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x8000, 0x80ff).ram();
}

void play_3_state::audio_io_map(address_map &map)
{
	map(0x01, 0x01).w(FUNC(play_3_state::port01_a_w)); // irq counter
	map(0x02, 0x02).r(FUNC(play_3_state::port02_a_r)); // sound code
}


static INPUT_PORTS_START( play_3 )
	PORT_START("X0") // 11-18
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP11")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP12")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP13")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP14")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP15")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP16")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP17")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP18")

	PORT_START("X1") // 21-28
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP21")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP22")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP23")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP24")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP25")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP26")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP27")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP28")

	PORT_START("X2") // 31-38
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP31")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP32")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP33")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP35")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP36")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP37")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP38")

	PORT_START("X3") // 41-48
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP48")

	PORT_START("X4") // 51-58
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP51")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP52")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP53")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP54")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP55")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP56")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP57")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP58")

	PORT_START("X5") // 61-68
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP61")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP62")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP63")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP64")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP65")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP66")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP67")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("INP68")

	PORT_START("X6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE PORT_NAME("Door Switch")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_TOGGLE PORT_NAME("Test") // test button

	PORT_START("X7")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0xE0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Service") // Marked as "Reset" on some schematics
INPUT_PORTS_END

static INPUT_PORTS_START( megaaton )
	PORT_INCLUDE( play_3 )
	PORT_MODIFY("X0") // 11-18
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_MODIFY("X6")
	//PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_TOGGLE PORT_NAME("Door Switch")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spain82 )
	PORT_INCLUDE( megaaton )
	PORT_MODIFY("X6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START )
INPUT_PORTS_END

void play_3_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_resetcnt_a));
	save_item(NAME(m_port03_old));
	save_item(NAME(m_a_irqset));
	save_item(NAME(m_a_irqcnt));
	save_item(NAME(m_kbdrow));
	save_item(NAME(m_segment));
	save_item(NAME(m_resetcnt));
	save_item(NAME(m_disp_sw));
	save_item(NAME(m_soundlatch));
}

void play_3_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_resetcnt = 0;
	m_resetcnt_a = 0;
	m_4013b->d_w(1);
	m_a_irqset = 54;  // default value of the CDP1863
	m_a_irqcnt = (m_a_irqset << 3) | 7;
	m_soundlatch = 0;
	m_kbdrow = 0;
	m_disp_sw = 0;
	std::fill(std::begin(m_segment), std::end(m_segment), 0);
	m_port03_old = 0;
}

void play_3_state::port01_w(u8 data)
{
	m_kbdrow = data;
	if (m_kbdrow && m_disp_sw)
	{
		m_disp_sw = 0;
		for (u8 j = 0; j < 6; j++)
			if (BIT(m_kbdrow, j))
				for (u8 i = 0; i < 5; i++)
				{
					m_digits[j*10 + i] = m_segment[i] & 0x7f;
					// decimal dot on tens controls if last 0 shows or not
					if ((j == 5) && BIT(m_segment[i], 7))
						m_digits[60 + i] = 0x3f;
				}
	}
}

// terrlake,flashman ball number needs to move over
void play_3_state::terrlake_port01_w(u8 data)
{
	u8 i,j,digit;
	m_kbdrow = data;
	if (m_kbdrow && m_disp_sw)
	{
		m_disp_sw = 0;
		for (j = 0; j < 6; j++)
			if (BIT(m_kbdrow, j))
				for (i = 0; i < 5; i++)
				{
					digit = j*10+i;
					if (digit == 24)
						digit = 64;
					m_digits[digit] = m_segment[i] & 0x7f;
					// decimal dot on tens controls if last 0 shows or not
					if ((j == 5) && (i < 4) && BIT(m_segment[i], 7))
						m_digits[60 + i] = 0x3f;
				}
	}
}

void play_3_state::port02_w(u8 data)
{
	m_soundlatch = data;
}

void play_3_state::spain82_port02_w(u8 data)
{
	m_soundlatch = data;
	m_sound3->input_w(data);
}

void play_3_state::port03_w(u8 data)
{
	if (!BIT(data, 6))
		m_audiocpu->ef1_w(1); // inverted

	if (BIT(data, 5))
	{
		if (m_soundlatch == 11)
			m_samples->start(0, 5); // outhole
		for (u8 i = 0; i < 16; i++)
			m_io_outputs[i] = (m_soundlatch == i) ? 1 : 0;
	}
}

void play_3_state::spain82_port03_w(u8 data)
{
	m_sound3->clock_w(BIT(data, 6));

	if (BIT(data, 5))
	{
		if (m_soundlatch == 11)
			m_samples->start(0, 5); // outhole
		for (u8 i = 0; i < 16; i++)
			m_io_outputs[i] = (m_soundlatch == i) ? 1 : 0;
	}
}

void play_3_state::sklflite_port03_w(u8 data)
{
	if (BIT(data, 6) && !BIT(m_port03_old, 6))
		m_zsu->sound_command_w(m_soundlatch);

	if (BIT(data, 5))
	{
		if (m_soundlatch == 11)
			m_samples->start(0, 5); // outhole
		for (u8 i = 0; i < 16; i++)
			m_io_outputs[i] = (m_soundlatch == i) ? 1 : 0;
	}

	m_port03_old = data;
}

u8 play_3_state::port04_r()
{
	if (m_kbdrow & 0x3f)
		for (u8 i = 0; i < 6; i++)
			if (BIT(m_kbdrow, i))
				return m_io_keyboard[i]->read();

	return 0;
}

u8 play_3_state::port05_r()
{
	u8 data = 0, key8 = m_io_keyboard[8]->read() & 0x0f;
	if (BIT(m_kbdrow, 0))
		data |= m_io_keyboard[6]->read();
	if (BIT(m_kbdrow, 1))
		data |= m_io_keyboard[7]->read();
	return (data & 0xf0) | key8;
}

void play_3_state::port06_w(u8 data)
{
	m_segment[4] = m_segment[3];
	m_segment[3] = m_segment[2];
	m_segment[2] = m_segment[1];
	m_segment[1] = m_segment[0];
	m_segment[0] = data;
	m_disp_sw = 1;
}

void play_3_state::flashman_port06_w(u8 data)
{
	port06_w(bitswap<8>(data, 0, 3, 1, 7, 6, 4, 2, 5));
}

void play_3_state::port07_w(u8 data)
{
	m_4013b->clear_w(0);
	m_4013b->clear_w(1);
}

void play_3_state::port01_a_w(u8 data)
{
	m_a_irqset = data;
	m_a_irqcnt = (m_a_irqset << 3) | 7;
}

u8 play_3_state::port02_a_r()
{
	m_audiocpu->ef1_w(0); // inverted
	return m_soundlatch;
}

int play_3_state::clear_r()
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xff)
		m_resetcnt++;
	return (m_resetcnt < 0xf0) ? 0 : 1;
}

int play_3_state::clear_a_r()
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt_a < 0xff)
		m_resetcnt_a++;
	return (m_resetcnt_a < 0xf0) ? 0 : 1;
}

int play_3_state::ef1_r()
{
	return (!BIT(m_4020->count(), 10)); // inverted
}

int play_3_state::ef4_r()
{
	return BIT(m_io_keyboard[9]->read(), 0); // inverted test/reset button - doesn't seem to do anything
}

void play_3_state::clockcnt_w(u16 data)
{
	m_4013b->preset_w(!BIT(data, 10)); // Q10 output, 4013 is inverted

	if (m_audiocpu.found())
	{
		// sound irq
		m_a_irqcnt--;
		if (m_a_irqcnt == 1)
			m_audiocpu->int_w(1); // inverted
		else
		if (m_a_irqcnt == 0)
		{
			m_a_irqcnt = (m_a_irqset << 3) | 7;
			m_audiocpu->int_w(0); // inverted
		}
	}
}

void play_3_state::clock2_w(int state)
{
	m_4013b->clock_w(state);
	m_maincpu->ef3_w(state); // inverted
}

void play_3_state::play_3(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &play_3_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &play_3_state::io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(play_3_state::clear_r));
	m_maincpu->ef1_cb().set(FUNC(play_3_state::ef1_r));
	m_maincpu->ef4_cb().set(FUNC(play_3_state::ef4_r));
	m_maincpu->q_cb().set(m_4013a, FUNC(ttl7474_device::clear_w)).invert(); // 4013 is inverted
	m_maincpu->tpb_cb().set(m_4013a, FUNC(ttl7474_device::clock_w));
	m_maincpu->tpb_cb().append(m_4020, FUNC(ripple_counter_device::clock_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_play_3);

	// Devices
	clock_device &xpoint(CLOCK(config, "xpoint", 60)); // crossing-point detector
	xpoint.signal_handler().set(FUNC(play_3_state::clock2_w));

	// This is actually a 4013 chip (has 2 RS flipflops)
	TTL7474(config, m_4013a, 0);
	m_4013a->comp_output_cb().set(m_4013a, FUNC(ttl7474_device::d_w));
	m_4013a->output_cb().set(m_4020, FUNC(ripple_counter_device::reset_w));

	TTL7474(config, m_4013b, 0);
	m_4013b->output_cb().set(m_maincpu, FUNC(cosmac_device::ef2_w)).invert(); // inverted
	m_4013b->comp_output_cb().set(m_maincpu, FUNC(cosmac_device::int_w)).invert(); // inverted

	RIPPLE_COUNTER(config, m_4020);
	m_4020->set_stages(14); // only Q10 is actually used
	m_4020->count_out_cb().set(FUNC(play_3_state::clockcnt_w));

	/* Sound */
	genpin_audio(config);

	CDP1802(config, m_audiocpu, 3.579545_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &play_3_state::audio_mem_map);
	m_audiocpu->set_addrmap(AS_IO, &play_3_state::audio_io_map);
	m_audiocpu->wait_cb().set_constant(1);
	m_audiocpu->clear_cb().set(FUNC(play_3_state::clear_a_r));

	SPEAKER(config, "speaker", 2).front();
	AY8910(config, m_ay1, 3.579545_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "speaker", 0.75, 0);
	AY8910(config, m_ay2, 3.579545_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "speaker", 0.75, 1);
	m_ay1->set_resistors_load(6900, 6900, 6900);
	m_ay2->set_resistors_load(6900, 6900, 6900);
	m_ay1->port_a_write_callback().set_nop();
	m_ay2->port_a_write_callback().set_nop();
}

void play_3_state::spain82(machine_config &config)
{
	play_3(config);
	m_maincpu->set_addrmap(AS_IO, &play_3_state::spain82_io);

	config.device_remove("audiocpu");
	config.device_remove("ay1");
	config.device_remove("ay2");
	config.device_remove("speaker");

	EFO_SOUND3(config, m_sound3);
}

void play_3_state::flashman(machine_config &config)
{
	play_3(config);
	m_maincpu->set_addrmap(AS_IO, &play_3_state::flashman_io);
}

void play_3_state::megaaton(machine_config &config)
{
	play_3(config);
	m_maincpu->set_clock(2.95_MHz_XTAL);
}

void play_3_state::sklflite(machine_config &config)
{
	play_3(config);
	m_maincpu->set_addrmap(AS_IO, &play_3_state::sklflite_io);

	config.device_remove("audiocpu");
	config.device_remove("ay1");
	config.device_remove("ay2");
	config.device_remove("speaker");

	EFO_ZSU1(config, m_zsu);
}

void play_3_state::terrlake(machine_config &config)
{
	sklflite(config);
	m_maincpu->set_addrmap(AS_IO, &play_3_state::terrlake_io);
}


/* PLAYMATIC MPU-3/4/5 ALTERNATE ROMS =======================================================================

This is a list of known alternate roms. Nothing has been tested.

KZ-26
ROM_LOAD( "sound2.su4",   0x0000, 0x1000, CRC(b66100d3) SHA1(85f5a319715f99d1b7afeca0d01c81aa615d416a) )
ROM_LOAD( "sound1.su3",   0x0000, 0x2000, CRC(f9550ab4) SHA1(7186158f515fd9fbe5a7a09c6b7d2e8dfc3b4bb2) )

Meg-aaton
ROM_LOAD( "smogot.bin",   0x0000, 0x2000, CRC(92fa0742) SHA1(ef3100a53323fd67e23b47fc3e72fdb4671e9b0a) )

Star Fire
ROM_LOAD( "sound 1b0 _0xd5ac.bin", 0x0000, 0x2000, CRC(9c304f90) SHA1(36114055ebf4f904c2c55f025867f777b33a6a7b) )

*/

/*-------------------------------------------------------------------
/ Spain 82 (10/82)
/-------------------------------------------------------------------*/
ROM_START(spain82)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("spaic12.bin", 0x0000, 0x1000, CRC(cd37ecdc) SHA1(ff2d406b6ac150daef868121e5857a956aabf005))
	ROM_LOAD("spaic11.bin", 0x1000, 0x0800, CRC(c86c0801) SHA1(1b52539538dae883f9c8fe5bc6454f9224780d11))

	ROM_REGION(0x2000, "sound3:rom", 0)
	ROM_LOAD("spasnd.bin", 0x0000, 0x2000, CRC(62412e2e) SHA1(9e48dc3295e78e1024f726906be6e8c3fe3e61b1))
ROM_END

/*-------------------------------------------------------------------
/ Meg Aaton
/-------------------------------------------------------------------*/
ROM_START(megaaton)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("cpumegat.bin", 0x0000, 0x2000, CRC(7e7a4ede) SHA1(3194b367cbbf6e0cb2629cd5d82ddee6fe36985a))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(fefc3ab2) SHA1(e748d9b443a69fcdd587f22c87d41818b6c0e436))
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
ROM_END

ROM_START(megaatona)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("mega_u12.bin", 0x0000, 0x1000, CRC(65761b02) SHA1(dd9586eaf70698ef7a80ce1be293322f64829aea))
	ROM_LOAD("mega_u11.bin", 0x1000, 0x1000, CRC(513f3683) SHA1(0f080a33426df1ffdb14e9b2e6382304e201e335))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(fefc3ab2) SHA1(e748d9b443a69fcdd587f22c87d41818b6c0e436))
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
ROM_END

/*-------------------------------------------------------------------
/ ??/84 Nautilus
/-------------------------------------------------------------------*/
ROM_START(nautilus)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("nautilus.rom", 0x0000, 0x2000, CRC(197e5492) SHA1(0f83fc2e742fd0cca0bd162add4bef68c6620067))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("nautilus.snd", 0x0000, 0x2000, CRC(413d110f) SHA1(8360f652296c46339a70861efb34c41e92b25d0e)) // Bad?
ROM_END

/*-------------------------------------------------------------------
/ ??/84 The Raid
/-------------------------------------------------------------------*/
ROM_START(theraid)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("theraid.rom", 0x0000, 0x2000, CRC(97aa1489) SHA1(6b691b287138cc78cfc1010f380ff8c66342c39b))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("theraid.snd", 0x0000, 0x2000, CRC(e33f8363) SHA1(e7f251c334b15e12b1eb7e079c2e9a5f64338052))
ROM_END

ROM_START(theraida)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("ph_6_1a0.u13", 0x0000, 0x2000, CRC(cc2b1872) SHA1(e61071450cc6b0fa5e6297f75bca0391039dca10))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("mrd_1a0.u3",   0x0000, 0x2000, CRC(e33f8363) SHA1(e7f251c334b15e12b1eb7e079c2e9a5f64338052))
ROM_END

/*-------------------------------------------------------------------
/ 11/84 UFO-X
/-------------------------------------------------------------------*/
ROM_START(ufo_x)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("ufoxcpu.rom", 0x0000, 0x2000, CRC(cf0f7c52) SHA1(ce52da05b310ac84bdd57609e21b0401ee3a2564))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("ufoxu3.rom", 0x0000, 0x2000, CRC(6ebd8ee1) SHA1(83522b76a755556fd38d7b292273b4c68bfc0ddf))
	ROM_LOAD("ufoxu4.rom", 0x2000, 0x0800, CRC(aa54ede6) SHA1(7dd7e2852d42aa0f971936dbb84c7708727ce0e7))
ROM_END

/*-------------------------------------------------------------------
/ KZ-26 (1984)
/-------------------------------------------------------------------*/
ROM_START(kz26)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("kz26.cpu", 0x0000, 0x2000, CRC(8030a699) SHA1(4f86b325801d8ce16011f7b6ba2f3633e2f2af35))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound1.su3", 0x0000, 0x2000, CRC(8ad1a804) SHA1(6177619f09af4302ffddd8c0c1b374dab7f47e91))
	ROM_LOAD("sound2.su4", 0x2000, 0x0800, CRC(355dc9ad) SHA1(eac8bc27157afd908f9bc5b5a7c40be5b9427269))
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Rock 2500
/-------------------------------------------------------------------*/
ROM_START(rock2500)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("r2500cpu.rom", 0x0000, 0x2000, CRC(9c07e373) SHA1(5bd4e69d11e69fdb911a6e65b3d0a7192075abc8))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("r2500snd.rom", 0x0000, 0x2000, CRC(24fbaeae) SHA1(20ff35ed689291f321e483287a977c02e84d4524))
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Star Fire
/-------------------------------------------------------------------*/
ROM_START(starfirp)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("starfcpu.rom", 0x0000, 0x2000, CRC(450ddf20) SHA1(c63c4e3833ffc1f69fcec39bafecae9c80befb2a))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("starfu3.rom", 0x0000, 0x2000, CRC(5d602d80) SHA1(19d21adbcbd0067c051f3033468eda8c5af57be1))
	ROM_LOAD("starfu4.rom", 0x2000, 0x0800, CRC(9af8be9a) SHA1(da6db3716db73baf8e1493aba91d4d85c5d613b4))
ROM_END

ROM_START(starfirpa)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("starcpua.rom", 0x0000, 0x2000, CRC(29bac350) SHA1(ab3e3ea4881be954f7fa7278800ffd791c4581da))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("starfu3.rom", 0x0000, 0x2000, CRC(5d602d80) SHA1(19d21adbcbd0067c051f3033468eda8c5af57be1))
	ROM_LOAD("starfu4.rom", 0x2000, 0x0800, CRC(9af8be9a) SHA1(da6db3716db73baf8e1493aba91d4d85c5d613b4))
ROM_END

/*-------------------------------------------------------------------
/ Trailer (1985)
/-------------------------------------------------------------------*/
ROM_START(trailer)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("trcpu.rom", 0x0000, 0x2000, CRC(cc81f84d) SHA1(7a3282a47de271fde84cfddbaceb118add0df116))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("trsndu3.rom", 0x0000, 0x2000, CRC(05975c29) SHA1(e54d3a5613c3e39fc0338a53dbadc2e91c09ffe3))
	ROM_LOAD("trsndu4.rom", 0x2000, 0x0800, CRC(bda2a735) SHA1(134b5abb813ed8bf2eeac0861b4c88c7176582d8))
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Stop Ship
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ ??/86 Flash Dragon
/-------------------------------------------------------------------*/
ROM_START(fldragon)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("fldrcpu1.rom", 0x0000, 0x2000, CRC(e513ded0) SHA1(64ed3dcff53311fb93bd50d105a4c1186043fdd7))
	ROM_LOAD("fldraudiocpu.rom", 0x2000, 0x2000, CRC(6ff2b276) SHA1(040b614f0b0587521ef5550b5587b94a7f3f178b))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("fdsndu3.rom", 0x0000, 0x2000, CRC(aa9c52a8) SHA1(97d5d63b14d10c70a5eb80c08ccf5a1f3df7596d))
	ROM_LOAD("fdsndu4.rom", 0x2000, 0x0800, CRC(0a7dc1d2) SHA1(32c7be5e9fbe4fa9ca661af7b7b5ea13ef250ce6))
ROM_END

ROM_START(fldragona)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("fldr_1a.cpu", 0x0000, 0x2000, CRC(21fda8e8) SHA1(feea608c2605cea1cdf9f7ed884297a95993f754))
	ROM_LOAD("fldr_2a.cpu", 0x2000, 0x2000, CRC(3592a0b7) SHA1(4c4ed7930dcbbf81ce2e5296c0b36bb615bd2270))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("fdsndu3.rom", 0x0000, 0x2000, CRC(aa9c52a8) SHA1(97d5d63b14d10c70a5eb80c08ccf5a1f3df7596d))
	ROM_LOAD("fdsndu4.rom", 0x2000, 0x0800, CRC(0a7dc1d2) SHA1(32c7be5e9fbe4fa9ca661af7b7b5ea13ef250ce6))
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Skill Flight
/-------------------------------------------------------------------*/
ROM_START(sklflite)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("skflcpu1.rom", 0x0000, 0x2000, CRC(8f833b55) SHA1(1729203582c22b51d1cc401aa8f270aa5cdadabe))
	ROM_LOAD("skflaudiocpu.rom", 0x2000, 0x2000, CRC(ffc497aa) SHA1(3e88539ae1688322b9268f502d8ca41cffb28df3))

	ROM_REGION(0x28000, "zsu:soundcpu", 0) // Z80A soundcard
	ROM_LOAD("skflsnd.rom", 0x0000, 0x8000, CRC(926a1da9) SHA1(16c762fbfe6a55597f26ff55d380192bb8647ee0))
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Phantom Ship
/-------------------------------------------------------------------*/
ROM_START(phntmshp)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("video1.bin", 0x0000, 0x2000, CRC(2b61a8d2) SHA1(1b5cabbab252b2ffb6ed12fb7e4181de7695ed9a))
	ROM_LOAD("video2.bin", 0x2000, 0x2000, CRC(50126db1) SHA1(58d89e44131554cb087c4cad62869f90366704ad))

	ROM_REGION(0x28000, "zsu:soundcpu", 0) // Z80A soundcard
	ROM_LOAD("sonido1.bin", 0x00000, 0x8000, CRC(3294611d) SHA1(5f790b41bcb6d87418c80e61ac8ae69c57864b1d))
	ROM_LOAD("sonido2.bin", 0x08000, 0x8000, CRC(c2efc826) SHA1(44ee144b902627745853011968e0d654b35b3b08))
	ROM_LOAD("sonido3.bin", 0x10000, 0x8000, CRC(13d50f39) SHA1(70624de2dd8412c83866183a83f16cc5b8bdccb8))
	ROM_LOAD("sonido4.bin", 0x18000, 0x8000, CRC(b53f73ed) SHA1(bb928cfee418e8d9698d7bee78a32426f793c6e9))
ROM_END

/*-------------------------------------------------------------------
/ ??/84 Flashman (Sport Matic)
/-------------------------------------------------------------------*/
ROM_START(flashman)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("pf7-1a0.u9", 0x0000, 0x2000, CRC(2cd16521) SHA1(bf9aa293e2ded3f5b1e61a10e6a8ebb8b4e9d4e1))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("mfm-1a0.u3", 0x00000, 0x2000, CRC(456fd555) SHA1(e91d6df15fdfc330ee9edb691ff925ad24afea35))
	ROM_LOAD("mfm-1b0.u4", 0x02000, 0x0800, CRC(90256257) SHA1(c7f2554e500c4e512999b4edc54c86f3335a2b30))
ROM_END

/*-------------------------------------------------------------------
/ ??/86 Rider's Surf (JocMatic)
/-------------------------------------------------------------------*/
ROM_START(ridersrf)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("cpu.bin", 0x0000, 0x2000, CRC(4941938e) SHA1(01e44054e65166d68602d6a38217eda7ea669761))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound.bin", 0x0000, 0x2000, CRC(2db2ecb2) SHA1(365fcac208607acc3e134affeababd6c89dbc74d))
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Iron Balls (Stargame)
/-------------------------------------------------------------------*/
ROM_START(ironball)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("video.bin", 0x0000, 0x2000, CRC(1867ebff) SHA1(485e46c742d914febcbdd58cb5a886f1d773282a))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound.bin", 0x0000, 0x2000, CRC(83165483) SHA1(5076e5e836105d69c4ba606d8b995ecb16f88504))
ROM_END

/*-------------------------------------------------------------------
/ ??/83 Miss Disco (Bingo machine)
/-------------------------------------------------------------------*/
ROM_START(msdisco)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x1000, CRC(06fb7da9) SHA1(36c6fda166b2a07a5ed9ad5d2b6fdfe8fd707b0f))

	ROM_REGION(0x4000, "audiocpu", ROMREGION_ERASEFF)
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Terrific Lake (Sport Matic)
/-------------------------------------------------------------------*/
ROM_START(terrlake)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("jtl_2a3.u9", 0x0000, 0x2000, CRC(f6d3cedd) SHA1(31e0daac1e9215ad0e1557d31d520745ead0f396))

	ROM_REGION(0x28000, "zsu:soundcpu", 0)
	ROM_LOAD("stl_1a0.u3", 0x00000, 0x8000, CRC(b5afdc39) SHA1(fb74de453dfc66b87f3d64508802b3de46d14631))
	ROM_LOAD("stl_1b0.u4", 0x08000, 0x8000, CRC(3bbdd791) SHA1(68cd86cb96a278538d18ca0a77b372309829edf4))
ROM_END

} // anonymous namespace

GAME(1982,  spain82,   0,        spain82,  spain82,  play_3_state, empty_init, ROT0, "Playmatic", "Spain '82",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1983,  megaaton,  0,        megaaton, megaaton, play_3_state, empty_init, ROT0, "Playmatic", "Meg-Aaton",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1983,  megaatona, megaaton, megaaton, megaaton, play_3_state, empty_init, ROT0, "Playmatic", "Meg-Aaton (alternate set)",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1984,  nautilus,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Nautilus",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1984,  theraid,   0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "The Raid",                     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1984,  theraida,  theraid,  play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "The Raid (alternate set)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1984,  ufo_x,     0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "UFO-X",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1984,  kz26,      0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "KZ-26",                        MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1985,  rock2500,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Rock 2500",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1985,  starfirp,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Star Fire",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1985,  starfirpa, starfirp, play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Star Fire (alternate set)",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1985,  trailer,   0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Trailer",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1986,  fldragon,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Flash Dragon",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1986,  fldragona, fldragon, play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Flash Dragon (alternate set)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1987,  phntmshp,  0,        sklflite, play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Phantom Ship",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1987,  sklflite,  0,        sklflite, play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Skill Flight (Playmatic)",     MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
// not by Playmatic, but same hardware
GAME(1984,  flashman,  0,        flashman, play_3,   play_3_state, empty_init, ROT0, "Sport Matic", "Flashman",                   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1986,  ridersrf,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "JocMatic",    "Rider's Surf",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1987,  ironball,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Stargame",    "Iron Balls",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1987,  terrlake,  0,        terrlake, play_3,   play_3_state, empty_init, ROT0, "Sport Matic", "Terrific Lake",              MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
// bingo hardware, to be split (?)
GAME(1983,  msdisco,   0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Miss Disco (Bingo)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
