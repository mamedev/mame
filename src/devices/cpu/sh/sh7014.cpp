// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH-2 SH7014

***************************************************************************/

#include "emu.h"

#include "sh7014.h"


DEFINE_DEVICE_TYPE(SH7014, sh7014_device,  "sh7014",  "Hitachi SH-2 (SH7014)")

sh7014_device::sh7014_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sh2_device(mconfig, SH7014, tag, owner, clock, CPU_TYPE_SH2, address_map_constructor(FUNC(sh7014_device::sh7014_map), this), 32, 0xffffffff)
	, m_sci(*this, "sci%u", 0u)
	, m_bsc(*this, "bsc")
	, m_dmac(*this, "dmac")
	, m_intc(*this, "intc")
	, m_mtu(*this, "mtu")
	, m_port(*this, "io_port")
	, m_sci_tx_cb(*this)
{
}

void sh7014_device::device_start()
{
	sh2_device::device_start();

	save_item(NAME(m_ccr));
}

void sh7014_device::device_reset()
{
	sh2_device::device_reset();

	// CAC
	m_ccr = 0;
}

void sh7014_device::device_add_mconfig(machine_config &config)
{
	SH7014_SCI(config, m_sci[0], DERIVED_CLOCK(1, 1), m_intc,
		0, // id
		sh7014_intc_device::INT_VECTOR_SCI_ERI0,
		sh7014_intc_device::INT_VECTOR_SCI_RXI0,
		sh7014_intc_device::INT_VECTOR_SCI_TXI0,
		sh7014_intc_device::INT_VECTOR_SCI_TEI0
	);

	SH7014_SCI(config, m_sci[1], DERIVED_CLOCK(1, 1), m_intc,
		1, // id
		sh7014_intc_device::INT_VECTOR_SCI_ERI1,
		sh7014_intc_device::INT_VECTOR_SCI_RXI1,
		sh7014_intc_device::INT_VECTOR_SCI_TXI1,
		sh7014_intc_device::INT_VECTOR_SCI_TEI1
	);

	SH7014_BSC(config, m_bsc);

	SH7014_DMAC(config, m_dmac, DERIVED_CLOCK(1, 1), *this, m_intc);
	m_dmac->set_notify_dma_source_callback(FUNC(sh7014_device::notify_dma_source));

	SH7014_INTC(config, m_intc);
	m_intc->set_irq_callback(FUNC(sh7014_device::set_irq));

	SH7014_MTU(config, m_mtu, DERIVED_CLOCK(1, 1), m_intc);

	SH7014_PORT(config, m_port);
}

void sh7014_device::sh7014_map(address_map &map)
{
	// SCI - Serial Communication Interface
	map(0xffff81a0, 0xffff81af).m(m_sci[0], FUNC(sh7014_sci_device::map));
	map(0xffff81b0, 0xffff81bf).m(m_sci[1], FUNC(sh7014_sci_device::map));

	// MTU - Multifunction Timer Pulse Unit
	map(0xffff8240, 0xffff82af).m(m_mtu, FUNC(sh7014_mtu_device::map));

	// INTC - Interrupt Controller
	map(0xffff8348, 0xffff835b).m(m_intc, FUNC(sh7014_intc_device::map));

	// I/O - I/O Ports (DR registers)
	// PFC - Pin Function Controller (IOR, CR registers)
	// TODO: SH7016/SH7017 support additionally C and D ports in addition to the A, B, E, F that the SH7014 supports
	map(0xffff8382, 0xffff8383).rw(m_port, FUNC(sh7014_port_device::padrl_r), FUNC(sh7014_port_device::padrl_w));
	map(0xffff8386, 0xffff8387).rw(m_port, FUNC(sh7014_port_device::paiorl_r), FUNC(sh7014_port_device::paiorl_w));
	map(0xffff838c, 0xffff838d).rw(m_port, FUNC(sh7014_port_device::pacrl1_r), FUNC(sh7014_port_device::pacrl1_w));
	map(0xffff838e, 0xffff838f).rw(m_port, FUNC(sh7014_port_device::pacrl2_r), FUNC(sh7014_port_device::pacrl2_w));

	map(0xffff8390, 0xffff8391).rw(m_port, FUNC(sh7014_port_device::pbdr_r), FUNC(sh7014_port_device::pbdr_w));
	map(0xffff8394, 0xffff8395).rw(m_port, FUNC(sh7014_port_device::pbior_r), FUNC(sh7014_port_device::pbior_w));
	map(0xffff8398, 0xffff8399).rw(m_port, FUNC(sh7014_port_device::pbcr1_r), FUNC(sh7014_port_device::pbcr1_w));
	map(0xffff839a, 0xffff839b).rw(m_port, FUNC(sh7014_port_device::pbcr2_r), FUNC(sh7014_port_device::pbcr2_w));

	map(0xffff83b0, 0xffff83b1).rw(m_port, FUNC(sh7014_port_device::pedr_r), FUNC(sh7014_port_device::pedr_w));
	map(0xffff83b4, 0xffff83b5).rw(m_port, FUNC(sh7014_port_device::peior_r), FUNC(sh7014_port_device::peior_w));
	map(0xffff83b8, 0xffff83b9).rw(m_port, FUNC(sh7014_port_device::pecr1_r), FUNC(sh7014_port_device::pecr1_w));
	map(0xffff83ba, 0xffff83bb).rw(m_port, FUNC(sh7014_port_device::pecr2_r), FUNC(sh7014_port_device::pecr2_w));

	map(0xffff83b3, 0xffff83b3).r(m_port, FUNC(sh7014_port_device::pfdr_r));

	// TODO: CMT - Compare Match Timer
	// 0xffff83d0 - 0xffff83df

	// TODO: A/D - A/D Converter (High Speed, for SH7014)
	// 0xffff83e0 - 0xffff83ff

	// TODO: A/D - A/D Converter (Mid Speed, for SH7016/SH7017)
	// 0xffff8420 - 0xffff8429

	// TODO: WDT - Watchdog Timer
	// 0xffff8610 - 0xffff8613

	// TODO: Power-down state
	// 0xffff8614

	// BSC - Bus State Controller
	map(0xffff8620, 0xffff8633).m(m_bsc, FUNC(sh7014_bsc_device::map));

	// DMAC - Direct Memory Access Controller
	map(0xffff86b0, 0xffff86df).m(m_dmac, FUNC(sh7014_dmac_device::map));

	// CAC - Cache Memory
	map(0xffff8740, 0xffff8741).rw(FUNC(sh7014_device::ccr_r), FUNC(sh7014_device::ccr_w));

	// Cache space
	map(0xfffff000, 0xffffffff).ram();
}

void sh7014_device::sh2_exception_internal(const char *message, int irqline, int vector)
{
	// IRQ was taken so clear it in the interrupt controller and pass it down
	m_intc->set_interrupt(vector, CLEAR_LINE);
	sh2_device::sh2_exception_internal(message, irqline, vector);
}

void sh7014_device::execute_set_input(int irqline, int state)
{
	/*
	Flow for SH7014 IRQs:
	sh7014_device::execute_set_input (for externally triggered IRQs)
	-> sh7014_intc_device::set_input
	-> sh7014_device::set_irq
	-> sh2_device::execute_set_input (if not internal peripheral IRQ) OR DMA interception OR set sh2_device's internal IRQ flags
	*/
	m_intc->set_input(irqline, state);
}

void sh7014_device::set_irq(int vector, int level, bool is_internal)
{
	if (!is_internal) {
		sh2_device::execute_set_input(vector, ASSERT_LINE);
		return;
	}

	// SH7014's DMA controller can be configured to trigger based on various
	// on-board peripheral IRQs, so on-board peripheral IRQs must go through here
	if (m_dmac->is_dma_activated(vector)) {
		m_intc->set_interrupt(vector, CLEAR_LINE);
		return;
	}

	m_sh2_state->internal_irq_level = level;
	m_internal_irq_vector = vector;
	m_test_irq = 1;
}

void sh7014_device::notify_dma_source(uint32_t source)
{
	if (source == sh7014_dmac_channel_device::RS_SCI_TXI0)
		m_sci[0]->set_dma_source_tx(true);
	else if (source == sh7014_dmac_channel_device::RS_SCI_TXI1)
		m_sci[1]->set_dma_source_tx(true);
	else if (source == sh7014_dmac_channel_device::RS_SCI_RXI0)
		m_sci[0]->set_dma_source_rx(true);
	else if (source == sh7014_dmac_channel_device::RS_SCI_RXI1)
		m_sci[1]->set_dma_source_rx(true);
}

///////
// CAC

uint16_t sh7014_device::ccr_r()
{
	// bits 15-5 are undefined
	return m_ccr & 0x1f;
}

void sh7014_device::ccr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// bit 0 - CECS0 CS0 Space Cache Enable
	// bit 1 - CECS1 CS1 Space Cache Enable
	// bit 2 - CECS2 CS2 Space Cache Enable
	// bit 3 - CECS3 CS3 Space Cache Enable
	// bit 4 - CEDRAM DRAM Space Cache Enable
	COMBINE_DATA(&m_ccr);
}
