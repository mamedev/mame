// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_dmac.h

    SH DMA controller

***************************************************************************/

#include "emu.h"
#include "sh_dmac.h"
#include "sh7042.h"

DEFINE_DEVICE_TYPE(SH_DMAC, sh_dmac_device, "sh_dmac", "SH DMA controller")
DEFINE_DEVICE_TYPE(SH_DMAC_CHANNEL, sh_dmac_channel_device, "sh_dmac_channel", "SH DMA controller channel")

sh_dmac_device::sh_dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_DMAC, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG)
{
}

void sh_dmac_device::device_start()
{
	save_item(NAME(m_dmaor));
}

void sh_dmac_device::device_reset()
{
	m_dmaor = 0;
}

u16 sh_dmac_device::dmaor_r()
{
	logerror("dmaor_r\n");
	return m_dmaor;
}

void sh_dmac_device::dmaor_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dmaor);
	logerror("dmaor_w %04x\n", m_dmaor);
}

sh_dmac_channel_device::sh_dmac_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_DMAC_CHANNEL, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG)
{
}

void sh_dmac_channel_device::device_start()
{
	save_item(NAME(m_sar));
	save_item(NAME(m_dar));
	save_item(NAME(m_dmatcr));
	save_item(NAME(m_chcr));
}

void sh_dmac_channel_device::device_reset()
{
	m_sar = 0;
	m_dar = 0;
	m_dmatcr = 0;
	m_chcr = 0;
}

u32 sh_dmac_channel_device::sar_r()
{
	logerror("sar_r\n");
	return m_sar;
}

void sh_dmac_channel_device::sar_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_sar);
	logerror("sar_w %08x\n", m_sar);
}

u32 sh_dmac_channel_device::dar_r()
{
	logerror("dar_r\n");
	return m_dar;
}

void sh_dmac_channel_device::dar_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dar);
	logerror("dar_w %08x\n", m_dar);
}

u32 sh_dmac_channel_device::dmatcr_r()
{
	logerror("dmatcr_r\n");
	return m_dmatcr;
}

void sh_dmac_channel_device::dmatcr_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dmatcr);
	logerror("dmatcr_w %08x\n", m_dmatcr);
}

u32 sh_dmac_channel_device::chcr_r()
{
	logerror("chcr_r\n");
	return m_chcr;
}

void sh_dmac_channel_device::chcr_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_chcr);
	logerror("chcr_w %08x\n", m_chcr);
}
