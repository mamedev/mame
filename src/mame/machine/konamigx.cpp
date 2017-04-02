// license:BSD-3-Clause
// copyright-holders:R. Belmont, Acho A. Tang, Phil Stroffolino, Olivier Galibert
/**************************************************************************
 *
 * machine/konamigx.c - contains various System GX hardware abstractions
 *
 */

#include "emu.h"
#include "includes/konamigx.h"

/***************************************************************************/
/*                                                                         */
/*                           GX Protections                             */
/*                                                                         */
/***************************************************************************/



void konamigx_state::konamigx_esc_alert(uint32_t *srcbase, int srcoffs, int count, int mode) // (WARNING: assumed big endianess)
{
// hand-filled but should be close
static const uint8_t ztable[7][8] =
{
	{5,4,3,2,1,7,6,0},
	{4,3,2,1,0,7,6,5},
	{4,3,2,1,0,7,6,5},
	{3,2,1,0,5,7,4,6},
	{6,5,1,4,3,7,0,2},
	{5,4,3,2,1,7,6,0},
	{5,4,3,2,1,7,6,0}
};

static const uint8_t ptable[7][8] =
{
	{0x00,0x00,0x00,0x10,0x20,0x00,0x00,0x30},
	{0x20,0x20,0x20,0x20,0x20,0x00,0x20,0x20},
	{0x00,0x00,0x00,0x20,0x20,0x00,0x00,0x00},
	{0x10,0x10,0x10,0x20,0x00,0x00,0x10,0x00},
	{0x00,0x00,0x20,0x00,0x10,0x00,0x20,0x20},
	{0x00,0x00,0x00,0x10,0x10,0x00,0x00,0x10},
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10}
};

	int32_t data1, data2, i, j, vpos, hpos, voffs, hoffs, vcorr, hcorr, vmask, magicid;
	uint32_t *src, *srcend, *obj, *objend;
	uint32_t *dst;
	const uint8_t  *zcode, *pcode;

	if (!count || !srcbase) return;

	if (mode == 0)
	{
		src = srcbase + srcoffs;
		dst = m_spriteram;
		data1 = count<<2;
		src += data1; dst += data1; i = -data1; j = -data1;
		do
		{
			data1 = src[i];
			data2 = src[i+1];
			i += 2;
			dst[j] = data1;
			dst[j+1] = data2;
		}
		while (j += 2);
	}
	else
	{
#define EXTRACT_ODD         \
if((data1=obj[0])&0x8000)   \
{                           \
	i      = data1 & 7;       \
	data1 &= 0xff00;          \
	dst[0] = data1 | zcode[i];\
	data1  = obj[1];          \
	dst[1] = data1>>16;       \
	vpos   = data1 & 0xffff;  \
	data1  = obj[2];          \
	vpos  += voffs;           \
	dst[4] = data1;           \
	vpos  &= vmask;           \
	hpos   = data1>>16;       \
	data1  = obj[3];          \
	hpos  += hoffs;           \
	dst[2] = vpos;            \
	dst[3] = hpos;            \
	dst[5] = data1>>16;       \
	i      = pcode[i];        \
	dst[6] = data1| i<<4;     \
	dst += 8;                 \
	if (!(--j)) return;       \
}

#define EXTRACT_EVEN         \
if((data1=obj[0])&0x80000000)\
{                            \
	dst[1] = data1;            \
	data1>>= 16;               \
	i      = data1 & 7;        \
	data1 &= 0xff00;           \
	dst[0] = data1 | zcode[i]; \
	data1  = obj[1];           \
	hpos   = data1 & 0xffff;   \
	vpos   = data1>>16;        \
	hpos  += hoffs;            \
	vpos  += voffs;            \
	data1  = obj[2];           \
	vpos  &= vmask;            \
	dst[3] = hpos;             \
	dst[2] = vpos;             \
	dst[5] = data1;            \
	dst[4] = data1>>16;        \
	data1  = obj[3]>>16;       \
	i      = pcode[i];         \
	dst[6] = data1 | i<<4;     \
	dst += 8;                  \
	if (!(--j)) return;        \
}

		// These suspecious looking flags might tell the ESC chip what zcode/priority combos to use.
		// At the beginning of each sprite chunk there're at least three pointers to the main ROM but
		// I can't make out anything meaningful.
		magicid = srcbase[0x71f0/4];

		vmask = 0x3ff;
		if (magicid != 0x11010111)
		{
			switch (magicid)
			{
				case 0x10010801: i = 6; break;
				case 0x11010010: i = 5; vmask = 0x1ff; break;
				case 0x01111018: i = 4; break;
				case 0x10010011: i = 3;
					//					if ((srcbase[0x1c75]&0xff)==32) m_mixer->K055555_write_reg(K55_BLEND_ENABLES,36); // (TEMPORARY)
				break;
				case 0x11010811: i = 2; break;
				case 0x10000010: i = 1; break;
				default:         i = 0;
			}
			vcorr = srcbase[0x26a0/4] & 0xffff;
			hcorr = srcbase[0x26a4/4] >> 16;
			hcorr -= 10;
		}
		else
			hcorr = vcorr = i = 0;

		zcode = ztable[i];
		pcode = ptable[i];

		dst = m_spriteram;
		j = 256;

		// decode Vic-Viper
		if (srcbase[0x049c/4] & 0xffff0000)
		{
			hoffs = srcbase[0x0502/4] & 0xffff;
			voffs = srcbase[0x0506/4] & 0xffff;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = &srcbase[0x049e/4];
			EXTRACT_ODD
			obj = &srcbase[0x04ae/4];
			EXTRACT_ODD
			obj = &srcbase[0x04be/4];
			EXTRACT_ODD
		}

		// decode Lord British (the designer must be a Richard Garriot fan too:)
		if (srcbase[0x0848/4] & 0x0000ffff)
		{
			hoffs = srcbase[0x08b0/4]>>16;
			voffs = srcbase[0x08b4/4]>>16;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = &srcbase[0x084c/4];
			EXTRACT_EVEN
			obj = &srcbase[0x085c/4];
			EXTRACT_EVEN
			obj = &srcbase[0x086c/4];
			EXTRACT_EVEN
		}

		// decode common sprites
		src = srcbase + srcoffs;
		srcend = src + count * 0x30;
		do
		{
			if (!src[0]) continue;
			i = src[7] & 0xf;
			if (!i) continue; // reject retired or zero-element groups
			i <<= 2;
			hoffs = src[5]>>16;
			voffs = src[6]>>16;
			hoffs -= hcorr;
			voffs -= vcorr;
			obj = src + 8;
			objend = obj + i;

			do
			{
				EXTRACT_EVEN
			}
			while ((obj+=4)<objend);
		}
		while ((src+=0x30)<srcend);

		// clear residual data
		if (j) do { *dst = 0; dst += 8; } while (--j);
	}

#undef EXTRACT_ODD
#undef EXTRACT_EVEN
}

void konamigx_state::fantjour_dma_install()
{
	save_item(NAME(m_fantjour_dma));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xdb0000, 0xdb001f, write32_delegate(FUNC(konamigx_state::fantjour_dma_w),this));
	memset(m_fantjour_dma, 0, sizeof(m_fantjour_dma));
}

WRITE32_MEMBER(konamigx_state::fantjour_dma_w)
{
	COMBINE_DATA(m_fantjour_dma + offset);
	if(!offset && ACCESSING_BITS_24_31) {
		uint32_t sa = m_fantjour_dma[1];
		//      uint16_t ss = (m_fantjour_dma[2] & 0xffff0000) >> 16;
		//      uint32_t sb = ((m_fantjour_dma[2] & 0xffff) << 16) | ((m_fantjour_dma[3] & 0xffff0000) >> 16);

		uint32_t da = ((m_fantjour_dma[3] & 0xffff) << 16) | ((m_fantjour_dma[4] & 0xffff0000) >> 16);
		//      uint16_t ds = m_fantjour_dma[4] & 0xffff;
		uint32_t db = m_fantjour_dma[5];

		//      uint8_t sz1 = m_fantjour_dma[0] >> 8;
		uint8_t sz2 = m_fantjour_dma[0] >> 16;
		uint8_t mode = m_fantjour_dma[0] >> 24;

		uint32_t x   = m_fantjour_dma[6];
		uint32_t i1, i2;

		if(mode == 0x93)
			for(i1=0; i1 <= sz2; i1++)
				for(i2=0; i2 < db; i2+=4) {
					space.write_dword(da, space.read_dword(sa) ^ x);
					da += 4;
					sa += 4;
				}
		else if(mode == 0x8f)
			for(i1=0; i1 <= sz2; i1++)
				for(i2=0; i2 < db; i2+=4) {
					space.write_dword(da, x);
					da += 4;
				}
	}
}
