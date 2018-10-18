// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    AMD Am79C90 CMOS Local Area Network Controller for Ethernet (C-LANCE)

    TODO:
        - Communication with the outside world
        - Error handling
        - Clocks

******************************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 48  Vdd
                  DAL7   2 |             | 47  DAL8
                  DAL6   3 |             | 46  DAL9
                  DAL5   4 |             | 45  DAL10
                  DAL4   5 |             | 44  DAL11
                  DAL3   6 |             | 43  DAL12
                  DAL2   7 |             | 42  DAL13
                  DAL1   8 |             | 41  DAL14
                  DAL0   9 |             | 40  DAL15
                  READ  10 |             | 39  A16
                 /INTR  11 |             | 38  A17
                 /DALI  12 |   Am79C90   | 37  A18
                 /DALI  13 |             | 36  A19
                  /DAS  14 |             | 35  A20
             /BM0,BYTE  15 |             | 34  A21
          /BM1,/BUSAKO  16 |             | 33  A22
          /HOLD,/BUSRQ  17 |             | 32  A23
               ALE,/AS  18 |             | 31  RX
                 /HLDA  19 |             | 30  RENA
                   /CS  20 |             | 29  TX
                   ADR  21 |             | 28  CLSN
                /READY  22 |             | 27  RCLK
                /RESET  23 |             | 26  TENA
                   Vss  24 |_____________| 25  TCLK

**********************************************************************/

#ifndef MAME_MACHINE_AM79C90_H
#define MAME_MACHINE_AM79C90_H

#pragma once

#include "hashing.h"

class am7990_device_base : public device_t
{
public:
	auto dma_out() { return m_dma_out_cb.bind(); }
	auto dma_in() { return m_dma_in_cb.bind(); }
	auto irq_out() { return m_irq_out_cb.bind(); }

	DECLARE_READ16_MEMBER(regs_r);
	DECLARE_WRITE16_MEMBER(regs_w);

protected:
	am7990_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

private:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_TRANSMIT_POLL = 0;
	static const device_timer_id TIMER_TRANSMIT = 1;
	//static const device_timer_id TIMER_RECEIVE_POLL = 2;
	static const device_timer_id TIMER_RECEIVE = 3;

	enum
	{
		CSR0_ERR      = 0x8000,
		CSR0_BABL     = 0x4000,
		CSR0_CERR     = 0x2000,
		CSR0_MISS     = 0x1000,
		CSR0_MERR     = 0x0800,
		CSR0_RINT     = 0x0400,
		CSR0_TINT     = 0x0200,
		CSR0_IDON     = 0x0100,
		CSR0_INTR     = 0x0080,
		CSR0_INEA     = 0x0040,
		CSR0_RXON     = 0x0020,
		CSR0_TXON     = 0x0010,
		CSR0_TDMD     = 0x0008,
		CSR0_STOP     = 0x0004,
		CSR0_STRT     = 0x0002,
		CSR0_INIT     = 0x0001,
		CSR0_ANY_INTR = CSR0_BABL | CSR0_MISS | CSR0_MERR | CSR0_RINT | CSR0_TINT | CSR0_IDON,
		CSR0_ANY_ERR  = CSR0_BABL | CSR0_CERR | CSR0_MISS | CSR0_MERR,

		CSR3_BSWP = 0x0004,
		CSR3_ACON = 0x0002,
		CSR3_BCON = 0x0001,

		MODE_DRX  = 0x0001,
		MODE_DTX  = 0x0002,
		MODE_LOOP = 0x0004,
		MODE_DTCR = 0x0008,
		MODE_COLL = 0x0010,
		MODE_DRTY = 0x0020,
		MODE_INTL = 0x0040,
		MODE_EMBA = 0x0080,
		MODE_PROM = 0x8000,

		TMD1_ENP     = 0x0100,
		TMD1_STP     = 0x0200,
		TMD1_DEF     = 0x0400,
		TMD1_ONE     = 0x0800,
		TMD1_MORE    = 0x1000,
		TMD1_ADD_FCS = 0x2000,
		TMD1_ERR     = 0x4000,
		TMD1_OWN     = 0x8000,

		TMD3_RTRY    = 0x0400,
		TMD3_LCAR    = 0x0800,
		TMD3_LCOL    = 0x1000,
		TMD3_RES     = 0x2000,
		TMD3_UFLO    = 0x4000,
		TMD3_BUFF    = 0x8000,

		RMD1_ENP     = 0x0100,
		RMD1_STP     = 0x0200,
		RMD1_BUFF    = 0x0400,
		RMD1_CRC     = 0x0800,
		RMD1_OFLO    = 0x1000,
		RMD1_FRAM    = 0x2000,
		RMD1_ERR     = 0x4000,
		RMD1_OWN     = 0x8000
	};

	void prepare_transmit_buf();
	void begin_receiving();
	void recv_fifo_push(uint32_t value);
	void fetch_receive_descriptor();
	void fetch_transmit_descriptor();
	void poll_transmit();
	//void poll_receive(); TODO
	void transmit();
	void receive();
	void update_interrupts();

	struct ring_descriptor
	{
		union
		{
			uint32_t m_tmd23;
			uint32_t m_rmd23;
		};
		union
		{
			uint32_t m_tmd01;
			uint32_t m_rmd01;
		};
		uint16_t m_byte_count;
		uint32_t m_buffer_addr;
	};

	ring_descriptor m_curr_transmit_desc;
	ring_descriptor m_next_transmit_desc;
	ring_descriptor m_curr_recv_desc;
	ring_descriptor m_next_recv_desc;

	uint16_t m_rap;
	uint16_t m_csr[4];
	uint16_t m_mode;
	uint64_t m_logical_addr_filter;
	uint64_t m_physical_addr;
	uint32_t m_recv_message_count;
	uint32_t m_recv_ring_addr;
	uint32_t m_recv_buf_addr;
	uint16_t m_recv_buf_count;
	uint8_t m_recv_ring_length;
	uint8_t m_recv_ring_pos;
	uint32_t m_recv_fifo[16];
	uint8_t m_recv_fifo_write;
	uint8_t m_recv_fifo_read;
	bool m_receiving;
	emu_timer *m_receive_timer;
	//emu_timer *m_receive_poll_timer;

	uint32_t m_transmit_ring_addr;
	uint32_t m_transmit_buf_addr;
	uint16_t m_transmit_buf_count;
	uint8_t m_transmit_ring_length;
	uint8_t m_transmit_ring_pos;
	uint32_t m_transmit_fifo[12];
	uint8_t m_transmit_fifo_write;
	uint8_t m_transmit_fifo_read;
	bool m_transmitting;
	emu_timer *m_transmit_timer;
	emu_timer *m_transmit_poll_timer;

	devcb_write_line m_irq_out_cb;
	devcb_write32 m_dma_out_cb; // TODO: Should be read/write16!
	devcb_read32 m_dma_in_cb;

	util::crc32_creator m_crc32;
};

class am7990_device : public am7990_device_base
{
public:
	am7990_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class am79c90_device : public am7990_device_base
{
public:
	am79c90_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(AM7990, am7990_device)
DECLARE_DEVICE_TYPE(AM79C90, am79c90_device)

#endif // MAME_MACHINE_AM79C90_H
