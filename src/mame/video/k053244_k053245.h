// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once
#ifndef __K053244_K053245_H__
#define __K053244_K053245_H__


typedef device_delegate<void (int *code, int *color, int *priority)> k05324x_cb_delegate;
#define K05324X_CB_MEMBER(_name)   void _name(int *code, int *color, int *priority)


#define MCFG_K05324X_BPP(_bpp) \
	k05324x_device::set_bpp(*device, _bpp);

#define MCFG_K05324X_CB(_class, _method) \
	k05324x_device::set_k05324x_callback(*device, k05324x_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K05324X_OFFSETS(_xoffs, _yoffs) \
	k05324x_device::set_offsets(*device, _xoffs, _yoffs);


class k05324x_device : public device_t,
							public device_gfx_interface
{
	static const gfx_layout spritelayout;
	static const gfx_layout spritelayout_6bpp;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_6bpp);

public:
	k05324x_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~k05324x_device() {}

	// static configuration
	static void set_bpp(device_t &device, int bpp);
	static void set_k05324x_callback(device_t &device, k05324x_cb_delegate callback) { downcast<k05324x_device &>(device).m_k05324x_cb = callback; }
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
	void clear_buffer();
	void update_buffer();
	void set_z_rejection(int zcode); // common to k053244/5

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<UINT16[]>  m_ram;
	std::unique_ptr<UINT16[]>  m_buffer;
	UINT8     *m_sprite_rom;
	UINT32    m_sprite_size;

	int m_dx, m_dy;
	k05324x_cb_delegate m_k05324x_cb;

	UINT8    m_regs[0x10];    // 053244
	int      m_rombank;       // 053244
	int      m_ramsize;
	int      m_z_rejection;
};


extern const device_type K053244;
#define K053245 K053244

#endif
