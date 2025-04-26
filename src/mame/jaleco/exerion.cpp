// license:BSD-3-Clause
// copyright-holders: Aaron Giles

/***************************************************************************

    Jaleco Exerion hardware
    Jaleco EX-8313 + EX-8315 PCBs

****************************************************************************

    Exerion is a unique driver in that it has idiosyncrasies that are straight
    out of Bizarro World. I submit for your approval:

    * The mystery reads from $d802 - timer-based protection?
    * The freakish graphics encoding scheme, which no other MAME-supported game uses
    * The sprite-ram, and all the funky parameters that go along with it


Stephh's notes (based on the games Z80 code and some tests) :

1) 'exerion'

  - The coin insertion routine (code at 0x0066) is buggy as you get a credit
    on first coin after initialisation even if you need more than 1 coin for 1 credit :
      * when coinage is set to 2C_1C, you get a credit when inserting
        1, 2, 4, 6 ... multiples of 2 coins
      * when coinage is set to 3C_1C, you get a credit when inserting
        1, 3, 6, 9 ... multiples of 3 coins
      * when coinage is set to 4C_1C, you get a credit when inserting
        1, 4, 8, 12 ... multiples of 4 coins
      * when coinage is set to 5C_1C, you get a credit when inserting
        1, 5, 10, 15 ... multiples of 5 coins
  - According to the Dip Switches sheet, difficulty is handled by DSW0 bits 5 and 6.
    In fact, bit 6 determines the overall difficulty (0x40 = OFF easy - 0x00 = ON hard)
    while bit 5 determines enemies' number of bullets (0x20 = OFF for less bullets and
    0x00 = ON for more bullets).
  - When starting a 1 or 2 players game, 2 checksums are computed (code at 0x00e4) :
    one from 0x05f0 to 0x06ee (stored at 0x6030), one from 0x00d8 to 0x01d6 (stored
    at 0x6031). Contents of 0x0625 is also stored to 0x6032.
  - Each time before attract mode sequence starts, a checksum is computed from 0x0000
    to 0x1fff (code at 0x28b8) if 17th score in the high-score table is not 0.
    If checksum doesn't match the hardcoded value (0xb5), you get one more credit
    and you are allowed to continue the game with an extra life (score, charge and
    level are not reset to original values).
  - At the beginning of each life of each player, a checksum is computed from 0x4100
    to 0x4dff (code at 0x07d8) if 1st score in the high-score table is >= 80000.
    If checksum doesn't match the hardcoded value (0x63), you get 255 credits !
    Notice that the displayed number of credits won't be correct as the game
    isn't supposed to allow more than 9 credits.
  - In a 2 players game, when player changes, if it was player 2 turn,
    values from 0x6030 to 0x6032 (see above) are compared with hard-coded values
    (code at 0x04c8). If they don't match respectively 0xfe, 0xb3 and 0x4c,
    and if 9th score in the high-score table is not 0, the game resets !
  - Before entering player's initials, a checksum is computed from 0x5f00 to 0x5fff
    (code at 0x5bd0) if player has reached level 6 (2nd kind of enemies after bonus
    stage). If checksum doesn't match the hardcoded value (0x9a), the game resets !
  - There is a sort of protection routine at 0x4120 which has an effect when
    player loses a life on reaching first bonus stage or after. If values read
    from 0x6008 to 0x600b don't match values from ROM area 0x33c0-0x33ff,
    the game resets. See 'protection_r' read handler.
  - There is an unknown routine at 0x5f70 which is called when the game boots
    which reads value from 0x600c and see if it matches a hardcoded value (0xbe).
    If they don't match, the game resets after displaying the high-scores table.
  - There is another unknown routine at 0x414e which is called when a game is over
    which reads value from 0x600c and see if it matches value from ROM area
    0x4000-0x400f based on internal timer value for a game at 0x604a. If they don't
    match, its only effect is to set lives to 0, which is always the case when the
    game is over, so it doesn't seem to have any real effect.
    Was it supposed to be called at another time ?
  - The routine at 0x5f90 writes to addresses 0x6008-0x600c values read from AY port A
    (one write after one read). This routine is called by the 2 unknown routines.

2) 'exeriont'

  - The coin insertion routine is fixed in this set (see the subtle changes
    in the code from 0x0077 to 0x0082).
  - The routine at 0x28b8 is the same as in 'exerion' (same hardcoded value).
  - The routine at 0x07d8 is the same as in 'exerion' (same hardcoded value).
  - The routine at 0x04c8 is the same as in 'exerion' (same hardcoded values).
  - The routine at 0x5bd0 is the same as in 'exerion' (same hardcoded value).
  - The routine at 0x4120 is the same as in 'exerion', but data from 0x33c0 to 0x33ff
    is slightly different :

      address   exerion  exeriont
      0x33c1:     0x3e     0x36
      0x33c2:     0x37     0x32
      0x33c8:     0x76     0x7e
      0x33ca:     0x32     0x26
      0x33cb:     0x34     0x1e
      0x33d5:     0x07     0x3f
      0x33fc:     0x76     0x40
      0x33fd:     0x37     0x00
      0x33fe:     0x32     0x00
      0x33ff:     0x26     0x00

  - The routine at 0x5f70 is similar to the one in 'exerion' (hardcoded value = 0x9e).
  - The routine at 0x414e is the same as in 'exerion', but data from 0x4000 to 0x400f
    is slightly different :

      address   exerion  exeriont
      0x4002:     0xb2     0x9e
      0x400f:     0xbe     0x9e

  - The routine at 0x5f90 is the same as in 'exerion'.

3) 'exerionb'

  - This set is based on 'exerion' as the coin insertion routine at 0x0066
    (and as a consequence the bug) is the same.
  - The routine at 0x28b8 has been patched, so you can never see the "continue" feature.
  - The routine at 0x07d8 has been patched, so you can never get 255 credits.
  - The routine at 0x04c8 and the computed values from 0x6030 to 0x6032 are surprisingly
    the same as in 'exerion'.
  - The routine at 0x5bd0 has been patched, so the game can't reset.
  - The "protection" routine at 0x4120 has been patched, so the game can't reset.
  - The first unknown routine at 0x5f70 has been patched, so the game can't reset.
  - The second unknown routine at 0x414e has been patched, so lives can't be set to 0.
  - The routine at 0x5f90 is completely different : it reads values from AY port A,
    but nothing is written to addresses 0x6008-0x600c, and there are lots of writes
    to AY port B (0xd001) due to extra code at 0x0050 and extra data at 0x0040.

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_AYPORTB     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_AYPORTB)

#include "logmacro.h"

#define LOGAYPORTB(...)     LOGMASKED(LOG_AYPORTB,     __VA_ARGS__)


namespace {

class exerion_state : public driver_device
{
public:
	exerion_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_main_ram(*this, "main_ram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu_region(*this, "maincpu"),
		m_background_mixer(*this, "bg_char_mixer_prom"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_inputs(*this, "P%u", 1U)
	{ }

	void exerion(machine_config &config);
	void irion(machine_config &config);

	void init_exerion();
	void init_exerionb();
	void init_irion();

	ioport_value controls_r();
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_main_ram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_maincpu_region;
	required_region_ptr<uint8_t> m_background_mixer;

	// video-related
	uint8_t m_cocktail_flip = 0U;
	uint8_t m_char_palette = 0U;
	uint8_t m_sprite_palette = 0U;
	uint8_t m_char_bank = 0U;
	std::unique_ptr<uint16_t[]>  m_background_gfx[4]{};
	uint8_t m_background_latches[13]{};

	// protection?
	uint8_t m_porta = 0U;
	uint8_t m_portb = 0U;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_ioport_array<2> m_inputs;

	uint8_t protection_r(offs_t offset);
	void videoreg_w(uint8_t data);
	void video_latch_w(offs_t offset, uint8_t data);
	uint8_t video_timing_r();
	uint8_t porta_r();
	void portb_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
};


static constexpr XTAL MASTER_CLOCK = XTAL(19'968'000);   // verified on PCB
static constexpr XTAL CPU_CLOCK    = MASTER_CLOCK / 6;
static constexpr XTAL AY8910_CLOCK = CPU_CLOCK / 2;
static constexpr XTAL PIXEL_CLOCK  = MASTER_CLOCK / 3;
static constexpr int HCOUNT_START  = 0x58;
static constexpr int HTOTAL        = 512 - HCOUNT_START;
static constexpr int HBEND         = 12 * 8;    // ??
static constexpr int HBSTART       = 52 * 8;    // ??
static constexpr int VTOTAL        = 256;
static constexpr int VBEND         = 16;
static constexpr int VBSTART       = 240;

static constexpr int BACKGROUND_X_START = 32;

static constexpr int VISIBLE_X_MIN      = 12 * 8;
static constexpr int VISIBLE_X_MAX      = 52 * 8;
static constexpr int VISIBLE_Y_MIN      = 2 * 8;
static constexpr int VISIBLE_Y_MAX      = 30 * 8;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

void exerion_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	// create a lookup table for the palette
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	// color_prom now points to the beginning of the lookup table
	color_prom += 0x20;

	// fg chars and sprites
	for (int i = 0; i < 0x200; i++)
	{
		uint8_t const ctabentry = 0x10 | (color_prom[(i & 0x1c0) | ((i & 3) << 4) | ((i >> 2) & 0x0f)] & 0x0f);
		palette.set_pen_indirect(i, ctabentry);
	}

	// bg chars (this is not the full story... there are four layers mixed using another PROM)
	for (int i = 0x200; i < 0x300; i++)
	{
		uint8_t const ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}
}



/*************************************
 *
 *  Video system startup
 *
 *************************************/

void exerion_state::video_start()
{
	// allocate memory for the decoded background graphics
	m_background_gfx[0] = std::make_unique<uint16_t[]>(256 * 256);
	m_background_gfx[1] = std::make_unique<uint16_t[]>(256 * 256);
	m_background_gfx[2] = std::make_unique<uint16_t[]>(256 * 256);
	m_background_gfx[3] = std::make_unique<uint16_t[]>(256 * 256);

	save_pointer(NAME(m_background_gfx[0]), 256 * 256);
	save_pointer(NAME(m_background_gfx[1]), 256 * 256);
	save_pointer(NAME(m_background_gfx[2]), 256 * 256);
	save_pointer(NAME(m_background_gfx[3]), 256 * 256);

	m_cocktail_flip = 0;

	/*---------------------------------
	 * Decode the background graphics
	 *
	 * We decode the 4 background layers separately, but shuffle the bits so that
	 * we can OR all four layers together. Each layer has 2 bits per pixel. Each
	 * layer is decoded into the following bit patterns:
	 *
	 *  000a 0000 00AA
	 *  00b0 0000 BB00
	 *  0c00 00CC 0000
	 *  d000 DD00 0000
	 *
	 * Where AA,BB,CC,DD are the 2bpp data for the pixel,and a,b,c,d are the OR
	 * of these two bits together.
	 */
	uint8_t *gfx = memregion("bgdata")->base();
	for (int i = 0; i < 4; i++)
	{
		uint8_t *src = gfx + i * 0x2000;
		uint16_t *dst = m_background_gfx[i].get();

		for (int y = 0; y < 0x100; y++)
		{
			int x;

			for (x = 0; x < 0x80; x += 4)
			{
				uint8_t data = *src++;
				uint16_t val;

				val = ((data >> 3) & 2) | ((data >> 0) & 1);
				if (val) val |= 0x100 >> i;
				*dst++ = val << (2 * i);

				val = ((data >> 4) & 2) | ((data >> 1) & 1);
				if (val) val |= 0x100 >> i;
				*dst++ = val << (2 * i);

				val = ((data >> 5) & 2) | ((data >> 2) & 1);
				if (val) val |= 0x100 >> i;
				*dst++ = val << (2 * i);

				val = ((data >> 6) & 2) | ((data >> 3) & 1);
				if (val) val |= 0x100 >> i;
				*dst++ = val << (2 * i);
			}

			for (; x < 0x100; x++)
				*dst++ = 0;
		}
	}
}



/*************************************
 *
 *  Video register I/O
 *
 *************************************/

void exerion_state::videoreg_w(uint8_t data)
{
	// bit 0 = flip screen and joystick input multiplexer
	m_cocktail_flip = data & 1;

	// bits 1-2 char lookup table bank
	m_char_palette = (data & 0x06) >> 1;

	// bits 3 char bank
	m_char_bank = (data & 0x08) >> 3;

	// bits 4-5 unused

	// bits 6-7 sprite lookup table bank
	m_sprite_palette = (data & 0xc0) >> 6;
}


void exerion_state::video_latch_w(offs_t offset, uint8_t data)
{
	int const scanline = m_screen->vpos();
	if (scanline > 0)
		m_screen->update_partial(scanline - 1);
	m_background_latches[offset] = data;
}


uint8_t exerion_state::video_timing_r()
{
	// bit 0 is the SNMI signal, which is the negated value of H6, if H7=1 & H8=1 & VBLANK=0, otherwise 1
	// bit 1 is VBLANK

	uint16_t const hcounter = m_screen->hpos() + HCOUNT_START;
	uint8_t snmi = 1;

	if (((hcounter & 0x180) == 0x180) && !m_screen->vblank())
		snmi = !((hcounter >> 6) & 0x01);

	return (m_screen->vblank() << 1) | snmi;
}


/*************************************
 *
 *  Background rendering
 *
 *************************************/

void exerion_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// loop over all visible scanlines
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint16_t const *src0 = &m_background_gfx[0][m_background_latches[1] * 256];
		uint16_t const *src1 = &m_background_gfx[1][m_background_latches[3] * 256];
		uint16_t const *src2 = &m_background_gfx[2][m_background_latches[5] * 256];
		uint16_t const *src3 = &m_background_gfx[3][m_background_latches[7] * 256];
		int xoffs0 = m_background_latches[0];
		int xoffs1 = m_background_latches[2];
		int xoffs2 = m_background_latches[4];
		int xoffs3 = m_background_latches[6];
		int start0 = m_background_latches[8] & 0x0f;
		int start1 = m_background_latches[9] & 0x0f;
		int start2 = m_background_latches[10] & 0x0f;
		int start3 = m_background_latches[11] & 0x0f;
		int stop0 = m_background_latches[8] >> 4;
		int stop1 = m_background_latches[9] >> 4;
		int stop2 = m_background_latches[10] >> 4;
		int stop3 = m_background_latches[11] >> 4;
		uint8_t *const mixer = &m_background_mixer[(m_background_latches[12] << 4) & 0xf0];
		uint16_t scanline[VISIBLE_X_MAX];
		pen_t const pen_base = 0x200 + ((m_background_latches[12] >> 4) << 4);

		// the cocktail flip flag controls whether we count up or down in X
		if (!m_cocktail_flip)
		{
			// skip processing anything that's not visible
			for (int x = BACKGROUND_X_START; x < cliprect.min_x; x++)
			{
				if (!(++xoffs0 & 0x1f)) start0++, stop0++;
				if (!(++xoffs1 & 0x1f)) start1++, stop1++;
				if (!(++xoffs2 & 0x1f)) start2++, stop2++;
				if (!(++xoffs3 & 0x1f)) start3++, stop3++;
			}

			// draw the rest of the scanline fully
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				uint16_t combined = 0;

				// the output enable is controlled by the carries on the start/stop counters
				// they are only active when the start has carried but the stop hasn't
				if ((start0 ^ stop0) & 0x10) combined |= src0[xoffs0 & 0xff];
				if ((start1 ^ stop1) & 0x10) combined |= src1[xoffs1 & 0xff];
				if ((start2 ^ stop2) & 0x10) combined |= src2[xoffs2 & 0xff];
				if ((start3 ^ stop3) & 0x10) combined |= src3[xoffs3 & 0xff];

				// bits 8-11 of the combined value contains the lookup for the mixer PROM
				uint8_t const lookupval = mixer[combined >> 8] & 3;

				// the color index comes from the looked up value combined with the pixel data
				scanline[x] = pen_base | (lookupval << 2) | ((combined >> (2 * lookupval)) & 3);

				// the start/stop counters are clocked when the low 5 bits of the X counter overflow
				if (!(++xoffs0 & 0x1f)) start0++, stop0++;
				if (!(++xoffs1 & 0x1f)) start1++, stop1++;
				if (!(++xoffs2 & 0x1f)) start2++, stop2++;
				if (!(++xoffs3 & 0x1f)) start3++, stop3++;
			}
		}
		else
		{
			// skip processing anything that's not visible
			for (int x = BACKGROUND_X_START; x < cliprect.min_x; x++)
			{
				if (!(xoffs0-- & 0x1f)) start0++, stop0++;
				if (!(xoffs1-- & 0x1f)) start1++, stop1++;
				if (!(xoffs2-- & 0x1f)) start2++, stop2++;
				if (!(xoffs3-- & 0x1f)) start3++, stop3++;
			}

			// draw the rest of the scanline fully
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				uint16_t combined = 0;

				// the output enable is controlled by the carries on the start/stop counters
				// they are only active when the start has carried but the stop hasn't
				if ((start0 ^ stop0) & 0x10) combined |= src0[xoffs0 & 0xff];
				if ((start1 ^ stop1) & 0x10) combined |= src1[xoffs1 & 0xff];
				if ((start2 ^ stop2) & 0x10) combined |= src2[xoffs2 & 0xff];
				if ((start3 ^ stop3) & 0x10) combined |= src3[xoffs3 & 0xff];

				// bits 8-11 of the combined value contains the lookup for the mixer PROM
				uint8_t const lookupval = mixer[combined >> 8] & 3;

				// the color index comes from the looked up value combined with the pixel data
				scanline[x] = pen_base | (lookupval << 2) | ((combined >> (2 * lookupval)) & 3);

				// the start/stop counters are clocked when the low 5 bits of the X counter overflow
				if (!(xoffs0-- & 0x1f)) start0++, stop0++;
				if (!(xoffs1-- & 0x1f)) start1++, stop1++;
				if (!(xoffs2-- & 0x1f)) start2++, stop2++;
				if (!(xoffs3-- & 0x1f)) start3++, stop3++;
			}
		}

		// draw the scanline
		draw_scanline16(bitmap, cliprect.min_x, y, cliprect.width(), &scanline[cliprect.min_x], nullptr);
	}
}


/*************************************
 *
 *  Core refresh routine
 *
 *************************************/

uint32_t exerion_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// draw background
	draw_background(bitmap, cliprect);

	// draw sprites
	for (int i = 0; i < m_spriteram.bytes(); i += 4)
	{
		int const flags = m_spriteram[i + 0];
		int y = m_spriteram[i + 1] ^ 255;
		int code = m_spriteram[i + 2];
		int x = m_spriteram[i + 3] * 2 + 72;

		int xflip = flags & 0x80;
		int yflip = flags & 0x40;
		int const doubled = flags & 0x10;
		int const wide = flags & 0x08;
		int code2 = code;

		int const color = ((flags >> 1) & 0x03) | ((code >> 5) & 0x04) | (code & 0x08) | (m_sprite_palette * 16);
		gfx_element *gfx = m_gfxdecode->gfx(doubled ? 2 : 1);

		if (m_cocktail_flip)
		{
			x = 64 * 8 - gfx->width() - x;
			y = 32 * 8 - gfx->height() - y;
			if (wide) y -= gfx->height();
			xflip = !xflip;
			yflip = !yflip;
		}

		if (wide)
		{
			if (yflip)
				code |= 0x10, code2 &= ~0x10;
			else
				code &= ~0x10, code2 |= 0x10;

			gfx->transmask(bitmap, cliprect, code2, color, xflip, yflip, x, y + gfx->height(),
					m_palette->transpen_mask(*gfx, color, 0x10));
		}

		gfx->transmask(bitmap, cliprect, code, color, xflip, yflip, x, y,
				m_palette->transpen_mask(*gfx, color, 0x10));

		if (doubled)
			i += 4;
	}

	// draw the visible text layer
	for (int sy = cliprect.min_y / 8; sy <= cliprect.max_y / 8; sy++)
		for (int sx = VISIBLE_X_MIN / 8; sx < VISIBLE_X_MAX / 8; sx++)
		{
			int const x = m_cocktail_flip ? (63 * 8 - 8 * sx) : 8 * sx;
			int const y = m_cocktail_flip ? (31 * 8 - 8 * sy) : 8 * sy;

			int const offs = sx + sy * 64;
			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					m_videoram[offs] + 256 * m_char_bank,
					((m_videoram[offs] & 0xf0) >> 4) + m_char_palette * 16,
					m_cocktail_flip, m_cocktail_flip, x, y, 0);
		}

	return 0;
}


/*************************************
 *
 *  Interrupts & inputs
 *
 *************************************/

// Players inputs are muxed at 0xa000
ioport_value exerion_state::controls_r()
{
	return m_inputs[m_cocktail_flip]->read() & 0x3f;
}


INPUT_CHANGED_MEMBER(exerion_state::coin_inserted)
{
	// coin insertion causes an NMI
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Protection??
 *
 *************************************/

// This is the first of many Exerion "features". No clue if it's
// protection or some sort of timer.
uint8_t exerion_state::porta_r()
{
	m_porta ^= 0x40;
	return m_porta;
}


void exerion_state::portb_w(uint8_t data)
{
	// pull the expected value from the ROM
	m_porta = m_maincpu_region[0x5f76];
	m_portb = data;

	LOGAYPORTB("Port B = %02X\n", data);
}


uint8_t exerion_state::protection_r(offs_t offset)
{
	if (m_maincpu->pc() == 0x4143)
		return m_maincpu_region[0x33c0 + (m_main_ram[0xd] << 2) + offset];
	else
		return m_main_ram[0x8 + offset];
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void exerion_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram().share(m_main_ram);
	map(0x6008, 0x600b).r(FUNC(exerion_state::protection_r));
	map(0x8000, 0x87ff).ram().share(m_videoram);
	map(0x8800, 0x887f).ram().share(m_spriteram);
	map(0x8880, 0x8bff).ram();
	map(0xa000, 0xa000).portr("IN0");
	map(0xa800, 0xa800).portr("DSW0");
	map(0xb000, 0xb000).portr("DSW1");
	map(0xc000, 0xc000).w(FUNC(exerion_state::videoreg_w));
	map(0xc800, 0xc800).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xd000, 0xd001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0xd800, 0xd801).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xd802, 0xd802).r("ay2", FUNC(ay8910_device::data_r));
}



/*************************************
 *
 *  Sub CPU memory handlers
 *
 *************************************/

void exerion_state::sub_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x6000, 0x6000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x8000, 0x800c).w(FUNC(exerion_state::video_latch_w));
	map(0xa000, 0xa000).r(FUNC(exerion_state::video_timing_r));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

// verified from Z80 code
static INPUT_PORTS_START( exerion )
	PORT_START("IN0")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(exerion_state::controls_r))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x05, "5" )                         // duplicated setting
	PORT_DIPSETTING(    0x06, "5" )                         // duplicated setting
	PORT_DIPSETTING(    0x07, "254 (Cheat)")
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Difficulty ) )       // see notes
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coinage ) )          // see notes
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(exerion_state::coin_inserted), 0)

	PORT_START("P1")          // fake input port
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")          // fake input port
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	16*8
};


// 16 x 16 sprites -- requires reorganizing characters in init_exerion()
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{  3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16+3, 16+2, 16+1, 16+0, 24+3, 24+2, 24+1, 24+0 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
			32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15 },
	64*8
};


static GFXDECODE_START( gfx_exerion )
	GFXDECODE_ENTRY( "fgchars", 0, charlayout,         0, 64 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,     256, 64 )
	GFXDECODE_SCALE( "sprites", 0, spritelayout,     256, 64, 2, 2 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void exerion_state::machine_start()
{
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_cocktail_flip));
	save_item(NAME(m_char_palette));
	save_item(NAME(m_sprite_palette));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_background_latches));
}

void exerion_state::machine_reset()
{
	m_porta = 0;
	m_portb = 0;
	m_cocktail_flip = 0;
	m_char_palette = 0;
	m_sprite_palette = 0;
	m_char_bank = 0;

	for (int i = 0; i < 13; i++)
		m_background_latches[i] = 0;
}

void exerion_state::exerion(machine_config &config)
{
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &exerion_state::main_map);

	z80_device &sub(Z80(config, "sub", CPU_CLOCK));
	sub.set_addrmap(AS_PROGRAM, &exerion_state::sub_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(exerion_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_exerion);
	PALETTE(config, m_palette, FUNC(exerion_state::palette), 256 * 3, 32);

	// audio hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, "ay1", AY8910_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.30);

	ay8910_device &ay2(AY8910(config, "ay2", AY8910_CLOCK));
	ay2.port_a_read_callback().set(FUNC(exerion_state::porta_r));
	ay2.port_b_write_callback().set(FUNC(exerion_state::portb_w));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.30);
}

void exerion_state::irion(machine_config &config)
{
	exerion(config);
	config.device_remove("sub");
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( exerion )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "exerion.07",   0x0000, 0x2000, CRC(4c78d57d) SHA1(ac702e9ad2bc05493fb1355858667c31c36acfe4) )
	ROM_LOAD( "exerion.08",   0x2000, 0x2000, CRC(dcadc1df) SHA1(91388f617cfaa4289ca1c84c697fcfdd8834ae15) )
	ROM_LOAD( "exerion.09",   0x4000, 0x2000, CRC(34cc4d14) SHA1(511c9de038f7bcaf6f7c96f2cbbe50a80673fa72) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "fgchars", 0 )
	ROM_LOAD( "exerion.06",   0x00000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) )

	ROM_REGION( 0x04000, "sprites", 0 )
	ROM_LOAD( "exerion.11",   0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) )
	ROM_LOAD( "exerion.10",   0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "bgdata", 0 )
	ROM_LOAD( "exerion.03",   0x00000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) )
	ROM_LOAD( "exerion.04",   0x02000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x04000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x06000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) // palette
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) // fg char lookup table
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) // sprite lookup table
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) // bg char lookup table

	ROM_REGION( 0x0100, "bg_char_mixer_prom", 0 )
	ROM_LOAD( "exerion.k4",   0x0000, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) )
ROM_END


ROM_START( exeriont )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "prom5.4p",     0x0000, 0x4000, CRC(58b4dc1b) SHA1(3e34d1eda0b0537dac1062e96259d4cc7c64049c) )
	ROM_LOAD( "prom6.4s",     0x4000, 0x2000, CRC(fca18c2d) SHA1(31077dada3ed4aa2e26af933f589e01e0c71e5cd) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "fgchars", 0 )
	ROM_LOAD( "exerion.06",   0x00000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) )

	ROM_REGION( 0x04000, "sprites", 0 )
	ROM_LOAD( "exerion.11",   0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) )
	ROM_LOAD( "exerion.10",   0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "bgdata", 0 )
	ROM_LOAD( "exerion.03",   0x00000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) )
	ROM_LOAD( "exerion.04",   0x02000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x04000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x06000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) // palette
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) // fg char lookup table
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) // sprite lookup table
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) // bg char lookup table

	ROM_REGION( 0x0100, "bg_char_mixer_prom", 0 )
	ROM_LOAD( "exerion.k4",   0x0000, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) )
ROM_END


ROM_START( exerionb )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "eb5.bin",      0x0000, 0x4000, CRC(da175855) SHA1(11ea46fd1d504e16e5ffc604d74c1ce210d6be1c) )
	ROM_LOAD( "eb6.bin",      0x4000, 0x2000, CRC(0dbe2eff) SHA1(5b0e5e8453619beec46c4350d1b2ed571fe3dc24) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "fgchars", 0 )
	ROM_LOAD( "exerion.06",   0x0000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) )

	ROM_REGION( 0x04000, "sprites", 0 )
	ROM_LOAD( "exerion.11",   0x0000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) )
	ROM_LOAD( "exerion.10",   0x2000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "bgdata", 0 )
	ROM_LOAD( "exerion.03",   0x0000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) )
	ROM_LOAD( "exerion.04",   0x2000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x4000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x6000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) // palette
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) // fg char lookup table
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) // sprite lookup table
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) // bg char lookup table

	ROM_REGION( 0x0100, "bg_char_mixer_prom", 0 )
	ROM_LOAD( "exerion.k4",   0x0000, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) )
ROM_END


ROM_START( exerionb2 )
	ROM_REGION( 0x6000, "maincpu", 0 ) // slight differences compared to the original, not only the usual copyright removal but also some routines modified / added
	ROM_LOAD( "e7.bin",   0x0000, 0x2000, CRC(349fc44e) SHA1(f0f60528366c860f532e8580310c4fb4eae9e8d6) )
	ROM_LOAD( "e8.bin",   0x2000, 0x2000, CRC(b7b5eb9b) SHA1(6980ba29ac9178adf93f6b89dff52d9aa8db17ae) )
	ROM_LOAD( "e9.bin",   0x4000, 0x2000, CRC(11a30c5a) SHA1(1fa512af5771939d54cea76c7d9c09a6ab39aca9) )

	ROM_REGION( 0x2000, "sub", 0 ) // same as the original
	ROM_LOAD( "e5.bin",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "fgchars", 0 ) // slight differences compared to the original
	ROM_LOAD( "e6.bin",  0x00000, 0x2000, CRC(24a2ceb5) SHA1(77fa649e75fe549091cf401307c583e9b6acfdce) )

	ROM_REGION( 0x04000, "sprites", 0 ) // same as the original
	ROM_LOAD( "e11.bin", 0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) )
	ROM_LOAD( "e10.bin", 0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "bgdata", ROMREGION_ERASE00 ) // smaller ROMs, but same contents but for 3.bin
	ROM_LOAD( "6.bin",   0x00000, 0x1000, CRC(e0dceacc) SHA1(ac1c08c71878dc10d762da811dbc565248006941) )
	ROM_LOAD( "7.bin",   0x01000, 0x1000, CRC(544d4194) SHA1(a37c69b74c09f60e91a12894b595adcddcb475d9) )
	ROM_LOAD( "5.bin",   0x02000, 0x1000, CRC(7cf28a3c) SHA1(064d05461320d4cd9c0d172551a85aae1ee29f02) )
	ROM_LOAD( "4.bin",   0x03000, 0x1000, CRC(c7e8a4eb) SHA1(5852ff31c0350a1d66b0c7781b3d3f3e0b003f9b) )
	ROM_LOAD( "2.bin",   0x04000, 0x1000, CRC(bda08550) SHA1(74528cdb30cee079647201f6e6227425d8a0a947) )
	ROM_LOAD( "3.bin",   0x05000, 0x1000, CRC(bc8510c4) SHA1(b2fe8cdebd555b9f0f585e017b1ce2e7e4956599) ) // slight differences compared to the original
	ROM_LOAD( "1.bin",   0x06000, 0x1000, CRC(33c73949) SHA1(fac662bd9c0ed769a3574074aba9ab4e0d7aaf33) )

	ROM_REGION( 0x0320, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, BAD_DUMP CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) // palette
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, BAD_DUMP CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) // fg char lookup table
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, BAD_DUMP CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) // sprite lookup table
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, BAD_DUMP CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) // bg char lookup table

	ROM_REGION( 0x0100, "bg_char_mixer_prom", 0 ) // not dumped for this set
	ROM_LOAD( "exerion.k4",   0x0000, 0x0100, BAD_DUMP CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) )
ROM_END


ROM_START( exerionba )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "2_7.bin",  0x0000, 0x2000, CRC(349fc44e) SHA1(f0f60528366c860f532e8580310c4fb4eae9e8d6) )
	ROM_LOAD( "2_8.bin",  0x2000, 0x2000, CRC(b7b5eb9b) SHA1(6980ba29ac9178adf93f6b89dff52d9aa8db17ae) )
	ROM_LOAD( "2_9.bin",  0x4000, 0x2000, CRC(11a30c5a) SHA1(1fa512af5771939d54cea76c7d9c09a6ab39aca9) )

	ROM_REGION( 0x2000, "sub", 0 )
	ROM_LOAD( "8.bin",    0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "fgchars", 0 )
	ROM_LOAD( "2_6.bin",  0x0000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) )

	ROM_REGION( 0x04000, "sprites", 0 )
	ROM_LOAD( "2_11.bin", 0x0000, 0x2000, CRC(101628ce) SHA1(f555dfcf142bd92e362054f573803e31d8db94ff) )
	ROM_LOAD( "2_10.bin", 0x2000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "bgdata", ROMREGION_ERASE00 )
	ROM_LOAD( "6.bin",   0x00000, 0x1000, CRC(e0dceacc) SHA1(ac1c08c71878dc10d762da811dbc565248006941) )
	ROM_LOAD( "7.bin",   0x01000, 0x1000, CRC(544d4194) SHA1(a37c69b74c09f60e91a12894b595adcddcb475d9) )
	ROM_LOAD( "5.bin",   0x02000, 0x1000, CRC(7cf28a3c) SHA1(064d05461320d4cd9c0d172551a85aae1ee29f02) )
	ROM_LOAD( "4.bin",   0x03000, 0x1000, CRC(c7e8a4eb) SHA1(5852ff31c0350a1d66b0c7781b3d3f3e0b003f9b) )
	ROM_LOAD( "2.bin",   0x04000, 0x1000, CRC(bda08550) SHA1(74528cdb30cee079647201f6e6227425d8a0a947) )
	ROM_LOAD( "3.bin",   0x05000, 0x1000, CRC(de30698d) SHA1(33eddae3aa5d4eea1e522d654fb86f505a4b99ac) )
	ROM_LOAD( "1.bin",   0x06000, 0x1000, CRC(33c73949) SHA1(fac662bd9c0ed769a3574074aba9ab4e0d7aaf33) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "82s123.1", 0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) // palette
	ROM_LOAD( "82s129.4", 0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) // fg char lookup table
	ROM_LOAD( "82s129.5", 0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) // sprite lookup table
	ROM_LOAD( "82s129.2", 0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) // bg char lookup table

	ROM_REGION( 0x0100, "bg_char_mixer_prom", 0 )
	ROM_LOAD( "82s129.3", 0x0000, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) )
ROM_END


ROM_START( irion )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "2.bin",    0x0000, 0x2000, CRC(bf55324e) SHA1(a310e953cc80d09111ba104f21461420ae3abcd5) )
	ROM_LOAD( "3.bin",    0x2000, 0x2000, CRC(0625bb49) SHA1(111edb1da2153c853d89e56a89ef813cee559730) )
	ROM_LOAD( "4.bin",    0x4000, 0x2000, CRC(918a9b1d) SHA1(e515f1b9c5ddda8115e68e8a499b252b09774bb6) )

	ROM_REGION( 0x02000, "fgchars", 0 )
	ROM_LOAD( "1.bin",    0x0000, 0x2000, CRC(56cd5ebf) SHA1(58d84c2dc3b3bac7371da5b9a230fa581ead31dc) )

	ROM_REGION( 0x04000, "sprites", 0 )
	ROM_LOAD( "5.bin",    0x0000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )
	ROM_LOAD( "6.bin",    0x2000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) )

	ROM_REGION( 0x08000, "bgdata", ROMREGION_ERASE00 )

	ROM_REGION( 0x0320, "proms", 0 ) // these are assumed to be on the board - the game won't run without them
	ROM_LOAD( "exerion.e1",  0x000, 0x020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) // palette
	ROM_LOAD( "exerion.i8",  0x020, 0x100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) // fg char lookup table
	ROM_LOAD( "exerion.h10", 0x120, 0x100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) // sprite lookup table
	ROM_LOAD( "exerion.i3",  0x220, 0x100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) // bg char lookup table

	ROM_REGION( 0x0100, "bg_char_mixer_prom", 0 )
	ROM_LOAD( "exerion.k4",  0x000, 0x100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void exerion_state::init_exerion()
{
	// allocate some temporary space
	std::vector<uint8_t> temp(0x10000);

	// make a temporary copy of the character data
	uint8_t *src = &temp[0];
	uint8_t *dst = memregion("fgchars")->base();
	uint32_t length = memregion("fgchars")->bytes();
	memcpy(src, dst, length);

	// decode the characters
	// the bits in the ROM are ordered: n8-n7 n6 n5 n4-v2 v1 v0 n3-n2 n1 n0 h2
	// we want them ordered like this:  n8-n7 n6 n5 n4-n3 n2 n1 n0-v2 v1 v0 h2
	for (uint32_t oldaddr = 0; oldaddr < length; oldaddr++)
	{
		uint32_t newaddr = ((oldaddr     ) & 0x1f00) |       // keep n8-n4
						   ((oldaddr << 3) & 0x00f0) |       // move n3-n0
						   ((oldaddr >> 4) & 0x000e) |       // move v2-v0
						   ((oldaddr     ) & 0x0001);        // keep h2
		dst[newaddr] = src[oldaddr];
	}

	// make a temporary copy of the sprite data
	src = &temp[0];
	dst = memregion("sprites")->base();
	length = memregion("sprites")->bytes();
	memcpy(src, dst, length);

	// decode the sprites
	// the bits in the ROMs are ordered: n9 n8 n3 n7-n6 n5 n4 v3-v2 v1 v0 n2-n1 n0 h3 h2
	// we want them ordered like this:   n9 n8 n7 n6-n5 n4 n3 n2-n1 n0 v3 v2-v1 v0 h3 h2
	for (uint32_t oldaddr = 0; oldaddr < length; oldaddr++)
	{
		uint32_t newaddr = ((oldaddr << 1) & 0x3c00) |       // move n7-n4
						   ((oldaddr >> 4) & 0x0200) |       // move n3
						   ((oldaddr << 4) & 0x01c0) |       // move n2-n0
						   ((oldaddr >> 3) & 0x003c) |       // move v3-v0
						   ((oldaddr     ) & 0xc003);        // keep n9-n8 h3-h2
		dst[newaddr] = src[oldaddr];
	}
}


void exerion_state::init_exerionb()
{
	uint8_t *ram = memregion("maincpu")->base();

	// the program ROMs have data lines D1 and D2 swapped. Decode them.
	for (int addr = 0; addr < 0x6000; addr++)
		ram[addr] = (ram[addr] & 0xf9) | ((ram[addr] & 2) << 1) | ((ram[addr] & 4) >> 1);

	// also convert the gfx as in Exerion
	init_exerion();
}

void exerion_state::init_irion()
{
	// convert the gfx and cpu ROMS like in ExerionB
	init_exerionb();

	// a further unscramble of gfx2
	uint8_t *ram = memregion("sprites")->base();
	for (uint16_t i = 0; i < 0x4000; i += 0x400)
	{
		for (uint16_t j = 0; j < 0x200; j++)
		{
			uint8_t k = ram[i + j];
			ram[i + j] = ram[i + j + 0x200];
			ram[i + j + 0x200] = k;
		}
	}
}

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, exerion,   0,       exerion, exerion, exerion_state, init_exerion,  ROT90, "Jaleco",                         "Exerion",                  MACHINE_SUPPORTS_SAVE )
GAME( 1983, exeriont,  exerion, exerion, exerion, exerion_state, init_exerion,  ROT90, "Jaleco (Taito America license)", "Exerion (Taito)",          MACHINE_SUPPORTS_SAVE )
GAME( 1983, exerionb,  exerion, exerion, exerion, exerion_state, init_exerionb, ROT90, "bootleg",                        "Exerion (bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exerionb2, exerion, exerion, exerion, exerion_state, init_exerion,  ROT90, "bootleg",                        "Exerion (bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exerionba, exerion, exerion, exerion, exerion_state, init_exerion,  ROT90, "bootleg (Assa)",                 "Exerion (Assa, bootleg)",  MACHINE_SUPPORTS_SAVE )
GAME( 1983, irion,     exerion, irion,   exerion, exerion_state, init_irion,    ROT90, "bootleg",                        "Irion",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
