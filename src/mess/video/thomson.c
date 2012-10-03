/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#include <math.h>
#include "includes/thomson.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/* One GPL is what is drawn in 1 us by the video system in the active window.
   Most of the time, it corresponds to a 8-pixel wide horizontal span.
   For some TO8/9/9+/MO6 modes, it can be 4-pixel or 16-pixel wide.
   There are always 40 GPLs in an active row, and it is always defined by
   two bytes in video memory (0x2000 bytes appart).
*/

#define THOM_GPL_PER_LINE 40


/****************** dynamic screen size *****************/

/* We allow choosing dynamically:
   - the border size
   - whether we use 640 pixels or 320 pixels in an active row
   (now this is automatically choosen by default for each frame)
*/



static UINT16 thom_bwidth;
static UINT16 thom_bheight;
/* border size */


static UINT8  thom_hires;
/* 0 = low res: 320x200 active area (faster)
   1 = hi res:  640x200 active area (can represent all video modes)
*/


static UINT8 thom_hires_better;
/* 1 = a 640 mode was used in the last frame */



static int thom_update_screen_size( running_machine &machine )
{
	screen_device *screen = machine.first_screen();
	const rectangle &visarea = screen->visible_area();
	UINT8 p = machine.root_device().ioport("vconfig")->read();
	int new_w, new_h, changed = 0;

	switch ( p & 3 )
	{
	case 0:  thom_bwidth = 56; thom_bheight = 47; break; /* as in original (?) */
	case 1:  thom_bwidth = 16; thom_bheight = 16; break; /* small */
	default: thom_bwidth =  0; thom_bheight =  0; break; /* none */
	}

	switch ( p & 0xc )
	{
	case 0:  thom_hires = 0; break;                 /* low */
	case 4:  thom_hires = 1; break;                 /* high */
	default: thom_hires = thom_hires_better; break; /* auto */
	}

	new_w = ( 320 + thom_bwidth * 2 ) * ( thom_hires + 1 ) - 1;
	new_h = ( 200 + thom_bheight * 2 ) /** (thom_hires + 1 )*/ - 1;
	if ( ( visarea.max_x != new_w ) || ( visarea.max_y != new_h ) )
	{
		changed = 1;
		machine.primary_screen->set_visible_area(0, new_w, 0, new_h );
	}

	return changed;
}



/*********************** video timing ******************************/

/* we use our own video timing to precisely cope with VBLANK and HBLANK */



static emu_timer* thom_video_timer; /* time elapsed from beginning of frame */



/* elapsed time from beginning of frame, in us */
INLINE unsigned thom_video_elapsed ( running_machine &machine )
{
	unsigned u;
	attotime elapsed = thom_video_timer ->elapsed( );
	u = (elapsed * 1000000 ).seconds;
	if ( u >= 19968 )
		u = 19968;
	return u;
}



struct thom_vsignal thom_get_vsignal ( running_machine &machine )
{
	struct thom_vsignal v;
	int gpl = thom_video_elapsed( machine ) - 64 * THOM_BORDER_HEIGHT - 7;
	if ( gpl < 0 )
		gpl += 19968;

	v.inil = ( gpl & 63 ) <= 40;

	v.init = gpl < (64 * THOM_ACTIVE_HEIGHT - 24);

	v.lt3 = ( gpl & 8 ) ? 1 : 0;

	v.line = gpl >> 6;

	v.count = v.line * 320 + ( gpl & 63 ) * 8;

	return v;
}



/************************** lightpen *******************************/



static void thom_get_lightpen_pos( running_machine &machine, int*x, int* y )
{
	*x = machine.root_device().ioport("lightpen_x")->read();
	*y = machine.root_device().ioport("lightpen_y")->read();

	if ( *x < 0 )
		*x = 0;

	if ( *y < 0 )
		*y = 0;

	if ( *x > 2 * thom_bwidth  + 319 )
		*x = 2 * thom_bwidth  + 319;

	if ( *y > 2 * thom_bheight + 199 )
		*y = 2 * thom_bheight + 199;
}



struct thom_vsignal thom_get_lightpen_vsignal ( running_machine &machine, int xdec, int ydec, int xdec2 )
{
	struct thom_vsignal v;
	int x, y;
	int gpl;

	thom_get_lightpen_pos( machine, &x, &y );
	x += xdec - thom_bwidth;
	y += ydec - thom_bheight;

	gpl = (x >> 3) + y * 64;
	if ( gpl < 0 )
		gpl += 19968;

	v.inil = (gpl & 63) <= 41;

	v.init = (gpl <= 64 * THOM_ACTIVE_HEIGHT - 24);

	v.lt3 = ( gpl & 8 ) ? 1 : 0;

	v.line = y;

	if ( v.inil && v.init )
		v.count =
			( gpl >> 6 ) * 320 +  /* line */
			( gpl & 63 ) *   8 +  /* gpl inside line */
			( (x + xdec2) & 7 );  /* pixel inside gpl */
	else
		v.count = 0;

	return v;
}



/* number of lightpen call-backs per frame */
static int thom_lightpen_nb;


/* called thom_lightpen_nb times */
static emu_timer *thom_lightpen_timer;


/* lightpen callback function to call from timer */
static void (*thom_lightpen_cb) ( running_machine &machine, int );



void thom_set_lightpen_callback ( running_machine &machine, int nb, void (*cb) ( running_machine &machine, int step ) )
{
	LOG (( "%f thom_set_lightpen_callback called\n", machine.time().as_double()));
	thom_lightpen_nb = nb;
	thom_lightpen_cb = cb;
}

static TIMER_CALLBACK( thom_lightpen_step )
{
	int step = param;

	if ( thom_lightpen_cb )
		thom_lightpen_cb( machine, step );

	if ( step < thom_lightpen_nb )
		thom_lightpen_timer->adjust(attotime::from_usec( 64 ), step + 1);
}



/***************** scan-line oriented engine ***********************/

/* This code, common for all Thomson machines, emulates the TO8
   video hardware, with its 16-colors chosen among 4096, 9 video modes,
   and 4 video pages. Moreover, it emulates changing the palette several times
   per frame to simulate more than 16 colors per frame (and the same for mode
   and page switchs) and cooper effects (distinguishing the left and right
   border color of each row).

   TO7, TO7/70 and MO5 video hardware are much simpler (8 or 16 fixed colors,
   one mode and one video page). Although the three are different, they can all
   be emulated by the TO8 video hardware.
   Thus, we use the same TO8-emulation code to deal with these simpler
   hardware (although it is somewhat of an overkill).
*/



/* ---------------- state & state changes ---------------- */



UINT8* thom_vram; /* pointer to video memory */

static emu_timer* thom_scanline_timer; /* scan-line udpate */

static UINT16 thom_last_pal[16];   /* palette at last scanline start */
static UINT16 thom_pal[16];        /* current palette */
static UINT8  thom_pal_changed;    /* whether pal != old_pal */
static UINT8  thom_border_index;   /* current border color index */

/* the left and right border color for each row (including top and bottom
   border rows); -1 means unchanged wrt last scanline
*/
static  INT16 thom_border_l[THOM_TOTAL_HEIGHT+1];
static  INT16 thom_border_r[THOM_TOTAL_HEIGHT+1];


/* active area, updated one scan-line at a time every 64us,
   then blitted in SCREEN_UPDATE_IND16
*/
static UINT16 thom_vbody[640*200];

static UINT8 thom_vmode; /* current vide mode */
static UINT8 thom_vpage; /* current video page */

/* this stores the video mode & page at each GPL in the current line
   (-1 means unchanged)
*/
static INT16 thom_vmodepage[41];
static UINT8 thom_vmodepage_changed;

/* one dirty flag for each video memory line */
static UINT8 thom_vmem_dirty[205];

/* set to 1 if undirty scanlines need to be redrawn due to other video state
   changes */
static UINT8 thom_vstate_dirty;
static UINT8 thom_vstate_last_dirty;



/* returns 1 if the mode is 640 pixel wide, 0 if it is 160 or 320 */
static int thom_mode_is_hires( int mode )
{
	return ( mode == THOM_VMODE_80 ) || ( mode == THOM_VMODE_80_TO9 );
}



/* either the border index or its palette entry has changed */
static void thom_border_changed( running_machine &machine )
{
	unsigned l = thom_video_elapsed( machine );
	unsigned y = l >> 6;
	unsigned x = l & 63;
	unsigned color =  thom_pal[ thom_border_index ];

	if ( y >= THOM_TOTAL_HEIGHT )
	{
		/* end of page */
		thom_border_r[ THOM_TOTAL_HEIGHT ] = color;
	}
	else if ( ! x )
	{
		/* start of line */
		thom_border_l[ y ] = color;
		thom_border_r[ y ] = color;
	}
	else if ( x <= 19 )
	{
		/* between left and right border */
		/* NOTE: this makes the lower right part of the color picker blink
           in the TO8/TO9/TO9+, which actually happens on the real computer!
        */
		thom_border_r[ y ] = color;
		thom_border_l[ y + 1 ] = color;
	}
	else
	{
		/* end of line */
		thom_border_l[ y + 1 ] = color;
		thom_border_r[ y + 1 ] = color;
	}
	thom_vstate_dirty = 1;
}



/* the video mode or page has changed */
static void thom_gplinfo_changed( running_machine &machine )
{
	unsigned l = thom_video_elapsed( machine ) - THOM_BORDER_HEIGHT * 64 - 7;
	unsigned y = l >> 6;
	unsigned x = l & 63;
	int modepage = ((int)thom_vmode << 8) | thom_vpage;
	if ( y >= 200 || x>= 40 )
		thom_vmodepage[ 40 ] = modepage;
	else
		thom_vmodepage[ x ] = modepage;
	thom_vmodepage_changed = 1;
}



void thom_set_border_color ( running_machine &machine, unsigned index )
{
	assert( index < 16 );
	if ( index != thom_border_index )
	{
		LOG (( "thom_set_border_color: %i at line %i col %i\n", index, thom_video_elapsed( machine ) / 64, thom_video_elapsed( machine ) % 64  ));
		thom_border_index = index;
		thom_border_changed( machine );
	}
}



void thom_set_palette ( running_machine &machine, unsigned index, UINT16 color )
{
	assert( index < 16 );

	if ( color != 0x1000 )
		color &= 0xfff;

	if ( thom_pal[ index ] == color )
		return;

	LOG (( "thom_set_palette: %i to %03x at line %i col %i\n", index, color, thom_video_elapsed( machine ) / 64, thom_video_elapsed( machine ) % 64  ));

	thom_pal[ index ] = color;
	if ( index == thom_border_index )
		thom_border_changed( machine );
	thom_pal_changed = 1;
	thom_vstate_dirty = 1;
}



void thom_set_video_mode ( running_machine &machine, unsigned mode )
{
	assert( mode < THOM_VMODE_NB );

	if ( mode != thom_vmode )
	{
		LOG (( "thom_set_video_mode: %i at line %i, col %i\n", mode, thom_video_elapsed( machine ) / 64, thom_video_elapsed( machine ) % 64 ));
		thom_vmode = mode;
		thom_gplinfo_changed( machine );
		thom_vstate_dirty = 1;
		thom_hires_better |= thom_mode_is_hires( mode );
	}
}



void thom_set_video_page ( running_machine &machine, unsigned page )
{
	assert( page < THOM_NB_PAGES )
		;
	if ( page != thom_vpage ) {
		LOG (( "thom_set_video_page: %i at line %i col %i\n", page, thom_video_elapsed( machine ) / 64, thom_video_elapsed( machine ) % 64  ));
		thom_vpage = page;
		thom_gplinfo_changed( machine );
		thom_vstate_dirty = 1;
	}
}



/* -------------- drawing --------------- */



typedef void ( *thom_scandraw ) ( running_machine &machine, UINT8* vram, UINT16* dst, UINT16* pal,
				  int org, int len );



#define UPDATE( name, res )						\
	static void name##_scandraw_##res ( running_machine &machine,	\
					    UINT8* vram, UINT16* dst,	UINT16* pal, \
					    int org, int len )		\
	{								\
		unsigned gpl;						\
		vram += org;						\
		dst += org * res;					\
		for ( gpl = 0; gpl < len; gpl++, dst += res, vram++ ) {	\
			UINT8 rama = vram[ 0      ];			\
			UINT8 ramb = vram[ 0x2000 ];

#define END_UPDATE							\
		}							\
	}

#define UPDATE_HI( name )  UPDATE( name, 16 )
#define UPDATE_LOW( name ) UPDATE( name,  8 )



/* 320x200, 16-colors, constraints: 2 distinct colors per GPL (8 pixels) */
/* this also works for TO7, assuming the 2 top bits of each color byte are 1 */

UPDATE_HI( to770 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ ((ramb & 7) | ((ramb>>4) & 8)) ^ 8 ];
	c[1] = pal[ ((ramb >> 3) & 15) ^ 8 ];
	for ( i = 0; i < 16; i += 2, rama >>= 1 )
		dst[ 15 - i ] = dst[ 14 - i ] = c[ rama & 1 ];
}
END_UPDATE

UPDATE_LOW( to770 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ ((ramb & 7) | ((ramb>>4) & 8)) ^ 8 ];
	c[1] = pal[ ((ramb >> 3) & 15) ^ 8 ];
	for ( i = 0; i < 8; i++, rama >>= 1 )
		dst[ 7 - i ] = c[ rama & 1 ];
}
END_UPDATE



/* as above, different (more logical but TO7-incompatible) color encoding */

UPDATE_HI( mo5 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ ramb & 15 ];
	c[1] = pal[ ramb >> 4 ];
	for ( i = 0; i < 16; i += 2, rama >>= 1 )
		dst[ 15 - i ] = dst[ 14 - i ] = c[ rama & 1 ];
}
END_UPDATE

UPDATE_LOW( mo5 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ ramb & 15 ];
	c[1] = pal[ ramb >> 4 ];
	for ( i = 0; i < 8; i++, rama >>= 1 )
		dst[ 7 - i ] = c[ rama & 1 ];
}
END_UPDATE



/* as to770, but with pastel color bit unswitched */

UPDATE_HI( to9 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ (ramb & 7) | ((ramb>>4) & 8) ];
	c[1] = pal[ (ramb >> 3) & 15 ];
	for ( i = 0; i < 16; i += 2, rama >>= 1 )
		dst[ 15 - i ] = dst[ 14 - i ] = c[ rama & 1 ];
}
END_UPDATE

UPDATE_LOW( to9 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ (ramb & 7) | ((ramb>>4) & 8) ];
	c[1] = pal[ (ramb >> 3) & 15 ];
	for ( i = 0; i < 8; i++, rama >>= 1 )
		dst[ 7 - i ] = c[ rama & 1 ];
}
END_UPDATE



/* 320x200, 4-colors, no constraint */

UPDATE_HI( bitmap4 )
{
	int i;
	pen_t c[2][2];
	c[0][0] = pal[ 0 ];
	c[0][1] = pal[ 1 ];
	c[1][0] = pal[ 2 ];
	c[1][1] = pal[ 3 ];
	for ( i = 0; i < 16; i += 2, rama >>= 1, ramb >>= 1 )
		dst[ 15 - i ] =  dst[ 14 - i ] = c[ rama & 1 ] [ ramb & 1 ];
}
END_UPDATE

UPDATE_LOW( bitmap4 )
{
	int i;
	pen_t c[2][2];
	c[0][0] = pal[ 0 ];
	c[0][1] = pal[ 1 ];
	c[1][0] = pal[ 2 ];
	c[1][1] = pal[ 3 ];
	for ( i = 0; i < 8; i++, rama >>= 1, ramb >>= 1 )
		dst[ 7 - i ] = c[ rama & 1 ] [ ramb & 1 ];
}
END_UPDATE



/* as above, but using 2-bit pixels instead of 2 planes of 1-bit pixels  */

UPDATE_HI( bitmap4alt )
{
	int i;
	pen_t c[4];
	c[0] = pal[ 0 ];
	c[1] = pal[ 1 ];
	c[2] = pal[ 2 ];
	c[3] = pal[ 3 ];
	for ( i = 0; i < 8; i += 2, ramb >>= 2 )
		dst[ 15 - i ] = dst[ 14 - i ] = c[ ramb & 3 ];
	for ( i = 0; i < 8; i += 2, rama >>= 2 )
		dst[ 7 - i ] = dst[ 6 - i ] = c[ rama & 3 ];
}
END_UPDATE

UPDATE_LOW( bitmap4alt )
{
	int i;
	pen_t c[4];
	c[0] = pal[ 0 ];
	c[1] = pal[ 1 ];
	c[2] = pal[ 2 ];
	c[3] = pal[ 3 ];
	for ( i = 0; i < 4; i++, ramb >>= 2 )
		dst[ 7 - i ] = c[ ramb & 3 ];
	for ( i = 0; i < 4; i++, rama >>= 2 )
		dst[ 3 - i ] = c[ rama & 3 ];
}
END_UPDATE



/* 160x200, 16-colors, no constraint */

UPDATE_HI( bitmap16 )
{
	dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = pal[ rama >> 4 ];
	dst[ 4] = dst[ 5] = dst[ 6] = dst[ 7] = pal[ rama & 15 ];
	dst[ 8] = dst[ 9] = dst[10] = dst[11] = pal[ ramb >> 4 ];
	dst[12] = dst[13] = dst[14] = dst[15] = pal[ ramb & 15 ];
}
END_UPDATE

UPDATE_LOW( bitmap16 )
{
	dst[0] = dst[1] = pal[ rama >> 4 ];
	dst[2] = dst[3] = pal[ rama & 15 ];
	dst[4] = dst[5] = pal[ ramb >> 4 ];
	dst[6] = dst[7] = pal[ ramb & 15 ];
}
END_UPDATE



/* 640x200 (80 text column), 2-colors, no constraint */

UPDATE_HI( mode80 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ 0 ];
	c[1] = pal[ 1 ];
	for ( i = 0; i < 8; i++, ramb >>= 1 )
		dst[ 15 - i ] = c[ ramb & 1 ];
	for ( i = 0; i < 8; i++, rama >>= 1 )
		dst[  7 - i ] = c[ rama & 1 ];
}
END_UPDATE

UPDATE_LOW( mode80 )
{
	/* 640-pixel mode but 320 pixels emulated => we merge pixels */
	int i;
	pen_t c[4];
	c[0] = pal[ 0 ];
	c[1] = c[2] = c[3] = pal[ 1 ];
	for ( i = 0; i < 4; i++, ramb >>= 2 )
		dst[ 7 - i ] = c[ ramb & 3 ];
	for ( i = 0; i < 4; i++, rama >>= 2 )
		dst[ 3 - i ] = c[ rama & 3 ];
}
END_UPDATE



/* as above, but TO9 flavor */

UPDATE_HI( mode80_to9 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ 0 ];
	c[1] = pal[ 6 ];
	for ( i = 0; i < 8; i++, ramb >>= 1 )
		dst[ 15 - i ] = c[ ramb & 1 ];
	for ( i = 0; i < 8; i++, rama >>= 1 )
		dst[  7 - i ] = c[ rama & 1 ];
}
END_UPDATE

UPDATE_LOW( mode80_to9 )
{
	int i;
	pen_t c[4];
	c[0] = pal[ 0 ];
	c[1] = c[2] = c[3] = pal[ 6 ];
	for ( i = 0; i < 4; i++, ramb >>= 2 )
		dst[ 7 - i ] = c[ ramb & 3 ];
	for ( i = 0; i < 4; i++, rama >>= 2 )
		dst[ 3 - i ] = c[ rama & 3 ];
}
END_UPDATE



/* 320x200, 2-colors, two pages (untested) */

UPDATE_HI( page1 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ 0 ];
	c[1] = pal[ 1 ];
	for ( i = 0; i < 16; i += 2, rama >>= 1 )
		dst[ 15 - i ] = dst[ 14 - i ] = c[ rama & 1 ];
	(void)ramb;
}
END_UPDATE

UPDATE_LOW( page1 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ 0 ];
	c[1] = pal[ 1 ];
	for ( i = 0; i < 8; i++, rama >>= 1 )
		dst[ 7 - i ] = c[ rama & 1 ];
	(void)ramb;
}
END_UPDATE

UPDATE_HI( page2 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ 0 ];
	c[1] = pal[ 2 ];
	for ( i = 0; i < 16; i += 2, ramb >>= 1 )
		dst[ 15 - i ] = dst[ 14 - i ] = c[ ramb & 1 ];
	(void)rama;
}
END_UPDATE

UPDATE_LOW( page2 )
{
	int i;
	pen_t c[2];
	c[0] = pal[ 0 ];
	c[1] = pal[ 2 ];
	for ( i = 0; i < 8; i++, ramb >>= 1 )
		dst[ 7 - i ] = c[ ramb & 1 ];
	(void)rama;
}
END_UPDATE



/* 320x200, 2-colors, two overlaid pages (untested) */

UPDATE_HI( overlay )
{
	int i;
	pen_t c[2][2];
	c[0][0] = pal[ 0 ];
	c[0][1] = c[1][1] = pal[ 1 ];
	c[1][0] = pal[ 2 ];
	for ( i = 0; i < 16; i += 2, rama >>= 1, ramb >>= 1 )
		dst[ 15 - i ] =  dst[ 14 - i ] = c[ ramb & 1 ] [ rama & 1 ];
}
END_UPDATE

UPDATE_LOW( overlay )
{
	int i;
	pen_t c[2][2];
	c[0][0] = pal[ 0 ];
	c[0][1] = c[1][1] = pal[ 1 ];
	c[1][0] = pal[ 2 ];
	for ( i = 0; i < 8; i++, rama >>= 1, ramb >>= 1 )
		dst[ 7 - i ] = c[ ramb & 1 ] [ rama & 1 ];
}
END_UPDATE



/* 160x200, 4-colors, four overlaid pages (untested) */

UPDATE_HI( overlay3 )
{
	static const int p[2][2][2][2] = {
		  { { { 0, 1 }, { 2, 1 }, }, { { 4, 1 }, { 2, 1 } } },
		  { { { 8, 1 }, { 2, 1 }, }, { { 4, 1 }, { 2, 1 } } }
	};
	int i;
	for ( i = 0; i < 16; i += 4, rama >>= 1, ramb >>= 1 )
		dst[ 15 - i ] = dst[ 14 - i ] = dst[ 13 - i ] = dst[ 12 - i ] =
			pal[ p[ ramb & 1 ] [ (ramb >> 4) & 1 ]
					    [ rama & 1 ] [ (rama >> 4) & 1 ] ];
}
END_UPDATE

UPDATE_LOW( overlay3 )
{
	static const int p[2][2][2][2] = {
		  { { { 0, 1 }, { 2, 1 }, }, { { 4, 1 }, { 2, 1 } } },
		  { { { 8, 1 }, { 2, 1 }, }, { { 4, 1 }, { 2, 1 } } }
	};
	int i;
	for ( i = 0; i < 8; i += 2, rama >>= 1, ramb >>= 1 )
		dst[ 7 - i ] = dst[ 6 - i ] =
			pal[ p[ ramb & 1 ] [ (ramb >> 4) & 1 ]
					    [ rama & 1 ] [ (rama >> 4) & 1 ] ];
}
END_UPDATE



#define FUN(x) { x##_scandraw_8, x##_scandraw_16 }


static const thom_scandraw thom_scandraw_funcs[THOM_VMODE_NB][2] =
{
	FUN(to770),    FUN(mo5),    FUN(bitmap4), FUN(bitmap4alt),  FUN(mode80),
	FUN(bitmap16), FUN(page1),  FUN(page2),   FUN(overlay),     FUN(overlay3),
	FUN(to9), FUN(mode80_to9),
};



/* called at the start of each scanline in the active area, just after
   left border (-1<=y<199), and also after the last scanline (y=199)
*/
static TIMER_CALLBACK( thom_scanline_start )
{
	int y = param;

	/* update active-area */
	if ( y >= 0 && (thom_vstate_dirty || thom_vstate_last_dirty || thom_vmem_dirty[y]) )
	{
		int x = 0;
		while ( x < 40 )
		{
			int xx = x;
			unsigned mode = thom_vmodepage[x] >> 8;
			unsigned page = thom_vmodepage[x] & 0xff;
			assert( mode < THOM_VMODE_NB );
			assert( page < 4 );
			if ( thom_vmodepage_changed )
                        {
                                do
				{
					xx++;
				}
                                while ( xx < 40 && thom_vmodepage[xx] == -1 );
                        }
			else
                        {
				xx = 40;
                        }
			thom_scandraw_funcs[ mode ][ thom_hires ]
				( machine,
				  thom_vram + y * 40 + page * 0x4000,
				  thom_vbody + y * 320 * (thom_hires+1),
				  thom_last_pal, x, xx-x );
			x = xx;
		}
		thom_vmem_dirty[y] = 0;
	}

	/* prepare for next scanline */
	if ( y == 199 )
		thom_scanline_timer->adjust(attotime::never);
	else
	{

		if ( thom_vmodepage_changed )
		{
			int x, m = 0;
			for ( x = 0; x <= 40; x++ )
			{
				if ( thom_vmodepage[x] !=-1 )
				{
					m = thom_vmodepage[x];
					thom_vmodepage[x] = -1;
				}
			}
			thom_vmodepage[0] = m;
			thom_vmodepage_changed = 0;
		}

		if ( thom_pal_changed )
		{
			memcpy( thom_last_pal, thom_pal, 32 );
			thom_pal_changed = 0;
		}

		thom_scanline_timer->adjust(attotime::from_usec(64), y + 1);
	}
}



/* -------------- misc --------------- */



static UINT32 thom_mode_point;

static UINT32 thom_floppy_wcount;
static UINT32 thom_floppy_rcount;

#define FLOP_STATE (thom_floppy_wcount ? 2 : thom_floppy_rcount ? 1 : 0)



void thom_set_mode_point ( running_machine &machine, int point )
{
	assert( point >= 0 && point <= 1 );
	thom_mode_point = ( ! point ) * 0x2000;
	machine.root_device().membank( THOM_VRAM_BANK )->set_entry( ! point );
}



void thom_floppy_active ( running_machine &machine, int write )
{
	int fold = FLOP_STATE, fnew;

	/* stays up for a few frames */
	if ( write )
		thom_floppy_wcount = 25;
	else
		thom_floppy_rcount = 25;

	/* update icon */
	fnew = FLOP_STATE;
	if ( fold != fnew )
		output_set_value( "floppy", fnew );
}



/* -------------- main update function --------------- */



SCREEN_UPDATE_IND16 ( thom )
{
	int y, ypos;
	const int scale = thom_hires ? 2 : 1;
	const int xbleft = thom_bwidth * scale;
	const int xbright = ( thom_bwidth + THOM_ACTIVE_WIDTH ) * scale;
	const int xright = ( thom_bwidth * 2 + THOM_ACTIVE_WIDTH ) * scale;
	const int xwidth = THOM_ACTIVE_WIDTH * scale;
	const int yup = THOM_BORDER_HEIGHT + THOM_ACTIVE_HEIGHT;
	const int ybot = THOM_BORDER_HEIGHT + thom_bheight + 200;
	UINT16* v = thom_vbody;
	pen_t border = 0;
	rectangle wrect(0, xright - 1, 0, 0);
	rectangle lrect(0, xbleft - 1, 0, 0);
	rectangle rrect(xbright, xright - 1, 0, 0);

	//LOG (( "%f thom: video update called\n", machine.time().as_double()));

	/* upper border */
	for ( y = 0; y < THOM_BORDER_HEIGHT - thom_bheight; y++ )
	{
		if ( thom_border_l[ y ] != -1 )
			border = thom_border_l[ y ];
	}
	ypos = 0;
	while ( y < THOM_BORDER_HEIGHT )
	{
		if ( thom_border_l[ y ] != -1 )
			border = thom_border_l[ y ];
		wrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < THOM_BORDER_HEIGHT && thom_border_l[ y ] == -1 );
		wrect.max_y = ypos - 1;
		bitmap.fill(border, wrect );
	}

	/* left border */
	while ( y < yup )
	{
		if ( thom_border_l[ y ] != -1 )
			border = thom_border_l[ y ];
		lrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < yup && thom_border_l[ y ] == -1 );
		lrect.max_y = ypos - 1;
		bitmap.fill(border, lrect );
	}

	/* lower border */
	while (y < ybot )
	{
		if ( thom_border_l[ y ] != -1 )
			border = thom_border_l[ y ];
		wrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		} while ( y < ybot && thom_border_l[ y ] == -1 );
		wrect.max_y = ypos - 1;
		bitmap.fill(border, wrect );
	}

	/* right border */
	for ( y = 0; y < THOM_BORDER_HEIGHT; y++ ) {
		if ( thom_border_r[ y ] != -1 )
			border = thom_border_r[ y ];
	}
	ypos = thom_bheight /* * scale */;
	while ( y < yup )
	{
		if ( thom_border_r[ y ] != -1 )
			border = thom_border_r[ y ];
		rrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < yup && thom_border_r[ y ] == -1 );
		rrect.max_y = ypos - 1;
		bitmap.fill(border, rrect );
	}

	/* active area */
	ypos = thom_bheight /* * scale */;
	for ( y = 0; y < 200; v += xwidth, y++ , ypos ++ /* += scale */ )
	{
		draw_scanline16( bitmap, xbleft, ypos, xwidth, v, NULL );
#if 0
		if ( thom_hires )
			draw_scanline16( bitmap, xbleft, ypos+1, xwidth, v, NULL );
#endif
	}

	return 0;
}



/* -------------- frame start ------------------ */



static emu_timer *thom_init_timer;

static void (*thom_init_cb) ( running_machine &machine, int init );



void thom_set_init_callback ( running_machine &machine, void (*cb) ( running_machine &machine, int init ) )
{
	thom_init_cb = cb;
}



static TIMER_CALLBACK( thom_set_init )
{
	int init = param;
	LOG (( "%f thom_set_init: %i, at line %i col %i\n", machine.time().as_double(), init, thom_video_elapsed( machine ) / 64, thom_video_elapsed( machine ) % 64 ));

	if ( thom_init_cb )
		thom_init_cb( machine, init );
	if ( ! init )
		thom_init_timer->adjust(attotime::from_usec( 64 * THOM_ACTIVE_HEIGHT - 24 ), 1-init);
}

/* call this at the very beginning of each new frame */
SCREEN_VBLANK ( thom )
{
	// rising edge
	if (vblank_on)
	{
		int fnew, fold = FLOP_STATE;
		int i;
		UINT16 b = 0;
		struct thom_vsignal l = thom_get_lightpen_vsignal( screen.machine(), 0, -1, 0 );

		LOG (( "%f thom: video eof called\n", screen.machine().time().as_double() ));

		/* floppy indicator count */
		if ( thom_floppy_wcount )
			thom_floppy_wcount--;
		if ( thom_floppy_rcount )
			thom_floppy_rcount--;
		fnew = FLOP_STATE;
		if ( fnew != fold )
			output_set_value( "floppy", fnew );

		/* prepare state for next frame */
		for ( i = 0; i <= THOM_TOTAL_HEIGHT; i++ )
		{
			if ( thom_border_l[ i ] != -1 )
				b = thom_border_l[ i ];
			if ( thom_border_r[ i ] != -1 )
				b = thom_border_r[ i ];
		}
		memset( thom_border_l, 0xff, sizeof( thom_border_l ) );
		memset( thom_border_r, 0xff, sizeof( thom_border_r ) );
		thom_border_l[ 0 ] = b;
		thom_border_r[ 0 ] = b;
		thom_vstate_last_dirty = thom_vstate_dirty;
		thom_vstate_dirty = 0;

		/* schedule first init signal */
		thom_init_timer->adjust(attotime::from_usec( 64 * THOM_BORDER_HEIGHT + 7 ));

		/* schedule first lightpen signal */
		l.line &= ~1; /* hack (avoid lock in MO6 palette selection) */
		thom_lightpen_timer->adjust(
				   attotime::from_usec( 64 * ( THOM_BORDER_HEIGHT + l.line - 2 ) + 16 ), 0);

		/* schedule first active-area scanline call-back */
		thom_scanline_timer->adjust(attotime::from_usec( 64 * THOM_BORDER_HEIGHT + 7), -1);

		/* reset video frame time */
		thom_video_timer->adjust(attotime::zero);

		/* update screen size according to user options */
		if ( thom_update_screen_size( screen.machine() ) )
			thom_vstate_dirty = 1;

		/* hi-res automatic */
		thom_hires_better = thom_mode_is_hires( thom_vmode );
	}
}



/* -------------- initialization --------------- */



static const UINT16 thom_pal_init[16] =
{
	0x1000, /* 0: black */        0x000f, /* 1: red */
	0x00f0, /* 2: geen */         0x00ff, /* 3: yellow */
	0x0f00, /* 4: blue */         0x0f0f, /* 5: purple */
	0x0ff0, /* 6: cyan */         0x0fff, /* 7: white */
	0x0777, /* 8: gray */         0x033a, /* 9: pink */
	0x03a3, /* a: light green */  0x03aa, /* b: light yellow */
	0x0a33, /* c: light blue */   0x0a3a, /* d: redish pink */
	0x0ee7, /* e: light cyan */   0x003b, /* f: orange */
};



VIDEO_START ( thom )
{
	LOG (( "thom: video start called\n" ));

	/* scan-line state */
	memcpy( thom_last_pal, thom_pal_init, 32 );
	memcpy( thom_pal, thom_pal_init, 32 );
	memset( thom_border_l, 0xff, sizeof( thom_border_l ) );
	memset( thom_border_r, 0xff, sizeof( thom_border_r ) );
	memset( thom_vbody, 0, sizeof( thom_vbody ) );
	memset( thom_vmodepage, 0xffff, sizeof( thom_vmodepage ) );
	memset( thom_vmem_dirty, 0, sizeof( thom_vmem_dirty ) );
	thom_border_l[ 0 ] = 0;
	thom_border_r[ 0 ] = 0;
	thom_vmodepage[ 0 ] = 0;
	thom_vmodepage_changed = 0;
	thom_vmode = 0;
	thom_vpage = 0;
	thom_border_index = 0;
	thom_vstate_dirty = 1;
	thom_vstate_last_dirty = 1;
	state_save_register_global_array(machine,  thom_last_pal );
	state_save_register_global_array(machine,  thom_pal );
	state_save_register_global_array(machine,  thom_border_l );
	state_save_register_global_array(machine,  thom_border_r );
	state_save_register_global_array(machine,  thom_vbody );
	state_save_register_global_array(machine,  thom_vmodepage );
	state_save_register_global_array(machine,  thom_vmem_dirty );
	state_save_register_global(machine,  thom_pal_changed );
	state_save_register_global(machine,  thom_vmodepage_changed );
	state_save_register_global(machine,  thom_vmode );
	state_save_register_global(machine,  thom_vpage );
	state_save_register_global(machine,  thom_border_index );
	state_save_register_global(machine,  thom_vstate_dirty );
	state_save_register_global(machine,  thom_vstate_last_dirty );

	thom_mode_point = 0;
	state_save_register_global(machine,  thom_mode_point );
	machine.root_device().membank( THOM_VRAM_BANK )->set_entry( 0 );

	thom_floppy_rcount = 0;
	thom_floppy_wcount = 0;
	state_save_register_global(machine,  thom_floppy_wcount );
	state_save_register_global(machine,  thom_floppy_rcount );
	output_set_value( "floppy", 0 );

	thom_video_timer = machine.scheduler().timer_alloc(FUNC_NULL);

	thom_scanline_timer = machine.scheduler().timer_alloc(FUNC(thom_scanline_start));

	thom_lightpen_nb = 0;
	thom_lightpen_cb = NULL;
	thom_lightpen_timer = machine.scheduler().timer_alloc(FUNC(thom_lightpen_step));
	state_save_register_global(machine,  thom_lightpen_nb );

	thom_init_cb = NULL;
	thom_init_timer = machine.scheduler().timer_alloc(FUNC(thom_set_init));

	state_save_register_global(machine,  thom_bwidth );
	state_save_register_global(machine,  thom_bheight );
	state_save_register_global(machine,  thom_hires );
	state_save_register_global(machine,  thom_hires_better );
}



PALETTE_INIT ( thom )
{
	double gamma = 0.6f;
	unsigned i;

	LOG (( "thom: palette init called\n" ));

	for ( i = 0; i < 4097; i++ )
	{
		UINT8 r = 255. * pow( (i & 15) / 15., gamma );
		UINT8 g = 255. * pow( ((i>> 4) & 15) / 15., gamma );
		UINT8 b = 255. * pow( ((i >> 8) & 15) / 15., gamma );
		/* UINT8 alpha = i & 0x1000 ? 0 : 255;  TODO: transparency */
		palette_set_color_rgb(machine,  i, r, g, b );
	}
}



/***************************** TO7 / T9000 *************************/



/* write to video memory through addresses 0x4000-0x5fff */
WRITE8_HANDLER ( to7_vram_w )
{
	assert( offset < 0x2000 );
	/* force two topmost color bits to 1 */
	if ( thom_mode_point )
		data |= 0xc0;
	if ( thom_vram[ offset + thom_mode_point ] == data )
		return;
	thom_vram[ offset + thom_mode_point ] = data;
	/* dirty whole scanline */
	thom_vmem_dirty[ offset / 40 ] = 1;
}



/* bits 0-13 : latched gpl of lightpen position */
/* bit    14:  latched INIT */
/* bit    15:  latched INIL */
unsigned to7_lightpen_gpl ( running_machine &machine, int decx, int decy )
{
	int x,y;
	thom_get_lightpen_pos( machine, &x, &y );
	x -= thom_bwidth;
	y -= thom_bheight;
	if ( x < 0 || y < 0 || x >= 320 || y >= 200 )
		return 0;
	x += decx;
	y += decy;
	return y*40 + x/8 + (x < 320 ? 0x4000 : 0) + 0x8000;
}



/************************** TO7/70 / MO5 ****************************/



/* write to video memory through addresses 0x4000-0x5fff (TO)
   or 0x0000-0x1fff (MO) */
WRITE8_HANDLER ( to770_vram_w )
{
	assert( offset < 0x2000 );
	if ( thom_vram[ offset + thom_mode_point ] == data )
		return;
	thom_vram[ offset + thom_mode_point ] = data;
	/* dirty whole scanline */
	thom_vmem_dirty[ offset / 40 ] = 1;
}




/***************************** TO8 ******************************/



/* write to video memory through system space (always page 1) */

WRITE8_HANDLER ( to8_sys_lo_w )
{
	UINT8* dst = thom_vram + offset + 0x6000;
	assert( offset < 0x2000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	thom_vmem_dirty[ offset / 40 ] = 1;
}



WRITE8_HANDLER ( to8_sys_hi_w )
{
	UINT8* dst = thom_vram + offset + 0x4000;
	assert( offset < 0x2000 );
	if ( *dst == data ) return;
	*dst = data;
	/* dirty whole scanline */
	thom_vmem_dirty[ offset / 40 ] = 1;
}



/* write to video memory through data space */

WRITE8_HANDLER ( to8_data_lo_w )
{
	UINT8* dst = thom_vram + offset + 0x4000 * to8_data_vpage + 0x2000;
	assert( offset < 0x2000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	if ( to8_data_vpage >= 4 )
		return;
	thom_vmem_dirty[ offset / 40 ] = 1;
}



WRITE8_HANDLER ( to8_data_hi_w )
{
	UINT8* dst = thom_vram + offset + 0x4000 * to8_data_vpage;
	assert( offset < 0x2000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	if ( to8_data_vpage >= 4 )
		return;
	thom_vmem_dirty[ offset / 40 ] = 1;
}



/* write to video memory page through cartridge addresses space */
WRITE8_HANDLER ( to8_vcart_w )
{
	UINT8* dst = thom_vram + offset + 0x4000 * to8_cart_vpage;
	assert( offset < 0x4000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	if ( to8_cart_vpage >= 4  )
		return;
	thom_vmem_dirty[ (offset & 0x1fff) / 40 ] = 1;
}

