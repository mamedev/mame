#pragma once
#ifndef __K007342_H__
#define __K007342_H__

typedef void (*k007342_callback)(running_machine &machine, int tmap, int bank, int *code, int *color, int *flags);

struct k007342_interface
{
	int                m_gfxnum;
	k007342_callback   m_callback;
};

class k007342_device : public device_t,
										public k007342_interface
{
public:
	k007342_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k007342_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	
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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    *m_ram;
	UINT8    *m_scroll_ram;
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

	TILEMAP_MAPPER_MEMBER(scan);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	void get_tile_info( tile_data &tileinfo, int tile_index, int layer, UINT8 *cram, UINT8 *vram );
};

extern const device_type K007342;

#define MCFG_K007342_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K007342, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K007342_GFXDECODE(_gfxtag) \
	k007342_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#endif
