// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Marcos Hernández
/****************************************************************

Wheels & Fire
Power Ball (prototype)
Kong Ball (prototype, TCH / Digital Dreams Multimedia)
Radical Enduro (early prototype, TCH / Sator Videogames)

Original driver by Tomasz Slanina.
The video model and the blitter rewritten from the games' use of the hardware by Marcos
Hernández, in collaboration, within MS-DOS Club's TCH preservation project
(github.com/MS-DOS-Club/tch-blitter).


The Wheels & Fire PCB:
----------------------------------
Produttore  TCH
N.revisione E133



CPU

2x MC68HC000FN16

2x TPC1020BFN-084C
1x BT478KPJ50-615-9132F
1x S9530AG-ADC0808-CCV
1x oscillator 32.000000MHz
1x ST93C46 EEPROM

ROMs

12x TMS27C040

1x GAL16V8QS
2x GAL22V10

Note

1x JAMMA connector
1x VGA connector - Used to link 2 PCBs together
2x trimmer (volume)


----

Main 68000 interrupts:
- level 1: the blitter, a level asserted while it is idle and when a blit completes, cleared
  by the arming write; the games pace their blit chain with their own interrupt mask
- level 3: vblank
- level 5: the line counter loaded at 0x7a0000, counting the 256 scanned lines (not the vblank
  ones) and only while bit 5 of the byte at 0x7c0000 is set; it clocks the sound streamer (one word per
  line to the sub CPU) and, in Wheels & Fire, the road's per-line scroll

Sub 68000: level 1 on every line, the 15625 Hz rate the sound mixer counts against.

The blitter (register map in blit_w):
- armed by writing 0x0000 to word 15, fired by writing 0xffff
- transparency is selected by writing word 12 (transparent) or word 13 (opaque), reset by
  the arming write
- word 14 makes the command a fill of the band of rows it declares, edge to edge of the
  plane; word 7 bit 2 reads the source and bit 3 writes index 0 too (both: a colour fill,
  bit 3 alone: an erase that walks its destination in columns)
- a page above the ROM opens the direct-write channel: the command defines a window, and the
  low byte of word 6 streams pixels into it
- two 512x512 planes, both wrapping: a scrolled background, with a per-line vertical scroll
  for the road, and a sprite plane of two 256-line pages, one displayed while the game draws
  into the other
- the zoom is two counters per axis: a repeat field of n reads one source pixel of every n - 1
  twice, a stall field of n drops one write of every n + 1 (0 is idle), and a clock bit halves
  or doubles the result, which spans 25% to 400%

The field is derived from the 32 MHz crystal: 512 pixels by 312 lines, a 15625 Hz line rate.

TODO:
- the zoom counters' starting phase, and what bit 5 of each field does, are not known
- a zoomed blit stopping one step short of its endpoint, and a 1:1 vertical blit skipping its
  first source row, are fitted to the games' output
- DISPLAY_LAG_LINES is fitted to Wheels & Fire; the write for screen line 251 lands after the
  frame is drawn
- Wheels & Fire's race shows two rows of ground fill above its horizon, and the width its road
  starts at is not confirmed
- radendur_state's sprite-plane flip and frame scroll are fitted
- how the board acknowledges a line-counter request is not known
- no layer enable is identified, so both planes are always composed
- word 4's low byte, with its bit 8 in word 8, is written by the games and read by nothing
- the link between two cabinets (0x7c0000) is not emulated

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>
#include <bit>
#include <utility>

#include "wheelfir.lh"

#define LOG_DIRECT (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

// The games clip their sprites in software against (0, 0, 0x150, 0xff), kept at 0x20b4b8 in
// Wheels & Fire, so they draw rows 0..255; 252 of them are shown.
constexpr int NUM_SCANLINES = 256;
constexpr int FIRST_VISIBLE_LINE = 0;
constexpr int NUM_VBLANK_LINES = 56; // the rest of the 312-line field
constexpr int VISIBLE_LINES = 252;   // of the 256 scanned, the lines shown
constexpr int VISIBLE_COLUMNS = 336; // the width the games clip to
constexpr int DISPLAY_LAG_LINES = 1; // how far the screen runs behind the line counter, fitted (see the TODO)
constexpr int PIXELS_PER_LINE = 512;
constexpr XTAL MASTER_CLOCK = 32_MHz_XTAL; // the board's only crystal
constexpr int LAYER_BG = 0;
constexpr int LAYER_FG = 1;
constexpr int NUM_COLORS = 256;
constexpr int GFX_PAGE_BYTES = 512 * 512; // one 8-bit graphics page

// the flag bits of the blitter registers, by word (register map in blit_w)
constexpr uint16_t SRC_U_BIT8    = 0x0100; // word 6: bit 8 of the source column
constexpr uint16_t SRC_V_BIT8    = 0x0200; // word 6: bit 8 of the source row
constexpr uint16_t DST_X_FORWARD = 0x0001; // word 7: the destination walks with increasing x
constexpr uint16_t DST_Y_FORWARD = 0x0002; // word 7: and with increasing y
constexpr uint16_t READS_SOURCE  = 0x0004; // word 7: the blit reads its graphic
constexpr uint16_t WRITES_ZERO   = 0x0008; // word 7: the blit also writes index 0
constexpr uint16_t DST_PLANE_BG  = 0x0010; // word 7: destination plane, set = background
constexpr uint16_t DST_X_BIT8    = 0x0040; // word 7: bit 8 of the destination x0
constexpr uint16_t DST_Y_BIT8    = 0x0080; // word 7: bit 8 of the destination y0
constexpr uint16_t VISIBLE_PAGE  = 0x0020; // word 8: the sprite page displayed
constexpr uint16_t SCROLL_X_BIT8 = 0x0040; // word 8: bit 8 of the background scroll x
constexpr uint16_t SCROLL_Y_BIT8 = 0x0080; // word 8: bit 8 of the background scroll y
constexpr uint16_t SRC_U_FORWARD = 0x4000; // word 8: the source walks with increasing u
constexpr uint16_t SRC_V_FORWARD = 0x8000; // word 8: and with increasing v
constexpr uint16_t DST_X1_BIT8   = 0x0004; // word 9: bit 8 of the destination x1
constexpr uint16_t DST_Y1_BIT8   = 0x0008; // word 9: bit 8 of the destination y1

class wheelfir_state : public driver_device
{
public:
	wheelfir_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_blitter_data(*this, "blitram"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ramdac(*this, "ramdac"),
		m_eeprom(*this, "eeprom"),
		m_adc(*this, "adc"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_tilepages(*this, "gfx1"),
		m_soundlatch(*this, "soundlatch"),
		m_adc_eoc(0)
	{ }

	void wheelfir(machine_config &config) ATTR_COLD;
	void kongball(machine_config &config) ATTR_COLD;

	int adc_eoc_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// what Radical Enduro's board does differently (see radendur_state)
	virtual void sprite_plane_blit() { }
	virtual int32_t frame_top_yscroll() const { return m_scroll_y_latched; }
	virtual int32_t background_xscroll() const { return m_scroll_x_latched; }
	virtual int32_t displayed_sprite_page() const { return m_page_at_line0; }

	// the 9-bit background scroll the registers hold, bit 8 from word 8
	int32_t scroll_x_registers() const { return (m_blitter_data[0xa] & 0x00ff) | ((m_blitter_data[0x8] & SCROLL_X_BIT8) ? 0x100 : 0); }
	int32_t scroll_y_registers() const { return (m_blitter_data[0xb] & 0x00ff) | ((m_blitter_data[0x8] & SCROLL_Y_BIT8) ? 0x100 : 0); }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_shared_ptr<uint16_t> m_blitter_data;

private:
	// The zoom of one axis (see the header): the period of each counter, 0 when it is idle.
	struct axis_scale
	{
		int stall_period;
		int repeat_period;
		bool clock_halve;
		bool clock_double;

		bool identity() const { return !stall_period && !repeat_period && !clock_halve && !clock_double; }
	};

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ramdac_device> m_ramdac;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<adc0808_device> m_adc;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	required_region_ptr<uint8_t> m_tilepages;
	required_device<generic_latch_16_device> m_soundlatch;

	// The sprite plane page the game selected on line 0 of the frame, the half displayed. Word 8 is
	// rewritten by every blit, and from line 1 on Wheels & Fire's road writer reuses it without the
	// page bit, so it is sampled here.
	uint8_t m_page_at_line0;
	// 0xffff was written to word 14 since the last arming: the next trigger is a fill, not a blit.
	bool m_word14_fill;
	bool m_blit_opaque;   // the last mode write since the arming selected opaque

	int32_t m_direct_write_x0;
	int32_t m_direct_write_x1;
	int32_t m_direct_write_y0;
	int32_t m_direct_write_y1;
	int32_t m_direct_write_idx;
	// the plane the direct-write channel streams into: the plane bit of the command that opened it
	int32_t m_direct_write_layer;

	int16_t m_scanline_cnt;
	bool m_counter_enabled;

	uint32_t m_gfx_mask;   // the gfx1 region's length - 1, a power of two

	// The 9-bit background scroll, latched when the game writes the Y byte. Its bit 8s live in
	// word 8, which every blit rewrites, so they are captured with the coordinates.
	int32_t m_scroll_x_latched;
	int32_t m_scroll_y_latched;

	int32_t m_adc_eoc;

	bitmap_ind16 m_tmp_bitmap[2];
	int32_t m_current_yscroll;

	// a repeat field of 1 would never advance the source, so it is treated as idle
	static axis_scale decode_scale(int stall, int repeat, bool clock_halve, bool clock_double)
	{
		return axis_scale{ stall ? (stall + 1) : 0, (repeat > 1) ? (repeat - 1) : 0, clock_halve, clock_double };
	}

	// The source walk of one axis: each source pixel read moves the write position on once, twice
	// when the repeat counter wraps, and a move is lost when the stall counter wraps; the clock bit
	// halves or doubles the position. Each destination step takes the first source pixel to reach it.
	static void source_map(const axis_scale &scale, int steps, int *src)
	{
		int x = 0, written = 0, repeat = 0, stall = 0;
		const auto move = [&scale, &stall, &written] ()
		{
			if (scale.stall_period && ++stall == scale.stall_period)
				stall = 0;
			else
				++written;
		};
		for (int d = 0; d <= steps; d++)
		{
			while ((scale.clock_halve ? (written >> 1) : scale.clock_double ? (written << 1) : written) < d)
			{
				++x;
				move();
				if (scale.repeat_period && ++repeat == scale.repeat_period)
				{
					repeat = 0;
					move();
				}
			}
			src[d] = x;
		}
	}

	void scanline_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void do_direct_write(uint8_t sixdat);
	void do_blit();
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void blit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void control_w(uint8_t data);
	uint8_t control_r();
	void coin_cnt_w(uint16_t data);
	void adc_eoc_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer_callback);
	void board(machine_config &config) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};

void wheelfir_state::video_start()
{
	for (int i = 0; i < 2; i++)
	{
		m_tmp_bitmap[i].allocate(512, 512);
		save_item(NAME(m_tmp_bitmap[i]), i);
	}
}

void wheelfir_state::do_blit()
{
	// The blitter IRQ is a held level: up when a blit completes, down at the arming write. The game
	// paces the chain with its own mask (#$2000 opens it, #$2100 closes it), one blit per interrupt.
	m_maincpu->set_input_line(1, ASSERT_LINE);

	uint8_t const *const rom = m_tilepages;

	const int src_u0 = (m_blitter_data[0] >> 8) + ((m_blitter_data[6] & SRC_U_BIT8) ? 256 : 0);
	const int src_v0 = (m_blitter_data[2] >> 8) + ((m_blitter_data[6] & SRC_V_BIT8) ? 256 : 0);

	int dst_x0 = (m_blitter_data[0] & 0xff) + ((m_blitter_data[7] & DST_X_BIT8) ? 256 : 0);
	int dst_y0 = (m_blitter_data[2] & 0xff) + ((m_blitter_data[7] & DST_Y_BIT8) ? 256 : 0);

	int dst_x1 = (m_blitter_data[1] & 0xff) + ((m_blitter_data[9] & DST_X1_BIT8) ? 256 : 0);
	int dst_y1 = (m_blitter_data[3] & 0xff) + ((m_blitter_data[9] & DST_Y1_BIT8) ? 256 : 0);
	// a word 14 fill clears the band of rows it declares, edge to edge of the plane; one naming no
	// ROM page draws nothing
	const int page = (m_blitter_data[6] >> 10) * GFX_PAGE_BYTES;
	const bool fill_band = m_word14_fill && page < 0x700000;
	if (m_word14_fill && !fill_band)
		return;

	// Word 7 bits 0 and 1 are the destination walk (set = increasing), word 8 bits 14 and 15 the
	// source walk: a mirror is a backward destination under a forward source.
	const bool dst_x_forward = (m_blitter_data[7] & DST_X_FORWARD);
	const bool dst_y_forward = (m_blitter_data[7] & DST_Y_FORWARD);

	// bit 3 alone is an erase: it writes zeros and walks its destination in columns
	const bool writes_zero = (m_blitter_data[7] & WRITES_ZERO) != 0;
	const bool erase = writes_zero && !(m_blitter_data[7] & READS_SOURCE);

	// transposed with the endpoints when an erase transposes them, so each step stays on its axis
	int x_dst_step = dst_x_forward ? 1 : -1;
	int y_dst_step = dst_y_forward ? 1 : -1;

	const int u_src_step = (m_blitter_data[8] & SRC_U_FORWARD) ? 1 : -1;
	const int v_src_step = (m_blitter_data[8] & SRC_V_FORWARD) ? 1 : -1;

	if (page >= 0x700000) /* src set to  unav. page before direct write to the framebuffer */
	{
		LOGMASKED(LOG_DIRECT, "%s: page %08x names no ROM: direct-write channel opened\n", machine().describe_context(), page);
		// wheelfir sets 0xfc0000 and 0xf00000, both of which are out of range of any GFX ROM
		// kongball sets 0xfc0000 but only on startup
		m_direct_write_x0 = dst_x0;
		m_direct_write_x1 = dst_x1;
		m_direct_write_y0 = dst_y0;
		m_direct_write_y1 = dst_y1;
		m_direct_write_idx = 0;
		// the channel writes to the plane selected by the blit's plane bit
		m_direct_write_layer = (m_blitter_data[0x7] & DST_PLANE_BG) ? LAYER_BG : LAYER_FG;
		return;
	}

	dst_x0 &= 0x1ff;
	dst_x1 &= 0x1ff;
	dst_y0 &= 0x1ff;
	dst_y1 &= 0x1ff;

	const int vpage = (m_blitter_data[0x7] & DST_PLANE_BG) ? LAYER_BG : LAYER_FG;
	if (vpage == LAYER_FG)
		sprite_plane_blit();

	// The height is counted on the torus: the screen clear on a transition arrives as y=511..510,
	// 512 rows going round.
	if (fill_band)
	{
		const int rows = ((dst_y1 - dst_y0) & 0x1ff) + 1;
		const int end = dst_y0 + rows - 1;
		m_tmp_bitmap[vpage].fill(0, rectangle(0, 511, dst_y0, std::min(end, 511)));
		if (end > 511)
			m_tmp_bitmap[vpage].fill(0, rectangle(0, 511, 0, end - 512));
		return;
	}

	if (erase)
	{
		using std::swap;
		swap(dst_x0, dst_y0);
		swap(dst_x1, dst_y1);
		swap(x_dst_step, y_dst_step);
	}

	// each axis's stall and repeat fields and its two clock bits (register map in blit_w)
	const axis_scale scale_u = decode_scale(
			BIT(m_blitter_data[0x0a], 8, 5),
			BIT(m_blitter_data[0x0b], 8, 5),
			BIT(m_blitter_data[0x09], 0), BIT(m_blitter_data[0x08], 12));
	const axis_scale scale_v = decode_scale(
			BIT(m_blitter_data[0x0b], 14, 2) | (BIT(m_blitter_data[0x0c], 14, 2) << 2) | (BIT(m_blitter_data[0x0a], 14) << 4),
			BIT(m_blitter_data[0x0c], 8, 5),
			BIT(m_blitter_data[0x09], 1), BIT(m_blitter_data[0x08], 13));
	const bool ident_u = scale_u.identity();
	const bool ident_v = scale_v.identity();

	// 1:1 blits draw their endpoint, zoomed ones do not (fitted, see the TODO)
	const bool inclusive = ident_u && ident_v;

	// Both planes wrap: the walk is counted with the 9-bit destination counters, which carry a walk
	// past 511 onto row or column 0, so every write lands on the plane.
	const int steps_x = ((dst_x1 - dst_x0) * x_dst_step) & 0x1ff;
	const int steps_y = ((dst_y1 - dst_y0) * y_dst_step) & 0x1ff;

	int y = dst_y0;

	// each destination step's source offset, per zoomed axis
	int src_u[512], src_v[512];
	if (!ident_u)
		source_map(scale_u, steps_x, src_u);
	if (!ident_v)
		source_map(scale_v, steps_y, src_v);

	const bool opaque = writes_zero || m_blit_opaque;
	bitmap_ind16 &plane = m_tmp_bitmap[vpage];

	int step_y = 0;
	while (true)
	{
		int x = dst_x0;
		int step_x = 0;

		// at 1:1 vertically the first source row, a border the art carries, is skipped (fitted, see
		// the TODO); at other scales, whether the board does the same is not known
		const int yy = src_v0 + v_src_step * (ident_v ? (step_y + 1) : src_v[step_y]);
		uint16_t *const row = &plane.pix(y & 0x1ff);

		while (true)
		{
			const int xx = src_u0 + u_src_step * (ident_u ? step_x : src_u[step_x]);

			// keep the source inside its own 512x512 page
			const int address = page + (yy & 0x1ff) * 512 + (xx & 0x1ff);

			const int pix = erase ? 0 : rom[address & m_gfx_mask];

			const int px = x & 0x1ff;
			if (pix || opaque)
				row[px] = pix;

			if (step_x >= steps_x + (inclusive ? 0 : -1))
				break;
			step_x++;

			x += x_dst_step;
		}

		if (step_y >= steps_y + (inclusive ? 0 : -1))
			break;
		step_y++;

		y += y_dst_step;
	}
}

void wheelfir_state::do_direct_write(uint8_t sixdat)
{
	const int direct_width = m_direct_write_x1 - m_direct_write_x0 + 1;
	const int direct_height = m_direct_write_y1 - m_direct_write_y0 + 1;

	if (direct_width > 0 && direct_height > 0)
	{
		int x = m_direct_write_idx % direct_width;
		int y = (m_direct_write_idx / direct_width) % direct_height;

		x += m_direct_write_x0;
		y += m_direct_write_y0;

		if (x < 512 && y < 512)
		{
			m_tmp_bitmap[m_direct_write_layer].pix(y, x) = sixdat;
		}
	}

	++m_direct_write_idx;
}

void wheelfir_state::blit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	Blitter data format ( offset in words, offset in bytes )

	0  0    uuuuuuuu   u = src_u0 bits 0-7
	   1    xxxxxxxx   x = dest_x0 bits 0-7
	1  2    --------
	   3    XXXXXXXX   X = dest_x1 bits 0-7
	2  4    vvvvvvvv   v = src_v0 bits 0-7
	   5    yyyyyyyy   y = dest_y0 bits 0-7
	3  6    --------
	   7    YYYYYYYY   Y = dest_y1 bits 0-7
	4  8    --------
	   9    wwwwwwww   w = a 9-bit value's low byte, bit 8 in word 8 bit 0x10; the games write it, nothing here reads it
	5  a    --------
	   b    --------
	6  c    ppppppvu   p = gfx page (six bits; 0x1c and above name no ROM and open the direct-write channel), v = src_v0 bit 8, u = src_u0 bit 8
	   d    DDDDDDDD   D = direct bitmap/framebuffer write port
	7  e    --------
	   f    yx-Ledff   y = dest_y0 bit 8, x = dest_x0 bit 8, L = dest layer (1 = background), e = writes index 0 too, d = reads the source
	                   (e+d = colour fill, e alone = erase, which walks the destination in columns), ff = Y/X direction of the destination walk
	8 10    FFZzZzZz   FF = Y/X direction of the source walk, Z = y clock doubled, z = x clock doubled, Z = y repeat field bit 5, z = x repeat field bit 5, Z = y stall field bit 5, z = x stall field bit 5 (see the TODO)
	  11    SsPW----   s = scroll x of back layer bit 8, S = scroll y of back layer bit 8, P = which page of the sprite plane is displayed, sampled on line 0, W = bit 8 of word 4's value
	9 12    --------
	  13    ----YXZz   Y = dest_y1 bit 8, X = dest_x1 bit 8, Z = y clock halved, z = x clock halved
	a 14    -Z-zzzzz   Z = y stall field bit 4, z = x stall field bits 0-4,
	  15    ssssssss   s = scroll x of back layer
	b 16    ZZ-zzzzz   Z = y stall field bits 0-1, z = x repeat field bits 0-4
	  17    SSSSSSSS   S = scroll y of back layer
	c 18    ZZ-ZZZZZ   Z = y stall field bits 2-3, Z = y repeat field bits 0-4
	  19    tttttttt   t = a write here selects the transparent mode (index 0 is not written); the games write 0xff
	d 1a    --------
	  1b    oooooooo   o = a write here selects the opaque mode (index 0 is written); the games write 0xff
	e 1c    FFFFFFFF
	  1d    FFFFFFFF   F = 0xffff marks the next trigger as a fill: it clears the declared band of rows, edge to edge, to index 0
	f 1e    TTTTTTTT
	  1f    TTTTTTTT   T = trigger: 0x0000 arms the command (resets the mode latch and the fill mark, drops the IRQ line), 0xffff fires it
	*/

	// the screen trails the line counter by DISPLAY_LAG_LINES: lines before the one this write
	// reaches are final
	m_screen->update_partial(m_screen->vpos() - 1 - DISPLAY_LAG_LINES);
	COMBINE_DATA(&m_blitter_data[offset]);

	if (offset == 0xe && m_blitter_data[0xe] == 0xffff)
		m_word14_fill = true;

	// a write reaching the low byte of word 12 selects transparent and of word 13 opaque; the
	// arming write resets it
	if (offset == 0xc && ACCESSING_BITS_0_7)
		m_blit_opaque = false;
	if (offset == 0xd && ACCESSING_BITS_0_7)
		m_blit_opaque = true;

	if (offset == 0x6)
	{
		if (!ACCESSING_BITS_8_15)  //LSB only!
		{
			do_direct_write(data & 0xff);
			return;
		}
	}

	// bit 8 of the scroll x is live in word 8: Wheels & Fire rewrites it without the coordinates
	if (offset == 0x8 && !ACCESSING_BITS_8_15)
		m_scroll_x_latched = (m_scroll_x_latched & 0x0ff) | ((m_blitter_data[0x8] & SCROLL_X_BIT8) ? 0x100 : 0);

	// the scroll is latched when the Y byte is written, which always follows the X, and the row
	// counter loaded here advances one row per line from the screen line the write reaches; one
	// written in vblank or on lines 0..DISPLAY_LAG_LINES is where the frame starts
	if (offset == 0xb && !ACCESSING_BITS_8_15)
	{
		m_scroll_y_latched = scroll_y_registers();
		m_scroll_x_latched = scroll_x_registers();
		m_current_yscroll = m_scroll_y_latched - std::max(m_screen->vpos() - DISPLAY_LAG_LINES, 0);
		return;
	}

	if (offset == 0xf)
	{
		if (m_blitter_data[0xf] == 0xffff)
		{
			do_blit();
		}
		else if (m_blitter_data[0xf] == 0x0000)
		{
			// arming resets the mode and the fill mark and acknowledges the blitter: the held
			// IRQ line drops here.
			m_blit_opaque = false;
			m_word14_fill = false;
			m_maincpu->set_input_line(1, CLEAR_LINE);
		}
		else
		{
			logerror("%s: write to offset 0xf (blit start) but with data %04x\n", machine().describe_context(), m_blitter_data[0xf]);
		}
	}
}

void wheelfir_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const int32_t xscroll = background_xscroll();

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const int scrolly = (y + m_current_yscroll) & 0x1ff;
		uint16_t const *const source = &m_tmp_bitmap[LAYER_BG].pix(scrolly);
		uint16_t *const dest = &bitmap.pix(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
			dest[x] = source[(x + xscroll) & 0x1ff];
	}
}

uint32_t wheelfir_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_background(bitmap, cliprect);

	// the sprite plane is not scrolled
	copybitmap_trans(bitmap, m_tmp_bitmap[LAYER_FG], 0, 0, 0, -displayed_sprite_page() * 256, cliprect, 0);
	return 0;
}


void wheelfir_state::scanline_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scanline_cnt);
}

void wheelfir_state::control_w(uint8_t data)
{
	m_eeprom->di_write(BIT(data, 1));
	m_eeprom->clk_write(BIT(data, 0));
	m_eeprom->cs_write(BIT(data, 2));

	// bit 5 enables the line counter; clearing it withdraws a pending level-5 request
	const bool was_enabled = m_counter_enabled;
	m_counter_enabled = BIT(data, 5);
	if (was_enabled && !m_counter_enabled)
		m_maincpu->set_input_line(5, CLEAR_LINE);
}

uint8_t wheelfir_state::control_r()
{
	// bit 5 is the link receiver's status: at 1, nothing pending, a cabinet with
	// no partner
	return 0x20 | (m_eeprom->do_read() << 7);
}

void wheelfir_state::coin_cnt_w(uint16_t data)
{
	/* bits 0/1 coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
}

int wheelfir_state::adc_eoc_r()
{
	return m_adc_eoc;
}

void wheelfir_state::adc_eoc_w(int state)
{
	m_adc_eoc = state;
}

void wheelfir_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	// Kong Ball addresses its level-5 handler and a few variables at 0x21xxxx; a partial decode is
	// assumed rather than a second bank
	map(0x200000, 0x20ffff).mirror(0x010000).ram();

	map(0x700000, 0x70001f).w(FUNC(wheelfir_state::blit_w)).share(m_blitter_data);
	map(0x720001, 0x720001).w(m_ramdac, FUNC(ramdac_device::index_w));
	map(0x720003, 0x720003).w(m_ramdac, FUNC(ramdac_device::pal_w));
	map(0x720005, 0x720005).w(m_ramdac, FUNC(ramdac_device::mask_w)); // word write?
	map(0x740000, 0x740001).w(m_soundlatch, FUNC(generic_latch_16_device::write));
	map(0x760000, 0x760001).w(FUNC(wheelfir_state::coin_cnt_w));
	map(0x780000, 0x78000f).umask16(0x00ff).rw(m_adc, FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w));
	map(0x7a0000, 0x7a0001).w(FUNC(wheelfir_state::scanline_cnt_w));
	// the low byte is the link's data byte to a second cabinet, which is not modelled
	map(0x7c0000, 0x7c0001).umask16(0xff00).rw(FUNC(wheelfir_state::control_r), FUNC(wheelfir_state::control_w));
	map(0x7e0000, 0x7e0001).portr("P1");
	map(0x7e0002, 0x7e0003).portr("P2");
}

void wheelfir_state::sub_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram();

	map(0x700000, 0x700001).w(m_ldac, FUNC(dac_word_interface::data_w));
	map(0x740000, 0x740001).w(m_rdac, FUNC(dac_word_interface::data_w));
	map(0x780000, 0x780001).r(m_soundlatch, FUNC(generic_latch_16_device::read));
}


void wheelfir_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw(m_ramdac, FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb888_w));
}

static INPUT_PORTS_START( pwball )
	PORT_START("P1")    /* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( "Test" )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(wheelfir_state::adc_eoc_r))
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    /* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STEERING")
	PORT_START("ACCELERATOR")
	PORT_START("BRAKE")
INPUT_PORTS_END


static INPUT_PORTS_START( wheelfir )
	PORT_INCLUDE(pwball)

	PORT_MODIFY("P1")
	// The board serves a steering-wheel cabinet and a joystick one. The game folds up into the
	// accelerator and down into the brake, so the wheel cabinet has no joystick directions.
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CABINET", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CABINET", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CABINET", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("CABINET", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("CABINET", 0x01, EQUALS, 0x00)
	// the gear shifter, a low/high lever that stays where it is put
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Gear Shift") PORT_TOGGLE

	PORT_MODIFY("STEERING")
	// 12 per frame at 50.08 Hz is about 600 counts per second
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(12) PORT_NAME("Steering Wheel") PORT_REVERSE

	PORT_MODIFY("ACCELERATOR")
	// The game reads each pedal as a switch, pressed above 128, so a key takes a pedal to full travel
	// at once and a tap registers in the menus.
	PORT_BIT(0xff, 0x00, IPT_PEDAL)  PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(255) PORT_NAME("Accelerator Pedal") PORT_MINMAX(0x00, 0xff) PORT_REVERSE

	PORT_MODIFY("BRAKE")
	PORT_BIT(0xff, 0x00, IPT_PEDAL2) PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(255) PORT_NAME("Brake Pedal") PORT_MINMAX(0x00, 0xff) PORT_REVERSE

	PORT_START("CABINET")
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Controls ) )
	PORT_CONFSETTING( 0x00, "Steering wheel" )
	PORT_CONFSETTING( 0x01, DEF_STR( Joystick ) )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(wheelfir_state::scanline_timer_callback)
{
	if (param == 0)
		m_current_yscroll = frame_top_yscroll();

	if (param < NUM_SCANLINES)
	{
		// the displayed half, sampled on line 0 (see m_page_at_line0)
		if (param == 0)
			m_page_at_line0 = (m_blitter_data[0x8] & VISIBLE_PAGE) ? 1 : 0;

		// the line counter: the game writes how many lines to wait and level 5 fires at zero
		if (m_counter_enabled && m_scanline_cnt > 0)
		{
			m_scanline_cnt--;

			if (m_scanline_cnt == 0)
				m_maincpu->set_input_line(5, HOLD_LINE);
		}
	}
	else
	{
		if (param == NUM_SCANLINES)
		{
			// FIXME: withdrawing a pending level-5 request at vblank is fitted to Power Ball's boot
			m_maincpu->set_input_line(5, CLEAR_LINE);
			m_maincpu->set_input_line(3, HOLD_LINE);
		}
	}

	m_subcpu->set_input_line(1, HOLD_LINE);
}

void wheelfir_state::machine_start()
{
	// the source wraps inside the gfx region with a mask, so the region has to be a power of two
	const uint32_t gfx_bytes = uint32_t(m_tilepages.length());
	assert(std::has_single_bit(gfx_bytes));
	m_gfx_mask = gfx_bytes - 1;

	m_current_yscroll = 0;
	m_page_at_line0 = 0;
	m_scroll_x_latched = 0;
	m_scroll_y_latched = 0;

	save_item(NAME(m_page_at_line0));
	save_item(NAME(m_word14_fill));
	save_item(NAME(m_direct_write_x0));
	save_item(NAME(m_direct_write_x1));
	save_item(NAME(m_direct_write_y0));
	save_item(NAME(m_direct_write_y1));
	save_item(NAME(m_direct_write_idx));
	save_item(NAME(m_direct_write_layer));
	save_item(NAME(m_scanline_cnt));
	save_item(NAME(m_counter_enabled));
	save_item(NAME(m_blit_opaque));
	save_item(NAME(m_scroll_x_latched));
	save_item(NAME(m_scroll_y_latched));
	save_item(NAME(m_current_yscroll));
	save_item(NAME(m_adc_eoc));
}

// The blitter is idle after reset, so its line is asserted: Power Ball's boot waits for the
// level-1 handler to find an empty list before it fires its first blit. The driver also clears
// the blitter's command state and the line counter.
void wheelfir_state::machine_reset()
{
	m_maincpu->set_input_line(1, ASSERT_LINE);
	m_counter_enabled = false;
	m_scanline_cnt = 0;
	m_word14_fill = false;
	m_blit_opaque = false;
	m_direct_write_x0 = 0;
	m_direct_write_x1 = 0;
	m_direct_write_y0 = 0;
	m_direct_write_y1 = 0;
	m_direct_write_idx = 0;
	m_direct_write_layer = LAYER_BG;
}


static const uint32_t texlayout_xoffset[512] = { STEP512(0,8) };
static const uint32_t texlayout_yoffset[512] = { STEP512(0,4096) };

static const gfx_layout wheelfir512x5125x8_texlayout =
{
	512, 512,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	512*512*8,
	texlayout_xoffset,
	texlayout_yoffset
};

static GFXDECODE_START( gfx_wheelfir )
	GFXDECODE_ENTRY( "gfx1", 0, wheelfir512x5125x8_texlayout, 0x0, 1 )
GFXDECODE_END

void wheelfir_state::board(machine_config &config)
{
	M68000(config, m_maincpu, MASTER_CLOCK / 2);
	M68000(config, m_subcpu, MASTER_CLOCK / 2);

	config.set_maximum_quantum(attotime::from_hz(12000));

	ADC0808(config, m_adc, 500'000); // unknown clock
	m_adc->eoc_ff_callback().set(FUNC(wheelfir_state::adc_eoc_w));
	m_adc->in_callback<0>().set_ioport("STEERING");
	m_adc->in_callback<1>().set_ioport("ACCELERATOR");
	m_adc->in_callback<2>().set_ioport("BRAKE");

	TIMER(config, "scan_timer").configure_scanline(FUNC(wheelfir_state::scanline_timer_callback), m_screen, 0, 1);

	SCREEN(config, m_screen);
	// 32 MHz / 4 over 512 pixels is the 15625 Hz line rate the sound mixer counts against, and 312
	// lines give 50.08 Hz. No register is known to move the level-3 interrupt, which comes after
	// the 256 scanned lines; the monitor shows the first 252 of them.
	m_screen->set_raw(MASTER_CLOCK / 4, PIXELS_PER_LINE, 0, VISIBLE_COLUMNS,
			NUM_SCANLINES + NUM_VBLANK_LINES, FIRST_VISIBLE_LINE, VISIBLE_LINES);

	m_screen->set_screen_update(FUNC(wheelfir_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_wheelfir);

	PALETTE(config, m_palette).set_entries(NUM_COLORS);
	RAMDAC(config, m_ramdac, m_palette);
	m_ramdac->set_addrmap(0, &wheelfir_state::ramdac_map);

	EEPROM_93C46_16BIT(config, m_eeprom);

	GENERIC_LATCH_16(config, m_soundlatch);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();
	DAC_10BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 0); // unknown DAC
	DAC_10BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 1); // unknown DAC
}

void wheelfir_state::wheelfir(machine_config &config)
{
	board(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &wheelfir_state::main_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &wheelfir_state::sub_map);
}


void wheelfir_state::kongball(machine_config& config)
{
	wheelfir(config);

	m_subcpu->set_disable(); // sound ROMS were not present, so don't run sound CPU
}


// Radical Enduro, on the board's earlier layout, and what it does differently from the later one:
// the sprite plane's displayed half is decided on the blit rather than on line 0, and each frame
// starts from the scroll the registers hold, with the X scroll read live.
class radendur_state : public wheelfir_state
{
public:
	radendur_state(const machine_config &mconfig, device_type type, const char *tag) :
		wheelfir_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void sprite_plane_blit() override;
	virtual int32_t frame_top_yscroll() const override { return scroll_y_registers(); }
	virtual int32_t background_xscroll() const override { return scroll_x_registers(); }
	virtual int32_t displayed_sprite_page() const override { return m_fg_dispbuf; }

private:
	// the sprite plane's two 256-row pages: the one being drawn and the one shown
	int32_t m_fg_drawbuf;
	int32_t m_fg_dispbuf;

	void flip_page(int buf);
};

// the game issues its drawing chain inside vblank, so the page flips on the blit; the half coming
// in is cleared by the game's own fill
void radendur_state::flip_page(int buf)
{
	if (buf == m_fg_drawbuf)
		return;
	m_fg_dispbuf = m_fg_drawbuf;
	m_fg_drawbuf = buf;
}

// bit 0x20 of word 8 is the displayed half; the blits draw into the other one
void radendur_state::sprite_plane_blit()
{
	flip_page((m_blitter_data[0x8] & VISIBLE_PAGE) ? 0 : 1);
}

void radendur_state::machine_start()
{
	wheelfir_state::machine_start();

	m_fg_drawbuf = 0;
	m_fg_dispbuf = 0;

	save_item(NAME(m_fg_drawbuf));
	save_item(NAME(m_fg_dispbuf));
}


ROM_START( wheelfir )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tch1.u19", 0x00001, 0x80000, CRC(33bbbc67) SHA1(c2ecc0ab522ee442076ea7b9536aee6e1fad0540) )
	ROM_LOAD16_BYTE( "tch2.u21", 0x00000, 0x80000, CRC(ed6b9e8a) SHA1(214c5aaf55963a219db33dd5d530492e09ad5e07) )

	ROM_REGION( 0x100000, "subcpu", 0 ) // 68000 Code + sound samples
	ROM_LOAD16_BYTE( "tch3.u83",  0x00001, 0x80000, CRC(43c014a6) SHA1(6c01a08dda204f36e8768795dd5d405576a49140) )
	ROM_LOAD16_BYTE( "tch11.u65", 0x00000, 0x80000, CRC(fc894b2e) SHA1(ebe6d1adf889731fb6f53b4ce5f09c60e2aefb97) )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // each ROM contains 2 512x512 8bpp gfx pages
	ROM_LOAD( "tch4.u52", 0x000000, 0x80000, CRC(fe4bc2c7) SHA1(33a2ef79cb13f9e7e7d513915c6e13c4e7fe0188) )
	ROM_LOAD( "tch5.u53", 0x080000, 0x80000, CRC(a38b9ca5) SHA1(083c9f700b9df1039fb553e918e205c6d32057ad) )
	ROM_LOAD( "tch6.u54", 0x100000, 0x80000, CRC(2733ae6b) SHA1(ebd91e123b670159f79be19a552d1ae0c8a0faff) )
	ROM_LOAD( "tch7.u55", 0x180000, 0x80000, CRC(6d98f27f) SHA1(d39f7f184abce645b9165b64e89e3b5354187eea) )
	ROM_LOAD( "tch8.u56", 0x200000, 0x80000, CRC(22b661fe) SHA1(b6edf8e1e8b479ee8813502157615f54627dc7c1) )
	ROM_LOAD( "tch9.u57", 0x280000, 0x80000, CRC(83c66de3) SHA1(50deaf3338d590340b928f891548c47ba8f3ca38) )
	ROM_LOAD( "tch10.u58",0x300000, 0x80000, CRC(2036ed80) SHA1(910381e2ccdbc2d06f873021d8af02795d22f595) )
	ROM_LOAD( "tch12.u59",0x380000, 0x80000, CRC(cce2e675) SHA1(f3d8916077b2e057169d0f254005cd959789a3b3) )

	ROM_REGION16_BE(0x80, "eeprom", 0)
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x000000, 0x000080, CRC(961e4bc9) SHA1(8944504bf56a272e9aa08185e73c6b4212d52383) )

	ROM_REGION(0x2e5, "plds", 0)
	ROM_LOAD( "1_gal22v10.u24", 0x000, 0x2e5, NO_DUMP )
	ROM_LOAD( "2_gal22v10.u23", 0x000, 0x2e5, NO_DUMP )
	ROM_LOAD( "3_pal16v8.u40",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "4_pal16v8.u73",  0x000, 0x117, NO_DUMP )
ROM_END


ROM_START( pwball )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "pball.u19", 0x00001, 0x80000, CRC(52f433ce) SHA1(f25c43188c320636d69fec2de2605303ef79a2f8) )
	ROM_LOAD16_BYTE( "pball.u21", 0x00000, 0x80000, CRC(c0250bc8) SHA1(c7ad2dd0e7fde337a638b09fd325c0cfe1b0b966) )

	ROM_REGION( 0x100000, "subcpu", 0 ) // 68000 Code + sound samples
	ROM_LOAD16_BYTE( "pball.u63", 0x00001, 0x80000, CRC(ab8bba31) SHA1(cc89d1c8b5998b712161dda4283856acd8d91723) )
	ROM_LOAD16_BYTE( "pball.u65", 0x00000, 0x80000, CRC(f2583796) SHA1(f044a575ae3dafaac8ad05aee6d4b672166af969) )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // each ROM contains 2 512x512 8bpp gfx pages
	ROM_LOAD( "pball.u52", 0x000000, 0x80000, CRC(ad8567d3) SHA1(348a85388a5a98b3baa9484c947af6f878c4b70b) )
	ROM_LOAD( "pball.u53", 0x080000, 0x80000, CRC(9b954f59) SHA1(2cd9afea1677bb11e6d914c14e013da84e623356) )
	ROM_LOAD( "pball.u54", 0x100000, 0x80000, CRC(6979682d) SHA1(517abe80f4a44810427dd6da9187141ffd4a881c) )
	ROM_LOAD( "pball.u55", 0x180000, 0x80000, CRC(33593547) SHA1(9d13af4644e7fab01005609897c61acc50db8282) )
	ROM_LOAD( "pball.u56", 0x200000, 0x80000, CRC(bb64c0ed) SHA1(fa39121978412c02e7eb7adcf4d3b0b012904fee) )
	ROM_LOAD( "pball.u57", 0x280000, 0x80000, CRC(eb187063) SHA1(accd174cf5f582d6ed7ab3373bbe9e1bb1d1dbb8) )
	ROM_LOAD( "pball.u58", 0x300000, 0x80000, CRC(2cf2057d) SHA1(da075980892b43cac89d11c76d30b91c85c09af6) )
	ROM_LOAD( "pball.u59", 0x380000, 0x80000, CRC(e1c656f4) SHA1(9bc3713d4b5185b417f4d027ae97dace1feec065) )
	ROM_LOAD( "pball.u200",0x400000, 0x80000, CRC(26e68e24) SHA1(d88994a521ad14ac91e2c3cdc646ea7fb1ca8e72) )
	ROM_LOAD( "pball.u201",0x480000, 0x80000, CRC(bc8575fc) SHA1(0b1e237ee321f046f9b268e3515d07be206b4d76) )
	ROM_LOAD( "pball.u202",0x500000, 0x80000, CRC(97e2cf4f) SHA1(82ce199b0918762b0593b56bc8c19947993cfb59) )
	ROM_LOAD( "pball.u203",0x580000, 0x80000, CRC(89605b33) SHA1(6fb4ebacfd686266142317b3f864705d90ce97ca) )
	ROM_LOAD( "pball.u204",0x600000, 0x80000, CRC(efddd379) SHA1(903a931a7012f16df516b3cbec8b689a4fc0ba9c) )
	// u205 is empty (missing or unused?)

	ROM_REGION(0x2e5, "plds", 0)
	ROM_LOAD( "1_gal22v10.u24", 0x000, 0x2e5, NO_DUMP )
	ROM_LOAD( "2_gal22v10.u23", 0x000, 0x2e5, NO_DUMP )
	ROM_LOAD( "3_pal16v8.u40",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "4_pal16v8.u73",  0x000, 0x117, NO_DUMP )
ROM_END


ROM_START( kongball )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "kong.u19", 0x00001, 0x80000, CRC(0283df0a) SHA1(559b547127728c78ba66191278bba4b4cce37eba) )
	ROM_LOAD16_BYTE( "kong.u21", 0x00000, 0x80000, CRC(ca6ad0da) SHA1(d5e07d7827587263b8d3028ab006dc0be454a7c7) )

	ROM_REGION( 0x100000, "subcpu", 0 ) // 68000 Code + sound samples
	// these were not present on the PCB (empty sockets) was sound never programmed, or were they removed at some point?
	ROM_LOAD16_BYTE( "kong.u63", 0x00001, 0x80000, NO_DUMP )
	ROM_LOAD16_BYTE( "kong.u65", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // each ROM contains 2 512x512 8bpp gfx pages
	ROM_LOAD( "kong.u52", 0x000000, 0x80000, CRC(52487271) SHA1(a15a4011b1b23f5b2af0d7b6d05b3df3ae1daebb) )
	ROM_LOAD( "kong.u53", 0x080000, 0x80000, CRC(aaaa3840) SHA1(0dcd2d70687d3c646198c40743b8b0a64cafced0) )
	ROM_LOAD( "kong.u54", 0x100000, 0x80000, CRC(cb7ef2c2) SHA1(293b694e201221a5b27b22a1b729db5d7e52bf99) )
	ROM_LOAD( "kong.u55", 0x180000, 0x80000, CRC(c9596c6f) SHA1(dd989d3e0c0be696331a8c42e03439fec8685bc8) )
	ROM_LOAD( "kong.u56", 0x200000, 0x80000, CRC(cbb69adc) SHA1(244d36e6913feb7edc606fdaa7a80f2a8a480efa) )
	ROM_LOAD( "kong.u57", 0x280000, 0x80000, CRC(f33af567) SHA1(8cbf9cc78b95edf33fb250bd232f032b4d7f4881) )
	ROM_LOAD( "kong.u58", 0x300000, 0x80000, CRC(17818ee8) SHA1(7e857f7c5c146cfcdaa9b1077c63bbee1d143438) )
	ROM_LOAD( "kong.u59", 0x380000, 0x80000, CRC(6d35e911) SHA1(d37bc2d364ced0d828598af9b2fe7abf6ffb4f82) )
	ROM_LOAD( "kong.u200",0x400000, 0x80000, CRC(ec552e97) SHA1(7a9a06ff77ef7fc51782caa4d4514864bf8099e7) )

	ROM_REGION(0x2e5, "plds", 0)
	ROM_LOAD( "1_gal22v10.u24", 0x000, 0x2e5, NO_DUMP )
	ROM_LOAD( "2_gal22v10.u23", 0x000, 0x2e5, NO_DUMP )
	ROM_LOAD( "kongball_mump3_275f_palce16v8h.u40", 0x000, 0x117, CRC(c6cf0b71) SHA1(c6581f87a4a97c1aeef91f6444c90fc5f4a32e57) )
	// There's no PLD at U73 on Kong Ball
ROM_END


ROM_START( radendur )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "endu.u19", 0x00001, 0x80000, CRC(d24ca61c) SHA1(64d2648959579d35fbf4c8479c0ff75781d28146) )
	ROM_LOAD16_BYTE( "endu.u21", 0x00000, 0x80000, CRC(45244675) SHA1(69be2eca3548644f97b9f6956bb3ff979740b1e4) )

	ROM_REGION( 0x100000, "subcpu", 0 ) // 68000 Code + sound samples
	ROM_LOAD16_BYTE( "endu.u63", 0x00001, 0x80000, CRC(43c35368) SHA1(62167a434bfe00825f2f97ecadd0af3e2d2b63f2) )
	ROM_LOAD16_BYTE( "endu.u65", 0x00000, 0x80000, CRC(25259894) SHA1(b260324760dba1595ab8bd53e1bbc20c04b640dd) )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // each ROM contains 2 512x512 8bpp gfx pages
	ROM_LOAD( "endu.u52", 0x000000, 0x80000, CRC(2b686f81) SHA1(9d9aa0f6e1a3dc7e96f647ddda3214f1f4c6bf1d) )
	ROM_LOAD( "endu.u53", 0x080000, 0x80000, CRC(1a224c69) SHA1(ed1a8fdd97ea0caa649d3f1238fa13b1fd19376c) )
	ROM_LOAD( "endu.u54", 0x100000, 0x80000, CRC(217a9725) SHA1(14240e457967c5b23c54d4395fc1ab7394a59fc9) )
	ROM_LOAD( "endu.u55", 0x180000, 0x80000, CRC(bfce1b17) SHA1(415ab6815836dafb1a0b86464e84f3aaa5d1075d) )
	ROM_LOAD( "endu.u56", 0x200000, 0x80000, CRC(4b9460a8) SHA1(d2b9484f133d5d9abbe6a163c57d079944b3221f) )
	ROM_LOAD( "endu.u57", 0x280000, 0x80000, CRC(f17d5752) SHA1(354a76e9e130f10fc103ab83ea15bf1f47fca391) ) // mostly blank ROM (contains a single image)
	ROM_LOAD( "endu.u58", 0x300000, 0x80000, CRC(75660aac) SHA1(6a521e1d2a632c26e53b83d2cc4b0edecfc1e68c) ) // blank ROM (intentional)
	ROM_LOAD( "endu.u59", 0x380000, 0x80000, CRC(037648df) SHA1(19b0cbdb60ac4b14f06dd71ddf4e3b2d2dbdb19d) ) // FIXED BITS (111xxxxx) but correct, not all 8bpp used
	ROM_LOAD( "endu.u200",0x400000, 0x80000, CRC(e1417f94) SHA1(dba02ce1f9bbe18e78c67e770fe84184e227799b) )
	ROM_LOAD( "endu.u201",0x480000, 0x80000, CRC(25d9db1c) SHA1(c61491ae2dc97ea5c9f0f0d58f32edf0a3a94d95) )
	ROM_LOAD( "endu.u202",0x500000, 0x80000, CRC(15f4fbd4) SHA1(2ad28af1bd0924fc3f1f6e026ca834c7df774b64) )
	ROM_LOAD( "endu.u203",0x580000, 0x80000, CRC(e6361a27) SHA1(826be3b326e68c3b7c0f5c3d84993a971b32f84e) )
	ROM_LOAD( "endu.u204",0x600000, 0x80000, CRC(0db97872) SHA1(56e38a9034082c3ceb78c0f3aba6125ce6fabd49) )
	ROM_LOAD( "endu.u205",0x680000, 0x80000, CRC(75660aac) SHA1(6a521e1d2a632c26e53b83d2cc4b0edecfc1e68c) ) // blank ROM (intentional)

	ROM_REGION(0x2e5, "plds", 0)
	ROM_LOAD( "radenduro_2_d579_gal22v10.u24", 0x000, 0x2e5, CRC(ebe162d0) SHA1(910e3733df70db4df704f3e105ed27eaa4c32ae6) ) // Checksum does not match chip label, buf verified OK
	ROM_LOAD( "radenduro_gal22v10b.u23",       0x000, 0x2e5, CRC(f1b0b2b7) SHA1(f85c87c7956813fe78f399694dc999877b1dcb60) )
	ROM_LOAD( "3_pal16v8.u40",  0x000, 0x117, NO_DUMP )
	ROM_LOAD( "4_pal16v8.u73",  0x000, 0x117, NO_DUMP )
ROM_END

} // anonymous namespace

GAMEL( 199?, wheelfir,   0, wheelfir,    wheelfir, wheelfir_state, empty_init,    ROT0,  "TCH", "Wheels & Fire", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_wheelfir )
GAME( 199?, pwball,      0, wheelfir,    pwball,   wheelfir_state, empty_init,    ROT0,  "TCH", "Power Ball (prototype)",    MACHINE_SUPPORTS_SAVE )

// sound ROMs were missing on PCB, so sound emulation is not possible at this point in time
GAME( 1997, kongball,    0, kongball,    pwball,   wheelfir_state, empty_init,    ROT0,  "TCH / Digital Dreams Multimedia", "Kong Ball (prototype)",    MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

// crashes (always?) when selecting track on PCB
GAME( 199?, radendur,    0,        wheelfir,    pwball,   radendur_state, empty_init,    ROT0,  "TCH / Sator Videogames", "Radical Enduro (early prototype)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
