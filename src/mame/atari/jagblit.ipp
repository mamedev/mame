// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari Jaguar blitter

****************************************************************************

    How to use this module:

    #define FUNCNAME -- name of the function to be generated
    #define COMMAND  -- blitter command bits for blitter
    #define A1FIXED  -- fixed A1 flag bits for blitter
    #define A2FIXED  -- fixed A2 flag bits for blitter
    #include "jagblit.hxx"

****************************************************************************/


#ifndef FUNCNAME
#error jagblit.inc -- requires FUNCNAME to be predefined
#endif

#ifndef COMMAND
#error jagblit.inc -- requires COMMAND to be predefined
#endif

#ifndef A1FIXED
#error jagblit.inc -- requires A1FIXED to be predefined
#endif

#ifndef A2FIXED
#error jagblit.inc -- requires A2FIXED to be predefined
#endif

// TODO: fix all these "huh" (relevant for Z scissoring in 3d games)

#ifndef PIXEL_SHIFT_1
#define PIXEL_SHIFT_1(a)      ((~a##_x >> 16) & 7)
#define PIXEL_OFFSET_1(a)     (((uint32_t)a##_y >> 16) * (a##_width / 8) * (1 + a##_pitch) + (((uint32_t)a##_x >> 19) & ~7) * (1 + a##_pitch) + (((uint32_t)a##_x >> 19) & 7))
#define ZDATA_OFFSET_1(a)     0 /* huh? */
#define READ_RDATA_1(r,a,p)   ((p) ? (((((uint8_t *)&m_blitter_regs[r])[BYTE4_XOR_BE(((uint32_t)a##_x >> 19) & 7)]) >> PIXEL_SHIFT_1(a)) & 0x01) : (m_blitter_regs[r] & 0x01))
#define READ_PIXEL_1(a)       ((m_gpu->space(AS_PROGRAM).read_byte(a##_base + PIXEL_OFFSET_1(a)) >> PIXEL_SHIFT_1(a)) & 0x01)
#define READ_ZDATA_1(a)       0 /* huh? */

#define PIXEL_SHIFT_2(a)      ((~a##_x >> 15) & 6)
#define PIXEL_OFFSET_2(a)     (((uint32_t)a##_y >> 16) * (a##_width / 4) * (1 + a##_pitch) + (((uint32_t)a##_x >> 18) & ~7) * (1 + a##_pitch) + (((uint32_t)a##_x >> 18) & 7))
#define ZDATA_OFFSET_2(a)     0 /* huh? */
#define READ_RDATA_2(r,a,p)   ((p) ? (((((uint8_t *)&m_blitter_regs[r])[BYTE4_XOR_BE(((uint32_t)a##_x >> 18) & 7)]) >> PIXEL_SHIFT_2(a)) & 0x03) : (m_blitter_regs[r] & 0x03))
#define READ_PIXEL_2(a)       ((m_gpu->space(AS_PROGRAM).read_byte(a##_base + PIXEL_OFFSET_2(a)) >> PIXEL_SHIFT_2(a)) & 0x03)
#define READ_ZDATA_2(a)       0 /* huh? */

#define PIXEL_SHIFT_4(a)      ((~a##_x >> 14) & 4)
#define PIXEL_OFFSET_4(a)     (((uint32_t)a##_y >> 16) * (a##_width / 2) * (1 + a##_pitch) + (((uint32_t)a##_x >> 17) & ~7) * (1 + a##_pitch) + (((uint32_t)a##_x >> 17) & 7))
#define ZDATA_OFFSET_4(a)     0 /* huh? */
#define READ_RDATA_4(r,a,p)   ((p) ? (((((uint8_t *)&m_blitter_regs[r])[BYTE4_XOR_BE(((uint32_t)a##_x >> 17) & 7)]) >> PIXEL_SHIFT_4(a)) & 0x0f) : (m_blitter_regs[r] & 0x0f))
#define READ_PIXEL_4(a)       ((m_gpu->space(AS_PROGRAM).read_byte(a##_base + PIXEL_OFFSET_4(a)) >> PIXEL_SHIFT_4(a)) & 0x0f)
#define READ_ZDATA_4(a)       0 /* huh? */

#define PIXEL_OFFSET_8(a)     (((uint32_t)a##_y >> 16) * a##_width * (1 + a##_pitch) + (((uint32_t)a##_x >> 16) & ~7) * (1 + a##_pitch) + (((uint32_t)a##_x >> 16) & 7))
#define ZDATA_OFFSET_8(a)     (PIXEL_OFFSET_8(a) + a##_zoffs * 8)
#define READ_RDATA_8(r,a,p)   ((p) ? (((uint8_t *)&m_blitter_regs[r])[BYTE4_XOR_BE(((uint32_t)a##_x >> 16) & 7)]) : (m_blitter_regs[r] & 0xff))
#define READ_PIXEL_8(a)       (m_gpu->space(AS_PROGRAM).read_byte(a##_base + PIXEL_OFFSET_8(a)))
#define READ_ZDATA_8(a)       (m_gpu->space(AS_PROGRAM).read_byte(a##_base + ZDATA_OFFSET_8(a)))

#define PIXEL_OFFSET_16(a)    (((uint32_t)a##_y >> 16) * a##_width * (1 + a##_pitch) + (((uint32_t)a##_x >> 16) & ~3) * (1 + a##_pitch) + (((uint32_t)a##_x >> 16) & 3))
#define ZDATA_OFFSET_16(a)    (PIXEL_OFFSET_16(a) + a##_zoffs * 4)
#define READ_RDATA_16(r,a,p)  ((p) ? (((uint16_t *)&m_blitter_regs[r])[BYTE_XOR_BE(((uint32_t)a##_x >> 16) & 3)]) : (m_blitter_regs[r] & 0xffff))
#define READ_PIXEL_16(a)      (m_gpu->space(AS_PROGRAM).read_word(a##_base + (PIXEL_OFFSET_16(a)<<1)))
#define READ_ZDATA_16(a)      (m_gpu->space(AS_PROGRAM).read_word(a##_base + (ZDATA_OFFSET_16(a)<<1)))

#define PIXEL_OFFSET_32(a)    (((uint32_t)a##_y >> 16) * a##_width * (1 + a##_pitch) + (((uint32_t)a##_x >> 16) & ~1) * (1 + a##_pitch) + (((uint32_t)a##_x >> 16) & 1))
#define ZDATA_OFFSET_32(a)    (PIXEL_OFFSET_32(a) + a##_zoffs * 2)
#define READ_RDATA_32(r,a,p)  ((p) ? (m_blitter_regs[r + (((uint32_t)a##_x >> 16) & 1)]) : m_blitter_regs[r])
#define READ_PIXEL_32(a)      (m_gpu->space(AS_PROGRAM).read_dword(a##_base + (PIXEL_OFFSET_32(a)<<2)))
#define READ_ZDATA_32(a)      (m_gpu->space(AS_PROGRAM).read_dword(a##_base + (ZDATA_OFFSET_32(a)<<2)))

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

#define PIXEL_SHIFT_WRITE_1      ((~adest_x >> 16) & 7)
#define PIXEL_SHIFT_WRITE_2      ((~adest_x >> 15) & 6)
#define PIXEL_SHIFT_WRITE_4      ((~adest_x >> 14) & 4)

#define PIXEL_OFFSET_WRITE_1     (((uint32_t)adest_y >> 16) * (adest_width / 8) * (1 + adest_pitch) + (((uint32_t)adest_x >> 19) & ~7) * (1 + adest_pitch) + (((uint32_t)adest_x >> 19) & 7))
#define PIXEL_OFFSET_WRITE_2     (((uint32_t)adest_y >> 16) * (adest_width / 4) * (1 + adest_pitch) + (((uint32_t)adest_x >> 18) & ~7) * (1 + adest_pitch) + (((uint32_t)adest_x >> 18) & 7))
#define PIXEL_OFFSET_WRITE_4     (((uint32_t)adest_y >> 16) * (adest_width / 2) * (1 + adest_pitch) + (((uint32_t)adest_x >> 17) & ~7) * (1 + adest_pitch) + (((uint32_t)adest_x >> 17) & 7))
#define PIXEL_OFFSET_WRITE_8     (((uint32_t)adest_y >> 16) * adest_width * (1 + adest_pitch) + (((uint32_t)adest_x >> 16) & ~7) * (1 + adest_pitch) + (((uint32_t)adest_x >> 16) & 7))
#define PIXEL_OFFSET_WRITE_16    (((uint32_t)adest_y >> 16) * adest_width * (1 + adest_pitch) + (((uint32_t)adest_x >> 16) & ~3) * (1 + adest_pitch) + (((uint32_t)adest_x >> 16) & 3))
#define PIXEL_OFFSET_WRITE_32    (((uint32_t)adest_y >> 16) * adest_width * (1 + adest_pitch) + (((uint32_t)adest_x >> 16) & ~1) * (1 + adest_pitch) + (((uint32_t)adest_x >> 16) & 1))

#define ZDATA_OFFSET_WRITE_1     0 /* huh? */
#define ZDATA_OFFSET_WRITE_2     0 /* huh? */
#define ZDATA_OFFSET_WRITE_4     0 /* huh? */
#define ZDATA_OFFSET_WRITE_8     (PIXEL_OFFSET_WRITE_8 + adest_zoffs * 8)
#define ZDATA_OFFSET_WRITE_16    (PIXEL_OFFSET_WRITE_16 + adest_zoffs * 4)
#define ZDATA_OFFSET_WRITE_32    (PIXEL_OFFSET_WRITE_32 + adest_zoffs * 2)


#define WRITE_PIXEL_1(d)    do { int writeoffs = PIXEL_OFFSET_WRITE_1; int shift = PIXEL_SHIFT_WRITE_1; uint8_t pix = m_gpu->space(AS_PROGRAM).read_byte(adest_base + (writeoffs)); pix = (pix & ~(0x01 << shift)) | ((d) << shift); m_gpu->space(AS_PROGRAM).write_byte(adest_base + (writeoffs), pix); } while (0)
#define WRITE_ZDATA_1(d)    /* huh? */
#define WRITE_PIXEL_2(d)    do { int writeoffs = PIXEL_OFFSET_WRITE_2; int shift = PIXEL_SHIFT_WRITE_2; uint8_t pix = m_gpu->space(AS_PROGRAM).read_byte(adest_base + (writeoffs)); pix = (pix & ~(0x03 << shift)) | ((d) << shift); m_gpu->space(AS_PROGRAM).write_byte(adest_base + (writeoffs), pix); } while (0)
#define WRITE_ZDATA_2(d)    /* huh? */
#define WRITE_PIXEL_4(d)    do { int writeoffs = PIXEL_OFFSET_WRITE_4; int shift = PIXEL_SHIFT_WRITE_4; uint8_t pix = m_gpu->space(AS_PROGRAM).read_byte(adest_base + (writeoffs)); pix = (pix & ~(0x0f << shift)) | ((d) << shift); m_gpu->space(AS_PROGRAM).write_byte(adest_base + (writeoffs), pix); } while (0)
#define WRITE_ZDATA_4(d)    /* huh? */
#define WRITE_PIXEL_8(d)    do { int writeoffs = PIXEL_OFFSET_WRITE_8;  m_gpu->space(AS_PROGRAM).write_byte(adest_base + (writeoffs), (d)); } while (0)
#define WRITE_ZDATA_8(d)    do { int writeoffs = ZDATA_OFFSET_WRITE_8;  m_gpu->space(AS_PROGRAM).write_byte(adest_base + (writeoffs), (d)); } while (0)
#define WRITE_PIXEL_16(d)   do { int writeoffs = PIXEL_OFFSET_WRITE_16; m_gpu->space(AS_PROGRAM).write_word(adest_base + (writeoffs<<1), (d)); } while (0)
#define WRITE_ZDATA_16(d)   do { int writeoffs = ZDATA_OFFSET_WRITE_16; m_gpu->space(AS_PROGRAM).write_word(adest_base + (writeoffs<<1), (d)); } while (0)
#define WRITE_PIXEL_32(d)   do { int writeoffs = PIXEL_OFFSET_WRITE_32; m_gpu->space(AS_PROGRAM).write_dword(adest_base + (writeoffs<<2), (d)); } while (0)
#define WRITE_ZDATA_32(d)   do { int writeoffs = ZDATA_OFFSET_WRITE_32; m_gpu->space(AS_PROGRAM).write_dword(adest_base + (writeoffs<<2), (d)); } while (0)



#define WRITE_PIXEL(f,d) \
	do \
	{ \
				if (((f) & 0x38) == (0 << 3)) WRITE_PIXEL_1(d); \
		else if (((f) & 0x38) == (1 << 3)) WRITE_PIXEL_2(d); \
		else if (((f) & 0x38) == (2 << 3)) WRITE_PIXEL_4(d); \
		else if (((f) & 0x38) == (3 << 3)) WRITE_PIXEL_8(d); \
		else if (((f) & 0x38) == (4 << 3)) WRITE_PIXEL_16(d); \
		else if (((f) & 0x38) == (5 << 3)) WRITE_PIXEL_32(d); \
	} while (0)

#define WRITE_ZDATA(f,d) \
	do \
	{ \
				if (((f) & 0x38) == (0 << 3)) WRITE_ZDATA_1(d); \
		else if (((f) & 0x38) == (1 << 3)) WRITE_ZDATA_2(d); \
		else if (((f) & 0x38) == (2 << 3)) WRITE_ZDATA_4(d); \
		else if (((f) & 0x38) == (3 << 3)) WRITE_ZDATA_8(d); \
		else if (((f) & 0x38) == (4 << 3)) WRITE_ZDATA_16(d); \
		else if (((f) & 0x38) == (5 << 3)) WRITE_ZDATA_32(d); \
	} while (0)
#endif

void jaguar_state::FUNCNAME(uint32_t command, uint32_t a1flags, uint32_t a2flags)
{
	uint32_t a1_base = m_blitter_regs[A1_BASE] & ~0x7;
	int32_t a1_pitch = (A1FIXED & 3) ^ ((A1FIXED & 2) >> 1);
	int32_t a1_zoffs = (A1FIXED >> 6) & 7;
	int32_t a1_width = ((4 | ((a1flags >> 9) & 3)) << ((a1flags >> 11) & 15)) >> 2;
	int32_t a1_xadd = (A1FIXED >> 16) & 0x03;
	int32_t a1_yadd = (A1FIXED >> 18) & 0x01;
	int32_t a1_x = (m_blitter_regs[A1_PIXEL] << 16) | (m_blitter_regs[A1_FPIXEL] & 0xffff);
	int32_t a1_y = (m_blitter_regs[A1_PIXEL] & 0xffff0000) | (m_blitter_regs[A1_FPIXEL] >> 16);
	int32_t a1_xstep = 0;
	int32_t a1_ystep = 0;
	uint32_t a1_xmask = 0xffffffff;
	uint32_t a1_ymask = 0xffffffff;

	uint32_t a2_base = m_blitter_regs[A2_BASE] & ~0x7;
	int32_t a2_pitch = (A2FIXED & 3) ^ ((A2FIXED & 2) >> 1);
	int32_t a2_zoffs = (A2FIXED >> 6) & 7;
	int32_t a2_width = ((4 | ((a2flags >> 9) & 3)) << ((a2flags >> 11) & 15)) >> 2;
	int32_t a2_xadd = (A2FIXED >> 16) & 0x03;
	int32_t a2_yadd = (A1FIXED >> 18) & 0x01;     // From Jaguar HW errata: "If the A1 Y add control bit is set it will affect both address generators."
	int32_t a2_x = (m_blitter_regs[A2_PIXEL] << 16);
	int32_t a2_y = (m_blitter_regs[A2_PIXEL] & 0xffff0000);
	int32_t a2_xstep = 0;
	int32_t a2_ystep = 0;
	uint32_t a2_xmask = 0xffffffff;
	uint32_t a2_ymask = 0xffffffff;

	int inner_count = m_blitter_regs[B_COUNT] & 0xffff;
	int outer_count = m_blitter_regs[B_COUNT] >> 16;
	int inner, outer;

	uint8_t a1_phrase_mode = 0;
	uint8_t a2_phrase_mode = 0;

	uint32_t asrc_base =    (COMMAND & 0x00000800) ? a1_base : a2_base;
	uint32_t asrcflags =      (COMMAND & 0x00000800) ? A1FIXED : A2FIXED;
	int32_t asrc_x =          (COMMAND & 0x00000800) ? a1_x : a2_x;
	int32_t asrc_y =          (COMMAND & 0x00000800) ? a1_y : a2_y;
	int32_t asrc_width =      (COMMAND & 0x00000800) ? a1_width : a2_width;
	int32_t asrc_pitch =      (COMMAND & 0x00000800) ? a1_pitch : a2_pitch;
	int32_t asrc_zoffs =      (COMMAND & 0x00000800) ? a1_zoffs : a2_zoffs;
	uint8_t asrc_phrase_mode;
	int32_t asrc_xadd, asrc_xstep, asrc_yadd, asrc_ystep;
	uint32_t asrc_xmask, asrc_ymask;

	uint32_t adest_base =  (COMMAND & 0x00000800) ? a2_base : a1_base;
	uint32_t adestflags =     (COMMAND & 0x00000800) ? A2FIXED : A1FIXED;
	int32_t adest_x =         (COMMAND & 0x00000800) ? a2_x : a1_x;
	int32_t adest_y =         (COMMAND & 0x00000800) ? a2_y : a1_y;
	int32_t adest_width =     (COMMAND & 0x00000800) ? a2_width : a1_width;
	int32_t adest_pitch =     (COMMAND & 0x00000800) ? a2_pitch : a1_pitch;
	int32_t adest_zoffs =     (COMMAND & 0x00000800) ? a2_zoffs : a1_zoffs;
	uint8_t adest_phrase_mode;
	int32_t adest_xadd, adest_xstep, adest_yadd, adest_ystep;
	uint32_t adest_xmask, adest_ymask;

	/* determine actual xadd/yadd for A1 */
	a1_yadd <<= 16;
	if (A1FIXED & 0x00100000)
		a1_yadd = -a1_yadd;

	a1_phrase_mode = (a1_xadd == 0);
	if (a1_xadd == 3)
	{
		a1_xadd = (m_blitter_regs[A1_INC] << 16) | (m_blitter_regs[A1_FINC] & 0xffff);
		a1_yadd = (m_blitter_regs[A1_INC] & 0xffff0000) | (m_blitter_regs[A1_FINC] >> 16);
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
		a2_xmask = ((m_blitter_regs[A2_MASK] & 0x0000ffff) << 16) | 0xffff;
		a2_ymask = (m_blitter_regs[A2_MASK] & 0xffff0000) | 0xffff;
	}

	/* modify outer loop steps based on command */
	if (command & 0x00000100)
	{
		a1_xstep = m_blitter_regs[A1_FSTEP] & 0xffff;
		a1_ystep = m_blitter_regs[A1_FSTEP] >> 16;
	}
	if (command & 0x00000200)
	{
		a1_xstep += m_blitter_regs[A1_STEP] << 16;
		a1_ystep += m_blitter_regs[A1_STEP] & 0xffff0000;
	}
	if (command & 0x00000400)
	{
		a2_xstep = m_blitter_regs[A2_STEP] << 16;
		a2_ystep = m_blitter_regs[A2_STEP] & 0xffff0000;
	}

	asrc_phrase_mode    = (COMMAND & 0x00000800) ? a1_phrase_mode : a2_phrase_mode;
	asrc_xstep          = (COMMAND & 0x00000800) ? a1_xstep : a2_xstep;
	asrc_ystep          = (COMMAND & 0x00000800) ? a1_ystep : a2_ystep;
	asrc_xadd           = (COMMAND & 0x00000800) ? a1_xadd : a2_xadd;
	asrc_yadd           = (COMMAND & 0x00000800) ? a1_yadd : a2_yadd;
	asrc_xmask          = (COMMAND & 0x00000800) ? a1_xmask : a2_xmask;
	asrc_ymask          = (COMMAND & 0x00000800) ? a1_ymask : a2_ymask;

	adest_phrase_mode   = (COMMAND & 0x00000800) ? a2_phrase_mode : a1_phrase_mode;
	adest_xstep         = (COMMAND & 0x00000800) ? a2_xstep : a1_xstep;
	adest_ystep         = (COMMAND & 0x00000800) ? a2_ystep : a1_ystep;
	adest_xadd          = (COMMAND & 0x00000800) ? a2_xadd : a1_xadd;
	adest_yadd          = (COMMAND & 0x00000800) ? a2_yadd : a1_yadd;
	adest_xmask         = (COMMAND & 0x00000800) ? a2_xmask : a1_xmask;
	adest_ymask         = (COMMAND & 0x00000800) ? a2_ymask : a1_ymask;

	int gouraud_color[4];
	gouraud_color[0] = (m_blitter_regs[B_PATD_H] >> 16) & 0xff00;
	gouraud_color[1] = m_blitter_regs[B_PATD_H] & 0xff00;
	gouraud_color[2] = (m_blitter_regs[B_PATD_L] >> 16) & 0xff00;
	gouraud_color[3] = m_blitter_regs[B_PATD_L] & 0xff00;

	int gouraud_inten[4];
	gouraud_inten[3] = m_blitter_regs[B_I0] & 0xffffff;
	gouraud_inten[2] = m_blitter_regs[B_I1] & 0xffffff;
	gouraud_inten[1] = m_blitter_regs[B_I2] & 0xffffff;
	gouraud_inten[0] = m_blitter_regs[B_I3] & 0xffffff;

	// TODO: gouraud color increments (bits 31-24, battlesp only?)
	int gouraud_iinc = m_blitter_regs[B_IINC] & 0xffffff;
	if (gouraud_iinc & 0x800000)
		gouraud_iinc |= 0xff000000;

	u32 z_index[4];
	z_index[3] = m_blitter_regs[B_Z0];
	z_index[2] = m_blitter_regs[B_Z1];
	z_index[1] = m_blitter_regs[B_Z2];
	z_index[0] = m_blitter_regs[B_Z3];
	s32 z_inc = m_blitter_regs[B_ZINC];

	u16 a1_clipx = m_blitter_regs[A1_CLIP] & 0x7fff;
	u16 a1_clipy = (m_blitter_regs[A1_CLIP] >> 16) & 0x7fff;

	LOGMASKED(LOG_BLITS, "%s:Blit!\n", machine().describe_context());
	LOGMASKED(LOG_BLITS, "  a1_base  = %08X\n", a1_base);
	LOGMASKED(LOG_BLITS, "  a1_pitch = %d\n", a1_pitch);
	LOGMASKED(LOG_BLITS, "  a1_psize = %d\n", 1 << ((A1FIXED >> 3) & 7));
	LOGMASKED(LOG_BLITS, "  a1_width = %d\n", a1_width);
	LOGMASKED(LOG_BLITS, "  a1_xadd  = %f (phrase=%d)\n", (double)a1_xadd / 65536.0, a1_phrase_mode);
	LOGMASKED(LOG_BLITS, "  a1_yadd  = %f\n", (double)a1_yadd / 65536.0);
	LOGMASKED(LOG_BLITS, "  a1_xstep = %f\n", (double)a1_xstep / 65536.0);
	LOGMASKED(LOG_BLITS, "  a1_ystep = %f\n", (double)a1_ystep / 65536.0);
	LOGMASKED(LOG_BLITS, "  a1_x     = %f\n", (double)a1_x / 65536.0);
	LOGMASKED(LOG_BLITS, "  a1_y     = %f\n", (double)a1_y / 65536.0);

	LOGMASKED(LOG_BLITS, "  a2_base  = %08X\n", a2_base);
	LOGMASKED(LOG_BLITS, "  a2_pitch = %d\n", a2_pitch);
	LOGMASKED(LOG_BLITS, "  a2_psize = %d\n", 1 << ((A2FIXED >> 3) & 7));
	LOGMASKED(LOG_BLITS, "  a2_width = %d\n", a2_width);
	LOGMASKED(LOG_BLITS, "  a2_xadd  = %f (phrase=%d)\n", (double)a2_xadd / 65536.0, a2_phrase_mode);
	LOGMASKED(LOG_BLITS, "  a2_yadd  = %f\n", (double)a2_yadd / 65536.0);
	LOGMASKED(LOG_BLITS, "  a2_xstep = %f\n", (double)a2_xstep / 65536.0);
	LOGMASKED(LOG_BLITS, "  a2_ystep = %f\n", (double)a2_ystep / 65536.0);
	LOGMASKED(LOG_BLITS, "  a2_x     = %f\n", (double)a2_x / 65536.0);
	LOGMASKED(LOG_BLITS, "  a2_y     = %f\n", (double)a2_y / 65536.0);

	LOGMASKED(LOG_BLITS, "  count    = %d x %d\n", inner_count, outer_count);
	LOGMASKED(LOG_BLITS, "  command  = %08X\n", COMMAND);

	/* check for unhandled command bits */
	// NOTE: disabled check for GOURZ (it's pretty obvious everywhere is used)
	if (COMMAND & 0x24000000)
		LOGMASKED(LOG_UNHANDLED_BLITS, "Blitter unhandled: these command bits: %08X\n", COMMAND & 0x24000000);

	/* top of the outer loop */
	outer = outer_count;
	while (outer--)
	{
		/* top of the inner loop */
		inner = inner_count;
		while (inner--)
		{
			uint32_t srcdata;
			uint32_t srczdata = 0;
			uint32_t dstdata;
			uint32_t dstzdata = 0;
			uint32_t writedata = 0;
			int inhibit = 0;

				/* load src data and Z (SRCEN) */
				if (COMMAND & 0x00000001)
				{
					srcdata = READ_PIXEL(asrc, asrcflags);
					// (SRCENZ)
					if (COMMAND & 0x00000002)
						srczdata = READ_ZDATA(asrc, asrcflags);
					else if (COMMAND & 0x001c020)
						srczdata = READ_RDATA(B_SRCZ1_H, asrc, asrcflags, asrc_phrase_mode);
				}
				else
				{
					srcdata = READ_RDATA(B_SRCD_H, asrc, asrcflags, asrc_phrase_mode);
					// (PATDSEL | TOPBEN | TOPNEN | DSTWRZ)
					if (COMMAND & 0x001c020)
						srczdata = READ_RDATA(B_SRCZ1_H, asrc, asrcflags, asrc_phrase_mode);
				}

				/* load dst data and Z (DSTEN) */
				if (COMMAND & 0x00000008)
				{
					dstdata = READ_PIXEL(adest, adestflags);
					// (DSTENZ)
					if (COMMAND & 0x00000010)
						dstzdata = READ_ZDATA(adest, adestflags);
					else
						dstzdata = READ_RDATA(B_DSTZ_H, adest, adestflags, adest_phrase_mode);
				}
				else
				{
					dstdata = READ_RDATA(B_DSTD_H, adest, adestflags, adest_phrase_mode);
					// (DSTENZ)
					if (COMMAND & 0x00000010)
						dstzdata = READ_RDATA(B_DSTZ_H, adest, adestflags, adest_phrase_mode);
				}

				/* handle clipping (CLIP_A1) */
				// NOTE: we need to rollback from the *updated* A1 value target
				// - BIOS spinning cube
				// - atarikrt track borders
				// - spacewar intro
				// - missil3d (uses both clipping types)
				if (COMMAND & 0x00000040)
				{
					s32 target_x = COMMAND & 0x00000800 ? asrc_x : adest_x;
					s32 target_y = COMMAND & 0x00000800 ? asrc_y : adest_y;
					if (target_x < 0 || target_y < 0 ||
						(target_x >> 16) >= a1_clipx ||
						(target_y >> 16) >= a1_clipy)
						inhibit = 1;
				}

				// (GOURZ)
				// cybermor and others
				// TODO: iwar still cuts badly (just in a different way compared to before)
				if (COMMAND & 0x00001000)
				{
					int p = asrc_phrase_mode ? (asrc_x & 3) : 3;
					srczdata = z_index[p] >> 16;
				}

				/* apply Z comparator (ZMODE) */
				if (COMMAND & 0x00040000 && srczdata < dstzdata)
					inhibit = 1;
				if (COMMAND & 0x00080000 && srczdata == dstzdata)
					inhibit = 1;
				if (COMMAND & 0x00100000 && srczdata > dstzdata)
					inhibit = 1;

				// apply bit comparator (BCOMPEN)
				// - missil3d, clubdriv, avsp score/automap, trevmcfr
				if (COMMAND & 0x04000000)
				{
					// TODO: should really shift by 1bpp
					if (srcdata == 0)
						inhibit = 1;
				}

				/* apply data comparator (DCOMPEN) */
				if (COMMAND & 0x08000000)
				{
					// !(CMPDST)
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
					// (PATDSEL)
					if (COMMAND & 0x00010000)
						writedata = READ_RDATA(B_PATD_H, adest, adestflags, adest_phrase_mode);
					else if (COMMAND & 0x00020000) // (ADDDSEL)
					{
						writedata = (srcdata & 0xff) + (dstdata & 0xff);

						// !(TOPBEN)
						// - battlesp/battlesg main menu
						// - hstrike difficulty select (dim in background)
						if (!(COMMAND & 0x00004000))
						{
							s16 s = util::sext(srcdata & 0xff, 8);
							s16 d = dstdata & 0xff;
							s16 sum = s + d;

							writedata = (u32)std::clamp(sum, (s16)0, (s16)0xff);
						}
						writedata |= (srcdata & 0xf00) + (dstdata & 0xf00);

						// !(TOPNEN)
						if (!(COMMAND & 0x00008000) && writedata > 0xfff)
						{
							writedata &= 0xfff;
						}
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

					/* handle source shading (SRCSHADE) */
					if (COMMAND & 0x40000000)
					{
						int intensity = srcdata & 0x00ff;
						intensity += (int8_t) (m_blitter_regs[B_IINC] >> 16);
						if (intensity < 0)
							intensity = 0;
						else if (intensity > 0xff)
							intensity = 0xff;
						writedata = (srcdata & 0xff00) | intensity;
					}

					/* handle gouraud shading (GOURD) */
					if (COMMAND & 0x1000)
					{
						int p = asrc_phrase_mode ? (asrc_x & 3) : 3;
						writedata = ((gouraud_inten[p] >> 16) & 0xff) | gouraud_color[p];

						// TODO: this may not be the right place for the increment
						int intensity = gouraud_inten[p];
						intensity += gouraud_iinc;
						if (intensity < 0) intensity = 0;
						if (intensity > 0xffffff) intensity = 0xffffff;
						gouraud_inten[p] = intensity;
					}
				}
				else
				{
					writedata = dstdata;
					// TODO: verify me
					srczdata = dstzdata;
				}

			// (BKGWREN)
			if ((command & 0x10000000) || !inhibit)
			{
				/* write to the destination */
				WRITE_PIXEL(adestflags, writedata);
				// (DSTWRZ)
				if (COMMAND & 0x00000020)
					WRITE_ZDATA(adestflags, srczdata);
			}

				/* update X/Y */
			asrc_x = (asrc_x + asrc_xadd) & asrc_xmask;
			asrc_y = (asrc_y + asrc_yadd) & asrc_ymask;
			adest_x = (adest_x + adest_xadd) & adest_xmask;
			adest_y = (adest_y + adest_yadd) & adest_ymask;

			// GOURZ increments
			if (COMMAND & 0x00001000)
			{
				int p = asrc_phrase_mode ? (asrc_x & 3) : 3;
				z_index[p] += z_inc;
			}
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
	a1_x =  (COMMAND & 0x00000800) ? asrc_x : adest_x;
	a1_y =  (COMMAND & 0x00000800) ? asrc_y : adest_y;
	a2_x =  (COMMAND & 0x00000800) ? adest_x : asrc_x;
	a2_y =  (COMMAND & 0x00000800) ? adest_y : asrc_y;

	m_blitter_regs[A1_PIXEL] = (a1_y & 0xffff0000) | ((a1_x >> 16) & 0xffff);
	m_blitter_regs[A1_FPIXEL] = (a1_y << 16) | (a1_x & 0xffff);
	m_blitter_regs[A2_PIXEL] = (a2_y & 0xffff0000) | ((a2_x >> 16) & 0xffff);
}
