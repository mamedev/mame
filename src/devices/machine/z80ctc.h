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

#include "machine/z80daisy.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration
class z80ctc_device;

// ======================> z80ctc_channel_device

// a single channel within the CTC
class z80ctc_channel_device : public device_t
{
	friend class z80ctc_device;

public:
	// construction/destruction
	z80ctc_channel_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 read();
	void write(u8 data);

	attotime period() const;
	void trigger(bool state);
	TIMER_CALLBACK_MEMBER(timer_callback);
	TIMER_CALLBACK_MEMBER(zc_to_callback);

	required_device<z80ctc_device> m_device; // pointer back to our device
	int             m_index;                // our channel index
	u16             m_mode;                 // current mode
	u16             m_tconst;               // time constant
	u16             m_down;                 // down counter (clock mode only)
	bool            m_extclk;               // current signal from the external clock
	emu_timer *     m_timer;                // array of active timers
	u8              m_int_state;            // interrupt status (for daisy chain)
	emu_timer *     m_zc_to_timer;          // zc to pulse timer
};

// ======================> z80ctc_device

class z80ctc_device :   public device_t,
						public device_z80daisy_interface
{
	friend class z80ctc_channel_device;

public:
	// construction/destruction
	z80ctc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto intr_callback() { return m_intr_cb.bind(); }
	template <int Channel> auto zc_callback() { return m_zc_cb[Channel].bind(); } // m_zc_cb[3] not supported on a standard ctc, only used for the tmpz84c015
	template <int Channel> void set_clk(u32 clock) { channel_config(Channel).set_clock(clock); }
	template <int Channel> void set_clk(const XTAL &xtal) { channel_config(Channel).set_clock(xtal); }

	// read/write handlers
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void trg0(int state);
	void trg1(int state);
	void trg2(int state);
	void trg3(int state);

	u16 get_channel_constant(int ch) const { return m_channel[ch]->m_tconst; }

protected:
	z80ctc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	// z80daisy_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

	// internal helpers
	u8 channel_int_state(int ch) const noexcept { return m_channel[ch]->m_int_state; }
	void interrupt_check();

	z80ctc_channel_device &channel_config(int ch) { return *m_channel[ch].lookup(); }

	// internal state
	required_device_array<z80ctc_channel_device, 4> m_channel;  // subdevice for each channel
	devcb_write_line                                m_intr_cb;  // interrupt callback
	devcb_write_line::array<4>                      m_zc_cb;    // zero crossing/timer output callbacks

	u8                                              m_vector;   // interrupt vector
};


// device type definitions
DECLARE_DEVICE_TYPE(Z80CTC, z80ctc_device)
DECLARE_DEVICE_TYPE(Z80CTC_CHANNEL, z80ctc_channel_device)

#endif // MAME_MACHINE_Z80CTC_H
