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

#ifndef __Z80CTC_H__
#define __Z80CTC_H__

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
	z80ctc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_intr_callback(device_t &device, _Object object) { return downcast<z80ctc_device &>(device).m_intr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc0_callback(device_t &device, _Object object) { return downcast<z80ctc_device &>(device).m_zc0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc1_callback(device_t &device, _Object object) { return downcast<z80ctc_device &>(device).m_zc1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_zc2_callback(device_t &device, _Object object) { return downcast<z80ctc_device &>(device).m_zc2_cb.set_callback(object); }

	// read/write handlers
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( trg0 );
	DECLARE_WRITE_LINE_MEMBER( trg1 );
	DECLARE_WRITE_LINE_MEMBER( trg2 );
	DECLARE_WRITE_LINE_MEMBER( trg3 );

	UINT16 get_channel_constant(UINT8 channel) { return m_channel[channel].m_tconst; }

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

		UINT8 read();
		void write(UINT8 data);

		attotime period() const;
		void trigger(UINT8 data);
		void timer_callback();

		z80ctc_device * m_device;               // pointer back to our device
		int             m_index;                // our channel index
		UINT16          m_mode;                 // current mode
		UINT16          m_tconst;               // time constant
		UINT16          m_down;                 // down counter (clock mode only)
		UINT8           m_extclk;               // current signal from the external clock
		emu_timer *     m_timer;                // array of active timers
		UINT8           m_int_state;            // interrupt status (for daisy chain)

	private:
		static TIMER_CALLBACK( static_timer_callback ) { reinterpret_cast<z80ctc_device::ctc_channel *>(ptr)->timer_callback(); }
	};

	// internal state
	devcb_write_line   m_intr_cb;              // interrupt callback
	devcb_write_line   m_zc0_cb;               // channel 0 zero crossing callbacks
	devcb_write_line   m_zc1_cb;               // channel 1 zero crossing callbacks
	devcb_write_line   m_zc2_cb;               // channel 2 zero crossing callbacks
	devcb_write_line   m_zc3_cb;               // channel 3 zero crossing callbacks = NULL ?

	UINT8               m_vector;               // interrupt vector
	attotime            m_period16;             // 16/system clock
	attotime            m_period256;            // 256/system clock
	ctc_channel         m_channel[4];           // data for each channel
};


// device type definition
extern const device_type Z80CTC;


#endif
