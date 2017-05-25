// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Z80 CTC (Z8430) implementation

****************************************************************************
                            _____   _____
                    D4   1 |*    \_/     | 28  D3
                    D5   2 |             | 27  D2
                    D6   3 |             | 26  D1
                    D7   4 |             | 25  D0
                   GND   5 |             | 24  +5V
                   _RD   6 |             | 23  CLK/TRG0
                ZC/TOO   7 |   Z80-CTC   | 22  CLK/TRG1
                ZC/TO1   8 |             | 21  CLK/TRG2
                ZC/TO2   9 |             | 20  CLK/TRG3
                 _IORQ  10 |             | 19  CS1
                   IEO  11 |             | 18  CS0
                  _INT  12 |             | 17  _RESET
                   IEI  13 |             | 16  _CE
                   _M1  14 |_____________| 15  CLK

***************************************************************************/

#ifndef MAME_MACHINE_Z80CTC_H
#define MAME_MACHINE_Z80CTC_H

#pragma once

#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_Z80CTC_INTR_CB(_devcb) \
	devcb = &z80ctc_device::set_intr_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80CTC_ZC0_CB(_devcb) \
	devcb = &z80ctc_device::set_zc0_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80CTC_ZC1_CB(_devcb) \
	devcb = &z80ctc_device::set_zc1_callback(*device, DEVCB_##_devcb);

#define MCFG_Z80CTC_ZC2_CB(_devcb) \
	devcb = &z80ctc_device::set_zc2_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80ctc_device

class z80ctc_device :   public device_t,
						public device_z80daisy_interface
{
public:
	// construction/destruction
	z80ctc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_intr_callback(device_t &device, Object &&cb) { return downcast<z80ctc_device &>(device).m_intr_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_zc0_callback(device_t &device, Object &&cb) { return downcast<z80ctc_device &>(device).m_zc0_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_zc1_callback(device_t &device, Object &&cb) { return downcast<z80ctc_device &>(device).m_zc1_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_zc2_callback(device_t &device, Object &&cb) { return downcast<z80ctc_device &>(device).m_zc2_cb.set_callback(std::forward<Object>(cb)); }

	// read/write handlers
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( trg0 );
	DECLARE_WRITE_LINE_MEMBER( trg1 );
	DECLARE_WRITE_LINE_MEMBER( trg2 );
	DECLARE_WRITE_LINE_MEMBER( trg3 );

	uint16_t get_channel_constant(uint8_t channel) { return m_channel[channel].m_tconst; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	// internal helpers
	void interrupt_check();

	// a single channel within the CTC
	class ctc_channel
	{
	public:
		ctc_channel();

		void start(z80ctc_device *device, int index);
		void reset();

		uint8_t read();
		void write(uint8_t data);

		attotime period() const;
		void trigger(uint8_t data);
		TIMER_CALLBACK_MEMBER(timer_callback);

		z80ctc_device * m_device;               // pointer back to our device
		int             m_index;                // our channel index
		uint16_t          m_mode;                 // current mode
		uint16_t          m_tconst;               // time constant
		uint16_t          m_down;                 // down counter (clock mode only)
		uint8_t           m_extclk;               // current signal from the external clock
		emu_timer *     m_timer;                // array of active timers
		uint8_t           m_int_state;            // interrupt status (for daisy chain)
	};

	// internal state
	devcb_write_line   m_intr_cb;              // interrupt callback
	devcb_write_line   m_zc0_cb;               // channel 0 zero crossing callbacks
	devcb_write_line   m_zc1_cb;               // channel 1 zero crossing callbacks
	devcb_write_line   m_zc2_cb;               // channel 2 zero crossing callbacks
	devcb_write_line   m_zc3_cb;               // channel 3 zero crossing callbacks = nullptr ?

	uint8_t               m_vector;               // interrupt vector
	attotime            m_period16;             // 16/system clock
	attotime            m_period256;            // 256/system clock
	ctc_channel         m_channel[4];           // data for each channel
};


// device type definition
DECLARE_DEVICE_TYPE(Z80CTC, z80ctc_device)


#endif // MAME_MACHINE_Z80CTC_H
