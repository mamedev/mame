// license: BSD-3-Clause
// copyright-holders:Sergio Galiano
/***************************************************************************

  NMK IRQ Generator

***************************************************************************/

#ifndef MAME_NMK_NMK_IRQ_H
#define MAME_NMK_NMK_IRQ_H

#pragma once

#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nmk_irq_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	nmk_irq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto irq_callback() { return m_irq_cb.bind(); }
	auto sprite_dma_callback() { return m_sprite_dma_cb.bind(); }
	void set_prom_start_offset(u32 offset) { m_prom_start_offset = offset; }
	void set_prom_frame_offset(u32 offset) { m_prom_frame_offset = offset; }
	void set_vtiming_prom_usage(u32 usage) { m_vtiming_prom_usage = usage; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(scanline_callback);

private:
	// memory pointers
	//required_region_ptr<u8> m_htiming_prom;
	required_region_ptr<u8> m_vtiming_prom;

	// callbacks
	devcb_write8 m_irq_cb;
	devcb_write_line m_sprite_dma_cb;

	// configurations
	u32 m_prom_start_offset;
	u32 m_prom_frame_offset;
	u32 m_vtiming_prom_usage;

	// internal states
	emu_timer *m_scanline_timer;

	u8 m_vtiming_val;
};


// device type declaration
DECLARE_DEVICE_TYPE(NMK_IRQ, nmk_irq_device)


#endif // MAME_NMK_NMK_IRQ_H
