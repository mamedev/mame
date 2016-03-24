// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "sound/dac.h"

class taitosj_state : public driver_device
{
public:
	taitosj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_kikstart_scrollram(*this, "kikstart_scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	typedef void (taitosj_state::*copy_layer_func_t)(bitmap_ind16 &,
									const rectangle &, int, int *, rectangle *);
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
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_taitosj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kikstart(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(taitosj_mcu_real_data_w);
	TIMER_CALLBACK_MEMBER(taitosj_mcu_data_real_r);
	TIMER_CALLBACK_MEMBER(taitosj_mcu_status_real_w);
	void init_common();
	void reset_common();
	void set_pens();
	void compute_draw_order();
	inline int get_sprite_xy(UINT8 which, UINT8* sx, UINT8* sy);
	inline gfx_element *get_sprite_gfx_element(UINT8 which);
	void check_sprite_sprite_collision();
	void calculate_sprite_areas(int *sprites_on, rectangle *sprite_areas);
	int check_sprite_layer_bitpattern(int which, rectangle *sprite_areas);
	void check_sprite_layer_collision(int *sprites_on, rectangle *sprite_areas);
	void draw_layers();
	void draw_sprites(bitmap_ind16 &bitmap);
	void check_collision(int *sprites_on, rectangle *sprite_areas);
	int check_sprite_sprite_bitpattern(int sx1, int sy1, int which1,int sx2, int sy2, int which2);
	void taitosj_copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect,int which, int *sprites_on, rectangle *sprite_areas);
	void kikstart_copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect,int which, int *sprites_on, rectangle *sprite_areas);
	void copy_layer(bitmap_ind16 &bitmap, const rectangle &cliprect,copy_layer_func_t copy_layer_func, int which, int *sprites_on, rectangle *sprite_areas);
	void copy_layers(bitmap_ind16 &bitmap, const rectangle &cliprect,copy_layer_func_t copy_layer_func, int *sprites_on, rectangle *sprite_areas);
	int video_update_common(bitmap_ind16 &bitmap, const rectangle &cliprect, copy_layer_func_t copy_layer_func);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
