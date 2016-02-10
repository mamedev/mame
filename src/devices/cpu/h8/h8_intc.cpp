// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8_intc.h"

const device_type H8_INTC  = &device_creator<h8_intc_device>;
const device_type H8H_INTC = &device_creator<h8h_intc_device>;
const device_type H8S_INTC = &device_creator<h8s_intc_device>;

h8_intc_device::h8_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, H8_INTC, "H8 INTC", tag, owner, clock, "h8_intc", __FILE__),
	cpu(*this, DEVICE_SELF_OWNER), nmi_input(false), irq_input(0), ier(0), isr(0), iscr(0), icr_filter(0), ipr_filter(0)
{
	irq_vector_base = 4;
	irq_vector_nmi = 3;
}

h8_intc_device::h8_intc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source), irq_vector_base(0), irq_vector_nmi(0),
	cpu(*this, DEVICE_SELF_OWNER), nmi_input(false), irq_input(0), ier(0), isr(0), iscr(0), icr_filter(0), ipr_filter(0)
{
}

void h8_intc_device::device_start()
{
	memset(pending_irqs, 0, sizeof(pending_irqs));
	save_item(NAME(pending_irqs));
	save_item(NAME(irq_type));
	save_item(NAME(nmi_input));
	save_item(NAME(irq_input));
	save_item(NAME(ier));
	save_item(NAME(isr));
	save_item(NAME(iscr));
	save_item(NAME(icr_filter));
	save_item(NAME(ipr_filter));
}

void h8_intc_device::device_reset()
{
	memset(irq_type, 0, sizeof(irq_type));
	memset(pending_irqs, 0, sizeof(pending_irqs));
	ier = isr = irq_input = 0x00;
	iscr = 0x0000;
}

int h8_intc_device::interrupt_taken(int vector)
{
	if(0)
		logerror("%s: taking internal interrupt %d\n", tag(), vector);
	pending_irqs[vector >> 5] &= ~(1 << (vector & 31));
	if(vector >= irq_vector_base && vector < irq_vector_base + 8) {
		int irq = vector - irq_vector_base;
		if(irq_type[irq] != IRQ_LEVEL || !(irq_input & (1 << irq)))
			isr &= ~(1 << irq);
		update_irq_state();
		return irq;
	}
	update_irq_state();
	if(vector == irq_vector_nmi)
		return INPUT_LINE_NMI;
	return 8;
}

void h8_intc_device::internal_interrupt(int vector)
{
	if(0)
		logerror("%s: internal interrupt %d\n", tag(), vector);
	pending_irqs[vector >> 5] |= 1 << (vector & 31);
	update_irq_state();
}

void h8_intc_device::set_input(int inputnum, int state)
{
	if(inputnum == INPUT_LINE_NMI) {
		if(state == ASSERT_LINE && !nmi_input)
			pending_irqs[0] |= 1 << irq_vector_nmi;
		nmi_input = state == ASSERT_LINE;
		update_irq_state();
	} else {
		bool set = false;
		bool cur = irq_input & (1 << inputnum);
		switch(irq_type[inputnum]) {
		case IRQ_LEVEL: set = state == ASSERT_LINE; break;
		case IRQ_EDGE: set = state == ASSERT_LINE && !cur; break;
		case IRQ_DUAL_EDGE: set = (state == ASSERT_LINE && !cur) || (state == CLEAR_LINE && cur); break;
		}
		if(state == ASSERT_LINE)
			irq_input |= 1 << inputnum;
		else
			irq_input &= ~(1 << inputnum);
		if(set) {
			isr |= 1 << inputnum;
			update_irq_state();
		}
	}
}

void h8_intc_device::set_filter(int _icr_filter, int _ipr_filter)
{
	icr_filter = _icr_filter;
	ipr_filter = _ipr_filter;
	update_irq_state();
}

READ8_MEMBER(h8_intc_device::ier_r)
{
	return ier;
}

WRITE8_MEMBER(h8_intc_device::ier_w)
{
	ier = data;
	logerror("%s: ier = %02x\n", tag(), data);
	update_irq_state();
}

void h8_intc_device::check_level_irqs(bool force_update)
{
	logerror("%s: irq_input=%02x\n", tag(), irq_input);
	bool update = force_update;
	for(int i=0; i<8; i++) {
		unsigned char mask = 1 << i;
		if(irq_type[i] == IRQ_LEVEL && (irq_input & mask) && !(isr & mask)) {
			isr |= mask;
			update = true;
		}
	}
	if(update)
		update_irq_state();
}


READ8_MEMBER(h8_intc_device::iscr_r)
{
	return iscr;
}

WRITE8_MEMBER(h8_intc_device::iscr_w)
{
	iscr = data;
	logerror("%s: iscr = %02x\n", tag(), iscr);
	update_irq_types();
}

void h8_intc_device::update_irq_types()
{
	for(int i=0; i<8; i++)
		switch((iscr >> (i)) & 1) {
		case 0:
			irq_type[i] = IRQ_LEVEL;
			break;
		case 1:
			irq_type[i] = IRQ_EDGE;
			break;
		}
	check_level_irqs();
}

void h8_intc_device::update_irq_state()
{
	pending_irqs[0] &= ~(255 << irq_vector_base);
	pending_irqs[0] |= (isr & ier) << irq_vector_base;

	int cur_vector = 0;
	int cur_level = -1;

	for(int i=0; i<MAX_VECTORS/32; i++) {
		unsigned int pending = pending_irqs[i];
		if(pending)
			for(int j=0; j<32; j++)
				if(pending & (1 << j)) {
					int vect = i*32+j;
					int icr_pri, ipr_pri;
					get_priority(vect, icr_pri, ipr_pri);
					if(icr_pri >= icr_filter && ipr_pri > ipr_filter) {
						int level = ipr_filter == -1 ? icr_pri : ipr_pri;
						if(level > cur_level) {
							cur_vector = vect;
							cur_level = level;
						}
					}
				}
	}
	cpu->set_irq(cur_vector, cur_level, cur_vector == irq_vector_nmi);
}

void h8_intc_device::get_priority(int vect, int &icr_pri, int &ipr_pri) const
{
	icr_pri = 0;
	ipr_pri = 0;
}


h8h_intc_device::h8h_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	h8_intc_device(mconfig, H8H_INTC, "H8H INTC", tag, owner, clock, "h8h_intc", __FILE__)
{
	irq_vector_base = 12;
	irq_vector_nmi = 7;
}

h8h_intc_device::h8h_intc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	h8_intc_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

void h8h_intc_device::device_start()
{
	h8_intc_device::device_start();
	save_item(NAME(icr));
}

void h8h_intc_device::device_reset()
{
	h8_intc_device::device_reset();
	icr = 0x000000;
}

READ8_MEMBER(h8h_intc_device::isr_r)
{
	return isr;
}

WRITE8_MEMBER(h8h_intc_device::isr_w)
{
	isr &= data; // edge/level
	logerror("%s: isr = %02x / %02x\n", tag(), data, isr);
	check_level_irqs(true);
}

READ8_MEMBER(h8h_intc_device::icr_r)
{
	return icr >> (8*offset);
}

WRITE8_MEMBER(h8h_intc_device::icr_w)
{
	icr = (icr & (0xff << (8*offset))) | (data << (8*offset));
	logerror("%s: icr %d = %02x\n", tag(), offset, data);
}

READ8_MEMBER(h8h_intc_device::icrc_r)
{
	return icr_r(space, 2, mem_mask);
}

WRITE8_MEMBER(h8h_intc_device::icrc_w)
{
	icr_w(space, 2, data, mem_mask);
}

READ8_MEMBER(h8h_intc_device::iscrh_r)
{
	return iscr >> 8;
}

WRITE8_MEMBER(h8h_intc_device::iscrh_w)
{
	iscr = (iscr & 0x00ff) | (data << 8);
	logerror("%s: iscr = %04x\n", tag(), iscr);
	update_irq_types();
}

READ8_MEMBER(h8h_intc_device::iscrl_r)
{
	return iscr;
}

WRITE8_MEMBER(h8h_intc_device::iscrl_w)
{
	iscr = (iscr & 0xff00) | data;
	logerror("%s: iscr = %04x\n", tag(), iscr);
	update_irq_types();
}

void h8h_intc_device::update_irq_types()
{
	for(int i=0; i<8; i++)
		switch((iscr >> (2*i)) & 3) {
		case 0:
			irq_type[i] = IRQ_LEVEL;
			break;
		case 1: case 2:
			irq_type[i] = IRQ_EDGE;
			break;
		case 3:
			irq_type[i] = IRQ_DUAL_EDGE;
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

	icr_pri = (icr >> (slot ^ 7)) & 1;
}

h8s_intc_device::h8s_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	h8h_intc_device(mconfig, H8S_INTC, "H8S INTC", tag, owner, clock, "h8s_intc", __FILE__)
{
	irq_vector_base = 16;
	irq_vector_nmi = 7;
}

void h8s_intc_device::device_reset()
{
	h8h_intc_device::device_reset();
	memset(ipr, 0x77, sizeof(ipr));
}

READ8_MEMBER(h8s_intc_device::ipr_r)
{
	return ipr[offset];
}

WRITE8_MEMBER(h8s_intc_device::ipr_w)
{
	ipr[offset] = data;
	logerror("%s: ipr %d = %02x\n", tag(), offset, data);
}

READ8_MEMBER(h8s_intc_device::iprk_r)
{
	return ipr_r(space, 10, mem_mask);
}

WRITE8_MEMBER(h8s_intc_device::iprk_w)
{
	ipr_w(space, 10, data, mem_mask);
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

	icr_pri = (icr >> (slot ^ 7)) & 1;
	ipr_pri = (ipr[slot >> 1] >> (slot & 1 ? 4 : 0)) & 7;
}
