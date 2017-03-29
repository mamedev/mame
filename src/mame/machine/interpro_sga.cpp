// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
* An implementation of the SGA device found on Intergraph InterPro family workstations. There is no
* public documentation on this device, so the implementation is being built to follow the logic of the
* system boot ROM and its diagnostic tests.
*
* Please be aware that code in here is not only broken, it's likely wrong in many cases.
*
* TODO
*   - too long to list
*/
#include "emu.h"
#include "interpro_sga.h"

#define VERBOSE 0

DEVICE_ADDRESS_MAP_START(map, 32, interpro_sga_device)
	AM_RANGE(0x00, 0x03) AM_READWRITE(gcs_r, gcs_w)
	AM_RANGE(0x04, 0x07) AM_READWRITE(ipoll_r, ipoll_w)
	AM_RANGE(0x08, 0x0b) AM_READWRITE(imask_r, imask_w)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE(range_base_r, range_base_w)
	AM_RANGE(0x10, 0x13) AM_READWRITE(range_end_r, range_end_w)
	AM_RANGE(0x14, 0x17) AM_READWRITE(cttag_r, cttag_w)
	AM_RANGE(0x18, 0x1b) AM_READWRITE(address_r, address_w)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE(dmacs_r, dmacs_w)
	AM_RANGE(0x20, 0x23) AM_READWRITE(edmacs_r, edmacs_w)

	AM_RANGE(0xa4, 0xa7) AM_READWRITE(dspad1_r, dspad1_w)
	AM_RANGE(0xa8, 0xab) AM_READWRITE(dsoff1_r, dsoff1_w)

	AM_RANGE(0xb4, 0xb7) AM_READWRITE(unknown1_r, unknown1_w)
	AM_RANGE(0xb8, 0xbb) AM_READWRITE(unknown2_r, unknown2_w)
	AM_RANGE(0xbc, 0xbf) AM_READWRITE(ddtc1_r, ddtc1_w)
ADDRESS_MAP_END

const device_type INTERPRO_SGA = device_creator<interpro_sga_device>;

interpro_sga_device::interpro_sga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_SGA, "InterPro SGA", tag, owner, clock, "sga", __FILE__)
{
}

void interpro_sga_device::device_start()
{
}

void interpro_sga_device::device_reset()
{
}

WRITE32_MEMBER(interpro_sga_device::ddtc1_w)
{
	// we assume that when this register is written, we should start a
	// memory to memory dma transfer

	logerror("   gcs = 0x%08x  dmacs = 0x%08x\n", m_gcs, m_dmacs);
	logerror(" ipoll = 0x%08x  imask = 0x%08x\n", m_ipoll, m_imask);
	logerror("dspad1 = 0x%08x dsoff1 = 0x%08x\n", m_dspad1, m_dsoff1);
	logerror("  unk1 = 0x%08x   unk2 = 0x%08x\n", m_unknown1, m_unknown2);
	logerror(" ddtc1 = 0x%08x\n", data);

	m_ddtc1 = data;

	// when complete, we indicate by setting DMAEND(2) - 2 is probably the channel
	// we also turn off the INTBERR and INTMMBE flags
	m_ipoll &= ~(0x20000 | 0x10000);
	m_ipoll |= 0x200;

	// if the address is invalid, fake a bus error
	if (m_dspad1 == 0x40000000 || m_unknown1 == 0x40000000
		|| m_dspad1 == 0x40000200 || m_unknown1 == 0x40000200)
	{
		m_ipoll |= 0x10000;

		// error cycle - bit 0x10 indicates source address error (dspad1)
		// now expecting 0x5463?
#if 0
		if ((m_dspad1 & 0xfffff000) == 0x40000000)
			m_ioga->bus_error(m_dspad1, 0x5433);
		else
			m_ioga->bus_error(m_unknown1, 0x5423);
#endif
		// 0x5423 = BERR|SNAPOK | BG(ICAMMU)? | CT(23)
		// 0x5433 = BERR|SNAPOK | BG(ICAMMU)? | CT(33)
		// 0x5463 = BERR|SNAPOK | BG(ICAMMU)? | TAG(1) | CT(23)
	}
}
