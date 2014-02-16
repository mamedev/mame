#ifndef _TC0180VCU_H_
#define _TC0180VCU_H_

struct tc0180vcu_interface
{
	int            m_bg_color_base;
	int            m_fg_color_base;
	int            m_tx_color_base;
};

class tc0180vcu_device : public device_t,
											public tc0180vcu_interface
{
public:
	tc0180vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0180vcu_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);

	DECLARE_READ8_MEMBER( get_fb_page );
	DECLARE_WRITE8_MEMBER( set_fb_page );
	DECLARE_READ8_MEMBER( get_videoctrl );
	DECLARE_READ16_MEMBER( ctrl_r );
	DECLARE_WRITE16_MEMBER( ctrl_w );
	DECLARE_READ16_MEMBER( scroll_r );
	DECLARE_WRITE16_MEMBER( scroll_w );
	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, int plane);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	private:
	// internal state
	UINT16         m_ctrl[0x10];

	UINT16 *       m_ram;
	UINT16 *       m_scrollram;

	tilemap_t      *m_tilemap[3];

	UINT16         m_bg_rambank[2], m_fg_rambank[2], m_tx_rambank;
	UINT8          m_framebuffer_page;
	UINT8          m_video_control;
	required_device<gfxdecode_device> m_gfxdecode;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void video_control( UINT8 data );
};

extern const device_type TC0180VCU;

#define MCFG_TC0180VCU_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0180VCU, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0180VCU_GFXDECODE(_gfxtag) \
	tc0180vcu_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
#endif
