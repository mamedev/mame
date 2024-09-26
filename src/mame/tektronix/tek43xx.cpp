// license:BSD-3-Clause
// copyright-holders:AJR
/************************************************************************************************************

    Skeleton driver for Tektronix 4300 series MC68020-based graphics workstations.

************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68020.h"
#include "screen.h"


namespace {

class tek43xx_state : public driver_device
{
public:
	tek43xx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void tek4319(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


u32 tek43xx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tek43xx_state::mem_map(address_map &map)
{
	map(0x00000000, 0x00000007).rom().region("program", 0);
	map(0x00000008, 0x000fffff).ram();
	map(0x01cb0000, 0x01cb0003).nopr();
	map(0x10000000, 0x1001ffff).rom().region("program", 0);
}

static INPUT_PORTS_START(tek4319)
INPUT_PORTS_END

void tek43xx_state::tek4319(machine_config &config)
{
	M68020(config, m_maincpu, 20'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tek43xx_state::mem_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(60 * 1600 * 1066, 1600, 0, 1280, 1066, 0, 1024); // not confirmed except for displayed resolution
	screen.set_screen_update(FUNC(tek43xx_state::screen_update));
}

ROM_START(tek4319)
	ROM_REGION32_BE(0x20000, "program", 0)
	ROM_LOAD16_BYTE("160-5528-00.bin", 0x00000, 0x10000, CRC(a04ffe7f) SHA1(601c27c47138c38dc54b9cbabca8467170cc580b))
	ROM_LOAD16_BYTE("160-5528-01.bin", 0x00001, 0x10000, CRC(869ec49f) SHA1(145a20272b1f9eb42618d0b4aefe9ea79a27dcce))
ROM_END

} // anonymous namespace

COMP(1988, tek4319, 0, 0, tek4319, tek4319, tek43xx_state, empty_init, "Tektronix", "4319 Graphics Workstation", MACHINE_IS_SKELETON)
