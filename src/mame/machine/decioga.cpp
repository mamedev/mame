// license:BSD-3-Clause
// copyright-holders:R. Belmont
/******************************************************************************
*
*   MIPS DECstation I/O Gate Array emulation
*
*
*/

#include "decioga.h"

DEFINE_DEVICE_TYPE(DECSTATION_IOGA, dec_ioga_device, "decioga", "DECstation I/O Gate Array")

void dec_ioga_device::map(address_map &map)
{
	map(0x040100, 0x040103).rw(FUNC(dec_ioga_device::csr_r), FUNC(dec_ioga_device::csr_w)); 
}

dec_ioga_device::dec_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECSTATION_IOGA, tag, owner, clock)
{
}

void dec_ioga_device::device_start()
{
	save_item(NAME(m_csr));
}

void dec_ioga_device::device_reset()
{
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