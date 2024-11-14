// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
/***************************************************************************

 Motorola 6844 emulation

****************************************************************************
                            _____   _____
                   VSS   1 |*    \_/     | 40  E
         *CS / Tx AKBW   2 |             | 39 *RESET
                R / *W   3 |             | 38  DGRNT
                    A0   4 |             | 37 *DRQ1
                    A1   5 |             | 36 *DRQ2
                    A2   6 |             | 35  Tx AKA
                    A3   7 |             | 34 *TX STB
                    A4   8 |             | 33 *IRQ / *DEND
                    A5   9 |             | 32  Tx RQ0
                    A6  10 |   MC6844    | 31  Tx RQ1
                    A7  11 |             | 30  Tx RQ2
                    A8  12 |             | 29  Tx RQ3
                    A9  13 |             | 28  D0
                   A10  14 |             | 27  D1
                   A11  15 |             | 26  D2
                   A12  16 |             | 25  D3
                   A13  17 |             | 24  D4
                   A14  18 |             | 23  D5
                   A15  19 |             | 22  D6
                   VCC  20 |_____________| 21  D7

***************************************************************************/

#ifndef MAME_MACHINE_MC6844_H
#define MAME_MACHINE_MC6844_H

#pragma once

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mc6844_device

class mc6844_device : public device_t, public device_execute_interface
{
public:
	// construction/destruction
	mc6844_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_int_callback() { return m_out_int_cb.bind(); }
	auto out_txak_callback() { return m_out_txak_cb.bind(); }
	auto out_drq1_callback() { return m_out_drq1_cb.bind(); }
	auto out_drq2_callback() { return m_out_drq2_cb.bind(); }
	auto in_memr_callback() { return m_in_memr_cb.bind(); }
	auto out_memw_callback() { return m_out_memw_cb.bind(); }

	// I/O operations
	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);
	template <unsigned CH> auto in_ior_callback() { return m_in_ior_cb[CH].bind(); }
	template <unsigned CH> auto out_iow_callback() { return m_out_iow_cb[CH].bind(); }

	template <unsigned CH> void dreq_w(int state) { dma_request(CH, state); }

	void dgrnt_w(int state) { m_dgrnt = state; trigger(1); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void execute_run() override;

	devcb_write_line    m_out_int_cb;
	devcb_write8        m_out_txak_cb;
	devcb_write_line    m_out_drq1_cb;
	devcb_write_line    m_out_drq2_cb;
	devcb_read8 m_in_memr_cb;
	devcb_write8 m_out_memw_cb;
	devcb_read8::array<4> m_in_ior_cb;
	devcb_write8::array<4> m_out_iow_cb;

	/* channel_data structure holds info about each 6844 DMA channel */
	struct m6844_channel_data
	{
		int active;
		int address;
		int counter;
		// Channel control register.
		//  bit 0: Read / Write mode
		//  bit 1: Mode control B
		//  bit 2: Mode control A
		//  bit 3: Address up (0) / down (1).
		//  bit 4: Not used
		//  bit 5: Not used
		//  bit 6: Busy / Ready. Read only. Set when request
		//         made. Cleared when transfer completed.
		//  bit 7: DMA end flag. Read only? Set when transfer
		//         completed. Cleared when control register
		//          read. Sets IRQ.
		// Mode control A,B: 0,0 Mode2; 0,1 Mode 3; 1,0 Mode 0;
		//                   1,1 Undefined.
		uint8_t control;
		int start_address;
		int start_counter;
	};

	/* 6844 description */
	m6844_channel_data m_m6844_channel[4];
	uint8_t m_m6844_priority;
	// Interrupt control register.
	// Bit 0-3: channel interrupt enable, 1 enabled, 0 masked.
	// Bit 4-6: unused
	// Bit 7: Read only. Set to 1 when IRQ asserted. Clear when the
	// control register associated with the channel that caused the
	// interrut is read.
	uint8_t m_m6844_interrupt;
	uint8_t m_m6844_chain;
	void m6844_update_interrupt();

	// State machine
	enum {
	  STATE_SI,
	  STATE_S0,
	  STATE_S1,
	  STATE_S2
	};

	int m_state;
	int m_icount;
	int m_current_channel;
	int m_last_channel;

	// input states
	bool m_dgrnt;
	bool m_dreq[4];
private:
	void dma_request(int channel, int state);
};

// device type definition
DECLARE_DEVICE_TYPE(MC6844, mc6844_device)

#endif // MAME_MACHINE_MC6844_H
