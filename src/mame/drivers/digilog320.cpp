// license: BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Digilog 320

    Protocol analyzer

    Status: Skeleton

    Hardware:

    PCB 24-0710-02B:
    - Speaker
    - XTAL 1.344 MHz
    - R80186-10
    - XTAL 20 MHz
    - DS1241
    - HM62256LP-15 x4
    - AT27C512R x6

    PCB 24-0706-02C:
    - MB8877A
    - FDC9229BT
    - XTAL 16 MHz
    - P8251A
    - XTAL 3.6864 MHz
    - SCN2681AC1N40
    - HD46505SP-2
    - HM6264ALP-12 x2
    - AM27128DC "24-1140-00 A"
    - XTAL 5.659200 MHz
    - PAL labeled "20-1110-1 A"

    PCB 24-0709-02D:
    - XTAL 16 MHz
    - Z0840008PSC Z80 CPU
    - HM62256LP-12
    - Z8530H-8PC x2
    - AT27C256 "24-0196-01 C"
    - AM9519A-1PC
    - XTAL 2.688 MHz
    - XTAL 3.6864 MHz

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/z80/z80.h"
#include "machine/mc68681.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class digilog320_state : public driver_device
{
public:
	digilog320_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram"),
		m_chargen(*this, "chargen")
	{ }

	void digilog320(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<z80_device> m_subcpu;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_vram;
	required_region_ptr<uint8_t> m_chargen;

	void main_mem_map(address_map &map);
	void main_io_map(address_map &map);
	void sub_mem_map(address_map &map);
	void sub_io_map(address_map &map);

	MC6845_UPDATE_ROW(update_row);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void digilog320_state::main_mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0x80000, 0x83fff).ram().share("vram");
	map(0xa0000, 0xfffff).rom().region("maincpu", 0);
}

void digilog320_state::main_io_map(address_map &map)
{
	map(0x080, 0x09f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0x180, 0x180).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x182, 0x182).w(m_crtc, FUNC(mc6845_device::register_w));
}

void digilog320_state::sub_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("subcpu", 0);
	map(0x8000, 0xffff).ram();
}

void digilog320_state::sub_io_map(address_map &map)
{
	map.global_mask(0xff);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

MC6845_UPDATE_ROW( digilog320_state::update_row )
{
	pen_t const *const pen = m_palette->pens();

	for (int x = 0; x < x_count; x++)
	{
		uint16_t const data = (m_vram[ma + x]);
		uint8_t gfx = m_chargen[((data & 0xff) << 4) | ra];

		// draw 8 pixels of the character
		for (int i = 0; i < 8; i++)
			bitmap.pix(y, x * 8 + i) = pen[BIT(gfx, i)];
	}
}

static const gfx_layout char_layout =
{
	8, 12,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void digilog320_state::machine_start()
{
}

void digilog320_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void digilog320_state::digilog320(machine_config &config)
{
	I80186(config, m_maincpu, 20_MHz_XTAL); // R80186-10
	m_maincpu->set_addrmap(AS_PROGRAM, &digilog320_state::main_mem_map);
	m_maincpu->set_addrmap(AS_IO, &digilog320_state::main_io_map);

	Z80(config, m_subcpu, 16_MHz_XTAL / 2); // Z0840008PSC
	m_subcpu->set_addrmap(AS_PROGRAM, &digilog320_state::sub_mem_map);
	m_subcpu->set_addrmap(AS_IO, &digilog320_state::sub_io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(5.6592_MHz_XTAL, 320, 0, 256, 262, 0, 192);
	screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	MC6845(config, m_crtc, 5.6592_MHz_XTAL / 8); // HD46505SP-2
	m_crtc->set_char_width(8);
	m_crtc->set_show_border_area(false);
	m_crtc->set_update_row_callback(FUNC(digilog320_state::update_row));

	SCN2681(config, "duart", 3.6864_MHz_XTAL);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( digilog320 )
	ROM_REGION(0x60000, "maincpu", 0)
	ROM_LOAD16_BYTE("24-0197-01_e.u33", 0x00000, 0x10000, CRC(d1be8c1f) SHA1(5cef9d54e341da4cfa6ca50939c889c6d502240a))
	ROM_LOAD16_BYTE("24-0197-02_e.u24", 0x00001, 0x10000, CRC(1bbe293d) SHA1(69d22456e6b20fc3ce1c5208af169310e414e955))
	ROM_LOAD16_BYTE("24-0197-03_e.u34", 0x20000, 0x10000, CRC(d492b6fd) SHA1(474220128e63133eeb8591401edb3ad65d71e027))
	ROM_LOAD16_BYTE("24-0197-04_e.u25", 0x20001, 0x10000, CRC(a5667746) SHA1(ebecd69ebf394365e9bedfcbe98d71ce4ec7feb1))
	ROM_LOAD16_BYTE("24-0197-05_e.u35", 0x40000, 0x10000, CRC(17273a19) SHA1(0c28b304e1447d4afd821b3b1a4fb6029bf2a24d))
	ROM_LOAD16_BYTE("24-0197-06_e.u26", 0x40001, 0x10000, CRC(cb29ca28) SHA1(8827de70c006250ba2989c4dc57e402ed222f948))

	ROM_REGION(0x8000, "subcpu", 0)
	ROM_LOAD("24-0196-01_c.u12", 0x0000, 0x8000, CRC(20291d21) SHA1(1151414531040af59f4f692aab31bfc04e7fc56c))

	ROM_REGION(0x4000, "chargen", 0)
	ROM_LOAD("24-1140-00_a.bin", 0x0000, 0x4000, CRC(7a4d0b82) SHA1(15952655cef77918a76c0c268b749be34b28634b))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT  CLASS             INIT        COMPANY    FULLNAME  FLAGS
COMP( 1988, digilog320, 0,          0,      digilog320, 0,     digilog320_state, empty_init, "Digilog", "320",    MACHINE_IS_SKELETON )
