// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    1801VP1-033 Gate Array emulation

**********************************************************************/

#ifndef MAME_USSR_1801VP033_H
#define MAME_USSR_1801VP033_H

#pragma once

#include "machine/pdp11.h"
#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> k1801vp033_device

class k1801vp033_device : public device_t,
						  public device_z80daisy_interface
{
public:
	// construction/destruction
	k1801vp033_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// BPIC mode

	void set_txvec(int vec) { m_txvec = vec; }
	void set_rxvec(int vec) { m_rxvec = vec; }

	auto bpic_txrdy_wr_callback() { return m_bpic_write_txrdy.bind(); }
	auto bpic_reset_wr_callback() { return m_bpic_write_reset.bind(); }
	auto bpic_strobe_wr_callback() { return m_bpic_write_strobe.bind(); }
	auto bpic_pd_wr_callback() { return m_bpic_write_pd.bind(); }

	uint16_t bpic_read(offs_t offset);
	void bpic_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void bpic_write_drq(int state) { if (state) { m_tcsr |= BPICCSR_DRQ; raise_virq(m_bpic_write_txrdy, m_tcsr, CSR_IE, m_txrdy); } }
	void bpic_write_err(int state) { if (state) m_tcsr |= CSR_ERR; else m_tcsr &= ~CSR_ERR; }

	// PIC mode

	void set_vec_a(int vec) { m_txvec = vec; }
	void set_vec_b(int vec) { m_rxvec = vec; }

	auto pic_out_wr_callback() { return m_pic_write_out.bind(); }
	auto pic_csr0_wr_callback() { return m_pic_write_csr0.bind(); }
	auto pic_csr1_wr_callback() { return m_pic_write_csr1.bind(); }

	uint16_t pic_read(offs_t offset);
	void pic_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void pic_write_reqa(int state) { if (state) { m_tcsr |= PICCSR_REQA; raise_virq(m_pic_write_reqa, m_tcsr, PICCSR_IEA, m_txrdy); } }
	void pic_write_reqb(int state) { if (state) { m_tcsr |= PICCSR_REQB; raise_virq(m_pic_write_reqb, m_tcsr, PICCSR_IEB, m_rxrdy); } }
	void pic_write_rbuf(uint16_t data) { m_rbuf = data; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override {};

private:
	static constexpr uint16_t BPICCSR_RDY    = 0000040;
	static constexpr uint16_t BPICCSR_DRQ    = 0000200;
	static constexpr uint16_t BPICCSR_RESET  = 0040000;
	static constexpr uint16_t BPICCSR_RD     = (CSR_DONE | CSR_IE | CSR_ERR | BPICCSR_RDY | BPICCSR_DRQ);
	static constexpr uint16_t BPICCSR_WR     = (CSR_IE | BPICCSR_RESET);

	static constexpr uint16_t PICCSR_CSR0    = 0000001;
	static constexpr uint16_t PICCSR_CSR1    = 0000002;
	static constexpr uint16_t PICCSR_IEB     = 0000040;
	static constexpr uint16_t PICCSR_IEA     = 0000100;
	static constexpr uint16_t PICCSR_REQA    = 0000200;
	static constexpr uint16_t PICCSR_REQB    = 0100000;
	static constexpr uint16_t PICCSR_RD      = (PICCSR_REQB | PICCSR_REQA | PICCSR_IEA | PICCSR_IEB | PICCSR_CSR1 | PICCSR_CSR0);
	static constexpr uint16_t PICCSR_WR      = (PICCSR_IEA | PICCSR_IEB | PICCSR_CSR1 | PICCSR_CSR0);

	devcb_write_line   m_bpic_write_rxrdy;
	devcb_write_line   m_bpic_write_txrdy;
	devcb_write_line   m_bpic_write_reset;
	devcb_write_line   m_bpic_write_strobe;
	devcb_write8       m_bpic_write_pd;

	devcb_write_line   m_pic_write_reqa;
	devcb_write_line   m_pic_write_reqb;
	devcb_write_line   m_pic_write_csr0;
	devcb_write_line   m_pic_write_csr1;
	devcb_write16      m_pic_write_out;

	line_state m_rxrdy;
	line_state m_txrdy;

	int m_rxvec;
	int m_txvec;

	uint16_t m_rcsr;
	uint16_t m_rbuf;
	uint16_t m_tcsr;
	uint16_t m_tbuf;
};


// device type definition
DECLARE_DEVICE_TYPE(K1801VP033, k1801vp033_device)

#endif // MAME_USSR_1801VP033_H
