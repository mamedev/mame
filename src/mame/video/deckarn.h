// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood


class deco_karnovsprites_device : public device_t
{
public:
	deco_karnovsprites_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_gfxregion(int region) { m_gfxregion = region; };
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, UINT16* spriteram, int size, int priority );

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int region);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8 m_gfxregion;
private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

extern const device_type DECO_KARNOVSPRITES;

#define MCFG_DECO_KARNOVSPRITES_GFXDECODE(_gfxtag) \
	deco_karnovsprites_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_DECO_KARNOVSPRITES_PALETTE(_palette_tag) \
	deco_karnovsprites_device::static_set_palette_tag(*device, "^" _palette_tag);
