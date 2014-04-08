/* */

#pragma once
#ifndef __K053244_K053245_H__
#define __K053244_K053245_H__



#define NORMAL_PLANE_ORDER 0x0123

typedef void (*k05324x_callback)(running_machine &machine, int *code, int *color, int *priority);


struct k05324x_interface
{
	const char         *m_gfx_memory_region;
	int                m_gfx_num;
	int                m_plane_order;
	int                m_dx, m_dy;
	int                m_deinterleave;
	k05324x_callback   m_callback;
};


class k05324x_device : public device_t,
										public k05324x_interface
{
public:
	k05324x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k05324x_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);

	DECLARE_READ16_MEMBER( k053245_word_r );
	DECLARE_WRITE16_MEMBER( k053245_word_w );
	DECLARE_READ8_MEMBER( k053245_r );
	DECLARE_WRITE8_MEMBER( k053245_w );
	DECLARE_READ8_MEMBER( k053244_r );
	DECLARE_WRITE8_MEMBER( k053244_w );
	DECLARE_READ16_MEMBER( k053244_lsb_r );
	DECLARE_WRITE16_MEMBER( k053244_lsb_w );
	DECLARE_READ16_MEMBER( k053244_word_r );
	DECLARE_WRITE16_MEMBER( k053244_word_w );
	void k053244_bankselect(int bank);    /* used by TMNT2, Asterix and Premier Soccer for ROM testing */
	void k053245_sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);
	void k053245_sprites_draw_lethal(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap); /* for lethal enforcers */
	void k053245_clear_buffer();
	void k053245_update_buffer();
	void k053245_set_sprite_offs(int offsx, int offsy);
	void k05324x_set_z_rejection(int zcode); // common to k053244/5

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT16    *m_ram;
	UINT16    *m_buffer;

	gfx_element *m_gfx;

	UINT8    m_regs[0x10];    // 053244
	int      m_rombank;       // 053244
	int      m_ramsize;
	int      m_z_rejection;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ16_MEMBER( k053244_reg_word_r );    // OBJSET0 debug handler
};



extern const device_type K053244;

#define K053245 K053244



#define MCFG_K053244_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053244, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053244_GFXDECODE(_gfxtag) \
	k05324x_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_K053244_PALETTE(_palette_tag) \
	k05324x_device::static_set_palette_tag(*device, "^" _palette_tag);

#define MCFG_K053245_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053245, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053245_GFXDECODE(_gfxtag) \
	k05324x_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_K053245_PALETTE(_palette_tag) \
	k05324x_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
