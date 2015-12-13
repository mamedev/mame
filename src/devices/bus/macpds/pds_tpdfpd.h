// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __PDS_SEDISPLAY_H__
#define __PDS_SEDISPLAY_H__

#include "emu.h"
#include "macpds.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> macpds_sedisplay_device

class macpds_sedisplay_device :
		public device_t,
		public device_video_interface,
		public device_macpds_card_interface
{
public:
		// construction/destruction
		macpds_sedisplay_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		macpds_sedisplay_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const override;
		virtual const rom_entry *device_rom_region() const override;

		UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

		DECLARE_READ16_MEMBER(sedisplay_r);
		DECLARE_WRITE16_MEMBER(sedisplay_w);
		DECLARE_READ16_MEMBER(ramdac_r);
		DECLARE_WRITE16_MEMBER(ramdac_w);

public:
		UINT8 *m_vram;
		UINT32 m_vbl_disable;
		UINT32 m_palette[256], m_colors[3], m_count, m_clutoffs;
		emu_timer *m_timer;
		std::string m_assembled_tag;
};


// device type definition
extern const device_type PDS_SEDISPLAY;

#endif  /* __MACPDS_SEDISPLAY_H__ */
