class rollrace_state : public driver_device
{
public:
	rollrace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_colorram;
	int m_ra_charbank[2];
	int m_ra_bkgpage;
	int m_ra_bkgflip;
	int m_ra_chrbank;
	int m_ra_bkgpen;
	int m_ra_bkgcol;
	int m_ra_flipy;
	int m_ra_flipx;
	int m_ra_spritebank;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;

	UINT8 m_nmi_mask;
	UINT8 m_sound_nmi_mask;
	DECLARE_READ8_MEMBER(ra_fake_d800_r);
	DECLARE_WRITE8_MEMBER(ra_fake_d800_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(rollrace_charbank_w);
	DECLARE_WRITE8_MEMBER(rollrace_bkgpen_w);
	DECLARE_WRITE8_MEMBER(rollrace_spritebank_w);
	DECLARE_WRITE8_MEMBER(rollrace_backgroundpage_w);
	DECLARE_WRITE8_MEMBER(rollrace_backgroundcolor_w);
	DECLARE_WRITE8_MEMBER(rollrace_flipy_w);
	DECLARE_WRITE8_MEMBER(rollrace_flipx_w);
};


/*----------- defined in video/rollrace.c -----------*/

PALETTE_INIT( rollrace );
SCREEN_UPDATE_IND16( rollrace );


