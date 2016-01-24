// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood



class st0020_device : public device_t
{
public:
	st0020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_is_st0032(device_t &device, int is_st0032);
	static void set_is_jclub2o(device_t &device, int is_jclub2o);

	int m_gfx_index;

	// see if we can handle the difference between this and the st0032 in here, or if we need another
	// device
	int m_is_st0032;

	// per-game hack
	int m_is_jclub2;

	void st0020_draw_all(bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER(st0020_gfxram_r);
	DECLARE_WRITE16_MEMBER(st0020_gfxram_w);
	DECLARE_READ16_MEMBER(st0020_blitram_r);
	DECLARE_WRITE16_MEMBER(st0020_blitram_w);
	DECLARE_READ16_MEMBER(st0020_sprram_r);
	DECLARE_WRITE16_MEMBER(st0020_sprram_w);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	int m_st0020_gfxram_bank;
	std::unique_ptr<UINT16[]> m_st0020_gfxram;
	std::unique_ptr<UINT16[]> m_st0020_spriteram;
	std::unique_ptr<UINT16[]> m_st0020_blitram;
	void st0020_draw_zooming_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	DECLARE_READ16_MEMBER(st0020_blit_r);
	DECLARE_WRITE16_MEMBER(st0020_blit_w);
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_region_ptr<UINT8> m_rom_ptr;
};

#define ST0020_ST0032_BYTESWAP_DATA \
	if (m_is_st0032) data = ((data & 0x00ff)<<8) | ((data & 0xff00)>>8);
#define ST0020_ST0032_BYTESWAP_MEM_MASK \
	if (m_is_st0032) mem_mask = ((mem_mask & 0x00ff)<<8) | ((mem_mask & 0xff00)>>8);
extern const device_type ST0020_SPRITES;

#define MCFG_ST0020_SPRITES_GFXDECODE(_gfxtag) \
	st0020_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_ST0020_SPRITES_PALETTE(_palette_tag) \
	st0020_device::static_set_palette_tag(*device, "^" _palette_tag);
