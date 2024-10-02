// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    PC-9801 Keyboard simulation

***************************************************************************/
#ifndef MAME_NEC_PC9801_KBD_H
#define MAME_NEC_PC9801_KBD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_kbd_device

class pc9801_kbd_device : public device_t
{
public:
	// construction/destruction
	pc9801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_wr_callback() { return m_write_irq.bind(); }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// I/O operations
	void tx_w(uint8_t data);
	uint8_t rx_r(offs_t offset);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(rx_timer_tick);

	devcb_write_line    m_write_irq;

	emu_timer *         m_rxtimer;
	uint8_t             m_rx_buf[0x80];
	uint8_t             m_keyb_tx;
	uint8_t             m_keyb_rx;
	bool                m_key_avail;
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_KBD, pc9801_kbd_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif // MAME_NEC_PC9801_KBD_H
