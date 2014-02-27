#pragma once
#ifndef __K051960_H__
#define __K051960_H__

typedef void (*k051960_callback)(running_machine &machine, int *code, int *color, int *priority, int *shadow);

struct k051960_interface
{
	const char         *m_gfx_memory_region;
	int                m_gfx_num;
	int                m_plane_order;
	int                m_deinterleave;
	k051960_callback   m_callback;
};

class k051960_device : public device_t,
										public k051960_interface
{
public:
	k051960_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051960_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);

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
	DECLARE_READ16_MEMBER( k051960_word_r );
	DECLARE_WRITE16_MEMBER( k051960_word_w );

	DECLARE_READ8_MEMBER( k051937_r );
	DECLARE_WRITE8_MEMBER( k051937_w );
	DECLARE_READ16_MEMBER( k051937_word_r );
	DECLARE_WRITE16_MEMBER( k051937_word_w );

	void k051960_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int min_priority, int max_priority);
	int k051960_is_irq_enabled();
	int k051960_is_nmi_enabled();
	void k051960_set_sprite_offsets(int dx, int dy);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    *m_ram;

	gfx_element *m_gfx;

	UINT8    m_spriterombank[3];
	int      m_dx, m_dy;
	int      m_romoffset;
	int      m_spriteflip, m_readroms;
	int      m_irq_enabled, m_nmi_enabled;

	int      m_k051937_counter;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int k051960_fetchromdata( int byte );
};

extern const device_type K051960;

#define MCFG_K051960_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K051960, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K051960_GFXDECODE(_gfxtag) \
	k051960_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_K051960_PALETTE(_palette_tag) \
	k051960_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
