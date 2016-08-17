// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

  RIOT 6532 emulation

***************************************************************************/

#pragma once

#ifndef __RIOT6532_H__
#define __RIOT6532_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RIOT6532_IN_PA_CB(_devcb) \
	devcb = &riot6532_device::set_in_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_RIOT6532_OUT_PA_CB(_devcb) \
	devcb = &riot6532_device::set_out_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_RIOT6532_IN_PB_CB(_devcb) \
	devcb = &riot6532_device::set_in_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_RIOT6532_OUT_PB_CB(_devcb) \
	devcb = &riot6532_device::set_out_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_RIOT6532_IRQ_CB(_devcb) \
	devcb = &riot6532_device::set_irq_callback(*device, DEVCB_##_devcb);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> riot6532_device

class riot6532_device :  public device_t
{
public:
	// construction/destruction
	riot6532_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_in_pa_callback(device_t &device, _Object object) { return downcast<riot6532_device &>(device).m_in_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pa_callback(device_t &device, _Object object) { return downcast<riot6532_device &>(device).m_out_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pb_callback(device_t &device, _Object object) { return downcast<riot6532_device &>(device).m_in_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pb_callback(device_t &device, _Object object) { return downcast<riot6532_device &>(device).m_out_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<riot6532_device &>(device).m_irq_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 reg_r(UINT8 offset, bool debugger_access = false);
	void reg_w(UINT8 offset, UINT8 data);

	void porta_in_set(UINT8 data, UINT8 mask);
	void portb_in_set(UINT8 data, UINT8 mask);

	UINT8 porta_in_get();
	UINT8 portb_in_get();

	UINT8 porta_out_get();
	UINT8 portb_out_get();

	void timer_end();

protected:
	class riot6532_port
	{
	public:
		UINT8                   m_in;
		UINT8                   m_out;
		UINT8                   m_ddr;
		devcb_read8             *m_in_cb;
		devcb_write8            *m_out_cb;
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }


private:
	void update_irqstate();
	UINT8 apply_ddr(const riot6532_port *port);
	void update_pa7_state();
	UINT8 get_timer();

	riot6532_port   m_port[2];

	devcb_read8         m_in_pa_cb;
	devcb_write8        m_out_pa_cb;
	devcb_read8         m_in_pb_cb;
	devcb_write8        m_out_pb_cb;
	devcb_write_line    m_irq_cb;

	UINT8           m_irqstate;
	UINT8           m_irqenable;
	int             m_irq;

	UINT8           m_pa7dir;     /* 0x80 = high-to-low, 0x00 = low-to-high */
	UINT8           m_pa7prev;

	UINT8           m_timershift;
	UINT8           m_timerstate;
	emu_timer *     m_timer;

	enum
	{
		TIMER_END_CB
	};
};


// device type definition
extern const device_type RIOT6532;

#endif
