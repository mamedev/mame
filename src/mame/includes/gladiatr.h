class gladiatr_state : public driver_device
{
public:
	gladiatr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_nvram(*this, "nvram") ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_textram(*this, "textram"){ }

	required_shared_ptr<UINT8>	m_nvram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_textram;

	int m_data1;
	int m_data2;
	int m_flag1;
	int m_flag2;
	int m_video_attributes;
	int m_fg_scrollx;
	int m_fg_scrolly;
	int m_bg_scrollx;
	int m_bg_scrolly;
	int m_sprite_bank;
	int m_sprite_buffer;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_fg_tile_bank;
	int m_bg_tile_bank;

	DECLARE_WRITE8_MEMBER(gladiatr_videoram_w);
	DECLARE_WRITE8_MEMBER(gladiatr_colorram_w);
	DECLARE_WRITE8_MEMBER(gladiatr_textram_w);
	DECLARE_WRITE8_MEMBER(gladiatr_paletteram_w);
	DECLARE_WRITE8_MEMBER(gladiatr_spritebuffer_w);
	DECLARE_WRITE8_MEMBER(gladiatr_spritebank_w);
	DECLARE_WRITE8_MEMBER(ppking_video_registers_w);
	DECLARE_WRITE8_MEMBER(gladiatr_video_registers_w);
	DECLARE_WRITE8_MEMBER(gladiatr_bankswitch_w);
	DECLARE_WRITE8_MEMBER(glad_cpu_sound_command_w);
	DECLARE_READ8_MEMBER(glad_cpu_sound_command_r);
	DECLARE_WRITE8_MEMBER(gladiatr_flipscreen_w);
	DECLARE_WRITE8_MEMBER(gladiatr_irq_patch_w);
	DECLARE_WRITE8_MEMBER(qx0_w);
	DECLARE_WRITE8_MEMBER(qx1_w);
	DECLARE_WRITE8_MEMBER(qx2_w);
	DECLARE_WRITE8_MEMBER(qx3_w);
	DECLARE_READ8_MEMBER(qx2_r);
	DECLARE_READ8_MEMBER(qx3_r);
	DECLARE_READ8_MEMBER(qx0_r);
	DECLARE_READ8_MEMBER(qx1_r);
	DECLARE_READ8_MEMBER(f6a3_r);
	DECLARE_WRITE8_MEMBER(gladiator_int_control_w);
	DECLARE_WRITE8_MEMBER(glad_adpcm_w);
	DECLARE_READ8_MEMBER(f1_r);
	DECLARE_DRIVER_INIT(gladiatr);
	DECLARE_DRIVER_INIT(ppking);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	DECLARE_MACHINE_RESET(ppking);
	DECLARE_VIDEO_START(ppking);
	DECLARE_MACHINE_RESET(gladiator);
	DECLARE_VIDEO_START(gladiatr);
	UINT32 screen_update_ppking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gladiatr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
