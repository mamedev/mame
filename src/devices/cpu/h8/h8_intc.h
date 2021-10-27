// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_intc.h

    H8 interrupt controllers family


***************************************************************************/

#ifndef MAME_CPU_H8_H8_INTC_H
#define MAME_CPU_H8_H8_INTC_H

#pragma once

#include "h8.h"


class h8_intc_device : public device_t {
public:
	h8_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	int interrupt_taken(int vector);
	void internal_interrupt(int vector);
	void set_input(int inputnum, int state);
	void set_filter(int icr_filter, int ipr_filter);

	uint8_t ier_r();
	void ier_w(uint8_t data);
	uint8_t iscr_r();
	void iscr_w(uint8_t data);

protected:
	enum { IRQ_LEVEL, IRQ_EDGE, IRQ_DUAL_EDGE };
	enum { MAX_VECTORS = 256 };

	int irq_vector_base;
	int irq_vector_nmi;

	required_device<h8_device> cpu;

	uint32_t pending_irqs[MAX_VECTORS/32];
	int irq_type[8];
	bool nmi_input;
	uint8_t irq_input;
	uint8_t ier;
	uint8_t isr;
	uint16_t iscr;
	int icr_filter, ipr_filter;

	h8_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const;
	void update_irq_state();
	void update_irq_types();
	void check_level_irqs(bool force_update = false);
};

class gt913_intc_device : public h8_intc_device {
public:
	gt913_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	gt913_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class h8h_intc_device : public h8_intc_device {
public:
	h8h_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t isr_r();
	void isr_w(uint8_t data);
	uint8_t icr_r(offs_t offset);
	void icr_w(offs_t offset, uint8_t data);
	uint8_t icrc_r();
	void icrc_w(uint8_t data);
	uint8_t iscrh_r();
	void iscrh_w(uint8_t data);
	uint8_t iscrl_r();
	void iscrl_w(uint8_t data);

protected:
	static const int vector_to_slot[];

	uint32_t icr;

	h8h_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const override;
	void update_irq_types();
};

class h8s_intc_device : public h8h_intc_device {
public:
	h8s_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t ipr_r(offs_t offset);
	void ipr_w(offs_t offset, uint8_t data);
	uint8_t iprk_r();
	void iprk_w(uint8_t data);
private:
	static const int vector_to_slot[];
	uint8_t ipr[11];

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(H8_INTC,    h8_intc_device)
DECLARE_DEVICE_TYPE(H8H_INTC,   h8h_intc_device)
DECLARE_DEVICE_TYPE(H8S_INTC,   h8s_intc_device)
DECLARE_DEVICE_TYPE(GT913_INTC, gt913_intc_device)

#endif // MAME_CPU_H8_H8_INTC_H
