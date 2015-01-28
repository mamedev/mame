


//============================================================
//  INLINE
//============================================================

inline UINT32 premult32(const UINT32 pixel)
{
	const UINT16 a = (pixel >> 24) & 0xff;
	const UINT16 r = (pixel >> 16) & 0xff;
	const UINT16 g = (pixel >> 8) & 0xff;
	const UINT16 b = (pixel >> 0) & 0xff;

	return 0xFF000000 |
		((r * a) / 255) << 16 |
		((g * a) / 255) << 8 |
		((b * a) / 255);
}

inline UINT32 CLUL(const UINT32 x)
{
	return ((INT32) x < 0) ? 0 : ((x > 65535) ? 255 : x >> 8);
}

inline UINT32 ycc_to_rgb(const UINT8 y, const UINT8 cb, const UINT8 cr)
{
	const UINT32 common = 298 * y - 56992;
	const UINT32 r = (common +            409 * cr);
	const UINT32 g = (common - 100 * cb - 208 * cr + 91776);
	const UINT32 b = (common + 516 * cb - 13696);

	return 0xff000000 | (CLUL(r)<<16) | (CLUL(g)<<8) | (CLUL(b));
}

inline UINT32 pixel_ycc_to_rgb(const UINT16 *pixel)
{

	const UINT32 p = *(UINT32 *)((FPTR) pixel & ~3);
	return ycc_to_rgb((*pixel >> 8) & 0xff, (p) & 0xff, (p>>16) & 0xff);
}

inline UINT32 pixel_ycc_to_rgb_pal(const UINT16 *pixel, const rgb_t *palette)
{
	const UINT32 p = *(UINT32 *)((FPTR) pixel & ~3);
	return ycc_to_rgb(palette[(*pixel >> 8) & 0xff], (p) & 0xff, (p>>16) & 0xff);
}

//============================================================
//  Pixel conversions
//============================================================


#define FUNC_DEF(source) op(const source &src, const rgb_t *palbase)
#define FUNCTOR(name, x...) \
    template<typename _source, typename _dest> \
    struct name { _dest FUNC_DEF(_source) { x } };

FUNCTOR(op_argb32_argb32, return src; )
FUNCTOR(op_rgb32_argb32,  return src | 0xff000000; )
FUNCTOR(op_pal16_argb32,  return 0xff000000 |palbase[src]; )
FUNCTOR(op_pal16_rgb32,   return palbase[src]; )
FUNCTOR(op_rgb32pal_argb32,
    return palbase[0x200 + (((src) >> 16) & 0xff) ] |
        palbase[0x100 + (((src) >> 8) & 0xff) ] |
        palbase[((src) & 0xff) ] | 0xff000000; )

FUNCTOR(op_pal16a_argb32, return palbase[src]; )

FUNCTOR(op_rgb15_argb32,
		return 0xff000000 | ((src & 0x7c00) << 9) | ((src & 0x03e0) << 6)
				| ((src & 0x001f) << 3) | ((((src & 0x7c00) << 9)
				| ((src & 0x03e0) << 6) | ((src & 0x001f) << 3) >> 5) & 0x070707); )

FUNCTOR(op_rgb15pal_argb32,
		return 0xff000000 | palbase[0x40 + ((src >> 10) & 0x1f)] |
        palbase[0x20 + ((src >> 5) & 0x1f)] | palbase[0x00 + ((src >> 0) & 0x1f)]; )

FUNCTOR(op_argb32_rgb32, return premult32(src); )
FUNCTOR(op_pal16a_rgb32, return premult32(palbase[src]); )
FUNCTOR(op_pal16_argb1555,
		return (palbase[src]&0xf80000) >> 9 |
            (palbase[src]&0x00f800) >> 6 |
            (palbase[src]&0x0000f8) >> 3 | 0x8000; )

FUNCTOR(op_rgb15_argb1555, return src | 0x8000; )

FUNCTOR(op_rgb15pal_argb1555,
		return (palbase[src >> 10] & 0xf8) << 7 |
            (palbase[(src >> 5) & 0x1f] & 0xf8) << 2 |
            (palbase[src & 0x1f] & 0xf8) >> 3 | 0x8000; )

FUNCTOR(op_yuv16_uyvy, return src; )
FUNCTOR(op_yuv16pal_uyvy, return (palbase[(src >> 8) & 0xff] << 8) | (src & 0x00ff); )

// FIXME: wrong ... see non_pal version
FUNCTOR(op_yuv16pal_yvyu, return (palbase[(src >> 8) & 0xff] & 0xff) | ((src & 0xff) << 8); )
FUNCTOR(op_yuv16_yvyu, return ((src & 0xff00ff00) >> 8 ) | (src << 24) | ((src >> 8) & 0x00ff00); )

FUNCTOR(op_yuv16_yuy2, return ((src & 0xff00ff00) >> 8) | ((src & 0x00ff00ff) << 8); )

FUNCTOR(op_yuv16pal_yuy2,
		return  (palbase[(src>>8) & 0xff]) |
				(palbase[(src>>24) & 0xff]<<16) |
				((src<<8)&0xff00ff00);)

FUNCTOR(op_yuv16_argb32,
		return (UINT64) ycc_to_rgb((src >>  8) & 0xff, src & 0xff , (src>>16) & 0xff)
    | ((UINT64)ycc_to_rgb((src >> 24) & 0xff, src & 0xff , (src>>16) & 0xff) << 32); )

FUNCTOR(op_yuv16pal_argb32,
		return (UINT64)ycc_to_rgb(palbase[(src >>  8) & 0xff], src & 0xff , (src>>16) & 0xff)
    | ((UINT64)ycc_to_rgb(palbase[(src >> 24) & 0xff], src & 0xff , (src>>16) & 0xff) << 32);)

FUNCTOR(op_yuv16_argb32rot, return pixel_ycc_to_rgb(&src) ; )

FUNCTOR(op_yuv16pal_argb32rot, return pixel_ycc_to_rgb_pal(&src, palbase); )

//============================================================
//  Copy and rotation
//============================================================

template<typename _src_type, typename _dest_type, typename _op, int _len_div>
void texcopy(const texture_info *texture, const render_texinfo *texsource)
{
	ATTR_UNUSED const rgb_t *palbase = texsource->palette();
	int x, y;
    _op op;
	/* loop over Y */
	for (y = 0; y < texsource->height; y++) {
		_src_type *src = (_src_type *)texsource->base + y * texsource->rowpixels / (_len_div);
		_dest_type *dst = (_dest_type *)((UINT8 *)texture->m_pixels + y * texture->m_pitch);
		x = texsource->width / (_len_div);
		while (x > 0) {
			*dst++ = op.op(*src, palbase);
			src++;
			x--;
		}
	}
}

#define TEXCOPYA(a, b, c, d) \
inline void texcopy_ ## a(const texture_info *texture, const render_texinfo *texsource) \
{ return texcopy<b, c, op_ ## a <b, c>, d>(texture, texsource); }

template<typename _src_type, typename _dest_type, typename _op>
void texcopy_rot(const texture_info *texture, const render_texinfo *texsource)
{
    ATTR_UNUSED const rgb_t *palbase = texsource->palette();
    int x, y;
    const quad_setup_data *setup = &texture->m_setup;
    int dudx = setup->dudx;
    int dvdx = setup->dvdx;
    _op op;
    /* loop over Y */
    for (y = 0; y < setup->rotheight; y++) {
        INT32 curu = setup->startu + y * setup->dudy;
        INT32 curv = setup->startv + y * setup->dvdy;
        _dest_type *dst = (_dest_type *)((UINT8 *)texture->m_pixels + y * texture->m_pitch);
        x = setup->rotwidth;
        while (x>0) {
            _src_type *src = (_src_type *) texsource->base + (curv >> 16) * texsource->rowpixels + (curu >> 16);
            *dst++ = op.op(*src, palbase);
            curu += dudx;
            curv += dvdx;
            x--;
        }
    }
}

#define TEXROTA(a, b, c) \
inline void texcopy_rot_ ## a(const texture_info *texture, const render_texinfo *texsource) \
{ return texcopy_rot<b, c, op_ ## a <b, c> >(texture, texsource); }

TEXCOPYA(rgb32_argb32,  UINT32, UINT32, 1)
TEXCOPYA(rgb32pal_argb32,  UINT32, UINT32, 1)
TEXCOPYA(pal16_argb32,  UINT16, UINT32, 1)
TEXCOPYA(pal16a_argb32,  UINT16, UINT32, 1)
TEXCOPYA(rgb15_argb32,  UINT16, UINT32, 1)
TEXCOPYA(rgb15pal_argb32,  UINT16, UINT32, 1)

TEXCOPYA(pal16_argb1555,  UINT16, UINT16, 1)
TEXCOPYA(rgb15_argb1555,  UINT16, UINT16, 1)
TEXCOPYA(rgb15pal_argb1555,  UINT16, UINT16, 1)

TEXCOPYA(argb32_rgb32, UINT32, UINT32, 1)
TEXCOPYA(pal16a_rgb32,  UINT16, UINT32, 1)

TEXCOPYA(yuv16_argb32, UINT32, UINT64, 2)
TEXCOPYA(yuv16pal_argb32, UINT32, UINT64, 2)

//TEXCOPYA(yuv16_uyvy, UINT16, UINT16, OP_YUV16_UYVY)

TEXCOPYA(yuv16pal_uyvy, UINT16, UINT16, 1)

TEXCOPYA(yuv16_yvyu, UINT32, UINT32, 2)
TEXCOPYA(yuv16pal_yvyu, UINT16, UINT16, 1)

TEXCOPYA(yuv16_yuy2, UINT32, UINT32, 2)
TEXCOPYA(yuv16pal_yuy2, UINT32, UINT32, 2)



TEXROTA(argb32_argb32, UINT32, UINT32)
TEXROTA(rgb32_argb32,  UINT32, UINT32)
TEXROTA(pal16_argb32,  UINT16, UINT32)
TEXROTA(pal16_rgb32,  UINT16, UINT32)

TEXROTA(rgb32pal_argb32,  UINT32, UINT32)
TEXROTA(pal16a_argb32,  UINT16, UINT32)
TEXROTA(rgb15_argb32,  UINT16, UINT32)
TEXROTA(rgb15pal_argb32,  UINT16, UINT32)

TEXROTA(pal16_argb1555,  UINT16, UINT16)
TEXROTA(rgb15_argb1555,  UINT16, UINT16)
TEXROTA(rgb15pal_argb1555,  UINT16, UINT16)

TEXROTA(argb32_rgb32, UINT32, UINT32)
TEXROTA(pal16a_rgb32,  UINT16, UINT32)

TEXROTA(yuv16_argb32rot, UINT16, UINT32)
TEXROTA(yuv16pal_argb32rot, UINT16, UINT32)
