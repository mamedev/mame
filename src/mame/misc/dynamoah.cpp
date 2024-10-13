// license:BSD-3-Clause
// copyright-holders:AJR
/******************************************************************************

    Skeleton driver for Dynamo air hockey games.

    There are mostly-illegible scans of schematics for earlier Dynamo air
    hockey controllers, which were based on a 8748 MCU with internal ROM.
    However, the boards Dynamo later made in the 1990s, including the one
    represented here, employ a 8031 with external ROM, sometimes with a
    GAL20V8 for encryption.

******************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/eepromser.h"

#define VERBOSE 0
#include "logmacro.h"


namespace {

class dynamoah_state : public driver_device
{
public:
	dynamoah_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void dynamoah(machine_config &config);
	void security_decrypt();

private:
	void p1_w(uint8_t data);
	void p3_w(uint8_t data);
	uint8_t ext_r(offs_t offset);
	void ext_w(offs_t offset, uint8_t data);

	void i8031_mem(address_map &map) ATTR_COLD;
	void i8031_ext_mem(address_map &map) ATTR_COLD;
};

void dynamoah_state::p1_w(uint8_t data)
{
	if (data != 0x1f)
		LOG("%s: P1 = %02X\n", machine().describe_context(), data);
}

void dynamoah_state::p3_w(uint8_t data)
{
	LOG("%s: P3 = %02X\n", machine().describe_context(), data);
}

uint8_t dynamoah_state::ext_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOG("%s: RD(%04X)\n", machine().describe_context(), offset);
	return 0xff;
}

void dynamoah_state::ext_w(offs_t offset, uint8_t data)
{
	LOG("%s: WR(%04X, %02X)\n", machine().describe_context(), offset, data);
}

void dynamoah_state::i8031_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
}

void dynamoah_state::i8031_ext_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(dynamoah_state::ext_r), FUNC(dynamoah_state::ext_w));
}

void dynamoah_state::dynamoah(machine_config &config)
{
	i80c31_device &maincpu(I80C31(config, "maincpu", 6'144'000)); // SC80C31BCCN40
	// clock needs verification, being the XTAL value from a different board
	maincpu.set_addrmap(AS_PROGRAM, &dynamoah_state::i8031_mem);
	maincpu.set_addrmap(AS_IO, &dynamoah_state::i8031_ext_mem);
	maincpu.port_in_cb<1>().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	maincpu.port_in_cb<1>().append_constant(0x7f).mask(0x7f);
	maincpu.port_out_cb<1>().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(7);
	maincpu.port_out_cb<1>().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(6);
	maincpu.port_out_cb<1>().append("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	maincpu.port_out_cb<1>().append(FUNC(dynamoah_state::p1_w)).mask(0x1f);
	maincpu.port_out_cb<3>().set(FUNC(dynamoah_state::p3_w));

	EEPROM_93C46_16BIT(config, "eeprom");
}

static INPUT_PORTS_START(dynamoah)
INPUT_PORTS_END

ROM_START(dynamoah)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("air_hock_6.12", 0x0000, 0x2000, CRC(8e3847ff) SHA1(b8885f957bf324185583765a2879d850997bf615)) // 2764A; from "new default file" Air_hock.mix
ROM_END

ROM_START(dynamoaha)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("a-hocky6.03", 0x0000, 0x2000, CRC(6a4ff3e4) SHA1(b38637e0e9dc046e3b9e48da84fba29e23db9585)) // 2764A
ROM_END

void dynamoah_state::security_decrypt()
{
	u8 *romdata = memregion("maincpu")->base();
	for (offs_t addr = 0; addr < 0x2000; addr++)
	{
		// This method seems plausible; some resulting code bugs may be original
		if (BIT(addr, 0))
			romdata[addr] = bitswap<8>(romdata[addr], 6, 0, 3, 5, 4, 1, 2, 7);
		else
			romdata[addr] = bitswap<8>(romdata[addr], 4, 1, 2, 7, 6, 0, 3, 5);
	}
}

} // anonymous namespace


GAME( 199?, dynamoah,  0,        dynamoah, dynamoah, dynamoah_state, security_decrypt, ROT0, "Dynamo", "Air Hockey (6.12?, encrypted)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 199?, dynamoaha, dynamoah, dynamoah, dynamoah, dynamoah_state, security_decrypt, ROT0, "Dynamo", "Air Hockey (6.03, encrypted)", MACHINE_IS_SKELETON_MECHANICAL )
