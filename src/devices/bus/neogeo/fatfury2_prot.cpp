// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood

#include "emu.h"
#include "fatfury2_prot.h"



extern const device_type FATFURY2_PROT = &device_creator<fatfury2_prot_device>;


fatfury2_prot_device::fatfury2_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FATFURY2_PROT, "NeoGeo Protection (Fatal Fury 2)", tag, owner, clock, "fatfury2_prot", __FILE__),
	m_bankdev(NULL),
	m_fatfury2_prot_data(0)
{
}


void fatfury2_prot_device::device_start()
{
	save_item(NAME(m_fatfury2_prot_data));
}

void fatfury2_prot_device::device_reset()
{
}



/************************ Fatal Fury 2 *************************/

READ16_MEMBER( fatfury2_prot_device::fatfury2_protection_16_r )
{
	UINT16 res = m_fatfury2_prot_data >> 24;

	switch (offset)
	{
		case 0x55550/2:
		case 0xffff0/2:
		case 0x00000/2:
		case 0xff000/2:
		case 0x36000/2:
		case 0x36008/2:
			return res;

		case 0x36004/2:
		case 0x3600c/2:
			return ((res & 0xf0) >> 4) | ((res & 0x0f) << 4);

		default:
			logerror("unknown protection read at pc %06x, offset %08x\n", space.device().safe_pc(), offset << 1);
			return 0;
	}
}


WRITE16_MEMBER( fatfury2_prot_device::fatfury2_protection_16_w )
{
	switch (offset)
	{
		case 0x11112/2: /* data == 0x1111; expects 0xff000000 back */
			m_fatfury2_prot_data = 0xff000000;
			break;

		case 0x33332/2: /* data == 0x3333; expects 0x0000ffff back */
			m_fatfury2_prot_data = 0x0000ffff;
			break;

		case 0x44442/2: /* data == 0x4444; expects 0x00ff0000 back */
			m_fatfury2_prot_data = 0x00ff0000;
			break;

		case 0x55552/2: /* data == 0x5555; read back from 55550, ffff0, 00000, ff000 */
			m_fatfury2_prot_data = 0xff00ff00;
			break;

		case 0x56782/2: /* data == 0x1234; read back from 36000 *or* 36004 */
			m_fatfury2_prot_data = 0xf05a3601;
			break;

		case 0x42812/2: /* data == 0x1824; read back from 36008 *or* 3600c */
			m_fatfury2_prot_data = 0x81422418;
			break;

		case 0x55550/2:
		case 0xffff0/2:
		case 0xff000/2:
		case 0x36000/2:
		case 0x36004/2:
		case 0x36008/2:
		case 0x3600c/2:
			m_fatfury2_prot_data <<= 8;
			break;

		default:
			logerror("unknown protection write at pc %06x, offset %08x, data %02x\n", space.device().safe_pc(), offset, data);
			break;
	}
}


void fatfury2_prot_device::fatfury2_install_protection(cpu_device* maincpu, neogeo_banked_cart_device* bankdev)
{
	/* the protection involves reading and writing addresses in the */
	/* 0x2xxxxx range. There are several checks all around the code. */
	maincpu->space(AS_PROGRAM).install_readwrite_handler(0x200000, 0x2fffff, read16_delegate(FUNC(fatfury2_prot_device::fatfury2_protection_16_r),this), write16_delegate(FUNC(fatfury2_prot_device::fatfury2_protection_16_w),this));
	m_bankdev = bankdev;

	m_fatfury2_prot_data = 0;

}
