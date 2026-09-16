// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
#ifndef MAME_BUS_COCO_COCO_WPK_H
#define MAME_BUS_COCO_COCO_WPK_H

#pragma once

#include "cococart.h"
#include "video/mc6845.h"
#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_wpk_device

class coco_wpk_device :
	public device_t,
	public device_cococart_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	coco_wpk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// construction/destruction
	coco_wpk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_device<r6545_1_device> m_crtc;
	required_device<palette_device> m_palette;
	required_memory_region m_chargen;

	u16 m_video_addr;

	std::unique_ptr<u8[]> m_video_ram;

	void crtc_display_w(u8 data);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	MC6845_UPDATE_ROW(crtc_update_row);
};


// ======================> coco_wpk2_device

class coco_wpk2_device : public coco_wpk_device
{
public:
	// construction/destruction
	coco_wpk2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void video_select_w(u8 data);
};


// ======================> coco_wpkrs_device

class coco_wpkrs_device : public coco_wpk_device
{
public:
	// construction/destruction
	coco_wpkrs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(COCO_WPK, coco_wpk_device)
DECLARE_DEVICE_TYPE(COCO_WPK2, coco_wpk2_device)
DECLARE_DEVICE_TYPE(COCO_WPKRS, coco_wpkrs_device)

#endif // MAME_BUS_COCO_COCO_WPK_H
