#include "emu.h"
#include "video/stic.h"
#include "includes/intv.h"

#define FOREGROUND_BIT 0x0010

// conversion from Intellivision color to internal representation
#define SET_COLOR(c)	((c * 2) + 1)
#define GET_COLOR(c)	((c - 1) / 2)

/* initialized to non-zero, because we divide by it */

INLINE void intv_set_pixel(intv_state *state, bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	int w, h;

	// output scaling
	x *= state->m_x_scale;
	y *= state->m_y_scale;
	color = SET_COLOR(color);

	for (h = 0; h < state->m_y_scale; h++)
		for (w = 0; w < state->m_x_scale; w++)
			bitmap.pix16(y + h, x + w) = color;
}

INLINE UINT32 intv_get_pixel(intv_state *state, bitmap_ind16 &bitmap, int x, int y)
{
	return GET_COLOR(bitmap.pix16(y * state->m_y_scale, x * state->m_x_scale));
}

INLINE void intv_plot_box(intv_state *state, bitmap_ind16 &bm, int x, int y, int w, int h, int color)
{
	bm.plot_box(x * state->m_x_scale, y * state->m_y_scale, w * state->m_x_scale, h * state->m_y_scale, SET_COLOR(color));
}

void intv_state::video_start()
{
	//int i,j,k;

	m_tms9927_num_rows = 25;

	machine().primary_screen->register_screen_bitmap(m_bitmap);

#if 0
	for (i = 0; i < STIC_MOBS; i++)
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
		for (j = 0; j < 16; j++)
		{
			for (k = 0; k < 128; k++)
			{
				m_sprite_buffers[i][j][k] = 0;
			}
		}
	}
	for(i = 0; i < STIC_REGISTERS; i++)
	{
		m_stic_registers[i] = 0;
	}
	m_color_stack_mode = 0;
	m_color_stack_offset = 0;
	m_stic_handshake = 0;
	m_border_color = 0;
	m_col_delay = 0;
	m_row_delay = 0;
	m_left_edge_inhibit = 0;
	m_top_edge_inhibit = 0;

	m_gramdirty = 1;
	for(i=0;i<64;i++)
	{
		m_gram[i] = 0;
		m_gramdirtybytes[i] = 1;
	}
#endif
}


static int sprites_collide(intv_state *state, int spriteNum1, int spriteNum2)
{
	INT16 x0, y0, w0, h0, x1, y1, w1, h1, x2, y2, w2, h2;

	intv_sprite_type* s1 = &state->m_sprite[spriteNum1];
	intv_sprite_type* s2 = &state->m_sprite[spriteNum2];

	x0 = STIC_OVERSCAN_LEFT_WIDTH + state->m_col_delay - STIC_CARD_WIDTH;
	y0 = STIC_OVERSCAN_TOP_HEIGHT + state->m_row_delay - STIC_CARD_HEIGHT;
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
			if (state->m_sprite_buffers[spriteNum1][x0 + x1][y0 + y1] &&
				state->m_sprite_buffers[spriteNum2][x0 + x2][y0 + y2])
				return TRUE;
		}
	}

	return FALSE;
}

static void determine_sprite_collisions(intv_state *state)
{
	// check sprite to sprite collisions
	int i, j;
	for (i = 0; i < STIC_MOBS - 1; i++)
	{
		intv_sprite_type* s1 = &state->m_sprite[i];
		if (s1->xpos == 0 || !s1->coll)
			continue;

		for (j = i + 1; j < STIC_MOBS; j++)
		{
			intv_sprite_type* s2 = &state->m_sprite[j];
			if (s2->xpos == 0 || !s2->coll)
				continue;

			if (sprites_collide(state, i, j))
			{
				s1->collision |= (1 << j);
				s2->collision |= (1 << i);
			}
		}
	}
}

static void render_sprites(running_machine &machine)
{
	intv_state *state = machine.driver_data<intv_state>();
	INT32 cardMemoryLocation, pixelSize;
	INT32 spritePixelHeight;
	INT32 nextMemoryLocation;
	INT32 nextData;
	INT32 nextX;
	INT32 nextY;
	INT32 xInc;
	INT32 i, j, k;

	UINT8* memory = state->memregion("maincpu")->base();

	for (i = 0; i < STIC_MOBS; i++)
	{
		intv_sprite_type* s = &state->m_sprite[i];

		if (s->grom)
			cardMemoryLocation = (s->card * STIC_CARD_HEIGHT);
		else
			cardMemoryLocation = ((s->card & 0x003F) * STIC_CARD_HEIGHT);

		pixelSize = (s->quady ? 4 : 1) * (s->doubley ? 2 : 1);
		spritePixelHeight = pixelSize * (s->doubleyres ? 2 : 1) * STIC_CARD_HEIGHT;

		for (j = 0; j < spritePixelHeight; j++)
		{
			nextMemoryLocation = (cardMemoryLocation + (j/pixelSize));
			if (s->grom)
				nextData = memory[(0x3000+nextMemoryLocation)<<1];
			else if (nextMemoryLocation < 0x200)
				nextData = state->m_gram[nextMemoryLocation];
			else
				nextData = 0xFFFF;
			nextX = (s->xflip ? ((s->doublex ? 2 : 1) * STIC_CARD_WIDTH - 1) : 0);
			nextY = (s->yflip ? (spritePixelHeight - j - 1) : j);
			xInc = (s->xflip ? -1: 1);

			for (k = 0; k < STIC_CARD_WIDTH * (1 + s->doublex); k++)
			{
				state->m_sprite_buffers[i][nextX + k * xInc][nextY] = (nextData & (1 << ((STIC_CARD_WIDTH - 1) - k / (1 + s->doublex)))) != 0;
			}
		}
	}
}

static void render_line(running_machine &machine, bitmap_ind16 &bitmap,
	UINT8 nextByte, UINT16 x, UINT16 y, UINT8 fgcolor, UINT8 bgcolor)
{
	intv_state *state = machine.driver_data<intv_state>();
	UINT32 color;
	UINT8 i;

	for (i = 0; i < STIC_CARD_WIDTH; i++)
	{
		color = (nextByte & (1 << ((STIC_CARD_WIDTH - 1) - i)) ? fgcolor : bgcolor);
		intv_set_pixel(state, bitmap, x+i, y, color);
		intv_set_pixel(state, bitmap, x+i, y+1, color);
	}
}

static void render_colored_squares(running_machine &machine, bitmap_ind16 &bitmap,
	UINT16 x, UINT16 y, UINT8 color0, UINT8 color1, UINT8 color2, UINT8 color3)
{
	intv_state *state = machine.driver_data<intv_state>();

	intv_plot_box(state, bitmap, x, y, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, color0);
	intv_plot_box(state, bitmap, x + STIC_CSQM_WIDTH * STIC_X_SCALE, y, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, color1);
	intv_plot_box(state, bitmap, x, y + STIC_CSQM_HEIGHT * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, color2);
	intv_plot_box(state, bitmap, x + STIC_CSQM_WIDTH * STIC_X_SCALE, y + STIC_CSQM_HEIGHT * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, color3);
}

static void render_color_stack_mode(running_machine &machine, bitmap_ind16 &bitmap)
{
	intv_state *state = machine.driver_data<intv_state>();
	INT16 w, h, nextx, nexty;
	UINT8 csPtr = 0;
	UINT16 nextCard;
	UINT8 *ram = state->memregion("maincpu")->base();

	for (h = 0, nexty = (STIC_OVERSCAN_TOP_HEIGHT + state->m_row_delay) * STIC_Y_SCALE;
		 h < STIC_BACKTAB_HEIGHT;
		 h++, nexty += STIC_CARD_HEIGHT * STIC_Y_SCALE)
	{
		for (w = 0, nextx = (STIC_OVERSCAN_LEFT_WIDTH + state->m_col_delay) * STIC_X_SCALE;
			 w < STIC_BACKTAB_WIDTH;
			 w++, nextx += STIC_CARD_WIDTH * STIC_X_SCALE)
		{
			nextCard = state->m_backtab_buffer[h][w];

			// colored squares mode
			if ((nextCard & (STIC_CSTM_FG3|STIC_CSTM_SEL)) == STIC_CSTM_FG3)
			{
				UINT8 csColor = state->m_stic_registers[STIC_CSR + csPtr];
				UINT8 color0 = nextCard & STIC_CSQM_A;
				UINT8 color1 = (nextCard & STIC_CSQM_B) >> 3;
				UINT8 color2 = (nextCard & STIC_CSQM_C) >> 6;
				UINT8 color3 = ((nextCard & STIC_CSQM_D2) >> 11) |
						((nextCard & (STIC_CSQM_D10)) >> 9);
				render_colored_squares(machine, bitmap, nextx, nexty,
						(color0 == 7 ? csColor : (color0 | FOREGROUND_BIT)),
						(color1 == 7 ? csColor : (color1 | FOREGROUND_BIT)),
						(color2 == 7 ? csColor : (color2 | FOREGROUND_BIT)),
						(color3 == 7 ? csColor : (color3 | FOREGROUND_BIT)));
			}
			//color stack mode
			else
			{
				UINT8 isGrom, j;
				UINT16 memoryLocation, fgcolor, bgcolor;
				UINT8* memory;

				//advance the color pointer, if necessary
				if (nextCard & STIC_CSTM_ADV)
					csPtr = (csPtr+1) & (STIC_CSRS - 1);

				fgcolor = ((nextCard & STIC_CSTM_FG3) >> 9) |
						(nextCard & (STIC_CSTM_FG20)) | FOREGROUND_BIT;
				bgcolor = state->m_stic_registers[STIC_CSR + csPtr] & STIC_CSR_BG;

				isGrom = !(nextCard & STIC_CSTM_SEL);
				if (isGrom)
				{
					memoryLocation = 0x3000 + (nextCard & STIC_CSTM_C);
					memory = ram;
					for (j = 0; j < STIC_CARD_HEIGHT; j++)
						render_line(machine, bitmap, memory[(memoryLocation + j) * 2],
							nextx, nexty + j * STIC_Y_SCALE, fgcolor, bgcolor);
				}
				else
				{
					memoryLocation = (nextCard & STIC_CSTM_C50);
					memory = state->m_gram;
					for (j = 0; j < STIC_CARD_HEIGHT; j++)
						render_line(machine, bitmap, memory[memoryLocation + j],
								nextx, nexty + j * STIC_Y_SCALE, fgcolor, bgcolor);
				}
			}
		}
	}
}

static void render_fg_bg_mode(running_machine &machine, bitmap_ind16 &bitmap)
{
	intv_state *state = machine.driver_data<intv_state>();
	INT16 w, h, nextx, nexty;
	UINT8 j, isGrom, fgcolor, bgcolor;
	UINT16 nextCard, memoryLocation;
	UINT8* memory;
	UINT8* ram = state->memregion("maincpu")->base();

	for (h = 0, nexty = (STIC_OVERSCAN_TOP_HEIGHT + state->m_row_delay) * STIC_Y_SCALE;
		 h < STIC_BACKTAB_HEIGHT;
		 h++, nexty += STIC_CARD_HEIGHT * STIC_Y_SCALE)
	{
		for (w = 0, nextx = (STIC_OVERSCAN_LEFT_WIDTH + state->m_col_delay) * STIC_X_SCALE;
			 w < STIC_BACKTAB_WIDTH;
			 w++, nextx += STIC_CARD_WIDTH * STIC_X_SCALE)
		{
			nextCard = state->m_backtab_buffer[h][w];
			fgcolor = (nextCard & STIC_FBM_FG) | FOREGROUND_BIT;
			bgcolor = ((nextCard & STIC_FBM_BG2) >> 11) |
					((nextCard & STIC_FBM_BG310) >> 9);

			isGrom = !(nextCard & STIC_FBM_SEL);
			if (isGrom)
			{
				memoryLocation = 0x3000 + (nextCard & STIC_FBM_C);
				memory = ram;
				for (j = 0; j < STIC_CARD_HEIGHT; j++)
					render_line(machine, bitmap, memory[(memoryLocation + j) * 2],
							nextx, nexty + j * STIC_Y_SCALE, fgcolor, bgcolor);
			}
			else
			{
				memoryLocation = (nextCard & STIC_FBM_C);
				memory = state->m_gram;
				for (j = 0; j < STIC_CARD_HEIGHT; j++)
					render_line(machine, bitmap, memory[memoryLocation + j],
							nextx, nexty + j * STIC_Y_SCALE, fgcolor, bgcolor);
			}
		}
	}
}

static void copy_sprites_to_background(running_machine &machine, bitmap_ind16 &bitmap)
{
	intv_state *state = machine.driver_data<intv_state>();
	UINT8 width, currentPixel;
	UINT8 borderCollision, foregroundCollision;
	UINT8 spritePixelHeight, x, y;
	INT16 leftX, nextY, i;
	INT16 leftBorder, rightBorder, topBorder, bottomBorder;
	INT32 nextX;

	for (i = STIC_MOBS - 1; i >= 0; i--)
	{
		intv_sprite_type *s = &state->m_sprite[i];
		if (s->xpos == 0 || (!s->coll && !s->visible))
			continue;

		borderCollision = FALSE;
		foregroundCollision = FALSE;

		spritePixelHeight = (s->quady ? 4 : 1) * (s->doubley ? 2 : 1) * (s->doubleyres ? 2 : 1) * STIC_CARD_HEIGHT;
		width = (s->doublex ? 2 : 1) * STIC_CARD_WIDTH;

		leftX = (s->xpos - STIC_CARD_WIDTH + STIC_OVERSCAN_LEFT_WIDTH + state->m_col_delay) * STIC_X_SCALE;
		nextY = (s->ypos - STIC_CARD_HEIGHT + STIC_OVERSCAN_TOP_HEIGHT + state->m_row_delay) * STIC_Y_SCALE;

		leftBorder =  (STIC_OVERSCAN_LEFT_WIDTH + (state->m_left_edge_inhibit ? STIC_CARD_WIDTH : 0)) * STIC_X_SCALE;
		rightBorder = (STIC_OVERSCAN_LEFT_WIDTH + STIC_BACKTAB_WIDTH * STIC_CARD_WIDTH - 1 - 1) * STIC_X_SCALE;
		topBorder = (STIC_OVERSCAN_TOP_HEIGHT + (state->m_top_edge_inhibit ? STIC_CARD_HEIGHT : 0)) * STIC_Y_SCALE;
		bottomBorder = (STIC_OVERSCAN_TOP_HEIGHT + STIC_BACKTAB_HEIGHT * STIC_CARD_HEIGHT) * STIC_Y_SCALE - 1;

		for (y = 0; y < spritePixelHeight; y++)
		{
			for (x = 0; x < width; x++)
			{
				//if this sprite pixel is not on, then don't paint it
				if (!state->m_sprite_buffers[i][x][y])
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

				currentPixel = intv_get_pixel(state, bitmap, nextX, nextY);

				//check for foreground collision
				if (currentPixel & FOREGROUND_BIT)
				{
					foregroundCollision = TRUE;
					if (s->behind_foreground)
						continue;
				}

				if (s->visible)
				{
					intv_set_pixel(state, bitmap, nextX, nextY, s->color | (currentPixel & FOREGROUND_BIT));
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

static void render_background(running_machine &machine, bitmap_ind16 &bitmap)
{
	intv_state *state = machine.driver_data<intv_state>();
	if (state->m_color_stack_mode)
		render_color_stack_mode(machine, bitmap);
	else
		render_fg_bg_mode(machine, bitmap);
}

#ifdef UNUSED_CODE
static void draw_background(running_machine &machine, bitmap_ind16 &bitmap, int transparency)
{
	intv_state *state = machine.driver_data<intv_state>();
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

	int x0 = STIC_OVERSCAN_LEFT_WIDTH + state->m_col_delay;
	int y0 = STIC_OVERSCAN_TOP_HEIGHT + state->m_row_delay;

	if (state->m_color_stack_mode == 1)
	{
		state->m_color_stack_offset = 0;
		for(row = 0; row < STIC_BACKTAB_HEIGHT; row++)
		{
			for(col = 0; col < STIC_BACKTAB_WIDTH; col++)
			{
				value = state->m_ram16[offs];

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
					if (colora == 7) colora = state->m_stic_registers[STIC_CSR + STIC_CSR3];
					if (colorb == 7) colorb = state->m_stic_registers[STIC_CSR + STIC_CSR3];
					if (colorc == 7) colorc = state->m_stic_registers[STIC_CSR + STIC_CSR3];
					if (colord == 7) colord = state->m_stic_registers[STIC_CSR + STIC_CSR3];
					intv_plot_box(state, bitmap, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, colora);
					intv_plot_box(state, bitmap, (x0 + col * STIC_CARD_WIDTH + STIC_CSQM_WIDTH)) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, colorb);
					intv_plot_box(state, bitmap, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT + STIC_CSQM_HEIGHT) * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, colorc);
					intv_plot_box(state, bitmap, (x0 + col * STIC_CARD_WIDTH + STIC_CSQM_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT + STIC_CSQM_HEIGHT) * STIC_Y_SCALE, STIC_CSQM_WIDTH * STIC_X_SCALE, STIC_CSQM_HEIGHT * STIC_Y_SCALE, colord);
				}
				else // normal color stack mode
				{
					if (n_bit) // next color
					{
						state->m_color_stack_offset += 1;
						state->m_color_stack_offset &= (STIC_CSRS - 1);
					}

					if (p_bit) // pastel color set
						fgcolor = (value & STIC_CSTM_FG20) + 8;
					else
						fgcolor = value & STIC_CSTM_FG20;

					bgcolor = state->m_stic_registers[STIC_CSR + state->m_color_stack_offset];
					code = (value & STIC_CSTM_C)>>3;

					if (g_bit) // read from gram
					{
						code &= (STIC_CSTM_C50 >> 3);  // keep from going outside the array
						//if (state->m_gramdirtybytes[code] == 1)
						{
							decodechar(machine.gfx[1],
								code,
								state->m_gram,
								machine.config()->gfxdecodeinfo[1].gfxlayout);
							state->m_gramdirtybytes[code] = 0;
						}
						// Draw GRAM char
						drawgfx(bitmap,machine.gfx[1],
							code,
							bgcolor*16+fgcolor,
							0,0, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE,
							0,transparency,bgcolor);

						for(j=0;j<8;j++)
						{
							//intv_set_pixel(state, bitmap, (x0 + col * STIC_CARD_WIDTH + j) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT + 7) * STIC_Y_SCALE + 1, 1);
						}

					}
					else // read from grom
					{
						drawgfx(bitmap,machine.gfx[0],
							code,
							bgcolor*16+fgcolor,
							0,0, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE,
							0,transparency,bgcolor);

						for(j=0;j<8;j++)
						{
							//intv_set_pixel(state, bitmap, (x0 + col * STIC_CARD_WIDTH + j) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT + 7) * STIC_Y_SCALE + 1, 2);
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
				value = state->m_ram16[offs];
				fgcolor = value & STIC_FBM_FG;
				bgcolor = ((value & STIC_FBM_BG2) >> 11) + ((value & STIC_FBM_BG310) >> 9);
				code = (value & STIC_FBM_C) >> 3;

				if (value & STIC_FBM_SEL) // read for GRAM
				{
					//if (state->m_gramdirtybytes[code] == 1)
					{
						decodechar(machine.gfx[1],
							code,
							state->m_gram,
							machine.config()->gfxdecodeinfo[1].gfxlayout);
						state->m_gramdirtybytes[code] = 0;
					}
					// Draw GRAM char
					drawgfx(bitmap,machine.gfx[1],
						code,
						bgcolor*16+fgcolor,
						0,0, (x0 + col * STIC_CARD_WIDTH) * STIC_X_SCALE, (y0 + row * STIC_CARD_HEIGHT) * STIC_Y_SCALE,
						0,transparency,bgcolor);
				}
				else // read from GROM
				{
					drawgfx(bitmap,machine.gfx[0],
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
static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, int behind_foreground)
{
	intv_state *state = machine.driver_data<intv_state>();
	int i;
	int code;
	int x0 = STIC_OVERSCAN_LEFT_WIDTH + state->m_col_delay - STIC_CARD_WIDTH;
	int y0 = STIC_OVERSCAN_TOP_HEIGHT + state->m_row_delay - STIC_CARD_HEIGHT;

	for(i = STIC_MOBS - 1; i >= 0; --i)
	{
		intv_sprite_type *s = &state->m_sprite[i];
		if (s->visible && (s->behind_foreground == behind_foreground))
		{
			code = s->card;
			if (!s->grom)
			{
				code %= 64;  // keep from going outside the array
				if (s->yres == 1)
				{
					//if (state->m_gramdirtybytes[code] == 1)
					{
						decodechar(machine.gfx[1],
							code,
							state->m_gram,
							machine.config()->gfxdecodeinfo[1].gfxlayout);
						state->m_gramdirtybytes[code] = 0;
					}
					// Draw GRAM char
					drawgfxzoom_transpen(bitmap,&machine.screen[0].visarea,machine.gfx[1],
						code,
						s->color,
						s->xflip,s->yflip,
						(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE,
						0x8000 * s->xsize, 0x8000 * s->ysize,0);
				}
				else
				{
					//if ((state->m_gramdirtybytes[code] == 1) || (state->m_gramdirtybytes[code+1] == 1))
					{
						decodechar(machine.gfx[1],
							code,
							state->m_gram,
							machine.config()->gfxdecodeinfo[1].gfxlayout);
						decodechar(machine.gfx[1],
							code+1,
							state->m_gram,
							machine.config()->gfxdecodeinfo[1].gfxlayout);
						state->m_gramdirtybytes[code] = 0;
						state->m_gramdirtybytes[code+1] = 0;
					}
					// Draw GRAM char
					drawgfxzoom_transpen(bitmap,&machine.screen[0].visarea,machine.gfx[1],
						code,
						s->color,
						s->xflip,s->yflip,
						(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE + s->yflip * s->ysize * STIC_CARD_HEIGHT,
						0x8000*s->xsize, 0x8000*s->ysize,0);
					drawgfxzoom_transpen(bitmap,&machine.screen[0].visarea,machine.gfx[1],
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
					drawgfxzoom_transpen(bitmap,&machine.screen[0].visarea,machine.gfx[0],
						code,
						s->color,
						s->xflip,s->yflip,
						(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE,
						0x8000*s->xsize, 0x8000*s->ysize,0);
				}
				else
				{
					drawgfxzoom_transpen(bitmap,&machine.screen[0].visarea,machine.gfx[0],
						code,
						s->color,
						s->xflip,s->yflip,
						(s->xpos + x0) * STIC_X_SCALE, (s->ypos + y0) * STIC_Y_SCALE + s->yflip * s->ysize * STIC_CARD_HEIGHT,
						0x8000*s->xsize, 0x8000*s->ysize,0);
					drawgfxzoom_transpen(bitmap,&machine.screen[0].visarea,machine.gfx[0],
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

static void draw_borders(running_machine &machine, bitmap_ind16 &bm)
{
	intv_state *state = machine.driver_data<intv_state>();

	intv_plot_box(state, bm, 0, 0, (STIC_OVERSCAN_LEFT_WIDTH + (state->m_left_edge_inhibit ? STIC_CARD_WIDTH : state->m_col_delay)) * STIC_X_SCALE, (STIC_OVERSCAN_TOP_HEIGHT + STIC_BACKTAB_HEIGHT * STIC_CARD_HEIGHT + STIC_OVERSCAN_BOTTOM_HEIGHT) * STIC_Y_SCALE, state->m_border_color);
	intv_plot_box(state, bm, (STIC_OVERSCAN_LEFT_WIDTH + STIC_BACKTAB_WIDTH * STIC_CARD_WIDTH - 1) * STIC_X_SCALE, 0, STIC_OVERSCAN_RIGHT_WIDTH, (STIC_OVERSCAN_TOP_HEIGHT + STIC_BACKTAB_HEIGHT * STIC_CARD_HEIGHT + STIC_OVERSCAN_BOTTOM_HEIGHT) * STIC_Y_SCALE, state->m_border_color);

	intv_plot_box(state, bm, 0, 0, (STIC_OVERSCAN_LEFT_WIDTH + STIC_BACKTAB_WIDTH * STIC_CARD_WIDTH - 1 + STIC_OVERSCAN_RIGHT_WIDTH) * STIC_X_SCALE, (STIC_OVERSCAN_TOP_HEIGHT + (state->m_top_edge_inhibit ? STIC_CARD_HEIGHT : state->m_row_delay)) * STIC_Y_SCALE, state->m_border_color);
	intv_plot_box(state, bm, 0, (STIC_OVERSCAN_TOP_HEIGHT + STIC_BACKTAB_HEIGHT * STIC_CARD_HEIGHT) * STIC_Y_SCALE, (STIC_OVERSCAN_LEFT_WIDTH + STIC_BACKTAB_WIDTH * STIC_CARD_WIDTH - 1 + STIC_OVERSCAN_RIGHT_WIDTH) * STIC_X_SCALE, STIC_OVERSCAN_BOTTOM_HEIGHT * STIC_Y_SCALE, state->m_border_color);
}

void intv_stic_screenrefresh(running_machine &machine)
{
	intv_state *state = machine.driver_data<intv_state>();
	int i;

	if (state->m_stic_handshake != 0)
	{
		state->m_stic_handshake = 0;
		// Render the background
		render_background(machine, state->m_bitmap);
		// Render the sprites into their buffers
		render_sprites(machine);
		for (i = 0; i < STIC_MOBS; i++) state->m_sprite[i].collision = 0;
		// Copy the sprites to the background
		copy_sprites_to_background(machine, state->m_bitmap);
		determine_sprite_collisions(state);
		for (i = 0; i < STIC_MOBS; i++) state->m_stic_registers[STIC_MCR + i] |= state->m_sprite[i].collision;
		/* draw the screen borders if enabled */
		draw_borders(machine, state->m_bitmap);
	}
	else
	{
		/* STIC disabled, just fill with border color */
		state->m_bitmap.fill(SET_COLOR(state->m_border_color));
	}
}


/* very rudimentary support for the tms9927 character generator IC */


 READ8_MEMBER( intv_state::intvkbd_tms9927_r )
{
	//intv_state *state = space.machine().driver_data<intv_state>();
	UINT8 rv;
	switch (offset)
	{
		case 8:
			rv = m_tms9927_cursor_row;
			break;
		case 9:
			/* note: this is 1-based */
			rv = m_tms9927_cursor_col;
			break;
		case 11:
			m_tms9927_last_row = (m_tms9927_last_row + 1) % m_tms9927_num_rows;
			rv = m_tms9927_last_row;
			break;
		default:
			rv = 0;
	}
	return rv;
}

WRITE8_MEMBER( intv_state::intvkbd_tms9927_w )
{
	//intv_state *state = space.machine().driver_data<intv_state>();
	switch (offset)
	{
		case 3:
			m_tms9927_num_rows = (data & 0x3f) + 1;
			break;
		case 6:
			m_tms9927_last_row = data;
			break;
		case 11:
			m_tms9927_last_row = (m_tms9927_last_row + 1) % m_tms9927_num_rows;
			break;
		case 12:
			/* note: this is 1-based */
			m_tms9927_cursor_col = data;
			break;
		case 13:
			m_tms9927_cursor_row = data;
			break;
	}
}

UINT32 intv_state::screen_update_intv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

UINT32 intv_state::screen_update_intvkbd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *videoram = m_videoram;
	int x,y,offs;
	int current_row;
//  char c;

	/* Draw the underlying INTV screen first */
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	/* if the intvkbd text is not blanked, overlay it */
	if (!m_intvkbd_text_blanked)
	{
		current_row = (m_tms9927_last_row + 1) % m_tms9927_num_rows;
		for(y=0;y<24;y++)
		{
			for(x=0;x<40;x++)
			{
				offs = current_row*64+x;
				drawgfx_transpen(bitmap, cliprect,
					screen.machine().gfx[1],
					videoram[offs],
					7, /* white */
					0,0,
					x<<3,y<<3, 0);
			}
			if (current_row == m_tms9927_cursor_row)
			{
				/* draw the cursor as a solid white block */
				/* (should use a filled rect here!) */
				drawgfx_transpen(bitmap, cliprect,
					screen.machine().gfx[1],
					191, /* a block */
					7,   /* white   */
					0,0,
					(m_tms9927_cursor_col-1)<<3,y<<3, 0);
			}
			current_row = (current_row + 1) % m_tms9927_num_rows;
		}
	}

#if 0
	// debugging
	c = tape_motor_mode_desc[m_tape_motor_mode][0];
	drawgfx_transpen(bitmap,&machine().screen[0].visarea, machine().gfx[1],
		c,
		1,
		0,0,
		0*8,0*8, 0);
	for(y=0;y<5;y++)
	{
		drawgfx_transpen(bitmap,&machine().screen[0].visarea, machine().gfx[1],
			m_tape_unknown_write[y]+'0',
			1,
			0,0,
			0*8,(y+2)*8, 0);
	}
	drawgfx_transpen(bitmap,&machine().screen[0].visarea, machine().gfx[1],
			m_tape_unknown_write[5]+'0',
			1,
			0,0,
			0*8,8*8, 0);
	drawgfx_transpen(bitmap,&machine().screen[0].visarea, machine().gfx[1],
			m_tape_interrupts_enabled+'0',
			1,
			0,0,
			0*8,10*8, 0);
#endif
	return 0;
}
