// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, Marcos Hernández
/****************************************************************

Wheels & Fire
Power Ball (prototype)
Kong Ball (prototype, TCH / Digital Dreams Multimedia)
Radical Enduro (early prototype, TCH / Sator Videogames)

Original driver by Tomasz Slanina.
The video model, the blitter and their confirmation against the machine by Marcos
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
  ones) and only while bit 13 of 0x7c0000 is set; it clocks the sound streamer (one word per
  line to the sub cpu) and, in Wheels & Fire, the road's per-line scroll

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

The field is derived from the 32 MHz crystal: 512 pixels by 312 lines, a 15625 Hz line rate.
The games clip their sprites against 336x256; 252 lines are shown.

TODO:
- the zoom appears to be a stall and a repeat counter per axis on the board; here each step's
  source comes from the scale descriptors the program ROM carries for its own blit setup (found
  at start by their known answers: 100% the identity, 200% exactly double), with an
  approximation from ZOOM_INDEX for a program without them. Power Ball and Radical Enduro carry
  none and were not seen drawing a scaled blit
- a zoomed blit stops one step short of its endpoint, and a blit at 1:1 vertically skips its
  first source row, a border the art carries: both fitted to the games' output
- the background is drawn three rows lower than its scroll places it (BACKGROUND_ROW_BIAS), but
  not across the road/scenery boundary or the plane's wrap: an exact fit for Wheels & Fire's
  road with no mechanism identified
- a background blit covering the whole window is drawn opaque (full_background in do_blit): a
  rule fitted to the High Scores photo, which the game asks for transparent with nothing
  under it
- Radical Enduro's sprite-plane flip, palette and frame scroll differ from the other games
  (radendur_state) on the same board; one model for all four is not found yet, and its
  palette routing by the plane of each upload stands in for a mechanism not identified
- the analogue accelerator is two steps, and the game needs button 1 held to accelerate past a
  low speed; what the cabinet wired to that button is not known
- a line-counter request the main has not taken by vblank is withdrawn: fitted to Power Ball's
  warm boot, whose sound CPU otherwise dies on the block header the held request sends
- clearing bit 13 of 0x7c0000 also withdraws a pending line-counter request; that the board
  does so is not established
- no layer enable is identified: no bit seen written (Wheels & Fire and Power Ball in play, Kong
  Ball's attract) behaves as one, so both planes are always composed
- word 4's low byte, with its bit 8 in word 8, is written by the games and read by nothing
- the link between two cabinets (0x7c0000) answers as a cabinet alone

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
#include <cstring>

#define LOG_DIRECT (1U << 1)
#define LOG_SCALE  (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

constexpr int ZOOM_TABLE_SIZE = 1 << 14;
// The games clip their sprites in software against (0, 0, 0x150, 0xff), kept at 0x20b4b8 in
// Wheels & Fire, so they draw rows 0..255; 252 of them are shown.
constexpr int NUM_SCANLINES = 256;
constexpr int FIRST_VISIBLE_LINE = 0;
constexpr int NUM_VBLANK_LINES = 56; // the field stays at 312 lines
constexpr int VISIBLE_LINES = 252;   // of the 256 scanned, what leaves the monitor
constexpr int PIXELS_PER_LINE = 512; // MASTER_CLOCK/4 over these is the 15625 Hz line rate
constexpr XTAL MASTER_CLOCK = 32_MHz_XTAL; // the board's only crystal
constexpr int LAYER_BG = 0;
constexpr int LAYER_FG = 1;
constexpr int NUM_COLORS = 256;
constexpr int GFX_PAGE_BYTES = 512 * 512; // one 8-bit graphics page

// The flag bits of the blitter block, by word. The register map in blit_w is the source and
// says what each one is; these are only its names.
constexpr uint16_t SRC_U_BIT8    = 0x100;  // word 6: bit 8 of the source column
constexpr uint16_t SRC_V_BIT8    = 0x200;  // word 6: bit 8 of the source row
constexpr uint16_t DST_X_FORWARD = 0x1;    // word 7: the destination walks with increasing x
constexpr uint16_t DST_Y_FORWARD = 0x2;    // word 7: and with increasing y
constexpr uint16_t READS_SOURCE  = 0x4;    // word 7: the blit reads its graphic
constexpr uint16_t WRITES_ZERO   = 0x8;    // word 7: the blit also writes index 0
constexpr uint16_t DST_PLANE_BG  = 0x10;   // word 7: destination plane, set = background
constexpr uint16_t DST_X_BIT8    = 0x40;   // word 7: bit 8 of the destination x0
constexpr uint16_t DST_Y_BIT8    = 0x80;   // word 7: bit 8 of the destination y0
constexpr uint16_t VISIBLE_PAGE  = 0x0020; // word 8: the sprite page displayed
constexpr uint16_t SCROLL_X_BIT8 = 0x0040; // word 8: bit 8 of the background scroll x
constexpr uint16_t SCROLL_Y_BIT8 = 0x0080; // word 8: bit 8 of the background scroll y
constexpr uint16_t SRC_U_FORWARD = 0x4000; // word 8: the source walks with increasing u
constexpr uint16_t SRC_V_FORWARD = 0x8000; // word 8: and with increasing v
constexpr uint16_t DST_X1_BIT8   = 4;      // word 9: bit 8 of the destination x1
constexpr uint16_t DST_Y1_BIT8   = 8;      // word 9: bit 8 of the destination y1

// The blitter's scale fields for each percentage, from the descriptors of the Wheels & Fire ROM;
// machine_start checks them against the horizontal table it finds.
constexpr uint16_t ZOOM_INDEX[400] =
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1801, 0x1ea1, 0x1ba1, 0x1f61, 0x19e1,
	0x19a1, 0x1961, 0x1961, 0x1921, 0x1fe2, 0x1d62, 0x1ba2, 0x1aa2, 0x1a22, 0x1ea3,
	0x1804, 0x1fe4, 0x1d64, 0x1fe5, 0x1ae4, 0x1d66, 0x1fe9, 0x1fec, 0x1eac, 0x181f,
	0x0801, 0x0fe1, 0x0ea1, 0x0ca1, 0x0ba1, 0x0ae1, 0x0f61, 0x0a21, 0x09e1, 0x09e1,
	0x09a1, 0x09a1, 0x0961, 0x0961, 0x0961, 0x0802, 0x0921, 0x0921, 0x0fe2, 0x0fa2,
	0x0d62, 0x0c22, 0x0ba2, 0x0b22, 0x0aa2, 0x08e1, 0x0a22, 0x0fe3, 0x0ea3, 0x0d23,
	0x0804, 0x0ba3, 0x0fe4, 0x0f24, 0x0d64, 0x0fe5, 0x0fe5, 0x0e25, 0x0ae4, 0x0fe7,
	0x0d66, 0x0ea7, 0x0fe9, 0x0fe9, 0x0fec, 0x0ce9, 0x0eac, 0x0ff4, 0x081f, 0x0fff,
	0x0000, 0x0fff, 0x07e0, 0x0bb6, 0x06a0, 0x0560, 0x0bbf, 0x0aba, 0x09a9, 0x0abf,
	0x0a7f, 0x0a3f, 0x09ae, 0x09ff, 0x09b3, 0x09b7, 0x09bd, 0x09bf, 0x09bf, 0x0974,
	0x01a0, 0x097e, 0x097f, 0x097f, 0x092d, 0x0160, 0x0931, 0x0934, 0x0938, 0x093e,
	0x093f, 0x093f, 0x093f, 0x08a2, 0x0120, 0x08e9, 0x08ea, 0x08eb, 0x08ec, 0x08ed,
	0x08ee, 0x08f0, 0x08f2, 0x08f4, 0x08f8, 0x08fd, 0x08ff, 0x08ff, 0x08ff, 0x08ff,
	0x00e0, 0x00e0, 0x00e0, 0x00e0, 0x00e0, 0x00e0, 0x08a4, 0x08a4, 0x08a4, 0x08a4,
	0x08a4, 0x08a4, 0x08a4, 0x08a4, 0x08a5, 0x08a5, 0x08a5, 0x08a5, 0x08a5, 0x08a6,
	0x08a6, 0x08a6, 0x08a6, 0x08a6, 0x08a7, 0x08a7, 0x08a7, 0x08a8, 0x08a8, 0x08a9,
	0x08a9, 0x08aa, 0x08aa, 0x08ab, 0x08ac, 0x08ad, 0x08ad, 0x08ae, 0x08b0, 0x08b1,
	0x08b3, 0x08b5, 0x08b8, 0x08bc, 0x08bf, 0x08bf, 0x08bf, 0x08bf, 0x08bf, 0x00a0,
	0x00a0, 0x2000, 0x2fff, 0x2fff, 0x27e0, 0x27e0, 0x2bb6, 0x2bb6, 0x26a0, 0x26a0,
	0x2560, 0x2560, 0x2bbf, 0x2bbf, 0x2aba, 0x2aba, 0x29a9, 0x29a9, 0x2abf, 0x2abf,
	0x2a7f, 0x2a7f, 0x2a3f, 0x2a3f, 0x29ae, 0x29ae, 0x29ff, 0x29ff, 0x29b3, 0x29b3,
	0x29b7, 0x29b7, 0x29bd, 0x29bd, 0x29bf, 0x29bf, 0x29bf, 0x29bf, 0x2974, 0x2974,
	0x21a0, 0x21a0, 0x297e, 0x297e, 0x297f, 0x297f, 0x297f, 0x297f, 0x292d, 0x292d,
	0x2160, 0x2160, 0x2931, 0x2931, 0x2934, 0x2934, 0x2938, 0x2938, 0x293e, 0x293e,
	0x293f, 0x293f, 0x293f, 0x293f, 0x293f, 0x293f, 0x28a2, 0x28a2, 0x2120, 0x2120,
	0x28e9, 0x28e9, 0x28ea, 0x28ea, 0x28eb, 0x28eb, 0x28ec, 0x28ec, 0x28ed, 0x28ed,
	0x28ee, 0x28ee, 0x28f0, 0x28f0, 0x28f2, 0x28f2, 0x28f4, 0x28f4, 0x28f8, 0x28f8,
	0x28fd, 0x28fd, 0x28ff, 0x28ff, 0x28ff, 0x28ff, 0x28ff, 0x28ff, 0x28ff, 0x28ff,
	0x20e0, 0x20e0, 0x20e0, 0x20e0, 0x20e0, 0x20e0, 0x20e0, 0x20e0, 0x20e0, 0x20e0,
	0x20e0, 0x20e0, 0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a4,
	0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a4, 0x28a5, 0x28a5,
	0x28a5, 0x28a5, 0x28a5, 0x28a5, 0x28a5, 0x28a5, 0x28a5, 0x28a5, 0x28a6, 0x28a6,
	0x28a6, 0x28a6, 0x28a6, 0x28a6, 0x28a6, 0x28a6, 0x28a6, 0x28a6, 0x28a7, 0x28a7,
	0x28a7, 0x28a7, 0x28a7, 0x28a7, 0x28a8, 0x28a8, 0x28a8, 0x28a8, 0x28a9, 0x28a9,
	0x28a9, 0x28a9, 0x28aa, 0x28aa, 0x28aa, 0x28aa, 0x28ab, 0x28ab, 0x28ac, 0x28ac,
	0x28ad, 0x28ad, 0x28ad, 0x28ad, 0x28ae, 0x28ae, 0x28b0, 0x28b0, 0x28b1, 0x28b1,
	0x28b3, 0x28b3, 0x28b5, 0x28b5, 0x28b8, 0x28b8, 0x28bc, 0x28bc, 0x28bf, 0x28bf,
	0x28bf, 0x28bf, 0x28bf, 0x28bf, 0x28bf, 0x28bf, 0x28bf, 0x28bf, 0x20a0, 0x20a0
};

class wheelfir_state : public driver_device
{
public:
	wheelfir_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_blitter_data(*this, "blitram"),
		m_eeprom(*this, "eeprom"),
		m_tilepages(*this, "gfx1"),
		m_maincpurom(*this, "maincpu")
	{ }

	void wheelfir(machine_config &config) ATTR_COLD;
	void kongball(machine_config &config) ATTR_COLD;

	int adc_eoc_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// what Radical Enduro does differently on the same board (see radendur_state)
	virtual void sprite_plane_blit() { }
	virtual int32_t frame_top_yscroll() const { return m_scroll_y_latched; }

	void board(machine_config &config) ATTR_COLD;
	void board_main_map(address_map &map) ATTR_COLD;
	void board_sub_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_blitter_data;

	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[2]{};
	int32_t m_current_yscroll = 0;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_region_ptr<uint8_t> m_tilepages;
	required_region_ptr<uint16_t> m_maincpurom;

	std::unique_ptr<int32_t[]> m_zoom_table;

	// The sprite plane page the game selected on line 0 of the frame, the half displayed. Word 8 is
	// rewritten by every blit, so it is sampled here.
	uint8_t m_page_at_line0 = 0;
	// 0xffff was written to word 14 since the last arming: the next trigger is a fill, not a blit.
	bool m_word14_fill = false;
	bool m_blit_opaque = false;   // the last mode write since the arming selected opaque

	int32_t m_direct_write_x0 = 0;
	int32_t m_direct_write_x1 = 0;
	int32_t m_direct_write_y0 = 0;
	int32_t m_direct_write_y1 = 0;
	int32_t m_direct_write_idx = 0;
	// the plane the direct-write channel streams into: the plane bit of the command that opened it
	int32_t m_direct_write_layer = LAYER_BG;

	int16_t m_scanline_cnt = 0;
	bool m_counter_enabled = false;

	// The two tables of 400 16-byte scale descriptors in the 68000 ROM, in the order found, the first
	// taken as horizontal; -1 when the known answers do not find them, and the approximation is used.
	int32_t m_scale_desc[2] = { -1, -1 };
	bool m_board_scale = false;
	uint32_t m_rom_mask = 0;
	uint32_t m_gfx_mask = 0;   // the gfx1 region's length - 1, a power of two
	// destination steps that asked for more source than fits in a 512 page, reported per frame
	int m_source_exhausted = 0;

	// The 9-bit background scroll, latched when the game writes the Y byte. Its bit 8s live in
	// word 8, which every blit rewrites, so they are captured with the coordinates.
	int32_t m_scroll_x_latched = 0;
	int32_t m_scroll_y_latched = 0;

	int32_t m_adc_eoc = 0;

	// A word of the 68000 ROM by byte address; an AND because the region is a power of two.
	uint16_t rom_w(uint32_t byteaddr) const
	{
		return m_maincpurom[(byteaddr >> 1) & m_rom_mask];
	}

	// The scaling law of the routine at 0xe47fe of the Wheels & Fire program: a source offset maps
	// to the destination as (2x + tableA[x] - tableB[...]) >> shift, tables and shift from the
	// descriptor.
	struct scale_law
	{
		int shift;
		uint32_t table_a;
		uint32_t table_b;
	};

	scale_law law_at(uint32_t base, int j) const
	{
		const uint32_t d = base + uint32_t(j) * 16;
		return scale_law{ rom_w(d + 6), (uint32_t(rom_w(d + 8)) << 16) | rom_w(d + 10), (uint32_t(rom_w(d + 12)) << 16) | rom_w(d + 14) };
	}

	int scale_at(const scale_law &law, int x) const
	{
		uint16_t v = uint16_t(2 * x + int16_t(rom_w(law.table_a + 2 * x)));
		v = uint16_t(v - int16_t(rom_w(law.table_b + v)));
		return v >> law.shift;
	}

	// The law maps source to destination and the blit walks the destination, so it is inverted:
	// assuming the law never decreases, each destination step takes the first source offset that
	// reaches it.
	void source_map(int axis, int j, int steps, int *inv)
	{
		const scale_law law = law_at(uint32_t(m_scale_desc[axis]), j);
		int x = 0;
		for (int d = 0; d <= steps; d++)
		{
			while (x < 511 && scale_at(law, x) < d)
				++x;
			if (x >= 511 && scale_at(law, x) < d)
				++m_source_exhausted;
			inv[d] = x;
		}
	}

	int32_t get_scale(int32_t index)
	{
		while (index < ZOOM_TABLE_SIZE)
		{
			if (m_zoom_table[index] >= 0)
			{
				return m_zoom_table[index];
			}
			++index;
		}
		return 0;
	}

	void scanline_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void do_direct_write(uint8_t sixdat);
	void do_blit();
	void blit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t control_r(offs_t offset, uint16_t mem_mask = ~0);
	void coin_cnt_w(uint16_t data);
	void adc_eoc_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void report_exhausted_source();
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer_callback);
	void ramdac_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};

void wheelfir_state::video_start()
{
	m_tmp_bitmap[0] = std::make_unique<bitmap_ind16>(512, 512);
	m_tmp_bitmap[1] = std::make_unique<bitmap_ind16>(512, 512);

	// Both planes persist between frames and are painted blit by blit, so they are state.
	for (int i = 0; i < 2; i++)
		save_item(NAME(*m_tmp_bitmap[i]), i);
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

	// word 7 bit 2 reads the source and bit 3 also writes index 0: both together are a colour fill,
	// bit 3 alone an erase that writes zeros and walks its destination in columns
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
		// the channel's plane is the plane bit every blit carries: the game's video reset (0xe8d3c)
		// opens it on the sprite plane and the scenery loaders on the background
		m_direct_write_layer = (m_blitter_data[0x7] & DST_PLANE_BG) ? LAYER_BG : LAYER_FG;

		// this trigger defines the channel's window and draws nothing: its "source" is above the ROM
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
		const int y0 = dst_y0 & 0x1ff;
		const int end = y0 + rows - 1;
		m_tmp_bitmap[vpage]->fill(0, rectangle(0, 511, y0, std::min(end, 511)));
		if (end > 511)
			m_tmp_bitmap[vpage]->fill(0, rectangle(0, 511, 0, end - 512));
		return;
	}

	if (erase)
	{
		std::swap(dst_x0, dst_y0);
		std::swap(dst_x1, dst_y1);
		std::swap(x_dst_step, y_dst_step);
	}

	// calculate u zoom (horizontal source scale)
	const int d1u = ((m_blitter_data[0x0a] & 0x1f00) >> 8) |
					((m_blitter_data[0x08] & 0x0100) >> 3);
	const int d2u = ((m_blitter_data[0x0b] & 0x1f00) >> 8) |
					((m_blitter_data[0x08] & 0x0400) >> 5);
	const int hflagu = (m_blitter_data[0x09] & 0x0001) ? 1 : 0;
	const int dflagu = (m_blitter_data[0x08] & 0x1000) ? 1 : 0;
	const int indexu = d1u | (d2u << 6) | (hflagu << 12) | (dflagu << 13);
	const float scale_u = get_scale(indexu);

	// calculate v zoom (vertical source scale)
	const int d1v = ((m_blitter_data[0x0b] & 0xc000) >> 14) |
					((m_blitter_data[0x0c] & 0xc000) >> 12) |
					((m_blitter_data[0x0a] & 0x4000) >> 10) |
					((m_blitter_data[0x08] & 0x0200) >> 4);

	const int d2v = ((m_blitter_data[0x0c] & 0x1f00) >> 8) |
					((m_blitter_data[0x08] & 0x0800) >> 6);
	const int hflagv = (m_blitter_data[0x09] & 0x0002) ? 1 : 0;
	const int dflagv = (m_blitter_data[0x08] & 0x2000) ? 1 : 0;
	const int indexv = d1v | (d2v << 6) | (hflagv << 12) | (dflagv << 13);
	const float scale_v = get_scale(indexv);

	if (scale_u == 0 || scale_v == 0)
	{
		return;
	}

	const float scale_u_step = 100.f / scale_u;
	const float scale_v_step = 100.f / scale_v;

	// 1:1 blits draw their endpoint, zoomed ones do not (fitted, see the TODO)
	const bool inclusive = (scale_u == 100.f && scale_v == 100.f);

	// Both planes wrap: the walk is counted with the 9-bit destination counters, which carry a walk
	// past 511 onto row or column 0, so every write lands on the plane.
	const int steps_x = ((dst_x1 - dst_x0) * x_dst_step) & 0x1ff;
	const int steps_y = ((dst_y1 - dst_y0) * y_dst_step) & 0x1ff;

	int y = dst_y0;

	// at 1:1 vertically the first source row, a border the art carries, is skipped (fitted, see the
	// TODO); at other scales, whether the board does the same is not known
	const bool ident_u = (scale_u == 100.f);
	const bool ident_v = (scale_v == 100.f);
	const float v_bias = ident_v ? 1.0f : 0.0f;

	// with the program's descriptors, each destination step's source comes from the inverse map;
	// built per axis, only when the scale is not the identity
	int inv_u[512], inv_v[512];
	const int j_u = int(scale_u), j_v = int(scale_v);
	const bool exact_u = m_board_scale && j_u != 100 && j_u < 400;
	const bool exact_v = m_board_scale && j_v != 100 && j_v < 400;
	if (exact_u) source_map(0, j_u, steps_x, inv_u);
	if (exact_v) source_map(1, j_v, steps_y, inv_v);

	// a background blit covering the whole window is drawn opaque (fitted, see the TODO)
	static constexpr int FULL_BACKGROUND_COLUMNS = 336;
	static constexpr int FULL_BACKGROUND_ROWS = 254;
	const bool full_background = vpage == LAYER_BG
			&& (dst_x1 - dst_x0 + 1) >= FULL_BACKGROUND_COLUMNS && (dst_y1 - dst_y0 + 1) >= FULL_BACKGROUND_ROWS;
	const bool opaque = writes_zero || m_blit_opaque || full_background;
	bitmap_ind16 &plane = *m_tmp_bitmap[vpage];

	float idx_v = 0;
	int step_y = 0;
	do
	{
		int x = dst_x0;
		int step_x = 0;
		float idx_u = 0;

		// at 1:1 the source step is the destination step, in integers; the float accumulator is
		// for the approximation only
		const int yy = exact_v ? src_v0 + v_src_step * inv_v[step_y]
				: ident_v ? src_v0 + v_src_step * (step_y + int(v_bias))
				: int(src_v0 + v_src_step * (idx_v + v_bias));
		const int py = y & 0x1ff;

		do
		{
			const int xx = exact_u ? src_u0 + u_src_step * inv_u[step_x]
					: ident_u ? src_u0 + u_src_step * step_x
					: int(src_u0 + u_src_step * idx_u);

			// keep the source inside its own 512x512 page
			const int address = page + (yy & 0x1ff) * 512 + (xx & 0x1ff);

			// an erase writes index 0, not the source pixel
			const int pix = erase ? 0 : rom[address & m_gfx_mask];

			const int px = x & 0x1ff;
			if (pix || opaque)
				plane.pix(py, px) = pix;

			// the endpoint is part of the blit at 1:1 only
			if (step_x >= steps_x + (inclusive ? 0 : -1)) break;
			step_x++;

			x += x_dst_step;
			if (!ident_u) idx_u += scale_u_step;

		} while (true);

		// and so is the endpoint row, same restriction
		if (step_y >= steps_y + (inclusive ? 0 : -1)) break;
		step_y++;

		y += y_dst_step;
		if (!ident_v) idx_v += scale_v_step;

	} while (true);

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
			m_tmp_bitmap[m_direct_write_layer]->pix(y, x) = sixdat;
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
	8 10    FFZzZzZz   FF = Y/X direction of the source walk, Z = y scale x2 (201..399%) z = x scale x2, Z = y repeat counter bit 5, z = x repeat counter bit 5, Z = y stall counter bit 5, z = x stall counter bit 5
	  11    SsPW----   s = scroll x of back layer bit 8, S = scroll y of back layer bit 8, P = which page of the sprite plane is displayed, sampled on line 0, W = bit 8 of word 4's value
	9 12    --------
	  13    ----YXZz   Y = dest_y1 bit 8, X = dest_x1 bit 8, Z = y scale x1/2 (25..49%), z = x scale x1/2
	a 14    -Z-zzzzz   Z = y stall counter bit 4, z = x stall counter bits 0-4,
	  15    ssssssss   s = scroll x of back layer
	b 16    ZZ-zzzzz   Z = y stall counter bits 0-1, z = x repeat counter bits 0-4
	  17    SSSSSSSS   S = scroll y of back layer
	c 18    ZZ-ZZZZZ   Z = y stall counter bits 2-3, Z = y repeat counter bits 0-4
	  19    tttttttt   t = a write here selects the transparent mode (index 0 is not written); the games write 0xff
	d 1a    --------
	  1b    oooooooo   o = a write here selects the opaque mode (index 0 is written); the games write 0xff
	e 1c    FFFFFFFF
	  1d    FFFFFFFF   F = 0xffff marks the next trigger as a fill: it clears the declared band of rows, edge to edge, to index 0
	f 1e    TTTTTTTT
	  1f    TTTTTTTT   T = trigger: 0x0000 arms the command (resets the mode latch, drops the IRQ line), 0xffff fires it
	*/

	m_screen->update_partial(m_screen->vpos() - 1);
	// 0xffff to word 14 marks the next trigger as a fill
	if (offset == 0xe && data == 0xffff) m_word14_fill = true;

	COMBINE_DATA(&m_blitter_data[offset]);

	// A write to the low byte of word 12 selects transparent and to word 13 opaque, word writes
	// included (Power Ball writes word 13 as a word); the arming write resets it. The byte ports
	// below test the other way, because the games write words 6, 8 and 11 as words for their high
	// halves.
	if (offset == 0xc && ACCESSING_BITS_0_7) m_blit_opaque = false;
	if (offset == 0xd && ACCESSING_BITS_0_7) m_blit_opaque = true;

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
		m_scroll_x_latched = (m_scroll_x_latched & 0x0ff) | (m_blitter_data[0x8] & SCROLL_X_BIT8) << 2;

	// the scroll is latched when the Y byte is written, which always follows the X, and the row
	// counter loaded here advances one row per line
	if (offset == 0xb && !ACCESSING_BITS_8_15)
	{
		m_scroll_y_latched = (m_blitter_data[0xb] & 0x00ff) | (m_blitter_data[0x8] & SCROLL_Y_BIT8) << 1;
		m_scroll_x_latched = (m_blitter_data[0xa] & 0x00ff) | (m_blitter_data[0x8] & SCROLL_X_BIT8) << 2;
		m_current_yscroll = m_scroll_y_latched - m_screen->vpos();
		return;
	}

	if (offset == 0xf)
	{
		if (data == 0xffff)
		{
			do_blit();
		}
		else if (data == 0x0000)
		{
			// arming resets the mode and the fill mark and acknowledges the blitter: the held
			// IRQ line drops here.
			m_blit_opaque = false;
			m_word14_fill = false;
			m_maincpu->set_input_line(1, CLEAR_LINE);
		}
		else
		{
			// writes 0x0000 before filling in some of the other params then sending 0xffff above
			logerror("%s: write to offset 0xf (blit start) but with data %04x\n", machine().describe_context(), data);
		}
	}
}

uint32_t wheelfir_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// the background is drawn three rows lower than its scroll places it (fitted, see the TODO),
	// except where that would cross the road/scenery boundary (row 256) or the plane's wrap
	static constexpr int BACKGROUND_ROW_BIAS = 3;

	auto row_read = [&](int y)
	{
		const int wanted = (y + m_current_yscroll) & 0x1ff;
		const int biased = (y + m_current_yscroll - BACKGROUND_ROW_BIAS) & 0x1ff;

		if (wanted >= 256 && biased < 256)      // crosses into the scenery region
			return wanted;
		if (wanted < BACKGROUND_ROW_BIAS)       // goes round the top of the plane
			return wanted;
		return biased;
	};
	bitmap.fill(0, cliprect);

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		const int scrolly = row_read(y);
		uint16_t const *const source = &m_tmp_bitmap[LAYER_BG]->pix(scrolly);
		uint16_t *const dest = &bitmap.pix(y);

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			// the latched scroll, and the window wraps at the plane's edge: the road band lives at
			// scroll 500..511 and wants the other end of the plane
			int sourcex = x + m_scroll_x_latched;
			sourcex &= 0x1ff;
			dest[x] = source[sourcex];
		}
	}

	// the displayed half of the sprite plane is the bit the game declared on line 0 of the frame:
	// from line 1 on, Wheels & Fire's road writer reuses word 8 without it
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		const int source_row = y + m_page_at_line0 * 256;
		if (source_row < 0 || source_row >= 512) continue;
		uint16_t const *const src = &m_tmp_bitmap[LAYER_FG]->pix(source_row);
		uint16_t *const dst = &bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			if (src[x]) dst[x] = src[x];
	}
	return 0;
}

void wheelfir_state::report_exhausted_source()
{
	// destination steps that found no source inside the page, per frame
	if (m_source_exhausted)
	{
		LOGMASKED(LOG_SCALE, "scale: %d destination steps with no source inside the page (frame %d)\n",
				m_source_exhausted, int(m_screen->frame_number()));
		m_source_exhausted = 0;
	}
}


void wheelfir_state::scanline_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scanline_cnt);
}

void wheelfir_state::control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_eeprom->di_write(BIT(data, 9));
		m_eeprom->clk_write(BIT(data, 8));
		m_eeprom->cs_write(BIT(data, 10));

		// bit 13 enables the line counter; clearing it withdraws a pending level-5 request
		const bool was_enabled = m_counter_enabled;
		m_counter_enabled = BIT(data, 13);
		if (was_enabled && !m_counter_enabled)
			m_maincpu->set_input_line(5, CLEAR_LINE);
	}

	// the low byte is the link's data byte to a second cabinet, which is not modelled
}

uint16_t wheelfir_state::control_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	if (ACCESSING_BITS_8_15)
	{
		// bit 13 is the link receiver's status: at 1, nothing pending, a cabinet with no partner
		data |= 0x2000;
		data |= m_eeprom->do_read() << 15;
	}

	return data;
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

void wheelfir_state::board_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	// Kong Ball addresses its level-5 handler and a few variables at 0x21xxxx; a partial decode is
	// assumed rather than a second bank
	map(0x200000, 0x20ffff).ram().mirror(0x010000);

	map(0x700000, 0x70001f).w(FUNC(wheelfir_state::blit_w)).share(m_blitter_data);
	map(0x720001, 0x720001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x720003, 0x720003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x720005, 0x720005).w("ramdac", FUNC(ramdac_device::mask_w)); // word write?
	map(0x760000, 0x760001).w(FUNC(wheelfir_state::coin_cnt_w));
	map(0x780000, 0x78000f).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask16(0x00ff);
	map(0x7a0000, 0x7a0001).w(FUNC(wheelfir_state::scanline_cnt_w));
	map(0x7c0000, 0x7c0001).rw(FUNC(wheelfir_state::control_r), FUNC(wheelfir_state::control_w));
	map(0x7e0000, 0x7e0001).portr("P1");
	map(0x7e0002, 0x7e0003).portr("P2");
}

void wheelfir_state::main_map(address_map &map)
{
	board_main_map(map);
	map(0x740000, 0x740001).w("soundlatch", FUNC(generic_latch_16_device::write));
}


void wheelfir_state::board_sub_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram();

	map(0x700000, 0x700001).w("ldac", FUNC(dac_word_interface::data_w));
	map(0x740000, 0x740001).w("rdac", FUNC(dac_word_interface::data_w));
}

void wheelfir_state::sub_map(address_map &map)
{
	board_sub_map(map);
	map(0x780000, 0x780001).r("soundlatch", FUNC(generic_latch_16_device::read));
}


void wheelfir_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb888_w));
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
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(wheelfir_state::adc_eoc_r))
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
	// A field whose condition fails leaves its bits at zero, which on this active-low port reads as
	// all four directions held: the wheel cabinet needs the bits declared unused so they read idle.
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("CABINET", 0x01, EQUALS, 0x00)
	// The car does not accelerate past a low speed unless button 1 is held with the pedal, so on a
	// keyboard button 1 shares the accelerator's key; a key code replaces MAME's default, so
	// the pad's first button is listed too.
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(JOYCODE_BUTTON1)

	PORT_MODIFY("STEERING")
	// 12 per frame at 50.08 Hz is about 600 counts per second
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(12) PORT_NAME("Steering Wheel") PORT_REVERSE

	PORT_MODIFY("ACCELERATOR")
	// The game reads each pedal as a switch, pressed above 128, so a key takes a pedal to full travel
	// at once and a tap registers in the menus.
	PORT_BIT(0xff, 0x00, IPT_PEDAL)  PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(255) PORT_NAME("Accelerator Pedal") PORT_MINMAX(0x00, 0xff) PORT_REVERSE PORT_CODE_INC(KEYCODE_LSHIFT)

	PORT_MODIFY("BRAKE")
	PORT_BIT(0xff, 0x00, IPT_PEDAL2) PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(255) PORT_NAME("Brake Pedal") PORT_MINMAX(0x00, 0xff) PORT_REVERSE

	PORT_START("CABINET")
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_CONFSETTING( 0x00, "Steering wheel" )
	PORT_CONFSETTING( 0x01, "Joystick" )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(wheelfir_state::scanline_timer_callback)
{
	if (param == 0)
	{
		// the top of the frame starts from the last scroll the game wrote, not from word 8
		m_current_yscroll = frame_top_yscroll();
	}

	if (param < NUM_SCANLINES) // a scanned line
	{
		// the displayed half, sampled on line 0 (see screen_update)
		if (param == 0) m_page_at_line0 = (m_blitter_data[0x8] & VISIBLE_PAGE) ? 1 : 0;

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
		if (param == NUM_SCANLINES) /* vblank */
		{
			// a level-5 request still pending at vblank is withdrawn (see the TODO above)
			m_maincpu->set_input_line(5, CLEAR_LINE);
			m_maincpu->set_input_line(3, HOLD_LINE);
			report_exhausted_source();
		}
	}

	m_subcpu->set_input_line(1, HOLD_LINE);
}

void wheelfir_state::machine_start()
{
	m_zoom_table = std::make_unique<int32_t[]>(ZOOM_TABLE_SIZE);

	for (int i = 0; i < (ZOOM_TABLE_SIZE); ++i)
	{
		m_zoom_table[i] = -1;
	}

	for (int j = 0; j < 400; ++j)
		m_zoom_table[ZOOM_INDEX[j]] = j;

	// rom_w wraps inside the program with a mask, so the program has to be a power of two
	const uint32_t rom_words = uint32_t(m_maincpurom.length());
	assert(!(rom_words & (rom_words - 1)));
	m_rom_mask = rom_words - 1;
	// the source wraps inside the gfx region with a mask, so the region has to be a power of two
	const uint32_t gfx_bytes = uint32_t(m_tilepages.length());
	assert(!(gfx_bytes & (gfx_bytes - 1)));
	m_gfx_mask = gfx_bytes - 1;

	// Locate the program's two scale descriptor tables by their known answers: scale 100 is the
	// identity and 200 exactly double over 256 columns. A program without them keeps the
	// approximation (see the TODO).
	const uint32_t rom_bytes = rom_words * 2;
	int found = 0;
	for (uint32_t off = 0; off + 400 * 16 <= rom_bytes && found < 2; off += 2)
	{
		// Cheap filter before the expensive check: in the scale 100 descriptor both tables must be
		// the same, because only then can the mapping be the identity.
		const uint32_t d100 = off + 100 * 16;
		const uint32_t a = (uint32_t(rom_w(d100 + 8)) << 16) | rom_w(d100 + 10);
		const uint32_t b = (uint32_t(rom_w(d100 + 12)) << 16) | rom_w(d100 + 14);
		if (a != b || a == 0 || a >= rom_bytes || rom_w(d100 + 6) > 8)
			continue;

		const scale_law law100 = law_at(off, 100), law200 = law_at(off, 200);
		bool matches = true;
		for (int x = 0; x <= 255 && matches; ++x)
			matches = (scale_at(law100, x) == x) && (scale_at(law200, x) == 2 * x);
		if (matches)
			m_scale_desc[found++] = int32_t(off);
	}

	// The horizontal table must also decode to ZOOM_INDEX[], which ties the two together.
	bool table_matches = (found == 2);
	for (int j = 0; j < 400 && table_matches; ++j)
	{
		const uint32_t d = uint32_t(m_scale_desc[0]) + uint32_t(j) * 16;
		const uint16_t w0 = rom_w(d), w1 = rom_w(d + 2), w2 = rom_w(d + 4);
		int repeat = w0 & 0x1f;
		int stall = (w0 >> 8) & 0x1f;
		stall |= (w1 & 1) ? 0x20 : 0;
		repeat |= (w1 & 4) ? 0x20 : 0;
		const int idx = stall | (repeat << 6) | ((w2 & 0x100) ? 0x1000 : 0) | ((w1 & 0x10) ? 0x2000 : 0);
		table_matches = (idx == ZOOM_INDEX[j]);
	}

	m_board_scale = table_matches;

	// once per start: which scale law this machine runs (Power Ball's program carries no descriptors,
	// and none of its blits was seen scaled)
	if (m_board_scale)
		logerror("scale: the board's law, descriptor tables at x=%06x y=%06x\n", m_scale_desc[0], m_scale_desc[1]);
	else if (found == 2)
		logerror("scale: descriptor tables at x=%06x y=%06x do not decode to ZOOM_INDEX; approximating\n", m_scale_desc[0], m_scale_desc[1]);
	else
		logerror("scale: no descriptor tables in this program; approximating\n");

	// What influences future behaviour is saved; what startup reads out of the ROM is not, or a
	// loaded state would clobber what this machine found.
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
// level-1 handler to find an empty list before it fires its first blit.
void wheelfir_state::machine_reset()
{
	m_maincpu->set_input_line(1, ASSERT_LINE);
	m_counter_enabled = false;
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

	adc0808_device& adc(ADC0808(config, "adc", 500000)); // unknown clock
	adc.eoc_ff_callback().set(FUNC(wheelfir_state::adc_eoc_w));
	adc.in_callback<0>().set_ioport("STEERING");
	adc.in_callback<1>().set_ioport("ACCELERATOR");
	adc.in_callback<2>().set_ioport("BRAKE");

	TIMER(config, "scan_timer").configure_scanline(FUNC(wheelfir_state::scanline_timer_callback), "screen", 0, 1);

	SCREEN(config, m_screen);
	// 32 MHz / 4 over 512 pixels is the 15625 Hz line rate the sound mixer counts against, and 312
	// lines give 50.08 Hz. No register is known to move the level-3 interrupt, which comes after
	// the 256 scanned lines; the monitor shows the first 252 of them.
	m_screen->set_raw(MASTER_CLOCK / 4, PIXELS_PER_LINE, 0, 336,
			NUM_SCANLINES + NUM_VBLANK_LINES, FIRST_VISIBLE_LINE, VISIBLE_LINES);

	m_screen->set_screen_update(FUNC(wheelfir_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_wheelfir);

	PALETTE(config, m_palette).set_entries(NUM_COLORS);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", m_palette));
	ramdac.set_addrmap(0, &wheelfir_state::ramdac_map);

	EEPROM_93C46_16BIT(config, "eeprom");

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();
	DAC_10BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 0); // unknown DAC
	DAC_10BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 1); // unknown DAC
}

void wheelfir_state::wheelfir(machine_config &config)
{
	board(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &wheelfir_state::main_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &wheelfir_state::sub_map);

	GENERIC_LATCH_16(config, "soundlatch");
}


void wheelfir_state::kongball(machine_config& config)
{
	wheelfir(config);
	m_subcpu->set_disable(); // sound ROMS were not present, so don't run sound CPU
}


// Radical Enduro, on the same board. Its game issues the drawing chain inside vblank, so the
// sprite plane's displayed half is decided on the blit rather than on line 0, and the palette the
// game uploads for a page takes effect when that page is shown. Each frame starts from the scroll
// the registers hold, and the X scroll is read live. Its two CPUs talk through a plain register.
class radendur_state : public wheelfir_state
{
public:
	radendur_state(const machine_config &mconfig, device_type type, const char *tag) :
		wheelfir_state(mconfig, type, tag),
		m_ramdac(*this, "ramdac")
	{ }

	void radendur(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void sprite_plane_blit() override;
	virtual int32_t frame_top_yscroll() const override;

private:
	required_device<ramdac_device> m_ramdac;

	// The FG is two pages of 256 rows that page-flip: m_fg_drawbuf is the half being drawn and
	// m_fg_dispbuf the finished half screen_update shows. Which one is bit 0x20 of word 8.
	int32_t m_fg_drawbuf = 0;
	int32_t m_fg_dispbuf = 0;

	// One 256-colour palette drives both planes and the game uploads one per sprite page, so the
	// sprite plane's pens are taken at the flip: pens uploaded while word 7 names the sprite plane
	// accumulate in m_pal_fg_cur, become m_pal_fg_shown when their page flips in, and are applied
	// at vblank; the background's pens stay live.
	bool m_pal_fg_primed = false;                    // a flip has snapshotted a palette to apply
	bool m_pal_fg_owned[NUM_COLORS] = { false };     // index last written in FG (flip-synced) vs BG (live) context
	uint32_t m_pal_fg_cur[NUM_COLORS] = { 0 };       // FG palette accumulating live from the RAMDAC writes
	uint32_t m_pal_fg_shown[NUM_COLORS] = { 0 };     // cur snapshotted at the flip = palette of the displayed page
	uint8_t m_rd_idx = 0;
	uint8_t m_rd_sub = 0;
	uint8_t m_rd_comp[3] = { 0 };
	uint16_t m_latch_value = 0;

	void flip_page(int buf);
	void ramdac_index_w(uint8_t data);
	void ramdac_pal_w(uint8_t data);
	void latch_w(uint16_t data);
	uint16_t latch_r();
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void radendur_main_map(address_map &map) ATTR_COLD;
	void radendur_sub_map(address_map &map) ATTR_COLD;
};

// The page flip, decided on the blit: this game issues its drawing chain inside vblank, after
// line 0. The half coming in is cleared by the game's own fill.
void radendur_state::flip_page(int buf)
{
	if (buf == m_fg_drawbuf) return;
	m_fg_dispbuf = m_fg_drawbuf;
	m_fg_drawbuf = buf;
	std::memcpy(m_pal_fg_shown, m_pal_fg_cur, sizeof(m_pal_fg_shown));
	m_pal_fg_primed = true;
}

// The displayed half is bit 0x20 of word 8, composed once per frame by the vblank handler (0x506);
// with the bit set the blits draw into the low half, shown from the next flip.
void radendur_state::sprite_plane_blit()
{
	flip_page((m_blitter_data[0x8] & VISIBLE_PAGE) ? 0 : 1);
}

// the frame starts from the scroll the game's registers hold, bit 8 from word 8
int32_t radendur_state::frame_top_yscroll() const
{
	return (m_blitter_data[0xb] & 0x00ff) | (m_blitter_data[0x8] & SCROLL_Y_BIT8) << 1;
}

uint32_t radendur_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{

	bitmap.fill(0, cliprect);

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		int scrolly = y;
		scrolly += m_current_yscroll;
		scrolly &= 0x1ff;
		uint16_t const *const source = &m_tmp_bitmap[LAYER_BG]->pix(scrolly);
		uint16_t *const dest = &bitmap.pix(y);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int sourcex = x;
			sourcex += (m_blitter_data[0xa] & 0x00ff) | (m_blitter_data[0x8] & SCROLL_X_BIT8) << 2;
			sourcex &= 0x1ff;
			dest[x] = source[sourcex];
		}
	}


	// The FG is composited unscrolled: given the BG scroll, it would move the attract logos with it.
	const int displayed_page = m_fg_dispbuf;
	copybitmap_trans(bitmap, *m_tmp_bitmap[LAYER_FG], 0, 0, 0, -displayed_page * 256, cliprect, 0);

	return 0;
}

void radendur_state::screen_vblank(int state)
{
	// nothing on the rising edge
	if (!state)
	{
		// Falling edge, after this vblank's RAMDAC upload: push the palette snapshotted at the flip
		// for the FG-owned indices. Earlier, the live upload overwrites it.
		if (m_pal_fg_primed)
			for (int i = 0; i < NUM_COLORS; i++)
				if (m_pal_fg_owned[i])
					m_palette->set_pen_color(i, rgb_t(uint8_t(m_pal_fg_shown[i] >> 16), uint8_t(m_pal_fg_shown[i] >> 8), uint8_t(m_pal_fg_shown[i])));
	}
}

// The latch between the two CPUs: one word, no queue, so a second write before the read drops the
// first, and the sub CPU patches each word into its mix loop, so a shifted stream corrupts its code.
// TODO: modelled as a plain register; with generic_latch_16_device, whose write synchronises the
// scheduler, the sub CPU loses its stream partway into a race. How the board's latch keeps the two
// CPUs in step is not known.
void radendur_state::latch_w(uint16_t data)
{
	m_latch_value = data;
}

uint16_t radendur_state::latch_r()
{
	return m_latch_value;
}

// The RAMDAC writes are passed on and followed by the BT478 protocol (index, then R, G, B with
// auto-increment) to route each pen by the plane context of its upload; see the palette members.
void radendur_state::ramdac_index_w(uint8_t data)
{
	m_ramdac->index_w(data);
	m_rd_idx = data;
	m_rd_sub = 0;
}

void radendur_state::ramdac_pal_w(uint8_t data)
{
	m_ramdac->pal_w(data);
	m_rd_comp[m_rd_sub] = data;
	if (++m_rd_sub == 3) // R, G, B collected -> one pen
	{
		if (m_blitter_data[0x7] & DST_PLANE_BG)
		{
			m_pal_fg_owned[m_rd_idx] = false;
		}
		else
		{
			m_pal_fg_cur[m_rd_idx] = (m_rd_comp[0] << 16) | (m_rd_comp[1] << 8) | m_rd_comp[2];   // 0x00RRGGBB
			m_pal_fg_owned[m_rd_idx] = true;
		}
		m_rd_idx = (m_rd_idx + 1) & 0xff;   // BT478 auto-increments the index
		m_rd_sub = 0;
	}
}

void radendur_state::machine_start()
{
	wheelfir_state::machine_start();

	save_item(NAME(m_fg_drawbuf));
	save_item(NAME(m_fg_dispbuf));
	save_item(NAME(m_pal_fg_primed));
	save_item(NAME(m_pal_fg_owned));
	save_item(NAME(m_pal_fg_cur));
	save_item(NAME(m_pal_fg_shown));
	save_item(NAME(m_rd_idx));
	save_item(NAME(m_rd_sub));
	save_item(NAME(m_rd_comp));
	save_item(NAME(m_latch_value));
}

void radendur_state::radendur_main_map(address_map &map)
{
	board_main_map(map);
	map(0x720001, 0x720001).w(FUNC(radendur_state::ramdac_index_w));
	map(0x720003, 0x720003).w(FUNC(radendur_state::ramdac_pal_w));
	map(0x740000, 0x740001).w(FUNC(radendur_state::latch_w));
}

void radendur_state::radendur_sub_map(address_map &map)
{
	board_sub_map(map);
	map(0x780000, 0x780001).r(FUNC(radendur_state::latch_r));
}

void radendur_state::radendur(machine_config &config)
{
	board(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &radendur_state::radendur_main_map);
	m_subcpu->set_addrmap(AS_PROGRAM, &radendur_state::radendur_sub_map);

	m_screen->set_screen_update(FUNC(radendur_state::screen_update));
	m_screen->screen_vblank().set(FUNC(radendur_state::screen_vblank));
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

GAME( 199?, wheelfir,    0, wheelfir,    wheelfir, wheelfir_state, empty_init,    ROT0,  "TCH", "Wheels & Fire", MACHINE_SUPPORTS_SAVE )
GAME( 199?, pwball,      0, wheelfir,    pwball,   wheelfir_state, empty_init,    ROT0,  "TCH", "Power Ball (prototype)",    MACHINE_SUPPORTS_SAVE )

// sound ROMs were missing on PCB, so sound emulation is not possible at this point in time
GAME( 1997, kongball,    0, kongball,    pwball,   wheelfir_state, empty_init,    ROT0,  "TCH / Digital Dreams Multimedia", "Kong Ball (prototype)",    MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

// crashes (always?) when selecting track on PCB
GAME( 199?, radendur,    0,        radendur, pwball,   radendur_state, empty_init, ROT0, "TCH / Sator Videogames", "Radical Enduro (early prototype)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
