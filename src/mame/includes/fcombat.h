// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina


/* this is compied from Exerion, but it should be correct */
#define FCOMBAT_MASTER_CLOCK        (20000000)
#define FCOMBAT_CPU_CLOCK           (FCOMBAT_MASTER_CLOCK / 6)
#define FCOMBAT_AY8910_CLOCK        (FCOMBAT_CPU_CLOCK / 2)
#define FCOMBAT_PIXEL_CLOCK         (FCOMBAT_MASTER_CLOCK / 3)
#define FCOMBAT_HCOUNT_START        (0x58)
#define FCOMBAT_HTOTAL              (512-FCOMBAT_HCOUNT_START)
#define FCOMBAT_HBEND               (12*8)  /* ?? */
#define FCOMBAT_HBSTART             (52*8)  /* ?? */
#define FCOMBAT_VTOTAL              (256)
#define FCOMBAT_VBEND               (16)
#define FCOMBAT_VBSTART             (240)

#define BACKGROUND_X_START      32
#define BACKGROUND_X_START_FLIP 72

#define VISIBLE_X_MIN           (12*8)
#define VISIBLE_X_MAX           (52*8)
#define VISIBLE_Y_MIN           (2*8)
#define VISIBLE_Y_MAX           (30*8)


class fcombat_state : public driver_device
{
public:
	fcombat_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bgmap;
	UINT8      m_cocktail_flip;
	UINT8      m_char_palette;
	UINT8      m_sprite_palette;
	UINT8      m_char_bank;

	/* misc */
	int        m_fcombat_sh;
	int        m_fcombat_sv;
	int        m_tx;
	int        m_ty;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ8_MEMBER(fcombat_protection_r);
	DECLARE_READ8_MEMBER(fcombat_port01_r);
	DECLARE_WRITE8_MEMBER(e900_w);
	DECLARE_WRITE8_MEMBER(ea00_w);
	DECLARE_WRITE8_MEMBER(eb00_w);
	DECLARE_WRITE8_MEMBER(ec00_w);
	DECLARE_WRITE8_MEMBER(ed00_w);
	DECLARE_READ8_MEMBER(e300_r);
	DECLARE_WRITE8_MEMBER(ee00_w);
	DECLARE_WRITE8_MEMBER(fcombat_videoreg_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(fcombat);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(fcombat);
	UINT32 screen_update_fcombat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
