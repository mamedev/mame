#ifndef _VIDEO_RDPTRI_H_
#define _VIDEO_RDPTRI_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class MiscState;
class Processor;

class Triangle
{
	public:
		Triangle() { fatalerror("Please don't use the default constructor for N64::RDP::Triangle\n"); }
		Triangle(running_machine* machine, bool shade, bool texture, bool zbuffer);

		void InitFromData(running_machine* machine, bool shade, bool texture, bool zbuffer);
		void Draw();

	private:
		running_machine*	m_machine;
		UINT32*				m_cmd_data;
		MiscState*			m_misc_state;
		Processor*			m_rdp;
		bool				m_shade;
		bool				m_texture;
		bool				m_zbuffer;
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPTRI_H_
