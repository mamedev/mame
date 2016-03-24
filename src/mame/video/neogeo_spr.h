// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
#define VERBOSE     (0)

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
class neosprite_base_device : public device_t
{
public:
	neosprite_base_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock,  device_type type);
//  neosprite_base_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void optimize_sprite_data();
	virtual void set_optimized_sprite_data(UINT8* sprdata, UINT32 mask);

	virtual void draw_fixed_layer_2pixels(UINT32*&pixel_addr, int offset, UINT8* gfx_base, const pen_t* char_pens);
	void draw_fixed_layer( bitmap_rgb32 &bitmap, int scanline );
	void set_videoram_offset( UINT16 data );
	UINT16 get_videoram_data(  );
	void set_videoram_data( UINT16 data);
	void set_videoram_modulo( UINT16 data);
	UINT16 get_videoram_modulo(  );
	void set_auto_animation_speed( UINT8 data);
	void set_auto_animation_disabled( UINT8 data);
	UINT8 neogeo_get_auto_animation_counter(  );
	void create_auto_animation_timer(  );
	void start_auto_animation_timer(  );
	void neogeo_set_fixed_layer_source( UINT8 data );
	inline bool sprite_on_scanline(int scanline, int y, int rows);
	virtual void draw_pixel(int romaddr, UINT32* dst, const pen_t *line_pens) = 0;
	void draw_sprites( bitmap_rgb32 &bitmap, int scanline );
	void parse_sprites( int scanline );
	void create_sprite_line_timer(  );
	void start_sprite_line_timer(  );
	virtual void set_sprite_region(UINT8* region_sprites, UINT32 region_sprites_size);
	void set_fixed_regions(UINT8* fix_cart, UINT32 fix_cart_size, memory_region* fix_bios);
	void set_screen(screen_device* screen);
	void set_pens(const pen_t* pens);

	std::unique_ptr<UINT16[]>     m_videoram;
	UINT16     *m_videoram_drawsource;

	UINT16     m_vram_offset;
	UINT16     m_vram_read_buffer;
	UINT16     m_vram_modulo;

	const UINT8 *m_region_zoomy;

	UINT32     m_sprite_gfx_address_mask;

	UINT8      m_auto_animation_speed;
	UINT8      m_auto_animation_disabled;
	UINT8      m_auto_animation_counter;
	UINT8      m_auto_animation_frame_counter;

	UINT8      m_fixed_layer_source;
	UINT8      m_fixed_layer_bank_type;

	emu_timer  *m_auto_animation_timer;
	emu_timer  *m_sprite_line_timer;

	TIMER_CALLBACK_MEMBER(auto_animation_timer_callback);
	TIMER_CALLBACK_MEMBER(sprite_line_timer_callback);


	int m_bppshift; // 4 for 4bpp gfx (NeoGeo) 8 for 8bpp gfx (Midas)

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	UINT32 get_region_mask(UINT8* rgn, UINT32 rgn_size);
	UINT8* m_region_sprites; UINT32 m_region_sprites_size;
	UINT8* m_region_fixed; UINT32 m_region_fixed_size;
	memory_region* m_region_fixedbios;
	screen_device* m_screen;
	const pen_t   *m_pens;

private:

};

//extern const device_type NEOGEO_SPRITE_BASE;


class neosprite_regular_device : public neosprite_base_device
{
public:
	neosprite_regular_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void draw_pixel(int romaddr, UINT32* dst, const pen_t *line_pens) override;
	virtual void set_sprite_region(UINT8* region_sprites, UINT32 region_sprites_size) override;

};

extern const device_type NEOGEO_SPRITE_REGULAR;


class neosprite_optimized_device : public neosprite_base_device
{
public:
	neosprite_optimized_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void optimize_sprite_data() override;
	virtual void set_optimized_sprite_data(UINT8* sprdata, UINT32 mask) override;
	virtual void draw_pixel(int romaddr, UINT32* dst, const pen_t *line_pens) override;
	std::vector<UINT8> m_sprite_gfx;
	UINT8* m_spritegfx8;

};

extern const device_type NEOGEO_SPRITE_OPTIMZIED;





class neosprite_midas_device : public neosprite_base_device
{
public:
	neosprite_midas_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void draw_pixel(int romaddr, UINT32* dst, const pen_t *line_pens) override;

	std::unique_ptr<UINT16[]> m_videoram_buffer;
	void buffer_vram();
	virtual void draw_fixed_layer_2pixels(UINT32*&pixel_addr, int offset, UINT8* gfx_base, const pen_t* char_pens) override;
	virtual void set_sprite_region(UINT8* region_sprites, UINT32 region_sprites_size) override;

	protected:
	virtual void device_start() override;

};

extern const device_type NEOGEO_SPRITE_MIDAS;
