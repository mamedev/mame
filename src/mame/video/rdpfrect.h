#ifndef _VIDEO_RDPFILLRECT_H_
#define _VIDEO_RDPFILLRECT_H_

#include "emu.h"
#include "video/rdpblend.h"

namespace N64
{

namespace RDP
{

class OtherModes;
class MiscState;
class Processor;
class Blender;
class Color;

class Rectangle
{
	public:
		Rectangle() {}
		Rectangle(running_machine *machine, UINT32 *data)
		{
			SetMachine(machine);
			InitFromBuffer(data);
		}

		void Draw();
		void SetMachine(running_machine *machine);
		void InitFromBuffer(UINT32 *data);

		UINT16 m_xl;	// 10.2 fixed-point
		UINT16 m_yl;	// 10.2 fixed-point
		UINT16 m_xh;	// 10.2 fixed-point
		UINT16 m_yh;	// 10.2 fixed-point

	private:
		void 				Draw1Cycle();
		void 				Draw2Cycle();
		void 				DrawFill();

		running_machine*	m_machine;
		Processor*			m_rdp;
		MiscState*			m_misc_state;
		OtherModes*			m_other_modes;
		Blender*			m_blender;
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPFILLRECT_H_