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
	k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	k1ge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_PALETTE_INIT(k1ge);

	void update( bitmap_ind16 &bitmap, const rectangle &cliprect );

	// Static methods
	template<class _Object> static devcb_base &static_set_vblank_callback(device_t &device, _Object object) { return downcast<k1ge_device &>(device).m_vblank_pin_w.set_callback(object); }
	template<class _Object> static devcb_base &static_set_hblank_callback(device_t &device, _Object object) { return downcast<k1ge_device &>(device).m_hblank_pin_w.set_callback(object); }

	static const int K1GE_SCREEN_HEIGHT = 199;
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	devcb_write_line m_vblank_pin_w;
	devcb_write_line m_hblank_pin_w;
	UINT8 *m_vram;
	UINT8 m_wba_h, m_wba_v, m_wsi_h, m_wsi_v;

	emu_timer *m_timer;
	emu_timer *m_hblank_on_timer;
	bitmap_ind16 *m_bitmap;

	virtual void draw(int line);

	void draw_scroll_plane( UINT16 *p, UINT16 base, int line, int scroll_x, int scroll_y, int pal_base );
	void draw_sprite_plane( UINT16 *p, UINT16 priority, int line, int scroll_x, int scroll_y );
	TIMER_CALLBACK_MEMBER( hblank_on_timer_callback );
	TIMER_CALLBACK_MEMBER( timer_callback );

};


class k2ge_device : public k1ge_device
{
public:
	k2ge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_PALETTE_INIT(k2ge);
protected:
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void draw(int line);

	void draw_scroll_plane( UINT16 *p, UINT16 base, int line, int scroll_x, int scroll_y, UINT16 pal_base );
	void draw_sprite_plane( UINT16 *p, UINT16 priority, int line, int scroll_x, int scroll_y );
	void k1ge_draw_scroll_plane( UINT16 *p, UINT16 base, int line, int scroll_x, int scroll_y, UINT16 pal_lut_base, UINT16 k2ge_lut_base );
	void k1ge_draw_sprite_plane( UINT16 *p, UINT16 priority, int line, int scroll_x, int scroll_y );

};

extern const device_type K1GE;
extern const device_type K2GE;


#endif
