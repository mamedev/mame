// license:BSD-3-Clause
// copyright-holders:

/*
Astro Corp. 'Hummer' (VGA)
Main components:
Astro V102PX-0XX CPU (004 for Jack's Venture, 013 for Penguin Party)
DigiArt AM001 ADPCM & MP3 sound chip
22.579 MHz XTAL
Actel Igloo AGLP125-CSG289
Astro ROHS (GFX?)
4x LY61L25616ML-20 SRAM
2x LY621024SL-70LL SRAM
4-DIP bank

+*** ASTRO PASSWORDS ***
pengprty : Funny Penguins / Penguin Party
  → Taken from manual, Version: USA-20111216
+------------+---------+------------+---------+------------+---------+
| Password # | Code    | Password # | Code    | Password # | Code    |
+------------+---------+------------+---------+------------+---------+
| OFF (0)    | No use  | 1          | 724670  | 2          | 540810  |
| 3          | 167008  | 4          | 281630  | 5          | 403432  |
| 6          | 685640  | 7          | 961012  |            |         |
+------------+---------+------------+---------+------------+---------+

jackvent : Jack's Venture / Inca Treasure
  → Taken from manual, Version: USA-090728
+------------+---------+------------+---------+------------+---------+
| Password # | Code    | Password # | Code    | Password # | Code    |
+------------+---------+------------+---------+------------+---------+
| OFF (0)    | No use  | 1          | 548516  | 2          | 754248  |
| 3          | 936415  | 4          | 864578  | 5          | 102647  |
| 6          | 748652  | 7          | 664852  |            |         |
+------------+---------+------------+---------+------------+---------+


pengprty memory map (deduced from disassembly + execution trace):

    000000-07FFFF  ROM (decrypted program)
    800000-87FFFF  Encrypted ROM image (POST checksums against this)

    900000.w  R    Input port LOW word  (the 68k builds a 32-bit value as
                   ($902000 << 16) | $900000 and tests bits with eor.l #-1
                   to handle active-low switches)
    902000.w  R    Input port HIGH word
    904000.w  R    DIP switches (3 bits used in init: bits 4-6, masked $0070)
    906000.w  W    Output latch LOW word  (lamps/coin lockout/coin meter)
    908000.w  W    Output latch HIGH word (also bit-banged as a secondary
                   serial line for an unidentified device — bits 0/1)

    A00000-A0000F  Astro custom I/O — 4 serial input channels, 4 bytes each:
                   +1 (R.b)  channel data
                   +2 (W.w)  channel control
                   Channel 2 is initialized with ctrl = 2 (special mode);
                   the others get ctrl = 0. Polled by the input scanner at
                   $54f7c using $a00011 as a per-channel status bitmap.
    A00010   .w R/W  Astro ROHS blitter status (R) / command (W)
                     Status bits seen polled:
                       $20 (bit 5) — command done / data ready
                       $40 (bit 6) — FIFO full / blitter busy (loops while 0)
                       $80 (bit 7) — another busy/done flag
                     Commands written: #$0001, #$0003 (likely "start"/"flush")
    A00012   .w W    Sprite/blit FIFO. Sprites are stored in RAM as 4-word
                     entries starting at $e0421a; the program writes 4 words
                     per sprite with (A1)+ then waits on $a00010 bit 6.
    A00014   .w W    Blitter control register, written as #$8000 then real
                     value (probable strobe + palette/CLUT data).
    A00018-A0001F R  4 "magic ID" words, compared against a per-game table
                     ($2b34 "Pengriun"), see magic_id_r.

    A80000   .b R  External I2C EEPROM SDA in (read on odd byte $a80001)
    B80000   .w W  External I2C EEPROM bus: bit 0 = SDA, bit 1 = SCL
    E80000   .w W  Written by the bit-bang routines, purpose unknown

    B00000   .w R  Astro V102PX internal cpucode DO (bit 0)
    F00000   .w W  Astro V102PX cpucode CS (written 0 then 1 at $19776)

      The I2C read loop at $19944 samples both DO lines for each data bit:
      D2 accumulates the external EEPROM bits, D4 the cpucode bits (stored
      to $e03f3a every other byte, checked against $7547 at $1e84c).

    E00000-E0FFFF  Work RAM / NVRAM. Stack at $e0fff0.
                   $e00100..$e0013f: misc state + pointer table for I/O bases
                   $e04200..       : sprite list / scratch buffers


Interrupt usage (vector table at $00000064-$0000007c):

    Vec 25 IRQ1 → $10868: jsr $1aaba            (game tick, 60 Hz / VBLANK)
    Vec 26 IRQ2 → $10878: jsr $1ad20            (serial channel service,
                                                 $54f7c)
    Vec 27 IRQ3 → $10852: $1ad20 + $1aaba       (combined)
    Vec 28 IRQ4 → $10888: alias of IRQ2
    Vec 29 IRQ5 → $10888: alias of IRQ2
    Vec 30 IRQ6 → $1088e: alias of IRQ1
    Vec 31 IRQ7 → $1056e: spin-halt

    IRQ1 maintains $e00104..$e0010a counters (frame ticks, sleep timer).
    $1ad54 is a "wait N frames" used everywhere via pea N; jsr $1ad54.

    For now only the game tick is wired (vblank). The serial channel IRQ is
    not hooked up.


jackvent: same program structure (the I/O pointer table at $e774, the I2C /
cpucode bit-bang routines at $de18-$e08a and the descriptor builders are the
same code as pengprty's), with these address differences:

    pengprty          jackvent
    900000-908000     B80000-B88000   inputs / DSW / output latches
    A00000-A0001F     D00000-D0001F   Astro custom I/O + ROHS blitter
    B80000            C00000          serial bus (SDA/SCL)
    E00000-E0FFFF     C80000-C8FFFF   work RAM / NVRAM (stack at $c8fff0)

    A80000, B00000, E80000 and F00000 are the same on both.

    IRQ1 and IRQ2 are swapped compared to pengprty: IRQ1 ($b236 → $eb90)
    only reads the serial channels, IRQ2 ($b226 → $e98e) is the game tick
    (decrements the wait counter $c8010a); IRQ3 does both. So vblank is
    wired to IRQ2 on jackvent.

    Boot checks: external EEPROM logical word $18 (byte $30) vs. ROM table
    $2000 (0012 0023 0034 0056); cpucode vs. $fa25 at $122f4; magic ID at
    $1230c vs. $2008 (only the first word is actually compared, the loop
    never advances its index).

GFX checksum (both games): after boot the program draws the whole sprite ROM
as 40x30-tile sprites at (0,0) (pengprty $213d0, jackvent $132ca) with the
blitter in checksum mode (command $0003), then reads Astro I/O +$16 in the
IRQ and loops until it matches a ROM table (pengprty $195fc: $3b44e tiles,
$e81f; jackvent $de0e: $14325 tiles, $98b9). Both expected values are exactly
the 16-bit sum of the descrambled pixels of tiles 0 to count-1, which also
confirms the graphics descrambling. See checksum_r.

*/

#include "emu.h"

#include "astro_cpucode.h"

#include "cpu/m68000/m68000.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"

#include "screen.h"
#include "speaker.h"

#include <algorithm>
#include <vector>

#define LOG_BLIT    (1U << 1)
#define LOG_CHANNEL (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGBLIT(...)    LOGMASKED(LOG_BLIT, __VA_ARGS__)
#define LOGCHANNEL(...) LOGMASKED(LOG_CHANNEL, __VA_ARGS__)


namespace {

class hummer_state : public driver_device
{
public:
	hummer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_cpucode(*this, "astro_cpucode"),
		m_eeprom(*this, "eeprom"),
		m_gfx(*this, "sprites"),
		m_magic_id(*this, "magic_id")
	{ }

	void jackvent(machine_config &config) ATTR_COLD;
	void pengprty(machine_config &config) ATTR_COLD;

	void init_px004() ATTR_COLD { decrypt_rom(v102_px004_table); descramble_gfx(); }
	void init_px013() ATTR_COLD { decrypt_rom(v102_px013_table); descramble_gfx(); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<astro_cpucode_device> m_cpucode;
	required_device<i2cmem_device> m_eeprom;
	required_memory_region m_gfx;
	required_region_ptr<uint16_t> m_magic_id;

	struct decryption_info {
		struct {
			// Address bits used for bitswap/xor selection
			uint8_t bits[3];
			struct {
				// 8-8 Bitswap
				uint8_t bits[8];
				// Xor
				uint8_t xor_mask;
			} entries[8];
		} rom[2];
		// Global address bitswap (src -> dest, some sets use bits 12-8 only, while others 12-2)
		uint8_t bits[11];
	};

	// Astro ROHS blitter: sprite descriptors are 4 words = 8 bytes each. The
	// CPU streams them to $a00012 (4 word writes per sprite) and polls $a00010
	// bit 6 for FIFO-busy. Format derived from the sprite-builder routine at
	// $202ce / $20bd4:
	//
	//   word 0  attributes, OR'd with global flags at $e080be (see also
	//           $20c82 which sets bit 10 = $0400). Bits 2-0 are bits 18-16
	//           of the tile number: jackvent's builder at $12e2a takes them
	//           from (code & $70000) >> 16; pengprty's sprite tables (e.g.
	//           $10894, $151bc, $17ff4) only use 0-3 there. Sprites of both
	//           games decode correctly with tile = (w0 & 7) << 16 | w1.
	//           Other bits (e.g. $38 on the screen-clear columns) unknown.
	//   word 1  tile number, bits 15-0 (16x16 tiles, see descramble_gfx)
	//   word 2  bits 15:10 = X size in 16-pixel tiles (6 bits, max 0x3f)
	//           bits  9: 0 = X position on screen (10 bits, max $3ff)
	//   word 3  bits 15:10 = Y size in 16-pixel tiles
	//           bits  9: 0 = Y position
	//           bit  10    = forced to 1 by $20c82 — meaning unclear
	//                        (depth/transparency/layer enable?)
	//
	// Descriptors are collected in m_sprite_list as they come in; the
	// end-of-list descriptor (W0 bit 15, see rohs_data_w) copies them to
	// m_disp_list, which screen_update rasterizes. The real blitter
	// presumably draws into a frame buffer instead.
	static constexpr uint32_t MAX_SPRITES = 0x800;

	static const decryption_info v102_px004_table;
	static const decryption_info v102_px013_table;
	static const uint8_t gfx_bitswap[8][16];

	uint32_t m_output_latch = 0; // $906000 (low) + $908000 (high)
	uint16_t m_blitter_ctrl = 0; // $a00014

	uint16_t m_sprite_list[MAX_SPRITES][4] = {};
	uint16_t m_sprite_count = 0;
	uint16_t m_disp_list[MAX_SPRITES][4] = {};
	uint16_t m_disp_count = 0;
	uint8_t m_sprite_word = 0; // 0..3, position within the descriptor being received
	uint16_t m_cur_desc[4] = { }; // descriptor being received

	// GFX checksum, see checksum_r
	std::vector<uint16_t> m_tile_sum; // 16-bit sum of the pixels of each tile
	uint16_t m_checksum = 0;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// output latch
	uint16_t output_lo_r();
	uint16_t output_hi_r();
	void output_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void output_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	// Astro ROHS blitter (stub)
	uint16_t rohs_status_r();
	void rohs_cmd_w(uint16_t data);
	void rohs_data_w(uint16_t data);
	void rohs_ctrl_w(uint16_t data);
	uint16_t checksum_r();

	// external I2C EEPROM and internal cpucode (pengprty addresses)
	void eeprom_clk_w(uint16_t data); // $b80000 I2C SDA / SCL
	void int_eeprom_cs_w(uint16_t data); // $f00000 cpucode CS
	uint16_t ext_eeprom_data_r(); // $a80000 I2C SDA in (bit 0)
	uint16_t int_eeprom_data_r(); // $b00000 cpucode DO (bit 0)

	// Astro custom I/O — 4 serial channel block at $a00000-$a0000f
	uint8_t channel_data_r(offs_t offset); // odd byte at +1/+5/+9/+d
	uint16_t channel_ctrl_r(); // RMW read side of clr.w on ctrl word
	void channel_ctrl_w(offs_t offset, uint16_t data);
	uint16_t magic_id_r(offs_t offset); // $a00018-$a0001f — see implementation

	void jackvent_map(address_map &map) ATTR_COLD;
	void pengprty_map(address_map &map) ATTR_COLD;
	void astro_io_map(address_map &map) ATTR_COLD;

	void decrypt_rom(const decryption_info &table) ATTR_COLD;
	void descramble_gfx() ATTR_COLD;

	void hummer(machine_config &config) ATTR_COLD;
};


uint32_t hummer_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	// "sprites" holds linear 16x16 tiles of xRGB_555 after descramble_gfx():
	// tile n at word n * 256, row-major. Bit 15 set = transparent pixel.
	uint16_t const *const gfx = &m_gfx->as_u16();
	const uint32_t tiles = m_gfx->bytes() / 512;

	// TODO: positions are drawn as plain 10-bit values (no wraparound), and
	// the W0 bits other than the tile bank are ignored (flip, blending, the
	// $38 used by the screen-clear columns, ...)
	for (uint32_t i = 0; i < m_disp_count; i++)
	{
		const uint16_t w0 = m_disp_list[i][0];
		const uint16_t w1 = m_disp_list[i][1];
		const uint16_t w2 = m_disp_list[i][2];
		const uint16_t w3 = m_disp_list[i][3];

		const int xsize = (w2 >> 10) & 0x3f;
		const int xpos =  w2 & 0x3ff;
		const int ysize = (w3 >> 10) & 0x3f;
		const int ypos =  w3 & 0x3ff;

		uint32_t tile = (uint32_t(w0 & 0x0007) << 16) | w1;

		for (int ty = 0; ty < ysize; ty++)
		{
			for (int tx = 0; tx < xsize; tx++, tile++)
			{
				uint16_t const *const src = &gfx[(tile % tiles) * 256];

				for (int py = 0; py < 16; py++)
				{
					const int sy = ypos + ty * 16 + py;

					if (sy < cliprect.top() || sy > cliprect.bottom())
						continue;

					uint32_t *const dst = &bitmap.pix(sy);

					for (int px = 0; px < 16; px++)
					{
						const int sx = xpos + tx * 16 + px;

						if (sx < cliprect.left() || sx > cliprect.right())
							continue;

						const uint16_t pix = src[py * 16 + px];

						if (!BIT(pix, 15))
							dst[sx] = rgb_t(pal5bit(pix >> 10), pal5bit(pix >> 5), pal5bit(pix));
					}
				}
			}
		}
	}

	return 0;
}


void hummer_state::machine_start()
{
	save_item(NAME(m_output_latch));
	save_item(NAME(m_blitter_ctrl));
	save_item(NAME(m_sprite_list));
	save_item(NAME(m_sprite_count));
	save_item(NAME(m_sprite_word));
	save_item(NAME(m_disp_list));
	save_item(NAME(m_disp_count));
	save_item(NAME(m_cur_desc));
	save_item(NAME(m_checksum));
}

void hummer_state::machine_reset()
{
	m_output_latch = 0;
	m_blitter_ctrl = 0;
	m_sprite_count = 0;
	m_sprite_word = 0;
	m_disp_count = 0;
	m_checksum = 0;
}


// -----------------------------------------------------------------------
// Output latch
// -----------------------------------------------------------------------

void hummer_state::output_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// $906000 / $b86000 — low word of 32-bit output latch. Written every
	// frame from IRQ1 (pengprty $1a90c) with bit 31 ($80000000) =
	// generic-error and bit 28 ($10000000) = waiting flags ORed in. Drives
	// lamps, coin lockout, coin meter, hopper.
	m_output_latch = (m_output_latch & 0xffff0000u) | uint32_t(data & mem_mask);
}

void hummer_state::output_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// $908000 / $b88000 — high word of the same latch.
	// NOTE: bits 0 and 1 of this register are ALSO bit-banged as a secondary
	// serial line by routine $19a66 (clock/data for an unidentified device,
	// possibly a security chip or RTC). Shadow state lives in $e080d2.
	m_output_latch = (m_output_latch & 0x0000ffffu) | (uint32_t(data & mem_mask) << 16);
}

uint16_t hummer_state::output_lo_r()
{
	// Read-back side of the output latch. The init code at $19dfa does
	// "clr.w $906000" which the 68000 implements as a read-modify-write,
	// so this port has to be readable too.
	return uint16_t(m_output_latch & 0xffff);
}

uint16_t hummer_state::output_hi_r()
{
	// Same rationale as output_lo_r — clr.w $908000 at $19e00 does an RMW.
	return uint16_t((m_output_latch >> 16) & 0xffff);
}


// Astro ROHS blitter (stub)

uint16_t hummer_state::rohs_status_r()
{
	// Astro I/O +$10 status — return all-zero so the FIFO never appears full and the
	// program can stream sprite descriptors without blocking. Real semantics:
	//   bit 5 ($20) — command/data ready
	//   bit 6 ($40) — FIFO full / blitter busy   (the program waits here)
	//   bit 7 ($80) — secondary busy
	return 0;
}

void hummer_state::rohs_cmd_w(uint16_t data)
{
	// Astro I/O +$10 command, $0001 or $0003 ($0003 while the program is
	// checksumming the graphics). Meaning unknown, so it does nothing here:
	// - pengprty sends it once per frame in the vblank service, right after
	//   the end-of-list descriptor ($1fe2e);
	// - jackvent also sends it right after the frame's descriptors
	//   ($12d04 / $12d36), before the end-of-list descriptor that its vblank
	//   service sends with another command ($12b94).
	// So it isn't the end of the frame's list; the end-of-list descriptor
	// is (see rohs_data_w).
	LOGBLIT("%s: ROHS cmd %04x (sprite_count=%u so far)\n",
			machine().describe_context(), data, m_sprite_count);
}

void hummer_state::rohs_data_w(uint16_t data)
{
	// Astro I/O +$12 — sprite/blit FIFO. Each descriptor is 4 words (see comments
	// next to m_sprite_list above). Log each completed descriptor with a
	// fully decoded breakdown of all fields, so we can spot how the program
	// uses the bit-fields in practice (e.g. attribute bits used for palette /
	// flip / depth, code-bank bits in W0, etc.). Capped to MAX_LOGGED entries
	// per frame to keep traces manageable.
	static constexpr uint32_t MAX_LOGGED = 50;

	m_cur_desc[m_sprite_word] = data;

	if (++m_sprite_word < 4)
		return;

	m_sprite_word = 0;

	const uint16_t w0 = m_cur_desc[0];
	const uint16_t w1 = m_cur_desc[1];
	const uint16_t w2 = m_cur_desc[2];
	const uint16_t w3 = m_cur_desc[3];

	const uint32_t xsize = (w2 >> 10) & 0x3f;
	const uint32_t xpos = w2 & 0x3ff;
	const uint32_t ysize = (w3 >> 10) & 0x3f;
	const uint32_t ypos = w3 & 0x3ff;
	const uint32_t tile = (uint32_t(w0 & 7) << 16) | w1;

	// End-of-list descriptor: W0 bit 15 is only set in $8038 $0001 $07x0 $0400,
	// that both games send once per frame from the vblank service (pengprty
	// $1fe2e, jackvent $12b94). It ends the frame: the list goes to the
	// display and the checksum restarts.
	if (BIT(w0, 15))
	{
		std::copy_n(&m_sprite_list[0][0], m_sprite_count * 4, &m_disp_list[0][0]);

		m_disp_count = m_sprite_count;

		m_sprite_count = 0;
		m_checksum = 0;

		return;
	}

	// the checksum covers every descriptor received since the last end of list
	const uint32_t tiles = m_tile_sum.size();

	for (uint32_t i = 0; i < xsize * ysize; i++)
		m_checksum += m_tile_sum[(tile + i) % tiles];

	if (m_sprite_count < MAX_LOGGED)
	{
		LOGBLIT("spr#%3u  W0=%04x W1=%04x W2=%04x W3=%04x"
				"  tile=%05x  pos=(%4u,%4u)  size=%2ux%2u (tiles)"
				"  attr=%04x w3.b10=%u\n",
				m_sprite_count, w0, w1, w2, w3, tile,
				xpos, ypos, xsize, ysize,
				w0, BIT(w3, 10));
	}

	if (m_sprite_count < MAX_SPRITES)
	{
		std::copy(std::begin(m_cur_desc), std::end(m_cur_desc), std::begin(m_sprite_list[m_sprite_count]));

		m_sprite_count++;
	}
}

uint16_t hummer_state::checksum_r()
{
	// Astro I/O +$16. 16-bit sum of the source pixels of the sprites sent
	// since the last end-of-list descriptor; the expected values of the GFX
	// checksum check of both games match this sum (see header).
	// Both games read it in the vblank service (pengprty $21398, jackvent
	// $13288) after waiting for status bit 7 to clear and before sending the
	// end-of-list descriptor; jackvent has already sent the command by then
	// ($12d04), pengprty not yet. So the command can't clear the sum.
	// Unknown: whether bit 1 of the command ($0003) affects the sum, and
	// what exactly clears it on the hardware.
	return m_checksum;
}

void hummer_state::rohs_ctrl_w(uint16_t data)
{
	// Astro I/O +$14 — control register. Often written as $8000 (strobe) then a
	// "real" value. Log every value so we can correlate with sprite-format
	// changes; this is the most likely register to hold the bpp / palette /
	// banking selector that drives the GFX ROM decode.
	LOGBLIT("CTRL $a00014 = %04x (prev=%04x)\n", data, m_blitter_ctrl);

	m_blitter_ctrl = data;
}


// -----------------------------------------------------------------------
// External I2C EEPROM and internal cpucode
// -----------------------------------------------------------------------

void hummer_state::eeprom_clk_w(uint16_t data)
{
	// $b80000 (pengprty) / $c00000 (jackvent) — external I2C EEPROM bus:
	//   bit 0 = SDA (I2C data, bidirectional)
	//   bit 1 = SCL
	//   bit 2 = (held high during a transfer; not needed by the I2C model)
	//
	// The EEPROM is an I2C 24C64: every transaction begins with the control
	// byte $a0 (write) / $a1 (read), each byte is followed by an ACK, and
	// reads use a 16-bit word address.
	m_eeprom->write_sda(BIT(data, 0));
	m_eeprom->write_scl(BIT(data, 1));
}

void hummer_state::int_eeprom_cs_w(uint16_t data)
{
	// $f00000 — CS for the internal Astro cpucode, active low.
	// $19776 writes 0 then 1 here at the start of each transaction.
	m_cpucode->cs_write(BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
}

uint16_t hummer_state::ext_eeprom_data_r()
{
	// $a80000 — the program reads the I2C SDA line here (btst #0 on $a80001).
	// Return the device's SDA state in bit 0; the wired-AND with the master's
	// SDA is handled inside i2cmem::read_sda().
	return m_eeprom->read_sda() & 1;
}

uint16_t hummer_state::int_eeprom_data_r()
{
	// $b00000 — bit 0 = DO from the Astro cpucode, other bits read as 1
	const uint16_t data = 0xfffe | m_cpucode->do_read();

	m_cpucode->next_bit();

	return data;
}


// -----------------------------------------------------------------------
// Astro custom I/O — $a00000-$a0001f (per-channel + global regs)
// -----------------------------------------------------------------------

uint8_t hummer_state::channel_data_r(offs_t offset)
{
	// 4 serial input channels read at byte offsets +1/+5/+9/+D.
	// Channel 2 is initialized with ctrl=2 ("special mode") and is most
	// likely the touchscreen / coin acceptor link.
	if (!machine().side_effects_disabled())
		LOGCHANNEL("%s: channel data r @ %04x\n", machine().describe_context(), offset);

	return 0xff;
}

void hummer_state::channel_ctrl_w(offs_t offset, uint16_t data)
{
	// 4 channel control words at +2/+6/+A/+E. Init at $54b92 writes 0 to
	// channels 0/1/3 and 2 to channel 2.
	LOGCHANNEL("%s: channel ctrl w @ %04x = %04x\n",
			machine().describe_context(), offset, data);
}

uint16_t hummer_state::channel_ctrl_r()
{
	// $54b92 does clr.w on $a00002/$a00006/$a0000e which the 68000 implements
	// as a read-modify-write. Returning 0 keeps the RMW path quiet without
	// affecting the subsequent write.
	return 0;
}

uint16_t hummer_state::magic_id_r(offs_t offset)
{
	// +$18-$1f of the Astro custom I/O block. After the cpucode check the
	// program reads these four words and compares them against a table in
	// program ROM, halting on mismatch (pengprty $1e866 vs. $2b34
	// "Pengriun", jackvent $1230c vs. $2008). The tables differ per game, so
	// whatever the chip returns is game specific; what it really is (fixed
	// fingerprint, or something latched at boot) is unknown. For now return
	// the expected values, from the "magic_id" region.
	return m_magic_id[offset & 3];
}


// -----------------------------------------------------------------------
// Memory maps
// -----------------------------------------------------------------------

void hummer_state::jackvent_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x800000, 0x87ffff).rom().region("encrypted_rom", 0); // POST checks for encrypted ROM checksum here

	// 32-bit input port pair (low/high words), DIP switches, 32-bit output latch
	map(0xb80000, 0xb80001).portr("IN0");
	map(0xb82000, 0xb82001).portr("IN1");
	map(0xb84000, 0xb84001).portr("DSW");
	map(0xb86000, 0xb86001).rw(FUNC(hummer_state::output_lo_r), FUNC(hummer_state::output_lo_w));
	map(0xb88000, 0xb88001).rw(FUNC(hummer_state::output_hi_r), FUNC(hummer_state::output_hi_w));

	// external I2C EEPROM and internal cpucode
	map(0xa80000, 0xa80001).r(FUNC(hummer_state::ext_eeprom_data_r));
	map(0xb00000, 0xb00001).r(FUNC(hummer_state::int_eeprom_data_r));
	map(0xc00000, 0xc00001).w(FUNC(hummer_state::eeprom_clk_w));
	map(0xe80000, 0xe80001).nopw(); // written by the bit-bang routines, purpose unknown
	map(0xf00000, 0xf00001).w(FUNC(hummer_state::int_eeprom_cs_w));

	map(0xc80000, 0xc8ffff).ram().share("nvram"); // stack at $c8fff0

	map(0xd00000, 0xd0001f).m(FUNC(hummer_state::astro_io_map));
}

void hummer_state::pengprty_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x800000, 0x87ffff).rom().region("encrypted_rom", 0); // POST checks for encrypted ROM checksum here

	// 32-bit input port pair (low/high words), DIP switches, 32-bit output
	// latch. The latches need rw() because the boot init at $19dfa does
	// "clr.w" which the 68000 implements as RMW.
	map(0x900000, 0x900001).portr("IN0");
	map(0x902000, 0x902001).portr("IN1");
	map(0x904000, 0x904001).portr("DSW");
	map(0x906000, 0x906001).rw(FUNC(hummer_state::output_lo_r), FUNC(hummer_state::output_lo_w));
	map(0x908000, 0x908001).rw(FUNC(hummer_state::output_hi_r), FUNC(hummer_state::output_hi_w));

	// external I2C EEPROM and internal cpucode
	map(0xa80000, 0xa80001).r(FUNC(hummer_state::ext_eeprom_data_r));
	map(0xb00000, 0xb00001).r(FUNC(hummer_state::int_eeprom_data_r));
	map(0xb80000, 0xb80001).w(FUNC(hummer_state::eeprom_clk_w));
	map(0xe80000, 0xe80001).nopw(); // written by the bit-bang routines, purpose unknown
	map(0xf00000, 0xf00001).w(FUNC(hummer_state::int_eeprom_cs_w));

	map(0xa00000, 0xa0001f).m(FUNC(hummer_state::astro_io_map));

	map(0xe00000, 0xe0ffff).ram().share("nvram"); // stack at $e0fff0
}

// Astro custom I/O + ROHS blitter, same layout on both games
void hummer_state::astro_io_map(address_map &map)
{
	// 4 serial input channels (bill validator / coin / touch)
	map(0x01, 0x01).r(FUNC(hummer_state::channel_data_r));
	map(0x02, 0x03).rw(FUNC(hummer_state::channel_ctrl_r), FUNC(hummer_state::channel_ctrl_w));
	map(0x05, 0x05).r(FUNC(hummer_state::channel_data_r));
	map(0x06, 0x07).rw(FUNC(hummer_state::channel_ctrl_r), FUNC(hummer_state::channel_ctrl_w));
	map(0x09, 0x09).r(FUNC(hummer_state::channel_data_r));
	map(0x0a, 0x0b).rw(FUNC(hummer_state::channel_ctrl_r), FUNC(hummer_state::channel_ctrl_w));
	map(0x0d, 0x0d).r(FUNC(hummer_state::channel_data_r));
	map(0x0e, 0x0f).rw(FUNC(hummer_state::channel_ctrl_r), FUNC(hummer_state::channel_ctrl_w));

	// ROHS blitter
	map(0x10, 0x11).rw(FUNC(hummer_state::rohs_status_r), FUNC(hummer_state::rohs_cmd_w));
	map(0x12, 0x13).w(FUNC(hummer_state::rohs_data_w));
	map(0x14, 0x15).w(FUNC(hummer_state::rohs_ctrl_w));
	map(0x16, 0x17).r(FUNC(hummer_state::checksum_r));

	map(0x18, 0x1f).r(FUNC(hummer_state::magic_id_r));
}


static INPUT_PORTS_START( hummer )
	// Names from the I/O test: labels at pengprty $a924 / jackvent $62c4,
	// masks at $aa24 / $63c4, tested against the inverted 32-bit input
	// (pengprty $3c06e, jackvent $4189c). Bit 11 is "COLLECT" in pengprty,
	// "PAY OUT" in jackvent. The polarity of the sensors (hopper, ticket,
	// door) is unknown.
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Info")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double / Select Lines")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Auto Play")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ticket Switch")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Switch")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_DOOR )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Account 1")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Account 2")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ticket") PORT_CODE(KEYCODE_T)

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// 4-DIP bank on the PCB. Init code at $1fb68 isolates bits 4-6 ($70 mask)
	// of the inverted read; meaning of individual switches not yet known.
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, 0x0020, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, 0x0040, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW1:4" )
	PORT_BIT( 0xff0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void hummer_state::hummer(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 22.579_MHz_XTAL / 2);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Astro V102PX internal cpucode protection
	ASTRO_CPUCODE(config, m_cpucode);

	// External serial EEPROM — an I2C 24C64 (8 Kbyte). Identified by sniffing
	// the serial bus on pengprty: transactions use I2C control bytes $a0/$a1
	// (the "1010" device prefix + R/W bit), per-byte ACKs, and 16-bit word
	// addressing. jackvent uses the same bit-bang code.
	//
	// The boot code validates 4 words against a ROM table and errors out on
	// mismatch (see init_pengprty / init_jackvent). Default contents come
	// from the "eeprom" region.
	I2C_24C64(config, m_eeprom);

	// video hardware
	// 16x16 tiles; jackvent's full-screen layouts are 40x30 tiles (640x480),
	// pengprty's 50x38 (800x608, see pengprty()).
	// TODO: real timings unknown
	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 639, 0, 479);
	m_screen->set_screen_update(FUNC(hummer_state::screen_update));

	// (No palette device: graphics are direct xRGB_555, see descramble_gfx.)

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// AM001 for sound
	// TODO: the serial channel IRQ (pengprty IRQ2, jackvent IRQ1) isn't hooked up
}

void hummer_state::jackvent(machine_config &config)
{
	hummer(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hummer_state::jackvent_map);

	// game tick ($b226 → $e98e, decrements the wait counter at $c8010a)
	// TODO: how the IRQ is acknowledged is unknown
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_2, HOLD_LINE);
}

void hummer_state::pengprty(machine_config &config)
{
	hummer(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hummer_state::pengprty_map);

	// The program's full screen layouts are 800 pixels wide (e.g. the frame
	// at $13e3c: 50 x 38 tiles, the last 8 lines transparent), jackvent's
	// are 640 x 480. What selects the resolution (PCB revision, a register
	// not identified yet) is unknown.
	m_screen->set_size(800, 600);
	m_screen->set_visarea(0, 799, 0, 599);

	// game tick ($10868 → $1aaba, decrements the wait counter at $e0010a)
	// TODO: how the IRQ is acknowledged is unknown
	m_screen->screen_vblank().set_inputline(m_maincpu, M68K_IRQ_1, HOLD_LINE);
}


ROM_START( pengprty ) // PCBHR REV:E + Flash Card V1.1 riser board for GFX ROMs + CS350P093 TSOP to DIP riser board for sound ROM
	ROM_REGION16_BE( 0x80000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION16_BE( 0x80000, "encrypted_rom", 0 )
	ROM_LOAD16_BYTE( "2-tm_fpus01.01b.tu3", 0x00000, 0x40000, CRC(23cda107) SHA1(f2c5cba9a3c2c8bfea6bce0b221fd9209810fdf3) )
	ROM_LOAD16_BYTE( "1-tm_fpus01.01b.tu1", 0x00001, 0x40000, CRC(2569e2b9) SHA1(dcec1e9bfe73a062b891812f2c8eb8407066b993) )

	ROM_REGION( 0x8000000, "sprites", 0 ) // two banks of 4 parallel chips, scrambled, see descramble_gfx
	ROM_LOAD( "mx29gl128eh.u1", 0x0000000, 0x1000000, CRC(80f0d70f) SHA1(82de9bb82a2c5901d5e2dc8f93cd2eee5d65a20b) )
	ROM_LOAD( "mx29gl128eh.u2", 0x1000000, 0x1000000, CRC(9a9da6b4) SHA1(41d95e2de41a99c172ba210e51170e14219d393e) )
	ROM_LOAD( "mx29gl128eh.u3", 0x2000000, 0x1000000, CRC(458f22e5) SHA1(cf3b3084a980568646c588d33123576bd25261a5) )
	ROM_LOAD( "mx29gl128eh.u4", 0x3000000, 0x1000000, CRC(91f1069c) SHA1(67c7e56345f63c01279d39527b31fa9eeb7b4bf0) )
	ROM_LOAD( "mx29gl128eh.u5", 0x4000000, 0x1000000, CRC(fd4bc0d5) SHA1(0829ed0762311b3afd3b2a86c128077138793b53) )
	ROM_LOAD( "mx29gl128eh.u6", 0x5000000, 0x1000000, CRC(2280b3db) SHA1(601a7c1a7b868a0bb9395f38999426b45f008199) )
	ROM_LOAD( "mx29gl128eh.u7", 0x6000000, 0x1000000, CRC(8cfc06f9) SHA1(cc4386c3cde41145672102e3e29cb5c949246f62) )
	ROM_LOAD( "mx29gl128eh.u8", 0x7000000, 0x1000000, CRC(83907de9) SHA1(480df6092e9a78da69b220bfd0f2727a58d677c0) )

	ROM_REGION( 0x800000, "am001", 0 )
	ROM_LOAD( "mx29lv640eb.u53", 0x000000, 0x800000, CRC(0289fef0) SHA1(f349abd69fcbb9b92ba1362f01c3a83a521fdcc3) )

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "pengprty_cpucode.key", 0x00, 0x02, CRC(de8e4e8e) SHA1(4e76359310290f00157b30e332792025a4d70835) ) // value checked at $1e84c

	ROM_REGION16_BE( 0x08, "magic_id", 0 )
	ROM_LOAD( "pengprty_magic_id", 0x00, 0x08, CRC(8e14d3ce) SHA1(3b3100bbcd1c62a2ca4c28fcb6d04f16c05cc7f6) ) // values checked at $1e866 (table at $2b34)

	ROM_REGION( 0x2000, "eeprom", 0 )
	ROM_LOAD( "pengprty_eeprom", 0x0000, 0x02000, CRC(68aa8a90) SHA1(389323361d49c281a63118f7a611a652748a3896) ) // factory default
ROM_END

ROM_START( jackvent ) // PCBHR REV:C + Flash Card V1.1 riser board for GFX ROMs + CS350P093 TSOP to DIP riser board for sound ROM
	ROM_REGION16_BE( 0x80000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION16_BE( 0x80000, "encrypted_rom", 0 )
	ROM_LOAD16_BYTE( "2_jvus02.01a.tu3", 0x00000, 0x40000, CRC(945a01b1) SHA1(e621c1dd0db573dd5e3bc2202b04c997d84af4fc) )
	ROM_LOAD16_BYTE( "1_jvus02.01a.tu1", 0x00001, 0x40000, CRC(71471ff7) SHA1(b342d93417b9f8d2e5e36152a31acb09b6a5acd3) )

	ROM_REGION( 0x4000000, "sprites", 0 ) // two banks of 4 parallel chips, scrambled, see descramble_gfx
	ROM_LOAD( "29lv640.u1", 0x0000000, 0x0800000, CRC(5be4c27d) SHA1(f16bb283e7d28148efacae7d42091985d96825b8) )
	ROM_LOAD( "29lv640.u2", 0x0800000, 0x0800000, CRC(1dd17d97) SHA1(a8e3b9f47bc8cf85f1008e16223245bbe6dc7f79) )
	ROM_LOAD( "29lv640.u3", 0x1000000, 0x0800000, CRC(970f6dd2) SHA1(b5db6c64e3ad6c8b0c0014b9cbdc2efadeb9b900) )
	ROM_LOAD( "29lv640.u4", 0x1800000, 0x0800000, CRC(d92453da) SHA1(612aacbb6724c195cf28ed25a73a6220c2e1fc32) )
	ROM_LOAD( "29lv640.u5", 0x2000000, 0x0800000, CRC(113218d6) SHA1(1182fe1ce29fbbbbfdd7ea0a5f6f6f85eab7acc6) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u6", 0x2800000, 0x0800000, CRC(e8e06adb) SHA1(4c7871c405d6caa5ab5516410f730017c713984d) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u7", 0x3000000, 0x0800000, CRC(ef777222) SHA1(5a53c72584c1a8b098df085bade866d887f48b29) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u8", 0x3800000, 0x0800000, CRC(a48acc31) SHA1(4d35fd060dc86a10bd69715e17a9399ff97e4343) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "am001", 0 )
	ROM_LOAD( "29lv640.u53", 0x000000, 0x800000, CRC(c891b5ff) SHA1(5e0dce5b33230bd181f3eed94f64da328d11be28) )

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "jackvent_cpucode.key", 0x00, 0x02, CRC(f442d91a) SHA1(c6286f7cc262e52f35472a6c340f202cfbba6094) ) // value checked at $122f4

	ROM_REGION16_BE( 0x08, "magic_id", 0 )
	ROM_LOAD( "jackvent_magic_id", 0x00, 0x08, CRC(f33f0a7a) SHA1(0ec1540e4e60c4eff99393ae5148660987e50336) ) // table at $2008, only the first word is checked ($1230c)

	ROM_REGION( 0x2000, "eeprom", 0 )
	ROM_LOAD( "jackvent_eeprom", 0x0000, 0x02000, CRC(bc6d8465) SHA1(6aa05b443d5d68db609e88d8fc75d285dcde40be) ) // factory default
ROM_END


void hummer_state::decrypt_rom(const decryption_info &table)
{
	const uint32_t size = memregion("maincpu")->bytes();
	uint16_t const* const rom = &memregion("encrypted_rom")->as_u16();
	uint16_t *const decrypted = &memregion("maincpu")->as_u16();
	std::unique_ptr<uint16_t[]> tmp = std::make_unique<uint16_t []>(size/2);

	// Pass 1: decrypt high and low byte independently.  They go
	// through a bitswap and an xor, choosing between 8 possibilities
	// through address bits.

	for (uint32_t i = 0; i != size; i += 2) {
		uint16_t orig = rom[i >> 1];
		uint16_t result = 0;
		for (uint32_t rb = 0; rb < 2; rb ++) {
			uint8_t val = orig >> (rb ? 0 : 8);
			uint32_t index =
				(BIT(i, table.rom[rb].bits[0]) << 2) |
				(BIT(i, table.rom[rb].bits[1]) << 1) |
				BIT(i, table.rom[rb].bits[2]);
			val = bitswap(val,
						  table.rom[rb].entries[index].bits[0],
						  table.rom[rb].entries[index].bits[1],
						  table.rom[rb].entries[index].bits[2],
						  table.rom[rb].entries[index].bits[3],
						  table.rom[rb].entries[index].bits[4],
						  table.rom[rb].entries[index].bits[5],
						  table.rom[rb].entries[index].bits[6],
						  table.rom[rb].entries[index].bits[7]);
			val = val ^ table.rom[rb].entries[index].xor_mask;

			result |= val << (rb ? 0 : 8);
		}
		tmp[i >> 1] = result;
	}

	// Pass 2: copy back the decrypted data following the address
	// scrambling
	for (uint32_t i = 0; i != size; i += 2) {
		uint32_t dest =
			(i & 0xffffe003) |
			(BIT(i, table.bits[0])  << 12) |
			(BIT(i, table.bits[1])  << 11) |
			(BIT(i, table.bits[2])  << 10) |
			(BIT(i, table.bits[3])  <<  9) |
			(BIT(i, table.bits[4])  <<  8) |
			(BIT(i, table.bits[5])  <<  7) |
			(BIT(i, table.bits[6])  <<  6) |
			(BIT(i, table.bits[7])  <<  5) |
			(BIT(i, table.bits[8])  <<  4) |
			(BIT(i, table.bits[9])  <<  3) |
			(BIT(i, table.bits[10]) <<  2);
		decrypted[dest >> 1] = tmp[i >> 1];
	}

	// TODO: are these overlays provided by the custom CPU? It does seem so.
	// both the encrypted and decrypted bytes at these positions make no sense.
	// gostopac explicitly checks that 0x4-0x7 are 0x0400'0400
	decrypted[0x00004/2] = 0x0400;
	decrypted[0x00006/2] = 0x0400;
}

// TODO: this is the same as v102_px001_table, shouldn't be?
const hummer_state::decryption_info hummer_state::v102_px004_table = {
	{
		{
			{ 8, 9, 10 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 9, 11 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 10, 8, 11, 9, 7, 2, 4, 6, 5, 3 }
};

const hummer_state::decryption_info hummer_state::v102_px013_table = {
	{
		{
			{ 8, 9, 10 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 9, 11 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 10, 8, 11, 9, 7, 6, 5, 4, 3, 2 }
};

/*
    Graphics ROMs (pengprty, jackvent)

    Derived from the ROM contents alone (no PCB access), checked on both games
    by decoding the sprites listed in the program's own sprite tables:

    - u1-u4 and u5-uint8_t form two banks; within a bank the four 16-bit chips are
      read in parallel and chip n holds the pixels with x & 3 == n. u5-uint8_t
      end with the same amount of $ff (2 MB pengprty, 5 MB jackvent),
      consistent with this.
    - Graphics are 16x16 tiles of 16-bit pixels, 512 bytes per tile, one
      tile per 128 bytes of chip (0x20000 tiles per bank on pengprty, 0x10000
      on jackvent). Tile number = (sprite word 0 & 7) << 16 | word 1.
    - Inside a bank, word address bits (word = chip address * 4 + chip):
        0-1  x0-x1 (chip)        2    x2           3    x3
        4-6  y0-y2               7    y3 ^ x3      8    x2 ^ tile bit 0
        9+   tile bits 1+
    - The 16 data bits are permuted depending on x & 3 and on tile bit 0
      (table below; no XOR). After that the pixel is xRGB_555 with bit 15 set
      for transparent pixels.
*/
const uint8_t hummer_state::gfx_bitswap[8][16] =
{
	{ 15, 11, 10,  9,  8, 14, 13, 12,  5,  4,  3,  2,  1,  0,  7,  6 }, // even tile, x & 3 == 0
	{ 15, 13, 14, 11, 12,  9, 10,  7,  8,  5,  6,  3,  4,  1,  2,  0 }, // even tile, x & 3 == 1
	{ 15,  1,  0, 14, 13, 12, 11,  3,  2,  7,  6,  5, 10,  9,  8,  4 }, // even tile, x & 3 == 2
	{ 11, 15, 14, 13, 12,  6,  5,  4,  3,  2,  1,  0, 10,  9,  8,  7 }, // even tile, x & 3 == 3
	{  1,  0, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2 }, // odd tile,  x & 3 == 0
	{  7,  6,  5,  4,  3,  2,  1,  0, 11, 10,  9,  8, 14, 13, 12, 15 }, // odd tile,  x & 3 == 1
	{ 14, 15, 13,  1,  0,  3,  2, 12, 11, 10,  9,  8,  7,  6,  5,  4 }, // odd tile,  x & 3 == 2
	{  7,  6,  5,  4,  3,  2, 15, 14, 13, 12, 11, 10,  9,  1,  0,  8 }, // odd tile,  x & 3 == 3
};

void hummer_state::descramble_gfx()
{
	memory_region *const region = memregion("sprites");
	uint8_t *const rom = region->base();
	const uint32_t size = region->bytes();
	const uint32_t chip_size = size / 8;
	const uint32_t bank_words = chip_size * 2;  // 4 chips of chip_size / 2 words
	const std::vector<uint8_t> buffer(&rom[0], &rom[size]);
	uint16_t *const dst = &region->as_u16();

	for (uint32_t bank = 0; bank < 2; bank++)
	{
		for (uint32_t i = 0; i < bank_words; i++)
		{
			const uint32_t src = (bank * 4 + (i & 3)) * chip_size + (i >> 2) * 2;
			const uint16_t data = buffer[src] | (buffer[src + 1] << 8);

			const uint32_t odd = ((i >> 2) ^ (i >> 8)) & 1;
			const uint8_t *const b = gfx_bitswap[(odd << 2) | (i & 3)];
			const uint16_t pix = bitswap<16>(data,
					b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
					b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);

			const uint32_t tile = bank * (bank_words / 256) + ((i >> 9) << 1) + odd;
			const uint32_t x = i & 0x0f;
			const uint32_t y = ((i >> 4) & 0x07) | ((((i >> 7) ^ (i >> 3)) & 1) << 3);
			dst[tile * 256 + y * 16 + x] = pix;
		}
	}

	// per-tile pixel sums for the blitter checksum (see checksum_r)
	m_tile_sum.resize(size / 512);
	for (uint32_t t = 0; t < m_tile_sum.size(); t++)
	{
		uint16_t sum = 0;
		for (uint32_t p = 0; p < 256; p++)
			sum += dst[t * 256 + p];
		m_tile_sum[t] = sum;
	}
}

} // anonymous namespace


GAME ( 2012,  pengprty, 0, pengprty, hummer, hummer_state, init_px013, ROT0, "Astro Corp.", "Penguin Party (US-TM.01.01.B, 2012/01/16)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME ( 2010,  jackvent, 0, jackvent, hummer, hummer_state, init_px004, ROT0, "Astro Corp.", "Jack's Venture - Inca Treasure (US.02.01.A, 2010/01/21)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
