// license:BSD-3-Clause
// copyright-holders:David Haywood

class excellent_spr_device : public device_t,
						public device_video_interface
{
public:
	excellent_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void aquarium_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int y_offs);
	void gcpinbal_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int y_offs, int priority);

protected:
	std::unique_ptr<uint8_t[]> m_ram;

	virtual void device_start() override;
	virtual void device_reset() override;
private:
};

extern const device_type EXCELLENT_SPRITE;
