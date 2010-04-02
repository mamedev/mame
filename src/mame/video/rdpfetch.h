#ifndef _VIDEO_RDPFETCH_H_
#define _VIDEO_RDPFETCH_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class Processor;
class OtherModes;
class MiscState;
class Tile;

class TexFetch
{
	public:
		TexFetch() { }

		UINT32				Fetch(UINT32 s, UINT32 t, Tile* tile);

		void				SetMachine(running_machine* machine);

	private:
		running_machine*	m_machine;
		Processor*			m_rdp;
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;
		Tile*				m_tiles;

		UINT32				FetchRGBA(UINT32 s, UINT32 t, Tile* tile);
		UINT32				FetchYUV(UINT32 s, UINT32 t, Tile* tile);
		UINT32				FetchCI(UINT32 s, UINT32 t, Tile* tile);
		UINT32				FetchIA(UINT32 s, UINT32 t, Tile* tile);
		UINT32				FetchI(UINT32 s, UINT32 t, Tile* tile);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPFETCH_H_
