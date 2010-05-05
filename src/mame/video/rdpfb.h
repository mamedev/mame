#ifndef _VIDEO_RDPFB_H_
#define _VIDEO_RDPFB_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class OtherModes;
class MiscState;

class Framebuffer
{
	public:
		Framebuffer() { }

		bool				Write(void* fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b);

		void				SetOtherModes(OtherModes* other_modes) { m_other_modes = other_modes; }
		void				SetMiscState(MiscState* misc_state) { m_misc_state = misc_state; }

	private:
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;

		bool				Write16Bit(UINT16* fb, UINT8* hb, UINT32 r, UINT32 g, UINT32 b);
		bool				Write32Bit(UINT32* fb, UINT32 r, UINT32 g, UINT32 b);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPFB_H_
