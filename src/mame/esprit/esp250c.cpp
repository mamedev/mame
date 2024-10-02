// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Esprit Systems 250C

    16 color serial terminal

    Hardware:
    - Intel N80C188XL25
    - AMIC A290021 256k flash memory
    - 2x W24257AJ 32k SRAM
    - TERMTEK TKA-200 A410A0011 ASIC
    - 32 MHz XTAL, 50 MHz XTAL, XTAL labeled "7.3F5G"
    - Buzzer

    External:
    - PC/PS2 keyboard connector
    - 25-pin RS232 host port
    - 9-pin serial printer
    - Parallel printer
    - VGA monitor (48.4 kHz horizontal, 70 Hz vertical)

    TODO:
    - Almost everything (shows an initial screen)

    Notes:
    - Other models in this line: 350C (with Ethernet)
    - PCB labeled "TKP-635 VER:1.5"
    - 10x16 cell 800x416 in 80 column mode
    - 9x16 cell 1188x416 in 132 column mode

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"

#include "emupal.h"
#include "multibyte.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class esp250c_state : public driver_device
{
public:
	esp250c_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vram(*this, "vram"),
		m_chargen(*this, "chargen")
	{ }

	void esp250c(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i80188_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_chargen;

	bool m_nmi_enable = false;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void vblank_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void unk_f0_w(uint8_t data);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void esp250c_state::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram();
	map(0x10000, 0x1ffff).ram().share("vram");
	map(0x20000, 0x23fff).ram().share("chargen");
	map(0xc0000, 0xfffff).rom().region("maincpu", 0);
}

void esp250c_state::io_map(address_map &map)
{
	map(0x00, 0x00).lr8(NAME([this] () { logerror("read 0x00\n"); return 0x10; }));
	map(0x11, 0x11).lr8(NAME([this] () { logerror("read 0x11\n"); return 0x09; }));
	map(0x19, 0x19).lr8(NAME([this] () { logerror("read 0x19\n"); return 0x09; }));
	map(0xf0, 0xf0).w(FUNC(esp250c_state::unk_f0_w));
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( esp250c )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void esp250c_state::vblank_w(int state)
{
	// nmi needs a periodic source, might be vblank
	if (m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

uint32_t esp250c_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_gfxdecode->gfx(0)->mark_all_dirty();

	const pen_t *const pen = m_palette->pens();

	for (unsigned y = 0; y < 26; y++)
	{
		uint32_t addr = get_u24le(&m_vram[y * 3]);

		for (unsigned x = 0; x < 80; x++)
		{
			uint8_t code = m_vram[addr++];

			for (int i = 0; i < 16; i++)
			{
				uint16_t data = m_chargen[(code << 5) + i];

				// fixed for now
				rgb_t fg = pen[7];
				rgb_t bg = pen[0];

				for (int p = 0; p < 10; p++)
					bitmap.pix(y * 16 + i, x * 10 + p) = BIT(data, 9 - p) ? fg : bg;
			}
		}
	}

	return 0;
}

static const gfx_layout char_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8 * 16
};

static GFXDECODE_START( chars )
	GFXDECODE_RAM("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void esp250c_state::unk_f0_w(uint8_t data)
{
	logerror("io 0xf0 = %02x\n", data);

	// maybe
	m_nmi_enable = bool(BIT(data, 3));
}

void esp250c_state::machine_start()
{
	// register for save states
	save_item(NAME(m_nmi_enable));
}

void esp250c_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void esp250c_state::esp250c(machine_config &config)
{
	I80188(config, m_maincpu, 50_MHz_XTAL); // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &esp250c_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &esp250c_state::io_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(50_MHz_XTAL, 1034, 0, 800, 690, 0, 416); // wrong
	m_screen->set_screen_update(FUNC(esp250c_state::screen_update));
	m_screen->screen_vblank().set(FUNC(esp250c_state::vblank_w));

	PALETTE(config, m_palette).set_entries(16);

	GFXDECODE(config, m_gfxdecode, m_palette, chars);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( esp250c )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("tk-635_v035.bin", 0x00000, 0x40000, CRC(8e907958) SHA1(e0d5d39656e460933aa4e7d9d0c1e16bef414f6d))
	// character data from 0x20000 to 0x2ffff
	// version at 0x30000 "V0.35 13MAY"
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY           FULLNAME  FLAGS
COMP( 2005, esp250c, 0,      0,      esp250c, esp250c, esp250c_state, empty_init, "Esprit Systems", "250C",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
