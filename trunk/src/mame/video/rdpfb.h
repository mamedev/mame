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
		Framebuffer()
		{
			m_pre_wrap = false;
		}

		void				Write(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b);
		void				Read(UINT32 index);
		void				Copy(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b);
		void				Fill(UINT32 curpixel);

		void				SetProcessor(Processor* rdp) { m_rdp = rdp; }
		void				SetOtherModes(OtherModes* other_modes) { m_other_modes = other_modes; }
		void				SetMiscState(MiscState* misc_state) { m_misc_state = misc_state; }
		void				SetPreWrap(bool prewrap) { m_pre_wrap = prewrap; }

	private:
		Processor*			m_rdp;
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;
		bool				m_pre_wrap;

		void				Write16Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b);
		void				Write32Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b);
		void				Read16Bit(UINT32 curpixel);
		void				Read32Bit(UINT32 curpixel);
		void				Copy16Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b);
		void				Copy32Bit(UINT32 curpixel, UINT32 r, UINT32 g, UINT32 b);
		void				Fill16Bit(UINT32 curpixel);
		void				Fill32Bit(UINT32 curpixel);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPFB_H_
