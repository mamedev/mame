#ifndef _VIDEO_RDPTEXPIPE_H_
#define _VIDEO_RDPTEXPIPE_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class OtherModes;
class MiscState;
class Processor;
class Color;

class TexturePipeT
{
	public:
		typedef UINT32 (N64::RDP::TexturePipeT::*TexelFetcher) (INT32 s, INT32 t, INT32 tbase, INT32 tpal);

		TexturePipeT()
		{
			m_maskbits_table[0] = 0x3ff;
			for(int i = 1; i < 16; i++)
			{
				m_maskbits_table[i] = ((UINT16)(0xffff) >> (16 - i)) & 0x3ff;
			}
			m_start_span = false;
			m_precomp_s = 0;
			m_precomp_t = 0;

			for (int idx = 0; idx < 80; idx++)
			{
				TexelFetch[idx] = &N64::RDP::TexturePipeT::_FetchNOP;
			}

			TexelFetch[ 8] = &N64::RDP::TexturePipeT::_FetchRGBA_16_RAW;
			TexelFetch[ 9] = &N64::RDP::TexturePipeT::_FetchRGBA_16_RAW;
			TexelFetch[10] = &N64::RDP::TexturePipeT::_FetchRGBA_16_TLUT0;
			TexelFetch[11] = &N64::RDP::TexturePipeT::_FetchRGBA_16_TLUT1;
			TexelFetch[12] = &N64::RDP::TexturePipeT::_FetchRGBA_32_RAW;
			TexelFetch[13] = &N64::RDP::TexturePipeT::_FetchRGBA_32_RAW;
			TexelFetch[14] = &N64::RDP::TexturePipeT::_FetchRGBA_32_TLUT0;
			TexelFetch[15] = &N64::RDP::TexturePipeT::_FetchRGBA_32_TLUT1;

			TexelFetch[24] = &N64::RDP::TexturePipeT::_FetchYUV;
			TexelFetch[25] = &N64::RDP::TexturePipeT::_FetchYUV;
			TexelFetch[26] = &N64::RDP::TexturePipeT::_FetchYUV;
			TexelFetch[27] = &N64::RDP::TexturePipeT::_FetchYUV;

			TexelFetch[32] = &N64::RDP::TexturePipeT::_FetchCI_4_RAW;
			TexelFetch[33] = &N64::RDP::TexturePipeT::_FetchCI_4_RAW;
			TexelFetch[34] = &N64::RDP::TexturePipeT::_FetchCI_4_TLUT0;
			TexelFetch[35] = &N64::RDP::TexturePipeT::_FetchCI_4_TLUT1;
			TexelFetch[36] = &N64::RDP::TexturePipeT::_FetchCI_8_RAW;
			TexelFetch[37] = &N64::RDP::TexturePipeT::_FetchCI_8_RAW;
			TexelFetch[38] = &N64::RDP::TexturePipeT::_FetchCI_8_TLUT0;
			TexelFetch[39] = &N64::RDP::TexturePipeT::_FetchCI_8_TLUT1;

			TexelFetch[48] = &N64::RDP::TexturePipeT::_FetchIA_4_RAW;
			TexelFetch[49] = &N64::RDP::TexturePipeT::_FetchIA_4_RAW;
			TexelFetch[50] = &N64::RDP::TexturePipeT::_FetchIA_4_TLUT0;
			TexelFetch[51] = &N64::RDP::TexturePipeT::_FetchIA_4_TLUT1;
			TexelFetch[52] = &N64::RDP::TexturePipeT::_FetchIA_8_RAW;
			TexelFetch[53] = &N64::RDP::TexturePipeT::_FetchIA_8_RAW;
			TexelFetch[54] = &N64::RDP::TexturePipeT::_FetchIA_8_TLUT0;
			TexelFetch[55] = &N64::RDP::TexturePipeT::_FetchIA_8_TLUT1;
			TexelFetch[56] = &N64::RDP::TexturePipeT::_FetchIA_16_RAW;
			TexelFetch[57] = &N64::RDP::TexturePipeT::_FetchIA_16_RAW;
			TexelFetch[58] = &N64::RDP::TexturePipeT::_FetchIA_16_TLUT0;
			TexelFetch[59] = &N64::RDP::TexturePipeT::_FetchIA_16_TLUT1;

			TexelFetch[64] = &N64::RDP::TexturePipeT::_FetchI_4_RAW;
			TexelFetch[65] = &N64::RDP::TexturePipeT::_FetchI_4_RAW;
			TexelFetch[66] = &N64::RDP::TexturePipeT::_FetchI_4_TLUT0;
			TexelFetch[67] = &N64::RDP::TexturePipeT::_FetchI_4_TLUT1;
			TexelFetch[68] = &N64::RDP::TexturePipeT::_FetchI_8_RAW;
			TexelFetch[69] = &N64::RDP::TexturePipeT::_FetchI_8_RAW;
			TexelFetch[70] = &N64::RDP::TexturePipeT::_FetchI_8_TLUT0;
			TexelFetch[71] = &N64::RDP::TexturePipeT::_FetchI_8_TLUT1;
		}

		void				Cycle(Color* TEX, Color* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle);
		void				Copy(Color* TEX, INT32 SSS, INT32 SST, UINT32 tilenum);
		UINT32				Fetch(INT32 SSS, INT32 SST, INT32 tile);
		void				CalculateClampDiffs(UINT32 prim_tile);
		void				LOD1Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc);
		void				LOD2Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, INT32* t2);
		void				LOD2CycleLimited(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1);

		void				SetMachine(running_machine& machine);

		bool				m_start_span;
		INT32				m_precomp_s;
		INT32				m_precomp_t;

	private:
		UINT32 				Expand16To32Table[0x10000];

		void				Mask(INT32* S, INT32* T, INT32 num);
		void				MaskCoupled(INT32* S, INT32* S1, INT32* T, INT32* T1, INT32 num);

		void				ShiftCycle(INT32* S, INT32* T, INT32* maxs, INT32* maxt, UINT32 num);
		void				ShiftCopy(INT32* S, INT32* T, UINT32 num);

		void				ClampCycle(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, INT32 num);
		void				ClampCycleLight(INT32* S, INT32* T, bool maxs, bool maxt, INT32 num);

		UINT32				_FetchNOP(INT32 s, INT32 t, INT32 tbase, INT32 tpal);

		UINT32				_FetchRGBA_16_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchRGBA_16_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchRGBA_16_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchRGBA_32_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchRGBA_32_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchRGBA_32_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);

		UINT32				_FetchYUV(INT32 s, INT32 t, INT32 tbase, INT32 tpal);

		UINT32				_FetchCI_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchCI_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchCI_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchCI_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchCI_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchCI_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);

		UINT32				_FetchIA_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchIA_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchIA_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchIA_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchIA_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchIA_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchIA_16_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchIA_16_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchIA_16_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);

		UINT32				_FetchI_4_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchI_4_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchI_4_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchI_8_TLUT0(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchI_8_TLUT1(INT32 s, INT32 t, INT32 tbase, INT32 tpal);
		UINT32				_FetchI_8_RAW(INT32 s, INT32 t, INT32 tbase, INT32 tpal);

		TexelFetcher		TexelFetch[16*5];

		running_machine*	m_machine;
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;
		Processor*			m_rdp;

		INT32				m_maskbits_table[16];
		INT32				m_clamp_t_diff[8];
		INT32				m_clamp_s_diff[8];
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPTEXPIPE_H_
