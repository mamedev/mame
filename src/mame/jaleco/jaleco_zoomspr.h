// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Jaleco Zoomable Sprite Generator Hardware

***************************************************************************/
#ifndef MAME_JALECO_JALECO_ZOOMSPR_H
#define MAME_JALECO_JALECO_ZOOMSPR_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jaleco_zoomspr_device

class jaleco_zoomspr_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	template <typename T>
	jaleco_zoomspr_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: jaleco_zoomspr_device(mconfig, tag, owner, (u32)0)
	{
		set_palette(std::forward<T>(palette_tag));
		set_info(gfxinfo);
	}

	jaleco_zoomspr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// drawing
	virtual void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority1, int priority2, const u16 *source, u32 ramsize);

protected:
	jaleco_zoomspr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 m_drawmode_table[16];
};

class jaleco_zoomspr_bigrun_device : public jaleco_zoomspr_device
{
public:
	// construction/destruction
	template <typename T>
	jaleco_zoomspr_bigrun_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: jaleco_zoomspr_bigrun_device(mconfig, tag, owner, (u32)0)
	{
		set_palette(std::forward<T>(palette_tag));
		set_info(gfxinfo);
	}

	jaleco_zoomspr_bigrun_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// drawing
	virtual void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority1, int priority2, const u16 *source, u32 ramsize) override;
};

// device type declaration
DECLARE_DEVICE_TYPE(JALECO_ZOOMSPR,        jaleco_zoomspr_device)
DECLARE_DEVICE_TYPE(JALECO_ZOOMSPR_BIGRUN, jaleco_zoomspr_bigrun_device)

#endif  // MAME_JALECO_JALECO_ZOOMSPR_H
