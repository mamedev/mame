// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for ADDS Viewpoint 122 terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "video/scn2674.h"
#include "screen.h"

class vp122_state : public driver_device
{
public:
	vp122_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_p_chargen(*this, "chargen")
	{ }

	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void vp122(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
	void vram_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_p_chargen;
};

void vp122_state::mem_map(address_map &map)
{
	map(0x0000, 0x9fff).rom().region("maincpu", 0);
	map(0xa000, 0xa7ff).ram().share("nvram");
	map(0xe000, 0xe7ff).noprw();
}

void vp122_state::io_map(address_map &map)
{
	map(0x00, 0x07).rw("avdc", FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x10, 0x1f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x20, 0x20).rw("usart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x21, 0x21).rw("usart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x50, 0x50).nopw();
	map(0x60, 0x60).nopw();
	map(0x70, 0x73).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}


SCN2674_DRAW_CHARACTER_MEMBER(vp122_state::draw_character)
{
}

void vp122_state::vram_map(address_map &map)
{
	map(0x0000, 0x07ff).noprw();
	map(0x1800, 0x2fff).noprw();
}


static INPUT_PORTS_START( vp122 )
INPUT_PORTS_END

MACHINE_CONFIG_START(vp122_state::vp122)
	MCFG_DEVICE_ADD("maincpu", I8085A, XTAL(8'000'000))
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	MCFG_NVRAM_ADD_0FILL("nvram") // MK48Z02

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(14'916'000), 960, 0, 800, 259, 0, 240)
	//MCFG_SCREEN_RAW_PARAMS(XTAL(22'096'000), 1422, 0, 1188, 259, 0, 240)
	MCFG_SCREEN_UPDATE_DEVICE("avdc", scn2674_device, screen_update)

	MCFG_DEVICE_ADD("avdc", SCN2674, XTAL(14'916'000) / 10)
	MCFG_SCN2674_INTR_CALLBACK(INPUTLINE("maincpu", I8085_RST65_LINE))
	MCFG_SCN2674_CHARACTER_WIDTH(10) // 9 in 132-column modes
	MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(vp122_state, draw_character)
	MCFG_DEVICE_ADDRESS_MAP(0, vram_map)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL(3'686'400))
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", I8085_RST55_LINE))
	// OP3 timer output likely provides 8251 serial clocks
	// OP7 = 0 for 80-column modes, 1 for 132-column modes

	MCFG_DEVICE_ADD("usart", I8251, XTAL(8'000'000) / 4)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	// Input clocks are video-related and should differ for 80-column and 132-column modes
MACHINE_CONFIG_END

/**************************************************************************************************************

ADDS Viewpoint 122 (VPT-122).
Chips: D8085AC-2, SCN2674B, SCB2675T, D8251AFC, SCN2681A, D8253C-2, 5x MB8128-15, MK48Z02B-20
Crystals: 22.096, 14.916, 3.6864, 8.000

***************************************************************************************************************/

ROM_START( vp122 )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD( "223-48600.uj1", 0x0000, 0x4000, CRC(4d140c69) SHA1(04aa5a4f0c0e0d07b9dc983a6d626ee88ef8b8ba) )
	ROM_LOAD( "223-48500.ug1", 0x4000, 0x4000, CRC(4e98554d) SHA1(0cbb9cb7efd02a3209caed410ccc8495a5ec1772) )
	ROM_LOAD( "223-49400.uj2", 0x8000, 0x4000, CRC(447d90d3) SHA1(f8c0db824198b5a571eef80cc3eaf1e829aa2c2a) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "223-48700.uk4", 0x0000, 0x2000, CRC(4dbab4bd) SHA1(18e9a23ba22e2096fa529541fa329f5a56740e62) )
ROM_END

COMP( 1985, vp122, 0, 0, vp122, vp122, vp122_state, empty_init, "ADDS", "Viewpoint 122", MACHINE_IS_SKELETON )
