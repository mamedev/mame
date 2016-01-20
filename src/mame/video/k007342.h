// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#pragma once
#ifndef __K007342_H__
#define __K007342_H__

typedef device_delegate<void (int layer, int bank, int *code, int *color, int *flags)> k007342_delegate;

class k007342_device : public device_t
{
public:
	k007342_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~k007342_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, std::string tag);
	static void static_set_gfxnum(device_t &device, int gfxnum) { downcast<k007342_device &>(device).m_gfxnum = gfxnum; }
	static void static_set_callback(device_t &device, k007342_delegate callback) { downcast<k007342_device &>(device).m_callback = callback; }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( scroll_r );
	DECLARE_WRITE8_MEMBER( scroll_w );
	DECLARE_WRITE8_MEMBER( vreg_w );

	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int num, int flags, UINT32 priority);
	int is_int_enabled();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	std::unique_ptr<UINT8[]>    m_ram;
	std::unique_ptr<UINT8[]>    m_scroll_ram;
	UINT8    *m_videoram_0;
	UINT8    *m_videoram_1;
	UINT8    *m_colorram_0;
	UINT8    *m_colorram_1;

	tilemap_t  *m_tilemap[2];
	int      m_flipscreen, m_int_enabled;
	UINT8    m_regs[8];
	UINT16   m_scrollx[2];
	UINT8    m_scrolly[2];
	required_device<gfxdecode_device> m_gfxdecode;
	k007342_delegate m_callback;
	int m_gfxnum;

	TILEMAP_MAPPER_MEMBER(scan);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	void get_tile_info( tile_data &tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram );
};

extern const device_type K007342;

#define MCFG_K007342_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, K007342, 0)
#define MCFG_K007342_GFXDECODE(_gfxtag) \
	k007342_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_K007342_GFXNUM(_gfxnum) \
	k007342_device::static_set_gfxnum(*device, _gfxnum);

#define MCFG_K007342_CALLBACK_OWNER(_class, _method) \
	k007342_device::static_set_callback(*device, k007342_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K007342_CALLBACK_DEVICE(_tag, _class, _method) \
	k007342_device::static_set_callback(*device, k007342_delegate(&_class::_method, #_class "::" #_method, _tag));

// function definition for a callback
#define K007342_CALLBACK_MEMBER(_name)     void _name(int layer, int bank, int *code, int *color, int *flags)

#endif
