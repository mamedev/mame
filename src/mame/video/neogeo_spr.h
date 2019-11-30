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
	virtual void set_optimized_sprite_data(uint8_t* sprdata, uint32_t mask);

	virtual void draw_fixed_layer_2pixels(uint32_t*&pixel_addr, int offset, uint8_t* gfx_base, const pen_t* char_pens);
	void draw_fixed_layer(bitmap_rgb32 &bitmap, int scanline);
	void set_videoram_offset(uint16_t data);
	uint16_t get_videoram_data();
	void set_videoram_data(uint16_t data);
	void set_videoram_modulo(uint16_t data);
	uint16_t get_videoram_modulo();
	void set_auto_animation_speed(uint8_t data);
	void set_auto_animation_disabled(uint8_t data);
	uint8_t neogeo_get_auto_animation_counter();
	void create_auto_animation_timer();
	void start_auto_animation_timer();
	void neogeo_set_fixed_layer_source(uint8_t data);
	inline bool sprite_on_scanline(int scanline, int y, int rows);
	virtual void draw_pixel(int romaddr, uint32_t* dst, const pen_t *line_pens) = 0;
	void draw_sprites(bitmap_rgb32 &bitmap, int scanline);
	void parse_sprites(int scanline);
	void create_sprite_line_timer();
	void start_sprite_line_timer();
	virtual void set_sprite_region(uint8_t* region_sprites, uint32_t region_sprites_size);
	void set_fixed_regions(uint8_t* fix_cart, uint32_t fix_cart_size, memory_region* fix_bios);
	void set_pens(const pen_t* pens);

	std::unique_ptr<uint16_t[]>     m_videoram;
	uint16_t     *m_videoram_drawsource;

	uint16_t     m_vram_offset;
	uint16_t     m_vram_read_buffer;
	uint16_t     m_vram_modulo;

	uint32_t     m_sprite_gfx_address_mask;

	uint8_t      m_auto_animation_speed;
	uint8_t      m_auto_animation_disabled;
	uint8_t      m_auto_animation_counter;
	uint8_t      m_auto_animation_frame_counter;

	uint8_t      m_fixed_layer_source;
	uint8_t      m_fixed_layer_bank_type;

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
			uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	uint32_t get_region_mask(uint8_t* rgn, uint32_t rgn_size);
	uint8_t* m_region_sprites; uint32_t m_region_sprites_size;
	uint8_t* m_region_fixed; uint32_t m_region_fixed_size;
	memory_region* m_region_fixedbios;
	const pen_t   *m_pens;

	required_region_ptr<uint8_t> m_region_zoomy;
};


class neosprite_regular_device : public neosprite_base_device
{
public:
	neosprite_regular_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void draw_pixel(int romaddr, uint32_t* dst, const pen_t *line_pens) override;
	virtual void set_sprite_region(uint8_t* region_sprites, uint32_t region_sprites_size) override;

};

DECLARE_DEVICE_TYPE(NEOGEO_SPRITE_REGULAR, neosprite_regular_device)


class neosprite_optimized_device : public neosprite_base_device
{
public:
	neosprite_optimized_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void optimize_sprite_data() override;
	virtual void set_optimized_sprite_data(uint8_t* sprdata, uint32_t mask) override;
	virtual void draw_pixel(int romaddr, uint32_t* dst, const pen_t *line_pens) override;
	std::vector<uint8_t> m_sprite_gfx;
	uint8_t* m_spritegfx8;

private:
	uint32_t optimize_helper(std::vector<uint8_t> &spritegfx, uint8_t* region_sprites, uint32_t region_sprites_size);
};

DECLARE_DEVICE_TYPE(NEOGEO_SPRITE_OPTIMZIED, neosprite_optimized_device)


class neosprite_midas_device : public neosprite_base_device
{
public:
	neosprite_midas_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void draw_pixel(int romaddr, uint32_t* dst, const pen_t *line_pens) override;

	std::unique_ptr<uint16_t[]> m_videoram_buffer;
	void buffer_vram();
	virtual void draw_fixed_layer_2pixels(uint32_t*&pixel_addr, int offset, uint8_t* gfx_base, const pen_t* char_pens) override;
	virtual void set_sprite_region(uint8_t* region_sprites, uint32_t region_sprites_size) override;

	protected:
	virtual void device_start() override;

};

DECLARE_DEVICE_TYPE(NEOGEO_SPRITE_MIDAS, neosprite_midas_device)

#endif // MAME_VIDEO_NEOGEO_SPR_H
