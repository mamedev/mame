/*************************************************************************

    Skyfox

*************************************************************************/

class skyfox_state : public driver_device
{
public:
	skyfox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	UINT8      m_vreg[8];
	int        m_bg_pos;
	int        m_bg_ctrl;

	/* misc */
	int        m_palette_selected;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(skyfox_vregs_r);
	DECLARE_WRITE8_MEMBER(skyfox_vregs_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(skyfox);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(skyfox);
	UINT32 screen_update_skyfox(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(skyfox_interrupt);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
