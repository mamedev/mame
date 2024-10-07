// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 Interrupt Controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_INTC_H
#define MAME_CPU_SH_SH7014_INTC_H

#pragma once


class sh7014_intc_device : public device_t
{
public:
	enum {
		INT_VECTOR_NMI = 11,

		INT_VECTOR_IRQ0 = 64,
		INT_VECTOR_IRQ1 = 65,
		INT_VECTOR_IRQ2 = 66,
		INT_VECTOR_IRQ3 = 67,
		// 68 and 69 are reserved
		INT_VECTOR_IRQ6 = 70,
		INT_VECTOR_IRQ7 = 71,

		INT_VECTOR_DMA_CH0 = 72,
		INT_VECTOR_DMA_CH1 = 76,

		INT_VECTOR_MTU_TGI0A = 88,
		INT_VECTOR_MTU_TGI0B = 89,
		INT_VECTOR_MTU_TGI0C = 90,
		INT_VECTOR_MTU_TGI0D = 91,
		INT_VECTOR_MTU_TGI0V = 92,

		INT_VECTOR_MTU_TGI1A = 96,
		INT_VECTOR_MTU_TGI1B = 97,
		INT_VECTOR_MTU_TGI1V = 100,
		INT_VECTOR_MTU_TGI1U = 101,

		INT_VECTOR_MTU_TGI2A = 104,
		INT_VECTOR_MTU_TGI2B = 105,
		INT_VECTOR_MTU_TGI2V = 108,
		INT_VECTOR_MTU_TGI2U = 109,

		INT_VECTOR_SCI_ERI0 = 128,
		INT_VECTOR_SCI_RXI0 = 129,
		INT_VECTOR_SCI_TXI0 = 130,
		INT_VECTOR_SCI_TEI0 = 131,

		INT_VECTOR_SCI_ERI1 = 132,
		INT_VECTOR_SCI_RXI1 = 133,
		INT_VECTOR_SCI_TXI1 = 134,
		INT_VECTOR_SCI_TEI1 = 135,

		INT_VECTOR_AD = 136,

		INT_VECTOR_CMT_CH0 = 144,
		INT_VECTOR_CMT_CH1 = 148,

		INT_VECTOR_WDT = 152,

		INT_VECTOR_BSC = 153,
	};

	typedef device_delegate<void (int vector, int level, bool is_internal)> set_irq_delegate;

	sh7014_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename... T> void set_irq_callback(T &&... args) { m_set_irq_cb.set(std::forward<T>(args)...); }

	void map(address_map &map) ATTR_COLD;

	void set_input(int inputnum, int state);

	void set_interrupt(int vector, int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum {
		IRQ_LEVEL,
		IRQ_EDGE,
	};

	enum {
		MAX_VECTORS = 256,
	};

	enum {
		ICR_NMIE = 1 << 8,
		ICR_NMIL = 1 << 15,
	};

	uint16_t ipra_r();
	void ipra_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t iprb_r();
	void iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t iprc_r();
	void iprc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t iprd_r();
	void iprd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t ipre_r();
	void ipre_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t iprf_r();
	void iprf_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t iprg_r();
	void iprg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t iprh_r();
	void iprh_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t icr_r();
	void icr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t isr_r();
	void isr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void update_irq_state();

	set_irq_delegate m_set_irq_cb;

	uint16_t m_ipra, m_iprb, m_iprc, m_iprd, m_ipre, m_iprf, m_iprg, m_iprh;
	uint16_t m_icr, m_isr;

	int8_t m_irq_levels[MAX_VECTORS];

	uint32_t m_irq_type[8];
	bool m_nmi_input;

	uint32_t m_pending_irqs[MAX_VECTORS/32];
};


DECLARE_DEVICE_TYPE(SH7014_INTC, sh7014_intc_device)

#endif // MAME_CPU_SH_SH7014_INTC_H
