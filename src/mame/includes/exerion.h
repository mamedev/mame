// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Jaleco Exerion

*************************************************************************/


#define EXERION_MASTER_CLOCK      (XTAL_19_968MHz)   /* verified on pcb */
#define EXERION_CPU_CLOCK         (EXERION_MASTER_CLOCK / 6)
#define EXERION_AY8910_CLOCK      (EXERION_CPU_CLOCK / 2)
#define EXERION_PIXEL_CLOCK       (EXERION_MASTER_CLOCK / 3)
#define EXERION_HCOUNT_START      (0x58)
#define EXERION_HTOTAL            (512-EXERION_HCOUNT_START)
#define EXERION_HBEND             (12*8)    /* ?? */
#define EXERION_HBSTART           (52*8)    /* ?? */
#define EXERION_VTOTAL            (256)
#define EXERION_VBEND             (16)
#define EXERION_VBSTART           (240)


class exerion_state : public driver_device
{
public:
	exerion_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_main_ram(*this, "main_ram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_main_ram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	UINT8    m_cocktail_flip;
	UINT8    m_char_palette;
	UINT8    m_sprite_palette;
	UINT8    m_char_bank;
	std::unique_ptr<UINT16[]>  m_background_gfx[4];
	UINT8    *m_background_mixer;
	UINT8    m_background_latches[13];

	/* protection? */
	UINT8 m_porta;
	UINT8 m_portb;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(exerion_protection_r);
	DECLARE_WRITE8_MEMBER(exerion_videoreg_w);
	DECLARE_WRITE8_MEMBER(exerion_video_latch_w);
	DECLARE_READ8_MEMBER(exerion_video_timing_r);
	DECLARE_CUSTOM_INPUT_MEMBER(exerion_controls_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_READ8_MEMBER(exerion_porta_r);
	DECLARE_WRITE8_MEMBER(exerion_portb_w);
	DECLARE_DRIVER_INIT(exerion);
	DECLARE_DRIVER_INIT(exerionb);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(exerion);
	UINT32 screen_update_exerion(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect);
};
