#ifndef _VIDEO_RDPBLEND_H_
#define _VIDEO_RDPBLEND_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class OtherModes;
class MiscState;
class Processor;
class Color;

class Blender
{
	public:
		Blender()
		{
			m_blend_enable = false;
		}

		bool				Blend2Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int bsel0, int bsel1);
		bool				Blend1Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel);

		void				SetOtherModes(OtherModes* other_modes) { m_other_modes = other_modes; }
		void				SetMiscState(MiscState* misc_state) { m_misc_state = misc_state; }
		void				SetMachine(running_machine* machine) { m_machine = machine; }
		void				SetProcessor(Processor* rdp) { m_rdp = rdp; }

		void				SetBlendEnable(bool enable) { m_blend_enable = enable; }
		bool				GetBlendEnable() { return m_blend_enable; }

		void				SetShiftA(INT32 shift) { m_shift_a = shift; }
		void				SetShiftB(INT32 shift) { m_shift_b = shift; }

	private:
		running_machine*	m_machine;
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;
		Processor*			m_rdp;

		bool				m_blend_enable;
		INT32				m_shift_a;
		INT32				m_shift_b;

		void				BlendEquationCycle0(INT32* r, INT32* g, INT32* b, int bsel_special);
		void				BlendEquationCycle1(INT32* r, INT32* g, INT32* b, int bsel_special);

		bool				AlphaCompare(UINT8 alpha);

		void				DitherRGB(INT32* r, INT32* g, INT32* b, int dith);
		void				DitherA(UINT8* a, int dith);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPBLEND_H_
