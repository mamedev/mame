// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Pinball Action

*************************************************************************/

class pbaction_state : public driver_device
{
public:
	pbaction_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_colorram(*this, "colorram"),
		m_colorram2(*this, "colorram2"),
		m_work_ram(*this, "work_ram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_work_ram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	int        m_scroll;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT8> m_decrypted_opcodes;

	UINT8      m_nmi_mask;
	DECLARE_WRITE8_MEMBER(pbaction_sh_command_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_READ8_MEMBER(pbactio3_prot_kludge_r);
	DECLARE_WRITE8_MEMBER(pbaction_videoram_w);
	DECLARE_WRITE8_MEMBER(pbaction_colorram_w);
	DECLARE_WRITE8_MEMBER(pbaction_videoram2_w);
	DECLARE_WRITE8_MEMBER(pbaction_colorram2_w);
	DECLARE_WRITE8_MEMBER(pbaction_scroll_w);
	DECLARE_WRITE8_MEMBER(pbaction_flipscreen_w);
	DECLARE_DRIVER_INIT(pbactio3);
	DECLARE_DRIVER_INIT(pbactio4);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_pbaction(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pbaction_interrupt);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
