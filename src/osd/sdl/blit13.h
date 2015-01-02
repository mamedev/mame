


//============================================================
//  INLINE
//============================================================

INLINE UINT32 premult32(UINT32 pixel)
{
	UINT8 a = (pixel >> 24) & 0xff;
	UINT8 r = (pixel >> 16) & 0xff;
	UINT8 g = (pixel >> 8) & 0xff;
	UINT8 b = (pixel >> 0) & 0xff;

	return 0xFF000000 |
		(((UINT16)r * (UINT16)a) / 255) << 16 |
		(((UINT16)g * (UINT16)a) / 255) << 8 |
		(((UINT16)b * (UINT16)a) / 255);
}

#define CLUL(x) ((int) (x) < 0 ? 0 : ((x) > 65535 ? 255 : (x)>>8))

INLINE UINT32 ycc_to_rgb(UINT8 y, UINT8 cb, UINT8 cr)
{
	unsigned int r, g, b, common;

	common = 298 * y - 56992;
	r = (common +            409 * cr);
	g = (common - 100 * cb - 208 * cr + 91776);
	b = (common + 516 * cb - 13696);

	return 0xff000000 | (CLUL(r)<<16) | (CLUL(g)<<8) | (CLUL(b));
}

INLINE UINT32 pixel_ycc_to_rgb(UINT16 *pixel)
{
	UINT32 p = *(UINT32 *)((FPTR) pixel & ~1);
	return ycc_to_rgb((*pixel >> 8) & 0xff, (p) & 0xff, (p>>16) & 0xff);
}

INLINE UINT32 pixel_ycc_to_rgb_pal(UINT16 *pixel, const rgb_t *palette)
{
	UINT32 p = *(UINT32 *)((FPTR) pixel & ~1);
	return ycc_to_rgb(palette[(*pixel >> 8) & 0xff], (p) & 0xff, (p>>16) & 0xff);
}

//============================================================
//  Pixel conversions
//============================================================

#define OP_ARGB32_ARGB32(_src) (_src)

#define OP_RGB32_ARGB32(_src) ((_src) | 0xff000000)

#define OP_RGB32PAL_ARGB32(_src) \
	(palbase[0x200 + (((_src) >> 16) & 0xff) ] | \
		palbase[0x100 + (((_src) >> 8) & 0xff) ] | \
		palbase[((_src) & 0xff) ] | 0xff000000)

#define OP_PAL16_ARGB32(_src) (0xff000000 | palbase[_src])

#define OP_PAL16A_ARGB32(_src) (palbase[_src])

#define OP_RGB15_ARGB32(_src) (0xff000000 | ((_src & 0x7c00) << 9) | ((_src & 0x03e0) << 6) | ((_src & 0x001f) << 3) | \
	((((_src & 0x7c00) << 9) | ((_src & 0x03e0) << 6) | ((_src & 0x001f) << 3) >> 5) & 0x070707))

#define OP_RGB15PAL_ARGB32(_src) (0xff000000 | palbase[0x40 + ((_src >> 10) & 0x1f)] | \
		palbase[0x20 + ((_src >> 5) & 0x1f)] | palbase[0x00 + ((_src >> 0) & 0x1f)])

#define OP_ARGB32_RGB32(_pixel) premult32(_pixel)

#define OP_PAL16A_RGB32(_src) premult32(palbase[_src])

#define OP_PAL16_ARGB1555(_src) ((palbase[_src]&0xf80000) >> 9 | \
			(palbase[_src]&0x00f800) >> 6 | \
			(palbase[_src]&0x0000f8) >> 3 | 0x8000)

#define OP_RGB15_ARGB1555(_src) ((_src) | 0x8000)

#define OP_RGB15PAL_ARGB1555(_src) ((palbase[(_src) >> 10] & 0xf8) << 7 | \
			(palbase[((_src) >> 5) & 0x1f] & 0xf8) << 2 | \
			(palbase[(_src) & 0x1f] & 0xf8) >> 3 | 0x8000)

#define OP_YUV16_UYVY(_src) (_src)

#define OP_YUV16PAL_UYVY(_src) ((palbase[((_src) >> 8) & 0xff] << 8) | ((_src) & 0x00ff))

#define OP_YUV16PAL_YVYU(_src) ((palbase[((_src) >> 8) & 0xff] & 0xff) | ((_src & 0xff) << 8))

#define OP_YUV16_YVYU(_src) ((((_src) >> 8) & 0xff) | ((_src & 0xff) << 8))

#define OP_YUV16_YUY2(_src) ( ((_src) & 0xff00ff00) | \
	(((_src)>>16)&0xff) | (((_src)<<16)&0xff0000) )

#define OP_YUV16PAL_YUY2(_src) ( (palbase[((_src)>>8) & 0xff]) | \
		(palbase[((_src)>>24) & 0xff]<<16) | \
	(((_src)<<8)&0xff00ff00) )

#define OP_YUV16_ARGB32(_src) \
		(UINT64) ycc_to_rgb(((_src) >>  8) & 0xff, (_src) & 0xff , ((_src)>>16) & 0xff) \
	| ((UINT64)ycc_to_rgb(((_src) >> 24) & 0xff, (_src) & 0xff , ((_src)>>16) & 0xff) << 32)

#define OP_YUV16PAL_ARGB32(_src) \
		(UINT64)ycc_to_rgb(palbase[((_src) >>  8) & 0xff], (_src) & 0xff , ((_src)>>16) & 0xff) \
	| ((UINT64)ycc_to_rgb(palbase[((_src) >> 24) & 0xff], (_src) & 0xff , ((_src)>>16) & 0xff) << 32)

#define OP_YUV16_ARGB32ROT(_src) pixel_ycc_to_rgb(&(_src))

#define OP_YUV16PAL_ARGB32ROT(_src) pixel_ycc_to_rgb_pal(&(_src), palbase)

//============================================================
//  Copy and rotation
//============================================================

#define TEXCOPY_M( _name, _src_type, _dest_type,  _op, _len_div) \
INLINE void texcopy_##_name (texture_info *texture, const render_texinfo *texsource) { \
    ATTR_UNUSED const rgb_t *palbase = texsource->palette(); \
	int x, y; \
	/* loop over Y */ \
	for (y = 0; y < texsource->height; y++) { \
		_src_type *src = (_src_type *)texsource->base + y * texsource->rowpixels / (_len_div); \
		_dest_type *dst = (_dest_type *)((UINT8 *)texture->pixels + y * texture->pitch); \
		x = texsource->width / (_len_div); \
		while (x > 0) { \
			*dst++ = _op(*src); \
			src++; \
			x--; \
		} \
	} \
}

#define TEXCOPY( _name, _src_type, _dest_type,  _op) \
	TEXCOPY_M( _name, _src_type, _dest_type,  _op, 1)

#define TEXROT( _name, _src_type, _dest_type, _op) \
INLINE void texcopy_rot_##_name (texture_info *texture, const render_texinfo *texsource) { \
    ATTR_UNUSED const rgb_t *palbase = texsource->palette(); \
	int x, y; \
	quad_setup_data *setup = &texture->setup; \
	int dudx = setup->dudx; \
	int dvdx = setup->dvdx; \
	/* loop over Y */ \
	for (y = 0; y < setup->rotheight; y++) { \
		INT32 curu = setup->startu + y * setup->dudy; \
		INT32 curv = setup->startv + y * setup->dvdy; \
		_dest_type *dst = (_dest_type *)((UINT8 *)texture->pixels + y * texture->pitch); \
		x = setup->rotwidth; \
		while (x>0) { \
			_src_type *src = (_src_type *) texsource->base + (curv >> 16) * texsource->rowpixels + (curu >> 16); \
			*dst++ = _op(*src); \
			curu += dudx; \
			curv += dvdx; \
			x--; \
		} \
	} \
}

//TEXCOPY(argb32_argb32, UINT32, UINT32, OP_ARGB32_ARGB32)

TEXCOPY(rgb32_argb32,  UINT32, UINT32, OP_RGB32_ARGB32)
TEXCOPY(rgb32pal_argb32,  UINT32, UINT32, OP_RGB32PAL_ARGB32)
TEXCOPY(pal16_argb32,  UINT16, UINT32, OP_PAL16_ARGB32)
TEXCOPY(pal16a_argb32,  UINT16, UINT32, OP_PAL16A_ARGB32)
TEXCOPY(rgb15_argb32,  UINT16, UINT32, OP_RGB15_ARGB32)
TEXCOPY(rgb15pal_argb32,  UINT16, UINT32, OP_RGB15PAL_ARGB32)

TEXCOPY(pal16_argb1555,  UINT16, UINT16, OP_PAL16_ARGB1555)
TEXCOPY(rgb15_argb1555,  UINT16, UINT16, OP_RGB15_ARGB1555)
TEXCOPY(rgb15pal_argb1555,  UINT16, UINT16, OP_RGB15PAL_ARGB1555)

TEXCOPY(argb32_rgb32, UINT32, UINT32, OP_ARGB32_RGB32)
TEXCOPY(pal16a_rgb32,  UINT16, UINT32, OP_PAL16A_RGB32)

TEXCOPY_M(yuv16_argb32, UINT32, UINT64, OP_YUV16_ARGB32, 2)
TEXCOPY_M(yuv16pal_argb32, UINT32, UINT64, OP_YUV16PAL_ARGB32, 2)

//TEXCOPY(yuv16_uyvy, UINT16, UINT16, OP_YUV16_UYVY)

TEXCOPY(yuv16pal_uyvy, UINT16, UINT16, OP_YUV16PAL_UYVY)

TEXCOPY(yuv16_yvyu, UINT16, UINT16, OP_YUV16_YVYU)
TEXCOPY(yuv16pal_yvyu, UINT16, UINT16, OP_YUV16PAL_YVYU)

TEXCOPY_M(yuv16_yuy2, UINT32, UINT32, OP_YUV16_YUY2, 2)
TEXCOPY_M(yuv16pal_yuy2, UINT32, UINT32, OP_YUV16PAL_YUY2, 2)


TEXROT(argb32_argb32, UINT32, UINT32, OP_ARGB32_ARGB32)
TEXROT(rgb32_argb32,  UINT32, UINT32, OP_RGB32_ARGB32)
TEXROT(rgb32pal_argb32,  UINT32, UINT32, OP_RGB32PAL_ARGB32)
TEXROT(pal16_argb32,  UINT16, UINT32, OP_PAL16_ARGB32)
TEXROT(pal16a_argb32,  UINT16, UINT32, OP_PAL16A_ARGB32)
TEXROT(rgb15_argb32,  UINT16, UINT32, OP_RGB15_ARGB32)
TEXROT(rgb15pal_argb32,  UINT16, UINT32, OP_RGB15PAL_ARGB32)

TEXROT(pal16_argb1555,  UINT16, UINT16, OP_PAL16_ARGB1555)
TEXROT(rgb15_argb1555,  UINT16, UINT16, OP_RGB15_ARGB1555)
TEXROT(rgb15pal_argb1555,  UINT16, UINT16, OP_RGB15PAL_ARGB1555)

TEXROT(argb32_rgb32, UINT32, UINT32, OP_ARGB32_RGB32)
TEXROT(pal16a_rgb32,  UINT16, UINT32, OP_PAL16A_RGB32)

TEXROT(yuv16_argb32, UINT16, UINT32, OP_YUV16_ARGB32ROT)
TEXROT(yuv16pal_argb32, UINT16, UINT32, OP_YUV16PAL_ARGB32ROT)
