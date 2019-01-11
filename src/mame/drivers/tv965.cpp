// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for TeleVideo 965 video display terminal.

TeleVideo 9320 appears to run on similar hardware with a 2681 DUART replacing the ACIAs.

************************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/g65816/g65816.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
#include "screen.h"

class tv965_state : public driver_device
{
public:
	tv965_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void tv965(machine_config &config);
private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	DECLARE_READ8_MEMBER(ga_hack_r);

	void mem_map(address_map &map);
	void program_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};

SCN2672_DRAW_CHARACTER_MEMBER(tv965_state::draw_character)
{
}

READ8_MEMBER(tv965_state::ga_hack_r)
{
	return 0x08;
}

void tv965_state::mem_map(address_map &map)
{
	map(0x00000, 0x01fff).ram().share("nvram");
	map(0x02000, 0x02007).rw("crtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x04000, 0x04000).r(this, FUNC(tv965_state::ga_hack_r));
	map(0x06200, 0x06203).rw("acia1", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x06400, 0x06403).rw("acia2", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x08000, 0x09fff).ram().mirror(0x2000).share("charram");
	map(0x0c000, 0x0dfff).ram().mirror(0x2000).share("attrram");
	map(0x10000, 0x1ffff).rom().region("eprom1", 0);
	map(0x30000, 0x3ffff).rom().region("eprom2", 0);
}

void tv965_state::program_map(address_map &map)
{
	map.global_mask(0x2ffff);
	map(0x00000, 0x0ffff).rom().region("eprom1", 0);
	map(0x20000, 0x2ffff).rom().region("eprom2", 0);
}

static INPUT_PORTS_START( tv965 )
INPUT_PORTS_END

MACHINE_CONFIG_START(tv965_state::tv965)
	MCFG_DEVICE_ADD("maincpu", G65816, 44.4528_MHz_XTAL / 10)
	MCFG_DEVICE_DATA_MAP(mem_map)
	MCFG_DEVICE_PROGRAM_MAP(program_map)

	MCFG_NVRAM_ADD_0FILL("nvram") // CXK5864BP-10L + battery

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(26.9892_MHz_XTAL, 1020, 0, 800, 441, 0, 416)
	//MCFG_SCREEN_RAW_PARAMS(44.4528_MHz_XTAL, 1680, 0, 1320, 441, 0, 416)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", scn2672_device, screen_update)

	MCFG_DEVICE_ADD("crtc", SCN2672, 26.9892_MHz_XTAL / 10)
	MCFG_SCN2672_CHARACTER_WIDTH(10)
	MCFG_SCN2672_DRAW_CHARACTER_CALLBACK_OWNER(tv965_state, draw_character)
	MCFG_SCN2672_INTR_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_NMI))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("acia1", MOS6551, 0)
	MCFG_MOS6551_XTAL(3.6864_MHz_XTAL / 2) // divider not verified, possibly even programmable

	MCFG_DEVICE_ADD("acia2", MOS6551, 0)
	MCFG_MOS6551_XTAL(3.6864_MHz_XTAL / 2) // divider not verified, possibly even programmable
MACHINE_CONFIG_END

/**************************************************************************************************************

Televideo TVI-965 (P/N 132970-00)
Chips: G65SC816P-5, SCN2672TC5N40, Silicon Logic 271582-00, 2x UM6551A, Beeper, 2x MK48H64LN-70, HY6264LP-10 (next to gate array), CXK5864BP-10L, DS1231, round battery
Crystals: 44.4528, 26.9892, 3.6864

***************************************************************************************************************/

ROM_START( tv965 )
	ROM_REGION(0x10000, "eprom1", 0)
	ROM_LOAD( "180003-30h.u8", 0x00000, 0x10000, CRC(c7b9ca39) SHA1(1d95a8b0a4ea5caf3fb628c44c7a3567700a0b59) )

	ROM_REGION(0x10000, "eprom2", ROMREGION_ERASE00)
	ROM_LOAD( "180003-38h.u9", 0x00000, 0x08000, CRC(30fae408) SHA1(f05bb2a9ce2df60b046733f746d8d8a1eb3ac8bc) )
ROM_END

COMP( 1989, tv965, 0, 0, tv965, tv965, tv965_state, empty_init, "TeleVideo Systems", "TeleVideo 965", MACHINE_IS_SKELETON )
