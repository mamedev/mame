// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    PC-9801 Keyboard simulation

***************************************************************************/

#pragma once

#ifndef __PC9801_KBDDEV_H__
#define __PC9801_KBDDEV_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PC9801_KBD_IRQ_CALLBACK(_write) \
	devcb = &pc9801_kbd_device::set_irq_wr_callback(*device, DEVCB_##_write);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_kbd_device

class pc9801_kbd_device : public device_t
{
public:
	// construction/destruction
	pc9801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<pc9801_kbd_device &>(device).m_write_irq.set_callback(object); }

	virtual ioport_constructor device_input_ports() const override;

	// I/O operations
	void tx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void key_stroke(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	devcb_write_line   m_write_irq;

	static const device_timer_id RX_TIMER = 1;
	emu_timer *         m_rxtimer;
	uint8_t               m_rx_buf[0x80];
	uint8_t               m_keyb_tx;
	uint8_t               m_keyb_rx;
	bool                m_key_avail;
};


// device type definition
extern const device_type PC9801_KBD;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
