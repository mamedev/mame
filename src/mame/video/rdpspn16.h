#ifndef _VIDEO_RDPSPAN_H_
#define _VIDEO_RDPSPAN_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

#define RDP_CVG_SPAN_MAX    1024

class OtherModes;
class MiscState;
class Processor;

class SpanParam
{
	public:
		union
		{
			UINT32 w;
#ifdef LSB_FIRST
			struct { UINT16 l; INT16 h; } h;
#else
			struct { INT16 h; UINT16 l; } h;
#endif
		};
};

class Span
{
	public:
		Span() { }

		void	Dump();
		void	SetMachine(running_machine* machine);

		void	Draw1Cycle(int index, int tilenum, bool flip);
		void	Draw2Cycle(int index, int tilenum, bool flip);
		void	DrawCopy(int index, int tilenum, bool flip);
		void	DrawFill(int index, int tilenum, bool flip);

	public:
		int m_lx;
		int m_rx;
		int m_unscissored_rx;

		SpanParam m_s;
		SpanParam m_t;
		SpanParam m_w;
		SpanParam m_r;
		SpanParam m_g;
		SpanParam m_b;
		SpanParam m_a;
		SpanParam m_z;

		UINT16 m_cvg[RDP_CVG_SPAN_MAX];

	private:
		void RGBAZClip(int sr, int sg, int sb, int sa, int *sz);
		void RGBAZCorrectTriangle(INT32 offx, INT32 offy, INT32* r, INT32* g, INT32* b, INT32* a, INT32* z);

		running_machine* m_machine;
		Processor* m_rdp;
		MiscState* m_misc_state;
		OtherModes* m_other_modes;
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPSPAN_H_
