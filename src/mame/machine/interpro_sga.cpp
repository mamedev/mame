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

void interpro_sga_device::map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(interpro_sga_device::gcsr_r), FUNC(interpro_sga_device::gcsr_w));
	map(0x04, 0x07).rw(FUNC(interpro_sga_device::ipoll_r), FUNC(interpro_sga_device::ipoll_w));
	map(0x08, 0x0b).rw(FUNC(interpro_sga_device::imask_r), FUNC(interpro_sga_device::imask_w));
	map(0x0c, 0x0f).rw(FUNC(interpro_sga_device::range_base_r), FUNC(interpro_sga_device::range_base_w));
	map(0x10, 0x13).rw(FUNC(interpro_sga_device::range_end_r), FUNC(interpro_sga_device::range_end_w));
	map(0x14, 0x17).rw(FUNC(interpro_sga_device::cttag_r), FUNC(interpro_sga_device::cttag_w));           // aka diag1
	map(0x18, 0x1b).rw(FUNC(interpro_sga_device::address_r), FUNC(interpro_sga_device::address_w));       // aka diag0
	map(0x1c, 0x1f).rw(FUNC(interpro_sga_device::dmacsr_r), FUNC(interpro_sga_device::dmacsr_w));
	map(0x20, 0x23).rw(FUNC(interpro_sga_device::edmacsr_r), FUNC(interpro_sga_device::edmacsr_w));       // esga
	map(0x24, 0x27).rw(FUNC(interpro_sga_device::reg6_range_r), FUNC(interpro_sga_device::reg6_range_w)); // esga

	map(0x80, 0x83).rw(FUNC(interpro_sga_device::ddpta0_r), FUNC(interpro_sga_device::ddpta0_w)); // dma 0 device page table address (esga)
	map(0x84, 0x87).rw(FUNC(interpro_sga_device::ddpad0_r), FUNC(interpro_sga_device::ddpad0_w)); // dma 0 device page address
	map(0x88, 0x8b).rw(FUNC(interpro_sga_device::ddoff0_r), FUNC(interpro_sga_device::ddoff0_w)); // dma 0 device page offset
	map(0x8c, 0x8f).rw(FUNC(interpro_sga_device::ddtc0_r), FUNC(interpro_sga_device::ddtc0_w));   // dma 0 device transfer context

	map(0x90, 0x93).rw(FUNC(interpro_sga_device::dspta0_r), FUNC(interpro_sga_device::dspta0_w)); // dma 0 SRX page table address (esga)
	map(0x94, 0x97).rw(FUNC(interpro_sga_device::dspad0_r), FUNC(interpro_sga_device::dspad0_w)); // dma 0 SRX page address
	map(0x98, 0x9b).rw(FUNC(interpro_sga_device::dsoff0_r), FUNC(interpro_sga_device::dsoff0_w)); // dma 0 SRX page offset
	map(0x9c, 0x9f).rw(FUNC(interpro_sga_device::dstc0_r), FUNC(interpro_sga_device::dstc0_w));   // dma 0 SRX transfer context

	map(0xa4, 0xa7).rw(FUNC(interpro_sga_device::dspad1_r), FUNC(interpro_sga_device::dspad1_w)); // dma 1 source page address
	map(0xa8, 0xab).rw(FUNC(interpro_sga_device::dsoff1_r), FUNC(interpro_sga_device::dsoff1_w)); // dma 1 source page offset
	map(0xac, 0xaf).rw(FUNC(interpro_sga_device::dstc1_r), FUNC(interpro_sga_device::dstc1_w));   // dma 1 source transfer count

	map(0xb4, 0xb7).rw(FUNC(interpro_sga_device::ddpad1_r), FUNC(interpro_sga_device::ddpad1_w)); // dma 1 destination page address
	map(0xb8, 0xbb).rw(FUNC(interpro_sga_device::ddoff1_r), FUNC(interpro_sga_device::ddoff1_w)); // dma 1 destination page offset
	map(0xbc, 0xbf).rw(FUNC(interpro_sga_device::ddtc1_r), FUNC(interpro_sga_device::ddtc1_w));   // dma 1 destination transfer count

	map(0xc0, 0xc3).rw(FUNC(interpro_sga_device::ddpta2_r), FUNC(interpro_sga_device::ddpta2_w)); // dma 2 device page table address (esga)
	map(0xc4, 0xc7).rw(FUNC(interpro_sga_device::ddpad2_r), FUNC(interpro_sga_device::ddpad2_w)); // dma 2 device page address
	map(0xc8, 0xcb).rw(FUNC(interpro_sga_device::ddoff2_r), FUNC(interpro_sga_device::ddoff2_w)); // dma 2 device page offset
	map(0xcc, 0xcf).rw(FUNC(interpro_sga_device::ddtc2_r), FUNC(interpro_sga_device::ddtc2_w));   // dma 2 device transfer context

	map(0xd0, 0xd3).rw(FUNC(interpro_sga_device::dspta2_r), FUNC(interpro_sga_device::dspta2_w)); // dma 2 SRX page table address (esga)
	map(0xd4, 0xd7).rw(FUNC(interpro_sga_device::dspad2_r), FUNC(interpro_sga_device::dspad2_w)); // dma 2 SRX page address
	map(0xd8, 0xdb).rw(FUNC(interpro_sga_device::dsoff2_r), FUNC(interpro_sga_device::dsoff2_w)); // dma 2 SRX page offset
	map(0xdc, 0xdf).rw(FUNC(interpro_sga_device::dstc2_r), FUNC(interpro_sga_device::dstc2_w));   // dma 2 SRX transfer context

	map(0xe0, 0xe3).rw(FUNC(interpro_sga_device::ddrd2_r), FUNC(interpro_sga_device::ddrd2_w));     // dma 2 device record descriptor (esga)
	map(0xe4, 0xe7).rw(FUNC(interpro_sga_device::dsrd2_r), FUNC(interpro_sga_device::dsrd2_w));     // dma 2 SRX record descriptor (esga)
	map(0xe8, 0xeb).rw(FUNC(interpro_sga_device::dcksum0_r), FUNC(interpro_sga_device::dcksum0_w)); // dma 1 device checksum register 0 (esga)
	map(0xec, 0xef).rw(FUNC(interpro_sga_device::dcksum1_r), FUNC(interpro_sga_device::dcksum1_w)); // dma 1 device checksum register 1 (esga)
}

DEFINE_DEVICE_TYPE(INTERPRO_SGA, interpro_sga_device, "sga", "SRX Gate Array")

interpro_sga_device::interpro_sga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_SGA, tag, owner, clock)
	, m_berr_func(*this)
{
}

void interpro_sga_device::device_start()
{
	m_berr_func.resolve();

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

void interpro_sga_device::ddtc1_w(u32 data)
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
			m_berr_func(space, 0x5433, m_dspad1); // BINFO_SNAPOK | BINFO_BERR | BINFO_BG_ICAMMU | 0x30 | CT(3)
		else
			m_berr_func(space, 0x5423, m_ddpad1); // BINFO_SNAPOK | BINFO_BERR | BINFO_BG_ICAMMU | 0x20 | CT(3)

		// 0x5423 = BERR|SNAPOK | BG(ICAMMU)? | CT(23)
		// 0x5433 = BERR|SNAPOK | BG(ICAMMU)? | CT(33)
		// 0x5463 = BERR|SNAPOK | BG(ICAMMU)? | TAG(40=1) | CT(23)
	}
#endif
}
