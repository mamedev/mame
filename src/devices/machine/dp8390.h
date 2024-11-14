// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_MACHINE_DP8390_H
#define MAME_MACHINE_DP8390_H

#pragma once

#include "dinetwork.h"


// device stuff

class dp8390_device : public device_t, public device_network_interface
{
public:
	auto irq_callback() { return m_irq_cb.bind(); }
	auto breq_callback() { return m_breq_cb.bind(); }
	auto mem_read_callback() { return m_mem_read_cb.bind(); }
	auto mem_write_callback() { return m_mem_write_cb.bind(); }

	void remote_write(uint16_t data);
	void cs_write(offs_t offset, uint8_t data);
	uint16_t remote_read();
	uint8_t cs_read(offs_t offset);
	void dp8390_reset(int state);
	void recv_cb(uint8_t *buf, int len) override;

protected:
	enum class TYPE {
		DP8390D,
		RTL8019A
	};

	// construction/destruction
	dp8390_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, TYPE varian, u32 bandwidth);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TYPE const m_variant;

private:
	devcb_write_line    m_irq_cb;
	devcb_write_line    m_breq_cb;
	devcb_read8         m_mem_read_cb;
	devcb_write8        m_mem_write_cb;

	void set_cr(uint8_t newcr);
	void check_dma_complete();
	void do_tx();
	void check_irq() { m_irq_cb((m_regs.imr & m_regs.isr & 0x7f)?ASSERT_LINE:CLEAR_LINE); }
	void recv_overflow();
	void stop();
	void recv(uint8_t *buf, int len);

	int m_reset;
	int m_rdma_active;

	struct {
		uint8_t cr;
		uint16_t clda;
		uint8_t pstart;
		uint8_t pstop;
		uint8_t bnry;
		uint8_t tsr;
		uint8_t tpsr;
		uint8_t ncr;
		uint8_t fifo;
		uint16_t tbcr;
		uint8_t isr;
		uint16_t crda;
		uint16_t rsar;
		uint16_t rbcr;
		uint8_t rsr;
		uint8_t rcr;
		uint8_t cntr0;
		uint8_t tcr;
		uint8_t cntr1;
		uint8_t dcr;
		uint8_t cntr2;
		uint8_t imr;

		uint8_t par[6];
		uint8_t curr;
		uint8_t mar[8];

		uint8_t rnpp;
		uint8_t lnpp;
		uint16_t ac;
	} m_regs;

	struct {
		uint8_t cr9346;
		uint8_t bpage;
		uint8_t config0;
		uint8_t config1;
		uint8_t config2;
		uint8_t config3;
		uint8_t config4;
		uint8_t csnsav;
		uint8_t intr;
	} m_8019regs;
};

class rtl8019a_device : public dp8390_device
{
public:
	rtl8019a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class dp8390d_device : public dp8390_device
{
public:
	dp8390d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(DP8390D,  dp8390d_device)
DECLARE_DEVICE_TYPE(RTL8019A, rtl8019a_device)

#endif // MAME_MACHINE_DP8390_H
