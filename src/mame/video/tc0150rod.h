// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef __TC0150ROD_H__
#define __TC0150ROD_H__

class tc0150rod_device : public device_t
{
public:
	tc0150rod_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0150rod_device() {}

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, bitmap_ind8 &priority_bitmap, UINT32 low_priority, UINT32 high_priority);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	std::vector<UINT16> m_ram;
	UINT16 *m_roadgfx;
};

extern const device_type TC0150ROD;


#endif
