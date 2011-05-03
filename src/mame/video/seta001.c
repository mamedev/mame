 /*
   emulation of Seta sprite chips
   X1-001A  X1-002A (SDIP64)

   these always seem to be used as a pair, some board have been seen without the
   'A', so it's probably a chip revision / bugfix.

  used by:

  seta.c
  taito_x.c
  tnzs.c
  srmp2.c
  champbwl.c
  cchance.c

  note: the data bus is almost certainly 8-bit, dating back to the earliest
        hardware the games were used on.  the RAM arrangements changes
		slightly between games depending on how the RAM is hooked up to the
		main cpu.


 todo: unify implementation, there are currently 3 different ones when there should only be one!


  'y' low bits are NEVER buffered?

 */

#include "emu.h"
#include "seta001.h"


const device_type SETA001_SPRITE = &device_creator<seta001_device>;

seta001_device::seta001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SETA001_SPRITE, "seta001_device", tag, owner, clock)
{
}

void seta001_device::device_start()
{
	m_fg_flipxoffs = 0;
	m_fg_noflipxoffs = 0;
	
	m_fg_flipyoffs = 0;
	m_fg_noflipyoffs = 0;

	m_bg_flipyoffs = 0;
	m_bg_noflipyoffs = 0;

	m_transpen = 0;
	m_colorbase = 0;

	m_spritelimit = 0x1ff;

	m_bankcallback = NULL;
}

void seta001_device::device_reset()
{

}

READ16_DEVICE_HANDLER( spritectrl_r16 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spritectrl[offset];
}

WRITE16_DEVICE_HANDLER( spritectrl_w16 )
{
	seta001_device *dev = (seta001_device *)device;
	
	if (ACCESSING_BITS_0_7)
	{
		dev->m_spritectrl[offset] = data;
	}
}

READ8_DEVICE_HANDLER( spritectrl_r8 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spritectrl[offset];
}

WRITE8_DEVICE_HANDLER( spritectrl_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spritectrl[offset] = data;
}

// why is this needed? bug in the rendering?
WRITE8_DEVICE_HANDLER( spritectrl_w8_champbwl )
{
	seta001_device *dev = (seta001_device *)device;

	// hack??
	if (offset!=0) data ^=0xff;

	dev->m_spritectrl[offset] = data;
}

READ16_DEVICE_HANDLER( spriteylow_r16 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spriteylow[offset];
}

WRITE16_DEVICE_HANDLER( spriteylow_w16 )
{
	seta001_device *dev = (seta001_device *)device;
	
	if (ACCESSING_BITS_0_7)
	{
		dev->m_spriteylow[offset] = data;
	}
}

READ8_DEVICE_HANDLER( spriteylow_r8 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spriteylow[offset];
}

WRITE8_DEVICE_HANDLER( spriteylow_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spriteylow[offset] = data;
}


READ8_DEVICE_HANDLER( spritecodelow_r8 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spritecodelow[offset];
}

WRITE8_DEVICE_HANDLER( spritecodelow_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spritecodelow[offset] = data;
}

READ8_DEVICE_HANDLER( spritecodehigh_r8 )
{
	seta001_device *dev = (seta001_device *)device;
	return dev->m_spritecodehigh[offset];
}

WRITE8_DEVICE_HANDLER( spritecodehigh_w8 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spritecodehigh[offset] = data;
}

READ16_DEVICE_HANDLER( spritecode_r16 )
{
	seta001_device *dev = (seta001_device *)device;
	UINT16 ret = dev->m_spritecodelow[offset];
	ret |= dev->m_spritecodehigh[offset] << 8;
	return ret;
}

WRITE16_DEVICE_HANDLER( spritecode_w16 )
{
	seta001_device *dev = (seta001_device *)device;
	dev->m_spritecodelow[offset] = data & 0x00ff;
	dev->m_spritecodehigh[offset] = (data & 0xff00)>>8;
}

/***************************************************************************


                                Sprites Drawing


***************************************************************************/


void seta001_device::setac_draw_background(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int bank_size)
{
	int offs, col;
	int xoffs, yoffs;

	int total_color_codes	=	machine.config().m_gfxdecodeinfo[0].total_color_codes;

	int ctrl	=	m_spritectrl[0];
	int ctrl2	=	m_spritectrl[1];

	int flip	=	ctrl & 0x40;
	int numcol	=	ctrl2 & 0x000f;

	/* Sprites Banking and/or Sprites Buffering */
	UINT16 bank = ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? bank_size : 0 );

	int upper	=	( m_spritectrl[2] ) +
					( m_spritectrl[3] ) * 256;

	int max_y	=	0xf0;

	int col0;		/* Kludge, needed for krzybowl and kiwame */
	switch (ctrl & 0x0f)
	{
		case 0x01:	col0	=	0x4;	break;	// krzybowl
		case 0x06:	col0	=	0x8;	break;	// kiwame

		default:	col0	=	0x0;
	}

	xoffs = 0;
	yoffs = flip ? 1 : -1;

	/* Number of columns to draw - the value 1 seems special, meaning:
       draw every column */
	if (numcol == 1)
		numcol = 16;


	/* The first column is the frontmost, see twineagl test mode
        BM 071204 - first column frontmost breaks superman.
    */
//  for ( col = numcol - 1 ; col >= 0; col -- )
	for ( col = 0 ; col < numcol; col ++ )
	{
		int x	=	m_spriteylow[(col * 0x20 + 0x08 + 0x400)/2] & 0xff;
		int y	=	m_spriteylow[(col * 0x20 + 0x00 + 0x400)/2] & 0xff;

		/* draw this column */
		for ( offs = 0 ; offs < 0x40/2; offs += 2/2 )
		{
			int	code	=	(m_spritecodehigh[((col+col0)&0xf) * 0x40/2 + offs + bank + 0x800/2]<<8) | (m_spritecodelow[((col+col0)&0xf) * 0x40/2 + offs + bank + 0x800/2]);
			int	color	=	(m_spritecodehigh[((col+col0)&0xf) * 0x40/2 + offs + bank + 0xc00/2]<<8) | (m_spritecodelow[((col+col0)&0xf) * 0x40/2 + offs + bank + 0xc00/2]);

			int	flipx	=	code & 0x8000;
			int	flipy	=	code & 0x4000;

			int gfx_bank	=	(color & 0x0600) >> 9;

/*
twineagl:   010 02d 0f 10   (ship)
tndrcade:   058 02d 07 18   (start of game - yes, flip on!)
arbalest:   018 02d 0f 10   (logo)
metafox :   018 021 0f f0   (bomb)
zingzip :   010 02c 00 0f   (bomb)
wrofaero:   010 021 00 ff   (test mode)
thunderl:   010 06c 00 ff   (always?)
krzybowl:   011 028 c0 ff   (game)
kiwame  :   016 021 7f 00   (logo)
oisipuzl:   059 020 00 00   (game - yes, flip on!)

superman:   010 021 07 38   (game)
twineagl:   000 027 00 0f   (test mode)
*/

			int sx		=	  x + xoffs  + (offs & 1) * 16;
			int sy		=	-(y + yoffs) + (offs / 2) * 16;

			if (upper & (1 << col))	sx += 256;

			if (flip)
			{
				sy = max_y - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			color	=	( color >> (16-5) ) % total_color_codes;
			code	=	(code & 0x3fff) + (gfx_bank * 0x4000);

			drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
					code,
					color,
					flipx, flipy,
					((sx + 0x10) & 0x1ff) - 0x10,((sy + 8) & 0x0ff) - 8, m_transpen);
		}
	/* next column */
	}

}


void seta001_device::tnzs_draw_background( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int bank_size, UINT8* bg_flag)
{
	int screenflip = (m_spritectrl[0] & 0x40) >> 6;

	int x, y, column, tot, transpen;
	int scrollx, scrolly;
	UINT32 upperbits;
	int ctrl2 = m_spritectrl[1];

	UINT8* m = m_spritecodelow + 0x400;
	UINT8* m2 = m_spritecodehigh + 0x400;
	UINT8* scrollram = m_spriteylow+0x200;

	if ((ctrl2 ^ (~ctrl2 << 1)) & 0x40)
		m += bank_size;

	if (bg_flag[0] & 0x80)
		transpen = -1;
	else
		transpen = m_transpen;


	/* The byte at f200 is the y-scroll value for the first column.
       The byte at f204 is the LSB of x-scroll value for the first column.

       The other columns follow at 16-byte intervals.

       The 9th bit of each x-scroll value is combined into 2 bytes
       at f302-f303 */

	/* f301 controls how many columns are drawn. */
	tot = m_spritectrl[1] & 0x1f;
	if (tot == 1)
		tot = 16;

	upperbits = m_spritectrl[2] + m_spritectrl[3] * 256;

	for (column = 0; column < tot; column++)
	{
		scrollx = scrollram[column * 16 + 4] - ((upperbits & 0x01) * 256);
		if (screenflip)
			scrolly = scrollram[column * 16] + 1 - 256;
		else
			scrolly = -scrollram[column * 16] + 1;

		for (y = 0; y < 16; y++)
		{
			for (x = 0; x < 2; x++)
			{
				int code, color, flipx, flipy, sx, sy;
				int i = 32 * (column ^ 8) + 2 * y + x;

				code  = m[i];
				code +=(m2[i] & 0x3f) << 8;
				flipx = m2[i] & 0x80;
				flipy = m2[i] & 0x40;
	
				color = (m2[i + 0x200] & 0xf8) >> 3; /* colours at d600-d7ff */
				sx = x * 16;
				sy = y * 16;
				if (screenflip)
				{
					sy = 240 - sy;
					flipx = !flipx;
					flipy = !flipy;
				}

				drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						code,
						color,
						flipx,flipy,
						sx + scrollx,(sy + scrolly) & 0xff,
						transpen);

				/* wrap around x */
				drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
						code,
						color,
						flipx,flipy,
						sx + 512 + scrollx,(sy + scrolly) & 0xff,
						transpen);
			}
		}

		upperbits >>= 1;
	}
}

void seta001_device::setac_eof()
{
	// is this handling right?
	// it differs to tnzs, and thundercade has sprite flickering issues (not related to the devicification) 

	int ctrl2	=	m_spritectrl[1];

	if (~ctrl2 & 0x20)
	{
		if (ctrl2 & 0x40)
		{
			memcpy( &m_spritecodelow[0x0000], &m_spritecodelow[0x1000],0x800);
			memcpy(&m_spritecodehigh[0x0000],&m_spritecodehigh[0x1000],0x800);
		}
		else
		{
			memcpy( &m_spritecodelow[0x1000], &m_spritecodelow[0x0000],0x800);
			memcpy(&m_spritecodehigh[0x1000],&m_spritecodehigh[0x0000],0x800);
		}
	}
}

void seta001_device::tnzs_draw_foreground( running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int bank_size)
{
	int screenflip = (m_spritectrl[0] & 0x40) >> 6;
	int i;
	int ctrl2 = m_spritectrl[1];
	int xoffs, yoffs;

	int total_color_codes	=	machine.config().m_gfxdecodeinfo[0].total_color_codes;

	UINT8 *char_pointer = m_spritecodelow + 0x0000;
	UINT8 *x_pointer = m_spritecodelow + 0x0200;
	UINT8 *ctrl_pointer = m_spritecodehigh + 0x0000;
	UINT8 *color_pointer = m_spritecodehigh + 0x0200;

	xoffs	=	screenflip ? m_fg_flipxoffs : m_fg_noflipxoffs;
	yoffs	=	screenflip ? m_fg_flipyoffs : m_fg_noflipyoffs;

	if ((ctrl2 ^ (~ctrl2 << 1)) & 0x40)
	{
		char_pointer += bank_size;
		x_pointer += bank_size;
		ctrl_pointer += bank_size;
		color_pointer += bank_size;
	}

	int max_y = machine.primary_screen->height();


	/* Draw up to 512 sprites, mjyuugi has glitches if you draw them all.. */
	for (i = m_spritelimit; i >= 0; i--)
	{
		int code, color, sx, sy, flipx, flipy;

		code = char_pointer[i] + ((ctrl_pointer[i] & 0x3f) << 8);
		color = (color_pointer[i] & 0xf8) >> 3;
	
		
		sx = x_pointer[i] - ((color_pointer[i] & 1) << 8);
		sy =  (m_spriteylow[i] & 0xff);
		flipx = ctrl_pointer[i] & 0x80;
		flipy = ctrl_pointer[i] & 0x40;
		
		if (m_bankcallback) code = m_bankcallback(machine, code, color_pointer[i]);
		
		color %= total_color_codes;

		color += m_colorbase;
		
		if (screenflip)
		{
			sy = max_y - sy
				+(machine.primary_screen->height() - (machine.primary_screen->visible_area().max_y + 1));
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code,
				color,
				flipx,flipy,
				sx,
				max_y - ((sy + yoffs) & 0x0ff),m_transpen);

		/* wrap around x */
		drawgfx_transpen(bitmap,cliprect,machine.gfx[0],
				code,
				color,
				flipx,flipy,
				sx+512,
				max_y - ((sy + yoffs) & 0x0ff),m_transpen);
	}
}

	


void seta001_device::tnzs_eof( void )
{
	int ctrl2 =	m_spritectrl[1];
	if (~ctrl2 & 0x20)
	{
		// note I copy sprites only. setac implementation also copies the "floating tilemap"
		if (ctrl2 & 0x40)
		{
			memcpy( &m_spritecodelow[0x0000],  &m_spritecodelow[0x0800], 0x0400);
			memcpy(&m_spritecodehigh[0x0000], &m_spritecodehigh[0x0800], 0x0400);
		}
		else
		{
			memcpy( &m_spritecodelow[0x0800],  &m_spritecodelow[0x0000], 0x0400);
			memcpy(&m_spritecodehigh[0x0800], &m_spritecodehigh[0x0000], 0x0400);
		}

		// and I copy the "floating tilemap" BACKWARDS - this fixes kabukiz
		memcpy( &m_spritecodelow[0x0400],  &m_spritecodelow[0x0c00], 0x0400);
		memcpy(&m_spritecodehigh[0x0400], &m_spritecodehigh[0x0c00], 0x0400);
	}

}




void seta001_device::mjyuugi_draw_background(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, int bank_size)
{
	int offs, col;
	int xoffs, yoffs;

	int total_color_codes	=	machine.config().m_gfxdecodeinfo[0].total_color_codes;

	int ctrl	=	m_spritectrl[ 0 ];
	int ctrl2	=	m_spritectrl[ 1 ];

	int flip	=	ctrl & 0x40;
	int numcol	=	ctrl2 & 0x000f;

	/* Sprites Banking and/or Sprites Buffering */
	UINT16 bank = ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? bank_size : 0 );

	int upper	=	( m_spritectrl[2] ) +
					( m_spritectrl[3] ) * 256;

	int max_y	=	0xf0;

	xoffs	=	flip ? 0x10 : 0x10;
	yoffs	=	flip ? m_bg_flipyoffs : m_bg_noflipyoffs;


	/* Number of columns to draw - the value 1 seems special, meaning:
       draw every column */
	if (numcol == 1)	numcol = 16;

	/* The first column is the frontmost, see twineagl test mode */
	for (col = numcol - 1; col >= 0; col--)
	{
		int x	=	m_spriteylow[(col * 0x20 + 0x08 + 0x400)/2] & 0xff;
		int y	=	m_spriteylow[(col * 0x20 + 0x00 + 0x400)/2] & 0xff;

		/* draw this column */
		for (offs = 0; offs < 0x40/2; offs += 2/2)
		{
			int code	=	(m_spritecodehigh[((col)&0xf) * 0x40/2 + offs + bank + 0x800/2]<<8) | (m_spritecodelow[((col)&0xf) * 0x40/2 + offs + bank + 0x800/2]);
			int color	=	(m_spritecodehigh[((col)&0xf) * 0x40/2 + offs + bank + 0xc00/2]<<8) | (m_spritecodelow[((col)&0xf) * 0x40/2 + offs + bank + 0x800/2]);

			int gfxbank	=	color & 0x0200;

			int flipx	=	code & 0x8000;
			int flipy	=	code & 0x4000;

			int sx		=	  x + xoffs  + (offs & 1) * 16;
			int sy		=	-(y + yoffs) + (offs / 2) * 16 -
							(machine.primary_screen->height() - (machine.primary_screen->visible_area().max_y + 1));

			if (upper & (1 << col))	sx += 256;

			if (flip)
			{
				sy = max_y - 16 - sy - 0x100;
				flipx = !flipx;
				flipy = !flipy;
			}

			color	=	((color >> (16-5)) % total_color_codes);
			code	=	(code & 0x3fff) + (gfxbank ? 0x4000 : 0);

#define DRAWTILE(_x_, _y_)  \
			drawgfx_transpen(bitmap, \
					cliprect, machine.gfx[0], \
					code, \
					color, \
					flipx, flipy, \
					_x_, _y_, m_transpen);

			DRAWTILE(sx - 0x000, sy + 0x000)
			DRAWTILE(sx - 0x200, sy + 0x000)
			DRAWTILE(sx - 0x000, sy + 0x100)
			DRAWTILE(sx - 0x200, sy + 0x100)

		}
		/* next column */
	}
}

void seta001_device::tnzs_draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8* bg_flag )
{
	tnzs_draw_background(machine, bitmap, cliprect, 0x800, bg_flag);
	tnzs_draw_foreground(machine, bitmap, cliprect,	0x800); 
}

void seta001_device::mjyuugi_draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	mjyuugi_draw_background(machine, bitmap, cliprect, 0x1000);
	tnzs_draw_foreground(machine, bitmap, cliprect,	0x1000); 
}

void seta001_device::setac_draw_sprites(running_machine &machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	setac_draw_background(machine,bitmap,cliprect, 0x1000);
	tnzs_draw_foreground(machine, bitmap, cliprect, 0x1000);
}
