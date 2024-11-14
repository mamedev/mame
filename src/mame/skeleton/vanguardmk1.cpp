// license:BSD-3-Clause
// copyright-holders:

/*
    Skeleton driver for Vanguard MK1 multipurpose bomb disposal robot by EOD Performance Inc.
*/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"


namespace {

class vanguardmk1_state : public driver_device
{
public:
	vanguardmk1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		{ }

	void vanguardmk1(machine_config &config);

private:
	void mcu_map(address_map &map) ATTR_COLD;
};

void vanguardmk1_state::mcu_map(address_map &map)
{
}


static INPUT_PORTS_START( vanguardmk1 )
INPUT_PORTS_END

void vanguardmk1_state::vanguardmk1(machine_config &config)
{
	mc68hc811e2_device &mcu(MC68HC811E2(config, "mcu", 8000000)); // unknown clock
	mcu.set_addrmap(AS_PROGRAM, &vanguardmk1_state::mcu_map);
	mcu.set_default_config(0xff);
}


ROM_START(vngrdmk1)
	ROM_REGION(0x800, "mcu:eeprom", 0)
	ROM_LOAD( "van24_aug04", 0x000, 0x800, CRC(ce63fcb9) SHA1(8f688e866e8fea888c77aa5be92ad09f684afd59) )
ROM_END

} // anonymous namespace


SYST( 2004?, vngrdmk1, 0, 0, vanguardmk1, vanguardmk1, vanguardmk1_state, empty_init, "EOD Performance Inc.", "Vanguard MK1", MACHINE_IS_SKELETON_MECHANICAL )
