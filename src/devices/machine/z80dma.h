// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

    Zilog Z80 DMA Direct Memory Access Controller emulation

****************************************************************************
                            _____   _____
                    A5   1 |*    \_/     | 40  A6
                    A4   2 |             | 39  A7
                    A3   3 |             | 38  IEI
                    A2   4 |             | 37  _INT/_PULSE
                    A1   5 |             | 36  IEO
                    A0   6 |             | 35  D0
                   CLK   7 |             | 34  D1
                   _WR   8 |             | 33  D2
                   _RD   9 |             | 32  D3
                 _IORQ  10 |    Z8410    | 31  D4
                   +5V  11 |             | 30  GND
                 _MREQ  12 |             | 29  D5
                  _BAO  13 |             | 28  D6
                  _BAI  14 |             | 27  D7
               _BUSREQ  15 |             | 26  _M1
             _CE/_WAIT  16 |             | 25  RDY
                   A15  17 |             | 24  A8
                   A14  18 |             | 23  A9
                   A13  19 |             | 22  A10
                   A12  20 |_____________| 21  A11

***************************************************************************/

#ifndef MAME_MACHINE_Z80DMA_H
#define MAME_MACHINE_Z80DMA_H

#pragma once

#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80dma_device

class z80dma_device :   public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	z80dma_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_busreq_callback() { return m_out_busreq_cb.bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto out_ieo_callback() { return m_out_ieo_cb.bind(); }
	auto out_bao_callback() { return m_out_bao_cb.bind(); }
	auto in_mreq_callback() { return m_in_mreq_cb.bind(); }
	auto out_mreq_callback() { return m_out_mreq_cb.bind(); }
	auto in_iorq_callback() { return m_in_iorq_cb.bind(); }
	auto out_iorq_callback() { return m_out_iorq_cb.bind(); }

	uint8_t read();
	void write(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(iei_w) { m_iei = state; interrupt_check(); }
	DECLARE_WRITE_LINE_MEMBER(rdy_w);
	DECLARE_WRITE_LINE_MEMBER(wait_w);
	DECLARE_WRITE_LINE_MEMBER(bai_w);

private:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal helpers
	int is_ready();
	void interrupt_check();
	void trigger_interrupt(int level);
	void do_read();
	int do_write();
	void do_transfer_write();
	void do_search();

	TIMER_CALLBACK_MEMBER(timerproc);

	void update_status();

	TIMER_CALLBACK_MEMBER(rdy_write_callback);

	// internal state
	devcb_write_line   m_out_busreq_cb;
	devcb_write_line   m_out_int_cb;
	devcb_write_line   m_out_ieo_cb;
	devcb_write_line   m_out_bao_cb;
	devcb_read8        m_in_mreq_cb;
	devcb_write8       m_out_mreq_cb;
	devcb_read8        m_in_iorq_cb;
	devcb_write8       m_out_iorq_cb;

	emu_timer *m_timer;

	uint16_t  m_regs[(6<<3)+1+1];
	uint8_t   m_num_follow;
	uint8_t   m_cur_follow;
	uint8_t   m_regs_follow[5];
	uint8_t   m_read_num_follow;
	uint8_t   m_read_cur_follow;
	uint8_t   m_read_regs_follow[7];
	uint8_t   m_status;
	uint8_t   m_dma_enabled;

	uint16_t m_addressA;
	uint16_t m_addressB;
	uint16_t m_count;
	uint16_t m_byte_counter;

	int m_rdy;
	int m_force_ready;
	uint8_t m_reset_pointer;

	bool m_is_read;
	uint8_t m_cur_cycle;
	uint8_t m_latch;

	// interrupts
	int m_iei;					// interrupt enable input
	int m_ip;                   // interrupt pending
	int m_ius;                  // interrupt under service
	uint8_t m_vector;             // interrupt vector
};


// device type definition
DECLARE_DEVICE_TYPE(Z80DMA, z80dma_device)

#endif // MAME_MACHINE_Z80DMA_H
