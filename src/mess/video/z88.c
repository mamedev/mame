// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/***************************************************************************

  z88.c

  Functions to emulate the video hardware of the Cambridge Z88

***************************************************************************/

#include "includes/z88.h"


inline void z88_state::plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT16 color)
{
	if (x<Z88_SCREEN_WIDTH)
		bitmap.pix16(y, x) = color;
}

// convert absolute offset into correct address to get data from
inline UINT8* z88_state::convert_address(UINT32 offset)
{
	return (offset < 0x80000 ? m_bios : m_ram_base) + (offset & 0x7ffff);
}

/***************************************************************************
  Start the video hardware emulation.
***************************************************************************/

// Initialise the palette
PALETTE_INIT_MEMBER(z88_state, z88)
{
	m_palette->set_pen_color(0, rgb_t(138, 146, 148));
	m_palette->set_pen_color(1, rgb_t(92,  83,  88));
	m_palette->set_pen_color(2, rgb_t(122, 126, 129));
}

/* temp - change to gfxelement structure */

void z88_state::vh_render_8x8(bitmap_ind16 &bitmap, int x, int y, UINT16 pen0, UINT16 pen1, UINT8 *gfx)
{
	for (int h=0; h<8; h++)
	{
		UINT8 data = gfx[h];

		for (int b=0; b<8; b++)
		{
			plot_pixel(bitmap, x+b, y+h, (data & 0x80) ? pen1 : pen0);

			data = data<<1;
		}
	}
}

void z88_state::vh_render_6x8(bitmap_ind16 &bitmap, int x, int y, UINT16 pen0, UINT16 pen1, UINT8 *gfx)
{
	for (int h=0; h<8; h++)
	{
		UINT8 data = gfx[h]<<2;

		for (int b=0; b<6; b++)
		{
			plot_pixel(bitmap, x+1+b, y+h, (data & 0x80) ? pen1 : pen0);
			data = data<<1;
		}
	}
}

void z88_state::vh_render_line(bitmap_ind16 &bitmap, int x, int y, UINT16 pen)
{
	for (int i=0; i<8; i++)
		plot_pixel(bitmap, x + i, y + 7, pen);
}

UPD65031_SCREEN_UPDATE(z88_state::lcd_update)
{
	if (sbf == 0)
	{
		// LCD disabled
		bitmap.fill(0);
	}
	else
	{
		UINT8 *vram = convert_address(sbf<<11);

		for (int y=0; y<(Z88_SCREEN_HEIGHT>>3); y++)
		{
			int x = 0, c = 0;

			while (x < Z88_SCREEN_WIDTH)
			{
				UINT16 pen0, pen1;
				UINT8 *char_gfx;
				UINT8 byte0 = vram[(y * 0x100) + c];
				UINT8 byte1 = vram[(y * 0x100) + c + 1];

				// inverted graphics?
				if (byte1 & Z88_SCR_HW_REV)
				{
					pen0 = (byte1 & Z88_SCR_HW_GRY) ? 2 : 1;
					pen1 = 0;
				}
				else
				{
					pen0 = 0;
					pen1 = (byte1 & Z88_SCR_HW_GRY) ? 2 : 1;
				}

				if ((byte1 & Z88_SCR_HW_NULL) == Z88_SCR_HW_NULL)
				{
					// hidden
				}
				else if (!(byte1 & Z88_SCR_HW_HRS) || (((byte1 & Z88_SCR_HW_CURS) == Z88_SCR_HW_CURS)))
				{
					// low-res 6x8
					UINT16 ch = (byte0 | (byte1<<8)) & 0x1ff;

					if ((ch & 0x01c0) == 0x01c0)
					{
						ch &= 0x3f;

						char_gfx = convert_address(lores0<<9);
					}
					else
					{
						char_gfx = convert_address(lores1<<12);
					}

					char_gfx += (ch<<3);

					// cursor flash
					if (flash && (byte1 & Z88_SCR_HW_CURS) == Z88_SCR_HW_CURS)
						vh_render_6x8(bitmap, x,(y<<3), pen1, pen0, char_gfx);
					else
						vh_render_6x8(bitmap, x,(y<<3), pen0, pen1, char_gfx);

					// underline?
					if (byte1 & Z88_SCR_HW_UND)
						vh_render_line(bitmap, x, (y<<3), pen1);

					x += 6;
				}
				else if ((byte1 & Z88_SCR_HW_HRS) && !(byte1 & Z88_SCR_HW_REV))
				{
					// high-res 8x8
					UINT16 ch = (byte0 | (byte1<<8)) & 0x3ff;

					if (ch & 0x0100)
					{
						ch &= 0xff;
						char_gfx = convert_address(hires1<<11);
					}
					else
					{
						ch &= 0xff;
						char_gfx = convert_address(hires0<<13);
					}

					char_gfx += (ch<<3);

					// flash
					if ((byte1 & Z88_SCR_HW_FLS) && flash)
						pen0 = pen1 = 0;

					vh_render_8x8(bitmap, x,(y<<3), pen0, pen1, char_gfx);

					x += 8;
				}

				// every char takes 2 bytes
				c += 2;
			}
		}
	}
}
