// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Data General Dasher 400 series terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/mc68681.h"
#include "machine/x2212.h"
#include "video/crt9007.h"
#include "screen.h"


namespace {

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

	u8 novram_recall_r();
	u8 novram_store_r();
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<x2210_device> m_novram;
};

u32 d400_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u8 d400_state::novram_recall_r()
{
	if (!machine().side_effects_disabled())
	{
		m_novram->recall(1);
		m_novram->recall(0);
	}
	return 0xff;
}

u8 d400_state::novram_store_r()
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
	map(0x4000, 0x403f).rw("vpac", FUNC(crt9007_device::read), FUNC(crt9007_device::write));
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

static INPUT_PORTS_START(d461)
INPUT_PORTS_END

void d400_state::d461(machine_config &config)
{
	MC6809E(config, m_maincpu, 59.292_MHz_XTAL / 30); // HD68B09EP (clock not verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &d400_state::mem_map);

	X2210(config, "novram");

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(59.292_MHz_XTAL / 3, 1080, 0, 810, 305, 0, 300); // yes, 81 columns
	//screen.set_raw(59.292_MHz_XTAL / 2, 1620, 0, 1215, 305, 0, 300); // for 135-column mode
	screen.set_screen_update(FUNC(d400_state::screen_update));

	crt9007_device &vpac(CRT9007(config, "vpac", 59.292_MHz_XTAL / 30));
	vpac.set_screen("screen");
	vpac.set_character_width(10); // 9 in 135-column mode
	vpac.int_callback().set_inputline(m_maincpu, M6809_IRQ_LINE);
}



/**************************************************************************************************************

Data General D461.
Chips: SCN2681A, X2210P, 2x HM6116P-2, 2x HM6264P-20, HD68B09EP, CRT9007, 1x 8-sw dip.
Crystals: 3.6864, 59.2920

***************************************************************************************************************/

ROM_START( d461 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "dgc_100_5776-05.bin", 0x0000, 0x8000, CRC(fdce2132) SHA1(82eac1751c31f99d4490505e16af5e7e7a52b310) )
ROM_END

} // anonymous namespace


COMP( 1986, d461, 0, 0, d461, d461, d400_state, empty_init, "Data General", "Dasher D461", MACHINE_IS_SKELETON )
