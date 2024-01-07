// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 Bus State Controller

***************************************************************************/

#include "emu.h"
#include "sh7014_bsc.h"

// #define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(SH7014_BSC, sh7014_bsc_device, "sh7014bsc", "SH7014 Bus State Controller")


sh7014_bsc_device::sh7014_bsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_BSC, tag, owner, clock)
{
}

void sh7014_bsc_device::device_start()
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

void sh7014_bsc_device::device_reset()
{
	m_bcr1 = 0;
	m_bcr2 = 0xffff;
	m_wcr1 = 0xffff;
	m_wcr2 = 0x000f;
	m_dcr = 0;
	m_rtcsr = 0;
	m_rtcnt = 0;
	m_rtcor = 0;
}

void sh7014_bsc_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(sh7014_bsc_device::bcr1_r), FUNC(sh7014_bsc_device::bcr1_w));
	map(0x02, 0x03).rw(FUNC(sh7014_bsc_device::bcr2_r), FUNC(sh7014_bsc_device::bcr2_w));
	map(0x04, 0x05).rw(FUNC(sh7014_bsc_device::wcr1_r), FUNC(sh7014_bsc_device::wcr1_w));
	map(0x06, 0x07).rw(FUNC(sh7014_bsc_device::wcr2_r), FUNC(sh7014_bsc_device::wcr2_w));
	map(0x0a, 0x0b).rw(FUNC(sh7014_bsc_device::dcr_r), FUNC(sh7014_bsc_device::dcr_w));
	map(0x0c, 0x0d).rw(FUNC(sh7014_bsc_device::rtcsr_r), FUNC(sh7014_bsc_device::rtcsr_w));
	map(0x0e, 0x0f).rw(FUNC(sh7014_bsc_device::rtcnt_r), FUNC(sh7014_bsc_device::rtcnt_w));
	map(0x10, 0x11).rw(FUNC(sh7014_bsc_device::rtcor_r), FUNC(sh7014_bsc_device::rtcor_w));
}

///

uint16_t sh7014_bsc_device::bcr1_r()
{
	// bit 0 [R/W] CS0 Space Size Specification (A0SZ)
	// bit 1 [R/W] CS1 Space Size Specification (A1SZ)
	// bit 2 [R/W] CS2 Space Size Specification (A2SZ)
	// bit 3 [R/W] CS3 Space Size Specification (A3SZ)
	// bit 8 [R/W] Multiplex I/O Enable (IOE)
	// bit 13 [R] Always returns 1
	// Everything else is read-only and will return 0
	return (m_bcr1 & 0x10f) | (1 << 13);
}

void sh7014_bsc_device::bcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr1);
}

uint16_t sh7014_bsc_device::bcr2_r()
{
	return m_bcr2;
}

void sh7014_bsc_device::bcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr2);
}

uint16_t sh7014_bsc_device::wcr1_r()
{
	return m_wcr1;
}

void sh7014_bsc_device::wcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wcr1);
}

uint16_t sh7014_bsc_device::wcr2_r()
{
	return m_wcr2 & 0x3ff;
}

void sh7014_bsc_device::wcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wcr2);
}

uint16_t sh7014_bsc_device::dcr_r()
{
	return m_dcr & ~0x48;
}

void sh7014_bsc_device::dcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dcr);
}

uint16_t sh7014_bsc_device::rtcsr_r()
{
	return m_rtcsr & 0x7f;
}

void sh7014_bsc_device::rtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcsr);
}

uint16_t sh7014_bsc_device::rtcnt_r()
{
	return m_rtcnt & 0xff;
}

void sh7014_bsc_device::rtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcnt);
}

uint16_t sh7014_bsc_device::rtcor_r()
{
	return m_rtcor & 0xff;
}

void sh7014_bsc_device::rtcor_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcor);
}
