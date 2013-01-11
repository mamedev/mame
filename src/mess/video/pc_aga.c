/*****************************************************************************
 *
 * video/pc_aga.c
 *
 ****************************************************************************/

#include "emu.h"
#include "video/pc_aga.h"
#include "video/pc_cga.h"
#include "includes/amstr_pc.h"
#include "video/mc6845.h"
#include "video/cgapal.h"


#define CGA_MONITOR     (space.machine().root_device().ioport("VIDEO")->read() & 0x1C)
#define CGA_MONITOR_RGB         0x00    /* Colour RGB */
#define CGA_MONITOR_MONO        0x04    /* Greyscale RGB */
#define CGA_MONITOR_COMPOSITE   0x08    /* Colour composite */
#define CGA_MONITOR_TELEVISION  0x0C    /* Television */
#define CGA_MONITOR_LCD         0x10    /* LCD, eg PPC512 */


static VIDEO_START( pc_aga );
static PALETTE_INIT( pc_aga );
static MC6845_UPDATE_ROW( aga_update_row );
static WRITE_LINE_DEVICE_HANDLER( aga_hsync_changed );
static WRITE_LINE_DEVICE_HANDLER( aga_vsync_changed );
static VIDEO_START( pc200 );


static const mc6845_interface mc6845_aga_intf = {
	AGA_SCREEN_NAME,    /* screen number */
	8,                  /* numbers of pixels per video memory address */
	NULL,               /* begin_update */
	aga_update_row,     /* update_row */
	NULL,               /* end_update */
	DEVCB_NULL,         /* on_de_chaged */
	DEVCB_NULL,         /* on_cur_chaged */
	DEVCB_LINE(aga_hsync_changed),  /* on_hsync_changed */
	DEVCB_LINE(aga_vsync_changed),  /* on_vsync_changed */
	NULL
};


static struct {
	AGA_MODE    mode;
	UINT8   mda_mode_control;
	UINT8   mda_status;
	UINT8   *mda_chr_gen;

	UINT8   cga_mode_control;
	UINT8   cga_color_select;
	UINT8   cga_status;
	UINT8   *cga_chr_gen;

	UINT8   pc_framecnt;

	mc6845_update_row_func  update_row;
	UINT8   cga_palette_lut_2bpp[4];
	UINT8   vsync;
	UINT8   hsync;

	UINT8  *videoram;
} aga;


MACHINE_CONFIG_FRAGMENT( pcvideo_aga )
	MCFG_SCREEN_ADD( AGA_SCREEN_NAME, RASTER )
	MCFG_SCREEN_RAW_PARAMS( XTAL_14_31818MHz,912,0,640,262,0,200 )
	MCFG_SCREEN_UPDATE_DEVICE( AGA_MC6845_NAME, mc6845_device, screen_update )

	MCFG_PALETTE_LENGTH( CGA_PALETTE_SETS * 16 )
	MCFG_PALETTE_INIT( pc_aga )

	MCFG_MC6845_ADD( AGA_MC6845_NAME, MC6845, XTAL_14_31818MHz/8, mc6845_aga_intf )

	MCFG_VIDEO_START( pc_aga )
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( pcvideo_pc200 )
	MCFG_FRAGMENT_ADD( pcvideo_aga )
	MCFG_VIDEO_START( pc200 )
MACHINE_CONFIG_END


/* Initialise the cga palette */
PALETTE_INIT( pc_aga )
{
	int i;
	for(i = 0; i < CGA_PALETTE_SETS * 16; i++)
		palette_set_color_rgb(machine, i, cga_palette[i][0], cga_palette[i][1], cga_palette[i][2]);
}


static MC6845_UPDATE_ROW( aga_update_row ) {
	if ( aga.update_row ) {
		aga.update_row( device, bitmap, cliprect, ma, ra, y, x_count, cursor_x, param );
	}
}


static WRITE_LINE_DEVICE_HANDLER( aga_hsync_changed ) {
	aga.hsync = state ? 1 : 0;
}


static WRITE_LINE_DEVICE_HANDLER( aga_vsync_changed ) {
	aga.vsync = state ? 8 : 0;
	if ( state ) {
		aga.pc_framecnt++;
	}
}


/*************************************
 *
 * row update functions
 *
 *************************************/

/* colors need fixing in the mda_text_* functions ! */
static MC6845_UPDATE_ROW( mda_text_inten_update_row ) {
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *videoram = aga.videoram;
	UINT32  *p = &bitmap.pix32(y);
	UINT16  chr_base = ( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	if ( y == 0 ) logerror("mda_text_inten_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset + 1 ];
		UINT8 data = aga.mda_chr_gen[ chr_base + chr * 8 ];
		UINT8 fg = ( attr & 0x08 ) ? 3 : 2;
		UINT8 bg = 0;

		if ( ( attr & ~0x88 ) == 0 ) {
			data = 0x00;
		}

		switch( attr ) {
		case 0x70:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
			bg = 2;
			fg = 1;
			break;
		case 0xF0:
			bg = 3;
			fg = 0;
			break;
		case 0xF8:
			bg = 3;
			fg = 1;
			break;
		}

		if ( i == cursor_x || ( attr & 0x07 ) == 0x01 ) {
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		if ( ( chr & 0xE0 ) == 0xC0 ) {
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		} else {
			*p = palette[bg]; p++;
		}
	}
}


static MC6845_UPDATE_ROW( mda_text_blink_update_row ) {
	UINT8 *videoram = aga.videoram;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT32  *p = &bitmap.pix32(y);
	UINT16  chr_base = ( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	if ( y == 0 ) logerror("mda_text_blink_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset + 1 ];
		UINT8 data = aga.mda_chr_gen[ chr_base + chr * 8 ];
		UINT8 fg = ( attr & 0x08 ) ? 3 : 2;
		UINT8 bg = 0;

		if ( ( attr & ~0x88 ) == 0 ) {
			data = 0x00;
		}

		switch( attr ) {
		case 0x70:
		case 0xF0:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
		case 0xF8:
			bg = 2;
			fg = 1;
			break;
		}

		if ( i == cursor_x ) {
			data = 0xFF;
		} else {
			if ( ( attr & 0x07 ) == 0x01 ) {
				data = 0xFF;
			}
			if ( ( attr & 0x80 ) && ( aga.pc_framecnt & 0x40 ) ) {
				data = 0x00;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		if ( ( chr & 0xE0 ) == 0xC0 ) {
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		} else {
			*p = palette[bg]; p++;
		}
	}
}


static MC6845_UPDATE_ROW( cga_text_inten_update_row ) {
	UINT8 *videoram = aga.videoram;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_text_inten_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = aga.cga_chr_gen[ chr * 16 + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = ( attr >> 4 ) & 0x07;

		if ( i == cursor_x ) {
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

static MC6845_UPDATE_ROW( cga_text_inten_alt_update_row ) {
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *videoram = aga.videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_text_inten_alt_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = aga.cga_chr_gen[ chr * 16 + ra ];
		UINT16 fg = attr & 0x0F;

		if ( i == cursor_x ) {
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : 0]; p++;
		*p = palette[( data & 0x40 ) ? fg : 0]; p++;
		*p = palette[( data & 0x20 ) ? fg : 0]; p++;
		*p = palette[( data & 0x10 ) ? fg : 0]; p++;
		*p = palette[( data & 0x08 ) ? fg : 0]; p++;
		*p = palette[( data & 0x04 ) ? fg : 0]; p++;
		*p = palette[( data & 0x02 ) ? fg : 0]; p++;
		*p = palette[( data & 0x01 ) ? fg : 0]; p++;
	}
}

static MC6845_UPDATE_ROW( cga_text_blink_update_row ) {
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *videoram = aga.videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;

	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = aga.cga_chr_gen[ chr * 16 + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = (attr >> 4) & 0x07;

		if ( i == cursor_x ) {
			data = 0xFF;
		} else {
			if ( ( attr & 0x80 ) && ( aga.pc_framecnt & 0x10 ) ) {
				data = 0x00;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

static MC6845_UPDATE_ROW( cga_text_blink_alt_update_row ) {
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *videoram = aga.videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_text_blink_alt_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = aga.cga_chr_gen[ chr * 16 + ra ];
		UINT16 fg = attr & 0x07;
		UINT16 bg = 0;

		if ( i == cursor_x ) {
			data = 0xFF;
		} else {
			if ( ( attr & 0x80 ) && ( aga.pc_framecnt & 0x10 ) ) {
				data = 0x00;
				bg = ( attr >> 4 ) & 0x07;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

static MC6845_UPDATE_ROW( cga_gfx_4bppl_update_row ) {
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 *videoram = aga.videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_gfx_4bppl_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( y & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;

		data = videoram[ offset + 1 ];

		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
	}
}

static MC6845_UPDATE_ROW( cga_gfx_4bpph_update_row ) {
	UINT8 *videoram = aga.videoram;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_gfx_4bpph_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( y & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;

		data = videoram[ offset + 1 ];

		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
	}
}

static MC6845_UPDATE_ROW( cga_gfx_2bpp_update_row ) {
	UINT8 *videoram = aga.videoram;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT32  *p = &bitmap.pix32(y);
	int i;

//  if ( y == 0 ) logerror("cga_gfx_2bpp_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( y & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[aga.cga_palette_lut_2bpp[ ( data >> 6 ) & 0x03 ]]; p++;
		*p = palette[aga.cga_palette_lut_2bpp[ ( data >> 4 ) & 0x03 ]]; p++;
		*p = palette[aga.cga_palette_lut_2bpp[ ( data >> 2 ) & 0x03 ]]; p++;
		*p = palette[aga.cga_palette_lut_2bpp[   data        & 0x03 ]]; p++;

		data = videoram[ offset+1 ];

		*p = palette[aga.cga_palette_lut_2bpp[ ( data >> 6 ) & 0x03 ]]; p++;
		*p = palette[aga.cga_palette_lut_2bpp[ ( data >> 4 ) & 0x03 ]]; p++;
		*p = palette[aga.cga_palette_lut_2bpp[ ( data >> 2 ) & 0x03 ]]; p++;
		*p = palette[aga.cga_palette_lut_2bpp[   data        & 0x03 ]]; p++;
	}
}

static MC6845_UPDATE_ROW( cga_gfx_1bpp_update_row ) {
	UINT8 *videoram = aga.videoram;
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT32  *p = &bitmap.pix32(y);
	UINT8   fg = aga.cga_color_select & 0x0F;
	int i;

	if ( y == 0 ) logerror("cga_gfx_1bpp_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( ra & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[( data & 0x80 ) ? fg : 0]; p++;
		*p = palette[( data & 0x40 ) ? fg : 0]; p++;
		*p = palette[( data & 0x20 ) ? fg : 0]; p++;
		*p = palette[( data & 0x10 ) ? fg : 0]; p++;
		*p = palette[( data & 0x08 ) ? fg : 0]; p++;
		*p = palette[( data & 0x04 ) ? fg : 0]; p++;
		*p = palette[( data & 0x02 ) ? fg : 0]; p++;
		*p = palette[( data & 0x01 ) ? fg : 0]; p++;

		data = videoram[ offset + 1 ];

		*p = palette[( data & 0x80 ) ? fg : 0]; p++;
		*p = palette[( data & 0x40 ) ? fg : 0]; p++;
		*p = palette[( data & 0x20 ) ? fg : 0]; p++;
		*p = palette[( data & 0x10 ) ? fg : 0]; p++;
		*p = palette[( data & 0x08 ) ? fg : 0]; p++;
		*p = palette[( data & 0x04 ) ? fg : 0]; p++;
		*p = palette[( data & 0x02 ) ? fg : 0]; p++;
		*p = palette[( data & 0x01 ) ? fg : 0]; p++;
	}
}

/*************************************
 *
 *  AGA MDA/CGA read/write handlers
 *
 *************************************/

static READ8_HANDLER ( pc_aga_mda_r )
{
	UINT8 data = 0xFF;

	if ( aga.mode == AGA_MONO ) {
		mc6845_device *mc6845 = space.machine().device<mc6845_device>(AGA_MC6845_NAME);
		switch( offset )
		{
		case 0: case 2: case 4: case 6:
			/* return last written mc6845 address value here? */
			break;
		case 1: case 3: case 5: case 7:
			data = mc6845->register_r(space, offset);
			break;
		case 10:
			data = (space.machine().root_device().ioport("IN0")->read() & 0x80 ) | 0x08 | aga.mda_status;
			aga.mda_status ^= 0x01;
			break;
		/* 12, 13, 14  are the LPT1 ports */
		}
	}
	return data;
}

static WRITE8_HANDLER ( pc_aga_mda_w )
{
	if ( aga.mode == AGA_MONO ) {
		mc6845_device *mc6845 = space.machine().device<mc6845_device>(AGA_MC6845_NAME);
		switch( offset )
		{
			case 0: case 2: case 4: case 6:
				mc6845->address_w( space, offset, data );
				break;
			case 1: case 3: case 5: case 7:
				mc6845->register_w( space, offset, data );
				break;
			case 8:
				aga.mda_mode_control = data;

				switch( aga.mda_mode_control & 0x2a ) {
				case 0x08:
					aga.update_row = mda_text_inten_update_row;
					break;
				case 0x28:
					aga.update_row = mda_text_blink_update_row;
					break;
				default:
					aga.update_row = NULL;
				}
				break;
		}
	}
}


static READ8_HANDLER ( pc_aga_cga_r )
{
	UINT8 data = 0xFF;

	if ( aga.mode == AGA_COLOR ) {
		mc6845_device *mc6845 = space.machine().device<mc6845_device>(AGA_MC6845_NAME);
		switch( offset ) {
		case 0: case 2: case 4: case 6:
			/* return last written mc6845 address value here? */
			break;
		case 1: case 3: case 5: case 7:
			data = mc6845->register_r( space, offset);
			break;
		case 10:
			data = aga.vsync | ( ( data & 0x40 ) >> 4 ) | aga.hsync;
			break;
		}
	}
	return data;
}


static void pc_aga_set_palette_luts(void) {
	/* Setup 2bpp palette lookup table */
	if ( aga.cga_mode_control & 0x10 ) {
		aga.cga_palette_lut_2bpp[0] = 0;
	} else {
		aga.cga_palette_lut_2bpp[0] = aga.cga_color_select & 0x0F;
	}
	if ( aga.cga_mode_control & 0x04 ) {
		aga.cga_palette_lut_2bpp[1] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 3;
		aga.cga_palette_lut_2bpp[2] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 4;
		aga.cga_palette_lut_2bpp[3] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 7;
	} else {
		if ( aga.cga_color_select & 0x20 ) {
			aga.cga_palette_lut_2bpp[1] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 3;
			aga.cga_palette_lut_2bpp[2] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 5;
			aga.cga_palette_lut_2bpp[3] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 7;
		} else {
			aga.cga_palette_lut_2bpp[1] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 2;
			aga.cga_palette_lut_2bpp[2] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 4;
			aga.cga_palette_lut_2bpp[3] = ( ( aga.cga_color_select & 0x10 ) >> 1 ) | 6;
		}
	}
	//logerror("2bpp lut set to %d,%d,%d,%d\n", aga.cga_palette_lut_2bpp[0], aga.cga_palette_lut_2bpp[1], aga.cga_palette_lut_2bpp[2], aga.cga_palette_lut_2bpp[3]);
}


static WRITE8_HANDLER ( pc_aga_cga_w )
{
	if ( aga.mode == AGA_COLOR ) {
		mc6845_device *mc6845 = space.machine().device<mc6845_device>(AGA_MC6845_NAME);

		switch(offset) {
		case 0: case 2: case 4: case 6:
			mc6845->address_w( space, offset, data );
			break;
		case 1: case 3: case 5: case 7:
			mc6845->register_w( space, offset, data );
			break;
		case 8:
			aga.cga_mode_control = data;

			//logerror("mode set to %02X\n", aga.cga_mode_control & 0x3F );
			switch ( aga.cga_mode_control & 0x3F ) {
			case 0x08: case 0x09: case 0x0C: case 0x0D:
				mc6845->set_hpixels_per_column( 8 );
				aga.update_row = cga_text_inten_update_row;
				break;
			case 0x0A: case 0x0B: case 0x2A: case 0x2B:
				mc6845->set_hpixels_per_column( 8 );
				if ( CGA_MONITOR == CGA_MONITOR_COMPOSITE ) {
					aga.update_row = cga_gfx_4bppl_update_row;
				} else {
					aga.update_row = cga_gfx_2bpp_update_row;
				}
				break;
			case 0x0E: case 0x0F: case 0x2E: case 0x2F:
				mc6845->set_hpixels_per_column( 8 );
				aga.update_row = cga_gfx_2bpp_update_row;
				break;
			case 0x18: case 0x19: case 0x1C: case 0x1D:
				mc6845->set_hpixels_per_column( 8 );
				aga.update_row = cga_text_inten_alt_update_row;
				break;
			case 0x1A: case 0x1B: case 0x3A: case 0x3B:
				mc6845->set_hpixels_per_column( 8 );
				if ( CGA_MONITOR == CGA_MONITOR_COMPOSITE ) {
					aga.update_row = cga_gfx_4bpph_update_row;
				} else {
					aga.update_row = cga_gfx_1bpp_update_row;
				}
				break;
			case 0x1E: case 0x1F: case 0x3E: case 0x3F:
				mc6845->set_hpixels_per_column( 16 );
				aga.update_row = cga_gfx_1bpp_update_row;
				break;
			case 0x28: case 0x29: case 0x2C: case 0x2D:
				mc6845->set_hpixels_per_column( 8 );
				aga.update_row = cga_text_blink_update_row;
				break;
			case 0x38: case 0x39: case 0x3C: case 0x3D:
				mc6845->set_hpixels_per_column( 8 );
				aga.update_row = cga_text_blink_alt_update_row;
				break;
			default:
				aga.update_row = NULL;
				break;
			}

			pc_aga_set_palette_luts();
			break;
		case 9:
			aga.cga_color_select = data;
			pc_aga_set_palette_luts();
			break;
		}
	}
}

/*************************************/

void pc_aga_set_mode(running_machine &machine, AGA_MODE mode)
{
	mc6845_device *mc6845 = machine.device<mc6845_device>(AGA_MC6845_NAME);

	aga.mode = mode;

	switch (aga.mode) {
	case AGA_COLOR:
		mc6845->set_clock( XTAL_14_31818MHz/8 );
		break;
	case AGA_MONO:
		mc6845->set_clock( 16257000/9 );
		break;
	case AGA_OFF:
		break;
	}
}


VIDEO_START( pc_aga )
{
	address_space &space = machine.firstcpu->space(AS_PROGRAM);
	address_space &spaceio = machine.firstcpu->space(AS_IO);
	int buswidth = machine.firstcpu->space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			space.install_legacy_readwrite_handler(0xb0000, 0xbffff, FUNC(pc200_videoram_r), FUNC(pc200_videoram_w) );
			spaceio.install_legacy_readwrite_handler(0x3b0, 0x3bf, FUNC(pc_aga_mda_r), FUNC(pc_aga_mda_w) );
			spaceio.install_legacy_readwrite_handler(0x3d0, 0x3df, FUNC(pc_aga_cga_r), FUNC(pc_aga_cga_w) );
			break;

		case 16:
			space.install_legacy_readwrite_handler(0xb0000, 0xbffff, FUNC(pc200_videoram_r), FUNC(pc200_videoram_w), 0xffff );
			spaceio.install_legacy_readwrite_handler(0x3b0, 0x3bf, FUNC(pc_aga_mda_r), FUNC(pc_aga_mda_w), 0xffff );
			spaceio.install_legacy_readwrite_handler(0x3d0, 0x3df, FUNC(pc_aga_cga_r), FUNC(pc_aga_cga_w), 0xffff );
			break;

		default:
			fatalerror("AGA:  Bus width %d not supported\n", buswidth);
			break;
	}

	memset( &aga, 0, sizeof( aga ) );
	aga.mode = AGA_COLOR;
	aga.mda_chr_gen = machine.root_device().memregion("gfx1")->base() + 0x1000;
	aga.cga_chr_gen = machine.root_device().memregion("gfx1")->base();
	aga.videoram = auto_alloc_array(machine, UINT8, 0x10000);
}

VIDEO_START( pc200 )
{
	address_space &space = machine.firstcpu->space(AS_PROGRAM);
	address_space &spaceio = machine.firstcpu->space(AS_IO);
	int buswidth = machine.firstcpu->space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			space.install_legacy_readwrite_handler(0xb0000, 0xbffff, FUNC(pc_aga_videoram_r), FUNC(pc_aga_videoram_w) );
			spaceio.install_legacy_readwrite_handler(0x3b0, 0x3bf, FUNC(pc_aga_mda_r), FUNC(pc_aga_mda_w) );
			spaceio.install_legacy_readwrite_handler(0x3d0, 0x3df, FUNC(pc200_cga_r),  FUNC(pc200_cga_w) );
			break;

		case 16:
			space.install_legacy_readwrite_handler(0xb0000, 0xbffff, FUNC(pc_aga_videoram_r), FUNC(pc_aga_videoram_w), 0xffff );
			spaceio.install_legacy_readwrite_handler(0x3b0, 0x3bf, FUNC(pc_aga_mda_r), FUNC(pc_aga_mda_w), 0xffff );
			spaceio.install_legacy_readwrite_handler(0x3d0, 0x3df, FUNC(pc200_cga_r),  FUNC(pc200_cga_w), 0xffff );
			break;

		default:
			fatalerror("AGA:  Bus width %d not supported\n", buswidth);
			break;
	}
	memset( &aga, 0, sizeof( aga ) );

	aga.mode = AGA_COLOR;
	aga.mda_chr_gen = machine.root_device().memregion("gfx1")->base();
	aga.cga_chr_gen = machine.root_device().memregion("gfx1")->base() + 0x1000;
	aga.videoram = auto_alloc_array(machine, UINT8, 0x10000);
}


WRITE8_HANDLER ( pc_aga_videoram_w )
{
	switch (aga.mode) {
	case AGA_COLOR:
		if (offset>=0x8000)
			aga.videoram[offset-0x8000]=data;
		break;
	case AGA_MONO:
		aga.videoram[offset]=data;
		break;
	case AGA_OFF: break;
	}
}

	READ8_HANDLER( pc_aga_videoram_r )
{
	UINT8 *videoram = aga.videoram;
	switch (aga.mode) {
	case AGA_COLOR:
		if (offset>=0x8000) return videoram[offset-0x8000];
		return 0;
	case AGA_MONO:
		return videoram[offset];
	case AGA_OFF: break;
	}
	return 0;
}

READ8_HANDLER( pc200_videoram_r )
{
	UINT8 *videoram = aga.videoram;
	switch (aga.mode)
	{
		default:
			if (offset>=0x8000) return videoram[offset-0x8000];
			return 0;
		case AGA_MONO:
			return videoram[offset];
	}
	return 0;
}

WRITE8_HANDLER ( pc200_videoram_w )
{
	switch (aga.mode)
	{
		default:
			if (offset>=0x8000)
				aga.videoram[offset-0x8000]=data;
			break;
		case AGA_MONO:
			aga.videoram[offset]=data;
			break;
	}
}

static struct {
	UINT8 port8, portd, porte;
} pc200= { 0 };

// in reality it is of course only 1 graphics adapter,
// but now cga and mda are splitted in mess
WRITE8_HANDLER( pc200_cga_w )
{
	pc_aga_cga_w(space, offset,data,mem_mask);
	switch(offset) {
	case 4:
		pc200.portd |= 0x20;
		break;
	case 8:
		pc200.port8 = data;
		pc200.portd |= 0x80;
		break;
	case 0xe:
		pc200.portd = 0x1f;
		if (data & 0x80)
			pc200.portd |= 0x40;

/* The bottom 3 bits of this port are:
 * Bit 2: Disable AGA
 * Bit 1: Select MDA
 * Bit 0: Select external display (monitor) rather than internal display
 *       (TV for PC200; LCD for PPC512) */
		if ((pc200.porte & 7) != (data & 7))
		{
			if (data & 4)
				pc_aga_set_mode(space.machine(), AGA_OFF);
			else if (data & 2)
				pc_aga_set_mode(space.machine(), AGA_MONO);
			else
				pc_aga_set_mode(space.machine(), AGA_COLOR);
		}
		pc200.porte = data;
		break;

	default:
		break;
	}
}

READ8_HANDLER ( pc200_cga_r )
{
	UINT8 result = 0xff;

	switch(offset) {
	case 8:
		result = pc200.port8;
		break;

	case 0xd:
		// after writing 0x80 to 0x3de, bits 7..5 of 0x3dd from the 2nd read must be 0
		result=pc200.portd;
		pc200.portd&=0x1f;
		break;

	case 0xe:
		// 0x20 low cga
		// 0x10 low special
		result = space.machine().root_device().ioport("DSW0")->read() & 0x38;
		break;

	default:
		result = pc_aga_cga_r(space, offset, mem_mask);
		break;
	}
	return result;
}
