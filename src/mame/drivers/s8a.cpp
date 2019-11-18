// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

  PINBALL
  Williams System 8: Still Crazy

The first time run, the display will show the model number (543).
Press F3 to clear this.

A novelty game where the playfield is completely vertical. It has 4 flippers and the
  idea is to get the ball up to the alcohol 'still' before the 'revenuers' do. The
  idea didn't catch on, and the game was not officially released. 1 player.
  The display shows Score and Batch. There is no credit display.
  If the number of batches exceeds 9, the 'hidden digit' will show the tens.
  You cannot get more than 99 batches.
  The score only has 5 digits, but the game stores the 100,000 digit internally.

ToDo:

************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "s8a.lh"


class s8a_state : public genpin_class
{
public:
	s8a_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_pias(*this, "pias")
		, m_pia21(*this, "pia21")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void s8a(machine_config &config);

	void init_s8a();

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);
	DECLARE_INPUT_CHANGED_MEMBER(audio_nmi);

private:
	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_WRITE8_MEMBER(dig1_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_WRITE8_MEMBER(sol2_w) { }; // solenoids 8-15
	DECLARE_WRITE8_MEMBER(sol3_w); // solenoids 0-7
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ_LINE_MEMBER(pia21_ca1_r);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { }; // enable solenoids
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { }; // dummy to stop error log filling up
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { }; // comma3&4
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { }; // comma1&2
	DECLARE_WRITE_LINE_MEMBER(pia_irq);
	DECLARE_MACHINE_RESET(s8a);

	void s8a_audio_map(address_map &map);
	void s8a_main_map(address_map &map);

	uint8_t m_sound_data;
	uint8_t m_strobe;
	uint8_t m_kbdrow;
	bool m_data_ok;
	emu_timer* m_irq_timer;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	static const device_timer_id TIMER_IRQ = 0;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<pia6821_device> m_pias;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	output_finder<61> m_digits;
};

void s8a_state::s8a_main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(s8a_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x6000, 0x7fff).rom().region("roms", 0);
}

void s8a_state::s8a_audio_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x2000, 0x2003).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc000, 0xffff).rom().region("audioroms", 0);
}

static INPUT_PORTS_START( s8a )
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X8")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X10")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X20")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X40")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s8a_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s8a_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s8a_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s8a_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE8_MEMBER( s8a_state::sol3_w )
{
	if (data==0x0a)
		m_samples->start(0, 7); // mechanical drum when you have 2 or more batches
}

WRITE8_MEMBER( s8a_state::sound_w )
{
	m_sound_data = data;
}

READ_LINE_MEMBER( s8a_state::pia21_ca1_r )
{
// sound busy
	return 1;
}

WRITE_LINE_MEMBER( s8a_state::pia21_ca2_w )
{
// sound ns
	m_pias->ca1_w(state);
}

WRITE8_MEMBER( s8a_state::lamp0_w )
{
}

WRITE8_MEMBER( s8a_state::dig0_w )
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_data_ok = true;
	m_digits[60] = patterns[data>>4]; // diag digit
}

WRITE8_MEMBER( s8a_state::dig1_w )
{
	static const uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

READ8_MEMBER( s8a_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ioport(kbdrow)->read() ^ 0xff;
}

WRITE8_MEMBER( s8a_state::switch_w )
{
	m_kbdrow = data;
}

READ8_MEMBER( s8a_state::sound_r )
{
	return m_sound_data;
}

WRITE_LINE_MEMBER( s8a_state::pia_irq )
{
	if(state == CLEAR_LINE)
	{
		// restart IRQ timer
		m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
	}
	else
	{
		// disable IRQ timer while other IRQs are being handled
		// (counter is reset every 32 cycles while a PIA IRQ is handled)
		m_irq_timer->adjust(attotime::zero);
	}
}

void s8a_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IRQ:
		if(param == 1)
		{
			m_maincpu->set_input_line(M6802_IRQ_LINE, ASSERT_LINE);
			m_irq_timer->adjust(attotime::from_ticks(32,1e6),0);
			m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
			m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
		}
		else
		{
			m_maincpu->set_input_line(M6802_IRQ_LINE, CLEAR_LINE);
			m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
			m_pia28->ca1_w(1);
			m_pia28->cb1_w(1);
		}
		break;
	}
}

MACHINE_RESET_MEMBER( s8a_state, s8a )
{
}

void s8a_state::init_s8a()
{
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
}

void s8a_state::s8a(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &s8a_state::s8a_main_map);
	MCFG_MACHINE_RESET_OVERRIDE(s8a_state, s8a)

	/* Video */
	config.set_default_layout(layout_s8a);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21, 0);
	m_pia21->readpa_handler().set(FUNC(s8a_state::sound_r));
	m_pia21->readca1_handler().set(FUNC(s8a_state::pia21_ca1_r));
	m_pia21->writepa_handler().set(FUNC(s8a_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s8a_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s8a_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s8a_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(FUNC(s8a_state::pia_irq));
	m_pia21->irqb_handler().set(FUNC(s8a_state::pia_irq));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s8a_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s8a_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s8a_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(s8a_state::pia_irq));
	m_pia24->irqb_handler().set(FUNC(s8a_state::pia_irq));

	PIA6821(config, m_pia28, 0);
	m_pia28->writepa_handler().set(FUNC(s8a_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s8a_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s8a_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s8a_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(FUNC(s8a_state::pia_irq));
	m_pia28->irqb_handler().set(FUNC(s8a_state::pia_irq));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s8a_state::switch_r));
	m_pia30->writepb_handler().set(FUNC(s8a_state::switch_w));
	m_pia30->irqa_handler().set(FUNC(s8a_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s8a_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Add the soundcard */
	M6808(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &s8a_state::s8a_audio_map);

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	PIA6821(config, m_pias, 0);
	m_pias->readpa_handler().set(FUNC(s8a_state::sound_r));
	m_pias->writepb_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pias->irqa_handler().set_inputline("audiocpu", M6808_IRQ_LINE);
	m_pias->irqa_handler().set_inputline("audiocpu", M6808_IRQ_LINE);
}


/*----------------------------
/ Still Crazy (#543) 06/1984
/-----------------------------*/
ROM_START(scrzy_l1)
	ROM_REGION(0x2000, "roms", 0)
	ROM_LOAD("ic20.bin", 0x0000, 0x2000, CRC(b0df42e6) SHA1(bb10268d7b820d1de0c20e1b79aba558badd072b) )

	ROM_REGION(0x4000, "audioroms", 0)
	// 1st and 2nd halves are identical
	ROM_LOAD("ic49.bin", 0x0000, 0x4000, CRC(bcc8ccc4) SHA1(2312f9cc4f5a2dadfbfa61d13c31bb5838adf152) )
ROM_END

GAME( 1984, scrzy_l1, 0, s8a, s8a, s8a_state, init_s8a, ROT0, "Williams", "Still Crazy", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
