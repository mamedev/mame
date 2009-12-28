/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

    2009-04: Changed NES PPU to be a device (Nathan Woods)
    2009-07: Changed NES PPU to use a device memory map (Robert Bohms)

    Current known bugs

    General:

    * PPU timing is imprecise for updates that happen mid-scanline. Some games
      may demand more precision.

    NES-specific:

    * Micro Machines has minor rendering glitches (needs better timing).
    * Mach Rider has minor road rendering glitches (needs better timing).
    * Rad Racer demonstrates road glitches: it changes horizontal scrolling mid-line.

******************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "video/ppu2c0x.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* constant definitions */
#define VISIBLE_SCREEN_WIDTH	(32*8)	/* Visible screen width */
#define VISIBLE_SCREEN_HEIGHT	(30*8)	/* Visible screen height */
#define VIDEOMEM_SIZE			0x1000	/* videomem size */
#define VIDEOMEM_PAGE_SIZE		0x400	/* videomem page size */
#define SPRITERAM_SIZE			0x100	/* spriteram size */
#define SPRITERAM_MASK			(0x100-1)	/* spriteram size */
#define CHARGEN_NUM_CHARS		512		/* max number of characters handled by the chargen */

enum
{
	PPU2C0XINFO_INT_SCANLINES_PER_FRAME = DEVINFO_INT_DEVICE_SPECIFIC
};

/* default monochromatic colortable */
static const pen_t default_colortable_mono[] =
{
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
	0,1,2,3,
};

/* default colortable */
static const pen_t default_colortable[] =
{
	0,1,2,3,
	0,5,6,7,
	0,9,10,11,
	0,13,14,15,
	0,17,18,19,
	0,21,22,23,
	0,25,26,27,
	0,29,30,31,
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* our chip state */
typedef struct
{
	bitmap_t				*bitmap;				/* target bitmap */
	UINT8					*spriteram;				/* sprite ram */
	pen_t					*colortable;			/* color table modified at run time */
	pen_t					*colortable_mono;		/* monochromatic color table modified at run time */
	emu_timer				*scanline_timer;		/* scanline timer */
	emu_timer				*hblank_timer;			/* hblank period at end of each scanline */
	emu_timer				*nmi_timer;				/* NMI timer */
	int						scanline;				/* scanline count */
	ppu2c0x_scanline_cb		scanline_callback_proc;	/* optional scanline callback */
	ppu2c0x_hblank_cb		hblank_callback_proc;	/* optional hblank callback */
	ppu2c0x_vidaccess_cb	vidaccess_callback_proc;/* optional video access callback */
	int						regs[PPU_MAX_REG];		/* registers */
	int						refresh_data;			/* refresh-related */
	int						refresh_latch;			/* refresh-related */
	int						x_fine;					/* refresh-related */
	int						toggle;					/* used to latch hi-lo scroll */
	int						add;					/* vram increment amount */
	int						videomem_addr;			/* videomem address pointer */
	int						addr_latch;				/* videomem address latch */
	int						data_latch;				/* latched videomem data */
	int						buffered_data;
	int						tile_page;				/* current tile page */
	int						sprite_page;			/* current sprite page */
	int						back_color;				/* background color */
	UINT8					palette_ram[0x20];		/* shouldn't be in main memory! */
	int						scan_scale;				/* scan scale */
	int						scanlines_per_frame;	/* number of scanlines per frame */
	int						mirror_state;
	rgb_t					palette[64*4];			/* palette for this chip */
} ppu2c0x_chip;


/***************************************************************************
    PROTOTYPES
***************************************************************************/

static void update_scanline( const device_config *device );

static TIMER_CALLBACK( scanline_callback );
static TIMER_CALLBACK( hblank_callback );
static TIMER_CALLBACK( nmi_callback );

void (*ppu_latch)( const device_config *device, offs_t offset );

/* palette handlers */
static WRITE8_HANDLER( ppu2c0x_palette_write );
static READ8_HANDLER( ppu2c0x_palette_read );


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE ppu2c0x_chip *get_token( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == PPU_2C02) || (device->type == PPU_2C03B)
		 || (device->type == PPU_2C04) || (device->type == PPU_2C05)
		 || (device->type == PPU_2C07));
	return (ppu2c0x_chip *) device->token;
}


INLINE const ppu2c0x_interface *get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == PPU_2C02) || (device->type == PPU_2C03B)
		 || (device->type == PPU_2C04) || (device->type == PPU_2C05)
		 || (device->type == PPU_2C07));
	return (const ppu2c0x_interface *) device->static_config;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*************************************
 *
 *  PPU Palette Initialization
 *
 *************************************/

/* default address map */
// make this INTERNAL, default should just be enough to avoid compile errors, print error messages!
static ADDRESS_MAP_START( ppu2c0x, 0, 8 )
	AM_RANGE(0x3f00, 0x3fff) AM_READWRITE(ppu2c0x_palette_read, ppu2c0x_palette_write)
ADDRESS_MAP_END

void ppu2c0x_init_palette( running_machine *machine, int first_entry )
{

	/* This routine builds a palette using a transformation from */
	/* the YUV (Y, B-Y, R-Y) to the RGB color space */

	/* The NES has a 64 color palette                        */
	/* 16 colors, with 4 luminance levels for each color     */
	/* The 16 colors circle around the YUV color space,      */

	int colorIntensity, colorNum, colorEmphasis;

	double R, G, B;

	double tint = 0.22;	/* adjust to taste */
	double hue = 287.0;

	double Kr = 0.2989;
	double Kb = 0.1145;
	double Ku = 2.029;
	double Kv = 1.140;

	static const double brightness[3][4] =
	{
		{ 0.50, 0.75, 1.0, 1.0 },
		{ 0.29, 0.45, 0.73, 0.9 },
		{ 0, 0.24, 0.47, 0.77 }
	};

	/* Loop through the emphasis modes (8 total) */
	for (colorEmphasis = 0; colorEmphasis < 8; colorEmphasis ++)
	{
		/*
        double r_mod = 0.0;
        double g_mod = 0.0;
        double b_mod = 0.0;

        switch (colorEmphasis)
        {
            case 0: r_mod = 1.0; g_mod = 1.0; b_mod = 1.0; break;
            case 1: r_mod = 1.24; g_mod = .915; b_mod = .743; break;
            case 2: r_mod = .794; g_mod = 1.09; b_mod = .882; break;
            case 3: r_mod = .905; g_mod = 1.03; b_mod = 1.28; break;
            case 4: r_mod = .741; g_mod = .987; b_mod = 1.0; break;
            case 5: r_mod = 1.02; g_mod = .908; b_mod = .979; break;
            case 6: r_mod = 1.02; g_mod = .98; b_mod = .653; break;
            case 7: r_mod = .75; g_mod = .75; b_mod = .75; break;
        }
        */

		/* loop through the 4 intensities */
		for (colorIntensity = 0; colorIntensity < 4; colorIntensity++)
		{
			/* loop through the 16 colors */
			for (colorNum = 0; colorNum < 16; colorNum++)
			{
				double sat;
				double y, u, v;
				double rad;

				switch (colorNum)
				{
					case 0:
						sat = 0; rad = 0;
						y = brightness[0][colorIntensity];
						break;

					case 13:
						sat = 0; rad = 0;
						y = brightness[2][colorIntensity];
						break;

					case 14:
					case 15:
						sat = 0; rad = 0; y = 0;
						break;

					default:
						sat = tint;
						rad = M_PI * ((colorNum * 30 + hue) / 180.0);
						y = brightness[1][colorIntensity];
						break;
				}

				u = sat * cos(rad);
				v = sat * sin(rad);

				/* Transform to RGB */
				R = (y + Kv * v) * 255.0;
				G = (y - (Kb * Ku * u + Kr * Kv * v) / (1 - Kb - Kr)) * 255.0;
				B = (y + Ku * u) * 255.0;

				/* Clipping, in case of saturation */
				if (R < 0)
					R = 0;
				if (R > 255)
					R = 255;
				if (G < 0)
					G = 0;
				if (G > 255)
					G = 255;
				if (B < 0)
					B = 0;
				if (B > 255)
					B = 255;

				/* Round, and set the value */
				palette_set_color_rgb(machine, first_entry++, floor(R + .5), floor(G + .5), floor(B + .5));
			}
		}
	}

	/* color tables are modified at run-time, and are initialized on 'ppu2c0x_reset' */
}

/* the charlayout we use for the chargen */
static const gfx_layout ppu_charlayout =
{
	8, 8,	/* 8*8 characters */
	0,
	2,		/* 2 bits per pixel */
	{ 8*8, 0 },	/* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 consecutive bytes */
};

/*************************************
 *
 *  PPU Initialization and Disposal
 *
 *************************************/

static DEVICE_START( ppu2c0x )
{
	ppu2c0x_chip *chip = get_token(device);

	memset(chip, 0, sizeof(*chip));
	chip->scanlines_per_frame = (int) device_get_info_int(device, PPU2C0XINFO_INT_SCANLINES_PER_FRAME);

	/* initialize the scanline handling portion */
	chip->scanline_timer = timer_alloc(device->machine, scanline_callback, (void *) device);
	chip->hblank_timer = timer_alloc(device->machine, hblank_callback, (void *) device);
	chip->nmi_timer = timer_alloc(device->machine, nmi_callback, (void *) device);
	chip->scanline = 0;
	chip->scan_scale = 1;

	/* allocate a screen bitmap, videomem and spriteram, a dirtychar array and the monochromatic colortable */
	chip->bitmap = auto_bitmap_alloc(device->machine, VISIBLE_SCREEN_WIDTH, VISIBLE_SCREEN_HEIGHT, video_screen_get_format(device->machine->primary_screen));
	chip->spriteram = auto_alloc_array_clear(device->machine, UINT8, SPRITERAM_SIZE);
	chip->colortable = auto_alloc_array(device->machine, pen_t, ARRAY_LENGTH(default_colortable));
	chip->colortable_mono = auto_alloc_array(device->machine, pen_t, ARRAY_LENGTH(default_colortable_mono));
}

static TIMER_CALLBACK( hblank_callback )
{
	const device_config *device = (const device_config *)ptr;
	ppu2c0x_chip *this_ppu = get_token(device);
	int *ppu_regs = &this_ppu->regs[0];

	int blanked = (ppu_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
	int vblank = ((this_ppu->scanline >= PPU_VBLANK_FIRST_SCANLINE - 1) && (this_ppu->scanline < this_ppu->scanlines_per_frame - 1)) ? 1 : 0;

//  update_scanline(device);

	if (this_ppu->hblank_callback_proc)
		(*this_ppu->hblank_callback_proc) (device, this_ppu->scanline, vblank, blanked);

	timer_adjust_oneshot(this_ppu->hblank_timer, attotime_never, 0);
}

static TIMER_CALLBACK( nmi_callback )
{
	const device_config *device = (const device_config *)ptr;
	ppu2c0x_chip *this_ppu = get_token(device);
	const ppu2c0x_interface *intf = get_interface(device);
	int *ppu_regs = &this_ppu->regs[0];

	// Actually fire the VMI
	if (intf->nmi_handler != NULL)
		(*intf->nmi_handler) (device, ppu_regs);

	timer_adjust_oneshot(this_ppu->nmi_timer, attotime_never, 0);
}

static void draw_background( const device_config *device, UINT8 *line_priority )
{
	const ppu2c0x_interface *intf = get_interface(device);
	ppu2c0x_chip *this_ppu = get_token(device);

	/* cache some values locally */
	bitmap_t *bitmap = this_ppu->bitmap;
	const int *ppu_regs = &this_ppu->regs[0];
	const int scanline = this_ppu->scanline;
	const int refresh_data = this_ppu->refresh_data;
	const int tile_page = this_ppu->tile_page;

	int	start_x = (this_ppu->x_fine ^ 0x07) - 7;
	UINT16 back_pen;
	UINT16 *dest;

	UINT8 scroll_x_coarse, scroll_y_coarse, scroll_y_fine, color_mask;
	int x, tile_index, i;

	const pen_t *color_table;
	const pen_t *paldata;

	int tilecount = 0;

	/* setup the color mask and colortable to use */
	if (ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
	{
		color_mask = 0xf0;
		color_table = this_ppu->colortable_mono;
	}
	else
	{
		color_mask = 0xff;
		color_table = this_ppu->colortable;
	}

	/* cache the background pen */
	back_pen = (this_ppu->back_color & color_mask) + intf->color_base;

	/* determine where in the nametable to start drawing from */
	/* based on the current scanline and scroll regs */
	scroll_x_coarse = refresh_data & 0x1f;
	scroll_y_coarse = (refresh_data & 0x3e0) >> 5;
	scroll_y_fine = (refresh_data & 0x7000) >> 12;

	x = scroll_x_coarse;

	/* get the tile index */
	tile_index = ((refresh_data & 0xc00) | 0x2000) + scroll_y_coarse * 32;

	/* set up dest */
	dest = ((UINT16 *) bitmap->base) + (bitmap->rowpixels * scanline) + start_x;

	/* draw the 32 or 33 tiles that make up a line */
	while (tilecount < 34)
	{
		int color_byte;
		int color_bits;
		int pos;
		int index1;
		int page, page2, address;
		UINT16 pen;

		index1 = tile_index + x;

		// this is attribute table stuff! (actually read 2 in PPUspeak)!
		/* Figure out which byte in the color table to use */
		pos = ((index1 & 0x380) >> 4) | ((index1 & 0x1f) >> 2);
		page = (index1 & 0x0c00) >> 10;
		address = 0x3c0 + pos;
		color_byte = memory_read_byte(device->space[0], (((page * 0x400) + address) & 0xfff) + 0x2000);

		/* figure out which bits in the color table to use */
		color_bits = ((index1 & 0x40) >> 4) + (index1 & 0x02);

		// page2 is the output of the nametable read (this section is the FIRST read per tile!)
		address = index1 & 0x3ff;
		page2 = memory_read_byte(device->space[0], index1);

		// 27/12/2002
		if (ppu_latch)
		{
			(*ppu_latch)(device, (tile_page << 10) | (page2 << 4));
		}

		if (start_x < VISIBLE_SCREEN_WIDTH)
		{
			UINT8 plane1, plane2;
			paldata = &color_table[4 * (((color_byte >> color_bits) & 0x03))];

			// need to read 0x0000 or 0x1000 + 16*nametable data
			address = ((this_ppu->tile_page) ? 0x1000 : 0) + (page2 * 16);
			// plus something that accounts for y
			address += scroll_y_fine;

			plane1 = memory_read_byte(device->space[0], (address & 0x1fff));
			plane2 = memory_read_byte(device->space[0], (address + 8) & 0x1fff);

			/* render the pixel */
			for (i = 0; i < 8; i++)
			{
				UINT8 pix;
				pix = ((plane1 >> 7) & 1) | (((plane2 >> 7) & 1) << 1);
				plane1 = plane1 << 1;
				plane2 = plane2 << 1;
				if ((start_x + i) >= 0 && (start_x + i) < VISIBLE_SCREEN_WIDTH)
				{

					if (pix)
					{
						pen = paldata[pix];
						line_priority[start_x + i] |= 0x02;
					}
					else
					{
						pen = back_pen;
					}
					*dest = pen;
				}
				dest++;
			}

			start_x += 8;

			/* move to next tile over and toggle the horizontal name table if necessary */
			x++;
			if (x > 31)
			{
				x = 0;
				tile_index ^= 0x400;
			}
		}
		tilecount++;
	}

	/* if the left 8 pixels for the background are off, blank 'em */
	if (!(ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND_L8))
	{
		dest = ((UINT16 *) bitmap->base) + (bitmap->rowpixels * scanline);
		for (i = 0; i < 8; i++)
		{
			*(dest++) = back_pen;
			line_priority[i] ^= 0x02;
		}
	}
}

static void draw_sprites( const device_config *device, UINT8 *line_priority )
{
	ppu2c0x_chip *this_ppu = get_token(device);

	/* cache some values locally */
	bitmap_t *bitmap = this_ppu->bitmap;
	const int scanline = this_ppu->scanline;
	const int sprite_page = this_ppu->sprite_page;
	const UINT8 *sprite_ram = this_ppu->spriteram;
	pen_t *color_table = this_ppu->colortable;
	int *ppu_regs = &this_ppu->regs[0];

	int spriteXPos, spriteYPos, spriteIndex;
	int tile, index1, page;
	int pri;

	int flipx, flipy, color;
	int size;
	int spriteCount = 0;
	int sprite_line;

	int first_pixel;

	const pen_t *paldata;
	int pixel;

	/* determine if the sprites are 8x8 or 8x16 */
	size = (ppu_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE) ? 16 : 8;

	first_pixel = (ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES_L8)? 0: 8;

	for (spriteIndex = 0; spriteIndex < SPRITERAM_SIZE; spriteIndex += 4)
	{
		UINT8 plane1;
		UINT8 plane2;

		spriteYPos = sprite_ram[spriteIndex] + 1;
		spriteXPos = sprite_ram[spriteIndex + 3];

		// The sprite collision acts funny on the last pixel of a scanline.
		// The various scanline latches update while the last few pixels
		// are being drawn. Since we don't do cycle-by-cycle PPU emulation,
		// we fudge it a bit here so that sprite 0 collisions are detected
		// when, e.g., sprite x is 254, sprite y is 29 and we're rendering
		// at the end of scanline 28.
		// Battletoads needs this level of precision to be playable.
		if ((spriteIndex == 0) && (spriteXPos == 254))
		{
			spriteYPos --;
			/* set the "sprite 0 hit" flag if appropriate */
			if (line_priority[spriteXPos] & 0x02)
				ppu_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
		}

		/* if the sprite isn't visible, skip it */
		if ((spriteYPos + size <= scanline) || (spriteYPos > scanline))
			continue;

		tile = sprite_ram[spriteIndex + 1];
		color = (sprite_ram[spriteIndex + 2] & 0x03) + 4;
		pri = sprite_ram[spriteIndex + 2] & 0x20;
		flipx = sprite_ram[spriteIndex + 2] & 0x40;
		flipy = sprite_ram[spriteIndex + 2] & 0x80;

		if (size == 16)
		{
			/* if it's 8x16 and odd-numbered, draw the other half instead */
			if (tile & 0x01)
			{
				tile &= ~0x01;
				tile |= 0x100;
			}
			/* note that the sprite page value has no effect on 8x16 sprites */
			page = tile >> 6;
		}
		else
			page = (tile >> 6) | sprite_page;

		if (ppu_latch)
			(*ppu_latch)(device, (sprite_page << 10) | ((tile & 0xff) << 4));

		/* compute the character's line to draw */
		sprite_line = scanline - spriteYPos;

		if (flipy)
			sprite_line = (size - 1) - sprite_line;

		paldata = &color_table[4 * color];

		if (size == 16 && sprite_line > 7)
		{
			tile++;
			sprite_line -= 8;
		}

		index1 = tile * 16;
		if (size == 8)
			index1 += ((sprite_page == 0) ? 0 : 0x1000);

		plane1 = memory_read_byte(device->space[0], (index1 + sprite_line) & 0x1fff);
		plane2 = memory_read_byte(device->space[0], (index1 + sprite_line + 8) & 0x1fff);

		/* if there are more than 8 sprites on this line, set the flag */
		if (spriteCount == 8)
		{
			ppu_regs[PPU_STATUS] |= PPU_STATUS_8SPRITES;
//          logerror ("> 8 sprites, scanline: %d\n", scanline);

			/* the real NES only draws up to 8 sprites - the rest should be invisible */
			break;
		}

		spriteCount++;

		/* abort drawing if sprites aren't rendered */
		if (!(ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES))
			continue;

		if (pri)
		{
			/* draw the low-priority sprites */
			for (pixel = 0; pixel < 8; pixel++)
			{
				UINT8 pixelData;
				if (flipx)
				{
					pixelData = (plane1 & 1) + ((plane2 & 1) << 1);
					plane1 = plane1 >> 1;
					plane2 = plane2 >> 1;
				}
				else
				{
					pixelData = ((plane1 >> 7) & 1) | (((plane2 >> 7) & 1) << 1);
					plane1 = plane1 << 1;
					plane2 = plane2 << 1;
				}

				/* is this pixel non-transparent? */
				if (spriteXPos + pixel >= first_pixel)
				{
					if (pixelData)
					{
						/* has the background (or another sprite) already been drawn here? */
						if (!line_priority[spriteXPos + pixel])
						{
							/* no, draw */
							if ((spriteXPos + pixel) < VISIBLE_SCREEN_WIDTH)
								*BITMAP_ADDR16(bitmap, scanline, spriteXPos + pixel) = paldata[pixelData];
						}
						/* indicate that a sprite was drawn at this location, even if it's not seen */
						if ((spriteXPos + pixel) < VISIBLE_SCREEN_WIDTH)
							line_priority[spriteXPos + pixel] |= 0x01;
					}

					/* set the "sprite 0 hit" flag if appropriate */
					if (spriteIndex == 0 && (pixelData & 0x03) && ((spriteXPos + pixel) < 255) && (line_priority[spriteXPos + pixel] & 0x02))
						ppu_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
				}
			}
		}
		else
		{
			/* draw the high-priority sprites */
			for (pixel = 0; pixel < 8; pixel++)
			{
				UINT8 pixelData;
				if (flipx)
				{
					pixelData = (plane1 & 1) + ((plane2 & 1) << 1);
					plane1 = plane1 >> 1;
					plane2 = plane2 >> 1;
				}
				else
				{
					pixelData = ((plane1 >> 7) & 1) | (((plane2 >> 7) & 1) << 1);
					plane1 = plane1 << 1;
					plane2 = plane2 << 1;
				}

				/* is this pixel non-transparent? */
				if (spriteXPos + pixel >= first_pixel)
				{
					if (pixelData)
					{
						/* has another sprite been drawn here? */
						if (!(line_priority[spriteXPos + pixel] & 0x01))
						{
							/* no, draw */
							if ((spriteXPos + pixel) < VISIBLE_SCREEN_WIDTH)
							{
								*BITMAP_ADDR16(bitmap, scanline, spriteXPos + pixel) = paldata[pixelData];
								line_priority[spriteXPos + pixel] |= 0x01;
							}
						}
					}

					/* set the "sprite 0 hit" flag if appropriate */
					if (spriteIndex == 0 && (pixelData & 0x03) && ((spriteXPos + pixel) < 255) && (line_priority[spriteXPos + pixel] & 0x02))
						ppu_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
				}
			}
		}
	}
}

/*************************************
 *
 *  Scanline Rendering and Update
 *
 *************************************/

static void render_scanline( const device_config *device )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	const ppu2c0x_interface *intf = get_interface(device);
	UINT8 line_priority[VISIBLE_SCREEN_WIDTH];
	int *ppu_regs = &this_ppu->regs[0];

	/* lets see how long it takes */
	profiler_mark_start(PROFILER_USER1);

	/* clear the line priority for this scanline */
	memset(line_priority, 0, VISIBLE_SCREEN_WIDTH);

	/* see if we need to render the background */
	if (ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND)
		draw_background(device, line_priority);
	else
	{
		bitmap_t *bitmap = this_ppu->bitmap;
		const int scanline = this_ppu->scanline;
		UINT8 color_mask;
		UINT16 back_pen;
		int i;

		/* setup the color mask and colortable to use */
		if (ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
			color_mask = 0xf0;
		else
			color_mask = 0xff;

		/* cache the background pen */
		back_pen = (this_ppu->back_color & color_mask) + intf->color_base;

		// Fill this scanline with the background pen.
		for (i = 0; i < bitmap->width; i++)
			*BITMAP_ADDR16(bitmap, scanline, i) = back_pen;
	}

	/* if sprites are on, draw them, but we call always to process them */
	draw_sprites(device, line_priority);

	/* done updating, whew */
	profiler_mark_end();
}

static void update_scanline( const device_config *device )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	const ppu2c0x_interface *intf = get_interface(device);
	int scanline = this_ppu->scanline;
	int *ppu_regs = &this_ppu->regs[0];

	if (scanline <= PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		/* Render this scanline if appropriate */
		if (ppu_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES))
		{
			/* If background or sprites are enabled, copy the ppu address latch */
			/* Copy only the scroll x-coarse and the x-overflow bit */
			this_ppu->refresh_data &= ~0x041f;
			this_ppu->refresh_data |= (this_ppu->refresh_latch & 0x041f);

//          logerror("updating refresh_data: %04x (scanline: %d)\n", this_ppu->refresh_data, this_ppu->scanline);
			render_scanline(device);
		}
		else
		{
			bitmap_t *bitmap = this_ppu->bitmap;
			const int scanline = this_ppu->scanline;
			UINT8 color_mask;
			UINT16 back_pen;
			int i;

			/* setup the color mask and colortable to use */
			if (ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
				color_mask = 0xf0;
			else
				color_mask = 0xff;

			/* cache the background pen */
			if (this_ppu->videomem_addr >= 0x3f00)
			{
				// If the PPU's VRAM address happens to point into palette ram space while
				// both the sprites and background are disabled, the PPU paints the scanline
				// with the palette entry at the VRAM address instead of the usual background
				// pen. Micro Machines makes use of this feature.
				int penNum;

				if (this_ppu->videomem_addr & 0x03)
					penNum = this_ppu->palette_ram[this_ppu->videomem_addr & 0x1f];
				else
					penNum = this_ppu->palette_ram[0];

				back_pen = penNum + intf->color_base;
			}
			else
				back_pen = (this_ppu->back_color & color_mask) + intf->color_base;

			// Fill this scanline with the background pen.
			for (i = 0; i < bitmap->width; i++)
				*BITMAP_ADDR16(bitmap, scanline, i) = back_pen;
		}

		/* increment the fine y-scroll */
		this_ppu->refresh_data += 0x1000;

		/* if it's rolled, increment the coarse y-scroll */
		if (this_ppu->refresh_data & 0x8000)
		{
			UINT16 tmp;
			tmp = (this_ppu->refresh_data & 0x03e0) + 0x20;
			this_ppu->refresh_data &= 0x7c1f;

			/* handle bizarro scrolling rollover at the 30th (not 32nd) vertical tile */
			if (tmp == 0x03c0)
				this_ppu->refresh_data ^= 0x0800;
			else
				this_ppu->refresh_data |= (tmp & 0x03e0);

//          logerror("updating refresh_data: %04x\n", this_ppu->refresh_data);
		}
	}

}

static TIMER_CALLBACK( scanline_callback )
{
	const device_config *device = (const device_config *)ptr;
	ppu2c0x_chip *this_ppu = get_token(device);
	int *ppu_regs = &this_ppu->regs[0];
	int blanked = (ppu_regs[PPU_CONTROL1] & (PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES)) == 0;
	int vblank = ((this_ppu->scanline >= PPU_VBLANK_FIRST_SCANLINE - 1) && (this_ppu->scanline < this_ppu->scanlines_per_frame - 1)) ? 1 : 0;
	int next_scanline;

	/* if a callback is available, call it */
	if (this_ppu->scanline_callback_proc != NULL)
		(*this_ppu->scanline_callback_proc)(device, this_ppu->scanline, vblank, blanked);

	/* update the scanline that just went by */
	update_scanline(device);

	/* increment our scanline count */
	this_ppu->scanline++;

//  logerror("starting scanline %d (MAME %d, beam %d)\n", this_ppu->scanline, video_screen_get_vpos(device->machine->primary_screen), video_screen_get_hpos(device->machine->primary_screen));

	/* Note: this is called at the _end_ of each scanline */
	if (this_ppu->scanline == PPU_VBLANK_FIRST_SCANLINE)
	{
		// logerror("vblank starting\n");
		/* We just entered VBLANK */
		ppu_regs[PPU_STATUS] |= PPU_STATUS_VBLANK;

		/* If NMI's are set to be triggered, go for it */
		if (ppu_regs[PPU_CONTROL0] & PPU_CONTROL0_NMI)
		{
			// We need an ever-so-slight delay between entering vblank and firing an NMI - enough so that
			// a game can read the high bit of $2002 before the NMI is called (potentially resetting the bit
			// via a read from $2002 in the NMI handler).
			// B-Wings is an example game that needs this.
			timer_adjust_oneshot(this_ppu->nmi_timer, cputag_clocks_to_attotime(device->machine, "maincpu", 4), 0);
		}
	}

	if (this_ppu->scanline == this_ppu->scanlines_per_frame - 1)
	{
		// logerror("vblank ending\n");
		/* clear the vblank & sprite hit flag */
		ppu_regs[PPU_STATUS] &= ~(PPU_STATUS_VBLANK | PPU_STATUS_SPRITE0_HIT | PPU_STATUS_8SPRITES);
	}

	/* see if we rolled */
	else if (this_ppu->scanline == this_ppu->scanlines_per_frame)
	{
		/* if background or sprites are enabled, copy the ppu address latch */
		if (!blanked)
			this_ppu->refresh_data = this_ppu->refresh_latch;

		/* reset the scanline count */
		this_ppu->scanline = 0;
//      logerror("sprite 0 x: %d y: %d num: %d\n", this_ppu->spriteram[3], this_ppu->spriteram[0] + 1, this_ppu->spriteram[1]);
	}

	next_scanline = this_ppu->scanline + 1;
	if (next_scanline == this_ppu->scanlines_per_frame)
		next_scanline = 0;

	// Call us back when the hblank starts for this scanline
	timer_adjust_oneshot(this_ppu->hblank_timer, cputag_clocks_to_attotime(device->machine, "maincpu", 86.67), 0); // ??? FIXME - hardcoding NTSC, need better calculation

	// trigger again at the start of the next scanline
	timer_adjust_oneshot(this_ppu->scanline_timer, video_screen_get_time_until_pos(device->machine->primary_screen, next_scanline * this_ppu->scan_scale, 0), 0);
}

/*************************************
 *
 *  PPU Reset
 *
 *************************************/

static DEVICE_RESET( ppu2c0x )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	const ppu2c0x_interface *intf = get_interface(device);
	int scan_scale = 1;
	int i;

	/* reset the scanline count */
	this_ppu->scanline = 0;

	/* set the scan scale (this is for dual monitor vertical setups) */
	this_ppu->scan_scale = scan_scale;

	timer_adjust_oneshot(this_ppu->nmi_timer, attotime_never, 0);

	// Call us back when the hblank starts for this scanline
	timer_adjust_oneshot(this_ppu->hblank_timer, cputag_clocks_to_attotime(device->machine, "maincpu", 86.67), 0); // ??? FIXME - hardcoding NTSC, need better calculation

	// Call us back at the start of the next scanline
	timer_adjust_oneshot(this_ppu->scanline_timer, video_screen_get_time_until_pos(device->machine->primary_screen, 1, 0), 0);

	/* reset the callbacks */
	this_ppu->scanline_callback_proc = 0;
	this_ppu->vidaccess_callback_proc = 0;

	for (i = 0; i < PPU_MAX_REG; i++)
		this_ppu->regs[i] = 0;

	/* initialize the rest of the members */
	this_ppu->refresh_data = 0;
	this_ppu->refresh_latch = 0;
	this_ppu->x_fine = 0;
	this_ppu->toggle = 0;
	this_ppu->add = 1;
	this_ppu->videomem_addr = 0;
	this_ppu->addr_latch = 0;
	this_ppu->data_latch = 0;
	this_ppu->tile_page = 0;
	this_ppu->sprite_page = 0;
	this_ppu->back_color = 0;

	/* initialize the color tables */
	{
		int color_base = intf->color_base;

		for (i = 0; i < ARRAY_LENGTH(default_colortable_mono); i++)
		{
			/* monochromatic table */
			this_ppu->colortable_mono[i] = default_colortable_mono[i] + color_base;

			/* color table */
			this_ppu->colortable[i] = default_colortable[i] + color_base;
		}
	}
}

/*************************************
*
*   PPU Memory functions
*
*************************************/

static WRITE8_HANDLER( ppu2c0x_palette_write )
{
	ppu2c0x_chip *this_ppu = get_token(space->cpu);
	const ppu2c0x_interface *intf = get_interface(space->cpu);
	int color_base = intf->color_base;

	int colorEmphasis = (this_ppu->regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) * 2;
	// it's a palette
	// transparent pens are mirrored!
	if (offset & 0x3)
	{
		this_ppu->palette_ram[offset & 0x1f] = data;
		this_ppu->colortable[offset & 0x1f] = color_base + data + colorEmphasis;
		this_ppu->colortable_mono[offset & 0x1f] = color_base + (data & 0xf0) + colorEmphasis;
	}
	else
	{
		int i;
		if (0 == (offset & 0xf))
		{
			this_ppu->back_color = data;
			for (i = 0; i < 32; i += 4)
			{
				this_ppu->colortable[i] = color_base + data + colorEmphasis;
				this_ppu->colortable_mono[i] = color_base + (data & 0xf0) + colorEmphasis;
			}
		}
		this_ppu->palette_ram[offset & 0xf] = this_ppu->palette_ram[(offset & 0xf) + 0x10] = data;
	}
}

static READ8_HANDLER( ppu2c0x_palette_read )
{
	ppu2c0x_chip *this_ppu = get_token(space->cpu);
	{
		if (this_ppu->regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
			return (this_ppu->palette_ram[offset & 0x1f] & 0x30);

		else
			return (this_ppu->palette_ram[offset & 0x1f] & 0x3f);
	}
}

/*************************************
 *
 *  PPU Registers Read
 *
 *************************************/

READ8_DEVICE_HANDLER( ppu2c0x_r )
{
	ppu2c0x_chip *this_ppu = get_token(device);

	if (offset >= PPU_MAX_REG)
	{
		logerror("PPU %s(r): Attempting to read past the chip\n", device->tag);
		offset &= PPU_MAX_REG - 1;
	}

	// see which register to read
	switch (offset & 7)
	{
		case PPU_STATUS:
			// The top 3 bits of the status register are the only ones that report data. The
			// remainder contain whatever was last in the PPU data latch.
			this_ppu->data_latch = this_ppu->regs[PPU_STATUS] | (this_ppu->data_latch & 0x1f);

			// Reset hi/lo scroll toggle
			this_ppu->toggle = 0;

			// If the vblank bit is set, clear all status bits but the 2 sprite flags
			if (this_ppu->data_latch & PPU_STATUS_VBLANK)
				this_ppu->regs[PPU_STATUS] &= 0x60;
			break;

		case PPU_SPRITE_DATA:
			this_ppu->data_latch = this_ppu->spriteram[this_ppu->regs[PPU_SPRITE_ADDRESS]];
			break;

		case PPU_DATA:
			if (ppu_latch)
				(*ppu_latch)(device, this_ppu->videomem_addr & 0x3fff);

			if (this_ppu->videomem_addr >= 0x3f00)
			{
				this_ppu->data_latch = memory_read_byte(device->space[0], this_ppu->videomem_addr);
				// buffer the mirrored NT data
				this_ppu->buffered_data = memory_read_byte(device->space[0], this_ppu->videomem_addr & 0x2fff);
			}
			else
			{
				this_ppu->data_latch = this_ppu->buffered_data;
				this_ppu->buffered_data = memory_read_byte(device->space[0], this_ppu->videomem_addr);
			}

			this_ppu->videomem_addr += this_ppu->add;
			break;

		default:
			break;
	}

	return this_ppu->data_latch;
}


/*************************************
 *
 *  PPU Registers Write
 *
 *************************************/

WRITE8_DEVICE_HANDLER( ppu2c0x_w )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	const ppu2c0x_interface *intf = get_interface(device);
	int color_base;

	color_base = intf->color_base;

	if (offset >= PPU_MAX_REG)
	{
		logerror("PPU: Attempting to write past the chip\n");
		offset &= PPU_MAX_REG - 1;
	}

#ifdef MAME_DEBUG
	if (this_ppu->scanline <= PPU_BOTTOM_VISIBLE_SCANLINE)
	{
		const device_config *screen = device->machine->primary_screen;
		logerror("PPU register %d write %02x during non-vblank scanline %d (MAME %d, beam pos: %d)\n", offset, data, this_ppu->scanline, video_screen_get_vpos(screen), video_screen_get_hpos(screen));
	}
#endif

	switch (offset & 7)
	{
		case PPU_CONTROL0:
			this_ppu->regs[PPU_CONTROL0] = data;

			/* update the name table number on our refresh latches */
			this_ppu->refresh_latch &= 0x73ff;
			this_ppu->refresh_latch |= (data & 3) << 10;

			/* the char ram bank points either 0x0000 or 0x1000 (page 0 or page 4) */
			this_ppu->tile_page = (data & PPU_CONTROL0_CHR_SELECT) >> 2;
			this_ppu->sprite_page = (data & PPU_CONTROL0_SPR_SELECT) >> 1;

			this_ppu->add = (data & PPU_CONTROL0_INC) ? 32 : 1;
//          logerror("control0 write: %02x (scanline: %d)\n", data, this_ppu->scanline);
			break;

		case PPU_CONTROL1:
			/* if color intensity has changed, change all the color tables to reflect them */
			if ((data & PPU_CONTROL1_COLOR_EMPHASIS) != (this_ppu->regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS))
			{
				int i;
				for (i = 0; i <= 0x1f; i ++)
				{
					UINT8 oldColor = this_ppu->palette_ram[i];

					this_ppu->colortable[i] = color_base + oldColor + (data & PPU_CONTROL1_COLOR_EMPHASIS) * 2;
				}
			}

//          logerror("control1 write: %02x (scanline: %d)\n", data, this_ppu->scanline);
			this_ppu->regs[PPU_CONTROL1] = data;
			break;

		case PPU_SPRITE_ADDRESS:
			this_ppu->regs[PPU_SPRITE_ADDRESS] = data;
			break;

		case PPU_SPRITE_DATA:
			// If the PPU is currently rendering the screen, 0xff is written instead of the desired data.
			if (this_ppu->scanline <= PPU_BOTTOM_VISIBLE_SCANLINE)
				data = 0xff;
			this_ppu->spriteram[this_ppu->regs[PPU_SPRITE_ADDRESS]] = data;
			this_ppu->regs[PPU_SPRITE_ADDRESS] = (this_ppu->regs[PPU_SPRITE_ADDRESS] + 1) & 0xff;
			break;

		case PPU_SCROLL:
			if (this_ppu->toggle)
			{
				/* second write */
				this_ppu->refresh_latch &= 0x0c1f;
				this_ppu->refresh_latch |= (data & 0xf8) << 2;
				this_ppu->refresh_latch |= (data & 0x07) << 12;
//              logerror("scroll write 2: %d, %04x (scanline: %d)\n", data, this_ppu->refresh_latch, this_ppu->scanline);
			}
			else
			{
				/* first write */
				this_ppu->refresh_latch &= 0x7fe0;
				this_ppu->refresh_latch |= (data & 0xf8) >> 3;

				this_ppu->x_fine = data & 7;
//              logerror("scroll write 1: %d, %04x (scanline: %d)\n", data, this_ppu->refresh_latch, this_ppu->scanline);
			}

			this_ppu->toggle ^= 1;
			break;

		case PPU_ADDRESS:
			if (this_ppu->toggle)
			{
				/* second write */
				this_ppu->refresh_latch &= 0x7f00;
				this_ppu->refresh_latch |= data;
				this_ppu->refresh_data = this_ppu->refresh_latch;

				this_ppu->videomem_addr = this_ppu->refresh_latch;
//              logerror("vram addr write 2: %02x, %04x (scanline: %d)\n", data, this_ppu->refresh_latch, this_ppu->scanline);
			}
			else
			{
				/* first write */
				this_ppu->refresh_latch &= 0x00ff;
				this_ppu->refresh_latch |= (data & 0x3f) << 8;
//              logerror("vram addr write 1: %02x, %04x (scanline: %d)\n", data, this_ppu->refresh_latch, this_ppu->scanline);
			}

			this_ppu->toggle ^= 1;
			break;

		case PPU_DATA:
			{
				int tempAddr = this_ppu->videomem_addr & 0x3fff;

				if (ppu_latch)
					(*ppu_latch)(device, tempAddr);

				/* if there's a callback, call it now */
				if (this_ppu->vidaccess_callback_proc)
					data = (*this_ppu->vidaccess_callback_proc)(device, tempAddr, data);

				/* see if it's on the chargen portion */
				if (tempAddr < 0x2000)
				{
					/* store the data */
					memory_write_byte(device->space[0], tempAddr, data);
				}

				else
				{
					memory_write_byte(device->space[0], tempAddr, data);
				}
				/* increment the address */
				this_ppu->videomem_addr += this_ppu->add;
			}
			break;

		default:
			/* ignore other registers writes */
			break;
	}
}

/*************************************
 *
 *  Sprite DMA
 *
 *************************************/

void ppu2c0x_spriteram_dma( const address_space *space, const device_config *device, const UINT8 page )
{
	int i;
	int address = page << 8;

	for (i = 0; i < SPRITERAM_SIZE; i++)
	{
		UINT8 spriteData = memory_read_byte(space, address + i);
		memory_write_byte(space, 0x2004, spriteData);
	}

	// should last 513 CPU cycles.
	cpu_adjust_icount(space->cpu, -513);
}

/*************************************
 *
 *  PPU Rendering
 *
 *************************************/

void ppu2c0x_render( const device_config *device, bitmap_t *bitmap, int flipx, int flipy, int sx, int sy )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	copybitmap(bitmap, this_ppu->bitmap, flipx, flipy, sx, sy, 0);
}

/*************************************
 *
 *  Utility functions
 *
 *************************************/

int ppu2c0x_get_pixel( const device_config *device, int x, int y )
{
	ppu2c0x_chip *this_ppu = get_token(device);

	if (x >= VISIBLE_SCREEN_WIDTH)
		x = VISIBLE_SCREEN_WIDTH - 1;

	if (y >= VISIBLE_SCREEN_HEIGHT)
		y = VISIBLE_SCREEN_HEIGHT - 1;

	return *BITMAP_ADDR16(this_ppu->bitmap, y, x);
}

int ppu2c0x_get_colorbase( const device_config *device )
{
	const ppu2c0x_interface *intf = get_interface(device);
	return intf->color_base;
}

int ppu2c0x_get_current_scanline( const device_config *device )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	return this_ppu->scanline;
}

void ppu2c0x_set_scanline_callback( const device_config *device, ppu2c0x_scanline_cb cb )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	this_ppu->scanline_callback_proc = cb;
}

void ppu2c0x_set_hblank_callback( const device_config *device, ppu2c0x_hblank_cb cb )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	this_ppu->hblank_callback_proc = cb;
}

void ppu2c0x_set_vidaccess_callback( const device_config *device, ppu2c0x_vidaccess_cb cb )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	this_ppu->vidaccess_callback_proc = cb;
}

void ppu2c0x_set_scanlines_per_frame( const device_config *device, int scanlines )
{
	ppu2c0x_chip *this_ppu = get_token(device);
	this_ppu->scanlines_per_frame = scanlines;
}


/***************************************************************************
    GET INFO FUNCTIONS
***************************************************************************/

DEVICE_GET_INFO(ppu2c02)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ppu2c0x_chip);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;			break;
		case PPU2C0XINFO_INT_SCANLINES_PER_FRAME:		info->i = PPU_NTSC_SCANLINES_PER_FRAME;		break;
		case DEVINFO_INT_DATABUS_WIDTH_0:				info->i = 8;								break;
		case DEVINFO_INT_ADDRBUS_WIDTH_0:				info->i = 14;								break;
		case DEVINFO_INT_ADDRBUS_SHIFT_0:				info->i = 0;								break;


		/* --- the following bits of info are returned as pointers to data --- */
		case DEVINFO_PTR_DEFAULT_MEMORY_MAP_0:		info->default_map8 = ADDRESS_MAP_NAME(ppu2c0x);break;


		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(ppu2c0x);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(ppu2c0x);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "2C02 PPU");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "2C0X PPU");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						/* Nothing */								break;
	}
}

DEVICE_GET_INFO(ppu2c03b)
{
	switch (state)
	{
		case DEVINFO_STR_NAME:							strcpy(info->s, "2C02B PPU");				break;
		case PPU2C0XINFO_INT_SCANLINES_PER_FRAME:		info->i = PPU_NTSC_SCANLINES_PER_FRAME;		break;
		default:										DEVICE_GET_INFO_CALL(ppu2c02);				break;
	}
}

DEVICE_GET_INFO(ppu2c04)
{
	switch (state)
	{
		case DEVINFO_STR_NAME:							strcpy(info->s, "2C04 PPU");				break;
		case PPU2C0XINFO_INT_SCANLINES_PER_FRAME:		info->i = PPU_NTSC_SCANLINES_PER_FRAME;		break;
		default:										DEVICE_GET_INFO_CALL(ppu2c02);				break;
	}
}

DEVICE_GET_INFO(ppu2c05)
{
	switch (state)
	{
		case DEVINFO_STR_NAME:							strcpy(info->s, "2C05 PPU");				break;
		case PPU2C0XINFO_INT_SCANLINES_PER_FRAME:		info->i = PPU_NTSC_SCANLINES_PER_FRAME;		break;
		default:										DEVICE_GET_INFO_CALL(ppu2c02);				break;
	}
}

DEVICE_GET_INFO(ppu2c07)
{
	switch (state)
	{
		case DEVINFO_STR_NAME:							strcpy(info->s, "2C07 PPU");				break;
		case PPU2C0XINFO_INT_SCANLINES_PER_FRAME:		info->i = PPU_PAL_SCANLINES_PER_FRAME;		break;
		default:										DEVICE_GET_INFO_CALL(ppu2c02);				break;
	}
}
