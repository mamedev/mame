// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
class rollrace_state : public driver_device
{
public:
	rollrace_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	int m_charbank[2];
	int m_bkgpage;
	int m_bkgflip;
	int m_chrbank;
	int m_bkgpen;
	int m_bkgcol;
	int m_flipy;
	int m_flipx;
	int m_spritebank;

	UINT8 m_nmi_mask;
	UINT8 m_sound_nmi_mask;

	DECLARE_READ8_MEMBER(fake_d800_r);
	DECLARE_WRITE8_MEMBER(fake_d800_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(charbank_w);
	DECLARE_WRITE8_MEMBER(bkgpen_w);
	DECLARE_WRITE8_MEMBER(spritebank_w);
	DECLARE_WRITE8_MEMBER(backgroundpage_w);
	DECLARE_WRITE8_MEMBER(backgroundcolor_w);
	DECLARE_WRITE8_MEMBER(flipy_w);
	DECLARE_WRITE8_MEMBER(flipx_w);

	DECLARE_PALETTE_INIT(rollrace);
	virtual void machine_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);
};
