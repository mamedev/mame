// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#pragma once
#ifndef __K051960_H__
#define __K051960_H__

enum
{
	K051960_PLANEORDER_BASE = 0,
	K051960_PLANEORDER_MIA,
	K051960_PLANEORDER_GRADIUS3
};


typedef device_delegate<void (int *code, int *color, int *priority, int *shadow)> k051960_cb_delegate;
#define K051960_CB_MEMBER(_name)   void _name(int *code, int *color, int *priority, int *shadow)

#define MCFG_K051960_CB(_class, _method) \
	k051960_device::set_k051960_callback(*device, k051960_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K051960_PLANEORDER(_order) \
	k051960_device::set_plane_order(*device, _order);

#define MCFG_K051960_SCREEN_TAG(_tag) \
		k051960_device::set_screen_tag(*device, owner, _tag);

#define MCFG_K051960_IRQ_HANDLER(_devcb) \
	devcb = &k051960_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_K051960_NMI_HANDLER(_devcb) \
	devcb = &k051960_device::set_nmi_handler(*device, DEVCB_##_devcb);


class k051960_device : public device_t,
							public device_gfx_interface
{
	static const gfx_layout spritelayout;
	static const gfx_layout spritelayout_reverse;
	static const gfx_layout spritelayout_gradius3;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_reverse);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_gradius3);

public:
	k051960_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051960_device() {}

	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object)
		{ return downcast<k051960_device &>(device).m_irq_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_nmi_handler(device_t &device, _Object object)
		{ return downcast<k051960_device &>(device).m_nmi_handler.set_callback(object); }

	// static configuration
	static void set_k051960_callback(device_t &device, k051960_cb_delegate callback) { downcast<k051960_device &>(device).m_k051960_cb = callback; }
	static void set_plane_order(device_t &device, int order);
	static void set_screen_tag(device_t &device, device_t *owner, const char *tag);

	/*
	The callback is passed:
	- code (range 00-1FFF, output of the pins CA5-CA17)
	- color (range 00-FF, output of the pins OC0-OC7). Note that most of the
	  time COL7 seems to be "shadow", but not always (e.g. Aliens).
	The callback must put:
	- in code the resulting sprite number
	- in color the resulting color index
	- if necessary, in priority the priority of the sprite wrt tilemaps
	- if necessary, alter shadow to indicate whether the sprite has shadows enabled.
	  shadow is preloaded with color & 0x80 so it doesn't need to be changed unless
	  the game has special treatment (Aliens)
	*/

	DECLARE_READ8_MEMBER( k051960_r );
	DECLARE_WRITE8_MEMBER( k051960_w );

	DECLARE_READ8_MEMBER( k051937_r );
	DECLARE_WRITE8_MEMBER( k051937_w );

	void k051960_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int min_priority, int max_priority);

	TIMER_CALLBACK_MEMBER(scanline_callback);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT8    *m_ram;

	UINT8 *m_sprite_rom;
	UINT32 m_sprite_size;

	const char *m_screen_tag;
	screen_device *m_screen;
	emu_timer *m_scanline_timer;

	k051960_cb_delegate m_k051960_cb;

	devcb_write_line m_irq_handler;
	devcb_write_line m_firq_handler;
	devcb_write_line m_nmi_handler;

	UINT8    m_spriterombank[3];
	int      m_romoffset;
	int      m_spriteflip, m_readroms;
	int m_nmi_enabled;

	int k051960_fetchromdata( int byte );
};

extern const device_type K051960;

#endif
