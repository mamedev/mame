#ifndef _VIDEO_RDPBLEND_H_
#define _VIDEO_RDPBLEND_H_

#include "emu.h"

struct OtherModesT;
struct MiscStateT;
class n64_rdp;
struct rdp_span_aux;
class Color;
struct rdp_poly_state;

class N64BlenderT
{
	public:
		N64BlenderT()
		{
		}

		bool				Blend2Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int bsel0, int bsel1, rdp_span_aux *userdata, const rdp_poly_state& object);
		bool				Blend1Cycle(UINT32* fr, UINT32* fg, UINT32* fb, int dith, int adseed, int partialreject, int special_bsel, rdp_span_aux *userdata, const rdp_poly_state& object);

		void				SetMachine(running_machine& machine) { m_machine = &machine; }
		void				SetProcessor(n64_rdp* rdp) { m_rdp = rdp; }

		running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }

	private:
		running_machine*	m_machine;
		n64_rdp*			m_rdp;

		void				BlendEquationCycle0(INT32* r, INT32* g, INT32* b, int bsel_special, rdp_span_aux *userdata, const rdp_poly_state& object);
		void				BlendEquationCycle1(INT32* r, INT32* g, INT32* b, int bsel_special, rdp_span_aux *userdata, const rdp_poly_state& object);

		bool				AlphaCompare(UINT8 alpha, const rdp_span_aux *userdata, const rdp_poly_state& object);

		void				DitherRGB(INT32* r, INT32* g, INT32* b, int dith);
		void				DitherA(UINT8* a, int dith);
};

#endif // _VIDEO_RDPBLEND_H_
