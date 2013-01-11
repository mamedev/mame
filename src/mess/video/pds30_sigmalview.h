#pragma once

#ifndef __NUBUS_LVIEW_H__
#define __NUBUS_LVIEW_H__

#include "emu.h"
#include "machine/nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nubus_lview_device

class nubus_lview_device :
		public device_t,
		public device_nubus_card_interface
{
public:
		// construction/destruction
		nubus_lview_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		nubus_lview_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;

		UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

		DECLARE_READ32_MEMBER(lview_r);
		DECLARE_WRITE32_MEMBER(lview_w);
		DECLARE_READ32_MEMBER(vram_r);
		DECLARE_WRITE32_MEMBER(vram_w);

public:
		UINT8 *m_vram;
		UINT32 *m_vram32;
		UINT32 m_vbl_disable, m_toggle;
		UINT32 m_palette[256];
		screen_device *m_screen;
		emu_timer *m_timer;
		int m_protstate;
};


// device type definition
extern const device_type PDS030_LVIEW;

#endif  /* __NUBUS_LVIEW_H__ */
