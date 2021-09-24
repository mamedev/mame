// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

  PINBALL
  Williams System 6a
  The only difference to System 6 is that the display is 7 digits instead of 6.

Diagnostic actions:
- You must be in game over mode. All buttons are in the number-pad. When you are
  finished, you must reboot.

- Setup: 6 must be in auto/up position. Press 5 to enter setup mode, press 6 to
         change direction.

- Tests: 6 must be in manual/down position. Press 5 twice and tests will begin.
         Press 5 and 6 together to get from test 1 to test 2. Press 6 to switch
         between auto/manual stepping.

- Auto Diag Test: Set Dips to SW6. Press 4. Press 9. Press 5. Tests will begin.

- Other: Set Dips to SW7 or SW8. Press 4. Press 9.


Each game has its own switches, you need to know the outhole and slam-tilt ones.
Note that T is also a tilt, but it may take 3 hits to activate it.


Game          Outhole   Tilt
------------------------------------
Algar         X
Alien Poker   X         =

Alien Poker: wait for the background sound before attempting to score.

ToDo:
- Mechanical sounds


************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "sound/hc55516.h"
#include "speaker.h"

#include "s6a.lh"


namespace {

class s6a_state : public genpin_class
{
public:
	s6a_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_hc55516(*this, "hc55516")
		, m_pias(*this, "pias")
		, m_pia22(*this, "pia22")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_swarray(*this, "SW.%u", 0U)
		, m_dips(*this, "DS%u", 1U)
		, m_io_snd(*this, "SND")
	{ }

	void s6a(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

protected:
	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	uint8_t sound_r();
	void dig0_w(uint8_t data);
	void dig1_w(uint8_t data);
	void lamp0_w(uint8_t data);
	void lamp1_w(uint8_t data);
	void sol0_w(uint8_t data);
	void sol1_w(uint8_t data);
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
	DECLARE_WRITE_LINE_MEMBER(pia_irq);

	void s6a_audio_map(address_map &map);
	void s6a_main_map(address_map &map);

	uint8_t m_sound_data;
	uint8_t m_strobe;
	uint8_t m_switch_col;
	bool m_data_ok;
	emu_timer* m_irq_timer;
	static const device_timer_id TIMER_IRQ = 0;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<hc55516_device> m_hc55516;
	required_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia22;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	output_finder<32> m_digits;
	output_finder<2> m_leds;
	required_ioport_array<8> m_swarray;
	required_ioport_array<2> m_dips;
	required_ioport m_io_snd;
};

void s6a_state::s6a_main_map(address_map &map)
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

void s6a_state::s6a_audio_map(address_map &map)
{
	map(0x0080, 0x00ff).ram(); // external 6810 RAM
	map(0x0400, 0x0403).mirror(0x8000).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xb000, 0xffff).rom().region("audioroms", 0);
}

static INPUT_PORTS_START( s6a )
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

	PORT_START("SND")
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Speech") PORT_CODE(KEYCODE_3_PAD) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Music") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s6a_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s6a_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Manual/Auto") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Enter") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("DS1") // DS1 only 3 switches do anything
	PORT_DIPNAME( 0x70, 0x70, "Diagnostic" )
	PORT_DIPSETTING(    0x70, "Off" )
	PORT_DIPSETTING(    0x60, "SW8 - Zero Audit Tables" )
	PORT_DIPSETTING(    0x50, "SW7 - Reset to Defaults" )
	PORT_DIPSETTING(    0x30, "SW6 - Auto Diagnostic Test" )
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DS2") // DS2 switches exist but do nothing
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s6a_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s6a_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void s6a_state::sol0_w(uint8_t data)
{
	if (BIT(data, 4))
		m_samples->start(2, 5); // outhole
}

void s6a_state::sol1_w(uint8_t data)
{
	uint8_t sound_data = m_io_snd->read();
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

	bool cb1 = ((sound_data & 0x9f) != 0x9f);

	if (cb1)
		m_sound_data = sound_data;

	m_pias->cb1_w(cb1);

	if (BIT(data, 5))
		m_samples->start(0, 6); // knocker
}

void s6a_state::lamp0_w(uint8_t data)
{
}

void s6a_state::lamp1_w(uint8_t data)
{
}

uint8_t s6a_state::dips_r()
{
	if (BIT(ioport("DIAGS")->read(), 4))
		return m_dips[BIT(~m_strobe, 1)]->read() << (BIT(m_strobe, 0) ? 4 : 0);

	return 0xff;
}

void s6a_state::dig0_w(uint8_t data)
{
	m_strobe = data & 15;
	m_data_ok = true;
	m_leds[0] = !BIT(data, 4);
	m_leds[1] = !BIT(data, 5);
}

void s6a_state::dig1_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // MC14558
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

uint8_t s6a_state::switch_r()
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

void s6a_state::switch_w(uint8_t data)
{
	// this drives the pulldown 7406 quad open collector inverters at IC17 and IC18, each inverter drives one column of the switch matrix low
	// it is possible for multiple columns to be enabled at once, this is handled in switch_r above.
	m_switch_col = data;
}

uint8_t s6a_state::sound_r()
{
	return m_sound_data;
}

WRITE_LINE_MEMBER( s6a_state::pia_irq )
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
	}
}

void s6a_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
}

void s6a_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IRQ:
		if(param == 1)
		{
			m_maincpu->set_input_line(M6808_IRQ_LINE, ASSERT_LINE);
			m_irq_timer->adjust(attotime::from_ticks(32,3580000/4),0);
			m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
			m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
		}
		else
		{
			m_maincpu->set_input_line(M6808_IRQ_LINE, CLEAR_LINE);
			m_irq_timer->adjust(attotime::from_ticks(980,3580000/4),1);
			m_pia28->ca1_w(1);
			m_pia28->cb1_w(1);
		}
		break;
	}
}

void s6a_state::s6a(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_maincpu, 3580000);
	m_maincpu->set_addrmap(AS_PROGRAM, &s6a_state::s6a_main_map);

	/* Video */
	config.set_default_layout(layout_s6a);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia22, 0);
	m_pia22->writepa_handler().set(FUNC(s6a_state::sol0_w));
	m_pia22->writepb_handler().set(FUNC(s6a_state::sol1_w));
	m_pia22->ca2_handler().set(FUNC(s6a_state::pia22_ca2_w));
	m_pia22->cb2_handler().set(FUNC(s6a_state::pia22_cb2_w));
	m_pia22->irqa_handler().set(FUNC(s6a_state::pia_irq));
	m_pia22->irqb_handler().set(FUNC(s6a_state::pia_irq));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s6a_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s6a_state::lamp1_w));
	m_pia24->ca2_handler().set(FUNC(s6a_state::pia24_ca2_w));
	m_pia24->cb2_handler().set(FUNC(s6a_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(s6a_state::pia_irq));
	m_pia24->irqb_handler().set(FUNC(s6a_state::pia_irq));

	PIA6821(config, m_pia28, 0);
	m_pia28->readpa_handler().set(FUNC(s6a_state::dips_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->writepa_handler().set(FUNC(s6a_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s6a_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s6a_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s6a_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(FUNC(s6a_state::pia_irq));
	m_pia28->irqb_handler().set(FUNC(s6a_state::pia_irq));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s6a_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s6a_state::switch_w));
	m_pia30->ca2_handler().set(FUNC(s6a_state::pia30_ca2_w));
	m_pia30->cb2_handler().set(FUNC(s6a_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(FUNC(s6a_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s6a_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Add the soundcard */
	M6802(config, m_audiocpu, 3580000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &s6a_state::s6a_audio_map);

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);

	SPEAKER(config, "speech").front_center();
	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, "speech", 1.00);

	PIA6821(config, m_pias, 0);
	m_pias->readpb_handler().set(FUNC(s6a_state::sound_r));
	m_pias->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pias->ca2_handler().set("hc55516", FUNC(hc55516_device::digit_w));
	m_pias->cb2_handler().set("hc55516", FUNC(hc55516_device::clock_w));
	m_pias->irqa_handler().set_inputline(m_audiocpu, M6802_IRQ_LINE); // FIXME: needs an input merger
	m_pias->irqb_handler().set_inputline(m_audiocpu, M6802_IRQ_LINE);
}


/*--------------------------
/ Algar - Sys.6 (Game #499)
/-------------------------*/
ROM_START(algar_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716", 0x0000, 0x0800, CRC(6711da23) SHA1(80a46f5a2630977bc1c6e17466e8865083eb9a18))
	ROM_LOAD("green1.716",  0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",  0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x5000, "audioroms", 0)
	ROM_LOAD("sound4.716",  0x4800, 0x0800, CRC(67ea12e7) SHA1(f81e97183442736d5766a7e5e074bc6539e8ced0))
ROM_END

/*-------------------------------
/ Alien Poker - Sys.6 (Game #501)
/-------------------------------*/
ROM_START(alpok_l6)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom6.716", 0x0000, 0x0800, CRC(20538a4a) SHA1(6cdd6b7ded76b3cbd954d371e126e1bbd95a6219))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x5000, "audioroms", 0)
	ROM_LOAD("v_ic7.532",    0x0000, 0x1000, CRC(a66c7ca6) SHA1(6e90081f853fcf66bfeac0a8ee1c762b3760b90b))
	ROM_LOAD("v_ic5.532",    0x1000, 0x1000, CRC(f16a237a) SHA1(a904138fad5cbc19946bcf0de824e27537dcd621))
	ROM_LOAD("v_ic6.532",    0x2000, 0x1000, CRC(15a3cc85) SHA1(86002ac78189415ae912e8bc23c92b3b67610d87))
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

ROM_START(alpok_l2)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom.716",  0x0000, 0x0800, CRC(79c07603) SHA1(526a45b139394e475fc052636e98d880a8908168))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x5000, "audioroms", 0)
	ROM_LOAD("v_ic7.532",    0x0000, 0x1000, CRC(a66c7ca6) SHA1(6e90081f853fcf66bfeac0a8ee1c762b3760b90b))
	ROM_LOAD("v_ic5.532",    0x1000, 0x1000, CRC(f16a237a) SHA1(a904138fad5cbc19946bcf0de824e27537dcd621))
	ROM_LOAD("v_ic6.532",    0x2000, 0x1000, CRC(15a3cc85) SHA1(86002ac78189415ae912e8bc23c92b3b67610d87))
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

ROM_START(alpok_f6)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("gamerom6.716", 0x0000, 0x0800, CRC(20538a4a) SHA1(6cdd6b7ded76b3cbd954d371e126e1bbd95a6219))
	ROM_LOAD("green1.716",   0x1000, 0x0800, CRC(2145f8ab) SHA1(ddf63208559a3a08d4e88327c55426b0eed27654))
	ROM_LOAD("green2.716",   0x1800, 0x0800, CRC(1c978a4a) SHA1(1959184764643d58f1740c54bb74c2aad7d667d2))

	ROM_REGION(0x5000, "audioroms", 0)
	ROM_LOAD("5t5014fr.dat", 0x0000, 0x1000, CRC(1d961517) SHA1(c71ee324becfc8cdbecabd1e64b11b5a39ff2483))
	ROM_LOAD("5t5015fr.dat", 0x1000, 0x1000, CRC(8d065f80) SHA1(0ab22c9b20ab6fe41abab620435ad03652db7a8e))
	ROM_LOAD("5t5016fr.dat", 0x2000, 0x1000, CRC(0ddf91e9) SHA1(48f5fdfc0c5a66dd318fecb7c90e5f5a684a3876))
	ROM_LOAD("5t5017fr.dat", 0x3000, 0x1000, CRC(7e546dc1) SHA1(58f8286403978b0d929987189089881d754a9a83))
	ROM_LOAD("sound3.716",   0x4800, 0x0800, CRC(55a10d13) SHA1(521d4cdfb0ed8178b3594cedceae93b772a951a4))
ROM_END

} // anonymous namespace


GAME( 1980, algar_l1, 0,        s6a, s6a, s6a_state, empty_init, ROT0, "Williams", "Algar (L-1)",                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1980, alpok_l6, 0,        s6a, s6a, s6a_state, empty_init, ROT0, "Williams", "Alien Poker (L-6)",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1980, alpok_l2, alpok_l6, s6a, s6a, s6a_state, empty_init, ROT0, "Williams", "Alien Poker (L-2)",               MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME( 1980, alpok_f6, alpok_l6, s6a, s6a, s6a_state, empty_init, ROT0, "Williams", "Alien Poker (L-6 French speech)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
