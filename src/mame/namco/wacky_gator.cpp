// license:GPL-2.0+
// copyright-holders:FelipeSanches, Sandro Ronco
//
// Namco Gator Panic (Wani Wani Panic)
// USA version distributed by Data East and renamed "Wacky Gator"
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
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"
#include "speaker.h"

#include "wackygtr.lh"


namespace {

class wackygtr_state : public driver_device
{
public:
	wackygtr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_pit8253(*this, "pit8253%u", 0U),
		m_ticket(*this, "ticket"),
		m_samples(*this, "oki"),
		m_alligator(*this, "alligator%u", 0U),
		m_digit(*this, "digit%u", 0U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	ioport_value alligators_rear_sensors_r();
	ioport_value alligators_front_sensors_r();

	void wackygtr(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void adpcm_int(int state);
	void sample_ctrl_w(uint8_t data);
	void alligators_ctrl1_w(uint8_t data);
	void alligators_ctrl2_w(uint8_t data);

	void set_lamps(int p, uint8_t value);
	void status_lamps_w(uint8_t data);
	template <unsigned N> void timing_lamps_w(uint8_t data) { set_lamps((N + 1) << 3, data); }

	void set_digits(int p, uint8_t value);
	template <unsigned N> void disp_w(uint8_t data) { set_digits(N << 1, data); }

	void pmm8713_ck(int i, int state);
	template <unsigned N> void alligator_ck(int state) { pmm8713_ck(N, state); }

	void irq_ack_w(uint8_t data)            { m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE); }
	void firq_ack_w(uint8_t data)           { m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE); }

	TIMER_DEVICE_CALLBACK_MEMBER(nmi_timer)     { m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero); }

	void program_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device_array<pit8253_device, 2> m_pit8253;
	required_device<ticket_dispenser_device> m_ticket;
	required_memory_region m_samples;
	output_finder<5> m_alligator;
	output_finder<8> m_digit;
	output_finder<32> m_lamps;

	int     m_adpcm_sel;
	uint16_t  m_adpcm_pos;
	uint8_t   m_adpcm_ctrl;

	uint8_t   m_alligators_ctrl;
	int       m_motors_pos[5] = { };
};


void wackygtr_state::status_lamps_w(uint8_t data)
{
	/*
	    ---x xxxx   status lamps
	    --x- ----   game over lamp
	    -x-- ----   coin counter
	    x--- ----   ticket dispenser
	*/

	set_lamps(0, data & 0x3f);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	m_ticket->motor_w(BIT(data, 7));
}

void wackygtr_state::sample_ctrl_w(uint8_t data)
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

void wackygtr_state::alligators_ctrl1_w(uint8_t data)
{
	m_pit8253[0]->write_gate0(BIT(data, 0));
	m_pit8253[0]->write_gate1(BIT(data, 1));
	m_pit8253[0]->write_gate2(BIT(data, 2));
	m_pit8253[1]->write_gate1(BIT(data, 3));
	m_pit8253[1]->write_gate2(BIT(data, 4));

	machine().bookkeeping().coin_lockout_w(0, data & 0x40 ? 0 : 1);
}

void wackygtr_state::alligators_ctrl2_w(uint8_t data)
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
		m_alligator[i] = alligator_state;
	}
}

ioport_value wackygtr_state::alligators_rear_sensors_r()
{
	return  ((m_motors_pos[0] < 10) ? 0x01 : 0) |
			((m_motors_pos[1] < 10) ? 0x02 : 0) |
			((m_motors_pos[2] < 10) ? 0x04 : 0) |
			((m_motors_pos[3] < 10) ? 0x08 : 0) |
			((m_motors_pos[4] < 10) ? 0x10 : 0) |
			(m_alligators_ctrl ^ 0x1f);
}

ioport_value wackygtr_state::alligators_front_sensors_r()
{
	return  ((m_motors_pos[0] < 5 || m_motors_pos[0] > 55) ? 0x01 : 0) |
			((m_motors_pos[1] < 5 || m_motors_pos[1] > 55) ? 0x02 : 0) |
			((m_motors_pos[2] < 5 || m_motors_pos[2] > 55) ? 0x04 : 0) |
			((m_motors_pos[3] < 5 || m_motors_pos[3] > 55) ? 0x08 : 0) |
			((m_motors_pos[4] < 5 || m_motors_pos[4] > 55) ? 0x10 : 0);
}

void wackygtr_state::machine_start()
{
	m_alligator.resolve();
	m_digit.resolve();
	m_lamps.resolve();

	save_item(NAME(m_adpcm_sel));
	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_ctrl));
	save_item(NAME(m_alligators_ctrl));
	save_item(NAME(m_motors_pos));
}

void wackygtr_state::machine_reset()
{
	m_adpcm_pos = 0;
	m_adpcm_sel = 0;
	m_adpcm_ctrl = 0x80;
	m_alligators_ctrl = 0;
}

void wackygtr_state::set_digits(int p, uint8_t value)
{
	static constexpr uint8_t bcd2hex[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 };  // not accurate
	m_digit[p + 0] = bcd2hex[value & 0x0f];
	m_digit[p + 1] = bcd2hex[(value >> 4) & 0x0f];
}

void wackygtr_state::set_lamps(int p, uint8_t value)
{
	for (int i=0; i<8; i++)
		m_lamps[p + i] = BIT(value, i);
}

static INPUT_PORTS_START( wackygtr )
	PORT_START("IN0")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(wackygtr_state, alligators_rear_sensors_r)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("IN1")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_CUSTOM_MEMBER(wackygtr_state, alligators_front_sensors_r)
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

void wackygtr_state::adpcm_int(int state)
{
	if (!(m_adpcm_ctrl & 0x80))
	{
		uint8_t data = m_samples->base()[m_adpcm_pos & 0xffff];
		m_msm->data_w((m_adpcm_sel ? data : (data >> 4)) & 0x0f);
		m_adpcm_pos += m_adpcm_sel;
		m_adpcm_sel ^= 1;
	}
}

void wackygtr_state::program_map(address_map &map)
{
	map(0x0200, 0x0200).nopr().w(FUNC(wackygtr_state::irq_ack_w));
	map(0x0400, 0x0400).nopr().w(FUNC(wackygtr_state::firq_ack_w));
	map(0x0600, 0x0600).w(FUNC(wackygtr_state::disp_w<0>));
	map(0x0800, 0x0800).w(FUNC(wackygtr_state::disp_w<1>));
	map(0x0a00, 0x0a00).w(FUNC(wackygtr_state::disp_w<2>));
	map(0x0c00, 0x0c00).w(FUNC(wackygtr_state::disp_w<3>));
	map(0x0e00, 0x0e00).w(FUNC(wackygtr_state::sample_ctrl_w));

	map(0x1000, 0x1001).w("ymsnd", FUNC(ym2413_device::write));

	map(0x2000, 0x2003).rw(m_pit8253[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x3000, 0x3003).rw(m_pit8253[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));

	map(0x4000, 0x4003).rw("i8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5000, 0x5003).rw("i8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6000, 0x6003).rw("i8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0x7000, 0x7fff).ram();
	map(0x8000, 0xffff).rom();
}

void wackygtr_state::wackygtr(machine_config &config)
{
	MC6809(config, m_maincpu, XTAL(3'579'545)); // HD68B09P
	m_maincpu->set_addrmap(AS_PROGRAM, &wackygtr_state::program_map);
	m_maincpu->set_periodic_int(FUNC(wackygtr_state::irq0_line_assert), attotime::from_hz(50)); // FIXME

	TIMER(config, "nmi_timer").configure_periodic(FUNC(wackygtr_state::nmi_timer), attotime::from_hz(100)); // FIXME

	/* Video */
	config.set_default_layout(layout_wackygtr);

	/* Sound */
	SPEAKER(config, "mono").front_center();
	MSM5205(config, m_msm, XTAL(384'000));
	m_msm->vck_legacy_callback().set(FUNC(wackygtr_state::adpcm_int));  /* IRQ handler */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 8 KHz, 4 Bits  */
	m_msm->add_route(ALL_OUTPUTS, "mono", 1.0);

	ym2413_device &ymsnd(YM2413(config, "ymsnd", XTAL(3'579'545)));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	i8255_device &ppi0(I8255(config, "i8255_0"));
	ppi0.out_pa_callback().set(FUNC(wackygtr_state::status_lamps_w));
	ppi0.out_pb_callback().set(FUNC(wackygtr_state::alligators_ctrl1_w));
	ppi0.out_pc_callback().set(FUNC(wackygtr_state::alligators_ctrl2_w));

	i8255_device &ppi1(I8255(config, "i8255_1"));
	ppi1.out_pa_callback().set(FUNC(wackygtr_state::timing_lamps_w<0>));
	ppi1.out_pb_callback().set(FUNC(wackygtr_state::timing_lamps_w<1>));
	ppi1.out_pc_callback().set(FUNC(wackygtr_state::timing_lamps_w<2>));

	i8255_device &ppi2(I8255(config, "i8255_2"));
	ppi2.in_pa_callback().set_ioport("IN0");
	ppi2.in_pb_callback().set_ioport("IN1");
	ppi2.in_pc_callback().set_ioport("IN2");

	PIT8253(config, m_pit8253[0], 0);
	m_pit8253[0]->set_clk<0>(XTAL(3'579'545)/16);  // this is a guess
	m_pit8253[0]->out_handler<0>().set(FUNC(wackygtr_state::alligator_ck<0>));
	m_pit8253[0]->set_clk<1>(XTAL(3'579'545)/16);  // this is a guess
	m_pit8253[0]->out_handler<1>().set(FUNC(wackygtr_state::alligator_ck<1>));
	m_pit8253[0]->set_clk<2>(XTAL(3'579'545)/16);  // this is a guess
	m_pit8253[0]->out_handler<2>().set(FUNC(wackygtr_state::alligator_ck<2>));

	PIT8253(config, m_pit8253[1], 0);
	m_pit8253[1]->set_clk<0>(XTAL(3'579'545)/16);  // this is a guess
	m_pit8253[1]->out_handler<0>().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	m_pit8253[1]->set_clk<1>(XTAL(3'579'545)/16);  // this is a guess
	m_pit8253[1]->out_handler<1>().set(FUNC(wackygtr_state::alligator_ck<3>));
	m_pit8253[1]->set_clk<2>(XTAL(3'579'545)/16);  // this is a guess
	m_pit8253[1]->out_handler<2>().set(FUNC(wackygtr_state::alligator_ck<4>));

	TICKET_DISPENSER(config, "ticket", attotime::from_msec(200));
}


ROM_START( wackygtr )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("wp3-pr0.4d", 0x8000, 0x8000, CRC(71ca4437) SHA1(c7d948c5593e6053fd0a65601f6c06871f5861f0))

	ROM_REGION(0x10000, "oki", 0)
	ROM_LOAD("wp3-vo0.2h", 0x0000, 0x10000, CRC(91c7986f) SHA1(bc9fa0d41c1caa0f909a349f511d022b7e42c6cd))
ROM_END

} // anonymous namespace


GAME(1988, wackygtr,    0, wackygtr,  wackygtr, wackygtr_state, empty_init, ROT0, "Namco (Data East license)", "Wacky Gator (US)", MACHINE_IS_SKELETON_MECHANICAL)
