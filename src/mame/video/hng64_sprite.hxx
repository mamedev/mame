// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, ElSemi, Andrew Gardner

/* Hyper NeoGeo 64 Sprite bits */

/*
 * Sprite Format
 * ------------------
 *
 * uint32_t | Bits                                    | Use
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
 ** Sprite Global Registers
 * -----------------------
 *
 * uint32_t | Bits                                    | Use
 *        | 3322 2222 2222 1111 1111 11             |
 * -------+-1098-7654-3210-9876-5432-1098-7654-3210-+----------------
 *   0    | ssss z--f b--- -aap ---- ---- ---- ---- | s = unknown, samsho  z = zooming mode, f = priority sort mode (unset set in roadedge ingame) b = bpp select a = always, p = post, disable?
 *   1    | yyyy yyyy yyyy yyyy xxxx xxxx xxxx xxxx | global sprite offset (ss64 rankings in attract, roadedge HUD scroll when starting game)
 *   2    | ---- ---- ---- uuuu uuuu uuuu uuuu uuuu | u = unknown, set to 0x000fffff in roadedge ingame, bbust2, samsho - also possible depthfilter related
 *   3    | ---- ---- ---- ---- ---- ---- ---- ---- |
 *   4    | ---- ---- ---- ---- ---- ---- ---- ---- |
 * (anything else is unknown at the current time)
 * Notes:
 * [4]
 * 0x0e0 in Samurai Shodown/Xrally games, 0x1c0 in all the others, zooming factor?

 */

void hng64_state::draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx;
	uint32_t *source = m_spriteram;
	uint32_t *finish = m_spriteram + 0xc000/4;

	// global offsets in sprite regs
	int spriteoffsx = (m_spriteregs[1]>>0)&0xffff;
	int spriteoffsy = (m_spriteregs[1]>>16)&0xffff;

	// This flips between ingame and other screens for roadedge, where the sprites which are filtered definitely needs to change and the game explicitly swaps the values in the sprite list at the same time.
	// m_spriteregs[2] could also play a part as it also flips between 0x00000000 and 0x000fffff at the same time
	// Samsho games also set the upper 3 bits which could be related, samsho games still have some unwanted sprites (but also use the other 'sprite clear' mechanism)
	// Could also be draw order related, check if it inverts the z value?
	bool zsort = !(m_spriteregs[0] & 0x01000000);

#if 0
	for (int iii = 0; iii < 0x0f; iii++)
		osd_printf_debug("%.8x ", m_videoregs[iii]);
	osd_printf_debug("\n");
#endif

	// start with empty list
	m_spritelist.clear();

	while(source < finish)
	{
		if (source[4]&0x04000000) // disable bit, ss64 rankings ?
		{
			source += 8;
		}
		else
		{
			if ((!zsort && (source[2]&0x07ff0000) != 0x07ff0000) || (zsort && (source[2]&0x07ff0000) != 0))
			{
				m_spritelist.emplace_back((source[2]&0x7ff0000)>>16, source);
				if (source[2]&0x00000100) // inline chain mode
					source += 8 * (1 + (source[2]&0x0000000f)) * (1 + ((source[2]&0x000000f0)>>4));
				else
					source += 8;
			}
			else
				source += 8;
		}
	}

	if (zsort)
		std::stable_sort(m_spritelist.begin(), m_spritelist.end(), [] (auto const &a, auto const &b) { return a.first > b.first; });

	for(auto it : m_spritelist)
	{
		source = it.second;

		int tileno,chainx,chainy,xflip;
		int pal,xinc,yinc,yflip;
		uint16_t xpos, ypos;
		int xdrw,ydrw;
		int chaini;
		uint32_t zoomx,zoomy;
		float foomX, foomY;
		int blend;

		ypos = (source[0]&0xffff0000)>>16;
		xpos = (source[0]&0x0000ffff)>>0;
		xpos += (spriteoffsx);
		ypos += (spriteoffsy);

		tileno= (source[4]&0x0007ffff);
		blend=  (source[4]&0x00800000);
		yflip=  (source[4]&0x01000000)>>24;
		xflip=  (source[4]&0x02000000)>>25;

		pal =(source[3]&0x00ff0000)>>16;

		chainy=(source[2]&0x0000000f);
		chainx=(source[2]&0x000000f0)>>4;
		chaini=(source[2]&0x00000100);

		zoomy = (source[1]&0xffff0000)>>16;
		zoomx = (source[1]&0x0000ffff)>>0;

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
				int16_t drawx = xpos+(xinc*xdrw);
				int16_t drawy = ypos+(yinc*ydrw);

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
	}
}
