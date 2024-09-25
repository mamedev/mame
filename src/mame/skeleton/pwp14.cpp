// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
// thanks-to:FozzTexx
/***************************************************************************

    Smith Corona PWP System 14

    08/08/2021 Skeleton driver.

    https://twitter.com/FozzTexx/status/1424043157264617478

    XTAL :  7.938 MHz
           14.364 MHz
            6.000 MHz

    FDC - WD2793A
    2 * AM9128 (2048x8 SRAM)
    NEC D4364 (8192x8 SRAM)
    2 * MT4067-12 (64Kx4 DRAM)
    CRT5037 - SMC 8621

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "video/tms9927.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"

#include "emupal.h"
#include "screen.h"


namespace {

class pwp14_state : public driver_device
{
public:
	pwp14_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_chargen(*this, "chargen")
		, m_fdc(*this, "fdc")
		, m_bank_reg(0)
	{ }

	void pwp14(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	uint32_t screen_update_pwp14(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pwp14_io(address_map &map) ATTR_COLD;
	void pwp14_mem(address_map &map) ATTR_COLD;
	void bankswitch_w(offs_t offset, uint8_t data);
	void set_bank();
	u8 typewriter_r(offs_t offset);
	void ram_memory_w(offs_t offset, uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_bank m_bank4;
	required_device<ram_device> m_ram;
	required_region_ptr<u8> m_rom;
	required_device<crt5037_device> m_crtc;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_chargen;
	required_device<wd2793_device> m_fdc;
	uint8_t m_bank_reg;
};


void pwp14_state::pwp14_mem(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x7fff).bankrw("bank2");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void pwp14_state::set_bank()
{
	uint8_t *ram = m_ram->pointer();
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_write_bank(0x0000, 0x3fff, m_bank1);
	space.install_write_bank(0x4000, 0x7fff, m_bank2);
	space.install_write_bank(0x8000, 0xbfff, m_bank3);
	space.install_write_bank(0xc000, 0xffff, m_bank4);
	if (m_bank_reg & 1) {
		m_bank1->set_base(ram + 0x0000);
	} else {
		space.install_write_handler(0x0000, 0x3fff, write8sm_delegate(*this, FUNC(pwp14_state::ram_memory_w)));
		m_bank1->set_base(m_rom + 0x0000);
	}
	if (m_bank_reg & 2) {
		m_bank2->set_base(ram + 0x4000);
	} else {
		space.install_write_handler(0x4000, 0x7fff, write8sm_delegate(*this, FUNC(pwp14_state::ram_memory_w)));
		m_bank2->set_base(m_rom + 0x4000);
	}
	if (m_bank_reg & 3) {
		m_bank3->set_base(ram + 0x8000);
	} else {
		space.unmap_write(0x8000, 0xbfff);
		m_bank3->set_base(m_rom + 0x8000);
	}
	if (m_bank_reg & 4) {
		m_bank4->set_base(ram + 0xc000);
	} else {
		space.unmap_write(0xc000, 0xffff);
		m_bank4->set_base(m_rom + 0xc000);
	}
}

void pwp14_state::ram_memory_w(offs_t offset, uint8_t data)
{
	uint8_t *ram = m_ram->pointer();
	ram[offset] = data;
}

void pwp14_state::bankswitch_w(offs_t offset, uint8_t data)
{
	m_bank_reg = data;
	logerror("bankswitch_w %02x at %04X\n", data, m_maincpu->pc());
	set_bank();
}

u8 pwp14_state::typewriter_r(offs_t offset)
{
	return 0x00;//0xff;
}

void pwp14_state::pwp14_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x0f).rw("crtc", FUNC(crt5037_device::read), FUNC(crt5037_device::write));
	map(0x20, 0x23).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));  // FDC ???
	//map(0x40, 0x40) // ???
	map(0x60, 0x60).rw(FUNC(pwp14_state::typewriter_r), FUNC(pwp14_state::bankswitch_w));
}

/* Input ports */
static INPUT_PORTS_START( pwp14 )
INPUT_PORTS_END


void pwp14_state::machine_reset()
{
	m_bank_reg = 0;
	set_bank();
}

void pwp14_state::machine_start()
{
}

uint32_t pwp14_state::screen_update_pwp14(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rgb_t const *const pen = m_palette->palette()->entry_list_raw();
	uint8_t *vram = m_ram->pointer() + 0x10000;
	rectangle cursor;
	m_crtc->cursor_bounds(cursor);

	for (int y = 0; y < 24; y++)
	{
		for (int ra = 0; ra < 8; ra++)
		{
			for (int x = 0; x < 80; x++)
			{
				uint8_t chr = vram[(y * 80) + x  + 0];
				uint8_t data = m_chargen[(chr << 3) | ra];

				//if (cursor.contains(x * 8, y * 8))
				//  data ^= 0xff;

				// draw 8 pixels of the char
				for (int i = 0; i < 8; i++)
					bitmap.pix(y * 8 + ra, x * 8 + i) = pen[BIT(data, 7 - i)];
			}
		}
	}

	return 0;
}

/* F4 Character Displayer */
static const gfx_layout pwp14_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	1024,                   /* 1024 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 2 x 8 bytes */
};

static GFXDECODE_START( gfx_pwp14 )
	GFXDECODE_ENTRY( "chargen", 0x0000, pwp14_charlayout, 0, 1 )
GFXDECODE_END

void pwp14_state::pwp14(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(6'000'000)); // Z8400BPS
	m_maincpu->set_addrmap(AS_PROGRAM, &pwp14_state::pwp14_mem);
	m_maincpu->set_addrmap(AS_IO, &pwp14_state::pwp14_io);


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(pwp14_state::screen_update_pwp14));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_pwp14);

	PALETTE(config, "palette", palette_device::MONOCHROME);
	/* internal ram */
	RAM(config, m_ram).set_default_size("76K").set_default_value(0x00); // 64K DRAM + 2*2K SRAM (VRAM) + 8K SRAM

	CRT5037(config, m_crtc, 14'364'000 / 8); // unknown clock
	m_crtc->set_char_width(8);
	m_crtc->set_screen("screen");

	WD2793(config, m_fdc, XTAL(2'000'000)); // Need proper xtal used
}

/* ROM definition */
ROM_START( pwp14 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "750524.bin", 0x0000, 0x8000, CRC(775b0e1a) SHA1(d546ef2bd4e09b2f182ffe0e393895ebad613fcc))
	ROM_LOAD( "750525.bin", 0x8000, 0x8000, CRC(673e20b5) SHA1(66e3a98cbc5969e6c64a25e39fb0df7f741e6345))

	ROM_REGION(0x2000, "chargen",0)
	ROM_LOAD( "750504.bin", 0x0000, 0x2000, CRC(b9062df6) SHA1(e79044765093b1d1954254d4a839a9e443d624d6))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY         FULLNAME  FLAGS
COMP( 1986, pwp14, 0,      0,      pwp14,   pwp14, pwp14_state, empty_init, "Smith Corona", "PWP System 14",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
