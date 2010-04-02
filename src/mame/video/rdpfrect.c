#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

#define LookUpCC(A, B, C, D) m_rdp->GetCCLUT2()[(m_rdp->GetCCLUT1()[(A << 16) | (B << 8) | C] << 8) | D]

void Rectangle::SetMachine(running_machine* machine)
{
	_n64_state *state = (_n64_state *)machine->driver_data;

	m_machine = machine;
	m_rdp = &state->m_rdp;
	m_misc_state = m_rdp->GetMiscState();
	m_other_modes = m_rdp->GetOtherModes();
	m_blender = m_rdp->GetBlender();
}

void Rectangle::Draw()
{
	switch(m_other_modes->cycle_type)
	{
		case CYCLE_TYPE_1:
			Draw1Cycle();
			return;

		case CYCLE_TYPE_2:
			Draw2Cycle();
			return;

		case CYCLE_TYPE_FILL:
			DrawFill();
			return;

		default:
			fatalerror("Unsupported cycle type for Textured Rectangle: %d\n", m_other_modes->cycle_type);
			return;
	}
}

void Rectangle::Draw1Cycle()
{
	UINT16 *fb = (UINT16*)&rdram[(m_misc_state->m_fb_address / 4)];
	UINT8* hb = &m_rdp->GetHiddenBits()[m_misc_state->m_fb_address >> 1];

	int index, i, j;
	int x1 = m_xh;
	int x2 = m_xl;
	int y1 = m_yh;
	int y2 = m_yl;
	UINT16 fill_color1 = (m_rdp->GetFillColor32() >> 16) & 0xffff;
	UINT16 fill_color2 = (m_rdp->GetFillColor32() >>  0) & 0xffff;
	int fill_cvg1 = (fill_color1 & 1) ? 8 : 1;
	int fill_cvg2 = (fill_color2 & 1) ? 8 : 1;

	if (x2 <= x1)
	{
		x2=x1+1; // SCARS (E)
	}
	if (y2 == y1)
	{
		y2=y1+1; // Goldeneye
	}

	// clip
	if (x1 < m_rdp->GetScissor()->m_xh)
	{
		x1 = m_rdp->GetScissor()->m_xh;
	}
	if (y1 < m_rdp->GetScissor()->m_yh)
	{
		y1 = m_rdp->GetScissor()->m_yh;
	}
	if (x2 >= m_rdp->GetScissor()->m_xl)
	{
		x2 = m_rdp->GetScissor()->m_xl - 1;
	}
	if (y2 >= m_rdp->GetScissor()->m_yl)
	{
		y2 = m_rdp->GetScissor()->m_yl - 1;
	}

	m_rdp->GetShadeColor()->c = 0;	// Needed by Command & Conquer menus

	if(m_other_modes->rgb_dither_sel == 0)
	{
		for (int j = y1; j <= y2; j++)
		{
			Color c1;
			int dith = 0;
			index = j * m_misc_state->m_fb_width;
			for (i = x1; i <= x2; i++)
			{
				m_misc_state->m_curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;

				c1.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[1]);
				c1.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[1]);
				c1.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[1]);
				c1.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[1],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[1],
								  *m_rdp->GetColorInputs()->combiner_alphamul[1],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[1]);
				m_rdp->GetAlphaCvg(&c1.i.a);

				dith = m_rdp->GetMagicMatrix()[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];

				m_blender->Blend(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c1, *m_rdp->GetZero(), dith);
			}
		}
	}
	else if(m_other_modes->rgb_dither_sel == 1)
	{
		for (j = y1; j <= y2; j++)
		{
			Color c1;
			int dith = 0;
			index = j * m_misc_state->m_fb_width;
			for (i = x1; i <= x2; i++)
			{
				m_misc_state->m_curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;

				c1.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[1]);
				c1.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[1]);
				c1.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[1]);
				c1.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[1],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[1],
								  *m_rdp->GetColorInputs()->combiner_alphamul[1],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[1]);
				m_rdp->GetAlphaCvg(&c1.i.a);

				dith = m_rdp->GetBayerMatrix()[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
				m_blender->Blend(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c1, *m_rdp->GetZero(), dith);
			}
		}
	}
	else
	{
		for (j = y1; j <= y2; j++)
		{
			Color c1;
			index = j * m_misc_state->m_fb_width;
			for (i = x1; i <= x2; i++)
			{
				m_misc_state->m_curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;

				c1.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[1]);
				c1.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[1]);
				c1.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[1]);
				c1.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[1],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[1],
								  *m_rdp->GetColorInputs()->combiner_alphamul[1],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[1]);
				m_rdp->GetAlphaCvg(&c1.i.a);

				m_blender->Blend(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c1, *m_rdp->GetZero(), 0);
			}
		}
	}
}

void Rectangle::Draw2Cycle()
{
	UINT16 *fb = (UINT16*)&rdram[(m_misc_state->m_fb_address / 4)];
	UINT8* hb = &m_rdp->GetHiddenBits()[m_misc_state->m_fb_address >> 1];

	int index, i, j;
	int x1 = m_xh;
	int x2 = m_xl;
	int y1 = m_yh;
	int y2 = m_yl;
	UINT16 fill_color1;
	UINT16 fill_color2;
	int fill_cvg1;
	int fill_cvg2;

	if (x2 <= x1)
	{
		x2=x1+1; // SCARS (E)
	}
	if (y2 == y1)
	{
		y2=y1+1; // Goldeneye
	}


	fill_color1 = (m_rdp->GetFillColor32() >> 16) & 0xffff;
	fill_color2 = (m_rdp->GetFillColor32() >>  0) & 0xffff;
	fill_cvg1 = (fill_color1 & 1) ? 8 : 1;
	fill_cvg2 = (fill_color2 & 1) ? 8 : 1;

	// clip
	if (x1 < m_rdp->GetScissor()->m_xh)
	{
		x1 = m_rdp->GetScissor()->m_xh;
	}
	if (y1 < m_rdp->GetScissor()->m_yh)
	{
		y1 = m_rdp->GetScissor()->m_yh;
	}
	if (x2 >= m_rdp->GetScissor()->m_xl)
	{
		x2 = m_rdp->GetScissor()->m_xl - 1;
	}
	if (y2 >= m_rdp->GetScissor()->m_yl)
	{
		y2 = m_rdp->GetScissor()->m_yl - 1;
	}

	m_rdp->GetShadeColor()->c = 0;	// Needed by Command & Conquer menus

	if(m_other_modes->rgb_dither_sel == 0)
	{
		for (j=y1; j <= y2; j++)
		{
			Color c1;
			Color c2;
			int dith = 0;
			index = j * m_misc_state->m_fb_width;
			for (i=x1; i <= x2; i++)
			{
				m_misc_state->m_curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;

				c1.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[0]);
				c1.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[0]);
				c1.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[0]);
				c1.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[0],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[0],
								  *m_rdp->GetColorInputs()->combiner_alphamul[0],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[0]);
				m_rdp->GetCombinedColor()->c = c1.c;
				c2.c = m_rdp->GetTexel0Color()->c;
				m_rdp->GetTexel0Color()->c = m_rdp->GetTexel1Color()->c;
				m_rdp->GetTexel1Color()->c = c2.c;
				c2.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[1]);
				c2.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[1]);
				c2.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[1]);
				c2.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[1],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[1],
								  *m_rdp->GetColorInputs()->combiner_alphamul[1],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[1]);
				m_rdp->GetAlphaCvg(&c2.i.a);

				dith = m_rdp->GetMagicMatrix()[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
				m_blender->Blend(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c1, c2, dith);
			}
		}
	}
	else if(m_other_modes->rgb_dither_sel == 1)
	{
		for (j=y1; j <= y2; j++)
		{
			Color c1;
			Color c2;
			int dith = 0;
			index = j * m_misc_state->m_fb_width;
			for (i=x1; i <= x2; i++)
			{
				m_misc_state->m_curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;

				c1.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[0]);
				c1.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[0]);
				c1.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[0]);
				c1.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[0],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[0],
								  *m_rdp->GetColorInputs()->combiner_alphamul[0],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[0]);
				m_rdp->GetCombinedColor()->c = c1.c;
				c2.c = m_rdp->GetTexel0Color()->c;
				m_rdp->GetTexel0Color()->c = m_rdp->GetTexel1Color()->c;
				m_rdp->GetTexel1Color()->c = c2.c;
				c2.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[1]);
				c2.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[1]);
				c2.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[1]);
				c2.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[1],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[1],
								  *m_rdp->GetColorInputs()->combiner_alphamul[1],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[1]);
				m_rdp->GetAlphaCvg(&c2.i.a);

				dith = m_rdp->GetBayerMatrix()[(((j) & 3) << 2) + ((i ^ WORD_ADDR_XOR) & 3)];
				m_blender->Blend(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c1, c2, dith);
			}
		}
	}
	else
	{
		for (j=y1; j <= y2; j++)
		{
			Color c1, c2;
			index = j * m_misc_state->m_fb_width;
			for (i=x1; i <= x2; i++)
			{
				m_misc_state->m_curpixel_cvg = (i & 1) ? fill_cvg1 : fill_cvg2;

				c1.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[0]);
				c1.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[0]);
				c1.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[0],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[0]);
				c1.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[0],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[0],
								  *m_rdp->GetColorInputs()->combiner_alphamul[0],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[0]);
				m_rdp->GetCombinedColor()->c = c1.c;
				c2.c = m_rdp->GetTexel0Color()->c;
				m_rdp->GetTexel0Color()->c = m_rdp->GetTexel1Color()->c;
				m_rdp->GetTexel1Color()->c = c2.c;
				c2.i.r = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_r[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_r[1]);
				c2.i.g = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_g[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_g[1]);
				c2.i.b = LookUpCC(*m_rdp->GetColorInputs()->combiner_rgbsub_a_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbsub_b_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbmul_b[1],
								  *m_rdp->GetColorInputs()->combiner_rgbadd_b[1]);
				c2.i.a = LookUpCC(*m_rdp->GetColorInputs()->combiner_alphasub_a[1],
								  *m_rdp->GetColorInputs()->combiner_alphasub_b[1],
								  *m_rdp->GetColorInputs()->combiner_alphamul[1],
								  *m_rdp->GetColorInputs()->combiner_alphaadd[1]);
				m_rdp->GetAlphaCvg(&c2.i.a);

				m_blender->Blend(&fb[(index + i) ^ WORD_ADDR_XOR], &hb[(index + i) ^ BYTE_ADDR_XOR], c1, c2, 0);
			}
		}
	}
}

void Rectangle::DrawFill()
{
	UINT16 *fb = (UINT16*)&rdram[(m_misc_state->m_fb_address / 4)];
	UINT8* hb = &m_rdp->GetHiddenBits()[m_misc_state->m_fb_address >> 1];

	int index, i, j;
	int x1 = m_xh;
	int x2 = m_xl;
	int y1 = m_yh;
	int y2 = m_yl;
	UINT16 fill_color1;
	UINT16 fill_color2;
	int fill_cvg1;
	int fill_cvg2;

	if (x2 <= x1)
	{
		x2=x1+1; // SCARS (E)
	}
	if (y2 == y1)
	{
		y2=y1+1; // Goldeneye
	}


	fill_color1 = (m_rdp->GetFillColor32() >> 16) & 0xffff;
	fill_color2 = (m_rdp->GetFillColor32() >>  0) & 0xffff;
	fill_cvg1 = (fill_color1 & 1) ? 8 : 1;
	fill_cvg2 = (fill_color2 & 1) ? 8 : 1;

	// clip
	if (x1 < m_rdp->GetScissor()->m_xh)
	{
		x1 = m_rdp->GetScissor()->m_xh;
	}
	if (y1 < m_rdp->GetScissor()->m_yh)
	{
		y1 = m_rdp->GetScissor()->m_yh;
	}
	if (x2 >= m_rdp->GetScissor()->m_xl)
	{
		x2 = m_rdp->GetScissor()->m_xl - 1;
	}
	if (y2 >= m_rdp->GetScissor()->m_yl)
	{
		y2 = m_rdp->GetScissor()->m_yl - 1;
	}

	m_rdp->GetShadeColor()->c = 0;	// Needed by Command & Conquer menus

	fill_cvg1 = (fill_color1 & 1) ? 3 : 0;
	fill_cvg2 = (fill_color2 & 1) ? 3 : 0;

	if(x1 & 1)
	{
		for (j=y1; j <= y2; j++)
		{
			index = j * m_misc_state->m_fb_width;
			for (i=x1; i <= x2; i += 2)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = fill_color2;
				hb[curpixel ^ BYTE_ADDR_XOR] = fill_cvg2;
			}
		}
		for (j=y1; j <= y2; j++)
		{
			index = j * m_misc_state->m_fb_width;
			for (i=x1+1; i <= x2; i += 2)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = fill_color1;
				hb[curpixel ^ BYTE_ADDR_XOR] = fill_cvg1;
			}
		}
	}
	else
	{
		for (j=y1; j <= y2; j++)
		{
			index = j * m_misc_state->m_fb_width;
			for (i=x1; i <= x2; i += 2)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = fill_color1;
				hb[curpixel ^ BYTE_ADDR_XOR] = fill_cvg1;
			}
		}
		for (j=y1; j <= y2; j++)
		{
			index = j * m_misc_state->m_fb_width;
			for (i=x1+1; i <= x2; i += 2)
			{
				int curpixel = index + i;
				fb[curpixel ^ WORD_ADDR_XOR] = fill_color2;
				hb[curpixel ^ BYTE_ADDR_XOR] = fill_cvg2;
			}
		}
	}
}

} // namespace RDP

} // namespace N64
