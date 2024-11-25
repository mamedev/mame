// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_bsc.h

    SH DMA controller

***************************************************************************/

#include "emu.h"
#include "sh_bsc.h"

DEFINE_DEVICE_TYPE(SH_BSC, sh_bsc_device, "sh_bsc", "SH Bus State Controller")

sh_bsc_device::sh_bsc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SH_BSC, tag, owner, clock)
{
}

void sh_bsc_device::device_start()
{
	save_item(NAME(m_bcr1));
	save_item(NAME(m_bcr2));
	save_item(NAME(m_wcr1));
	save_item(NAME(m_wcr2));
	save_item(NAME(m_dcr));
	save_item(NAME(m_rtcsr));
	save_item(NAME(m_rtcnt));
	save_item(NAME(m_rtcor));
}

void sh_bsc_device::device_reset()
{
	m_bcr1 = 0x200f;
	m_bcr2 = 0xffff;
	m_wcr1 = 0xffff;
	m_wcr2 = 0x000f;
	m_dcr = 0;
	m_rtcsr = 0;
	m_rtcnt = 0;
	m_rtcor = 0;
}

u16 sh_bsc_device::bcr1_r()
{
	logerror("bcr1_r\n");
	return m_bcr1;
}

void sh_bsc_device::bcr1_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bcr1);
	logerror("bcr1_w %04x\n", m_bcr1);
}

u16 sh_bsc_device::bcr2_r()
{
	logerror("bcr2_r\n");
	return m_bcr2;
}

void sh_bsc_device::bcr2_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bcr2);
	logerror("bcr2_w %04x\n", m_bcr2);
}

u16 sh_bsc_device::wcr1_r()
{
	logerror("wcr1_r\n");
	return m_wcr1;
}

void sh_bsc_device::wcr1_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_wcr1);
	logerror("wcr1_w %04x\n", m_wcr1);
}

u16 sh_bsc_device::wcr2_r()
{
	logerror("wcr2_r\n");
	return m_wcr2;
}

void sh_bsc_device::wcr2_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_wcr2);
	logerror("wcr2_w %04x\n", m_wcr2);
}

u16 sh_bsc_device::dcr_r()
{
	logerror("dcr_r\n");
	return m_dcr;
}

void sh_bsc_device::dcr_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dcr);
	logerror("dcr_w %04x\n", m_dcr);
}

u16 sh_bsc_device::rtcsr_r()
{
	logerror("rtcsr_r\n");
	return m_rtcsr;
}

void sh_bsc_device::rtcsr_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rtcsr);
	logerror("rtcsr_w %04x\n", m_rtcsr);
}

u16 sh_bsc_device::rtcnt_r()
{
	logerror("rtcnt_r\n");
	return m_rtcnt;
}

void sh_bsc_device::rtcnt_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rtcnt);
	logerror("rtcnt_w %04x\n", m_rtcnt);
}

u16 sh_bsc_device::rtcor_r()
{
	logerror("rtcor_r\n");
	return m_rtcor;
}

void sh_bsc_device::rtcor_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rtcor);
	logerror("rtcor_w %04x\n", m_rtcor);
}
