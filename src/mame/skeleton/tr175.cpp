// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Relisys TR-175 II color terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "video/ramdac.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class tr175_state : public driver_device
{
public:
	tr175_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void tr175(machine_config &config);

private:
	void ffec01_w(uint8_t data);
	void fff000_w(uint8_t data);
	uint8_t fff400_r();
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
	void vram_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void tr175_state::ffec01_w(uint8_t data)
{
	logerror("%s: Writing %02X to FFEC01\n", machine().describe_context(), data);
}

void tr175_state::fff000_w(uint8_t data)
{
	logerror("%s: Writing %02X to FFF000\n", machine().describe_context(), data);
}

uint8_t tr175_state::fff400_r()
{
	return 0;
}

void tr175_state::mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("maincpu", 0);
	map(0xfe8000, 0xfebfff).ram(); // 8-bit?
	map(0xfefe00, 0xfefedd).nopw(); // 8-bit; cleared at startup
	map(0xff8000, 0xffbfff).ram(); // main RAM
	map(0xff0000, 0xff7fff).ram(); // video RAM?
	map(0xffe000, 0xffe01f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0xff00);
	map(0xffe400, 0xffe40f).rw("avdc", FUNC(scn2674_device::read), FUNC(scn2674_device::write)).umask16(0xff00);
	map(0xffe800, 0xffe805).unmaprw(); //.rw("pai", FUNC(um82c11_device::read), FUNC(um82c11_device::write)).umask16(0xff00);
	map(0xffec01, 0xffec01).w(FUNC(tr175_state::ffec01_w));
	map(0xfff000, 0xfff000).w(FUNC(tr175_state::fff000_w));
	map(0xfff400, 0xfff400).r(FUNC(tr175_state::fff400_r));
	map(0xfffc01, 0xfffc01).w("ramdac", FUNC(ramdac_device::index_w));
	map(0xfffc03, 0xfffc03).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0xfffc05, 0xfffc05).w("ramdac", FUNC(ramdac_device::mask_w));
}

SCN2674_DRAW_CHARACTER_MEMBER(tr175_state::draw_character)
{
}

void tr175_state::vram_map(address_map &map)
{
	map(0x0000, 0x3fff).nopr();
}

void tr175_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

static INPUT_PORTS_START( tr175 )
INPUT_PORTS_END

void tr175_state::tr175(machine_config &config)
{
	M68000(config, m_maincpu, 12'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tr175_state::mem_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(28.322_MHz_XTAL, 900, 0, 720, 449, 0, 416); // guess
	screen.set_screen_update("avdc", FUNC(scn2674_device::screen_update));

	scn2674_device &avdc(SCN2674(config, "avdc", 28.322_MHz_XTAL / 18)); // guess
	avdc.intr_callback().set_inputline("maincpu", M68K_IRQ_2);
	avdc.set_character_width(18); // guess
	avdc.set_display_callback(FUNC(tr175_state::draw_character));
	avdc.set_addrmap(0, &tr175_state::vram_map);
	avdc.set_screen("screen");

	scn2681_device &duart(SCN2681(config, "duart", 11.0592_MHz_XTAL / 3)); // is this the right clock?
	duart.irq_cb().set_inputline("maincpu", M68K_IRQ_1);

	PALETTE(config, "palette").set_entries(0x100);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, "palette"));
	ramdac.set_addrmap(0, &tr175_state::ramdac_map);
}



/**************************************************************************************************************

Relisys TR-175 II.
Chips: MC68000P12, HM82C11C, SCN2681, 3x W24257-70L, KDA0476BCN-66 (RAMDAC), 4 undumped proms, Beeper, Button battery
Crystals: 28.322, 46.448, 11.0592, unknown.
Colour screen (VGA).

***************************************************************************************************************/

ROM_START( tr175 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD16_BYTE( "v6.05.u50", 0x00001, 0x10000, CRC(5a33b6b3) SHA1(d673f50dd88f8a154ddaabe34cfcc9ab91435a4c) )
	ROM_LOAD16_BYTE( "v6.05.u45", 0x00000, 0x10000, CRC(e220befe) SHA1(8402280577e6de4b85843222bbd6b06a3f625b3b) )
ROM_END

} // anonymous namespace


COMP( 1982, tr175, 0, 0, tr175, tr175, tr175_state, empty_init, "Relisys", "TR-175 II", MACHINE_IS_SKELETON )
