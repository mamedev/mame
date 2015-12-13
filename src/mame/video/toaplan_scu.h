// license:BSD-3-Clause
// copyright-holders:Quench
/* toaplan SCU */



class toaplan_scu_device : public device_t, public device_gfx_interface
{
	static const gfx_layout spritelayout;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

public:
	toaplan_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_xoffsets(device_t &device, int xoffs, int xoffs_flipped);

	void draw_sprites_to_tempbitmap(const rectangle &cliprect, UINT16* spriteram, UINT32 bytes );
	void copy_sprites_from_tempbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void alloc_sprite_bitmap(screen_device &screen);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	bitmap_ind16 m_temp_spritebitmap;
	int m_xoffs;
	int m_xoffs_flipped;
};

extern const device_type TOAPLAN_SCU;

#define MCFG_TOAPLAN_SCU_ADD(_tag, _palette_tag, _xoffs, _xoffs_flipped ) \
	MCFG_DEVICE_ADD(_tag, TOAPLAN_SCU, 0) \
	MCFG_GFX_PALETTE(_palette_tag) \
	toaplan_scu_device::static_set_xoffsets(*device, _xoffs, _xoffs_flipped);
