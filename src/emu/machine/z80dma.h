/***************************************************************************

    Zilog Z80 DMA Direct Memory Access Controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#define MCFG_Z80DMA_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, Z80DMA, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define Z80DMA_INTERFACE(_name) \
	const z80dma_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80dma_interface

struct z80dma_interface
{
	devcb_write_line	m_out_busreq_cb;
	devcb_write_line	m_out_int_cb;
	devcb_write_line	m_out_bao_cb;

	// memory accessors
	devcb_read8			m_in_mreq_cb;
	devcb_write8		m_out_mreq_cb;

	// I/O accessors
	devcb_read8			m_in_iorq_cb;
	devcb_write8		m_out_iorq_cb;
};



// ======================> z80dma_device

class z80dma_device :	public device_t,
						public device_z80daisy_interface,
						public z80dma_interface
{
public:
	// construction/destruction
	z80dma_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	UINT8 read();
	void write(UINT8 data);

	void rdy_w(int state);
	void wait_w(int state);
	void bai_w(int state);

private:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	// internal helpers
	int is_ready();
	void interrupt_check();
	void trigger_interrupt(int level);
	void do_read();
	int do_write();
	void do_transfer_write();
	void do_search();

	static TIMER_CALLBACK( static_timerproc ) { reinterpret_cast<z80dma_device *>(ptr)->timerproc(); }
	void timerproc();

	void update_status();

	static TIMER_CALLBACK( static_rdy_write_callback ) { reinterpret_cast<z80dma_device *>(ptr)->rdy_write_callback(param); }
	void rdy_write_callback(int state);

	// internal state
	devcb_resolved_write_line	m_out_busreq_func;
	devcb_resolved_write_line	m_out_int_func;
	devcb_resolved_write_line	m_out_bao_func;
	devcb_resolved_read8		m_in_mreq_func;
	devcb_resolved_write8		m_out_mreq_func;
	devcb_resolved_read8		m_in_iorq_func;
	devcb_resolved_write8		m_out_iorq_func;

	emu_timer *m_timer;

	UINT16	m_regs[(6<<3)+1+1];
	UINT8	m_num_follow;
	UINT8	m_cur_follow;
	UINT8	m_regs_follow[4];
	UINT8	m_read_num_follow;
	UINT8	m_read_cur_follow;
	UINT8	m_read_regs_follow[7];
	UINT8	m_status;
	UINT8	m_dma_enabled;

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
	int m_ip;					// interrupt pending
	int m_ius;					// interrupt under service
	UINT8 m_vector;				// interrupt vector
};


// device type definition
extern const device_type Z80DMA;



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// register access
DECLARE_READ8_DEVICE_HANDLER( z80dma_r );
DECLARE_WRITE8_DEVICE_HANDLER( z80dma_w );

// ready
WRITE_LINE_DEVICE_HANDLER( z80dma_rdy_w );

// wait
WRITE_LINE_DEVICE_HANDLER( z80dma_wait_w );

// bus acknowledge in
WRITE_LINE_DEVICE_HANDLER( z80dma_bai_w );

#endif
