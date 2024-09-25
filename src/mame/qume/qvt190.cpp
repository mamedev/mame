// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************

    Qume QVT-190

    Hardware:
    - MC68B00P
    - 2x MC68B50P
    - MC68B45P
    - V61C16P55L
    - M5M5165P-70L
    - ABHGA101006
    - button battery, 7-DIL-jumper

    Crystal: unreadable (but likely to be 16.6698)

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

class qvt190_state : public driver_device
{
public:
	qvt190_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
		, m_videoram(*this, "videoram")
	{ }

	void qvt190(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);

	void qvt190_mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_videoram;
};

MC6845_UPDATE_ROW(qvt190_state::update_row)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	for (int x = 0; x < x_count; x++)
	{
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_videoram[mem];
		uint16_t gfx = m_p_chargen[(chr << 4) | ra];

		if (x == cursor_x)
			gfx = ~gfx;

		for (int i = 0; i < 9; i++)
			bitmap.pix(y, x*9 + (8-i)) = palette[BIT(gfx, i) ? 2 : 0];
	}
}

void qvt190_state::qvt190_mem_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x2500, 0x2501).rw("acia1", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x2600, 0x2601).rw("acia2", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x2800, 0x2800).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x2801, 0x2801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x4000, 0x47ff).ram().share("videoram");
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( qvt190 )
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8,12,
	RGN_FRAC(1,1), // 512
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static const gfx_layout drawing_char_layout =
{
	8,12,
	16,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{
		0x000*8+12*8, 0x000*8+13*8, 0x000*8+14*8, 0x000*8+15*8,
		0x200*8+12*8, 0x200*8+13*8, 0x200*8+14*8, 0x200*8+15*8,
		0x400*8+12*8, 0x400*8+13*8, 0x400*8+14*8, 0x400*8+15*8
	},
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
	GFXDECODE_ENTRY("chargen", 0, drawing_char_layout, 0, 1)
GFXDECODE_END

void qvt190_state::qvt190(machine_config &config)
{
	M6800(config, m_maincpu, 16.6698_MHz_XTAL / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt190_state::qvt190_mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // V61C16P55L + battery

	ACIA6850(config, "acia1", 0);

	ACIA6850(config, "acia2", 0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16.6698_MHz_XTAL, 882, 18, 738, 315, 0, 300);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	mc6845_device &crtc(MC6845(config, "crtc", 16.6698_MHz_XTAL / 9));
	crtc.set_screen("screen");
	crtc.set_char_width(9);
	crtc.set_update_row_callback(FUNC(qvt190_state::update_row));
}

ROM_START( qvt190 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "95987-267.u19", 0x0000, 0x8000, CRC(78894d8e) SHA1(0a0f6883dd18872bddeb3ed18ebe496080e6591b) )

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD( "95864-304.u17", 0x0000, 0x2000, CRC(2792e99b) SHA1(4a84d029d0e63975fc95dc7056d2523193dff986) )
ROM_END

} // anonymous namespace


COMP( 1987, qvt190, 0, 0, qvt190, qvt190, qvt190_state, empty_init, "Qume", "QVT-190", MACHINE_IS_SKELETON )
