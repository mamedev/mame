// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Falco 5220 terminal.

    Three variations of this terminal were introduced in 1986. Falco 5220
    primarily emulates the DEC VT220; Falco 542 primarily emulates the LSI
    ADM 42; and Falco 5500 primarily emulates the Wyse WY-50.

    All three of these terminals have the same video characteristics, with
    9x12 characters rendered in a 10x16 cell, displayed in up to 2 screen
    windows on 24 or 44 lines (not counting the status line). They also
    likely share the gate array which is thus labeled on the Falco 5220
    PCB:

        LIA3417
        041500-001
        FALCO
        TAE8379Δ

    The Falco 500 is also supposed to be part of the series, though it was
    introduced earlier and has definitely different video capabilities.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "emupal.h"
#include "screen.h"

class falco5220_state : public driver_device
{
public:
	falco5220_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rambank(*this, "rambank")
		, m_screen(*this, "screen")
		, m_rombank(*this, "rombank")
		, m_ram(*this, "ram")
		, m_charram(*this, "charram")
		, m_5a(0)
	{
	}

	void falco5220(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void rombank_w(u8 data);
	void rambank_w(u8 data);
	void unk5a_w(u8 data);

	void mem_map(address_map &map);
	void bank_map(address_map &map);
	void io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<address_map_bank_device> m_rambank;
	required_device<screen_device> m_screen;
	required_memory_bank m_rombank;
	required_shared_ptr<u8> m_ram;
	required_shared_ptr<u8> m_charram;

	u8 m_5a;
};

u32 falco5220_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t start = m_5a ? 0x0000 : 0x2000;

	for (int col = 0; col < 25; col++)
	{
		for (int row = 0; row < 80; row++)
		{
			u8 code = m_ram[start + col * 80 + row];
			u8 attr = m_ram[start + 0x1000 + col * 80 + row];

			for (int y = 0; y < 16; y++)
			{
				u8 gfx = m_charram[code << 4 | y];

				// underline
				if (BIT(attr, 0) && y == 15)
					gfx = 0xff;

				for (int x = 0; x < 8; x++)
					bitmap.pix(col*16 + y, row*8 + x) = BIT(gfx, x) ? rgb_t::white() : rgb_t::black();
			}
		}
	}

	return 0;
}

void falco5220_state::rambank_w(u8 data)
{
	if (data < 9)
		m_rambank->set_bank(data);
	else
		logerror("rambank_w: %02x\n", data);
}

void falco5220_state::rombank_w(u8 data)
{
	m_rombank->set_entry(data & 3);
}

void falco5220_state::unk5a_w(u8 data)
{
	logerror("5a = %02x\n", data);
	m_5a = data;
}

void falco5220_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("roms", 0);
	map(0x8000, 0xbfff).bankr("rombank");
	map(0xc000, 0xdfff).ram().share("nvram");
	map(0xe000, 0xffff).m(m_rambank, FUNC(address_map_bank_device::amap8));
}

void falco5220_state::bank_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share("ram"); // 2x NEC D43256C-10L? (5220s only has 2x HY6264P-10)
	map(0x10000, 0x11fff).ram().share("charram"); // CXK5864AP-10L / HY6264LP-12?
}

void falco5220_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(falco5220_state::rombank_w));
//  map(0x50, 0x50).r // keyboard data
//  map(0x51, 0x51).r // keyboard status
	map(0x52, 0x52).w(FUNC(falco5220_state::rambank_w));
//  map(0x54, 0x54).w // vram address (low)?
//  map(0x55, 0x55).w // vram address (high)?
	map(0x5a, 0x5a).w(FUNC(falco5220_state::unk5a_w)); // 5220s only?
	map(0x60, 0x63).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x70, 0x73).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x88, 0x8b).nopw(); // second SIO?
	map(0x90, 0x93).nopw(); // second CTC?
}

void falco5220_state::machine_start()
{
	m_rombank->configure_entries(0, 4, memregion("roms")->base(), 0x4000);
}

void falco5220_state::machine_reset()
{
	m_rombank->set_entry(0);
	m_rambank->set_bank(0);
}

static INPUT_PORTS_START(falco5220)
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8,16,
	512,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ STEP16(0, 8) },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_RAM("charram", 0, char_layout, 0, 1)
GFXDECODE_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ nullptr }
};

void falco5220_state::falco5220(machine_config &config)
{
	Z80(config, m_maincpu, 12.288_MHz_XTAL / 2); // Z0840006PSC
	m_maincpu->set_addrmap(AS_PROGRAM, &falco5220_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falco5220_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	ADDRESS_MAP_BANK(config, m_rambank);
	m_rambank->set_map(&falco5220_state::bank_map);
	m_rambank->set_data_width(8);
	m_rambank->set_addr_width(17);
	m_rambank->set_stride(0x2000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CXK5864AP-10L + battery

	z80ctc_device &ctc(Z80CTC(config, "ctc", 12.288_MHz_XTAL / 2)); // Z0843006PSC
	ctc.set_clk<0>(12.288_MHz_XTAL / 10);
	ctc.set_clk<1>(12.288_MHz_XTAL / 10);
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80sio_device &sio(Z80SIO(config, "sio", 12.288_MHz_XTAL / 2)); // Z0844006PSC
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::amber());
	screen.set_raw(37.98_MHz_XTAL, 1500, 0, 1320, 422, 0, 400); // 25.32 kHz/60 Hz confirmed
	screen.set_screen_update(FUNC(falco5220_state::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", "palette", chars);
}

ROM_START(falco5220e)
	ROM_REGION(0x10000, "roms", 0) // (c) 1987 FDP, Inc 2321
	ROM_LOAD("152321-000.bin", 0x0000, 0x8000, CRC(45ef4a68) SHA1(71e12dce710f9b66290618e299b2382834845057))
	ROM_LOAD("152321-001.bin", 0x8000, 0x8000, CRC(91056626) SHA1(217ca3de76d5e9861284f5b64f8eff8e541fad3d))
ROM_END

ROM_START(falco5220s)
	ROM_REGION(0x10000, "roms", 0) // (c) 1989 FDP, Inc 0412
	ROM_LOAD("168412-00.bin", 0x0000, 0x8000, CRC(de34b149) SHA1(6a4824eb5941f4c6475949011e64b28ab185ba59))
	ROM_LOAD("168412-01.bin", 0x8000, 0x8000, CRC(e6facd5b) SHA1(2b9bf3ca18e3e30032dcb6faf0809b6cf6f467ac))
ROM_END

COMP(1987, falco5220e, 0,          0, falco5220, falco5220, falco5220_state, empty_init, "Falco Data Products", "Falco 5220e", MACHINE_IS_SKELETON)
COMP(1989, falco5220s, falco5220e, 0, falco5220, falco5220, falco5220_state, empty_init, "Falco Data Products", "Falco 5220s", MACHINE_IS_SKELETON)
