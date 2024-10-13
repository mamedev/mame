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
	z80dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_busreq_callback() { return m_out_busreq_cb.bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto out_ieo_callback() { return m_out_ieo_cb.bind(); }
	auto out_bao_callback() { return m_out_bao_cb.bind(); }
	auto in_mreq_callback() { return m_in_mreq_cb.bind(); }
	auto out_mreq_callback() { return m_out_mreq_cb.bind(); }
	auto in_iorq_callback() { return m_in_iorq_cb.bind(); }
	auto out_iorq_callback() { return m_out_iorq_cb.bind(); }

	u8 read();
	virtual void write(u8 data);

	void iei_w(int state) { m_iei = state; interrupt_check(); }
	void rdy_w(int state);
	void wait_w(int state) { m_wait = state; }
	void bai_w(int state);

protected:
	static inline constexpr int COMMAND_RESET                         = 0xc3;
	static inline constexpr int COMMAND_RESET_PORT_A_TIMING           = 0xc7;
	static inline constexpr int COMMAND_RESET_PORT_B_TIMING           = 0xcb;
	static inline constexpr int COMMAND_LOAD                          = 0xcf;
	static inline constexpr int COMMAND_CONTINUE                      = 0xd3;
	static inline constexpr int COMMAND_DISABLE_INTERRUPTS            = 0xaf;
	static inline constexpr int COMMAND_ENABLE_INTERRUPTS             = 0xab;
	static inline constexpr int COMMAND_RESET_AND_DISABLE_INTERRUPTS  = 0xa3;
	static inline constexpr int COMMAND_ENABLE_AFTER_RETI             = 0xb7;
	static inline constexpr int COMMAND_READ_STATUS_BYTE              = 0xbf;
	static inline constexpr int COMMAND_REINITIALIZE_STATUS_BYTE      = 0x8b;
	static inline constexpr int COMMAND_INITIATE_READ_SEQUENCE        = 0xa7;
	static inline constexpr int COMMAND_FORCE_READY                   = 0xb3;
	static inline constexpr int COMMAND_ENABLE_DMA                    = 0x87;
	static inline constexpr int COMMAND_DISABLE_DMA                   = 0x83;
	static inline constexpr int COMMAND_READ_MASK_FOLLOWS             = 0xbb;

	static inline constexpr int TM_TRANSFER           = 0x01;
	static inline constexpr int TM_SEARCH             = 0x02;
	static inline constexpr int TM_SEARCH_TRANSFER    = 0x03;

	z80dma_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal helpers
	void enable();
	void disable();
	u8 num_follow() const noexcept { return m_num_follow; }
	virtual int is_ready();
	void interrupt_check();
	void trigger_interrupt(int level);
	void do_read();
	virtual void do_write();
	void do_transfer_write();
	void do_search();

	u16 &REG(unsigned m, unsigned s) noexcept { return m_regs[REGNUM(m, s)]; }

	static constexpr unsigned REGNUM(unsigned m, unsigned s) { return (m << 3) + s; }

	u16 m_addressA;
	u16 m_addressB;
	u16 m_count;
	u16 m_byte_counter;

private:
	// device_z80daisy_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	TIMER_CALLBACK_MEMBER(clock_w);
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

	u16  m_regs[(6 << 3) + 1 + 1];
	u8   m_num_follow;
	u8   m_cur_follow;
	u8   m_regs_follow[5];
	u8   m_read_num_follow;
	u8   m_read_cur_follow;
	u8   m_read_regs_follow[7];
	u8   m_status;
	int  m_dma_seq;

	int m_rdy;
	int m_force_ready;
	u8  m_reset_pointer;

	int  m_wait;
	int  m_busrq_ack;
	bool m_is_pulse;
	u8   m_latch;

	// interrupts
	int m_iei;                  // interrupt enable input
	int m_ip;                   // interrupt pending
	int m_ius;                  // interrupt under service
	u8  m_vector;               // interrupt vector
};


// device type definition
DECLARE_DEVICE_TYPE(Z80DMA, z80dma_device)

#endif // MAME_MACHINE_Z80DMA_H
