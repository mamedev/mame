// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

  K1GE/K2GE graphics emulation

******************************************************************************/

#ifndef __K2GE_H_
#define __K2GE_H_


#define MCFG_K1GE_ADD(_tag, _clock, _screen, _vblank, _hblank ) \
	MCFG_DEVICE_ADD( _tag, K1GE, _clock ) \
	MCFG_VIDEO_SET_SCREEN( _screen ) \
	devcb = &k1ge_device::static_set_vblank_callback( *device, DEVCB_##_vblank ); \
	devcb = &k1ge_device::static_set_hblank_callback( *device, DEVCB_##_hblank );

#define MCFG_K2GE_ADD(_tag, _clock, _screen, _vblank, _hblank ) \
	MCFG_DEVICE_ADD( _tag, K2GE, _clock ) \
	MCFG_VIDEO_SET_SCREEN( _screen ) \
	devcb = &k1ge_device::static_set_vblank_callback( *device, DEVCB_##_vblank ); \
	devcb = &k1ge_device::static_set_hblank_callback( *device, DEVCB_##_hblank );


class k1ge_device : public device_t,
					public device_video_interface
{
public:
	k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	k1ge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void palette_init_k1ge(palette_device &palette);

	void update( bitmap_ind16 &bitmap, const rectangle &cliprect );

	// Static methods
	template<class _Object> static devcb_base &static_set_vblank_callback(device_t &device, _Object object) { return downcast<k1ge_device &>(device).m_vblank_pin_w.set_callback(object); }
	template<class _Object> static devcb_base &static_set_hblank_callback(device_t &device, _Object object) { return downcast<k1ge_device &>(device).m_hblank_pin_w.set_callback(object); }

	static const int K1GE_SCREEN_HEIGHT = 199;
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	devcb_write_line m_vblank_pin_w;
	devcb_write_line m_hblank_pin_w;
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t m_wba_h, m_wba_v, m_wsi_h, m_wsi_v;

	emu_timer *m_timer;
	emu_timer *m_hblank_on_timer;
	std::unique_ptr<bitmap_ind16> m_bitmap;

	virtual void draw(int line);

	void draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, int pal_base );
	void draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y );
	void hblank_on_timer_callback(void *ptr, int32_t param);
	void timer_callback(void *ptr, int32_t param);

};


class k2ge_device : public k1ge_device
{
public:
	k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void palette_init_k2ge(palette_device &palette);
protected:
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual void draw(int line) override;

	void draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, uint16_t pal_base );
	void draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y );
	void k1ge_draw_scroll_plane( uint16_t *p, uint16_t base, int line, int scroll_x, int scroll_y, uint16_t pal_lut_base, uint16_t k2ge_lut_base );
	void k1ge_draw_sprite_plane( uint16_t *p, uint16_t priority, int line, int scroll_x, int scroll_y );

};

extern const device_type K1GE;
extern const device_type K2GE;


#endif
