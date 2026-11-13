// license:BSD-3-Clause
// copyright-holders: Styne13, stonedDiscord
/******************************************************************************

NSM Rotamint Duplex - and related machines using Fairchild F8 architecture with F3853+F3850
and 3x2KB EPROM (F3870 chip, which combines F3850 + F38T56)

ST95-I Hardware:
- Fairchild F3870 MCU (F3850 CPU + F38T56 PSU), label SL90594 or similar
- 3x2KB EPROMs for program code (e.g. A111157, A111156, A111158)
- Fairchild F3853 SMI for I/O and interrupt handling
- GI ER1400 serial EAROM (100x14 bits; candidate port-1 wiring under test)
- PWM display with 7-segment outputs for digit/segment output
- 40-pin DIP package at 1MHz clock
- rest of the hardware description is ongoing
   by viewing schematics and tracking traces

PCB markings: Ersatzteil-Nr. 111 147, Sach-Nr. 217 438.

  ________________________________________________
  | [Service-Connector]                 [3853PC SMI]     |A|    |A| |A|      [RST-BTN] [SERV-SW] |
  |                                                      |6|    |7| |8|      [74LS03] [74LS??]   |
  | [ER1400]  [74LS???] [74LS???]       [3850PC CPU]     [74LS139] [74LS155] [74LS00]            |
  |                                                                                              |
  | [74LS244]? [74LS244]? [74LS244]?                      [74LS374] [74LS374] [74LS374]? [74LS??]|
  |                                 [74LS7?][XTAL][74LS04]                                       |
  | [74LS244]? [74LS245]? [74LS244]?                      [74LS374] [74LS374] [74LS374]?         |
  |=== edge connector 01 (31pin)================================edge connector 02 (49 pin)=======|

TODO: Peripheral decoding is incomplete.

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/er1400.h"
#include "machine/f3853.h"
#include "machine/timer.h"
#include "video/pwm.h"

#include "rtmnt.lh"
#include "rtmnt_dplx.lh"

namespace {

class pre_st95_state : public driver_device
{
public:
	pre_st95_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_f3853(*this, "f3853"),
		m_earom(*this, "earom"),
		m_display(*this, "display"),
		m_risk_lamps(*this, "lamp_risk%u", 1U)
	{ }

	void rtmnt(machine_config &config) ATTR_COLD;

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
	bool m_earom_test_clock = false;
	bool m_earom_test_data_out = false;
	u8 m_risk_ladder_data = 0;

	u16 m_ram_address = 0;

	void rtmnt_mem(address_map &map) ATTR_COLD;
	void rtmnt_io(address_map &map) ATTR_COLD;

	u8 port0_r();
	void ram_address_w(u8 data);
	u8 port1_test_r();
	void port1_test_w(u8 data);
	void digit_select_w(u8 data);
	void segments_w(offs_t offset, u8 data);
	void update_display();
	void risk_ladder_w(u8 data);
	void update_risk_lamps();
	TIMER_DEVICE_CALLBACK_MEMBER(external_interrupt);
};

void pre_st95_state::machine_start()
{
	save_item(NAME(m_ram_address));
	save_item(NAME(m_digit_select));
	save_item(NAME(m_segments));
	save_item(NAME(m_risk_ladder_data));
	machine().save().register_postload(save_prepost_delegate(FUNC(pre_st95_state::update_risk_lamps), this));
	update_risk_lamps();
}

/*******************************************************************************
    Address Maps
*******************************************************************************/

void pre_st95_state::rtmnt_mem(address_map &map)
{
	map(0x0000, 0x17ff).rom().region("maincpu", 0);
	map(0x0800, 0x0800).w(FUNC(pre_st95_state::segments_w));
	map(0x3000, 0x3000).lw8(NAME([this](u8 data) { segments_w(1, data); }));
	map(0x5000, 0x5000).w(FUNC(pre_st95_state::digit_select_w));
	map(0x7000, 0x7000).w(FUNC(pre_st95_state::risk_ladder_w));
}

void pre_st95_state::rtmnt_io(address_map &map)
{
	/* F3853 I/O ports for program/data bus access */
	map(0x00, 0x00).r(FUNC(pre_st95_state::port0_r));
	map(0x00, 0x00).w(FUNC(pre_st95_state::ram_address_w));
	map(0x01, 0x01).rw(FUNC(pre_st95_state::port1_test_r), FUNC(pre_st95_state::port1_test_w));
	map(0x02, 0x0b).w(FUNC(pre_st95_state::ram_address_w)); /* Placeholder - actual I/O wiring unknown */
	map(0x0c, 0x0f).rw(m_f3853, FUNC(f3853_device::read), FUNC(f3853_device::write));
}

/*******************************************************************************
    Input Ports (placeholder - to be filled based on actual hardware)
*******************************************************************************/

static INPUT_PORTS_START( rtmnt )

INPUT_PORTS_END

void pre_st95_state::ram_address_w(u8 data)
{
	m_ram_address = data; /* Handle address writing to PSU registers */
}

void pre_st95_state::port1_test_w(u8 data)
{
	bool const clock = BIT(data, 6);
	u8 const command = ((!BIT(data, 2)) << 2) | ((!BIT(data, 1)) << 1) | (!BIT(data, 0));

	if (clock && !m_earom_test_clock)
		logerror("%s: ER1400 candidate port-1 edge command=%u data=%u\n", machine().describe_context(), command, !BIT(data, 7));
	m_earom_test_clock = clock;

	m_earom->c1_w(!BIT(data, 2));
	m_earom->c2_w(!BIT(data, 1));
	m_earom->c3_w(!BIT(data, 0));
	m_earom->data_w(!BIT(data, 7));
	m_earom->clock_w(clock);
	ram_address_w(data);
}

u8 pre_st95_state::port1_test_r()
{
	u8 const data = m_earom->data_r() << 4;
	if (BIT(data, 4) != m_earom_test_data_out)
		logerror("%s: ER1400 candidate data output=%u\n", machine().describe_context(), BIT(data, 4));
	m_earom_test_data_out = BIT(data, 4);
	return data;
}

u8 pre_st95_state::port0_r()
{
	// Preserve the all-low unmapped read while keeping the confirmed undervoltage input low.
	return 0x00; // only bit 3 is for under voltage probably
}

void pre_st95_state::digit_select_w(u8 data)
{
	m_digit_select = BIT(data, 4, 3);
	update_display();
}

void pre_st95_state::segments_w(offs_t offset, u8 data)
{
	m_segments[offset] = data;
	update_display();
}

void pre_st95_state::update_display()
{
	// The ROM scans seven positions on the first bus. Only positions 4/5
	// on the second bus carry the two-digit Chancen counter. Physical
	// ordering within the three displays is inferred from the BCD fields.
	// Outputs 0-2: Serienspiele, 3-4: Chancen, 5-8: Muenzspeicher.
	static constexpr int digit_map[7] = { 8, 7, 2, 6, 1, 5, 0 };
	u16 rows = 0;
	if (m_digit_select < 7)
		rows = 1U << digit_map[m_digit_select];
	m_display->matrix_partial(0, 3, rows, ~m_segments[0] & 0x7f);
	m_display->matrix_partial(5, 4, rows >> 5, ~m_segments[0] & 0x7f);
	rows = 0;
	if (m_digit_select == 4 || m_digit_select == 5)
		rows = 1U << (m_digit_select == 4 ? 4 : 3);
	// Update only the two counter rows, preserving the first segment bus.
	m_display->matrix_partial(3, 2, rows >> 3, ~m_segments[1] & 0x7f);
}

void pre_st95_state::risk_ladder_w(u8 data)
{
	m_risk_ladder_data = data;
	update_risk_lamps();
}

void pre_st95_state::update_risk_lamps()
{
	// Selector order is observed; the layout's prize-label assignment is provisional.
	static constexpr u8 selector_codes[13] =
	{
		0x0f, 0x03, 0x0e, 0x02, 0x0d, 0x01, 0x06,
		0x05, 0x04, 0x0a, 0x09, 0x08, 0x0b
	};
	u8 const selector = BIT(m_risk_ladder_data, 3, 4);

	for (unsigned lamp = 0; lamp < std::size(selector_codes); ++lamp)
		m_risk_lamps[lamp] = (selector == selector_codes[lamp]);
}

TIMER_DEVICE_CALLBACK_MEMBER(pre_st95_state::external_interrupt)
{
	// The F3853 latches the edge and holds INT REQ until acknowledged.
	m_f3853->ext_int_w(1);
	m_f3853->ext_int_w(0);
}

void pre_st95_state::rtmnt(machine_config &config)
{
	// Separate 3850 CPU and 3853 SMI.
	// TODO: verify the oscillator frequency and divider on the PCB.
	F8(config, m_maincpu, 1'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pre_st95_state::rtmnt_mem);
	m_maincpu->set_addrmap(AS_IO, &pre_st95_state::rtmnt_io);

	F3853(config, m_f3853, 1'000'000);
	m_f3853->int_req_callback().set_inputline(m_maincpu, F8_INPUT_LINE_INT_REQ);
	m_maincpu->int_cycle_callback().set(m_f3853, FUNC(f3853_device::int_acknowledge));

	// TODO: identify the external interrupt source and frequency. Startup counts
	// repeated external interrupts before enabling the F3853's internal timer.
	// Use a provisional 50 Hz pulse source pending verification of the hardware.
	TIMER(config, "external_interrupt").configure_periodic(FUNC(pre_st95_state::external_interrupt), attotime::from_hz(50));

	ER1400(config, m_earom);

	PWM_DISPLAY(config, m_display).set_size(9, 7);
	m_display->set_segmask(0x1ff, 0x7f);
	config.set_default_layout(layout_rtmnt);
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
GAMEL( 1982, rtduplex, 0, rtmnt, rtmnt, pre_st95_state, empty_init, ROT0, "NSM", "Rotamint Duplex", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW, layout_rtmnt)
