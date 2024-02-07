// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_intc.cpp

    H8 interrupt controllers family

    TODO:
    - H8/325 ICSR has bits for inverting the active state (low/high, rise/fall)

***************************************************************************/

#include "emu.h"
#include "h8_intc.h"

#include "h8.h"


DEFINE_DEVICE_TYPE(H8_INTC,    h8_intc_device,    "h8_intc",    "H8 interrupt controller")
DEFINE_DEVICE_TYPE(H8325_INTC, h8325_intc_device, "h8325_intc", "H8/325 interrupt controller")
DEFINE_DEVICE_TYPE(H8H_INTC,   h8h_intc_device,   "h8h_intc",   "H8H interrupt controller")
DEFINE_DEVICE_TYPE(H8S_INTC,   h8s_intc_device,   "h8s_intc",   "H8S interrupt controller")
DEFINE_DEVICE_TYPE(GT913_INTC, gt913_intc_device, "gt913_intc", "Casio GT913F interrupt controller")

h8_intc_device::h8_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8_intc_device(mconfig, H8_INTC, tag, owner, clock)
{
	m_irq_vector_base = 4;
	m_irq_vector_count = 8;
	m_irq_vector_nmi = 3;
}

h8_intc_device::h8_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock), m_irq_vector_base(0), m_irq_vector_count(0), m_irq_vector_nmi(0),
	m_cpu(*this, finder_base::DUMMY_TAG), m_nmi_input(false), m_irq_input(0), m_ier(0), m_isr(0), m_iscr(0), m_icr_filter(0), m_ipr_filter(0)
{
}

void h8_intc_device::device_start()
{
	memset(m_pending_irqs, 0, sizeof(m_pending_irqs));
	save_item(NAME(m_pending_irqs));
	save_item(NAME(m_irq_type));
	save_item(NAME(m_nmi_input));
	save_item(NAME(m_irq_input));
	save_item(NAME(m_ier));
	save_item(NAME(m_isr));
	save_item(NAME(m_iscr));
	save_item(NAME(m_icr_filter));
	save_item(NAME(m_ipr_filter));
}

void h8_intc_device::device_reset()
{
	memset(m_irq_type, 0, sizeof(m_irq_type));
	memset(m_pending_irqs, 0, sizeof(m_pending_irqs));
	m_ier = m_isr = m_irq_input = 0x00;
	m_iscr = 0x0000;
}

int h8_intc_device::interrupt_taken(int vector)
{
	if(0)
		logerror("taking internal interrupt %d\n", vector);
	m_pending_irqs[vector >> 5] &= ~(1 << (vector & 31));
	if(vector >= m_irq_vector_base && vector < m_irq_vector_base + m_irq_vector_count) {
		int irq = vector - m_irq_vector_base;
		if(m_irq_type[irq] != IRQ_LEVEL || !(m_irq_input & (1 << irq)))
			m_isr &= ~(1 << irq);
		update_irq_state();
		return irq;
	}
	update_irq_state();
	if(vector == m_irq_vector_nmi)
		return INPUT_LINE_NMI;
	return 8;
}

void h8_intc_device::internal_interrupt(int vector)
{
	if(0)
		logerror("internal interrupt %d\n", vector);
	if(!m_cpu->trigger_dma(vector)) {
		m_pending_irqs[vector >> 5] |= 1 << (vector & 31);
		update_irq_state();
	}
}

void h8_intc_device::set_input(int inputnum, int state)
{
	if(inputnum == INPUT_LINE_NMI) {
		if(state == ASSERT_LINE && !m_nmi_input)
			m_pending_irqs[0] |= 1 << m_irq_vector_nmi;
		m_nmi_input = state == ASSERT_LINE;
		update_irq_state();
	} else {
		bool set = false;
		bool cur = m_irq_input & (1 << inputnum);
		switch(m_irq_type[inputnum]) {
		case IRQ_LEVEL: set = state == ASSERT_LINE; break;
		case IRQ_EDGE: set = state == ASSERT_LINE && !cur; break;
		case IRQ_DUAL_EDGE: set = (state == ASSERT_LINE && !cur) || (state == CLEAR_LINE && cur); break;
		}
		if(state == ASSERT_LINE)
			m_irq_input |= 1 << inputnum;
		else
			m_irq_input &= ~(1 << inputnum);
		if(set) {
			m_isr |= 1 << inputnum;
			update_irq_state();
		}
	}
}

void h8_intc_device::set_filter(int icr_filter, int ipr_filter)
{
	m_icr_filter = icr_filter;
	m_ipr_filter = ipr_filter;
	update_irq_state();
}

uint8_t h8_intc_device::ier_r()
{
	return m_ier;
}

void h8_intc_device::ier_w(uint8_t data)
{
	m_ier = data;
	//  logerror("ier = %02x\n", data);
	update_irq_state();
}

void h8_intc_device::check_level_irqs(bool force_update)
{
	logerror("irq_input=%02x\n", m_irq_input);
	bool update = force_update;
	for(int i=0; i<8; i++) {
		unsigned char mask = 1 << i;
		if(m_irq_type[i] == IRQ_LEVEL && (m_irq_input & mask) && !(m_isr & mask)) {
			m_isr |= mask;
			update = true;
		}
	}
	if(update)
		update_irq_state();
}


uint8_t h8_intc_device::iscr_r()
{
	return m_iscr;
}

void h8_intc_device::iscr_w(uint8_t data)
{
	m_iscr = data;
	logerror("iscr = %02x\n", m_iscr);
	update_irq_types();
}

void h8_intc_device::update_irq_types()
{
	for(int i=0; i<8; i++)
		switch((m_iscr >> (i)) & 1) {
		case 0:
			m_irq_type[i] = IRQ_LEVEL;
			break;
		case 1:
			m_irq_type[i] = IRQ_EDGE;
			break;
		}
	check_level_irqs();
}

void h8_intc_device::update_irq_state()
{
	if(m_irq_vector_count > 0) {
		const unsigned mask = (1 << m_irq_vector_count) - 1;

		m_pending_irqs[0] &= ~(mask << m_irq_vector_base);
		m_pending_irqs[0] |= (m_isr & m_ier & mask) << m_irq_vector_base;
	}

	int cur_vector = 0;
	int cur_level = -1;

	for(int i=0; i<MAX_VECTORS/32; i++) {
		unsigned int pending = m_pending_irqs[i];
		if(pending)
			for(int j=0; j<32; j++)
				if(pending & (1 << j)) {
					int vect = i*32+j;
					int icr_pri, ipr_pri;
					get_priority(vect, icr_pri, ipr_pri);
					if(icr_pri >= m_icr_filter && ipr_pri > m_ipr_filter) {
						int level = m_ipr_filter == -1 ? icr_pri : ipr_pri;
						if(level > cur_level) {
							cur_vector = vect;
							cur_level = level;
						}
					}
				}
	}
	m_cpu->set_irq(cur_vector, cur_level, cur_vector == m_irq_vector_nmi);
}

void h8_intc_device::get_priority(int vect, int &icr_pri, int &ipr_pri) const
{
	icr_pri = vect == 3 ? 2 : 0; // NMI
	ipr_pri = 0;
}


h8325_intc_device::h8325_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8_intc_device(mconfig, H8325_INTC, tag, owner, clock)
{
	m_irq_vector_base = 4;
	m_irq_vector_count = 3;
	m_irq_vector_nmi = 3;
}


h8h_intc_device::h8h_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8h_intc_device(mconfig, H8H_INTC, tag, owner, clock)
{
	m_irq_vector_base = 12;
	m_irq_vector_count = 8;
	m_irq_vector_nmi = 7;
}

h8h_intc_device::h8h_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	h8_intc_device(mconfig, type, tag, owner, clock)
{
}

void h8h_intc_device::device_start()
{
	h8_intc_device::device_start();
	save_item(NAME(m_icr));
}

void h8h_intc_device::device_reset()
{
	h8_intc_device::device_reset();
	m_icr = 0x000000;
}

uint8_t h8h_intc_device::isr_r()
{
	return m_isr;
}

void h8h_intc_device::isr_w(uint8_t data)
{
	m_isr &= data; // edge/level
	logerror("isr = %02x / %02x\n", data, m_isr);
	check_level_irqs(true);
}

uint8_t h8h_intc_device::icr_r(offs_t offset)
{
	return m_icr >> (8*offset);
}

void h8h_intc_device::icr_w(offs_t offset, uint8_t data)
{
	m_icr = (m_icr & (0xff << (8*offset))) | (data << (8*offset));
	logerror("icr %d = %02x\n", offset, data);
}

uint8_t h8h_intc_device::icrc_r()
{
	return icr_r(2);
}

void h8h_intc_device::icrc_w(uint8_t data)
{
	icr_w(2, data);
}

uint8_t h8h_intc_device::iscrh_r()
{
	return m_iscr >> 8;
}

void h8h_intc_device::iscrh_w(uint8_t data)
{
	m_iscr = (m_iscr & 0x00ff) | (data << 8);
	logerror("iscr = %04x\n", m_iscr);
	update_irq_types();
}

uint8_t h8h_intc_device::iscrl_r()
{
	return m_iscr;
}

void h8h_intc_device::iscrl_w(uint8_t data)
{
	m_iscr = (m_iscr & 0xff00) | data;
	logerror("iscr = %04x\n", m_iscr);
	update_irq_types();
}

void h8h_intc_device::update_irq_types()
{
	for(int i=0; i<8; i++)
		switch((m_iscr >> (2*i)) & 3) {
		case 0:
			m_irq_type[i] = IRQ_LEVEL;
			break;
		case 1: case 2:
			m_irq_type[i] = IRQ_EDGE;
			break;
		case 3:
			m_irq_type[i] = IRQ_DUAL_EDGE;
			break;
		}
	check_level_irqs();
}

const int h8h_intc_device::vector_to_slot[64] = {
	-1, -1, -1, -1, -1, -1, -1, -1, // NMI at 7
	-1, -1, -1, -1,  0,  1,  2,  2, // IRQ 0-3
	 3,  3,  3,  3,  4,  4,  4,  4, // IRQ 4-5, (reservedx2), WOVI, CMI, (reserved), ADI
	 5,  5,  5,  5,  6,  6,  6,  6, // IMIA0, IMIB0, OVI0, (reserved), IMIA1, IMIB1, OVI1, (reserved)
	 7,  7,  7,  7,  8,  8,  8,  8, // IMIA2, IMIB2, OVI2, (reserved), CMIA0, CMIB0, CMIx1, TOVI0/1
	 9,  9,  9,  9, 10, 10, 10, 10, // CMIA2, CMIB2, CMIx3, TOVI2/3, DEND0A, DEND0B, DEND1A, DEND1B
	11, 11, 11, 11, 12, 12, 12, 12, // (reservedx4), ERI0, RXI0, TXI0, TEI0
	13, 13, 13, 13, 14, 14, 14, 14  // ERI1, RXI1, TXI1, TEI1, ERI2, RXI2, TXI2, TEI2
};

void h8h_intc_device::get_priority(int vect, int &icr_pri, int &ipr_pri) const
{
	ipr_pri = 0;

	if(vect == 7) {
		icr_pri = 2;
		return;
	}
	int slot = vector_to_slot[vect];
	if(slot == -1) {
		icr_pri = 0;
		return;
	}

	icr_pri = (m_icr >> (slot ^ 7)) & 1;
}

h8s_intc_device::h8s_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8h_intc_device(mconfig, H8S_INTC, tag, owner, clock)
{
	m_irq_vector_base = 16;
	m_irq_vector_count = 8;
	m_irq_vector_nmi = 7;
}

void h8s_intc_device::device_reset()
{
	h8h_intc_device::device_reset();
	memset(m_ipr, 0x77, sizeof(m_ipr));
}

uint8_t h8s_intc_device::ipr_r(offs_t offset)
{
	return m_ipr[offset];
}

void h8s_intc_device::ipr_w(offs_t offset, uint8_t data)
{
	m_ipr[offset] = data;
	logerror("ipr %d = %02x\n", offset, data);
}

uint8_t h8s_intc_device::iprk_r()
{
	return ipr_r(10);
}

void h8s_intc_device::iprk_w(uint8_t data)
{
	ipr_w(10, data);
}

const int h8s_intc_device::vector_to_slot[92] = {
	-1, -1, -1, -1, -1, -1, -1, -1, // NMI at 7
	-1, -1, -1, -1, -1, -1, -1, -1,
	 0,  1,  2,  2,  3,  3,  4,  4, // IRQ 0-7
	 5,  6,  7,  8,  9,  9,  9,  9, // SWDTEND, WOVI, CMI, (reserved), ADI
	10, 10, 10, 10, 10, 10, 10, 10, // TGI0A, TGI0B, TGI0C, TGI0D, TGI0V
	11, 11, 11, 11, 12, 12, 12, 12, // TGI1A, TGI1B, TGI1V, TGI1U, TGI2A, TGI2B, TGI2V, TGI2U
	13, 13, 13, 13, 13, 13, 13, 13, // TGI3A, TGI3B, TGI3C, TGI3D, TGI3V
	14, 14, 14, 14, 15, 15, 15, 15, // TGI4A, TGI4B, TGI4V, TGI4U, TGI5A, TGI5B, TGI5V, TGI5U
	16, 16, 16, 16, 17, 17, 17, 17, // CMIA0, CMIB0, OVI0, CMIA1, CMIB1, OVI1
	18, 18, 18, 18, 18, 18, 18, 18, // DEND0A, DEND0B, DEND1B, DEND1B
	19, 19, 19, 19, 20, 20, 20, 20, // ERI0, RXI0, TXI0, TEI0, ERI1, RXI1, TXI1, TEI1
	21, 21, 21, 21                  // ERI2, RXI2, TXI2, TEI2
};

void h8s_intc_device::get_priority(int vect, int &icr_pri, int &ipr_pri) const
{
	if(vect == 7) {
		icr_pri = 2;
		ipr_pri = 8;
		return;
	}
	int slot = vector_to_slot[vect];
	if(slot == -1) {
		icr_pri = 0;
		ipr_pri = 0;
		return;
	}

	icr_pri = (m_icr >> (slot ^ 7)) & 1;
	ipr_pri = (m_ipr[slot >> 1] >> (slot & 1 ? 0 : 4)) & 7;
}


gt913_intc_device::gt913_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h8_intc_device(mconfig, GT913_INTC, tag, owner, clock)
{
	m_irq_vector_base = 4;
	m_irq_vector_count = 1;
	m_irq_vector_nmi = 3;
}

void gt913_intc_device::device_reset()
{
	h8_intc_device::device_reset();

	m_ier = 0x01;
}

void gt913_intc_device::clear_interrupt(int vector)
{
	m_pending_irqs[vector >> 5] &= ~(1 << (vector & 31));
	update_irq_state();
}
