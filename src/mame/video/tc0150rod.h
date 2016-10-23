// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef __TC0150ROD_H__
#define __TC0150ROD_H__

class tc0150rod_device : public device_t
{
public:
	tc0150rod_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0150rod_device() {}

	uint16_t word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, bitmap_ind8 &priority_bitmap, uint32_t low_priority, uint32_t high_priority);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	std::vector<uint16_t> m_ram;
	required_region_ptr<uint16_t> m_roadgfx;
};

extern const device_type TC0150ROD;


#endif
