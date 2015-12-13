// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

    PINBALL
    Game Plan MPU-1
    These are "cocktail" cabinets, although there is only one seating position.

When first turned on, you need to press 9 to enter the setup program, then keep
pressing 9 until 05 shows in the balls display. Press the credit button to set
the first high score at which a free credit is awarded. Then press 9 to set the
2nd high score, then 9 to set the 3rd high score. Keep pressing 9 until you exit
back to normal operation. If this setup is not done, each player will get 3 free
games at the start of ball 1.


ToDo:
- Mechanical

********************************************************************************/

#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "sound/sn76477.h"
#include "gp_1.lh"

class gp_1_state : public genpin_class
{
public:
	gp_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
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
	{ }

	DECLARE_DRIVER_INIT(gp_1);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portas_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_READ8_MEMBER(portb_r);
	TIMER_DEVICE_CALLBACK_MEMBER(zero_timer);
private:
	UINT8 m_u14;
	UINT8 m_digit;
	UINT8 m_segment[16];
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
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
};


static ADDRESS_MAP_START( gp_1_map, AS_PROGRAM, 8, gp_1_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x8c00, 0x8cff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( gp_1_io, AS_IO, 8, gp_1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x0f)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi", i8255_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
ADDRESS_MAP_END

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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Accounting Reset") // This pushbutton on the MPU board is called "S33"
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_TILT1 ) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L and R Target") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinner C") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinner B") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Slingshot") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Special when lit") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Slingshot") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Extra when lit") PORT_CODE(KEYCODE_J)

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Spinner") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("1000 and advance") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Advance and Change") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Bumper") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("R. Bumper") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("L. Spinner") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Spinner A") PORT_CODE(KEYCODE_N)

	PORT_START("XA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("1000 Rollover") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("XB")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

READ8_MEMBER( gp_1_state::portb_r )
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

WRITE8_MEMBER( gp_1_state::porta_w )
{
	m_u14 = data >> 4;
	if ((data > 0x0f) && (data < 0x30))
	{
		switch (data)
		{
			case 0x10: // chime c
				m_samples->start(3, 3);
				break;
			case 0x11: // chime b
				m_samples->start(2, 2);
				break;
			case 0x12: // knocker
				m_samples->start(0, 6);
				break;
			case 0x13: // not used
			case 0x14: // not used
				break;
			case 0x15: // chime a
				m_samples->start(1, 1);
				break;
			case 0x16: // chime d
				m_samples->start(4, 4);
				break;
			case 0x17: // outhole
				m_samples->start(5, 5);
				break;
			case 0x18: // r sling
			case 0x19: // l sling
				m_samples->start(0, 7);
				break;
			case 0x1a: // C kickout
				m_samples->start(5, 5);
				break;
			case 0x1b: // r bumper
				m_samples->start(0, 0);
				break;
			case 0x1c: // B kickout
				m_samples->start(5, 5);
				break;
			case 0x1d: // l bumper
				m_samples->start(0, 0);
				break;
			case 0x1e: // A kickout
				m_samples->start(5, 5);
				break;
			case 0x1f: // not used
				break;
		}
	}

	static const UINT8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0 }; // 7448
	if (m_digit == 7)
		m_segment[m_u14] = data & 15; // only 8,9,10,11 are needed
	else
	if (m_u14 == 8)
	{
		output_set_digit_value(m_digit, patterns[m_segment[8]]);
		output_set_digit_value(m_digit+8, patterns[m_segment[9]]);
		output_set_digit_value(m_digit+16, patterns[m_segment[10]]);
		output_set_digit_value(m_digit+24, patterns[m_segment[11]]);
	}
}

WRITE8_MEMBER( gp_1_state::portas_w )
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

	porta_w(space, offset, data);
}

WRITE8_MEMBER( gp_1_state::portc_w )
{
	output_set_value("led0", !BIT(data, 3));
	m_digit = data & 7;
}

void gp_1_state::machine_reset()
{
	m_u14 = 0;
	m_digit = 0xff;
}

// zero-cross detection
TIMER_DEVICE_CALLBACK_MEMBER( gp_1_state::zero_timer )
{
	m_ctc->trg2(0);
	m_ctc->trg2(1);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

static MACHINE_CONFIG_START( gp_1, gp_1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2457600)
	MCFG_CPU_PROGRAM_MAP(gp_1_map)
	MCFG_CPU_IO_MAP(gp_1_io)
	MCFG_CPU_CONFIG(daisy_chain)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_gp_1)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("ppi", I8255A, 0 )
	MCFG_I8255_OUT_PORTA_CB(WRITE8(gp_1_state, porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(gp_1_state, portb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(gp_1_state, portc_w))

	MCFG_DEVICE_ADD("ctc", Z80CTC, 2457600 )
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0)) // Todo: absence of ints will cause a watchdog reset
	MCFG_TIMER_DRIVER_ADD_PERIODIC("gp1", gp_1_state, zero_timer, attotime::from_hz(120)) // mains freq*2
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gp_1s, gp_1 )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                         // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, 0)                  // attack_decay_cap + attack_res: N/C
	MCFG_SN76477_AMP_RES(RES_K(220))                  // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))              // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(56)) // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                   // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(220))   // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                 // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(0)                          // VCO mode: N/C
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                // mixer A, B, C: N/C
	MCFG_SN76477_ENVELOPE_PARAMS(0, 1)                // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                            // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_REMOVE("ppi")
	MCFG_DEVICE_ADD("ppi", I8255A, 0 )
	MCFG_I8255_OUT_PORTA_CB(WRITE8(gp_1_state, portas_w))
	MCFG_I8255_IN_PORTB_CB(READ8(gp_1_state, portb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(gp_1_state, portc_w))
MACHINE_CONFIG_END


ROM_START( gp_110 )
	ROM_REGION(0x1000, "roms", 0)
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
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "family.u12", 0x0000, 0x0800, CRC(98f27fdf) SHA1(8bcff1e13b9b978f91110f1e83a3288723930a1d))
	ROM_LOAD( "family.u13", 0x0800, 0x0800, CRC(b941a1a8) SHA1(a43f8acadb3db3e2274162d5305e30006f912339))
ROM_END

/*-------------------------------------------------------------------
/ Star Trip (April 1979) - Model: Cocktail #120
/-------------------------------------------------------------------*/
ROM_START(startrip)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "startrip.u12", 0x0000, 0x0800, CRC(98f27fdf) SHA1(8bcff1e13b9b978f91110f1e83a3288723930a1d))
	ROM_LOAD( "startrip.u13", 0x0800, 0x0800, CRC(b941a1a8) SHA1(a43f8acadb3db3e2274162d5305e30006f912339))
ROM_END

/*-------------------------------------------------------------------
/ Vegas (August 1979) - Cocktail Model #140
/-------------------------------------------------------------------*/
ROM_START(vegasgp)
	ROM_REGION(0x1000, "roms", 0)
	ROM_LOAD( "140a.12", 0x0000, 0x0800, CRC(2c00bc19) SHA1(521d4b44f46dea0a08e90cd3aea5799462215863))
	ROM_LOAD( "140b.13", 0x0800, 0x0800, CRC(cf26d67b) SHA1(05481e880e23a7bc1d1716b52ac1effc0db437f2))
ROM_END

GAME(1978, gp_110,   0,        gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Model 110",    MACHINE_IS_BIOS_ROOT)
GAME(1978, blvelvet, gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Black Velvet", MACHINE_MECHANICAL)
GAME(1978, camlight, gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Camel Lights", MACHINE_MECHANICAL)
GAME(1978, foxylady, gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Foxy Lady",    MACHINE_MECHANICAL)
GAME(1978, real,     gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Real",         MACHINE_MECHANICAL)
GAME(1978, rio,      gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Rio",          MACHINE_MECHANICAL)
GAME(1978, chucklck, gp_110,   gp_1,     gp_1,     driver_device, 0,   ROT0, "Game Plan", "Chuck-A-Luck", MACHINE_MECHANICAL)
GAME(1979, famlyfun, 0,        gp_1s,    gp_1,     driver_device, 0,   ROT0, "Game Plan", "Family Fun!",  MACHINE_MECHANICAL)
GAME(1979, startrip, 0,        gp_1s,    gp_1,     driver_device, 0,   ROT0, "Game Plan", "Star Trip",    MACHINE_MECHANICAL)
GAME(1979, vegasgp,  0,        gp_1s,    gp_1,     driver_device, 0,   ROT0, "Game Plan", "Vegas (Game Plan)", MACHINE_MECHANICAL)
