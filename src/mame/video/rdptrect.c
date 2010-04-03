#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

#define LookUpCC(A, B, C, D) m_rdp->GetCCLUT2()[(m_rdp->GetCCLUT1()[(A << 16) | (B << 8) | C] << 8) | D]

void TexRectangle::SetMachine(running_machine* machine)
{
	_n64_state *state = (_n64_state *)machine->driver_data;

	m_machine = machine;
	m_rdp = &state->m_rdp;
	m_other_modes = m_rdp->GetOtherModes();
	m_misc_state = m_rdp->GetMiscState();
}

void TexRectangle::Draw()
{
	switch(m_other_modes->cycle_type)
	{
		case CYCLE_TYPE_1:
		case CYCLE_TYPE_2:
			DrawDefault();
			return;

		case CYCLE_TYPE_COPY:
			DrawCopy();
			return;

		default:
			fatalerror("Unsupported cycle type for Textured Rectangle: %d\n", m_other_modes->cycle_type);
			return;
	}
}

void TexRectangle::DrawDefault()
{
	UINT16 *fb = (UINT16*)&rdram[(m_misc_state->m_fb_address / 4)];
	UINT8 *hb = &m_rdp->GetHiddenBits()[m_misc_state->m_fb_address >> 1];
	UINT16 *zb = (UINT16*)&rdram[m_misc_state->m_zb_address / 4];
	UINT8 *zhb = &m_rdp->GetHiddenBits()[m_misc_state->m_zb_address >> 1];

	UINT32 tilenum = m_tilenum;
	N64::RDP::Tile *tex_tile = &m_rdp->GetTiles()[m_tilenum];
	UINT32 tilenum2 = 0;
	N64::RDP::Tile *tex_tile2 = NULL;

	int x1 = m_xh >> 2;
	int x2 = m_xl >> 2;
	int y1 = m_yh >> 2;
	int y2 = m_yl >> 2;

	if (x2 <= x1)
	{
		x2 = x1 + 1;
	}
	if (y1 == y2)
	{
		y2 = y1 + 1; // Needed by Goldeneye
	}

	if ((m_xl & 3) == 3) // Needed by Mega Man 64
	{
		x2++;
	}
	if ((m_yl & 3) == 3)
	{
		y2++;
	}

	m_rdp->GetTexPipe()->CalculateClampDiffs(tilenum);

	if(m_other_modes->cycle_type == CYCLE_TYPE_2)
	{
		if (!m_other_modes->tex_lod_en)
		{
			tilenum2 = (tilenum + 1) & 7;
			tex_tile2 = &m_rdp->GetTiles()[tilenum2];
		}
		else
		{
			tilenum2 = (tilenum + 1) & 7;
			tex_tile2 = &m_rdp->GetTiles()[tilenum2];
		}
	}

	m_rdp->GetShadeColor()->c = 0;	// Needed by Pilotwings 64

	if(y1 < m_rdp->GetScissor()->m_yh)
	{
		m_t += m_dtdy * (m_rdp->GetScissor()->m_yh - y1);
		y1 = m_rdp->GetScissor()->m_yh;
	}
	if(y2 > m_rdp->GetScissor()->m_yl)
	{
		y2 = m_rdp->GetScissor()->m_yl;
	}
	if(x1 < m_rdp->GetScissor()->m_xh)
	{
		m_s += m_dsdx * (m_rdp->GetScissor()->m_xh - x1);
		x1 = m_rdp->GetScissor()->m_xh;
	}
	if(x2 > m_rdp->GetScissor()->m_xl)
	{
		x2 = m_rdp->GetScissor()->m_xl;
	}
	m_dsdx >>= 5;
	m_dtdy >>= 5;

	int t = ((int)(m_t));

	if(m_flip)
	{
		for (int j = y1; j < y2; j++)
		{
			int fb_index = j * m_misc_state->m_fb_width;
			int mline = 0;
			if(!(m_other_modes->rgb_dither_sel & 2))
			{
				mline = (j & 3) << 2;
			}
			int s = ((int)(m_s));

			for (int i = x1; i < x2; i++)
			{
				N64::RDP::Color c1;
				N64::RDP::Color c2;
				int dith = 0;
				int curpixel = fb_index + i;
				UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
				UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
				UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
				UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

				m_misc_state->m_curpixel_cvg = 8;

				if(m_other_modes->cycle_type == CYCLE_TYPE_1)
				{
					m_rdp->GetTexel0Color()->c = m_rdp->GetTexPipe()->Fetch(t, s, tex_tile);

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
				}
				else
				{
					m_rdp->GetTexel0Color()->c = m_rdp->GetTexPipe()->Fetch(t, s, tex_tile);
					m_rdp->GetTexel1Color()->c = m_rdp->GetTexPipe()->Fetch(t, s, tex_tile2);

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
				}

				if(m_other_modes->rgb_dither_sel == 0)
				{
					dith = m_rdp->GetMagicMatrix()[mline + ((i ^ WORD_ADDR_XOR) & 3)];
				}
				else if(m_other_modes->rgb_dither_sel == 1)
				{
					dith = m_rdp->GetBayerMatrix()[mline + ((i ^ WORD_ADDR_XOR) & 3)];
				}

				bool z_compare_result = true;
				if(m_other_modes->z_compare_en)
				{
					z_compare_result = m_rdp->ZCompare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)m_misc_state->m_primitive_z)<<3, m_misc_state->m_primitive_delta_z);
				}

				if(z_compare_result)
				{
					bool rendered = m_rdp->GetBlender()->Blend(fbcur, hbcur, c1, c2, dith);

					if(m_other_modes->z_update_en && rendered)
					{
						m_rdp->ZStore(zbcur, zhbcur, ((UINT32)m_misc_state->m_primitive_z) << 3, m_misc_state->m_primitive_delta_z);
					}
				}

				s += (int)(m_dsdx);
			}
			t += (int)(m_dtdy);
		}
	}
	else
	{
		for (int j = y1; j < y2; j++)
		{
			int fb_index = j * m_misc_state->m_fb_width;
			int mline = 0;
			if(!(m_other_modes->rgb_dither_sel & 2))
			{
				mline = (j & 3) << 2;
			}

			int s = ((int)(m_s));

			for (int i = x1; i < x2; i++)
			{
				N64::RDP::Color c1;
				N64::RDP::Color c2;
				int dith = 0;
				int curpixel = fb_index + i;
				UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
				UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
				UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
				UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];

				m_misc_state->m_curpixel_cvg = 8;

				if(m_other_modes->cycle_type == CYCLE_TYPE_1)
				{
					m_rdp->GetTexel0Color()->c = m_rdp->GetTexPipe()->Fetch(s, t, tex_tile);

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
				}
				else
				{
					m_rdp->GetTexel0Color()->c = m_rdp->GetTexPipe()->Fetch(s, t, tex_tile);
					m_rdp->GetTexel1Color()->c = m_rdp->GetTexPipe()->Fetch(s, t, tex_tile2);

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
				}

				if(m_other_modes->rgb_dither_sel == 0)
				{
					dith = m_rdp->GetMagicMatrix()[mline + ((i ^ WORD_ADDR_XOR) & 3)];
				}
				else if(m_other_modes->rgb_dither_sel == 1)
				{
					dith = m_rdp->GetBayerMatrix()[mline + ((i ^ WORD_ADDR_XOR) & 3)];
				}

				bool z_compare_result = true;
				if(m_other_modes->z_compare_en)
				{
					z_compare_result = m_rdp->ZCompare(fbcur, hbcur, zbcur, zhbcur, ((UINT32)m_misc_state->m_primitive_z)<<3, m_misc_state->m_primitive_delta_z);
				}

				if(z_compare_result)
				{
					bool rendered = m_rdp->GetBlender()->Blend(fbcur, hbcur, c1, c2, dith);

					if(m_other_modes->z_update_en && rendered)
					{
						m_rdp->ZStore(zbcur, zhbcur, ((UINT32)m_misc_state->m_primitive_z) << 3, m_misc_state->m_primitive_delta_z);
					}
				}

				s += (int)(m_dsdx);
			}
			t += (int)(m_dtdy);
		}
	}
}

void TexRectangle::DrawCopy()
{
	UINT16 *fb = (UINT16*)&rdram[(m_misc_state->m_fb_address / 4)];

	N64::RDP::Tile *tex_tile = &m_rdp->GetTiles()[m_tilenum];

	int x1 = m_xh >> 2;
	int x2 = m_xl >> 2;
	int y1 = m_yh >> 2;
	int y2 = m_yl >> 2;

	if (x2 <= x1)
	{
		x2 = x1 + 1;
	}
	if (y1 == y2)
	{
		y2 = y1 + 1; // Needed by Goldeneye
	}

	m_dsdx /= 4;
	x2 += 1;
	y2 += 1;

	m_rdp->GetShadeColor()->c = 0;	// Needed by Pilotwings 64

	if(y1 < m_rdp->GetScissor()->m_yh)
	{
		m_t += m_dtdy * (m_rdp->GetScissor()->m_yh - y1);
		y1 = m_rdp->GetScissor()->m_yh;
	}
	if(y2 > m_rdp->GetScissor()->m_yl)
	{
		y2 = m_rdp->GetScissor()->m_yl;
	}
	if(x1 < m_rdp->GetScissor()->m_xh)
	{
		m_s += m_dsdx * (m_rdp->GetScissor()->m_xh - x1);
		x1 = m_rdp->GetScissor()->m_xh;
	}
	if(x2 > m_rdp->GetScissor()->m_xl)
	{
		x2 = m_rdp->GetScissor()->m_xl;
	}
	m_dsdx >>= 5;
	m_dtdy >>= 5;

	int t = (int)m_t;

	if(m_flip)
	{
		for (int j = y1; j < y2; j++)
		{
			int fb_index = j * m_misc_state->m_fb_width;
			int s = (int)(m_s);

			for (int i = x1; i < x2; i++)
			{
				m_rdp->GetTexel0Color()->c = m_rdp->GetTexPipe()->Fetch(t, s, tex_tile);

				m_misc_state->m_curpixel_cvg = 8;

				if ((m_rdp->GetTexel0Color()->i.a != 0)||(!m_other_modes->alpha_compare_en))
				{
					fb[(fb_index + i) ^ WORD_ADDR_XOR] = ((m_rdp->GetTexel0Color()->i.r >> 3) << 11) | ((m_rdp->GetTexel0Color()->i.g >> 3) << 6) | ((m_rdp->GetTexel0Color()->i.b >> 3) << 1)|1;
				}
				s += m_dsdx;
			}
			t += m_dtdy;
		}
	}
	else
	{
		for (int j = y1; j < y2; j++)
		{
			int fb_index = j * m_misc_state->m_fb_width;
			int s = (int)(m_s);

			for (int i = x1; i < x2; i++)
			{
				m_rdp->GetTexel0Color()->c = m_rdp->GetTexPipe()->Fetch(s, t, tex_tile);

				m_misc_state->m_curpixel_cvg = 8;

				if ((m_rdp->GetTexel0Color()->i.a != 0)||(!m_other_modes->alpha_compare_en))
				{
					fb[(fb_index + i) ^ WORD_ADDR_XOR] = ((m_rdp->GetTexel0Color()->i.r >> 3) << 11) | ((m_rdp->GetTexel0Color()->i.g >> 3) << 6) | ((m_rdp->GetTexel0Color()->i.b >> 3) << 1)|1;
				}
				s += m_dsdx;
			}
			t += m_dtdy;
		}
	}
}

} // namespace RDP

} // namespace N64