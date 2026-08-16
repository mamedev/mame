// license:BSD-3-Clause
// copyright-holders:

/*
Dyna games using the DYNA DC4000 GFX custom chip.
Seen on DYNA D9105 and D9106 PCBs.

Main components (from PCB silkscreen + code analysis):
  - Zilog Z0840006 (Z80 @ ~6 MHz, 24 MHz XTAL / 4)
  - DYNA DC4000 (GFX custom, gate array)
  - DYNA 22A078803 (custom I/O; 8255-like pinout but NOT an 8255, see custom_io_r)
  - 6116 (2 KB SRAM, work RAM)
  - 4x M5M4C264L or equivalent (64 KB x4 DRAM, DC4000 internal VRAM/framebuffer)
  - 24 MHz XTAL
  - Winbond WF19054 (AY8910-compatible PSG)
  - Up to 6 banks of 8 DIP switches (4-5 populated in practice)
  - 2 or 3 bipolar color PROMs (MB7114H / 82S135)

-------------------------------------------------------------------------------
DC4000 REGISTER FILE
-------------------------------------------------------------------------------

  The DC4000 exposes a 32-byte register file through two ports:
      port $10 = write pointer ("command")
      port $11 = data, POST-INCREMENTING the pointer
  A write to port $10 sets the pointer to (value & 0x1F); every subsequent
  write to port $11 stores one byte at the pointer and advances it.
  Value $08 is the one exception: it is an opcode, not a pointer (see below).

  This is proved by the power-on init routine (Aladdin $00C9, byte-identical
  in all four games).  It writes CMD $00 once and then streams exactly 32
  data bytes without touching port $10 again:

      offset  data              meaning
      ------  ----------------  -------------------------------------------
       0- 7   F0 00 F2 00       4 x (VRAM slot page, vertical scroll)
              F4 00 F6 00
       8-14   00 x7             unknown, never written again by any game
      15      1F                unknown; (0x1F + 1) * 8 = 256 would match the
                                visible height, but that is only a guess
      16-31   00 01 .. 0F       palette remap LUT, loaded with identity

  Every other command observed in the games is simply a pointer into this
  file, which explains all of them at once:

      CMD $00/$02/$04/$06  -> regs 0/2/4/6 : VRAM slot page + scroll (2 bytes)
      CMD $03              -> reg 3        : vertical scroll of slot 1
      CMD $10 .. $1F       -> regs 16..31  : palette LUT entry 0..15
      CMD $11              -> reg 17       : palette LUT entry *1*

  NOTE: CMD $11 is NOT a display-page select.  The only code in Aladdin that
  emits CMD $11 to port $10 is $4A06, which sends a single byte (C & $0F) --
  exactly the same shape as $4A90 emitting CMD $10 and CMD $1F for LUT[0] and
  LUT[15].  All the other "ld a,$11" instructions in the ROM are
  "ld ($E626),a", i.e. they load the blit BANK variable, not a command.

  Page flipping is done by the slot registers: $468F in Aladdin walks a bitmask
  and, for each set bit i, writes CMD ($02*i) with data ($F1 + 2*i), $00.  The
  init values are $F0/$F2/$F4/$F6 and the flip values are $F1/$F3/$F5/$F7, so
  a flip toggles bit 0 of the DRAM page of the selected slots.

  CMD $08  Blit opcode: 8 data bytes
             [0] dst_x   (in BYTES, 2 pixels each)
             [1] dst_y   (scanlines)
             [2] w - 1   (width in bytes)
             [3] h - 1   (height in rows)
             [4] src_lo
             [5] src_hi
             [6] attr
             [7] bank
           dst_x is in bytes: the font routines advance it by 4 after a
           4-byte-wide glyph ($4BC9) and by 8 after the 8-byte-wide ones
           ($4CAE, $4D94).

-------------------------------------------------------------------------------
DISPLAY MODEL  (four-slot compositor)
-------------------------------------------------------------------------------

  The DC4000 is not double buffered.  It composites up to four independent
  slots, each of which has

      reg 2s      low 3 bits = DRAM page shown by slot s
      reg 2s+1    vertical scroll of slot s
      port $32    bit s = slot s takes part in the composite

  Slots are layered lowest-index-on-top; colour 0 is transparent on every
  slot except the bottom-most enabled one, which provides the background.

  The GFX page comes from the ATTR byte, five bits of it:

      GFX page = attr & 0x1F

  The page is only the STARTING point: the running source address is linear
  and carries past the end of the page.  Cuty Line Limited 2's title banner starts at
  page $11 offset $FC80 and is 248x48 bytes, so it finishes inside page $12.

  Bit 4 selects the second GFX ROM socket.  These boards have two: H2, the
  big shared art mask ROM (pages $00-$0F), and H1, the game-specific one
  (pages $10-$1F).  The "gfx" region must be built with H2 at 0x000000 and
  H1 at 0x100000, whatever their sizes.

    Aladdin     attr $00,$01,$02,$03,$07,$0A,$0B  -- H1 socket not populated
    CM'92       attr $10,$11,$12,$13              -- H1 = 256 KB = 4 pages
    Cuty Line 2 attr $10,$11,$12,$13              -- H1 = 256 KB = 4 pages

  The attr values a game uses are therefore a direct measurement of how big its
  H1 has to be.  That is how the original Cherry Master '92 H1 was caught as a
  half dump, despite it being marked as a 27010 it was actually a 27020.

  Neither port $33 nor the bank byte take any part in it.

  The blit does not name a destination page.  The low nibble of the BANK byte
  is a BITMASK OF SLOTS, and the blit is written to each selected slot at
  whatever page that slot's register currently points at:

      bank $11 -> slot 0             bank $12 -> slot 1
      bank $01 -> slot 0, transparent
      bank $13 -> slots 0 AND 1      (the clear-screen path: it must wipe
                                      every layer, not just one)

  It is the same bitmask that $468F walks when it rewrites the slot page
  registers, and the clear blit that routine issues uses bank = (mask | $10),
  i.e. literally the same field.

  The Aladdin title screen shows the whole mechanism at work.  Text and the
  "T H E" / "A L A D D I N" sprites are blitted with bank $01/$11 and land on
  page 1, which slot 0 displays; the palace and the two decorative bands are
  blitted with bank $12 to page 2, which slot 1 displays.  Neither page on its
  own is the title screen -- only the composite is.  Slot 0 flips between
  pages 0 and 1, slot 1 between pages 2 and 3, and $468F performs the flip by
  rewriting the slot page registers while momentarily clearing the matching
  bits of port $32.

  The per-slot scroll is what animates everything: reg 3 ramps $FF, $FE, ...
  down through $CE while the logo rises, and the reel spin is the same
  mechanism on the reel slot.  A single global scroll register cannot
  reproduce either.

  OPEN: the priority order between slots is assumed to be 0,1,2,3.  Only
  slots 0 and 1 are ever enabled in the four games dumped so far, and for
  those the order is confirmed against the reference photo.  The meaning of
  the port $32 high nibble is also still unknown.

-------------------------------------------------------------------------------
BLITTER SOURCE ADDRESSING  (packed linear, stride = blit width)
-------------------------------------------------------------------------------

  src is a FLAT byte offset (src_hi:src_lo) and each source row is (w) bytes
  long, immediately following the previous one:

      source byte = gfx[ page_base + src + row * w + col ]

  It is NOT a 256-byte-per-row bitmap.  Two independent proofs:

  1) The eight overlay sprites of the Aladdin title ($4525, parameter table at
     $7E63) chain exactly by w*h:
         $6B00 16x24 (+$180) -> $6C80 16x24 (+$180) -> $6E00 16x24 (+$180) ->
         $6F80 36x40 (+$5A0) -> $7520 32x40 (+$500) -> $7A20 32x40 (+$500) ->
         $7F20 20x40 (+$320) -> $8240 36x40
     Eight consecutive entries with four different widths all land on the
     next boundary.  Rendering $6F80 as 36x40 packed gives the letter "A";
     the seven blits spell A-L-A-D-D-I-N.
  2) The five "logo rises" blits of $4458 are
         ($5B00,h16) ($4B00,h32) ($3B00,h48) ($2B00,h64) ($1B00,h80)
     with w=256; src + h*256 = $6B00 for all five, i.e. one bottom-anchored
     image progressively revealed from the top.

  A 256-byte-per-row model happens to give the same result for every
  full-width (w=256) image, which is why it survives a casual look at the
  backgrounds; it is wrong for every font, sprite and panel.

-------------------------------------------------------------------------------
PALETTE LUT AND TRANSPARENCY
-------------------------------------------------------------------------------

  The 16-entry LUT (regs 16-31) is applied DURING THE BLIT: each source nibble
  is rewritten as LUT[nibble] before being stored in VRAM.  It is not a
  display-time lookup.  Three idioms:
    - $4A90  LUT[0] = hi(colour), LUT[15] = lo(colour), blit a glyph with
             attr $00, then restore identity ($4AC4).  The font contains only
             nibbles 0 and 15, so this paints ink AND background colour.
    - $4AAF  fill all 16 entries with one value -> blit any artwork as a solid
             band of that colour (title bars at $43A8).
    - $4AC4  write 0,1,...,15 -> identity, i.e. normal blitting.

  Transparency: the "opaque" blits have NO transparency at all.  Proof: the
  clear-screen path ($46A5) fills the LUT with 0 and then issues a 256x256
  blit.  For the screen to actually clear, source nibbles equal to 0 must be
  written; and the test cannot be on the remapped value either, because then
  every pixel would be transparent and nothing would clear.  Consistently,
  $4A90 loading LUT[0] with a real colour only makes sense if nibble 0 is
  painted.

  The flag is bit 4 of the BANK byte, which is the only bit that separates the
  two groups across all four games:
    bank $11/$12/$13 (bit 4 set) : text, clear screen, curtain, full-screen
                                   artwork -> opaque
    bank $01         (bit 4 clr) : title overlay sprites, reel symbols
                                   -> nibble 0 transparent

-------------------------------------------------------------------------------
PALETTE
-------------------------------------------------------------------------------

  PROM index = (palette bank << 4) | pixel nibble, and

      slot 0 palette bank = port $33 & 0x0F
      slot 1 palette bank = port $33 >> 4

  It is not in the blit command at all: attr is the GFX page and the bank byte
  is the slot bitmask, so there were no spare bits left.  It is PER SLOT, one
  nibble each.

  Verified on The Aladdin against the title-screen photo, one element at a
  time -- every one of these is wrong on every other bank:

      port33   bank   what it selects
      $90      9      the "girls" bonus pictures: skin tones, blonde hair
      $88      8      the ribbon and the ALADDIN letters (light blue; every
                      other bank renders them red or green), and the light
                      blue "1991 DYNA" band with gold lettering
      $78      7      the palace: white walls, gold spires, orange/green/blue
                      spiral domes
      $10      1      the reel screen

  Cherry Master '92 confirms it independently.  Its PROM has banks 2, 3, 4 and
  5 identical except for colour 1 -- white, cream, light blue, light green --
  and during a reel screen the game cycles port $33 between $20, $30 and $50.
  Four palettes that differ only in the reel background colour, selected by
  the high nibble: that is the ALL BLUE / ALL GREEN bonus recolour.

  Cherry Master '92 is what pinned the per-slot split down, because it is the
  only game that puts different values in the two nibbles.  On its double-up
  screen it writes port $33 = $61 and then draws the cards to slot 0 and the
  girl picture to slot 1; with a single shared bank the cards came out in the
  pastel bank 6 while the girl was correct, and photographs of the real PCB
  show the card backs are blue, which only bank 1 -- the low nibble -- can
  produce.  Its reel screen is the mirror image: the panel and the frame go to
  slot 0 with low nibble 1 while the reel strips go to slot 1, and the game
  cycles the high nibble through 2, 3 and 5 to recolour the reel background.
  Those four PROM banks are identical except for colour 1, which is white,
  cream, light blue and light green: that is the ALL BLUE / ALL GREEN bonus.

  The Aladdin hid the split for a long time.  It writes $88 when both layers
  are drawn together, so the two nibbles agree, and each of its other values
  is read by only one of the two slots.

  OPEN: slots 2 and 3 have no nibble of their own.  No game enables them, so
  where their palette bank would come from is unknown.


  TODO:
  - complete / verify inputs / outputs for all games (what's in the driver is for set aladdin)
  - check complete playability (games seem to work fine through a quick playthrough by someone
    who doesn't know slot games)
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define LOG_BLIT (1U << 1)
#define LOG_REG (1U << 2)
#define LOG_PORT (1U << 3)
#define LOG_PAGE (1U << 4)
#define LOG_LAMPS (1U << 5)

// #define VERBOSE (LOG_BLIT | LOG_REG | LOG_PORT | LOG_PAGE | LOG_LAMPS)

#include "logmacro.h"

#define LOGBLIT(...) LOGMASKED(LOG_BLIT, __VA_ARGS__)
#define LOGREG(...) LOGMASKED(LOG_REG,  __VA_ARGS__)
#define LOGPORT(...) LOGMASKED(LOG_PORT, __VA_ARGS__)
#define LOGPAGE(...) LOGMASKED(LOG_PAGE, __VA_ARGS__)
#define LOGLAMPS(...) LOGMASKED(LOG_LAMPS, __VA_ARGS__)


namespace {

class dyna_dc4000_state : public driver_device
{
public:
	dyna_dc4000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_hopper(*this, "hopper")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_vram(*this, "vram%u", 0U, 0x10000U, ENDIANNESS_LITTLE)
		, m_attr(*this, "attr%u", 0U, 0x20000U, ENDIANNESS_LITTLE)
		, m_gfx(*this, "gfx")
		, m_proms(*this, "proms")
		, m_io_dsw(*this, "DSW%u", 1U)
	{ }

	void d9106(machine_config &config) ATTR_COLD;
	void eldv1(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<hopper_device> m_hopper;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	memory_share_array_creator<uint8_t, 8> m_vram;
	memory_share_array_creator<uint8_t, 8> m_attr;
	required_region_ptr<uint8_t> m_gfx;
	required_region_ptr<uint8_t> m_proms;

	required_ioport_array<3> m_io_dsw;

	uint8_t m_slot_enable = 0;

	uint8_t m_dc4000_reg[32] {};
	uint8_t m_dc4000_reg_ptr = 0;
	bool m_blit_mode = false;
	uint8_t m_dc4000_count = 0;
	uint8_t m_dc4000_buf[8] {};

	uint8_t m_palbank = 0;

	uint8_t m_io_ctrl = 0xfb;

	// Convenience accessors on the register file
	uint8_t lut(int i) const { return m_dc4000_reg[16 + (i & 0x0f)] & 0x0f; }
	uint8_t slot_page(int slot) const { return m_dc4000_reg[(slot & 3) * 2] & 0x07; }
	uint8_t slot_scroll(int slot) const { return m_dc4000_reg[(slot & 3) * 2 + 1]; }
	bool slot_enabled(int slot) const{ return BIT(m_slot_enable, slot & 3); }

	int gfx_page() const;
	void log_reg_write(uint8_t idx, uint8_t data);

	void dc4000_cmd_w(uint8_t data);
	void dc4000_data_w(uint8_t data);
	void dc4000_ctrl_w(uint8_t data);

	void palbank_w(uint8_t data);

	uint8_t custom_io_r(offs_t offset);
	void custom_io_w(offs_t offset, uint8_t data);
	void lamps_w(uint8_t data);
	void counters_w(uint8_t data);

	void do_blit();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void palette_init(palette_device &palette) const ATTR_COLD;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void eldv1_io_map(address_map &map) ATTR_COLD;
};


void dyna_dc4000_state::machine_start()
{
	save_item(NAME(m_slot_enable));
	save_item(NAME(m_dc4000_count));
	save_item(NAME(m_dc4000_buf));
	save_item(NAME(m_io_ctrl));
	save_item(NAME(m_dc4000_reg));
	save_item(NAME(m_dc4000_reg_ptr));
	save_item(NAME(m_blit_mode));
	save_item(NAME(m_palbank));
}

void dyna_dc4000_state::machine_reset()
{
	m_dc4000_count = 0;
	m_io_ctrl = 0xfb;
	m_dc4000_reg_ptr = 0;
	m_blit_mode = false;
	m_slot_enable = 0;
	m_palbank = 0;

	// Register file power-on state, mirroring what the init routine writes
	// (Aladdin $00C9, byte-identical in all four games).
	std::fill(std::begin(m_dc4000_reg), std::end(m_dc4000_reg), 0);
	m_dc4000_reg[0] = 0xf0;
	m_dc4000_reg[2] = 0xf2;
	m_dc4000_reg[4] = 0xf4;
	m_dc4000_reg[6] = 0xf6;
	m_dc4000_reg[15] = 0x1f;

	for (int i = 0; i < 16; i++)
		m_dc4000_reg[16 + i] = i; // identity palette LUT

	for (int i = 0; i < 8; i++)
	{
		std::fill(std::begin(m_vram[i]), std::end(m_vram[i]), 0);
		std::fill(std::begin(m_attr[i]), std::end(m_attr[i]), 0);
	}
}


void dyna_dc4000_state::palette_init(palette_device &palette) const
{
	// sets with 3 PROMs
	if (m_proms.length() >= 0x300)
	{
		for (int i = 0; i < 256; i++)
		{
			uint8_t const r = (m_proms[0x000 + i] & 0x0f) * 0x11;
			uint8_t const g = (m_proms[0x100 + i] & 0x0f) * 0x11;
			uint8_t const b = (m_proms[0x200 + i] & 0x0f) * 0x11;
			m_palette->set_pen_color(i, r, g, b);
		}
	}
	// sets with 2 PROMs
	else if (m_proms.length() >= 0x200)
	{
		for (int i = 0; i < 256; i++)
		{
			uint16_t const dat = (m_proms[0x000 + i] << 8) | m_proms[0x100 + i];

			m_palette->set_pen_color(i, pal5bit((dat >> 6) & 0x1f), pal5bit((dat >> 11) & 0x1f), pal5bit((dat >> 1) & 0x1f));
		}
	}
}

// The DC4000 has no real "command set": The single exception is $08, which is the blit opcode.
void dyna_dc4000_state::dc4000_cmd_w(uint8_t data)
{
	m_dc4000_count = 0;

	if (data == 0x08)
	{
		m_blit_mode = true;
	}
	else
	{
		m_blit_mode = false;

		m_dc4000_reg_ptr = data & 0x1f;

		if (data & 0xe0)
			LOGREG("DC4000, CMDHI, %02x\n", data); // not seen yet
	}
}

void dyna_dc4000_state::log_reg_write(uint8_t idx, uint8_t data)
{
	LOGREG("DC4000, REG, %02X, %02X\n", idx, data);

	if (idx < 8 && !(idx & 1))
		LOGPAGE("DC4000, SLOT, %d, page = $%02x\n", idx / 2, data);
}

void dyna_dc4000_state::dc4000_data_w(uint8_t data)
{
	if (m_blit_mode)
	{
		if (m_dc4000_count < 8)
			m_dc4000_buf[m_dc4000_count] = data;

		m_dc4000_count++;

		if (m_dc4000_count == 8)
		{
			do_blit();
			m_dc4000_count = 0;
		}

		return;
	}

	if (m_dc4000_reg_ptr < 0x20)
	{
		m_dc4000_reg[m_dc4000_reg_ptr] = data;

		log_reg_write(m_dc4000_reg_ptr, data);

		m_dc4000_reg_ptr++;
	}
	else
	{
		LOGREG("DC4000, REGOVF, %02x\n", data);
	}
}

// DYNA 22A078803 custom I/O ($20-$23)
//
// This is not an i8255 and must not be emulated with one.  The three control
// words the games write are $FB (init, $00D1), $F0 and $FF (the DIP read
// routine at $13CA).
//
// The custom chip evidently treats the control byte as plain mode-0 direction
// bits, so that is what is implemented here:
//
//   bit 4 : port A direction, 1 = input
//   bit 3 : port C upper nibble, 1 = input
//   bit 1 : port B direction, 1 = input
//   bit 0 : port C lower nibble, 1 = input
//
// Reading a port that is currently an output returns the output latch, as a
// mode-0 8255 would.  That matters: the check at $13CA (and $31D0) reads the
// three ports once with $F0 and once with $FF and compares the second set
// against the complement of the first, so the $F0 pass has to return the
// latches rather than the switches for the test to mean anything.

uint8_t dyna_dc4000_state::custom_io_r(offs_t offset)
{
	switch (offset & 3)
	{
	case 0: // port A
		return BIT(m_io_ctrl, 4) ? (m_io_dsw[0]->read() ^ 0xff) : 0x00;

	case 1: // port B
		return BIT(m_io_ctrl, 1) ? (m_io_dsw[1]->read() ^ 0xff) : 0x00;

	case 2: // port C, the two nibbles have independent directions
	{
		uint8_t const dsw = m_io_dsw[2]->read() ^ 0xff;
		uint8_t res = 0;

		if (BIT(m_io_ctrl, 0)) res |= dsw & 0x0f;
		if (BIT(m_io_ctrl, 3)) res |= dsw & 0xf0;

		return res;
	}

	default: // control port is write only
		return 0xff;
	}
}

void dyna_dc4000_state::custom_io_w(offs_t offset, uint8_t data)
{
	if ((offset & 3) == 3)
	{
		if (BIT(data, 7))
		{
			m_io_ctrl = data;
			LOGPORT("DC4000, IOCTRL, %02x\n", data);
		}
		else
		{
			// bit-set/reset on port C; never used by any of the four games
			LOGPORT("DC4000, IOBSR, %02x\n", data);
		}
		return;
	}

	// The games only ever read; log anything else so it is not missed.
	LOGPORT("DC4000, IOW, %d, %02x\n", offset & 3, data);
}

void dyna_dc4000_state::dc4000_ctrl_w(uint8_t data)
{
	m_slot_enable = data & 0x0f;

	LOGPORT("DC4000, P32, %02x\n", data);
	LOGPAGE("DC4000, SLOTS, en = %x, pages=%d %d %d %d, scroll = %02x %02x %02x %02x\n",
			m_slot_enable,
			slot_page(0), slot_page(1), slot_page(2), slot_page(3),
			slot_scroll(0), slot_scroll(1), slot_scroll(2), slot_scroll(3));
}

void dyna_dc4000_state::palbank_w(uint8_t data)
{
	m_palbank = data;

	LOGPORT("DC4000, P33,%02x\n", data);
}

int dyna_dc4000_state::gfx_page() const
{
	return m_dc4000_buf[6] & 0x1f;
}

void dyna_dc4000_state::do_blit()
{
	uint8_t const dst_x = m_dc4000_buf[0];
	uint8_t const dst_y = m_dc4000_buf[1];
	int const width = m_dc4000_buf[2] + 1; // bytes
	int const height = m_dc4000_buf[3] + 1; // rows
	uint8_t const src_lo = m_dc4000_buf[4];
	uint8_t const src_hi = m_dc4000_buf[5];
	uint8_t const attr = m_dc4000_buf[6];
	uint8_t const bank = m_dc4000_buf[7];

	uint16_t const src = ((uint16_t)src_hi << 8) | src_lo;

	constexpr int DST_STRIDE = 256;

	int const page = gfx_page();

	bool const opaque = BIT(bank, 4);

	uint32_t const page_base = (uint32_t)page * 0x10000;

	// One machine-parsable line per blit, so the whole frame can be replayed
	// offline against the GFX ROM without re-running the emulation.
	char lutstr[17];
	for (int i = 0; i < 16; i++)
		lutstr[i] = "0123456789ABCDEF"[lut(i)];
	lutstr[16] = 0;

	LOGBLIT("DC4000, BLIT, %u, %u, %u, %u, %04x, %02x, %02x, %02x, %x, %05x, %s, %u,%s, slots = %x, pages = %d%d%d%d\n",
			dst_x, dst_y, width, height, src, attr, bank, m_palbank,
			page, page_base + src, opaque ? "OPQ" : "TRN", m_palbank, lutstr,
			bank & 0x0f,
			slot_page(0), slot_page(1), slot_page(2), slot_page(3));

	if (page_base >= m_gfx.bytes())
	{
		LOGBLIT("DC4000, PAGEOOR, %x, %02x, %02x\n", page, bank, m_palbank);
		return;
	}

	for (int slot = 0; slot < 4; slot++)
	{
		if (!BIT(bank, slot))
			continue;

		int const dst_page = slot_page(slot);

		uint8_t const pal_bank = slot ? (m_palbank >> 4) : (m_palbank & 0x0f);

		uint8_t *dst_ptr = m_vram[dst_page].target();
		uint8_t *attr_ptr = m_attr[dst_page].target();

		for (int row = 0; row < height; row++)
		{
			int const dy = (dst_y + row) & 0xff;

			for (int col = 0; col < width; col++)
			{
				int const dx = (dst_x + col) & 0xff;

				uint32_t const soff = page_base + src + (uint32_t)row * width + col;
				uint32_t const doff = (uint32_t)dy * DST_STRIDE + dx;

				if (soff >= m_gfx.bytes())
					continue;

				uint8_t const src_byte = m_gfx[soff];
				uint8_t const dst_byte = dst_ptr[doff];

				uint8_t const hi = (src_byte >> 4) & 0x0f;
				uint8_t const lo = src_byte & 0x0f;

				bool const wr_hi = opaque || (hi != 0);
				bool const wr_lo = opaque || (lo != 0);

				uint8_t const out_hi = wr_hi ? lut(hi) : ((dst_byte >> 4) & 0x0f);
				uint8_t const out_lo = wr_lo ? lut(lo) : (dst_byte & 0x0f);

				dst_ptr[doff] = (out_hi << 4) | out_lo;

				if (wr_hi)
					attr_ptr[doff * 2] = pal_bank;
				if (wr_lo)
					attr_ptr[doff * 2 + 1] = pal_bank;
			}
		}
	}
}


uint32_t dyna_dc4000_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Build the list of slots to composite, top first.
	int slots[4];
	int nslots = 0;

	for (int s = 0; s < 4; s++)
		if (slot_enabled(s))
			slots[nslots++] = s;

	if (nslots == 0)
	{
		bitmap.fill(rgb_t::black(), cliprect);

		return 0;
	}

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *dst = &bitmap.pix(y);

		for (int bx = 0; bx < 256; bx++)
		{
			int const px = bx * 2;

			// Two pixels per byte, resolved independently so that one can be
			// transparent while the other is not.
			for (int half = 0; half < 2; half++)
			{
				int const xpix = px + half;

				if (xpix < cliprect.left() || xpix > cliprect.right())
					continue;

				uint8_t pen = 0;

				for (int i = 0; i < nslots; i++)
				{
					int const s = slots[i];
					uint8_t const *src = m_vram[slot_page(s)].target();
					uint8_t const *attr = m_attr[slot_page(s)].target();
					int const vram_y = (y + slot_scroll(s)) & 0xff;
					uint32_t const off = (uint32_t)vram_y * 256 + bx;
					uint8_t const byte_val= src[off];

					uint8_t const nib = half ? (byte_val & 0x0f) : ((byte_val >> 4) & 0x0f);

					// Colour 0 is transparent, except on the bottom-most
					// enabled slot where it is the background colour.
					if (nib == 0 && i != nslots - 1)
						continue;

					// No LUT here: it is applied at blit time, so the nibble
					// already stored in VRAM is the final PROM sub-index.
					pen = (attr[off * 2 + half] << 4) | nib;
					break;
				}

				dst[xpix] = m_palette->pen_color(pen);
			}
		}
	}
	return 0;
}

void dyna_dc4000_state::lamps_w(uint8_t data)
{
	LOGLAMPS("%s unknown lamps_w bits written %02x\n", machine().describe_context(), data);
}

void dyna_dc4000_state::counters_w(uint8_t data)
{
	// exact counter - coin mech match to be verified
	machine().bookkeeping().coin_counter_w(3, BIT(data, 0)); // KEYOUT
	machine().bookkeeping().coin_counter_w(3, BIT(data, 2)); // PAYOUT
	machine().bookkeeping().coin_counter_w(2, BIT(data, 3)); // COIN 3
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5)); // COIN 2
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6)); // COIN 1

	m_hopper->motor_w(BIT(data, 7));

	if (data & 0x12)
		logerror("%s unknown counters_w bits written %02x\n", machine().describe_context(), data);
}


void dyna_dc4000_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
	map(0xe000, 0xffff).ram().share("nvram");
}

void dyna_dc4000_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x01, 0x01).r("ay", FUNC(ay8910_device::data_r));
	map(0x02, 0x02).w("ay", FUNC(ay8910_device::data_w));
	map(0x03, 0x03).w("ay", FUNC(ay8910_device::address_w));

	map(0x10, 0x10).w(FUNC(dyna_dc4000_state::dc4000_cmd_w));
	map(0x11, 0x11).w(FUNC(dyna_dc4000_state::dc4000_data_w));

	map(0x20, 0x23).rw(FUNC(dyna_dc4000_state::custom_io_r), FUNC(dyna_dc4000_state::custom_io_w));

	map(0x30, 0x30).portr("IN0").w(FUNC(dyna_dc4000_state::lamps_w));
	map(0x31, 0x31).portr("IN1").w(FUNC(dyna_dc4000_state::counters_w));

	map(0x32, 0x32).portr("SERVICE").w(FUNC(dyna_dc4000_state::dc4000_ctrl_w));
	map(0x33, 0x33).w(FUNC(dyna_dc4000_state::palbank_w));
}

void dyna_dc4000_state::eldv1_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x00).w(FUNC(dyna_dc4000_state::dc4000_cmd_w));
	map(0x01, 0x01).w(FUNC(dyna_dc4000_state::dc4000_data_w));

	map(0x11, 0x11).r("ay", FUNC(ay8910_device::data_r));
	map(0x12, 0x12).w("ay", FUNC(ay8910_device::data_w));
	map(0x13, 0x13).w("ay", FUNC(ay8910_device::address_w));

	map(0x20, 0x23).rw(FUNC(dyna_dc4000_state::custom_io_r), FUNC(dyna_dc4000_state::custom_io_w));

	map(0x30, 0x30).portr("IN0");
	map(0x31, 0x31).portr("IN1");
	map(0x32, 0x32).portr("SERVICE");

	map(0x33, 0x33).w(FUNC(dyna_dc4000_state::lamps_w));
	map(0x34, 0x34).w(FUNC(dyna_dc4000_state::counters_w));
	map(0x35, 0x35).w(FUNC(dyna_dc4000_state::dc4000_ctrl_w));
	map(0x36, 0x36).w(FUNC(dyna_dc4000_state::palbank_w));
}


static INPUT_PORTS_START( aladdin )
	// inputs are for The Aladdin 1.2U. TODO: complete, check other sets
	PORT_START("IN0") // PLAYER in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Low / Show Odds")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	// Coin / key in related keys aren't confirmed to be correctly assigned between them
	PORT_START("IN1") // COIN in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("SERVICE") // TEST in test mode
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) // blitter busy (0 = ready)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) // sync/ready (1 = ready)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(ticket_dispenser_device::line_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) // hopper related
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

	// definitions are for The Aladdin 1.2U. TODO: complete, check other sets
	// custom port A
	PORT_START("DSW1")
	PORT_DIPNAME(    0x01, 0x01, "Enable Bookkeeping Menu" ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(       0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x02, 0x02, "Double Up Game" ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(       0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(       0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x08, 0x08, "Double Up Girl Display" ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(       0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(       0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x20, 0x20, "Reel Speed" ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(       0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(       0x00, DEF_STR( High ) )
	PORT_DIPNAME(    0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(       0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(       0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )

	// custom port B
	PORT_START("DSW2")
	PORT_DIPNAME(    0x07, 0x07, "Main Game Rate" ) PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(       0x00, "55%" )
	PORT_DIPSETTING(       0x01, "60%" )
	PORT_DIPSETTING(       0x02, "65%" )
	PORT_DIPSETTING(       0x03, "70%" )
	PORT_DIPSETTING(       0x04, "75%" )
	PORT_DIPSETTING(       0x05, "80%" )
	PORT_DIPSETTING(       0x06, "85%" )
	PORT_DIPSETTING(       0x07, "90%" )
	PORT_DIPNAME(    0x08, 0x08, "Double Up Rate" ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(       0x00, "80%" )
	PORT_DIPSETTING(       0x08, "90%" )
	PORT_DIPNAME(    0x30, 0x30, "Maximum Bet" ) PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(       0x00, "8" )
	PORT_DIPSETTING(       0x10, "16" )
	PORT_DIPSETTING(       0x20, "32" )
	PORT_DIPSETTING(       0x30, "64" )
	PORT_DIPNAME(    0x40, 0x40, "Minimum Bet" ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(       0x00, "8" )
	PORT_DIPSETTING(       0x40, "16" )
	PORT_DIPNAME(    0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(       0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )

	// custom port C
	PORT_START("DSW3")
	PORT_DIPNAME(    0x03, 0x03, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(       0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(       0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(       0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(       0x03, DEF_STR( 1C_10C ) )
	PORT_DIPNAME(    0x1c, 0x1c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DSW3:3,4,5")
	PORT_DIPSETTING(       0x00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(       0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(       0x08, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(       0x0c, DEF_STR( 1C_20C ) )
	PORT_DIPSETTING(       0x10, DEF_STR( 1C_25C ) )
	PORT_DIPSETTING(       0x14, "1 Coin/40 Credits" )
	PORT_DIPSETTING(       0x18, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(       0x1c, DEF_STR( 1C_100C ) )
	PORT_DIPNAME(    0x60, 0x60, "Coin C" )     PORT_DIPLOCATION("DSW3:6,7")
	PORT_DIPSETTING(       0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(       0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(       0x40, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(       0x60, DEF_STR( 1C_5C ) )
	PORT_DIPNAME(    0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(       0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )

	// AY8910 port A
	PORT_START("DSW4")
	PORT_DIPNAME(    0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(       0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(       0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(       0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(       0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(       0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(       0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(       0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(       0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )

	// AY8910 port B
	PORT_START("DSW5")
	PORT_DIPNAME(    0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(       0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(       0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(       0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(       0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(       0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(       0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(       0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(       0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
INPUT_PORTS_END


void dyna_dc4000_state::d9106(machine_config &config)
{
	Z80(config, m_maincpu, 24_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &dyna_dc4000_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &dyna_dc4000_state::io_map);

	// DYNA 22A078803 custom

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	HOPPER(config, m_hopper, attotime::from_msec(50));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(512, 256);
	screen.set_visarea(0, 511, 0, 255);
	screen.set_screen_update(FUNC(dyna_dc4000_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0, HOLD_LINE);

	PALETTE(config, m_palette, FUNC(dyna_dc4000_state::palette_init), 0x100);

	SPEAKER(config, "mono").front_center();
	ay8910_device &ay(AY8910(config, "ay", 24_MHz_XTAL / 16));
	ay.port_a_read_callback().set_ioport("DSW4");
	ay.port_b_read_callback().set_ioport("DSW5");
	ay.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void dyna_dc4000_state::eldv1(machine_config &config)
{
	d9106(config);

	m_maincpu->set_addrmap(AS_IO, &dyna_dc4000_state::eldv1_io_map);
}


//  The Aladdin. Dyna, 1991
//  V1.2U - PCB D9106
ROM_START( aladdin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "ald2_v1.2u_27c512.d15", 0x00000, 0x10000, CRC(66c638ca) SHA1(ac0e9af5cd7535e8a86573723851b987c4a80c63) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "538000_as_27c080.h2", 0x000000, 0x100000, CRC(800c6c8d) SHA1(bf8d8f05b21e6cd4f0efed1ae7b66c2d9d8f43ee) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114h.h14", 0x000, 0x100, CRC(a69819b8) SHA1(5818046aae387f5b137c379cc4d78a15739c71cc) )
	ROM_LOAD( "mb7114h.h15", 0x100, 0x100, CRC(36c08918) SHA1(fa87dea8fd27c1ac7e007e2cdef77ef5eabf1a7b) )
	ROM_LOAD( "mb7114h.h16", 0x200, 0x100, CRC(71e66913) SHA1(800b05ea8eb1bb89e933a6f44632c7ebfea52e03) )
ROM_END

//  The Aladdin. Dyna, 1991
//  V1.1A - PCB D9106
ROM_START( aladdina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "ald2_v1.1a_27c512.d15", 0x00000, 0x10000, CRC(b13baf47) SHA1(2c45edca22add535a5cf367810ac26d84f7abd82) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "538000_as_27c080.h2", 0x000000, 0x100000, CRC(800c6c8d) SHA1(bf8d8f05b21e6cd4f0efed1ae7b66c2d9d8f43ee) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114h.h14", 0x000, 0x100, CRC(a69819b8) SHA1(5818046aae387f5b137c379cc4d78a15739c71cc) )
	ROM_LOAD( "mb7114h.h15", 0x100, 0x100, CRC(36c08918) SHA1(fa87dea8fd27c1ac7e007e2cdef77ef5eabf1a7b) )
	ROM_LOAD( "mb7114h.h16", 0x200, 0x100, CRC(71e66913) SHA1(800b05ea8eb1bb89e933a6f44632c7ebfea52e03) )
ROM_END

//  V1.1U - PCB D9106
ROM_START( aladdinb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "al_20u.d15", 0x00000, 0x10000, CRC(69f93957) SHA1(2797ac17caa9ed32f77709534f33a8e17ccb26d4) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "538000_as_27c080.h2", 0x000000, 0x100000, CRC(800c6c8d) SHA1(bf8d8f05b21e6cd4f0efed1ae7b66c2d9d8f43ee) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114h.h14", 0x000, 0x100, CRC(a69819b8) SHA1(5818046aae387f5b137c379cc4d78a15739c71cc) )
	ROM_LOAD( "mb7114h.h15", 0x100, 0x100, CRC(36c08918) SHA1(fa87dea8fd27c1ac7e007e2cdef77ef5eabf1a7b) )
	ROM_LOAD( "mb7114h.h16", 0x200, 0x100, CRC(71e66913) SHA1(800b05ea8eb1bb89e933a6f44632c7ebfea52e03) )
ROM_END

ROM_START( cmast92 ) // DYNA D9106B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm9230d.rom", 0x00000, 0x10000, CRC(214a0a2d) SHA1(2d349e0888ac2da3df954517fdeb9214a3b17ae1) )  // V1.2D

	ROM_REGION( 0x140000, "gfx", 0 )
	ROM_LOAD( "dyna dm9105.2h", 0x000000, 0x100000, CRC(800c6c8d) SHA1(bf8d8f05b21e6cd4f0efed1ae7b66c2d9d8f43ee) )
	ROM_LOAD( "1h",             0x100000, 0x040000, CRC(2d98366b) SHA1(462bac759e79d8429c0b69fc903edb15f4ee6325) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "14h", 0x000, 0x100, CRC(20e594fe) SHA1(d798f142732e8da6ec9764133955c041d2259f64) )
	ROM_LOAD( "15h", 0x100, 0x100, CRC(83fab238) SHA1(7c5451d69f865a10b63c013169ddbf57405bc3a9) )
	ROM_LOAD( "16h", 0x200, 0x100, CRC(706e7ee6) SHA1(dca1cc0e2c1c27bc211516ad369f557eb4b3980a) )
ROM_END

ROM_START( cmast92a ) // DYNA D9106B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15d", 0x00000, 0x10000, CRC(d703c8e5) SHA1(77d8228878b64a299b4b6f3fe3befcea179ca4af) )  // V1.1D

	ROM_REGION( 0x140000, "gfx", 0 )
	ROM_LOAD( "dyna dm9105.2h", 0x000000, 0x100000, CRC(800c6c8d) SHA1(bf8d8f05b21e6cd4f0efed1ae7b66c2d9d8f43ee) )
	ROM_LOAD( "1h",             0x100000, 0x040000, CRC(2d98366b) SHA1(462bac759e79d8429c0b69fc903edb15f4ee6325) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "14h", 0x000, 0x100, CRC(20e594fe) SHA1(d798f142732e8da6ec9764133955c041d2259f64) )
	ROM_LOAD( "15h", 0x100, 0x100, CRC(83fab238) SHA1(7c5451d69f865a10b63c013169ddbf57405bc3a9) )
	ROM_LOAD( "16h", 0x200, 0x100, CRC(706e7ee6) SHA1(dca1cc0e2c1c27bc211516ad369f557eb4b3980a) )
ROM_END

ROM_START( eldoradd ) // String "DYNA ELD3 V5.1DR" on program ROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "51d_el3_m27c512.15d", 0x00000, 0x10000, CRC(a7769d4a) SHA1(2ccd14be94a0b752113f529431b3dd4fadbf619b) )

	ROM_REGION( 0x180000, "gfx", 0 )
	ROM_LOAD( "2h_el3_tms27c040.2h", 0x000000, 0x080000, CRC(79a37ee1) SHA1(510e4ab168003d48173d5f8ddbf396668caf8e3e) )
	ROM_LOAD( "1h_el3_tms27c040.1h", 0x100000, 0x080000, CRC(0ba677ac) SHA1(4492183cd01ba6f8ba3da233a6fd4fcb86447308) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "eh_82s135.15h", 0x000, 0x100, CRC(bc64fea7) SHA1(7aef1bd14936c8f445a7ce08547e7ab962cea797) )
	ROM_LOAD( "eg_82s135.15g", 0x100, 0x100, CRC(19214600) SHA1(33a62cd91bf73fa5aa37ab961797b8c5e4ac4e30) )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "pal16l8.13f", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.11e", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "gal16v8.9f",  0x400, 0x104, NO_DUMP )
ROM_END

ROM_START( eldoraddo ) // String "DYNA ELD3 V1.1TA" on program ROM, DYNA D9105B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "dyna nel 20t.c14", 0x00000, 0x10000, CRC(77b3b2ce) SHA1(e94b976ae9e5a899d916fffc8118486cbedab8b6) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "tc538000p-dyna dm9106.g15", 0x000000, 0x100000, CRC(fa84c372) SHA1(a71e57e76321b7ebb16933d9bc983b9160995b4a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114.e8",  0x000, 0x100, CRC(fa274678) SHA1(6712cb1f7ead1a7aa703ec799e7199c33ace857c) )
	ROM_LOAD( "mb7114.e10", 0x100, 0x100, CRC(e58877ea) SHA1(30fa873fc05d91610ef68eef54b78f2c7301a62a) )
	ROM_LOAD( "mb7114.e12", 0x200, 0x100, CRC(781b2842) SHA1(566667d4f81e93b29bb01dbc51bf144c02dff75d) )

	ROM_REGION( 0x400, "plds", 0 )  // available as brute-forced dumps, need to be verified and converted
	ROM_LOAD( "pal16l8.d13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.e11", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( eldoraddob ) // String "DYNA ELD3 V2.0D" in program ROM, DYNA D9105B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "elb.50d.d15", 0x00000, 0x10000, CRC(34d55507) SHA1(8cc293bb5e493a837320e14d0316a0658084dde3) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "tc538000p-dyna dm9106.h2", 0x000000, 0x100000, CRC(fa84c372) SHA1(a71e57e76321b7ebb16933d9bc983b9160995b4a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "e14", 0x000, 0x100, CRC(fa274678) SHA1(6712cb1f7ead1a7aa703ec799e7199c33ace857c) )
	ROM_LOAD( "e15", 0x100, 0x100, CRC(e58877ea) SHA1(30fa873fc05d91610ef68eef54b78f2c7301a62a) )
	ROM_LOAD( "e16", 0x200, 0x100, CRC(781b2842) SHA1(566667d4f81e93b29bb01dbc51bf144c02dff75d) )

	ROM_REGION( 0x400, "plds", 0 ) // available as brute-forced dumps, need to be verified and converted
	ROM_LOAD( "pal16l8.d13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.e11", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( eldoraddoc ) // String "DYNA ELD3 V1.1J" in program ROM, DYNA D9105B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "nel.20d.14c", 0x00000, 0x10000, CRC(fee901b9) SHA1(d304fd5ea39cada5787c9f742f6b7801cf12670c) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "tc538000p-dyna dm9106.g15", 0x000000, 0x100000, CRC(fa84c372) SHA1(a71e57e76321b7ebb16933d9bc983b9160995b4a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114.e8",  0x000, 0x100, CRC(fa274678) SHA1(6712cb1f7ead1a7aa703ec799e7199c33ace857c) )
	ROM_LOAD( "mb7114.e10", 0x100, 0x100, CRC(e58877ea) SHA1(30fa873fc05d91610ef68eef54b78f2c7301a62a) )
	ROM_LOAD( "mb7114.e12", 0x200, 0x100, CRC(781b2842) SHA1(566667d4f81e93b29bb01dbc51bf144c02dff75d) )

	ROM_REGION( 0x400, "plds", 0 )  // available as brute-forced dumps, need to be verified and converted
	ROM_LOAD( "pal16l8.d13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.e11", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( eldoraddod ) // String "DYNA ELD3 V1.1U" in program ROM, DYNA D9105B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "27c512_v1.1u.c14", 0x00000, 0x10000, CRC(3274d388) SHA1(180c9389fe7ccdee716b28a87effdc3970e057bf) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "tc538009.g15", 0x000000, 0x100000, CRC(fa84c372) SHA1(a71e57e76321b7ebb16933d9bc983b9160995b4a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "bprom.d8",  0x000, 0x100, CRC(fa274678) SHA1(6712cb1f7ead1a7aa703ec799e7199c33ace857c) )
	ROM_LOAD( "bprom.d10", 0x100, 0x100, CRC(e58877ea) SHA1(30fa873fc05d91610ef68eef54b78f2c7301a62a) )
	ROM_LOAD( "bprom.d12", 0x200, 0x100, CRC(781b2842) SHA1(566667d4f81e93b29bb01dbc51bf144c02dff75d) )

	ROM_REGION( 0x400, "plds", 0 )  // available as brute-forced dumps, need to be verified and converted
	ROM_LOAD( "pal16l8.d13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.e11", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( cll2 ) // DYNA D9106C PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15d", 0x00000, 0x10000, CRC(34007d38) SHA1(f33c1b99df6d3048fd6e1d870f7d407a1e0cc4fb) )  // DYNA MAH-1 V1.60, but also DYNA QLL2 V4.1D? Is it a multigame?

	ROM_REGION( 0x140000, "gfx", 0 )
	ROM_LOAD( "dyna dm9105.2h", 0x000000, 0x100000, CRC(800c6c8d) SHA1(bf8d8f05b21e6cd4f0efed1ae7b66c2d9d8f43ee) )
	ROM_LOAD( "1h",             0x100000, 0x040000, CRC(2d98366b) SHA1(462bac759e79d8429c0b69fc903edb15f4ee6325) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s135.15h", 0x000, 0x100, CRC(ac2c0cc2) SHA1(8dc81ea1b258b19c6840a23eac4a363a848ff008) )
	ROM_LOAD( "82s135.15g", 0x100, 0x100, CRC(2686eec2) SHA1(8965af1f57041a8eb2e07a611128223f988b8569) )
ROM_END

} // anonymous namespace


// Dyna D9106 / D9106B / D9106C PCB
GAME(  1991, eldoradd,   0,        d9106, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (Dyna D9106 HW, V5.1DR)", MACHINE_NOT_WORKING )
GAME(  1991, aladdin,    0,        d9106, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "The Aladdin (V1.2U)",               MACHINE_NOT_WORKING )
GAME(  1991, aladdina,   aladdin,  d9106, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "The Aladdin (V1.1A)",               MACHINE_NOT_WORKING )
GAME(  1991, aladdinb,   aladdin,  d9106, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "The Aladdin (V1.1U)",               MACHINE_NOT_WORKING )
GAME(  1992, cmast92,    0,        d9106, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "Cherry Master '92 (V1.2D)",         MACHINE_NOT_WORKING )
GAME(  1992, cmast92a,   cmast92,  d9106, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "Cherry Master '92 (V1.1D)",         MACHINE_NOT_WORKING )
GAME(  1992, cll2,       0,        d9106, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "Cuty Line Limited 2 (V1.60)",       MACHINE_NOT_WORKING )

// Dyna D9105 PCB
GAME(  1991, eldoraddo,  eldoradd, eldv1, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (Dyna D9105 HW, V1.1TA)", MACHINE_NOT_WORKING )
GAME(  1991, eldoraddob, eldoradd, d9106, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (Dyna D9105 HW, V2.0D)",  MACHINE_NOT_WORKING )
GAME(  1991, eldoraddoc, eldoradd, eldv1, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (Dyna D9105 HW, V1.1J)",  MACHINE_NOT_WORKING )
GAME(  1991, eldoraddod, eldoradd, eldv1, aladdin, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (Dyna D9105 HW, V1.1U)",  MACHINE_NOT_WORKING )
