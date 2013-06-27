#ifndef _VIDEO_RDPTEXPIPE_H_
#define _VIDEO_RDPTEXPIPE_H_

#include "emu.h"

struct OtherModesT;
struct MiscStateT;
class Color;
struct rdp_span_aux;
struct rdp_poly_state;

class N64TexturePipeT
{
	public:
		typedef UINT32 (N64TexturePipeT::*TexelFetcher) (INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);

		N64TexturePipeT()
		{
			m_maskbits_table[0] = 0x3ff;
			for(int i = 1; i < 16; i++)
			{
				m_maskbits_table[i] = ((UINT16)(0xffff) >> (16 - i)) & 0x3ff;
			}
			m_start_span = false;

			for (int idx = 0; idx < 80; idx++)
			{
				TexelFetch[idx] = &N64TexturePipeT::_FetchNOP;
			}

			TexelFetch[ 8] = &N64TexturePipeT::_FetchRGBA_16_RAW;
			TexelFetch[ 9] = &N64TexturePipeT::_FetchRGBA_16_RAW;
			TexelFetch[10] = &N64TexturePipeT::_FetchRGBA_16_TLUT0;
			TexelFetch[11] = &N64TexturePipeT::_FetchRGBA_16_TLUT1;
			TexelFetch[12] = &N64TexturePipeT::_FetchRGBA_32_RAW;
			TexelFetch[13] = &N64TexturePipeT::_FetchRGBA_32_RAW;
			TexelFetch[14] = &N64TexturePipeT::_FetchRGBA_32_TLUT0;
			TexelFetch[15] = &N64TexturePipeT::_FetchRGBA_32_TLUT1;

			TexelFetch[24] = &N64TexturePipeT::_FetchYUV;
			TexelFetch[25] = &N64TexturePipeT::_FetchYUV;
			TexelFetch[26] = &N64TexturePipeT::_FetchYUV;
			TexelFetch[27] = &N64TexturePipeT::_FetchYUV;

			TexelFetch[32] = &N64TexturePipeT::_FetchCI_4_RAW;
			TexelFetch[33] = &N64TexturePipeT::_FetchCI_4_RAW;
			TexelFetch[34] = &N64TexturePipeT::_FetchCI_4_TLUT0;
			TexelFetch[35] = &N64TexturePipeT::_FetchCI_4_TLUT1;
			TexelFetch[36] = &N64TexturePipeT::_FetchCI_8_RAW;
			TexelFetch[37] = &N64TexturePipeT::_FetchCI_8_RAW;
			TexelFetch[38] = &N64TexturePipeT::_FetchCI_8_TLUT0;
			TexelFetch[39] = &N64TexturePipeT::_FetchCI_8_TLUT1;

			TexelFetch[48] = &N64TexturePipeT::_FetchIA_4_RAW;
			TexelFetch[49] = &N64TexturePipeT::_FetchIA_4_RAW;
			TexelFetch[50] = &N64TexturePipeT::_FetchIA_4_TLUT0;
			TexelFetch[51] = &N64TexturePipeT::_FetchIA_4_TLUT1;
			TexelFetch[52] = &N64TexturePipeT::_FetchIA_8_RAW;
			TexelFetch[53] = &N64TexturePipeT::_FetchIA_8_RAW;
			TexelFetch[54] = &N64TexturePipeT::_FetchIA_8_TLUT0;
			TexelFetch[55] = &N64TexturePipeT::_FetchIA_8_TLUT1;
			TexelFetch[56] = &N64TexturePipeT::_FetchIA_16_RAW;
			TexelFetch[57] = &N64TexturePipeT::_FetchIA_16_RAW;
			TexelFetch[58] = &N64TexturePipeT::_FetchIA_16_TLUT0;
			TexelFetch[59] = &N64TexturePipeT::_FetchIA_16_TLUT1;

			TexelFetch[64] = &N64TexturePipeT::_FetchI_4_RAW;
			TexelFetch[65] = &N64TexturePipeT::_FetchI_4_RAW;
			TexelFetch[66] = &N64TexturePipeT::_FetchI_4_TLUT0;
			TexelFetch[67] = &N64TexturePipeT::_FetchI_4_TLUT1;
			TexelFetch[68] = &N64TexturePipeT::_FetchI_8_RAW;
			TexelFetch[69] = &N64TexturePipeT::_FetchI_8_RAW;
			TexelFetch[70] = &N64TexturePipeT::_FetchI_8_TLUT0;
			TexelFetch[71] = &N64TexturePipeT::_FetchI_8_TLUT1;
		}

		void                Cycle(Color* TEX, Color* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle, rdp_span_aux *userdata, const rdp_poly_state& object, INT32 *m_clamp_s_diff, INT32 *m_clamp_t_diff);
		void                Copy(Color* TEX, INT32 SSS, INT32 SST, UINT32 tilenum, const rdp_poly_state& object, rdp_span_aux *userdata);
		UINT32              Fetch(INT32 SSS, INT32 SST, INT32 tile, const rdp_poly_state& object, rdp_span_aux *userdata);
		void                CalculateClampDiffs(UINT32 prim_tile, rdp_span_aux *userdata, const rdp_poly_state& object, INT32 *m_clamp_s_diff, INT32 *m_clamp_t_diff);
		void                LOD1Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, rdp_span_aux *userdata, const rdp_poly_state& object);
		void                LOD2Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, INT32* t2, rdp_span_aux *userdata, const rdp_poly_state& object);
		void                LOD2CycleLimited(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, const rdp_poly_state& object);

		void                SetMachine(running_machine& machine);

		bool                m_start_span;

	private:
		UINT32              Expand16To32Table[0x10000];

		void                Mask(INT32* S, INT32* T, INT32 num, const rdp_poly_state& object);
		void                MaskCoupled(INT32* S, INT32* S1, INT32* T, INT32* T1, INT32 num, const rdp_poly_state& object);

		void                ShiftCycle(INT32* S, INT32* T, INT32* maxs, INT32* maxt, UINT32 num, const rdp_poly_state& object);
		void                ShiftCopy(INT32* S, INT32* T, UINT32 num, const rdp_poly_state& object);

		void                ClampCycle(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, INT32 num, rdp_span_aux *userdata, const rdp_poly_state& object, INT32 *m_clamp_s_diff, INT32 *m_clamp_t_diff);
		void                ClampCycleLight(INT32* S, INT32* T, bool maxs, bool maxt, INT32 num, rdp_span_aux *userdata, const rdp_poly_state& object, INT32 *m_clamp_s_diff, INT32 *m_clamp_t_diff);

		UINT32              _FetchNOP(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);

		UINT32              _FetchRGBA_16_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchRGBA_16_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchRGBA_16_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchRGBA_32_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchRGBA_32_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchRGBA_32_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);

		UINT32              _FetchYUV(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);

		UINT32              _FetchCI_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchCI_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchCI_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchCI_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchCI_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchCI_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);

		UINT32              _FetchIA_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchIA_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchIA_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchIA_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchIA_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchIA_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchIA_16_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchIA_16_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchIA_16_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);

		UINT32              _FetchI_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchI_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchI_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchI_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchI_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);
		UINT32              _FetchI_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal, rdp_span_aux *userdata);

		TexelFetcher        TexelFetch[16*5];

		n64_rdp*            m_rdp;

		INT32               m_maskbits_table[16];
};

#endif // _VIDEO_RDPTEXPIPE_H_
