// license:BSD-3-Clause
// copyright-holders:AJR
/************************************************************************************************************

    Skeleton driver for Cablenet, Inc. 2039 coax/twinax Lexmark printer controller card.

    Main CPU is a Chips & Technologies F8680 PC/CHIP (x86 with custom opcodes).

************************************************************************************************************/

#include "emu.h"
//#include "cpu/i86/f8680.h"
#include "cpu/bcp/dp8344.h"
//#include "machine/eepromser.h"


namespace {

class cbnt2039_state : public driver_device
{
public:
	cbnt2039_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bcp(*this, "bcp")
	{
	}

	void cbnt2039(machine_config &config);

private:
	void bcp_prog_map(address_map &map) ATTR_COLD;
	void bcp_data_map(address_map &map) ATTR_COLD;

	required_device<dp8344_device> m_bcp;
};

void cbnt2039_state::bcp_prog_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("sbprom", 0x10906); // FIXME: should be RAM (2x MCM6264CP25), with code uploaded by F8680
}

void cbnt2039_state::bcp_data_map(address_map &map)
{
}


static INPUT_PORTS_START(cbnt2039)
INPUT_PORTS_END


void cbnt2039_state::cbnt2039(machine_config &config)
{
	//F8680(config, m_maincpu, OSC1);

	DP8344B(config, m_bcp, 18.8696_MHz_XTAL); // DP8344BV; Y1 = "RXD8.000"; Y2 = "RXD18.86"
	m_bcp->set_addrmap(AS_PROGRAM, &cbnt2039_state::bcp_prog_map);
	m_bcp->set_addrmap(AS_DATA, &cbnt2039_state::bcp_data_map);

	//EEPROM_93C56_16BIT(config, "eeprom");
}


ROM_START(cbnt2039)
	ROM_REGION16_LE(0x20000, "sbprom", 0)
	ROM_LOAD("sbprom_27c_2ea2-1001.u12", 0x00000, 0x20000, CRC(4ee02833) SHA1(17c8b02bbef7b855a91dfb8bd9758ffb5cc9b9e7)) // handwritten label
ROM_END

} // anonymous namespace


COMP(1993, cbnt2039, 0, 0, cbnt2039, cbnt2039, cbnt2039_state, empty_init, "Cablenet", "2039 Controller", MACHINE_IS_SKELETON)
