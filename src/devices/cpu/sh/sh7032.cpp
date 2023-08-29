// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "sh7032.h"

DEFINE_DEVICE_TYPE(SH1_SH7032,  sh1_sh7032_device,  "sh1_sh7032",  "Hitachi SH-1 (SH7032)")


sh1_sh7032_device::sh1_sh7032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH1_SH7032, tag, owner, clock, CPU_TYPE_SH1, address_map_constructor(FUNC(sh1_sh7032_device::sh7032_map), this), 28, 0xc7ffffff)
{
}

void sh1_sh7032_device::device_start()
{
	sh2_device::device_start();

	save_item(NAME(m_sh7032_regs));
}

void sh1_sh7032_device::device_reset()
{
	sh2_device::device_reset();

	std::fill(std::begin(m_sh7032_regs), std::end(m_sh7032_regs), 0);
}

void sh1_sh7032_device::sh7032_map(address_map &map)
{
//  fall-back
	map(0x05fffe00, 0x05ffffff).rw(FUNC(sh1_sh7032_device::sh7032_r), FUNC(sh1_sh7032_device::sh7032_w)); // SH-7032H internal i/o
}

uint16_t sh1_sh7032_device::sh7032_r(offs_t offset)
{
	return m_sh7032_regs[offset];
}

void sh1_sh7032_device::sh7032_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sh7032_regs[offset]);
}
