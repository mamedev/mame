// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************

PINBALL
Williams System 8: Still Crazy (#543)

The first time run, the display will show the model number. Press F3 to clear this.

IMDB shows the number as 534, however both the game and the manual say 543. The
undumped System 10 game 4-in-1 (Pigskin/Poker/Willy at the Bat/Willy's Cup) also
was assigned number 543.

A novelty game where the playfield is completely vertical. It has 4 flippers and the
  idea is to get the ball up to the alcohol 'still' before the 'revenuers' do. The
  idea didn't catch on, and the game was not officially released. 1 player.
  The display shows Score and Batch. There is no credit display.
  If the number of batches exceeds 9, the 'hidden digit' will show the tens.
  You cannot get more than 99 batches.
  The score only has 5 digits, but the game stores the 100,000 digit internally.

How to play: Press 5, press 1, press Z. Add points by pressing F,G,H,J. Press J
  a number of times to increment the batches. To end the game, hit X until the siren
  is heard. If you scored > 99999 points, the high score will show 99999.

Status:
- Playable

ToDo:
- Nothing

************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/m6800/m6800.h"
#include "audio/williams.h"
#include "machine/6821pia.h"
#include "speaker.h"

#include "s8a.lh"


namespace {

class s8a_state : public genpin_class
{
public:
	s8a_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s9sound(*this, "s9sound")
		, m_pia21(*this, "pia21")
		, m_pia24(*this, "pia24")
		, m_pia28(*this, "pia28")
		, m_pia30(*this, "pia30")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void s8a(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(main_nmi);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

private:
	void dig0_w(u8 data);
	void dig1_w(u8 data);
	void lamp0_w(u8 data);
	void lamp1_w(u8 data);
	void sol2_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[8U+i] = BIT(data, i); }; // solenoids 8-15
	void sol3_w(u8 data); // solenoids 0-7
	u8 switch_r();
	void switch_w(u8 data);
	DECLARE_READ_LINE_MEMBER(pia21_ca1_r);
	DECLARE_WRITE_LINE_MEMBER(pia21_cb2_w) { } // enable solenoids
	DECLARE_WRITE_LINE_MEMBER(pia24_cb2_w) { m_io_outputs[16] = state; } // not used
	DECLARE_WRITE_LINE_MEMBER(pia28_ca2_w) { } // comma3&4 (not used)
	DECLARE_WRITE_LINE_MEMBER(pia28_cb2_w) { } // comma1&2 (not used)
	DECLARE_WRITE_LINE_MEMBER(pia_irq);

	void main_map(address_map &map);

	u8 m_strobe = 0;
	u8 m_row = 0;
	bool m_data_ok = 0;
	u8 m_lamp_data = 0;
	emu_timer* m_irq_timer = 0;
	static const device_timer_id TIMER_IRQ = 0;
	required_device<m6802_cpu_device> m_maincpu;
	required_device<williams_s9_sound_device> m_s9sound;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia30;
	required_ioport_array<8> m_io_keyboard;
	output_finder<61> m_digits;
	output_finder<86> m_io_outputs; // 22 solenoids + 64 lamps
};

void s8a_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(s8a_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x6000, 0x7fff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( s8a )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Top L Flip") // INP04
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Bot L Flip") // INP05
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Top R Flip") // INP06
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Bot R Flip") // INP07
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Low L Drain") // INP08

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Lower Ramp Limit") // INP09 game mechanics are ready to start
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Top Drain") // INP10
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Middle R Drain") //INP11
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Level 1") // INP12
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Level 2") // INP13
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Level 3") // INP14
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("Level 4") // INP15
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Upper Ramp Limit") // INP16 revenuers have reached the still

	PORT_START("X2")
	PORT_START("X3")
	PORT_START("X4")
	PORT_START("X5")
	PORT_START("X6")
	PORT_START("X7")

	PORT_START("DIAGS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_0_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s8a_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Advance") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( s8a_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void s8a_state::sol3_w(u8 data)
{
	if (data==0x0a)
		m_samples->start(0, 7); // mechanical drum when you have 2 or more batches

	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

READ_LINE_MEMBER( s8a_state::pia21_ca1_r )
{
// sound busy
	return 1;
}

void s8a_state::lamp0_w(u8 data)
{
	m_lamp_data = data ^ 0xff;
}

void s8a_state::lamp1_w(u8 data)
{
	// find out which row is active
	for (u8 i = 0; i < 8; i++)
		if (BIT(data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[22U+i*8U+j] = BIT(m_lamp_data, j);
}

void s8a_state::dig0_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_data_ok = true;
	m_digits[60] = patterns[data>>4]; // diag digit
}

void s8a_state::dig1_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // MC14543
	if (m_data_ok)
	{
		m_digits[m_strobe+16] = patterns[data&15];
		m_digits[m_strobe] = patterns[data>>4];
	}
	m_data_ok = false;
}

u8 s8a_state::switch_r()
{
	u8 data = 0;
	// there's hardware for 8 rows, but machine uses 2
	for (u8 i = 0; i < 2; i++)
		if (BIT(m_row, i))
			data |= m_io_keyboard[i]->read();

	return data;
}

void s8a_state::switch_w(u8 data)
{
	m_row = data;
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

void s8a_state::device_timer(emu_timer &timer, device_timer_id id, int param)
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

void s8a_state::machine_start()
{
	genpin_class::machine_start();
	m_io_outputs.resolve();
	m_digits.resolve();

	save_item(NAME(m_strobe));
	save_item(NAME(m_row));
	save_item(NAME(m_data_ok));
	save_item(NAME(m_lamp_data));

	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(980,1e6),1);
}

void s8a_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}

void s8a_state::s8a(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_ram_enable(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &s8a_state::main_map);

	/* Video */
	config.set_default_layout(layout_s8a);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21, 0);
	m_pia21->set_port_a_input_overrides_output_mask(0xff);
	m_pia21->readca1_handler().set(FUNC(s8a_state::pia21_ca1_r));
	m_pia21->writepa_handler().set("s9sound", FUNC(williams_s9_sound_device::write));
	m_pia21->writepb_handler().set(FUNC(s8a_state::sol2_w));
	m_pia21->ca2_handler().set("s9sound", FUNC(williams_s9_sound_device::strobe));
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
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s8a_state::switch_w));
	m_pia30->irqa_handler().set(FUNC(s8a_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s8a_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Add the soundcard */
	SPEAKER(config, "mono").front_center();
	WILLIAMS_S9_SOUND(config, m_s9sound, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*----------------------------
/ Still Crazy (#543) 06/1984
/-----------------------------*/
ROM_START(scrzy_l1)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("ic20.bin", 0x0000, 0x2000, CRC(b0df42e6) SHA1(bb10268d7b820d1de0c20e1b79aba558badd072b) )

	ROM_REGION(0x8000, "s9sound:audiocpu", ROMREGION_ERASEFF)
	// 1st and 2nd halves are identical
	ROM_LOAD("ic49.bin", 0x4000, 0x4000, CRC(bcc8ccc4) SHA1(2312f9cc4f5a2dadfbfa61d13c31bb5838adf152) )
ROM_END

} // Anonymous namespace

GAME( 1984, scrzy_l1, 0, s8a, s8a, s8a_state, empty_init, ROT0, "Williams", "Still Crazy", MACHINE_IS_SKELETON_MECHANICAL )
