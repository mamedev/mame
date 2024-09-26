// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    VTech Intelligence Advance E/R Lerncomputer

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/st2204.h"
#include "cpu/m6502/st2205u.h"


namespace {

class inteladv_state : public driver_device
{
public:
	inteladv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void inteladv(machine_config &config);
	void dyndesk(machine_config &config);

	void st2205u_power_w(int state);
	void st2202_power_w(int state);

private:
	void inteladv_map(address_map &map) ATTR_COLD;
	void dyndesk_map(address_map &map) ATTR_COLD;

	required_device<st2xxx_device> m_maincpu;
};

void inteladv_state::inteladv_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("maincpu", 0);
}

void inteladv_state::dyndesk_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0).mirror(0x400000); // why mirrored?
	map(0x800000, 0x807fff).ram();
}

void inteladv_state::st2205u_power_w(int state)
{
	if (!state)
		m_maincpu->set_state_int(st2xxx_device::ST_IREQ, m_maincpu->state_int(st2xxx_device::ST_IREQ) | 0x0020);
}

void inteladv_state::st2202_power_w(int state)
{
	if (!state)
		m_maincpu->set_state_int(st2xxx_device::ST_IREQ, m_maincpu->state_int(st2xxx_device::ST_IREQ) | 0x0010);
}

static INPUT_PORTS_START(inteladv)
	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_POWER_ON) PORT_WRITE_LINE_MEMBER(inteladv_state, st2205u_power_w)
INPUT_PORTS_END

static INPUT_PORTS_START(dyndesk)
	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_POWER_ON) PORT_WRITE_LINE_MEMBER(inteladv_state, st2202_power_w)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_START)
INPUT_PORTS_END

void inteladv_state::inteladv(machine_config &config)
{
	ST2205U(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_DATA, &inteladv_state::inteladv_map);
	m_maincpu->in_pa_callback().set_ioport("POWER");
}

void inteladv_state::dyndesk(machine_config &config)
{
	ST2202(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_DATA, &inteladv_state::dyndesk_map);
	m_maincpu->in_pa_callback().set_ioport("POWER");
}

ROM_START( inteladv )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "vtechinteladv.bin", 0x000000, 0x800000, CRC(e24dbbcb) SHA1(7cb7f25f5eb123ae4c46cd4529aafd95508b2210) )
ROM_END

ROM_START( dyndesk )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "27-07710-000.u9", 0x000000, 0x200000, CRC(092b0303) SHA1(e3a58cac9b0a1c68f1bdb5ea0af0b0dd223fb340) )
	// PCB also has Hynix HY62WT081ED70C (32Kx8 CMOS SRAM) at U2
ROM_END

ROM_START( cars2lap )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// Flash dump contains some 65C02 code starting at $000D6A, but mostly appears to be custom bytecode or non-executable data banked in 32K blocks
	ROM_LOAD("n25s16.u6", 0x00000, 0x200000, CRC(ec1ba96e) SHA1(51b8844ae77adf20f74f268d380d268c9ce19785))
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME                              FLAGS
COMP( 2005, inteladv, 0,      0,      inteladv, inteladv, inteladv_state, empty_init, "VTech", "Intelligence Advance E/R (Germany)", MACHINE_IS_SKELETON )
COMP( 2003, dyndesk,  0,      0,      dyndesk,  dyndesk,  inteladv_state, empty_init, "VTech", "DynamiDesk (Germany)",               MACHINE_IS_SKELETON )
COMP( 2012, cars2lap, 0,      0,      dyndesk,  dyndesk,  inteladv_state, empty_init, "VTech", "CARS 2 Laptop (Germany)",            MACHINE_IS_SKELETON ) // probably doesn't belong here
