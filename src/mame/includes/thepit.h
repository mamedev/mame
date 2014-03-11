class thepit_state : public driver_device
{
public:
	thepit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_attributesram(*this, "attributesram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	int m_question_address;
	int m_question_rom;
	int m_remap_address[16];
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_attributesram;
	required_shared_ptr<UINT8> m_spriteram;
	UINT8 m_graphics_bank;
	UINT8 m_flip_screen_x;
	UINT8 m_flip_screen_y;
	tilemap_t *m_solid_tilemap;
	tilemap_t *m_tilemap;
	UINT8 *m_dummy_tile;
	UINT8 m_nmi_mask;
	DECLARE_READ8_MEMBER(thepit_colorram_r);
	DECLARE_WRITE8_MEMBER(thepit_sound_enable_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_READ8_MEMBER(rtriv_question_r);
	DECLARE_WRITE8_MEMBER(thepit_videoram_w);
	DECLARE_WRITE8_MEMBER(thepit_colorram_w);
	DECLARE_WRITE8_MEMBER(thepit_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(thepit_flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(intrepid_graphics_bank_w);
	DECLARE_READ8_MEMBER(thepit_input_port_0_r);
	DECLARE_DRIVER_INIT(rtriv);
	TILE_GET_INFO_MEMBER(solid_get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(thepit);
	DECLARE_PALETTE_INIT(suprmous);
	UINT32 screen_update_thepit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_desertdan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
