// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

    SH7014 Interrupt Controller

***************************************************************************/

#include "emu.h"
#include "sh7014_intc.h"

// #define VERBOSE (LOG_GENERAL)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(SH7014_INTC, sh7014_intc_device, "sh7014intc", "SH7014 Interrupt Controller")


sh7014_intc_device::sh7014_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_INTC, tag, owner, clock)
	, m_set_irq_cb(*this)
{
}

void sh7014_intc_device::device_start()
{
	m_set_irq_cb.resolve();

	save_item(NAME(m_ipra));
	save_item(NAME(m_iprb));
	save_item(NAME(m_iprc));
	save_item(NAME(m_iprd));
	save_item(NAME(m_ipre));
	save_item(NAME(m_iprf));
	save_item(NAME(m_iprg));
	save_item(NAME(m_iprh));
	save_item(NAME(m_icr));
	save_item(NAME(m_isr));
	save_item(NAME(m_irq_type));
	save_item(NAME(m_nmi_input));
	save_item(NAME(m_pending_irqs));
	save_item(NAME(m_irq_levels));
}

void sh7014_intc_device::device_reset()
{
	m_ipra = 0;
	m_iprb = 0;
	m_iprc = 0;
	m_iprd = 0;
	m_ipre = 0;
	m_iprf = 0;
	m_iprg = 0;
	m_iprh = 0;
	m_icr = m_isr = 0;
	m_nmi_input = false;

	std::fill(std::begin(m_irq_type), std::end(m_irq_type), IRQ_LEVEL);
	std::fill(std::begin(m_irq_levels), std::end(m_irq_levels), 0);
	std::fill(std::begin(m_pending_irqs), std::end(m_pending_irqs), 0);

	m_irq_levels[INT_VECTOR_NMI] = 16; // fixed value
}

void sh7014_intc_device::map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(sh7014_intc_device::ipra_r), FUNC(sh7014_intc_device::ipra_w));
	map(0x02, 0x03).rw(FUNC(sh7014_intc_device::iprb_r), FUNC(sh7014_intc_device::iprb_w));
	map(0x04, 0x05).rw(FUNC(sh7014_intc_device::iprc_r), FUNC(sh7014_intc_device::iprc_w));
	map(0x06, 0x07).rw(FUNC(sh7014_intc_device::iprd_r), FUNC(sh7014_intc_device::iprd_w));
	map(0x08, 0x09).rw(FUNC(sh7014_intc_device::ipre_r), FUNC(sh7014_intc_device::ipre_w));
	map(0x0a, 0x0b).rw(FUNC(sh7014_intc_device::iprf_r), FUNC(sh7014_intc_device::iprf_w));
	map(0x0c, 0x0d).rw(FUNC(sh7014_intc_device::iprg_r), FUNC(sh7014_intc_device::iprg_w));
	map(0x0e, 0x0f).rw(FUNC(sh7014_intc_device::iprh_r), FUNC(sh7014_intc_device::iprh_w));
	map(0x10, 0x11).rw(FUNC(sh7014_intc_device::icr_r), FUNC(sh7014_intc_device::icr_w));
	map(0x12, 0x13).rw(FUNC(sh7014_intc_device::isr_r), FUNC(sh7014_intc_device::isr_w));
}

///

uint16_t sh7014_intc_device::ipra_r()
{
	return m_ipra;
}

void sh7014_intc_device::ipra_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ipra);

	m_irq_levels[INT_VECTOR_IRQ0] = BIT(m_ipra, 12, 4);
	m_irq_levels[INT_VECTOR_IRQ1] = BIT(m_ipra, 8, 4);
	m_irq_levels[INT_VECTOR_IRQ2] = BIT(m_ipra, 4, 4);
	m_irq_levels[INT_VECTOR_IRQ3] = BIT(m_ipra, 0, 4);

	update_irq_state();
}

uint16_t sh7014_intc_device::iprb_r()
{
	return m_iprb & 0xff00;
}

void sh7014_intc_device::iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprb);

	m_irq_levels[INT_VECTOR_IRQ6] = BIT(m_iprb, 4, 4);
	m_irq_levels[INT_VECTOR_IRQ7] = BIT(m_iprb, 0, 4);

	update_irq_state();
}

uint16_t sh7014_intc_device::iprc_r()
{
	return m_iprc;
}

void sh7014_intc_device::iprc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprc);

	m_irq_levels[INT_VECTOR_DMA_CH0] = BIT(m_iprc, 12, 4);
	m_irq_levels[INT_VECTOR_DMA_CH1] = BIT(m_iprc, 8, 4);

	update_irq_state();
}

uint16_t sh7014_intc_device::iprd_r()
{
	return m_iprd;
}

void sh7014_intc_device::iprd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprd);

	m_irq_levels[INT_VECTOR_MTU_TGI0A] = m_irq_levels[INT_VECTOR_MTU_TGI0B] = m_irq_levels[INT_VECTOR_MTU_TGI0C] = m_irq_levels[INT_VECTOR_MTU_TGI0D] = BIT(m_iprd, 12, 4);
	m_irq_levels[INT_VECTOR_MTU_TGI0V] = BIT(m_iprd, 8, 4);
	m_irq_levels[INT_VECTOR_MTU_TGI1A] = m_irq_levels[INT_VECTOR_MTU_TGI1B] = BIT(m_iprd, 4, 4);
	m_irq_levels[INT_VECTOR_MTU_TGI1V] = m_irq_levels[INT_VECTOR_MTU_TGI1U] = BIT(m_iprd, 0, 4);

	update_irq_state();
}

uint16_t sh7014_intc_device::ipre_r()
{
	return m_ipre & 0xff00;
}

void sh7014_intc_device::ipre_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ipre);

	m_irq_levels[INT_VECTOR_MTU_TGI2A] = m_irq_levels[INT_VECTOR_MTU_TGI2B] = BIT(m_ipre, 12, 4);
	m_irq_levels[INT_VECTOR_MTU_TGI2V] = m_irq_levels[INT_VECTOR_MTU_TGI2U] = BIT(m_ipre, 8, 4);

	update_irq_state();
}

uint16_t sh7014_intc_device::iprf_r()
{
	return m_iprf & 0xff;
}

void sh7014_intc_device::iprf_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprf);

	m_irq_levels[INT_VECTOR_SCI_ERI0] = m_irq_levels[INT_VECTOR_SCI_RXI0] = m_irq_levels[INT_VECTOR_SCI_TXI0] = m_irq_levels[INT_VECTOR_SCI_TEI0] = BIT(m_iprf, 4, 4);
	m_irq_levels[INT_VECTOR_SCI_ERI1] = m_irq_levels[INT_VECTOR_SCI_RXI1] = m_irq_levels[INT_VECTOR_SCI_TXI1] = m_irq_levels[INT_VECTOR_SCI_TEI1] = BIT(m_iprf, 0, 4);

	update_irq_state();
}

uint16_t sh7014_intc_device::iprg_r()
{
	return m_iprg & 0xf0ff;
}

void sh7014_intc_device::iprg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprg);

	m_irq_levels[INT_VECTOR_AD] = BIT(m_iprg, 12, 4);
	m_irq_levels[INT_VECTOR_CMT_CH0] = BIT(m_iprg, 4, 4);
	m_irq_levels[INT_VECTOR_CMT_CH1] = BIT(m_iprg, 0, 4);

	update_irq_state();
}

uint16_t sh7014_intc_device::iprh_r()
{
	return m_iprh & 0xf000;
}

void sh7014_intc_device::iprh_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprh);

	m_irq_levels[INT_VECTOR_WDT] = m_irq_levels[INT_VECTOR_BSC] = BIT(m_iprh, 12, 4);

	update_irq_state();
}

uint16_t sh7014_intc_device::icr_r()
{
	uint16_t r = m_icr & 0x01f3;

	if (m_nmi_input == ((m_icr & ICR_NMIE) ? ASSERT_LINE : CLEAR_LINE))
		r |= ICR_NMIL;

	return r;
}

void sh7014_intc_device::icr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_icr);

	for (int i = 0; i < 8; i++)
		m_irq_type[i] = BIT(m_icr, 7 - i);
}

uint16_t sh7014_intc_device::isr_r()
{
	return m_isr & 0xf3;
}

void sh7014_intc_device::isr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// ICR will be set to 1 for edge detection, and it's only possible to
	// clear flags in ISR for IRQs set to edge detection
	auto old = m_isr;

	COMBINE_DATA(&m_isr);

	m_isr = (old & ~m_icr) | (old & m_isr & m_icr);

	update_irq_state();
}

void sh7014_intc_device::set_interrupt(int vector, int state)
{
	if (vector != -1) {
		if (state == ASSERT_LINE)
			m_pending_irqs[vector >> 5] |= 1 << (vector & 31);
		else if (state == CLEAR_LINE)
			m_pending_irqs[vector >> 5] &= ~(1 << (vector & 31));

		if (vector == INT_VECTOR_NMI) {
			m_nmi_input = state == ASSERT_LINE;
		} else if ((vector >= INT_VECTOR_IRQ0 && vector <= INT_VECTOR_IRQ3) || vector == INT_VECTOR_IRQ6 || vector == INT_VECTOR_IRQ7) {
			const int irq = vector - INT_VECTOR_IRQ0;

			if (state == ASSERT_LINE) {
				if (m_irq_type[irq] == IRQ_LEVEL)
					m_isr |= 1 << (7 - irq);
			} else if (state == CLEAR_LINE) {
				if (m_irq_type[irq] == IRQ_LEVEL)
					m_isr &= ~(1 << (7 - irq));
			}
		}
	}

	update_irq_state();
}

void sh7014_intc_device::update_irq_state()
{
	int cur_vector = 0;
	int cur_level = -1;

	// isr has the bits in reverse order
	m_pending_irqs[INT_VECTOR_IRQ0 >> 5] &= ~0xff;
	m_pending_irqs[INT_VECTOR_IRQ0 >> 5] |= BIT(m_isr, 7)
		| (BIT(m_isr, 6) << 1)
		| (BIT(m_isr, 5) << 2)
		| (BIT(m_isr, 4) << 3)
		| (BIT(m_isr, 1) << 6)
		| (BIT(m_isr, 0) << 7);

	for (int i = 0; i < MAX_VECTORS / 32; i++) {
		if (!m_pending_irqs[i])
			continue;

		for (int j = 0; j < 32; j++) {
			if (BIT(m_pending_irqs[i], j)) {
				const int vector = i * 32 + j;
				const int level = m_irq_levels[vector];

				if (level > cur_level) {
					cur_vector = vector;
					cur_level = level;
				}
			}
		}
	}

	if (cur_vector == INT_VECTOR_NMI)
		m_set_irq_cb(INPUT_LINE_NMI, cur_level, false);
	else if ((cur_vector >= INT_VECTOR_IRQ0 && cur_vector <= INT_VECTOR_IRQ3) || cur_vector == INT_VECTOR_IRQ6 || cur_vector == INT_VECTOR_IRQ7)
		m_set_irq_cb(cur_vector - INT_VECTOR_IRQ0, cur_level, false);
	else if (cur_vector > INT_VECTOR_IRQ7)
		m_set_irq_cb(cur_vector, cur_level, true);
}

void sh7014_intc_device::set_input(int inputnum, int state)
{
	if (inputnum == INPUT_LINE_NMI)
		set_interrupt(INT_VECTOR_NMI, state);
	else if ((inputnum >= INPUT_LINE_IRQ0 && inputnum <= INPUT_LINE_IRQ3) || inputnum == INPUT_LINE_IRQ6 || inputnum == INPUT_LINE_IRQ7)
		set_interrupt(INT_VECTOR_IRQ0 + (inputnum - INPUT_LINE_IRQ0), state);
}
