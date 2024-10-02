// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Adaptec AIC-565 Bus Auxiliary Interface Chip

**********************************************************************/

#ifndef MAME_MACHINE_AIC565_H
#define MAME_MACHINE_AIC565_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aic565_device

class aic565_device : public device_t
{
public:
	// construction/destruction
	aic565_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback configuration
	auto hrst_callback() { return m_hrst_callback.bind(); }
	auto srst_callback() { return m_srst_callback.bind(); }
	auto irq_callback() { return m_irq_callback.bind(); }

	// host register access
	u8 host_r(offs_t offset);
	void host_w(offs_t offset, u8 data);

	// local CPU access
	u8 local_r(offs_t offset);
	void local_w(offs_t offset, u8 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// synchronization helpers
	TIMER_CALLBACK_MEMBER(host_sync_w);
	TIMER_CALLBACK_MEMBER(local_sync_w);

	// callback objects
	devcb_write_line m_hrst_callback;
	devcb_write_line m_srst_callback;
	devcb_write_line m_irq_callback;

	// internal state
	uint8_t m_data_to_host;
	uint8_t m_data_from_host;
	uint8_t m_local_status;
	uint8_t m_aux_status;
	uint8_t m_interrupt_flags;
};

// device type declaration
DECLARE_DEVICE_TYPE(AIC565, aic565_device)

#endif // MAME_MACHINE_AIC565_H
