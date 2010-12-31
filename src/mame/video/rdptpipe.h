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

class TexturePipe
{
	public:
		TexturePipe()
		{
			m_maskbits_table[0] = 0x3ff;
			for(int i = 1; i < 16; i++)
			{
				m_maskbits_table[i] = ((UINT16)(0xffff) >> (16 - i)) & 0x3ff;
			}
			m_start_span = false;
			m_precomp_s = 0;
			m_precomp_t = 0;
		}

		void				Cycle(Color* TEX, Color* prev, INT32 SSS, INT32 SST, UINT32 tilenum, UINT32 cycle);
		void				Copy(Color* TEX, INT32 SSS, INT32 SST, UINT32 tilenum);
		UINT32				Fetch(INT32 SSS, INT32 SST, INT32 tile);
		void				CalculateClampDiffs(UINT32 prim_tile);
		void				LOD1Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc);
		void				LOD2Cycle(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1, INT32* t2);
		void				LOD2CycleLimited(INT32* sss, INT32* sst, INT32 s, INT32 t, INT32 w, INT32 dsinc, INT32 dtinc, INT32 dwinc, INT32 prim_tile, INT32* t1);

		void				SetMachine(running_machine* machine);

		bool				m_start_span;
		INT32				m_precomp_s;
		INT32				m_precomp_t;

	private:
		void				Mask(INT32* S, INT32* T, INT32 num);
		void				MaskCoupled(INT32* S, INT32* S1, INT32* T, INT32* T1, INT32 num);

		void				ShiftCycle(INT32* S, INT32* T, INT32* maxs, INT32* maxt, UINT32 num);
		void				ShiftCopy(INT32* S, INT32* T, UINT32 num);

		void				ClampCycle(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, INT32 maxs, INT32 maxt, INT32 num);
		void				ClampCycleLight(INT32* S, INT32* T, bool maxs, bool maxt, INT32 num);

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
