/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/

/******************************************************************************

Current known bugs

General:

* PPU timing is imprecise for updates that happen mid-scanline. Some games
 may demand more precision.

NES-specific:

* Micro Machines has minor rendering glitches (needs better timing).

* Mach Rider has minor road rendering glitches (needs better timing).

******************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "deprecat.h"
#include "video/ppu2c0x.h"

/* constant definitions */
#define VISIBLE_SCREEN_WIDTH	(32*8)	/* Visible screen width */
#define VISIBLE_SCREEN_HEIGHT	(30*8)	/* Visible screen height */
#define VIDEORAM_SIZE			0x4000	/* videoram size */
#define SPRITERAM_SIZE			0x100	/* spriteram size */
#define SPRITERAM_MASK			(0x100-1)	/* spriteram size */
#define CHARGEN_NUM_CHARS		512		/* max number of characters handled by the chargen */

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

/* our chip state */
typedef struct {
	running_machine 		*machine;				/* execution context */
	mame_bitmap				*bitmap;				/* target bitmap */
	UINT8					*videoram;				/* video ram */
	UINT8					*spriteram;				/* sprite ram */
	pen_t					*colortable;			/* color table modified at run time */
	pen_t					*colortable_mono;		/* monochromatic color table modified at run time */
	UINT8					*dirtychar;				/* an array flagging dirty characters */
	int						chars_are_dirty;		/* master flag to check if theres any dirty character */
	emu_timer				*scanline_timer;		/* scanline timer */
	emu_timer				*hblank_timer;			/* hblank period at end of each scanline */
	emu_timer				*nmi_timer;				/* NMI timer */
	int						scanline;				/* scanline count */
	ppu2c0x_scanline_cb		scanline_callback_proc;	/* optional scanline callback */
	ppu2c0x_hblank_cb		hblank_callback_proc;	/* optional hblank callback */
	ppu2c0x_vidaccess_cb	vidaccess_callback_proc;/* optional video access callback */
	int						has_videorom;			/* whether we access a video rom or not */
	int						videorom_banks;			/* number of banks in the videorom (if available) */
	int						regs[PPU_MAX_REG];		/* registers */
	int						refresh_data;			/* refresh-related */
	int						refresh_latch;			/* refresh-related */
	int						x_fine;					/* refresh-related */
	int						toggle;					/* used to latch hi-lo scroll */
	int						add;					/* vram increment amount */
	int						videoram_addr;			/* videoram address pointer */
	int						addr_latch;				/* videoram address latch */
	int						data_latch;				/* latched videoram data */
	int						buffered_data;
	int						tile_page;				/* current tile page */
	int						sprite_page;			/* current sprite page */
	int						back_color;				/* background color */
	UINT8					*ppu_page[4];			/* ppu pages */
	int						nes_vram[8];			/* keep track of 8 .5k vram pages to speed things up */
	int						scan_scale;				/* scan scale */
	int						scanlines_per_frame;	/* number of scanlines per frame */
	int						mirror_state;
	rgb_t					palette[64*4];			/* palette for this chip */
} ppu2c0x_chip;

/* our local copy of the interface */
static ppu2c0x_interface *intf;

/* chips state - allocated at init time */
static ppu2c0x_chip *chips = 0;

static void update_scanline(int num );

static TIMER_CALLBACK( scanline_callback );
static TIMER_CALLBACK( hblank_callback );
static TIMER_CALLBACK( nmi_callback );

void (*ppu_latch)( offs_t offset );


/*************************************
 *
 *  PPU Palette Initialization
 *
 *************************************/
void ppu2c0x_init_palette(running_machine *machine, int first_entry )
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

				u = sat * cos( rad );
				v = sat * sin( rad );

				/* Transform to RGB */
				R = ( y + Kv * v ) * 255.0;
				G = ( y - ( Kb * Ku * u + Kr * Kv * v) / (1 - Kb - Kr) ) * 255.0;
				B = ( y + Ku * u ) * 255.0;

				/* Clipping, in case of saturation */
				if ( R < 0 )
					R = 0;
				if ( R > 255 )
					R = 255;
				if ( G < 0 )
					G = 0;
				if ( G > 255 )
					G = 255;
				if ( B < 0 )
					B = 0;
				if ( B > 255 )
					B = 255;

				/* Round, and set the value */
				palette_set_color_rgb(machine, first_entry++, floor(R+.5), floor(G+.5), floor(B+.5));
			}
		}
	}

	/* color tables are modified at run-time, and are initialized on 'ppu2c0x_reset' */
}

/* the charlayout we use for the chargen */
static const gfx_layout ppu_charlayout =
{
	8,8,	/* 8*8 characters */
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
void ppu2c0x_init(running_machine *machine, const ppu2c0x_interface *interface )
{
	int i;
	UINT32 total;

	/* keep a local copy of the interface */
	intf = auto_malloc(sizeof(*interface));
	memcpy(intf, interface, sizeof(*interface));

	/* safety check */
	assert_always ( intf->num > 0, "Invalid intf->num" );

	chips = auto_malloc( intf->num * sizeof( ppu2c0x_chip ) );
	memset(chips, 0, intf->num * sizeof( ppu2c0x_chip ));

	/* intialize our virtual chips */
	for( i = 0; i < intf->num; i++ )
	{
		chips[i].machine = machine;

		switch (intf->type)
		{
			case PPU_2C02:
				chips[i].scanlines_per_frame = PPU_NTSC_SCANLINES_PER_FRAME;
				break;
			case PPU_2C03B:
				chips[i].scanlines_per_frame = PPU_NTSC_SCANLINES_PER_FRAME;
				break;
			case PPU_2C04:
				chips[i].scanlines_per_frame = PPU_NTSC_SCANLINES_PER_FRAME;
				break;
			case PPU_2C05:
				chips[i].scanlines_per_frame = PPU_NTSC_SCANLINES_PER_FRAME;
				break;
			case PPU_2C07:
				chips[i].scanlines_per_frame = PPU_PAL_SCANLINES_PER_FRAME;
				break;
			default:
				chips[i].scanlines_per_frame = PPU_NTSC_SCANLINES_PER_FRAME;
				break;
		}

		/* initialize the scanline handling portion */
		chips[i].scanline_timer = timer_alloc(scanline_callback, NULL);
		chips[i].hblank_timer = timer_alloc(hblank_callback, NULL);
		chips[i].nmi_timer = timer_alloc(nmi_callback, NULL);
		chips[i].scanline = 0;
		chips[i].scan_scale = 1;

		/* allocate a screen bitmap, videoram and spriteram, a dirtychar array and the monochromatic colortable */
		chips[i].bitmap = auto_bitmap_alloc( VISIBLE_SCREEN_WIDTH, VISIBLE_SCREEN_HEIGHT, machine->screen[0].format );
		chips[i].videoram = auto_malloc( VIDEORAM_SIZE );
		chips[i].spriteram = auto_malloc( SPRITERAM_SIZE );
		chips[i].dirtychar = auto_malloc( CHARGEN_NUM_CHARS );
		chips[i].colortable = auto_malloc( sizeof( default_colortable ) );
		chips[i].colortable_mono = auto_malloc( sizeof( default_colortable_mono ) );

		/* clear videoram & spriteram */
		memset( chips[i].videoram, 0, VIDEORAM_SIZE );
		memset( chips[i].spriteram, 0, SPRITERAM_SIZE );

		/* set all characters dirty */
		memset( chips[i].dirtychar, 1, CHARGEN_NUM_CHARS );

		/* initialize the video ROM portion, if available */
		if ( ( intf->vrom_region[i] != REGION_INVALID ) && ( memory_region( intf->vrom_region[i] ) != 0 ) )
		{
			/* mark that we have a videorom */
			chips[i].has_videorom = 1;

			/* find out how many banks */
			chips[i].videorom_banks = memory_region_length( intf->vrom_region[i] ) / 0x2000;

			/* tweak the layout accordingly */
			total = chips[i].videorom_banks * CHARGEN_NUM_CHARS;
		}
		else
		{
			chips[i].has_videorom = chips[i].videorom_banks = 0;

			/* we need to reset this in case of mame running multisession */
			total = CHARGEN_NUM_CHARS;
		}

		/* now create the gfx region */
		{
			gfx_layout gl;
			UINT8 *src = chips[i].has_videorom ? memory_region( intf->vrom_region[i] ) : chips[i].videoram;

			memcpy(&gl, &ppu_charlayout, sizeof(gl));
			gl.total = total;
			machine->gfx[intf->gfx_layout_number[i]] = allocgfx( &gl );
			decodegfx( machine->gfx[intf->gfx_layout_number[i]], src, 0, machine->gfx[intf->gfx_layout_number[i]]->total_elements );
			machine->gfx[intf->gfx_layout_number[i]]->total_colors = 8;
		}

		/* setup our videoram handlers based on mirroring */
		ppu2c0x_set_mirroring( i, intf->mirroring[i] );
	}
}

static TIMER_CALLBACK( hblank_callback )
{
	int num = param;
	ppu2c0x_chip* this_ppu = &chips[num];
	int *ppu_regs = &chips[num].regs[0];

	int blanked = ( ppu_regs[PPU_CONTROL1] & ( PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES ) ) == 0;
	int vblank = ((this_ppu->scanline >= PPU_VBLANK_FIRST_SCANLINE-1) && (this_ppu->scanline < this_ppu->scanlines_per_frame-1)) ? 1 : 0;

//  update_scanline (num);

	if (this_ppu->hblank_callback_proc)
		(*this_ppu->hblank_callback_proc) (num, this_ppu->scanline, vblank, blanked);

	timer_adjust(chips[num].hblank_timer, attotime_never, num, attotime_never);
}

static TIMER_CALLBACK( nmi_callback )
{
	int num = param;
	int *ppu_regs = &chips[num].regs[0];

	// Actually fire the VMI
	if (intf->nmi_handler[num])
		(*intf->nmi_handler[num]) (num, ppu_regs);

	timer_adjust(chips[num].nmi_timer, attotime_never, num, attotime_never);
}

static void draw_background(const int num, UINT8 *line_priority )
{
	/* cache some values locally */
	mame_bitmap *bitmap = chips[num].bitmap;
	const int *ppu_regs = &chips[num].regs[0];
	const int scanline = chips[num].scanline;
	const int refresh_data = chips[num].refresh_data;
	const int gfx_bank = intf->gfx_layout_number[num];
	const int total_elements = chips[num].machine->gfx[gfx_bank]->total_elements;
	const int *nes_vram = &chips[num].nes_vram[0];
	const int tile_page = chips[num].tile_page;
	const int char_modulo = chips[num].machine->gfx[gfx_bank]->char_modulo;
	const int line_modulo = chips[num].machine->gfx[gfx_bank]->line_modulo;
	UINT8 *gfx_data = chips[num].machine->gfx[gfx_bank]->gfxdata;
	UINT8 **ppu_page = chips[num].ppu_page;
	int	start_x = ( chips[num].x_fine ^ 0x07 ) - 7;
	UINT16 back_pen;
	UINT16 *dest;

	UINT8 scroll_x_coarse, scroll_y_coarse, scroll_y_fine, color_mask;
	int x, tile_index, start, i;

	const pen_t *color_table;
	const pen_t *paldata;
	const UINT8 *sd;

	int tilecount=0;

	/* setup the color mask and colortable to use */
	if ( ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO )
	{
		color_mask = 0xf0;
		color_table = chips[num].colortable_mono;
	}
	else
	{
		color_mask = 0xff;
		color_table = chips[num].colortable;
	}

	/* cache the background pen */
	back_pen = chips[num].machine->pens[(chips[num].back_color & color_mask)+intf->color_base[num]];

	/* determine where in the nametable to start drawing from */
	/* based on the current scanline and scroll regs */
	scroll_x_coarse = refresh_data & 0x1f;
	scroll_y_coarse = ( refresh_data & 0x3e0 ) >> 5;
	scroll_y_fine = ( refresh_data & 0x7000 ) >> 12;

	x = scroll_x_coarse;

	/* get the tile index */
	tile_index = ( ( refresh_data & 0xc00 ) | 0x2000 ) + scroll_y_coarse * 32;

	/* set up dest */
	dest = ((UINT16 *) bitmap->base) + (bitmap->rowpixels * scanline) + start_x;

	/* draw the 32 or 33 tiles that make up a line */
	while ( tilecount <34)
	{
		int color_byte;
		int color_bits;
		int pos;
		int index1;
		int page, page2, address;
		int index2;
		UINT16 pen;

		index1 = tile_index + x;

		/* Figure out which byte in the color table to use */
		pos = ( ( index1 & 0x380 ) >> 4 ) | ( ( index1 & 0x1f ) >> 2 );
		page = (index1 & 0x0c00) >> 10;
		address = 0x3c0 + pos;
		color_byte = ppu_page[page][address];

		/* figure out which bits in the color table to use */
		color_bits = ( ( index1 & 0x40 ) >> 4 ) + ( index1 & 0x02 );

		address = index1 & 0x3ff;
		page2 = ppu_page[page][address];
		index2 = nes_vram[ ( page2 >> 6 ) | tile_page ] + ( page2 & 0x3f );

		//27/12/2002
		if( ppu_latch )
		{
			(*ppu_latch)(( tile_page << 10 ) | ( page2 << 4 ));
		}

		if(start_x < VISIBLE_SCREEN_WIDTH )
		{
			paldata = &color_table[ 4 * ( ( ( color_byte >> color_bits ) & 0x03 ) ) ];
			start = ( index2 % total_elements ) * char_modulo + scroll_y_fine * line_modulo;
			sd = &gfx_data[start];

			/* render the pixel */
			for( i = 0; i < 8; i++ )
			{
				if ( ( start_x+i ) >= 0 && ( start_x+i ) < VISIBLE_SCREEN_WIDTH )
				{
					if ( sd[i] )
					{
						pen = paldata[sd[i]];
						line_priority[ start_x+i ] |= 0x02;
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
			if ( x > 31 )
			{
				x = 0;
				tile_index ^= 0x400;
			}
		}
		tilecount++;
	}

	/* if the left 8 pixels for the background are off, blank 'em */
	if ( !( ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND_L8 ) )
	{
		dest = ((UINT16 *) bitmap->base) + (bitmap->rowpixels * scanline);
		for( i = 0; i < 8; i++ )
		{
			*(dest++) = back_pen;
			line_priority[ i ] ^= 0x02;
		}
	}
}

static void draw_sprites(const int num, UINT8 *line_priority )
{
	/* cache some values locally */
	mame_bitmap *bitmap = chips[num].bitmap;
	const int scanline = chips[num].scanline;
	const int gfx_bank = intf->gfx_layout_number[num];
	const int total_elements = chips[num].machine->gfx[gfx_bank]->total_elements;
	const int sprite_page = chips[num].sprite_page;
	const int char_modulo = chips[num].machine->gfx[gfx_bank]->char_modulo;
	const int line_modulo = chips[num].machine->gfx[gfx_bank]->line_modulo;
	const UINT8 *sprite_ram = chips[num].spriteram;
	pen_t *color_table = chips[num].colortable;
	UINT8 *gfx_data = chips[num].machine->gfx[gfx_bank]->gfxdata;
	int *ppu_regs = &chips[num].regs[0];

	int spriteXPos, spriteYPos, spriteIndex;
	int tile, index1, page;
	int pri;

	int flipx, flipy, color;
	int size;
	int spriteCount = 0;
	int sprite_line;
	int drawn;
	int start;

	int first_pixel;

	const pen_t *paldata;
	const UINT8 *sd;
	int pixel;

	/* determine if the sprites are 8x8 or 8x16 */
	size = ( ppu_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE ) ? 16 : 8;

	first_pixel = (ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES_L8)? 0: 8;

	for( spriteIndex = 0; spriteIndex < SPRITERAM_SIZE; spriteIndex += 4 )
	{
		spriteYPos = sprite_ram[spriteIndex] + 1;
		spriteXPos = sprite_ram[spriteIndex+3];

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
		if ( ( spriteYPos + size <= scanline ) || ( spriteYPos > scanline ) )
			continue;

		/* clear our drawn flag */
		drawn = 0;

		tile = sprite_ram[spriteIndex+1];
		color = ( sprite_ram[spriteIndex+2] & 0x03 ) + 4;
		pri = sprite_ram[spriteIndex+2] & 0x20;
		flipx = sprite_ram[spriteIndex+2] & 0x40;
		flipy = sprite_ram[spriteIndex+2] & 0x80;

		if ( size == 16 )
		{
			/* if it's 8x16 and odd-numbered, draw the other half instead */
			if ( tile & 0x01 )
			{
				tile &= ~0x01;
				tile |= 0x100;
			}
			/* note that the sprite page value has no effect on 8x16 sprites */
			page = tile >> 6;
		}
		else
			page = ( tile >> 6 ) | sprite_page;


		index1 = chips[num].nes_vram[page] + ( tile & 0x3f );

		if ( ppu_latch )
			(*ppu_latch)(( sprite_page << 10 ) | ( (tile & 0xff) << 4 ));

		/* compute the character's line to draw */
		sprite_line = scanline - spriteYPos;

		if ( flipy )
			sprite_line = ( size - 1 ) - sprite_line;

		paldata = &color_table[4 * color];
		start = ( index1 % total_elements ) * char_modulo + sprite_line * line_modulo;
		sd = &gfx_data[start];

		if ( pri )
		{
			/* draw the low-priority sprites */

			for ( pixel = 0; pixel < 8; pixel++ )
			{
				UINT8 pixelData = flipx ? sd[7-pixel] : sd[pixel];

				/* is this pixel non-transparent? */
				if ( spriteXPos + pixel >= first_pixel)
				{
					if (pixelData)
					{
						/* has the background (or another sprite) already been drawn here? */
						if ( !line_priority[ spriteXPos + pixel ] )
						{
							/* no, draw */
							if ( ( spriteXPos + pixel ) < VISIBLE_SCREEN_WIDTH )
								*BITMAP_ADDR16(bitmap, scanline, spriteXPos + pixel) = paldata[pixelData];
							drawn = 1;
						}
						/* indicate that a sprite was drawn at this location, even if it's not seen */
						if ( ( spriteXPos + pixel ) < VISIBLE_SCREEN_WIDTH )
							line_priority[ spriteXPos + pixel ] |= 0x01;
					}

					/* set the "sprite 0 hit" flag if appropriate */
					if ( spriteIndex == 0 && (pixelData & 0x03) && ((spriteXPos + pixel) < 255) && ( line_priority[ spriteXPos + pixel ] & 0x02 ))
						ppu_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
				}
			}
		}
		else
		{
			/* draw the high-priority sprites */

			for ( pixel = 0; pixel < 8; pixel++ )
			{
				UINT8 pixelData = flipx ? sd[7-pixel] : sd[pixel];

				/* is this pixel non-transparent? */
				if ( spriteXPos + pixel >= first_pixel)
				{
					if (pixelData)
					{
						/* has another sprite been drawn here? */
						if ( !( line_priority[ spriteXPos + pixel ] & 0x01 ) )
						{
							/* no, draw */
							if ( ( spriteXPos + pixel ) < VISIBLE_SCREEN_WIDTH )
							{
								*BITMAP_ADDR16(bitmap, scanline, spriteXPos + pixel) = paldata[pixelData];
								line_priority[ spriteXPos + pixel ] |= 0x01;
							}
							drawn = 1;
						}
					}

					/* set the "sprite 0 hit" flag if appropriate */
					if ( spriteIndex == 0 && (pixelData & 0x03) && ((spriteXPos + pixel) < 255) && ( line_priority[ spriteXPos + pixel ] & 0x02 ))
						ppu_regs[PPU_STATUS] |= PPU_STATUS_SPRITE0_HIT;
				}
			}
		}

		if ( drawn )
		{
			/* if there are more than 8 sprites on this line, set the flag */
			spriteCount++;
			if ( spriteCount == 8 )
			{
				ppu_regs[PPU_STATUS] |= PPU_STATUS_8SPRITES;
//              logerror ("> 8 sprites, scanline: %d\n", scanline);

				/* the real NES only draws up to 8 sprites - the rest should be invisible */
				break;
			}
		}
	}
}

/*************************************
 *
 *  Scanline Rendering and Update
 *
 *************************************/
static void render_scanline(int num)
{
	UINT8	line_priority[VISIBLE_SCREEN_WIDTH];
	int		*ppu_regs = &chips[num].regs[0];

	/* lets see how long it takes */
	profiler_mark(PROFILER_USER1+num);

	/* clear the line priority for this scanline */
	memset( line_priority, 0, VISIBLE_SCREEN_WIDTH );

	/* clear the sprite count for this line */
	ppu_regs[PPU_STATUS] &= ~PPU_STATUS_8SPRITES;

	/* see if we need to render the background */
	if ( ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_BACKGROUND )
		draw_background(num, line_priority );
	else
	{
		mame_bitmap *bitmap = chips[num].bitmap;
		const int scanline = chips[num].scanline;
		UINT8 color_mask;
		UINT16 back_pen;
		int i;

		/* setup the color mask and colortable to use */
		if ( ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO )
			color_mask = 0xf0;
		else
			color_mask = 0xff;

		/* cache the background pen */
		back_pen = chips[num].machine->pens[(chips[num].back_color & color_mask)+intf->color_base[num]];

		// Fill this scanline with the background pen.
		for (i = 0; i < bitmap->width; i ++)
			*BITMAP_ADDR16(bitmap, scanline, i) = back_pen;
	}

	/* if sprites are on, draw them */
	if ( ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_SPRITES )
		draw_sprites( num, line_priority );

 	/* done updating, whew */
	profiler_mark(PROFILER_END);
}

static void update_scanline(int num )
{
	ppu2c0x_chip* this_ppu;
	int scanline = chips[num].scanline;
	int *ppu_regs = &chips[num].regs[0];

	this_ppu = &chips[num];

	if ( scanline <= PPU_BOTTOM_VISIBLE_SCANLINE )
	{
		/* Render this scanline if appropriate */
		if ( ppu_regs[PPU_CONTROL1] & ( PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES ) )
		{
			/* If background or sprites are enabled, copy the ppu address latch */
			/* Copy only the scroll x-coarse and the x-overflow bit */
			this_ppu->refresh_data &= ~0x041f;
			this_ppu->refresh_data |= ( this_ppu->refresh_latch & 0x041f );

//logerror("   updating refresh_data: %04x (scanline: %d)\n", this_ppu->refresh_data, this_ppu->scanline);
			render_scanline( num );
		}
		else
		{
			mame_bitmap *bitmap = this_ppu->bitmap;
			const int scanline = this_ppu->scanline;
			UINT8 color_mask;
			UINT16 back_pen;
			int i;

			/* setup the color mask and colortable to use */
			if ( ppu_regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO )
				color_mask = 0xf0;
			else
				color_mask = 0xff;

			/* cache the background pen */
			if (this_ppu->videoram_addr >= 0x3f00)
			{
				// If the PPU's VRAM address happens to point into palette ram space while
				// both the sprites and background are disabled, the PPU paints the scanline
				// with the palette entry at the VRAM address instead of the usual background
				// pen. Micro Machines makes use of this feature.
				int penNum;
				if (this_ppu->videoram_addr & 0x03)
				{
					penNum = this_ppu->videoram[this_ppu->videoram_addr & 0x3f1f] & 0x3f;
				}
				else
				{
					penNum = this_ppu->videoram[this_ppu->videoram_addr & 0x3f00] & 0x3f;
				}
				back_pen = chips[num].machine->pens[penNum + intf->color_base[num]];
			}
			else
				back_pen = chips[num].machine->pens[(this_ppu->back_color & color_mask)+intf->color_base[num]];

			// Fill this scanline with the background pen.
			for (i = 0; i < bitmap->width; i ++)
				*BITMAP_ADDR16(bitmap, scanline, i) = back_pen;
		}

		/* increment the fine y-scroll */
		this_ppu->refresh_data += 0x1000;

		/* if it's rolled, increment the coarse y-scroll */
		if ( this_ppu->refresh_data & 0x8000 )
		{
			UINT16 tmp;
			tmp = ( this_ppu->refresh_data & 0x03e0 ) + 0x20;
			this_ppu->refresh_data &= 0x7c1f;
			/* handle bizarro scrolling rollover at the 30th (not 32nd) vertical tile */
			if ( tmp == 0x03c0 )
			{
				this_ppu->refresh_data ^= 0x0800;
			}
			else
			{
				this_ppu->refresh_data |= ( tmp & 0x03e0 );
			}
//logerror("updating refresh_data: %04x\n", this_ppu->refresh_data);
	    }
	}

}

static TIMER_CALLBACK( scanline_callback )
{
	int num = param;
	ppu2c0x_chip* this_ppu = &chips[num];
	int *ppu_regs = &chips[num].regs[0];
	int i;
	int blanked = ( ppu_regs[PPU_CONTROL1] & ( PPU_CONTROL1_BACKGROUND | PPU_CONTROL1_SPRITES ) ) == 0;
	int vblank = ((this_ppu->scanline >= PPU_VBLANK_FIRST_SCANLINE-1) && (this_ppu->scanline < this_ppu->scanlines_per_frame-1)) ? 1 : 0;
	int next_scanline;

	/* if a callback is available, call it */
	if ( this_ppu->scanline_callback_proc )
		(*this_ppu->scanline_callback_proc)( num, this_ppu->scanline, vblank, blanked );

	/* update the scanline that just went by */
	update_scanline( num );

	/* increment our scanline count */
	this_ppu->scanline++;

//logerror("starting scanline %d (MAME %d, beam %d)\n", this_ppu->scanline, video_screen_get_vpos(0), video_screen_get_hpos(0));

	/* Note: this is called at the _end_ of each scanline */
	if (this_ppu->scanline == PPU_VBLANK_FIRST_SCANLINE)
	{
logerror("vlbank starting\n");
		/* We just entered VBLANK */
		ppu_regs[PPU_STATUS] |= PPU_STATUS_VBLANK;

		/* If NMI's are set to be triggered, go for it */
		if (ppu_regs[PPU_CONTROL0] & PPU_CONTROL0_NMI)
		{
			// We need an ever-so-slight delay between entering vblank and firing an NMI - enough so that
			// a game can read the high bit of $2002 before the NMI is called (potentially resetting the bit
			// via a read from $2002 in the NMI handler).
			// B-Wings is an example game that needs this.
			timer_adjust(this_ppu->nmi_timer, ATTOTIME_IN_CYCLES(4, 0), num, attotime_never);
		}
	}

	/* decode any dirty chars if we're using vram */

	/* first, check the master dirty char flag */
	if ( !this_ppu->has_videorom && this_ppu->chars_are_dirty )
	{
		/* cache some values */
		UINT8 *dirtyarray = this_ppu->dirtychar;
		UINT8 *vram = this_ppu->videoram;
		gfx_element *gfx = chips[num].machine->gfx[intf->gfx_layout_number[num]];

		/* then iterate and decode */
		for( i = 0; i < CHARGEN_NUM_CHARS; i++ )
		{
			if ( dirtyarray[i] )
			{
				decodechar( gfx, i, vram );
				dirtyarray[i] = 0;
			}
		}

		this_ppu->chars_are_dirty = 0;
	}

	if ( this_ppu->scanline == this_ppu->scanlines_per_frame - 1 )
	{
logerror("vlbank ending\n");
		/* clear the vblank & sprite hit flag */
		ppu_regs[PPU_STATUS] &= ~( PPU_STATUS_VBLANK | PPU_STATUS_SPRITE0_HIT );
	}

	/* see if we rolled */
	else if ( this_ppu->scanline == this_ppu->scanlines_per_frame )
	{
		/* if background or sprites are enabled, copy the ppu address latch */
		if ( !blanked )
			this_ppu->refresh_data = this_ppu->refresh_latch;

		/* reset the scanline count */
		this_ppu->scanline = 0;
//logerror("   sprite 0 x: %d y: %d num: %d\n", this_ppu->spriteram[3], this_ppu->spriteram[0]+1, this_ppu->spriteram[1]);
	}

	next_scanline = this_ppu->scanline+1;
	if (next_scanline == this_ppu->scanlines_per_frame)
		next_scanline = 0;

	// Call us back when the hblank starts for this scanline
	timer_adjust(this_ppu->hblank_timer, ATTOTIME_IN_CYCLES(86.67, 0), num, attotime_never); // ??? FIXME - hardcoding NTSC, need better calculation

	// trigger again at the start of the next scanline
	timer_adjust(this_ppu->scanline_timer, video_screen_get_time_until_pos(0, next_scanline * this_ppu->scan_scale, 0), num, attotime_zero);
}

/*************************************
 *
 *  PPU Reset
 *
 *************************************/
void ppu2c0x_reset(int num, int scan_scale )
{
	int i;

	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(reset): Attempting to access an unmapped chip\n" );
		return;
	}

	/* reset the scanline count */
	chips[num].scanline = 0;

	/* set the scan scale (this is for dual monitor vertical setups) */
	chips[num].scan_scale = scan_scale;

	timer_adjust(chips[num].nmi_timer, attotime_never, num, attotime_never);

	// Call us back when the hblank starts for this scanline
	timer_adjust(chips[num].hblank_timer, ATTOTIME_IN_CYCLES(86.67, 0), num, attotime_never); // ??? FIXME - hardcoding NTSC, need better calculation

	// Call us back at the start of the next scanline
	timer_adjust(chips[num].scanline_timer, video_screen_get_time_until_pos(0, 1, 0), num, attotime_zero);

	/* reset the callbacks */
	chips[num].scanline_callback_proc = 0;
	chips[num].vidaccess_callback_proc = 0;

	for( i = 0; i < PPU_MAX_REG; i++ )
		chips[num].regs[i] = 0;

	/* initialize the rest of the members */
	chips[num].refresh_data = 0;
	chips[num].refresh_latch = 0;
	chips[num].x_fine = 0;
	chips[num].toggle = 0;
	chips[num].add = 1;
	chips[num].videoram_addr = 0;
	chips[num].addr_latch = 0;
	chips[num].data_latch = 0;
	chips[num].tile_page = 0;
	chips[num].sprite_page = 0;
	chips[num].back_color = 0;
	chips[num].chars_are_dirty = 1;

	/* initialize the color tables */
	{
		int color_base = intf->color_base[num];

		for( i = 0; i < ARRAY_LENGTH( default_colortable_mono ); i++ )
		{
			/* monochromatic table */
			chips[num].colortable_mono[i] = chips[num].machine->pens[default_colortable_mono[i] + color_base];

			/* color table */
			chips[num].colortable[i] = chips[num].machine->pens[default_colortable[i] + color_base];
		}
	}

	/* set the vram bank-switch values to the default */
	for( i = 0; i < 8; i++ )
		chips[num].nes_vram[i] = i * 64;

	if ( chips[num].has_videorom )
		ppu2c0x_set_videorom_bank( num, 0, 8, 0, 512 );
}


/*************************************
 *
 *  PPU Registers Read
 *
 *************************************/
int ppu2c0x_r( int num, offs_t offset )
{
	ppu2c0x_chip* this_ppu;

	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU %d(r): Attempting to access an unmapped chip\n", num );
		return 0;
	}

	this_ppu = &chips[num];

	if ( offset >= PPU_MAX_REG )
	{
		logerror( "PPU %d(r): Attempting to read past the chip\n", num );
		offset &= PPU_MAX_REG - 1;
	}

	// see which register to read
	switch( offset & 7 )
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
			if ( this_ppu->videoram_addr >= 0x3f00  )
			{
				this_ppu->data_latch = this_ppu->videoram[this_ppu->videoram_addr & 0x3F1F];
				if (this_ppu->regs[PPU_CONTROL1] & PPU_CONTROL1_DISPLAY_MONO)
					this_ppu->data_latch &= 0x30;
			}
			else
				this_ppu->data_latch = this_ppu->buffered_data;

			if ( ppu_latch )
				(*ppu_latch)( this_ppu->videoram_addr & 0x3fff );

			if ( ( this_ppu->videoram_addr >= 0x2000 ) && ( this_ppu->videoram_addr <= 0x3fff ) )
				this_ppu->buffered_data = this_ppu->ppu_page[ ( this_ppu->videoram_addr & 0xc00) >> 10][ this_ppu->videoram_addr & 0x3ff ];
			else
				this_ppu->buffered_data = this_ppu->videoram[ this_ppu->videoram_addr & 0x3fff ];

			this_ppu->videoram_addr += this_ppu->add;
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
void ppu2c0x_w( int num, offs_t offset, UINT8 data )
{
	ppu2c0x_chip* this_ppu;
	int color_base;

	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(w): Attempting to access an unmapped chip\n" );
		return;
	}

	this_ppu = &chips[num];
	color_base = intf->color_base[num];

	if ( offset >= PPU_MAX_REG )
	{
		logerror( "PPU: Attempting to write past the chip\n" );
		offset &= PPU_MAX_REG - 1;
	}

#ifdef MAME_DEBUG
	if (this_ppu->scanline <= PPU_BOTTOM_VISIBLE_SCANLINE)
		logerror("  PPU register %d write %02x during non-vblank scanline %d (MAME %d, beam pos: %d)\n", offset, data, this_ppu->scanline, video_screen_get_vpos(0), video_screen_get_hpos(0));
#endif

	switch( offset & 7 )
	{
		case PPU_CONTROL0:
			this_ppu->regs[PPU_CONTROL0] = data;

			/* update the name table number on our refresh latches */
			this_ppu->refresh_latch &= 0x73ff;
			this_ppu->refresh_latch |= ( data & 3 ) << 10;

			/* the char ram bank points either 0x0000 or 0x1000 (page 0 or page 4) */
			this_ppu->tile_page = ( data & PPU_CONTROL0_CHR_SELECT ) >> 2;
			this_ppu->sprite_page = ( data & PPU_CONTROL0_SPR_SELECT ) >> 1;

			this_ppu->add = ( data & PPU_CONTROL0_INC ) ? 32 : 1;
//logerror("   control0 write: %02x (scanline: %d)\n", data, this_ppu->scanline);
			break;

		case PPU_CONTROL1:
			/* if color intensity has changed, change all the color tables to reflect them */
			if ( ( data & PPU_CONTROL1_COLOR_EMPHASIS ) != ( this_ppu->regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS ) )
			{
				int i;
				for (i = 0; i <= 0x1f; i ++)
				{
					UINT8 oldColor = this_ppu->videoram[i+0x3f00];

					this_ppu->colortable[i] = chips[num].machine->pens[color_base + oldColor + (data & PPU_CONTROL1_COLOR_EMPHASIS)*2];
				}
			}

//logerror("   control1 write: %02x (scanline: %d)\n", data, this_ppu->scanline);
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
			this_ppu->regs[PPU_SPRITE_ADDRESS] = ( this_ppu->regs[PPU_SPRITE_ADDRESS] + 1 ) & 0xff;
			break;

		case PPU_SCROLL:
			if ( this_ppu->toggle )
			{
				/* second write */
				this_ppu->refresh_latch &= 0x0c1f;
				this_ppu->refresh_latch |= ( data & 0xf8 ) << 2;
				this_ppu->refresh_latch |= ( data & 0x07 ) << 12;
//logerror("   scroll write 2: %d, %04x (scanline: %d)\n", data, this_ppu->refresh_latch, this_ppu->scanline);
			}
			else
			{
				/* first write */
				this_ppu->refresh_latch &= 0x7fe0;
				this_ppu->refresh_latch |= (data & 0xf8) >> 3;

				this_ppu->x_fine = data & 7;
//logerror("   scroll write 1: %d, %04x (scanline: %d)\n", data, this_ppu->refresh_latch, this_ppu->scanline);
			}

			this_ppu->toggle ^= 1;
			break;

		case PPU_ADDRESS:
			if ( this_ppu->toggle )
			{
				/* second write */
				this_ppu->refresh_latch &= 0x7f00;
				this_ppu->refresh_latch |= data;
				this_ppu->refresh_data = this_ppu->refresh_latch;

				this_ppu->videoram_addr = this_ppu->refresh_latch;
//logerror("   vram addr write 2: %02x, %04x (scanline: %d)\n", data, this_ppu->refresh_latch, this_ppu->scanline);
			}
			else
			{
				/* first write */
				this_ppu->refresh_latch &= 0x00ff;
				this_ppu->refresh_latch |= ( data & 0x3f ) << 8;
//logerror("   vram addr write 1: %02x, %04x (scanline: %d)\n", data, this_ppu->refresh_latch, this_ppu->scanline);
			}

			this_ppu->toggle ^= 1;
			break;

		case PPU_DATA:
			{
				int tempAddr = this_ppu->videoram_addr & 0x3fff;

				if ( ppu_latch )
					(*ppu_latch)( tempAddr );

				/* if there's a callback, call it now */
				if ( this_ppu->vidaccess_callback_proc )
					data = (*this_ppu->vidaccess_callback_proc)( num, tempAddr, data );

				/* see if it's on the chargen portion */
				if ( tempAddr < 0x2000 )
				{
					/* if we have a videorom mapped there, dont write and log the problem */
					if ( this_ppu->has_videorom )
					{
						/* if there is a vidaccess callback, assume it coped with it */
						if ( this_ppu->vidaccess_callback_proc == NULL )
							logerror( "PPU: Attempting to write to the chargen when there's a ROM there!\n" );
					}
					else
					{
						/* store the data */
						this_ppu->videoram[tempAddr] = data;

						/* setup the master dirty switch */
						this_ppu->chars_are_dirty = 1;

						/* mark the char dirty */
						this_ppu->dirtychar[tempAddr >> 4] = 1;
					}
				}

				else if ( tempAddr >= 0x3f00 )
				{
					int colorEmphasis = (this_ppu->regs[PPU_CONTROL1] & PPU_CONTROL1_COLOR_EMPHASIS) * 2;

					/* store the data */
					if (tempAddr & 0x03)
						this_ppu->videoram[tempAddr & 0x3F1F] = data;
					else
					{
						this_ppu->videoram[0x3F10+(tempAddr&0xF)] = data;
						this_ppu->videoram[0x3F00+(tempAddr&0xF)] = data;
					}

					/* As usual, some games attempt to write values > the number of colors so we must mask the data. */
					data &= 0x3f;

					if ( tempAddr & 0x03 )
					{
						this_ppu->colortable[ tempAddr & 0x1f ] = chips[num].machine->pens[color_base + data + colorEmphasis];
						this_ppu->colortable_mono[tempAddr & 0x1f] = chips[num].machine->pens[color_base + (data & 0xf0) + colorEmphasis];
					}

					/* The only valid background colors are writes to 0x3f00 and 0x3f10 */
					/* and even then, they are mirrors of each other. */
					if ( ( tempAddr & 0x0f ) == 0 )
					{
						int i;

						this_ppu->back_color = data;
						for( i = 0; i < 32; i += 4 )
						{
							this_ppu->colortable[ i ] = chips[num].machine->pens[color_base + data + colorEmphasis];
							this_ppu->colortable_mono[i] = chips[num].machine->pens[color_base + (data & 0xf0) + colorEmphasis];
						}
					}
				}

				/* everything else */
				/* writes to $3000-$3eff are mirrors of $2000-$2eff */
				else
				{
					int page = ( tempAddr & 0x0c00) >> 10;
					int address = tempAddr & 0x3ff;

					this_ppu->ppu_page[page][address] = data;
				}

				/* increment the address */
				this_ppu->videoram_addr += this_ppu->add;
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
void ppu2c0x_spriteram_dma (int num, const UINT8 page)
{
	int i;
	int address = page << 8;

	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(w): Attempting to access an unmapped chip\n" );
		return;
	}

//logerror("   sprite DMA: %d (scanline: %d)\n", page, chips[num].scanline);
	for (i = 0; i < SPRITERAM_SIZE; i++)
	{
		UINT8 spriteData = program_read_byte_8 (address + i);
		ppu2c0x_w (num, PPU_SPRITE_DATA, spriteData);
	}

	// should last 513 CPU cycles.
	activecpu_adjust_icount(-513);

	// ????TODO : need to account for PPU rendering - this is roughly 4.5 scanlines eaten up.
	// Because the DMA is only useful during vblank, this may not be strictly necessary since
	// the scanline timers should catch us up before drawing actually happens.
#if 0
	scanline_callback(Machine, num);
	scanline_callback(Machine, num);
	scanline_callback(Machine, num);
	scanline_callback(Machine, num);
#endif
}

/*************************************
 *
 *  PPU Rendering
 *
 *************************************/
void ppu2c0x_render( int num, mame_bitmap *bitmap, int flipx, int flipy, int sx, int sy )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(render): Attempting to access an unmapped chip\n" );
		return;
	}

	copybitmap( bitmap, chips[num].bitmap, flipx, flipy, sx, sy, 0, TRANSPARENCY_NONE, 0 );
}

/*************************************
 *
 *  PPU VideoROM banking
 *
 *************************************/
void ppu2c0x_set_videorom_bank( int num, int start_page, int num_pages, int bank, int bank_size )
{
	int i;

	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(set vrom bank): Attempting to access an unmapped chip\n" );
		return;
	}

	if ( !chips[num].has_videorom )
	{
		logerror( "PPU(set vrom bank): Attempting to switch videorom banks and no rom is mapped\n" );
		return;
	}

	bank &= ( chips[num].videorom_banks * ( CHARGEN_NUM_CHARS / bank_size ) ) - 1;

	for( i = start_page; i < ( start_page + num_pages ); i++ )
		chips[num].nes_vram[i] = bank * bank_size + 64 * ( i - start_page );

	{
		int vram_start = start_page * 0x400;
		int count = num_pages * 0x400;
		int rom_start = bank * bank_size * 16;

		memcpy( &chips[num].videoram[vram_start], &memory_region( intf->vrom_region[num] )[rom_start], count );
	}
}

/*************************************
 *
 *  Utility functions
 *
 *************************************/
int ppu2c0x_get_pixel( int num, int x, int y )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(get_pixel): Attempting to access an unmapped chip\n" );
		return 0;
	}

	if ( x >= VISIBLE_SCREEN_WIDTH )
		x = VISIBLE_SCREEN_WIDTH - 1;

	if ( y >= VISIBLE_SCREEN_HEIGHT )
		y = VISIBLE_SCREEN_HEIGHT - 1;

	return *BITMAP_ADDR16(chips[num].bitmap, y, x);
}

int ppu2c0x_get_colorbase( int num )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(get_colorbase): Attempting to access an unmapped chip\n" );
		return 0;
	}

	return intf->color_base[num];
}

int ppu2c0x_get_current_scanline( int num )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(get_colorbase): Attempting to access an unmapped chip\n" );
		return 0;
	}

	return chips[num].scanline;
}

void ppu2c0x_set_mirroring( int num, int mirroring )
{
	ppu2c0x_chip* this_ppu;

	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU %d: Attempting to access an unmapped chip\n", num );
		return;
	}

	this_ppu = &chips[num];

	// Once we've set 4-screen mirroring, do not change. Some games
	// (notably Gauntlet) use mappers that can change the mirroring
	// state, but are also hard-coded for 4-screen VRAM.
	if (this_ppu->mirror_state == PPU_MIRROR_4SCREEN)
		return;

	/* setup our videoram handlers based on mirroring */
	switch( mirroring )
	{
		case PPU_MIRROR_VERT:
			this_ppu->ppu_page[0] = &(this_ppu->videoram[0x2000]);
			this_ppu->ppu_page[1] = &(this_ppu->videoram[0x2400]);
			this_ppu->ppu_page[2] = &(this_ppu->videoram[0x2000]);
			this_ppu->ppu_page[3] = &(this_ppu->videoram[0x2400]);
			break;

		case PPU_MIRROR_HORZ:
			this_ppu->ppu_page[0] = &(this_ppu->videoram[0x2000]);
			this_ppu->ppu_page[1] = &(this_ppu->videoram[0x2000]);
			this_ppu->ppu_page[2] = &(this_ppu->videoram[0x2400]);
			this_ppu->ppu_page[3] = &(this_ppu->videoram[0x2400]);
			break;

		case PPU_MIRROR_HIGH:
			this_ppu->ppu_page[0] = &(this_ppu->videoram[0x2400]);
			this_ppu->ppu_page[1] = &(this_ppu->videoram[0x2400]);
			this_ppu->ppu_page[2] = &(this_ppu->videoram[0x2400]);
			this_ppu->ppu_page[3] = &(this_ppu->videoram[0x2400]);
			break;

		case PPU_MIRROR_LOW:
			this_ppu->ppu_page[0] = &(this_ppu->videoram[0x2000]);
			this_ppu->ppu_page[1] = &(this_ppu->videoram[0x2000]);
			this_ppu->ppu_page[2] = &(this_ppu->videoram[0x2000]);
			this_ppu->ppu_page[3] = &(this_ppu->videoram[0x2000]);
			break;

		case PPU_MIRROR_NONE:
		case PPU_MIRROR_4SCREEN:
		default:
			this_ppu->ppu_page[0] = &(this_ppu->videoram[0x2000]);
			this_ppu->ppu_page[1] = &(this_ppu->videoram[0x2400]);
			this_ppu->ppu_page[2] = &(this_ppu->videoram[0x2800]);
			this_ppu->ppu_page[3] = &(this_ppu->videoram[0x2c00]);
			break;
	}

	this_ppu->mirror_state = mirroring;
}

#ifdef UNUSED_FUNCTION
void ppu2c0x_set_nmi_callback( int num, ppu2c0x_nmi_cb cb )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(set_nmi_callback): Attempting to access an unmapped chip\n" );
		return;
	}

	intf->nmi_handler[num] = cb;
}
#endif

void ppu2c0x_set_scanline_callback( int num, ppu2c0x_scanline_cb cb )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(set_scanline_callback): Attempting to access an unmapped chip\n" );
		return;
	}

	chips[num].scanline_callback_proc = cb;
}

void ppu2c0x_set_hblank_callback( int num, ppu2c0x_hblank_cb cb )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(set_scanline_callback): Attempting to access an unmapped chip\n" );
		return;
	}

	chips[num].hblank_callback_proc = cb;
}

void ppu2c0x_set_vidaccess_callback( int num, ppu2c0x_vidaccess_cb cb )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(set_vidaccess_callback): Attempting to access an unmapped chip\n" );
		return;
	}

	chips[num].vidaccess_callback_proc = cb;
}

void ppu2c0x_set_scanlines_per_frame( int num, int scanlines )
{
	/* check bounds */
	if ( num >= intf->num )
	{
		logerror( "PPU(set_scanlines_per_frame): Attempting to access an unmapped chip\n" );
		return;
	}

	chips[num].scanlines_per_frame = scanlines;
}

/*************************************
 *
 *  Accesors
 *
 *************************************/

READ8_HANDLER( ppu2c0x_0_r )
{
	return ppu2c0x_r( 0, offset );
}

READ8_HANDLER( ppu2c0x_1_r )
{
	return ppu2c0x_r( 1, offset );
}

WRITE8_HANDLER( ppu2c0x_0_w )
{
	ppu2c0x_w( 0, offset, data );
}

WRITE8_HANDLER( ppu2c0x_1_w )
{
	ppu2c0x_w( 1, offset, data );
}
