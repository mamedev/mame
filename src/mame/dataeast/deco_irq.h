// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Data East IRQ Controller

***************************************************************************/

#ifndef MAME_DATAEAST_DECO_IRQ_H
#define MAME_DATAEAST_DECO_IRQ_H

#pragma once

#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class deco_irq_device : public device_t
{
public:
	// construction/destruction
	deco_irq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto lightgun1_callback() { return m_lightgun1_cb.bind(); }
	auto lightgun2_callback() { return m_lightgun2_cb.bind(); }
	auto lightgun_irq_callback() { return m_lightgun_irq_cb.bind(); }
	auto raster1_irq_callback() { return m_raster1_irq_cb.bind(); }
	auto raster2_irq_callback() { return m_raster2_irq_cb.bind(); }
	auto vblank_irq_callback() { return m_vblank_irq_cb.bind(); }

	// configuration
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	TIMER_CALLBACK_MEMBER(scanline_callback);

	void map(address_map &map) ATTR_COLD;

	void control_w(u8 data);
	u8 scanline_r();
	void scanline_w(u8 data);
	u8 raster_irq_ack_r();
	void vblank_irq_ack_w(u8 data);
	u8 status_r();

	void raster_irq_ack();

	void lightgun1_trigger_w(int state);
	void lightgun2_trigger_w(int state);
	void lightgun_irq_ack_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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
	u8 m_raster_irq_scanline;

	int m_lightgun_latch;
};


// device type definition
DECLARE_DEVICE_TYPE(DECO_IRQ, deco_irq_device)


#endif // MAME_DATAEAST_DECO_IRQ_H
