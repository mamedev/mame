// license:BSD-3-Clause
// copyright-holders:David Haywood

class excellent_spr_device : public device_t,
						public device_video_interface
{
public:
	excellent_spr_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void aquarium_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int y_offs);
	void gcpinbal_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int y_offs, int priority);

protected:
	std::unique_ptr<UINT8[]> m_ram;

	virtual void device_start() override;
	virtual void device_reset() override;
private:
};

extern const device_type EXCELLENT_SPRITE;
