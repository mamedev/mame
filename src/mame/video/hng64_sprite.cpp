// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner

/* Hyper NeoGeo 64 Sprite bits */

#include "includes/hng64.h"

/*
 * Sprite Format
 * ------------------
 *
 * UINT32 | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | x/y position
 *   1    | YYYY YYYY YYYY YYYY XXXX XXXX XXXX XXXX | x/y zoom (*)
 *   2    | ---- -zzz zzzz zzzz ---- ---I cccc CCCC | Z-buffer value, 'Inline' chain flag, x/y chain
 *   3    | ---- ---- pppp pppp ---- ---- ---- ---- | palette entry
 *   4    | mmmm -?fF a??? tttt tttt tttt tttt tttt | mosaic factor, unknown (**) , flip bits, additive blending, unknown (***), tile number
 *   5    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   6    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *   7    | ---- ---- ---- ---- ---- ---- ---- ---- | not used ??
 *
 * (*) Fatal Fury WA standard elements are 0x1000-0x1000, all the other games sets 0x100-0x100, related to the bit 27 of sprite regs 0?
 * (**) setted by black squares in ranking screen in Samurai Shodown 64 1, sprite disable?
 * (***) bit 22 is setted on some Fatal Fury WA snow (not all of them), bit 21 is setted on Xrally how to play elements in attract mode
 *
 * Sprite Global Registers
 * -----------------------
 *
 * UINT32 | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | ---- z--- b--- ---- ---- ---- ---- ---- | zooming mode, bpp select
 *   1    | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | global sprite offset (ss64 rankings in attract)
 *   2    | ---- ---- ---- ---- ---- ---- ---- ---- |
 *   3    | ---- ---- ---- ---- ---- ---- ---- ---- |
 *   4    | ---- ---- ---- ---- ---- ---- ---- ---- |
 * (anything else is unknown at the current time)
 * Notes:
 * [0]
 * 0xf0000000 setted in both Samurai Shodown
 * 0x00060000 always setted in all the games
 * 0x00010000 setted in POST, sprite disable?
 * [4]
 * 0x0e0 in Samurai Shodown/Xrally games, 0x1c0 in all the others, zooming factor?
 */

void hng64_state::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx;
	UINT32 *source = m_spriteram;
	UINT32 *finish = m_spriteram + 0xc000/4;

	// global offsets in sprite regs
	int spriteoffsx = (m_spriteregs[1]>>0)&0xffff;
	int spriteoffsy = (m_spriteregs[1]>>16)&0xffff;

#if 0
	for (int iii = 0; iii < 0x0f; iii++)
		osd_printf_debug("%.8x ", m_videoregs[iii]);
	osd_printf_debug("\n");
#endif

	while( source<finish )
	{
		int tileno,chainx,chainy,xflip;
		int pal,xinc,yinc,yflip;
		UINT16 xpos, ypos;
		int xdrw,ydrw;
		int chaini;
		int zbuf;
		UINT32 zoomx,zoomy;
		float foomX, foomY;
		int blend;
		int disable;



		ypos = (source[0]&0xffff0000)>>16;
		xpos = (source[0]&0x0000ffff)>>0;
		xpos += (spriteoffsx);
		ypos += (spriteoffsy);

		tileno= (source[4]&0x0007ffff);
		blend=  (source[4]&0x00800000);
		yflip=  (source[4]&0x01000000)>>24;
		xflip=  (source[4]&0x02000000)>>25;
		disable=(source[4]&0x04000000)>>26; // ss64 rankings?

		pal =(source[3]&0x00ff0000)>>16;

		chainy=(source[2]&0x0000000f);
		chainx=(source[2]&0x000000f0)>>4;
		chaini=(source[2]&0x00000100);
		zbuf = (source[2]&0x07ff0000)>>16; //?

		zoomy = (source[1]&0xffff0000)>>16;
		zoomx = (source[1]&0x0000ffff)>>0;

		#if 1
		if(zbuf == 0x7ff) //temp kludge to avoid garbage on screen
		{
			source+=8;
			continue;
		}
		#endif
		if(disable)
		{
			source+=8;
			continue;
		}

#if 0
		if (!(source[4] == 0x00000000 || source[4] == 0x000000aa))
			osd_printf_debug("unknown : %.8x %.8x %.8x %.8x %.8x %.8x %.8x %.8x \n", source[0], source[1], source[2], source[3],
				source[4], source[5], source[6], source[7]);
#endif

		/* Calculate the zoom */
		{
			int zoom_factor;

			/* FIXME: regular zoom mode has precision bugs, can be easily seen in Samurai Shodown 64 intro */
			zoom_factor = (m_spriteregs[0] & 0x08000000) ? 0x1000 : 0x100;
			if(!zoomx) zoomx=zoom_factor;
			if(!zoomy) zoomy=zoom_factor;

			/* First, prevent any possible divide by zero errors */
			foomX = (float)(zoom_factor) / (float)zoomx;
			foomY = (float)(zoom_factor) / (float)zoomy;

			zoomx = ((int)foomX) << 16;
			zoomy = ((int)foomY) << 16;

			zoomx += (int)((foomX - floor(foomX)) * (float)0x10000);
			zoomy += (int)((foomY - floor(foomY)) * (float)0x10000);
		}

		if (m_spriteregs[0] & 0x00800000) //bpp switch
		{
			gfx= m_gfxdecode->gfx(4);
		}
		else
		{
			gfx= m_gfxdecode->gfx(5);
			tileno>>=1;
			pal&=0xf;
		}

		// Accommodate for chaining and flipping
		if(xflip)
		{
			xinc=-(int)(16.0f*foomX);
			xpos-=xinc*chainx;
		}
		else
		{
			xinc=(int)(16.0f*foomX);
		}

		if(yflip)
		{
			yinc=-(int)(16.0f*foomY);
			ypos-=yinc*chainy;
		}
		else
		{
			yinc=(int)(16.0f*foomY);
		}

#if 0
		if (((source[2) & 0xffff0000) >> 16) == 0x0001)
		{
			popmessage("T %.8x %.8x %.8x %.8x %.8x", source[0], source[1], source[2], source[3], source[4]);
			//popmessage("T %.8x %.8x %.8x %.8x %.8x", source[0], source[1], source[2], source[3], source[4]);
		}
#endif

		for(ydrw=0;ydrw<=chainy;ydrw++)
		{
			for(xdrw=0;xdrw<=chainx;xdrw++)
			{
				INT16 drawx = xpos+(xinc*xdrw);
				INT16 drawy = ypos+(yinc*ydrw);

				// 0x3ff (0x200 sign bit) based on sams64_2 char select
				drawx &= 0x3ff;
				drawy &= 0x3ff;

				if (drawx&0x0200)drawx-=0x400;
				if (drawy&0x0200)drawy-=0x400;

				if (!chaini)
				{
					if (!blend) gfx->prio_zoom_transpen(bitmap,cliprect,tileno,pal,xflip,yflip,drawx,drawy,zoomx,zoomy/*0x10000*/,screen.priority(), 0,0);
					else gfx->prio_zoom_transpen_additive(bitmap,cliprect,tileno,pal,xflip,yflip,drawx,drawy,zoomx,zoomy/*0x10000*/,screen.priority(), 0,0);
					tileno++;
				}
				else // inline chain mode, used by ss64
				{
					tileno=(source[4]&0x0007ffff);
					pal =(source[3]&0x00ff0000)>>16;

					if (m_spriteregs[0] & 0x00800000) //bpp switch
					{
						gfx= m_gfxdecode->gfx(4);
					}
					else
					{
						gfx= m_gfxdecode->gfx(5);
						tileno>>=1;
						pal&=0xf;
					}

					if (!blend) gfx->prio_zoom_transpen(bitmap,cliprect,tileno,pal,xflip,yflip,drawx,drawy,zoomx,zoomy/*0x10000*/,screen.priority(), 0,0);
					else gfx->prio_zoom_transpen_additive(bitmap,cliprect,tileno,pal,xflip,yflip,drawx,drawy,zoomx,zoomy/*0x10000*/,screen.priority(), 0,0);
					source +=8;
				}

			}
		}

		if (!chaini) source +=8;
	}
}
