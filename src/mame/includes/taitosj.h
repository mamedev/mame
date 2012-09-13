class taitosj_state : public driver_device
{
public:
	taitosj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram_1(*this, "videoram_1"),
		m_videoram_2(*this, "videoram_2"),
		m_videoram_3(*this, "videoram_3"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_characterram(*this, "characterram"),
		m_scroll(*this, "scroll"),
		m_colscrolly(*this, "colscrolly"),
		m_gfxpointer(*this, "gfxpointer"),
		m_colorbank(*this, "colorbank"),
		m_video_mode(*this, "video_mode"),
		m_video_priority(*this, "video_priority"),
		m_collision_reg(*this, "collision_reg"),
		m_kikstart_scrollram(*this, "kikstart_scroll"){ }

	UINT8 m_sndnmi_disable;
	UINT8 m_input_port_4_f0;
	UINT8 m_kikstart_gears[2];
	INT8 m_dac_out;
	UINT8 m_dac_vol;
	required_shared_ptr<UINT8> m_videoram_1;
	required_shared_ptr<UINT8> m_videoram_2;
	required_shared_ptr<UINT8> m_videoram_3;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_characterram;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_colscrolly;
	required_shared_ptr<UINT8> m_gfxpointer;
	required_shared_ptr<UINT8> m_colorbank;
	required_shared_ptr<UINT8> m_video_mode;
	required_shared_ptr<UINT8> m_video_priority;
	required_shared_ptr<UINT8> m_collision_reg;
	optional_shared_ptr<UINT8> m_kikstart_scrollram;
	UINT8 m_fromz80;
	UINT8 m_toz80;
	UINT8 m_zaccept;
	UINT8 m_zready;
	UINT8 m_busreq;
	UINT8 m_portA_in;
	UINT8 m_portA_out;
	UINT8 m_spacecr_prot_value;
	UINT8 m_protection_value;
	UINT32 m_address;
	bitmap_ind16 m_layer_bitmap[3];
	bitmap_ind16 m_sprite_sprite_collbitmap1;
	bitmap_ind16 m_sprite_sprite_collbitmap2;
	bitmap_ind16 m_sprite_layer_collbitmap1;
	bitmap_ind16 m_sprite_layer_collbitmap2[3];
	int m_draw_order[32][4];
	DECLARE_WRITE8_MEMBER(taitosj_soundcommand_w);
	DECLARE_WRITE8_MEMBER(taitosj_bankswitch_w);
	DECLARE_READ8_MEMBER(taitosj_fake_data_r);
	DECLARE_WRITE8_MEMBER(taitosj_fake_data_w);
	DECLARE_READ8_MEMBER(taitosj_fake_status_r);
	DECLARE_READ8_MEMBER(taitosj_mcu_data_r);
	DECLARE_WRITE8_MEMBER(taitosj_mcu_data_w);
	DECLARE_READ8_MEMBER(taitosj_mcu_status_r);
	DECLARE_READ8_MEMBER(taitosj_68705_portA_r);
	DECLARE_WRITE8_MEMBER(taitosj_68705_portA_w);
	DECLARE_READ8_MEMBER(taitosj_68705_portB_r);
	DECLARE_WRITE8_MEMBER(taitosj_68705_portB_w);
	DECLARE_READ8_MEMBER(taitosj_68705_portC_r);
	DECLARE_READ8_MEMBER(spacecr_prot_r);
	DECLARE_WRITE8_MEMBER(alpine_protection_w);
	DECLARE_WRITE8_MEMBER(alpinea_bankswitch_w);
	DECLARE_READ8_MEMBER(alpine_port_2_r);
	DECLARE_READ8_MEMBER(taitosj_gfxrom_r);
	DECLARE_WRITE8_MEMBER(taitosj_characterram_w);
	DECLARE_WRITE8_MEMBER(junglhbr_characterram_w);
	DECLARE_WRITE8_MEMBER(taitosj_collision_reg_clear_w);
	DECLARE_CUSTOM_INPUT_MEMBER(input_port_4_f0_r);
	DECLARE_CUSTOM_INPUT_MEMBER(kikstart_gear_r);
	DECLARE_WRITE8_MEMBER(taitosj_sndnmi_msk_w);
	DECLARE_WRITE8_MEMBER(input_port_4_f0_w);
	DECLARE_WRITE8_MEMBER(dac_out_w);
	DECLARE_WRITE8_MEMBER(dac_vol_w);
	DECLARE_DRIVER_INIT(alpinea);
	DECLARE_DRIVER_INIT(alpine);
	DECLARE_DRIVER_INIT(taitosj);
	DECLARE_DRIVER_INIT(junglhbr);
	DECLARE_DRIVER_INIT(spacecr);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in machine/taitosj.c -----------*/






/*----------- defined in video/taitosj.c -----------*/


SCREEN_UPDATE_IND16( taitosj );
SCREEN_UPDATE_IND16( kikstart );
