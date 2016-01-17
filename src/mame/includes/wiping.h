// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
class wiping_state : public driver_device
{
public:
	wiping_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;

	int m_flipscreen;
	UINT8 *m_soundregs;  // if 0-ed
	UINT8 m_main_irq_mask;
	UINT8 m_sound_irq_mask;

	DECLARE_READ8_MEMBER(ports_r);
	DECLARE_WRITE8_MEMBER(subcpu_reset_w);
	DECLARE_WRITE8_MEMBER(main_irq_mask_w);
	DECLARE_WRITE8_MEMBER(sound_irq_mask_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);

	DECLARE_PALETTE_INIT(wiping);
	virtual void machine_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);
};
