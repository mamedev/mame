// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**********************************************************************************

PINBALL
Playmatic MPU-C

The IOS board common to all games provides sound effects through the CDP1863.
4 different add-on sound boards were also used:
- Black Fever has the Speaking System board, which produces analog signals for
  controlling an 8-track tape player.
- Zira uses the Sound-2 board with a COP402 and AY-3-8910. The latter device is
  also supposedly used to control lights through a separate connector. These
  lights pulse along with the sounds, something like a disco.
- Cerberus uses the Sound-3 board with a 90435 processor, which is most likely a
  CDP1802 by another name. The 90503 "synthesizer" is the only sound IC on this
  board; it has a TI logo and seems at least pin-compatible with TMS52xx.
- Mad Race uses a Sound Board IV (same as MPU-3 and later), but I/O ports
  that talk to it are unknown.

Test mode:
- Hold down NUM-0 and hit F3. The displays will show the digits one at a time.
    The number of any stuck switch will show in the credit area. Pressing Start
    will do a test of the solenoids (04 will show during this test). Press F3 to
    exit.

Adjustments:
- While game is over, press NUM-0. Keep pressing to go through the bookkeeping
    and the setup. See the manual for specifics.

Status:
- antar, storm, evlfight, attack, blkfever: Working
- Mad Race: J is the outhole. Working, no sound.
- Zira, Cerberus: not working
- Hold down the outhole key (usually X), when starting a game.

ToDo:
- Add remaining mechanical sounds
- Some sound boards to add

***********************************************************************************/


#include "emu.h"
#include "machine/genpin.h"

#include "cpu/cop400/cop400.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/7474.h"
#include "machine/clock.h"
#include "machine/ripple_counter.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/cdp1863.h"
#include "speaker.h"

#include "play_2.lh"

namespace {

class play_2_state : public genpin_class
{
public:
	play_2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_4013a(*this, "4013a")
		, m_4013b(*this, "4013b")
		, m_4020(*this, "4020")
		, m_1863(*this, "1863")
		, m_snd_off(*this, "snd_off")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void play_2(machine_config &config);

protected:
	void port01_w(u8 data);
	void port02_w(u8 data);
	u8 port04_r();
	u8 port05_r();
	void port06_w(u8 data);
	void port07_w(u8 data);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef1_r);
	DECLARE_READ_LINE_MEMBER(ef4_r);
	void clockcnt_w(u16 data);
	DECLARE_WRITE_LINE_MEMBER(clock2_w);
	TIMER_DEVICE_CALLBACK_MEMBER(snd_off_callback) { m_1863->set_output_gain(0, 0.00); }

	void play_2_io(address_map &map);
	void play_2_map(address_map &map);

	u8 m_resetcnt = 0U;
	u8 m_kbdrow = 0U;
	u8 m_segment[5]{};
	bool m_disp_sw = 0;
	u8 m_port06 = 0U;
	u8 m_old_solenoids[8]{};
	u8 m_soundlatch = 0U;
	bool m_snd_on = false;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_device<cosmac_device> m_maincpu;
	required_device<ttl7474_device> m_4013a;
	required_device<ttl7474_device> m_4013b;
	required_device<ripple_counter_device> m_4020;
	required_device<cdp1863_device> m_1863;
	required_device<timer_device> m_snd_off;
	required_ioport_array<8> m_io_keyboard;
	output_finder<55> m_digits;
	output_finder<56> m_io_outputs;   // 8 solenoids + 48 lamps
};

class zira_state : public play_2_state
{
public:
	zira_state(const machine_config &mconfig, device_type type, const char *tag)
		: play_2_state(mconfig, type, tag)
		, m_ay(*this, "ay")
	{ }

	void zira(machine_config &config);
	void init_zira();

private:
	void zira_sound_map(address_map &map);
	void sound_d_w(u8 data);
	void sound_g_w(u8 data);
	u8 psg_r();
	void psg_w(u8 data);
	u8 sound_in_r();
	u8 m_psg_latch = 0U;
	required_device<ay8910_device> m_ay;
};

void play_2_state::play_2_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("roms", 0);
	map(0x2000, 0x20ff).ram().share("nvram"); // pair of 5101, battery-backed
}

void play_2_state::play_2_io(address_map &map)
{
	map(0x01, 0x01).w(FUNC(play_2_state::port01_w)); // digits
	map(0x02, 0x02).w(FUNC(play_2_state::port02_w));
	map(0x03, 0x03).w(m_1863, FUNC(cdp1863_device::str_w));
	map(0x04, 0x04).r(FUNC(play_2_state::port04_r));
	map(0x05, 0x05).r(FUNC(play_2_state::port05_r));
	map(0x06, 0x06).w(FUNC(play_2_state::port06_w));
	map(0x07, 0x07).w(FUNC(play_2_state::port07_w));
}

void zira_state::zira_sound_map(address_map &map)
{
	map(0x000, 0x3ff).bankr("bank1");
}


static INPUT_PORTS_START( play_2 )
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP22") // outhole on Mad Race
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP58")

	PORT_START("X5") // 61-68
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP61")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP62")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP63")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP64")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP65")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP66")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP67")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("INP68")

	PORT_START("X6") // 01-08
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole") // outhole
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Setup")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Reset IOS") // reset button on the ios board

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Reset CPU") // reset button on main cpu EF4
INPUT_PORTS_END

void play_2_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_segment));
	save_item(NAME(m_resetcnt));
	save_item(NAME(m_disp_sw));
	save_item(NAME(m_soundlatch));
	//save_item(NAME(m_psg_latch));
	save_item(NAME(m_port06));
	save_item(NAME(m_old_solenoids));
	save_item(NAME(m_snd_on));
}

void play_2_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_resetcnt = 0;
	m_4013b->d_w(1);
	m_kbdrow = 0;
	m_disp_sw = 0;
	m_port06 = 0;
	m_snd_on = false;
	std::fill(std::begin(m_segment), std::end(m_segment), 0);
	std::fill(std::begin(m_old_solenoids), std::end(m_old_solenoids), 0);
	m_1863->oe_w(1);
	m_1863->set_output_gain(0, 0.00);
}

void play_2_state::port01_w(u8 data)
{
	m_kbdrow = data;
	if (m_kbdrow && m_disp_sw)
	{
		m_disp_sw = 0;
		for (u8 j = 0; j < 6; j++)
			if (BIT(m_kbdrow, j))
				for (u8 i = 0; i < 5; i++)
					m_digits[j*10 + i] = m_segment[i] & 0x7f;
	}
	if (BIT(data, 7))
	{
		m_1863->set_output_gain(0, 1.00);
		m_snd_on = true;
	}
	else
	{
		if (m_snd_on)
			m_snd_off->adjust(attotime::from_msec(200));
		m_snd_on = false;
	}
}

void play_2_state::port02_w(u8 data)
{
	m_segment[4] = m_segment[3];
	m_segment[3] = m_segment[2];
	m_segment[2] = m_segment[1];
	m_segment[1] = m_segment[0];
	m_segment[0] = data;
	m_disp_sw = 1;
}

u8 play_2_state::port04_r()
{
	if (m_kbdrow & 0x3f)
		for (u8 i = 0; i < 6; i++)
			if (BIT(m_kbdrow, i))
				return m_io_keyboard[i]->read();

	return 0;
}

u8 play_2_state::port05_r()
{
	return m_io_keyboard[6]->read();
}

// d0-2 choose a strobe 0-7 for lamps and solenoids
// d3 = data bit for solenoids (1 = on)
void play_2_state::port06_w(u8 data)
{
	m_port06 = data & 15;
}

// d0-3 data bit for each set of 8 lamps
// d4-6 sound command
// d7   indicate if solenoid (0 = solenoid)
void play_2_state::port07_w(u8 data)
{
	u8 t = 30;
	m_soundlatch = BIT(data, 4, 3); // Zira, Cerberus
	m_4013b->clear_w(0);
	m_4013b->clear_w(1);
	// Solenoids
	if (!BIT(data, 7) && BIT(m_port06, 3) && (m_old_solenoids[m_port06 & 7] == 0)) // enabled and on
	{
		if (m_port06 == 12)
			m_samples->start(0, 5); // outhole
		else
		if (m_port06 == 13)
			m_samples->start(0, 6); // knocker
		t = m_port06 & 7;
		m_old_solenoids[t] = 1;
	}

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = (t == i) ? 1 : 0; // turn off all solenoids except the current one, if any

	if (!BIT(data, 7) && !BIT(m_port06, 3))
		m_old_solenoids[m_port06] = 0;  // if current solenoid is off

	// Lamps
	t = m_port06 & 7;
	data &= 15;
	for (u8 i = 0; i < 16; i++)
		for (u8 j = 0; j < 3; j++)
			m_io_outputs[8+i*3+j] = (BIT(t, j) && (data == i)) ? 1 : 0;
}

READ_LINE_MEMBER( play_2_state::clear_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xff)
		m_resetcnt++;
	return (m_resetcnt < 0xf0) ? 0 : 1;
}

READ_LINE_MEMBER( play_2_state::ef1_r )
{
	return (!BIT(m_4020->count(), 10)); // inverted
}

READ_LINE_MEMBER( play_2_state::ef4_r )
{
	return BIT(m_io_keyboard[7]->read(), 0); // inverted test button - doesn't seem to do anything
}

void play_2_state::clockcnt_w(u16 data)
{
	if ((data & 0x3ff) == 0)
		m_4013b->preset_w(!BIT(data, 10)); // Q10 output
}

WRITE_LINE_MEMBER( play_2_state::clock2_w )
{
	m_4013b->clock_w(state);
	m_maincpu->ef3_w(state); // inverted
}

// *********** Zira Sound handlers ***************** (same as cidelsa.cpp)
void zira_state::sound_d_w(u8 data)
{
//    D3      2716 A10
	membank("bank1")->set_entry(BIT(data, 3));
}

void zira_state::sound_g_w(u8 data)
{
	switch (data)
	{
	case 0x01:
		m_ay->data_w(m_psg_latch);
		break;

	case 0x02:
		m_psg_latch = m_ay->data_r();
		break;

	case 0x03:
		m_ay->address_w(m_psg_latch);
		break;
	}
}

u8 zira_state::sound_in_r()
{
	return ~m_soundlatch & 7;
}

u8 zira_state::psg_r()
{
	return m_psg_latch;
}

void zira_state::psg_w(u8 data)
{
	m_psg_latch = data;
}

// **************** Machine *****************************

void play_2_state::play_2(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 2.95_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &play_2_state::play_2_map);
	m_maincpu->set_addrmap(AS_IO, &play_2_state::play_2_io);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(play_2_state::clear_r));
	m_maincpu->ef1_cb().set(FUNC(play_2_state::ef1_r));
	m_maincpu->ef4_cb().set(FUNC(play_2_state::ef4_r));
	m_maincpu->q_cb().set(m_4013a, FUNC(ttl7474_device::clear_w)).invert(); // actually active high on 4013
	m_maincpu->tpb_cb().set(m_4013a, FUNC(ttl7474_device::clock_w));
	m_maincpu->tpb_cb().append(m_4020, FUNC(ripple_counter_device::clock_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_play_2);

	CLOCK(config, "xpoint", 60).signal_handler().set(FUNC(play_2_state::clock2_w)); // crossing-point detector

	// This is actually a 4013 chip (has 2 RS flipflops)
	TTL7474(config, m_4013a, 0);
	m_4013a->comp_output_cb().set(m_4013a, FUNC(ttl7474_device::d_w));
	m_4013a->output_cb().set(m_4020, FUNC(ripple_counter_device::reset_w)); // TODO: also CKD for display

	TTL7474(config, m_4013b, 0);
	m_4013b->output_cb().set(m_maincpu, FUNC(cosmac_device::ef2_w));
	m_4013b->comp_output_cb().set(m_maincpu, FUNC(cosmac_device::int_w)).invert(); // int is reversed in mame

	RIPPLE_COUNTER(config, m_4020);
	m_4020->set_stages(14); // only Q10 is actually used
	m_4020->count_out_cb().set(FUNC(play_2_state::clockcnt_w));

	/* Sound */
	genpin_audio(config);

	SPEAKER(config, "mono").front_center();
	CDP1863(config, m_1863, 0);
	m_1863->set_clock2(2.95_MHz_XTAL / 8);
	m_1863->add_route(ALL_OUTPUTS, "mono", 0.75);
	TIMER(config, m_snd_off).configure_generic(FUNC(play_2_state::snd_off_callback));
}

void zira_state::zira(machine_config &config)
{
	play_2(config);
	cop402_cpu_device &cop402(COP402(config, "cop402", 2_MHz_XTAL));
	cop402.set_addrmap(AS_PROGRAM, &zira_state::zira_sound_map);
	cop402.set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false);
	cop402.write_d().set(FUNC(zira_state::sound_d_w));
	cop402.write_g().set(FUNC(zira_state::sound_g_w));
	cop402.read_l().set(FUNC(zira_state::psg_r));
	cop402.write_l().set(FUNC(zira_state::psg_w));
	cop402.read_in().set(FUNC(zira_state::sound_in_r));

	AY8910(config, m_ay, 2_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void zira_state::init_zira()
{
	/* setup COP402 memory banking */
	membank("bank1")->configure_entries(0, 2, memregion("cop402")->base(), 0x400);
	membank("bank1")->set_entry(0);
}

/* PLAYMATIC MPU-2 ALTERNATE ROMS =======================================================================

This is a list of known alternate roms. Nothing has been tested.

Antar
CI-08  2708  EFO A1 02   0724
CI-09  2708  EFO B1 02   0776
CI-10  2708  EFO C1 02   0389   mala(bad)?
CI-10  2708  EFO CI 02   0A18   buena(good)?
CI-11  2708  EFO D1 02   F5BA
Probably overdumps
ROM_LOAD( "antar08.bin",  0x0000, 0x0800, CRC(5f5a41cf) SHA1(136ac4d265849b196fda5fd19014b53f7074a84f) )
ROM_LOAD( "antar09.bin",  0x0000, 0x0800, CRC(23c65808) SHA1(a3a68c5e12ae3a5301b4e9849a48f31cd61e1c71) )
ROM_LOAD( "antar10.bin",  0x0000, 0x0800, CRC(e0a00191) SHA1(73d1fcdfa97cd5f512eb7a939dab4f3aa6d29cf3) )
ROM_LOAD( "antar10_.bin", 0x0000, 0x0800, CRC(f013ab1d) SHA1(ce3eca93c73273ca6ab58d00d9baf927c17dc7b3) )
ROM_LOAD( "antar11.bin",  0x0000, 0x0800, CRC(a5f8ab17) SHA1(65a9dac981211e8c7510b0c21e9e83a3e90e80f1) )

Evil Fight
Probably overdumps
ROM_LOAD( "evfg08.bin",   0x0000, 0x0800, CRC(a5e1a267) SHA1(ed472fc035a8f36ffe55cb6257fd4374c7e1d594) )
ROM_LOAD( "evfg09.bin",   0x0000, 0x0800, CRC(7eff9133) SHA1(eb42a1499d6db455df13f7b3bb2e2eb13105ed00) )
ROM_LOAD( "evfg10.bin",   0x0000, 0x0800, CRC(adb0d1f9) SHA1(e40bb452b8e05a8a416f8059ed2e5319c5962a01) )
ROM_LOAD( "evfg11.bin",   0x0000, 0x0800, CRC(3faa127a) SHA1(332f55ab7affa8c22bef3b990de69679a2dee5a7) )

Zira
ROM_LOAD( "zira.snd",     0x0000, 0x0400, CRC(c8a54854) SHA1(6c0367dcb2a11f0478c44b4e2115c1cb1e8052f3) )
ROM_LOAD( "ziraalt.snd",  0x0000, 0x0800, CRC(d2a26fcf) SHA1(d212c2cdaade6c9da0e7f88a254c8f7f27eabb19) )

*/
/*-------------------------------------------------------------------
/ Antar (11/79)
/-------------------------------------------------------------------*/
ROM_START(antar)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("antar08.8",      0x0000, 0x0400, CRC(f6207f77) SHA1(f68ce967c6189457bd0ce8638e9c477f16e65763))
	ROM_LOAD("antar09.9",      0x0400, 0x0400, CRC(2c954f1a) SHA1(fa83a5f1c269ea28d4eeff181f493cbb4dc9bc47))
	ROM_LOAD("antar10.10",     0x0800, 0x0400, CRC(a6ce5667) SHA1(85ecd4fce94dc419e4c210262f867310b0889cd3))
	ROM_LOAD("antar11.11",     0x0c00, 0x0400, CRC(6474b17f) SHA1(e4325ceff820393b06eb2e8e4a85412b0d01a385))
ROM_END

ROM_START(antar2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("antar08.8",      0x0000, 0x0400, CRC(f6207f77) SHA1(f68ce967c6189457bd0ce8638e9c477f16e65763))
	ROM_LOAD("antar09.9",      0x0400, 0x0400, CRC(2c954f1a) SHA1(fa83a5f1c269ea28d4eeff181f493cbb4dc9bc47))
	ROM_LOAD("antar10a.10",    0x0800, 0x0400, CRC(520eb401) SHA1(1d5e3f829a7e7f38c7c519c488e6b7e1a4d34321))
	ROM_LOAD("antar11a.11",    0x0c00, 0x0400, CRC(17ad38bf) SHA1(e2c9472ed8fbe9d5965a5c79515a1b7ea9edaa79))
ROM_END

/*-------------------------------------------------------------------
/ Storm (??/79)
/-------------------------------------------------------------------*/
ROM_START(storm)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("a-1.8",          0x0000, 0x0400, CRC(12e37664) SHA1(d7095975cd9d4445fd1f4cd711992c7367deae89))
	ROM_LOAD("b-1.9",          0x0400, 0x0400, CRC(3ac3cea3) SHA1(c6197911d25661cb647ea606eee5f3f1bd9b4ba2))
	ROM_LOAD("c-1.10",         0x0800, 0x0400, CRC(8bedf1ea) SHA1(7633ebf8a65e3fc7afa21d50aaa441f87a86efd3))
	ROM_LOAD("d-1.11",         0x0c00, 0x0400, CRC(f717ef3e) SHA1(cd5126360471c06539e445fecbf2f0ddeb1b156c))
ROM_END

/*-------------------------------------------------------------------
/ Evil Fight (03/80)
/-------------------------------------------------------------------*/
ROM_START(evlfight)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("evfg08.8",       0x0000, 0x0400, CRC(2cc2e79a) SHA1(17440512c419b3bb2012539666a5f052f3cd8c1d))
	ROM_LOAD("evfg09.9",       0x0400, 0x0400, CRC(5232dc4c) SHA1(6f95a578e9f09688e6ce8b0a622bcee887936c82))
	ROM_LOAD("evfg10.10",      0x0800, 0x0400, CRC(de2f754d) SHA1(0287a9975095bcbf03ddb2b374ff25c080c8020f))
	ROM_LOAD("evfg11.11",      0x0c00, 0x0400, CRC(5eb8ac02) SHA1(31c80e74a4272becf7014aa96eaf7de555e26cd6))
ROM_END

/*-------------------------------------------------------------------
/ Attack (10/80)
/-------------------------------------------------------------------*/
ROM_START(attack)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("attack8.8",      0x0000, 0x0400, CRC(a5204b58) SHA1(afb4b81720f8d56e88f47fc842b23313824a1085))
	ROM_LOAD("attack9.9",      0x0400, 0x0400, CRC(bbd086b4) SHA1(6fc94b94beea482d8c8f5b3c69d3f218e2b2dfc4))
	ROM_LOAD("attack10.10",    0x0800, 0x0400, CRC(764925e4) SHA1(2f207ef87786d27d0d856c5816a570a59d89b718))
	ROM_LOAD("attack11.11",    0x0c00, 0x0400, CRC(972157b4) SHA1(23c90f23a34b34acfe445496a133b6022a749ccc))
ROM_END

/*-------------------------------------------------------------------
/ Black Fever (12/80)
/-------------------------------------------------------------------*/
ROM_START(blkfever)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("blackf8.8",      0x0000, 0x0400, CRC(916b8ed8) SHA1(ddc7e09b68e3e1a033af5dc5ec32ab5b0922a833))
	ROM_LOAD("blackf9.9",      0x0400, 0x0400, CRC(ecb72fdc) SHA1(d3598031b7170fab39727b3402b7053d4f9e1ca7))
	ROM_LOAD("blackf10.10",    0x0800, 0x0400, CRC(b3fae788) SHA1(e14e09cc7da1098abf2f60f26a8ec507e123ff7c))
	ROM_LOAD("blackf11.11",    0x0c00, 0x0400, CRC(5a97c1b4) SHA1(b9d7eb0dd55ef6d959c0fab48f710e4b1c8d8003))
ROM_END

/*-------------------------------------------------------------------
/ Zira (??/81)
/-------------------------------------------------------------------*/
ROM_START(zira)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("zira_u8.8",      0x0000, 0x0800, CRC(53f8bf17) SHA1(5eb74f27bc65374a85dd44bbc8f6142488c226a2))
	ROM_LOAD("zira_u9.9",      0x0800, 0x0800, CRC(d50a2419) SHA1(81b157f579a433389506817b1b6e02afaa2cf0d5))

	ROM_REGION(0x800, "cop402", 0) // according to the schematic this is a 2716 with a size of 0x800; according to PinMAME it contains the same code twice
	ROM_LOAD("zira.snd",       0x0000, 0x0800, CRC(008cb743) SHA1(8e9677f08189638d669b265bb6943275a08ec8b4))
ROM_END

/*-------------------------------------------------------------------
/ Cerberus (03/82)
/-------------------------------------------------------------------*/
ROM_START(cerberup)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("cerb8.8",        0x0000, 0x0800, CRC(021d0452) SHA1(496010e6892311b1cabcdac62296cd6aa0782c5d))
	ROM_LOAD("cerb9.9",        0x0800, 0x0800, CRC(0fd41156) SHA1(95d1bf42c82f480825e3d907ae3c87b5f994fd2a))
	ROM_LOAD("cerb10.10",      0x1000, 0x0800, CRC(785602e0) SHA1(f38df3156cd14ab21752dbc849c654802079eb33))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("cerb.snd",       0x0000, 0x2000, CRC(8af53a23) SHA1(a80b57576a1eb1b4544b718b9abba100531e3942))
ROM_END

/*-------------------------------------------------------------------
/ Mad Race (??/85?)
/-------------------------------------------------------------------*/
ROM_START(madrace)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("madrace.2a0.8",  0x0000, 0x0800, CRC(ab487c79) SHA1(a5df29b2af4c9d94d8bf54c5c91d1e9b5ca4d065))
	ROM_LOAD("madrace.2b0.9",  0x0800, 0x0800, CRC(dcb54b39) SHA1(8e2ca7180f5ea3a28feb34b01f3387b523dbfa3b))
	ROM_LOAD("madrace.2c0.10", 0x1000, 0x0800, CRC(b24ea245) SHA1(3f868ccbc4bfb77c40c4cc05dcd8eeca85ecd76f))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("madrace1.snd",   0x0000, 0x2000, CRC(49e956a5) SHA1(8790cc27a0fda7b8e07bee65109874140b4018a2))
	ROM_LOAD("madrace2.snd",   0x2000, 0x0800, CRC(c19283d3) SHA1(42f9770c46030ef20a80cc94fdbe6548772aa525))
ROM_END

} // Anonymous namespace

GAME(1979, antar,     0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic",      "Antar (set 1)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME(1979, antar2,    antar, play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic",      "Antar (set 2)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME(1979, storm,     0,     play_2, play_2, play_2_state, empty_init, ROT0, "SegaSA / Sonic", "Storm",              MACHINE_IS_SKELETON_MECHANICAL )
GAME(1980, evlfight,  0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic",      "Evil Fight",         MACHINE_IS_SKELETON_MECHANICAL )
GAME(1980, attack,    0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic",      "Attack",             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1980, blkfever,  0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic",      "Black Fever",        MACHINE_IS_SKELETON_MECHANICAL )
GAME(1982, cerberup,  0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic",      "Cerberus (Pinball)", MACHINE_IS_SKELETON_MECHANICAL )
GAME(1985, madrace,   0,     play_2, play_2, play_2_state, empty_init, ROT0, "Playmatic",      "Mad Race",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(1980, zira,      0,     zira,   play_2, zira_state,   init_zira,  ROT0, "Playmatic",      "Zira",               MACHINE_IS_SKELETON_MECHANICAL )
