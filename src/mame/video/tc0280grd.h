#ifndef _TC0280GRD_H_
#define _TC0280GRD_H_

struct tc0280grd_interface
{
	int                m_gfxnum;
};

class tc0280grd_device : public device_t,
							public tc0280grd_interface
{
public:
	tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0280grd_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);

	DECLARE_READ16_MEMBER( tc0280grd_word_r );
	DECLARE_WRITE16_MEMBER( tc0280grd_word_w );
	DECLARE_WRITE16_MEMBER( tc0280grd_ctrl_word_w );
	void tc0280grd_tilemap_update(int base_color);
	void tc0280grd_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority);

	DECLARE_READ16_MEMBER( tc0430grw_word_r );
	DECLARE_WRITE16_MEMBER( tc0430grw_word_w );
	DECLARE_WRITE16_MEMBER( tc0430grw_ctrl_word_w );
	void tc0430grw_tilemap_update(int base_color);
	void tc0430grw_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16 *       m_ram;

	tilemap_t      *m_tilemap;

	UINT16         m_ctrl[8];
	int            m_base_color;
	required_device<gfxdecode_device> m_gfxdecode;

	TILE_GET_INFO_MEMBER(tc0280grd_get_tile_info);
	void zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority, int xmultiply );
};

extern const device_type TC0280GRD;

#define TC0430GRW TC0280GRD

#define MCFG_TC0280GRD_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0280GRD, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0430GRW_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0430GRW, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0280GRD_GFXDECODE(_gfxtag) \
	tc0280grd_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_TC0430GRW_GFXDECODE(_gfxtag) \
	tc0280grd_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
#endif
