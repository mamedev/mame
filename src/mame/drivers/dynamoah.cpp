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
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_WRITE8_MEMBER(p3_w);
	DECLARE_READ8_MEMBER(ext_r);
	DECLARE_WRITE8_MEMBER(ext_w);

	void i8031_mem(address_map &map);
	void i8031_ext_mem(address_map &map);
};

WRITE8_MEMBER(dynamoah_state::p1_w)
{
	if (data != 0x1f)
		LOG("%s: P1 = %02X\n", machine().describe_context(), data);
}

WRITE8_MEMBER(dynamoah_state::p3_w)
{
	LOG("%s: P3 = %02X\n", machine().describe_context(), data);
}

READ8_MEMBER(dynamoah_state::ext_r)
{
	if (!machine().side_effects_disabled())
		LOG("%s: RD(%04X)\n", machine().describe_context(), offset);
	return 0xff;
}

WRITE8_MEMBER(dynamoah_state::ext_w)
{
	LOG("%s: WR(%04X, %02X)\n", machine().describe_context(), offset, data);
}

void dynamoah_state::i8031_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
}

void dynamoah_state::i8031_ext_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(this, FUNC(dynamoah_state::ext_r), FUNC(dynamoah_state::ext_w));
}

MACHINE_CONFIG_START(dynamoah_state::dynamoah)
	MCFG_DEVICE_ADD("maincpu", I80C31, 6'144'000) // SC80C31BCCN40
	// clock needs verification, being the XTAL value from a different board
	MCFG_DEVICE_PROGRAM_MAP(i8031_mem)
	MCFG_DEVICE_IO_MAP(i8031_ext_mem)
	MCFG_MCS51_PORT_P1_IN_CB(READLINE("eeprom", eeprom_serial_93cxx_device, do_read)) MCFG_DEVCB_BIT(7)
	MCFG_DEVCB_CHAIN_INPUT(CONSTANT(0x7f))
	MCFG_MCS51_PORT_P1_OUT_CB(WRITELINE("eeprom", eeprom_serial_93cxx_device, di_write)) MCFG_DEVCB_BIT(7)
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("eeprom", eeprom_serial_93cxx_device, cs_write)) MCFG_DEVCB_BIT(6)
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("eeprom", eeprom_serial_93cxx_device, clk_write)) MCFG_DEVCB_BIT(5)
	MCFG_DEVCB_CHAIN_OUTPUT(WRITE8(*this, dynamoah_state, p1_w)) MCFG_DEVCB_MASK(0x1f)
	MCFG_MCS51_PORT_P3_OUT_CB(WRITE8(*this, dynamoah_state, p3_w))

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
MACHINE_CONFIG_END

static INPUT_PORTS_START(dynamoah)
INPUT_PORTS_END

ROM_START(dynamoah)
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

GAME( 199?, dynamoah, 0, dynamoah, dynamoah, dynamoah_state, security_decrypt, ROT0, "Dynamo", "Air Hockey (6.03, encrypted)", MACHINE_IS_SKELETON_MECHANICAL )
