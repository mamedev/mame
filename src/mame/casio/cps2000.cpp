// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*
    Skeleton driver for Casio CPS-2000 piano.

    Sound hardware:
    - 2x uPD932 consonant-vowel synth
    - uPD934 PCM rhythm
    - monophonic square wave bass w/ RC envelope and filter
*/

#include "emu.h"

#include "cpu/upd7810/upd7810.h"
#include "machine/msm6200.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"
#include "sound/upd934g.h"
#include "sound/va_eg.h"
#include "sound/va_vca.h"

#include "speaker.h"

namespace {

class cps2000_state : public driver_device
{
public:
	cps2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kbd(*this, "kbd")
		, m_bass_env(*this, "bass_env")
		, m_pcm(*this, "pcm")
		, m_keys(*this, "KC%u", 0)
	{ }

	void cps2000(machine_config &config);

	void keys_w(ioport_value val) { m_key_sel = val; }
	ioport_value keys_r();

	void bass_w(ioport_value val);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void maincpu_map(address_map &map) ATTR_COLD;

	void io_w(offs_t offset, u8 data);

	required_device<upd78c10_device> m_maincpu;
	required_device<msm6200_device> m_kbd;
	required_device<va_rc_eg_device> m_bass_env;
	required_device<upd934g_device> m_pcm;

	required_ioport_array<5> m_keys;

	ioport_value m_key_sel;

	u8 m_932a_regs[256];
	u8 m_932b_regs[256];
};

/**************************************************************************/
static INPUT_PORTS_START(cps2000)
	PORT_START("PA")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(cps2000_state::keys_w))
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_OUTPUT ) // LED outputs

	PORT_START("PB")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(cps2000_state::keys_r))
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_OUTPUT ) // LED outputs

	PORT_START("PC")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_OUTPUT ) // LED outputs
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_CUSTOM ) // 934 busy
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(cps2000_state::bass_w))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("kbd:KI1")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G7")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#7")

	PORT_START("kbd:KI2")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F7")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E7")

	PORT_START("kbd:KI3")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#7")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D7")

	PORT_START("kbd:KI4")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#7")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C7")

	PORT_START("kbd:KI5")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B6")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#6")

	PORT_START("kbd:KI6")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A6")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#6")

	PORT_START("kbd:KI7")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G6")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#6")

	PORT_START("kbd:KI8")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F6")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E6")

	PORT_START("kbd:KI9")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#6")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D6")

	PORT_START("kbd:KI10")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#6")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C6")

	PORT_START("kbd:KI11")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#5")

	PORT_START("kbd:KI12")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#5")

	PORT_START("kbd:KI13")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#5")

	PORT_START("kbd:KI14")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E5")

	PORT_START("kbd:KI15")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D5")

	PORT_START("kbd:KI16")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#5")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C5")

	PORT_START("kbd:KI17")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#4")

	PORT_START("kbd:KI18")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#4")

	PORT_START("kbd:KI19")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#4")

	PORT_START("kbd:KI20")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E4")

	PORT_START("kbd:KI21")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D4")

	PORT_START("kbd:KI22")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#4")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C4")

	PORT_START("kbd:KI23")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#3")

	PORT_START("kbd:KI24")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#3")

	PORT_START("kbd:KI25")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#3")

	PORT_START("kbd:KI26")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E3")

	PORT_START("kbd:KI27")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D3")

	PORT_START("kbd:KI28")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#3")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C3")

	PORT_START("kbd:KI29")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#2")

	PORT_START("kbd:KI30")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#2")

	PORT_START("kbd:KI31")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#2")

	PORT_START("kbd:KI32")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E2")

	PORT_START("kbd:KI33")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D#2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("D2")

	PORT_START("kbd:KI34")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C#2")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("C2")

	PORT_START("kbd:KI35")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("B1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A#1")

	PORT_START("kbd:KI36")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("A1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G#1")

	PORT_START("kbd:KI37")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("G1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F#1")

	PORT_START("kbd:KI38")
	PORT_BIT(0x1, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("F1")
	PORT_BIT(0x2, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("E1")

	PORT_START("kbd:VELOCITY")
	PORT_BIT(0x3f, 0x3f, IPT_POSITIONAL_V) PORT_NAME("Key Velocity") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("AN0")
	PORT_BIT(0xff, 0x80, IPT_POSITIONAL_V) PORT_NAME("Tempo") PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("AN1")
	PORT_BIT(0xff, 0x80, IPT_POSITIONAL_H) PORT_NAME("Tuning") PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_PLAYER(2) PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("KC0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Octave Unison")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Sustain")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Chorus")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Fingered")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Casio Chord On")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Casio Chord Off")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Start / Stop")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Synchro")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Intro / Fill-In")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Super Accomp")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Memory")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Variation")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm Select")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Touch Response")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 5")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 6")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 7")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 3")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone Select")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Sustain Pedal")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 6")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

/**************************************************************************/
void cps2000_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).r(m_kbd, FUNC(msm6200_device::read));
	map(0x8000, 0xffff).w(FUNC(cps2000_state::io_w));
}

/**************************************************************************/
void cps2000_state::cps2000(machine_config &config)
{
	UPD78C10(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &cps2000_state::maincpu_map);
	m_maincpu->pa_out_cb().set_ioport("PA");
	m_maincpu->pb_in_cb().set_ioport("PB");
	m_maincpu->pb_out_cb().set_ioport("PB");
	m_maincpu->pc_in_cb().set_ioport("PC");
	m_maincpu->pc_out_cb().set_ioport("PC");
	m_maincpu->an0_func().set_ioport("AN0");
	m_maincpu->an1_func().set_ioport("AN1");

	MSM6200(config, m_kbd, 4.9468_MHz_XTAL);
	m_kbd->irq_cb().set_inputline(m_maincpu, UPD7810_INTF1);

	SPEAKER(config, "speaker", 2).front();

	// UPD932(config, "upd932a", 4.9468_MHz_XTAL);
	// UPD932(config, "upd932b", 4.9468_MHz_XTAL);

	DAC_1BIT(config, "bass").add_route(0, "bass_vca", 1.0);
	m_maincpu->co0_func().set("bass", FUNC(dac_1bit_device::write));

	VA_RC_EG(config, m_bass_env).set_c(CAP_U(3.3)).add_route(0, "bass_vca", 1.0 / 5);
	VA_VCA(config, "bass_vca").configure_streaming_cv(true).add_route(0, "bass_rc1", 1.0);

	FILTER_RC(config, "bass_rc1").set_lowpass(RES_K(47), CAP_N(47)).add_route(0, "bass_rc2", 1.0);
	FILTER_RC(config, "bass_rc2").set_lowpass(RES_K(22), CAP_N(47))
		.add_route(0, "speaker", 0.25, 0).add_route(0, "speaker", 0.25, 1);

	UPD934G(config, m_pcm, 4.9468_MHz_XTAL / 4);
	m_pcm->add_route(0, "speaker", 0.25,  0);
	m_pcm->add_route(0, "speaker", 0.125, 1);
	m_pcm->add_route(1, "speaker", 0.25,  0);
	m_pcm->add_route(1, "speaker", 0.125, 1);
	m_pcm->add_route(2, "speaker", 0.125, 0);
	m_pcm->add_route(2, "speaker", 0.25,  1);
	m_pcm->add_route(3, "speaker", 0.125, 0);
	m_pcm->add_route(3, "speaker", 0.25,  1);
}

/**************************************************************************/
void cps2000_state::machine_start()
{
	m_key_sel = 0;

	std::fill(std::begin(m_932a_regs), std::end(m_932a_regs), 0);
	std::fill(std::begin(m_932b_regs), std::end(m_932b_regs), 0);

	save_item(NAME(m_key_sel));
	save_item(NAME(m_932a_regs));
	save_item(NAME(m_932b_regs));
}

/**************************************************************************/
void cps2000_state::machine_reset()
{
}

/**************************************************************************/
ioport_value cps2000_state::keys_r()
{
	if (m_key_sel < 5)
		return m_keys[m_key_sel]->read();

	return 0xff;
}

/**************************************************************************/
void cps2000_state::bass_w(ioport_value val)
{
	if (val)
		m_bass_env->set_r(RES_R(270)).set_target_v(5.0);
	else
		m_bass_env->set_r(RES_K(22)).set_target_v(0.0);
}

/**************************************************************************/
void cps2000_state::io_w(offs_t offset, u8 data)
{
	if (!BIT(offset, 14))
		m_kbd->write(offset, data);
	if (!BIT(offset, 13))
		m_pcm->write(offset >> 8, offset);
	if (!BIT(offset, 12))
		m_932a_regs[offset & 0xff] = data;
	if (!BIT(offset, 11))
		m_932b_regs[offset & 0xff] = data;
}

/**************************************************************************/
ROM_START( cps2000 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "hn613256p-db6.bin", 0x0000, 0x8000, CRC(92b68074) SHA1(eda13d59a15077b06d95004b5f2a2a8432109893) )

	ROM_REGION(0x10000, "pcm", 0)
	ROM_LOAD( "hn613256p-da5.bin", 0x0000, 0x8000, CRC(63d6dd54) SHA1(231ffb6a4cc113133accd19dd9ed1e5165e3837e) )
ROM_END

} // anonymous namespace

SYST( 1986, cps2000, 0,    0, cps2000, cps2000, cps2000_state, empty_init, "Casio", "CPS-2000", MACHINE_NOT_WORKING )
