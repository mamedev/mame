// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_EXCELLENT_EXCELLENT_SPR_H
#define MAME_EXCELLENT_EXCELLENT_SPR_H

#pragma once

class excellent_spr_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	typedef device_delegate<void (u32 &colour, u32 &pri_mask)> colpri_cb_delegate;

	excellent_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_color_base(u16 base) { m_colbase = base; }
	template <typename... T> void set_colpri_callback(T &&... args) { m_colpri_cb.set(std::forward<T>(args)...); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void aquarium_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gcpinbal_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	colpri_cb_delegate m_colpri_cb;
	u16 m_colbase;

	std::unique_ptr<u8[]> m_ram;
	std::unique_ptr<u8[]> m_ram_buffered;

	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
};

DECLARE_DEVICE_TYPE(EXCELLENT_SPRITE, excellent_spr_device)

#endif // MAME_EXCELLENT_EXCELLENT_SPR_H
