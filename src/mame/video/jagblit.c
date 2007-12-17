/***************************************************************************

    Atari Jaguar blitter

****************************************************************************

    How to use this module:

    #define FUNCNAME -- name of the function to be generated
    #define COMMAND  -- blitter command bits for blitter
    #define A1FIXED  -- fixed A1 flag bits for blitter
    #define A2FIXED  -- fixed A2 flag bits for blitter
    #include "jagblit.c"

****************************************************************************/


#ifndef FUNCNAME
#error jagblit.c -- requires FUNCNAME to be predefined
#endif

#ifndef COMMAND
#error jagblit.c -- requires COMMAND to be predefined
#endif

#ifndef A1FIXED
#error jagblit.c -- requires A1FIXED to be predefined
#endif

#ifndef A2FIXED
#error jagblit.c -- requires A2FIXED to be predefined
#endif

#ifndef PIXEL_SHIFT_1
#define PIXEL_SHIFT_1(a)      ((~a##_x >> 16) & 7)
#define PIXEL_OFFSET_1(a)     BYTE4_XOR_BE(((UINT32)a##_y >> 16) * a##_width / 8 + (((UINT32)a##_x >> 19) & ~7) * (1 + a##_pitch) + (((UINT32)a##_x >> 19) & 7))
#define ZDATA_OFFSET_1(a)     0 /* huh? */
#define READ_RDATA_1(r,a,p)   ((p) ? (((((UINT8 *)&blitter_regs[r])[BYTE4_XOR_BE(((UINT32)a##_x >> 19) & 7)]) >> PIXEL_SHIFT_1(a)) & 0x01) : (blitter_regs[r] & 0x01))
#define READ_PIXEL_1(a)       (((((UINT8 *)a##_base_mem)[PIXEL_OFFSET_1(a)]) >> PIXEL_SHIFT_1(a)) & 0x01)
#define READ_ZDATA_1(a)       0 /* huh? */
#define WRITE_PIXEL_1(a,d)    do { UINT8 *pix = &((UINT8 *)a##_base_mem)[PIXEL_OFFSET_1(a)]; *pix = (*pix & ~(0x01 << PIXEL_SHIFT_1(a))) | ((d) << PIXEL_SHIFT_1(a)); } while (0)
#define WRITE_ZDATA_1(a,d)    /* huh? */

#define PIXEL_SHIFT_2(a)      ((~a##_x >> 15) & 6)
#define PIXEL_OFFSET_2(a)     BYTE4_XOR_BE(((UINT32)a##_y >> 16) * a##_width / 4 + (((UINT32)a##_x >> 18) & ~7) * (1 + a##_pitch) + (((UINT32)a##_x >> 18) & 7))
#define ZDATA_OFFSET_2(a)     0 /* huh? */
#define READ_RDATA_2(r,a,p)   ((p) ? (((((UINT8 *)&blitter_regs[r])[BYTE4_XOR_BE(((UINT32)a##_x >> 18) & 7)]) >> PIXEL_SHIFT_2(a)) & 0x03) : (blitter_regs[r] & 0x03))
#define READ_PIXEL_2(a)       (((((UINT8 *)a##_base_mem)[PIXEL_OFFSET_2(a)]) >> PIXEL_SHIFT_2(a)) & 0x03)
#define READ_ZDATA_2(a)       0 /* huh? */
#define WRITE_PIXEL_2(a,d)    do { UINT8 *pix = &((UINT8 *)a##_base_mem)[PIXEL_OFFSET_2(a)]; *pix = (*pix & ~(0x03 << PIXEL_SHIFT_2(a))) | ((d) << PIXEL_SHIFT_2(a)); } while (0)
#define WRITE_ZDATA_2(a,d)    /* huh? */

#define PIXEL_SHIFT_4(a)      ((~a##_x >> 14) & 4)
#define PIXEL_OFFSET_4(a)     BYTE4_XOR_BE(((UINT32)a##_y >> 16) * a##_width / 2 + (((UINT32)a##_x >> 17) & ~7) * (1 + a##_pitch) + (((UINT32)a##_x >> 17) & 7))
#define ZDATA_OFFSET_4(a)     0 /* huh? */
#define READ_RDATA_4(r,a,p)   ((p) ? (((((UINT8 *)&blitter_regs[r])[BYTE4_XOR_BE(((UINT32)a##_x >> 17) & 7)]) >> PIXEL_SHIFT_4(a)) & 0x0f) : (blitter_regs[r] & 0x0f))
#define READ_PIXEL_4(a)       (((((UINT8 *)a##_base_mem)[PIXEL_OFFSET_4(a)]) >> PIXEL_SHIFT_4(a)) & 0x0f)
#define READ_ZDATA_4(a)       0 /* huh? */
#define WRITE_PIXEL_4(a,d)    do { UINT8 *pix = &((UINT8 *)a##_base_mem)[PIXEL_OFFSET_4(a)]; *pix = (*pix & ~(0x0f << PIXEL_SHIFT_4(a))) | ((d) << PIXEL_SHIFT_4(a)); } while (0)
#define WRITE_ZDATA_4(a,d)    /* huh? */

#define PIXEL_OFFSET_8(a)     BYTE4_XOR_BE(((UINT32)a##_y >> 16) * a##_width + (((UINT32)a##_x >> 16) & ~7) * (1 + a##_pitch) + (((UINT32)a##_x >> 16) & 7))
#define ZDATA_OFFSET_8(a)     (PIXEL_OFFSET_8(a) + a##_zoffs * 8)
#define READ_RDATA_8(r,a,p)   ((p) ? (((UINT8 *)&blitter_regs[r])[BYTE4_XOR_BE(((UINT32)a##_x >> 16) & 7)]) : (blitter_regs[r] & 0xff))
#define READ_PIXEL_8(a)       (((UINT8 *)a##_base_mem)[PIXEL_OFFSET_8(a)])
#define READ_ZDATA_8(a)       (((UINT8 *)a##_base_mem)[ZDATA_OFFSET_8(a)])
#define WRITE_PIXEL_8(a,d)    do { ((UINT8 *)a##_base_mem)[PIXEL_OFFSET_8(a)] = (d); } while (0)
#define WRITE_ZDATA_8(a,d)    do { ((UINT8 *)a##_base_mem)[ZDATA_OFFSET_8(a)] = (d); } while (0)

#define PIXEL_OFFSET_16(a)    BYTE_XOR_BE(((UINT32)a##_y >> 16) * a##_width + (((UINT32)a##_x >> 16) & ~3) * (1 + a##_pitch) + (((UINT32)a##_x >> 16) & 3))
#define ZDATA_OFFSET_16(a)    (PIXEL_OFFSET_16(a) + a##_zoffs * 4)
#define READ_RDATA_16(r,a,p)  ((p) ? (((UINT16 *)&blitter_regs[r])[BYTE_XOR_BE(((UINT32)a##_x >> 16) & 3)]) : (blitter_regs[r] & 0xffff))
#define READ_PIXEL_16(a)      (((UINT16 *)a##_base_mem)[PIXEL_OFFSET_16(a)])
#define READ_ZDATA_16(a)      (((UINT16 *)a##_base_mem)[ZDATA_OFFSET_16(a)])
#define WRITE_PIXEL_16(a,d)   do { ((UINT16 *)a##_base_mem)[PIXEL_OFFSET_16(a)] = (d); } while (0)
#define WRITE_ZDATA_16(a,d)   do { ((UINT16 *)a##_base_mem)[ZDATA_OFFSET_16(a)] = (d); } while (0)

#define PIXEL_OFFSET_32(a)    (((UINT32)a##_y >> 16) * a##_width + (((UINT32)a##_x >> 16) & ~1) * (1 + a##_pitch) + (((UINT32)a##_x >> 16) & 1))
#define ZDATA_OFFSET_32(a)    (PIXEL_OFFSET_32(a) + a##_zoffs * 2)
#define READ_RDATA_32(r,a,p)  ((p) ? (blitter_regs[r + (((UINT32)a##_x >> 16) & 1)]) : blitter_regs[r])
#define READ_PIXEL_32(a)      (((UINT32 *)a##_base_mem)[PIXEL_OFFSET_32(a)])
#define READ_ZDATA_32(a)      (((UINT32 *)a##_base_mem)[ZDATA_OFFSET_32(a)])
#define WRITE_PIXEL_32(a,d)   do { ((UINT32 *)a##_base_mem)[PIXEL_OFFSET_32(a)] = (d); } while (0)
#define WRITE_ZDATA_32(a,d)   do { ((UINT32 *)a##_base_mem)[ZDATA_OFFSET_32(a)] = (d); } while (0)

#define READ_RDATA(r,a,f,p) \
	((((f) & 0x38) == (0 << 3)) ? (READ_RDATA_1(r,a,p)) : \
	 (((f) & 0x38) == (1 << 3)) ? (READ_RDATA_2(r,a,p)) : \
	 (((f) & 0x38) == (2 << 3)) ? (READ_RDATA_4(r,a,p)) : \
	 (((f) & 0x38) == (3 << 3)) ? (READ_RDATA_8(r,a,p)) : \
	 (((f) & 0x38) == (4 << 3)) ? (READ_RDATA_16(r,a,p)) : \
	 (((f) & 0x38) == (5 << 3)) ? (READ_RDATA_32(r,a,p)) : 0)

#define READ_PIXEL(a,f) \
	((((f) & 0x38) == (0 << 3)) ? (READ_PIXEL_1(a)) : \
	 (((f) & 0x38) == (1 << 3)) ? (READ_PIXEL_2(a)) : \
	 (((f) & 0x38) == (2 << 3)) ? (READ_PIXEL_4(a)) : \
	 (((f) & 0x38) == (3 << 3)) ? (READ_PIXEL_8(a)) : \
	 (((f) & 0x38) == (4 << 3)) ? (READ_PIXEL_16(a)) : \
	 (((f) & 0x38) == (5 << 3)) ? (READ_PIXEL_32(a)) : 0)

#define READ_ZDATA(a,f) \
	((((f) & 0x38) == (0 << 3)) ? (READ_ZDATA_1(a)) : \
	 (((f) & 0x38) == (1 << 3)) ? (READ_ZDATA_2(a)) : \
	 (((f) & 0x38) == (2 << 3)) ? (READ_ZDATA_4(a)) : \
	 (((f) & 0x38) == (3 << 3)) ? (READ_ZDATA_8(a)) : \
	 (((f) & 0x38) == (4 << 3)) ? (READ_ZDATA_16(a)) : \
	 (((f) & 0x38) == (5 << 3)) ? (READ_ZDATA_32(a)) : 0)

#define WRITE_PIXEL(a,f,d) \
	do \
	{ \
		     if (((f) & 0x38) == (0 << 3)) WRITE_PIXEL_1(a,d); \
		else if (((f) & 0x38) == (1 << 3)) WRITE_PIXEL_2(a,d); \
		else if (((f) & 0x38) == (2 << 3)) WRITE_PIXEL_4(a,d); \
		else if (((f) & 0x38) == (3 << 3)) WRITE_PIXEL_8(a,d); \
		else if (((f) & 0x38) == (4 << 3)) WRITE_PIXEL_16(a,d); \
		else if (((f) & 0x38) == (5 << 3)) WRITE_PIXEL_32(a,d); \
	} while (0)

#define WRITE_ZDATA(a,f,d) \
	do \
	{ \
		     if (((f) & 0x38) == (0 << 3)) WRITE_ZDATA_1(a,d); \
		else if (((f) & 0x38) == (1 << 3)) WRITE_ZDATA_2(a,d); \
		else if (((f) & 0x38) == (2 << 3)) WRITE_ZDATA_4(a,d); \
		else if (((f) & 0x38) == (3 << 3)) WRITE_ZDATA_8(a,d); \
		else if (((f) & 0x38) == (4 << 3)) WRITE_ZDATA_16(a,d); \
		else if (((f) & 0x38) == (5 << 3)) WRITE_ZDATA_32(a,d); \
	} while (0)
#endif

static void FUNCNAME(UINT32 command, UINT32 a1flags, UINT32 a2flags)
{
	UINT32 a1_base = blitter_regs[A1_BASE];
	INT32 a1_pitch = (A1FIXED & 3) ^ ((A1FIXED & 2) >> 1);
	INT32 a1_zoffs = (A1FIXED >> 6) & 7;
	INT32 a1_width = ((4 | ((a1flags >> 9) & 3)) << ((a1flags >> 11) & 15)) >> 2;
	INT32 a1_xadd = (A1FIXED >> 16) & 0x03;
	INT32 a1_yadd = (A1FIXED >> 18) & 0x01;
	INT32 a1_x = (blitter_regs[A1_PIXEL] << 16) | (blitter_regs[A1_FPIXEL] & 0xffff);
	INT32 a1_y = (blitter_regs[A1_PIXEL] & 0xffff0000) | (blitter_regs[A1_FPIXEL] >> 16);
	INT32 a1_xstep = 0;
	INT32 a1_ystep = 0;
	UINT32 a1_xmask = 0xffffffff;
	UINT32 a1_ymask = 0xffffffff;

	UINT32 a2_base = blitter_regs[A2_BASE];
	INT32 a2_pitch = (A2FIXED & 3) ^ ((A2FIXED & 2) >> 1);
	INT32 a2_zoffs = (A2FIXED >> 6) & 7;
	INT32 a2_width = ((4 | ((a2flags >> 9) & 3)) << ((a2flags >> 11) & 15)) >> 2;
	INT32 a2_xadd = (A2FIXED >> 16) & 0x03;
	INT32 a2_yadd = (A2FIXED >> 18) & 0x01;
	INT32 a2_x = (blitter_regs[A2_PIXEL] << 16);
	INT32 a2_y = (blitter_regs[A2_PIXEL] & 0xffff0000);
	INT32 a2_xstep = 0;
	INT32 a2_ystep = 0;
	UINT32 a2_xmask = 0xffffffff;
	UINT32 a2_ymask = 0xffffffff;

	int inner_count = blitter_regs[B_COUNT] & 0xffff;
	int outer_count = blitter_regs[B_COUNT] >> 16;
	int inner, outer;

	UINT8 a1_phrase_mode = 0;
	UINT8 a2_phrase_mode = 0;

	void *a1_base_mem = get_jaguar_memory(a1_base);
	void *a2_base_mem = get_jaguar_memory(a2_base);

	void *asrc_base_mem =	(COMMAND & 0x00000800) ? a1_base_mem : a2_base_mem;
	UINT32 asrcflags =		(COMMAND & 0x00000800) ? A1FIXED : A2FIXED;
	INT32 asrc_x =			(COMMAND & 0x00000800) ? a1_x : a2_x;
	INT32 asrc_y =			(COMMAND & 0x00000800) ? a1_y : a2_y;
	INT32 asrc_width =		(COMMAND & 0x00000800) ? a1_width : a2_width;
	INT32 asrc_pitch =		(COMMAND & 0x00000800) ? a1_pitch : a2_pitch;
	INT32 asrc_zoffs =		(COMMAND & 0x00000800) ? a1_zoffs : a2_zoffs;
	UINT8 asrc_phrase_mode;
	INT32 asrc_xadd, asrc_xstep, asrc_yadd, asrc_ystep;
	UINT32 asrc_xmask, asrc_ymask;

	void *adest_base_mem =	(COMMAND & 0x00000800) ? a2_base_mem : a1_base_mem;
	UINT32 adestflags =		(COMMAND & 0x00000800) ? A2FIXED : A1FIXED;
	INT32 adest_x =			(COMMAND & 0x00000800) ? a2_x : a1_x;
	INT32 adest_y =			(COMMAND & 0x00000800) ? a2_y : a1_y;
	INT32 adest_width =		(COMMAND & 0x00000800) ? a2_width : a1_width;
	INT32 adest_pitch =		(COMMAND & 0x00000800) ? a2_pitch : a1_pitch;
	INT32 adest_zoffs =		(COMMAND & 0x00000800) ? a2_zoffs : a1_zoffs;
	UINT8 adest_phrase_mode;
	INT32 adest_xadd, adest_xstep, adest_yadd, adest_ystep;
	UINT32 adest_xmask, adest_ymask;

	/* don't blit if pointer bad */
	if (!a1_base_mem || !a2_base_mem)
	{
#if LOG_BAD_BLITS
		logerror("%08X:Blit!\n", activecpu_get_previouspc());
		logerror("  a1_base  = %08X\n", a1_base);
		logerror("  a2_base  = %08X\n", a2_base);
#endif
		return;
	}

	/* determine actual xadd/yadd for A1 */
	a1_yadd <<= 16;
	if (A1FIXED & 0x00100000)
		a1_yadd = -a1_yadd;

	a1_phrase_mode = (a1_xadd == 0);
	if (a1_xadd == 3)
	{
		a1_xadd = (blitter_regs[A1_INC] << 16) | (blitter_regs[A1_FINC] & 0xffff);
		a1_yadd = (blitter_regs[A1_INC] & 0xffff0000) | (blitter_regs[A1_FINC] >> 16);
	}
	else if (a1_xadd == 2)
		a1_xadd = 0;
	else
		a1_xadd = 1 << 16;
	if (A1FIXED & 0x00080000)
		a1_xadd = -a1_xadd;

	/* determine actual xadd/yadd for A2 */
	a2_yadd <<= 16;
	if (A2FIXED & 0x00100000)
		a2_yadd = -a2_yadd;

	a2_phrase_mode = (a2_xadd == 0);
	if (a2_xadd == 2)
		a2_xadd = 0;
	else
		a2_xadd = 1 << 16;
	if (A2FIXED & 0x00080000)
		a2_xadd = -a2_xadd;

	/* set up the A2 mask */
	if (A2FIXED & 0x00008000)
	{
		a2_xmask = (blitter_regs[A2_MASK] << 16) | 0xffff;
		a2_ymask = (blitter_regs[A2_MASK] & 0xffff) | 0xffff;
	}

	/* modify outer loop steps based on command */
	if (command & 0x00000100)
	{
		a1_xstep = blitter_regs[A1_FSTEP] & 0xffff;
		a1_ystep = blitter_regs[A1_FSTEP] >> 16;
	}
	if (command & 0x00000200)
	{
		a1_xstep += blitter_regs[A1_STEP] << 16;
		a1_ystep += blitter_regs[A1_STEP] & 0xffff0000;
	}
	if (command & 0x00000400)
	{
		a2_xstep = blitter_regs[A2_STEP] << 16;
		a2_ystep = blitter_regs[A2_STEP] & 0xffff0000;
	}

	asrc_phrase_mode	= (COMMAND & 0x00000800) ? a1_phrase_mode : a2_phrase_mode;
	asrc_xstep			= (COMMAND & 0x00000800) ? a1_xstep : a2_xstep;
	asrc_ystep			= (COMMAND & 0x00000800) ? a1_ystep : a2_ystep;
	asrc_xadd			= (COMMAND & 0x00000800) ? a1_xadd : a2_xadd;
	asrc_yadd			= (COMMAND & 0x00000800) ? a1_yadd : a2_yadd;
	asrc_xmask			= (COMMAND & 0x00000800) ? a1_xmask : a2_xmask;
	asrc_ymask			= (COMMAND & 0x00000800) ? a1_ymask : a2_ymask;

	adest_phrase_mode	= (COMMAND & 0x00000800) ? a2_phrase_mode : a1_phrase_mode;
	adest_xstep			= (COMMAND & 0x00000800) ? a2_xstep : a1_xstep;
	adest_ystep			= (COMMAND & 0x00000800) ? a2_ystep : a1_ystep;
	adest_xadd			= (COMMAND & 0x00000800) ? a2_xadd : a1_xadd;
	adest_yadd			= (COMMAND & 0x00000800) ? a2_yadd : a1_yadd;
	adest_xmask			= (COMMAND & 0x00000800) ? a2_xmask : a1_xmask;
	adest_ymask			= (COMMAND & 0x00000800) ? a2_ymask : a1_ymask;

#if LOG_BLITS
	logerror("%08X:Blit!\n", activecpu_get_previouspc());
	logerror("  a1_base  = %08X\n", a1_base);
	logerror("  a1_pitch = %d\n", a1_pitch);
	logerror("  a1_psize = %d\n", 1 << ((A1FIXED >> 3) & 7));
	logerror("  a1_width = %d\n", a1_width);
	logerror("  a1_xadd  = %f (phrase=%d)\n", (float)a1_xadd / 65536.0, a1_phrase_mode);
	logerror("  a1_yadd  = %f\n", (float)a1_yadd / 65536.0);
	logerror("  a1_xstep = %f\n", (float)a1_xstep / 65536.0);
	logerror("  a1_ystep = %f\n", (float)a1_ystep / 65536.0);
	logerror("  a1_x     = %f\n", (float)a1_x / 65536.0);
	logerror("  a1_y     = %f\n", (float)a1_y / 65536.0);

	logerror("  a2_base  = %08X\n", a2_base);
	logerror("  a2_pitch = %d\n", a2_pitch);
	logerror("  a2_psize = %d\n", 1 << ((A2FIXED >> 3) & 7));
	logerror("  a2_width = %d\n", a2_width);
	logerror("  a2_xadd  = %f (phrase=%d)\n", (float)a2_xadd / 65536.0, a2_phrase_mode);
	logerror("  a2_yadd  = %f\n", (float)a2_yadd / 65536.0);
	logerror("  a2_xstep = %f\n", (float)a2_xstep / 65536.0);
	logerror("  a2_ystep = %f\n", (float)a2_ystep / 65536.0);
	logerror("  a2_x     = %f\n", (float)a2_x / 65536.0);
	logerror("  a2_y     = %f\n", (float)a2_y / 65536.0);

	logerror("  count    = %d x %d\n", inner_count, outer_count);
	logerror("  command  = %08X\n", COMMAND);
#endif

#if LOG_UNHANDLED_BLITS
	/* check for unhandled command bits */
	if (COMMAND & 0x24003000)
		logerror("Blitter unhandled: these command bits: %08X\n", COMMAND & 0x24003000);
#endif /* LOG_UNHANDLED_BLITS */

	/* top of the outer loop */
	outer = outer_count;
	while (outer--)
	{
		/* top of the inner loop */
		inner = inner_count;
		while (inner--)
		{
			UINT32 srcdata;
			UINT32 srczdata = 0;
			UINT32 dstdata;
			UINT32 dstzdata = 0;
			UINT32 writedata = 0;
			int inhibit = 0;

				/* load src data and Z */
				if (COMMAND & 0x00000001)
				{
				srcdata = READ_PIXEL(asrc, asrcflags);
					if (COMMAND & 0x00000002)
					srczdata = READ_ZDATA(asrc, asrcflags);
					else if (COMMAND & 0x001c020)
					srczdata = READ_RDATA(B_SRCZ1_H, asrc, asrcflags, asrc_phrase_mode);
				}
				else
				{
				srcdata = READ_RDATA(B_SRCD_H, asrc, asrcflags, asrc_phrase_mode);
					if (COMMAND & 0x001c020)
					srczdata = READ_RDATA(B_SRCZ1_H, asrc, asrcflags, asrc_phrase_mode);
				}

				/* load dst data and Z */
				if (COMMAND & 0x00000008)
				{
				dstdata = READ_PIXEL(adest, adestflags);
					if (COMMAND & 0x00000010)
					dstzdata = READ_ZDATA(adest, adestflags);
					else
					dstzdata = READ_RDATA(B_DSTZ_H, adest, adestflags, adest_phrase_mode);
				}
				else
				{
				dstdata = READ_RDATA(B_DSTD_H, adest, adestflags, adest_phrase_mode);
					if (COMMAND & 0x00000010)
					dstzdata = READ_RDATA(B_DSTZ_H, adest, adestflags, adest_phrase_mode);
				}

				/* handle clipping */
				if (COMMAND & 0x00000040)
				{
				if (adest_x < 0 || adest_y < 0 ||
					(adest_x >> 16) >= (blitter_regs[A1_CLIP] & 0x7fff) ||
					(adest_y >> 16) >= ((blitter_regs[A1_CLIP] >> 16) & 0x7fff))
						inhibit = 1;
				}

				/* apply Z comparator */
				if (COMMAND & 0x00040000)
					if (srczdata < dstzdata) inhibit = 1;
				if (COMMAND & 0x00080000)
					if (srczdata == dstzdata) inhibit = 1;
				if (COMMAND & 0x00100000)
					if (srczdata > dstzdata) inhibit = 1;

				/* apply data comparator */
				if (COMMAND & 0x08000000)
				{
					if (!(COMMAND & 0x02000000))
					{
					if (srcdata == READ_RDATA(B_PATD_H, asrc, asrcflags, asrc_phrase_mode))
							inhibit = 1;
					}
					else
					{
					if (dstdata == READ_RDATA(B_PATD_H, adest, adestflags, adest_phrase_mode))
							inhibit = 1;
					}
				}

				/* compute the write data and store */
				if (!inhibit)
				{
					/* handle patterns/additive/LFU */
					if (COMMAND & 0x00010000)
					writedata = READ_RDATA(B_PATD_H, adest, adestflags, adest_phrase_mode);
					else if (COMMAND & 0x00020000)
					{
						writedata = (srcdata & 0xff) + (dstdata & 0xff);
						if (!(COMMAND & 0x00004000) && writedata > 0xff)
							writedata = 0xff;
						writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);
						if (!(COMMAND & 0x00008000) && writedata > 0xfff)
							writedata = 0xfff;
						writedata |= (srcdata & 0xf000) + (dstdata & 0xf000);
					}
					else
					{
						if (COMMAND & 0x00200000)
							writedata |= ~srcdata & ~dstdata;
						if (COMMAND & 0x00400000)
							writedata |= ~srcdata & dstdata;
						if (COMMAND & 0x00800000)
							writedata |= srcdata & ~dstdata;
						if (COMMAND & 0x01000000)
							writedata |= srcdata & dstdata;
					}

				/* handle source shading */
				if (COMMAND & 0x40000000)
				{
					int intensity = srcdata & 0x00ff;
					intensity += (INT8) (blitter_regs[B_Z3] >> 16);
					if (intensity < 0)
						intensity = 0;
					else if (intensity > 0xff)
						intensity = 0xff;
					writedata = (srcdata & 0xff00) | intensity;
				}
				}
				else
					writedata = dstdata;

			if (adest_phrase_mode || (command & 0x10000000) || !inhibit)
				{
					/* write to the destination */
				WRITE_PIXEL(adest, adestflags, writedata);
					if (COMMAND & 0x00000020)
					WRITE_ZDATA(adest, adestflags, srczdata);
				}

				/* update X/Y */
			asrc_x = (asrc_x + asrc_xadd) & asrc_xmask;
			asrc_y = (asrc_y + asrc_yadd) & asrc_ymask;
			adest_x = (adest_x + adest_xadd) & adest_xmask;
			adest_y = (adest_y + adest_yadd) & adest_ymask;
		}

		/* adjust for phrase mode */
		if (asrc_phrase_mode)
		{
			if (asrc_xadd > 0)
				asrc_x += 3 << 16;
			else
				asrc_x -= 3 << 16;
			asrc_x &= ~(3 << 16);
		}
		if (adest_phrase_mode)
		{
			if (adest_xadd > 0)
				adest_x += 3 << 16;
			else
				adest_x -= 3 << 16;
			adest_x &= ~(3 << 16);
		}

		/* update for outer loop */
		asrc_x += asrc_xstep;
		asrc_y += asrc_ystep;
		adest_x += adest_xstep;
		adest_y += adest_ystep;
	}

	/* write values back to registers */
	a1_x =	(COMMAND & 0x00000800) ? asrc_x : adest_x;
	a1_y =	(COMMAND & 0x00000800) ? asrc_y : adest_y;
	a2_x =	(COMMAND & 0x00000800) ? adest_x : asrc_x;
	a2_y =	(COMMAND & 0x00000800) ? adest_y : asrc_y;

	blitter_regs[A1_PIXEL] = (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff);
	blitter_regs[A1_FPIXEL] = (a1_y << 16) | (a1_x & 0xffff);
	blitter_regs[A2_PIXEL] = (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff);
}
