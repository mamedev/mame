// license:BSD-3-Clause
// copyright-holders:R. Belmont
/******************************************************************************
*
*   MIPS DECstation I/O Gate Array emulation
*
*
*/
#ifndef MAME_MACHINE_DECIOGA_H
#define MAME_MACHINE_DECIOGA_H

#pragma once


class dec_ioga_device : public device_t
{
public:
	dec_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void map(address_map &map);

	// irq inputs
	DECLARE_WRITE_LINE_MEMBER(rtc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(lance_irq_w);
	DECLARE_WRITE_LINE_MEMBER(scc0_irq_w);
	DECLARE_WRITE_LINE_MEMBER(scc1_irq_w);

	// multiplex irq output
	auto irq_out() { return m_irq_out_cb.bind(); }

	// DMA interface
	void set_dma_space(address_space *space);
	u16 lance_dma_r(offs_t offset);
	void lance_dma_w(offs_t offset, u16 data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	u32 csr_r();
	void csr_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 intr_r();
	void intr_w(u32 data);
	u32 imsk_r();
	void imsk_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 dmaptr_r(offs_t offset);
	void dmaptr_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	address_space *m_maincpu_space;

private:
	u32 m_csr, m_intr, m_imsk;
	u32 m_dmaptrs[0x10];

	devcb_write_line m_irq_out_cb;

	void recalc_irq();
};

DECLARE_DEVICE_TYPE(DECSTATION_IOGA, dec_ioga_device)

#endif // MAME_MACHINE_DECIOGA_H
