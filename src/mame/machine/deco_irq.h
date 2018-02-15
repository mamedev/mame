// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Data East IRQ Controller

***************************************************************************/

#ifndef MAME_MACHINE_DECO_IRQ_H
#define MAME_MACHINE_DECO_IRQ_H

#pragma once

#include "screen.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DECO_IRQ_ADD(_tag, _screen_tag) \
	MCFG_DEVICE_ADD(_tag, DECO_IRQ, 0) \
	deco_irq_device::set_screen_tag(*device, "^" _screen_tag);

#define MCFG_DECO_IRQ_LIGHTGUN1_CB(_devcb) \
	devcb = &deco_irq_device::set_lightgun1_callback(*device, DEVCB_##_devcb);

#define MCFG_DECO_IRQ_LIGHTGUN2_CB(_devcb) \
	devcb = &deco_irq_device::set_lightgun2_callback(*device, DEVCB_##_devcb);

#define MCFG_DECO_IRQ_LIGHTGUN_IRQ_CB(_devcb) \
	devcb = &deco_irq_device::set_lightgun_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_DECO_IRQ_RASTER1_IRQ_CB(_devcb) \
	devcb = &deco_irq_device::set_raster1_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_DECO_IRQ_RASTER2_IRQ_CB(_devcb) \
	devcb = &deco_irq_device::set_raster2_irq_callback(*device, DEVCB_##_devcb);

#define MCFG_DECO_IRQ_VBLANK_IRQ_CB(_devcb) \
	devcb = &deco_irq_device::set_vblank_irq_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class deco_irq_device : public device_t
{
public:
	// construction/destruction
	deco_irq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> static devcb_base &set_lightgun1_callback(device_t &device, Object &&cb)
	{ return downcast<deco_irq_device &>(device).m_lightgun1_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_lightgun2_callback(device_t &device, Object &&cb)
	{ return downcast<deco_irq_device &>(device).m_lightgun2_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_lightgun_irq_callback(device_t &device, Object &&cb)
	{ return downcast<deco_irq_device &>(device).m_lightgun_irq_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_raster1_irq_callback(device_t &device, Object &&cb)
	{ return downcast<deco_irq_device &>(device).m_raster1_irq_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_raster2_irq_callback(device_t &device, Object &&cb)
	{ return downcast<deco_irq_device &>(device).m_raster2_irq_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_vblank_irq_callback(device_t &device, Object &&cb)
	{ return downcast<deco_irq_device &>(device).m_vblank_irq_cb.set_callback(std::forward<Object>(cb)); }

	// static configuration
	static void set_screen_tag(device_t &device, const char *tag);

	TIMER_CALLBACK_MEMBER(scanline_callback);

	void map(address_map &map);

	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(scanline_r);
	DECLARE_WRITE8_MEMBER(scanline_w);
	DECLARE_READ8_MEMBER(raster_irq_ack_r);
	DECLARE_WRITE8_MEMBER(vblank_irq_ack_w);
	DECLARE_READ8_MEMBER(status_r);

	DECLARE_WRITE_LINE_MEMBER(lightgun1_trigger_w);
	DECLARE_WRITE_LINE_MEMBER(lightgun2_trigger_w);
	DECLARE_WRITE_LINE_MEMBER(lightgun_irq_ack_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<screen_device> m_screen;
	emu_timer *m_scanline_timer;

	enum
	{
		RASTER1_IRQ = 1,
		RASTER2_IRQ = 0
	};

	// callbacks
	devcb_read8 m_lightgun1_cb;
	devcb_read8 m_lightgun2_cb;
	devcb_write_line m_lightgun_irq_cb;
	devcb_write_line m_raster1_irq_cb;
	devcb_write_line m_raster2_irq_cb;
	devcb_write_line m_vblank_irq_cb;

	bool m_lightgun_irq, m_raster_irq, m_vblank_irq;

	int m_raster_irq_target;
	bool m_raster_irq_masked;
	uint8_t m_raster_irq_scanline;

	uint8_t m_lightgun_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(DECO_IRQ, deco_irq_device)


#endif // MAME_MACHINE_DECO_IRQ_H
