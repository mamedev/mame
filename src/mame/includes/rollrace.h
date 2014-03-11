class rollrace_state : public driver_device
{
public:
	rollrace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	int m_ra_charbank[2];
	int m_ra_bkgpage;
	int m_ra_bkgflip;
	int m_ra_chrbank;
	int m_ra_bkgpen;
	int m_ra_bkgcol;
	int m_ra_flipy;
	int m_ra_flipx;
	int m_ra_spritebank;
	required_shared_ptr<UINT8> m_spriteram;

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
	DECLARE_PALETTE_INIT(rollrace);
	UINT32 screen_update_rollrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
