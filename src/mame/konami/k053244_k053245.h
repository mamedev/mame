// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KONAMI_K053244_K053245_H
#define MAME_KONAMI_K053244_K053245_H

#pragma once


#define K05324X_CB_MEMBER(_name)   void _name(int *code, int *color, int *priority)


class k05324x_device : public device_t, public device_gfx_interface
{
	static const gfx_layout spritelayout;
	static const gfx_layout spritelayout_6bpp;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_6bpp);

public:
	using sprite_delegate = device_delegate<void (int *code, int *color, int *priority)>;

	k05324x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_bpp(int bpp);
	template <typename... T> void set_sprite_callback(T &&... args) { m_k05324x_cb.set(std::forward<T>(args)...); }
	void set_offsets(int x_offset, int y_offset)
	{
		m_dx = x_offset;
		m_dy = y_offset;
	}

	u16 k053245_word_r(offs_t offset);
	void k053245_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 k053245_r(offs_t offset);
	void k053245_w(offs_t offset, u8 data);
	u8 k053244_r(offs_t offset);
	void k053244_w(offs_t offset, u8 data);
	void bankselect(int bank);    /* used by TMNT2, Asterix and Premier Soccer for ROM testing */
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);
	void clear_buffer();
	void update_buffer();
	void set_z_rejection(int zcode); // common to k053244/5

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	std::unique_ptr<uint16_t[]>  m_ram;
	std::unique_ptr<uint16_t[]>  m_buffer;
	required_region_ptr<uint8_t> m_sprite_rom;

	int m_dx, m_dy;
	sprite_delegate m_k05324x_cb;

	uint8_t  m_regs[0x10];    // 053244
	int      m_rombank;       // 053244
	int      m_ramsize;
	int      m_z_rejection;
};


DECLARE_DEVICE_TYPE(K053244, k05324x_device)
static auto &K053245 = K053244;

#endif // MAME_KONAMI_K053244_K053245_H
