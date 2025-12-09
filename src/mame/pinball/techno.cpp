// license:BSD-3-Clause
// copyright-holders:Robbbert
/**********************************************************************************

PINBALL
Technoplay "2-2C 8008 LS" (68000 CPU). Company also known as Tecnoplay. They still
exist, mainly as importer and reseller of amusement machines made by others.
Schematic and PinMAME used as references
Machines by this manufacturer: Devil King, Hi-Ball, Scramble, Space Team, X Force.
Scramble uses Zaccaria-2 system.

Status:
- X-Force displays correctly, you can coin up and start a game, then no response.
  Test mode works.

ToDo:
- Once you press the credit button, nothing responds (game requires 4 balls)
- Sliding display is too fast to read (much better if cpu xtal changed to 4MHz).
  DTACK needs to be hooked up but it is unsupported. Inserting waitstates doesn't
  work either.
- Mechanical sounds
- Unemulated TKY2016-A and TKY2016-B pair of audio processor chips.
  PinMAME thinks it might be a Y8950, but I'm not so sure. It's hooked up for now.
- Schematic shows sound rom banking, but no machine has those roms, so not coded.
- xforce: sound rom is missing
- spcteam: Bad display - can't tell if keys are doing anything.

***********************************************************************************/


#include "emu.h"
#include "genpin.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms7000/tms7000.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "speaker.h"
#include "techno.lh"

namespace {

class techno_state : public genpin_class
{
public:
	techno_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_dac(*this, "dac")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void techno(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void audio_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(set_irq1);
	TIMER_CALLBACK_MEMBER(clear_irq1);

	u16 key_r();
	u16 rtrg_r();
	u16 sound_r();
	void disp1_w(u16 data);
	void disp2_w(u16 data);
	void lamp1_w(u16 data);
	void lamp2_w(u16 data);
	void setout_w(u16 data);
	void sol1_w(u16 data);
	void sol2_w(u16 data);
	void sound_w(u16 data);

	u8 pa_r();
	void pb_w(u8);

	required_device<m68000_device> m_maincpu;
	required_device<tms7000_device> m_audiocpu;
	required_device<dac_byte_interface> m_dac;
	required_ioport_array<8> m_io_keyboard;
	output_finder<48> m_digits;
	output_finder<96> m_io_outputs;   // 64 lamps + 32 solenoids

	emu_timer *m_irq_set_timer = 0;
	emu_timer *m_irq_advance_timer = 0;

	bool m_digwait = false;
	u8 m_keyrow = 0U;
	u16 m_digit = 0U;
	u8 m_vector = 0U;
	u8 m_snd_cmd = 0U;
	bool m_snd_ack = false;
	u32 m_last_solenoid = 0U;
};


void techno_state::mem_map(address_map &map)
{
	map.global_mask(0x1ffff);
	map(0x00000, 0x03fff).rom();
	map(0x04000, 0x04fff).ram().share("nvram"); // battery backed-up
	map(0x06000, 0x0ffff).rom();
	map(0x14000, 0x147ff).rw(FUNC(techno_state::key_r), FUNC(techno_state::lamp1_w));
	map(0x14800, 0x14fff).w(FUNC(techno_state::lamp2_w));
	map(0x15000, 0x157ff).rw(FUNC(techno_state::rtrg_r), FUNC(techno_state::sol1_w));
	map(0x15800, 0x15fff).rw(FUNC(techno_state::sound_r), FUNC(techno_state::sol2_w)); // sch has sound_r @14800 by mistake
	map(0x16000, 0x167ff).w(FUNC(techno_state::sound_w));
	map(0x16800, 0x16fff).w(FUNC(techno_state::disp1_w));
	map(0x17000, 0x177ff).w(FUNC(techno_state::disp2_w));
	map(0x17800, 0x17fff).w(FUNC(techno_state::setout_w));
}

void techno_state::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_maincpu, FUNC(m68000_base_device::autovectors_map));
	map(0xfffff2, 0xfffff3).lr16(NAME([this] () -> u16 { return m_vector; }));
}

void techno_state::audio_map(address_map &map)
{
	map(0x3000, 0x3000).r("ym1", FUNC(y8950_device::read));   // TKY2016
	map(0x5800, 0x5801).w("ym1", FUNC(y8950_device::write));   // TKY2016
	map(0x6800, 0x6800).w(m_dac, FUNC(dac_byte_interface::write));   // DAC0808
	map(0x7000, 0x7000).nopw(); //w(FUNC(techno_state::led_w));  // LED (bit 0; rest unused)
	map(0x4000, 0xbfff).rom().region("audiocpu", 0x4000); // ic8,9,10,11 - 4x 32k roms bankswitched (not used by any machine)
	map(0xc000, 0xffff).rom().region("audiocpu", 0); // ic12, 1st half ignored, 2nd half used
}

void techno_state::disp1_w(u16 data)
{
	m_digits[m_digit] = bitswap<16>(data, 12, 10, 8, 14, 13, 9, 11, 15, 7, 6, 5, 4, 3, 2, 1, 0);
	// need to hold DTACK here to momentarily slow the main cpu to half speed so that the displays can be read
}

void techno_state::disp2_w(u16 data)
{
	m_digits[m_digit+30] = bitswap<16>(data, 12, 10, 8, 14, 13, 9, 11, 15, 7, 6, 5, 4, 3, 2, 1, 0);
	// need to hold DTACK here to momentarily slow the main cpu to half speed so that the displays can be read
}

void techno_state::sound_w(u16 data)
{
/*
d0..d7 : to sound board
d8     : strobe to display board
d9     : reset (unknown purpose)
d10    : data clock to display board
d11-d15: AUX outputs
*/

	m_snd_cmd = data & 0xff;
	m_audiocpu->set_input_line(TMS7000_INT3_LINE, HOLD_LINE);

// this code derived from PinMAME
	if (m_digwait)
		m_digit = (m_digit+1) % 16;

	if (BIT(data, 10))
	{
		m_digwait = 1;
		m_digit = 0;
	}
}

// lamp/keymatrix one line high
void techno_state::lamp1_w(u16 data)
{
// Work out key row
	for (int i = 0; i < 8; i++)
		if (BIT(data, i))
			m_keyrow = i;
}

// lamps - up to 8 lines high
// sch shows bits 0-7 going to aux outputs, but they are the same as bits 8-15,
//  so probably do nothing or not exist.
void techno_state::lamp2_w(u16 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[32+m_keyrow*8+i] = BIT(data ,i+8);
}

// solenoids
void techno_state::sol1_w(u16 data)
{
	for (u8 i = 0; i < 16; i++)
		if (BIT(data, i) != BIT(m_last_solenoid, i))
			m_io_outputs[i] = BIT(data ,i);
	m_last_solenoid = (m_last_solenoid & 0xffff0000) | data;
}

// more solenoids
void techno_state::sol2_w(u16 data)
{
	for (u8 i = 0; i < 16; i++)
		if (BIT(data, i) != BIT(m_last_solenoid, i+16))
			m_io_outputs[i+16] = BIT(data ,i);

	if (BIT(data, 6) && !BIT(m_last_solenoid, 22))
		m_samples->start(0, 5);  // outhole

	m_last_solenoid = (m_last_solenoid & 0xffff) | (data << 16);
}

// blanking circuit
void techno_state::setout_w(u16 data)
{
	// data not used
}

// inputs
u16 techno_state::key_r()
{
	u8 i = m_io_keyboard[m_keyrow]->read();
	return i | (i << 8);
}

// blanking circuit
u16 techno_state::rtrg_r()
{
	// data not used
	return 0xffff;
}

// feedback from sound board, and some AUX inputs
u16 techno_state::sound_r()
{
	return 0xfe | u16(m_snd_ack);
}

// These switch names are for xforce; it's the only manual we have
static INPUT_PORTS_START( techno )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Letter select+")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Setup")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Tilt2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Test-")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Test+")
	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Fix top target right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Horizontal rail right")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Mini Post")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Letter select-")
	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Inner Canal Left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Exit Canal Right")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Ball 1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Ball 2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Ball 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("Ball 4")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Out Hole") PORT_CODE(KEYCODE_X)
	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Top Bumper")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Bottom Bumper")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Top Right Kicker")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Right Kicker")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("Left Kicker")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("Exit Canal Left")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Inner Canal Right")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Fix left target bottom")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Left rollover")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Barrier 1 Target")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("Barrier 2 Target")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Barrier 3 Target")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Left Bumper")
	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Fix right target top")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Fix right target middle-top")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Spinning Target")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Fixed contact")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Special Target")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Fix left target top")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Fix left target centre")
	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Fix top left target left")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Ball 1 Bridge")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Ball 2 Bridge")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Ball 3 Bridge")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_NAME("Ball 4 Bridge")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("Fix right target middle-bottom")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Fix right target bottom")
	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("Fix top target middle-right")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Fix top target middle-left")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Fix top target Left")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Horizontal Rail Left")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Chopper Exit")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Chopper Entry")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Fix top left target right")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Fix top left target middle")
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(techno_state::clear_irq1)
{
	// vectors change per int: 88-8F, 98-9F)
	if ((m_vector & 7) == 7)
		m_vector = (m_vector ^ 0x10) & 0x97;
	m_vector++;

	// schematics show a 74HC74 cleared only upon IRQ acknowledgment or reset, but this is clearly incorrect for xforce
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(techno_state::set_irq1)
{
	m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	m_irq_advance_timer->adjust(attotime::from_hz(XTAL(8'000'000) / 32));
}

// E04C
u8 techno_state::pa_r()
{
	return m_snd_cmd;
}

void techno_state::pb_w(u8 data)
{
	// d0: enable TKY2016
	// d1: ack back to maincpu
	m_snd_ack = BIT(data, 1);
	// d2,3 : bank
	// d4: address/data - internal
	// d5: r/w - internal
	// d6: ena - internal
	// d7: ck.oot - not used
	// Banking of sound roms 4000-BFFF goes here, however no machine uses it
	// m_bank->set_entry(BIT(data, 2, 2));
}

void techno_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_digwait));
	save_item(NAME(m_digit));
	save_item(NAME(m_keyrow));
	save_item(NAME(m_vector));
	save_item(NAME(m_snd_cmd));
	save_item(NAME(m_snd_ack));
	save_item(NAME(m_last_solenoid));

	m_irq_set_timer = timer_alloc(FUNC(techno_state::set_irq1), this);
	m_irq_advance_timer = timer_alloc(FUNC(techno_state::clear_irq1), this);
}

void techno_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_vector = 0x88;
	m_digit = 0;
	m_last_solenoid = 0;

	attotime freq = attotime::from_hz(XTAL(8'000'000) / 256); // 31250Hz
	m_irq_set_timer->adjust(freq, 0, freq);
	m_irq_advance_timer->adjust(attotime::never);
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

void techno_state::techno(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &techno_state::mem_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &techno_state::cpu_space_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TMS7000(config, m_audiocpu, 4_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &techno_state::audio_map);
	m_audiocpu->in_porta().set(FUNC(techno_state::pa_r));
	m_audiocpu->out_portb().set(FUNC(techno_state::pb_w));

	/* Video */
	config.set_default_layout(layout_techno);

	// Sound
	genpin_audio(config);
	SPEAKER(config, "speaker", 2).front();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 1); // DAC0808
	Y8950(config, "ym1", 3580000).add_route(ALL_OUTPUTS, "speaker", 0.5, 0); // TKY2016 - no crystal, just a random oscillator, sch says 3.58MHz
}

ROM_START(xforce)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE("ic15", 0x0001, 0x8000, CRC(fb8d2853) SHA1(0b0004abfe32edfd3ac15d66f90695d264c97eba))
	ROM_LOAD16_BYTE("ic17", 0x0000, 0x8000, CRC(122ef649) SHA1(0b425f81869bc359841377a91c39f44395502bff))

	ROM_REGION(0xc000, "audiocpu", ROMREGION_ERASEFF)
	// nothing dumped
ROM_END

ROM_START(spcteam)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE("cpu_top.ic15", 0x0001, 0x8000, CRC(b11dcf1f) SHA1(084eb98ee4c9f32d5518897a891ad1a601850d80))
	ROM_LOAD16_BYTE("cpu_bot.ic17", 0x0000, 0x8000, CRC(892a5592) SHA1(c30dce37a5aae2834459179787f6c99353aadabb))

	ROM_REGION(0xc000, "audiocpu", ROMREGION_ERASEFF)
	// ic12 first half not used, inaccessible (all FF)
	ROM_LOAD("sound.ic12", 0x0000, 0x4000, CRC(6a87370f) SHA1(51e055dcf23a30e337ff439bba3c40e5c51c490a))
	ROM_CONTINUE(0x0000, 0x4000)
ROM_END

} // Anonymous namespace

GAME(1987,  xforce,  0,  techno,  techno, techno_state, empty_init, ROT0, "Tecnoplay", "X Force",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
GAME(1988,  spcteam, 0,  techno,  techno, techno_state, empty_init, ROT0, "Tecnoplay", "Space Team", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK | MACHINE_SUPPORTS_SAVE )
