// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for AT&T 630 MTG terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "screen.h"


namespace {

class att630_state : public driver_device
{
public:
	att630_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vram(*this, "vram")
		, m_bram_data(*this, "bram", 0x2000, ENDIANNESS_BIG)
	{ }

	void att630(machine_config &config);
	void att730x(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 bram_r(offs_t offset);
	void bram_w(offs_t offset, u8 data);

	void att630_map(address_map &map) ATTR_COLD;
	void att730_map(address_map &map) ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	required_shared_ptr<u16> m_vram;
	memory_share_creator<u8> m_bram_data;
};

void att630_state::machine_start()
{
}

u32 att630_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 1024; y++)
		for (int x = 0; x < 1024; x++)
			bitmap.pix(y, x) = BIT(m_vram[y * (1024 / 16) + x / 16], 15 - (x % 16)) ? rgb_t::white() : rgb_t::black();

	return 0;
}

u8 att630_state::bram_r(offs_t offset)
{
	return m_bram_data[offset];
}

void att630_state::bram_w(offs_t offset, u8 data)
{
	m_bram_data[offset] = data;
}

void att630_state::att630_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x040000, 0x0fffff).noprw(); // additional space for rom
	map(0x100000, 0x1fffff).noprw(); // cartridge space
	map(0x200000, 0x20001f).rw("duart1", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x200020, 0x20003f).rw("duart2", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x400000, 0x6fffff).noprw(); // expansion i/o card
//  map(0x700000, 0x75ffff).noprw(); // video controller
	map(0x760000, 0x77ffff).ram().share("vram");
	map(0x780000, 0x7fffff).ram(); // program ram
	map(0xe00000, 0xe03fff).mirror(0x1fc000).rw(FUNC(att630_state::bram_r), FUNC(att630_state::bram_w)).umask16(0x00ff);
}

void att630_state::att730_map(address_map &map)
{
	att630_map(map);
	map(0x040000, 0x05ffff).rom().region("maincpu", 0x40000);
	map(0x100000, 0x15ffff).rom().region("cart", 0);
	map(0x800000, 0x87ffff).ram(); // expansion RAM
	map(0xde0000, 0xdfffff).rom().region("starlan", 0);
}

static INPUT_PORTS_START( att630 )
INPUT_PORTS_END

void att630_state::att630(machine_config &config)
{
	M68000(config, m_maincpu, 40_MHz_XTAL / 4); // clock not confirmed
	m_maincpu->set_addrmap(AS_PROGRAM, &att630_state::att630_map);
	m_maincpu->set_cpu_space(AS_PROGRAM); // vectors are in BRAM

	NVRAM(config, "bram", nvram_device::DEFAULT_ALL_0); // 5264 + battery

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(87.18336_MHz_XTAL, 1376, 0, 1024, 1056, 0, 1024);
	screen.set_color(rgb_t::amber());
	screen.set_screen_update(FUNC(att630_state::screen_update));

	scn2681_device &duart1(SCN2681(config, "duart1", 3.6864_MHz_XTAL));
	duart1.irq_cb().set_inputline(m_maincpu, M68K_IRQ_3);

	scn2681_device &duart2(SCN2681(config, "duart2", 3.6864_MHz_XTAL));
	duart2.irq_cb().set_inputline(m_maincpu, M68K_IRQ_2);
}

void att630_state::att730x(machine_config &config)
{
	att630(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &att630_state::att730_map);
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

ROM_START( att730x )
	ROM_REGION(0x60000, "maincpu", 0) // D27512-200V10 EPROMs "© AT&T 1990"
	ROM_LOAD16_BYTE( "106219637_mln3.bin", 0x00000, 0x10000, CRC(8bb2f5f9) SHA1(e7a50b8da4dde5479c8184f89f59daf511989377) )
	ROM_LOAD16_BYTE( "106219645_mln2.bin", 0x00001, 0x10000, CRC(5663da5b) SHA1(a572381a4ca73c3ddac2eff461603c8b46949687) )
	ROM_LOAD16_BYTE( "106219652_mlq3.bin", 0x20000, 0x10000, CRC(e0f88160) SHA1(2def2bb1ad11eb976bfbab25ffa536fd14eb75ca) )
	ROM_LOAD16_BYTE( "106219660_mlq2.bin", 0x20001, 0x10000, CRC(66cdd13c) SHA1(902f653ef88c4f4d93701f0b578a45618f6caa63) )
	ROM_LOAD16_BYTE( "106219678_mls3.bin", 0x40000, 0x10000, CRC(675b8d6f) SHA1(748f42021ff9655dbc4670ac412444171e7c0e95) )
	ROM_LOAD16_BYTE( "106219686_mls2.bin", 0x40001, 0x10000, CRC(87155cfd) SHA1(2e7a2a654193979330a4cadbf4a8f00617a73c05) )

	// 730X CARTRIDGE PC Code 33503 © 1989 AT&T ISSUE 1 MADE IN USA
	ROM_REGION16_BE(0x60000, "cart", 0) // D27512-200V05 EPROMs "© AT&T-1989"
	ROM_LOAD16_BYTE( "846481877.b1", 0x00000, 0x10000, CRC(5797e737) SHA1(0406add8373ab1b8c00511fd038b6548fa431d0a) )
	ROM_LOAD16_BYTE( "846481828.a1", 0x00001, 0x10000, CRC(d58e9ad6) SHA1(787a5011778be470b01036d2053945bb330da948) )
	ROM_LOAD16_BYTE( "846481844.b2", 0x20000, 0x10000, CRC(4a426790) SHA1(33209daa61255fe9b7f010e337f8de1e9446ee2b) )
	ROM_LOAD16_BYTE( "846481851.a2", 0x20001, 0x10000, CRC(57e0d7bd) SHA1(7b664c26ed9e9496d229a9907c9340e298934aba) )
	ROM_LOAD16_BYTE( "846481885.b3", 0x40000, 0x10000, CRC(eb4b0c4c) SHA1(0a520485be72992a45d5113ac053db7234adf609) )
	ROM_LOAD16_BYTE( "846481836.a3", 0x40001, 0x10000, CRC(8becf605) SHA1(8b5820d8b931a235e0c0a375973c461e14d55fed) )

	ROM_REGION16_BE(0x20000, "starlan", 0) // D27512-200V05 EPROMs "© AT&T 1990"
	ROM_LOAD16_BYTE( "846541910_abf1_001.d10", 0x00000, 0x10000, CRC(3fce193e) SHA1(e5f9704e9d06f97700ccf6f88fc935efbcd0cd4e) )
	ROM_LOAD16_BYTE( "846541902_6d82_000.b10", 0x00001, 0x10000, CRC(dcf9165c) SHA1(3f264193228bbf935a2699e64bbff08ceb1c2164) )
ROM_END

} // anonymous namespace


COMP( 1986, att630, 0, 0, att630, att630, att630_state, empty_init, "AT&T", "630 MTG", MACHINE_IS_SKELETON )
COMP( 1990, att730x, 0, 0, att730x, att630, att630_state, empty_init, "AT&T", "730X", MACHINE_IS_SKELETON )
