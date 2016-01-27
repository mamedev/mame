// license:BSD-3-Clause
// copyright-holders:David Haywood

class tigeroad_spr_device : public device_t
{
public:
	tigeroad_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfxdecode_device *gfxdecode, int region, UINT16* ram, UINT32 size, int flip_screen, int rev_y);

protected:

	virtual void device_start() override;
	virtual void device_reset() override;
private:
};

extern const device_type TIGEROAD_SPRITE;
