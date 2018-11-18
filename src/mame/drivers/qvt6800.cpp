// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for M6800-based display terminals by Qume.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "video/mc6845.h"
#include "screen.h"

class qvt6800_state : public driver_device
{
public:
	qvt6800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_videoram(*this, "videoram")
	{ }

	void qvt190(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);

	void qvt190_mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_videoram;
};

MC6845_UPDATE_ROW(qvt6800_state::update_row)
{
}

void qvt6800_state::qvt190_mem_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2500, 0x2501).rw("acia1", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x2600, 0x2601).rw("acia2", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x2800, 0x2800).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x2801, 0x2801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x47ff).ram().share("videoram");
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( qvt6800 )
INPUT_PORTS_END

MACHINE_CONFIG_START(qvt6800_state::qvt190)
	MCFG_DEVICE_ADD("maincpu", M6800, XTAL(16'669'800) / 9)
	MCFG_DEVICE_PROGRAM_MAP(qvt190_mem_map)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // V61C16P55L + battery

	MCFG_DEVICE_ADD("acia1", ACIA6850, 0)

	MCFG_DEVICE_ADD("acia2", ACIA6850, 0)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(16'669'800), 882, 0, 720, 315, 0, 300)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_DEVICE_ADD("crtc", MC6845, XTAL(16'669'800) / 9)
	MCFG_MC6845_CHAR_WIDTH(9)
	MCFG_MC6845_UPDATE_ROW_CB(qvt6800_state, update_row)
	MCFG_VIDEO_SET_SCREEN("screen")
MACHINE_CONFIG_END

/**************************************************************************************************************

Qume QVT-190.
Chips: MC68B00P, 2x MC68B50P, MC68B45P, V61C16P55L, M5M5165P-70L, ABHGA101006, button battery, 7-DIL-jumper
Crystal: unreadable (but likely to be 16.6698)

***************************************************************************************************************/

ROM_START( qvt190 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "95987-267.u19", 0x0000, 0x8000, CRC(78894d8e) SHA1(0a0f6883dd18872bddeb3ed18ebe496080e6591b) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "95864-304.u17", 0x0000, 0x2000, CRC(2792e99b) SHA1(4a84d029d0e63975fc95dc7056d2523193dff986) )
ROM_END

COMP( 1987, qvt190, 0, 0, qvt190, qvt6800, qvt6800_state, empty_init, "Qume", "QVT-190", MACHINE_IS_SKELETON )
