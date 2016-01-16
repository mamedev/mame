// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood

typedef device_delegate<int (UINT16 code, UINT8 color)> gfxbank_cb_delegate;

#define SETA001_SPRITE_GFXBANK_CB_MEMBER(_name) int _name(UINT16 code, UINT8 color)

#define MCFG_SETA001_SPRITE_GFXBANK_CB(_class, _method) \
	seta001_device::set_gfxbank_callback(*device, gfxbank_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

class seta001_device : public device_t
{
public:
	seta001_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, std::string tag);
	static void static_set_palette_tag(device_t &device, std::string tag);
	static void set_gfxbank_callback(device_t &device, gfxbank_cb_delegate callback) { downcast<seta001_device &>(device).m_gfxbank_cb = callback; }

	DECLARE_WRITE8_MEMBER( spritebgflag_w8 );

	DECLARE_READ16_MEMBER( spritectrl_r16 );
	DECLARE_WRITE16_MEMBER( spritectrl_w16 );
	DECLARE_READ8_MEMBER( spritectrl_r8 );
	DECLARE_WRITE8_MEMBER( spritectrl_w8 );

	DECLARE_READ16_MEMBER( spriteylow_r16 );
	DECLARE_WRITE16_MEMBER( spriteylow_w16 );
	DECLARE_READ8_MEMBER( spriteylow_r8 );
	DECLARE_WRITE8_MEMBER( spriteylow_w8 );

	DECLARE_READ8_MEMBER( spritecodelow_r8 );
	DECLARE_WRITE8_MEMBER( spritecodelow_w8 );
	DECLARE_READ8_MEMBER( spritecodehigh_r8 );
	DECLARE_WRITE8_MEMBER( spritecodehigh_w8 );
	DECLARE_READ16_MEMBER( spritecode_r16 );
	DECLARE_WRITE16_MEMBER( spritecode_w16 );

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size, int setac);

	void setac_eof( void );
	void tnzs_eof( void );

	// position kludges for seta.c & srmp2.c
	void set_fg_xoffsets( int flip, int noflip ) { m_fg_flipxoffs = flip; m_fg_noflipxoffs = noflip; };
	void set_fg_yoffsets( int flip, int noflip ) { m_fg_flipyoffs = flip; m_fg_noflipyoffs = noflip; };
	void set_bg_yoffsets( int flip, int noflip ) { m_bg_flipyoffs = flip; m_bg_noflipyoffs = noflip; };
	void set_bg_xoffsets( int flip, int noflip ) { m_bg_flipxoffs = flip; m_bg_noflipxoffs = noflip; };

	void set_colorbase(int base) { m_colorbase = base; };
	void set_spritelimit(int limit) { m_spritelimit = limit; };
	void set_transpen ( int pen ) { m_transpen = pen; };

	int is_flipped() { return ((m_spritectrl[ 0 ] & 0x40) >> 6); };

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size, int setac_type);
	void draw_foreground( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size);
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	gfxbank_cb_delegate m_gfxbank_cb;

	// configuration
	int m_fg_flipxoffs, m_fg_noflipxoffs;
	int m_fg_flipyoffs, m_fg_noflipyoffs;
	int m_bg_flipyoffs, m_bg_noflipyoffs;
	int m_bg_flipxoffs, m_bg_noflipxoffs;
	int m_colorbase;
	int m_spritelimit;
	int m_transpen;

	// live state
	UINT8 m_bgflag;
	UINT8 m_spritectrl[4];
	UINT8 m_spriteylow[0x300]; // 0x200 low y + 0x100 bg stuff
	UINT8 m_spritecodelow[0x2000]; // tnzs.c stuff only uses half?
	UINT8 m_spritecodehigh[0x2000]; // ^
};

extern const device_type SETA001_SPRITE;

#define MCFG_SETA001_SPRITE_GFXDECODE(_gfxtag) \
	seta001_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_SETA001_SPRITE_PALETTE(_palette_tag) \
	seta001_device::static_set_palette_tag(*device, "^" _palette_tag);
