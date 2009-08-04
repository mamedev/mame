/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "driver.h"
#include "video/resnet.h"
#include "redalert.h"


#define NUM_CHARMAP_PENS	0x200
#define NUM_BITMAP_PENS		8



/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT8 *redalert_bitmap_videoram;
UINT8 *redalert_bitmap_color;
UINT8 *redalert_charmap_videoram;

UINT8 *redalert_video_control;



/*************************************
 *
 *  Local variable
 *
 *************************************/

static UINT8 *redalert_bitmap_colorram;
static UINT8  redalert_control_xor;


/*************************************
 *
 *  Bitmap videoram write handler
 *
 *************************************/

WRITE8_HANDLER( redalert_bitmap_videoram_w )
{
	redalert_bitmap_videoram[offset     ] = data;
	redalert_bitmap_colorram[offset >> 3] = *redalert_bitmap_color & 0x07;
}



/*************************************
 *
 *  Color generation
 *
 *************************************/

static void get_pens(running_machine *machine, pen_t *pens)
{
	static const int resistances_bitmap[]     = { 100 };
	static const int resistances_charmap_rg[] = { 390, 220, 180 };
	static const int resistances_charmap_b[]  = { 220, 100 };
	static const int resistances_back_r[]     = { 1000 + 100 };
	static const int resistances_back_gb[]    = { 100 + 470 };

	offs_t offs;
	double scaler;
	double bitmap_weight[1];
	double charmap_rg_weights[3];
	double charmap_b_weights[2];
	double back_r_weight[1];
	double back_gb_weight[1];
	const UINT8 *prom = memory_region(machine, "proms");

	scaler = compute_resistor_weights(0, 0xff, -1,
									  1, resistances_bitmap,     bitmap_weight,      470, 0,
									  3, resistances_charmap_rg, charmap_rg_weights, 470, 0,
									  2, resistances_charmap_b,  charmap_b_weights,  470, 0);

			 compute_resistor_weights(0, 0xff, scaler,
									  1, resistances_back_r,     back_r_weight,      470, 0,
									  1, resistances_back_gb,    back_gb_weight,     470, 0,
									  0, 0, 0, 0, 0);

	/* the character layer colors come from the PROM */
	for (offs = 0; offs < NUM_CHARMAP_PENS; offs++)
	{
		UINT8 data = prom[offs];

		/* very strange mapping */
		UINT8 r0_bit = (data >> 2) & 0x01;
		UINT8 r1_bit = (data >> 6) & 0x01;
		UINT8 r2_bit = (data >> 4) & 0x01;
		UINT8 g0_bit = (data >> 1) & 0x01;
		UINT8 g1_bit = (data >> 3) & 0x01;
		UINT8 g2_bit = (data >> 5) & 0x01;
		UINT8 b0_bit = (data >> 0) & 0x01;
		UINT8 b1_bit = (data >> 7) & 0x01;

		UINT8 r = combine_3_weights(charmap_rg_weights, r0_bit, r1_bit, r2_bit);
		UINT8 g = combine_3_weights(charmap_rg_weights, g0_bit, g1_bit, g2_bit);
		UINT8 b = combine_2_weights(charmap_b_weights,  b0_bit, b1_bit);

		pens[offs] = MAKE_RGB(r, g, b);
	}

	/* the bitmap layer colors are directly mapped */
	for (offs = 0; offs < NUM_BITMAP_PENS; offs++)
	{
		UINT8 r_bit = (offs >> 2) & 0x01;
		UINT8 g_bit = (offs >> 1) & 0x01;
		UINT8 b_bit = (offs >> 0) & 0x01;

		UINT8 r = bitmap_weight[r_bit];
		UINT8 g = bitmap_weight[g_bit];
		UINT8 b = bitmap_weight[b_bit];

		pens[NUM_CHARMAP_PENS + offs] = MAKE_RGB(r, g, b);
	}

	/* background color */
	pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS] = MAKE_RGB(back_r_weight[0], back_gb_weight[0], back_gb_weight[0]);
}

/* this uses the same color hook-up between bitmap and chars. */
/* TODO: clean me up */
static void get_panther_pens(running_machine *machine, pen_t *pens)
{
	static const int resistances_bitmap[]     = { 100 };
	static const int resistances_charmap_rg[] = { 390, 220, 180 };
	static const int resistances_charmap_b[]  = { 220, 100 };
	static const int resistances_back_r[]     = { 1000 + 100 };
	static const int resistances_back_gb[]    = { 100 + 470 };

	offs_t offs;
	double scaler;
	double bitmap_weight[1];
	double charmap_rg_weights[3];
	double charmap_b_weights[2];
	double back_r_weight[1];
	double back_gb_weight[1];
	const UINT8 *prom = memory_region(machine, "proms");

	scaler = compute_resistor_weights(0, 0xff, -1,
									  1, resistances_bitmap,     bitmap_weight,      470, 0,
									  3, resistances_charmap_rg, charmap_rg_weights, 470, 0,
									  2, resistances_charmap_b,  charmap_b_weights,  470, 0);

			 compute_resistor_weights(0, 0xff, scaler,
									  1, resistances_back_r,     back_r_weight,      470, 0,
									  1, resistances_back_gb,    back_gb_weight,     470, 0,
									  0, 0, 0, 0, 0);

	/* the character layer colors come from the PROM */
	for (offs = 0; offs < NUM_CHARMAP_PENS; offs++)
	{
		UINT8 data = prom[offs];

		UINT8 r_bit = (~data >> 2) & 0x01;
		UINT8 g_bit = (~data >> 1) & 0x01;
		UINT8 b_bit = (~data >> 0) & 0x01;

		UINT8 r = bitmap_weight[r_bit];
		UINT8 g = bitmap_weight[g_bit];
		UINT8 b = bitmap_weight[b_bit];

		pens[offs] = MAKE_RGB(r, g, b);
	}

	/* the bitmap layer colors are directly mapped */
	for (offs = 0; offs < NUM_BITMAP_PENS; offs++)
	{
		UINT8 r_bit = (offs >> 2) & 0x01;
		UINT8 g_bit = (offs >> 1) & 0x01;
		UINT8 b_bit = (offs >> 0) & 0x01;

		UINT8 r = bitmap_weight[r_bit];
		UINT8 g = bitmap_weight[g_bit];
		UINT8 b = bitmap_weight[b_bit];

		pens[NUM_CHARMAP_PENS + offs] = MAKE_RGB(r, g, b);
	}

	/* background color */
	pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS] = MAKE_RGB(back_r_weight[0], back_gb_weight[0], back_gb_weight[0]);
}

/*************************************
 *
 *  Video hardware start
 *
 *************************************/

static VIDEO_START( redalert )
{
	redalert_bitmap_colorram = auto_alloc_array(machine, UINT8, 0x0400);

	state_save_register_global_pointer(machine, redalert_bitmap_colorram, 0x0400);

	redalert_control_xor = 0x00;
}

static VIDEO_START( ww3 )
{
	VIDEO_START_CALL( redalert );

	redalert_control_xor = 0x04;
}


/*************************************
 *
 *  Red Alert video update
 *
 *************************************/

static VIDEO_UPDATE( redalert )
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];
	offs_t offs;

	get_pens(screen->machine, pens);

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;
		UINT8 charmap_data_1;
		UINT8 charmap_data_2;

		UINT8 y = offs & 0xff;
		UINT8 x = (~offs >> 8) << 3;

		UINT8 bitmap_data = redalert_bitmap_videoram[offs];
		UINT8 bitmap_color = redalert_bitmap_colorram[offs >> 3];

		UINT8 charmap_code = redalert_charmap_videoram[0x0000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		if (charmap_code & 0x80)
		{
			charmap_data_1 = redalert_charmap_videoram[0x0400 | charmap_data_base];
			charmap_data_2 = redalert_charmap_videoram[0x0c00 | charmap_data_base];
		}
		else
		{
			charmap_data_1 = 0; /* effectively disables A0 of the color PROM */
			charmap_data_2 = redalert_charmap_videoram[0x0800 | charmap_data_base];
		}

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			int bitmap_bit = bitmap_data & 0x80;
			UINT8 color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if ((*redalert_video_control ^ redalert_control_xor) & 0x04)
				*BITMAP_ADDR32(bitmap, y, x) = pen;
			else
				*BITMAP_ADDR32(bitmap, y ^ 0xff, x ^ 0xff) = pen;

			/* next pixel */
			x = x + 1;

			bitmap_data    = bitmap_data    << 1;
			charmap_data_1 = charmap_data_1 << 1;
			charmap_data_2 = charmap_data_2 << 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Demoneye-X video update
 *
 *************************************/

static VIDEO_UPDATE( demoneye )
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];
	offs_t offs;

	get_pens(screen->machine, pens);

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;
		UINT8 charmap_data_1;
		UINT8 charmap_data_2;

		UINT8 y = offs & 0xff;
		UINT8 x = (~offs >> 8) << 3;

		UINT8 bitmap_data = redalert_bitmap_videoram[offs];
		UINT8 bitmap_color = redalert_bitmap_colorram[offs >> 3];

		UINT8 charmap_code = redalert_charmap_videoram[0x1000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		if (charmap_code & 0x80)
		{
			charmap_data_1 = redalert_charmap_videoram[0x0400 | charmap_data_base];
			charmap_data_2 = redalert_charmap_videoram[0x0c00 | charmap_data_base];
		}
		else
		{
			charmap_data_1 = redalert_charmap_videoram[0x0000 | charmap_data_base];
			charmap_data_2 = redalert_charmap_videoram[0x0800 | charmap_data_base];
		}

		/* this is the mapping of the 3rd char set */
		//charmap_data_1 = redalert_charmap_videoram[0x1400 | charmap_data_base];
		//charmap_data_2 = redalert_charmap_videoram[0x1c00 | charmap_data_base];

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			int bitmap_bit = bitmap_data & 0x80;
			UINT8 color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if (*redalert_video_control & 0x04)
				*BITMAP_ADDR32(bitmap, y ^ 0xff, x ^ 0xff) = pen;
			else
				*BITMAP_ADDR32(bitmap, y, x) = pen;

			/* next pixel */
			x = x + 1;

			bitmap_data    = bitmap_data    << 1;
			charmap_data_1 = charmap_data_1 << 1;
			charmap_data_2 = charmap_data_2 << 1;
		}
	}

	return 0;
}

/*************************************
 *
 *  Panther video update
 *
 *************************************/

static VIDEO_UPDATE( panther )
{
	pen_t pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS + 1];
	offs_t offs;

	get_panther_pens(screen->machine, pens);

	for (offs = 0; offs < 0x2000; offs++)
	{
		int i;
		UINT8 charmap_data_1;
		UINT8 charmap_data_2;

		UINT8 y = offs & 0xff;
		UINT8 x = (~offs >> 8) << 3;

		UINT8 bitmap_data = redalert_bitmap_videoram[offs];
		UINT8 bitmap_color = redalert_bitmap_colorram[offs >> 3];

		UINT8 charmap_code = redalert_charmap_videoram[0x0000 | (offs >> 3)];
		offs_t charmap_data_base = ((charmap_code & 0x7f) << 3) | (offs & 0x07);

		/* D7 of the char code selects the char set to use */
		if (charmap_code & 0x80)
		{
			charmap_data_1 = redalert_charmap_videoram[0x0400 | charmap_data_base];
			charmap_data_2 = redalert_charmap_videoram[0x0c00 | charmap_data_base];
		}
		else
		{
			charmap_data_1 = 0; /* effectively disables A0 of the color PROM */
			charmap_data_2 = redalert_charmap_videoram[0x0800 | charmap_data_base];
		}

		for (i = 0; i < 8; i++)
		{
			pen_t pen;

			int bitmap_bit = bitmap_data & 0x80;
			UINT8 color_prom_a0_a1 = ((charmap_data_2 & 0x80) >> 6) | ((charmap_data_1 & 0x80) >> 7);

			/* determine priority */
			if ((color_prom_a0_a1 == 0) || (bitmap_bit && ((charmap_code & 0xc0) == 0xc0)))
				pen = bitmap_bit ? pens[NUM_CHARMAP_PENS + bitmap_color] : pens[NUM_CHARMAP_PENS + NUM_BITMAP_PENS];
			else
				pen = pens[((charmap_code & 0xfe) << 1) | color_prom_a0_a1];

			if ((*redalert_video_control ^ redalert_control_xor) & 0x04)
				*BITMAP_ADDR32(bitmap, y, x) = pen;
			else
				*BITMAP_ADDR32(bitmap, y ^ 0xff, x ^ 0xff) = pen;

			/* next pixel */
			x = x + 1;

			bitmap_data    = bitmap_data    << 1;
			charmap_data_1 = charmap_data_1 << 1;
			charmap_data_2 = charmap_data_2 << 1;
		}
	}

	return 0;
}


/*************************************
 *
 *  Red Alert machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( redalert_video_common )

	MDRV_VIDEO_UPDATE(redalert)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

MACHINE_DRIVER_END

MACHINE_DRIVER_START( redalert_video )

	MDRV_VIDEO_START(redalert)
	MDRV_IMPORT_FROM( redalert_video_common )

MACHINE_DRIVER_END

MACHINE_DRIVER_START( ww3_video )

	MDRV_VIDEO_START( ww3 )
	MDRV_IMPORT_FROM( redalert_video_common )

MACHINE_DRIVER_END


/*************************************
 *
 *  Demoneye-X machine driver
 *
 *************************************/

MACHINE_DRIVER_START( demoneye_video )

	MDRV_VIDEO_START(redalert)
	MDRV_VIDEO_UPDATE(demoneye)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

MACHINE_DRIVER_END


MACHINE_DRIVER_START( panther_video )

	MDRV_VIDEO_START(ww3)
	MDRV_VIDEO_UPDATE(panther)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

MACHINE_DRIVER_END

