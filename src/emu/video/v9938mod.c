/*
 * This file is included for a number of different situations:
 * V9938_WIDTH : can be 512 + 32 or 256 + 16
 * V9938_BPP : can be 8 or 16
 */

#if (V9938_WIDTH < 512)
	#if (V9938_BPP == 8)
		#define PEN_TYPE	UINT8
		#define FNAME(name)	v9938_##name##_8s
	#else
		#define PEN_TYPE	UINT16
		#define FNAME(name)	v9938_##name##_16s
	#endif
#else
	#if (V9938_BPP == 8)
		#define PEN_TYPE	UINT8
		#define FNAME(name)	v9938_##name##_8
	#else
		#define PEN_TYPE	UINT16
		#define FNAME(name)	v9938_##name##_16
	#endif
#endif


#define V9938_BORDER_FUNC(name) 	\
	static void FNAME (name) (PEN_TYPE *ln)

#define V9938_MODE_FUNC(name) 		\
	static void FNAME (name) (PEN_TYPE *ln, int line)

#define V9938_SPRITE_FUNC(name)		\
	static void FNAME (name) (PEN_TYPE *ln, UINT8 *col)

V9938_BORDER_FUNC (default_border)
    {
    PEN_TYPE pen;
	int	i;

    pen = Machine->pens[pal_ind16[(vdp.contReg[7]&0x0f)]];
	i = V9938_WIDTH;
	while (i--) *ln++ = pen;

	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
    }

V9938_BORDER_FUNC (graphic7_border)
	{
	PEN_TYPE pen;
	int i;

	pen = Machine->pens[pal_ind256[vdp.contReg[7]]];
	i = V9938_WIDTH;
	while (i--) *ln++ = pen;

	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
	}

V9938_BORDER_FUNC (graphic5_border)
	{
	int i;
	PEN_TYPE pen0;
#if (V9938_WIDTH > 512)
	PEN_TYPE pen1;

	pen1 = Machine->pens[pal_ind16[(vdp.contReg[7]&0x03)]];
	pen0 = Machine->pens[pal_ind16[((vdp.contReg[7]>>2)&0x03)]];
	i = (V9938_WIDTH) / 2;
	while (i--) { *ln++ = pen0; *ln++ = pen1; }
#else
	pen0 = Machine->pens[pal_ind16[((vdp.contReg[7]>>2)&0x03)]];
	i = V9938_WIDTH;
	while (i--) *ln++ = pen0;
#endif
	vdp.size_now = RENDER_HIGH;
	}

V9938_MODE_FUNC (mode_text1)
	{
	int pattern, x, xx, name, xxx;
	PEN_TYPE fg, bg, pen;
	UINT8 *nametbl, *patterntbl;

	patterntbl = vdp.vram + (vdp.contReg[4] << 11);
	nametbl = vdp.vram + (vdp.contReg[2] << 10);

    fg = Machine->pens[pal_ind16[vdp.contReg[7] >> 4]];
    bg = Machine->pens[pal_ind16[vdp.contReg[7] & 15]];

	name = (line/8)*40;

	pen = Machine->pens[pal_ind16[(vdp.contReg[7]&0x0f)]];

	xxx = vdp.offset_x + 8;
#if (V9938_WIDTH > 512)
	xxx *= 2;
#endif
	while (xxx--) *ln++ = pen;

	for (x=0;x<40;x++)
		{
		pattern = patterntbl[(nametbl[name] * 8) +
			((line + vdp.contReg[23]) & 7)];
		for (xx=0;xx<6;xx++)
			{
			*ln++ = (pattern & 0x80) ? fg : bg;
#if (V9938_WIDTH > 512)
			*ln++ = (pattern & 0x80) ? fg : bg;
#endif
			pattern <<= 1;
			}
		/* width height 212, characters start repeating at the bottom */
		name = (name + 1) & 0x3ff;
		}

	xxx = (16 - vdp.offset_x) + 8;
#if (V9938_WIDTH > 512)
	xxx *= 2;
#endif
	while (xxx--) *ln++ = pen;
	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
	}

V9938_MODE_FUNC (mode_text2)
	{
	int pattern, x, charcode, name, xxx, patternmask, colourmask;
	PEN_TYPE fg, bg, fg0, bg0, pen;
	UINT8 *nametbl, *patterntbl, *colourtbl;

	patterntbl = vdp.vram + (vdp.contReg[4] << 11);
	colourtbl = vdp.vram + ((vdp.contReg[3] & 0xf8) << 6) + (vdp.contReg[10] << 14);
#if 0
	colourmask = ((vdp.contReg[3] & 7) << 5) | 0x1f; /* cause a bug in Forth+ v1.0 on Geneve */
#else
	colourmask = ((vdp.contReg[3] & 7) << 6) | 0x3f; /* verify! */
#endif
	nametbl = vdp.vram + ((vdp.contReg[2] & 0xfc) << 10);
	patternmask = ((vdp.contReg[2] & 3) << 10) | 0x3ff; /* seems correct */

    fg = Machine->pens[pal_ind16[vdp.contReg[7] >> 4]];
    bg = Machine->pens[pal_ind16[vdp.contReg[7] & 15]];
    fg0 = Machine->pens[pal_ind16[vdp.contReg[12] >> 4]];
    bg0 = Machine->pens[pal_ind16[vdp.contReg[12] & 15]];

	name = (line/8)*80;

	xxx = vdp.offset_x + 8;
	pen = Machine->pens[pal_ind16[(vdp.contReg[7]&0x0f)]];
#if (V9938_WIDTH > 512)
	xxx *= 2;
#endif
	while (xxx--) *ln++ = pen;

	for (x=0;x<80;x++)
		{
		charcode = nametbl[name&patternmask];
		if (vdp.blink)
			{
			pattern = colourtbl[(name/8)&colourmask];
			if (pattern & (0x80 >> (name & 7) ) )
				{
				pattern = patterntbl[(charcode * 8) +
					((line + vdp.contReg[23]) & 7)];

#if (V9938_WIDTH > 512)
				*ln++ = (pattern & 0x80) ? fg0 : bg0;
				*ln++ = (pattern & 0x40) ? fg0 : bg0;
				*ln++ = (pattern & 0x20) ? fg0 : bg0;
				*ln++ = (pattern & 0x10) ? fg0 : bg0;
				*ln++ = (pattern & 0x08) ? fg0 : bg0;
				*ln++ = (pattern & 0x04) ? fg0 : bg0;
#else
				*ln++ = (pattern & 0x80) ? fg0 : bg0;
				*ln++ = (pattern & 0x20) ? fg0 : bg0;
				*ln++ = (pattern & 0x08) ? fg0 : bg0;
#endif

				name++;
				continue;
				}
			}

		pattern = patterntbl[(charcode * 8) +
			((line + vdp.contReg[23]) & 7)];

#if (V9938_WIDTH > 512)
		*ln++ = (pattern & 0x80) ? fg : bg;
		*ln++ = (pattern & 0x40) ? fg : bg;
		*ln++ = (pattern & 0x20) ? fg : bg;
		*ln++ = (pattern & 0x10) ? fg : bg;
		*ln++ = (pattern & 0x08) ? fg : bg;
		*ln++ = (pattern & 0x04) ? fg : bg;
#else
		*ln++ = (pattern & 0x80) ? fg : bg;
		*ln++ = (pattern & 0x20) ? fg : bg;
		*ln++ = (pattern & 0x08) ? fg : bg;
#endif

		name++;
		}

	xxx = 16  - vdp.offset_x + 8;
#if (V9938_WIDTH > 512)
	xxx *= 2;
#endif
	while (xxx--) *ln++ = pen;
	vdp.size_now = RENDER_HIGH;
	}

V9938_MODE_FUNC (mode_multi)
	{
	UINT8 *nametbl, *patterntbl, colour;
	int name, line2, x, xx;
	PEN_TYPE pen, pen_bg;

	nametbl = vdp.vram + (vdp.contReg[2] << 10);
	patterntbl = vdp.vram + (vdp.contReg[4] << 11);

	line2 = (line - vdp.contReg[23]) & 255;
	name = (line2/8)*32;

	pen_bg = Machine->pens[pal_ind16[(vdp.contReg[7]&0x0f)]];
#if (V9938_WIDTH < 512)
	xx = vdp.offset_x;
#else
	xx = vdp.offset_x * 2;
#endif
	while (xx--) *ln++ = pen_bg;

	for (x=0;x<32;x++)
		{
		colour = patterntbl[(nametbl[name] * 8) + ((line2/4)&7)];
		pen = Machine->pens[pal_ind16[colour>>4]];
		/* eight pixels */
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
#if (V9938_WIDTH > 512)
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
#endif
		pen = Machine->pens[pal_ind16[colour&15]];
		/* eight pixels */
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
#if (V9938_WIDTH > 512)
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
		*ln++ = pen;
#endif
		name++;
		}

	xx = 16 - vdp.offset_x;
#if (V9938_WIDTH > 512)
	xx *= 2;
#endif
	while (xx--) *ln++ = pen_bg;
	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
	}

V9938_MODE_FUNC (mode_graphic1)
	{
	PEN_TYPE fg, bg, pen;
	UINT8 *nametbl, *patterntbl, *colourtbl;
	int pattern, x, xx, line2, name, charcode, colour, xxx;

	nametbl = vdp.vram + (vdp.contReg[2] << 10);
	colourtbl = vdp.vram + (vdp.contReg[3] << 6) + (vdp.contReg[10] << 14);
	patterntbl = vdp.vram + (vdp.contReg[4] << 11);

	line2 = (line - vdp.contReg[23]) & 255;

	name = (line2/8)*32;

	pen = Machine->pens[pal_ind16[(vdp.contReg[7]&0x0f)]];
#if (V9938_WIDTH < 512)
	xxx = vdp.offset_x;
#else
	xxx = vdp.offset_x * 2;
#endif
	while (xxx--) *ln++ = pen;

	for (x=0;x<32;x++)
		{
		charcode = nametbl[name];
		colour = colourtbl[charcode/8];
		fg = Machine->pens[pal_ind16[colour>>4]];
		bg = Machine->pens[pal_ind16[colour&15]];
		pattern = patterntbl[charcode * 8 + (line2 & 7)];

		for (xx=0;xx<8;xx++)
			{
			*ln++ = (pattern & 0x80) ? fg : bg;
#if (V9938_WIDTH > 512)
			*ln++ = (pattern & 0x80) ? fg : bg;
#endif
			pattern <<= 1;
			}
		name++;
		}

	xx = 16 - vdp.offset_x;
#if (V9938_WIDTH > 512)
	xx *= 2;
#endif
	while (xx--) *ln++ = pen;
	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
	}

V9938_MODE_FUNC (mode_graphic23)
	{
	PEN_TYPE fg, bg, pen;
	UINT8 *nametbl, *patterntbl, *colourtbl;
	int pattern, x, xx, line2, name, charcode,
		colour, colourmask, patternmask, xxx;

	colourmask = (vdp.contReg[3] & 0x7f) * 8 | 7;
	patternmask = (vdp.contReg[4] & 0x03) * 256 | (colourmask & 255);

	nametbl = vdp.vram + (vdp.contReg[2] << 10);
 	colourtbl = vdp.vram + ((vdp.contReg[3] & 0x80) << 6) + (vdp.contReg[10] << 14);
	patterntbl = vdp.vram + ((vdp.contReg[4] & 0x3c) << 11);

	line2 = (line + vdp.contReg[23]) & 255;
	name = (line2/8)*32;

	pen = Machine->pens[pal_ind16[(vdp.contReg[7]&0x0f)]];
#if (V9938_WIDTH < 512)
	xxx = vdp.offset_x;
#else
	xxx = vdp.offset_x * 2;
#endif
	while (xxx--) *ln++ = pen;

	for (x=0;x<32;x++)
		{
		charcode = nametbl[name] + (line2&0xc0)*4;
		colour = colourtbl[(charcode&colourmask)*8+(line2&7)];
		pattern = patterntbl[(charcode&patternmask)*8+(line2&7)];
        fg = Machine->pens[pal_ind16[colour>>4]];
        bg = Machine->pens[pal_ind16[colour&15]];
		for (xx=0;xx<8;xx++)
			{
			*ln++ = (pattern & 0x80) ? fg : bg;
#if (V9938_WIDTH > 512)
			*ln++ = (pattern & 0x80) ? fg : bg;
#endif
            pattern <<= 1;
			}
		name++;
		}

	xx = 16 - vdp.offset_x;
#if (V9938_WIDTH > 512)
	xx *= 2;
#endif
	while (xx--) *ln++ = pen;
	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
	}

V9938_MODE_FUNC (mode_graphic4)
	{
	UINT8 *nametbl, colour;
	int line2, linemask, x, xx;
	PEN_TYPE pen, pen_bg;

	linemask = ((vdp.contReg[2] & 0x1f) << 3) | 7;

	line2 = ((line + vdp.contReg[23]) & linemask) & 255;

	nametbl = vdp.vram + ((vdp.contReg[2] & 0x40) << 10) + line2 * 128;
	if ( (vdp.contReg[2] & 0x20) && (V9938_SECOND_FIELD) )
		nametbl += 0x8000;

	pen_bg = Machine->pens[pal_ind16[(vdp.contReg[7]&0x0f)]];
#if (V9938_WIDTH < 512)
	xx = vdp.offset_x;
#else
	xx = vdp.offset_x * 2;
#endif
	while (xx--) *ln++ = pen_bg;

	for (x=0;x<128;x++)
		{
		colour = *nametbl++;
        pen = Machine->pens[pal_ind16[colour>>4]];
		*ln++ = pen;
#if (V9938_WIDTH > 512)
		*ln++ = pen;
#endif
        pen = Machine->pens[pal_ind16[colour&15]];
		*ln++ = pen;
#if (V9938_WIDTH > 512)
		*ln++ = pen;
#endif
		}

	xx = 16 - vdp.offset_x;
#if (V9938_WIDTH > 512)
	xx *= 2;
#endif
	while (xx--) *ln++ = pen_bg;
	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
	}

V9938_MODE_FUNC (mode_graphic5)
	{
	UINT8 *nametbl, colour;
	int line2, linemask, x, xx;
	PEN_TYPE pen_bg0[4];
#if (V9938_WIDTH > 512)
	PEN_TYPE pen_bg1[4];
#endif

	linemask = ((vdp.contReg[2] & 0x1f) << 3) | 7;

	line2 = ((line + vdp.contReg[23]) & linemask) & 255;

	nametbl = vdp.vram + ((vdp.contReg[2] & 0x40) << 10) + line2 * 128;
	if ( (vdp.contReg[2] & 0x20) && (V9938_SECOND_FIELD) )
		nametbl += 0x8000;

#if (V9938_WIDTH > 512)
	pen_bg1[0] = Machine->pens[pal_ind16[(vdp.contReg[7]&0x03)]];
	pen_bg0[0] = Machine->pens[pal_ind16[((vdp.contReg[7]>>2)&0x03)]];

	xx = vdp.offset_x;
	while (xx--) { *ln++ = pen_bg0[0]; *ln++ = pen_bg1[0]; }

	x = (vdp.contReg[8] & 0x20) ? 0 : 1;

    for (;x<4;x++)
		{
		pen_bg0[x] = Machine->pens[pal_ind16[x]];
		pen_bg1[x] = Machine->pens[pal_ind16[x]];
		}

	for (x=0;x<128;x++)
		{
		colour = *nametbl++;

        *ln++ = pen_bg0[colour>>6];
		*ln++ = pen_bg1[(colour>>4)&3];
        *ln++ = pen_bg0[(colour>>2)&3];
       	*ln++ = pen_bg1[(colour&3)];
		}

	pen_bg1[0] = Machine->pens[pal_ind16[(vdp.contReg[7]&0x03)]];
	pen_bg0[0] = Machine->pens[pal_ind16[((vdp.contReg[7]>>2)&0x03)]];
	xx = 16 - vdp.offset_x;
	while (xx--) { *ln++ = pen_bg0[0]; *ln++ = pen_bg1[0]; }
#else
	pen_bg0[0] = Machine->pens[pal_ind16[((vdp.contReg[7]>>2)&0x03)]];

	x = (vdp.contReg[8] & 0x20) ? 0 : 1;

    for (;x<4;x++)
		pen_bg0[x] = Machine->pens[pal_ind16[x]];

	xx = vdp.offset_x;
	while (xx--) *ln++ = pen_bg0[0];

	for (x=0;x<128;x++)
		{
		colour = *nametbl++;
        *ln++ = pen_bg0[colour>>6];
        *ln++ = pen_bg0[(colour>>2)&3];
		}

	pen_bg0[0] = Machine->pens[pal_ind16[((vdp.contReg[7]>>2)&0x03)]];
	xx = 16 - vdp.offset_x;
	while (xx--) *ln++ = pen_bg0[0];
#endif
	vdp.size_now = RENDER_HIGH;
	}

V9938_MODE_FUNC (mode_graphic6)
	{
    UINT8 colour;
    int line2, linemask, x, xx, nametbl;
    PEN_TYPE pen_bg, fg0;
#if (V9938_WIDTH > 512)
	PEN_TYPE fg1;
#endif

    linemask = ((vdp.contReg[2] & 0x1f) << 3) | 7;

	line2 = ((line + vdp.contReg[23]) & linemask) & 255;

    nametbl = line2 << 8 ;
   	if ( (vdp.contReg[2] & 0x20) && (V9938_SECOND_FIELD) )
        nametbl += 0x10000;

	pen_bg = Machine->pens[pal_ind16[(vdp.contReg[7]&0x0f)]];
#if (V9938_WIDTH < 512)
	xx = vdp.offset_x;
#else
	xx = vdp.offset_x * 2;
#endif
	while (xx--) *ln++ = pen_bg;

	if (vdp.contReg[2] & 0x40)
		{
		for (x=0;x<32;x++)
			{
			nametbl++;
			colour = vdp.vram[((nametbl&1) << 16) | (nametbl>>1)];
        	fg0 = Machine->pens[pal_ind16[colour>>4]];
#if (V9938_WIDTH < 512)
			*ln++ = fg0; *ln++ = fg0;
			*ln++ = fg0; *ln++ = fg0;
			*ln++ = fg0; *ln++ = fg0;
			*ln++ = fg0; *ln++ = fg0;
#else
        	fg1 = Machine->pens[pal_ind16[colour&15]];
			*ln++ = fg0; *ln++ = fg1; *ln++ = fg0; *ln++ = fg1;
			*ln++ = fg0; *ln++ = fg1; *ln++ = fg0; *ln++ = fg1;
			*ln++ = fg0; *ln++ = fg1; *ln++ = fg0; *ln++ = fg1;
			*ln++ = fg0; *ln++ = fg1; *ln++ = fg0; *ln++ = fg1;
#endif
			nametbl += 7;
			}
		}
	else
		{
		for (x=0;x<256;x++)
			{
			colour = vdp.vram[((nametbl&1) << 16) | (nametbl>>1)];
        	*ln++ = Machine->pens[pal_ind16[colour>>4]];
#if (V9938_WIDTH > 512)
        	*ln++ = Machine->pens[pal_ind16[colour&15]];
#endif
			nametbl++;
        	}
		}

	xx = 16 - vdp.offset_x;
#if (V9938_WIDTH > 512)
	xx *= 2;
#endif
	while (xx--) *ln++ = pen_bg;
	vdp.size_now = RENDER_HIGH;
	}

V9938_MODE_FUNC (mode_graphic7)
	{
    UINT8 colour;
    int line2, linemask, x, xx, nametbl;
    PEN_TYPE pen, pen_bg;

   	linemask = ((vdp.contReg[2] & 0x1f) << 3) | 7;

	line2 = ((line + vdp.contReg[23]) & linemask) & 255;

	nametbl = line2 << 8;
   	if ( (vdp.contReg[2] & 0x20) && (V9938_SECOND_FIELD) )
		nametbl += 0x10000;

	pen_bg = Machine->pens[pal_ind256[vdp.contReg[7]]];
#if (V9938_WIDTH < 512)
	xx = vdp.offset_x;
#else
	xx = vdp.offset_x * 2;
#endif
	while (xx--) *ln++ = pen_bg;

	if (vdp.contReg[2] & 0x40)
		{
		for (x=0;x<32;x++)
			{
			nametbl++;
			colour = vdp.vram[((nametbl&1) << 16) | (nametbl>>1)];
			pen = Machine->pens[pal_ind256[colour]];
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
#if (V9938_WIDTH > 512)
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
			*ln++ = pen; *ln++ = pen;
#endif
			nametbl++;
			}
		}
	else
		{
  		for (x=0;x<256;x++)
        	{
			colour = vdp.vram[((nametbl&1) << 16) | (nametbl>>1)];
			pen = Machine->pens[pal_ind256[colour]];
			*ln++ = pen;
#if (V9938_WIDTH > 512)
		 	*ln++ = pen;
#endif
			nametbl++;
       		}
		}

	xx = 16 - vdp.offset_x;
#if (V9938_WIDTH > 512)
	xx *= 2;
#endif
	while (xx--) *ln++ = pen_bg;
	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
	}

V9938_MODE_FUNC (mode_unknown)
	{
	PEN_TYPE fg, bg;
	int x;

    fg = Machine->pens[pal_ind16[vdp.contReg[7] >> 4]];
    bg = Machine->pens[pal_ind16[vdp.contReg[7] & 15]];

#if (V9938_WIDTH < 512)
	x = vdp.offset_x;
	while (x--) *ln++ = bg;

	x = 256;
	while (x--) *ln++ = fg;

	x = 16 - vdp.offset_x;
	while (x--) *ln++ = bg;
#else
	x = vdp.offset_x * 2;
	while (x--) *ln++ = bg;

	x = 512;
	while (x--) *ln++ = fg;

	x = (16 - vdp.offset_x) * 2;
	while (x--) *ln++ = bg;
#endif
	if (vdp.size_now != RENDER_HIGH) vdp.size_now = RENDER_LOW;
	}

V9938_SPRITE_FUNC (default_draw_sprite)
	{
	int i;
#if (V9938_WIDTH > 512)
	ln += vdp.offset_x * 2;
#else
	ln += vdp.offset_x;
#endif

	for (i=0;i<256;i++)
		{
		if (col[i] & 0x80)
			{
			*ln++ = Machine->pens[pal_ind16[col[i]&0x0f]];
#if (V9938_WIDTH > 512)
			*ln++ = Machine->pens[pal_ind16[col[i]&0x0f]];
#endif
			}
		else
#if (V9938_WIDTH > 512)
			ln += 2;
#else
			ln++;
#endif
		}
	}
V9938_SPRITE_FUNC (graphic5_draw_sprite)
	{
	int i;
#if (V9938_WIDTH > 512)
	ln += vdp.offset_x * 2;
#else
	ln += vdp.offset_x;
#endif

	for (i=0;i<256;i++)
		{
		if (col[i] & 0x80)
			{
			*ln++ = Machine->pens[pal_ind16[(col[i]>>2)&0x03]];
#if (V9938_WIDTH > 512)
			*ln++ = Machine->pens[pal_ind16[col[i]&0x03]];
#endif
			}
		else
#if (V9938_WIDTH > 512)
			ln += 2;
#else
			ln++;
#endif
		}
	}


V9938_SPRITE_FUNC (graphic7_draw_sprite)
	{
	static const UINT16 g7_ind16[16] = {
		0, 2, 192, 194, 48, 50, 240, 242,
		482, 7, 448, 455, 56, 63, 504, 511  };
	int i;

#if (V9938_WIDTH > 512)
	ln += vdp.offset_x * 2;
#else
	ln += vdp.offset_x;
#endif

	for (i=0;i<256;i++)
		{
		if (col[i] & 0x80)
			{
			*ln++ = Machine->pens[g7_ind16[col[i]&0x0f]];
#if (V9938_WIDTH > 512)
			*ln++ = Machine->pens[g7_ind16[col[i]&0x0f]];
#endif
			}
		else
#if (V9938_WIDTH > 512)
			ln += 2;
#else
			ln++;
#endif
		}
	}


#undef PEN_TYPE
#undef FNAME
#undef V9938_BORDER_FUNC
#undef V9938_MODE_FUNC
#undef V9938_SPRITE_FUNC
