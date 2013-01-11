
#ifdef DEST_TYPE
#undef DEST_TYPE
#endif

#ifdef DEST_NAME
#undef DEST_NAME
#endif

#ifdef TEXSRC_TYPE
#undef TEXSRC_TYPE
#endif

#ifdef TEXSRC_TO_DEST
#undef TEXSRC_TO_DEST
#endif

#ifdef SRC_EQUALS_DEST
#undef SRC_EQUALS_DEST
#endif

#ifdef FUNC_NAME
#undef FUNC_NAME
#endif

#if SDL_TEXFORMAT == SDL_TEXFORMAT_ARGB32
	#define DEST_TYPE UINT32
	#define DEST_NAME(name) name ## _32bpp
	#define TEXSRC_TYPE UINT32
	#define TEXSRC_TO_DEST(src) (src)
	#define SRC_EQUALS_DEST
	#define FUNC_NAME(name) name ## _argb32
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_RGB32
	#define DEST_TYPE UINT32
	#define DEST_NAME(name) name ## _32bpp
	#define TEXSRC_TYPE UINT32
	#define TEXSRC_TO_DEST(src) ((src) | 0xff000000)
	#define FUNC_NAME(name) name ## _rgb32
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_RGB32_PALETTED
	#define DEST_TYPE UINT32
	#define DEST_NAME(name) name ## _32bpp
	#define TEXSRC_TYPE UINT32
	#define TEXSRC_TO_DEST(src) \
		(texsource->palette[0x200 + RGB_RED(src)]   | \
			texsource->palette[0x100 + RGB_GREEN(src)] | \
			texsource->palette[RGB_BLUE(src)] | 0xff000000)
	#define FUNC_NAME(name) name ## _rgb32_paletted
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_PALETTE16
	#define DEST_TYPE UINT32
	#define DEST_NAME(name) name ## _32bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) \
		(0xff000000 | texsource->palette[src])
	#define FUNC_NAME(name) name ## _palette16
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_PALETTE16A
	#define DEST_TYPE UINT32
	#define DEST_NAME(name) name ## _32bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) \
		(texsource->palette[src])
	#define FUNC_NAME(name) name ## _palette16a
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_RGB15
	#define DEST_TYPE UINT32
	#define DEST_NAME(name) name ## _32bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) (0xff000000 | ((src & 0x7c00) << 9) | ((src & 0x03e0) << 6) | ((src & 0x001f) << 3) | \
		((((src & 0x7c00) << 9) | ((src & 0x03e0) << 6) | ((src & 0x001f) << 3) >> 5) & 0x070707))
	#define FUNC_NAME(name) name ## _rgb15
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_RGB15_PALETTED
	#define DEST_TYPE UINT32
	#define DEST_NAME(name) name ## _32bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) (0xff000000 | texsource->palette[0x40 + ((src >> 10) & 0x1f)] | \
		texsource->palette[0x20 + ((src >> 5) & 0x1f)] | texsource->palette[0x00 + ((src >> 0) & 0x1f)])
	#define FUNC_NAME(name) name ## _rgb15_paletted
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_YUY16 && defined(SDLMAME_MACOSX)
	#define DEST_TYPE UINT16
	#define DEST_NAME(name) name ## _16bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) ((src >> 8) | (src << 8))
	#define FUNC_NAME(name) name ## _yuv16_apple
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_YUY16_PALETTED && defined(SDLMAME_MACOSX)
	#define DEST_TYPE UINT16
	#define DEST_NAME(name) name ## _16bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) (texsource->palette[0x000 + (src >> 8)] | (src << 8))
	#define FUNC_NAME(name) name ## _yuv16_paletted_apple
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_PALETTE16_ARGB1555
	#define DEST_TYPE UINT16
	#define DEST_NAME(name) name ## _16bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) \
		((texsource->palette[src]&0xf80000) >> 9 | \
			(texsource->palette[src]&0x00f800) >> 6 | \
			(texsource->palette[src]&0x0000f8) >> 3 | 0x8000)
	#define FUNC_NAME(name) name ## _palette16_argb1555
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_RGB15_ARGB1555
	#define DEST_TYPE UINT16
	#define DEST_NAME(name) name ## _16bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) ((src) | 0x8000)
	#define FUNC_NAME(name) name ## _rgb15_argb1555
#elif SDL_TEXFORMAT == SDL_TEXFORMAT_RGB15_PALETTED_ARGB1555
	#define DEST_TYPE UINT16
	#define DEST_NAME(name) name ## _16bpp
	#define TEXSRC_TYPE UINT16
	#define TEXSRC_TO_DEST(src) \
		((texsource->palette[(src) >> 10] & 0xf8) << 7 | \
			(texsource->palette[((src) >> 5) & 0x1f] & 0xf8) << 2 | \
			(texsource->palette[(src) & 0x1f] & 0xf8) >> 3 | 0x8000)
	#define FUNC_NAME(name) name ## _rgb15_paletted_argb1555
#else
	#error Unknown SRC_TEXFORMAT
#endif
