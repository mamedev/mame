
#ifndef __K2GE_H_
#define __K2GE_H_

#include "devcb.h"


#define MCFG_K1GE_ADD(_tag, _clock, _screen, _vram, _vblank, _hblank ) \
	MCFG_DEVICE_ADD( _tag, K1GE, _clock ) \
	k1ge_device::static_set_screen( *device, _screen ); \
	k1ge_device::static_set_vram( *device, _vram ); \
	devcb = &k1ge_device::static_set_vblank_callback( *device, DEVCB2_##_vblank ); \
	devcb = &k1ge_device::static_set_hblank_callback( *device, DEVCB2_##_hblank );

#define MCFG_K2GE_ADD(_tag, _clock, _screen, _vram, _vblank, _hblank ) \
	MCFG_DEVICE_ADD( _tag, K2GE, _clock ) \
	k1ge_device::static_set_screen( *device, _screen ); \
	k1ge_device::static_set_vram( *device, _vram ); \
 	devcb = &k1ge_device::static_set_vblank_callback( *device, DEVCB2_##_vblank ); \
	devcb = &k1ge_device::static_set_hblank_callback( *device, DEVCB2_##_hblank );


class k1ge_device : public device_t
{
public:
	k1ge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	k1ge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	void update( bitmap_ind16 &bitmap, const rectangle &cliprect );

	// Static methods
	static void static_set_screen(device_t &device, const char *screen_name) { downcast<k1ge_device &>(device).m_screen_tag = screen_name; }
	static void static_set_vram(device_t &device, const char *vram_name) { downcast<k1ge_device &>(device).m_vram_tag = vram_name; }
    template<class _Object> static devcb2_base &static_set_vblank_callback(device_t &device, _Object object) { return downcast<k1ge_device &>(device).m_vblank_pin_w.set_callback(object); }
    template<class _Object> static devcb2_base &static_set_hblank_callback(device_t &device, _Object object) { return downcast<k1ge_device &>(device).m_hblank_pin_w.set_callback(object); }

	static const int K1GE_SCREEN_HEIGHT = 199;
protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	const char *m_screen_tag;
	const char *m_vram_tag;
	screen_device *m_screen;
	devcb2_write_line m_vblank_pin_w;
	devcb2_write_line m_hblank_pin_w;
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

protected:
	virtual void draw(int line);

	void draw_scroll_plane( UINT16 *p, UINT16 base, int line, int scroll_x, int scroll_y, UINT16 pal_base );
	void draw_sprite_plane( UINT16 *p, UINT16 priority, int line, int scroll_x, int scroll_y );
	void k1ge_draw_scroll_plane( UINT16 *p, UINT16 base, int line, int scroll_x, int scroll_y, UINT16 pal_lut_base, UINT16 k2ge_lut_base );
	void k1ge_draw_sprite_plane( UINT16 *p, UINT16 priority, int line, int scroll_x, int scroll_y );

};


PALETTE_INIT( k1ge );
PALETTE_INIT( k2ge );


extern const device_type K1GE;
extern const device_type K2GE;


#endif
