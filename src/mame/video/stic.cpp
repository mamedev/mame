// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
/**********************************************************************

 General Instruments AY-3-8900-1 a.k.a. Standard Television Interface Chip
 (STIC) emulation for Mattel Intellivision

 *********************************************************************/

#include "emu.h"
#include "video/stic.h"


const device_type STIC = &device_creator<stic_device>;

//-------------------------------------------------
//  stic_device - constructor
//-------------------------------------------------

stic_device::stic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
				device_t(mconfig, STIC, "STIC (Standard Television Interface Chip) Video Chip", tag, owner, clock, "stic", __FILE__),
				m_grom(*this, "grom"),
				m_x_scale(1),
				m_y_scale(1)
{
}


//-------------------------------------------------
//  ~stic_device - destructor
//-------------------------------------------------

stic_device::~stic_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void stic_device::device_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);

	save_item(NAME(m_stic_registers));
	save_item(NAME(m_gramdirty));
	save_item(NAME(m_gram));
	save_item(NAME(m_gramdirtybytes));
	save_item(NAME(m_color_stack_mode));
	save_item(NAME(m_color_stack_offset));
	save_item(NAME(m_stic_handshake));
	save_item(NAME(m_border_color));
	save_item(NAME(m_col_delay));
	save_item(NAME(m_row_delay));
	save_item(NAME(m_left_edge_inhibit));
	save_item(NAME(m_top_edge_inhibit));
	save_item(NAME(m_backtab_buffer));
	for (int sp = 0; sp < STIC_MOBS; sp++)
	{
		save_item(m_sprite[sp].visible, "STIC sprite/m_sprite[sp].visible", sp);
		save_item(m_sprite[sp].xpos, "STIC sprite/m_sprite[sp].xpos", sp);
		save_item(m_sprite[sp].ypos, "STIC sprite/m_sprite[sp].ypos", sp);
		save_item(m_sprite[sp].coll, "STIC sprite/m_sprite[sp].coll", sp);
		save_item(m_sprite[sp].collision, "STIC sprite/m_sprite[sp].collision", sp);
		save_item(m_sprite[sp].doublex, "STIC sprite/m_sprite[sp].doublex", sp);
		save_item(m_sprite[sp].doubley, "STIC sprite/m_sprite[sp].doubley", sp);
		save_item(m_sprite[sp].quady, "STIC sprite/m_sprite[sp].quady", sp);
		save_item(m_sprite[sp].xflip, "STIC sprite/m_sprite[sp].xflip", sp);
		save_item(m_sprite[sp].yflip, "STIC sprite/m_sprite[sp].yflip", sp);
		save_item(m_sprite[sp].behind_foreground, "STIC sprite/m_sprite[sp].behind_foreground", sp);
		save_item(m_sprite[sp].grom, "STIC sprite/m_sprite[sp].grom", sp);
		save_item(m_sprite[sp].card, "STIC sprite/m_sprite[sp].card", sp);
		save_item(m_sprite[sp].color, "STIC sprite/m_sprite[sp].color", sp);
		save_item(m_sprite[sp].doubleyres, "STIC sprite/m_sprite[sp].doubleyres", sp);
		save_item(m_sprite[sp].dirty, "STIC sprite/m_sprite[sp].dirty", sp);
		save_item(m_sprite_buffers[sp], "STIC sprite/m_sprite[sp].sprite_buffers", sp);
	}
}

void stic_device::device_reset()
{
	for (int i = 0; i < STIC_MOBS; i++)
	{
		intv_sprite_type* s = &m_sprite[i];
		s->visible = 0;
		s->xpos = 0;
		s->ypos = 0;
		s->coll = 0;
		s->collision = 0;
		s->doublex = 0;
		s->doubley = 0;
		s->quady = 0;
		s->xflip = 0;
		s->yflip = 0;
		s->behind_foreground = 0;
		s->grom = 0;
		s->card = 0;
		s->color = 0;
		s->doubleyres = 0;
		s->dirty = 1;
		for (int j = 0; j < 16; j++)
		{
			for (int k = 0; k < 128; k++)
			{
				m_sprite_buffers[i][j][k] = 0;
			}
		}
	}

	memset(m_stic_registers, 0, sizeof(m_stic_registers));
	m_gramdirty = 1;
	for (int i = 0; i < 64; i++)
	{
		m_gram[i] = 0;
		m_gramdirtybytes[i] = 1;
	}

	m_color_stack_mode = 0;
	m_color_stack_offset = 0;
	m_stic_handshake = 0;
	m_border_color = 0;
	m_col_delay = 0;
	m_row_delay = 0;
	m_left_edge_inhibit = 0;
	m_top_edge_inhibit = 0;
}

ROM_START( stic_grom )
	ROM_REGION( 0x800, "grom", ROMREGION_ERASEFF )
	ROM_LOAD( "ro-3-9503-003.u21", 0, 0x0800, CRC(683a4158) SHA1(f9608bb4ad1cfe3640d02844c7ad8e0bcd974917))
ROM_END

const rom_entry *stic_device::device_rom_region() const
{
	return ROM_NAME( stic_grom );
}



#define FOREGROUND_BIT 0x0010

// conversion from Intellivision color to internal representation
#define SET_COLOR(c)    ((c * 2) + 1)
#define GET_COLOR(c)    ((c - 1) / 2)

/* initialized to non-zero, because we divide by it */

void stic_device::intv_set_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	int w, h;

	// output scaling
	x *= m_x_scale;
	y *= m_y_scale;
	color = SET_COLOR(color);

	for (h = 0; h < m_y_scale; h++)
		for (w = 0; w < m_x_scale; w++)
			bitmap.pix16(y + h, x + w) = color;
}

UINT32 stic_device::intv_get_pixel(bitmap_ind16 &bitmap, int x, int y)
{
	return GET_COLOR(bitmap.pix16(y * m_y_scale, x * m_x_scale));
}

void stic_device::intv_plot_box(bitmap_ind16 &bitmap, int x, int y, int w, int h, int color)
{
	bitmap.plot_box(x * m_x_scale, y * m_y_scale, w * m_x_scale, h * m_y_scale, SET_COLOR(color));
}


int stic_device::sprites_collide(int spriteNum1, int spriteNum2)
{
	INT16 x0, y0, w0, h0, x1, y1, w1, h1, x2, y2, w2, h2;

	intv_sprite_type* s1 = &m_sprite[spriteNum1];
	intv_sprite_type* s2 = &m_sprite[spriteNum2];

	x0 = STIC_OVERSCAN_LEFT_WIDTH + m_col_delay - STIC_CARD_WIDTH;
	y0 = STIC_OVERSCAN_TOP_HEIGHT + m_row_delay - STIC_CARD_HEIGHT;
	x1 = (s1->xpos + x0) * STIC_X_SCALE; y1 = (s1->ypos + y0) * STIC_Y_SCALE;
	x2 = (s2->xpos + x0) * STIC_X_SCALE; y2 = (s2->ypos + y0) * STIC_Y_SCALE;
	w1 = (s1->doublex ? 2 : 1) * STIC_CARD_WIDTH;
	w2 = (s2->doublex ? 2 : 1) * STIC_CARD_WIDTH;
	h1 = (s1->quady ? 4 : 1) * (s1->doubley ? 2 : 1) * (s1->doubleyres ? 2 : 1) * STIC_CARD_HEIGHT;
	h2 = (s2->quady ? 4 : 1) * (s2->doubley ? 2 : 1) * (s2->doubleyres ? 2 : 1) * STIC_CARD_HEIGHT;

	if ((x1 >= x2 + w2) || (y1 >= y2 + h2) ||
		(x2 >= x1 + w1) || (y2 >= y1 + h1))
		return FALSE;

	// iterate over the intersecting bits to see if any touch
	x0 = MAX(x1, x2);
	y0 = MAX(y1, y2);
	w0 = MIN(x1 + w1, x2 + w2) - x0;
	h0 = MIN(y1 + h1, y2 + h2) - y0;
	x1 = x0 - x1;
	y1 = y0 - y1;
	x2 = x0 - x2;
	y2 = y0 - y2;
	for (x0 = 0; x0 < w0; x0++)
	{
		for (y0 = 0; y0 < h0; y0++)
		{
			if (m_sprite_buffers[spriteNum1][x0 + x1][y0 + y1] &&
				m_sprite_buffers[spriteNum2][x0 + x2][y0 + y2])
				return TRUE;
		}
	}

	return FALSE;
}

void stic_device::determine_sprite_collisions()
{
	// check sprite to sprite collisions
	for (int i = 0; i < STIC_MOBS - 1; i++)
	{
		intv_sprite_type* s1 = &m_sprite[i];
		if (s1->xpos == 0 || !s1->coll)
			continue;

		for (int j = i + 1; j < STIC_MOBS; j++)
		{
			intv_sprite_type* s2 = &m_sprite[j];
			if (s2->xpos == 0 || !s2->coll)
				continue;

			if (sprites_collide(i, j))
			{
				s1->collision |= (1 << j);
				s2->collision |= (1 << i);
			}
		}
	}
}

void stic_device::render_sprites()
{
	INT32 cardMemoryLocation, pixelSize;
	INT32 spritePixelHeight;
	INT32 nextMemoryLocation;
	INT32 nextData;
	INT32 nextX;
	INT32 nextY;
	INT32 xInc;

	for (int i = 0; i < STIC_MOBS; i++)
	{
		intv_sprite_type* s = &m_sprite[i];

		if (s->grom)
			cardMemoryLocation = (s->card * STIC_CARD_HEIGHT);
		else
			cardMemoryLocation = ((s->card & 0x003F) * STIC_CARD_HEIGHT);

		pixelSize = (s->quady ? 4 : 1) * (s->doubley ? 2 : 1);
		spritePixelHeight = pixelSize * (s->doubleyres ? 2 : 1) * STIC_CARD_HEIGHT;

		for (int j = 0; j < spritePixelHeight; j++)
		{
			nextMemoryLocation = (cardMemoryLocation + (j/pixelSize));
			if (s->grom)
				nextData = m_grom[nextMemoryLocation];
			else if (nextMemoryLocation < 0x200)
				nextData = m_gram[nextMemoryLocation];
			else
				nextData = 0xFFFF;
			nextX = (s->xflip ? ((s->doublex ? 2 : 1) * STIC_CARD_WIDTH - 1) : 0);
			nextY = (s->yflip ? (spritePixelHeight - j - 1) : j);
			xInc = (s->xflip ? -1: 1);

			for (int k = 0; k < STIC_CARD_WIDTH * (1 + s->doublex); k++)
			{
				m_sprite_buffers[i][nextX + k * xInc][nextY] = (nextData & (1 << ((STIC_CARD_WIDTH - 1) - k / (1 + s->doublex)))) != 0;
			}
		}
	}
}

void stic_device::render_line(bitmap_ind16 &bitmap, UINT8 nextByte, UINT16 x, UINT16 y, UINT8 fgcolor, UINT8 bgcolor)
{
	UINT32 color;

	for (int i = 0; i < STIC_CARD_WIDTH; i++)
	{
		color = (nextByte & (1 << ((STIC_CARD_WIDTH - 1) - i)) ? fgcolor : bgcolor);
		intv_set_pixel(bitmap, x+i, y, color);
		intv_set_pixel(bitmap, x+i, y+1, color);
	}
}

void stic_device::render_colored_squares(bitmap_ind16 &bitmap, UINT16 x, UINT16 y, UINT8 color0, UINT8 color1, UINT8 color2, UINT8 color3)
{
	intv_plot_box(bitmap, x, y, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, color0);
	intv_plot_box(bitmap, x + STIC_CSQM_WIDTH * STIC_X_SCALE, y, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, color1);
	intv_plot_box(bitmap, x, y + STIC_CSQM_HEIGHT * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, color2);
	intv_plot_box(bitmap, x + STIC_CSQM_WIDTH * STIC_X_SCALE, y + STIC_CSQM_HEIGHT * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, color3);
}

void stic_device::render_color_stack_mode(bitmap_ind16 &bitmap)
{
	INT16 w, h, nextx, nexty;
	UINT8 csPtr = 0;
	UINT16 nextCard;

	for (h = 0, nexty = (STIC_OVERSCAN_TOP_HEIGHT + m_row_delay) * STIC_Y_SCALE;
			h < STIC_BACKTAB_HEIGHT;
			h++, nexty += STIC_CARD_HEIGHT * STIC_Y_SCALE)
	{
		for (w = 0, nextx = (STIC_OVERSCAN_LEFT_WIDTH + m_col_delay) * STIC_X_SCALE;
				w < STIC_BACKTAB_WIDTH;
				w++, nextx += STIC_CARD_WIDTH * STIC_X_SCALE)
		{
			nextCard = m_backtab_buffer[h][w];

			// colored squares mode
			if ((nextCard & (STIC_CSTM_FG3|STIC_CSTM_SEL)) == STIC_CSTM_FG3)
			{
				UINT8 csColor = m_stic_registers[STIC_CSR + csPtr];
				UINT8 color0 = nextCard & STIC_CSQM_A;
				UINT8 color1 = (nextCard & STIC_CSQM_B) >> 3;
				UINT8 color2 = (nextCard & STIC_CSQM_C) >> 6;
				UINT8 color3 = ((nextCard & STIC_CSQM_D2) >> 11) |
				((nextCard & (STIC_CSQM_D10)) >> 9);
				render_colored_squares(bitmap, nextx, nexty,
										(color0 == 7 ? csColor : (color0 | FOREGROUND_BIT)),
										(color1 == 7 ? csColor : (color1 | FOREGROUND_BIT)),
										(color2 == 7 ? csColor : (color2 | FOREGROUND_BIT)),
										(color3 == 7 ? csColor : (color3 | FOREGROUND_BIT)));
			}
			//color stack mode
			else
			{
				UINT8 isGrom;
				UINT16 memoryLocation, fgcolor, bgcolor;
				UINT8* memory;

				//advance the color pointer, if necessary
				if (nextCard & STIC_CSTM_ADV)
					csPtr = (csPtr+1) & (STIC_CSRS - 1);

				fgcolor = ((nextCard & STIC_CSTM_FG3) >> 9) |
				(nextCard & (STIC_CSTM_FG20)) | FOREGROUND_BIT;
				bgcolor = m_stic_registers[STIC_CSR + csPtr] & STIC_CSR_BG;

				isGrom = !(nextCard & STIC_CSTM_SEL);
				if (isGrom)
				{
					memoryLocation = nextCard & STIC_CSTM_C;
					memory = m_grom;
					for (int j = 0; j < STIC_CARD_HEIGHT; j++)
						render_line(bitmap, memory[memoryLocation + j],
									nextx, nexty + j * STIC_Y_SCALE, fgcolor, bgcolor);
				}
				else
				{
					memoryLocation = nextCard & STIC_CSTM_C50;
					memory = m_gram;
					for (int j = 0; j < STIC_CARD_HEIGHT; j++)
						render_line(bitmap, memory[memoryLocation + j],
									nextx, nexty + j * STIC_Y_SCALE, fgcolor, bgcolor);
				}
			}
		}
	}
}

void stic_device::render_fg_bg_mode(bitmap_ind16 &bitmap)
{
	INT16 w, h, nextx, nexty;
	UINT8 isGrom, fgcolor, bgcolor;
	UINT16 nextCard, memoryLocation;
	UINT8* memory;

	for (h = 0, nexty = (STIC_OVERSCAN_TOP_HEIGHT + m_row_delay) * STIC_Y_SCALE;
			h < STIC_BACKTAB_HEIGHT;
			h++, nexty += STIC_CARD_HEIGHT * STIC_Y_SCALE)
	{
		for (w = 0, nextx = (STIC_OVERSCAN_LEFT_WIDTH + m_col_delay) * STIC_X_SCALE;
				w < STIC_BACKTAB_WIDTH;
				w++, nextx += STIC_CARD_WIDTH * STIC_X_SCALE)
		{
			nextCard = m_backtab_buffer[h][w];
			fgcolor = (nextCard & STIC_FBM_FG) | FOREGROUND_BIT;
			bgcolor = ((nextCard & STIC_FBM_BG2) >> 11) |
			((nextCard & STIC_FBM_BG310) >> 9);

			isGrom = !(nextCard & STIC_FBM_SEL);
			if (isGrom)
			{
				memoryLocation = nextCard & STIC_FBM_C;
				memory = m_grom;
				for (int j = 0; j < STIC_CARD_HEIGHT; j++)
					render_line(bitmap, memory[memoryLocation + j],
								nextx, nexty + j * STIC_Y_SCALE, fgcolor, bgcolor);
			}
			else
			{
				memoryLocation = nextCard & STIC_FBM_C;
				memory = m_gram;
				for (int j = 0; j < STIC_CARD_HEIGHT; j++)
					render_line(bitmap, memory[memoryLocation + j],
								nextx, nexty + j * STIC_Y_SCALE, fgcolor, bgcolor);
			}
		}
	}
}

void stic_device::copy_sprites_to_background(bitmap_ind16 &bitmap)
{
	UINT8 width, currentPixel;
	UINT8 borderCollision, foregroundCollision;
	UINT8 spritePixelHeight, x, y;
	INT16 leftX, nextY;
	INT16 leftBorder, rightBorder, topBorder, bottomBorder;
	INT32 nextX;

	for (int i = STIC_MOBS - 1; i >= 0; i--)
	{
		intv_sprite_type *s = &m_sprite[i];
		if (s->xpos == 0 || (!s->coll && !s->visible))
			continue;

		borderCollision = FALSE;
		foregroundCollision = FALSE;

		spritePixelHeight = (s->quady ? 4 : 1) * (s->doubley ? 2 : 1) * (s->doubleyres ? 2 : 1) * STIC_CARD_HEIGHT;
		width = (s->doublex ? 2 : 1) * STIC_CARD_WIDTH;

		leftX = (s->xpos - STIC_CARD_WIDTH + STIC_OVERSCAN_LEFT_WIDTH + m_col_delay) * STIC_X_SCALE;
		nextY = (s->ypos - STIC_CARD_HEIGHT + STIC_OVERSCAN_TOP_HEIGHT + m_row_delay) * STIC_Y_SCALE;

		leftBorder =  (STIC_OVERSCAN_LEFT_WIDTH + (m_left_edge_inhibit ? STIC_CARD_WIDTH : 0)) * STIC_X_SCALE;
		rightBorder = (STIC_OVERSCAN_LEFT_WIDTH + STIC_BACKTAB_WIDTH * STIC_CARD_WIDTH - 1 - 1) * STIC_X_SCALE;
		topBorder = (STIC_OVERSCAN_TOP_HEIGHT + (m_top_edge_inhibit ? STIC_CARD_HEIGHT : 0)) * STIC_Y_SCALE;
		bottomBorder = (STIC_OVERSCAN_TOP_HEIGHT + STIC_BACKTAB_HEIGHT * STIC_CARD_HEIGHT) * STIC_Y_SCALE - 1;

		for (y = 0; y < spritePixelHeight; y++)
		{
			for (x = 0; x < width; x++)
			{
				//if this sprite pixel is not on, then don't paint it
				if (!m_sprite_buffers[i][x][y])
					continue;

				nextX = leftX + x;
				//if the next pixel location is on the border, then we
				//have a border collision and we can ignore painting it
				if ((nextX < leftBorder) || (nextX > rightBorder) ||
					(nextY < topBorder) || (nextY > bottomBorder))
				{
					borderCollision = TRUE;
					continue;
				}

				currentPixel = intv_get_pixel(bitmap, nextX, nextY);

				//check for foreground collision
				if (currentPixel & FOREGROUND_BIT)
				{
					foregroundCollision = TRUE;
					if (s->behind_foreground)
						continue;
				}

				if (s->visible)
				{
					intv_set_pixel(bitmap, nextX, nextY, s->color | (currentPixel & FOREGROUND_BIT));
				}
			}
			nextY++;
		}

		//update the collision bits
		if (s->coll)
		{
			if (foregroundCollision)
				s->collision |= STIC_MCR_BKGD;
			if (borderCollision)
				s->collision |= STIC_MCR_BRDR;
		}
	}
}

void stic_device::render_background(bitmap_ind16 &bitmap)
{
	if (m_color_stack_mode)
		render_color_stack_mode(bitmap);
	else
		render_fg_bg_mode(bitmap);
}

#ifdef UNUSED_CODE
void stic_device::draw_background(bitmap_ind16 &bitmap, int transparency)
{
	// First, draw the background
	int offs = 0;
	int value = 0;
	int row,col;
	int fgcolor,bgcolor = 0;
	int code;

	int colora, colorb, colorc, colord;

	int n_bit;
	int p_bit;
	int g_bit;

	int j;

	int x0 = STIC_OVERSCAN_LEFT_WIDTH + m_col_delay;
	int y0 = STIC_OVERSCAN_TOP_HEIGHT + m_row_delay;

	if (m_color_stack_mode == 1)
	{
		m_color_stack_offset = 0;
		for(row = 0; row < STIC_BACKTAB_HEIGHT; row++)
		{
			for(col = 0; col < STIC_BACKTAB_WIDTH; col++)
			{
				value = m_ram16[offs];

				n_bit = value & STIC_CSTM_ADV;
				p_bit = value & STIC_CSTM_FG3;
				g_bit = value & STIC_CSTM_SEL;

				if (p_bit && (!g_bit)) // colored squares mode
				{
					colora = value & STIC_CSQM_A;
					colorb = (value & STIC_CSQM_B) >> 3;
					colorc = (value & STIC_CSQM_C) >> 6;
					colord = ((n_bit & STIC_CSQM_D2) >> 11) + ((value & STIC_CSQM_D10) >> 9);
					// color 7 if the top of the color stack in this mode
					if (colora == 7) colora = m_stic_registers[STIC_CSR + STIC_CSR3];
					if (colorb == 7) colorb = m_stic_registers[STIC_CSR + STIC_CSR3];
					if (colorc == 7) colorc = m_stic_registers[STIC_CSR + STIC_CSR3];
					if (colord == 7) colord = m_stic_registers[STIC_CSR + STIC_CSR3];
					intv_plot_box(bitmap, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, colora);
					intv_plot_box(bitmap, (x0 + col * STIC_CARD_WIDTH + STIC_CSQM_WIDTH)) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, colorb);
					intv_plot_box(bitmap, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT + STIC_CSQM_HEIGHT) * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, colorc);
					intv_plot_box(bitmap, (x0 + col * STIC_CARD_WIDTH + STIC_CSQM_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT + STIC_CSQM_HEIGHT) * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, colord);
				}
				else // normal color stack mode
				{
					if (n_bit) // next color
					{
						m_color_stack_offset += 1;
						m_color_stack_offset &= (STIC_CSRS - 1);
					}

					if (p_bit) // pastel color set
						fgcolor = (value & STIC_CSTM_FG20) + 8;
					else
						fgcolor = value & STIC_CSTM_FG20;

					bgcolor = m_stic_registers[STIC_CSR + m_color_stack_offset];
					code = (value & STIC_CSTM_C)>>3;

					if (g_bit) // read from gram
					{
						code &= (STIC_CSTM_C50 >> 3);  // keep from going outside the array
						//if (m_gramdirtybytes[code] == 1)
						{
							decodechar(m_gfxdecode->gfx(1),
										code,
										m_gram,
										machine().config()->gfxdecodeinfo[1].gfxlayout);
							m_gramdirtybytes[code] = 0;
						}
						// Draw GRAM char
						drawgfx(bitmap,m_gfxdecode->gfx(1),
								code,
								bgcolor*16+fgcolor,
								0,0, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE,
								0,transparency,bgcolor);

						for(j=0;j<8;j++)
						{
							//intv_set_pixel(bitmap, (x0 + col * STIC_CARD_WIDTH + j) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT + 7) * STIC_Y_SCALE + 1, 1);
						}

					}
					else // read from grom
					{
						drawgfx(bitmap,m_gfxdecode->gfx(0),
								code,
								bgcolor*16+fgcolor,
								0,0, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE,
								0,transparency,bgcolor);

						for(j=0;j<8;j++)
						{
							//intv_set_pixel(bitmap, (x0 + col * STIC_CARD_WIDTH + j) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT + 7) * STIC_Y_SCALE + 1, 2);
						}
					}
				}
				offs++;
			} // next col
		} // next row
	}
	else
	{
		// fg/bg mode goes here
		for(row = 0; row < STIC_BACKTAB_HEIGHT; row++)
		{
			for(col = 0; col < STIC_BACKTAB_WIDTH; col++)
			{
				value = m_ram16[offs];
				fgcolor = value & STIC_FBM_FG;
				bgcolor = ((value & STIC_FBM_BG2) >> 11) + ((value & STIC_FBM_BG310) >> 9);
				code = (value & STIC_FBM_C) >> 3;

				if (value & STIC_FBM_SEL) // read for GRAM
				{
					//if (m_gramdirtybytes[code] == 1)
					{
						decodechar(m_gfxdecode->gfx(1),
									code,
									m_gram,
									machine().config()->gfxdecodeinfo[1].gfxlayout);
						m_gramdirtybytes[code] = 0;
					}
					// Draw GRAM char
					drawgfx(bitmap,m_gfxdecode->gfx(1),
							code,
							bgcolor*16+fgcolor,
							0,0, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE,
							0,transparency,bgcolor);
				}
				else // read from GROM
				{
					drawgfx(bitmap,m_gfxdecode->gfx(0),
							code,
							bgcolor*16+fgcolor,
							0,0, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE,
							0,transparency,bgcolor);
				}
				offs++;
			} // next col
		} // next row
	}
}
#endif

/* TBD: need to handle sprites behind foreground? */
#ifdef UNUSED_FUNCTION
void stic_device::draw_sprites(bitmap_ind16 &bitmap, int behind_foreground)
{
	int code;
	int x0 = STIC_OVERSCAN_LEFT_WIDTH + m_col_delay - STIC_CARD_WIDTH;
	int y0 = STIC_OVERSCAN_TOP_HEIGHT + m_row_delay - STIC_CARD_HEIGHT;

	for (int i = STIC_MOBS - 1; i >= 0; --i)
	{
		intv_sprite_type *s = &m_sprite[i];
		if (s->visible && (s->behind_foreground == behind_foreground))
		{
			code = s->card;
			if (!s->grom)
			{
				code %= 64;  // keep from going outside the array
				if (s->yres == 1)
				{
					//if (m_gramdirtybytes[code] == 1)
					{
						decodechar(m_gfxdecode->gfx(1),
									code,
									m_gram,
									machine().config()->gfxdecodeinfo[1].gfxlayout);
						m_gramdirtybytes[code] = 0;
					}
					// Draw GRAM char
					m_gfxdecode->gfx(1)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE,
											0x8000 * s->xsize, 0x8000 * s->ysize,0);
				}
				else
				{
					//if ((m_gramdirtybytes[code] == 1) || (m_gramdirtybytes[code+1] == 1))
					{
						decodechar(m_gfxdecode->gfx(1),
									code,
									m_gram,
									machine().config()->gfxdecodeinfo[1].gfxlayout);
						decodechar(m_gfxdecode->gfx(1),
									code+1,
									m_gram,
									machine().config()->gfxdecodeinfo[1].gfxlayout);
						m_gramdirtybytes[code] = 0;
						m_gramdirtybytes[code+1] = 0;
					}
					// Draw GRAM char
					m_gfxdecode->gfx(1)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE + s->yflip * s->ysize * STIC_CARD_HEIGHT,
											0x8000*s->xsize, 0x8000*s->ysize,0);
					m_gfxdecode->gfx(1)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code+1,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE + (1 - s->yflip) * s->ysize * STIC_CARD_HEIGHT,
											0x8000*s->xsize, 0x8000*s->ysize,0);
				}
			}
			else
			{
				if (s->yres == 1)
				{
					// Draw GROM char
					m_gfxdecode->gfx(0)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE,
											0x8000*s->xsize, 0x8000*s->ysize,0);
				}
				else
				{
					m_gfxdecode->gfx(0)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE + s->yflip * s->ysize * STIC_CARD_HEIGHT,
											0x8000*s->xsize, 0x8000*s->ysize,0);
					m_gfxdecode->gfx(0)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code+1,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE + (1 - s->yflip) * s->ysize * STIC_CARD_HEIGHT,
											0x8000*s->xsize, 0x8000*s->ysize,0);
				}
			}
		}
	}
}
#endif

void stic_device::draw_borders(bitmap_ind16 &bitmap)
{
	intv_plot_box(bitmap, 0, 0, (STIC_OVERSCAN_LEFT_WIDTH + (m_left_edge_inhibit ? STIC_CARD_WIDTH : m_col_delay)) * STIC_X_SCALE, (STIC_OVERSCAN_TOP_HEIGHT + STIC_BACKTAB_HEIGHT * STIC_CARD_HEIGHT + STIC_OVERSCAN_BOTTOM_HEIGHT) * STIC_Y_SCALE, m_border_color);
	intv_plot_box(bitmap, (STIC_OVERSCAN_LEFT_WIDTH + STIC_BACKTAB_WIDTH * STIC_CARD_WIDTH - 1) * STIC_X_SCALE, 0, STIC_OVERSCAN_RIGHT_WIDTH, (STIC_OVERSCAN_TOP_HEIGHT + STIC_BACKTAB_HEIGHT * STIC_CARD_HEIGHT + STIC_OVERSCAN_BOTTOM_HEIGHT) * STIC_Y_SCALE, m_border_color);

	intv_plot_box(bitmap, 0, 0, (STIC_OVERSCAN_LEFT_WIDTH + STIC_BACKTAB_WIDTH * STIC_CARD_WIDTH - 1 + STIC_OVERSCAN_RIGHT_WIDTH) * STIC_X_SCALE, (STIC_OVERSCAN_TOP_HEIGHT + (m_top_edge_inhibit ? STIC_CARD_HEIGHT : m_row_delay)) * STIC_Y_SCALE, m_border_color);
	intv_plot_box(bitmap, 0, (STIC_OVERSCAN_TOP_HEIGHT + STIC_BACKTAB_HEIGHT * STIC_CARD_HEIGHT) * STIC_Y_SCALE, (STIC_OVERSCAN_LEFT_WIDTH + STIC_BACKTAB_WIDTH * STIC_CARD_WIDTH - 1 + STIC_OVERSCAN_RIGHT_WIDTH) * STIC_X_SCALE, STIC_OVERSCAN_BOTTOM_HEIGHT * STIC_Y_SCALE, m_border_color);
}

void stic_device::screenrefresh()
{
	if (m_stic_handshake != 0)
	{
		m_stic_handshake = 0;
		// Render the background
		render_background(m_bitmap);
		// Render the sprites into their buffers
		render_sprites();
		for (auto & elem : m_sprite)
			elem.collision = 0;
		// Copy the sprites to the background
		copy_sprites_to_background(m_bitmap);
		determine_sprite_collisions();
		for (int i = 0; i < STIC_MOBS; i++)
			m_stic_registers[STIC_MCR + i] |= m_sprite[i].collision;
		/* draw the screen borders if enabled */
		draw_borders(m_bitmap);
	}
	else
	{
		/* STIC disabled, just fill with border color */
		m_bitmap.fill(SET_COLOR(m_border_color));
	}
}



READ16_MEMBER( stic_device::read )
{
	//logerror("%x = stic_r(%x)\n",0,offset);
	switch (offset)
	{
		case STIC_MXR + STIC_MOB0:
		case STIC_MXR + STIC_MOB1:
		case STIC_MXR + STIC_MOB2:
		case STIC_MXR + STIC_MOB3:
		case STIC_MXR + STIC_MOB4:
		case STIC_MXR + STIC_MOB5:
		case STIC_MXR + STIC_MOB6:
		case STIC_MXR + STIC_MOB7:
			return 0x3800 | (m_stic_registers[offset] & 0x07FF);
		case STIC_MYR + STIC_MOB0:
		case STIC_MYR + STIC_MOB1:
		case STIC_MYR + STIC_MOB2:
		case STIC_MYR + STIC_MOB3:
		case STIC_MYR + STIC_MOB4:
		case STIC_MYR + STIC_MOB5:
		case STIC_MYR + STIC_MOB6:
		case STIC_MYR + STIC_MOB7:
			return 0x3000 | (m_stic_registers[offset] & 0x0FFF);
		case STIC_MAR + STIC_MOB0:
		case STIC_MAR + STIC_MOB1:
		case STIC_MAR + STIC_MOB2:
		case STIC_MAR + STIC_MOB3:
		case STIC_MAR + STIC_MOB4:
		case STIC_MAR + STIC_MOB5:
		case STIC_MAR + STIC_MOB6:
		case STIC_MAR + STIC_MOB7:
			return m_stic_registers[offset] & 0x3FFF;
		case STIC_MCR + STIC_MOB0:
		case STIC_MCR + STIC_MOB1:
		case STIC_MCR + STIC_MOB2:
		case STIC_MCR + STIC_MOB3:
		case STIC_MCR + STIC_MOB4:
		case STIC_MCR + STIC_MOB5:
		case STIC_MCR + STIC_MOB6:
		case STIC_MCR + STIC_MOB7:
			return 0x3C00 | (m_stic_registers[offset] & 0x03FF);
		case STIC_GMR:
			m_color_stack_mode = 1;
			//logerror("Setting color stack mode\n");
			/*** fall through ***/
		case STIC_DER:
			return 0x3FFF;
		case STIC_CSR + STIC_CSR0:
		case STIC_CSR + STIC_CSR1:
		case STIC_CSR + STIC_CSR2:
		case STIC_CSR + STIC_CSR3:
		case STIC_BCR:
			return 0x3FF0 | (m_stic_registers[offset] & 0x000F);
		case STIC_HDR:
		case STIC_VDR:
			return 0x3FF8 | (m_stic_registers[offset] & 0x0007);
		case STIC_CBR:
			return 0x3FFC | (m_stic_registers[offset] & 0x0003);
		default:
			//logerror("unmapped read from STIC register %02X\n", offset);
			return 0x3FFF;
	}
}

WRITE16_MEMBER( stic_device::write )
{
	intv_sprite_type *s;

	//logerror("stic_w(%x) = %x\n",offset,data);
	switch (offset)
	{
		// X Positions
		case STIC_MXR + STIC_MOB0:
		case STIC_MXR + STIC_MOB1:
		case STIC_MXR + STIC_MOB2:
		case STIC_MXR + STIC_MOB3:
		case STIC_MXR + STIC_MOB4:
		case STIC_MXR + STIC_MOB5:
		case STIC_MXR + STIC_MOB6:
		case STIC_MXR + STIC_MOB7:
			s =  &m_sprite[offset & (STIC_MOBS - 1)];
			s->doublex = !!(data & STIC_MXR_XSIZE);
			s->visible = !!(data & STIC_MXR_VIS);
			s->coll = !!(data & STIC_MXR_COL);
			s->xpos = (data & STIC_MXR_X);
			break;
		// Y Positions
		case STIC_MYR + STIC_MOB0:
		case STIC_MYR + STIC_MOB1:
		case STIC_MYR + STIC_MOB2:
		case STIC_MYR + STIC_MOB3:
		case STIC_MYR + STIC_MOB4:
		case STIC_MYR + STIC_MOB5:
		case STIC_MYR + STIC_MOB6:
		case STIC_MYR + STIC_MOB7:
			s =  &m_sprite[offset & (STIC_MOBS - 1)];
			s->yflip = !!(data & STIC_MYR_YFLIP);
			s->xflip = !!(data & STIC_MYR_XFLIP);
			s->quady = !!(data & STIC_MYR_YSIZE);
			s->doubley = !!(data & STIC_MYR_YFULL);
			s->doubleyres = !!(data & STIC_MYR_YRES);
			s->ypos = (data & STIC_MYR_Y);
			break;
		// Attributes
		case STIC_MAR + STIC_MOB0:
		case STIC_MAR + STIC_MOB1:
		case STIC_MAR + STIC_MOB2:
		case STIC_MAR + STIC_MOB3:
		case STIC_MAR + STIC_MOB4:
		case STIC_MAR + STIC_MOB5:
		case STIC_MAR + STIC_MOB6:
		case STIC_MAR + STIC_MOB7:
			s =  &m_sprite[offset & (STIC_MOBS - 1)];
			s->behind_foreground = !!(data & STIC_MAR_PRI);
			s->grom = !(data & STIC_MAR_SEL);
			s->card = ((data & STIC_MAR_C) >> 3);
			s->color = ((data & STIC_MAR_FG3) >> 9) | (data & STIC_MAR_FG20);
			break;
		// Collision Detection - TBD
		case STIC_MCR + STIC_MOB0:
		case STIC_MCR + STIC_MOB1:
		case STIC_MCR + STIC_MOB2:
		case STIC_MCR + STIC_MOB3:
		case STIC_MCR + STIC_MOB4:
		case STIC_MCR + STIC_MOB5:
		case STIC_MCR + STIC_MOB6:
		case STIC_MCR + STIC_MOB7:
			// a MOB's own collision bit is *always* zero, even if a
			// one is poked into it
			data &= ~(1 << (offset & (STIC_MOBS - 1)));
			break;
		// Display enable
		case STIC_DER:
			//logerror("***Writing a %x to the STIC handshake\n",data);
			m_stic_handshake = 1;
			break;
		// Graphics Mode
		case STIC_GMR:
			m_color_stack_mode = 0;
			break;
		// Color Stack
		case STIC_CSR + STIC_CSR0:
		case STIC_CSR + STIC_CSR1:
		case STIC_CSR + STIC_CSR2:
		case STIC_CSR + STIC_CSR3:
			logerror("Setting color_stack[%x] = %x (%x)\n", offset & (STIC_CSRS - 1),data & STIC_CSR_BG, space.device().safe_pc());
			break;
		// Border Color
		case STIC_BCR:
			//logerror("***Writing a %x to the border color\n",data);
			m_border_color = data & STIC_BCR_BC;
			break;
		// Framing
		case STIC_HDR:
			m_col_delay = data & STIC_HDR_DEL;
			break;
		case STIC_VDR:
			m_row_delay = data & STIC_VDR_DEL;
			break;
		case STIC_CBR:
			m_left_edge_inhibit = (data & STIC_CBR_COL);
			m_top_edge_inhibit = (data & STIC_CBR_ROW) >> 1;
			break;
		default:
			//logerror("unmapped write to STIC register %02X: %04X\n", offset, data);
			break;
	}

	if (offset < ARRAY_LENGTH(m_stic_registers))
		m_stic_registers[offset] = data;
}


READ16_MEMBER( stic_device::gram_read )
{
	return m_gram[offset];
}

WRITE16_MEMBER( stic_device::gram_write )
{
	data &= 0xff;
	m_gram[offset] = data;
	m_gramdirtybytes[offset] = 1;
	m_gramdirty = 1;
}



UINT32 stic_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}
