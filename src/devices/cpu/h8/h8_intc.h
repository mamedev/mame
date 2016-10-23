// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_intc.h

    H8 interrupt controllers family


***************************************************************************/

#ifndef __H8_INTC_H__
#define __H8_INTC_H__

#include "h8.h"

#define MCFG_H8_INTC_ADD( _tag )    \
	MCFG_DEVICE_ADD( _tag, H8_INTC, 0 )

#define MCFG_H8H_INTC_ADD( _tag )   \
	MCFG_DEVICE_ADD( _tag, H8H_INTC, 0 )

#define MCFG_H8S_INTC_ADD( _tag )   \
	MCFG_DEVICE_ADD( _tag, H8S_INTC, 0 )


class h8_intc_device : public device_t {
public:
	h8_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8_intc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	int interrupt_taken(int vector);
	void internal_interrupt(int vector);
	void set_input(int inputnum, int state);
	void set_filter(int icr_filter, int ipr_filter);

	uint8_t ier_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ier_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iscr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iscr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const;
	void update_irq_state();
	void update_irq_types();
	void check_level_irqs(bool force_update = false);
};

class h8h_intc_device : public h8_intc_device {
public:
	h8h_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	h8h_intc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	uint8_t isr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void isr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t icr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void icr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t icrc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void icrc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iscrh_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iscrh_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iscrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iscrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	static const int vector_to_slot[];

	uint32_t icr;

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const override;
	void update_irq_types();
};

class h8s_intc_device : public h8h_intc_device {
public:
	h8s_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ipr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ipr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t iprk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iprk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
private:
	static const int vector_to_slot[];
	uint8_t ipr[11];

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const override;
	virtual void device_reset() override;
};

extern const device_type H8_INTC;
extern const device_type H8H_INTC;
extern const device_type H8S_INTC;

#endif
