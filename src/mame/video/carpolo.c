// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

****************************************************************************/

#include "emu.h"
#include "includes/carpolo.h"


/* the screen elements' priorties determine their color */
#define CHARSET_COLOR_BASE  (0x00)
#define BACKGROUND_PEN      ((0x00 << 1) + 1)
#define FIELD_PEN           ((0x01 << 1) + 1)
#define CAR1_COLOR          (0x02)
#define LINE_PEN            ((0x03 << 1) + 1)
#define CAR4_COLOR          (0x04)
#define CAR3_COLOR          (0x05)
#define CAR2_COLOR          (0x06)
#define BALL_COLOR          (0x07)
#define NET_COLOR           (0x08)
#define LEFT_GOAL_COLOR     (0x09)
#define LEFT_GOAL_PEN       (0x18 + 0x08)
#define LEFT_SCORE_PEN      (0x18 + 0x06)
#define RIGHT_GOAL_COLOR    (0x0a)
#define RIGHT_GOAL_PEN      (0x18 + 0x18)
#define RIGHT_SCORE_PEN     (0x18 + 0x16)
#define SPECIAL_CHAR_COLOR  (0x0b)
#define ALPHA_COLOR_BASE    (0x0c)  /* 0x0c - 0x0f */

#define SPRITE_WIDTH        (16)
#define SPRITE_HEIGHT       (16)
#define GOAL_WIDTH          (16)
#define GOAL_HEIGHT         (64)

#define LEFT_GOAL_X         ((2 * 16) - 8)
#define RIGHT_GOAL_X        ((13 * 16) - 8)
#define GOAL_Y              (7 * 16)

#define TOP_BORDER          (16)
#define BOTTOM_BORDER       (255)
#define LEFT_BORDER         (0)
#define RIGHT_BORDER        (239)




/***************************************************************************
 *
 *  Palette generation
 *
 *  The palette PROM is connected to the RGB output this way.
 *
 *  bit 0 -- 220 ohm resistor  -- BLUE (probably an error on schematics)
 *        -- 470 ohm resistor  -- BLUE (probably an error on schematics)
 *        -- 220 ohm resistor  -- GREEN
 *        -- 470 ohm resistor  -- GREEN
 *        -- 1  kohm resistor  -- GREEN
 *        -- 220 ohm resistor  -- RED
 *        -- 470 ohm resistor  -- RED
 *  bit 7 -- 1  kohm resistor  -- RED
 *
 **************************************************************************/

PALETTE_INIT_MEMBER(carpolo_state, carpolo)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* thanks to Jarek Burczynski for analyzing the circuit */
	/* static const float MAX_VOLTAGE = 6.9620f; */
	static const float MIN_VOLTAGE = 1.7434f;
	static const float MAX_VOLTAGE = 5.5266f;

	static const float r_voltage[] =
	{
		1.7434f, 2.1693f, 2.5823f, 3.0585f, 3.4811f, 4.0707f, 4.7415f, 5.4251f
	};

	static const float g_voltage[] =
	{
		1.7434f, 2.1693f, 2.5823f, 3.0585f, 3.4811f, 4.0707f, 4.7415f, 5.4251f
		/* 4.7871f, 5.0613f, 5.3079f, 5.6114f, 5.7940f, 6.1608f, 6.5436f, 6.9620f */
	};

	static const float b_voltage[] =
	{
		1.9176f, 2.8757f, 3.9825f, 5.5266f
	};


	for (i = 0; i < palette.entries(); i++)
	{
		UINT8 pen, r, g, b;

		if (i < 0x18)
			/* sprites */
			pen = ((i - 0x00) & 0x01) ? CHARSET_COLOR_BASE + ((i - 0x00) >> 1) : 0;

		else if (i < 0x38)
			/* the bits in the goal gfx PROM are hooked up as follows (all active LO):
			   D3 - goal post
			   D2 - scoring area
			   D1 - net
			   D0 - n/c
			   I am only filling in the colors actually used. */
			switch (i - 0x18)
			{
			case (0x00 | (0x07 ^ 0x0f)): pen = LEFT_GOAL_COLOR; break;
			case (0x00 | (0x0d ^ 0x0f)): pen = NET_COLOR; break;
			case (0x00 | (0x09 ^ 0x0f)): pen = NET_COLOR; break;  /* score */
			case (0x10 | (0x07 ^ 0x0f)): pen = RIGHT_GOAL_COLOR; break;
			case (0x10 | (0x0d ^ 0x0f)): pen = NET_COLOR; break;
			case (0x10 | (0x09 ^ 0x0f)): pen = NET_COLOR; break; /* score */
			default: pen = 0; break;
			}

		else
			/* alpha layer */
			pen = ((i - 0x38) & 0x01) ? ALPHA_COLOR_BASE   + ((i - 0x38) >> 1) : 0;

		/* red component */
		r = ((r_voltage[(color_prom[pen] >> 5) & 0x07] - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 255.0f;

		/* green component */
		g = ((g_voltage[(color_prom[pen] >> 2) & 0x07] - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 255.0f;

		/* blue component */
		b = ((b_voltage[(color_prom[pen] >> 0) & 0x03] - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 255.0f;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*************************************
 *
 *  Video system start
 *
 *************************************/

void carpolo_state::video_start()
{
	m_sprite_sprite_collision_bitmap1 = auto_bitmap_ind16_alloc(machine(), SPRITE_WIDTH*2, SPRITE_HEIGHT*2);
	m_sprite_sprite_collision_bitmap2 = auto_bitmap_ind16_alloc(machine(), SPRITE_WIDTH*2, SPRITE_HEIGHT*2);

	m_sprite_goal_collision_bitmap1 = auto_bitmap_ind16_alloc(machine(), SPRITE_WIDTH+GOAL_WIDTH, SPRITE_HEIGHT+GOAL_HEIGHT);
	m_sprite_goal_collision_bitmap2 = auto_bitmap_ind16_alloc(machine(), SPRITE_WIDTH+GOAL_WIDTH, SPRITE_HEIGHT+GOAL_HEIGHT);

	m_sprite_border_collision_bitmap = auto_bitmap_ind16_alloc(machine(), SPRITE_WIDTH, SPRITE_HEIGHT);

	save_item(NAME(*m_sprite_sprite_collision_bitmap1));
	save_item(NAME(*m_sprite_sprite_collision_bitmap2));
	save_item(NAME(*m_sprite_goal_collision_bitmap1));
	save_item(NAME(*m_sprite_goal_collision_bitmap2));
	save_item(NAME(*m_sprite_border_collision_bitmap));
}


/*************************************
 *
 *  Core video refresh
 *
 *************************************/

void carpolo_state::draw_alpha_line(bitmap_ind16 &bitmap, const rectangle &cliprect,
							int alpha_line, int video_line)
{
	for (int x = 0; x < 32; x++)
	{
		UINT8 code, col;

		code = m_alpharam[alpha_line * 32 + x] >> 2;
		col  = m_alpharam[alpha_line * 32 + x] & 0x03;

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
				code,col,
				0,0,
				x*8,video_line*8,0);
	}
}


void carpolo_state::remap_sprite_code(int bank, int code, int *remapped_code, int *flipy)
{
	UINT8* PROM = memregion("user1")->base();

	code = (bank << 4) | code;
	*remapped_code = PROM[code] & 0x0f;
	*flipy = (PROM[code] & 0x10) >> 4;
}


void carpolo_state::draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect,
						UINT8 x, UINT8 y, int bank, int code, int col)
{
	int remapped_code, flipy;

	remap_sprite_code(bank, code, &remapped_code, &flipy);

	x = 240 - x;
	y = 240 - y;

	m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			remapped_code, col,
			0, flipy,
			x, y,0);

	/* draw with wrap around */
	m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			remapped_code, col,
			0, flipy,
			(INT16)x - 256, y,0);
}


UINT32 carpolo_state::screen_update_carpolo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw the playfield elements in the correct priority order */

	/* score area - position determined by bit 4 of the vertical timing PROM */
	bitmap.plot_box(0,0,RIGHT_BORDER+1,TOP_BORDER,BACKGROUND_PEN);

	/* field */
	bitmap.plot_box(0,TOP_BORDER,RIGHT_BORDER+1,BOTTOM_BORDER-TOP_BORDER+1,FIELD_PEN);

	/* car 1 */
	draw_sprite(bitmap, cliprect,
				m_spriteram[0x00], m_spriteram[0x01],
				0, m_spriteram[0x0c] & 0x0f, CAR1_COLOR);

	/* border - position determined by bit 4 and 7 of the vertical timing PROM */
	bitmap.plot_box(0,TOP_BORDER,   RIGHT_BORDER+1,1,LINE_PEN);
	bitmap.plot_box(0,BOTTOM_BORDER,RIGHT_BORDER+1,1,LINE_PEN);
	bitmap.plot_box(LEFT_BORDER,TOP_BORDER, 1,BOTTOM_BORDER-TOP_BORDER+1,LINE_PEN);
	bitmap.plot_box(RIGHT_BORDER,TOP_BORDER,1,BOTTOM_BORDER-TOP_BORDER+1,LINE_PEN);

	/* car 4 */
	draw_sprite(bitmap, cliprect,
				m_spriteram[0x06], m_spriteram[0x07],
				0, m_spriteram[0x0d] >> 4, CAR4_COLOR);

	/* car 3 */
	draw_sprite(bitmap, cliprect,
				m_spriteram[0x04], m_spriteram[0x05],
				0, m_spriteram[0x0d] & 0x0f, CAR3_COLOR);

	/* car 2 */
	draw_sprite(bitmap, cliprect,
				m_spriteram[0x02], m_spriteram[0x03],
				0, m_spriteram[0x0c] >> 4, CAR2_COLOR);

	/* ball */
	draw_sprite(bitmap, cliprect,
				m_spriteram[0x08], m_spriteram[0x09],
				1, m_spriteram[0x0e] & 0x0f, BALL_COLOR);

	/* left goal - position determined by bit 6 of the
	   horizontal and vertical timing PROMs */
	m_gfxdecode->gfx(1)->zoom_transpen(bitmap,cliprect,
				0,0,
				0,0,
				LEFT_GOAL_X,GOAL_Y,
				0x20000,0x20000,0);

	/* right goal */
	m_gfxdecode->gfx(1)->zoom_transpen(bitmap,cliprect,
				0,1,
				1,0,
				RIGHT_GOAL_X,GOAL_Y,
				0x20000,0x20000,0);

	/* special char - bit 0 of 0x0f enables it,
	                  bit 1 marked as WIDE, but never appears to be set */
	if (m_spriteram[0x0f] & 0x02)
		popmessage("WIDE!\n");

	if (m_spriteram[0x0f] & 0x01)
		draw_sprite(bitmap, cliprect,
					m_spriteram[0x0a], m_spriteram[0x0b],
					1, m_spriteram[0x0e] >> 4, SPECIAL_CHAR_COLOR);


	/* draw the alpha layer */

	/* there are only 8 lines of text repeated 4 times
	   and bit 3 of the vertical timing PROM controls in
	   which quadrant the line will actually appear */

	draw_alpha_line(bitmap, cliprect, 0, (0*4+0)*2  );
	draw_alpha_line(bitmap, cliprect, 1, (0*4+0)*2+1);
	draw_alpha_line(bitmap, cliprect, 2, (3*4+1)*2  );
	draw_alpha_line(bitmap, cliprect, 3, (3*4+1)*2+1);
	draw_alpha_line(bitmap, cliprect, 4, (1*4+2)*2  );
	draw_alpha_line(bitmap, cliprect, 5, (1*4+2)*2+1);
	draw_alpha_line(bitmap, cliprect, 6, (0*4+3)*2  );
	draw_alpha_line(bitmap, cliprect, 7, (0*4+3)*2+1);

	return 0;
}


/*************************************
 *
 *  End of frame callback
 *
 *************************************/

void carpolo_state::normalize_coordinates(int *x1, int *y1, int *x2, int *y2)
{
	if (*x1 < *x2)
	{
		*x2 = *x2 - *x1;
		*x1 = 0;
	}
	else
	{
		*x1 = *x1 - *x2;
		*x2 = 0;
	}

	if (*y1 < *y2)
	{
		*y2 = *y2 - *y1;
		*y1 = 0;
	}
	else
	{
		*y1 = *y1 - *y2;
		*y2 = 0;
	}
}


int carpolo_state::check_sprite_sprite_collision(int x1, int y1, int code1, int flipy1,
											int x2, int y2, int code2, int flipy2,
											int *col_x, int *col_y)
{
	int collided = 0;

	x1 = 240 - x1;
	y1 = 240 - y1;
	x2 = 240 - x2;
	y2 = 240 - y2;

	/* check if the two sprites are even within collision range */
	if ((abs(x1 - x2) < SPRITE_WIDTH) && (abs(y1 - y2) < SPRITE_HEIGHT))
	{
		int x,y;

		normalize_coordinates(&x1, &y1, &x2, &y2);

		m_sprite_sprite_collision_bitmap1->fill(0);
		m_sprite_sprite_collision_bitmap2->fill(0);

		m_gfxdecode->gfx(0)->opaque(*m_sprite_sprite_collision_bitmap1,m_sprite_sprite_collision_bitmap1->cliprect(),
				code1,0,
				0,flipy1,
				x1,y1);

		m_gfxdecode->gfx(0)->opaque(*m_sprite_sprite_collision_bitmap2,m_sprite_sprite_collision_bitmap2->cliprect(),
				code2,0,
				0,flipy2,
				x2,y2);

		for (x = x1; x < x1 + SPRITE_WIDTH; x++)
			for (y = y1; y < y1 + SPRITE_HEIGHT; y++)
				if ((m_sprite_sprite_collision_bitmap1->pix16(y, x) == 1) &&
					(m_sprite_sprite_collision_bitmap2->pix16(y, x) == 1))
				{
					*col_x = (x1 + x) & 0x0f;
					*col_y = (y1 + y) & 0x0f;

					collided = 1;

					break;
				}
	}

	return collided;
}


/* returns 1 for collision with goal post,
   2 for collision with scoring area */
int carpolo_state::check_sprite_left_goal_collision(int x1, int y1, int code1, int flipy1, int goalpost_only)
{
	int collided = 0;

	x1 = 240 - x1;
	y1 = 240 - y1;

	/* check if the sprite is even within the range of the goal */
	if (((y1 + 16) > GOAL_Y) && (y1 < (GOAL_Y + GOAL_HEIGHT)) &&
		((x1 + 16) > LEFT_GOAL_X) && (x1 < (LEFT_GOAL_X + GOAL_WIDTH)))
	{
		int x,y;
		int x2,y2;


		x2 = LEFT_GOAL_X;
		y2 = GOAL_Y;

		normalize_coordinates(&x1, &y1, &x2, &y2);

		m_sprite_goal_collision_bitmap1->fill(0);
		m_sprite_goal_collision_bitmap2->fill(0);

		m_gfxdecode->gfx(0)->opaque(*m_sprite_goal_collision_bitmap1,m_sprite_goal_collision_bitmap1->cliprect(),
				code1,0,
				0,flipy1,
				x1,y1);

		m_gfxdecode->gfx(1)->zoom_transpen(*m_sprite_goal_collision_bitmap2,m_sprite_goal_collision_bitmap2->cliprect(),
					0,0,
					0,0,
					x2,y2,
					0x20000,0x20000,0);

		for (x = x1; x < x1 + SPRITE_WIDTH; x++)
			for (y = y1; y < y1 + SPRITE_HEIGHT; y++)
				if (m_sprite_goal_collision_bitmap1->pix16(y, x) == 1)
				{
					pen_t pix = m_sprite_goal_collision_bitmap2->pix16(y, x);

					if (pix == LEFT_GOAL_PEN)
					{
						collided = 1;
						break;
					}

					if (!goalpost_only && (pix == LEFT_SCORE_PEN))
					{
						collided = 2;
						break;
					}
				}
	}

	return collided;
}


int carpolo_state::check_sprite_right_goal_collision(int x1, int y1, int code1, int flipy1, int goalpost_only)
{
	int collided = 0;

	x1 = 240 - x1;
	y1 = 240 - y1;

	/* check if the sprite is even within the range of the goal */
	if (((y1 + 16) > GOAL_Y) && (y1 < (GOAL_Y + GOAL_HEIGHT)) &&
		((x1 + 16) > RIGHT_GOAL_X) && (x1 < (RIGHT_GOAL_X + GOAL_WIDTH)))
	{
		int x,y;
		int x2,y2;

		x2 = RIGHT_GOAL_X;
		y2 = GOAL_Y;

		normalize_coordinates(&x1, &y1, &x2, &y2);

		m_sprite_goal_collision_bitmap1->fill(0);
		m_sprite_goal_collision_bitmap2->fill(0);

		m_gfxdecode->gfx(0)->opaque(*m_sprite_goal_collision_bitmap1,m_sprite_goal_collision_bitmap1->cliprect(),
				code1,0,
				0,flipy1,
				x1,y1);

		m_gfxdecode->gfx(1)->zoom_transpen(*m_sprite_goal_collision_bitmap2,m_sprite_goal_collision_bitmap2->cliprect(),
					0,1,
					1,0,
					x2,y2,
					0x20000,0x20000,0);

		for (x = x1; x < x1 + SPRITE_WIDTH; x++)
			for (y = y1; y < y1 + SPRITE_HEIGHT; y++)
				if (m_sprite_goal_collision_bitmap1->pix16(y, x) == 1)
				{
					pen_t pix = m_sprite_goal_collision_bitmap2->pix16(y, x);

					if (pix == RIGHT_GOAL_PEN)
					{
						collided = 1;
						break;
					}

					if (!goalpost_only && (pix == RIGHT_SCORE_PEN))
					{
						collided = 2;
						break;
					}
				}
	}

	return collided;
}


/* returns 1 for collision with vertical border,
   2 for collision with horizontal border */
int carpolo_state::check_sprite_border_collision(UINT8 x1, UINT8 y1, int code1, int flipy1)
{
	UINT8 x,y;
	int collided = 0;

	x1 = 240 - x1;
	y1 = 240 - y1;

	m_gfxdecode->gfx(0)->opaque(*m_sprite_border_collision_bitmap,m_sprite_border_collision_bitmap->cliprect(),
			code1,0,
			0,flipy1,
			0,0);

	for (x = 0; x < SPRITE_WIDTH; x++)
		for (y = 0; y < SPRITE_HEIGHT; y++)
			if (m_sprite_border_collision_bitmap->pix16(y, x) == 1)
			{
				if (((UINT8)(x1 + x) == LEFT_BORDER) ||
					((UINT8)(x1 + x) == RIGHT_BORDER))
				{
					collided = 1;
					break;
				}

				if (((UINT8)(y1 + y) == TOP_BORDER) ||
					((UINT8)(y1 + y) == BOTTOM_BORDER))
				{
					collided = 2;
					break;
				}
			}

	return collided;
}


void carpolo_state::screen_eof_carpolo(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		int col_x, col_y;
		int car1_x, car2_x, car3_x, car4_x, ball_x;
		int car1_y, car2_y, car3_y, car4_y, ball_y;
		int car1_code, car2_code, car3_code, car4_code, ball_code;
		int car1_flipy, car2_flipy, car3_flipy, car4_flipy, ball_flipy;


		/* check car-car collision first */

		car1_x = m_spriteram[0x00];
		car1_y = m_spriteram[0x01];
		remap_sprite_code(0, m_spriteram[0x0c] & 0x0f, &car1_code, &car1_flipy);

		car2_x = m_spriteram[0x02];
		car2_y = m_spriteram[0x03];
		remap_sprite_code(0, m_spriteram[0x0c] >> 4,   &car2_code, &car2_flipy);

		car3_x = m_spriteram[0x04];
		car3_y = m_spriteram[0x05];
		remap_sprite_code(0, m_spriteram[0x0d] & 0x0f, &car3_code, &car3_flipy);

		car4_x = m_spriteram[0x06];
		car4_y = m_spriteram[0x07];
		remap_sprite_code(0, m_spriteram[0x0d] >> 4,   &car4_code, &car4_flipy);

		ball_x = m_spriteram[0x08];
		ball_y = m_spriteram[0x09];
		remap_sprite_code(1, m_spriteram[0x0e] & 0x0f, &ball_code, &ball_flipy);


		/* cars 1 and 2 */
		if (check_sprite_sprite_collision(car1_x, car1_y, car1_code, car1_flipy,
											car2_x, car2_y, car2_code, car2_flipy,
											&col_x, &col_y))
			carpolo_generate_car_car_interrupt(0, 1);

		/* cars 1 and 3 */
		else if (check_sprite_sprite_collision(car1_x, car1_y, car1_code, car1_flipy,
												car3_x, car3_y, car3_code, car3_flipy,
												&col_x, &col_y))
			carpolo_generate_car_car_interrupt(0, 2);

		/* cars 1 and 4 */
		else if (check_sprite_sprite_collision(car1_x, car1_y, car1_code, car1_flipy,
												car4_x, car4_y, car4_code, car4_flipy,
												&col_x, &col_y))
			carpolo_generate_car_car_interrupt(0, 3);

		/* cars 2 and 3 */
		else if (check_sprite_sprite_collision(car2_x, car2_y, car2_code, car2_flipy,
												car3_x, car3_y, car3_code, car3_flipy,
												&col_x, &col_y))
			carpolo_generate_car_car_interrupt(1, 2);

		/* cars 2 and 4 */
		else if (check_sprite_sprite_collision(car2_x, car2_y, car2_code, car2_flipy,
												car4_x, car4_y, car4_code, car4_flipy,
												&col_x, &col_y))
			carpolo_generate_car_car_interrupt(1, 3);

		/* cars 3 and 4 */
		else if (check_sprite_sprite_collision(car3_x, car3_y, car3_code, car3_flipy,
												car4_x, car4_y, car4_code, car4_flipy,
												&col_x, &col_y))
			carpolo_generate_car_car_interrupt(2, 3);



		/* check car-ball collision */
		if (check_sprite_sprite_collision(car1_x, car1_y, car1_code, car1_flipy,
											ball_x, ball_y, ball_code, ball_flipy,
											&col_x, &col_y))
			carpolo_generate_car_ball_interrupt(0, col_x, col_y);

		else if (check_sprite_sprite_collision(car2_x, car2_y, car2_code, car2_flipy,
												ball_x, ball_y, ball_code, ball_flipy,
												&col_x, &col_y))
			carpolo_generate_car_ball_interrupt(1, col_x, col_y);

		else if (check_sprite_sprite_collision(car3_x, car3_y, car3_code, car3_flipy,
												ball_x, ball_y, ball_code, ball_flipy,
												&col_x, &col_y))
			carpolo_generate_car_ball_interrupt(2, col_x, col_y);

		else if (check_sprite_sprite_collision(car4_x, car4_y, car4_code, car4_flipy,
												ball_x, ball_y, ball_code, ball_flipy,
												&col_x, &col_y))
			carpolo_generate_car_ball_interrupt(3, col_x, col_y);


		/* check car-goal collision */
		if (check_sprite_left_goal_collision(car1_x, car1_y, car1_code, car1_flipy, 1))
			carpolo_generate_car_goal_interrupt(0, 0);

		else if (check_sprite_right_goal_collision(car1_x, car1_y, car1_code, car1_flipy, 1))
			carpolo_generate_car_goal_interrupt(0, 1);

		else if (check_sprite_left_goal_collision(car2_x, car2_y, car2_code, car2_flipy, 1))
			carpolo_generate_car_goal_interrupt(1, 0);

		else if (check_sprite_right_goal_collision(car2_x, car2_y, car2_code, car2_flipy, 1))
			carpolo_generate_car_goal_interrupt(1, 1);

		else if (check_sprite_left_goal_collision(car3_x, car3_y, car3_code, car3_flipy, 1))
			carpolo_generate_car_goal_interrupt(2, 0);

		else if (check_sprite_right_goal_collision(car3_x, car3_y, car3_code, car3_flipy, 1))
			carpolo_generate_car_goal_interrupt(2, 1);

		else if (check_sprite_left_goal_collision(car4_x, car4_y, car4_code, car4_flipy, 1))
			carpolo_generate_car_goal_interrupt(3, 0);

		else if (check_sprite_right_goal_collision(car4_x, car4_y, car4_code, car4_flipy, 1))
			carpolo_generate_car_goal_interrupt(3, 1);


		/* check ball collision with static screen elements */
		{
			int col;

			col = check_sprite_left_goal_collision(ball_x, ball_y, ball_code, ball_flipy, 0);

			if (col == 1)  carpolo_generate_ball_screen_interrupt(0x05);
			if (col == 2)  carpolo_generate_ball_screen_interrupt(0x03);


			col = check_sprite_right_goal_collision(ball_x, ball_y, ball_code, ball_flipy, 0);

			if (col == 1)  carpolo_generate_ball_screen_interrupt(0x05 | 0x08);
			if (col == 2)  carpolo_generate_ball_screen_interrupt(0x03 | 0x08);


			if (check_sprite_border_collision(ball_x, ball_y, ball_code, ball_flipy))
				carpolo_generate_ball_screen_interrupt(0x06);
		}


		/* check car-border collision */
		{
			int col;

			col = check_sprite_border_collision(car1_x, car1_y, car1_code, car1_flipy);

			if (col)
				carpolo_generate_car_border_interrupt(0, (col == 2));
			else
			{
				col = check_sprite_border_collision(car2_x, car2_y, car2_code, car2_flipy);

				if (col)
					carpolo_generate_car_border_interrupt(1, (col == 2));
				else
				{
					col = check_sprite_border_collision(car3_x, car3_y, car3_code, car3_flipy);

					if (col)
						carpolo_generate_car_border_interrupt(2, (col == 2));
					else
					{
						col = check_sprite_border_collision(car4_x, car4_y, car4_code, car4_flipy);

						if (col)
							carpolo_generate_car_border_interrupt(3, (col == 2));
					}
				}
			}
		}
	}
}
