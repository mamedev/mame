// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Jaleco Road Generator Hardware

***************************************************************************/
#ifndef MAME_JALECO_JALECO_ROAD_H
#define MAME_JALECO_JALECO_ROAD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class jaleco_road_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	template <typename T>
	jaleco_road_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, u16 colorbase)
		: jaleco_road_device(mconfig, tag, owner, (u32)0)
	{
		set_palette(std::forward<T>(palette_tag));
		set_colorbase(colorbase);
	}

	jaleco_road_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_colorbase(u16 colorbase) { m_colorbase = colorbase; }

	// drawing and layer control
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority1, int priority2, bool transparency);

protected:
	jaleco_road_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// shared memory finder
	required_shared_ptr<u16> m_roadram;

	// configuration
	u16 m_colorbase;

	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
};

class jaleco_zoom_road_device : public jaleco_road_device
{
public:
	// construction/destruction
	template <typename T>
	jaleco_zoom_road_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&palette_tag, u16 colorbase)
		: jaleco_zoom_road_device(mconfig, tag, owner, (u32)0)
	{
		set_palette(std::forward<T>(palette_tag));
		set_colorbase(colorbase);
	}

	jaleco_zoom_road_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// drawing and layer control
	virtual void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority1, int priority2, bool transparency) override;
};

// device type declaration
DECLARE_DEVICE_TYPE(JALECO_ROAD,      jaleco_road_device)
DECLARE_DEVICE_TYPE(JALECO_ZOOM_ROAD, jaleco_zoom_road_device)

#endif  // MAME_JALECO_JALECO_ROAD_H
