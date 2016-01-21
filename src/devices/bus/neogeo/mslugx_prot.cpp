// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood
#include "emu.h"
#include "mslugx_prot.h"



extern const device_type MSLUGX_PROT = &device_creator<mslugx_prot_device>;


mslugx_prot_device::mslugx_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSLUGX_PROT, "NeoGeo Protection (Metal Slug X)", tag, owner, clock, "mslugx_prot", __FILE__),
	m_mslugx_counter(0),
	m_mslugx_command(0)
{
}


void mslugx_prot_device::device_start()
{
	save_item(NAME(m_mslugx_command));
	save_item(NAME(m_mslugx_counter));
}

void mslugx_prot_device::device_reset()
{
}




/************************ Metal Slug X *************************
  Board used: NEO-MVS PROGEOP (1999.2.2)
  The board has an ALTERA chip (EPM7128SQC100-15) which is tied to 250-P1
  Also found is a QFP144 chip labeled with 0103 - function unknown
***************************************************************/

WRITE16_MEMBER( mslugx_prot_device::mslugx_protection_16_w )
{
	switch (offset)
	{
		case 0x0/2: // start new read?
			m_mslugx_command = 0;
		break;

		case 0x2/2: // command? These are pulsed with data and then 0
		case 0x4/2:
			m_mslugx_command |= data;
		break;

		case 0x6/2: // finished?
		break;

		case 0xa/2: // init?
			m_mslugx_counter = 0;
			m_mslugx_command = 0;
		break;

		default:
			logerror("unknown protection write at pc %06x, offset %08x, data %02x\n", space.device().safe_pc(), offset << 1, data);
		break;
	}
}


READ16_MEMBER( mslugx_prot_device::mslugx_protection_16_r )
{
	UINT16 res = 0;

	switch (m_mslugx_command)
	{
		case 0x0001: { // $3bdc(?) and $3c30 (Register D7)
			res = (space.read_byte(0xdedd2 + ((m_mslugx_counter >> 3) & 0xfff)) >> (~m_mslugx_counter & 0x07)) & 1;
			m_mslugx_counter++;
		}
		break;

		case 0x0fff: { // All other accesses (Register D2)
			INT32 select = space.read_word(0x10f00a) - 1; // How should this be calculated?
			res = (space.read_byte(0xdedd2 + ((select >> 3) & 0x0fff)) >> (~select & 0x07)) & 1;
		}
		break;

		default:
			logerror("unknown protection read at pc %06x, offset %08x\n", space.device().safe_pc(), offset << 1);
		break;
	}

	return res;
}


void mslugx_prot_device::mslugx_install_protection(cpu_device* maincpu)
{
	maincpu->space(AS_PROGRAM).install_readwrite_handler(0x2fffe0, 0x2fffef, read16_delegate(FUNC(mslugx_prot_device::mslugx_protection_16_r),this), write16_delegate(FUNC(mslugx_prot_device::mslugx_protection_16_w),this));

}
