#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

namespace RDP
{

void TexFetch::SetMachine(running_machine *machine)
{
	_n64_state *state = (_n64_state *)machine->driver_data;

	m_machine = machine;
	m_rdp = &state->m_rdp;
	m_other_modes = m_rdp->GetOtherModes();
	m_misc_state = m_rdp->GetMiscState();
	m_tiles = m_rdp->GetTiles();
}

UINT32 TexFetch::Fetch(UINT32 s, UINT32 t, Tile* tile)
{
	UINT32 tformat = tile->format;

	if (t < 0) t = 0;
	if (s < 0) s = 0;

	switch (tformat)
	{
		case 0:		// RGBA
			return FetchRGBA(s, t, tile);

		case 1:		// YUV: Bottom of the 9th, Pokemon Stadium, Ogre Battle 64
			return FetchYUV(s, t, tile);

		case 2:		// Color Index
			return FetchCI(s, t, tile);

		case 3:		// Intensity + Alpha
			return FetchIA(s, t, tile);

		case 4:		// Intensity
			return FetchI(s, t, tile);

		default:
			fatalerror("FETCH_TEXEL: unknown texture format %d\n", tformat);
			return 0xffffffff;
	}

	return 0;
}

UINT32 TexFetch::FetchRGBA(UINT32 s, UINT32 t, Tile* tile)
{
	UINT32 twidth	= tile->line;
	UINT32 tbase =	tile->tmem;
	UINT32 tpal	= tile->palette & 0xf;

	switch (tile->size)
	{
		case PIXEL_SIZE_4BIT:
		{
			UINT8 *tc = (UINT8*)m_rdp->GetTMEM();
			int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
			UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
			UINT16 c = m_rdp->GetTLUT()[(((tpal << 4) | p) ^ WORD_ADDR_XOR) << 2];

			if (m_other_modes->en_tlut)
			{
				if (m_other_modes->tlut_type == 0)
				{
					return m_rdp->LookUp16To32(c);
				}
				else
				{
					return m_rdp->LookUpIA8To32(c);
				}
			}
			else
			{
				return ((tpal << 4) | p) * 0x01010101;
			}
			break;
		}
		case PIXEL_SIZE_8BIT:
		{
			UINT8 *tc = m_rdp->GetTMEM();
			int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
			UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];
			UINT16 c = m_rdp->GetTLUT()[(p ^ WORD_ADDR_XOR) << 2];

			if (m_other_modes->en_tlut)
			{
				if (m_other_modes->tlut_type == 0)
				{
					return m_rdp->LookUp16To32(c);
				}
				else
				{
					return m_rdp->LookUpIA8To32(c);
				}
			}
			else
			{
				return p * 0x01010101;
			}
			break;
		}
		case PIXEL_SIZE_16BIT:
		{
			UINT16 *tc = m_rdp->GetTMEM16();
			int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
			UINT16 c = tc[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

			if (!m_other_modes->en_tlut)
			{
				return m_rdp->LookUp16To32(c);
			}
			else
			{
				if (m_other_modes->tlut_type == 0) //Golden Eye 007, sea, "frigate" level
				{
					return m_rdp->LookUp16To32(m_rdp->GetTLUT()[(c >> 8) << 2]);
				}
				else // Beetle Adventure Racing, Mount Mayhem
				{
					return m_rdp->LookUpIA8To32(m_rdp->GetTLUT()[(c >> 8) << 2]);
				}
			}
			break;
		}
		case PIXEL_SIZE_32BIT:
		{
			UINT32 *tc = m_rdp->GetTMEM32();
			int xorval = (m_misc_state->m_fb_size == PIXEL_SIZE_16BIT) ? XOR_SWAP_WORD : XOR_SWAP_DWORD; // Conker's Bad Fur Day, Jet Force Gemini, Super Smash Bros., Mickey's Speedway USA, Ogre Battle, Wave Race, Gex 3, South Park Rally
			int taddr = (((tbase >> 2) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? xorval : 0)) & 0x3ff;

			if (!m_other_modes->en_tlut)
			{
				return tc[taddr];
			}
			else
			{
				if (!m_other_modes->tlut_type)
				{
					return m_rdp->LookUp16To32(m_rdp->GetTLUT()[(tc[taddr] >> 24) << 2]);
				}
				else
				{
					return m_rdp->LookUpIA8To32(m_rdp->GetTLUT()[(tc[taddr] >> 24) << 2]);
				}
			}
			break;
		}
		default:
			fatalerror("FETCH_TEXEL: unknown RGBA texture size %d\n", tile->size);
			return 0xffffffff;
	}
}

UINT32 TexFetch::FetchYUV(UINT32 s, UINT32 t, Tile* tile)
{
	UINT32 twidth	= tile->line;
	UINT32 tsize =	tile->size;
	UINT32 tbase =	tile->tmem;
	Color out;

	out.c = 0;

	if(tsize == PIXEL_SIZE_16BIT)
	{
		INT32 newr = 0;
		INT32 newg = 0;
		INT32 newb = 0;
		UINT16 *tc = m_rdp->GetTMEM16();
		int taddr = ((tbase >> 1) + ((t) * (twidth)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
		UINT16 c1, c2;
		INT32 y;
		INT32 u, v;
		c1 = tc[taddr ^ WORD_ADDR_XOR];
		c2 = tc[taddr]; // other word

		if (!(taddr & 1))
		{
			v = c2 >> 8;
			u = c1 >> 8;
			y = c1 & 0xff;
		}
		else
		{
			v = c1 >> 8;
			u = c2 >> 8;
			y = c1 & 0xff;
		}
		v -= 128;
		u -= 128;

		if (!m_other_modes->bi_lerp0)
		{
			newr = y + ((m_rdp->GetK0() * v) >> 8);
			newg = y + ((m_rdp->GetK1() * u) >> 8) + ((m_rdp->GetK2() * v) >> 8);
			newb = y + ((m_rdp->GetK2() * u) >> 8);
		}
		out.i.r = (newr < 0) ? 0 : ((newr > 0xff) ? 0xff : newr);
		out.i.g = (newg < 0) ? 0 : ((newg > 0xff) ? 0xff : newg);
		out.i.b = (newb < 0) ? 0 : ((newb > 0xff) ? 0xff : newb);
		out.i.a = 0xff;
	}

	return out.c;
}

UINT32 TexFetch::FetchCI(UINT32 s, UINT32 t, Tile* tile)
{
	UINT32 twidth = tile->line;
	UINT32 tsize =	tile->size;
	UINT32 tbase =	tile->tmem;
	UINT32 tpal	= tile->palette & 0xf;

	switch (tsize)
	{
		case PIXEL_SIZE_4BIT:
		{
			UINT8 *tc = m_rdp->GetTMEM();
			int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
			UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
			UINT16 c = m_rdp->GetTLUT()[((tpal << 4) | p) << 2];

			if (m_other_modes->en_tlut)
			{
				if (m_other_modes->tlut_type == 0)
				{
					return m_rdp->LookUp16To32(c);
				}
				else
				{
					return m_rdp->LookUpIA8To32(c);
				}
			}
			else
			{
				return ((tpal << 4) | p) * 0x01010101;
			}
			break;
		}
		case PIXEL_SIZE_8BIT:
		{
			UINT8 *tc = m_rdp->GetTMEM();
			int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0x7ff;
			UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];
			UINT16 c = m_rdp->GetTLUT()[p << 2];

			if (m_other_modes->en_tlut)
			{
				if (m_other_modes->tlut_type == 0)
				{
					return m_rdp->LookUp16To32(c);
				}
				else
				{
					return m_rdp->LookUpIA8To32(c);
				}
			}
			else
			{
				return p * 0x01010101;
			}
			break;
		}
		case PIXEL_SIZE_16BIT:
		{
			// 16-bit CI is a "valid" mode; some games use it, it behaves the same as 16-bit RGBA
			UINT16 *tc = m_rdp->GetTMEM16();
			int taddr = ((tbase>>1) + ((t) * (twidth>>1)) + (s))  ^ ((t & 1) ? XOR_SWAP_WORD : 0);
			UINT16 c = tc[(taddr & 0x7ff) ^ WORD_ADDR_XOR]; // PGA European Tour (U)

			if (!m_other_modes->en_tlut)
			{
				return m_rdp->LookUp16To32(c);
			}
			else
			{
				if (!m_other_modes->tlut_type) // GoldenEye 007, sea, "frigate" level
				{
					return m_rdp->LookUp16To32(m_rdp->GetTLUT()[(c >> 8) << 2]);
				}
				else // Beetle Adventure Racing, Mount Mayhem
				{
					return m_rdp->LookUpIA8To32(m_rdp->GetTLUT()[(c >> 8) << 2]);
				}
			}
			break;
		}

		default:
			fatalerror("FETCH_TEXEL: unknown CI texture size %d\n", tsize);
			return 0xffffffff;
	}

	return 0;
}

UINT32 TexFetch::FetchIA(UINT32 s, UINT32 t, Tile* tile)
{
	UINT32 twidth = tile->line;
	UINT32 tsize =	tile->size;
	UINT32 tbase =	tile->tmem;
	UINT32 tpal	= tile->palette & 0xf;

	switch (tsize)
	{
		case PIXEL_SIZE_4BIT:
		{
			UINT8 *tc = m_rdp->GetTMEM();
			int taddr = (tbase + ((t) * twidth) + (s >> 1)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0);
			UINT8 p = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
			UINT8 i = ((p & 0xe) << 4) | ((p & 0xe) << 1) | (p & 0xe >> 2);

			if (!m_other_modes->en_tlut)
			{
				return (i * 0x01010100) | ((p & 0x1) ? 0xff : 0);
			}
			else
			{
				UINT16 c = m_rdp->GetTLUT()[((tpal << 4) | p) << 2];
				if (!m_other_modes->tlut_type)
				{
					return m_rdp->LookUp16To32(c);
				}
				else
				{
					return m_rdp->LookUpIA8To32(c);
				}
			}
			break;
		}
		case PIXEL_SIZE_8BIT:
		{
			UINT8 *tc = m_rdp->GetTMEM();
			int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
			UINT8 p = tc[taddr ^ BYTE_ADDR_XOR];

			if (!m_other_modes->en_tlut)
			{
				UINT8 i = (p >> 4) | (p & 0xf0);
				return (i * 0x01010100) | ((p & 0xf) | ((p << 4) & 0xf0));
			}
			else
			{
				UINT16 c = m_rdp->GetTLUT()[p << 2];
				if (!m_other_modes->tlut_type)
				{
					return m_rdp->LookUp16To32(c);
				}
				else
				{
					return m_rdp->LookUpIA8To32(c);
				}
			}
			break;
		}
		case PIXEL_SIZE_16BIT:
		{
			UINT16 *tc = m_rdp->GetTMEM16();
			int taddr = ((tbase >> 1) + ((t) * (twidth >> 1)) + (s)) ^ ((t & 1) ? XOR_SWAP_WORD : 0);
			UINT16 c = tc[taddr ^ WORD_ADDR_XOR];

			if (m_other_modes->en_tlut)
			{
				c = m_rdp->GetTLUT()[(c >> 8) << 2];
				if (m_other_modes->tlut_type == 0)
				{
					return m_rdp->LookUp16To32(c);
				}
				else
				{
					return m_rdp->LookUpIA8To32(c);
				}
			}
			else
			{
				return m_rdp->LookUpIA8To32(c);
			}

			break;
		}
		default:
			return 0xffffffff;
			fatalerror("FETCH_TEXEL: unknown IA texture size %d\n", tsize);
			break;
	}

	return 0;
}

UINT32 TexFetch::FetchI(UINT32 s, UINT32 t, Tile* tile)
{
	UINT32 twidth = tile->line;
	UINT32 tsize =	tile->size;
	UINT32 tbase =	tile->tmem;
	UINT32 tpal	= tile->palette & 0xf;

	switch (tsize)
	{
		case PIXEL_SIZE_4BIT:
		{
			UINT8 *tc = m_rdp->GetTMEM();
			int taddr = ((tbase + ((t) * twidth) + ((s) / 2)) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
			UINT8 c = ((s) & 1) ? (tc[taddr ^ BYTE_ADDR_XOR] & 0xf) : (tc[taddr ^ BYTE_ADDR_XOR] >> 4);
			c |= (c << 4);

			if (!m_other_modes->en_tlut)
			{
				return c * 0x01010101;
			}
			else
			{
				UINT16 k = m_rdp->GetTLUT()[((tpal << 4) | c) << 2];
				if (!m_other_modes->tlut_type)
				{
					return m_rdp->LookUp16To32(k);
				}
				else
				{
					return m_rdp->LookUpIA8To32(k);
				}
			}
			break;
		}
		case PIXEL_SIZE_8BIT:
		{
			UINT8 *tc = m_rdp->GetTMEM();
			int taddr = ((tbase + ((t) * twidth) + ((s))) ^ ((t & 1) ? XOR_SWAP_BYTE : 0)) & 0xfff;
			UINT8 c = tc[taddr ^ BYTE_ADDR_XOR];

			if (!m_other_modes->en_tlut)
			{
				return c * 0x01010101;
			}
			else
			{
				UINT16 k = m_rdp->GetTLUT()[ c << 2];
				if (!m_other_modes->tlut_type)
				{
					return m_rdp->LookUp16To32(k);
				}
				else
				{
					return m_rdp->LookUpIA8To32(k);
				}
			}
			break;
		}
		default:
			//fatalerror("FETCH_TEXEL: unknown I texture size %d\n", tsize);
			return 0xffffffff;
	}

	return 0;
}

} // namespace RDP

} // namespace N64
