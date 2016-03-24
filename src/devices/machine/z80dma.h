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

#ifndef __Z80DMA__
#define __Z80DMA__

#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80DMA_OUT_BUSREQ_CB(_devcb) \
	devcb = &z80dma_device::set_out_busreq_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DMA_OUT_INT_CB(_devcb) \
	devcb = &z80dma_device::set_out_int_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DMA_OUT_BAO_CB(_devcb) \
	devcb = &z80dma_device::set_out_bao_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DMA_IN_MREQ_CB(_devcb) \
	devcb = &z80dma_device::set_in_mreq_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DMA_OUT_MREQ_CB(_devcb) \
	devcb = &z80dma_device::set_out_mreq_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DMA_IN_IORQ_CB(_devcb) \
	devcb = &z80dma_device::set_in_iorq_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80DMA_OUT_IORQ_CB(_devcb) \
	devcb = &z80dma_device::set_out_iorq_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80dma_device

class z80dma_device :   public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	z80dma_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_busreq_callback(device_t &device, _Object object) { return downcast<z80dma_device &>(device).m_out_busreq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_int_callback(device_t &device, _Object object) { return downcast<z80dma_device &>(device).m_out_int_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_bao_callback(device_t &device, _Object object) { return downcast<z80dma_device &>(device).m_out_bao_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_mreq_callback(device_t &device, _Object object) { return downcast<z80dma_device &>(device).m_in_mreq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_mreq_callback(device_t &device, _Object object) { return downcast<z80dma_device &>(device).m_out_mreq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_iorq_callback(device_t &device, _Object object) { return downcast<z80dma_device &>(device).m_in_iorq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iorq_callback(device_t &device, _Object object) { return downcast<z80dma_device &>(device).m_out_iorq_cb.set_callback(object); }

	UINT8 read();
	void write(UINT8 data);
	DECLARE_READ8_MEMBER( read ) { return read(); }
	DECLARE_WRITE8_MEMBER( write ) { write(data); }

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
	devcb_write_line   m_out_bao_cb;
	devcb_read8        m_in_mreq_cb;
	devcb_write8       m_out_mreq_cb;
	devcb_read8        m_in_iorq_cb;
	devcb_write8       m_out_iorq_cb;

	emu_timer *m_timer;

	UINT16  m_regs[(6<<3)+1+1];
	UINT8   m_num_follow;
	UINT8   m_cur_follow;
	UINT8   m_regs_follow[5];
	UINT8   m_read_num_follow;
	UINT8   m_read_cur_follow;
	UINT8   m_read_regs_follow[7];
	UINT8   m_status;
	UINT8   m_dma_enabled;

	UINT16 m_addressA;
	UINT16 m_addressB;
	UINT16 m_count;

	int m_rdy;
	int m_force_ready;
	UINT8 m_reset_pointer;

	bool m_is_read;
	UINT8 m_cur_cycle;
	UINT8 m_latch;

	// interrupts
	int m_ip;                   // interrupt pending
	int m_ius;                  // interrupt under service
	UINT8 m_vector;             // interrupt vector
};


// device type definition
extern const device_type Z80DMA;

#endif
