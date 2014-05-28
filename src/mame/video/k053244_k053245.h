#pragma once
#ifndef __K053244_K053245_H__
#define __K053244_K053245_H__



#define NORMAL_PLANE_ORDER 0x0123


typedef device_delegate<void (int *code, int *color, int *priority)> k05324x_cb_delegate;
#define MCFG_K05324X_CB_MEMBER(_name)   void _name(int *code, int *color, int *priority)


#define MCFG_K05324X_CB(_class, _method) \
	k05324x_device::set_k05324x_callback(*device, k05324x_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K05324X_GFX(_tag) \
	k05324x_device::set_gfx_tag(*device, _tag);

#define MCFG_K05324X_OFFSETS(_xoffs, _yoffs) \
	k05324x_device::set_offsets(*device, _xoffs, _yoffs);

#define MCFG_K05324X_ORDER(_order) \
	k05324x_device::set_plane_order(*device, _order);

#define MCFG_K05324X_DEINTERLEAVE(_deinterleave) \
	k05324x_device::set_deinterleave(*device, _deinterleave);


class k05324x_device : public device_t, 
							public device_gfx_interface
{
public:
	k05324x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k05324x_device() {}

	static const gfx_layout spritelayout;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);

	static void set_k05324x_callback(device_t &device, k05324x_cb_delegate callback) { downcast<k05324x_device &>(device).m_k05324x_cb = callback; }
	static void set_gfx_tag(device_t &device, const char *tag) { downcast<k05324x_device &>(device).m_gfx_tag = tag; }
	static void set_plane_order(device_t &device, int plane_order) { downcast<k05324x_device &>(device).m_plane_order = plane_order; }
	static void set_deinterleave(device_t &device, int deinterleave) { downcast<k05324x_device &>(device).m_deinterleave = deinterleave; }
	static void set_offsets(device_t &device, int x_offset, int y_offset)
	{
		k05324x_device &dev = downcast<k05324x_device &>(device);
		dev.m_dx = x_offset;
		dev.m_dy = y_offset;
	}

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
	void bankselect(int bank);    /* used by TMNT2, Asterix and Premier Soccer for ROM testing */
	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap);
	void sprites_draw_lethal(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap); /* for lethal enforcers */
	void clear_buffer();
	void update_buffer();
	void set_z_rejection(int zcode); // common to k053244/5

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16    *m_ram;
	UINT16    *m_buffer;
	UINT8     *m_sprite_rom;
	UINT32    m_sprite_size;

	const char *m_gfx_tag;

	int m_dx, m_dy;
	int m_plane_order;
	int m_deinterleave;
	k05324x_cb_delegate m_k05324x_cb;

	UINT8    m_regs[0x10];    // 053244
	int      m_rombank;       // 053244
	int      m_ramsize;
	int      m_z_rejection;

	DECLARE_READ16_MEMBER( k053244_reg_word_r );    // OBJSET0 debug handler
};


extern const device_type K053244;
#define K053245 K053244

#endif
