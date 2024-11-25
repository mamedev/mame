// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Qume QVT-103 video display terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "video/crt9007.h"
#include "emupal.h"
#include "screen.h"


namespace {

class qvt103_state : public driver_device
{
public:
	qvt103_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_vram(*this, "vram")
	{ }

	void qvt103(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_vram;
};

u32 qvt103_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int col = 0; col < 25; col++)
	{
		for (int row = 0; row < 80; row++)
		{
			uint8_t code = m_vram[col * 80 + row] & 0x7f;

			for (int y = 0; y < 12; y++)
			{
				uint16_t gfx = m_p_chargen[code << 4 | y];

				for (int x = 0; x < 8; x++)
					bitmap.pix(col*12 + y, row*8 + (7 - x)) = BIT(gfx, x) ? rgb_t::white() : rgb_t::black();
			}
		}
	}

	return 0;
}

void qvt103_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x6001).rw("kbdmcu", FUNC(i8741a_device::upi41_master_r), FUNC(i8741a_device::upi41_master_w));
//  map(0x6000, 0x6001).lr8("test", [this]() -> u8 { return machine().rand(); }); // uncomment to pass kbd test
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xa000, 0xa03f).rw("vpac", FUNC(crt9007_device::read), FUNC(crt9007_device::write));
	map(0xc000, 0xdfff).ram(); // not entirely contiguous?
	map(0xe000, 0xffff).ram().share("vram");
}

void qvt103_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x14, 0x17).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x18, 0x1b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

static INPUT_PORTS_START( qvt103 )
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8,12,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 0, 1, 2, 3, 4, 5, 6 }, // drawing chars look better with 0, 1, 2, 3, 4, 5, 6, 7
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END

static const z80_daisy_config daisy_chain[] =
{
	{ "dart" },
	{ "ctc" },
	{ nullptr }
};

void qvt103_state::qvt103(machine_config &config)
{
	Z80(config, m_maincpu, 29.376_MHz_XTAL / 9); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt103_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &qvt103_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5516APL + battery

	z80ctc_device &ctc(Z80CTC(config, "ctc", 29.376_MHz_XTAL / 9));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device &dart(Z80DART(config, "dart", 29.376_MHz_XTAL / 9));
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(29.376_MHz_XTAL * 2 / 3, 102 * 10, 0, 80 * 10, 320, 0, 300);
	//screen.set_raw(29.376_MHz_XTAL, 170 * 9, 0, 132 * 9, 320, 0, 300);
	screen.set_screen_update(FUNC(qvt103_state::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", "palette", chars);

	crt9007_device &vpac(CRT9007(config, "vpac", 29.376_MHz_XTAL / 15));
	vpac.set_character_width(10);
	vpac.int_callback().set("ctc", FUNC(z80ctc_device::trg3));

	I8741A(config, "kbdmcu", 6_MHz_XTAL);
}

/**************************************************************************************************************

Qume QVT-103.
Chips: Z80A, Z80A DART, Z80A CTC, 2x CRT9212, 5x HM6116P-2, TC5516APL, D8741AD, CRT9007, 1x 10-sw dip, Button battery.
Crystals: (all hard to read) 29.376, 6.000
Keyboard CPU, Crystal, ROM are on the main board.

***************************************************************************************************************/

ROM_START( qvt103 )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD( "t103e1.u28", 0x0000, 0x2000, CRC(eace3cbe) SHA1(1e7f395c5233d8656df5305163d050275f0a8033) )
	ROM_LOAD( "t103e2.u27", 0x2000, 0x4000, CRC(100cf542) SHA1(4b2569d509790a0f94b4447fb9d3d42582fcaf66) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "c103b.u40",  0x0000, 0x1000, CRC(3419760d) SHA1(3455c70ed48c7f7769d73a84f152beddf508094f) )

	ROM_REGION(0x0400, "kbdmcu", 0)
	ROM_LOAD( "k304a.u24",  0x0000, 0x0400, CRC(e4b1f0da) SHA1(e9f8c48c34105464b3db206b34f67e7603484fea) )
ROM_END

} // anonymous namespace


COMP( 1983, qvt103, 0, 0, qvt103, qvt103, qvt103_state, empty_init, "Qume", "QVT-103", MACHINE_IS_SKELETON )
