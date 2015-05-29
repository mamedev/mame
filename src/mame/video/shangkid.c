// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/* video/shangkid */

#include "emu.h"
#include "includes/shangkid.h"


TILE_GET_INFO_MEMBER(shangkid_state::get_bg_tile_info){
	int attributes = m_videoram[tile_index+0x800];
	int tile_number = m_videoram[tile_index]+0x100*(attributes&0x3);
	int color;

	if( m_gfx_type==1 )
	{
		/* Shanghai Kid:
		    ------xx    bank
		    -----x--    flipx
		    xxxxx---    color
		*/
		color = attributes>>3;
		color = (color&0x03)|((color&0x1c)<<1);
		SET_TILE_INFO_MEMBER(0,
				tile_number,
				color,
				(attributes&0x04)?TILE_FLIPX:0);
	}
	else
	{
		/* Chinese Hero:
		    ------xx    bank
		    -xxxxx--    color
		    x-------    flipx?
		*/
		color = (attributes>>2)&0x1f;
		SET_TILE_INFO_MEMBER(0,
				tile_number,
				color,
				(attributes&0x80)?TILE_FLIPX:0);
	}

	tileinfo.category =
		(memregion( "proms" )->base()[0x800+color*4]==2)?1:0;
}

VIDEO_START_MEMBER(shangkid_state,shangkid)
{
	m_background = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(shangkid_state::get_bg_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);
}

WRITE8_MEMBER(shangkid_state::videoram_w)
{
	m_videoram[offset] = data;
	m_background->mark_tile_dirty(offset&0x7ff );
}

void shangkid_state::draw_sprite(const UINT8 *source, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx;
	int transparent_pen;
	int bank_index;
	int c,r;
	int width,height;
	int sx,sy;

	int ypos        = 209 - source[0];
	int tile        = source[1]&0x3f;
	int xflip       = (source[1]&0x40)?1:0;
	int yflip       = (source[1]&0x80)?1:0;
	int bank        = source[2]&0x3f;
	int xsize       = (source[2]&0x40)?1:0;
	int ysize       = (source[2]&0x80)?1:0;
	int yscale      = source[3]&0x07;   /* 0x0 = smallest; 0x7 = biggest */
	int xpos        = ((source[4]+source[5]*255)&0x1ff)-23;
	int color       = source[6]&0x3f;
	int xscale      = source[7]&0x07;   /* 0x0 = smallest; 0x7 = biggest */

	/* adjust placement for small sprites */
	if( xsize==0 && xflip ) xpos -= 16;
	if( ysize==0 && yflip==0 ) ypos += 16;

	if( m_gfx_type == 1 )
	{
		/* Shanghai Kid */
		switch( bank&0x30 )
		{
		case 0x00:
		case 0x10:
			tile += 0x40*(bank&0xf);
			break;

		case 0x20:
			tile += 0x40*((bank&0x3)|0x10);
			break;

		case 0x30:
			tile += 0x40*((bank&0x3)|0x14);
			break;
		}
		bank_index = 0;
		transparent_pen = 3;
	}
	else
	{
		/* Chinese Hero */
		color >>= 1;

		// It's needed in level 7 to hide "bogus" sprites. Is it a sprite disable flag or an end sprite list flag?
		if(color == 0)
			return;

		switch( bank>>2 )
		{
		case 0x0: bank_index = 0; break;
		case 0x9: bank_index = 1; break;
		case 0x6: bank_index = 2; break;
		case 0xf: bank_index = 3; break;
		default:
			bank_index = 0;
			break;
		}

		if( bank&0x01 ) tile += 0x40;

		transparent_pen = 7;
	}

	gfx = m_gfxdecode->gfx(1+bank_index);

	width = (xscale+1)*2;
	height = (yscale+1)*2;

	/* center zoomed sprites */
	xpos += (16-width)*(xsize+1)/2;
	ypos += (16-height)*(ysize+1)/2;

	for( r=0; r<=ysize; r++ )
	{
		for( c=0; c<=xsize; c++ )
		{
			sx = xpos+(c^xflip)*width;
			sy = ypos+(r^yflip)*height;

				gfx->zoom_transpen(
				bitmap,
				cliprect,
				tile+c*8+r,
				color,
				xflip,yflip,
				sx,sy,
				(width<<16)/16, (height<<16)/16,transparent_pen );

			// wrap around y

				gfx->zoom_transpen(
				bitmap,
				cliprect,
				tile+c*8+r,
				color,
				xflip,yflip,
				sx,sy+256,
				(width<<16)/16, (height<<16)/16,transparent_pen );
		}
	}
}

void shangkid_state::shangkid_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8 *source, *finish;

	finish = m_spriteram;
	source = m_spriteram+0x200;
	while( source>finish ){
		source -= 8;
		draw_sprite(source, bitmap,cliprect );
	}
}

UINT32 shangkid_state::screen_update_shangkid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flipscreen = m_videoreg[1]&0x80;
	m_background->set_flip(flipscreen?(TILEMAP_FLIPX|TILEMAP_FLIPY):0 );
	m_background->set_scrollx(0,m_videoreg[0]-40 );
	m_background->set_scrolly(0,m_videoreg[2]+0x10 );

	m_background->draw(screen, bitmap, cliprect, 0,0 );
	shangkid_draw_sprites(bitmap,cliprect );
	m_background->draw(screen, bitmap, cliprect, 1,0 ); /* high priority tiles */
	return 0;
}


PALETTE_INIT_MEMBER(shangkid_state,dynamski)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		UINT16 data = (color_prom[i | 0x20] << 8) | color_prom[i];
		rgb_t color = rgb_t(pal5bit(data >> 1), pal5bit(data >> 6), pal5bit(data >> 11));

		palette.set_indirect_color(i, color);
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x40;

	/* characters */
	for (i = 0; i < 0x40; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		palette.set_pen_indirect(i, ctabentry);
	}

	/* sprites */
	for (i = 0x40; i < 0x80; i++)
	{
		UINT8 ctabentry = (color_prom[(i - 0x40) + 0x100] & 0x0f) | 0x10;
		palette.set_pen_indirect(i, ctabentry);
	}
}


void shangkid_state::dynamski_draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri )
{
	int i;
	int sx,sy;
	int tile;
	int attr;
	int temp;

	for( i=0; i<0x400; i++ )
	{
		sx = (i%32)*8;
		sy = (i/32)*8;

		if( sy<16 )
		{
			temp = sx;
			sx = sy+256+16;
			sy = temp;
		}
		else if( sy>=256-16 )
		{
			temp = sx;
			sx = sy-256+16;
			sy = temp;
		}
		else
		{
			sx+=16;
		}

		tile = m_videoram[i];
		attr = m_videoram[i+0x400];
		/*
		    x---.----   priority?
		    -xx-.----   bank
		*/
		if( pri==0 || (attr>>7)==pri )
		{
			tile += ((attr>>5)&0x3)*256;

				m_gfxdecode->gfx(0)->transpen(
				bitmap,
				cliprect,
				tile,
				attr & 0x0f,
				0,0,//xflip,yflip,
				sx,sy,
				pri?3:-1 );
		}
	}
}

void shangkid_state::dynamski_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i;
	int sx,sy;
	int tile;
	int bank;
	int attr;
	int color;
	for( i=0x7e; i>=0x00; i-=2 )
	{
		bank = m_videoram[0x1b80+i];
		attr = m_videoram[0x1b81+i];
		tile = m_videoram[0xb80+i];
		color = m_videoram[0xb81+i];
		sy = 240-m_videoram[0x1380+i];

		sx = m_videoram[0x1381+i]-64+8+16;
		if( attr&1 ) sx += 0x100;


				m_gfxdecode->gfx(1)->transpen(
				bitmap,
				cliprect,
				bank*0x40 + (tile&0x3f),
				color,
				tile&0x80,tile&0x40, /* flipx,flipy */
				sx,sy,3 );
	}
}

UINT32 shangkid_state::screen_update_dynamski(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	dynamski_draw_background(bitmap,cliprect, 0 );
	dynamski_draw_sprites(bitmap,cliprect );
	dynamski_draw_background(bitmap,cliprect, 1 );
	return 0;
}
