// license: BSD-3-Clause
// copyright-holders: AJR, Dirk Best
/****************************************************************************

    Falco 500 series terminals

    Third generation of Falco terminals, after the TS and FAME series. Many
    functions are now handled by a custom ASIC that was updated a few times
    between models.

    List of models (incomplete):
    - 1986: 500, 542, 5220, 5500
    - 1987: 500e, 5220e, 5000, 5600
    - 1988: 5600s, 5220s, 580, 5330

    This drivers contains dumps for the 500e, 5220e and 5220s models. The
    5220e is emulated to an advanced stage and usable, the others are
    skeletons for now.

    Additional notes:
    - The keyboard interface is completely wrong. It should be serially
      connected to the ASIC (and support different models)
    - The ASIC emulation still needs lots of work. Lots of guesses and
      many unknown and unhandled writes.

    Detailed hardware descriptions for the dumped models:

    Falco 5220e:
    - Z0840006PSC Z80 CPU
    - Z0843006PSC Z80 CTC
    - Z0844006PSC Z80 SIO/0
    - L1A3417  041500-001  FALCO  TAE8739Δ
    - PAL labeled F-500E
    - D43256C-10L labeled ARAM1, socket label AROM0 empty
    - D43256C-10L labeled CRAM1, socket label CROM0 empty
    - CXK5864AP-10L (next to ASIC)
    - TMM27256AD-20*2 152321-000 (ROM0) 152321-001 (ROM1)
    - CXK5864AP-10L (next to ROMs)
    - 37.980 MHz XTAL
    - 12.288 MHz XTAL
    - Bell
    - Space for an IC labeled "CLOCK", not fitted

    Falco 500e
    - PCB labeled "PWB MINOTAUR V016 P/N 130165-016 FALCO DATA PRODUCTS COPYRIGHT 4-93"
    - Z0840006PSC Z80 CPU
    - Z0843006PSC Z80 CTC
    - Z0844006PSC Z80 SIO/0
    - L1A3641  041501-001  NAP 9314Δ (SG)  WA39038  HONG KONG
    - PALs labeled 150133 and 150424 (near ASIC/CPU)
    - PAL labeled 150423 (near CPU/CTC/SIO)
    - HY62256ALP-70 and empty socket
    - HY62256ALP-70 and empty socket
    - HY62256ALP-70 (next to ASIC)
    - 27C512-15*2 155710-000 and 155710-001
    - HY62256ALP-10
    - 54.046000 MHz XTAL
    - 12.288 MHz XTAL
    - Bell

    Falco 5220s:
    - Z0840006PSC Z80 CPU
    - Z0843006PSC Z80 CTC
    - Z0844006PSC Z80 SIO/0
    - L1A3641  041501-001  FALCO  TAG 8914 Δ  8276 HONG KONG
    - PAL unlabeled
    - HY6264P-10 labeled ARAM1, socket label AROM0 empty
    - HY6264LP labeled CRAM1, socket label CROM0 empty
    - HY6264P-10 (next to ASIC)
    - 27256*2 168412-000 (ROM0) 168412-001 (ROM1)
    - HY6264LP-12
    - 37.980 MHz XTAL
    - 12.288 MHz XTAL
    - Bell

****************************************************************************/

#include "emu.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "f5220_kbd.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class falco500_state : public driver_device
{
public:
	falco500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rambank(*this, "rambank"),
		m_ctc(*this, "ctc"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_rombank(*this, "rombank"),
		m_ram(*this, "ram"),
		m_charram(*this, "charram"),
		m_kbd(*this, "kbd"),
		m_asic_5a(0)
	{ }

	void falco500(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<address_map_bank_device> m_rambank;
	required_device<z80ctc_device> m_ctc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_memory_bank m_rombank;
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_charram;
	required_device<f5220_kbd_device> m_kbd;

	void mem_map(address_map &map) ATTR_COLD;
	void bank_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t asic_data_r();
	uint8_t asic_status_r();
	void asic_rambank_w(uint8_t data);
	void asic_settings_w(uint8_t data);
	void asic_cursor_addr_w(offs_t offset, uint8_t data);
	void asic_56_w(uint8_t data);
	void asic_5a_w(uint8_t data);
	void asic_mode_w(uint8_t data);
	void asic_cmd_w(uint8_t data);

	void kbd_int_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void rombank_w(uint8_t data);
	void unk_40_w(uint8_t data);

	uint8_t m_unk_40;
	uint8_t m_asic_status;
	uint8_t m_asic_settings;
	uint16_t m_asic_cursor_addr;
	uint8_t m_asic_56;
	uint8_t m_asic_5a;
	uint8_t m_asic_mode;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void falco500_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0xbfff).bankr("rombank");
	map(0xc000, 0xdfff).ram().share("nvram");
	map(0xe000, 0xffff).m(m_rambank, FUNC(address_map_bank_device::amap8));
}

void falco500_state::bank_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share("ram"); // 2x NEC D43256C-10L? (5220s only has 2x HY6264P-10)
	map(0x10000, 0x11fff).ram().share("charram"); // CXK5864AP-10L / HY6264LP-12?
}

void falco500_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(falco500_state::rombank_w));
	map(0x40, 0x40).w(FUNC(falco500_state::unk_40_w));
	map(0x50, 0x50).r(FUNC(falco500_state::asic_data_r));
	map(0x51, 0x51).r(FUNC(falco500_state::asic_status_r));
	map(0x52, 0x52).w(FUNC(falco500_state::asic_rambank_w));
	map(0x53, 0x53).w(FUNC(falco500_state::asic_settings_w));
	map(0x54, 0x55).w(FUNC(falco500_state::asic_cursor_addr_w));
	map(0x56, 0x56).w(FUNC(falco500_state::asic_56_w));
	map(0x5a, 0x5a).w(FUNC(falco500_state::asic_5a_w)); // L1A3641 only?
	map(0x5b, 0x5b).w(FUNC(falco500_state::asic_cmd_w));
	map(0x5d, 0x5d).w(FUNC(falco500_state::asic_mode_w));
	map(0x60, 0x63).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x70, 0x73).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x88, 0x8b).nopw(); // second SIO?
	map(0x90, 0x93).nopw(); // second CTC?
}


//**************************************************************************
//  ASIC
//**************************************************************************

void falco500_state::asic_rambank_w(u8 data)
{
	m_rambank->set_bank(data & 0x0f);

	if (data > 8)
		logerror("rambank_w: %02x\n", data);
}

uint8_t falco500_state::asic_data_r()
{
	return m_kbd->read();
}

uint8_t falco500_state::asic_status_r()
{
	// 765-----  unknown (not used?)
	// ---4----  keyboard data available
	// ----3---  unknown
	// -----2--  unknown (needs to toggle)
	// ------1-  unknown
	// -------0  unknown

	m_asic_status &= ~0x04;
	m_asic_status |= m_screen->vblank() ? 0x04 : 0x00; // maybe?

	return m_asic_status;
}

void falco500_state::asic_settings_w(u8 data)
{
	logerror("asic_settings_w: unk %d curblink %d curshape %d curen %d undrline %d\n",
		BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), data & 0x0f);

	// 7-------  unknown
	// -6------  cursor blink enabled
	// --5-----  cursor shape (underline, block)
	// ---4----  cursor enabled
	// ----3210  underline location

	m_asic_settings = data;
}

void falco500_state::asic_cursor_addr_w(offs_t offset, uint8_t data)
{
	if (offset)
	{
		m_asic_cursor_addr &= 0x00ff;
		m_asic_cursor_addr |= ((data & 0xf0) << 8) << 1;
		m_asic_cursor_addr |= (data & 0x0f) << 8;
	}
	else
	{
		m_asic_cursor_addr = (m_asic_cursor_addr & 0xff00) | (data << 0);
	}
}

void falco500_state::asic_56_w(uint8_t data)
{
	if (data != m_asic_56)
		logerror("asic_56_w: %02x\n", data);

	m_asic_56 = data;
}

void falco500_state::asic_5a_w(u8 data)
{
	logerror("asic_5a_w: %02x\n", data);
	m_asic_5a = data;
}

void falco500_state::asic_cmd_w(uint8_t data)
{
	//logerror("asic_cmd_w: %02x\n", data);

	// 0x7f -> reset asic?

	// 7-------  unknown
	// -6------  reset keyboard data available?
	// --543210  unknown

	if (BIT(data, 6))
		m_asic_status &= ~0x10;
}

void falco500_state::asic_mode_w(uint8_t data)
{
	if (data != m_asic_mode)
		logerror("asic_mode_w: %02x\n", data);

	m_asic_mode = data;

	// 7654321-  unknown
	// -------0  80/132 columns

	// timing wrong
	rectangle visarea(0, (BIT(data, 0) ? 1320 : 1120) - 1, 0, 400 - 1);
	m_screen->configure(1500, 422, visarea, HZ_TO_ATTOSECONDS(60));
}

void falco500_state::kbd_int_w(int state)
{
	if (state)
		m_asic_status |= 0x10;
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t falco500_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *const pen = m_palette->pens();

	bitmap.fill(pen[0], cliprect);

	int y = 0;

	for (int line = 0; line < 50; line++)
	{
		// line attributes for each line
		uint8_t la0 = m_ram[0x0000 + line * 2 + 0];
		uint8_t la1 = m_ram[0x0000 + line * 2 + 1];
		uint8_t la2 = m_ram[0x1000 + line * 2 + 0];
		uint8_t la3 = m_ram[0x1000 + line * 2 + 1];

		// la0 layout
		// 76543210  line address offset low bytes

		// la1 layout
		// 7654----  line height
		// ----3210  offset for double-height lines

		// la2 layout
		// 7-------  line address highest bit (only for graphics mode?)
		// -654----  line address bank (multiply by 2 to get the address)
		// ----3210  line address offset high bytes

		// la3 layout
		// 7-------  unknown
		// -6------  set on the line between windows
		// --5-----  80 columns/132 columns
		// ---4----  graphics/alphanumeric
		// ----3---  unknown
		// -----2--  double width
		// ------1-  double height first half
		// -------0  double height second half (but not working for line modes 26 and 44?)

		uint16_t line_addr = (la2 << 8) | la0;

		// move address highest bit to its place, shift bank left
		line_addr = bitswap<16>(line_addr, 14, 13, 12, 15, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

		//uint8_t unk_code = m_ram[0x0000 + line_addr]; // ?
		//uint8_t unk_attr = m_ram[0x1000 + line_addr]; // ?

		// skip unknown value
		line_addr++;

		uint8_t line_height = la1 >> 4;
		line_height++;

		// safety check to prevent writing out of bounds
		if (y + line_height > (cliprect.max_y + 1))
			return 0;

		if (BIT(la3, 4))
		{
			// horizontal resolution 560 pixels
			for (int col = 0; col < 560; col++)
			{
				uint16_t char_addr = line_addr + col;
				uint8_t code = m_ram[0x0000 + char_addr];

				// foreground/background colors
				rgb_t fg = pen[1];
				rgb_t bg = pen[0];

				for (int x = 0; x < 8; x++)
				{
					if (BIT(la3, 2))
					{
						bitmap.pix(y + (col / 70), (((col % 70) * 8) * 2) + (x * 2) + 0) = BIT(code, x) ? fg : bg;
						bitmap.pix(y + (col / 70), (((col % 70) * 8) * 2) + (x * 2) + 1) = BIT(code, x) ? fg : bg;
					}
					else
					{
						bitmap.pix(y + (col / 70), ((col % 70) * 8) + x) = BIT(code, x) ? fg : bg;
					}
				}
			}
		}
		else
		{
			// figure out number of columns
			int cols = BIT(la3, 5) ? 80 : 132;

			if (BIT(la3, 2))
				cols /= 2;

			for (int col = 0; col < cols; col++)
			{
				uint16_t char_addr = line_addr + col;

				uint8_t code = m_ram[0x0000 + char_addr];
				uint8_t attr = m_ram[0x1000 + char_addr];

				// code layout
				// 7-------  unknown
				// -6543210  chargen address

				// attr layout
				// 7-------  unknown
				// -6------  chargen address high bit
				// --5-----  unknown
				// ---4----  normal/bold
				// ----3---  unknown
				// -----2--  reverse
				// ------1-  blink
				// -------0  underline

				for (int y_char = 0; y_char < line_height; y_char++)
				{
					bool blink = bool(m_screen->frame_number() & 0x10); // timing?
					bool underline = bool(y_char == (m_asic_settings & 0x0f));

					int y_chargen = y_char;

					// double height enabled?
					if (la3 & 0x03)
						y_chargen /= 2;

					// double-height, first half
					if (BIT(la3, 1))
						y_chargen += 0;

					// double height, second half
					if (BIT(la3, 0))
						y_chargen += ((la1 & 0x0f) + 1);

					uint16_t gfx = m_charram[(BIT(attr, 6) << 12) | ((code & 0x7f) << 4) | y_chargen];

					if (BIT(attr, 0) && underline)
						gfx = 0x3ff;

					if (BIT(attr, 1) && blink)
						gfx = 0x000;

					if (BIT(attr, 2))
						gfx ^= 0x3ff;

					// cursor
					if (BIT(m_asic_settings, 4) && (m_asic_cursor_addr == char_addr))
					{
						if ((BIT(m_asic_settings, 5) == 1 && BIT(m_asic_settings, 6) == 0) || // block
							(BIT(m_asic_settings, 5) == 1 && BIT(m_asic_settings, 6) == 1 && blink) || // block-blink
							(BIT(m_asic_settings, 5) == 0 && underline && BIT(m_asic_settings, 6) == 0) || // underline
							(BIT(m_asic_settings, 5) == 0 && underline && BIT(m_asic_settings, 6) == 1 && blink)) // underline-blink
							gfx ^= 0x3ff; // might be solid instead
					}

					// foreground/background colors
					rgb_t fg = BIT(attr, 4) ? pen[1] : pen[2];
					rgb_t bg = pen[0];

					for (int x = 0; x < 10; x++)
					{
						if (BIT(la3, 2))
						{
							// double-width
							bitmap.pix(y + y_char, col * 2 * 10 + x * 2 + 0) = BIT(gfx, x) ? fg : bg;
							bitmap.pix(y + y_char, col * 2 * 10 + x * 2 + 1) = BIT(gfx, x) ? fg : bg;
						}
						else
						{
							bitmap.pix(y + y_char, col * 10 + x) = BIT(gfx, x) ? fg : bg;
						}
					}
				}
			}
		}

		y += line_height;
	}

	return 0;
}

static const gfx_layout char_layout =
{
	8, 16,
	512,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ STEP16(0, 8) },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_RAM("charram", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void falco500_state::rombank_w(u8 data)
{
	m_rombank->set_entry(data & 3);

	if (data > 3)
		logerror("rombank_w: %02x\n", data);
}

void falco500_state::unk_40_w(uint8_t data)
{
	if (data != m_unk_40)
		logerror("unk_40_w: %02x\n", data);

	m_unk_40 = data;
}

void falco500_state::machine_start()
{
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x4000);

	// register for save states
	save_item(NAME(m_unk_40));
	save_item(NAME(m_asic_status));
	save_item(NAME(m_asic_settings));
	save_item(NAME(m_asic_cursor_addr));
	save_item(NAME(m_asic_56));
	save_item(NAME(m_asic_5a));
	save_item(NAME(m_asic_mode));
}

void falco500_state::machine_reset()
{
	m_rombank->set_entry(0);
	m_rambank->set_bank(0);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ nullptr }
};

void falco500_state::falco500(machine_config &config)
{
	Z80(config, m_maincpu, 12.288_MHz_XTAL / 2); // Z0840006PSC
	m_maincpu->set_addrmap(AS_PROGRAM, &falco500_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falco500_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	ADDRESS_MAP_BANK(config, m_rambank);
	m_rambank->set_map(&falco500_state::bank_map);
	m_rambank->set_data_width(8);
	m_rambank->set_addr_width(17);
	m_rambank->set_stride(0x2000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CXK5864AP-10L + battery

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::amber());
	m_screen->set_raw(37.98_MHz_XTAL, 1500, 0, 800, 422, 0, 400); // 25.32 kHz/60 Hz confirmed
	m_screen->set_screen_update(FUNC(falco500_state::screen_update));
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI); // maybe?

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	Z80CTC(config, m_ctc, 12.288_MHz_XTAL / 2); // Z0843006PSC
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(12.288_MHz_XTAL / 10);
	m_ctc->zc_callback<0>().set("sio", FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<0>().append("sio", FUNC(z80sio_device::txca_w));
	m_ctc->set_clk<1>(12.288_MHz_XTAL / 10);
	m_ctc->zc_callback<1>().set("sio", FUNC(z80sio_device::rxtxcb_w));
	m_ctc->set_clk<2>(12.288_MHz_XTAL / 10); // ?
	m_ctc->zc_callback<2>().set("bell", FUNC(speaker_sound_device::level_w));
	m_ctc->set_clk<3>(12.288_MHz_XTAL / 10); // ?

	z80sio_device &sio(Z80SIO(config, "sio", 12.288_MHz_XTAL / 2)); // Z0844006PSC
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("porta", FUNC(rs232_port_device::write_txd));
	sio.out_rtsa_callback().set("porta", FUNC(rs232_port_device::write_rts));
	sio.out_txdb_callback().set("portb", FUNC(rs232_port_device::write_txd));
	sio.out_rtsb_callback().set("portb", FUNC(rs232_port_device::write_rts));

	rs232_port_device &porta(RS232_PORT(config, "porta", default_rs232_devices, nullptr));
	porta.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	porta.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	porta.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);

	rs232_port_device &portb(RS232_PORT(config, "portb", default_rs232_devices, nullptr));
	portb.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	portb.cts_handler().set("sio", FUNC(z80sio_device::ctsb_w));
	portb.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);

	SPEAKER(config, "mono").front_center();

	SPEAKER_SOUND(config, "bell").add_route(ALL_OUTPUTS, "mono", 1.00);

	F5220_KBD(config, m_kbd);
	m_kbd->int_handler().set(FUNC(falco500_state::kbd_int_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( falco5220e )
	ROM_REGION(0x10000, "maincpu", 0) // (c) 1987 FDP, Inc 2321
	ROM_LOAD("152321-000.bin", 0x0000, 0x8000, BAD_DUMP CRC(45ef4a68) SHA1(71e12dce710f9b66290618e299b2382834845057)) // wrong checksum, might be a bit flip
	ROM_LOAD("152321-001.bin", 0x8000, 0x8000, CRC(91056626) SHA1(217ca3de76d5e9861284f5b64f8eff8e541fad3d))
ROM_END

ROM_START( falco500e )
	ROM_REGION(0x20000, "maincpu", 0) //  (c) 1991 FDP, Inc 1710
	ROM_LOAD("155710-000.bin", 0x00000, 0x10000, CRC(cc7c53b8) SHA1(214e7f7b4ce2f0b4106292e4d73125df2c4ddc92))
	ROM_LOAD("155710-001.bin", 0x10000, 0x10000, CRC(d1b51d71) SHA1(fd5aee5da1422d4f19045a98663df4f09472ca6b))
ROM_END

ROM_START( falco5220s )
	ROM_REGION(0x10000, "maincpu", 0) // (c) 1989 FDP, Inc 0412
	ROM_LOAD("168412-00.bin", 0x0000, 0x8000, CRC(de34b149) SHA1(6a4824eb5941f4c6475949011e64b28ab185ba59))
	ROM_LOAD("168412-01.bin", 0x8000, 0x8000, CRC(e6facd5b) SHA1(2b9bf3ca18e3e30032dcb6faf0809b6cf6f467ac))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME        PARENT      COMPAT  MACHINE   INPUT  CLASS            INIT        COMPANY                FULLNAME       FLAGS
COMP( 1987, falco5220e, 0,          0,      falco500, 0,     falco500_state, empty_init, "Falco Data Products", "Falco 5220e", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1991, falco500e,  falco5220e, 0,      falco500, 0,     falco500_state, empty_init, "Falco Data Products", "Falco 500e",  MACHINE_IS_SKELETON )
COMP( 1989, falco5220s, falco5220e, 0,      falco500, 0,     falco500_state, empty_init, "Falco Data Products", "Falco 5220s", MACHINE_IS_SKELETON )
