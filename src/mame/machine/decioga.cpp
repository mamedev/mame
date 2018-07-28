// license:BSD-3-Clause
// copyright-holders:R. Belmont
/******************************************************************************
*
*   MIPS DECstation I/O Gate Array emulation
*   This IC contains some address decoding, an interrupt controller, and
*   a multi-channel DMA engine.
*/

#include "emu.h"
#include "decioga.h"

DEFINE_DEVICE_TYPE(DECSTATION_IOGA, dec_ioga_device, "decioga", "DECstation I/O Gate Array")

void dec_ioga_device::map(address_map &map)
{
	map(0x040100, 0x040103).rw(FUNC(dec_ioga_device::csr_r), FUNC(dec_ioga_device::csr_w));
	map(0x040110, 0x040113).rw(FUNC(dec_ioga_device::intr_r), FUNC(dec_ioga_device::intr_w));
	map(0x040120, 0x040123).rw(FUNC(dec_ioga_device::imsk_r), FUNC(dec_ioga_device::imsk_w));
}

dec_ioga_device::dec_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECSTATION_IOGA, tag, owner, clock)
{
}

void dec_ioga_device::device_start()
{
	save_item(NAME(m_csr));
	save_item(NAME(m_intr));
	save_item(NAME(m_imsk));
}

void dec_ioga_device::device_reset()
{
	m_csr = m_intr = m_imsk = 0;
}

READ32_MEMBER(dec_ioga_device::csr_r)
{
	return m_csr;
}

WRITE32_MEMBER(dec_ioga_device::csr_w)
{
	COMBINE_DATA(&m_csr);
#if 0
	printf("%08x to CSR: diag LEDs [", m_csr);
	for (int i = 7; i >= 0; i--)
	{
		printf("%c", (m_csr & (1<<i)) ? 'O' : '.');
	}
	printf("]\n");
#endif
}

READ32_MEMBER(dec_ioga_device::intr_r)
{
	uint32_t rv = m_intr;
	m_intr &= ~0x20;        // 5000/133 boot ROM tests that reading clears this bit
	//printf("m_intr = %08x\n", m_intr);
	return rv;
}

WRITE32_MEMBER(dec_ioga_device::intr_w)
{
	m_intr &= ~data;    // clear bits on write
}

READ32_MEMBER(dec_ioga_device::imsk_r)
{
	return m_imsk;
}

WRITE32_MEMBER(dec_ioga_device::imsk_w)
{
	COMBINE_DATA(&m_imsk);
}

WRITE_LINE_MEMBER(dec_ioga_device::rtc_irq_w)
{
	if (state == ASSERT_LINE)
	{
		m_intr |= 0x20; // tested by 5000/133 boot ROM circa BFC027C8
	}
}
