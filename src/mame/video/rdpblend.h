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
		Blender() { }

		bool				Blend(void* in_fb, UINT8* hb, Color c1, Color c2, int dith);

		void				SetOtherModes(OtherModes* other_modes) { m_other_modes = other_modes; }
		void				SetMiscState(MiscState* misc_state) { m_misc_state = misc_state; }
		void				SetMachine(running_machine* machine) { m_machine = machine; }
		void				SetProcessor(Processor* rdp) { m_rdp = rdp; }

	private:
		running_machine*	m_machine;
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;
		Processor*			m_rdp;

		bool				Blend16Bit(UINT16* fb, UINT8* hb, RDP::Color c1, RDP::Color c2, int dith);
		bool				Blend16Bit1Cycle(UINT16* fb, UINT8* hb, RDP::Color c, int dith);
		bool				Blend16Bit2Cycle(UINT16* fb, UINT8* hb, RDP::Color c1, RDP::Color c2, int dith);
		bool				Blend32Bit(UINT32* fb, RDP::Color c1, RDP::Color c2);
		bool				Blend32Bit1Cycle(UINT32* fb, RDP::Color c);
		bool				Blend32Bit2Cycle(UINT32* fb, RDP::Color c1, RDP::Color c2);
		bool				AlphaCompare(UINT8 alpha);

		void				BlendEquation0Force(INT32* r, INT32* g, INT32* b, int bsel_special);
		void				BlendEquation0NoForce(INT32* r, INT32* g, INT32* b, int bsel_special);
		void				BlendEquation1Force(INT32* r, INT32* g, INT32* b, int bsel_special);
		void				BlendEquation1NoForce(INT32* r, INT32* g, INT32* b, int bsel_special);

		void				DitherRGB(INT32* r, INT32* g, INT32* b, int dith);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPBLEND_H_
