// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Qume QVT-201 & QVT-202 display terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
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

	void qvt201(machine_config &config);
private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	DECLARE_WRITE8_MEMBER(offset_w);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_READ8_MEMBER(keyboard_r);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_dataram;
	required_shared_ptr<u8> m_attram;
};


SCN2672_DRAW_CHARACTER_MEMBER(qvt201_state::draw_character)
{
}

WRITE8_MEMBER(qvt201_state::offset_w)
{
}

WRITE8_MEMBER(qvt201_state::keyboard_w)
{
}

READ8_MEMBER(qvt201_state::keyboard_r)
{
	return 1;
}

void qvt201_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8800, 0x8fff).ram().share("nvram");
	map(0x9000, 0x9007).rw("crtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x9800, 0x980f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xa000, 0xa000).w(this, FUNC(qvt201_state::offset_w));
	map(0xa800, 0xa800).w(this, FUNC(qvt201_state::keyboard_w));
	map(0xb000, 0xb000).r(this, FUNC(qvt201_state::keyboard_r));
	map(0xc000, 0xdfff).ram().share("dataram");
	map(0xe000, 0xffff).ram().share("attram");
}

static INPUT_PORTS_START( qvt201 )
INPUT_PORTS_END

MACHINE_CONFIG_START(qvt201_state::qvt201)
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(3'686'400))
	MCFG_DEVICE_PROGRAM_MAP(mem_map) // IORQ is not used at all

	MCFG_INPUT_MERGER_ANY_HIGH("mainint") // open collector
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(INPUTLINE("maincpu", 0))

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL(3'686'400)) // XTAL not directly connected
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE("mainint", input_merger_device, in_w<1>))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(48'654'000) / 3, 102 * 10, 0, 80 * 10, 265, 0, 250)
	//MCFG_SCREEN_RAW_PARAMS(XTAL(48'654'000) / 2, 170 * 9, 0, 132 * 9, 265, 0, 250)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", scn2672_device, screen_update)

	MCFG_DEVICE_ADD("crtc", SCN2672, XTAL(48'654'000) / 30)
	MCFG_SCN2672_CHARACTER_WIDTH(10) // 9 in 132-column mode
	MCFG_SCN2672_INTR_CALLBACK(WRITELINE("mainint", input_merger_device, in_w<0>))
	MCFG_VIDEO_SET_SCREEN("screen")
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

COMP( 1986, qvt201, 0, 0, qvt201, qvt201, qvt201_state, empty_init, "Qume", "QVT-201 (Rev. T201VE)", MACHINE_IS_SKELETON )
