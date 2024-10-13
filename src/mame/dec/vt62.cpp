// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for DEC VT61 and VT62 terminals.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/vt61/vt61.h"
#include "machine/ay31015.h"
//#include "sound/spkrdev.h"
#include "screen.h"
//#include "speaker.h"

namespace {

class vt62_state : public driver_device
{
public:
	vt62_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
	{
	}

	void vt62(machine_config &mconfig);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void micro_map(address_map &map) ATTR_COLD;
	void memory_map(address_map &map) ATTR_COLD;
	void decode_map(address_map &map) ATTR_COLD;

	required_device<vt61_cpu_device> m_maincpu;
	required_device<ay31015_device> m_uart;
	//required_ioport_array<8> m_keys;
	//required_ioport m_baud_sw;
};

void vt62_state::machine_reset()
{
}

u32 vt62_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vt62_state::micro_map(address_map &map)
{
	map(00000, 01777).rom().region("crom", 0);
}

void vt62_state::memory_map(address_map &map)
{
	map(0000000, 0000377).mirror(0047400).ram(); // static RAM A
	map(0010000, 0013777).mirror(0040000).ram(); // dynamic RAM B
	map(0020000, 0023777).mirror(0040000).ram(); // dynamic RAM C
	map(0100000, 0117777).mirror(0060000).rom().region("mrom", 0);
}

void vt62_state::decode_map(address_map &map)
{
	map(000, 077).rom().region("idr", 0);
}

static INPUT_PORTS_START(vt62)
INPUT_PORTS_END

void vt62_state::vt62(machine_config &mconfig)
{
	VT61_CPU(mconfig, m_maincpu, 15.36_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt62_state::micro_map);
	m_maincpu->set_addrmap(AS_DATA, &vt62_state::memory_map);
	m_maincpu->set_addrmap(vt61_cpu_device::AS_IDR, &vt62_state::decode_map);

	AY51013(mconfig, m_uart);

	screen_device &screen(SCREEN(mconfig, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(15.36_MHz_XTAL, 1000, 0, 800, 256, 0, 240);
	screen.set_screen_update(FUNC(vt62_state::screen_update));
}

ROM_START(vt62)
	ROM_REGION16_LE(0x800, "crom", 0) // control ROM (microprogram)
	ROMX_LOAD("23-187f1_82s137.e93", 0x001, 0x400, CRC(73796e58) SHA1(38f8c984e7fb99d79fc13acc3a3b1e92aa136ad2), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD("23-188f1_82s137.e94", 0x001, 0x400, CRC(198831a9) SHA1(574552dbae185b6c2168313d4e8a1d87e88c1883), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))
	ROMX_LOAD("23-186f1_82s137.e90", 0x000, 0x400, CRC(b5691aa1) SHA1(d84cdbcf90dc687c56ac4e7f5d4372451d1afd7b), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1))
	ROMX_LOAD("23-189f1_82s137.e95", 0x000, 0x400, CRC(05a9437b) SHA1(160c57658070ee7569fc12414f6250261e2f5e8a), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1))

	ROM_REGION(0x2000, "mrom", 0) // macroprogram/user ROM (on memory board)
	ROM_LOAD("e3", 0x0000, 0x0800, NO_DUMP) // UPROMs may be substituted at E24, E23, E28, E27
	ROM_LOAD("e8", 0x0800, 0x0800, NO_DUMP) // UPROMs may be substituted at E32, E31, E36, E35
	ROM_LOAD("e13", 0x1000, 0x0800, NO_DUMP) // UPROMs may be substituted at E41, E40, E46, E45
	ROM_LOAD("e20", 0x1800, 0x0800, NO_DUMP) // UPROMs may be substituted at E51, E50, E55, E54

	ROM_REGION(0x40, "idr", 0) // macro instruction decode
	ROM_LOAD("23-195a1_82s23.e13", 0x00, 0x20, CRC(cd8d8020) SHA1(cd10097d7fc62676b62387495615f1e06a689cd3))
	ROM_LOAD("23-194a1_82s23.e12", 0x20, 0x20, CRC(b05df6b5) SHA1(e13b7b0f75dbbc0d262606280bd5cf8be3561849))

	ROM_REGION(0x20, "alu", 0) // ALU function decode (same as VT61)
	ROM_LOAD("23-114a1.e41", 0x00, 0x20, NO_DUMP)

	ROM_REGION(0x400, "cgrom", 0) // character generators (same as VT61)
	ROM_LOAD_NIB_HIGH("23-053a9_82s131.e24", 0x000, 0x100, CRC(9a242be8) SHA1(4c619b6c0cfdda4af097b5702e45fad78ed6d601))
	ROM_CONTINUE(                            0x300, 0x100)
	ROM_LOAD_NIB_LOW( "23-052a9_82s131.e23", 0x000, 0x100, CRC(6c09ac7b) SHA1(e50060489500b1ca1b89530209c59e610c605c3d))
	ROM_CONTINUE(                            0x300, 0x100)
	ROM_LOAD_NIB_HIGH("23-051a9_82s131.e17", 0x200, 0x100, CRC(089ea972) SHA1(210bf4e5d32e49e1d7183f13d3b81ce446f49551))
	ROM_CONTINUE(                            0x100, 0x100)
	ROM_LOAD_NIB_LOW( "23-050a9_82s131.e16", 0x200, 0x100, CRC(758125a1) SHA1(b1de4448ea90da07f2aacb665542545bbcd17371))
	ROM_CONTINUE(                            0x100, 0x100)
ROM_END

} // anonymous namespace


//COMP(1977, vt61t, 0, 0, vt61t, vt61t, vt61_state, empty_init, "Digital Equipment Corporation", "VT61/t", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP(1978, vt62, 0, 0, vt62, vt62, vt62_state, empty_init, "Digital Equipment Corporation", "VT62 DECscope", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
