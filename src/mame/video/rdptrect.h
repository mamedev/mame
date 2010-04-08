#ifndef _VIDEO_RDPTEXRECT_H_
#define _VIDEO_RDPTEXRECT_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class OtherModes;
class MiscState;
class Processor;
class Color;

class TexRectangle
{
	public:
		TexRectangle() {}
		TexRectangle(running_machine *machine, UINT32 *data, int flip)
		{
			SetMachine(machine);
			InitFromBuffer(data);
			m_flip = flip;
		}

		void Draw();
		void SetMachine(running_machine *machine);
		void InitFromBuffer(UINT32 *data);

		int m_tilenum;
		UINT16 m_xl;	// 10.2 fixed-point
		UINT16 m_yl;	// 10.2 fixed-point
		UINT16 m_xh;	// 10.2 fixed-point
		UINT16 m_yh;	// 10.2 fixed-point
		INT16 m_s;		// 10.5 fixed-point
		INT16 m_t;		// 10.5 fixed-point
		INT16 m_dsdx;	// 5.10 fixed-point
		INT16 m_dtdy;	// 5.10 fixed-point
		int m_flip;

	private:
		void				DrawDefault();
		void				DrawCopy();

		running_machine*	m_machine;
		Processor*			m_rdp;
		MiscState*			m_misc_state;
		OtherModes*			m_other_modes;
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPTEXRECT_H_
