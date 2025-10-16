// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KONAMI_K053244_K053245_H
#define MAME_KONAMI_K053244_K053245_H

#pragma once


#define K053244_CB_MEMBER(_name) void _name(int &code, int &color, int &priority)


class k053244_device : public device_t, public device_gfx_interface
{
	static const gfx_layout spritelayout;
	static const gfx_layout spritelayout_6bpp;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_6bpp);

public:
	using sprite_delegate = device_delegate<void (int &code, int &color, int &priority)>;

	k053244_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_bpp(int bpp);
	template <typename... T> void set_sprite_callback(T &&... args) { m_k053244_cb.set(std::forward<T>(args)...); }
	void set_offsets(int dx, int dy) { m_dx = dx; m_dy = dy; }
	void set_priority_shadows(bool pri) { m_priority_shadows = pri; }

	u16 k053245_word_r(offs_t offset);
	void k053245_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 k053245_r(offs_t offset);
	void k053245_w(offs_t offset, u8 data);
	u8 k053244_r(offs_t offset);
	void k053244_w(offs_t offset, u8 data);
	void bankselect(u32 bank); // used by TMNT2, Asterix and Premier Soccer for ROM testing
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);
	void set_z_rejection(s32 zcode); // common to k053244/5

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u32 RAM_SIZE = 0x800;

	// internal state
	std::unique_ptr<u16[]>  m_ram;
	std::unique_ptr<u16[]>  m_buffer;
	required_region_ptr<u8> m_sprite_rom;

	s32 m_dx, m_dy;
	bool m_priority_shadows;
	sprite_delegate m_k053244_cb;

	u8  m_regs[0x10];    // 053244
	u32 m_rombank;       // 053244
	s32 m_z_rejection;

	void clear_buffer();
	void update_buffer();
};


DECLARE_DEVICE_TYPE(K053244, k053244_device)
static auto &K053245 = K053244;

#endif // MAME_KONAMI_K053244_K053245_H
