// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#ifndef MAME_KANEKO_KANEKO_SPR_H
#define MAME_KANEKO_KANEKO_SPR_H

#pragma once

/* Kaneko Sprites */


struct priority_t
{
	int sprite[4];
};

struct tempsprite_t
{
	u32 code,color;
	int x,y;
	int xoffs,yoffs;
	bool flipx,flipy;
	int priority;
};



class kaneko16_sprite_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	// configuration
	void set_color_base(u16 base) { m_colbase = base; }
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

	void render_sprites(const rectangle &cliprect, u16* spriteram16, int spriteram16_bytes);

	void copybitmap(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);
	void copybitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);

	template<class BitmapClass>
	void copybitmap_common(BitmapClass &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);

	void bootleg_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u16* spriteram16, int spriteram16_bytes);

	u16 regs_r(offs_t offset);
	void regs_w(offs_t offset, u16 data, u16 mem_mask);

protected:
	kaneko16_sprite_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;


	// flip latching (set when declaring device in MCFG )  probably needs figuring out properly, only brapboys wants it?
	int m_sprite_fliptype;

	// offsets (set when declaring device in MCFG )
	u16 m_sprite_xoffs;
	u16 m_sprite_yoffs;

	// priority for mixing (set when declaring device in MCFG )
	priority_t m_priority;

	// pure virtual function for getting the attributes on sprites, the two different chip types have
	// them in a different order
	virtual void get_sprite_attributes(struct tempsprite_t *s, u16 attr) =0;

	required_memory_region m_gfx_region;
	u16 m_colbase;

private:
	// registers
	u16 m_sprite_flipx;
	u16 m_sprite_flipy;
	std::unique_ptr<u16[]> m_sprites_regs;

	std::unique_ptr<struct tempsprite_t[]> m_first_sprite;
	int m_keep_sprites;
	bitmap_ind16 m_sprites_bitmap;
	bitmap_ind8 m_sprites_maskmap;


	void draw_sprites(const rectangle &cliprect, u16* spriteram16, int spriteram16_bytes);


	void draw_sprites_custom(const rectangle &clip,gfx_element *gfx,
			u32 code,u32 color,bool flipx,bool flipy,int sx,int sy,
			int priority);

	int parse_sprite_type012(int i, struct tempsprite_t *s, u16* spriteram16, int spriteram16_bytes);
};

//extern const device_type KANEKO16_SPRITE;


/* berlwall, blazeon etc. */
class kaneko_vu002_sprite_device : public kaneko16_sprite_device
{
public:
	kaneko_vu002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: kaneko_vu002_sprite_device(mconfig, tag, owner, (u32)0)
	{
	}

	kaneko_vu002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	void get_sprite_attributes(struct tempsprite_t *s, u16 attr) override;
	int get_sprite_type(void) override{ return 0; }

protected:
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(KANEKO_VU002_SPRITE, kaneko_vu002_sprite_device)

/* gtmr, gtmr2, bloodwar etc. */
class kaneko_kc002_sprite_device : public kaneko16_sprite_device
{
public:
	kaneko_kc002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: kaneko_kc002_sprite_device(mconfig, tag, owner, (u32)0)
	{
	}

	kaneko_kc002_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	void get_sprite_attributes(struct tempsprite_t *s, u16 attr) override;
	int get_sprite_type(void) override{ return 1; }

protected:
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(KANEKO_KC002_SPRITE, kaneko_kc002_sprite_device)

#endif // MAME_KANEKO_KANEKO_SPR_H
