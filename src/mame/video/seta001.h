
typedef int (*seta001_gfxbank_callback_func)(running_machine &machine, UINT16 code, UINT8 color);

class seta001_device : public device_t
{
public:
	seta001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


	void seta001_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size, int setac);

	void setac_eof( void );
	void tnzs_eof( void );

	UINT8 m_bgflag;

	UINT8 m_spritectrl[4];
	UINT8 m_spriteylow[0x300]; // 0x200 low y + 0x100 bg stuff

	UINT8 m_spritecodelow[0x2000]; // tnzs.c stuff only uses half?
	UINT8 m_spritecodehigh[0x2000]; // ^

	// position kludges for seta.c & srmp2.c
	void set_fg_xoffsets( int flip, int noflip ) { m_fg_flipxoffs = flip; m_fg_noflipxoffs = noflip; };
	int m_fg_flipxoffs, m_fg_noflipxoffs;
	void set_fg_yoffsets( int flip, int noflip ) { m_fg_flipyoffs = flip; m_fg_noflipyoffs = noflip; };
	int m_fg_flipyoffs, m_fg_noflipyoffs;
	void set_bg_yoffsets( int flip, int noflip ) { m_bg_flipyoffs = flip; m_bg_noflipyoffs = noflip; };
	int m_bg_flipyoffs, m_bg_noflipyoffs;
	void set_bg_xoffsets( int flip, int noflip ) { m_bg_flipxoffs = flip; m_bg_noflipxoffs = noflip; };
	int m_bg_flipxoffs, m_bg_noflipxoffs;

	void set_transpen ( int pen ) { m_transpen = pen; };
	int m_transpen;

	int is_flipped() { return ((m_spritectrl[ 0 ] & 0x40) >> 6); };

	void set_gfxbank_callback(seta001_gfxbank_callback_func callback) { m_bankcallback = callback; };
	seta001_gfxbank_callback_func m_bankcallback;

	void set_colorbase(int base) { m_colorbase = base; };
	int m_colorbase;

	void set_spritelimit(int limit) { m_spritelimit = limit; };
	int m_spritelimit;


protected:
	virtual void device_start();
	virtual void device_reset();
	private:

private:

	void seta001_draw_background( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size, int setac_type);
	void seta001_draw_foreground( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bank_size);

};

DECLARE_WRITE8_DEVICE_HANDLER( spritebgflag_w8 );


DECLARE_READ16_DEVICE_HANDLER( spritectrl_r16 );
DECLARE_WRITE16_DEVICE_HANDLER( spritectrl_w16 );
DECLARE_READ8_DEVICE_HANDLER( spritectrl_r8 );
DECLARE_WRITE8_DEVICE_HANDLER( spritectrl_w8 );

DECLARE_READ16_DEVICE_HANDLER( spriteylow_r16 );
DECLARE_WRITE16_DEVICE_HANDLER( spriteylow_w16 );
DECLARE_READ8_DEVICE_HANDLER( spriteylow_r8 );
DECLARE_WRITE8_DEVICE_HANDLER( spriteylow_w8 );

DECLARE_READ8_DEVICE_HANDLER( spritecodelow_r8 );
DECLARE_WRITE8_DEVICE_HANDLER( spritecodelow_w8 );
DECLARE_READ8_DEVICE_HANDLER( spritecodehigh_r8 );
DECLARE_WRITE8_DEVICE_HANDLER( spritecodehigh_w8 );
DECLARE_READ16_DEVICE_HANDLER( spritecode_r16 );
DECLARE_WRITE16_DEVICE_HANDLER( spritecode_w16 );

extern const device_type SETA001_SPRITE;
