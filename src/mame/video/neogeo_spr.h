// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
#ifndef MAME_VIDEO_NEOGEO_SPR_H
#define MAME_VIDEO_NEOGEO_SPR_H

#pragma once

// todo, move these back, currently the sprite code needs some of the values tho
#define NEOGEO_MASTER_CLOCK                     (24000000)
#define NEOGEO_MAIN_CPU_CLOCK                   (NEOGEO_MASTER_CLOCK / 2)
#define NEOGEO_AUDIO_CPU_CLOCK                  (NEOGEO_MASTER_CLOCK / 6)
#define NEOGEO_YM2610_CLOCK                     (NEOGEO_MASTER_CLOCK / 3)
#define NEOGEO_PIXEL_CLOCK                      (NEOGEO_MASTER_CLOCK / 4)
#define NEOGEO_HTOTAL                           (0x180)
#define NEOGEO_HBEND                            (0x01e) /* this should really be 29.5 */
#define NEOGEO_HBSTART                          (0x15e) /* this should really be 349.5 */
#define NEOGEO_VTOTAL                           (0x108)
#define NEOGEO_VBEND                            (0x010)
#define NEOGEO_VBSTART                          (0x0f0)
#define NEOGEO_VSSTART                          (0x100)

// todo, sort out what needs to be public and make the rest private/protected
class neosprite_base_device : public device_t, public device_video_interface
{
public:
	virtual void optimize_sprite_data();
	virtual void set_optimized_sprite_data(u8* sprdata, u32 mask);

	virtual void draw_fixed_layer_2pixels(u32*&pixel_addr, int offset, u8* gfx_base, const pen_t* char_pens);
	void draw_fixed_layer(bitmap_rgb32 &bitmap, int scanline);
	void set_videoram_offset(u16 data);
	u16 get_videoram_data();
	void set_videoram_data(u16 data);
	void set_videoram_modulo(u16 data);
	u16 get_videoram_modulo();
	void set_auto_animation_speed(u8 data);
	void set_auto_animation_disabled(u8 data);
	u8 neogeo_get_auto_animation_counter();
	void create_auto_animation_timer();
	void start_auto_animation_timer();
	void neogeo_set_fixed_layer_source(u8 data);
	inline bool sprite_on_scanline(int scanline, int y, int rows);
	virtual void draw_pixel(int romaddr, u32* dst, const pen_t *line_pens) = 0;
	void draw_sprites(bitmap_rgb32 &bitmap, int scanline);
	void parse_sprites(int scanline);
	void create_sprite_line_timer();
	void start_sprite_line_timer();
	virtual void set_sprite_region(u8* region_sprites, u32 region_sprites_size);
	void set_fixed_regions(u8* fix_cart, u32 fix_cart_size, memory_region* fix_bios);
	void set_pens(const pen_t* pens);

	std::unique_ptr<u16[]>     m_videoram;
	u16     *m_videoram_drawsource;

	u16     m_vram_offset;
	u16     m_vram_read_buffer;
	u16     m_vram_modulo;

	u32     m_sprite_gfx_address_mask;

	u8      m_auto_animation_speed;
	u8      m_auto_animation_disabled;
	u8      m_auto_animation_counter;
	u8      m_auto_animation_frame_counter;

	u8      m_fixed_layer_source;
	u8      m_fixed_layer_bank_type;

	emu_timer  *m_auto_animation_timer;
	emu_timer  *m_sprite_line_timer;

	TIMER_CALLBACK_MEMBER(auto_animation_timer_callback);
	TIMER_CALLBACK_MEMBER(sprite_line_timer_callback);

	int m_bppshift; // 4 for 4bpp gfx (NeoGeo) 8 for 8bpp gfx (Midas)

protected:
	neosprite_base_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	u32 get_region_mask(u8* rgn, u32 rgn_size);
	u8* m_region_sprites; u32 m_region_sprites_size;
	u8* m_region_fixed; u32 m_region_fixed_size;
	memory_region* m_region_fixedbios;
	const pen_t   *m_pens;

	required_region_ptr<u8> m_region_zoomy;
};


class neosprite_regular_device : public neosprite_base_device
{
public:
	neosprite_regular_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void draw_pixel(int romaddr, u32* dst, const pen_t *line_pens) override;
	virtual void set_sprite_region(u8* region_sprites, u32 region_sprites_size) override;

};

DECLARE_DEVICE_TYPE(NEOGEO_SPRITE_REGULAR, neosprite_regular_device)


class neosprite_optimized_device : public neosprite_base_device
{
public:
	neosprite_optimized_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void optimize_sprite_data() override;
	virtual void set_optimized_sprite_data(u8* sprdata, u32 mask) override;
	virtual void draw_pixel(int romaddr, u32* dst, const pen_t *line_pens) override;
	std::vector<u8> m_sprite_gfx;
	u8* m_spritegfx8;

private:
	u32 optimize_helper(std::vector<u8> &spritegfx, u8* region_sprites, u32 region_sprites_size);
};

DECLARE_DEVICE_TYPE(NEOGEO_SPRITE_OPTIMZIED, neosprite_optimized_device)


class neosprite_midas_device : public neosprite_base_device
{
public:
	neosprite_midas_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void draw_pixel(int romaddr, u32* dst, const pen_t *line_pens) override;

	std::unique_ptr<u16[]> m_videoram_buffer;
	void buffer_vram();
	virtual void draw_fixed_layer_2pixels(u32*&pixel_addr, int offset, u8* gfx_base, const pen_t* char_pens) override;
	virtual void set_sprite_region(u8* region_sprites, u32 region_sprites_size) override;

	protected:
	virtual void device_start() override;

};

DECLARE_DEVICE_TYPE(NEOGEO_SPRITE_MIDAS, neosprite_midas_device)

#endif // MAME_VIDEO_NEOGEO_SPR_H
