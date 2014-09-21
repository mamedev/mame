#ifndef __TC0150ROD_H__
#define __TC0150ROD_H__

class tc0150rod_device : public device_t
{
public:
	tc0150rod_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0150rod_device() {}

	static void set_gfx_tag(device_t &device, const char *tag) { downcast<tc0150rod_device &>(device).m_gfx_region = tag; }

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, bitmap_ind8 &priority_bitmap, UINT32 low_priority, UINT32 high_priority);

protected:
	// device-level overrides
	virtual void device_start();

private:
	// internal state
	UINT16 *        m_ram;
	const char      *m_gfx_region;    /* gfx region for the road */
	UINT16 *        m_roadgfx;
};

extern const device_type TC0150ROD;


#define MCFG_TC0150ROD_GFXTAG(_tag) \
	tc0150rod_device::set_gfx_tag(*device, _tag);

#endif
