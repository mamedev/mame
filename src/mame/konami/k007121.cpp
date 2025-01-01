// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Acho A. Tang, R. Belmont
/*

Konami 007121
------
This is an interesting beast. It is an evolution of the 005885, with more
features. Many games use two of these in pair.
It manages sprites and two 32x32 tilemaps. The tilemaps can be joined to form
a single 64x32 one, or one of them can be moved to the side of screen, giving
a high score display suitable for vertical games.
The chip also generates clock and interrupt signals suitable for a 6809.
It uses 0x2000 bytes of static RAM for the tilemaps and sprite lists, and two
64kx4bit DRAMs, presumably as a double frame buffer. The maximum addressable
ROM is 0x80000 bytes (addressed 16 bits at a time). Tile and sprite data both
come from the same ROM space. Like the 005885, external circuitry can cause
tiles and sprites to be fetched from different ROMs (used by Haunted Castle).

Two 256x4 lookup PROMs are also used to increase the color combinations.
All tilemap / sprite priority handling is done internally and the chip exports
7 bits of color code, composed of 2 bits of palette bank, 1 bit indicating tile
or sprite, and 4 bits of ROM data remapped through the PROM.

inputs:
- address lines (A0-A13)
- data lines (DB0-DB7)
- misc interface stuff
- data from the gfx ROMs (RDL0-RDL7, RDU0-RDU7)
- data from the tile lookup PROMs (VCD0-VCD3)
- data from the sprite lookup PROMs (OCD0-OCD3)

outputs:
- address lines for tilemap RAM (AX0-AX12)
- data lines for tilemap RAM (VO0-VO7)
- address lines for the DRAMs (FA0-FA7)
- control lines for the DRAMs (NWR0, NWR1, NRAS, NCAS, NOE)
- data lines for the DRAMs (FD0-FD7)
- address lines for the gfx ROMs (R0-R17)
- address lines for the tile lookup PROMs (VCF0-VCF3, VCB0-VCB3)
- address lines for the sprite lookup PROMs (OCB0-OCB3, OCF0-OCF3)
- NNMI, NIRQ, NFIR, NE, NQ for the main CPU
- misc interface stuff
- color code to be output on screen (COA0-COA6)


control registers
000:          scroll x (low 8 bits)
001: -------x scroll x (high bit)
     ------x- enable rowscroll? (combatsc)
     -----x-- unknown (flak attack)
     ----x--- this probably selects an alternate screen layout used in combat
              school where tilemap #2 is overlayed on front and doesn't scroll.
              The 32 lines of the front layer can be individually turned on or
              off using the second 32 bytes of scroll RAM.
002:          scroll y
003: -------x bit 13 of the tile code
     ------x- unknown (contra)
     -----x-- might be sprite / tilemap priority (0 = sprites have priority)
              (combat school, contra, haunted castle(0/1), labyrunr)
     ----x--- selects sprite buffer (and makes a copy to a private buffer?)
     ---x---- screen layout selector:
              when this is set, 5 columns are added on the left of the screen
              (that means 5 rows at the top for vertical games), and the
              rightmost 2 columns are chopped away.
              Tilemap #2 is used to display the 5 additional columns on the
              left. The rest of tilemap #2 is not used and can be used as work
              RAM by the program.
              The visible area becomes 280x224.
              Note that labyrunr changes this at runtime, setting it during
              gameplay and resetting it on the title screen and crosshatch.
     --x----- might be sprite / tilemap priority (0 = sprites have priority)
              (combat school, contra, haunted castle(0/1), labyrunr)
     -x------ Chops away the leftmost and rightmost columns, switching the
              visible area from 256 to 240 pixels. This is used by combatsc on
              the scrolling stages, and by labyrunr on the title screen.
              At first I thought that this enabled an extra bank of 0x40
              sprites, needed by combatsc, but labyrunr proves that this is not
              the case
     x------- unknown (contra)
004: ----xxxx bits 9-12 of the tile code. Only the bits enabled by the following
              mask are actually used, and replace the ones selected by register
              005.
     xxxx---- mask enabling the above bits
005: selects where in the attribute byte to pick bits 9-12 of the tile code,
     output to pins R12-R15. The bit of the attribute byte to use is the
     specified bit (0-3) + 3, that is one of bits 3-6. Bit 7 is hardcoded as
     bit 8 of the code. Bits 0-2 are used for the color, however note that
     some games use bit 3 as well (see below).
     ------xx attribute bit to use for tile code bit  9
     ----xx-- attribute bit to use for tile code bit 10
     --xx---- attribute bit to use for tile code bit 11
     xx------ attribute bit to use for tile code bit 12
006: ----xxxx select additional effect for bits 3-6 of the tile attribute (the
              same ones indexed by register 005). Note that an attribute bit
              can therefore be used at the same time to be BOTH a tile code bit
              and an additional effect.
     -------x bit 3 of attribute is bit 3 of color (combatsc, fastlane, flkatck)
     ------x- bit 4 of attribute is tile flip X (assumption - no game uses this)
     -----x-- bit 5 of attribute is tile flip Y (flkatck)
     ----x--- bit 6 of attribute is tile priority over sprites (combatsc, hcastle,
              labyrunr)
              Note that hcastle sets this bit for layer 0, and bit 6 of the
              attribute is also used as bit 12 of the tile code, however that
              bit is ALWAYS set throughout the game.
              combatsc uses the bit in the "graduation" scene during attract mode,
              to place soldiers behind the stand.
              Use in labyrunr has not been investigated yet.
     --xx---- palette bank (both tiles and sprites, see contra)
007: -------x nmi enable
     ------x- irq enable
     -----x-- firq enable
     ----x--- flip screen
     ---x---- unknown (contra, labyrunr)

*/

#include "emu.h"
#include "k007121.h"
#include "konami_helper.h"
#include "tilemap.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


DEFINE_DEVICE_TYPE(K007121, k007121_device, "k007121", "K007121 Sprite/Tilemap Controller")

k007121_device::k007121_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K007121, tag, owner, clock)
	, device_gfx_interface(mconfig, *this)
	, m_flipscreen(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k007121_device::device_start()
{
	save_item(NAME(m_ctrlram));
	save_item(NAME(m_flipscreen));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k007121_device::device_reset()
{
	m_flipscreen = false;

	for (int i = 0; i < 8; i++)
		m_ctrlram[i] = 0;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

uint8_t k007121_device::ctrlram_r(offs_t offset)
{
	assert(offset < 8);

	return m_ctrlram[offset];
}


void k007121_device::ctrl_w(offs_t offset, uint8_t data)
{
	assert(offset < 8);

	switch (offset)
	{
	case 6:
		// palette bank change
		if ((m_ctrlram[offset] & 0x30) != (data & 0x30))
			machine().tilemap().mark_all_dirty();
		break;
	case 7:
		m_flipscreen = BIT(data, 3);
		break;
	}

	m_ctrlram[offset] = data;
}

/*
 * Sprite Format
 * ------------------
 *
 * There are 0x40 sprites, each one using 5 bytes. However the number of
 * sprites can be increased to 0x80 with a control register (Combat School
 * sets it on and off during the game).
 *
 * Byte | Bit(s)   | Use
 * -----+-76543210-+----------------
 *   0  | xxxxxxxx | sprite code
 *   1  | xxxx---- | color
 *   1  | ----xx-- | sprite code low 2 bits for 16x8/8x8 sprites
 *   1  | ------xx | sprite code bank bits 1/0
 *   2  | xxxxxxxx | y position
 *   3  | xxxxxxxx | x position (low 8 bits)
 *   4  | xx------ | sprite code bank bits 3/2
 *   4  | --x----- | flip y
 *   4  | ---x---- | flip x
 *   4  | ----xxx- | sprite size 000=16x16 001=16x8 010=8x16 011=8x8 100=32x32
 *   4  | -------x | x position (high bit)
 *
 */

void k007121_device::sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect,
		const uint8_t *source, int base_color, int global_x_offset, int bank_base, bitmap_ind8 &priority_bitmap, uint32_t pri_mask)
{
	// TODO: sprite limit is supposed to be per-line! (check MT #00185)
	int num = 0x40;
	//num = (m_ctrlram[0x03] & 0x40) ? 0x80 : 0x40; // WRONG!!! (needed by combatsc)

	int inc = 5;
	// when using priority buffer, draw front to back
	if (pri_mask != (uint32_t)-1)
	{
		source += (num - 1)*inc;
		inc = -inc;
	}

	for (int i = 0; i < num; i++)
	{
		int number = source[0];
		int sprite_bank = source[1] & 0x0f;
		int sx = source[3];
		int sy = source[2];
		int attr = source[4];
		int xflip = source[4] & 0x10;
		int yflip = source[4] & 0x20;
		int color = base_color + ((source[1] & 0xf0) >> 4);
		int width, height;
		int transparent_mask;
		static const int x_offset[4] = {0x0,0x1,0x4,0x5};
		static const int y_offset[4] = {0x0,0x2,0x8,0xa};
		int flipx, flipy, destx, desty;

		if (attr & 0x01) sx -= 256;
		if (sy >= 240) sy -= 256;

		number += ((sprite_bank & 0x3) << 8) + ((attr & 0xc0) << 4);
		number = number << 2;
		number += (sprite_bank >> 2) & 3;

		// Flak Attack doesn't use a lookup PROM, it maps the color code directly to a palette entry
		if (palette().indirect_entries() == 0)
			transparent_mask = 1 << 0;
		else
			transparent_mask = palette().transpen_mask(*gfx(0), color, 0);

		number += bank_base;

		switch (attr & 0xe)
		{
			case 0x06:
				width = height = 1;
				break;

			case 0x04:
				width = 1; height = 2;
				number &= ~2;
				break;

			case 0x02:
				width = 2; height = 1;
				number &= ~1;
				break;

			case 0x00:
				width = height = 2;
				number &= ~3;
				break;

			case 0x08:
				width = height = 4;
				number &= ~0xf;
				break;

			default:
				width = 1; height = 1;
				//logerror("Unknown sprite size %02x\n", attr & 0xe);
				break;
		}

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int ex = xflip ? (width - 1 - x) : x;
				int ey = yflip ? (height - 1 - y) : y;

				if (m_flipscreen)
				{
					flipx = !xflip;
					flipy = !yflip;
					destx = 248 - (sx + x * 8);
					desty = 248 - (sy + y * 8);
				}
				else
				{
					flipx = xflip;
					flipy = yflip;
					destx = global_x_offset + sx + x * 8;
					desty = sy + y * 8;
				}

				if (pri_mask != (uint32_t)-1)
				{
					gfx(0)->prio_transmask(bitmap,cliprect,
							number + x_offset[ex] + y_offset[ey],
							color,
							flipx,flipy,
							destx,desty,
							priority_bitmap,pri_mask,
							transparent_mask);
				}
				else
				{
					gfx(0)->transmask(bitmap,cliprect,
							number + x_offset[ex] + y_offset[ey],
							color,
							flipx,flipy,
							destx,desty,
							transparent_mask);
				}
			}
		}

		source += inc;
	}
}
