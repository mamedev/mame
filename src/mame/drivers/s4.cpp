// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

    PINBALL
    Williams System 4

    Phoenix and Pokerino are listed as System 4 systems, but use System 3 ROMs.
    They have been moved to s3.cpp, and are working there.

    The "Shuffle" games consist of a flat board with an air-driven puck and 10
    bowling pins. You must push the puck as if it was a bowling ball, and score
    strikes and spares. Since the maximum score is 300, the displays have 4 digits
    and 6 can play. They will most likely be split off to a separate driver.


Each game has its own switches, you need to know the outhole and slam-tilt ones.
Note that T is also a tilt, but it may take 3 hits to activate it.

Game          Outhole   Tilt
------------------------------------
Flash         O         I
Stellar Wars  X
TriZone       X         ]
Time Warp     X         E


ToDo:
- Shuffle games: need a layout, and don't work.


************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "speaker.h"

#include "s4.lh"


namespace {

class s4_state : public genpin_class
{
public:
	s4_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_swarray(*this, "SW.%u", 0U)
		, m_dips(*this, "DS%u", 1U)
	{
	}

	void s4(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;

private:
	void dig0_w(uint8_t data);
	void dig1_w(uint8_t data);
	void lamp0_w(uint8_t data);
	void lamp1_w(uint8_t data);
	void sol0_w(uint8_t data);
	void sol1_chimes_w(uint8_t data);
	uint8_t dips_r();
	uint8_t switch_r();
	void switch_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(pia22_ca2_w) { } //ST5
	DECLARE_WRITE_LINE_MEMBER(pia22_cb2_w) { } //ST-solenoids enable
	DECLARE_WRITE_LINE_MEMBER(pia24_ca2_w) { } //ST2
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { } //ST1
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { } //diag leds enable
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { } //ST6
	DECLARE_WRITE_LINE_MEMBER(pia30_ca2_w) { } //ST4
	DECLARE_WRITE_LINE_MEMBER(pia30_cb2_w) { } //ST3
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void s4_main_map(address_map &map);

	output_finder<32> m_digits;
	output_finder<2> m_leds;
	required_ioport_array<8> m_swarray;
	required_ioport_array<2> m_dips;

	uint8_t m_t_c;
	uint8_t m_strobe;
	uint8_t m_switch_col;
	bool m_data_ok;
};


class s4a_state : public s4_state
{
public:
	s4a_state(const machine_config &mconfig, device_type type, const char *tag)
		: s4_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_pias(*this, "pias")
		, m_io_snd(*this, "SND")
	{
	}

	void s4a(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:
	required_device<cpu_device> m_audiocpu;
	required_device<pia6821_device> m_pias;

private:
	uint8_t sound_r();
	void s4a_sol1_w(uint8_t data);

	void s4_audio_map(address_map &map);

	required_ioport m_io_snd;

	uint8_t m_sound_data;
};


void s4_state::s4_main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x01ff).ram().share("nvram");
	map(0x2200, 0x2203).rw(m_pia22, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x6000, 0x7fff).rom().region("roms", 0);
}

void s4a_state::s4_audio_map(address_map &map)
{
	map.global_mask(0x0fff);
	map(0x0000, 0x00ff).ram();
	map(0x0400, 0x0403).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sounds
	map(0x0800, 0x0fff).rom().region("audioroms", 0);
}


static INPUT_PORTS_START( s4 )
	PORT_START("SW.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("SW.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("SW.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("SW.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("SW.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("SW.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("SW.6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW.7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s4_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Manual/Auto") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("DS1")
	PORT_DIPNAME( 0xf0, 0xf0, "Data units" )
	PORT_DIPSETTING(    0xf0, "0" )
	PORT_DIPSETTING(    0x70, "1" )
	PORT_DIPSETTING(    0xb0, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0xd0, "4" )
	PORT_DIPSETTING(    0x50, "5" )
	PORT_DIPSETTING(    0x90, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0xe0, "8" )
	PORT_DIPSETTING(    0x60, "9" )
	PORT_DIPNAME( 0x0f, 0x0f, "Data tens" )
	PORT_DIPSETTING(    0x0f, "0" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x0b, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x0d, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x09, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x0e, "8" )
	PORT_DIPSETTING(    0x06, "9" )

	PORT_START("DS2")
	PORT_DIPNAME( 0xff, 0xff, "Function" )
	PORT_DIPSETTING(    0xff, "0" )
	PORT_DIPSETTING(    0x7f, "1" )
	PORT_DIPSETTING(    0xbf, "2" )
	PORT_DIPSETTING(    0x3f, "3" )
	PORT_DIPSETTING(    0xdf, "4" )
	PORT_DIPSETTING(    0x5f, "5" )
	PORT_DIPSETTING(    0x9f, "6" )
	PORT_DIPSETTING(    0x1f, "7" )
	PORT_DIPSETTING(    0xef, "8" )
	PORT_DIPSETTING(    0x6f, "9" )
	PORT_DIPSETTING(    0xaf, "10" )
	PORT_DIPSETTING(    0x2f, "11" )
	PORT_DIPSETTING(    0xcf, "12" )
	PORT_DIPSETTING(    0x4f, "13" )
	PORT_DIPSETTING(    0x8f, "14" )
	PORT_DIPSETTING(    0x0f, "15" )
	PORT_DIPSETTING(    0xf7, "16" )
	PORT_DIPSETTING(    0x77, "17" )
	PORT_DIPSETTING(    0xb7, "18" )
	PORT_DIPSETTING(    0x37, "19" )
	PORT_DIPSETTING(    0xd7, "20" )
	PORT_DIPSETTING(    0x57, "21" )
	PORT_DIPSETTING(    0x97, "22" )
	PORT_DIPSETTING(    0x17, "23" )
	PORT_DIPSETTING(    0xe7, "24" )
	PORT_DIPSETTING(    0x67, "25" )
	PORT_DIPSETTING(    0xa7, "26" )
	PORT_DIPSETTING(    0x27, "27" )
	PORT_DIPSETTING(    0xc7, "28" )
	PORT_DIPSETTING(    0x47, "29" )
	PORT_DIPSETTING(    0x87, "30" )
	PORT_DIPSETTING(    0x07, "31" )
INPUT_PORTS_END

static INPUT_PORTS_START( s4a )
	PORT_INCLUDE(s4)

	PORT_START("SND")
	PORT_BIT( 0xbf, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Music") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE

	PORT_MODIFY("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s4a_state, audio_nmi, 1)
INPUT_PORTS_END


void s4_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	m_strobe = 0;
}

void s4_state::machine_reset()
{
	m_t_c = 0;
}

INPUT_CHANGED_MEMBER( s4_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s4a_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void s4_state::sol0_w(uint8_t data)
{
	if (BIT(data, 4))
		m_samples->start(2, 5); // outhole
}

void s4_state::sol1_chimes_w(uint8_t data)
{
	if (BIT(data, 0))
		m_samples->start(1, 1); // 10 chime

	if (BIT(data, 1))
		m_samples->start(2, 2); // 100 chime

	if (BIT(data, 2))
		m_samples->start(3, 3); // 1000 chime

	if (BIT(data, 3))
		m_samples->start(1, 4); // 10k chime

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

void s4a_state::s4a_sol1_w(uint8_t data)
{
	uint8_t sound_data = m_io_snd->read(); // 0xff or 0xbf
	if (BIT(data, 0))
		sound_data &= 0xfe;

	if (BIT(data, 1))
		sound_data &= 0xfd;

	if (BIT(data, 2))
		sound_data &= 0xfb;

	if (BIT(data, 3))
		sound_data &= 0xf7;

	if (BIT(data, 4))
		sound_data &= 0xef;

	bool cb1 = ((sound_data & 0xbf) != 0xbf);

	if (cb1)
		m_sound_data = sound_data;

	m_pias->cb1_w(cb1);

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

void s4_state::lamp0_w(uint8_t data)
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

void s4_state::lamp1_w(uint8_t data)
{
}

uint8_t s4_state::dips_r()
{
	if (BIT(ioport("DIAGS")->read(), 4))
		return m_dips[BIT(~m_strobe, 1)]->read() << (BIT(m_strobe, 0) ? 4 : 0);

	return 0xff;
}

void s4_state::dig0_w(uint8_t data)
{
	m_strobe = data & 15;
	m_data_ok = true;
	m_leds[0] = !BIT(data, 4);
	m_leds[1] = !BIT(data, 5);
}

void s4_state::dig1_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

uint8_t s4_state::switch_r()
{
	uint8_t retval = 0xff;
	// scan all 8 input columns, since multiple can be selected at once
	for (int i = 0; i < 7; i++)
	{
		if (m_switch_col & (1<<i))
			retval &= m_swarray[i]->read();
	}
	return ~retval;
}

void s4_state::switch_w(uint8_t data)
{
	// this drives the pulldown 7406 quad open collector inverters at IC17 and IC18, each inverter drives one column of the switch matrix low
	// it is possible for multiple columns to be enabled at once, this is handled in switch_r above.
	m_switch_col = data;
}

uint8_t s4a_state::sound_r()
{
	return m_sound_data;
}

TIMER_DEVICE_CALLBACK_MEMBER( s4_state::irq )
{
	if (m_t_c > 0x70)
		m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
	else
		m_t_c++;
}

void s4_state::s4(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 3580000);
	m_maincpu->set_addrmap(AS_PROGRAM, &s4_state::s4_main_map);
	TIMER(config, "irq").configure_periodic(FUNC(s4_state::irq), attotime::from_hz(250));

	// Video
	config.set_default_layout(layout_s4);

	// Sound
	genpin_audio(config);

	// Devices
	PIA6821(config, m_pia22, 0);
	m_pia22->writepa_handler().set(FUNC(s4_state::sol0_w));
	m_pia22->writepb_handler().set(FUNC(s4_state::sol1_chimes_w));
	m_pia22->ca2_handler().set(FUNC(s4_state::pia22_ca2_w));
	m_pia22->cb2_handler().set(FUNC(s4_state::pia22_cb2_w));
	m_pia22->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia22->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s4_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s4_state::lamp1_w));
	m_pia24->ca2_handler().set(FUNC(s4_state::pia24_ca2_w));
	m_pia24->cb2_handler().set(FUNC(s4_state::pia24_cb2_w));
	m_pia24->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia24->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	PIA6821(config, m_pia28, 0);
	m_pia28->readpa_handler().set(FUNC(s4_state::dips_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->readca1_handler().set_ioport("DIAGS").bit(2); // advance button
	m_pia28->readcb1_handler().set_ioport("DIAGS").bit(3); // auto/manual switch
	m_pia28->writepa_handler().set(FUNC(s4_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s4_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s4_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s4_state::pia28_cb2_w));
	m_pia28->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia28->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s4_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s4_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(s4_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(s4_state::pia30_cb2_w));
	m_pia30->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia30->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void s4a_state::s4a(machine_config &config)
{
	s4(config);

	m_pia22->writepb_handler().set(FUNC(s4a_state::s4a_sol1_w));

	// Add the soundcard
	M6808(config, m_audiocpu, 3580000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &s4a_state::s4_audio_map);

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);

	PIA6821(config, m_pias, 0);
	m_pias->readpb_handler().set(FUNC(s4a_state::sound_r));
	m_pias->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pias->irqa_handler().set_inputline(m_audiocpu, M6808_IRQ_LINE);
	m_pias->irqb_handler().set_inputline(m_audiocpu, M6808_IRQ_LINE);
}


/*--------------------------------
/ Flash - Sys.4 (Game #486)
/-------------------------------*/
ROM_START(flash_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(287f12d6) SHA1(ede0c5b0ea2586d8bdf71ecadbd9cc8552bd6934))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(flash_l2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom2.716", 0x0000, 0x0800, CRC(b7c2e4c7) SHA1(00ea34900af679b1b7e2698f4aa2fc9703d54cf2))
	ROM_LOAD("yellow1.716",  0x1000, 0x0800, CRC(d251738c) SHA1(65ddbf5c36e429243331a4c5d2339df87a8a7f64))
	ROM_LOAD("yellow2.716",  0x1800, 0x0800, CRC(5049326d) SHA1(3b2f4ea054962bf4ba41d46663b7d3d9a77590ef))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(flash_t1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(287f12d6) SHA1(ede0c5b0ea2586d8bdf71ecadbd9cc8552bd6934))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716",  0x1800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Tri Zone - Sys.4 (Game #487)
/-------------------------------*/
ROM_START(trizn_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(757091c5) SHA1(00dac6c19b08d2528ea293619c4a39499a1a02c2))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(trizn_t1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(757091c5) SHA1(00dac6c19b08d2528ea293619c4a39499a1a02c2))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716",  0x1800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Time Warp - Sys.4 (Game #489)
/-------------------------------*/
ROM_START(tmwrp_l3)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("twarp_l3.716",  0x0000, 0x0800, CRC(1234710a) SHA1(a33e9edb79b6ea4c5982d28a55289897f82b7b3c))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(tmwrp_l2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(b168df09) SHA1(d4c97714636ce51be2e5f8cc5af89e10a2f82ac7))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

ROM_START(tmwrp_t2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(b168df09) SHA1(d4c97714636ce51be2e5f8cc5af89e10a2f82ac7))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2a.716",  0x1800, 0x0800, CRC(16621eec) SHA1(14e1cf5f7227860a3219b2b79fa66dcf252dce98))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*--------------------------------
/ Stellar Wars - Sys.4 (Game #490)
/-------------------------------*/
ROM_START(stlwr_l2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(874e7ef7) SHA1(271aeac2a0e61cb195811ae2e8d908cb1ab45874))
	ROM_LOAD("yellow1.716",  0x1000, 0x0800, CRC(d251738c) SHA1(65ddbf5c36e429243331a4c5d2339df87a8a7f64))
	ROM_LOAD("yellow2.716",  0x1800, 0x0800, CRC(5049326d) SHA1(3b2f4ea054962bf4ba41d46663b7d3d9a77590ef))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END



/* From here, are NOT pinball machines */

/*----------------------------
/ Pompeii (Shuffle)
/----------------------------*/
ROM_START(pomp_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Aristocrat (Shuffle) same roms as Pompeii
/----------------------------*/
ROM_START(arist_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(0f069ac2) SHA1(d651d49cdb50cf444e420241a1f9ed48c878feee))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Topaz (Shuffle)
/----------------------------*/
ROM_START(topaz_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(cb287b10) SHA1(7fb6b6a26237cf85d5e02cf35271231267f90fc1))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound1.716",   0x0000, 0x0800, CRC(f4190ca3) SHA1(ee234fb5c894fca5876ee6dc7ea8e89e7e0aec9c))
ROM_END

/*----------------------------
/ Taurus (Shuffle)
/----------------------------*/
ROM_START(taurs_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(3246e285) SHA1(4f76784ecb5063a49c24795ae61db043a51e2c89))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ King Tut
/----------------------------*/
ROM_START(kingt_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(54d3280a) SHA1(ca74636e35d2c3e0b3133f89b1ff1233d5d72a5c))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("soundx.716",   0x0000, 0x0800, CRC(539d64fb) SHA1(ff0d09c8d7c65e1072691b5b9e4fcaa3f38d67e8))
ROM_END

/*----------------------------
/ Omni (Shuffle)
/----------------------------*/
ROM_START(omni_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("omni-1a.u21",  0x0000, 0x0800, CRC(443bd170) SHA1(cc1ebd72d77ec2014cbd84534380e5ea1f12c022))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))

	ROM_REGION(0x0800, "audioroms", 0)
	ROM_LOAD("sound.716",    0x0000, 0x0800, CRC(db085cbb) SHA1(9a57abbad183ba16b3dba16d16923c3bfc46a0c3))
ROM_END

/*----------------------------
/ Big Strike (Shuffle)
/----------------------------*/
ROM_START(bstrk_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(323dbcde) SHA1(a75cbb5de97cb9afc1d36e9b6ff593bb482fcf8b))
	ROM_LOAD("b_ic20.716",   0x1000, 0x0800, CRC(c6f8e3b1) SHA1(cb78d42e1265162132a1ab2320148b6857106b0e))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
ROM_END

/*----------------------------
/ Triple Strike (Shuffle)
/----------------------------*/
ROM_START(tstrk_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(b034c059) SHA1(76b3926b87b3c137fcaf33021a586827e3c030af))
	ROM_LOAD("ic20.716",     0x1000, 0x0800, CRC(f163fc88) SHA1(988b60626f3d4dc8f4a1dbd0c99282418bc53aae))
	ROM_LOAD("b_ic17.716",   0x1800, 0x0800, CRC(cfc2518a) SHA1(5e99e40dcb7e178137db8d7d7d6da82ba87130fa))
ROM_END

} // Anonymous namespace


GAME( 1979, flash_l2, 0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Flash (Williams, L-2)",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1979, flash_l1, flash_l2, s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Flash (Williams, L-1)",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1979, flash_t1, flash_l2, s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Flash (Williams, T-1) Ted Estes", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1978, trizn_l1, 0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Tri Zone (L-1)",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1978, trizn_t1, trizn_l1, s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Tri Zone (T-1)",                MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1979, tmwrp_l3, 0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Time Warp (Williams, L-3)",     MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1979, tmwrp_l2, tmwrp_l3, s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Time Warp (Williams, L-2)",     MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1979, tmwrp_t2, tmwrp_l3, s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Time Warp (Williams, T-2)",     MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1979, stlwr_l2, 0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Stellar Wars (L-2)",            MACHINE_MECHANICAL | MACHINE_NOT_WORKING )

GAME( 1978, pomp_l1,  0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Pompeii (Shuffle) (L-1)",       MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1978, arist_l1, 0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Aristocrat (Shuffle) (L-1)",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1978, topaz_l1, 0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Topaz (Shuffle) (L-1)",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, taurs_l1, 0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Taurus (Shuffle) (L-1)",        MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1979, kingt_l1, 0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "King Tut (Shuffle) (L-1)",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING)
GAME( 1980, omni_l1,  0,        s4a, s4a, s4a_state, empty_init, ROT0, "Williams", "Omni (Shuffle) (L-1)",          MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 1983, bstrk_l1, 0,        s4,  s4,  s4_state,  empty_init, ROT0, "Williams", "Big Strike (Shuffle) (L-1)",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 1983, tstrk_l1, 0,        s4,  s4,  s4_state,  empty_init, ROT0, "Williams", "Triple Strike (Shuffle) (L-1)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
