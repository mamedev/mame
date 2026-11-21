// license:BSD-3-Clause
// copyright-holders: Styne13, stonedDiscord
/******************************************************************************

F8 Hardware:
- Fairchild F3850 CPU
- 3x2KB EPROMs for program code (A6, A7, A8)
- Fairchild F3853 SMI for I/O and interrupt handling
- GI ER1400 serial EAROM (100x14 bits)

PCB markings: Ersatzteil-Nr. 111 147, Sach-Nr. 217 438.

  _______________________________________________________________________________________________
  | [Service-Connector]                 [3853PC SMI]     |A|    |A| |A|      [RST-BTN] [SERV-SW] |
  |                                                      |6|    |7| |8|      [74LS03] [74LS??]   |
  | [ER1400]  [74LS???] [74LS???]       [3850PC CPU]     [74LS139] [74LS155] [74LS00]            |
  |                                                                                              |
  | [74LS244]? [74LS244]? [74LS244]?                      [74LS374] [74LS374] [74LS374]? [74LS??]|
  |                                 [74LS7?][XTAL][74LS04]                                       |
  | [74LS244]? [74LS245]? [74LS244]?                      [74LS374] [74LS374] [74LS374]?         |
  |=== edge connector 01 (31pin)================================edge connector 02 (49 pin)=======|

TODO: Peripheral decoding is incomplete: Coin acceptor/hopper, buttons, motor control unit. Clock frequency has to be validated on the board. Identify external inerrupt source for F3853.

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "logmacro.h"
#include "machine/clock.h"
#include "machine/er1400.h"
#include "machine/f3853.h"
#include "video/pwm.h"

#include "nsmf8.lh"
#include "rtduplex.lh"

namespace {

class nsmf8_state : public driver_device
{
public:
	nsmf8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_f3853(*this, "f3853"),
		m_earom(*this, "earom"),
		m_display(*this, "display"),
		m_risk_lamps(*this, "lamp_risk%u", 1U)
	{ }

	void nsmf8(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<f8_cpu_device> m_maincpu;
	required_device<f3853_device> m_f3853;
	required_device<er1400_device> m_earom;
	required_device<pwm_display_device> m_display;
	output_finder<13> m_risk_lamps;

	u8 m_digit_select = 7;
	u8 m_segments[2] = { 0xff, 0xff };


	u8 m_risk_ladder_data = 0;


	void nsmf8_mem(address_map &map) ATTR_COLD;
	void nsmf8_io(address_map &map) ATTR_COLD;

	u8 port0_r();
	u8 port1_r();
	void port1_w(u8 data);
	void digit_select_w(u8 data);
	template <u32 N> void segments_w(u8 data);
	void update_display();
	void risk_ladder_w(u8 data);
	void update_risk_lamps();
};

void nsmf8_state::machine_start()
{
	save_item(NAME(m_digit_select));
	save_item(NAME(m_segments));
	save_item(NAME(m_risk_ladder_data));
	machine().save().register_postload(save_prepost_delegate(FUNC(nsmf8_state::update_risk_lamps), this));
	update_risk_lamps();
}

/*******************************************************************************
    Address Maps
*******************************************************************************/

void nsmf8_state::nsmf8_mem(address_map &map)
{
	map(0x0000, 0x17ff).rom().region("maincpu", 0);
	map(0x0800, 0x0800).w(FUNC(nsmf8_state::segments_w<0>));
	map(0x3000, 0x3000).lw8(NAME([this](u8 data) { segments_w<1>(data); }));
	map(0x5000, 0x5000).w(FUNC(nsmf8_state::digit_select_w));
	map(0x7000, 0x7000).w(FUNC(nsmf8_state::risk_ladder_w));
}

void nsmf8_state::nsmf8_io(address_map &map)
{
	map(0x00, 0x00).r(FUNC(nsmf8_state::port0_r));
	map(0x01, 0x01).rw(FUNC(nsmf8_state::port1_r), FUNC(nsmf8_state::port1_w));
	/* F3853 I/O ports for program/data bus access */
	map(0x0c, 0x0f).rw(m_f3853, FUNC(f3853_device::read), FUNC(f3853_device::write));
}

/*******************************************************************************
    Input Ports (placeholder - to be filled based on actual hardware)
*******************************************************************************/

static INPUT_PORTS_START( nsmf8 )

INPUT_PORTS_END


void nsmf8_state::port1_w(u8 data)
{
	// BIT 6 = clock, bits 0-2 = command
	u8 const command = BIT(~data, 0, 3);

	if (BIT(data, 6))
		LOGMASKED(LOG_GENERAL, "%s: ER1400 port-1 edge command=%u data=%u\n", machine().describe_context(), command, BIT(~data, 7));

	m_earom->c1_w(BIT(~data, 2));
	m_earom->c2_w(BIT(~data, 1));
	m_earom->c3_w(BIT(~data, 0));
	m_earom->data_w(BIT(~data, 7));
	m_earom->clock_w(BIT(data, 6));
}

u8 nsmf8_state::port1_r()
{
	u8 const data = m_earom->data_r() << 4;
	return data;
}

u8 nsmf8_state::port0_r()
{
	// TODO: bit 3 is the undervoltage input, other bits are unidentified.
	return 0x00;
}

void nsmf8_state::digit_select_w(u8 data)
{
	m_digit_select = BIT(data, 4, 3);
	update_display();
}

template <u32 N> void nsmf8_state::segments_w(u8 data)
{
	m_segments[N] = data;
	update_display();
}

void nsmf8_state::update_display()
{
	// The ROM scans seven positions on the first bus. Only positions 4/5
	// on the second bus carry the two-digit Chancen counter.
	// Outputs 0-2: special counter, 3-4: chance, 5-8: coins.
	static constexpr u8 DIGIT_MAP[7] = { 8u, 7u, 2u, 6u, 1u, 5u, 0u };
	u16 rows = 0;
	if (m_digit_select < 7)
		rows = 1U << DIGIT_MAP[m_digit_select];
	m_display->matrix_partial(0, 3, rows, ~m_segments[0] & 0x7f);
	m_display->matrix_partial(5, 4, rows >> 5, ~m_segments[0] & 0x7f);
	rows = 0;
	if (m_digit_select == 4 || m_digit_select == 5)
		rows = 1U << (m_digit_select == 4 ? 4 : 3);
	// Update only the two counter rows, preserving the first segment bus.
	m_display->matrix_partial(3, 2, rows >> 3, ~m_segments[1] & 0x7f);
}

void nsmf8_state::risk_ladder_w(u8 data)
{
	m_risk_ladder_data = data;
	update_risk_lamps();
}

void nsmf8_state::update_risk_lamps()
{
	static constexpr u8 SELECTOR_CODES[13] =
	{
		0x0f, 0x03, 0x0e, 0x02, 0x0d, 0x01, 0x06,
		0x05, 0x04, 0x0a, 0x09, 0x08, 0x0b
	};
	u8 const selector = BIT(m_risk_ladder_data, 3, 4);

	for (u32 lamp = 0; lamp < std::size(SELECTOR_CODES); ++lamp)
		m_risk_lamps[lamp] = (selector == SELECTOR_CODES[lamp]);
}

void nsmf8_state::nsmf8(machine_config &config)
{
	// Separate 3850 CPU and 3853 SMI.
	F8(config, m_maincpu, 1'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &nsmf8_state::nsmf8_mem);
	m_maincpu->set_addrmap(AS_IO, &nsmf8_state::nsmf8_io);

	F3853(config, m_f3853, 1'000'000);
	m_f3853->int_req_callback().set_inputline(m_maincpu, F8_INPUT_LINE_INT_REQ);
	m_maincpu->int_cycle_callback().set(m_f3853, FUNC(f3853_device::int_acknowledge));

	CLOCK(config, "extint", 50).signal_handler().set(m_f3853, FUNC(f3853_device::ext_int_w));

	ER1400(config, m_earom);

	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x1ff, 0x7f);
	config.set_default_layout(layout_nsmf8);
}

ROM_START( rtduplex )
	ROM_REGION( 0x1800, "maincpu", 0 )
	ROM_LOAD( "a111156.a6", 0x0000, 0x0800, CRC(80cfba5d) SHA1(26c65501806b630173ef8db424c7fcfe21958082) )
	ROM_LOAD( "a111157.a7", 0x0800, 0x0800, CRC(5755b22b) SHA1(56602a117d2418e24b27e95a4ecf26b8045f2d94) )
	ROM_LOAD( "a111158.a8", 0x1000, 0x0800, CRC(d5eb9bc5) SHA1(3742ebfada503a756be5a8851950250c4d8af51b) )
ROM_END

} // anonymous namespace


/*******************************************************************************
    Drivers
*******************************************************************************/

//   YEAR  NAME       PARENT  MACHINE    INPUT      CLASS           INIT        COMPANY, FULLNAME, FLAGS
GAMEL( 1981, rtduplex, 0, nsmf8, nsmf8, nsmf8_state, empty_init, ROT0, "NSM", "Rotamint Duplex", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK, layout_rtduplex)
