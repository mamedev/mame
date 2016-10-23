// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood



class fuukivid_device : public device_t,
						public device_video_interface
{
public:
	fuukivid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip_screen, uint32_t* tilebank);
	std::unique_ptr<uint16_t[]> m_sprram;
	std::unique_ptr<uint16_t[]> m_sprram_old;
	std::unique_ptr<uint16_t[]> m_sprram_old2;


	void fuuki_sprram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff)
	{
		COMBINE_DATA(&m_sprram[offset]);
	};

	uint16_t fuuki_sprram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff)
	{
		return m_sprram[offset];
	}

	void buffer_sprites(void);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<gfxdecode_device> m_gfxdecode;
};

extern const device_type FUUKI_VIDEO;

#define MCFG_FUUKI_VIDEO_GFXDECODE(_gfxtag) \
	fuukivid_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
