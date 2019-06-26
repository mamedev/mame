// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for AT&T 630 MTG terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "screen.h"

class att630_state : public driver_device
{
public:
	att630_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vram(*this, "vram")
	{ }

	void att630(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u16> m_vram;
};

u32 att630_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 1024; y++)
		for (int x = 0; x < 1024; x++)
			bitmap.pix32(y, x) = BIT(m_vram[y * (1024 / 16) + x / 16], 15 - (x % 16)) ? rgb_t::white() : rgb_t::black();

	return 0;
}

void att630_state::mem_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x040000, 0x0fffff).noprw(); // additional space for rom
	map(0x100000, 0x1fffff).noprw(); // cartridge space
	map(0x200000, 0x20001f).rw("duart1", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x200020, 0x20003f).rw("duart2", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x400000, 0x6fffff).noprw(); // expansion i/o card
//	map(0x700000, 0x75ffff).noprw(); // video controller
	map(0x760000, 0x77ffff).ram().share("vram");
	map(0x780000, 0x7fffff).ram(); // program ram
	map(0xe00000, 0xe03fff).noprw(); // 0x00ff mask (bram)
	map(0xfee000, 0xffffff).noprw(); // nvram (size unknown)
}

static INPUT_PORTS_START( att630 )
INPUT_PORTS_END

void att630_state::att630(machine_config &config)
{
	M68000(config, m_maincpu, 40_MHz_XTAL / 4); // clock not confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &att630_state::mem_map);
	// TODO: interrupt vector callback

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(87.18336_MHz_XTAL, 1376, 0, 1024, 1056, 0, 1024);
	screen.set_screen_update(FUNC(att630_state::screen_update));

	scn2681_device &duart1(SCN2681(config, "duart1", 3.6864_MHz_XTAL));
	duart1.irq_cb().set_inputline(m_maincpu, M68K_IRQ_1);

	scn2681_device &duart2(SCN2681(config, "duart2", 3.6864_MHz_XTAL));
	duart2.irq_cb().set_inputline(m_maincpu, M68K_IRQ_1);
}


/**************************************************************************************************************

AT&T 630 MTG.
Chips: 2x SCN2681A, AT&T 492F proprietory, blank chip, MC68000P10, MB113F316 (square), MB113F316 (DIL), PAL16R4ACN
Crystals: 40MHz, 87.18336, 3.6864? (hard to read)

***************************************************************************************************************/

ROM_START( att630 )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD16_BYTE( "460621-1.bin", 0x00000, 0x10000, CRC(136749cd) SHA1(15378c292ddc7384cc69a35de55b69257a9f2a1c) )
	ROM_LOAD16_BYTE( "460620-1.bin", 0x00001, 0x10000, CRC(27ab77f0) SHA1(5ff1d9ee5a69dee308d62c447ee67e1888afab0e) )
	ROM_LOAD16_BYTE( "460623-1.bin", 0x20000, 0x10000, CRC(aeae12fb) SHA1(fa3ce26e4622875aa1dea7cf1bd1df237010ff2b) )
	ROM_LOAD16_BYTE( "460622-1.bin", 0x20001, 0x10000, CRC(c108c1e0) SHA1(ef01349e890b8a4117c01e78d1c23fbd113ba58f) )
ROM_END

COMP( 1987, att630, 0, 0, att630, att630, att630_state, empty_init, "AT&T", "630 MTG", MACHINE_IS_SKELETON )
