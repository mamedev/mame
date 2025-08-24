// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#include "emu.h"
#include "to_video.h"

#include "screen.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"


/* video modes */
enum
{
	THOM_VMODE_TO770        = 0,
	THOM_VMODE_MO5          = 1,
	THOM_VMODE_BITMAP4      = 2,
	THOM_VMODE_BITMAP4_ALT  = 3,
	THOM_VMODE_80           = 4,
	THOM_VMODE_BITMAP16     = 5,
	THOM_VMODE_PAGE1        = 6,
	THOM_VMODE_PAGE2        = 7,
	THOM_VMODE_OVERLAY      = 8,
	THOM_VMODE_OVERLAY3     = 9,
	THOM_VMODE_TO9          = 10,
	THOM_VMODE_80_TO9       = 11,
	THOM_VMODE_BITMAP4_ALT_HALF = 12,
	THOM_VMODE_MO5_ALT      = 13,
	THOM_VMODE_OVERLAY_HALF = 14,
	THOM_VMODE_BITMAP16_ALT = 15,
	THOM_VMODE_NB           = 16
};


// device type definitions
DEFINE_DEVICE_TYPE(TO7_VIDEO, to7_video_device, "to7_video", "Thomson TO7 video components")
DEFINE_DEVICE_TYPE(TO770_VIDEO, to770_video_device, "to770_video", "Thomson TO7/70 video components")
DEFINE_DEVICE_TYPE(TO9_VIDEO, to9_video_device, "to9_video", "Thomson TO9 video components")
DEFINE_DEVICE_TYPE(TO8_VIDEO, to8_video_device, "to8_video", "Thomson TO8 video components")

thomson_video_device::thomson_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_io_lightpen_x(*this, "^lightpen_x")
	, m_io_lightpen_y(*this, "^lightpen_y")
	, m_io_vconfig(*this, "vconfig")
	, m_vram_page_cb(*this)
	, m_lightpen_step_cb(*this)
	, m_init_cb(*this)
	, m_int_50hz_cb(*this)
{
}

to7_video_device::to7_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: thomson_video_device(mconfig, TO7_VIDEO, tag, owner, clock)
{
}

to770_video_device::to770_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: thomson_video_device(mconfig, TO770_VIDEO, tag, owner, clock)
{
}

to9_video_device::to9_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: to9_video_device(mconfig, TO9_VIDEO, tag, owner, clock)
{
	m_style = 0;
}

to9_video_device::to9_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: thomson_video_device(mconfig, type, tag, owner, clock)
{
}

to8_video_device::to8_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: to9_video_device(mconfig, TO8_VIDEO, tag, owner, clock)
	, m_update_ram_bank_cb(*this)
	, m_update_cart_bank_cb(*this)
	, m_lightpen_intr_cb(*this)
{
	m_lightpen_step_cb.set(*this, FUNC(to8_video_device::lightpen_cb));
}

void thomson_video_device::device_config_complete()
{
	if (!has_screen())
		return;

	if (!screen().refresh_attoseconds())
		screen().set_raw(clock(), 1024, 0, THOM_TOTAL_WIDTH * 2, 312, 0, THOM_TOTAL_HEIGHT);

	if (!screen().has_screen_update())
		screen().set_screen_update(*this, FUNC(thomson_video_device::screen_update));
}

static INPUT_PORTS_START ( to_vconfig )
	PORT_START ( "vconfig" )

	PORT_CONFNAME ( 0x03, 0x00, "Border" )
	PORT_CONFSETTING ( 0x00, "Normal (56x47)" )
	PORT_CONFSETTING ( 0x01, "Small (16x16)" )
	PORT_CONFSETTING ( 0x02, DEF_STR ( None ) )

	PORT_CONFNAME ( 0x0c, 0x08, "Resolution" )
	PORT_CONFSETTING ( 0x00, DEF_STR ( Low ) )
	PORT_CONFSETTING ( 0x04, DEF_STR ( High  ) )
	PORT_CONFSETTING ( 0x08, "Auto"  )
INPUT_PORTS_END

ioport_constructor thomson_video_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(to_vconfig);
}


/* One GPL is what is drawn in 1 us by the video system in the active window.
   Most of the time, it corresponds to a 8-pixel wide horizontal span.
   For some TO8/9/9+/MO6 modes, it can be 4-pixel or 16-pixel wide.
   There are always 40 GPLs in an active row, and it is always defined by
   two bytes in video memory (0x2000 bytes appart).
*/

#define THOM_GPL_PER_LINE 40

/* maximum number of video pages:
   1 for TO7 generation (including MO5)
   4 for TO8 generation (including TO9, MO6)
 */
#define THOM_NB_PAGES 4


/****************** dynamic screen size *****************/


bool thomson_video_device::update_screen_size()
{
	const rectangle &visarea = screen().visible_area();
	uint8_t p = m_io_vconfig->read();
	bool changed = false;

	switch ( p & 3 )
	{
	case 0:  m_bwidth = 56; m_bheight = 47; break; /* as in original (?) */
	case 1:  m_bwidth = 16; m_bheight = 16; break; /* small */
	default: m_bwidth =  0; m_bheight =  0; break; /* none */
	}

	switch ( p & 0xc )
	{
	case 0:  m_hires = 0; break;                 /* low */
	case 4:  m_hires = 1; break;                 /* high */
	default: m_hires = m_hires_better; break; /* auto */
	}

	int new_w = ( 320 + m_bwidth * 2 ) * ( m_hires + 1 ) - 1;
	int new_h = ( 200 + m_bheight * 2 ) /** (m_hires + 1 )*/ - 1;
	if ( ( visarea.max_x != new_w ) || ( visarea.max_y != new_h ) )
	{
		changed = true;
		screen().set_visible_area(0, new_w, 0, new_h );
	}

	return changed;
}



/*********************** video timing ******************************/


/* elapsed time from beginning of frame, in us */
unsigned thomson_video_device::video_elapsed()
{
	unsigned u;
	attotime elapsed = m_video_timer->elapsed();
	u = (elapsed * 1000000 ).seconds();
	if ( u >= 19968 )
		u = 19968;
	return u;
}



thomson_video_device::thom_vsignal thomson_video_device::get_vsignal()
{
	thom_vsignal v;
	int gpl = video_elapsed() - 64 * THOM_BORDER_HEIGHT - 7;
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



void thomson_video_device::get_lightpen_pos( int*x, int* y )
{
	*x = m_io_lightpen_x->read();
	*y = m_io_lightpen_y->read();

	if ( *x < 0 )
		*x = 0;

	if ( *y < 0 )
		*y = 0;

	if ( *x > 2 * m_bwidth  + 319 )
		*x = 2 * m_bwidth  + 319;

	if ( *y > 2 * m_bheight + 199 )
		*y = 2 * m_bheight + 199;
}



thomson_video_device::thom_vsignal thomson_video_device::get_lightpen_vsignal( int xdec, int ydec, int xdec2 )
{
	thom_vsignal v;
	int x, y;
	int gpl;

	get_lightpen_pos( &x, &y );
	x += xdec - m_bwidth;
	y += ydec - m_bheight;

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



TIMER_CALLBACK_MEMBER( thomson_video_device::lightpen_step )
{
	int step = param;

	if ( m_lightpen )
	{
		m_lightpen_step_cb( step );
		m_lightpen_step = step;
	}

	if ( step < m_lightpen_nb )
		m_lightpen_timer->adjust(attotime::from_usec( 64 ), step + 1);
}



/***************************** TO7/70 *************************/



uint8_t to770_video_device::gatearray_r(offs_t offset)
{
	thom_vsignal v = get_vsignal();
	thom_vsignal l = get_lightpen_vsignal( m_lightpen_decal, m_lightpen_step - 1, 0 );
	int count, inil, init, lt3;
	count = m_lightpen ? l.count : v.count;
	inil  = m_lightpen ? l.inil  : v.inil;
	init  = m_lightpen ? l.init  : v.init;
	lt3   = m_lightpen ? l.lt3   : v.lt3;

	switch ( offset )
	{
	case 0: return (count >> 8) & 0xff;
	case 1: return count & 0xff;
	case 2: return (lt3 << 7) | (inil << 6);
	case 3: return (init << 7);
	default:
		logerror("%s gatearray_r: invalid offset %i\n", machine().describe_context(), offset);
		return 0;
	}
}



void to770_video_device::gatearray_w(offs_t offset, uint8_t data)
{
	if ( ! offset )
		m_lightpen = data & 1;
}



/***************************** TO9 *************************/



/* ------------ system gate-array ------------ */



uint8_t to9_video_device::gatearray_r(offs_t offset)
{
	thom_vsignal v = get_vsignal();
	thom_vsignal l = get_lightpen_vsignal( m_lightpen_decal, m_lightpen_step - 1, 0 );
	int count, inil, init, lt3;
	count = m_lightpen ? l.count : v.count;
	inil  = m_lightpen ? l.inil  : v.inil;
	init  = m_lightpen ? l.init  : v.init;
	lt3   = m_lightpen ? l.lt3   : v.lt3;

	switch ( offset )
	{
	case 0: return (count >> 8) & 0xff;
	case 1: return count & 0xff;
	case 2: return (lt3 << 7) | (inil << 6);
	case 3: return (v.init << 7) | (init << 6); /* != TO7/70 */
	default:
		logerror("%s gatearray_r: invalid offset %i\n", machine().describe_context(), offset);
		return 0;
	}
}



void to9_video_device::gatearray_w(offs_t offset, uint8_t data)
{
	if ( ! offset )
		m_lightpen = data & 1;
}



/* ------------ video gate-array ------------ */



/* style: 0 => TO9, 1 => TO8/TO9, 2 => MO6 */
void to9_video_device::video_mode_w( uint8_t data )
{
	switch ( data & 0x7f )
	{
	case 0x00:
		if ( m_style == 2 )
			set_video_mode( THOM_VMODE_MO5 );
		else if ( m_style == 1 )
			set_video_mode( THOM_VMODE_TO770 );
		else
			set_video_mode( THOM_VMODE_TO9 );
		break;

	// undocumented, but tested on a real TO8D
	case 0x20: set_video_mode( THOM_VMODE_MO5_ALT );     break;

	case 0x21: set_video_mode( THOM_VMODE_BITMAP4 );     break;

	case 0x41: set_video_mode( THOM_VMODE_BITMAP4_ALT ); break;

	// also undocumented but tested
	case 0x59: set_video_mode( THOM_VMODE_BITMAP4_ALT_HALF ); break;

	case 0x2a:
		if ( m_style==0 )
			set_video_mode( THOM_VMODE_80_TO9 );
		else
			set_video_mode( THOM_VMODE_80 );
		break;

	case 0x7b: set_video_mode( THOM_VMODE_BITMAP16 );    break;

	case 0x24: set_video_mode( THOM_VMODE_PAGE1 );       break;

	case 0x25: set_video_mode( THOM_VMODE_PAGE2 );       break;

	case 0x26: set_video_mode( THOM_VMODE_OVERLAY );     break;

		// undocumented 160x200 variant of overlay
	case 0x3e: set_video_mode( THOM_VMODE_OVERLAY_HALF );     break;

	case 0x3f: set_video_mode( THOM_VMODE_OVERLAY3 );    break;

	// undocumented variant enconding for bitmap16
	case 0x5b: set_video_mode( THOM_VMODE_BITMAP16_ALT ); break;

	default:
		logerror("video_mode_w: unknown mode $%02X tr=%i phi=%i mod=%i\n", data, (data >> 5) & 3, (data >> 3) & 2, data & 7);
	}
}



void to9_video_device::border_color_w(uint8_t data)
{
	set_border_color( data & 15 );
}



/***************************** TO8 *************************/



/* ------------ system gate-array ------------ */


uint8_t to8_video_device::gatearray_r(offs_t offset)
{
	thom_vsignal v = get_vsignal();
	thom_vsignal l = get_lightpen_vsignal( m_lightpen_decal, m_lightpen_step - 1, 6 );
	int count, inil, init, lt3;
	uint8_t res;
	count = m_lightpen ? l.count : v.count;
	inil  = m_lightpen ? l.inil  : v.inil;
	init  = m_lightpen ? l.init  : v.init;
	lt3   = m_lightpen ? l.lt3   : v.lt3;

	switch ( offset )
	{
	case 0: /* system 2 / lightpen register 1 */
		if ( m_lightpen )
			res = (count >> 8) & 0xff;
		else
			res = m_reg_sys2 & 0xf0;
		break;

	case 1: /* ram register / lightpen register 2 */
		if ( m_lightpen )
		{
			if ( !machine().side_effects_disabled() )
			{
				m_lightpen_intr_cb(0);
				m_lightpen_intr = 0;
			}
			res = count & 0xff;
		}
		else
			res = m_reg_ram & 0x1f;
		break;

	case 2: /* cartrige register / lightpen register 3 */
		if ( m_lightpen )
			res = (lt3 << 7) | (inil << 6);
		else
			res = m_reg_cart;
		break;

	case 3: /* lightpen register 4 */
		res = (v.init << 7) | (init << 6) | (v.inil << 5) | (m_lightpen_intr << 1) | m_lightpen;
		break;

	default:
		logerror("%s gatearray_r: invalid offset %i\n", machine().describe_context(), offset);
		res = 0;
	}

	LOG("%s %f gatearray_r: off=%i res=$%02X lightpen=%i\n", machine().describe_context(), machine().time().as_double(), offset, res, m_lightpen);

	return res;
}



void to8_video_device::gatearray_w(offs_t offset, uint8_t data)
{
	LOG("%s %f gatearray_w: off=%i data=$%02X\n", machine().describe_context(), machine().time().as_double(), offset, data);

	switch ( offset )
	{
	case 0: /* switch */
		m_lightpen = data & 1;
		break;

	case 1: /* ram register */
		if ( m_reg_sys1 & 0x10 )
		{
			m_reg_ram = data;
			m_update_ram_bank_cb();
		}
		break;

	case 2: /* cartridge register */
		m_reg_cart = data;
		m_update_cart_bank_cb();
		break;

	case 3: /* system register 1 */
		m_reg_sys1 = data;
		m_update_ram_bank_cb();
		m_update_cart_bank_cb();
		break;

	default:
		logerror("%s gatearray_w: invalid offset %i (data=$%02X)\n", machine().describe_context(), offset, data);
	}
}



void to8_video_device::sys2_w(uint8_t data)
{
	m_reg_sys2 = data;
	set_video_page( data >> 6 );
	set_border_color( data & 15 );
	m_update_cart_bank_cb();
}


/* ------------ lightpen ------------ */


/* direct connection to interrupt line instead of through a PIA */
void to8_video_device::lightpen_cb( int step )
{
	m_lightpen_intr_cb(1);
	m_lightpen_intr = 1;
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
bool thomson_video_device::mode_is_hires( int mode ) const
{
	return ( mode == THOM_VMODE_80 ) || ( mode == THOM_VMODE_80_TO9 );
}



/* either the border index or its palette entry has changed */
void thomson_video_device::border_changed()
{
	unsigned l = video_elapsed();
	unsigned y = l >> 6;
	unsigned x = l & 63;
	unsigned color =  m_pal[ m_border_index ];

	if ( y >= THOM_TOTAL_HEIGHT )
	{
		/* end of page */
		m_border_r[ THOM_TOTAL_HEIGHT ] = color;
	}
	else if ( ! x )
	{
		/* start of line */
		m_border_l[ y ] = color;
		m_border_r[ y ] = color;
	}
	else if ( x <= 19 )
	{
		/* between left and right border */
		/* NOTE: this makes the lower right part of the color picker blink
		   in the TO8/TO9/TO9+, which actually happens on the real computer!
		*/
		m_border_r[ y ] = color;
		m_border_l[ y + 1 ] = color;
	}
	else
	{
		/* end of line */
		m_border_l[ y + 1 ] = color;
		m_border_r[ y + 1 ] = color;
	}
	m_vstate_dirty = true;
}



/* the video mode or page has changed */
void thomson_video_device::gplinfo_changed()
{
	unsigned l = video_elapsed() - THOM_BORDER_HEIGHT * 64 - 7;
	unsigned y = l >> 6;
	unsigned x = l & 63;
	int modepage = ((int)m_vmode << 8) | m_vpage;
	if ( y >= 200 || x>= 40 )
		m_vmodepage[ 40 ] = modepage;
	else
		m_vmodepage[ x ] = modepage;
	m_vmodepage_changed = 1;
}



void thomson_video_device::set_border_color( unsigned index )
{
	assert( index < 16 );
	if ( index != m_border_index )
	{
		LOG("set_border_color: %i at line %i col %i\n", index, video_elapsed() / 64, video_elapsed() % 64);
		m_border_index = index;
		border_changed();
	}
}



void thomson_video_device::set_palette( unsigned index, uint16_t color )
{
	assert( index < 16 );

	if ( color != 0x1000 )
		color &= 0xfff;

	if ( m_pal[ index ] == color )
		return;

	LOG("set_palette: %i to %03x at line %i col %i\n", index, color, video_elapsed() / 64, video_elapsed() % 64);

	m_pal[ index ] = color;
	if ( index == m_border_index )
		border_changed();
	m_pal_changed = true;
	m_vstate_dirty = true;
}



void thomson_video_device::set_video_mode( unsigned mode )
{
	assert( mode < THOM_VMODE_NB );

	if ( mode != m_vmode )
	{
		LOG("set_video_mode: %i at line %i, col %i\n", mode, video_elapsed() / 64, video_elapsed() % 64);
		m_vmode = mode;
		gplinfo_changed();
		m_vstate_dirty = true;
		if ( mode_is_hires( mode ) )
			m_hires_better = true;
	}
}



void thomson_video_device::set_video_page( unsigned page )
{
	assert( page < THOM_NB_PAGES );

	if ( page != m_vpage )
	{
		LOG("set_video_page: %i at line %i col %i\n", page, video_elapsed() / 64, video_elapsed() % 64);
		m_vpage = page;
		gplinfo_changed();
		m_vstate_dirty = true;
	}
}



/* -------------- drawing --------------- */



typedef void ( thomson_video_device::*thom_scandraw ) ( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );



#define UPDATE( name, res )                     \
	void thomson_video_device::name##_scandraw_##res ( \
						uint8_t* vram, uint16_t* dst,   uint16_t* pal, \
						int org, int len )      \
	{                               \
		unsigned gpl;                       \
		vram += org;                        \
		dst += org * res;                   \
		for ( gpl = 0; gpl < len; gpl++, dst += res, vram++ ) { \
			uint8_t rama = vram[ 0      ];            \
			uint8_t ramb = vram[ 0x2000 ];

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


/* 160x200, 16-colors, no constraint, alternate encoding, undocumented, tested */

static constexpr unsigned tbl_bit16[4][4] = {
		{  0,  2,  8, 10 },
		{  1,  3,  9, 11 },
		{  4,  6, 12, 14 },
		{  5,  7, 13, 15 } };

UPDATE_HI( bitmap16alt )
{
	unsigned p0 = tbl_bit16[ramb >> 6][rama >> 6];
	unsigned p1 = tbl_bit16[(ramb >> 4) & 3][(rama >> 4) & 3];
	unsigned p2 = tbl_bit16[(ramb >> 2) & 3][(rama >> 2) & 3];
	unsigned p3 = tbl_bit16[ramb & 3][rama & 3];
	dst[ 0] = dst[ 1] = dst[ 2] = dst[ 3] = pal[ p0 ];
	dst[ 4] = dst[ 5] = dst[ 6] = dst[ 7] = pal[ p1 ];
	dst[ 8] = dst[ 9] = dst[10] = dst[11] = pal[ p2 ];
	dst[12] = dst[13] = dst[14] = dst[15] = pal[ p3 ];
}
END_UPDATE

UPDATE_LOW( bitmap16alt )
{
	unsigned p0 = tbl_bit16[ramb >> 6][rama >> 6];
	unsigned p1 = tbl_bit16[(ramb >> 4) & 3][(rama >> 4) & 3];
	unsigned p2 = tbl_bit16[(ramb >> 2) & 3][(rama >> 2) & 3];
	unsigned p3 = tbl_bit16[ramb & 3][rama & 3];
	dst[0] = dst[1] = pal[ p0 ];
	dst[2] = dst[3] = pal[ p1 ];
	dst[4] = dst[5] = pal[ p2 ];
	dst[6] = dst[7] = pal[ p3 ];
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



#define FUN(x) { &thomson_video_device::x##_scandraw_8, &thomson_video_device::x##_scandraw_16 }


static const thom_scandraw thom_scandraw_funcs[THOM_VMODE_NB][2] =
{
	FUN(to770),    FUN(mo5),    FUN(bitmap4), FUN(bitmap4alt),  FUN(mode80),
	FUN(bitmap16), FUN(page1),  FUN(page2),   FUN(overlay),     FUN(overlay3),
	FUN(to9), FUN(mode80_to9),
	FUN(bitmap4althalf), FUN(mo5alt), FUN(overlayhalf),
	FUN(bitmap16alt)
};



/* called at the start of each scanline in the active area, just after
   left border (-1<=y<199), and also after the last scanline (y=199)
*/
TIMER_CALLBACK_MEMBER( thomson_video_device::scanline_start )
{
	int y = param;

	/* update active-area */
	if ( y >= 0 && (m_vstate_dirty || m_vstate_last_dirty || m_vmem_dirty[y]) )
	{
		int x = 0;
		while ( x < 40 )
		{
			int xx = x;
			unsigned mode = m_vmodepage[x] >> 8;
			unsigned page = m_vmodepage[x] & 0xff;
			assert( mode < THOM_VMODE_NB );
			assert( page < 4 );
			if ( m_vmodepage_changed )
			{
				do
				{
					xx++;
				}
				while ( xx < 40 && m_vmodepage[xx] == -1 );
			}
			else
			{
				xx = 40;
			}
			(this->*thom_scandraw_funcs[ mode ][ m_hires ])
				( m_vram_page_cb(page) + y * 40,
					m_vbody + y * 320 * (m_hires+1),
					m_last_pal, x, xx-x );
			x = xx;
		}
		m_vmem_dirty[y] = false;
	}

	/* prepare for next scanline */
	if ( y == 199 )
		m_scanline_timer->adjust(attotime::never);
	else
	{
		if ( m_vmodepage_changed )
		{
			int x, m = 0;
			for ( x = 0; x <= 40; x++ )
			{
				if ( m_vmodepage[x] !=-1 )
				{
					m = m_vmodepage[x];
					m_vmodepage[x] = -1;
				}
			}
			m_vmodepage[0] = m;
			m_vmodepage_changed = 0;
		}

		if ( m_pal_changed )
		{
			std::copy_n(&m_pal[0], 16, &m_last_pal[0]);
			m_pal_changed = false;
		}

		m_scanline_timer->adjust(attotime::from_usec(64), y + 1);
	}
}



/* ------------ periodic interrupt ------------ */

/* the MO5 & MO6 do not have a MC 6846 timer,
   they have a fixed 50 Hz timer instead
*/


TIMER_CALLBACK_MEMBER(thomson_video_device::synlt_50hz)
{
	/* pulse */
	m_int_50hz_cb( 1 );
	m_int_50hz_cb( 0 );
}



/* -------------- main update function --------------- */



uint32_t thomson_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y, ypos;
	const int scale = m_hires ? 2 : 1;
	const int xbleft = m_bwidth * scale;
	const int xbright = ( m_bwidth + THOM_ACTIVE_WIDTH ) * scale;
	const int xright = ( m_bwidth * 2 + THOM_ACTIVE_WIDTH ) * scale;
	const int xwidth = THOM_ACTIVE_WIDTH * scale;
	const int yup = THOM_BORDER_HEIGHT + THOM_ACTIVE_HEIGHT;
	const int ybot = THOM_BORDER_HEIGHT + m_bheight + 200;
	uint16_t* v = m_vbody;
	pen_t border = 0;
	rectangle wrect(0, xright - 1, 0, 0);
	rectangle lrect(0, xbleft - 1, 0, 0);
	rectangle rrect(xbright, xright - 1, 0, 0);

	//LOG("%f thom: video update called\n", machine().time().as_double());

	/* upper border */
	for ( y = 0; y < THOM_BORDER_HEIGHT - m_bheight; y++ )
	{
		if ( m_border_l[ y ] != -1 )
			border = m_border_l[ y ];
	}
	ypos = 0;
	while ( y < THOM_BORDER_HEIGHT )
	{
		if ( m_border_l[ y ] != -1 )
			border = m_border_l[ y ];
		wrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < THOM_BORDER_HEIGHT && m_border_l[ y ] == -1 );
		wrect.max_y = ypos - 1;
		bitmap.fill(border, wrect );
	}

	/* left border */
	while ( y < yup )
	{
		if ( m_border_l[ y ] != -1 )
			border = m_border_l[ y ];
		lrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < yup && m_border_l[ y ] == -1 );
		lrect.max_y = ypos - 1;
		bitmap.fill(border, lrect );
	}

	/* lower border */
	while (y < ybot )
	{
		if ( m_border_l[ y ] != -1 )
			border = m_border_l[ y ];
		wrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		} while ( y < ybot && m_border_l[ y ] == -1 );
		wrect.max_y = ypos - 1;
		bitmap.fill(border, wrect );
	}

	/* right border */
	for ( y = 0; y < THOM_BORDER_HEIGHT; y++ ) {
		if ( m_border_r[ y ] != -1 )
			border = m_border_r[ y ];
	}
	ypos = m_bheight /* * scale */;
	while ( y < yup )
	{
		if ( m_border_r[ y ] != -1 )
			border = m_border_r[ y ];
		rrect.min_y = ypos;
		do
		{
			y++;
			ypos ++ /* += scale */;
		}
		while ( y < yup && m_border_r[ y ] == -1 );
		rrect.max_y = ypos - 1;
		bitmap.fill(border, rrect );
	}

	/* active area */
	ypos = m_bheight /* * scale */;
	for ( y = 0; y < 200; v += xwidth, y++ , ypos ++ /* += scale */ )
	{
		draw_scanline16( bitmap, xbleft, ypos, xwidth, v, nullptr );
#if 0
		if ( thom_hires )
			draw_scanline16( bitmap, xbleft, ypos+1, xwidth, v, nullptr );
#endif
	}

	return 0;
}



/* -------------- frame start ------------------ */


TIMER_CALLBACK_MEMBER( thomson_video_device::set_init )
{
	int init = param;
	LOG("%f thom_set_init: %i, at line %i col %i\n", machine().time().as_double(), init, video_elapsed() / 64, video_elapsed() % 64);

	m_init_cb( init );
	if ( ! init )
		m_init_timer->adjust(attotime::from_usec( 64 * THOM_ACTIVE_HEIGHT - 24 ), 1-init);
}

/* call this at the very beginning of each new frame */
void thomson_video_device::thom_vblank(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		int i;
		uint16_t b = 0;
		thom_vsignal l = get_lightpen_vsignal( 0, -1, 0 );

		LOG("%f thom: video eof called\n", machine().time().as_double());

		/* prepare state for next frame */
		for ( i = 0; i <= THOM_TOTAL_HEIGHT; i++ )
		{
			if ( m_border_l[ i ] != -1 )
				b = m_border_l[ i ];
			if ( m_border_r[ i ] != -1 )
				b = m_border_r[ i ];
		}
		std::fill(std::begin(m_border_l), std::end(m_border_l), -1);
		std::fill(std::begin(m_border_r), std::end(m_border_r), -1);
		m_border_l[ 0 ] = b;
		m_border_r[ 0 ] = b;
		m_vstate_last_dirty = m_vstate_dirty;
		m_vstate_dirty = false;

		/* schedule first init signal */
		m_init_timer->adjust(attotime::from_usec( 64 * THOM_BORDER_HEIGHT + 7 ));

		/* schedule first lightpen signal */
		l.line &= ~1; /* hack (avoid lock in MO6 palette selection) */
		m_lightpen_timer->adjust(
					attotime::from_usec( 64 * ( THOM_BORDER_HEIGHT + l.line - 2 ) + 16 ), 0);

		/* schedule first active-area scanline call-back */
		m_scanline_timer->adjust(attotime::from_usec( 64 * THOM_BORDER_HEIGHT + 7), -1);

		/* reset video frame time */
		m_video_timer->adjust(attotime::zero);

		/* update screen size according to user options */
		if ( update_screen_size() )
			m_vstate_dirty = true;

		/* hi-res automatic */
		m_hires_better = mode_is_hires( m_vmode );
	}
}



/* -------------- initialization --------------- */


void thomson_video_device::device_start()
{
	m_vram_page_cb.resolve();
	m_lightpen_step_cb.resolve();

	/* scan-line state */
	std::fill(std::begin(m_border_l), std::end(m_border_l), -1);
	std::fill(std::begin(m_border_r), std::end(m_border_r), -1);
	std::fill(std::begin(m_vbody), std::end(m_vbody), 0);
	std::fill(std::begin(m_vmodepage), std::end(m_vmodepage), -1);
	std::fill(std::begin(m_vmem_dirty), std::end(m_vmem_dirty), false);
	m_border_l[ 0 ] = 0;
	m_border_r[ 0 ] = 0;
	m_vmodepage[ 0 ] = 0;
	m_vmodepage_changed = 0;
	m_vmode = 0;
	m_vpage = 0;
	m_border_index = 0;
	m_vstate_dirty = true;
	m_vstate_last_dirty = true;
	save_item(NAME(m_last_pal));
	save_item(NAME(m_pal));
	save_item(NAME(m_border_l));
	save_item(NAME(m_border_r));
	save_item(NAME(m_vbody));
	save_item(NAME(m_vmodepage));
	save_item(NAME(m_vmem_dirty));
	save_item(NAME(m_pal_changed));
	save_item(NAME(m_vmodepage_changed));
	save_item(NAME(m_vmode));
	save_item(NAME(m_vpage));
	save_item(NAME(m_border_index));
	save_item(NAME(m_vstate_dirty));
	save_item(NAME(m_vstate_last_dirty));

	m_video_timer = machine().scheduler().timer_alloc(timer_expired_delegate());

	m_scanline_timer = timer_alloc(FUNC(thomson_video_device::scanline_start), this);

	m_lightpen_timer = timer_alloc(FUNC(thomson_video_device::lightpen_step), this);
	save_item(NAME(m_lightpen));
	save_item(NAME(m_lightpen_step));

	m_init_timer = timer_alloc(FUNC(thomson_video_device::set_init), this);

	if (!m_int_50hz_cb.isunset())
		m_synlt_timer = timer_alloc(FUNC(thomson_video_device::synlt_50hz), this);

	screen().register_vblank_callback(vblank_state_delegate(&thomson_video_device::thom_vblank, this));

	m_bwidth = 0;
	m_bheight = 0;
	m_hires = 0;
	m_hires_better = false;
	save_item(NAME(m_bwidth));
	save_item(NAME(m_bheight));
	save_item(NAME(m_hires));
	save_item(NAME(m_hires_better));
}

void thomson_video_device::device_reset()
{
	/* lightpen */
	m_lightpen = 0;

	if (!m_int_50hz_cb.isunset())
	{
		/* time is a little faster than 50 Hz to match video framerate */
		m_synlt_timer->adjust(attotime::zero, 0, screen().frame_period());
	}
}


void to7_video_device::device_reset()
{
	thomson_video_device::device_reset();

	set_video_mode( THOM_VMODE_TO770 );
	set_border_color( 0 );
}

void to770_video_device::device_reset()
{
	thomson_video_device::device_reset();

	set_video_mode( m_is_mo ? THOM_VMODE_MO5 : THOM_VMODE_TO770 );
	set_border_color( m_is_mo ? 0 : 8 );
}

void to9_video_device::device_reset()
{
	thomson_video_device::device_reset();

	set_video_mode( THOM_VMODE_TO9 );
	set_border_color( 8 );
}

void to8_video_device::device_start()
{
	thomson_video_device::device_start();

	m_update_ram_bank_cb.resolve();
	m_update_cart_bank_cb.resolve();

	m_style = m_is_mo ? 2 : 1;

	save_item(NAME(m_reg_ram));
	save_item(NAME(m_reg_cart));
	save_item(NAME(m_reg_sys1));
	save_item(NAME(m_reg_sys2));
	save_item(NAME(m_lightpen_intr));
}

void to8_video_device::device_reset()
{
	thomson_video_device::device_reset();

	set_video_mode( m_is_mo ? THOM_VMODE_MO5 : THOM_VMODE_TO770 );
	set_border_color( 0 );

	m_reg_ram = 0;
	m_reg_cart = 0;
	m_reg_sys1 = 0;
	m_reg_sys2 = 0;
	m_lightpen_intr = 0;

	m_update_ram_bank_cb();
	m_update_cart_bank_cb();
	m_lightpen_intr_cb(0);
}


void thomson_video_device::set_fixed_palette(const uint16_t *pal)
{
	std::copy_n(&pal[0], 16, &m_last_pal[0]);
	std::copy_n(&pal[0], 16, &m_pal[0]);
}



/***************************** TO7 / T9000 *************************/



/* bits 0-13 : latched gpl of lightpen position */
/* bit    14:  latched INIT */
/* bit    15:  latched INIL */
unsigned thomson_video_device::to7_lightpen_gpl()
{
	int decx = m_lightpen_decal;
	int decy = m_lightpen_step;

	int x,y;
	get_lightpen_pos( &x, &y );
	x -= m_bwidth;
	y -= m_bheight;
	if ( x < 0 || y < 0 || x >= 320 || y >= 200 )
		return 0;
	x += decx;
	y += decy;
	return y*40 + x/8 + (x < 320 ? 0x4000 : 0) + 0x8000;
}
