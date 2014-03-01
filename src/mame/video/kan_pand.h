/*************************************************************************

    kan_pand.h

    Implementation of Kaneko Pandora sprite chip

**************************************************************************/

#ifndef __KAN_PAND_H__
#define __KAN_PAND_H__

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct kaneko_pandora_interface
{
	UINT8      m_gfx_region;
	int        m_xoffset;
	int        m_yoffset;
};

class kaneko_pandora_device : public device_t,
								public device_video_interface,
								public kaneko_pandora_interface
{
public:
	kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~kaneko_pandora_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);

	DECLARE_WRITE8_MEMBER ( spriteram_w );
	DECLARE_READ8_MEMBER( spriteram_r );
	DECLARE_WRITE16_MEMBER( spriteram_LSB_w );
	DECLARE_READ16_MEMBER( spriteram_LSB_r );
	void update( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void set_clear_bitmap( int clear );
	void eof();
	void set_bg_pen( int pen );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	void draw( bitmap_ind16 &bitmap, const rectangle &cliprect );

private:
	// internal state
	UINT8 *         m_spriteram;
	bitmap_ind16    *m_sprites_bitmap; /* bitmap to render sprites to, Pandora seems to be frame'buffered' */
	int             m_clear_bitmap;
	int             m_bg_pen; // might work some other way..
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

extern const device_type KANEKO_PANDORA;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_KANEKO_PANDORA_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, KANEKO_PANDORA, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_KANEKO_PANDORA_GFXDECODE(_gfxtag) \
	kaneko_pandora_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_KANEKO_PANDORA_PALETTE(_palette_tag) \
	kaneko_pandora_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif /* __KAN_PAND_H__ */
