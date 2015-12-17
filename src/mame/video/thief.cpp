// license:BSD-3-Clause
// copyright-holders:Victor Trucco, Mike Balfour, Phil Stroffolino
/*  video hardware for Pacific Novelty games:
**  Thief/Nato Defense
*/

#include "emu.h"
#include "includes/thief.h"


enum {
	IMAGE_ADDR_LO,      //0xe000
	IMAGE_ADDR_HI,      //0xe001
	SCREEN_XPOS,        //0xe002
	SCREEN_YPOS,        //0xe003
	BLIT_WIDTH,         //0xe004
	BLIT_HEIGHT,        //0xe005
	GFX_PORT,           //0xe006
	BARL_PORT,          //0xe007
	BLIT_ATTRIBUTES     //0xe008
};

/***************************************************************************/

READ8_MEMBER(thief_state::thief_context_ram_r){
	return m_coprocessor.context_ram[0x40*m_coprocessor.bank+offset];
}

WRITE8_MEMBER(thief_state::thief_context_ram_w){
	m_coprocessor.context_ram[0x40*m_coprocessor.bank+offset] = data;
}

WRITE8_MEMBER(thief_state::thief_context_bank_w){
	m_coprocessor.bank = data&0xf;
}

/***************************************************************************/

WRITE8_MEMBER(thief_state::thief_video_control_w){
	m_video_control = data;
/*
    bit 0: screen flip
    bit 1: working page
    bit 2: visible page
    bit 3: mirrors bit 1
    bit 4: mirrors bit 2
*/
}

WRITE8_MEMBER(thief_state::thief_color_map_w){
/*
    --xx----    blue
    ----xx--    green
    ------xx    red
*/
	static const UINT8 intensity[4] = {0x00,0x55,0xAA,0xFF};
	int r = intensity[(data & 0x03) >> 0];
	int g = intensity[(data & 0x0C) >> 2];
	int b = intensity[(data & 0x30) >> 4];
	m_palette->set_pen_color( offset,rgb_t(r,g,b) );
}

/***************************************************************************/

WRITE8_MEMBER(thief_state::thief_color_plane_w){
/*
    --xx----    selects bitplane to read from (0..3)
    ----xxxx    selects bitplane(s) to write to (0x0 = none, 0xf = all)
*/
	m_write_mask = data&0xf;
	m_read_mask = (data>>4)&3;
}

READ8_MEMBER(thief_state::thief_videoram_r){
	UINT8 *videoram = m_videoram;
	UINT8 *source = &videoram[offset];
	if( m_video_control&0x02 ) source+=0x2000*4; /* foreground/background */
	return source[m_read_mask*0x2000];
}

WRITE8_MEMBER(thief_state::thief_videoram_w){
	UINT8 *videoram = m_videoram;
	UINT8 *dest = &videoram[offset];
	if( m_video_control&0x02 )
		dest+=0x2000*4; /* foreground/background */
	if( m_write_mask&0x1 ) dest[0x2000*0] = data;
	if( m_write_mask&0x2 ) dest[0x2000*1] = data;
	if( m_write_mask&0x4 ) dest[0x2000*2] = data;
	if( m_write_mask&0x8 ) dest[0x2000*3] = data;
}

/***************************************************************************/

void thief_state::video_start(){
	memset( &m_coprocessor, 0x00, sizeof(m_coprocessor) );

	m_videoram = auto_alloc_array_clear(machine(), UINT8, 0x2000*4*2 );

	m_coprocessor.image_ram = std::make_unique<UINT8[]>(0x2000 );
	m_coprocessor.context_ram = std::make_unique<UINT8[]>(0x400 );
}

UINT32 thief_state::screen_update_thief(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){
	UINT8 *videoram = m_videoram;
	UINT32 offs;
	int flipscreen = m_video_control&1;
	const UINT8 *source = videoram;

	if (m_tms->screen_reset())
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	if( m_video_control&4 ) /* visible page */
		source += 0x2000*4;

	for( offs=0; offs<0x2000; offs++ ){
		int ypos = offs/32;
		int xpos = (offs%32)*8;
		int plane0 = source[0x2000*0+offs];
		int plane1 = source[0x2000*1+offs];
		int plane2 = source[0x2000*2+offs];
		int plane3 = source[0x2000*3+offs];
		int bit;
		if( flipscreen ){
			for( bit=0; bit<8; bit++ ){
				bitmap.pix16(0xff - ypos, 0xff - (xpos+bit)) =
						(((plane0<<bit)&0x80)>>7) |
						(((plane1<<bit)&0x80)>>6) |
						(((plane2<<bit)&0x80)>>5) |
						(((plane3<<bit)&0x80)>>4);
			}
		}
		else {
			for( bit=0; bit<8; bit++ ){
				bitmap.pix16(ypos, xpos+bit) =
						(((plane0<<bit)&0x80)>>7) |
						(((plane1<<bit)&0x80)>>6) |
						(((plane2<<bit)&0x80)>>5) |
						(((plane3<<bit)&0x80)>>4);
			}
		}
	}
	return 0;
}

/***************************************************************************/

UINT16 thief_state::fetch_image_addr( coprocessor_t &thief_coprocessor )
{
	int addr = thief_coprocessor.param[IMAGE_ADDR_LO]+256*thief_coprocessor.param[IMAGE_ADDR_HI];
	/* auto-increment */
	thief_coprocessor.param[IMAGE_ADDR_LO]++;
	if( thief_coprocessor.param[IMAGE_ADDR_LO]==0x00 ){
		thief_coprocessor.param[IMAGE_ADDR_HI]++;
	}
	return addr;
}

WRITE8_MEMBER(thief_state::thief_blit_w){
	coprocessor_t &thief_coprocessor = m_coprocessor;
	int i, offs, xoffset, dy;
	UINT8 *gfx_rom = memregion( "gfx1" )->base();
	UINT8 x = thief_coprocessor.param[SCREEN_XPOS];
	UINT8 y = thief_coprocessor.param[SCREEN_YPOS];
	UINT8 width = thief_coprocessor.param[BLIT_WIDTH];
	UINT8 height = thief_coprocessor.param[BLIT_HEIGHT];
	UINT8 attributes = thief_coprocessor.param[BLIT_ATTRIBUTES];

	UINT8 old_data;
	int xor_blit = data;
		/* making the xor behavior selectable fixes score display,
		but causes minor glitches on the playfield */

	x -= width*8;
	xoffset = x&7;

	if( attributes&0x10 ){
		y += 7-height;
		dy = 1;
	}
	else {
		dy = -1;
	}
	height++;
	while( height-- ){
		for( i=0; i<=width; i++ ){
			int addr = fetch_image_addr(thief_coprocessor);
			if( addr<0x2000 ){
				data = thief_coprocessor.image_ram[addr];
			}
			else {
				addr -= 0x2000;
				if( addr<0x2000*3 ) data = gfx_rom[addr];
			}
			offs = (y*32+x/8+i)&0x1fff;
			old_data = thief_videoram_r(space,offs );
			if( xor_blit ){
				thief_videoram_w(space,offs, old_data^(data>>xoffset) );
			}
			else {
				thief_videoram_w(space,offs,
					(old_data&(0xff00>>xoffset)) | (data>>xoffset)
				);
			}
			offs = (offs+1)&0x1fff;
			old_data = thief_videoram_r(space,offs );
			if( xor_blit ){
				thief_videoram_w(space,offs, old_data^((data<<(8-xoffset))&0xff) );
			}
			else {
				thief_videoram_w(space,offs,
					(old_data&(0xff>>xoffset)) | ((data<<(8-xoffset))&0xff)
				);
			}
		}
		y+=dy;
	}
}

READ8_MEMBER(thief_state::thief_coprocessor_r){
	coprocessor_t &thief_coprocessor = m_coprocessor;
	switch( offset ){
	case SCREEN_XPOS: /* xpos */
	case SCREEN_YPOS: /* ypos */
		{
		/* XLAT: given (x,y) coordinate, return byte address in videoram */
			int addr = thief_coprocessor.param[SCREEN_XPOS]+256*thief_coprocessor.param[SCREEN_YPOS];
			int result = 0xc000 | (addr>>3);
			return (offset==0x03)?(result>>8):(result&0xff);
		}

	case GFX_PORT:
		{
			int addr = fetch_image_addr(thief_coprocessor);
			if( addr<0x2000 ){
				return thief_coprocessor.image_ram[addr];
			}
			else {
				UINT8 *gfx_rom = memregion( "gfx1" )->base();
				addr -= 0x2000;
				if( addr<0x6000 ) return gfx_rom[addr];
			}
		}
		break;

	case BARL_PORT:
		{
			/* return bitmask for addressed pixel */
			int dx = thief_coprocessor.param[SCREEN_XPOS]&0x7;
			if( thief_coprocessor.param[BLIT_ATTRIBUTES]&0x01 ){
				return 0x01<<dx; // flipx
			}
			else {
				return 0x80>>dx; // no flip
			}
		}
	}

	return thief_coprocessor.param[offset];
}

WRITE8_MEMBER(thief_state::thief_coprocessor_w){
	coprocessor_t &thief_coprocessor = m_coprocessor;
	switch( offset ){
	case GFX_PORT:
		{
			int addr = fetch_image_addr(thief_coprocessor);
			if( addr<0x2000 ){
				thief_coprocessor.image_ram[addr] = data;
			}
		}
		break;

	default:
		thief_coprocessor.param[offset] = data;
		break;
	}
}
