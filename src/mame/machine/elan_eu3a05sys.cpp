// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a05sys.h"

// DMA size and destination are 16-bit here, they're 24-bit on EU3A14

DEFINE_DEVICE_TYPE(ELAN_EU3A05_SYS, elan_eu3a05sys_device, "elan_eu3a05sys", "Elan EU3A05 System")

elan_eu3a05sys_device::elan_eu3a05sys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05commonsys_device(mconfig, ELAN_EU3A05_SYS, tag, owner, clock)
{
}

void elan_eu3a05sys_device::device_start()
{
	elan_eu3a05commonsys_device::device_start();

	save_item(NAME(m_dmaparams));
}

void elan_eu3a05sys_device::device_reset()
{
	elan_eu3a05commonsys_device::device_reset();

	for (int i = 0; i < 7; i++)
		m_dmaparams[i] = 0x00;
}

READ8_MEMBER(elan_eu3a05sys_device::dma_param_r)
{
	return m_dmaparams[offset];
}

WRITE8_MEMBER(elan_eu3a05sys_device::dma_param_w)
{
	m_dmaparams[offset] = data;
}


READ8_MEMBER(elan_eu3a05sys_device::elan_eu3a05_dmatrg_r)
{
	logerror("%s: elan_eu3a05_dmatrg_r (DMA operation state?)\n", machine().describe_context());
	return 0x00;//m_dmatrg_data;
}


WRITE8_MEMBER(elan_eu3a05sys_device::elan_eu3a05_dmatrg_w)
{
	logerror("%s: elan_eu3a05_dmatrg_w (trigger DMA operation) %02x\n", machine().describe_context(), data);
	//m_dmatrg_data = data;

	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	address_space& destspace = m_cpu->space(AS_PROGRAM);

	if (data)
	{
		int src = (m_dmaparams[0]) | (m_dmaparams[1] << 8) | (m_dmaparams[2] << 16);
		uint16_t dest = m_dmaparams[3] | (m_dmaparams[4] << 8);
		uint16_t size = m_dmaparams[5] | (m_dmaparams[6] << 8);

		logerror(" Doing DMA %06x to %04x size %04x\n", src, dest, size);

		for (int i = 0; i < size; i++)
		{
			uint8_t dat = fullbankspace.read_byte(src + i);
			destspace.write_byte(dest + i, dat);
		}
	}
}



