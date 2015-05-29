// license:BSD-3-Clause
// copyright-holders:smf
#include "fdc37c665gt.h"

fdc37c665gt_device::fdc37c665gt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, FDC37C665GT, "FDC37C665GT", tag, owner, clock, "fdc37c665gt", __FILE__),
	m_uart1(*this, "uart1"),
	m_uart2(*this, "uart2")
{
}

READ8_MEMBER(fdc37c665gt_device::read)
{
	UINT8 data = 0;

	if ((offset & 0x3f8) == 0x3f8)
	{
		data = m_uart1->ins8250_r(space, offset & 7, mem_mask);
	}
	else if ((offset & 0x3f8) == 0x2f8)
	{
		data = m_uart2->ins8250_r(space, offset & 7, mem_mask);
	}
	else
	{
		printf("fdc37c665gt_device::read %04x %02x\n", offset, data);
	}
	return data;
}

WRITE8_MEMBER(fdc37c665gt_device::write)
{
	if ((offset & 0x3f8) == 0x3f8)
	{
		m_uart1->ins8250_w(space, offset & 7, data, mem_mask);
	}
	else if ((offset & 0x3f8) == 0x2f8)
	{
		m_uart2->ins8250_w(space, offset & 7, data, mem_mask);
	}
	else
	{
		printf("fdc37c665gt_device::write %04x %02x\n", offset, data);
	}
}

void fdc37c665gt_device::device_start()
{
}

static MACHINE_CONFIG_FRAGMENT(fdc37c665gt)
	MCFG_DEVICE_ADD("uart1", NS16550, XTAL_24MHz/13)
	MCFG_DEVICE_ADD("uart2", NS16550, XTAL_24MHz/13)
MACHINE_CONFIG_END

machine_config_constructor fdc37c665gt_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(fdc37c665gt);
}

const device_type FDC37C665GT = &device_creator<fdc37c665gt_device>;
