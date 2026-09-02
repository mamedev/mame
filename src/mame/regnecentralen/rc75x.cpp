// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Regnecentralen RC75x - shared implementation for the RC759 Piccoline
    and the RC750 Partner. See rc75x.h for the hardware overview.

***************************************************************************/

#include "emu.h"
#include "rc75x.h"

#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  VIDEO EMULATION (Intel 82730)
//**************************************************************************

I82730_UPDATE_ROW( rc75x_state::txt_update_row )
{
	// The RC759 runs the 82730 in one of two configurations, selected by the
	// mode block the CPU loads (there is no dedicated graphics bit -- the CPU
	// swaps the whole mode block): text is 80 char/row with 10-scanline cells
	// (lpr=9), graphics is 35 char/row with 16-scanline cells (lpr=15). Both
	// span the same 560 px active field, so the cell pitch is 560/x_count:
	// 7 px in text, 16 px in graphics. Myresnak's "switch to graphics" loads
	// the 16-scanline mode block and then draws a 560x256 bitmap into the
	// programmable character generator (m_vram), tiling the screen with unique
	// cell codes = (col<<4)|row. Detect that layout by the taller cell and
	// hand it to gfx_update_row.
	if (m_txt->rows_per_char() >= 12)
	{
		gfx_update_row(bitmap, data, lc, y, x_count, cursor);
		return;
	}

	// Text mode - original implementation
	for (int i = 0; i < x_count; i++)
	{
		bool cursor_here = (cursor == i);
		uint16_t gfx = m_vram[(data[i] & 0x3ff) << 4 | lc];

		// pretty crude detection if char sizes have been initialized, need
		// something better -- but never skip the cell under the cursor, so the
		// cursor stays visible even on a blank/space position.
		if ((gfx & 0xff) == 0 && !cursor_here)
			continue;

		// figure out char width. The font word packs 7 pixel columns in the
		// high bits (bit 15 = leftmost) and a unary width guard in the low
		// byte; a normal cell measures 7 (560 px / 80 cols).
		int width;
		for (width = 0; width < 16; width++)
			if (BIT(gfx, width) == 0)
				break;

		width = 15 - width;

		// clamp to the nominal 7-px cell. A cursor sitting on an otherwise
		// empty cell (gfx == 0) has no guard bits, which would otherwise
		// mis-measure as 15 and draw a double-wide reverse-video block.
		if (width <= 0 || width > 7)
			width = 7;

		for (int p = 0; p < width; p++)
		{
			bool on = BIT(gfx, 15 - p);
			if (cursor_here)
				on = !on; // reverse-video the cell under the cursor
			// Fixed 7-px cell pitch for the column origin (i * 7), so a cell
			// whose measured width differs never shifts the columns after it.
			// The RC759/RC750 shipped with an amber (P3-phosphor) monitor, not
			// a white one -- lit pixels amber, unlit a very dark amber (the
			// glass is never truly black on a P3 tube), not pure black.
			bitmap.pix(y, i * 7 + p) = on ? rgb_t(0xff, 0xb0, 0x00) : rgb_t(0x1a, 0x12, 0x00);
		}
	}
}


I82730_UPDATE_ROW( rc75x_state::gfx_update_row )
{
	// Graphics mode: 35 columns across the 560 px active field -> 16 px per
	// cell, i.e. the full 16-bit character-generator word is pixel data (bit
	// 15 = leftmost). Unlike text there is no proportional-width guard in the
	// low byte, so render all 16 bits and never skip a cell on a zero low
	// byte. The 82730 feeds one cell code per column; the CPU has arranged the
	// codes so that (code<<4 | lc) walks the 560x256 bitmap it drew into
	// m_vram.
	int cell_w = 560 / x_count; // 16 for the 35-column graphics layout
	for (int i = 0; i < x_count; i++)
	{
		uint16_t gfx = m_vram[(data[i] & 0x3ff) << 4 | lc];
		bool cursor_here = (cursor == i);
		for (int p = 0; p < cell_w; p++)
		{
			bool on = BIT(gfx, 15 - p);
			if (cursor_here)
				on = !on;
			int x_pos = i * cell_w + p;
			if (x_pos < bitmap.width())
				bitmap.pix(y, x_pos) = on ? rgb_t(0xff, 0xb0, 0x00) : rgb_t(0x1a, 0x12, 0x00);
		}
	}
}


void rc75x_state::txt_ca_w(uint16_t data)
{
	m_txt->ca_w(1);
	m_txt->ca_w(0);
}

void rc75x_state::txt_irst_w(uint16_t data)
{
	m_txt->irst_w(1);
	m_txt->irst_w(0);
}

uint8_t rc75x_state::palette_r(offs_t offset)
{
	// not sure if it's possible to read back
	logerror("palette_r(%02x)\n", offset);
	return 0xff;
}

void rc75x_state::palette_w(offs_t offset, uint8_t data)
{
	logerror("palette_w(%02x): %02x\n", offset, data);

	// two colors/byte. format: IRGBIRGB
	static constexpr uint8_t val[4] = { 0x00, 0x55, 0xaa, 0xff };
	int r, g, b;

	r = (BIT(data, 2) << 1) | BIT(data, 3);
	g = (BIT(data, 1) << 1) | BIT(data, 3);
	b = (BIT(data, 0) << 1) | BIT(data, 3);

	m_palette->set_pen_color(offset * 2 + 0, rgb_t(val[r], val[g], val[b]));

	r = (BIT(data, 6) << 1) | BIT(data, 7);
	g = (BIT(data, 5) << 1) | BIT(data, 7);
	b = (BIT(data, 4) << 1) | BIT(data, 7);

	m_palette->set_pen_color(offset * 2 + 1, rgb_t(val[r], val[g], val[b]));
}


//**************************************************************************
//  NVM (bank-switched 256x4 CMOS)
//**************************************************************************

// 256x4 nvram is bank-switched using ppi port c, bit 4 and 5.
// The RC75x PROM/XIOS reads its configuration (memory size, disk/data/dir
// buffer counts, autostart command, checksum in byte 0) from this NVRAM.
// A factory-blank NVRAM lacks that config, which makes the CCP/M-86 XIOS
// derail during memory setup. Seed a known-good configuration (identical to
// the working PCE rc759 nvm.dat) so the machine boots to A> out of the box.
// Nibble order here matches PCE / real hardware (even index -> high nibble).
void rc75x_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	// byte 0x00 is the checksum: sum(bytes[0..95]) & 0xff must equal 0xAA
	// (Piccoline Programmer's Guide 3.2). byte 0x19 is DEFAULT LOAD encoded as
	// the load-medium ASCII letter: 'M'=PROM, 'N'=NET, 'A'=DRIVE A, 'B'=DRIVE B.
	// PCE's nvm.dat ships 'M' (PROM), which makes the bootloader stop at the
	// "SELECT LOADMEDIUM" prompt. Seed 'A' (DRIVE A) instead so the machine
	// boots straight to A> from the floppy. byte 0x13 is the cursor config:
	// 4 MSB = height in video lines (0x0a = full-cell block), 4 LSB = blink
	// mode (1 = blinking). We ship 0xa1 for a blinking block cursor (rather
	// than the thin 0x21 two-line underline). byte 0 is recomputed so the
	// 0xAA checksum invariant still holds (0xdf - 0x80 == 0x5f).
	static const uint8_t defaults[128] =
	{
		0x5f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x90, 0x04, 0x00, 0x00, 0xc0, 0x80, 0xa1, 0x05, 0x00, 0x07, 0x07,
		0x01, 0x41, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	memset(data, 0x00, size);
	memcpy(data, defaults, std::min(size, sizeof(defaults)));
}

uint8_t rc75x_state::nvram_r(offs_t offset)
{
	offs_t addr = (m_nvram_bank << 6) | offset;

	logerror("nvram_r(%02x)\n", addr);

	// nibble order matches PCE / real RC759: even index -> high nibble, odd -> low
	if (addr & 1)
		return (m_nvram_mem[addr >> 1] & 0x0f) >> 0;
	else
		return (m_nvram_mem[addr >> 1] & 0xf0) >> 4;
}

void rc75x_state::nvram_w(offs_t offset, uint8_t data)
{
	offs_t addr = (m_nvram_bank << 6) | offset;

	logerror("nvram_w(%02x): %02x\n", addr, data);

	// nibble order matches PCE / real RC759: even index -> high nibble, odd -> low
	if (addr & 1)
		m_nvram_mem[addr >> 1] = (m_nvram_mem[addr >> 1] & 0xf0) | (data & 0x0f);
	else
		m_nvram_mem[addr >> 1] = ((data << 4) & 0xf0) | (m_nvram_mem[addr >> 1] & 0x0f);
}


//**************************************************************************
//  RTC (MM58167)
//**************************************************************************

void rc75x_state::rtc_data_w(uint8_t data)
{
	m_rtc_write_data = data;
}

uint8_t rc75x_state::rtc_data_r()
{
	return m_rtc_read_data;
}

void rc75x_state::rtc_addr_w(uint8_t data)
{
	if (BIT(data, 7))
		m_rtc_read_addr = data & 0x1f;
	else
		m_rtc_write_addr = data & 0x1f;

	if (BIT(data, 6) && BIT(m_rtc_strobe, 6) == 0)
		m_rtc->write(m_rtc_write_addr, m_rtc_write_data);

	if (BIT(data, 5) && BIT(m_rtc_strobe, 5) == 0)
		m_rtc_read_data = m_rtc->read(m_rtc_read_addr);

	m_rtc_strobe = data;
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void rc75x_state::i186_timer1_w(int state)
{
	m_speaker->level_w(state);
}

uint8_t rc75x_state::irq_callback()
{
	return m_pic->acknowledge();
}

void rc75x_state::machine_start()
{
	m_nvram_mem.resize(256 / 2);
	m_nvram->set_base(&m_nvram_mem[0], 256 / 2);
}


//**************************************************************************
//  SHARED MACHINE CONFIGURATION
//**************************************************************************

void rc75x_state::add_common_devices(machine_config &config)
{
	m_maincpu->read_slave_ack_callback().set(FUNC(rc75x_state::irq_callback));
	m_maincpu->tmrout1_handler().set(FUNC(rc75x_state::i186_timer1_w));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	NVRAM(config, m_nvram).set_custom_handler(FUNC(rc75x_state::nvram_init));

	MM58167(config, m_rtc, 32.768_kHz_XTAL).irq().set(m_pic, FUNC(pic8259_device::ir3_w));

	// video. The RC759 Piccoline shipped with the standard (~62.5 Hz, ~19.5 kHz)
	// monochrome screen -- the higher-quality 73 Hz / 22 kHz screen was the
	// RC750 Partner's, only later back-ported to the Piccoline. The firmware's
	// mode block programs 47*16 = 752 total dots per line and 312 lines, so the
	// 82730 character clock must be 62.5 * 47 * 312 = 916'500 Hz for the field
	// rate to be 62.5 Hz (a 16 ms frame period -- the firmware's system tick,
	// driven here via screen_vblank -> TMRIN0, assumes exactly 16 ms). The
	// set_raw values below are only the pre-modeset placeholder; the 82730
	// overrides them via screen().configure() at mode-set.
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(916'500 * 16, 752, 112, 672, 312, 31, 291);
	screen.set_screen_update(m_txt, FUNC(i82730_device::screen_update));
	screen.screen_vblank().set(m_maincpu, FUNC(i80186_cpu_device::tmrin0_w)); // TMRIN0 source not documented, but self-test needs something like this

	I82730(config, m_txt, 916'500, m_maincpu);
	m_txt->set_screen("screen");
	m_txt->set_update_row_callback(FUNC(rc75x_state::txt_update_row));
	m_txt->sint().set(m_pic, FUNC(pic8259_device::ir4_w));

	PALETTE(config, m_palette).set_entries(64);

	// sound
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76489A(config, m_snd, 20_MHz_XTAL / 10).add_route(ALL_OUTPUTS, "mono", 1.0);

	// keyboard
	RC759_KBD_HLE(config, m_kbd);
	m_kbd->int_handler().set(m_pic, FUNC(pic8259_device::ir1_w));
}
