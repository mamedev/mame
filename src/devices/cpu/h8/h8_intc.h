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

	DECLARE_READ8_MEMBER(ier_r);
	DECLARE_WRITE8_MEMBER(ier_w);
	DECLARE_READ8_MEMBER(iscr_r);
	DECLARE_WRITE8_MEMBER(iscr_w);

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

class h8h_intc_device : public h8_intc_device {
public:
	h8h_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_READ8_MEMBER(isr_r);
	DECLARE_WRITE8_MEMBER(isr_w);
	DECLARE_READ8_MEMBER(icr_r);
	DECLARE_WRITE8_MEMBER(icr_w);
	DECLARE_READ8_MEMBER(icrc_r);
	DECLARE_WRITE8_MEMBER(icrc_w);
	DECLARE_READ8_MEMBER(iscrh_r);
	DECLARE_WRITE8_MEMBER(iscrh_w);
	DECLARE_READ8_MEMBER(iscrl_r);
	DECLARE_WRITE8_MEMBER(iscrl_w);

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

	DECLARE_READ8_MEMBER(ipr_r);
	DECLARE_WRITE8_MEMBER(ipr_w);
	DECLARE_READ8_MEMBER(iprk_r);
	DECLARE_WRITE8_MEMBER(iprk_w);
private:
	static const int vector_to_slot[];
	uint8_t ipr[11];

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(H8_INTC,  h8_intc_device)
DECLARE_DEVICE_TYPE(H8H_INTC, h8h_intc_device)
DECLARE_DEVICE_TYPE(H8S_INTC, h8s_intc_device)

#endif // MAME_CPU_H8_H8_INTC_H
