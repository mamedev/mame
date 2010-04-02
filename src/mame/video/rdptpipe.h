#ifndef _VIDEO_RDPTEXPIPE_H_
#define _VIDEO_RDPTEXPIPE_H_

#include "emu.h"
#include "video/rdpfetch.h"

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
		}

		UINT32				Fetch(INT32 SSS, INT32 SST, Tile* tile);
		void 				CalculateClampDiffs(UINT32 prim_tile);

		void				SetMachine(running_machine* machine);

	private:
		void 				Mask(INT32* S, INT32* T, Tile* tile);
		void 				TexShift(INT32* S, INT32* T, bool* maxs, bool* maxt, Tile *tile);
		void 				Clamp(INT32* S, INT32* T, INT32* SFRAC, INT32* TFRAC, bool maxs, bool maxt, Tile* tile);
		void 				ClampLight(INT32* S, INT32* T, bool maxs, bool maxt, Tile* tile);

		running_machine*	m_machine;
		OtherModes*			m_other_modes;
		Processor*			m_rdp;
		TexFetch			m_tex_fetch;

		INT32 				m_maskbits_table[16];
		INT32 				m_clamp_t_diff[8];
		INT32 				m_clamp_s_diff[8];
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPTEXPIPE_H_