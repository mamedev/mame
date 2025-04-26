// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_AM79C90_H
#define MAME_MACHINE_AM79C90_H

#pragma once

#include "dinetwork.h"

class am7990_device_base : public device_t, public device_network_interface
{
public:
	auto intr_out() { return m_intr_out_cb.bind(); }
	auto dma_in() { return m_dma_in_cb.bind(); }
	auto dma_out() { return m_dma_out_cb.bind(); }

	u16 regs_r(address_space &space, offs_t offset);
	void regs_w(offs_t offset, u16 data);

	void reset_w(int state) { if (!state) device_reset(); }

protected:
	am7990_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_network_interface overrides
	virtual int recv_start_cb(u8 *buf, int length) override;
	virtual void recv_complete_cb(int result) override;
	virtual void send_complete_cb(int result) override;

	// device helpers
	void initialize();
	void interrupt(s32 param);
	void update_interrupts(attotime const delay = attotime::zero);
	int receive(u8 *buf, int length);
	void transmit_poll(s32 param);
	void transmit();
	bool address_filter(u8 *buf);

	void dma_in(u32 address, u8 *buf, int length);
	void dma_out(u32 address, u8 *buf, int length);
	void dump_bytes(u8 *buf, int length);

	virtual int get_buf_length(u16 data) const = 0;

	// constants and masks
	static constexpr u32 FCS_RESIDUE = 0xdebb20e3;
	static const u8 ETH_BROADCAST[];

	static constexpr u32 INIT_ADDR_MASK = 0xfffffe;
	static constexpr u32 RING_ADDR_MASK = 0xfffff8;
	static constexpr auto TX_POLL_PERIOD = attotime::from_usec(1600);

	enum csr0_mask : u16
	{
		CSR0_INIT = 0x0001, // initialize
		CSR0_STRT = 0x0002, // start
		CSR0_STOP = 0x0004, // stop
		CSR0_TDMD = 0x0008, // transmit demand
		CSR0_TXON = 0x0010, // transmitter on
		CSR0_RXON = 0x0020, // receiver on
		CSR0_INEA = 0x0040, // interrupt enable
		CSR0_INTR = 0x0080, // interrupt flag
		CSR0_IDON = 0x0100, // initialization done
		CSR0_TINT = 0x0200, // transmitter interrupt
		CSR0_RINT = 0x0400, // receiver interrupt
		CSR0_MERR = 0x0800, // memory error
		CSR0_MISS = 0x1000, // missed packet
		CSR0_CERR = 0x2000, // collision error
		CSR0_BABL = 0x4000, // babble
		CSR0_ERR  = 0x8000, // error

		CSR0_ANY_INTR = CSR0_BABL | CSR0_MISS | CSR0_MERR | CSR0_RINT | CSR0_TINT | CSR0_IDON,
		CSR0_ANY_ERR  = CSR0_BABL | CSR0_CERR | CSR0_MISS | CSR0_MERR,
	};

	enum csr3_mask : u16
	{
		CSR3_BCON = 0x0001, // byte control
		CSR3_ACON = 0x0002, // ale control
		CSR3_BSWP = 0x0004, // byte swap

		CSR3_MASK = 0x0007
	};

	enum mode_mask : u16
	{
		MODE_DRX  = 0x0001, // disable the receiver
		MODE_DTX  = 0x0002, // disable the transmitter
		MODE_LOOP = 0x0004, // loopback
		MODE_DTCR = 0x0008, // disable transmit crc
		MODE_COLL = 0x0010, // force collision
		MODE_DRTY = 0x0020, // disable retry
		MODE_INTL = 0x0040, // internal loopback
		MODE_EMBA = 0x0080, // enable modified back-off algorithm (C-LANCE only)
		MODE_PROM = 0x8000, // promiscuous
	};

	enum tmd1_mask : u16
	{
		TMD1_HADR    = 0x00ff, // high order address
		TMD1_ENP     = 0x0100, // end of packet
		TMD1_STP     = 0x0200, // start of packet
		TMD1_DEF     = 0x0400, // deferred
		TMD1_ONE     = 0x0800, // one retry
		TMD1_MORE    = 0x1000, // more than one retry
		TMD1_ADD_FCS = 0x2000, // append fcs (C-LANCE only)
		TMD1_ERR     = 0x4000, // error
		TMD1_OWN     = 0x8000, // owned by lance
	};

	enum tmd3_mask : u16
	{
		TMD3_TDR  = 0x03ff, // time domain reflectometry
		TMD3_RTRY = 0x0400, // retry error
		TMD3_LCAR = 0x0800, // loss of carrier
		TMD3_LCOL = 0x1000, // late collision
		TMD3_RES  = 0x2000,
		TMD3_UFLO = 0x4000, // underflow error
		TMD3_BUFF = 0x8000, // buffer error
	};

	enum rmd1_mask : u16
	{
		RMD1_HADR = 0x00ff, // high order address
		RMD1_ENP  = 0x0100, // end of packet
		RMD1_STP  = 0x0200, // start of packet
		RMD1_BUFF = 0x0400, // buffer error
		RMD1_CRC  = 0x0800, // crc error
		RMD1_OFLO = 0x1000, // overflow error
		RMD1_FRAM = 0x2000, // framing error
		RMD1_ERR  = 0x4000, // error
		RMD1_OWN  = 0x8000, // owned by lance
	};

	enum rmd3_mask : u16
	{
		RMD3_MCNT = 0x0fff, // message byte count
	};

private:
	devcb_write_line m_intr_out_cb;
	devcb_read16 m_dma_in_cb;
	devcb_write16 m_dma_out_cb;

	u16 m_rap;
	u16 m_csr[4];

	u16 m_mode;
	u64 m_logical_addr_filter;
	u8 m_physical_addr[6];

	u32 m_rx_ring_base;
	u8 m_rx_ring_mask;
	u8 m_rx_ring_pos;
	u16 m_rx_md[4];

	u32 m_tx_ring_base;
	u8 m_tx_ring_mask;
	u8 m_tx_ring_pos;
	u16 m_tx_md[4];

	emu_timer *m_interrupt;
	emu_timer *m_transmit_poll;
	int m_intr_out_state;
	bool m_idon;

	// internal loopback buffer
	u8 m_lb_buf[36];
	int m_lb_length;
};

class am7990_device : public am7990_device_base
{
public:
	am7990_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual int get_buf_length(u16 data) const override { return (data == 0xf000) ? 4096 : -s16(0xf000 | data); }
};

class am79c90_device : public am7990_device_base
{
public:
	am79c90_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual int get_buf_length(u16 data) const override { return data ? ((data == 0xf000) ? 4096 : -s16(0xf000 | data)) : 0; }
};

DECLARE_DEVICE_TYPE(AM7990, am7990_device)
DECLARE_DEVICE_TYPE(AM79C90, am79c90_device)

#endif // MAME_MACHINE_AM79C90_H
