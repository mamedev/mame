// license:GPL2+
// copyright-holders:FelipeSanches, Sandro Ronco
//
// Wacky Gator
//
// Driver by Sandro Ronco and
//  Felipe Correa da Silva Sanches <juca@members.fsf.org>
//

/*
   Most of this driver is based on guessing since I do not have access to the actual pcb for this game.
   Once we get a pcb, please review the correctness of the code in this driver before deleting this comment.

   TODO:
   - IRQ and NMI sources are unknown
   - proper PMM8713 and steppers emulation
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/ticket.h"
#include "sound/2413intf.h"
#include "sound/msm5205.h"

#include "wackygtr.lh"


class wackygtr_state : public driver_device
{
public:
	wackygtr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_pit8253_0(*this, "pit8253_0"),
		m_pit8253_1(*this, "pit8253_1"),
		m_ticket(*this, "ticket"),
		m_samples(*this, "oki")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<pit8253_device> m_pit8253_0;
	required_device<pit8253_device> m_pit8253_1;
	required_device<ticket_dispenser_device> m_ticket;
	required_memory_region m_samples;

	DECLARE_DRIVER_INIT(wackygtr);
	void machine_reset() override;

	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
	DECLARE_WRITE8_MEMBER(sample_ctrl_w);
	DECLARE_WRITE8_MEMBER(alligators_ctrl1_w);
	DECLARE_WRITE8_MEMBER(alligators_ctrl2_w);
	DECLARE_CUSTOM_INPUT_MEMBER(alligators_rear_sensors_r);
	DECLARE_CUSTOM_INPUT_MEMBER(alligators_front_sensors_r);

	void set_lamps(int p, UINT8 value);
	DECLARE_WRITE8_MEMBER(status_lamps_w);
	DECLARE_WRITE8_MEMBER(timing_lamps_0_w)     { set_lamps(8 , data); }
	DECLARE_WRITE8_MEMBER(timing_lamps_1_w)     { set_lamps(16, data); }
	DECLARE_WRITE8_MEMBER(timing_lamps_2_w)     { set_lamps(24, data); }

	void set_digits(int p, UINT8 value);
	DECLARE_WRITE8_MEMBER(disp0_w)              { set_digits(0, data); }
	DECLARE_WRITE8_MEMBER(disp1_w)              { set_digits(2, data); }
	DECLARE_WRITE8_MEMBER(disp2_w)              { set_digits(4, data); }
	DECLARE_WRITE8_MEMBER(disp3_w)              { set_digits(6, data); }

	void pmm8713_ck(int i, int state);
	DECLARE_WRITE_LINE_MEMBER(alligator0_ck)    { pmm8713_ck(0, state); }
	DECLARE_WRITE_LINE_MEMBER(alligator1_ck)    { pmm8713_ck(1, state); }
	DECLARE_WRITE_LINE_MEMBER(alligator2_ck)    { pmm8713_ck(2, state); }
	DECLARE_WRITE_LINE_MEMBER(alligator3_ck)    { pmm8713_ck(3, state); }
	DECLARE_WRITE_LINE_MEMBER(alligator4_ck)    { pmm8713_ck(4, state); }

	DECLARE_WRITE8_MEMBER(irq_ack_w)            { m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE); }
	DECLARE_WRITE8_MEMBER(firq_ack_w)           { m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE); }

	TIMER_DEVICE_CALLBACK_MEMBER(nmi_timer)     { m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE); }

private:
	int     m_adpcm_sel;
	UINT16  m_adpcm_pos;
	UINT8   m_adpcm_ctrl;

	UINT8   m_alligators_ctrl;
	int     m_motors_pos[5];
};


DRIVER_INIT_MEMBER(wackygtr_state, wackygtr)
{
}

WRITE8_MEMBER(wackygtr_state::status_lamps_w)
{
	/*
	    ---x xxxx   status lamps
	    --x- ----   game over lamp
	    -x-- ----   coin counter
	    x--- ----   ticket dispenser
	*/

	set_lamps(0, data & 0x3f);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	m_ticket->write(space, 0, data & 0x80);
}

WRITE8_MEMBER(wackygtr_state::sample_ctrl_w)
{
	/*
	    --xx xxxx    sample index
	    -x-- ----    ???
	    x--- ----    5205 reset
	*/

	m_adpcm_ctrl = data;
	m_adpcm_pos = (data & 0x3f) * 0x400;
	m_adpcm_sel = 0;
	m_msm->reset_w(BIT(data, 7));
}

WRITE8_MEMBER(wackygtr_state::alligators_ctrl1_w)
{
	m_pit8253_0->write_gate0(BIT(data, 0));
	m_pit8253_0->write_gate1(BIT(data, 1));
	m_pit8253_0->write_gate2(BIT(data, 2));
	m_pit8253_1->write_gate1(BIT(data, 3));
	m_pit8253_1->write_gate2(BIT(data, 4));

	machine().bookkeeping().coin_lockout_w(0, data & 0x40 ? 0 : 1);
}

WRITE8_MEMBER(wackygtr_state::alligators_ctrl2_w)
{
	/*
	    ---- ---x    PMM8713 0 U/D
	    ---- --x-    PMM8713 1 U/D
	    ---- -x--    PMM8713 2 U/D
	    ---- x---    PMM8713 3 U/D
	    ---x ----    PMM8713 4 U/D
	*/

	m_alligators_ctrl = data & 0x1f;
}

void wackygtr_state::pmm8713_ck(int i, int state)
{
	if (state)
	{
		m_motors_pos[i] += (BIT(m_alligators_ctrl, i) ? +1 : -1);

		int alligator_state = m_motors_pos[i] / 10;
		if (alligator_state > 5)    alligator_state = 5;
		if (alligator_state < 0)    alligator_state = 0;
		output().set_indexed_value("alligator", i, alligator_state);
	}
}

CUSTOM_INPUT_MEMBER(wackygtr_state::alligators_rear_sensors_r)
{
	return  ((m_motors_pos[0] < 10) ? 0x01 : 0) |
			((m_motors_pos[1] < 10) ? 0x02 : 0) |
			((m_motors_pos[2] < 10) ? 0x04 : 0) |
			((m_motors_pos[3] < 10) ? 0x08 : 0) |
			((m_motors_pos[4] < 10) ? 0x10 : 0) |
			(m_alligators_ctrl ^ 0x1f);
}

CUSTOM_INPUT_MEMBER(wackygtr_state::alligators_front_sensors_r)
{
	return  ((m_motors_pos[0] < 5 || m_motors_pos[0] > 55) ? 0x01 : 0) |
			((m_motors_pos[1] < 5 || m_motors_pos[1] > 55) ? 0x02 : 0) |
			((m_motors_pos[2] < 5 || m_motors_pos[2] > 55) ? 0x04 : 0) |
			((m_motors_pos[3] < 5 || m_motors_pos[3] > 55) ? 0x08 : 0) |
			((m_motors_pos[4] < 5 || m_motors_pos[4] > 55) ? 0x10 : 0);
}

void wackygtr_state::machine_reset()
{
	m_adpcm_pos = 0;
	m_adpcm_sel = 0;
	m_adpcm_ctrl = 0x80;
}

void wackygtr_state::set_digits(int p, UINT8 value)
{
	static UINT8 bcd2hex[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 };  // not accurate
	output().set_digit_value(p + 0, bcd2hex[value & 0x0f]);
	output().set_digit_value(p + 1, bcd2hex[(value >> 4) & 0x0f]);
}

void wackygtr_state::set_lamps(int p, UINT8 value)
{
	for(int i=0; i<8; i++)
		output().set_lamp_value(p + i, BIT(value, i));
}

static INPUT_PORTS_START( wackygtr )
	PORT_START("IN0")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_SPECIAL)  PORT_CUSTOM_MEMBER(DEVICE_SELF, wackygtr_state, alligators_rear_sensors_r, NULL)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("IN1")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_SPECIAL)   PORT_CUSTOM_MEMBER(DEVICE_SELF, wackygtr_state, alligators_front_sensors_r, NULL)
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5)
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Test ) ) PORT_DIPLOCATION("SW:4")   // For factory Test use ONLY! Do not change
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) ) PORT_DIPLOCATION("SW:6")   // For factory Test use ONLY! Do not change
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
INPUT_PORTS_END

WRITE_LINE_MEMBER(wackygtr_state::adpcm_int)
{
	if (!(m_adpcm_ctrl & 0x80))
	{
		UINT8 data = m_samples->base()[m_adpcm_pos & 0xffff];
		m_msm->data_w((m_adpcm_sel ? data : (data >> 4)) & 0x0f);
		m_adpcm_pos += m_adpcm_sel;
		m_adpcm_sel ^= 1;
	}
}

static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, wackygtr_state )
	AM_RANGE(0x0200, 0x0200) AM_READNOP AM_WRITE(irq_ack_w)
	AM_RANGE(0x0400, 0x0400) AM_READNOP AM_WRITE(firq_ack_w)
	AM_RANGE(0x0600, 0x0600) AM_WRITE(disp0_w)
	AM_RANGE(0x0800, 0x0800) AM_WRITE(disp1_w)
	AM_RANGE(0x0a00, 0x0a00) AM_WRITE(disp2_w)
	AM_RANGE(0x0c00, 0x0c00) AM_WRITE(disp3_w)
	AM_RANGE(0x0e00, 0x0e00) AM_WRITE(sample_ctrl_w)

	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ymsnd", ym2413_device, write)

	AM_RANGE(0x2000, 0x2003) AM_DEVREADWRITE("pit8253_0", pit8253_device, read, write)
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("pit8253_1", pit8253_device, read, write)

	AM_RANGE(0x4000, 0x4003) AM_DEVREADWRITE("i8255_0", i8255_device, read, write)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("i8255_1", i8255_device, read, write)
	AM_RANGE(0x6000, 0x6003) AM_DEVREADWRITE("i8255_2", i8255_device, read, write)

	AM_RANGE(0x7000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static MACHINE_CONFIG_START( wackygtr, wackygtr_state )

	MCFG_CPU_ADD("maincpu", M6809E, XTAL_3_579545MHz)   // HD68B09P
	MCFG_CPU_PROGRAM_MAP(program_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(wackygtr_state, irq0_line_assert, 50)  // FIXME

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmi_timer", wackygtr_state, nmi_timer, attotime::from_hz(100))  // FIXME

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_wackygtr)

	/* Sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz )
	MCFG_MSM5205_VCLK_CB(WRITELINE(wackygtr_state, adpcm_int))   /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 KHz, 4 Bits  */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_ADD("i8255_0", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(wackygtr_state, status_lamps_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(wackygtr_state, alligators_ctrl1_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(wackygtr_state, alligators_ctrl2_w))

	MCFG_DEVICE_ADD("i8255_1", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(wackygtr_state, timing_lamps_0_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(wackygtr_state, timing_lamps_1_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(wackygtr_state, timing_lamps_2_w))

	MCFG_DEVICE_ADD("i8255_2", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_DEVICE_ADD("pit8253_0", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_3_579545MHz/16)  // this is a guess
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(wackygtr_state, alligator0_ck))
	MCFG_PIT8253_CLK1(XTAL_3_579545MHz/16)  // this is a guess
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(wackygtr_state, alligator1_ck))
	MCFG_PIT8253_CLK2(XTAL_3_579545MHz/16)  // this is a guess
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(wackygtr_state, alligator2_ck))

	MCFG_DEVICE_ADD("pit8253_1", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_3_579545MHz/16)  // this is a guess
	MCFG_PIT8253_OUT0_HANDLER(INPUTLINE("maincpu", M6809_FIRQ_LINE))
	MCFG_PIT8253_CLK1(XTAL_3_579545MHz/16)  // this is a guess
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(wackygtr_state, alligator3_ck))
	MCFG_PIT8253_CLK2(XTAL_3_579545MHz/16)  // this is a guess
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(wackygtr_state, alligator4_ck))

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(200), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH)
MACHINE_CONFIG_END


ROM_START( wackygtr )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("wp3-pr0.4d", 0x8000, 0x8000, CRC(71ca4437) SHA1(c7d948c5593e6053fd0a65601f6c06871f5861f0))

	ROM_REGION(0x10000, "oki", 0)
	ROM_LOAD("wp3-vo0.2h", 0x0000, 0x10000, CRC(91c7986f) SHA1(bc9fa0d41c1caa0f909a349f511d022b7e42c6cd))
ROM_END

GAME(1990, wackygtr,    0, wackygtr,  wackygtr, wackygtr_state, wackygtr,  ROT0,   "Data East", "Wacky Gator", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_CLICKABLE_ARTWORK)
