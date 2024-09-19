// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Insight Enterprises Z80 Single Board Computer

    Possibly a prototype or part of a larger system. A video with more
    info is available at https://www.youtube.com/watch?v=_z1kBb-Zwpg.

    TODO:
    - Verify device hookup
    - Figure out what's at 0x20, 0x21, 0x3c, 0x3d
    - Hook up FDC
    - Hook up CRT8002 (video attributes)
    - Much more

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "video/tms9927.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class iez80_state : public driver_device
{
public:
	iez80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_chargen(*this, "chargen"),
		m_vram(*this, "vram")
	{ }

	void iez80(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<crt5037_device> m_crtc;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_chargen;
	required_shared_ptr<uint8_t> m_vram;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void iez80_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("maincpu", 0);
	map(0x2000, 0x2fff).ram().share("vram");
	map(0xe000, 0xffff).ram();
}

void iez80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
//  map(0x20, 0x20).lr8([this]() { return machine().rand(); }, "unk20");
//  map(0x21, 0x21).lr8([this]() { return machine().rand(); }, "unk21");
	map(0x24, 0x27).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x28, 0x2b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x2c, 0x2f).rw("dart1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x32, 0x32).rw("dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x38, 0x3b).rw("dart2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
//  map(0x3c, 0x3c).lr8([this]() { return machine().rand(); }, "unk3c");
//  map(0x3d, 0x3d).lr8([this]() { return machine().rand(); }, "unk3d");
	map(0x40, 0x4f).rw("crtc", FUNC(crt5037_device::read), FUNC(crt5037_device::write));
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( iez80 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t iez80_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t const *const pen = m_palette->palette()->entry_list_raw();

	rectangle cursor;
	m_crtc->cursor_bounds(cursor);

	for (int y = 0; y < 24; y++)
	{
		for (int ra = 0; ra < 12; ra++)
		{
			for (int x = 0; x < 80; x++)
			{
				uint8_t chr = m_vram[(y * 160) + (x * 2) + 0];
				// uint8_t attr = m_vram[(y * 160) + (x * 2) + 1];
				uint8_t data = m_chargen[((chr << 4) | ra) & 0x7ff];

				if (cursor.contains(x * 8, y * 12))
					data ^= 0xff;

				// draw 8 pixels of the char
				for (int i = 0; i < 8; i++)
					bitmap.pix(y * 12 + ra, x * 8 + i) = pen[BIT(data, 7 - i)];
			}
		}
	}

	return 0;
}

static const gfx_layout crt8002_charlayout =
{
	8, 12,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START( gfx_crt8002 )
	GFXDECODE_ENTRY("chargen", 0, crt8002_charlayout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void iez80_state::machine_start()
{
}

void iez80_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

void iez80_state::iez80(machine_config &config)
{
	Z80(config, m_maincpu, 2'500'000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &iez80_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &iez80_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80DMA(config, "dma", 4'000'000); // unknown clock

	z80ctc_device& ctc(Z80CTC(config, "ctc", 4'000'000)); // unknown clock
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80PIO(config, "pio", 4'000'000); // unknown clock

	Z80DART(config, "dart1", 4'000'000); // unknown clock (5.0688_MHz_XTAL near)

	Z80DART(config, "dart2", 4'000'000); // unknown clock (5.0688_MHz_XTAL near)

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(17'550'000, 816, 0, 640, 358, 0, 288); // unknown clock, hand-tuned to ~60fps
	screen.set_screen_update(FUNC(iez80_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_crt8002);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	CRT5037(config, m_crtc, 17'550'000 / 8); // unknown clock
	m_crtc->set_char_width(8);
	m_crtc->set_screen("screen");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( iez80 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("iez80.bin", 0x0000, 0x1000, CRC(d73137d2) SHA1(a340a47c5f37bbef244d35d581ff0beeeec5d677))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("8002.bin", 0x0000, 0x0800, CRC(fdd6eb13) SHA1(a094d416e66bdab916e72238112a6265a75ca690))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                FULLNAME                FLAGS
COMP( 1983, iez80, 0,      0,      iez80,   iez80, iez80_state, empty_init, "Insight Enterprises", "Z80 SBC (prototype?)", MACHINE_IS_SKELETON )
