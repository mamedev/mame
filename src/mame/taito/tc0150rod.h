// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_TC0150ROD_H
#define MAME_TAITO_TC0150ROD_H

#pragma once

class tc0150rod_device : public device_t
{
public:
	tc0150rod_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	uint16_t word_r(offs_t offset);
	void word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, bitmap_ind8 &priority_bitmap, u8 low_priority, u8 high_priority, u8 pmask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// internal state
	std::vector<u16> m_ram;
	required_region_ptr<u16> m_roadgfx;
};

DECLARE_DEVICE_TYPE(TC0150ROD, tc0150rod_device)


#endif // MAME_TAITO_TC0150ROD_H
