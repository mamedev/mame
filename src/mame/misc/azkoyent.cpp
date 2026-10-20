// license:BSD-3-Clause
// copyright-holders:Dirk Best, Tomás García-Merás (ClawGrip)
/*
    Azkoyen T series tobacco vending machines

    Two boards are known, both with an Intel 8279 scanning a four digit
    7-segment display and a 4x8 key matrix, and CD4099 addressable latches
    for the outputs:

    PCB 131000060-1 (T6, T8, T12): 8031/8051 running from external ROM,
    UM6104 4-bit SRAM (the firmware uses 256 nibbles) with battery, OKI
    M5832 RTC, five CD4099 (the last two not fitted on some boards).

      ___|||_||||||||||____________________________________
     |   ||| ||||||||||          ||||||||   |||||||||||   |
     | _____________                    _____             |
     ||::::::::::::|     __________     .....             |
     |                  |ULN2803A_|                       |
     |      __________   __________   __________         =|
     |     |CD4099BCN|  |CD4099BCN|  |CF74HC240E         =|
     |                                                   =|
     |  L7805CV          __________                      =|
     |                  |_UM6104__|                      =|
     |      __________   __________   __________          |
     |     |_TC4011BP|  |_TC4071BP|  |CD4099BCN|          |
     |      __________   __________   __________          |
     |     |_TC4011BP|  |GD74HC138|  |TC4099BP_|<-Not present on some versions
     |            ______________      __________         =|
     |           | EPROM       |     |TC4099BP_|<-Not present on some versions
     |           |_____________|                         =|
     |          ___   __________                         =|
     |       LM555CN |TC4069UBP|                         =|
     | ______        ___________           ___________    |
     || BATT|       |MM74HC373N|          |TD62083AP_|    |
     ||_____|        ___________                          |
     |              |MM74HC373N|    _____________         |
     | Osc                         |::::::::::::|         |
     | xxx MHz      _________________   _________________ |
     | __________  | Intel P80C51AH |  | NEC D8279C-2   | |
     ||OKI_M5832|  |________________|  |________________| |
     |                ____     Xtal     __________        |
     |                BDX53  6.000 MHz |SN74HC240N        |
     |                                                    |
     |_____________|_|____|_|__|__|||_|||||||||___||||____|

    PCB 104-4455-02-80/1 (T61): 80C39, two CD4099 and a battery but no RAM
    chip, so the internal RAM is presumably kept by the standby supply.

      ___________________________________________________________
     |                                       __________         |
    _|_           ___                       | BATT    |        _|_
    _|_         LM555CN                     |_________|        _|_
     |   ___          ____________________                     _|_
     |  BDX53A  Xtal | PCB 80C39 11P     |    __________       _|_
     |     6.144 MHz |___________________|   |_MC14069U|       _|_
    _|_                                          _____________ _|_
    _|_               ____________________      | EPROM      |  |
    _|_              | NEC D8279C-5      |      |____________| =|
     |        ___    |___________________|       __________    =|
     |=      |..|                               |M74HC373B1    =|
     |=      |..|                                _________     =|
     |=      |..|                               |TC4011BP|     =|
     |=      |..|                        ___     ___            |
     |       |..|              TC4011BP->|  |    |  |          =|
     |=      |..|                        |  |    |  <-TC4011BP =|
     |=      |..|                        |  |    |  |          =|
     |                                   |__|    |__|          =|
     |__________________________________________________________|

    The I/O maps and signal meanings come from the firmware.  "(assumption)"
    marks interpretations not verified on hardware and "(simulated)" marks
    machine mechanics modelled with made-up timings.  All the firmwares
    share one design.  A vend switches on one motor and waits for a common
    cam switch to leave its rest position and come back.  Hoppers 1, 2 and
    3 pay 5, 25 and 100 pesetas (the T61 has the first two); each has a
    coin sensor and a level switch, and the T8 and T12 read the hopper 1
    and 2 sensors with the opposite polarity to the T6.

    Usage
    -----
    The service switch selects NORMAL (selling) or CONTROL mode.  The
    operator keys inside the machine are Price +1, +10 and +100, Run
    Hopper 1 to 3, Clear Faults, Sales Counters and Clear Sales Counters.

    Setting up a new machine, which shows P.P.P.P.:
    1. Set the service switch to CONTROL.  On the T61, also press any
       selection.  The display shows 0000.
    2. Press the selection key of a line.  The display shows its price.
    3. Enter the price with Price +100, +10 and +1; the first press starts
       it from zero.  For 175 pesetas: +100, +10 seven times and +1 five
       times.  Pressing the selection again starts it over.
    4. Repeat steps 2 and 3 for every line.  Lines left at 0 don't sell.
    5. Set the service switch to NORMAL.  The display shows the time (0000
       on the T61): the machine is ready.

    Selling: insert coins and press a selection; the product comes out and
    the change is paid.  Coin Return gives the coins back.  No change is
    given while a Hopper Empty switch is on.

    More in control mode:
    - Sales Counters, then a selection: how many it has sold ("cont.").
    - Clear Sales Counters, then the service switch: clears all the sales
      counters ("borr."); any other key cancels.
    - Coin Return: set the clock ("HorA", not on the T61).  It starts from
      00:00; Price +1, +10 and +100 add a minute, ten minutes and an hour;
      the service switch saves it.
    - Run Hopper 1 to 3: empties that hopper, counting the coins; the same
      key stops it.

    Faults: "F_01" (hopper), "F_02" (vend motor) or "F_03" (coin mech)
    stop the machine.  Enter control mode, press Clear Faults ("rEAr.")
    and leave it.  "A_05", "A_25" or "A100" on entering control mode means
    that hopper ran out while paying; Clear Faults clears it too.

    Outputs, as the firmware drives them (8031 board latch 0 Q0-Q7 and
    latch 1, T61 latch Q outputs in brackets):
    - Q0, Q1 [Q0, Q1]: hopper 1 and 2 motors
    - Q2 [Q2]: on while running and with every hopper in control mode
    - Q3: on while the main loop polls for coins and keys
    - Q4 [Q6]: on when an enabled hopper is empty (exact change lamp, assumption)
    - Q5 [Q7]: set when a motor doesn't move the cam (sold out lamp, assumption)
    - Q6 [Q3]: pulsed when returning the inserted coins (escrow, assumption)
    - Q7 [Q4]: on for about a second after each vend
    - latch 1 Q0: hopper 3 motor
    - latch 1 Q1: pulsed three times when the coin lines are stuck
    - latch 1 Q4 [Q5]: off during vends and payouts (coin mech enable, assumption)

    Coin mechs: the 8031 board and the second T61 set read a 4-bit coin code
    with codes 1-11 being 500, 5, 10, 200, 25, 50, 100, 5, 25, 50 and 200
    pesetas, as in the Design.  The first T61 set has one line per coin (5,
    100, 25 and 200 pesetas).

    TODO:
    - Voice synthesizer (ROM undumped, T6 and T8 write a message number to
      0x70 and wait for INT1), watchdogs (P1.6 on the 8031 board, P2.7 on
      the T61)
*/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/i8051.h"
#include "machine/74259.h"
#include "machine/i8279.h"
#include "machine/msm5832.h"
#include "machine/nvram.h"
#include "machine/ticket.h"

#include "azkoyent12.lh"
#include "azkoyent6.lh"
#include "azkoyent61.lh"

#define LOG_VOICE (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_VOICE)
#include "logmacro.h"

#define LOGVOICE(...) LOGMASKED(LOG_VOICE, __VA_ARGS__)


namespace {

// parts common to both boards

class azkoyent_state : public driver_device
{
public:
	azkoyent_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_kdc(*this, "kdc")
		, m_io_keys(*this, "KEY%u", 0U)
		, m_digits(*this, "digit%u", 0U)
		, m_out_motor(*this, "motor%u", 1U)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

	int cam_home_r() { return !m_cam_away; }

protected:
	virtual void machine_start() override ATTR_COLD;

	required_device<i8279_device> m_kdc;
	required_ioport_array<4> m_io_keys;
	output_finder<4> m_digits;
	output_finder<24> m_out_motor;

	u8 m_coin = 0;

	void kdc_config(machine_config &config, const XTAL &clock) ATTR_COLD;

	void kdc_cmd_w(u8 data);
	template <unsigned N> void motors_w(u8 data);

private:
	emu_timer *m_coin_timer = nullptr;
	emu_timer *m_cam_timer = nullptr;

	u8 m_scan = 0;
	bool m_blank = false;
	u32 m_motors = 0;
	bool m_cam_away = false;

	void scan_w(u8 data);
	void disp_w(u8 data);
	u8 rl_r();

	TIMER_CALLBACK_MEMBER(coin_release);
	TIMER_CALLBACK_MEMBER(cam_update);
};


// PCB 131000060-1

class t6_state : public azkoyent_state
{
public:
	t6_state(const machine_config &mconfig, device_type type, const char *tag)
		: azkoyent_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_hopper(*this, "hopper%u", 1U)
		, m_xdata_view(*this, "xdata")
		, m_out_latch(*this, "out%u%u", 0U, 0U)
	{
	}

	void t6(machine_config &config) ATTR_COLD;
	void t12(machine_config &config) ATTR_COLD;

	ioport_value coin_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i8031_device> m_maincpu;
	required_device<msm5832_device> m_rtc;
	required_device_array<hopper_device, 3> m_hopper;
	memory_view m_xdata_view;
	output_finder<2, 8> m_out_latch;

	u8 m_rtc_ctrl = 0;
	bool m_kdc_irq = false;

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	void port1_w(u8 data);
	u8 port1_r();
	u8 port3_r();
	void rtc_w(u8 data);
	void voice_w(u8 data);
	template <unsigned N> void outlatch_w(u8 data);
};


// PCB 104-4455-02-80/1

class t61_state : public azkoyent_state
{
public:
	t61_state(const machine_config &mconfig, device_type type, const char *tag)
		: azkoyent_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_hopper(*this, "hopper%u", 1U)
		, m_out_latch(*this, "out1%u", 0U)
	{
	}

	void t61(machine_config &config) ATTR_COLD;

	// P1.0 idles low and P1.1-P1.3 idle high
	ioport_value coin_r() { return m_coin ^ 0x0e; }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i8039_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device_array<hopper_device, 2> m_hopper;
	output_finder<8> m_out_latch;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void outlatch_w(u8 data);
};



/**************************************************************************
    Common
**************************************************************************/

void azkoyent_state::machine_start()
{
	m_coin_timer = timer_alloc(FUNC(azkoyent_state::coin_release), this);
	m_cam_timer = timer_alloc(FUNC(azkoyent_state::cam_update), this);

	save_item(NAME(m_coin));
	save_item(NAME(m_scan));
	save_item(NAME(m_blank));
	save_item(NAME(m_motors));
	save_item(NAME(m_cam_away));
}

void azkoyent_state::kdc_cmd_w(u8 data)
{
	// FIXME: the 8279 device doesn't implement blanking, used to flash the display
	if ((data & 0xe0) == 0xa0)
		m_blank = (data & 0x03) == 0x03;

	m_kdc->cmd_w(data);
}

void azkoyent_state::scan_w(u8 data)
{
	// decoded scan, active low
	for (unsigned i = 0; i < 4; i++)
		if (!BIT(data, i))
			m_scan = i;
}

void azkoyent_state::disp_w(u8 data)
{
	// segment table in ROM: bit 7 = a ... bit 1 = g, bit 0 = dp
	m_digits[m_scan] = m_blank ? 0 : bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
}

u8 azkoyent_state::rl_r()
{
	return ~m_io_keys[m_scan]->read();
}

void azkoyent_state::kdc_config(machine_config &config, const XTAL &clock)
{
	I8279(config, m_kdc, clock);
	m_kdc->out_sl_callback().set(FUNC(azkoyent_state::scan_w));
	m_kdc->out_disp_callback().set(FUNC(azkoyent_state::disp_w));
	m_kdc->in_rl_callback().set(FUNC(azkoyent_state::rl_r));
	m_kdc->in_shift_callback().set_constant(0);
	m_kdc->in_ctrl_callback().set_constant(0);
}

template <unsigned N>
void azkoyent_state::motors_w(u8 data)
{
	const u32 old = m_motors;
	m_motors = (m_motors & ~(0xffU << (N * 8))) | (u32(data) << (N * 8));

	for (unsigned i = 0; i < 8; i++)
		m_out_motor[N * 8 + i] = BIT(data, i);

	// (simulated) the cam switch leaves its rest position a while after a
	// motor starts and is back one turn later
	if (!old && m_motors)
		m_cam_timer->adjust(attotime::from_msec(300));
	else if (!m_motors && !m_cam_away)
		m_cam_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(azkoyent_state::cam_update)
{
	m_cam_away = !m_cam_away;

	if (m_cam_away)
		m_cam_timer->adjust(attotime::from_msec(700));
	else if (m_motors)
		m_cam_timer->adjust(attotime::from_msec(300));
}

// (simulated) coin mech: the coin code (or line) is held for 50 ms
INPUT_CHANGED_MEMBER(azkoyent_state::coin_inserted)
{
	if (!newval || m_coin)
		return;

	m_coin = param;
	m_coin_timer->adjust(attotime::from_msec(50));
}

TIMER_CALLBACK_MEMBER(azkoyent_state::coin_release)
{
	m_coin = 0;
}



/**************************************************************************
    PCB 131000060-1
**************************************************************************/

void t6_state::program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
}

void t6_state::data_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_xdata_view);

	// P1.4 = 0: I/O; the T8 and T12 leave P2 at 0xff, so A8-A15 are ignored
	m_xdata_view[0](0x0000, 0x0000).mirror(0xff00).portr("IN0");
	m_xdata_view[0](0x0000, 0x0007).mirror(0xff00).w("outlatch0", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0010, 0x0010).mirror(0xff00).portr("IN1");
	m_xdata_view[0](0x0010, 0x0017).mirror(0xff00).w("outlatch1", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0020, 0x0027).mirror(0xff00).w("motlatch0", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0030, 0x0037).mirror(0xff00).w("motlatch1", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0040, 0x0047).mirror(0xff00).w("motlatch2", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0050, 0x0050).mirror(0xff00).rw(m_kdc, FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	m_xdata_view[0](0x0051, 0x0051).mirror(0xff00).r(m_kdc, FUNC(i8279_device::status_r)).w(FUNC(t6_state::kdc_cmd_w));
	m_xdata_view[0](0x0060, 0x0060).mirror(0xff00).w(FUNC(t6_state::rtc_w));
	m_xdata_view[0](0x0070, 0x0070).mirror(0xff00).w(FUNC(t6_state::voice_w));

	// P1.4 = 1: SRAM, only D0-D3 connected
	m_xdata_view[1](0x0000, 0x00ff).mirror(0xff00).ram().share("nvram");
}

void t6_state::port1_w(u8 data)
{
	// 0-3: RTC data
	// 4: external data space select
	// 5: low during SRAM writes (write enable, assumption)
	// 6: pulsed all the time (watchdog, assumption)
	m_xdata_view.select(BIT(data, 4));
	m_rtc->data_w(data & 0x0f);
}

u8 t6_state::port1_r()
{
	return (BIT(m_rtc_ctrl, 7) && BIT(m_rtc_ctrl, 5)) ? (0xf0 | m_rtc->data_r()) : 0xff;
}

u8 t6_state::port3_r()
{
	// INT0: 8279 IRQ
	// INT1: waited on after writing to 0x70 (voice synthesizer busy, assumption; low when absent)
	return 0xf3 | (m_kdc_irq ? 0x04 : 0x00);
}

void t6_state::rtc_w(u8 data)
{
	// 0-3: address, 4: write, 5: read, 6 and 7: hold and chip select (order assumed)
	m_rtc_ctrl = data;
	m_rtc->cs_w(BIT(data, 7));
	m_rtc->hold_w(BIT(data, 6));
	m_rtc->read_w(BIT(data, 5));
	m_rtc->address_w(data & 0x0f);
	m_rtc->write_w(BIT(data, 4));
}

void t6_state::voice_w(u8 data)
{
	LOGVOICE("%s: voice message %02x\n", machine().describe_context(), data);
}

template <unsigned N>
void t6_state::outlatch_w(u8 data)
{
	for (unsigned i = 0; i < 8; i++)
		m_out_latch[N][i] = BIT(data, i);
}

ioport_value t6_state::coin_r()
{
	// IN0 bits 1, 2, 3 and 5 are code bits 3, 1, 2 and 0
	return BIT(m_coin, 3) | (m_coin & 0x06) | (BIT(m_coin, 0) << 4);
}

void t6_state::machine_start()
{
	azkoyent_state::machine_start();

	save_item(NAME(m_rtc_ctrl));
	save_item(NAME(m_kdc_irq));
}

void t6_state::machine_reset()
{
	azkoyent_state::machine_reset();

	m_xdata_view.select(1);
}

void t6_state::t6(machine_config &config)
{
	I8031(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &t6_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &t6_state::data_map);
	m_maincpu->port_in_cb<1>().set(FUNC(t6_state::port1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(t6_state::port1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(t6_state::port3_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// clocked by ALE (assumption), the firmware sets a prescaler of 10
	kdc_config(config, 6_MHz_XTAL / 6);
	m_kdc->out_irq_callback().set([this] (int state) { m_kdc_irq = state; });

	MSM5832(config, m_rtc, 32.768_kHz_XTAL);

	cd4099_device &outlatch0(CD4099(config, "outlatch0"));
	outlatch0.q_out_cb<0>().set(m_hopper[0], FUNC(hopper_device::motor_w));
	outlatch0.q_out_cb<1>().set(m_hopper[1], FUNC(hopper_device::motor_w));
	outlatch0.parallel_out_cb().set(FUNC(t6_state::outlatch_w<0>));

	cd4099_device &outlatch1(CD4099(config, "outlatch1"));
	outlatch1.q_out_cb<0>().set(m_hopper[2], FUNC(hopper_device::motor_w));
	outlatch1.parallel_out_cb().set(FUNC(t6_state::outlatch_w<1>));

	CD4099(config, "motlatch0").parallel_out_cb().set(FUNC(t6_state::motors_w<0>));
	CD4099(config, "motlatch1").parallel_out_cb().set(FUNC(t6_state::motors_w<1>));
	CD4099(config, "motlatch2").parallel_out_cb().set(FUNC(t6_state::motors_w<2>));

	// (simulated)
	for (auto &hopper : m_hopper)
		HOPPER(config, hopper, attotime::from_msec(100));

	config.set_default_layout(layout_azkoyent6);
}

void t6_state::t12(machine_config &config)
{
	t6(config);

	config.set_default_layout(layout_azkoyent12);
}



/**************************************************************************
    PCB 104-4455-02-80/1
**************************************************************************/

void t61_state::program_map(address_map &map)
{
	map(0x000, 0xfff).rom().region("maincpu", 0);
}

void t61_state::io_map(address_map &map)
{
	map(0x10, 0x17).w("outlatch", FUNC(cd4099_device::write_d0));
	map(0x20, 0x27).w("motlatch", FUNC(cd4099_device::write_d0));
	map(0x40, 0x40).rw(m_kdc, FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	map(0x41, 0x41).r(m_kdc, FUNC(i8279_device::status_r)).w(FUNC(t61_state::kdc_cmd_w));
}

void t61_state::outlatch_w(u8 data)
{
	for (unsigned i = 0; i < 8; i++)
		m_out_latch[i] = BIT(data, i);
}

void t61_state::machine_start()
{
	azkoyent_state::machine_start();

	// the internal RAM is battery backed (assumption)
	memory_share *const ram = memshare("maincpu:data");
	m_nvram->set_base(ram->ptr(), ram->bytes());
}

void t61_state::t61(machine_config &config)
{
	I8039(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &t61_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &t61_state::io_map);
	m_maincpu->p1_in_cb().set_ioport("P1");
	m_maincpu->t0_in_cb().set(m_hopper[0], FUNC(hopper_device::line_r));
	m_maincpu->t1_in_cb().set(m_hopper[1], FUNC(hopper_device::line_r));
	// P2.6: high while the firmware updates its data (purpose unknown)
	// P2.7: pulsed all the time (watchdog, assumption)

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	// clocked by ALE (assumption), the firmware sets a prescaler of 2
	kdc_config(config, 6.144_MHz_XTAL / 15);
	m_kdc->out_irq_callback().set_inputline(m_maincpu, MCS48_INPUT_IRQ).invert();

	cd4099_device &outlatch(CD4099(config, "outlatch"));
	outlatch.q_out_cb<0>().set(m_hopper[0], FUNC(hopper_device::motor_w));
	outlatch.q_out_cb<1>().set(m_hopper[1], FUNC(hopper_device::motor_w));
	outlatch.parallel_out_cb().set(FUNC(t61_state::outlatch_w));

	CD4099(config, "motlatch").parallel_out_cb().set(FUNC(t61_state::motors_w<0>));

	// (simulated)
	for (auto &hopper : m_hopper)
		HOPPER(config, hopper, attotime::from_msec(100));

	config.set_default_layout(layout_azkoyent61);
}



/**************************************************************************
    Input ports
**************************************************************************/

static INPUT_PORTS_START( keys )
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_B) PORT_NAME("Coin Return")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Q) PORT_NAME("Selection 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_W) PORT_NAME("Selection 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_E) PORT_NAME("Selection 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_R) PORT_NAME("Selection 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_T) PORT_NAME("Selection 5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Y) PORT_NAME("Selection 6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_U) PORT_NAME("Selection 7")

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_I) PORT_NAME("Selection 8")
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_A) PORT_NAME("Price +1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_S) PORT_NAME("Price +10")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_D) PORT_NAME("Price +100")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F) PORT_NAME("Run Hopper 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_G) PORT_NAME("Run Hopper 2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Z) PORT_NAME("Clear Faults")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_X) PORT_NAME("Sales Counters")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_C) PORT_NAME("Clear Sales Counters")
INPUT_PORTS_END

static INPUT_PORTS_START( coins11 )
	PORT_START("COINS")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("5 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 8)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("10 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 3)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("25 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 9)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("50 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 6)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_COIN5) PORT_NAME("100 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 7)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_COIN6) PORT_NAME("200 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 4)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_COIN7) PORT_NAME("500 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 1)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_COIN8) PORT_NAME("5 Pesetas (alt)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 2)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_COIN9) PORT_NAME("25 Pesetas (alt)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 5)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_COIN10) PORT_NAME("50 Pesetas (alt)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 10)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_COIN11) PORT_NAME("200 Pesetas (alt)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 11)
INPUT_PORTS_END

static INPUT_PORTS_START( t6 )
	PORT_INCLUDE(keys)
	PORT_INCLUDE(coins11)

	PORT_MODIFY("KEY1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_H) PORT_NAME("Run Hopper 3")

	PORT_START("IN0")
	PORT_BIT(0x2e, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(t6_state::coin_r))
	PORT_BIT(0xd1, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper3", FUNC(hopper_device::line_r))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper2", FUNC(hopper_device::line_r))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper1", FUNC(hopper_device::line_r))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 3 Empty")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 2 Empty")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 1 Empty")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_TOGGLE PORT_NAME("Service Switch")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(azkoyent_state::cam_home_r))
INPUT_PORTS_END

static INPUT_PORTS_START( t8 )
	PORT_INCLUDE(t6)

	PORT_MODIFY("IN1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper2", FUNC(hopper_device::line_r))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper1", FUNC(hopper_device::line_r))
INPUT_PORTS_END

static INPUT_PORTS_START( t12 )
	PORT_INCLUDE(t8)

	PORT_MODIFY("KEY1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_O) PORT_NAME("Selection 9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_P) PORT_NAME("Selection 10")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_J) PORT_NAME("Selection 11")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_K) PORT_NAME("Selection 12")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_L) PORT_NAME("Selection 13")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_N) PORT_NAME("Selection 14")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_M) PORT_NAME("Selection 15")

	PORT_MODIFY("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Selection 16")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_STOP) PORT_NAME("Selection 17")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_H) PORT_NAME("Run Hopper 3")
INPUT_PORTS_END

static INPUT_PORTS_START( t61 )
	PORT_INCLUDE(keys)

	PORT_START("P1")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(t61_state::coin_r))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 1 Empty")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 2 Empty")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE) PORT_TOGGLE PORT_NAME("Service Switch")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(azkoyent_state::cam_home_r))

	// one line per coin
	PORT_START("COINS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("5 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("100 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("25 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 4)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("200 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(azkoyent_state::coin_inserted), 8)
INPUT_PORTS_END

static INPUT_PORTS_START( t61a )
	PORT_INCLUDE(keys)
	PORT_INCLUDE(coins11)

	PORT_START("P1")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(t61_state::coin_r))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 1 Empty")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 2 Empty")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE) PORT_TOGGLE PORT_NAME("Service Switch")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(azkoyent_state::cam_home_r))
INPUT_PORTS_END



/**************************************************************************
    ROM definitions
**************************************************************************/

ROM_START( azkoyent6 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("43504560-0_t-6.u04",   0x0000, 0x2000, CRC(a4289b26) SHA1(40587094b11c6cf9308673ffac2ed9d445d458e9))
ROM_END

ROM_START( azkoyent8 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("43504570-2_t8_3.u04",  0x0000, 0x2000, CRC(76ac54bf) SHA1(da4c4a9f1c9c85d59169d62682bb7b73a9dd133b))
ROM_END

ROM_START( azkoyent12 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("43504580-0_t12-17.u04", 0x0000, 0x2000, CRC(10d4d4a7) SHA1(96804bc173abf2d51de7e7f84decba286916eba7))
ROM_END

ROM_START( azkoyent61 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("t-61.u4",       0x0000, 0x1000, CRC(16d9b843) SHA1(7c6f177eca9163b5284d2cbe1bdeb3b0bf1a6698))
ROM_END

ROM_START( azkoyent61a )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("t-61-6_t-m.u4", 0x0000, 0x1000, CRC(ce1ed720) SHA1(42cb78ddd8d06764599e97b72b557d164940f7df))
ROM_END

} // anonymous namespace



/**************************************************************************
    System drivers
**************************************************************************/

//    YEAR   NAME         PARENT      COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY    FULLNAME                             FLAGS
SYST( 19??,  azkoyent6,   0,          0,      t6,      t6,    t6_state,  empty_init, "Azkoyen", "Vending machine model T6",          MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 19??,  azkoyent8,   0,          0,      t6,      t8,    t6_state,  empty_init, "Azkoyen", "Vending machine model T8",          MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 19??,  azkoyent12,  0,          0,      t12,     t12,   t6_state,  empty_init, "Azkoyen", "Vending machine model T12",         MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
SYST( 19??,  azkoyent61,  0,          0,      t61,     t61,   t61_state, empty_init, "Azkoyen", "Vending machine model T61 (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
SYST( 19??,  azkoyent61a, azkoyent61, 0,      t61,     t61a,  t61_state, empty_init, "Azkoyen", "Vending machine model T61 (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
