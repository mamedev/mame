// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Hohner ADAM keyboard synthesizer.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"

namespace {

class hohnadam_state : public driver_device
{
public:
	hohnadam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_panelcpu(*this, "panelcpu")
	{
	}

	void hohnadam(machine_config &config);

private:
	u8 c005fb_r();
	void d00000_w(offs_t offset, u8 data);
	void d40000_w(offs_t offset, u8 data);
	void d80000_w(offs_t offset, u8 data);

	void main_map(address_map &map) ATTR_COLD;
	void panel_map(address_map &map) ATTR_COLD;
	void panel_ext_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<mcs51_cpu_device> m_panelcpu;
};

u8 hohnadam_state::c005fb_r()
{
	return 0x20;
}

void hohnadam_state::d00000_w(offs_t offset, u8 data)
{
}

void hohnadam_state::d40000_w(offs_t offset, u8 data)
{
}

void hohnadam_state::d80000_w(offs_t offset, u8 data)
{
}

void hohnadam_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("program", 0);
	map(0x400000, 0x5fffff).ram();
	map(0x800000, 0x80ffff).ram().share("nvram");
	map(0xc004f2, 0xc004f3).nopr();
	map(0xc005fb, 0xc005fb).r(FUNC(hohnadam_state::c005fb_r));
	map(0xc007e4, 0xc007e5).nopw();
	map(0xc007e8, 0xc007e9).nopr();
	map(0xc007f2, 0xc007f3).noprw();
	map(0xd00000, 0xd00007).w(FUNC(hohnadam_state::d00000_w)).umask16(0x00ff);
	map(0xd40000, 0xd40009).w(FUNC(hohnadam_state::d40000_w)).umask16(0x00ff);
	map(0xd80000, 0xd80009).w(FUNC(hohnadam_state::d80000_w)).umask16(0x00ff);
}

void hohnadam_state::panel_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("panel", 0);
}

void hohnadam_state::panel_ext_map(address_map &map)
{
	map(0x001d, 0x001d).noprw();
}

static INPUT_PORTS_START(hohnadam)
INPUT_PORTS_END

void hohnadam_state::hohnadam(machine_config &config)
{
	M68000(config, m_maincpu, 12000000); // MC68000FN12, unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &hohnadam_state::main_map);

	I8032(config, m_panelcpu, 12000000); // unknown type and clock
	m_panelcpu->set_addrmap(AS_PROGRAM, &hohnadam_state::panel_map);
	m_panelcpu->set_addrmap(AS_IO, &hohnadam_state::panel_ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2x HY62256ALJ-70 + CR2032 battery

	//F82C721(config, "io", 24000000);

	// TODO: LCD, everything else
}

ROM_START(hohnadam)
	ROM_REGION16_BE(0x100000, "program", 0)
	ROM_LOAD16_BYTE("adam u33_2 version 1.1.bin", 0x00000, 0x80000, CRC(e5d60622) SHA1(9e105ed6d403ecf6713cd4c6643fab9586f25217)) // TMS27C040-10
	ROM_LOAD16_BYTE("adam u32_2 version 1.1.bin", 0x00001, 0x80000, CRC(7c52d112) SHA1(34b1f88ae4a2a575a41e542ab41159b25cb6fc6e)) // TMS27C040-10

	ROM_REGION(0x20000, "panel", 0)
	ROM_LOAD("adam panel version 1.1.bin", 0x00000, 0x20000, CRC(69c45992) SHA1(91c278defdf790ecdd3ff6df90e29d0bf508ba44)) // TMS27C010A-10

	ROM_REGION16_BE(0x300000, "waveroms", 0)
	ROM_LOAD16_BYTE("adam u36 1.bin", 0x000001, 0x080000, CRC(879b0acb) SHA1(7b86069e0939608ea2863fc56a91c344c8360add)) // Am27C040-150DC
	ROM_LOAD16_BYTE("adam u37_2.bin", 0x100001, 0x080000, CRC(dadc8167) SHA1(e37c46651868bb89dc9c768d46de34f1aedfbac9)) // Am27C040-150DC
	ROM_LOAD16_BYTE("adam u38_2.bin", 0x200001, 0x080000, CRC(d522acb3) SHA1(e4941163d4232a7ed119848e900c4848bef181f9)) // Am27C040-150DC
	ROM_LOAD16_BYTE("adam u39_2.bin", 0x000000, 0x080000, CRC(70990980) SHA1(7773d2e184b11cd83ac9682699bb7790d4455e6b)) // Am27C040-150DC
	ROM_LOAD16_BYTE("adam u40 1.bin", 0x100000, 0x080000, CRC(6cfa9a26) SHA1(3d07b3d3c2934dd8c917da6115a9786da64ba98f)) // Am27C040-150DC
	ROM_LOAD16_BYTE("adam u41_2.bin", 0x200000, 0x080000, CRC(46af2b27) SHA1(bb8d75afd65d25678797ee4b34cce3bcb7f13e5b)) // Am27C040-150DC

	ROM_REGION(0x400000, "maskroms", 0)
	ROM_LOAD("dream-gms931600n.u47", 0x000000, 0x200000, NO_DUMP)
	ROM_LOAD("dream-gms931601n.u48", 0x200000, 0x200000, NO_DUMP)
ROM_END

} // anonymous namespace

SYST(1994, hohnadam, 0, 0, hohnadam, hohnadam, hohnadam_state, empty_init, "Hohner", "ADAM Advanced Digital/Analog Musical Instrument", MACHINE_IS_SKELETON)
