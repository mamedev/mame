// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_K053244_K053245_H
#define MAME_VIDEO_K053244_K053245_H

#pragma once


typedef device_delegate<void (int *code, int *color, int *priority)> k05324x_cb_delegate;
#define K05324X_CB_MEMBER(_name)   void _name(int *code, int *color, int *priority)


class k05324x_device : public device_t, public device_gfx_interface
{
	static const gfx_layout spritelayout;
	static const gfx_layout spritelayout_6bpp;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_6bpp);

public:
	k05324x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_bpp(int bpp);
	template <typename... T> void set_sprite_callback(T &&... args) { m_k05324x_cb = k05324x_cb_delegate(std::forward<T>(args)...); }
	void set_offsets(int x_offset, int y_offset)
	{
		m_dx = x_offset;
		m_dy = y_offset;
	}

	DECLARE_READ16_MEMBER( k053245_word_r );
	DECLARE_WRITE16_MEMBER( k053245_word_w );
	DECLARE_READ8_MEMBER( k053245_r );
	DECLARE_WRITE8_MEMBER( k053245_w );
	DECLARE_READ8_MEMBER( k053244_r );
	DECLARE_WRITE8_MEMBER( k053244_w );
	DECLARE_READ16_MEMBER( k053244_lsb_r );
	DECLARE_WRITE16_MEMBER( k053244_lsb_w );
	DECLARE_READ16_MEMBER( k053244_word_r );
	DECLARE_WRITE16_MEMBER( k053244_word_w );
	void bankselect(int bank);    /* used by TMNT2, Asterix and Premier Soccer for ROM testing */
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);
	void clear_buffer();
	void update_buffer();
	void set_z_rejection(int zcode); // common to k053244/5

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<uint16_t[]>  m_ram;
	std::unique_ptr<uint16_t[]>  m_buffer;
	required_region_ptr<uint8_t> m_sprite_rom;

	int m_dx, m_dy;
	k05324x_cb_delegate m_k05324x_cb;

	uint8_t    m_regs[0x10];    // 053244
	int      m_rombank;       // 053244
	int      m_ramsize;
	int      m_z_rejection;
};


DECLARE_DEVICE_TYPE(K053244, k05324x_device)
DECLARE_DEVICE_TYPE(K053245, k05324x_device)

#endif // MAME_VIDEO_K053244_K053245_H
