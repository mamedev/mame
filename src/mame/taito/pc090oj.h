// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_PC090OJ_H
#define MAME_TAITO_PC090OJ_H

#pragma once

class pc090oj_device : public device_t, public device_gfx_interface
{
public:
	typedef device_delegate<void (u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl)> colpri_cb_delegate;

	pc090oj_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_usebuffer(bool use_buf) { m_use_buffer = use_buf; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
	}
	template <typename... T> void set_colpri_callback(T &&... args) { m_colpri_cb.set(std::forward<T>(args)...); }

	u16 word_r(offs_t offset);
	void word_w(offs_t offset, u16 data, u16 mem_mask = 0);
	void sprite_ctrl_w(u16 data);

	void eof_callback();
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	/* NB: pc090oj_ctrl is the internal register controlling flipping

	pc090oj_sprite_ctrl is a representation of the hardware OUTSIDE the pc090oj
	which impacts on sprite plotting, and which varies between games. It
	includes color banking and (optionally) priority. It allows each game to
	control these aspects of the sprites in different ways, while keeping the
	routines here modular.
	*/

	colpri_cb_delegate m_colpri_cb;

	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	u16     m_ctrl;
	u16     m_sprite_ctrl;

	std::unique_ptr<u16[]>  m_ram;
	std::unique_ptr<u16[]>  m_ram_buffered;

	int        m_x_offset, m_y_offset;
	bool       m_use_buffer;
};

DECLARE_DEVICE_TYPE(PC090OJ, pc090oj_device)

#endif // MAME_TAITO_PC090)J_H
