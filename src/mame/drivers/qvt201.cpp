// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Qume QVT-201 & QVT-202 display terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
//#include "video/scn2672.h"
#include "screen.h"

class qvt201_state : public driver_device
{
public:
	qvt201_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_p_chargen(*this, "chargen")
		, m_dataram(*this, "dataram")
		, m_attram(*this, "attram")
	{ }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void qvt201(machine_config &config);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_dataram;
	required_shared_ptr<u8> m_attram;
};


u32 qvt201_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

ADDRESS_MAP_START(qvt201_state::mem_map)
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x8800, 0x8fff) AM_RAM AM_SHARE("nvram")
	//AM_RANGE(0x9000, 0x9007) AM_DEVREADWRITE("crtc", scn2672_device, read, write)
	AM_RANGE(0x9800, 0x980f) AM_DEVREADWRITE("duart", scn2681_device, read, write)
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("dataram")
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("attram")
ADDRESS_MAP_END

static INPUT_PORTS_START( qvt201 )
INPUT_PORTS_END

MACHINE_CONFIG_START(qvt201_state::qvt201)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(3'686'400))
	MCFG_CPU_PROGRAM_MAP(mem_map) // IORQ is not used at all

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL(3'686'400)) // XTAL not directly connected

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(48'654'000) / 3, 102 * 10, 0, 80 * 10, 265, 0, 250)
	//MCFG_SCREEN_RAW_PARAMS(XTAL(48'654'000) / 2, 170 * 9, 0, 132 * 9, 265, 0, 250)
	MCFG_SCREEN_UPDATE_DRIVER(qvt201_state, screen_update)

	//MCFG_DEVICE_ADD("crtc", SCN2672, CHAR_CLOCK)
	//MCFG_SCN2672_CHAR_WIDTH(9 or 10)
MACHINE_CONFIG_END


/**************************************************************************************************************

Qume QVT-201.
Chips: Z80A, SCN2681A, SCN2672B, 4x HM6116P-2, D446C-2, button battery
Crystals: (from schematics, unreadable on photo) 48.654 MHz (Y1), 3.6864 MHz (Y2)
Board is marked QVT-202 LB10 REV2 74 6 26.
Printed label on PCB: 301488-02 REV.2
                      MFG:607   QC:PASS

***************************************************************************************************************/

ROM_START( qvt201 )
	ROM_REGION(0x8000, "maincpu", 0) // "Program Contents ©1986 Qume Corp."
	ROM_LOAD( "390410-002.u11", 0x0000, 0x4000, CRC(69337561) SHA1(022e49bf5e8d76a3c2cc5af65630d3f77cc32bc1) )
	ROM_LOAD( "390410-001.u10", 0x4000, 0x4000, CRC(977cc138) SHA1(a019980ea6da2dce53617bced420014ab4e03ec8) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "301847-01.u42",  0x0000, 0x1000, CRC(546ed236) SHA1(312d57a7012f50327310bd11bda000149f13342e) )
ROM_END

COMP( 1986, qvt201, 0, 0, qvt201, qvt201, qvt201_state, 0, "Qume", "QVT-201 (Rev. T201VE)", MACHINE_IS_SKELETON )
