// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************
Namco System 21 Video Hardware

- sprite hardware is identical to Namco System NB1
- there are no tilemaps
- 3d graphics are managed by DSP processors
*/
/*  Palette:
        0x0000..0x1fff  sprite palettes (0x10 sets of 0x100 colors)

        0x2000..0x3fff  polygon palette bank0 (0x10 sets of 0x200 colors)
            (in starblade, some palette animation effects are performed here)

        0x4000..0x5fff  polygon palette bank1 (0x10 sets of 0x200 colors)

        0x6000..0x7fff  polygon palette bank2 (0x10 sets of 0x200 colors)

        The polygon-dedicated color sets within a bank typically increase in
        intensity from very dark to full intensity.

        Probably the selected palette is determined by most significant bits of z-code.
        This is not yet hooked up.
    */

#include "emu.h"
#include "includes/namcos21.h"

#define FRAMEBUFFER_SIZE_IN_BYTES (sizeof(uint16_t)*NAMCOS21_POLY_FRAME_WIDTH*NAMCOS21_POLY_FRAME_HEIGHT)

READ16_MEMBER(namcos21_state::winrun_gpu_color_r)
{
	return m_winrun_color;
}

WRITE16_MEMBER(namcos21_state::winrun_gpu_color_w)
{
	COMBINE_DATA( &m_winrun_color );
}

READ16_MEMBER(namcos21_state::winrun_gpu_register_r)
{
	return m_winrun_gpu_register[offset];
}

WRITE16_MEMBER(namcos21_state::winrun_gpu_register_w)
{
	COMBINE_DATA( &m_winrun_gpu_register[offset] );
	m_screen->update_partial(m_screen->vpos());
}

WRITE16_MEMBER(namcos21_state::winrun_gpu_videoram_w)
{
	int color = data>>8;
	int mask  = data&0xff;
	int i;
	for( i=0; i<8; i++ )
	{
		if( mask&(0x01<<i) )
		{
			m_videoram[(offset+i)&0x7ffff] = color;
			m_maskram[(offset+i)&0x7ffff] = mask;
		}
	}
}

READ16_MEMBER(namcos21_state::winrun_gpu_videoram_r)
{
	return (m_videoram[offset]<<8) | m_maskram[offset];
}


VIDEO_START_MEMBER(namcos21_state,namcos21)
{
	if( m_gametype == NAMCOS21_WINRUN91 )
	{
		m_videoram = std::make_unique<uint8_t[]>(0x80000);
		m_maskram = std::make_unique<uint8_t[]>(0x80000);
	}
}

uint32_t namcos21_state::screen_update_namcos21(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//uint8_t *videoram = m_videoram.get();
	int pivot = 3;
	int pri;
	bitmap.fill(0xff, cliprect );

	m_c355spr->draw(screen, bitmap, cliprect, 2 );
	//draw(screen, bitmap, cliprect, 14 );   //driver's eyes

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0x7fc0, 0x7ffe);

	m_c355spr->draw(screen, bitmap, cliprect, 0 );
	m_c355spr->draw(screen, bitmap, cliprect, 1 );

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0, 0x7fbf);

	/* draw high priority 2d sprites */
	for( pri=pivot; pri<8; pri++ )
	{
		m_c355spr->draw(screen, bitmap, cliprect, pri );
	}
	// draw(screen, bitmap, cliprect, 15 );   //driver's eyes
	return 0;
}

uint32_t namcos21_state::screen_update_driveyes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//uint8_t *videoram = m_videoram.get();
	int pivot = 3;
	int pri;
	bitmap.fill(0xff, cliprect );

	m_c355spr->draw(screen, bitmap, cliprect, 2 );
	m_c355spr->draw(screen, bitmap, cliprect, 14 );   //driver's eyes

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0x7fc0, 0x7ffe);

	m_c355spr->draw(screen, bitmap, cliprect, 0 );
	m_c355spr->draw(screen, bitmap, cliprect, 1 );

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0, 0x7fbf);

	for (pri = pivot; pri < 8; pri++)
	{
		m_c355spr->draw(screen, bitmap, cliprect, pri);
	}

	m_c355spr->draw(screen, bitmap, cliprect, 15 );   //driver's eyes

	return 0;

}

void namcos21_state::winrun_bitmap_draw(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *videoram = m_videoram.get();
	//printf("%d %d (%d %d) - %04x %04x %04x|%04x %04x\n",cliprect.top(),cliprect.bottom(),m_screen->vpos(),m_gpu_intc->get_posirq_line(),m_winrun_gpu_register[0],m_winrun_gpu_register[2/2],m_winrun_gpu_register[4/2],m_winrun_gpu_register[0xa/2],m_winrun_gpu_register[0xc/2]);

	int yscroll = -cliprect.top()+(int16_t)m_winrun_gpu_register[0x2/2];
	int xscroll = 0;//m_winrun_gpu_register[0xc/2] >> 7;
	int base = 0x1000+0x100*(m_winrun_color&0xf);
	int sx,sy;
	for( sy=cliprect.top(); sy<=cliprect.bottom(); sy++ )
	{
		const uint8_t *pSource = &videoram[((yscroll+sy)&0x3ff)*0x200];
		uint16_t *pDest = &bitmap.pix16(sy);
		for( sx=cliprect.left(); sx<=cliprect.right(); sx++ )
		{
			int pen = pSource[(sx+xscroll) & 0x1ff];
			switch( pen )
			{
			case 0xff:
				break;
			// TODO: additive blending? winrun car select uses register [0xc] for a xscroll value
			case 0x00:
				pDest[sx] = (pDest[sx]&0x1fff)+0x4000;
				break;
			case 0x01:
				pDest[sx] = (pDest[sx]&0x1fff)+0x6000;
				break;
			default:
				pDest[sx] = base|pen;
				break;
			}
		}
	}
}


uint32_t namcos21_state::screen_update_winrun(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0xff, cliprect );

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0x7fc0, 0x7ffe);
	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0, 0x7fbf);
	winrun_bitmap_draw(bitmap,cliprect);

	//popmessage("%04x %04x %04x|%04x %04x",m_winrun_gpu_register[0],m_winrun_gpu_register[2/2],m_winrun_gpu_register[4/2],m_winrun_gpu_register[0xa/2],m_winrun_gpu_register[0xc/2]);

	return 0;
}
