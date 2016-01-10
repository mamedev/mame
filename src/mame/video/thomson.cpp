// license:BSD-3-Clause
// copyright-holders:Antoine Mine
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


int thomson_state::thom_update_screen_size()
{
	const rectangle &visarea = m_screen->visible_area();
	UINT8 p = m_io_vconfig->read();
	int new_w, new_h, changed = 0;

	switch ( p & 3 )
	{
	case 0:  m_thom_bwidth = 56; m_thom_bheight = 47; break; /* as in original (?) */
	case 1:  m_thom_bwidth = 16; m_thom_bheight = 16; break; /* small */
	default: m_thom_bwidth =  0; m_thom_bheight =  0; break; /* none */
	}

	switch ( p & 0xc )
	{
	case 0:  m_thom_hires = 0; break;                 /* low */
	case 4:  m_thom_hires = 1; break;                 /* high */
	default: m_thom_hires = m_thom_hires_better; break; /* auto */
	}

	new_w = ( 320 + m_thom_bwidth * 2 ) * ( m_thom_hires + 1 ) - 1;
	new_h = ( 200 + m_thom_bheight * 2 ) /** (m_thom_hires + 1 )*/ - 1;
	if ( ( visarea.max_x != new_w ) || ( visarea.max_y != new_h ) )
	{
		changed = 1;
		m_screen->set_visible_area(0, new_w, 0, new_h );
	}

	return changed;
}



/*********************** video timing ******************************/


/* elapsed time from beginning of frame, in us */
unsigned thomson_state::thom_video_elapsed()
{
	unsigned u;
	attotime elapsed = m_thom_video_timer->elapsed();
	u = (elapsed * 1000000 ).seconds();
	if ( u >= 19968 )
		u = 19968;
	return u;
}



struct thom_vsignal thomson_state::thom_get_vsignal()
{
	struct thom_vsignal v;
	int gpl = thom_video_elapsed() - 64 * THOM_BORDER_HEIGHT - 7;
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



void thomson_state::thom_get_lightpen_pos( int*x, int* y )
{
	*x = m_io_lightpen_x->read();
	*y = m_io_lightpen_y->read();

	if ( *x < 0 )
		*x = 0;

	if ( *y < 0 )
		*y = 0;

	if ( *x > 2 * m_thom_bwidth  + 319 )
		*x = 2 * m_thom_bwidth  + 319;

	if ( *y > 2 * m_thom_bheight + 199 )
		*y = 2 * m_thom_bheight + 199;
}



struct thom_vsignal thomson_state::thom_get_lightpen_vsignal( int xdec, int ydec, int xdec2 )
{
	struct thom_vsignal v;
	int x, y;
	int gpl;

	thom_get_lightpen_pos( &x, &y );
	x += xdec - m_thom_bwidth;
	y += ydec - m_thom_bheight;

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



void thomson_state::thom_set_lightpen_callback( int nb )
{
	LOG (( "%f thom_set_lightpen_callback called\n", machine().time().as_double()));
	m_thom_lightpen_nb = nb;
}

TIMER_CALLBACK_MEMBER( thomson_state::thom_lightpen_step )
{
	int step = param;

	if ( m_thom_lightpen_cb )
		(this->*m_thom_lightpen_cb)( step );

	if ( step < m_thom_lightpen_nb )
		m_thom_lightpen_timer->adjust(attotime::from_usec( 64 ), step + 1);
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


/* returns 1 if the mode is 640 pixel wide, 0 if it is 160 or 320 */
int thomson_state::thom_mode_is_hires( int mode )
{
	return ( mode == THOM_VMODE_80 ) || ( mode == THOM_VMODE_80_TO9 );
}



/* either the border index or its palette entry has changed */
void thomson_state::thom_border_changed()
{
	unsigned l = thom_video_elapsed();
	unsigned y = l >> 6;
	unsigned x = l & 63;
	unsigned color =  m_thom_pal[ m_thom_border_index ];

	if ( y >= THOM_TOTAL_HEIGHT )
	{
		/* end of page */
		m_thom_border_r[ THOM_TOTAL_HEIGHT ] = color;
	}
	else if ( ! x )
	{
		/* start of line */
		m_thom_border_l[ y ] = color;
		m_thom_border_r[ y ] = color;
	}
	else if ( x <= 19 )
	{
		/* between left and right border */
		/* NOTE: this makes the lower right part of the color picker blink
		   in the TO8/TO9/TO9+, which actually happens on the real computer!
		*/
		m_thom_border_r[ y ] = color;
		m_thom_border_l[ y + 1 ] = color;
	}
	else
	{
		/* end of line */
		m_thom_border_l[ y + 1 ] = color;
		m_thom_border_r[ y + 1 ] = color;
	}
	m_thom_vstate_dirty = 1;
}



/* the video mode or page has changed */
void thomson_state::thom_gplinfo_changed()
{
	unsigned l = thom_video_elapsed() - THOM_BORDER_HEIGHT * 64 - 7;
	unsigned y = l >> 6;
	unsigned x = l & 63;
	int modepage = ((int)m_thom_vmode << 8) | m_thom_vpage;
	if ( y >= 200 || x>= 40 )
		m_thom_vmodepage[ 40 ] = modepage;
	else
		m_thom_vmodepage[ x ] = modepage;
	m_thom_vmodepage_changed = 1;
}



void thomson_state::thom_set_border_color( unsigned index )
{
	assert( index < 16 );
	if ( index != m_thom_border_index )
	{
		LOG (( "thom_set_border_color: %i at line %i col %i\n", index, thom_video_elapsed() / 64, thom_video_elapsed() % 64  ));
		m_thom_border_index = index;
		thom_border_changed();
	}
}



void thomson_state::thom_set_palette( unsigned index, UINT16 color )
{
	assert( index < 16 );

	if ( color != 0x1000 )
		color &= 0xfff;

	if ( m_thom_pal[ index ] == color )
		return;

	LOG (( "thom_set_palette: %i to %03x at line %i col %i\n", index, color, thom_video_elapsed() / 64, thom_video_elapsed() % 64  ));

	m_thom_pal[ index ] = color;
	if ( index == m_thom_border_index )
		thom_border_changed();
	m_thom_pal_changed = 1;
	m_thom_vstate_dirty = 1;
}



void thomson_state::thom_set_video_mode( unsigned mode )
{
	assert( mode < THOM_VMODE_NB );

	if ( mode != m_thom_vmode )
	{
		LOG (( "thom_set_video_mode: %i at line %i, col %i\n", mode, thom_video_elapsed() / 64, thom_video_elapsed() % 64 ));
		m_thom_vmode = mode;
		thom_gplinfo_changed();
		m_thom_vstate_dirty = 1;
		m_thom_hires_better |= thom_mode_is_hires( mode );
	}
}



void thomson_state::thom_set_video_page( unsigned page )
{
	assert( page < THOM_NB_PAGES )
		;
	if ( page != m_thom_vpage ) {
		LOG (( "thom_set_video_page: %i at line %i col %i\n", page, thom_video_elapsed() / 64, thom_video_elapsed() % 64  ));
		m_thom_vpage = page;
		thom_gplinfo_changed();
		m_thom_vstate_dirty = 1;
	}
}



/* -------------- drawing --------------- */



typedef void ( thomson_state::*thom_scandraw ) ( UINT8* vram, UINT16* dst, UINT16* pal, int org, int len );



#define UPDATE( name, res )                     \
	void thomson_state::name##_scandraw_##res ( \
						UINT8* vram, UINT16* dst,   UINT16* pal, \
						int org, int len )      \
	{                               \
		unsigned gpl;                       \
		vram += org;                        \
		dst += org * res;                   \
		for ( gpl = 0; gpl < len; gpl++, dst += res, vram++ ) { \
			UINT8 rama = vram[ 0      ];            \
			UINT8 ramb = vram[ 0x2000 ];

#define END_UPDATE                          \
		}                           \
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


/* as mo5, but with pastel bit switched */

UPDATE_HI( mo5alt )
{
	int i;
	pen_t c[2];
	c[0] = pal[ (ramb & 15) ^ 8 ];
		c[1] = pal[ (ramb >> 4) ^ 8 ];
	for ( i = 0; i < 16; i += 2, rama >>= 1 )
		dst[ 15 - i ] = dst[ 14 - i ] = c[ rama & 1 ];
}
END_UPDATE

UPDATE_LOW( mo5alt )
{
	int i;
	pen_t c[2];
	c[0] = pal[ (ramb & 15) ^ 8 ];
	c[1] = pal[ (ramb >> 4) ^ 8 ];
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


/* 160x200, 4 colors, no constraint, using only one memory page (undocumented) */

UPDATE_HI( bitmap4althalf )
{
	dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = pal[ rama >> 6 ];
	dst[ 4] = dst[ 5] = dst[ 6] = dst[ 7] = pal[ (rama >> 4) & 3 ];
	dst[ 8] = dst[ 9] = dst[10] = dst[11] = pal[ (rama >> 2) & 3];
	dst[12] = dst[13] = dst[14] = dst[15] = pal[ rama & 3 ];
	(void)ramb; // ramb is not used
}
END_UPDATE

UPDATE_LOW( bitmap4althalf )
{
	dst[0] = dst[1] = pal[ rama >> 6 ];
	dst[2] = dst[3] = pal[ (rama >> 4) & 3 ];
	dst[4] = dst[5] = pal[ (rama >> 2) & 3];
	dst[6] = dst[7] = pal[ rama & 3 ];
	(void)ramb; // ramb is not used
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



/* 320x200, 2-colors, two overlaid pages */

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


/* 160x200 undocumented variant of the above (2-colors, two overlaid pages) */

UPDATE_HI( overlayhalf )
{
	int i;
	pen_t c[2][2];
	c[0][0] = pal[ 0 ];
	c[0][1] = c[1][1] = pal[ 1 ];
	c[1][0] = pal[ 2 ];
		rama >>= 4;
		ramb >>= 4;
	for ( i = 0; i < 16; i += 4, rama >>= 1, ramb >>= 1 )
		dst[ 15 - i ] =  dst[ 14 - i ] = dst[ 13 - i ] =  dst[ 12 - i ] =
						c[ ramb & 1 ] [ rama & 1 ];
}
END_UPDATE

UPDATE_LOW( overlayhalf )
{
	int i;
	pen_t c[2][2];
	c[0][0] = pal[ 0 ];
	c[0][1] = c[1][1] = pal[ 1 ];
	c[1][0] = pal[ 2 ];
		rama >>= 4;
		ramb >>= 4;
	for ( i = 0; i < 8; i += 2, rama >>= 1, ramb >>= 1 )
		dst[ 7 - i ] = dst[ 6 - i ] = c[ ramb & 1 ] [ rama & 1 ];
}
END_UPDATE



/* 160x200, 4-colors, four overlaid pages */

UPDATE_HI( overlay3 )
{
	/* Note: "Manuel Technique" doc implies that the palette entries are 0,1,2,4,8;
	   we now use palette entries 0,1,2,3,4 instead, as this is what the TEO emulator uses and it has been confirmed correct
	*/
	static const int p[2][2][2][2] = {
			{ { { 0, 1 }, { 2, 1 } }, { { 3, 1 }, { 2, 1 } } },
			{ { { 4, 1 }, { 2, 1 } }, { { 3, 1 }, { 2, 1 } } }
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
			{ { { 0, 1 }, { 2, 1 } }, { { 3, 1 }, { 2, 1 } } },
			{ { { 4, 1 }, { 2, 1 } }, { { 3, 1 }, { 2, 1 } } }
	};
	int i;
	for ( i = 0; i < 8; i += 2, rama >>= 1, ramb >>= 1 )
		dst[ 7 - i ] = dst[ 6 - i ] =
			pal[ p[ ramb & 1 ] [ (ramb >> 4) & 1 ]
						[ rama & 1 ] [ (rama >> 4) & 1 ] ];
}
END_UPDATE



#define FUN(x) { &thomson_state::x##_scandraw_8, &thomson_state::x##_scandraw_16 }


static const thom_scandraw thom_scandraw_funcs[THOM_VMODE_NB][2] =
{
	FUN(to770),    FUN(mo5),    FUN(bitmap4), FUN(bitmap4alt),  FUN(mode80),
	FUN(bitmap16), FUN(page1),  FUN(page2),   FUN(overlay),     FUN(overlay3),
	FUN(to9), FUN(mode80_to9),
		FUN(bitmap4althalf), FUN(mo5alt), FUN(overlayhalf),
};



/* called at the start of each scanline in the active area, just after
   left border (-1<=y<199), and also after the last scanline (y=199)
*/
TIMER_CALLBACK_MEMBER( thomson_state::thom_scanline_start )
{
	int y = param;

	/* update active-area */
	if ( y >= 0 && (m_thom_vstate_dirty || m_thom_vstate_last_dirty || m_thom_vmem_dirty[y]) )
	{
		int x = 0;
		while ( x < 40 )
		{
			int xx = x;
			unsigned mode = m_thom_vmodepage[x] >> 8;
			unsigned page = m_thom_vmodepage[x] & 0xff;
			assert( mode < THOM_VMODE_NB );
			assert( page < 4 );
			if ( m_thom_vmodepage_changed )
			{
				do
				{
					xx++;
				}
				while ( xx < 40 && m_thom_vmodepage[xx] == -1 );
			}
			else
			{
				xx = 40;
			}
			(this->*thom_scandraw_funcs[ mode ][ m_thom_hires ])
				( m_thom_vram + y * 40 + page * 0x4000,
					m_thom_vbody + y * 320 * (m_thom_hires+1),
					m_thom_last_pal, x, xx-x );
			x = xx;
		}
		m_thom_vmem_dirty[y] = 0;
	}

	/* prepare for next scanline */
	if ( y == 199 )
		m_thom_scanline_timer->adjust(attotime::never);
	else
	{
		if ( m_thom_vmodepage_changed )
		{
			int x, m = 0;
			for ( x = 0; x <= 40; x++ )
			{
				if ( m_thom_vmodepage[x] !=-1 )
				{
					m = m_thom_vmodepage[x];
					m_thom_vmodepage[x] = -1;
				}
			}
			m_thom_vmodepage[0] = m;
			m_thom_vmodepage_changed = 0;
		}

		if ( m_thom_pal_changed )
		{
			memcpy( m_thom_last_pal, m_thom_pal, 32 );
			m_thom_pal_changed = 0;
		}

		m_thom_scanline_timer->adjust(attotime::from_usec(64), y + 1);
	}
}



/* -------------- misc --------------- */


#define FLOP_STATE (m_thom_floppy_wcount ? 2 : m_thom_floppy_rcount ? 1 : 0)



void thomson_state::thom_set_mode_point( int point )
{
	assert( point >= 0 && point <= 1 );
	m_thom_mode_point = ( ! point ) * 0x2000;
	m_vrambank->set_entry( ! point );
}



void thomson_state::thom_floppy_active( int write )
{
	int fold = FLOP_STATE, fnew;

	/* stays up for a few frames */
	if ( write )
		m_thom_floppy_wcount = 25;
	else
		m_thom_floppy_rcount = 25;

	/* update icon */
	fnew = FLOP_STATE;
	if ( fold != fnew )
		output().set_value( "floppy", fnew );
}



/* -------------- main update function --------------- */



UINT32 thomson_state::screen_update_thom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y, ypos;
	const int scale = m_thom_hires ? 2 : 1;
	const int xbleft = m_thom_bwidth * scale;
	const int xbright = ( m_thom_bwidth + THOM_ACTIVE_WIDTH ) * scale;
	const int xright = ( m_thom_bwidth * 2 + THOM_ACTIVE_WIDTH ) * scale;
	const int xwidth = THOM_ACTIVE_WIDTH * scale;
	const int yup = THOM_BORDER_HEIGHT + THOM_ACTIVE_HEIGHT;
	const int ybot = THOM_BORDER_HEIGHT + m_thom_bheight + 200;
	UINT16* v = m_thom_vbody;
	pen_t border = 0;
	rectangle wrect(0, xright - 1, 0, 0);
	rectangle lrect(0, xbleft - 1, 0, 0);
	rectangle rrect(xbright, xright - 1, 0, 0);

	//LOG (( "%f thom: video update called\n", machine().time().as_double()));

	/* upper border */
	for ( y = 0; y < THOM_BORDER_HEIGHT - m_thom_bheight; y++ )
	{
		if ( m_thom_border_l[ y ] != -1 )
			border = m_thom_border_l[ y ];
	}
	ypos = 0;
	while ( y < THOM_BORDER_HEIGHT )
	{
		if ( m_thom_border_l[ y ] != -1 )
			border = m_thom_border_l[ y ];
		wrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < THOM_BORDER_HEIGHT && m_thom_border_l[ y ] == -1 );
		wrect.max_y = ypos - 1;
		bitmap.fill(border, wrect );
	}

	/* left border */
	while ( y < yup )
	{
		if ( m_thom_border_l[ y ] != -1 )
			border = m_thom_border_l[ y ];
		lrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < yup && m_thom_border_l[ y ] == -1 );
		lrect.max_y = ypos - 1;
		bitmap.fill(border, lrect );
	}

	/* lower border */
	while (y < ybot )
	{
		if ( m_thom_border_l[ y ] != -1 )
			border = m_thom_border_l[ y ];
		wrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		} while ( y < ybot && m_thom_border_l[ y ] == -1 );
		wrect.max_y = ypos - 1;
		bitmap.fill(border, wrect );
	}

	/* right border */
	for ( y = 0; y < THOM_BORDER_HEIGHT; y++ ) {
		if ( m_thom_border_r[ y ] != -1 )
			border = m_thom_border_r[ y ];
	}
	ypos = m_thom_bheight /* * scale */;
	while ( y < yup )
	{
		if ( m_thom_border_r[ y ] != -1 )
			border = m_thom_border_r[ y ];
		rrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < yup && m_thom_border_r[ y ] == -1 );
		rrect.max_y = ypos - 1;
		bitmap.fill(border, rrect );
	}

	/* active area */
	ypos = m_thom_bheight /* * scale */;
	for ( y = 0; y < 200; v += xwidth, y++ , ypos ++ /* += scale */ )
	{
		draw_scanline16( bitmap, xbleft, ypos, xwidth, v, nullptr );
#if 0
		if ( thom_hires )
			draw_scanline16( bitmap, xbleft, ypos+1, xwidth, v, NULL );
#endif
	}

	return 0;
}



/* -------------- frame start ------------------ */


TIMER_CALLBACK_MEMBER( thomson_state::thom_set_init )
{
	int init = param;
	LOG (( "%f thom_set_init: %i, at line %i col %i\n", machine().time().as_double(), init, thom_video_elapsed() / 64, thom_video_elapsed() % 64 ));

	if ( m_thom_init_cb )
		(this->*m_thom_init_cb)( init );
	if ( ! init )
		m_thom_init_timer->adjust(attotime::from_usec( 64 * THOM_ACTIVE_HEIGHT - 24 ), 1-init);
}

/* call this at the very beginning of each new frame */
void thomson_state::thom_vblank( screen_device &screen, bool state )
{
	// rising edge
	if (state)
	{
		int fnew, fold = FLOP_STATE;
		int i;
		UINT16 b = 0;
		struct thom_vsignal l = thom_get_lightpen_vsignal( 0, -1, 0 );

		LOG (( "%f thom: video eof called\n", machine().time().as_double() ));

		/* floppy indicator count */
		if ( m_thom_floppy_wcount )
			m_thom_floppy_wcount--;
		if ( m_thom_floppy_rcount )
			m_thom_floppy_rcount--;
		fnew = FLOP_STATE;
		if ( fnew != fold )
			output().set_value( "floppy", fnew );

		/* prepare state for next frame */
		for ( i = 0; i <= THOM_TOTAL_HEIGHT; i++ )
		{
			if ( m_thom_border_l[ i ] != -1 )
				b = m_thom_border_l[ i ];
			if ( m_thom_border_r[ i ] != -1 )
				b = m_thom_border_r[ i ];
		}
		memset( m_thom_border_l, 0xff, sizeof( m_thom_border_l ) );
		memset( m_thom_border_r, 0xff, sizeof( m_thom_border_r ) );
		m_thom_border_l[ 0 ] = b;
		m_thom_border_r[ 0 ] = b;
		m_thom_vstate_last_dirty = m_thom_vstate_dirty;
		m_thom_vstate_dirty = 0;

		/* schedule first init signal */
		m_thom_init_timer->adjust(attotime::from_usec( 64 * THOM_BORDER_HEIGHT + 7 ));

		/* schedule first lightpen signal */
		l.line &= ~1; /* hack (avoid lock in MO6 palette selection) */
		m_thom_lightpen_timer->adjust(
					attotime::from_usec( 64 * ( THOM_BORDER_HEIGHT + l.line - 2 ) + 16 ), 0);

		/* schedule first active-area scanline call-back */
		m_thom_scanline_timer->adjust(attotime::from_usec( 64 * THOM_BORDER_HEIGHT + 7), -1);

		/* reset video frame time */
		m_thom_video_timer->adjust(attotime::zero);

		/* update screen size according to user options */
		if ( thom_update_screen_size() )
			m_thom_vstate_dirty = 1;

		/* hi-res automatic */
		m_thom_hires_better = thom_mode_is_hires( m_thom_vmode );
	}
}



/* -------------- initialization --------------- */


/* TO7, TO7/70 palette, hardcoded in ROM
   without further information, we assume that the hardcoded values
   are the same as those setup by the TO8/TO9+/MO6 when booting up
 */
static const UINT16 thom_pal_init[16] =
{
	0x1000, /* 0: black */        0x000f, /* 1: red */
	0x00f0, /* 2: geen */         0x00ff, /* 3: yellow */
	0x0f00, /* 4: blue */         0x0f0f, /* 5: purple */
	0x0ff0, /* 6: cyan */         0x0fff, /* 7: white */
	0x0777, /* 8: gray */         0x033a, /* 9: pink */
	0x03a3, /* a: light green */  0x03aa, /* b: light yellow */
	0x0a33, /* c: light blue */   0x0a3a, /* d: redish pink */
	0x0ee7, /* e: light cyan */   0x007b, /* f: orange */
};

/* MO5 palette, hardcoded in a ROM
   values are from "Manuel Technique du MO5", p.19
 */
static const UINT16 mo5_pal_init[16] =
{
	0x1000, /* 0: black */        0x055f, /* 1: red */
	0x00f0, /* 2: geen */         0x00ff, /* 3: yellow */
	0x0f55, /* 4: blue */         0x0f0f, /* 5: purple */
	0x0ff5, /* 6: cyan */         0x0fff, /* 7: white */
	0x0aaa, /* 8: gray */         0x0aaf, /* 9: pink */
	0x0afa, /* a: light green */  0x0aff, /* b: light yellow */
	0x0fa5, /* c: light blue */   0x0faf, /* d: parama pink */
	0x0ffa, /* e: light cyan */   0x05af, /* f: orange */
};


VIDEO_START_MEMBER( thomson_state, thom )
{
	LOG (( "thom: video start called\n" ));

	/* scan-line state */
	memset( m_thom_border_l, 0xff, sizeof( m_thom_border_l ) );
	memset( m_thom_border_r, 0xff, sizeof( m_thom_border_r ) );
	memset( m_thom_vbody, 0, sizeof( m_thom_vbody ) );
	memset( m_thom_vmodepage, 0xffff, sizeof( m_thom_vmodepage ) );
	memset( m_thom_vmem_dirty, 0, sizeof( m_thom_vmem_dirty ) );
	m_thom_border_l[ 0 ] = 0;
	m_thom_border_r[ 0 ] = 0;
	m_thom_vmodepage[ 0 ] = 0;
	m_thom_vmodepage_changed = 0;
	m_thom_vmode = 0;
	m_thom_vpage = 0;
	m_thom_border_index = 0;
	m_thom_vstate_dirty = 1;
	m_thom_vstate_last_dirty = 1;
	save_pointer(NAME(m_thom_last_pal), sizeof(m_thom_last_pal));
	save_pointer(NAME(m_thom_pal), sizeof(m_thom_pal));
	save_pointer(NAME(m_thom_border_l), sizeof(m_thom_border_l));
	save_pointer(NAME(m_thom_border_r), sizeof(m_thom_border_r));
	save_pointer(NAME(m_thom_vbody), sizeof(m_thom_vbody));
	save_pointer(NAME(m_thom_vmodepage), sizeof(m_thom_vmodepage));
	save_pointer(NAME(m_thom_vmem_dirty), sizeof(m_thom_vmem_dirty));
	save_item(NAME(m_thom_pal_changed));
	save_item(NAME(m_thom_vmodepage_changed));
	save_item(NAME(m_thom_vmode));
	save_item(NAME(m_thom_vpage));
	save_item(NAME(m_thom_border_index));
	save_item(NAME(m_thom_vstate_dirty));
	save_item(NAME(m_thom_vstate_last_dirty));

	m_thom_mode_point = 0;
	save_item(NAME(m_thom_mode_point));
	m_vrambank->set_entry( 0 );

	m_thom_floppy_rcount = 0;
	m_thom_floppy_wcount = 0;
	save_item(NAME(m_thom_floppy_wcount));
	save_item(NAME(m_thom_floppy_rcount));
	output().set_value( "floppy", 0 );

	m_thom_video_timer = machine().scheduler().timer_alloc(FUNC_NULL);

	m_thom_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(thomson_state::thom_scanline_start),this));

	m_thom_lightpen_nb = 0;
	m_thom_lightpen_cb = nullptr;
	m_thom_lightpen_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(thomson_state::thom_lightpen_step),this));
	save_item(NAME(m_thom_lightpen_nb));

	m_thom_init_cb = nullptr;
	m_thom_init_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(thomson_state::thom_set_init),this));

	save_item(NAME(m_thom_bwidth));
	save_item(NAME(m_thom_bheight));
	save_item(NAME(m_thom_hires));
	save_item(NAME(m_thom_hires_better));
}


/* sets the fixed palette (for MO5,TO7,TO7/70) and gamma correction */
void thomson_state::thom_configure_palette(double gamma, const UINT16* pal, palette_device& palette)
{
	memcpy( m_thom_last_pal, pal, 32 );
	memcpy( m_thom_pal, pal, 32 );

	for ( int i = 0; i < 4097; i++ )
	{
		UINT8 r = 255. * pow( (i & 15) / 15., gamma );
		UINT8 g = 255. * pow( ((i>> 4) & 15) / 15., gamma );
		UINT8 b = 255. * pow( ((i >> 8) & 15) / 15., gamma );
		/* UINT8 alpha = i & 0x1000 ? 0 : 255;  TODO: transparency */
		palette.set_pen_color(i, r, g, b );
	}
}


PALETTE_INIT_MEMBER(thomson_state, thom)
{
	LOG (( "thom: palette init called\n" ));

		/* TO8 and later use an EF9369 color palette chip
		   The spec shows a built-in gamma correction for gamma=2.8
		   i.e., output is out = in ^ (1/2.8)

		   For the TO7, the gamma correction is irrelevant.

		   For the TO7/70, we use the same palette and gamma has the TO8,
		   which gives good results (but is not verified).
		 */
		thom_configure_palette(1.0 / 2.8, thom_pal_init, palette);
}

PALETTE_INIT_MEMBER(thomson_state, mo5)
{
	LOG (( "thom: MO5 palette init called\n" ));

		/* The MO5 has a different fixed palette than the TO7/70.
		   We use a smaller gamma correction which gives intutively better
		   results (but is not verified).
		 */
		thom_configure_palette(1.0, mo5_pal_init, palette);
}



/***************************** TO7 / T9000 *************************/



/* write to video memory through addresses 0x4000-0x5fff */
WRITE8_MEMBER( thomson_state::to7_vram_w )
{
	assert( offset < 0x2000 );
	/* force two topmost color bits to 1 */
	if ( m_thom_mode_point )
		data |= 0xc0;
	if ( m_thom_vram[ offset + m_thom_mode_point ] == data )
		return;
	m_thom_vram[ offset + m_thom_mode_point ] = data;
	/* dirty whole scanline */
	m_thom_vmem_dirty[ offset / 40 ] = 1;
}



/* bits 0-13 : latched gpl of lightpen position */
/* bit    14:  latched INIT */
/* bit    15:  latched INIL */
unsigned thomson_state::to7_lightpen_gpl ( int decx, int decy )
{
	int x,y;
	thom_get_lightpen_pos( &x, &y );
	x -= m_thom_bwidth;
	y -= m_thom_bheight;
	if ( x < 0 || y < 0 || x >= 320 || y >= 200 )
		return 0;
	x += decx;
	y += decy;
	return y*40 + x/8 + (x < 320 ? 0x4000 : 0) + 0x8000;
}



/************************** TO7/70 / MO5 ****************************/



/* write to video memory through addresses 0x4000-0x5fff (TO)
   or 0x0000-0x1fff (MO) */
WRITE8_MEMBER( thomson_state::to770_vram_w )
{
	assert( offset < 0x2000 );
	if ( m_thom_vram[ offset + m_thom_mode_point ] == data )
		return;
	m_thom_vram[ offset + m_thom_mode_point ] = data;
	/* dirty whole scanline */
	m_thom_vmem_dirty[ offset / 40 ] = 1;
}




/***************************** TO8 ******************************/



/* write to video memory through system space (always page 1) */

WRITE8_MEMBER( thomson_state::to8_sys_lo_w )
{
	UINT8* dst = m_thom_vram + offset + 0x6000;
	assert( offset < 0x2000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	m_thom_vmem_dirty[ offset / 40 ] = 1;
}



WRITE8_MEMBER( thomson_state::to8_sys_hi_w )
{
	UINT8* dst = m_thom_vram + offset + 0x4000;
	assert( offset < 0x2000 );
	if ( *dst == data ) return;
	*dst = data;
	/* dirty whole scanline */
	m_thom_vmem_dirty[ offset / 40 ] = 1;
}



/* write to video memory through data space */

WRITE8_MEMBER( thomson_state::to8_data_lo_w )
{
	UINT8* dst = m_thom_vram + ( ( offset + 0x4000 * m_to8_data_vpage + 0x2000 ) & m_ram->mask() );
	assert( offset < 0x2000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	if ( m_to8_data_vpage >= 4 )
		return;
	m_thom_vmem_dirty[ offset / 40 ] = 1;
}



WRITE8_MEMBER( thomson_state::to8_data_hi_w )
{
	UINT8* dst = m_thom_vram + ( ( offset + 0x4000 * m_to8_data_vpage ) & m_ram->mask() );
	assert( offset < 0x2000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	if ( m_to8_data_vpage >= 4 )
		return;
	m_thom_vmem_dirty[ offset / 40 ] = 1;
}



/* write to video memory page through cartridge addresses space */
WRITE8_MEMBER( thomson_state::to8_vcart_w )
{
	UINT8* dst = m_thom_vram + ( ( offset + 0x4000 * m_to8_cart_vpage ) & m_ram->mask() );
	assert( offset < 0x4000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	if ( m_to8_cart_vpage >= 4  )
		return;
	m_thom_vmem_dirty[ (offset & 0x1fff) / 40 ] = 1;
}

WRITE8_MEMBER( thomson_state::mo6_vcart_lo_w )
{
	UINT8* dst = m_thom_vram + ( ( offset + 0x3000 + 0x4000 * m_to8_cart_vpage ) & m_ram->mask() );
	assert( offset < 0x1000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	if ( m_to8_cart_vpage >= 4  )
		return;
	m_thom_vmem_dirty[ (offset & 0x1fff) / 40 ] = 1;
}

WRITE8_MEMBER( thomson_state::mo6_vcart_hi_w )
{
	UINT8* dst = m_thom_vram + ( ( offset + 0x4000 * m_to8_cart_vpage ) & m_ram->mask() );
	assert( offset < 0x3000 );
	if ( *dst == data )
		return;
	*dst = data;
	/* dirty whole scanline */
	if ( m_to8_cart_vpage >= 4  )
		return;
	m_thom_vmem_dirty[ (offset & 0x1fff) / 40 ] = 1;
}
