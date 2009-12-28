static void triangle_ns_nt_nz(UINT32 w1, UINT32 w2)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int sign_dxhdy = 0;

	int xfrac = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];
	w5 = rdp_cmd_data[rdp_cmd_cur+4];
	w6 = rdp_cmd_data[rdp_cmd_cur+5];
	w7 = rdp_cmd_data[rdp_cmd_cur+6];
	w8 = rdp_cmd_data[rdp_cmd_cur+7];

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	span[0].dymax = 0;

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl;
			xleft_inc = dxldy >> 2;
		}

		xstart = xleft >> 16;
		xend = xright >> 16;
		j = k >> 2;
		spix = k & 3;

		if (k >= 0 && k < 0x1000)
		{
			int m = 0;
			int n = 0;
			int length = 0;
			min = 0; max = 3;
			if (j == yhpix)
			{
				min = yh & 3;
			}
			if (j == ylpix)
			{
				max = yl & 3;
			}
			if (spix >= min && spix <= max)
			{
				if (spix == min)
				{
					minxmx = maxxmx = xstart;
					minxhx = maxxhx = xend;
				}
				else
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}
			}

			if (spix == max)
			{
				if (flip)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}
				else
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}
			}

			length = flip ? (xstart - xend) : (xend - xstart);

			if (spix == ldflag)
			{
				xfrac = ((xright >> 8) & 0xff);
			}

			m = flip ? (xend+1) : (xend-1);

			if (k >= yh && length >= 0 && k <= yl)
			{
				if (xstart>=0 && xstart <1024)
				{
					if (!flip)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					else
					{
						span[j].cvg[xstart] += addright(xleft);
					}
				}
				if (xend>=0 && xend<1024)
				{
					if (xstart != xend)
					{
						if (!flip)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] += addleft(xright);
						}
					}
					else
					{
						if (!flip)
						{
							span[j].cvg[xend] -= (2 - addright(xright));
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
						}
						if (span[j].cvg[xend] > 200)
						{
							span[j].cvg[xend] = 0;
						}
					}
				}
				for (n = 0; n < (length - 1); n++)
				{
					if (m>=0 && m < 640)
					{
						span[j].cvg[m] += 2;
					}

					m += m_inc;
				}
			}
		}

		xleft += xleft_inc;
		xright += xright_inc;
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:
			if(flip)
			{
				render_spans_16_ns_nt_nz_f(yh>>2, yl>>2, &tile[0]); break;
			}
			else
			{
				render_spans_16_ns_nt_nz_nf(yh>>2, yl>>2, &tile[0]); break;
			}
		case PIXEL_SIZE_32BIT:	render_spans_32(yh>>2, yl>>2, &tile[0], 0, 0, 0, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
	}
}

static void triangle_ns_nt_z(UINT32 w1, UINT32 w2)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int z = 0;
	int dzdx = 0;
	int dzdy = 0;
	int dzde = 0;
	int tilenum;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	int dzdy_dz, dzdx_dz;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int dzdiff = 0;
	int sign_dxhdy = 0;

	int dzdeh = 0, dzdyh = 0;
	int do_offset = 0;

	int xfrac = 0;
	int dzeoff = 0;

	int dzdxh = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

	int zbuffer_base = rdp_cmd_cur + 8;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];
	w5 = rdp_cmd_data[rdp_cmd_cur+4];
	w6 = rdp_cmd_data[rdp_cmd_cur+5];
	w7 = rdp_cmd_data[rdp_cmd_cur+6];
	w8 = rdp_cmd_data[rdp_cmd_cur+7];

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);
	tilenum = (w1 >> 16) & 0x7;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	z    = rdp_cmd_data[zbuffer_base+0];
	dzdx = rdp_cmd_data[zbuffer_base+1];
	dzde = rdp_cmd_data[zbuffer_base+2];
	dzdy = rdp_cmd_data[zbuffer_base+3];

	span[0].dz.w = dzdx;
	dzdy_dz = (dzdy >> 16) & 0xffff;
	dzdx_dz = (dzdx >> 16) & 0xffff;
	span[0].dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	span[0].dzpix = normalize_dzpix(span[0].dzpix);

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		dzdeh = dzde >> 9;	dzdyh = dzdy >> 9;

		dzdiff = (dzdeh*3 - dzdyh*3) << 7;
	}
	else
	{
		dzdiff = 0;
	}

	if (do_offset)
	{
		dzeoff = (dzdeh*3) << 7;
	}
	else
	{
		dzeoff = 0;
	}

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)
#define setvalues() {					\
			addvalues();				\
			adjust_attr();	\
}

	dzdxh = dzdx >> 8;

#define adjust_attr()		\
{							\
			span[j].z.w = z + dzdiff - (xfrac * dzdxh);				\
}

#define addvalues() {	\
			z += dzde; \
}

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl;
			xleft_inc = dxldy >> 2;
		}

		xstart = xleft >> 16;
		xend = xright >> 16;
		j = k >> 2;
		spix = k & 3;

		if (k >= 0 && k < 0x1000)
		{
			int m = 0;
			int n = 0;
			int length = 0;
			min = 0; max = 3;
			if (j == yhpix)
			{
				min = yh & 3;
			}
			if (j == ylpix)
			{
				max = yl & 3;
			}
			if (spix >= min && spix <= max)
			{
				if (spix == min)
				{
					minxmx = maxxmx = xstart;
					minxhx = maxxhx = xend;
				}
				else
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}
			}

			if (spix == max)
			{
				if (flip)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}
				else
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}
			}

			length = flip ? (xstart - xend) : (xend - xstart);

			if (spix == ldflag)
			{
				xfrac = ((xright >> 8) & 0xff);
				adjust_attr();
			}

			m = flip ? (xend+1) : (xend-1);

			if (k >= yh && length >= 0 && k <= yl)
			{
				if (xstart>=0 && xstart <1024)
				{
					if (!flip)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					else
					{
						span[j].cvg[xstart] += addright(xleft);
					}
				}
				if (xend>=0 && xend<1024)
				{
					if (xstart != xend)
					{
						if (!flip)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] += addleft(xright);
						}
					}
					else
					{
						if (!flip)
						{
							span[j].cvg[xend] -= (2 - addright(xright));
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
						}
						if (span[j].cvg[xend] > 200)
						{
							span[j].cvg[xend] = 0;
						}
					}
				}
				for (n = 0; n < (length - 1); n++)
				{
					if (m>=0 && m < 640)
					{
						span[j].cvg[m] += 2;
					}

					m += m_inc;
				}
			}
		}

		if (spix == 3)
		{
			addvalues();
		}
		xleft += xleft_inc;
		xright += xright_inc;
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:
			if(flip)
			{
				render_spans_16_ns_nt_z_f(yh>>2, yl>>2, &tile[0]); break;
			}
			else
			{
				render_spans_16_ns_nt_z_nf(yh>>2, yl>>2, &tile[0]); break;
			}
		case PIXEL_SIZE_32BIT:	render_spans_32(yh>>2, yl>>2, &tile[tilenum], 0, 0, 1, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
	}
}
#undef addvalues
#undef adjust_attr
#undef addleft
#undef addright
#undef setvalues

static void triangle_ns_t_nz(UINT32 w1, UINT32 w2)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int s = 0, t = 0, w = 0;
	int dsdx = 0, dtdx = 0, dwdx = 0;
	int dsdy = 0, dtdy = 0, dwdy = 0;
	int dsde = 0, dtde = 0, dwde = 0;
	int tilenum;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int dsdiff = 0, dtdiff = 0, dwdiff = 0;
	int sign_dxhdy = 0;

	int dsdeh = 0, dtdeh = 0, dwdeh = 0, dsdyh = 0, dtdyh = 0, dwdyh = 0;
	int do_offset = 0;

	int xfrac = 0;
	int dseoff = 0, dteoff = 0, dweoff = 0;

	int dsdxh = 0, dtdxh = 0, dwdxh = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

	int texture_base = rdp_cmd_cur + 8;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];
	w5 = rdp_cmd_data[rdp_cmd_cur+4];
	w6 = rdp_cmd_data[rdp_cmd_cur+5];
	w7 = rdp_cmd_data[rdp_cmd_cur+6];
	w8 = rdp_cmd_data[rdp_cmd_cur+7];

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);
	tilenum = (w1 >> 16) & 0x7;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	s = 0;	t = 0;	w = 0;

	s    = (rdp_cmd_data[texture_base+0 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+4 ] >> 16) & 0x0000ffff);
	t    = ((rdp_cmd_data[texture_base+0 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+4 ] & 0x0000ffff);
	w    = (rdp_cmd_data[texture_base+1 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+5 ] >> 16) & 0x0000ffff);
	dsdx = (rdp_cmd_data[texture_base+2 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+6 ] >> 16) & 0x0000ffff);
	dtdx = ((rdp_cmd_data[texture_base+2 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+6 ] & 0x0000ffff);
	dwdx = (rdp_cmd_data[texture_base+3 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+7 ] >> 16) & 0x0000ffff);
	dsde = (rdp_cmd_data[texture_base+8 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+12] >> 16) & 0x0000ffff);
	dtde = ((rdp_cmd_data[texture_base+8 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+12] & 0x0000ffff);
	dwde = (rdp_cmd_data[texture_base+9 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+13] >> 16) & 0x0000ffff);
	dsdy = (rdp_cmd_data[texture_base+10] & 0xffff0000) | ((rdp_cmd_data[texture_base+14] >> 16) & 0x0000ffff);
	dtdy = ((rdp_cmd_data[texture_base+10] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+14] & 0x0000ffff);
	dwdy = (rdp_cmd_data[texture_base+11] & 0xffff0000) | ((rdp_cmd_data[texture_base+15] >> 16) & 0x0000ffff);

	span[0].ds.w = dsdx;
	span[0].dt.w = dtdx;
	span[0].dw.w = dwdx;

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		dsdeh = dsde >> 9;	dsdyh = dsdy >> 9;
		dtdeh = dtde >> 9;	dtdyh = dtdy >> 9;
		dwdeh = dwde >> 9;	dwdyh = dwdy >> 9;

		dsdiff = (dsdeh*3 - dsdyh*3) << 7;
		dtdiff = (dtdeh*3 - dtdyh*3) << 7;
		dwdiff = (dwdeh*3 - dwdyh*3) << 7;
	}
	else
	{
		dsdiff = dtdiff = dwdiff = 0;
	}

	if (do_offset)
	{
		dseoff = (dsdeh*3) << 7;
		dteoff = (dtdeh*3) << 7;
		dweoff = (dwdeh*3) << 7;
	}
	else
	{
		dseoff = dteoff = dweoff;
	}

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)
#define setvalues() {					\
			addvalues();				\
			adjust_attr();	\
}

	dsdxh = dsdx >> 8;
	dtdxh = dtdx >> 8;
	dwdxh = dwdx >> 8;

#define adjust_attr()		\
{							\
			span[j].s.w = (s + dsdiff - (xfrac * dsdxh)) & ~0x1f;				\
			span[j].t.w = (t + dtdiff - (xfrac * dtdxh)) & ~0x1f;				\
			span[j].w.w = (w + dwdiff - (xfrac * dwdxh)) & ~0x1f;				\
}

#define addvalues() {	\
			s += dsde;	\
			t += dtde;	\
			w += dwde; \
}

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl;
			xleft_inc = dxldy >> 2;
		}

		xstart = xleft >> 16;
		xend = xright >> 16;
		j = k >> 2;
		spix = k & 3;

		if (k >= 0 && k < 0x1000)
		{
			int m = 0;
			int n = 0;
			int length = 0;
			min = 0; max = 3;
			if (j == yhpix)
			{
				min = yh & 3;
			}
			if (j == ylpix)
			{
				max = yl & 3;
			}
			if (spix >= min && spix <= max)
			{
				if (spix == min)
				{
					minxmx = maxxmx = xstart;
					minxhx = maxxhx = xend;
				}
				else
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}
			}

			if (spix == max)
			{
				if (flip)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}
				else
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}
			}

			length = flip ? (xstart - xend) : (xend - xstart);

			if (spix == ldflag)
			{
				xfrac = ((xright >> 8) & 0xff);
				adjust_attr();
			}

			m = flip ? (xend+1) : (xend-1);

			if (k >= yh && length >= 0 && k <= yl)
			{
				if (xstart>=0 && xstart <1024)
				{
					if (!flip)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					else
					{
						span[j].cvg[xstart] += addright(xleft);
					}
				}
				if (xend>=0 && xend<1024)
				{
					if (xstart != xend)
					{
						if (!flip)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] += addleft(xright);
						}
					}
					else
					{
						if (!flip)
						{
							span[j].cvg[xend] -= (2 - addright(xright));
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
						}
						if (span[j].cvg[xend] > 200)
						{
							span[j].cvg[xend] = 0;
						}
					}
				}
				for (n = 0; n < (length - 1); n++)
				{
					if (m>=0 && m < 640)
					{
						span[j].cvg[m] += 2;
					}

					m += m_inc;
				}
			}
		}

		if (spix == 3)
		{
			addvalues();
		}
		xleft += xleft_inc;
		xright += xright_inc;
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:
			if(flip)
			{
				render_spans_16_ns_t_nz_f(yh>>2, yl>>2, &tile[0]); break;
			}
			else
			{
				render_spans_16_ns_t_nz_nf(yh>>2, yl>>2, &tile[0]); break;
			}
		case PIXEL_SIZE_32BIT:	render_spans_32(yh>>2, yl>>2, &tile[tilenum], 0, 1, 0, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
	}
}
#undef addvalues
#undef adjust_attr
#undef addleft
#undef addright
#undef setvalues

static void triangle_ns_t_z(UINT32 w1, UINT32 w2)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int z = 0, s = 0, t = 0, w = 0;
	int dzdx = 0, dsdx = 0, dtdx = 0, dwdx = 0;
	int dzdy = 0, dsdy = 0, dtdy = 0, dwdy = 0;
	int dzde = 0, dsde = 0, dtde = 0, dwde = 0;
	int tilenum;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	int dzdy_dz, dzdx_dz;
	int dsdylod, dtdylod;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int dsdiff = 0, dtdiff = 0, dwdiff = 0, dzdiff = 0;
	int sign_dxhdy = 0;

	int dsdeh = 0, dtdeh = 0, dwdeh = 0, dzdeh = 0, dsdyh = 0, dtdyh = 0, dwdyh = 0, dzdyh = 0;
	int do_offset = 0;

	int xfrac = 0;
	int dseoff = 0, dteoff = 0, dweoff = 0, dzeoff = 0;

	int dsdxh = 0, dtdxh = 0, dwdxh = 0, dzdxh = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

	int texture_base = rdp_cmd_cur + 8;
	int zbuffer_base = rdp_cmd_cur + 8;

	zbuffer_base += 16;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];
	w5 = rdp_cmd_data[rdp_cmd_cur+4];
	w6 = rdp_cmd_data[rdp_cmd_cur+5];
	w7 = rdp_cmd_data[rdp_cmd_cur+6];
	w8 = rdp_cmd_data[rdp_cmd_cur+7];

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);
	tilenum = (w1 >> 16) & 0x7;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	z = 0;
	s = 0;	t = 0;	w = 0;

	s    = (rdp_cmd_data[texture_base+0 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+4 ] >> 16) & 0x0000ffff);
	t    = ((rdp_cmd_data[texture_base+0 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+4 ] & 0x0000ffff);
	w    = (rdp_cmd_data[texture_base+1 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+5 ] >> 16) & 0x0000ffff);
	dsdx = (rdp_cmd_data[texture_base+2 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+6 ] >> 16) & 0x0000ffff);
	dtdx = ((rdp_cmd_data[texture_base+2 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+6 ] & 0x0000ffff);
	dwdx = (rdp_cmd_data[texture_base+3 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+7 ] >> 16) & 0x0000ffff);
	dsde = (rdp_cmd_data[texture_base+8 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+12] >> 16) & 0x0000ffff);
	dtde = ((rdp_cmd_data[texture_base+8 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+12] & 0x0000ffff);
	dwde = (rdp_cmd_data[texture_base+9 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+13] >> 16) & 0x0000ffff);
	dsdy = (rdp_cmd_data[texture_base+10] & 0xffff0000) | ((rdp_cmd_data[texture_base+14] >> 16) & 0x0000ffff);
	dtdy = ((rdp_cmd_data[texture_base+10] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+14] & 0x0000ffff);
	dwdy = (rdp_cmd_data[texture_base+11] & 0xffff0000) | ((rdp_cmd_data[texture_base+15] >> 16) & 0x0000ffff);
	z    = rdp_cmd_data[zbuffer_base+0];
	dzdx = rdp_cmd_data[zbuffer_base+1];
	dzde = rdp_cmd_data[zbuffer_base+2];
	dzdy = rdp_cmd_data[zbuffer_base+3];

	span[0].ds.w = dsdx;
	span[0].dt.w = dtdx;
	span[0].dw.w = dwdx;
	span[0].dz.w = dzdx;
	dzdy_dz = (dzdy >> 16) & 0xffff;
	dzdx_dz = (dzdx >> 16) & 0xffff;
	span[0].dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	span[0].dzpix = normalize_dzpix(span[0].dzpix);
	dsdylod = dsdy >> 16;
	dtdylod = dtdy >> 16;
	if (dsdylod & 0x20000)
	{
		dsdylod = ~dsdylod & 0x1ffff;
	}
	if (dtdylod & 0x20000)
	{
		dtdylod = ~dtdylod & 0x1ffff;
	}
	span[0].dymax = (dsdylod > dtdylod)? dsdylod : dtdylod;

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		dsdeh = dsde >> 9;	dsdyh = dsdy >> 9;
		dtdeh = dtde >> 9;	dtdyh = dtdy >> 9;
		dwdeh = dwde >> 9;	dwdyh = dwdy >> 9;
		dzdeh = dzde >> 9;	dzdyh = dzdy >> 9;

		dsdiff = (dsdeh*3 - dsdyh*3) << 7;
		dtdiff = (dtdeh*3 - dtdyh*3) << 7;
		dwdiff = (dwdeh*3 - dwdyh*3) << 7;
		dzdiff = (dzdeh*3 - dzdyh*3) << 7;
	}
	else
	{
		dsdiff = dtdiff = dwdiff = dzdiff = 0;
	}

	if (do_offset)
	{
		dseoff = (dsdeh*3) << 7;
		dteoff = (dtdeh*3) << 7;
		dweoff = (dwdeh*3) << 7;
		dzeoff = (dzdeh*3) << 7;
	}
	else
	{
		dseoff = dteoff = dweoff = dzeoff = 0;
	}

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)
#define setvalues() {					\
			addvalues();				\
			adjust_attr();	\
}

	dsdxh = dsdx >> 8;
	dtdxh = dtdx >> 8;
	dwdxh = dwdx >> 8;
	dzdxh = dzdx >> 8;

#define adjust_attr()		\
{							\
			span[j].s.w = (s + dsdiff - (xfrac * dsdxh)) & ~0x1f;				\
			span[j].t.w = (t + dtdiff - (xfrac * dtdxh)) & ~0x1f;				\
			span[j].w.w = (w + dwdiff - (xfrac * dwdxh)) & ~0x1f;				\
			span[j].z.w = z + dzdiff - (xfrac * dzdxh);				\
}

#define addvalues() {	\
			s += dsde;	\
			t += dtde;	\
			w += dwde; \
			z += dzde; \
}

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl;
			xleft_inc = dxldy >> 2;
		}

		xstart = xleft >> 16;
		xend = xright >> 16;
		j = k >> 2;
		spix = k & 3;

		if (k >= 0 && k < 0x1000)
		{
			int m = 0;
			int n = 0;
			int length = 0;
			min = 0; max = 3;
			if (j == yhpix)
			{
				min = yh & 3;
			}
			if (j == ylpix)
			{
				max = yl & 3;
			}
			if (spix >= min && spix <= max)
			{
				if (spix == min)
				{
					minxmx = maxxmx = xstart;
					minxhx = maxxhx = xend;
				}
				else
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}
			}

			if (spix == max)
			{
				if (flip)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}
				else
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}
			}

			length = flip ? (xstart - xend) : (xend - xstart);

			if (spix == ldflag)
			{
				xfrac = ((xright >> 8) & 0xff);
				adjust_attr();
			}

			m = flip ? (xend+1) : (xend-1);

			if (k >= yh && length >= 0 && k <= yl)
			{
				if (xstart>=0 && xstart <1024)
				{
					if (!flip)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					else
					{
						span[j].cvg[xstart] += addright(xleft);
					}
				}
				if (xend>=0 && xend<1024)
				{
					if (xstart != xend)
					{
						if (!flip)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] += addleft(xright);
						}
					}
					else
					{
						if (!flip)
						{
							span[j].cvg[xend] -= (2 - addright(xright));
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
						}
						if (span[j].cvg[xend] > 200)
						{
							span[j].cvg[xend] = 0;
						}
					}
				}
				for (n = 0; n < (length - 1); n++)
				{
					if (m>=0 && m < 640)
					{
						span[j].cvg[m] += 2;
					}

					m += m_inc;
				}
			}
		}

		if (spix == 3)
		{
			addvalues();
		}
		xleft += xleft_inc;
		xright += xright_inc;
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:
			if(flip)
			{
				render_spans_16_ns_t_z_f(yh>>2, yl>>2, &tile[0]); break;
			}
			else
			{
				render_spans_16_ns_t_z_nf(yh>>2, yl>>2, &tile[0]); break;
			}
		case PIXEL_SIZE_32BIT:	render_spans_32(yh>>2, yl>>2, &tile[tilenum], 0, 1, 1, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
	}
}
#undef addvalues
#undef adjust_attr
#undef addleft
#undef addright
#undef setvalues

static void triangle_s_nt_nz(UINT32 w1, UINT32 w2)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int r = 0, g = 0, b = 0, a = 0;
	int dr, dg, db, da;
	int drdx = 0, dgdx = 0, dbdx = 0, dadx = 0;
	int drdy = 0, dgdy = 0, dbdy = 0, dady = 0;
	int drde = 0, dgde = 0, dbde = 0, dade = 0;
	int tilenum;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int drdiff = 0, dgdiff = 0, dbdiff = 0, dadiff = 0;
	int sign_dxhdy = 0;

	int drdeh = 0, dgdeh = 0, dbdeh = 0, dadeh = 0, drdyh = 0, dgdyh = 0, dbdyh = 0, dadyh = 0;
	int do_offset = 0;

	int xfrac = 0;
	int dreoff = 0, dgeoff = 0, dbeoff = 0, daeoff = 0;

	int drdxh = 0, dgdxh = 0, dbdxh = 0, dadxh = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

	int shade_base = rdp_cmd_cur + 8;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];
	w5 = rdp_cmd_data[rdp_cmd_cur+4];
	w6 = rdp_cmd_data[rdp_cmd_cur+5];
	w7 = rdp_cmd_data[rdp_cmd_cur+6];
	w8 = rdp_cmd_data[rdp_cmd_cur+7];

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);
	tilenum = (w1 >> 16) & 0x7;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	dr = 0;		dg = 0;		db = 0;		da = 0;

	r    = (rdp_cmd_data[shade_base+0 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+4 ] >> 16) & 0x0000ffff);
	g    = ((rdp_cmd_data[shade_base+0 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+4 ] & 0x0000ffff);
	b    = (rdp_cmd_data[shade_base+1 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+5 ] >> 16) & 0x0000ffff);
	a    = ((rdp_cmd_data[shade_base+1 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+5 ] & 0x0000ffff);
	drdx = (rdp_cmd_data[shade_base+2 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+6 ] >> 16) & 0x0000ffff);
	dgdx = ((rdp_cmd_data[shade_base+2 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+6 ] & 0x0000ffff);
	dbdx = (rdp_cmd_data[shade_base+3 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+7 ] >> 16) & 0x0000ffff);
	dadx = ((rdp_cmd_data[shade_base+3 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+7 ] & 0x0000ffff);
	drde = (rdp_cmd_data[shade_base+8 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+12] >> 16) & 0x0000ffff);
	dgde = ((rdp_cmd_data[shade_base+8 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+12] & 0x0000ffff);
	dbde = (rdp_cmd_data[shade_base+9 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+13] >> 16) & 0x0000ffff);
	dade = ((rdp_cmd_data[shade_base+9 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+13] & 0x0000ffff);
	drdy = (rdp_cmd_data[shade_base+10] & 0xffff0000) | ((rdp_cmd_data[shade_base+14] >> 16) & 0x0000ffff);
	dgdy = ((rdp_cmd_data[shade_base+10] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+14] & 0x0000ffff);
	dbdy = (rdp_cmd_data[shade_base+11] & 0xffff0000) | ((rdp_cmd_data[shade_base+15] >> 16) & 0x0000ffff);
	dady = ((rdp_cmd_data[shade_base+11] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+15] & 0x0000ffff);

	span[0].dr.w = drdx & ~0x1f;
	span[0].dg.w = dgdx & ~0x1f;
	span[0].db.w = dbdx & ~0x1f;
	span[0].da.w = dadx & ~0x1f;

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		drdeh = drde >> 9;	drdyh = drdy >> 9;
		dgdeh = dgde >> 9;	dgdyh = dgdy >> 9;
		dbdeh = dbde >> 9;	dbdyh = dbdy >> 9;
		dadeh = dade >> 9;	dadyh = dady >> 9;

		drdiff = (drdeh*3 - drdyh*3) << 7;
		dgdiff = (dgdeh*3 - dgdyh*3) << 7;
		dbdiff = (dbdeh*3 - dbdyh*3) << 7;
		dadiff = (dadeh*3 - dadyh*3) << 7;
	}
	else
	{
		drdiff = dgdiff = dbdiff = dadiff;
	}

	if (do_offset)
	{
		dreoff = (drdeh*3) << 7;
		dgeoff = (dgdeh*3) << 7;
		dbeoff = (dbdeh*3) << 7;
		daeoff = (dadeh*3) << 7;
	}
	else
	{
		dreoff = dgeoff = dbeoff = daeoff;
	}

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)
#define setvalues() {					\
			addvalues();				\
			adjust_attr();	\
}

	drdxh = drdx >> 8;
	dgdxh = dgdx >> 8;
	dbdxh = dbdx >> 8;
	dadxh = dadx >> 8;

#define adjust_attr()		\
{							\
			span[j].r.w = r + drdiff - (xfrac * drdxh);				\
			span[j].g.w = g + dgdiff - (xfrac * dgdxh);				\
			span[j].b.w = b + dbdiff - (xfrac * dbdxh);				\
			span[j].a.w = a + dadiff - (xfrac * dadxh);				\
}

#define addvalues() {	\
			r += drde; \
			g += dgde; \
			b += dbde; \
			a += dade; \
}

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl;
			xleft_inc = dxldy >> 2;
		}

		xstart = xleft >> 16;
		xend = xright >> 16;
		j = k >> 2;
		spix = k & 3;

		if (k >= 0 && k < 0x1000)
		{
			int m = 0;
			int n = 0;
			int length = 0;
			min = 0; max = 3;
			if (j == yhpix)
			{
				min = yh & 3;
			}
			if (j == ylpix)
			{
				max = yl & 3;
			}
			if (spix >= min && spix <= max)
			{
				if (spix == min)
				{
					minxmx = maxxmx = xstart;
					minxhx = maxxhx = xend;
				}
				else
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}
			}

			if (spix == max)
			{
				if (flip)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}
				else
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}
			}

			length = flip ? (xstart - xend) : (xend - xstart);

			if (spix == ldflag)
			{
				xfrac = ((xright >> 8) & 0xff);
				adjust_attr();
			}

			m = flip ? (xend+1) : (xend-1);

			if (k >= yh && length >= 0 && k <= yl)
			{
				if (xstart>=0 && xstart <1024)
				{
					if (!flip)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					else
					{
						span[j].cvg[xstart] += addright(xleft);
					}
				}
				if (xend>=0 && xend<1024)
				{
					if (xstart != xend)
					{
						if (!flip)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] += addleft(xright);
						}
					}
					else
					{
						if (!flip)
						{
							span[j].cvg[xend] -= (2 - addright(xright));
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
						}
						if (span[j].cvg[xend] > 200)
						{
							span[j].cvg[xend] = 0;
						}
					}
				}
				for (n = 0; n < (length - 1); n++)
				{
					if (m>=0 && m < 640)
					{
						span[j].cvg[m] += 2;
					}

					m += m_inc;
				}
			}
		}

		if (spix == 3)
		{
			addvalues();
		}
		xleft += xleft_inc;
		xright += xright_inc;
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:
			if(flip)
			{
				render_spans_16_s_nt_nz_f(yh>>2, yl>>2, &tile[0]); break;
			}
			else
			{
				render_spans_16_s_nt_nz_nf(yh>>2, yl>>2, &tile[0]); break;
			}
		case PIXEL_SIZE_32BIT:	render_spans_32(yh>>2, yl>>2, &tile[tilenum], 1, 0, 0, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
	}
}
#undef addvalues
#undef adjust_attr
#undef addleft
#undef addright
#undef setvalues

static void triangle_s_nt_z(UINT32 w1, UINT32 w2)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int r = 0, g = 0, b = 0, a = 0, z = 0;
	int dr, dg, db, da;
	int drdx = 0, dgdx = 0, dbdx = 0, dadx = 0, dzdx = 0;
	int drdy = 0, dgdy = 0, dbdy = 0, dady = 0, dzdy = 0;
	int drde = 0, dgde = 0, dbde = 0, dade = 0, dzde = 0;
	int tilenum;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	int dzdy_dz, dzdx_dz;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int drdiff = 0, dgdiff = 0, dbdiff = 0, dadiff = 0, dzdiff = 0;
	int sign_dxhdy = 0;

	int drdeh = 0, dgdeh = 0, dbdeh = 0, dadeh = 0, dzdeh = 0, drdyh = 0, dgdyh = 0, dbdyh = 0, dadyh = 0, dzdyh = 0;
	int do_offset = 0;

	int xfrac = 0;
	int dreoff = 0, dgeoff = 0, dbeoff = 0, daeoff = 0, dzeoff = 0;

	int drdxh = 0, dgdxh = 0, dbdxh = 0, dadxh = 0, dzdxh = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

	int shade_base = rdp_cmd_cur + 8;
	int zbuffer_base = rdp_cmd_cur + 8;

	zbuffer_base += 16;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];
	w5 = rdp_cmd_data[rdp_cmd_cur+4];
	w6 = rdp_cmd_data[rdp_cmd_cur+5];
	w7 = rdp_cmd_data[rdp_cmd_cur+6];
	w8 = rdp_cmd_data[rdp_cmd_cur+7];

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);
	tilenum = (w1 >> 16) & 0x7;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	z = 0;
	dr = 0;		dg = 0;		db = 0;		da = 0;

	r    = (rdp_cmd_data[shade_base+0 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+4 ] >> 16) & 0x0000ffff);
	g    = ((rdp_cmd_data[shade_base+0 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+4 ] & 0x0000ffff);
	b    = (rdp_cmd_data[shade_base+1 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+5 ] >> 16) & 0x0000ffff);
	a    = ((rdp_cmd_data[shade_base+1 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+5 ] & 0x0000ffff);
	drdx = (rdp_cmd_data[shade_base+2 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+6 ] >> 16) & 0x0000ffff);
	dgdx = ((rdp_cmd_data[shade_base+2 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+6 ] & 0x0000ffff);
	dbdx = (rdp_cmd_data[shade_base+3 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+7 ] >> 16) & 0x0000ffff);
	dadx = ((rdp_cmd_data[shade_base+3 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+7 ] & 0x0000ffff);
	drde = (rdp_cmd_data[shade_base+8 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+12] >> 16) & 0x0000ffff);
	dgde = ((rdp_cmd_data[shade_base+8 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+12] & 0x0000ffff);
	dbde = (rdp_cmd_data[shade_base+9 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+13] >> 16) & 0x0000ffff);
	dade = ((rdp_cmd_data[shade_base+9 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+13] & 0x0000ffff);
	drdy = (rdp_cmd_data[shade_base+10] & 0xffff0000) | ((rdp_cmd_data[shade_base+14] >> 16) & 0x0000ffff);
	dgdy = ((rdp_cmd_data[shade_base+10] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+14] & 0x0000ffff);
	dbdy = (rdp_cmd_data[shade_base+11] & 0xffff0000) | ((rdp_cmd_data[shade_base+15] >> 16) & 0x0000ffff);
	dady = ((rdp_cmd_data[shade_base+11] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+15] & 0x0000ffff);
	z    = rdp_cmd_data[zbuffer_base+0];
	dzdx = rdp_cmd_data[zbuffer_base+1];
	dzde = rdp_cmd_data[zbuffer_base+2];
	dzdy = rdp_cmd_data[zbuffer_base+3];

	span[0].dr.w = drdx & ~0x1f;
	span[0].dg.w = dgdx & ~0x1f;
	span[0].db.w = dbdx & ~0x1f;
	span[0].da.w = dadx & ~0x1f;
	span[0].dz.w = dzdx;
	dzdy_dz = (dzdy >> 16) & 0xffff;
	dzdx_dz = (dzdx >> 16) & 0xffff;
	span[0].dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	span[0].dzpix = normalize_dzpix(span[0].dzpix);

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		drdeh = drde >> 9;	drdyh = drdy >> 9;
		dgdeh = dgde >> 9;	dgdyh = dgdy >> 9;
		dbdeh = dbde >> 9;	dbdyh = dbdy >> 9;
		dadeh = dade >> 9;	dadyh = dady >> 9;
		dzdeh = dzde >> 9;	dzdyh = dzdy >> 9;

		drdiff = (drdeh*3 - drdyh*3) << 7;
		dgdiff = (dgdeh*3 - dgdyh*3) << 7;
		dbdiff = (dbdeh*3 - dbdyh*3) << 7;
		dadiff = (dadeh*3 - dadyh*3) << 7;
		dzdiff = (dzdeh*3 - dzdyh*3) << 7;
	}
	else
	{
		drdiff = dgdiff = dbdiff = dadiff = dzdiff = 0;
	}

	if (do_offset)
	{
		dreoff = (drdeh*3) << 7;
		dgeoff = (dgdeh*3) << 7;
		dbeoff = (dbdeh*3) << 7;
		daeoff = (dadeh*3) << 7;
		dzeoff = (dzdeh*3) << 7;
	}
	else
	{
		dreoff = dgeoff = dbeoff = daeoff = dzeoff = 0;
	}

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)
#define setvalues() {					\
			addvalues();				\
			adjust_attr();	\
}

	drdxh = drdx >> 8;
	dgdxh = dgdx >> 8;
	dbdxh = dbdx >> 8;
	dadxh = dadx >> 8;
	dzdxh = dzdx >> 8;

#define adjust_attr()		\
{							\
			span[j].r.w = r + drdiff - (xfrac * drdxh);				\
			span[j].g.w = g + dgdiff - (xfrac * dgdxh);				\
			span[j].b.w = b + dbdiff - (xfrac * dbdxh);				\
			span[j].a.w = a + dadiff - (xfrac * dadxh);				\
			span[j].z.w = z + dzdiff - (xfrac * dzdxh);				\
}

#define addvalues() {	\
			r += drde; \
			g += dgde; \
			b += dbde; \
			a += dade; \
			z += dzde; \
}

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl;
			xleft_inc = dxldy >> 2;
		}

		xstart = xleft >> 16;
		xend = xright >> 16;
		j = k >> 2;
		spix = k & 3;

		if (k >= 0 && k < 0x1000)
		{
			int m = 0;
			int n = 0;
			int length = 0;
			min = 0; max = 3;
			if (j == yhpix)
			{
				min = yh & 3;
			}
			if (j == ylpix)
			{
				max = yl & 3;
			}
			if (spix >= min && spix <= max)
			{
				if (spix == min)
				{
					minxmx = maxxmx = xstart;
					minxhx = maxxhx = xend;
				}
				else
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}
			}

			if (spix == max)
			{
				if (flip)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}
				else
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}
			}

			length = flip ? (xstart - xend) : (xend - xstart);

			if (spix == ldflag)
			{
				xfrac = ((xright >> 8) & 0xff);
				adjust_attr();
			}

			m = flip ? (xend+1) : (xend-1);

			if (k >= yh && length >= 0 && k <= yl)
			{
				if (xstart>=0 && xstart <1024)
				{
					if (!flip)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					else
					{
						span[j].cvg[xstart] += addright(xleft);
					}
				}
				if (xend>=0 && xend<1024)
				{
					if (xstart != xend)
					{
						if (!flip)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] += addleft(xright);
						}
					}
					else
					{
						if (!flip)
						{
							span[j].cvg[xend] -= (2 - addright(xright));
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
						}
						if (span[j].cvg[xend] > 200)
						{
							span[j].cvg[xend] = 0;
						}
					}
				}
				for (n = 0; n < (length - 1); n++)
				{
					if (m>=0 && m < 640)
					{
						span[j].cvg[m] += 2;
					}

					m += m_inc;
				}
			}
		}

		if (spix == 3)
		{
			addvalues();
		}
		xleft += xleft_inc;
		xright += xright_inc;
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:
			if(flip)
			{
				render_spans_16_s_nt_z_f(yh>>2, yl>>2, &tile[0]); break;
			}
			else
			{
				render_spans_16_s_nt_z_nf(yh>>2, yl>>2, &tile[0]); break;
			}
		case PIXEL_SIZE_32BIT:	render_spans_32(yh>>2, yl>>2, &tile[tilenum], 1, 0, 1, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
	}
}
#undef addvalues
#undef adjust_attr
#undef addleft
#undef addright
#undef setvalues

static void triangle_s_t_nz(UINT32 w1, UINT32 w2)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int r = 0, g = 0, b = 0, a = 0, s = 0, t = 0, w = 0;
	int dr, dg, db, da;
	int drdx = 0, dgdx = 0, dbdx = 0, dadx = 0, dsdx = 0, dtdx = 0, dwdx = 0;
	int drdy = 0, dgdy = 0, dbdy = 0, dady = 0, dsdy = 0, dtdy = 0, dwdy = 0;
	int drde = 0, dgde = 0, dbde = 0, dade = 0, dsde = 0, dtde = 0, dwde = 0;
	int tilenum;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	int dsdylod, dtdylod;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int dsdiff = 0, dtdiff = 0, dwdiff = 0, drdiff = 0, dgdiff = 0, dbdiff = 0, dadiff = 0;
	int sign_dxhdy = 0;

	int dsdeh = 0, dtdeh = 0, dwdeh = 0, drdeh = 0, dgdeh = 0, dbdeh = 0, dadeh = 0, dsdyh = 0, dtdyh = 0, dwdyh = 0, drdyh = 0, dgdyh = 0, dbdyh = 0, dadyh = 0;
	int do_offset = 0;

	int xfrac = 0;
	int dseoff = 0, dteoff = 0, dweoff = 0, dreoff = 0, dgeoff = 0, dbeoff = 0, daeoff = 0;

	int dsdxh = 0, dtdxh = 0, dwdxh = 0, drdxh = 0, dgdxh = 0, dbdxh = 0, dadxh = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

	int shade_base = rdp_cmd_cur + 8;
	int texture_base = rdp_cmd_cur + 8;

	texture_base += 16;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];
	w5 = rdp_cmd_data[rdp_cmd_cur+4];
	w6 = rdp_cmd_data[rdp_cmd_cur+5];
	w7 = rdp_cmd_data[rdp_cmd_cur+6];
	w8 = rdp_cmd_data[rdp_cmd_cur+7];

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);
	tilenum = (w1 >> 16) & 0x7;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	s = 0;	t = 0;	w = 0;
	dr = 0;		dg = 0;		db = 0;		da = 0;

	r    = (rdp_cmd_data[shade_base+0 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+4 ] >> 16) & 0x0000ffff);
	g    = ((rdp_cmd_data[shade_base+0 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+4 ] & 0x0000ffff);
	b    = (rdp_cmd_data[shade_base+1 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+5 ] >> 16) & 0x0000ffff);
	a    = ((rdp_cmd_data[shade_base+1 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+5 ] & 0x0000ffff);
	drdx = (rdp_cmd_data[shade_base+2 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+6 ] >> 16) & 0x0000ffff);
	dgdx = ((rdp_cmd_data[shade_base+2 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+6 ] & 0x0000ffff);
	dbdx = (rdp_cmd_data[shade_base+3 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+7 ] >> 16) & 0x0000ffff);
	dadx = ((rdp_cmd_data[shade_base+3 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+7 ] & 0x0000ffff);
	drde = (rdp_cmd_data[shade_base+8 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+12] >> 16) & 0x0000ffff);
	dgde = ((rdp_cmd_data[shade_base+8 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+12] & 0x0000ffff);
	dbde = (rdp_cmd_data[shade_base+9 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+13] >> 16) & 0x0000ffff);
	dade = ((rdp_cmd_data[shade_base+9 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+13] & 0x0000ffff);
	drdy = (rdp_cmd_data[shade_base+10] & 0xffff0000) | ((rdp_cmd_data[shade_base+14] >> 16) & 0x0000ffff);
	dgdy = ((rdp_cmd_data[shade_base+10] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+14] & 0x0000ffff);
	dbdy = (rdp_cmd_data[shade_base+11] & 0xffff0000) | ((rdp_cmd_data[shade_base+15] >> 16) & 0x0000ffff);
	dady = ((rdp_cmd_data[shade_base+11] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+15] & 0x0000ffff);
	s    = (rdp_cmd_data[texture_base+0 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+4 ] >> 16) & 0x0000ffff);
	t    = ((rdp_cmd_data[texture_base+0 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+4 ] & 0x0000ffff);
	w    = (rdp_cmd_data[texture_base+1 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+5 ] >> 16) & 0x0000ffff);
	dsdx = (rdp_cmd_data[texture_base+2 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+6 ] >> 16) & 0x0000ffff);
	dtdx = ((rdp_cmd_data[texture_base+2 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+6 ] & 0x0000ffff);
	dwdx = (rdp_cmd_data[texture_base+3 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+7 ] >> 16) & 0x0000ffff);
	dsde = (rdp_cmd_data[texture_base+8 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+12] >> 16) & 0x0000ffff);
	dtde = ((rdp_cmd_data[texture_base+8 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+12] & 0x0000ffff);
	dwde = (rdp_cmd_data[texture_base+9 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+13] >> 16) & 0x0000ffff);
	dsdy = (rdp_cmd_data[texture_base+10] & 0xffff0000) | ((rdp_cmd_data[texture_base+14] >> 16) & 0x0000ffff);
	dtdy = ((rdp_cmd_data[texture_base+10] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+14] & 0x0000ffff);
	dwdy = (rdp_cmd_data[texture_base+11] & 0xffff0000) | ((rdp_cmd_data[texture_base+15] >> 16) & 0x0000ffff);

	span[0].ds.w = dsdx;
	span[0].dt.w = dtdx;
	span[0].dw.w = dwdx;
	span[0].dr.w = drdx & ~0x1f;
	span[0].dg.w = dgdx & ~0x1f;
	span[0].db.w = dbdx & ~0x1f;
	span[0].da.w = dadx & ~0x1f;
	dsdylod = dsdy >> 16;
	dtdylod = dtdy >> 16;
	if (dsdylod & 0x20000)
	{
		dsdylod = ~dsdylod & 0x1ffff;
	}
	if (dtdylod & 0x20000)
	{
		dtdylod = ~dtdylod & 0x1ffff;
	}
	span[0].dymax = (dsdylod > dtdylod)? dsdylod : dtdylod;

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		dsdeh = dsde >> 9;	dsdyh = dsdy >> 9;
		dtdeh = dtde >> 9;	dtdyh = dtdy >> 9;
		dwdeh = dwde >> 9;	dwdyh = dwdy >> 9;
		drdeh = drde >> 9;	drdyh = drdy >> 9;
		dgdeh = dgde >> 9;	dgdyh = dgdy >> 9;
		dbdeh = dbde >> 9;	dbdyh = dbdy >> 9;
		dadeh = dade >> 9;	dadyh = dady >> 9;

		dsdiff = (dsdeh*3 - dsdyh*3) << 7;
		dtdiff = (dtdeh*3 - dtdyh*3) << 7;
		dwdiff = (dwdeh*3 - dwdyh*3) << 7;
		drdiff = (drdeh*3 - drdyh*3) << 7;
		dgdiff = (dgdeh*3 - dgdyh*3) << 7;
		dbdiff = (dbdeh*3 - dbdyh*3) << 7;
		dadiff = (dadeh*3 - dadyh*3) << 7;
	}
	else
	{
		dsdiff = dtdiff = dwdiff = drdiff = dgdiff = dbdiff = dadiff = 0;
	}

	if (do_offset)
	{
		dseoff = (dsdeh*3) << 7;
		dteoff = (dtdeh*3) << 7;
		dweoff = (dwdeh*3) << 7;
		dreoff = (drdeh*3) << 7;
		dgeoff = (dgdeh*3) << 7;
		dbeoff = (dbdeh*3) << 7;
		daeoff = (dadeh*3) << 7;
	}
	else
	{
		dseoff = dteoff = dweoff = dreoff = dgeoff = dbeoff = daeoff = 0;
	}

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)
#define setvalues() {					\
			addvalues();				\
			adjust_attr();	\
}

	dsdxh = dsdx >> 8;
	dtdxh = dtdx >> 8;
	dwdxh = dwdx >> 8;
	drdxh = drdx >> 8;
	dgdxh = dgdx >> 8;
	dbdxh = dbdx >> 8;
	dadxh = dadx >> 8;

#define adjust_attr()		\
{							\
			span[j].s.w = (s + dsdiff - (xfrac * dsdxh)) & ~0x1f;				\
			span[j].t.w = (t + dtdiff - (xfrac * dtdxh)) & ~0x1f;				\
			span[j].w.w = (w + dwdiff - (xfrac * dwdxh)) & ~0x1f;				\
			span[j].r.w = r + drdiff - (xfrac * drdxh);				\
			span[j].g.w = g + dgdiff - (xfrac * dgdxh);				\
			span[j].b.w = b + dbdiff - (xfrac * dbdxh);				\
			span[j].a.w = a + dadiff - (xfrac * dadxh);				\
}

#define addvalues() {	\
			s += dsde;	\
			t += dtde;	\
			w += dwde; \
			r += drde; \
			g += dgde; \
			b += dbde; \
			a += dade; \
}

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	for (k = ycur; k <= ylfar; k++)
	{
		if (k == ym)
		{
			xleft = xl;
			xleft_inc = dxldy >> 2;
		}

		xstart = xleft >> 16;
		xend = xright >> 16;
		j = k >> 2;
		spix = k & 3;

		if (k >= 0 && k < 0x1000)
		{
			int m = 0;
			int n = 0;
			int length = 0;
			min = 0; max = 3;
			if (j == yhpix)
			{
				min = yh & 3;
			}
			if (j == ylpix)
			{
				max = yl & 3;
			}
			if (spix >= min && spix <= max)
			{
				if (spix == min)
				{
					minxmx = maxxmx = xstart;
					minxhx = maxxhx = xend;
				}
				else
				{
					minxmx = (xstart < minxmx) ? xstart : minxmx;
					maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
					minxhx = (xend < minxhx) ? xend : minxhx;
					maxxhx = (xend > maxxhx) ? xend : maxxhx;
				}
			}

			if (spix == max)
			{
				if (flip)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}
				else
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}
			}

			length = flip ? (xstart - xend) : (xend - xstart);

			if (spix == ldflag)
			{
				xfrac = ((xright >> 8) & 0xff);
				adjust_attr();
			}

			m = flip ? (xend+1) : (xend-1);

			if (k >= yh && length >= 0 && k <= yl)
			{
				if (xstart>=0 && xstart <1024)
				{
					if (!flip)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					else
					{
						span[j].cvg[xstart] += addright(xleft);
					}
				}
				if (xend>=0 && xend<1024)
				{
					if (xstart != xend)
					{
						if (!flip)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] += addleft(xright);
						}
					}
					else
					{
						if (!flip)
						{
							span[j].cvg[xend] -= (2 - addright(xright));
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
						}
						if (span[j].cvg[xend] > 200)
						{
							span[j].cvg[xend] = 0;
						}
					}
				}
				for (n = 0; n < (length - 1); n++)
				{
					if (m>=0 && m < 640)
					{
						span[j].cvg[m] += 2;
					}

					m += m_inc;
				}
			}
		}

		if (spix == 3)
		{
			addvalues();
		}
		xleft += xleft_inc;
		xright += xright_inc;
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:
			if(flip)
			{
				render_spans_16_s_t_nz_f(yh>>2, yl>>2, &tile[0]); break;
			}
			else
			{
				render_spans_16_s_t_nz_nf(yh>>2, yl>>2, &tile[0]); break;
			}
		case PIXEL_SIZE_32BIT:	render_spans_32(yh>>2, yl>>2, &tile[tilenum], 1, 1, 0, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
	}
}
#undef addvalues
#undef adjust_attr
#undef addleft
#undef addright
#undef setvalues

static void triangle_s_t_z(UINT32 w1, UINT32 w2)
{
	int j;
	int xleft, xright, xleft_inc, xright_inc;
	int xstart, xend;
	int r = 0, g = 0, b = 0, a = 0, z = 0, s = 0, t = 0, w = 0;
	int dr, dg, db, da;
	int drdx = 0, dgdx = 0, dbdx = 0, dadx = 0, dzdx = 0, dsdx = 0, dtdx = 0, dwdx = 0;
	int drdy = 0, dgdy = 0, dbdy = 0, dady = 0, dzdy = 0, dsdy = 0, dtdy = 0, dwdy = 0;
	int drde = 0, dgde = 0, dbde = 0, dade = 0, dzde = 0, dsde = 0, dtde = 0, dwde = 0;
	int tilenum;
	int flip = (w1 & 0x800000) ? 1 : 0;

	INT32 yl, ym, yh;
	INT32 xl, xm, xh;
	INT32 dxldy, dxhdy, dxmdy;
	int dzdy_dz, dzdx_dz;
	int dsdylod, dtdylod;
	UINT32 w3, w4, w5, w6, w7, w8;

	int k = 0;

	INT32 limcvg = 0;
	INT32 startcvg = 0;

	int sign_dxldy = 0;
	int sign_dxmdy = 0;
	int samesign = 0;

	int dsdiff = 0, dtdiff = 0, dwdiff = 0, drdiff = 0, dgdiff = 0, dbdiff = 0, dadiff = 0, dzdiff = 0;
	int sign_dxhdy = 0;

	int dsdeh = 0, dtdeh = 0, dwdeh = 0, drdeh = 0, dgdeh = 0, dbdeh = 0, dadeh = 0, dzdeh = 0, dsdyh = 0, dtdyh = 0, dwdyh = 0, drdyh = 0, dgdyh = 0, dbdyh = 0, dadyh = 0, dzdyh = 0;
	int do_offset = 0;

	int xfrac = 0;
	int dseoff = 0, dteoff = 0, dweoff = 0, dreoff = 0, dgeoff = 0, dbeoff = 0, daeoff = 0, dzeoff = 0;

	int dsdxh = 0, dtdxh = 0, dwdxh = 0, drdxh = 0, dgdxh = 0, dbdxh = 0, dadxh = 0, dzdxh = 0;

	int m_inc;
	UINT32 min=0, max=3;
	INT32 maxxmx = 0, minxmx = 0, maxxhx = 0, minxhx = 0;

	int spix = 0; // Current subpixel
	int ycur;
	int ylfar;
	int ldflag;
	int yhpix;
	int ympix;
	int ylpix;

	int shade_base = rdp_cmd_cur + 8;
	int texture_base = rdp_cmd_cur + 24;
	int zbuffer_base = rdp_cmd_cur + 40;

	w3 = rdp_cmd_data[rdp_cmd_cur+2];
	w4 = rdp_cmd_data[rdp_cmd_cur+3];
	w5 = rdp_cmd_data[rdp_cmd_cur+4];
	w6 = rdp_cmd_data[rdp_cmd_cur+5];
	w7 = rdp_cmd_data[rdp_cmd_cur+6];
	w8 = rdp_cmd_data[rdp_cmd_cur+7];

	yl = (w1 & 0x3fff);
	ym = ((w2 >> 16) & 0x3fff);
	yh = ((w2 >>  0) & 0x3fff);
	xl = (INT32)(w3 & 0x3fffffff);
	xh = (INT32)(w5 & 0x3fffffff);
	xm = (INT32)(w7 & 0x3fffffff);
	// Inverse slopes in 16.16 format
	dxldy = (INT32)(w4);
	dxhdy = (INT32)(w6);
	dxmdy = (INT32)(w8);

	max_level = ((w1 >> 19) & 7);
	tilenum = (w1 >> 16) & 0x7;

	if (yl & 0x2000)  yl |= 0xffffc000;
	if (ym & 0x2000)  ym |= 0xffffc000;
	if (yh & 0x2000)  yh |= 0xffffc000;

	if (xl & 0x20000000)  xl |= 0xc0000000;
	if (xm & 0x20000000)  xm |= 0xc0000000;
	if (xh & 0x20000000)  xh |= 0xc0000000;

	z = 0;
	s = 0;	t = 0;	w = 0;
	dr = 0;		dg = 0;		db = 0;		da = 0;

	r    = (rdp_cmd_data[shade_base+0 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+4 ] >> 16) & 0x0000ffff);
	g    = ((rdp_cmd_data[shade_base+0 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+4 ] & 0x0000ffff);
	b    = (rdp_cmd_data[shade_base+1 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+5 ] >> 16) & 0x0000ffff);
	a    = ((rdp_cmd_data[shade_base+1 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+5 ] & 0x0000ffff);
	drdx = (rdp_cmd_data[shade_base+2 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+6 ] >> 16) & 0x0000ffff);
	dgdx = ((rdp_cmd_data[shade_base+2 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+6 ] & 0x0000ffff);
	dbdx = (rdp_cmd_data[shade_base+3 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+7 ] >> 16) & 0x0000ffff);
	dadx = ((rdp_cmd_data[shade_base+3 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+7 ] & 0x0000ffff);
	drde = (rdp_cmd_data[shade_base+8 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+12] >> 16) & 0x0000ffff);
	dgde = ((rdp_cmd_data[shade_base+8 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+12] & 0x0000ffff);
	dbde = (rdp_cmd_data[shade_base+9 ] & 0xffff0000) | ((rdp_cmd_data[shade_base+13] >> 16) & 0x0000ffff);
	dade = ((rdp_cmd_data[shade_base+9 ] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+13] & 0x0000ffff);
	drdy = (rdp_cmd_data[shade_base+10] & 0xffff0000) | ((rdp_cmd_data[shade_base+14] >> 16) & 0x0000ffff);
	dgdy = ((rdp_cmd_data[shade_base+10] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+14] & 0x0000ffff);
	dbdy = (rdp_cmd_data[shade_base+11] & 0xffff0000) | ((rdp_cmd_data[shade_base+15] >> 16) & 0x0000ffff);
	dady = ((rdp_cmd_data[shade_base+11] << 16) & 0xffff0000) | (rdp_cmd_data[shade_base+15] & 0x0000ffff);
	s    = (rdp_cmd_data[texture_base+0 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+4 ] >> 16) & 0x0000ffff);
	t    = ((rdp_cmd_data[texture_base+0 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+4 ] & 0x0000ffff);
	w    = (rdp_cmd_data[texture_base+1 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+5 ] >> 16) & 0x0000ffff);
	dsdx = (rdp_cmd_data[texture_base+2 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+6 ] >> 16) & 0x0000ffff);
	dtdx = ((rdp_cmd_data[texture_base+2 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+6 ] & 0x0000ffff);
	dwdx = (rdp_cmd_data[texture_base+3 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+7 ] >> 16) & 0x0000ffff);
	dsde = (rdp_cmd_data[texture_base+8 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+12] >> 16) & 0x0000ffff);
	dtde = ((rdp_cmd_data[texture_base+8 ] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+12] & 0x0000ffff);
	dwde = (rdp_cmd_data[texture_base+9 ] & 0xffff0000) | ((rdp_cmd_data[texture_base+13] >> 16) & 0x0000ffff);
	dsdy = (rdp_cmd_data[texture_base+10] & 0xffff0000) | ((rdp_cmd_data[texture_base+14] >> 16) & 0x0000ffff);
	dtdy = ((rdp_cmd_data[texture_base+10] << 16) & 0xffff0000)	| (rdp_cmd_data[texture_base+14] & 0x0000ffff);
	dwdy = (rdp_cmd_data[texture_base+11] & 0xffff0000) | ((rdp_cmd_data[texture_base+15] >> 16) & 0x0000ffff);
	z    = rdp_cmd_data[zbuffer_base+0];
	dzdx = rdp_cmd_data[zbuffer_base+1];
	dzde = rdp_cmd_data[zbuffer_base+2];
	dzdy = rdp_cmd_data[zbuffer_base+3];

	span[0].ds.w = dsdx;
	span[0].dt.w = dtdx;
	span[0].dw.w = dwdx;
	span[0].dr.w = drdx & ~0x1f;
	span[0].dg.w = dgdx & ~0x1f;
	span[0].db.w = dbdx & ~0x1f;
	span[0].da.w = dadx & ~0x1f;
	span[0].dz.w = dzdx;
	dzdy_dz = (dzdy >> 16) & 0xffff;
	dzdx_dz = (dzdx >> 16) & 0xffff;
	span[0].dzpix = ((dzdy_dz & 0x8000) ? ((~dzdy_dz) & 0x7fff) : dzdy_dz) + ((dzdx_dz & 0x8000) ? ((~dzdx_dz) & 0x7fff) : dzdx_dz);
	span[0].dzpix = normalize_dzpix(span[0].dzpix);
	dsdylod = dsdy >> 16;
	dtdylod = dtdy >> 16;
	if (dsdylod & 0x20000)
	{
		dsdylod = ~dsdylod & 0x1ffff;
	}
	if (dtdylod & 0x20000)
	{
		dtdylod = ~dtdylod & 0x1ffff;
	}
	span[0].dymax = (dsdylod > dtdylod)? dsdylod : dtdylod;

	xleft_inc = dxmdy >> 2;
	xright_inc = dxhdy >> 2;

	xright = xh;
	xleft = xm;

	limcvg = ((yl>>2) <= 1023) ? (yl>>2) : 1023; // Needed by 40 Winks
	if (limcvg < 0)
	{
		limcvg = 0;
	}

	startcvg = ((yh>>2)>=0) ? (yh>>2) : 0;
	for (k = startcvg; k <= limcvg; k++)
	{
		memset((void*)&span[k].cvg[0],0,640);
	}

	sign_dxldy = (dxldy & 0x80000000) ? 1 : 0;
	sign_dxmdy = (dxmdy & 0x80000000) ? 1 : 0;
	samesign = !(sign_dxldy ^ sign_dxmdy);

	sign_dxhdy = (dxhdy & 0x80000000) ? 1 : 0;

	do_offset = !(sign_dxhdy ^ (flip));

	if (do_offset)
	{
		dsdeh = dsde >> 9;	dsdyh = dsdy >> 9;
		dtdeh = dtde >> 9;	dtdyh = dtdy >> 9;
		dwdeh = dwde >> 9;	dwdyh = dwdy >> 9;
		drdeh = drde >> 9;	drdyh = drdy >> 9;
		dgdeh = dgde >> 9;	dgdyh = dgdy >> 9;
		dbdeh = dbde >> 9;	dbdyh = dbdy >> 9;
		dadeh = dade >> 9;	dadyh = dady >> 9;
		dzdeh = dzde >> 9;	dzdyh = dzdy >> 9;

		dsdiff = (dsdeh*3 - dsdyh*3) << 7;
		dtdiff = (dtdeh*3 - dtdyh*3) << 7;
		dwdiff = (dwdeh*3 - dwdyh*3) << 7;
		drdiff = (drdeh*3 - drdyh*3) << 7;
		dgdiff = (dgdeh*3 - dgdyh*3) << 7;
		dbdiff = (dbdeh*3 - dbdyh*3) << 7;
		dadiff = (dadeh*3 - dadyh*3) << 7;
		dzdiff = (dzdeh*3 - dzdyh*3) << 7;
	}
	else
	{
		dsdiff = dtdiff = dwdiff = drdiff = dgdiff = dbdiff = dadiff = dzdiff = 0;
	}

	if (do_offset)
	{
		dseoff = (dsdeh*3) << 7;
		dteoff = (dtdeh*3) << 7;
		dweoff = (dwdeh*3) << 7;
		dreoff = (drdeh*3) << 7;
		dgeoff = (dgdeh*3) << 7;
		dbeoff = (dbdeh*3) << 7;
		daeoff = (dadeh*3) << 7;
		dzeoff = (dzdeh*3) << 7;
	}
	else
	{
		dseoff = dteoff = dweoff = dreoff = dgeoff = dbeoff = daeoff = dzeoff = 0;
	}

#define addleft(x) addleftcvg(x,k)
#define addright(x) addrightcvg(x,k)
#define setvalues() {					\
			addvalues();				\
			adjust_attr();	\
}

	dsdxh = dsdx >> 8;
	dtdxh = dtdx >> 8;
	dwdxh = dwdx >> 8;
	drdxh = drdx >> 8;
	dgdxh = dgdx >> 8;
	dbdxh = dbdx >> 8;
	dadxh = dadx >> 8;
	dzdxh = dzdx >> 8;

#define adjust_attr()		\
{							\
			span[j].s.w = (s + dsdiff - (xfrac * dsdxh)) & ~0x1f;				\
			span[j].t.w = (t + dtdiff - (xfrac * dtdxh)) & ~0x1f;				\
			span[j].w.w = (w + dwdiff - (xfrac * dwdxh)) & ~0x1f;				\
			span[j].r.w = r + drdiff - (xfrac * drdxh);				\
			span[j].g.w = g + dgdiff - (xfrac * dgdxh);				\
			span[j].b.w = b + dbdiff - (xfrac * dbdxh);				\
			span[j].a.w = a + dadiff - (xfrac * dadxh);				\
			span[j].z.w = z + dzdiff - (xfrac * dzdxh);				\
}

#define addvalues() {	\
			s += dsde;	\
			t += dtde;	\
			w += dwde; \
			r += drde; \
			g += dgde; \
			b += dbde; \
			a += dade; \
			z += dzde; \
}

	m_inc = flip ? 1 : -1;

	ycur =	yh & ~3;
	ylfar = yl | 3;
	ldflag = (sign_dxhdy ^ flip) ? 0 : 3;
	yhpix = yh >> 2;
	ympix = ym >> 2;
	ylpix = yl >> 2;

	if(flip)
	{
		for (k = ycur; k <= ylfar; k++)
		{
			if (k == ym)
			{
				xleft = xl;
				xleft_inc = dxldy >> 2;
			}

			xstart = xleft >> 16;
			xend = xright >> 16;
			j = k >> 2;
			spix = k & 3;

			if (k >= 0 && k < 0x1000)
			{
				int m = 0;
				int n = 0;
				int length = 0;
				min = 0; max = 3;
				if (j == yhpix)
				{
					min = yh & 3;
				}
				if (j == ylpix)
				{
					max = yl & 3;
				}
				if (spix >= min && spix <= max)
				{
					if (spix == min)
					{
						minxmx = maxxmx = xstart;
						minxhx = maxxhx = xend;
					}
					else
					{
						minxmx = (xstart < minxmx) ? xstart : minxmx;
						maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
						minxhx = (xend < minxhx) ? xend : minxhx;
						maxxhx = (xend > maxxhx) ? xend : maxxhx;
					}
				}

				if (spix == max)
				{
					span[j].lx = maxxmx;
					span[j].rx = minxhx;
				}

				length = xstart - xend;

				if (spix == ldflag)
				{
					xfrac = ((xright >> 8) & 0xff);
					adjust_attr();
				}

				m = xend + 1;

				if (k >= yh && length >= 0 && k <= yl)
				{
					if (xstart>=0 && xstart <1024)
					{
						span[j].cvg[xstart] += addright(xleft);
					}
					if (xend>=0 && xend<1024)
					{
						if (xstart != xend)
						{
							span[j].cvg[xend] += addleft(xright);
						}
						else
						{
							span[j].cvg[xend] -= (2 - addleft(xright));
							if (span[j].cvg[xend] > 200)
							{
								span[j].cvg[xend] = 0;
							}
						}
					}
					for (n = 0; n < (length - 1); n++)
					{
						if (m>=0 && m < 640)
						{
							span[j].cvg[m] += 2;
						}

						m += m_inc;
					}
				}
			}

			if (spix == 3)
			{
				addvalues();
			}
			xleft += xleft_inc;
			xright += xright_inc;
		}
	}
	else
	{
		for (k = ycur; k <= ylfar; k++)
		{
			if (k == ym)
			{
				xleft = xl;
				xleft_inc = dxldy >> 2;
			}

			xstart = xleft >> 16;
			xend = xright >> 16;
			j = k >> 2;
			spix = k & 3;

			if (k >= 0 && k < 0x1000)
			{
				int m = 0;
				int n = 0;
				int length = 0;
				min = 0; max = 3;
				if (j == yhpix)
				{
					min = yh & 3;
				}
				if (j == ylpix)
				{
					max = yl & 3;
				}
				if (spix >= min && spix <= max)
				{
					if (spix == min)
					{
						minxmx = maxxmx = xstart;
						minxhx = maxxhx = xend;
					}
					else
					{
						minxmx = (xstart < minxmx) ? xstart : minxmx;
						maxxmx = (xstart > maxxmx) ? xstart : maxxmx;
						minxhx = (xend < minxhx) ? xend : minxhx;
						maxxhx = (xend > maxxhx) ? xend : maxxhx;
					}
				}

				if (spix == max)
				{
					span[j].lx = minxmx;
					span[j].rx = maxxhx;
				}

				length = xend - xstart;

				if (spix == ldflag)
				{
					xfrac = ((xright >> 8) & 0xff);
					adjust_attr();
				}

				m = xend - 1;

				if (k >= yh && length >= 0 && k <= yl)
				{
					if (xstart>=0 && xstart <1024)
					{
						span[j].cvg[xstart] += addleft(xleft);
					}
					if (xend>=0 && xend<1024)
					{
						if (xstart != xend)
						{
							span[j].cvg[xend] += addright(xright);
						}
						else
						{
							span[j].cvg[xend] -= (2 - addright(xright));
							if (span[j].cvg[xend] > 200)
							{
								span[j].cvg[xend] = 0;
							}
						}
					}
					for (n = 0; n < (length - 1); n++)
					{
						if (m>=0 && m < 640)
						{
							span[j].cvg[m] += 2;
						}

						m += m_inc;
					}
				}
			}

			if (spix == 3)
			{
				addvalues();
			}
			xleft += xleft_inc;
			xright += xright_inc;
		}
	}

	switch (fb_size) // 8bpp needs to be implemented
	{
		case PIXEL_SIZE_16BIT:
			if(flip)
			{
				render_spans_16_s_t_z_f(yh>>2, yl>>2, &tile[0]); break;
			}
			else
			{
				render_spans_16_s_t_z_nf(yh>>2, yl>>2, &tile[0]); break;
			}
		case PIXEL_SIZE_32BIT:	render_spans_32(yh>>2, yl>>2, &tile[tilenum], 1, 1, 1, flip); break;
		default: break; // V-Rally2 does this, fb_size=0
	}
}
#undef addvalues
#undef adjust_attr
#undef addleft
#undef addright
#undef setvalues
