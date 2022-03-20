// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

PINBALL
Game Plan MPU-1
These are "cocktail" cabinets, although there is only one seating position.

When first turned on, you need to press num-0 to enter the setup program, then keep
pressing num-0 until 05 shows in the balls display. Press the credit button to set
the first high score at which a free credit is awarded. Then press num-0 to set the
2nd high score, then num-0 to set the 3rd high score. Keep pressing num-0 until you exit
back to normal operation. If this setup is not done, each player will get 3 free
games at the start of ball 1.

Status
- Working

ToDo:
- Nothing

********************************************************************************/

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/i8255.h"
#include "machine/clock.h"
#include "machine/z80ctc.h"
#include "sound/sn76477.h"
#include "speaker.h"
#include "gp_1.lh"

namespace {

class gp_1_state : public genpin_class
{
public:
	gp_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_ctc(*this, "ctc")
		, m_sn(*this, "snsnd")
		, m_io_dsw0(*this, "DSW0")
		, m_io_dsw1(*this, "DSW1")
		, m_io_dsw2(*this, "DSW2")
		, m_io_dsw3(*this, "DSW3")
		, m_io_x7(*this, "X7")
		, m_io_x8(*this, "X8")
		, m_io_x9(*this, "X9")
		, m_io_xa(*this, "XA")
		, m_io_xb(*this, "XB")
		, m_digits(*this, "digit%d", 0U)
		, m_io_leds(*this, "led%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void gp_1(machine_config &config);
	void gp_1s(machine_config &config);

private:
	void porta_w(u8 data);
	void portas_w(u8 data);
	void portc_w(u8 data);
	u8 portb_r();
	void io_map(address_map &map);
	void mem_map(address_map &map);
	u8 m_u14 = 0U;
	u8 m_digit = 0U;
	u8 m_segment[16]{};
	u8 m_last_solenoid = 15U;
	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_device<z80_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<z80ctc_device> m_ctc;
	optional_device<sn76477_device> m_sn;
	required_ioport m_io_dsw0;
	required_ioport m_io_dsw1;
	required_ioport m_io_dsw2;
	required_ioport m_io_dsw3;
	required_ioport m_io_x7;
	required_ioport m_io_x8;
	required_ioport m_io_x9;
	required_ioport m_io_xa;
	required_ioport m_io_xb;
	output_finder<40> m_digits;
	output_finder<1> m_io_leds;
	output_finder<64> m_io_outputs;   // 16 solenoids + 48 lamps
};


void gp_1_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x8c00, 0x8cff).ram().share("nvram");
}

void gp_1_state::io_map(address_map &map)
{
	map.global_mask(0x0f);
	map(0x04, 0x07).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

static INPUT_PORTS_START( gp_1 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 1")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C )) // same as 01
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x13, "2 coins 9 credits")
	PORT_DIPSETTING(    0x14, "1 coin 10 credits")
	PORT_DIPSETTING(    0x15, "2 coins 10 credits")
	PORT_DIPSETTING(    0x16, "1 coin 11 credits")
	PORT_DIPSETTING(    0x17, "2 coins 11 credits")
	PORT_DIPSETTING(    0x18, "1 coin 12 credits")
	PORT_DIPSETTING(    0x19, "2 coins 12 credits")
	PORT_DIPSETTING(    0x1a, "1 coin 13 credits")
	PORT_DIPSETTING(    0x1b, "2 coins 13 credits")
	PORT_DIPSETTING(    0x1c, "1 coin 14 credits")
	PORT_DIPSETTING(    0x1d, "2 coins 14 credits")
	PORT_DIPSETTING(    0x1e, "1 coin 15 credits")
	PORT_DIPSETTING(    0x1f, "2 coins 15 credits")
	PORT_DIPNAME( 0x20, 0x00, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Free Play")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, "Coin Slot 2") // S09-12 determine coinage for slot 2
	PORT_DIPSETTING(    0x00, "Same as Slot 1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x0a, "1 coin 10 credits")
	PORT_DIPSETTING(    0x0b, "1 coin 11 credits")
	PORT_DIPSETTING(    0x0c, "1 coin 12 credits")
	PORT_DIPSETTING(    0x0d, "1 coin 13 credits")
	PORT_DIPSETTING(    0x0e, "1 coin 14 credits")
	PORT_DIPSETTING(    0x0f, "1 coin 15 credits")
	PORT_DIPNAME( 0x10, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x10, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Play Tunes")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x1f, 0x02, "Coin Slot 3")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C )) // same as 01
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_4C ))
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_5C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ))
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ))
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ))
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_8C ))
	PORT_DIPSETTING(    0x11, DEF_STR( 2C_8C ))
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_9C ))
	PORT_DIPSETTING(    0x13, "2 coins 9 credits")
	PORT_DIPSETTING(    0x14, "1 coin 10 credits")
	PORT_DIPSETTING(    0x15, "2 coins 10 credits")
	PORT_DIPSETTING(    0x16, "1 coin 11 credits")
	PORT_DIPSETTING(    0x17, "2 coins 11 credits")
	PORT_DIPSETTING(    0x18, "1 coin 12 credits")
	PORT_DIPSETTING(    0x19, "2 coins 12 credits")
	PORT_DIPSETTING(    0x1a, "1 coin 13 credits")
	PORT_DIPSETTING(    0x1b, "2 coins 13 credits")
	PORT_DIPSETTING(    0x1c, "1 coin 14 credits")
	PORT_DIPSETTING(    0x1d, "2 coins 14 credits")
	PORT_DIPSETTING(    0x1e, "1 coin 15 credits")
	PORT_DIPSETTING(    0x1f, "2 coins 15 credits")
	PORT_DIPNAME( 0x20, 0x00, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "S24")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x02, "Max number of credits")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10")
	PORT_DIPSETTING(    0x02, "15")
	PORT_DIPSETTING(    0x03, "20")
	PORT_DIPSETTING(    0x04, "25")
	PORT_DIPSETTING(    0x05, "30")
	PORT_DIPSETTING(    0x06, "35")
	PORT_DIPSETTING(    0x07, "40")
	PORT_DIPNAME( 0x08, 0x00, "Balls")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPSETTING(    0x08, "5")
	PORT_DIPNAME( 0x10, 0x10, "Award")
	PORT_DIPSETTING(    0x00, "Extra Ball")
	PORT_DIPSETTING(    0x10, "Replay")
	PORT_DIPNAME( 0x20, 0x20, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x20, DEF_STR( On ))
	PORT_DIPNAME( 0xC0, 0x80, "Credits for exceeding high score")
	PORT_DIPSETTING(    0x00, "0")
	PORT_DIPSETTING(    0x40, "1")
	PORT_DIPSETTING(    0x80, "2")
	PORT_DIPSETTING(    0xC0, "3")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Accounting Reset") // This pushbutton on the MPU board is called "S33"
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L and R Target") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Spinner C") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Spinner B") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R. Slingshot") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Special when lit") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L. Slingshot") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Extra when lit") PORT_CODE(KEYCODE_G)

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R. Spinner") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("1000 and advance") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Advance and Change") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L. Bumper") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("R. Bumper") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("L. Spinner") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Spinner A") PORT_CODE(KEYCODE_N)

	PORT_START("XA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("1000 Rollover") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Setup")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("XB")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

u8 gp_1_state::portb_r()
{
	switch (m_u14)
	{
		case 7:
			return m_io_x7->read();
		case 8:
			return m_io_x8->read();
		case 9:
			return m_io_x9->read();
		case 10:
			return m_io_xa->read();
		case 11:
			return m_io_xb->read();
		case 12:
			return m_io_dsw0->read();
		case 13:
			return m_io_dsw1->read();
		case 14:
			return m_io_dsw2->read();
		case 15:
			return m_io_dsw3->read();
	}
	return 0;
}

void gp_1_state::porta_w(u8 data)
{
	if (m_last_solenoid < 15)
		m_io_outputs[m_last_solenoid] = 0;

	m_u14 = data >> 4;
	if ((m_u14 >= 1) && (m_u14 <= 2))
	{
		switch (data)
		{
			case 0x10: // chime c
				m_samples->start(1, 1);
				break;
			case 0x11: // chime b
				m_samples->start(2, 2);
				break;
			case 0x12: // knocker
				m_samples->start(0, 6);
				break;
			case 0x15: // chime a
				m_samples->start(3, 3);
				break;
			case 0x16: // chime d
				m_samples->start(4, 4);
				break;
			case 0x17: // outhole
				m_samples->start(0, 5);
				break;
			case 0x18: // r sling
			case 0x19: // l sling
				m_samples->start(5, 7);
				break;
			case 0x1a: // C kickout
				m_samples->start(5, 5);
				break;
			case 0x1b: // r bumper
				m_samples->start(5, 0);
				break;
			case 0x1c: // B kickout
				m_samples->start(5, 5);
				break;
			case 0x1d: // l bumper
				m_samples->start(5, 0);
				break;
			case 0x1e: // A kickout
				m_samples->start(5, 5);
				break;
			default:
				data = 15;
				break;
		}
		if ((data & 15) < 15)
			m_io_outputs[data & 15] = 1;
		m_last_solenoid = data & 15;
	}
	else
		m_last_solenoid = 15;

	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0 }; // 7448
	if (m_digit == 7)
		m_segment[m_u14] = data & 15; // only 8,9,10,11 are needed
	else
	if (m_u14 == 8)
	{
		m_digits[m_digit] = patterns[m_segment[8]];
		m_digits[m_digit+8] = patterns[m_segment[9]];
		m_digits[m_digit+16] = patterns[m_segment[10]];
		m_digits[m_digit+24] = patterns[m_segment[11]];
	}

	// Lamps
	if ((m_u14 >= 3) && (m_u14 <= 5))
		for (u8 i = 0; i < 16; i++)
			m_io_outputs[(m_u14 - 3) * 16 + 16 + i] = ((data & 15) == i);
}

void gp_1_state::portas_w(u8 data)
{
	m_u14 = data >> 4;
	if (m_u14 == 1)
	{
		switch (data)
		{
			case 0x10: // chime c
				m_sn->vco_voltage_w(0.45);
				m_sn->enable_w(0);
				data = 0x1f;
				break;
			case 0x11: // chime b
				m_sn->vco_voltage_w(0.131);
				m_sn->enable_w(0);
				data = 0x1f;
				break;
			case 0x15: // chime a
				m_sn->vco_voltage_w(0.07);
				m_sn->enable_w(0);
				data = 0x1f;
				break;
			case 0x16: // chime d
				m_sn->vco_voltage_w(2.25);
				m_sn->enable_w(0);
				data = 0x1f;
				break;
			default:
				m_sn->enable_w(1);
		}
	}

	porta_w(data);
}

void gp_1_state::portc_w(u8 data)
{
	m_io_leds[0] = BIT(data, 3) ? 0 : 1;
	m_digit = data & 7;
}

void gp_1_state::machine_start()
{
	m_digits.resolve();
	m_io_leds.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_u14));
	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
	save_item(NAME(m_last_solenoid));
}

void gp_1_state::machine_reset()
{
	m_u14 = 0;
	m_digit = 0xff;
	m_last_solenoid = 15;
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
	for (u8 i = 0; i < std::size(m_segment); i++)
		m_segment[i] = 0;
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

void gp_1_state::gp_1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2457600);
	m_maincpu->set_addrmap(AS_PROGRAM, &gp_1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &gp_1_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_gp_1);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	I8255A(config, m_ppi);
	m_ppi->out_pa_callback().set(FUNC(gp_1_state::porta_w));
	m_ppi->in_pb_callback().set(FUNC(gp_1_state::portb_r));
	m_ppi->out_pc_callback().set(FUNC(gp_1_state::portc_w));

	Z80CTC(config, m_ctc, 2457600);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0); // Todo: absence of ints will cause a watchdog reset

	clock_device &cpoint_clock(CLOCK(config, "cpoint_clock", 120)); // crosspoint detector
	cpoint_clock.signal_handler().set(m_ctc, FUNC(z80ctc_device::trg2));
}

void gp_1_state::gp_1s(machine_config &config)
{
	gp_1(config);
	SPEAKER(config, "mono").front_center();
	SN76477(config, m_sn);
	m_sn->set_noise_params(0, 0, 0);
	m_sn->set_decay_res(0);
	m_sn->set_attack_params(0, 0);
	m_sn->set_amp_res(RES_K(220));
	m_sn->set_feedback_res(RES_K(47));
	m_sn->set_vco_params(0, CAP_U(0.1), RES_K(56));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(1.0), RES_K(220));
	m_sn->set_oneshot_params(0, 0);
	m_sn->set_vco_mode(0);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(0, 1);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);

	m_ppi->out_pa_callback().set(FUNC(gp_1_state::portas_w));
}


ROM_START( gp_110 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "a-110.u12", 0x0000, 0x0800, CRC(ed0d518b) SHA1(8f3ca8792ad907c660d9149a1aa3a3528c7573e3))
	ROM_LOAD( "b1-110.u13", 0x0800, 0x0800, CRC(a223f2e8) SHA1(767e15e19e11399935c890c1d1034dccf1ad7f92))
ROM_END


/*-------------------------------------------------------------------
/ Black Velvet (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_blvelvet    rom_gp_110
/*-------------------------------------------------------------------
/ Camel Lights (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_camlight    rom_gp_110
/*-------------------------------------------------------------------
/ Foxy Lady (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_foxylady    rom_gp_110
/*-------------------------------------------------------------------
/ Real (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_real    rom_gp_110
/*-------------------------------------------------------------------
/ Rio (May 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_rio    rom_gp_110

/*-------------------------------------------------------------------
/ Chuck-A-Luck (October 1978) - Model: Cocktail #110
/-------------------------------------------------------------------*/
#define rom_chucklck    rom_gp_110

/*-------------------------------------------------------------------
/ Family Fun! (April 1979) - Model: Cocktail #120
/-------------------------------------------------------------------*/
ROM_START(famlyfun)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "family.u12", 0x0000, 0x0800, CRC(98f27fdf) SHA1(8bcff1e13b9b978f91110f1e83a3288723930a1d))
	ROM_LOAD( "family.u13", 0x0800, 0x0800, CRC(b941a1a8) SHA1(a43f8acadb3db3e2274162d5305e30006f912339))
ROM_END

/*-------------------------------------------------------------------
/ Star Trip (April 1979) - Model: Cocktail #120
/-------------------------------------------------------------------*/
ROM_START(startrip)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "startrip.u12", 0x0000, 0x0800, CRC(98f27fdf) SHA1(8bcff1e13b9b978f91110f1e83a3288723930a1d))
	ROM_LOAD( "startrip.u13", 0x0800, 0x0800, CRC(b941a1a8) SHA1(a43f8acadb3db3e2274162d5305e30006f912339))
ROM_END

/*-------------------------------------------------------------------
/ Vegas (August 1979) - Cocktail Model #140
/-------------------------------------------------------------------*/
ROM_START(vegasgp)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD( "140a.12", 0x0000, 0x0800, CRC(2c00bc19) SHA1(521d4b44f46dea0a08e90cd3aea5799462215863))
	ROM_LOAD( "140b.13", 0x0800, 0x0800, CRC(cf26d67b) SHA1(05481e880e23a7bc1d1716b52ac1effc0db437f2))
ROM_END

} // anonymous namespace

GAME(1978, gp_110,   0,      gp_1,  gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Model 110",         MACHINE_IS_BIOS_ROOT | MACHINE_NOT_WORKING)

// Chimes
GAME(1978, blvelvet, gp_110, gp_1,  gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Black Velvet",      MACHINE_IS_SKELETON_MECHANICAL )
GAME(1978, camlight, gp_110, gp_1,  gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Camel Lights",      MACHINE_IS_SKELETON_MECHANICAL )
GAME(1978, foxylady, gp_110, gp_1,  gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Foxy Lady",         MACHINE_IS_SKELETON_MECHANICAL )
GAME(1978, real,     gp_110, gp_1,  gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Real",              MACHINE_IS_SKELETON_MECHANICAL )
GAME(1978, rio,      gp_110, gp_1,  gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Rio",               MACHINE_IS_SKELETON_MECHANICAL )
GAME(1978, chucklck, gp_110, gp_1,  gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Chuck-A-Luck",      MACHINE_IS_SKELETON_MECHANICAL )

// SN76477 sound
GAME(1979, famlyfun, 0,      gp_1s, gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Family Fun!",       MACHINE_IS_SKELETON_MECHANICAL )
GAME(1979, startrip, 0,      gp_1s, gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Star Trip",         MACHINE_IS_SKELETON_MECHANICAL )
GAME(1979, vegasgp,  0,      gp_1s, gp_1, gp_1_state, empty_init, ROT0, "Game Plan", "Vegas (Game Plan)", MACHINE_IS_SKELETON_MECHANICAL )
