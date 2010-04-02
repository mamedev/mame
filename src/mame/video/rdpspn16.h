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

		void	Draw(int index, int tilenum, bool shade, bool texture, bool zbuffer, bool flip);
		void	Dump();
		void	SetMachine(running_machine* machine);

	public:
		int m_lx;
		int m_rx;

		SpanParam m_s;
		SpanParam m_t;
		SpanParam m_w;
		SpanParam m_r;
		SpanParam m_g;
		SpanParam m_b;
		SpanParam m_a;
		SpanParam m_z;

		UINT8 m_cvg[RDP_CVG_SPAN_MAX];

		int m_dymax;

		SpanParam m_ds;
		SpanParam m_dt;
		SpanParam m_dw;
		SpanParam m_dr;
		SpanParam m_dg;
		SpanParam m_db;
		SpanParam m_da;
		SpanParam m_dz;

		int m_dzpix;

	private:
		running_machine* m_machine;
		Processor* m_rdp;
		MiscState* m_misc_state;
		OtherModes* m_other_modes;
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPSPAN_H_
