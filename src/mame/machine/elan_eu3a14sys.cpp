// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a14sys.h"

// DMA size and destination are 24-bit here, they're 16-bit on EU3A05

DEFINE_DEVICE_TYPE(ELAN_EU3A14_SYS, elan_eu3a14sys_device, "elan_eu3a14sys", "Elan EU3A14 System")

elan_eu3a14sys_device::elan_eu3a14sys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05commonsys_device(mconfig, ELAN_EU3A14_SYS, tag, owner, clock)
{
}

void elan_eu3a14sys_device::device_start()
{
	elan_eu3a05commonsys_device::device_start();

	save_item(NAME(m_dmaparams));
}

void elan_eu3a14sys_device::device_reset()
{
	elan_eu3a05commonsys_device::device_reset();

	for (int i = 0; i < 9; i++)
		m_dmaparams[i] = 0x00;
}


READ8_MEMBER(elan_eu3a14sys_device::dma_param_r)
{
	return m_dmaparams[offset];
}

WRITE8_MEMBER(elan_eu3a14sys_device::dma_param_w)
{
	m_dmaparams[offset] = data;
}

READ8_MEMBER(elan_eu3a14sys_device::dma_trigger_r)
{
	logerror("%s: dma_trigger_r\n", machine().describe_context());
	return 0;
}

WRITE8_MEMBER(elan_eu3a14sys_device::dma_trigger_w)
{
	uint32_t dmasrc = (m_dmaparams[2] << 16) | (m_dmaparams[1] << 8) | (m_dmaparams[0] << 0);
	uint32_t dmadst = (m_dmaparams[5] << 16) | (m_dmaparams[4] << 8) | (m_dmaparams[3] << 0);
	uint32_t dmalen = (m_dmaparams[8] << 16) | (m_dmaparams[7] << 8) | (m_dmaparams[6] << 0);

	//logerror("%s: dma_trigger_w %02x (src %08x dst %08x size %08x)\n", machine().describe_context(), data, dmasrc, dmadst, dmalen);

	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	address_space& destspace = m_cpu->space(AS_PROGRAM);

	if (data == 0x08)
	{
		for (int i = 0; i < dmalen; i++)
		{
			uint8_t dat = fullbankspace.read_byte(dmasrc + i);
			destspace.write_byte(dmadst + i, dat);
		}
	}
	else
	{
		logerror("UNKNOWN DMA TRIGGER VALUE\n");
	}
}
