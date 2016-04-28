// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood,Fabio Priuli


#include "emu.h"
#include "prot_kof98.h"

extern const device_type KOF98_PROT = &device_creator<kof98_prot_device>;


kof98_prot_device::kof98_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KOF98_PROT, "Neo Geo KOF 98 Protection", tag, owner, clock, "kof98_prot", __FILE__),
	m_prot_state(0)
{}


void kof98_prot_device::device_start()
{
	save_item(NAME(m_prot_state));
}

void kof98_prot_device::device_reset()
{
}


/* Kof98 uses an early encryption, quite different from the others */
void kof98_prot_device::decrypt_68k(UINT8* cpurom, UINT32 cpurom_size)
{
	UINT8 *src = cpurom;
	dynamic_buffer dst(0x200000);
	int i, j, k;
	static const UINT32 sec[]={ 0x000000, 0x100000, 0x000004, 0x100004, 0x10000a, 0x00000a, 0x10000e, 0x00000e };
	static const UINT32 pos[]={ 0x000, 0x004, 0x00a, 0x00e };

	memcpy(&dst[0], src, 0x200000);
	for (i = 0x800; i < 0x100000; i += 0x200)
	{
		for (j = 0; j < 0x100; j += 0x10)
		{
			for (k = 0; k < 16; k += 2)
			{
				memcpy(&src[i+j+k],       &dst[i+j+sec[k/2]+0x100], 2);
				memcpy(&src[i+j+k+0x100], &dst[i+j+sec[k/2]],       2);
			}
			if (i >= 0x080000 && i < 0x0c0000)
			{
				for (k = 0; k < 4; k++)
				{
					memcpy(&src[i+j+pos[k]],       &dst[i+j+pos[k]],       2);
					memcpy(&src[i+j+pos[k]+0x100], &dst[i+j+pos[k]+0x100], 2);
				}
			}
			else if (i >= 0x0c0000)
			{
				for (k = 0; k < 4; k++)
				{
					memcpy(&src[i+j+pos[k]],       &dst[i+j+pos[k]+0x100], 2);
					memcpy(&src[i+j+pos[k]+0x100], &dst[i+j+pos[k]],       2);
				}
			}
		}
		memcpy(&src[i+0x000000], &dst[i+0x000000], 2);
		memcpy(&src[i+0x000002], &dst[i+0x100000], 2);
		memcpy(&src[i+0x000100], &dst[i+0x000100], 2);
		memcpy(&src[i+0x000102], &dst[i+0x100100], 2);
	}
	memmove(&src[0x100000], &src[0x200000], 0x400000);

	UINT16* mem16 = (UINT16*)cpurom;
	m_default_rom[0] = mem16[0x100/2];
	m_default_rom[1] = mem16[0x102/2];
}


/************************ King of Fighters 98*******************
  The encrypted set has a rom overlay feature, checked at
  various points in the game.
  Boards used: NEO-MVS PROGSF1 (1998.6.17) / NEO-MVS PROGSF1E (1998.6.18)
  The boards have an ALTERA chip (EPM7128SQC100-15) which is tied to 242-P1
***************************************************************/

READ16_MEMBER(kof98_prot_device::protection_r)
{
	if (m_prot_state == 1)
	{
		if (!offset)
			return 0x00c2;
		else
			return 0x00fd;
	}
	if (m_prot_state == 2)
	{
		if (!offset)
			return 0x4e45;
		else
			return 0x4f2d;
	}

	if (!offset)
		return m_default_rom[0];
	else
		return m_default_rom[1];
}


/* when 0x20aaaa contains 0x0090 (word) then 0x100 (normally the neogeo header) should return 0x00c200fd worked out using real hw */
WRITE16_MEMBER( kof98_prot_device::protection_w )
{
	/* info from razoola */
	switch (data)
	{
	case 0x0090:
		logerror ("%06x kof98 - protection 0x0090x\n", space.device().safe_pc());
		m_prot_state = 1;
		break;

	case 0x00f0:
		logerror ("%06x kof98 - protection 0x00f0x\n", space.device().safe_pc());
		m_prot_state = 2;
		break;

	default: // 00aa is written, but not needed?
		logerror ("%06x kof98 - unknown protection write %04x\n", space.device().safe_pc(), data);
		break;
	}
}

