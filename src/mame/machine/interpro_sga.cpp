// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the SGA (SRX Gate Array) device found in Intergraph
 * InterPro family systems. There is no public documentation on this device,
 * so the implementation is being built to follow the logic of the system boot
 * ROM and its diagnostic tests.
 *
 * Please be aware that code in here is not only broken, it's likely wrong in
 * many cases.
 *
 * TODO
 *   - too long to list
 */

#include "emu.h"
#include "interpro_sga.h"

#define VERBOSE 0
#include "logmacro.h"

ADDRESS_MAP_START(interpro_sga_device::map)
	AM_RANGE(0x00, 0x03) AM_READWRITE(gcsr_r, gcsr_w)
	AM_RANGE(0x04, 0x07) AM_READWRITE(ipoll_r, ipoll_w)
	AM_RANGE(0x08, 0x0b) AM_READWRITE(imask_r, imask_w)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE(range_base_r, range_base_w)
	AM_RANGE(0x10, 0x13) AM_READWRITE(range_end_r, range_end_w)
	AM_RANGE(0x14, 0x17) AM_READWRITE(cttag_r, cttag_w)           // aka diag1
	AM_RANGE(0x18, 0x1b) AM_READWRITE(address_r, address_w)       // aka diag0
	AM_RANGE(0x1c, 0x1f) AM_READWRITE(dmacsr_r, dmacsr_w)
	AM_RANGE(0x20, 0x23) AM_READWRITE(edmacsr_r, edmacsr_w)       // esga
	AM_RANGE(0x24, 0x27) AM_READWRITE(reg6_range_r, reg6_range_w) // esga

	AM_RANGE(0x80, 0x83) AM_READWRITE(ddpta0_r, ddpta0_w) // dma 0 device page table address (esga)
	AM_RANGE(0x84, 0x87) AM_READWRITE(ddpad0_r, ddpad0_w) // dma 0 device page address
	AM_RANGE(0x88, 0x8b) AM_READWRITE(ddoff0_r, ddoff0_w) // dma 0 device page offset
	AM_RANGE(0x8c, 0x8f) AM_READWRITE(ddtc0_r, ddtc0_w)   // dma 0 device transfer context

	AM_RANGE(0x90, 0x93) AM_READWRITE(dspta0_r, dspta0_w) // dma 0 SRX page table address (esga)
	AM_RANGE(0x94, 0x97) AM_READWRITE(dspad0_r, dspad0_w) // dma 0 SRX page address
	AM_RANGE(0x98, 0x9b) AM_READWRITE(dsoff0_r, dsoff0_w) // dma 0 SRX page offset
	AM_RANGE(0x9c, 0x9f) AM_READWRITE(dstc0_r, dstc0_w)   // dma 0 SRX transfer context

	AM_RANGE(0xa4, 0xa7) AM_READWRITE(dspad1_r, dspad1_w) // dma 1 source page address
	AM_RANGE(0xa8, 0xab) AM_READWRITE(dsoff1_r, dsoff1_w) // dma 1 source page offset
	AM_RANGE(0xac, 0xaf) AM_READWRITE(dstc1_r, dstc1_w)   // dma 1 source transfer count

	AM_RANGE(0xb4, 0xb7) AM_READWRITE(ddpad1_r, ddpad1_w) // dma 1 destination page address
	AM_RANGE(0xb8, 0xbb) AM_READWRITE(ddoff1_r, ddoff1_w) // dma 1 destination page offset
	AM_RANGE(0xbc, 0xbf) AM_READWRITE(ddtc1_r, ddtc1_w)   // dma 1 destination transfer count

	AM_RANGE(0xc0, 0xc3) AM_READWRITE(ddpta2_r, ddpta2_w) // dma 2 device page table address (esga)
	AM_RANGE(0xc4, 0xc7) AM_READWRITE(ddpad2_r, ddpad2_w) // dma 2 device page address
	AM_RANGE(0xc8, 0xcb) AM_READWRITE(ddoff2_r, ddoff2_w) // dma 2 device page offset
	AM_RANGE(0xcc, 0xcf) AM_READWRITE(ddtc2_r, ddtc2_w)   // dma 2 device transfer context

	AM_RANGE(0xd0, 0xd3) AM_READWRITE(dspta2_r, dspta2_w) // dma 2 SRX page table address (esga)
	AM_RANGE(0xd4, 0xd7) AM_READWRITE(dspad2_r, dspad2_w) // dma 2 SRX page address
	AM_RANGE(0xd8, 0xdb) AM_READWRITE(dsoff2_r, dsoff2_w) // dma 2 SRX page offset
	AM_RANGE(0xdc, 0xdf) AM_READWRITE(dstc2_r, dstc2_w)   // dma 2 SRX transfer context

	AM_RANGE(0xe0, 0xe3) AM_READWRITE(ddrd2_r, ddrd2_w)     // dma 2 device record descriptor (esga)
	AM_RANGE(0xe4, 0xe7) AM_READWRITE(dsrd2_r, dsrd2_w)     // dma 2 SRX record descriptor (esga)
	AM_RANGE(0xe8, 0xeb) AM_READWRITE(dcksum0_r, dcksum0_w) // dma 1 device checksum register 0 (esga)
	AM_RANGE(0xec, 0xef) AM_READWRITE(dcksum1_r, dcksum1_w) // dma 1 device checksum register 1 (esga)
ADDRESS_MAP_END

DEFINE_DEVICE_TYPE(INTERPRO_SGA, interpro_sga_device, "sga", "SRX Gate Array")

interpro_sga_device::interpro_sga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_SGA, tag, owner, clock),
	out_berr_func(*this)
{
}

void interpro_sga_device::device_start()
{
	out_berr_func.resolve();

	save_item(NAME(m_gcsr));
	save_item(NAME(m_ipoll));
	save_item(NAME(m_imask));
	save_item(NAME(m_range_base));
	save_item(NAME(m_range_end));
	save_item(NAME(m_cttag));
	save_item(NAME(m_address));
	save_item(NAME(m_dmacsr));

	save_item(NAME(m_edmacsr));
	save_item(NAME(m_reg6_range));

	save_item(NAME(m_ddpta0));
	save_item(NAME(m_ddpad0));
	save_item(NAME(m_ddoff0));
	save_item(NAME(m_ddtc0));
	save_item(NAME(m_dspta0));
	save_item(NAME(m_dspad0));
	save_item(NAME(m_dsoff0));
	save_item(NAME(m_dstc0));

	save_item(NAME(m_dspad1));
	save_item(NAME(m_dsoff1));
	save_item(NAME(m_dstc1));
	save_item(NAME(m_ddpad1));
	save_item(NAME(m_ddoff1));
	save_item(NAME(m_ddtc1));

	save_item(NAME(m_ddpta2));
	save_item(NAME(m_ddpad2));
	save_item(NAME(m_ddoff2));
	save_item(NAME(m_ddtc2));
	save_item(NAME(m_dspta2));
	save_item(NAME(m_dspad2));
	save_item(NAME(m_dsoff2));
	save_item(NAME(m_dstc2));

	save_item(NAME(m_ddrd2));
	save_item(NAME(m_dsrd2));
	save_item(NAME(m_dcksum0));
	save_item(NAME(m_dcksum1));
}

void interpro_sga_device::device_reset()
{
}

WRITE32_MEMBER(interpro_sga_device::ddtc1_w)
{
	m_ddtc1 = data;

	// assume that when this register is written, we should start a
	// memory to memory dma transfer

	LOG("  gcsr = 0x%08x dmacsr = 0x%08x\n", m_gcsr, m_dmacsr);
	LOG(" ipoll = 0x%08x  imask = 0x%08x\n", m_ipoll, m_imask);
	LOG("dspad1 = 0x%08x dsoff1 = 0x%08x dstc1 = 0x%08x\n", m_dspad1, m_dsoff1, m_dstc1);
	LOG("ddpad1 = 0x%08x ddoff1 = 0x%08x ddtc1 = 0x%08x\n", m_ddpad1, m_ddoff1, m_ddtc1);

	// when complete, we indicate by setting DMAEND(2) - 2 is probably the channel
	// we also turn off the INTBERR and INTMMBE flags
	m_ipoll &= ~(IPOLL_INTBERR | IPOLL_INTMMBE);
	m_ipoll |= 0x200;

#if 0
	// if the address is invalid, fake a bus error
	if ((m_dspad1 & 0xfffff000) == 0x40000000 || (m_ddpad1 & 0xfffff) == 0x40000000)
	{
		m_ipoll |= IPOLL_INTBERR;

		// error cycle - bit 0x10 indicates source address error (dspad1)
		// now expecting 0x5463?
		if ((m_dspad1 & 0xfffff000) == 0x40000000)
			out_berr_func(space, 0x5433, m_dspad1); // BINFO_SNAPOK | BINFO_BERR | BINFO_BG_ICAMMU | 0x30 | CT(3)
		else
			out_berr_func(space, 0x5423, m_ddpad1); // BINFO_SNAPOK | BINFO_BERR | BINFO_BG_ICAMMU | 0x20 | CT(3)

		// 0x5423 = BERR|SNAPOK | BG(ICAMMU)? | CT(23)
		// 0x5433 = BERR|SNAPOK | BG(ICAMMU)? | CT(33)
		// 0x5463 = BERR|SNAPOK | BG(ICAMMU)? | TAG(40=1) | CT(23)
	}
#endif
}
