// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

  PINBALL
  Williams System 8
  These are not true pinballs in the normal sense, but are unusual novelty
  machines. Unfortunately they were mostly cancelled before production could
  begin.

  Games:
  - Pennant Fever (#526)
  - Gridiron (#538)
  - Still Crazy (#543)
  - Break Street

The first time run, the display will show the model number. Press F3 to clear this.

Pennant Fever is a baseball game where you aim for targets at the top of the
  playfield, and the players advance towards a home run. There are no bumpers
  or other 'usual' pinball items. 1 or 2 players.
  How to play:
  - Insert coin (credits shows in innings)
  - Start game
  - Player 1 is 'Visitors'; optional Player 2 is 'Home'
  - Press one of L,B,C,V to hit the ball; or comma,period,slash for a home run;
    or (F then A) for a Strike; or N,Z for Out.
  - Wait for score to start flashing
  - Press another key, etc
  - When you have 3 strikes, you are Out
  - When you have 3 Outs, your Innings ends (other player gets a turn)
  - After 3 Innings, it's game over.
  - Match digit appears in Outs digit.

Gridiron, a conversion kit for Pennant Fever. Didn't get past the prototype stage.

Still Crazy, also only a prototype. See s8a.c for more.

Break Street, another failed novelty, not much is known about it. Seems it
  features a break-dancing toy and a spinning disk.

ToDo:


************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "s8.lh"


class s8_state : public genpin_class
{
public:
	s8_state(const machine_config &mconfig, device_type type, const char *tag)
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

	void s8(machine_config &config);

	void init_s8();

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
	DECLARE_MACHINE_RESET(s8);

	void s8_audio_map(address_map &map);
	void s8_main_map(address_map &map);

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

void s8_state::s8_main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(s8_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x5000, 0x7fff).rom().region("roms", 0);
}

void s8_state::s8_audio_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x4000, 0x4003).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xc000, 0xffff).rom().region("audioroms", 0);
}

static INPUT_PORTS_START( s8 )
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xf6, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X10")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X20")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X40")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s8_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s8_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s8_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s8_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE8_MEMBER( s8_state::sol3_w )
{
	if (BIT(data, 1))
		m_samples->start(0, 6); // knocker
}

WRITE8_MEMBER( s8_state::sound_w )
{
	m_sound_data = data;
}

READ_LINE_MEMBER( s8_state::pia21_ca1_r )
{
// sound busy
	return 1;
}

WRITE_LINE_MEMBER( s8_state::pia21_ca2_w )
{
// sound ns
	m_pias->ca1_w(state);
}

WRITE8_MEMBER( s8_state::lamp0_w )
{
}

WRITE8_MEMBER( s8_state::dig0_w )
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_data_ok = true;
	m_digits[60] = patterns[data>>4]; // diag digit
}

WRITE8_MEMBER( s8_state::dig1_w )
{
	static const uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

READ8_MEMBER( s8_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",m_kbdrow);
	return ioport(kbdrow)->read() ^ 0xff;
}

WRITE8_MEMBER( s8_state::switch_w )
{
	m_kbdrow = data;
}

READ8_MEMBER( s8_state::sound_r )
{
	return m_sound_data;
}

WRITE_LINE_MEMBER( s8_state::pia_irq )
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

void s8_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
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

MACHINE_RESET_MEMBER( s8_state, s8 )
{
}

void s8_state::init_s8()
{
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
}

void s8_state::s8(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &s8_state::s8_main_map);
	MCFG_MACHINE_RESET_OVERRIDE(s8_state, s8)

	/* Video */
	config.set_default_layout(layout_s8);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21, 0);
	m_pia21->readpa_handler().set(FUNC(s8_state::sound_r));
	m_pia21->readca1_handler().set(FUNC(s8_state::pia21_ca1_r));
	m_pia21->writepa_handler().set(FUNC(s8_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s8_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s8_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s8_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(FUNC(s8_state::pia_irq));
	m_pia21->irqb_handler().set(FUNC(s8_state::pia_irq));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s8_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s8_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s8_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(s8_state::pia_irq));
	m_pia24->irqb_handler().set(FUNC(s8_state::pia_irq));

	PIA6821(config, m_pia28, 0);
	m_pia28->writepa_handler().set(FUNC(s8_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s8_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s8_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s8_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(FUNC(s8_state::pia_irq));
	m_pia28->irqb_handler().set(FUNC(s8_state::pia_irq));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s8_state::switch_r));
	m_pia30->writepb_handler().set(FUNC(s8_state::switch_w));
	m_pia30->irqa_handler().set(FUNC(s8_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s8_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Add the soundcard */
	M6808(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &s8_state::s8_audio_map);

	SPEAKER(config, "speaker").front_center();
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	PIA6821(config, m_pias, 0);
	m_pias->readpa_handler().set(FUNC(s8_state::sound_r));
	m_pias->writepb_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pias->irqa_handler().set_inputline("audiocpu", M6808_IRQ_LINE);
	m_pias->irqa_handler().set_inputline("audiocpu", M6808_IRQ_LINE);
}

/*------------------------------
/ Pennant Fever (#526) 05/1984
/-------------------------------*/
ROM_START(pfevr_l2)
	ROM_REGION(0x3000, "roms", 0)
	ROM_LOAD("pf-rom1.u19", 0x0000, 0x1000, CRC(00be42bd) SHA1(72ca21c96e3ffa3c43499165f3339b669c8e94a5))
	ROM_LOAD("pf-rom2.u20", 0x1000, 0x2000, CRC(7b101534) SHA1(21e886d5872104d71bb528b9affb12230268597a))

	ROM_REGION(0x4000, "audioroms", 0)
	ROM_LOAD("cpu_u49.128", 0x0000, 0x4000, CRC(b0161712) SHA1(5850f1f1f11e3ac9b9629cff2b26c4ad32436b55))
ROM_END

ROM_START(pfevr_p3)
	ROM_REGION(0x3000, "roms", 0)
	ROM_LOAD("cpu_u19.732", 0x0000, 0x1000, CRC(03796c6d) SHA1(38c95fcce9d0f357a74f041f0df006b9c6f6efc7))
	ROM_LOAD("cpu_u20.764", 0x1000, 0x2000, CRC(3a3acb39) SHA1(7844cc30a9486f718a556850fc9cef3be82f26b7))

	ROM_REGION(0x4000, "audioroms", 0)
	ROM_LOAD("cpu_u49.128", 0x0000, 0x4000, CRC(b0161712) SHA1(5850f1f1f11e3ac9b9629cff2b26c4ad32436b55))
ROM_END


GAME(1984, pfevr_l2, 0,        s8, s8, s8_state, init_s8, ROT0, "Williams", "Pennant Fever (L-2)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
GAME(1984, pfevr_p3, pfevr_l2, s8, s8, s8_state, init_s8, ROT0, "Williams", "Pennant Fever (P-3)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING )
