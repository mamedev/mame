// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen, Morten Shearman Kirkegaard
/**********************************************************************

    Axell AG-1 sprite / graphics display controller family

    Axell Corporation is a Japanese company that specialises in sound
    and graphics LSIs for the amusement industry (pachinko / pachislot,
    medal games).  The AG-1 line are sprite ("character") based display
    controllers with a double frame buffer, an integrated blitter with
    a Huffman/RLE decompressor for the character data, scaling,
    rotation, alpha blending and per-sheet intensity modulation.

    AX51101 (Gunpey, 2000)
    - dedicated high-speed Z80 interface (8-bit registers, connected
      to the V30 through a CPLD on Gunpey)
    - built-in RGB DAC, up to 15M dots/s, DRAM CGRAM
    - 64 registers; Gunpey maps them at I/O 0x7fc0-0x7fff

    AX51102 (Taito Hello Kitty Koi no Shugoshin Uranai / Dinoking Kids
    / Dinoking Battle, 2003-2005)
    - "various CPU" 16-bit host interface, 35M dots/s, SDRAM CGRAM
    - 127 built-in sprite control registers (2KB) which can be extended
      with external sprite control SRAM
    - 256 byte register window; the H8S on the Dinoking boards maps it
      at 0x61fc00-0x61fcff and the sprite control SRAM at 0x600000,
      hkuranai puts the chip on an 8-bit bus (see taito/dinoking.cpp)

    Memory organisation (both chips):
    - CG-ROM: linear, 2048 bytes per row (x = 11 bits, y = rest)
    - CGRAM: linear, 2048 x 2048 bytes (AX51101) or 4096 x 4096 bytes
      (AX51102), holding the character data fetched by the blitter, the
      palette and (AX51102) the frame buffers and any other render
      targets.  The palette is 256 sets of 256 RGB555 words at
      (colour * 2, set); bit 15 set marks a transparent entry.
    - sprite control memory: 64KB shared with the host, holding the
      sheet (sprite) table, the cell lists and (AX51102) the order
      list.  Pointer registers select where in that space the tables
      are.

    Rendering model:
    - AX51101: the sheet table is walked in order at display time and
      the cells are drawn straight to the screen (the frame buffers
      are not emulated, Gunpey draws complete frames).
    - AX51102: a display list command renders the part of the virtual
      screen selected by the window registers into CGRAM at the
      destination given by two more registers, which the games point at
      the back frame buffer - or at any other place when they want to
      render into a texture (knock-out sprites in dkkids, the whole
      title canvas in hkuranai).  Several passes with different
      windows make up a frame; nothing is cleared by the chip except
      the new back buffer when the displayed buffer is switched.  The
      displayed buffer is scanned out at the start of vertical blanking.

    Sheet entry, AX51101 (16 bytes):
      +0  bit 7 = end of table, bit 0 = sheet enabled
      +2  modulation (see below)
      +6  sheet x position (big-endian 16-bit)
      +8  sheet y position (high byte)
      +9  cell list offset low byte, +10 high byte (added to the cell
          list base pointer register)

    Sheet entry, AX51102 (12 bytes, big-endian fields):
      +0  bit 0 = sheet enabled (bits 3, 4, 5 and 7 are set by the
          games in fixed patterns, meaning unknown)
      +1  ?
      +2  modulation (see below)
      +3  ? (stale data)
      +4  horizontal scale, +5 vertical scale in 1/64 (0x40 = 1:1),
          applied about the sheet position to the cell offsets and
          sizes
      +6  sheet x position, +8 sheet y position
      +10 cell list offset

    Modulation byte: bits 0-4 are a 32 step level.  With bit 6 set the
    level is subtracted from the RGB components of every pixel of the
    sheet (intensity modulation; Gunpey fades its screens this way and
    uses 0x80 / 0xc0 for sheets drawn normally).  With bit 6 clear the
    level makes the whole sheet translucent (dkkids fades hit effects
    out with it, hkuranai twinkles the sparkles on its title screen).
    Bit 7 set disables both (dkkids' normal sheets use 0x80), bit 5
    toggles with the frame parity on some dkkids sheets and has no
    visible effect.

    Screen position of a cell = sheet position + cell position - display
    window origin (registers 0x24/0x26 on AX51101, 0xc4/0xc6 on AX51102).

    TODO:
    - 6bpp cells (not used by the supported games)
    - drawing time: the lists are rendered instantly, the real chips
      draw at up to 15M / 35M dots/s
    - AX51101: frame buffers, rotation quadrant location (Gunpey uses
      neither), CRTC registers (the display timings are configured by
      the driver)
    - AX51102: the meaning of most of the configuration registers at
      0xa0-0xbf and 0xe0-0xf1, register a6 (hkuranai writes 0xff to it
      from its frame interrupt handler), command bit 7 (set by the
      Dinoking games only), what exactly triggers interrupt 0x20 (used
      as the frame interrupt by hkuranai), the cell record unknowns

**********************************************************************/

#include "emu.h"

#include "axell_ag1.h"

#include "screen.h"

#include <algorithm>
#include <cstring>

#define LOG_BLIT    (1U << 1)
#define LOG_REGS    (1U << 2)
#define LOG_CMD     (1U << 3)
#define LOG_LIST    (1U << 4)

//#define VERBOSE (LOG_GENERAL | LOG_CMD | LOG_BLIT | LOG_REGS)
#include "logmacro.h"

#define LOGBLIT(...) LOGMASKED(LOG_BLIT, __VA_ARGS__)
#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)
#define LOGCMD(...)  LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGLIST(...) LOGMASKED(LOG_LIST, __VA_ARGS__)


DEFINE_DEVICE_TYPE(AX51101, ax51101_device, "ax51101", "Axell AG-1 AX51101 graphics display controller")
DEFINE_DEVICE_TYPE(AX51102, ax51102_device, "ax51102", "Axell AG-1 AX51102 graphics display controller")


//**************************************************************************
//  DECOMPRESSOR
//**************************************************************************

namespace {

u8 cgrom_read(const u8 *rom, u32 mask, u32 x, u32 y)
{
	return rom[((x & 0x7ff) | (y << 11)) & mask];
}


struct huffman_node_s;

struct state_s
{
	const u8 *rom;
	u32 rom_mask;
	u8 *buf;
	u32 xbits;
	u32 xmask;
	u32 ymask;
	u8 bits;
	int num_bits;
	u8 colour[16];
	int basex;
	int ix, iy, iw;
	int dx, dy;
	int ow, oh;
	int ox, oy;

	u32 addr(int x, int y) const { return (x & xmask) | ((y & ymask) << xbits); }

	int get_next_bit();
	bool next_node(const huffman_node_s **res);

	void set_o(u8 v);
	u8 get_o(int x, int y) const;

	static void hn_bytes_new_colour(state_s &s, u8 v, int n);
	static void hn_bytes_prev_colour(state_s &s, u8 v, int n);
	static void hn_copy_directly(state_s &s, u8 v, int n);
	static void hn_copy_plus_one(state_s &s, u8 v, int n);
	static void hn_copy_minus_one(state_s &s, u8 v, int n);
};


struct huffman_node_s
{
	const char *bits;
	void (*func)(state_s &, u8, int);
	int arg0_bits;
	int arg1_val;
};


constexpr huffman_node_s HUFFMAN_NODES[] = {
	{ "11",                 state_s::hn_bytes_new_colour,   8,  1 },
	{ "10111",              state_s::hn_bytes_new_colour,   8,  2 },
	{ "10110011",           state_s::hn_bytes_new_colour,   8,  3 },
	{ "1010011000",         state_s::hn_bytes_new_colour,   8,  4 },
	{ "1010101010000",      state_s::hn_bytes_new_colour,   8,  5 },
	{ "101010001011110",    state_s::hn_bytes_new_colour,   8,  6 },
	{ "101010010101110",    state_s::hn_bytes_new_colour,   8,  7 },
	{ "101010010101111",    state_s::hn_bytes_new_colour,   8,  8 },
	{ "101010010101100",    state_s::hn_bytes_new_colour,   8,  9 },
	{ "10101000101100",     state_s::hn_bytes_new_colour,   8, 10 },
	{ "101010010101101",    state_s::hn_bytes_new_colour,   8, 11 },
	{ "10101000101101",     state_s::hn_bytes_new_colour,   8, 12 },
	{ "0",                  state_s::hn_bytes_prev_colour,  4,  1 },
	{ "100",                state_s::hn_bytes_prev_colour,  4,  2 },
	{ "101011",             state_s::hn_bytes_prev_colour,  4,  3 },
	{ "1010010",            state_s::hn_bytes_prev_colour,  4,  4 },
	{ "101010100",          state_s::hn_bytes_prev_colour,  4,  5 },
	{ "1010100100",         state_s::hn_bytes_prev_colour,  4,  6 },
	{ "10101010110",        state_s::hn_bytes_prev_colour,  4,  7 },
	{ "10100111000",        state_s::hn_bytes_prev_colour,  4,  8 },
	{ "101010101001",       state_s::hn_bytes_prev_colour,  4,  9 },
	{ "101001111000",       state_s::hn_bytes_prev_colour,  4, 10 },
	{ "101010010100",       state_s::hn_bytes_prev_colour,  4, 11 },
	{ "1010011001",         state_s::hn_bytes_prev_colour,  4, 12 },
	{ "101101",             state_s::hn_copy_directly,      0,  2 },
	{ "10101011",           state_s::hn_copy_directly,      0,  3 },
	{ "101010011",          state_s::hn_copy_directly,      0,  4 },
	{ "101001101",          state_s::hn_copy_directly,      0,  5 },
	{ "1010011111",         state_s::hn_copy_directly,      0,  6 },
	{ "1010100011",         state_s::hn_copy_directly,      0,  7 },
	{ "10101000100",        state_s::hn_copy_directly,      0,  8 },
	{ "101010101111",       state_s::hn_copy_directly,      0,  9 },
	{ "101001110010",       state_s::hn_copy_directly,      0, 10 },
	{ "1010011100111",      state_s::hn_copy_directly,      0, 11 },
	{ "101100101",          state_s::hn_copy_directly,      0, 12 },
	{ "1011000",            state_s::hn_copy_plus_one,      0,  2 },
	{ "101010000",          state_s::hn_copy_plus_one,      0,  3 },
	{ "10101001011",        state_s::hn_copy_plus_one,      0,  4 },
	{ "101010001010",       state_s::hn_copy_plus_one,      0,  5 },
	{ "1010101011101",      state_s::hn_copy_plus_one,      0,  6 },
	{ "1010101011100",      state_s::hn_copy_plus_one,      0,  7 },
	{ "1010011110010",      state_s::hn_copy_plus_one,      0,  8 },
	{ "10101000101110",     state_s::hn_copy_plus_one,      0,  9 },
	{ "101010001011111",    state_s::hn_copy_plus_one,      0, 10 },
	{ "1010011110011",      state_s::hn_copy_plus_one,      0, 11 },
	{ "101000",             state_s::hn_copy_minus_one,     0,  2 },
	{ "101100100",          state_s::hn_copy_minus_one,     0,  3 },
	{ "1010011101",         state_s::hn_copy_minus_one,     0,  4 },
	{ "10101010101",        state_s::hn_copy_minus_one,     0,  5 },
	{ "101001111011",       state_s::hn_copy_minus_one,     0,  6 },
	{ "101001111010",       state_s::hn_copy_minus_one,     0,  7 },
	{ "1010101010001",      state_s::hn_copy_minus_one,     0,  8 },
	{ "1010011100110",      state_s::hn_copy_minus_one,     0,  9 },
	{ "10101001010101",     state_s::hn_copy_minus_one,     0, 10 },
	{ "10101001010100",     state_s::hn_copy_minus_one,     0, 11 },
	{ nullptr,              nullptr,                        0,  0 },
};


inline int state_s::get_next_bit()
{
	if (num_bits == 0)
	{
		if (ix >= basex + iw)
		{
			ix = basex;
			iy++;
		}
		bits = cgrom_read(rom, rom_mask, ix, iy);
		ix++;
		num_bits = 8;
	}

	const int res = 1 & bits;

	bits >>= 1;
	num_bits--;

	return res;
}


bool state_s::next_node(const huffman_node_s **res)
{
	char code[128];

	std::fill(std::begin(code), std::end(code), 0);

	for (int i = 0; i < sizeof(code) - 1; i++)
	{
		code[i] = '0' + get_next_bit();

		for (int j = 0;; j++)
		{
			if (!HUFFMAN_NODES[j].bits)
				return true;

			if (strncmp(code, HUFFMAN_NODES[j].bits, i + 1) == 0)
			{
				if (!HUFFMAN_NODES[j].bits[i + 1])
				{
					*res = &HUFFMAN_NODES[j];
					return false;
				}
				break;
			}
		}
	}

	return true;
}


void state_s::set_o(u8 v)
{
	u8 a = v;
	for (int i = 0; i < std::size(colour); i++)
	{
		u8 b = colour[i];
		colour[i] = a;
		a = b;
		if (a == v)
			break;
	}

	if (ox >= 0 && ox < ow && oy >= 0 && oy < oh)
		buf[addr(dx + ox, dy + oy)] = v;
	ox++;
}


u8 state_s::get_o(int x, int y) const
{
	if (x < 0 || x >= ow || y < 0 || y >= oh)
		return 0;

	return buf[addr(dx + x, dy + y)];
}


void state_s::hn_bytes_new_colour(state_s &s, u8 v, int n)
{
	for (int i = 0; i < n; i++)
		s.set_o(v);
}


void state_s::hn_bytes_prev_colour(state_s &s, u8 v, int n)
{
	const u8 c = s.colour[v];

	for (int i = 0; i < n; i++)
		s.set_o(c);
}


void state_s::hn_copy_directly(state_s &s, u8 v, int n)
{
	for (int i = 0; i < n; i++)
	{
		if ((s.ox / 12) & 1)
			s.set_o(s.get_o(s.ox, s.oy + 1));
		else
			s.set_o(s.get_o(s.ox, s.oy - 1));
	}
}


void state_s::hn_copy_plus_one(state_s &s, u8 v, int n)
{
	for (int i = 0; i < n; i++)
	{
		if ((s.ox / 12) & 1)
			s.set_o(s.get_o(s.ox + 1, s.oy + 1));
		else
			s.set_o(s.get_o(s.ox + 1, s.oy - 1));
	}
}


void state_s::hn_copy_minus_one(state_s &s, u8 v, int n)
{
	for (int i = 0; i < n; i++)
	{
		if ((s.ox / 12) & 1)
			s.set_o(s.get_o(s.ox - 1, s.oy + 1));
		else
			s.set_o(s.get_o(s.ox - 1, s.oy - 1));
	}
}

} // anonymous namespace


//**************************************************************************
//  BASE DEVICE
//**************************************************************************

axell_ag1_device_base::axell_ag1_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, endianness_t host_endianness, address_map_constructor list_map, u32 cgram_x_bits, u32 cgram_y_bits, u8 cgram_fill)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_irq_cb(*this)
	, m_irq_lines(0)
	, m_cgrom(*this, finder_base::DUMMY_TAG)
	, m_list_config("list", host_endianness, 16, 16, 0, list_map)
	, m_cgram_config("cgram", ENDIANNESS_LITTLE, 8, cgram_x_bits + cgram_y_bits, 0)
	, m_list(nullptr)
	, m_cgram_x_bits(cgram_x_bits)
	, m_cgram_x_mask((1U << cgram_x_bits) - 1)
	, m_cgram_y_mask((1U << cgram_y_bits) - 1)
	, m_cgram_fill(cgram_fill)
	, m_irq_timer{ nullptr, nullptr }
{
}


device_memory_interface::space_config_vector axell_ag1_device_base::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_list_config),
		std::make_pair(1, &m_cgram_config)
	};
}


void axell_ag1_device_base::device_start()
{
	// the CG-ROM size is used as an address mask
	if (m_cgrom.length() & (m_cgrom.length() - 1))
		fatalerror("%s: CG-ROM size must be a power of two\n", tag());

	m_list = &space(0);

	const u32 cgram_size = (m_cgram_x_mask + 1) * (m_cgram_y_mask + 1);
	m_cgram = std::make_unique<u8[]>(cgram_size);
	std::fill_n(&m_cgram[0], cgram_size, m_cgram_fill);

	// expose the CGRAM as a memory space so it can be inspected with the debugger
	space(1).install_ram(0, cgram_size - 1, &m_cgram[0]);

	// RGB555 to RGB32 conversion table
	m_rgb555_lut = std::make_unique<u32[]>(0x8000);
	for (u32 i = 0; i < 0x8000; i++)
		m_rgb555_lut[i] = rgb_t(pal5bit(i >> 10), pal5bit(i >> 5), pal5bit(i));

	for (auto &timer : m_irq_timer)
		timer = timer_alloc(FUNC(axell_ag1_device_base::irq_timer), this);

	screen().register_vblank_callback(vblank_state_delegate(&axell_ag1_device_base::vblank, this));

	save_item(NAME(m_regs));
	save_item(NAME(m_irq_cause));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_irq_lines));
	save_pointer(NAME(m_cgram), cgram_size);
}


void axell_ag1_device_base::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	for (auto &timer : m_irq_timer)
		timer->adjust(attotime::never);
	m_irq_cause = 0;
	m_irq_mask = 0;
	update_irq();
}


//-------------------------------------------------
//  interrupts
//-------------------------------------------------

void axell_ag1_device_base::raise_irq(u8 causes)
{
	m_irq_cause |= causes;
	update_irq();
}


void axell_ag1_device_base::ack_irq(u8 causes)
{
	m_irq_cause &= ~causes;
	update_irq();
}


void axell_ag1_device_base::set_irq_mask(u8 mask)
{
	m_irq_mask = mask;
	update_irq();
}


void axell_ag1_device_base::schedule_irq(u8 cause, const attotime &delay)
{
	// one timer per interrupt source (blitter, list transfer)
	const int index = (cause == IRQ_BLITTER) ? 0 : 1;
	m_irq_timer[index]->adjust(delay, cause);
}


TIMER_CALLBACK_MEMBER(axell_ag1_device_base::irq_timer)
{
	raise_irq(u8(param));
}


u8 axell_ag1_device_base::get_cgrom_byte(u32 x, u32 y) const
{
	return cgrom_read(&m_cgrom[0], m_cgrom.length() - 1, x, y);
}


//-------------------------------------------------
//  blitter - copies (optionally compressed)
//  character data from CG-ROM to CGRAM
//-------------------------------------------------

void axell_ag1_device_base::start_blit(u16 src_x, u16 src_y, u16 dst_x, u16 dst_y, u16 xsize, u16 ysize, u8 mode, u8 irq_on_completion)
{
	// horizontal destination position and size are in 16-bit words
	const u32 dstx = u32(dst_x) << 1;
	const u32 width = (u32(xsize) + 1) << 1;
	const u32 height = u32(ysize) + 1;

	LOGBLIT("blit mode %02x src (%04x,%04x) dst (%04x,%04x) size %u x %u\n", mode, src_x, src_y, dstx, dst_y, width, height);

	switch (mode)
	{
	case 0: // uncompressed copy from CG-ROM
		for (u32 y = 0; y < height; y++)
			for (u32 x = 0; x < width; x++)
				m_cgram[cgram_addr(dstx + x, dst_y + y)] = get_cgrom_byte(src_x + x, src_y + y);
		break;

	case 3: // copy from the sprite control memory (linear source, used by hkuranai to upload host generated image data)
		for (u32 y = 0; y < height; y++)
			for (u32 x = 0; x < width; x++)
				m_cgram[cgram_addr(dstx + x, dst_y + y)] = list_read_byte((src_x + y * width + x) & 0xffff);
		break;

	case 8: // Huffman/RLE compressed data from CG-ROM
		if (decompress_sprite(src_x, src_y, width, height, dstx, dst_y))
			logerror("Failed to decompress sprite at %04x %04x\n", src_x, src_y);
		break;

	default:
		logerror("Unknown blitter mode %02x\n", mode);
		break;
	}

	// roughly one byte every four clocks
	schedule_irq(irq_on_completion, clocks_to_attotime(u64(width) * height * 4));
}


bool axell_ag1_device_base::decompress_sprite(int ix, int iy, int ow, int oh, int dx, int dy)
{
	state_s s;

	s.rom = &m_cgrom[0];
	s.rom_mask = m_cgrom.length() - 1;
	s.buf = m_cgram.get();
	s.xbits = m_cgram_x_bits;
	s.xmask = m_cgram_x_mask;
	s.ymask = m_cgram_y_mask;
	s.num_bits = 0;

	std::fill(std::begin(s.colour), std::end(s.colour), 0);

	s.basex = ix;

	s.ix = ix;
	s.iy = iy;
	s.iw = get_cgrom_byte(s.ix++, s.iy) + 1;
	s.dx = dx;
	s.dy = dy;

	s.ow = ow;
	s.oh = oh;
	s.ox = 0;
	s.oy = 0;

	// the first byte of the data gives the width of the compressed block
	// in the CG-ROM minus one, the first three bits of the bit stream hold
	// bits 8-10 of it (large pictures in the Taito games)
	int temp = s.get_next_bit();
	temp |= s.get_next_bit() << 1;
	temp |= s.get_next_bit() << 2;
	s.iw += temp << 8;

	// the output is produced in vertical strips of 12 pixels, going
	// down in even strips and up in odd ones
	while (s.ox < s.ow)
	{
		const huffman_node_s *n;
		if (s.next_node(&n))
			return true;

		u8 v = 0;
		for (int i = 0; i < n->arg0_bits; i++)
			v |= s.get_next_bit() << i;

		n->func(s, v, n->arg1_val);

		bool eol;
		if ((s.ox % 12) == 0)
		{
			s.ox -= 12;
			if ((s.ox / 12) & 1)
				s.oy -= 1;
			else
				s.oy += 1;

			eol = true;
		}
		else if (s.ox == s.ow)
		{
			s.ox -= s.ow % 12;
			if ((s.ox / 12) & 1)
				s.oy -= 1;
			else
				s.oy += 1;

			eol = true;
		}
		else
		{
			eol = false;
		}

		if (eol)
		{
			if ((s.oy == s.oh) || (s.oy == -1))
			{
				s.ox += 12;
				if ((s.ox / 12) & 1)
					s.oy -= 1;
				else
					s.oy += 1;
			}
		}
	}

	return false;
}


//-------------------------------------------------
//  drawing
//-------------------------------------------------

// pixel sink drawing straight into a screen bitmap (AX51101: the frame
// buffers are not emulated, the display list is rendered at screen update
// time)
struct axell_ag1_device_base::bitmap_target
{
	bitmap_rgb32 &bitmap;
	const rectangle &clip;
	const u32 *lut;

	bool contains(int x, int y) const { return clip.contains(x, y); }
	u16 read(int x, int y) const
	{
		const rgb_t old(bitmap.pix(y, x));
		return ((old.r() >> 3) << 10) | ((old.g() >> 3) << 5) | (old.b() >> 3);
	}
	void write(int x, int y, u16 color) { bitmap.pix(y, x) = lut[color & 0x7fff]; }
};

// pixel sink rendering into an RGB555 frame buffer in CGRAM (AX51102)
struct axell_ag1_device_base::cgram_target
{
	axell_ag1_device_base &dev;
	rectangle clip;    // in virtual screen coordinates
	s32 offset_x;      // destination - window origin
	s32 offset_y;

	bool contains(int x, int y) const { return clip.contains(x, y); }
	u16 read(int x, int y) const { return dev.cgram_read16(x + offset_x, y + offset_y); }
	void write(int x, int y, u16 color) { dev.cgram_write16(x + offset_x, y + offset_y, color); }
};


u32 axell_ag1_device_base::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	draw_display_list(bitmap, cliprect);
	return 0;
}


template <typename Target>
inline void axell_ag1_device_base::draw_pixel(Target &target, int x, int y, u16 color_data, int alpha, int darken)
{
	// bit 15 set means transparent
	if (color_data & 0x8000)
		return;

	// per-sheet intensity modulation: subtract from each component
	if (darken)
	{
		const s16 r = std::max(0, BIT(color_data, 10, 5) - darken);
		const s16 g = std::max(0, BIT(color_data,  5, 5) - darken);
		const s16 b = std::max(0, BIT(color_data,  0, 5) - darken);

		color_data = (r << 10) | (g << 5) | (b << 0);
	}

	if (target.contains(x, y))
	{
		if (alpha != 0x00) // a value of 0x00 is solid
		{
			// blend in RGB555 space like the hardware does
			color_data = alpha_blend_r16(color_data, target.read(x, y), alpha) & 0x7fff;
		}

		target.write(x, y, color_data);
	}
}


/*
    Draw one cell.  The source position is stepped through CGRAM with
    fixed point vectors derived from the four magnitude bytes and the
    rotation quadrant of the cell record, which covers plain scaling
    (the common case: no sine components) as well as rotation and the
    rotated + scaled sprites used for knocked-out characters.  The
    source position is a bit address horizontally and has two
    fractional bits vertically, so the sub-pixel start positions of
    rotated cells are honoured.
*/
template <typename Target>
void axell_ag1_device_base::draw_cell(Target &target, const cell_params &cell, s32 x, s32 y, int darken)
{
	constexpr int ZOOM_SHIFT = 15;

	const s32 width = cell.dst_w;
	const s32 height = cell.dst_h;
	if (width <= 0 || height <= 0)
		return;

	if (cell.bpp_sel == 1)
	{
		LOG("6bpp cell at %d,%d not supported (pal %02x alpha %02x src %u,%u dst %ux%u)\n", x, y, cell.palette, cell.alpha, cell.src_x, cell.src_y, cell.dst_w, cell.dst_h);
		return;
	}

	// pixel size in bits: 4, 8 or 16
	const u32 pixel_shift = (cell.bpp_sel == 3) ? 4 : (cell.bpp_sel == 2) ? 3 : 2;

	// signs of the cosine and sine for the four quadrants
	const bool neg_cos = (cell.quadrant == 1) || (cell.quadrant == 2);
	const bool neg_sin = (cell.quadrant >= 2);

	// source steps per destination pixel, in 1/32768 source pixel
	auto step = [] (u32 extent, s32 size, bool negative) -> s32
	{
		const s32 v = s32((extent << ZOOM_SHIFT) / size);
		return negative ? -v : v;
	};
	const s32 ax = step(cell.w_cos, width, neg_cos);   // source x per destination column
	const s32 ay = step(cell.w_sin, width, !neg_sin);  // source y per destination column
	const s32 bx = step(cell.h_sin, height, neg_sin);  // source x per destination row
	const s32 by = step(cell.h_cos, height, neg_cos);  // source y per destination row

	// source start position in the same fixed point format
	s32 sx = s32(cell.src_x << (ZOOM_SHIFT - pixel_shift));
	s32 sy = s32(cell.src_y << (ZOOM_SHIFT - 2));

	const u32 color = (cell.bpp_sel == 0) ? (u32(cell.palette) << 4) : (u32(cell.palette) << 8);
	const int alpha = cell.alpha;

	for (s32 yi = 0; yi < height; yi++)
	{
		const int dy = cell.flip_y ? (height - 1 - yi) : yi;
		s32 px = sx;
		s32 py = sy;

		for (s32 xi = 0; xi < width; xi++)
		{
			const int dx = cell.flip_x ? (width - 1 - xi) : xi;
			const u32 u = u32(px >> ZOOM_SHIFT);
			const u32 v = u32(py >> ZOOM_SHIFT);
			u16 color_data;

			switch (cell.bpp_sel)
			{
			case 0: // 4bpp, 16 colour palette selected by the palette byte
			{
				const u8 data = m_cgram[cgram_addr(u >> 1, v)];
				color_data = palette_entry(color + BIT(data, BIT(u, 0) << 2, 4));
				break;
			}
			case 2: // 8bpp, 256 colour palette selected by the palette byte
				color_data = palette_entry(color + m_cgram[cgram_addr(u, v)]);
				break;
			default: // RGB555
				color_data = cgram_read16(u, v);
				break;
			}

			draw_pixel(target, x + dx, y + dy, color_data, alpha, darken);
			px += ax;
			py += ay;
		}
		sx += bx;
		sy += by;
	}
}


//**************************************************************************
//  AX51101
//**************************************************************************

/*
    Register map (Gunpey I/O 0x7fc0-0x7fff -> 0x00-0x3f):

    00-07  configuration (00 00 40 03 72 72 90 01 on Gunpey)
    08     interrupt mask
    09     interrupt cause (read) / acknowledge (write)
    0a-0f  configuration
    10     blitter trigger (write 2 to start)
    11     blitter mode (0 = raw copy, 8 = compressed)
    12-13  configuration (00 8c on Gunpey; 13 bit 2 = display enable?)
    14-15  blitter source x (CG-ROM, bytes)
    16-17  blitter source y
    18-19  blitter destination x (CGRAM, 16-bit words)
    1a-1b  blitter destination y
    1c-1d  blitter width - 1 (16-bit words)
    1e-1f  blitter height - 1
    20-23  ? (0x0240, 0x0000)
    24-25  display window left   (0x760 on Gunpey)
    26-27  display window top    (0x788)
    28-29  display window right  (0x89f -> 320 pixels wide)
    2a-2b  display window bottom (0x877 -> 240 lines)
    2c-2d  sheet table address, bit 15 = enable
    2e-2f  cell list base address, bit 15 = enable
    30-3f  ? (84 00 00 01 00 00 c7 56 3f 48 9a 22 0e 43 f0 15)

    Interrupt causes: 0x04 blitter done, 0x10 display list transferred,
    0x40 start of vertical blanking (0x01 and 0x80 unknown; Gunpey
    enables 0x55).
*/

ax51101_device::ax51101_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: axell_ag1_device_base(mconfig, AX51101, tag, owner, clock, ENDIANNESS_LITTLE, address_map_constructor(), 11, 11, 0xff)
{
}


void ax51101_device::update_irq()
{
	// single interrupt output
	const u8 active = active_irqs() ? 1 : 0;
	if (active != m_irq_lines)
	{
		m_irq_lines = active;
		m_irq_cb[0](active);
	}
}


void ax51101_device::vblank(screen_device &screen, bool state)
{
	// the sheet and cell tables are fetched automatically at the start
	// of each frame, so the list transfer interrupt comes with the frame one
	if (state)
		raise_irq(IRQ_FRAME | IRQ_LIST_DONE);
}


u8 ax51101_device::read(offs_t offset)
{
	offset &= 0x3f;

	if (offset == 0x09)
		return irq_cause();

	if (offset == 0x08)
		return irq_mask();

	return m_regs[offset];
}


void ax51101_device::write(offs_t offset, u8 data)
{
	offset &= 0x3f;

	LOGREGS("%s: register %02x = %02x\n", machine().describe_context(), offset, data);

	switch (offset)
	{
	case 0x08:
		set_irq_mask(data);
		break;

	case 0x09:
		ack_irq(data);
		break;

	case 0x10:
		m_regs[offset] = data;
		if (data == 2)
			start_blit(reg16(0x14), reg16(0x16), reg16(0x18), reg16(0x1a), reg16(0x1c) & 0x3ff, reg16(0x1e) & 0x3ff, m_regs[0x11], IRQ_BLITTER);
		break;

	default:
		m_regs[offset] = data;
		break;
	}
}


/*
    Cell entry (16 bytes):

    +0   e--b byxs   e = end of list, bb = bpp (0 = 4bpp, 1 = 6bpp,
                     2 = 8bpp, 3 = RGB), y / x = flip (assumed to match
                     the AX51102, Gunpey does not use them), s = skip
    +1   cccc cccc   palette (4bpp: set = c >> 4, entry = (c & 15) << 4)
    +2   u--t tttt   alpha (0 = solid), u = unknown (set on text)
    +3   xxxx xxxx   source x (bit address within the CGRAM row, i.e.
    +4   --xx xxxx   pixel * bpp) bits 7-0 / 13-8
    +5   yyyy yy--   source y bits 5-0 (the low two bits are fractional
    +6   ---y yyyy   source y bits 10-6               as on the AX51102)
    +7   xxxx xxxx   position x bits 7-0
    +8   -yyy -xxx   position y bits 10-8, position x bits 10-8
    +9   yyyy yyyy   position y bits 7-0
    +10  wwww wwww   width on screen
    +11  hhhh hhhh   height on screen
    +12  wwww wwww   source width (|width * cos| for a rotated cell)
    +13  ---- ----   presumably |width * sin|, |height * sin| as on the
    +14  ---- ----   AX51102 (always zero in Gunpey)
    +15  hhhh hhhh   source height
*/

void ax51101_device::decode_cell(offs_t address, cell_params &cell)
{
	u8 b[16];
	for (int i = 0; i < 16; i++)
		b[i] = list_read_byte(address + i);

	cell.end = BIT(b[0], 7);
	cell.disabled = BIT(b[0], 0);
	cell.flip_x = BIT(b[0], 1);
	cell.flip_y = BIT(b[0], 2);
	cell.bpp_sel = BIT(b[0], 3, 2);
	cell.palette = b[1];
	cell.alpha = (b[2] & 0x1f) << 3;
	cell.quadrant = 0; // TODO: location of the rotation quadrant unknown (b[4] bits 6-7?)
	cell.src_x = ((b[4] & 0x3f) << 8) | b[3];
	cell.src_y = ((b[6] & 0x1f) << 8) | b[5];
	cell.dst_x = b[7] | ((b[8] & 0x07) << 8);
	cell.dst_y = b[9] | ((b[8] & 0x70) << 4);
	cell.dst_w = b[10];
	cell.dst_h = b[11];
	cell.w_cos = b[12];
	cell.w_sin = b[13];
	cell.h_sin = b[14];
	cell.h_cos = b[15];
}


void ax51101_device::draw_display_list(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u16 sheet_table = reg16(0x2c);
	const u16 cell_base = reg16(0x2e);

	if (!BIT(sheet_table, 15) || !BIT(cell_base, 15))
		return;

	const s32 window_x = reg16(0x24);
	const s32 window_y = reg16(0x26);

	bitmap_target target{ bitmap, cliprect, m_rgb555_lut.get() };

	// up to 64 sheets in the table
	for (int i = 0; i < 64; i++)
	{
		const offs_t sheet = (sheet_table & 0x7fff) + i * 16;
		const u8 flags = list_read_byte(sheet + 0);

		if (BIT(flags, 0))
		{
			const u8 modulation = list_read_byte(sheet + 2);
			const int level = BIT(modulation, 7) ? 0 : (modulation & 0x1f);
			const int darken = BIT(modulation, 6) ? level : 0;
			const int sheet_alpha = BIT(modulation, 6) ? 0 : (level << 3);
			const s32 sheet_x = (list_read_byte(sheet + 6) << 8) | list_read_byte(sheet + 7);
			const s32 sheet_y = list_read_byte(sheet + 8) << 8;
			const u16 list_offset = (list_read_byte(sheet + 10) << 8) | list_read_byte(sheet + 9);
			offs_t cell_address = ((cell_base & 0x7fff) + list_offset) & 0xffff;

			LOGLIST("sheet %d: flags %02x modulation %02x pos %d,%d cells at %04x\n", i, flags, modulation, sheet_x, sheet_y, cell_address);

			// safety limit of 256 cells per sheet
			for (int c = 0; c < 256; c++)
			{
				cell_params cell;
				decode_cell(cell_address, cell);

				if (!cell.disabled)
				{
					cell.alpha = std::min<int>(0xff, cell.alpha + sheet_alpha);
					draw_cell(target, cell, sheet_x + cell.dst_x - window_x, sheet_y + cell.dst_y - window_y, darken);
				}

				if (cell.end)
					break;

				cell_address = (cell_address + 16) & 0xffff;
			}
		}

		if (BIT(flags, 7))
			break;
	}
}


//**************************************************************************
//  AX51102
//**************************************************************************

/*
    Register map (byte offsets within the 256 byte window, as seen by a
    big-endian 16-bit host):

    a0-a5  configuration (clock / display mode select etc.: c1 00 00 00
           00 1f on the Dinoking games, c0 80 80 80 00 17 on hkuranai)
    a6     ? (hkuranai writes 0xff from its frame interrupt handler)
    a7     command: bit 6 = start a blit (completion flagged by interrupt
           0x04), otherwise transfer and render the display lists
           (interrupt 0x10); bit 1 is set for both, bit 7 is set by the
           Dinoking games only, hkuranai writes 0x01 on its own
    ab-b5  configuration
    b8     interrupt mask
    b9     interrupt cause (read) / acknowledge (write)
    ba     second interrupt mask? (0xf0 on Dinoking)
    bc-bf  configuration
    c0-c1  render destination x (CGRAM pixels, 11 bits)
    c2-c3  render destination y (12 bits)
    c4-c5  display window left   (0x400 on the Taito games)
    c6-c7  display window top    (0x400)
    c8-c9  display window right  (0x67f -> 640 pixels wide, inclusive)
    ca-cb  display window bottom (0x5df -> 480 lines)
    cc-cd  sheet table address
    ce-cf  cell list base address
    d0-d1  order list address: 16-bit entries holding the offsets of the
           sheets to draw (in drawing order) within the sheet table, bit 0
           marks the last entry
    d2     control: bit 2 = display lists valid, bits 1-0 = ?
    d3     blitter mode (bits 4-0: 0 = raw copy from CG-ROM, 3 = copy from
           the sprite control memory, 8 = compressed, 0x1f = idle), bits
           7-5 = ?
    d4-d5  blitter source x (CG-ROM bytes, or sprite control memory
           address for mode 3)
    d6-d7  blitter source y (13 bits)
    d8-d9  blitter destination x (CGRAM, 16-bit words, 11 bits)
    da-db  blitter destination y (12 bits)
    dc-dd  blitter width - 1 (16-bit words, 10 bits)
    de-df  blitter height - 1 (10 bits)
    e0     display control: bit 7 = display enable, bits 6-5 = ?,
           bits 4, 2 = ?
    e1     ? (bit 0)
    e4-e7  ? (e7 bit 1 = displayed frame buffer, the other one is cleared
           to transparent when it changes)
    e8-eb  frame buffer 0 position in CGRAM (x, y)
    ec-ef  frame buffer 1 position in CGRAM
    f0-fd  CRTC: f2 = horizontal total / 2 - 3 (assumed), f4 = horizontal
           display width - 1, f8 = vertical total * 2, fa = vertical
           display lines, f0 / f6 / fc = ? (sync positions?)

    Addresses in the sprite control space: 0x8000-0x87ff select the
    built-in 2KB sprite control RAM, everything else the external SRAM.

    Interrupt causes: 0x02 sprite control memory write collision (the
    host's write was lost and must be repeated; never generated here),
    0x04 blitter done, 0x10 display lists transferred, 0x20 a second
    per-frame interrupt (hkuranai uses it as its frame interrupt), 0x40
    start of vertical blanking (0x01, 0x08, 0x80 unknown; the Dinoking
    games enable 0x7e, hkuranai 0x36).  Every cause has its own output
    pin: the Dinoking boards wire 0x40 to IRQ0 and the rest to IRQ1,
    hkuranai wires 0x20, 0x10, 0x04 and 0x02 to IRQ0-IRQ3.
*/

ax51102_device::ax51102_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: axell_ag1_device_base(mconfig, AX51102, tag, owner, clock, ENDIANNESS_BIG, address_map_constructor(FUNC(ax51102_device::list_map), this), 12, 12, 0x00)
{
}


// built-in sprite control RAM, the board supplies the external SRAM
void ax51102_device::list_map(address_map &map)
{
	map(0x8000, 0x87ff).ram();
}


void ax51102_device::update_irq()
{
	// one output per cause bit
	const u8 active = active_irqs();
	const u8 changed = active ^ m_irq_lines;
	m_irq_lines = active;
	for (u32 i = 0; i < 8; i++)
		if (BIT(changed, i))
			m_irq_cb[i](BIT(active, i));
}


u8 ax51102_device::read(offs_t offset)
{
	offset &= 0xff;

	switch (offset)
	{
	case 0xb8: return irq_mask();
	case 0xb9: return irq_cause();
	default:   return m_regs[offset];
	}
}


void ax51102_device::write(offs_t offset, u8 data)
{
	offset &= 0xff;

	LOGREGS("%s: register %02x = %02x\n", machine().describe_context(), offset, data);

	switch (offset)
	{
	case 0xa7:
		m_regs[offset] = data;
		command_w(data);
		break;

	case 0xb8:
		set_irq_mask(data);
		break;

	case 0xb9:
		ack_irq(data);
		break;

	case 0xf2: case 0xf3: case 0xf4: case 0xf5:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		m_regs[offset] = data;
		update_screen_timing();
		break;

	case 0xe7:
		// bit 1 selects the frame buffer to display; the other one becomes
		// the back buffer and is cleared (to transparent) for the next
		// frame.  Render targets elsewhere in CGRAM persist (hkuranai clears
		// its text canvas by hand).
		if (BIT(m_regs[offset] ^ data, 1))
			clear_frame_buffer(!BIT(data, 1));
		m_regs[offset] = data;
		break;

	default:
		m_regs[offset] = data;
		break;
	}
}


/*
    Display timing.  The games program the CRTC registers at boot: dkkids
    and dkbattle set up 640x480 (f2 = 0x18d, f4 = 639, f8 = 1050, fa = 480),
    hkuranai 640x468 with a longer line (f2 = 0x1a9, fa = 468).  The visible
    size is unambiguous (f4 + 1 pixels, fa lines); the totals are assumed to
    be (f2 + 3) * 2 pixels - which gives the standard 800 pixel VGA line for
    dkkids - and f8 / 2 lines, clocked at the pixel clock the device is
    given (the 25 MHz crystal next to the chip on the Taito boards).  The
    other CRTC registers (f0, f6, fc: sync positions?) are not understood.
*/
void ax51102_device::update_screen_timing()
{
	const int hvis = reg16(0xf4) + 1;
	const int vvis = reg16(0xfa);
	const int htotal = (reg16(0xf2) + 3) * 2;
	const int vtotal = reg16(0xf8) / 2;

	// ignore the intermediate states while the registers are being set up
	if (hvis < 100 || hvis > 1024 || vvis < 100 || vvis > 1024 || htotal <= hvis || vtotal <= vvis)
		return;

	const rectangle visible(0, hvis - 1, 0, vvis - 1);
	if (screen().width() == htotal && screen().height() == vtotal && screen().visible_area() == visible)
		return;

	LOG("display timing %d x %d, %d x %d total (%.2f Hz)\n", hvis, vvis, htotal, vtotal, double(clock()) / (htotal * vtotal));
	screen().configure(htotal, vtotal, visible, attotime::from_ticks(u64(htotal) * vtotal, clock()));
}


void ax51102_device::clear_frame_buffer(bool second)
{
	const u32 buf_x = reg16(second ? 0xec : 0xe8) & 0x7ff;
	const u32 buf_y = reg16(second ? 0xee : 0xea) & 0xfff;
	const rectangle &visible = screen().visible_area();

	for (int y = 0; y < visible.height(); y++)
		for (int x = 0; x < visible.width(); x++)
			cgram_write16(buf_x + x, buf_y + y, 0x8000);
}


void ax51102_device::command_w(u8 data)
{
	// bit 6 selects a blit from CG-ROM to CGRAM, otherwise the display lists
	// are transferred; bit 1 is set for both, bit 7 is set by dkkids/dkbattle
	// but not by hkuranai (meaning unknown), bit 0 is written on its own by
	// hkuranai's frame interrupt handler when a list transfer is pending
	if (BIT(data, 6))
	{
		LOGCMD("%s: blit command %02x, mode %02x\n", machine().describe_context(), data, m_regs[0xd3]);
		start_blit(reg16(0xd4), reg16(0xd6) & 0x1fff, reg16(0xd8) & 0x7ff, reg16(0xda) & 0xfff, reg16(0xdc) & 0x3ff, reg16(0xde) & 0x3ff, m_regs[0xd3] & 0x1f, IRQ_BLITTER);
	}
	else if (BIT(data, 1))
	{
		LOGCMD("%s: display list command %02x, order %04x sheets %04x cells %04x window %d,%d-%d,%d dest %d,%d\n", machine().describe_context(), data,
				reg16(0xd0), reg16(0xcc), reg16(0xce), reg16(0xc4) & 0xfff, reg16(0xc6) & 0xfff, reg16(0xc8) & 0xfff, reg16(0xca) & 0xfff, reg16(0xc0) & 0x7ff, reg16(0xc2) & 0xfff);
		render_display_list();
		// FIXME: the drawing itself takes time on the real chip (the data sheet
		// quotes up to 35M dots/s), here the list is rendered instantly
		schedule_irq(IRQ_LIST_DONE, clocks_to_attotime(64));
	}
	else
	{
		LOGCMD("%s: command %02x\n", machine().describe_context(), data);
	}

	if (data & 0x3c)
		logerror("%s: unknown command bits %02x\n", machine().describe_context(), data);
}


/*
    Cell entry (18 bytes, big-endian 16-bit fields):

    +0   e--b byxs   e = end of list, bb = bpp (0 = 4bpp, 1 = 6bpp,
                     2 = 8bpp, 3 = RGB), y = flip vertically,
                     x = flip horizontally, s = skip this cell
    +1   cccc cccc   palette (4bpp: set = c >> 4, entry = (c & 15) << 4)
    +2   -u-t tttt   alpha (0 = solid, 31 = nearly transparent), u = ?
    +3   qq-- ----   rotation quadrant (the rest is stale data)
    +4   source x (bit address within the CGRAM row, i.e. pixel * bpp,
         with the fractional position of rotated cells in the low bits)
    +6   source y, 2 fractional bits
    +8   position x: bits 10-0 magnitude, bit 11 sign, bits 15-12 stale
    +10  position y: same layout
    +12  |width * cos|      source distance covered across the cell
    +13  |width * sin|      horizontally (x and y) and vertically; an
    +14  |height * sin|     unrotated cell just has its source width
    +15  |height * cos|     and height here
    +16  width on screen
    +17  height on screen

    Rotation: the game writes the top two bits of the negated angle to
    byte 3, so quadrant 0 = 0..89 degrees, 1 = 90..179 (cosine
    negative), 2 = 180..269 (both negative), 3 = 270..359 (sine
    negative).  The destination is always the axis-aligned rectangle
    given by bytes 16/17; the source is stepped through along the two
    vectors.
*/

void ax51102_device::decode_cell(offs_t address, cell_params &cell)
{
	u8 b[18];
	for (int i = 0; i < 18; i++)
		b[i] = list_read_byte(address + i);

	cell.end = BIT(b[0], 7);
	cell.disabled = BIT(b[0], 0);
	cell.flip_x = BIT(b[0], 1);
	cell.flip_y = BIT(b[0], 2);
	cell.bpp_sel = BIT(b[0], 3, 2);
	cell.palette = b[1];
	cell.alpha = (b[2] & 0x1f) << 3;
	cell.quadrant = BIT(b[3], 6, 2);
	cell.src_x = (b[4] << 8) | b[5];
	cell.src_y = (b[6] << 8) | b[7];
	// positions are sign-magnitude, bits 15-12 are not used
	const u16 dx = (b[8] << 8) | b[9];
	const u16 dy = (b[10] << 8) | b[11];
	cell.dst_x = BIT(dx, 11) ? -(dx & 0x7ff) : (dx & 0x7ff);
	cell.dst_y = BIT(dy, 11) ? -(dy & 0x7ff) : (dy & 0x7ff);
	cell.w_cos = b[12];
	cell.w_sin = b[13];
	cell.h_sin = b[14];
	cell.h_cos = b[15];
	cell.dst_w = b[16];
	cell.dst_h = b[17];
}


void ax51102_device::draw_display_list(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// the display shows the frame buffer that was scanned out at the start
	// of vertical blanking (see vblank())
	if (BIT(m_regs[0xe0], 7) && m_display.valid())
		copybitmap(bitmap, m_display, 0, 0, 0, 0, cliprect);
}


void ax51102_device::vblank(screen_device &screen, bool state)
{
	if (!state)
		return;

	// scan out the selected RGB555 frame buffer from CGRAM
	const bool second = BIT(m_regs[0xe7], 1);
	const u32 buf_x = reg16(second ? 0xec : 0xe8) & 0x7ff;
	const u32 buf_y = reg16(second ? 0xee : 0xea) & 0xfff;

	const rectangle &visible = screen.visible_area();
	if (!m_display.valid() || m_display.width() != visible.width() || m_display.height() != visible.height())
		m_display.allocate(visible.width(), visible.height());

	for (int y = 0; y < visible.height(); y++)
	{
		u32 *dest = &m_display.pix(y);
		for (int x = 0; x < visible.width(); x++)
			*dest++ = m_rgb555_lut[cgram_read16(buf_x + x, buf_y + y) & 0x7fff];
	}

	// the second frame interrupt is signalled at the same time: dkkids just
	// acknowledges it, hkuranai uses it as its frame sync (TODO: it might be
	// tied to the start of the display period rather than vertical blanking)
	raise_irq(IRQ_FRAME | IRQ_FRAME_B);
}


/*
    Render the display lists into CGRAM.  The part of the virtual screen
    selected by the window registers (0xc4-0xcb, inclusive bounds) is drawn
    with its origin at the destination given by registers 0xc0/0xc2, which
    the games point at the back frame buffer (or at some other spot when
    they want to render a sprite to a texture).  Nothing is cleared first:
    the games draw a frame in several passes with different windows and rely
    on the first pass covering the whole buffer.
*/
void ax51102_device::render_display_list()
{
	const u16 order_list = reg16(0xd0);
	const u16 sheet_table = reg16(0xcc);
	const u16 cell_base = reg16(0xce);
	const s32 window_x0 = reg16(0xc4) & 0xfff;
	const s32 window_y0 = reg16(0xc6) & 0xfff;
	const s32 window_x1 = reg16(0xc8) & 0xfff;
	const s32 window_y1 = reg16(0xca) & 0xfff;
	const s32 dest_x = reg16(0xc0) & 0x7ff;
	const s32 dest_y = reg16(0xc2) & 0xfff;

	if (window_x1 < window_x0 || window_y1 < window_y0)
		return;

	cgram_target target{ *this, rectangle(window_x0, window_x1, window_y0, window_y1), dest_x - window_x0, dest_y - window_y0 };

	// the order list holds the offsets of the sheets to draw, in drawing
	// order; bit 0 marks the last entry
	for (int i = 0; i < 127; i++)
	{
		const u16 entry = list_read_word((order_list + i * 2) & 0xfffe);
		const offs_t sheet = (sheet_table + (entry & 0xfffe)) & 0xffff;
		const u8 flags = list_read_byte(sheet + 0);

		if (BIT(flags, 0))
			draw_sheet(target, sheet, cell_base);

		if (BIT(entry, 0))
			break;
	}
}


/*
    Draw one sheet (see the top of the file for the entry format): the
    sheet position is the point the cells are placed relative to and the
    sheet is scaled about.
*/
void ax51102_device::draw_sheet(cgram_target &target, offs_t sheet, u16 cell_base)
{
	const u8 modulation = list_read_byte(sheet + 2);
	const int level = BIT(modulation, 7) ? 0 : (modulation & 0x1f);
	const int darken = BIT(modulation, 6) ? level : 0;
	const int sheet_alpha = BIT(modulation, 6) ? 0 : (level << 3);
	const int scale_x = list_read_byte(sheet + 4);
	const int scale_y = list_read_byte(sheet + 5);
	const s32 sheet_x = (list_read_byte(sheet + 6) << 8) | list_read_byte(sheet + 7);
	const s32 sheet_y = (list_read_byte(sheet + 8) << 8) | list_read_byte(sheet + 9);
	const u16 list_offset = (list_read_byte(sheet + 10) << 8) | list_read_byte(sheet + 11);
	offs_t cell_address = (cell_base + list_offset) & 0xffff;

	LOGLIST("sheet %04x: %02x %02x %02x %02x scale %02x %02x pos %d,%d cells at %04x\n", sheet, list_read_byte(sheet + 0), list_read_byte(sheet + 1), modulation, list_read_byte(sheet + 3), scale_x, scale_y, sheet_x, sheet_y, cell_address);

	// safety limit of 256 cells per sheet
	for (int c = 0; c < 256; c++)
	{
		cell_params cell;
		decode_cell(cell_address, cell);

		if (!cell.disabled)
		{
			// the sheet translucency adds to the per-cell one (how the
			// hardware combines the two is a guess, the games use one or
			// the other)
			cell.alpha = std::min<int>(0xff, cell.alpha + sheet_alpha);

			// the sheet scale applies to the cell positions and sizes; both
			// edges are scaled the same way so that adjoining cells stay
			// seamless (the backgrounds are tiled from small cells)
			auto scaled = [] (s32 v, int scale) { return (v * scale) >> 6; };
			s32 x = sheet_x + cell.dst_x;
			s32 y = sheet_y + cell.dst_y;
			if (scale_x != 0x40)
			{
				x = sheet_x + scaled(cell.dst_x, scale_x);
				cell.dst_w = scaled(cell.dst_x + s32(cell.dst_w), scale_x) - scaled(cell.dst_x, scale_x);
			}
			if (scale_y != 0x40)
			{
				y = sheet_y + scaled(cell.dst_y, scale_y);
				cell.dst_h = scaled(cell.dst_y + s32(cell.dst_h), scale_y) - scaled(cell.dst_y, scale_y);
			}
			draw_cell(target, cell, x, y, darken);
		}

		if (cell.end)
			break;

		cell_address = (cell_address + 18) & 0xffff;
	}
}
