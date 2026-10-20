// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    h8500_intc.h

    H8/500 family interrupt controller.

    The base device implements the H8/510 configuration; subclasses
    adjust the external pin set and priority slot table for other family
    members.

    Register set (per-part address maps route to these):

      IPRA-IPRD   Interrupt priority registers.  Each register holds two
                  3-bit priority levels (bits 6-4 and 2-0), one per source
                  module.  H8/510: IPRA = IRQ0+WDT / IRQ1-IRQ3.  H8/520:
                  IPRA = IRQ0+WDT / IRQ1-IRQ7 (the WDT interval interrupt
                  shares IRQ0's vector as well).  IPRB-IPRD are FRT1/FRT2,
                  8-bit timer/SCI1 and SCI2/ADC on both.
      IPRE-IPRF   Additional priority registers on the H8/534 (and later
                  variants): IPRA/IPRB cover IRQ0+WDT/IRQ1 and IRQ2-IRQ5
                  in pairs, IPRC-IPRF cover FRT1-3, the 8-bit timer, the
                  two SCIs and the A/D converter.
      DTEA-DTED   Data transfer enable registers, one bit per source.
                  Storage only until the DTC is implemented; every
                  accepted interrupt is currently served by the CPU,
                  with a once-per-vector warning when its module has
                  DTC service enabled (see interrupt_taken).
      DTEE-DTEF   Additional DTC enable registers, paired with IPRE/IPRF.
      NMICR       NMI control register: NMIEG edge select in bit 0.
      IRQCR       IRQ control register: IRQnE pin enables (four pins on
                  the H8/510, eight on the H8/520).  Pin senses are fixed
                  in hardware - IRQ0 is level (low), the others are
                  falling-edge, and the NMI edge is chosen by NMICR.

***************************************************************************/

#ifndef MAME_CPU_H8500_H8500_INTC_H
#define MAME_CPU_H8500_H8500_INTC_H

#pragma once

#include "cpu/h8/h8_intc_base.h"

class h8_cpu_base;

class h8500_intc_device : public h8_intc_base
{
public:
	h8500_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T>
	h8500_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: h8500_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	// h8_intc_base
	virtual void internal_interrupt(int vector) override;

	int interrupt_taken(int vector);
	void set_input(int inputnum, int state);
	void set_filter(int ipr_filter);

	// Register handlers
	u8   ipra_r() { return ipr_r(0); }
	void ipra_w(u8 data) { ipr_w(0, data); }
	u8   iprb_r() { return ipr_r(1); }
	void iprb_w(u8 data) { ipr_w(1, data); }
	u8   iprc_r() { return ipr_r(2); }
	void iprc_w(u8 data) { ipr_w(2, data); }
	u8   iprd_r() { return ipr_r(3); }
	void iprd_w(u8 data) { ipr_w(3, data); }
	u8   ipre_r() { return ipr_r(4); }
	void ipre_w(u8 data) { ipr_w(4, data); }
	u8   iprf_r() { return ipr_r(5); }
	void iprf_w(u8 data) { ipr_w(5, data); }

	u8   dtea_r() { return m_dte[0]; }
	void dtea_w(u8 data) { m_dte[0] = data; }
	u8   dteb_r() { return m_dte[1]; }
	void dteb_w(u8 data) { m_dte[1] = data; }
	u8   dtec_r() { return m_dte[2]; }
	void dtec_w(u8 data) { m_dte[2] = data; }
	u8   dted_r() { return m_dte[3]; }
	void dted_w(u8 data) { m_dte[3] = data; }
	u8   dtee_r() { return m_dte[4]; }
	void dtee_w(u8 data) { m_dte[4] = data; }
	u8   dtef_r() { return m_dte[5]; }
	void dtef_w(u8 data) { m_dte[5] = data; }

	u8   nmicr_r();
	void nmicr_w(u8 data);
	u8   irqcr_r();
	void irqcr_w(u8 data);

protected:
	// The whole exception vector table fits in the first 80 entries
	// (the H8/534's A/D converter vector, 72, is the highest used)
	enum { MAX_VECTORS = 80 };

	static constexpr int NMI_VECTOR = 11;

	h8500_intc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual int get_priority(int vect) const;
	void update_irq_state();
	void set_irq_request(int pin, bool state);
	void check_external_irqs();

	u8   ipr_r(int n) const;
	void ipr_w(int n, u8 data);

	required_device<h8_cpu_base> m_cpu;

	// per-part configuration, set by the constructors
	const int *m_irq_vectors;     // vector number for each external IRQ pin
	int m_irq_pin_count;
	const int *m_vector_to_slot;  // vector -> IPR slot (2*reg + hi/lo), -1 = fixed
	u8 m_irqcr_bits;              // implemented IRQnE bits in IRQCR

	u32 m_pending_irqs[(MAX_VECTORS + 31) / 32];
	int m_ipr_filter;       // current CPU interrupt mask level (SR I2-I0)
	bool m_nmi_input;       // NMI pin state
	u8 m_irq_input;         // IRQn pin states
	u8 m_irq_req;           // active requests: bit 0 tracks the IRQ0 level, the rest are edge latches

	u8 m_ipr[6];
	u8 m_dte[6];
	u8 m_nmicr;
	u8 m_irqcr;
};

class h8520_intc_device : public h8500_intc_device
{
public:
	h8520_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T>
	h8520_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: h8520_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}
};

// The H8/532 has only NMI/IRQ0/IRQ1 and packs the interrupt pin enables
// and NMI edge select into the port 1 control register (P1CR) along with
// the bus release enable, instead of separate NMICR/IRQCR registers.
class h8532_intc_device : public h8500_intc_device
{
public:
	h8532_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T>
	h8532_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: h8532_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	u8   p1cr_r();
	void p1cr_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_brle;
};

// The H8/534 has NMI and IRQ0-IRQ5 and packs the pin enables into two
// system control registers instead of NMICR/IRQCR: SYSCR1 carries
// IRQ1E/IRQ0E/NMIEG/BRLE (the same layout as the H8/532's P1CR) and
// SYSCR2 carries IRQ5E-IRQ2E plus three pin-function selects.
class h8534_intc_device : public h8500_intc_device
{
public:
	h8534_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T>
	h8534_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu)
		: h8534_intc_device(mconfig, tag, owner)
	{
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	u8   syscr1_r();
	void syscr1_w(u8 data);
	u8   syscr2_r();
	void syscr2_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 m_brle;
	u8 m_pin_ctl;   // SYSCR2 bits 2-0: P6PWME, P9PWME, P9SCI2E (storage only)
};

DECLARE_DEVICE_TYPE(H8500_INTC, h8500_intc_device)
DECLARE_DEVICE_TYPE(H8520_INTC, h8520_intc_device)
DECLARE_DEVICE_TYPE(H8532_INTC, h8532_intc_device)
DECLARE_DEVICE_TYPE(H8534_INTC, h8534_intc_device)

#endif // MAME_CPU_H8500_H8500_INTC_H
