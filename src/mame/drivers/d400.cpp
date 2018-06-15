// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Data General Dasher 400 series terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/mc68681.h"
#include "machine/x2212.h"
//#include "video/crt9007.h"
#include "screen.h"

class d400_state : public driver_device
{
public:
	d400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_novram(*this, "novram")
	{ }

	void d461(machine_config &config);
private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(novram_recall_r);
	DECLARE_READ8_MEMBER(novram_store_r);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<x2210_device> m_novram;
};

u32 d400_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ8_MEMBER(d400_state::novram_recall_r)
{
	if (!machine().side_effects_disabled())
	{
		m_novram->recall(1);
		m_novram->recall(0);
	}
	return 0xff;
}

READ8_MEMBER(d400_state::novram_store_r)
{
	if (!machine().side_effects_disabled())
	{
		m_novram->store(1);
		m_novram->store(0);
	}
	return 0xff;
}

void d400_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	//AM_RANGE(0x4000, 0x403f) AM_DEVREADWRITE("vpac", crt9007_device, read, write)
	map(0x4800, 0x48ff).ram();
	map(0x5000, 0x50ff).ram();
	map(0x6000, 0x6fff).ram();
	map(0x7800, 0x780f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x7880, 0x78bf).rw("novram", FUNC(x2210_device::read), FUNC(x2210_device::write));
	map(0x7900, 0x7900).r(FUNC(d400_state::novram_recall_r));
	map(0x7980, 0x7980).r(FUNC(d400_state::novram_store_r));
	map(0x7c00, 0x7c00).nopw();
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( d461 )
INPUT_PORTS_END

MACHINE_CONFIG_START(d400_state::d461)
	MCFG_DEVICE_ADD("maincpu", MC6809E, 4'000'000) // HD68B09EP
	MCFG_DEVICE_PROGRAM_MAP(mem_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(59'292'000) / 3, 1080, 0, 810, 305, 0, 300) // yes, 81 columns
	//MCFG_SCREEN_RAW_PARAMS(XTAL(59'292'000) / 2, 1620, 0, 1215, 305, 0, 300) // for 135-column mode
	MCFG_SCREEN_UPDATE_DRIVER(d400_state, screen_update)

	MCFG_DEVICE_ADD("novram", X2210, 0)

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL(3'686'400))
	MCFG_MC68681_IRQ_CALLBACK(INPUTLINE("maincpu", M6809_FIRQ_LINE))
MACHINE_CONFIG_END



/**************************************************************************************************************

Data General D461.
Chips: SCN2681A, X2210P, 2x HM6116P-2, 2x HM6264P-20, HD68B09EP, CRT9007, 1x 8-sw dip.
Crystals: 3.6864, 59.2920

***************************************************************************************************************/

ROM_START( d461 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "dgc_100_5776-05.bin", 0x0000, 0x8000, CRC(fdce2132) SHA1(82eac1751c31f99d4490505e16af5e7e7a52b310) )
ROM_END

COMP( 1986, d461, 0, 0, d461, d461, d400_state, empty_init, "Data General", "Dasher D461", MACHINE_IS_SKELETON )
