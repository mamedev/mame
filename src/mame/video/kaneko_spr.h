// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#ifndef MAME_VIDEO_KANEKO_SPR_H
#define MAME_VIDEO_KANEKO_SPR_H

#pragma once

/* Kaneko Sprites */


struct kaneko16_priority_t
{
	int sprite[4];
};

struct kan_tempsprite
{
	int code,color;
	int x,y;
	int xoffs,yoffs;
	int flipx,flipy;
	int priority;
};



class kaneko16_sprite_device : public device_t, public device_video_interface
{
public:
	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_fliptype(int fliptype) { m_sprite_fliptype = fliptype; }
	void set_offsets(int xoffs, int yoffs)
	{
		m_sprite_xoffs = xoffs;
		m_sprite_yoffs = yoffs;
	}
	void set_priorities(int pri0, int pri1, int pri2, int pri3)
	{
		m_priority.sprite[0] = pri0;
		m_priority.sprite[1] = pri1;
		m_priority.sprite[2] = pri2;
		m_priority.sprite[3] = pri3;
	}

	// (legacy) used in the bitmap clear functions
	virtual int get_sprite_type(void) =0;

	void kaneko16_render_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, uint16_t* spriteram16, int spriteram16_bytes);
	void kaneko16_render_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, uint16_t* spriteram16, int spriteram16_bytes);


	template<class _BitmapClass>
	void kaneko16_render_sprites_common(_BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, uint16_t* spriteram16, int spriteram16_bytes);

	void bootleg_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t* spriteram16, int spriteram16_bytes);


	DECLARE_READ16_MEMBER(kaneko16_sprites_regs_r);
	DECLARE_WRITE16_MEMBER(kaneko16_sprites_regs_w);

protected:
	kaneko16_sprite_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;


	// flip latching (set when declaring device in MCFG )  probably needs figuring out properly, only brapboys wants it?
	int m_sprite_fliptype;

	// offsets (set when declaring device in MCFG )
	uint16_t m_sprite_xoffs;
	uint16_t m_sprite_yoffs;

	// priority for mixing (set when declaring device in MCFG )
	kaneko16_priority_t m_priority;

	// pure virtual function for getting the attributes on sprites, the two different chip types have
	// them in a different order
	virtual void get_sprite_attributes(struct kan_tempsprite *s, uint16_t attr) =0;


private:
	// registers
	uint16_t m_sprite_flipx;
	uint16_t m_sprite_flipy;
	std::unique_ptr<uint16_t[]> m_sprites_regs;

	std::unique_ptr<struct kan_tempsprite[]> m_first_sprite;
	int m_keep_sprites;
	bitmap_ind16 m_sprites_bitmap;


	template<class _BitmapClass>
	void kaneko16_draw_sprites(_BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, uint16_t* spriteram16, int spriteram16_bytes);


	template<class _BitmapClass>
	void kaneko16_draw_sprites_custom(_BitmapClass &dest_bmp,const rectangle &clip,gfx_element *gfx,
			uint32_t code,uint32_t color,int flipx,int flipy,int sx,int sy,
			bitmap_ind8 &priority_bitmap, int priority);

	int kaneko16_parse_sprite_type012(int i, struct kan_tempsprite *s, uint16_t* spriteram16, int spriteram16_bytes);

	void kaneko16_copybitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void kaneko16_copybitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<gfxdecode_device> m_gfxdecode;

};

//extern const device_type KANEKO16_SPRITE;


/* berlwall, blazeon etc. */
class kaneko_vu002_sprite_device : public kaneko16_sprite_device
{
public:
	kaneko_vu002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: kaneko_vu002_sprite_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	kaneko_vu002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void get_sprite_attributes(struct kan_tempsprite *s, uint16_t attr) override;
	int get_sprite_type(void) override{ return 0; };
};

DECLARE_DEVICE_TYPE(KANEKO_VU002_SPRITE, kaneko_vu002_sprite_device)

/* gtmr, gtmr2, bloodwar etc. */
class kaneko_kc002_sprite_device : public kaneko16_sprite_device
{
public:
	kaneko_kc002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: kaneko_kc002_sprite_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	kaneko_kc002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void get_sprite_attributes(struct kan_tempsprite *s, uint16_t attr) override;
	int get_sprite_type(void) override{ return 1; };
};

DECLARE_DEVICE_TYPE(KANEKO_KC002_SPRITE, kaneko_kc002_sprite_device)

#endif // MAME_VIDEO_KANEKO_SPR_H
