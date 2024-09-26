// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************************
PINBALL
Skeleton driver for Stargame pinballs (2 x Z80, Z80CTC, DAC, AY8910, MEA8000).
Hardware listing and ROM definitions from PinMAME.
Machines: Space Ship, White Force.
These 2 machines have similar but not identical hardware, and there's no manuals for White Force.
The biggest changes are in the switch matrix, and the display data is inverted too.

Status:
- spcship: working (At start of each ball, press Z to enable the playfield)
- whtforce: shows an error at start

Todo:
- investigate and fix whtforce error
- Add mechanical sounds


*****************************************************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/z80/z80.h"
#include "machine/7474.h"
#include "machine/74259.h"
#include "machine/clock.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/mea8000.h"
#include "speaker.h"
#include "stargame.lh"

namespace {

class stargame_state : public genpin_class
{
public:
	stargame_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_ctc(*this, "ctc")
		, m_7a(*this, "7a")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void stargame(machine_config &config);
	void init_0() { m_game = 0; }
	void init_1() { m_game = 1; }

private:
	void reset2_w(int state) { m_audiocpu->reset(); }
	void ckdis_w(u8);
	void cdig_w(u8);
	void cdriv_w(offs_t, u8);
	void csw2_w(u8);
	u8 csw1_r();
	u8 csw2_r();

	void audiocpu_io(address_map &map) ATTR_COLD;
	void audiocpu_map(address_map &map) ATTR_COLD;
	void maincpu_io(address_map &map) ATTR_COLD;
	void maincpu_map(address_map &map) ATTR_COLD;
	u8 m_segment[5]{};
	u8 m_game = 0U;
	u8 m_row = 0U;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<z80_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<z80ctc_device> m_ctc;
	required_device<ttl7474_device> m_7a;
	required_ioport_array<14> m_io_keyboard;
	output_finder<48> m_digits;
	output_finder<48> m_io_outputs;   // lamps and solenoids mixed together
};

void stargame_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0x87ff).mirror(0x7800).ram().share("nvram");
}

void stargame_state::maincpu_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x09).w("soundlatch", FUNC(generic_latch_8_device::write)); // CSON - command to the sound cpu - NMI to sound cpu
	map(0x10, 0x1f).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x20, 0x2f).r(FUNC(stargame_state::csw2_r)).w(FUNC(stargame_state::csw2_w)); // CSW2 - input lines (spcship), output lines (whtforce)
	map(0x30, 0x3f).r(FUNC(stargame_state::csw1_r)); // CSW1 - input lines
	map(0x40, 0x4f).w(FUNC(stargame_state::cdig_w)); // CDIG - score display
	map(0x50, 0x57).mirror(0x08).w(FUNC(stargame_state::cdriv_w)); // CDRIV - lamps and solenoids
	map(0x60, 0x67).mirror(0x08).w("mainlatch", FUNC(ls259_device::write_d0)); // CPOR
	map(0x68, 0x68).mirror(0x06).w(FUNC(stargame_state::ckdis_w)); // 68=CKDIS (sch has 68 and 69 the wrong way around)
	map(0x69, 0x69).mirror(0x06); // 69=CKPRI (not used?)
	map(0x70, 0x73).mirror(0x0c).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

void stargame_state::audiocpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4001).mirror(0x3ffe).rw("mea8000", FUNC(mea8000_device::read), FUNC(mea8000_device::write));
	map(0x8000, 0x87ff).mirror(0x3800).ram();
	map(0xc000, 0xdfff).lr8(NAME([this] () { m_7a->clear_w(0); m_7a->clear_w(1); return 0; })); // AINT - turn off interrupt of the audiocpu
	map(0xe000, 0xffff).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w)); // COMAND - acknowledge NMI and read the sound command
}

void stargame_state::audiocpu_io(address_map &map)
{
	map.global_mask(0x01);
	map(0x00, 0x01).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
}

static INPUT_PORTS_START( spcship )
	PORT_START("X0") // COL1, DAT1-8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP11")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP12")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP13")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP14")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP15")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP16")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP17")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Ball Enable")

	PORT_START("X1") // COL2 etc
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP21")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP22")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP23")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP24")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP25")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP26")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP27")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP28")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP31")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP32")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP33")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP35")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP36")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP37")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP38")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP48")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP51")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP52")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP53")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP54")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP55")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP56")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("INP57")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("INP58")

	PORT_START("X5")
	PORT_START("X6")
	PORT_START("X7") // whtforce only
	PORT_START("X8") // COL1-7, DAT9
	PORT_START("X9") // COL1, DAP1-3

	PORT_START("X10") // COL2 etc
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("X11") // COL3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Setup") // AJUSTE (setup)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("X12") // COL4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Avance") // avance? does nothing
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )

	PORT_START("X13") // COL5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Test")  // Press at the red screen
INPUT_PORTS_END

// None of these inputs have been determined
static INPUT_PORTS_START( whtforce )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP11")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP12")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP13")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP14")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP15")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP16")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP17")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP18")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP21")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP22")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP23")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP24")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP25")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP26")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP27")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP28")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP31")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP32")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP33")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP34")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP35")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP36")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP37")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP38")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP48")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP51")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP52")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP53")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP54")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP55")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP56")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("INP57")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("INP58")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP61")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP62")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP63")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP64")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP65")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP66")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP67")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP68")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP71")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP72")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP73")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP74")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP75")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP76")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP77")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP78")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP81")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP82")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP83")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP84")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP85")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP86")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("INP87")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP88") // Error dato1

	PORT_START("X8")
	PORT_START("X9")
	PORT_START("X10")
	PORT_START("X11")
	PORT_START("X12")
	PORT_START("X13")
INPUT_PORTS_END

// **** SWITCHES / CONTACTORS ****
u8 stargame_state::csw1_r()
{
	// 8 rows for whtforce; 7 rows for spcship
	if (m_row < 8)
		return m_io_keyboard[m_row]->read();
	else
		return 0xff;
}

// spcship only
u8 stargame_state::csw2_r()
{
	u8 data = 0xf0;  // guess
	// d7: TEST interlocked with RJUEGO via RL2, not used?
	// d6: connects to DAPRI port 61/69, never used?
	// d5: P1 (not connected to anything?)
	// d4: P0 (not connected to anything?)
	// d3: DAT9 from contactors (none used)
	// d2-0: DAP1,2,3 from front door area
	if (m_row < 5)
		data |= (m_io_keyboard[m_row+9]->read() | (BIT(m_io_keyboard[8]->read(), m_row) << 3));
	else
	if (m_row < 7)
		data |= (BIT(m_io_keyboard[8]->read(), m_row) << 3);
	else
		data |= 0xf;

	return data;
}

// whtforce only
void stargame_state::csw2_w(u8 data)
{
	m_row = 8;
	if (data)
		for (u8 i = 0; i < 8; i++)
			if (BIT(data, i))
				m_row = i;
}

// **** DISPLAY ****
void stargame_state::cdig_w(u8 data)
{
	u8 row = 8;
	if (data)
	{
		for (u8 i = 0; i < 7; i++)
			if (BIT(data, i))
				row = i;

		if (row < 7)
		{
			m_digits[row] = m_segment[0];
			m_digits[10+row] = m_segment[1];
			m_digits[20+row] = m_segment[2];
			m_digits[30+row] = m_segment[3];
			m_digits[40+row] = m_segment[4];
		}
	}
	if (m_game == 0)
		m_row = row;
}

// Serial data for the display segments
// The data is on bit 0 which also is mirrored to 0x60, which we ignore
void stargame_state::ckdis_w(u8 data)
{
	if (m_game == 1)
		data ^= 0xff;
	m_segment[4] = (m_segment[4] << 1) | BIT(m_segment[3], 7);
	m_segment[3] = (m_segment[3] << 1) | BIT(m_segment[2], 7);
	m_segment[2] = (m_segment[2] << 1) | BIT(m_segment[1], 7);
	m_segment[1] = (m_segment[1] << 1) | BIT(m_segment[0], 7);
	m_segment[0] = (m_segment[0] << 1) | BIT(data, 0);
}

// **** Lamps and Solenoids (d0-5 used)
void stargame_state::cdriv_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 6; i++)
		m_io_outputs[offset*6+i] = BIT(data, i);
}

// **** MACHINE ****
void stargame_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_segment));
	save_item(NAME(m_game));
}

void stargame_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_7a->preset_w(1);
	m_7a->d_w(1);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


void stargame_state::stargame(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 15000000 / 4); // freq=CK4; ic=7D
	m_maincpu->set_addrmap(AS_PROGRAM, &stargame_state::maincpu_map);
	m_maincpu->set_addrmap(AS_IO, &stargame_state::maincpu_io);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80(config, m_audiocpu, 15000000 / 3); // freq=CK6; ic=6E
	m_audiocpu->set_addrmap(AS_PROGRAM, &stargame_state::audiocpu_map);
	m_audiocpu->set_addrmap(AS_IO, &stargame_state::audiocpu_io);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	config.set_default_layout(layout_stargame);

	clock_device &ctc_clock(CLOCK(config, "ctc_clock", 50));
	ctc_clock.signal_handler().set(m_ctc, FUNC(z80ctc_device::trg2));
	ctc_clock.signal_handler().append(m_ctc, FUNC(z80ctc_device::trg3)).invert();

	Z80CTC(config, m_ctc, 15000000 / 4); // ic=3D
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_7a, FUNC(ttl7474_device::clock_w));

	/* sound hardware */
	genpin_audio(config);
	SPEAKER(config, "measnd").front_center();
	MEA8000(config, "mea8000", 15000000 / 4).add_route(ALL_OUTPUTS, "measnd", 2.0); // CK4 freq; ic=2E
	SPEAKER(config, "aysnd").front_center();
	ay8910_device &ay0(AY8910(config, "ay", 15000000 / 8)); // CK2 freq; ic=1E
	ay0.add_route(ALL_OUTPUTS, "aysnd", 1.5);
	ay0.set_resistors_load(20000, 20000, 20000);
	ay0.port_a_write_callback().set("dac", FUNC(dac_byte_interface::write));
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "aysnd", 0.5); // bunch of resistors

	ls259_device &mainlatch(LS259(config, "mainlatch")); // ic=6A
	mainlatch.q_out_cb<0>().set_nop(); // DADIS, handled by 0x68
	mainlatch.q_out_cb<1>().set_nop(); // DAPRI, handled by 0x69
	mainlatch.q_out_cb<2>().set_nop(); // RJUEGO
	mainlatch.q_out_cb<3>().set_nop(); // RFLIPPER
	mainlatch.q_out_cb<4>().set_nop(); // to AUXILLIAR socket
	mainlatch.q_out_cb<5>().set_nop(); // RFDIS
	mainlatch.q_out_cb<6>().set(FUNC(stargame_state::reset2_w)); // SRESET
	mainlatch.q_out_cb<7>().set_nop(); // MAKRES

	TTL7474(config, m_7a, 0);
	m_7a->comp_output_cb().set_inputline(m_audiocpu, INPUT_LINE_IRQ0).invert();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	WATCHDOG_TIMER(config, "watchdog");
}

ROM_START(spcship)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("sss-1g.6d", 0x0000, 0x4000, CRC(119a3064) SHA1(d915ecf44279a9e16a50a723eb9523afec1fb380))
	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sss-1a0.5e", 0x0000, 0x4000, CRC(eae78e63) SHA1(9fa3587ae3ee6f674bb16102680e70069e9d275e))
ROM_END


ROM_START(whtforce)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("m5l.6d", 0x0000, 0x4000, CRC(22495322) SHA1(b34a34dec875f215d566d18a5e877b9185a22ab7))
	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound.5e", 0x0000, 0x4000, CRC(4b2a1580) SHA1(62133fd186b1aab4f5aecfbff8151ba416328021))
ROM_END

} // Anonymous namespace

GAME( 1986, spcship,  0, stargame, spcship,  stargame_state, init_0, ROT0, "Stargame", "Space Ship (Pinball)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, whtforce, 0, stargame, whtforce, stargame_state, init_1, ROT0, "Stargame", "White Force",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
