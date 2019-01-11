// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_PC090OJ_H
#define MAME_VIDEO_PC090OJ_H

#pragma once

class pc090oj_device : public device_t
{
public:
	pc090oj_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }
	void set_palette_tag(const char *tag) { m_palette.set_tag(tag); }
	void set_gfx_region(int gfxregion) { m_gfxnum = gfxregion; }
	void set_usebuffer(int use_buf) { m_use_buffer = use_buf; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
	}

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );

	void set_sprite_ctrl(uint16_t sprctrl);
	void eof_callback();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri_type);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	/* NB: pc090oj_ctrl is the internal register controlling flipping

	pc090oj_sprite_ctrl is a representation of the hardware OUTSIDE the pc090oj
	which impacts on sprite plotting, and which varies between games. It
	includes color banking and (optionally) priority. It allows each game to
	control these aspects of the sprites in different ways, while keeping the
	routines here modular.

*/

	uint16_t     m_ctrl;
	uint16_t     m_sprite_ctrl;

	std::unique_ptr<uint16_t[]>  m_ram;
	std::unique_ptr<uint16_t[]>  m_ram_buffered;

	int        m_gfxnum;
	int        m_x_offset, m_y_offset;
	int        m_use_buffer;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

DECLARE_DEVICE_TYPE(PC090OJ, pc090oj_device)


#define MCFG_PC090OJ_GFX_REGION(_region) \
	downcast<pc090oj_device &>(*device).set_gfx_region(_region);

#define MCFG_PC090OJ_OFFSETS(_xoffs, _yoffs) \
	downcast<pc090oj_device &>(*device).set_offsets(_xoffs, _yoffs);

#define MCFG_PC090OJ_USEBUFFER(_use_buf) \
	downcast<pc090oj_device &>(*device).set_usebuffer(_use_buf);

#define MCFG_PC090OJ_GFXDECODE(_gfxtag) \
	downcast<pc090oj_device &>(*device).set_gfxdecode_tag(_gfxtag);

#define MCFG_PC090OJ_PALETTE(_palette_tag) \
	downcast<pc090oj_device &>(*device).set_palette_tag(_palette_tag);

#endif // MAME_VIDEO_PC090)J_H
