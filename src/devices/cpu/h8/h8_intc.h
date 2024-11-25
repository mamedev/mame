// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    h8_intc.h

    H8 interrupt controllers family

***************************************************************************/

#ifndef MAME_CPU_H8_H8_INTC_H
#define MAME_CPU_H8_H8_INTC_H

#pragma once

class h8_device;

class h8_intc_device : public device_t {
public:
	h8_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T> h8_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu) :
		h8_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	int interrupt_taken(int vector);
	void internal_interrupt(int vector);
	void set_input(int inputnum, int state);
	void set_filter(int icr_filter, int ipr_filter);
	void set_nmi_edge(int state) { m_nmi_type = state ? EDGE_RISE : EDGE_FALL; }

	u8 ier_r();
	void ier_w(u8 data);
	u8 iscr_r();
	void iscr_w(u8 data);

protected:
	enum {
		LEVEL_LOW,  // ASSERT
		EDGE_FALL,  // CLEAR->ASSERT
		EDGE_RISE,  // ASSERT->CLEAR
		EDGE_DUAL
	};
	enum { MAX_VECTORS = 256 };

	int m_irq_vector_base;
	int m_irq_vector_count;
	int m_irq_vector_nmi;
	bool m_has_isr;

	required_device<h8_device> m_cpu;

	u32 m_pending_irqs[MAX_VECTORS/32];
	u8 m_irq_type[8];
	u8 m_nmi_type;
	bool m_nmi_input;
	u8 m_irq_input;
	u8 m_ier;
	u8 m_isr;
	u16 m_iscr;
	int m_icr_filter, m_ipr_filter;

	h8_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const;
	void update_irq_state();
	virtual void update_irq_types();
	void check_level_irqs(bool update);
};

class h8325_intc_device : public h8_intc_device {
public:
	h8325_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T> h8325_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu) :
		h8325_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

protected:
	virtual void update_irq_types() override;
};

class h8h_intc_device : public h8_intc_device {
public:
	h8h_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T> h8h_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu) :
		h8h_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	u8 isr_r();
	void isr_w(u8 data);
	u8 icr_r(offs_t offset);
	void icr_w(offs_t offset, u8 data);

protected:
	static const int vector_to_slot[];

	u32 m_icr;

	h8h_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const override;
};

class h8s_intc_device : public h8h_intc_device {
public:
	h8s_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T> h8s_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu) :
		h8s_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	u8 ipr_r(offs_t offset);
	void ipr_w(offs_t offset, u8 data);
	u8 iscrh_r();
	void iscrh_w(u8 data);
	u8 iscrl_r();
	void iscrl_w(u8 data);

private:
	static const int vector_to_slot[];
	u8 m_ipr[11];

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void get_priority(int vect, int &icr_pri, int &ipr_pri) const override;
	virtual void update_irq_types() override;
};

class gt913_intc_device : public h8_intc_device {
public:
	gt913_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T> gt913_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu) :
		gt913_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	void clear_interrupt(int vector);

protected:
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(H8_INTC,    h8_intc_device)
DECLARE_DEVICE_TYPE(H8325_INTC, h8325_intc_device)
DECLARE_DEVICE_TYPE(H8H_INTC,   h8h_intc_device)
DECLARE_DEVICE_TYPE(H8S_INTC,   h8s_intc_device)
DECLARE_DEVICE_TYPE(GT913_INTC, gt913_intc_device)

#endif // MAME_CPU_H8_H8_INTC_H
