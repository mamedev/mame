// license:BSD-3-Clause
// copyright-holders: Styne13, stonedDiscord
/******************************************************************************

NSM Rotamint Duplex - and related machines using Fairchild F8 architecture with F3853+F3850
and 3x2KB EPROM (F3870 chip, which combines F3850 + F38T56)

ST95-I Hardware:
- Fairchild F3870 MCU (F3850 CPU + F38T56 PSU), label SL90594 or similar
- 3x2KB EPROMs for program code (e.g. A111157, A111156, A111158)
- Fairchild F3853 SMI for I/O and interrupt handling
- GI ER1400 serial EAROM (100x14 bits; signal wiring not yet implemented)
- 40-pin DIP package at 16MHz? clock
- rest of the hardware description is ongoing
   by viewing schematics and tracking traces

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/er1400.h"
#include "machine/f3853.h"

namespace {

class pre_st95_state : public driver_device
{
public:
	pre_st95_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_f3853(*this, "f3853"),
		m_earom(*this, "earom")
	{ }

	void rtmnt(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<f8_cpu_device> m_maincpu;
	required_device<f3853_device> m_f3853;
	required_device<er1400_device> m_earom;

	u16 m_ram_address = 0;

	void rtmnt_mem(address_map &map) ATTR_COLD;
	void rtmnt_io(address_map &map) ATTR_COLD;

	void ram_address_w(u8 data);
	void digit_w(u8 data);
};

void pre_st95_state::machine_start()
{
	save_item(NAME(m_ram_address));
}

/*******************************************************************************
    Address Maps
*******************************************************************************/

void pre_st95_state::rtmnt_mem(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0000, 0x07ff).rom().region("maincpu", 0x0000);
	map(0x1800, 0x1fff).rom().region("maincpu", 0x1800);
	map(0x3000, 0x37ff).rom().region("maincpu", 0x3000);
}

void pre_st95_state::rtmnt_io(address_map &map)
{
	/* F3853 I/O ports for program/data bus access */
	map(0x00, 0x0f).w(FUNC(pre_st95_state::ram_address_w)); /* RAM address register (for PSU) */
	map(0x10, 0x17).w(FUNC(pre_st95_state::digit_w));       /* Digit segment output */
	map(0x20, 0x23).rw(m_f3853, FUNC(f3853_device::read), FUNC(f3853_device::write));
}

/*******************************************************************************
    Input Ports (placeholder - to be filled based on actual hardware)
*******************************************************************************/

static INPUT_PORTS_START( rtmnt )
	PORT_START("INPUT0")
	/* Example inputs - adjust based on hardware layout */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH ) PORT_NAME("Risk")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Reset")

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
INPUT_PORTS_END

void pre_st95_state::ram_address_w(u8 data)
{
	m_ram_address = data; /* Handle address writing to PSU registers */
}

void pre_st95_state::digit_w(u8 data)
{
	/* Placeholder for segment digit handling */
}

void pre_st95_state::rtmnt(machine_config &config)
{
	/* basic machine hardware - F3870 (F3850 CPU + F38T56 PSU) at ~4MHz */
	F8(config, m_maincpu, 4'000'000 / 2); /* approximate clock divided by F3870 internal divider */
	m_maincpu->set_addrmap(AS_PROGRAM, &pre_st95_state::rtmnt_mem);
	m_maincpu->set_addrmap(AS_IO, &pre_st95_state::rtmnt_io);

	F3853(config, m_f3853, 4'000'000 / 2);
	m_f3853->int_req_callback().set_inputline(m_maincpu, F8_INPUT_LINE_INT_REQ);
	m_maincpu->int_cycle_callback().set(m_f3853, FUNC(f3853_device::int_acknowledge));

	ER1400(config, m_earom);
}

/*******************************************************************************
    ROM Definitions (placeholder - actual ROMs from rtmnt_dplx.zip)
*******************************************************************************/

ROM_START( rtmnt_dplx )
	ROM_REGION( 0x4000, "maincpu", 0 ) /* Total program space */
	ROM_LOAD( "dplx_a111157.ica7", 0x0000, 0x0800, CRC(5755b22b) SHA1(56602a117d2418e24b27e95a4ecf26b8045f2d94) ) /* First 2KB */
	ROM_LOAD( "dplx_a111156.ica6", 0x1800, 0x0800, CRC(80cfba5d) SHA1(26c65501806b630173ef8db424c7fcfe21958082)) /* Second 2KB */
	ROM_LOAD( "dplx_a111158.ica8", 0x3000, 0x0800, CRC(d5eb9bc5) SHA1(3742ebfada503a756be5a8851950250c4d8af51b) ) /* Third 2KB */
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//   YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS           INIT        COMPANY, FULLNAME, FLAGS
GAME( 1982, rtmnt_dplx, 0,      rtmnt,  rtmnt, pre_st95_state, empty_init, ROT0, "NSM", "Rotamint Duplex", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
