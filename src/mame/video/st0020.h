// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood



class st0020_device : public device_t, public device_gfx_interface
{
public:
	st0020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	static void set_is_st0032(device_t &device, int is_st0032);
	static void set_is_jclub2o(device_t &device, int is_jclub2o);

	// see if we can handle the difference between this and the st0032 in here, or if we need another
	// device
	int m_is_st0032;

	// per-game hack
	int m_is_jclub2;

	void st0020_draw_all(bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t st0020_gfxram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void st0020_gfxram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t st0020_blitram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void st0020_blitram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t st0020_sprram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void st0020_sprram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	int m_st0020_gfxram_bank;
	std::unique_ptr<uint16_t[]> m_st0020_gfxram;
	std::unique_ptr<uint16_t[]> m_st0020_spriteram;
	std::unique_ptr<uint16_t[]> m_st0020_blitram;
	void st0020_draw_zooming_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	uint16_t st0020_blit_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void st0020_blit_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t* m_rom_ptr;
	size_t m_rom_size;
};

#define ST0020_ST0032_BYTESWAP_DATA \
	if (m_is_st0032) data = ((data & 0x00ff)<<8) | ((data & 0xff00)>>8);
#define ST0020_ST0032_BYTESWAP_MEM_MASK \
	if (m_is_st0032) mem_mask = ((mem_mask & 0x00ff)<<8) | ((mem_mask & 0xff00)>>8);
extern const device_type ST0020_SPRITES;

#define MCFG_ST0020_SPRITES_PALETTE(_palette_tag) \
	MCFG_GFX_PALETTE(_palette_tag)
