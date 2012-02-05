#ifndef _VIDEO_RDPBLEND_H_
#define _VIDEO_RDPBLEND_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class OtherModesT;
class MiscStateT;
class Processor;
class Color;

class BlenderT
{
	public:
		BlenderT()
		{
			BlendEnable = false;
		}

		bool				Blend2Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int bsel0, int bsel1);
		bool				Blend1Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel);

		void				SetMachine(running_machine& machine) { m_machine = &machine; }
		void				SetProcessor(Processor* rdp) { m_rdp = rdp; }

		running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

		bool				BlendEnable;
		INT32				ShiftA;
		INT32				ShiftB;

	private:
		running_machine*	m_machine;
		Processor*			m_rdp;

		void				BlendEquationCycle0(INT32* r, INT32* g, INT32* b, int bsel_special);
		void				BlendEquationCycle1(INT32* r, INT32* g, INT32* b, int bsel_special);

		bool				AlphaCompare(UINT8 alpha);

		void				DitherRGB(INT32* r, INT32* g, INT32* b, int dith);
		void				DitherA(UINT8* a, int dith);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPBLEND_H_
