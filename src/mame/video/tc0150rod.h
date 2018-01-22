// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0150ROD_H
#define MAME_VIDEO_TC0150ROD_H

#pragma once

class tc0150rod_device : public device_t
{
public:
	tc0150rod_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, bitmap_ind8 &priority_bitmap, uint32_t low_priority, uint32_t high_priority);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	std::vector<uint16_t> m_ram;
	required_region_ptr<uint16_t> m_roadgfx;
};

DECLARE_DEVICE_TYPE(TC0150ROD, tc0150rod_device)


#endif // MAME_VIDEO_TC0150ROD_H
