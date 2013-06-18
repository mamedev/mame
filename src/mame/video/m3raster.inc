static void draw_scanline_normal(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	const cached_texture *texture = extra->texture;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	UINT16 *p = &destmap->pix16(scanline);
	UINT32 *d = &extra->zbuffer->pix32(scanline);
	float ooz = extent->param[0].start;
	float uoz = extent->param[1].start;
	float voz = extent->param[2].start;
	float doozdx = extent->param[0].dpdx;
	float duozdx = extent->param[1].dpdx;
	float dvozdx = extent->param[2].dpdx;
	UINT32 polyi = extra->polygon_intensity;
	UINT32 umask = (((extra->texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((extra->texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = ooz * 256.0f;
		if (iz > d[x])
		{
			float z = 1.0f / ooz;
			UINT32 u = uoz * z;
			UINT32 v = voz * z;
			UINT32 u1 = (u >> 8) & umask;
			UINT32 v1 = (v >> 8) & vmask;
			UINT32 u2 = (u1 + 1) & umask;
			UINT32 v2 = (v1 + 1) & vmask;
			UINT32 pix00 = texture->data[(v1 << width) + u1];
			UINT32 pix01 = texture->data[(v1 << width) + u2];
			UINT32 pix10 = texture->data[(v2 << width) + u1];
			UINT32 pix11 = texture->data[(v2 << width) + u2];
			UINT32 texel = rgba_bilinear_filter(pix00, pix01, pix10, pix11, u, v);
			UINT32 fr = ((texel & 0x00ff0000) * polyi) >> (8+9);
			UINT32 fg = ((texel & 0x0000ff00) * polyi) >> (8+6);
			UINT32 fb = ((texel & 0x000000ff) * polyi) >> (8+3);
			p[x] = (fr & 0x7c00) | (fg & 0x3e0) | (fb & 0x1f);
			d[x] = iz;
		}

		ooz += doozdx;
		uoz += duozdx;
		voz += dvozdx;
	}
}

static void draw_scanline_trans(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	const cached_texture *texture = extra->texture;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	UINT16 *p = &destmap->pix16(scanline);
	UINT32 *d = &extra->zbuffer->pix32(scanline);
	float ooz = extent->param[0].start;
	float uoz = extent->param[1].start;
	float voz = extent->param[2].start;
	float doozdx = extent->param[0].dpdx;
	float duozdx = extent->param[1].dpdx;
	float dvozdx = extent->param[2].dpdx;
	UINT32 polyi = (extra->polygon_intensity * extra->polygon_transparency) >> 5;
	int desttrans = 32 - extra->polygon_transparency;
	UINT32 umask = (((extra->texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((extra->texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = ooz * 256.0f;
		if (iz > d[x])
		{
			float z = 1.0f / ooz;
			UINT32 u = uoz * z;
			UINT32 v = voz * z;
			UINT32 u1 = (u >> 8) & umask;
			UINT32 v1 = (v >> 8) & vmask;
			UINT32 u2 = (u1 + 1) & umask;
			UINT32 v2 = (v1 + 1) & vmask;
			UINT32 pix00 = texture->data[(v1 << width) + u1];
			UINT32 pix01 = texture->data[(v1 << width) + u2];
			UINT32 pix10 = texture->data[(v2 << width) + u1];
			UINT32 pix11 = texture->data[(v2 << width) + u2];
			UINT32 texel = rgba_bilinear_filter(pix00, pix01, pix10, pix11, u, v);
			UINT32 fr = ((texel & 0x00ff0000) * polyi) >> (8+9);
			UINT32 fg = ((texel & 0x0000ff00) * polyi) >> (8+6);
			UINT32 fb = ((texel & 0x000000ff) * polyi) >> (8+3);
			UINT16 orig = p[x];
			fr += ((orig & 0x7c00) * desttrans) >> 5;
			fg += ((orig & 0x03e0) * desttrans) >> 5;
			fb += ((orig & 0x001f) * desttrans) >> 5;
			p[x] = (fr & 0x7c00) | (fg & 0x3e0) | (fb & 0x1f);
			d[x] = iz;
		}

		ooz += doozdx;
		uoz += duozdx;
		voz += dvozdx;
	}
}


static void draw_scanline_alpha(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	const cached_texture *texture = extra->texture;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	UINT16 *p = &destmap->pix16(scanline);
	UINT32 *d = &extra->zbuffer->pix32(scanline);
	float ooz = extent->param[0].start;
	float uoz = extent->param[1].start;
	float voz = extent->param[2].start;
	float doozdx = extent->param[0].dpdx;
	float duozdx = extent->param[1].dpdx;
	float dvozdx = extent->param[2].dpdx;
	UINT32 polyi = (extra->polygon_intensity * extra->polygon_transparency) >> 5;
	int desttrans = 32 - extra->polygon_transparency;
	UINT32 umask = (((extra->texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((extra->texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = ooz * 256.0f;
		if (iz > d[x])
		{
			float z = 1.0f / ooz;
			UINT32 u = uoz * z;
			UINT32 v = voz * z;
			UINT32 u1 = (u >> 8) & umask;
			UINT32 v1 = (v >> 8) & vmask;
			UINT32 u2 = (u1 + 1) & umask;
			UINT32 v2 = (v1 + 1) & vmask;
			UINT32 pix00 = texture->data[(v1 << width) + u1];
			UINT32 pix01 = texture->data[(v1 << width) + u2];
			UINT32 pix10 = texture->data[(v2 << width) + u1];
			UINT32 pix11 = texture->data[(v2 << width) + u2];
			UINT32 texel = rgba_bilinear_filter(pix00, pix01, pix10, pix11, u, v);
			UINT32 fa = texel >> 24;
			if (fa != 0)
			{
				UINT32 combined = ((fa + 1) * polyi) >> 8;
				UINT32 fr = ((texel & 0x00ff0000) * combined) >> (8+9);
				UINT32 fg = ((texel & 0x0000ff00) * combined) >> (8+6);
				UINT32 fb = ((texel & 0x000000ff) * combined) >> (8+3);
				UINT16 orig = p[x];
				combined = ((255 - fa) * desttrans) >> 5;
				fr += ((orig & 0x7c00) * combined) >> 8;
				fg += ((orig & 0x03e0) * combined) >> 8;
				fb += ((orig & 0x001f) * combined) >> 8;
				p[x] = (fr & 0x7c00) | (fg & 0x3e0) | (fb & 0x1f);
				d[x] = iz;
			}
		}

		ooz += doozdx;
		uoz += duozdx;
		voz += dvozdx;
	}
}


static void draw_scanline_alpha_test(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	const cached_texture *texture = extra->texture;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	UINT16 *p = &destmap->pix16(scanline);
	UINT32 *d = &extra->zbuffer->pix32(scanline);
	float ooz = extent->param[0].start;
	float uoz = extent->param[1].start;
	float voz = extent->param[2].start;
	float doozdx = extent->param[0].dpdx;
	float duozdx = extent->param[1].dpdx;
	float dvozdx = extent->param[2].dpdx;
	UINT32 polyi = (extra->polygon_intensity * extra->polygon_transparency) >> 5;
	int desttrans = 32 - extra->polygon_transparency;
	UINT32 umask = (((extra->texture_param & TRI_PARAM_TEXTURE_MIRROR_U) ? 64 : 32) << texture->width) - 1;
	UINT32 vmask = (((extra->texture_param & TRI_PARAM_TEXTURE_MIRROR_V) ? 64 : 32) << texture->height) - 1;
	UINT32 width = 6 + texture->width;
	int x;

	for (x = extent->startx; x < extent->stopx; x++)
	{
		UINT32 iz = ooz * 256.0f;
		if (iz > d[x])
		{
			float z = 1.0f / ooz;
			UINT32 u = uoz * z;
			UINT32 v = voz * z;
			UINT32 u1 = (u >> 8) & umask;
			UINT32 v1 = (v >> 8) & vmask;
			UINT32 u2 = (u1 + 1) & umask;
			UINT32 v2 = (v1 + 1) & vmask;
			UINT32 pix00 = texture->data[(v1 << width) + u1];
			UINT32 pix01 = texture->data[(v1 << width) + u2];
			UINT32 pix10 = texture->data[(v2 << width) + u1];
			UINT32 pix11 = texture->data[(v2 << width) + u2];
			UINT32 texel = rgba_bilinear_filter(pix00, pix01, pix10, pix11, u, v);
			UINT32 fa = texel >> 24;
			if (fa >= 0xf8)
			{
				UINT32 combined = ((fa + 1) * polyi) >> 8;
				UINT32 fr = ((texel & 0x00ff0000) * combined) >> (8+9);
				UINT32 fg = ((texel & 0x0000ff00) * combined) >> (8+6);
				UINT32 fb = ((texel & 0x000000ff) * combined) >> (8+3);
				UINT16 orig = p[x];
				combined = ((255 - fa) * desttrans) >> 8;
				fr += ((orig & 0x7c00) * combined) >> 5;
				fg += ((orig & 0x03e0) * combined) >> 5;
				fb += ((orig & 0x001f) * combined) >> 5;
				p[x] = (fr & 0x7c00) | (fg & 0x3e0) | (fb & 0x1f);
				d[x] = iz;
			}
		}

		ooz += doozdx;
		uoz += duozdx;
		voz += dvozdx;
	}
}

static void draw_scanline_color(void *dest, INT32 scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_ind16 *destmap = (bitmap_ind16 *)dest;
	UINT16 *p = &destmap->pix16(scanline);
	UINT32 *d = &extra->zbuffer->pix32(scanline);
	float ooz = extent->param[0].start;
	float doozdx = extent->param[0].dpdx;
	int fr = extra->color & 0x7c00;
	int fg = extra->color & 0x03e0;
	int fb = extra->color & 0x001f;
	int x;

	// apply intensity
	fr = (fr * extra->polygon_intensity) >> 8;
	fg = (fg * extra->polygon_intensity) >> 8;
	fb = (fb * extra->polygon_intensity) >> 8;

	/* simple case: no transluceny */
	if (extra->polygon_transparency >= 32)
	{
		UINT32 color = (fr & 0x7c00) | (fg & 0x03e0) | (fb & 0x1f);
		for (x = extent->startx; x < extent->stopx; x++)
		{
			UINT32 iz = ooz * 256.0f;
			if (iz > d[x])
			{
				p[x] = color;
				d[x] = iz;
			}
			ooz += doozdx;
		}
	}

	/* translucency */
	else
	{
		int polytrans = extra->polygon_transparency;

		fr = (fr * polytrans) >> 5;
		fg = (fg * polytrans) >> 5;
		fb = (fb * polytrans) >> 5;
		polytrans = 32 - polytrans;

		for (x = extent->startx; x < extent->stopx; x++)
		{
			UINT32 iz = ooz * 256.0f;
			if (iz > d[x])
			{
				UINT16 orig = p[x];
				int r = fr + (((orig & 0x7c00) * polytrans) >> 5);
				int g = fg + (((orig & 0x03e0) * polytrans) >> 5);
				int b = fb + (((orig & 0x001f) * polytrans) >> 5);

				p[x] = (r & 0x7c00) | (g & 0x3e0) | (b & 0x1f);
				d[x] = iz;
			}
			ooz += doozdx;
		}
	}
}
