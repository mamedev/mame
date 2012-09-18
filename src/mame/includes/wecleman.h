class wecleman_state : public driver_device
{
public:
	wecleman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videostatus(*this, "videostatus"),
		m_protection_ram(*this, "protection_ram"),
		m_blitter_regs(*this, "blitter_regs"),
		m_pageram(*this, "pageram"),
		m_txtram(*this, "txtram"),
		m_spriteram(*this, "spriteram"),
		m_roadram(*this, "roadram"){ }

	optional_shared_ptr<UINT16> m_videostatus;
	optional_shared_ptr<UINT16> m_protection_ram;
	required_shared_ptr<UINT16> m_blitter_regs;
	optional_shared_ptr<UINT16> m_pageram;
	optional_shared_ptr<UINT16> m_txtram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_roadram;

	int m_multiply_reg[2];
	int m_spr_color_offs;
	int m_prot_state;
	int m_selected_ip;
	int m_irqctrl;
	int m_bgpage[4];
	int m_fgpage[4];
	const int *m_gfx_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_txt_tilemap;
	int *m_spr_idx_list;
	int *m_spr_pri_list;
	int *m_t32x32pm;
	int m_gameid;
	int m_spr_offsx;
	int m_spr_offsy;
	int m_spr_count;
	UINT16 *m_rgb_half;
	int m_cloud_blend;
	int m_cloud_ds;
	int m_cloud_visible;
	pen_t m_black_pen;
	struct sprite *m_sprite_list;
	struct sprite **m_spr_ptr_list;
	DECLARE_READ16_MEMBER(wecleman_protection_r);
	DECLARE_WRITE16_MEMBER(wecleman_protection_w);
	DECLARE_WRITE16_MEMBER(irqctrl_w);
	DECLARE_WRITE16_MEMBER(selected_ip_w);
	DECLARE_READ16_MEMBER(selected_ip_r);
	DECLARE_WRITE16_MEMBER(blitter_w);
	DECLARE_READ8_MEMBER(multiply_r);
	DECLARE_WRITE8_MEMBER(multiply_w);
	DECLARE_WRITE16_MEMBER(hotchase_soundlatch_w);
	DECLARE_WRITE8_MEMBER(hotchase_sound_control_w);
	DECLARE_WRITE16_MEMBER(wecleman_soundlatch_w);
	DECLARE_WRITE16_MEMBER(wecleman_txtram_w);
	DECLARE_WRITE16_MEMBER(wecleman_pageram_w);
	DECLARE_WRITE16_MEMBER(wecleman_videostatus_w);
	DECLARE_WRITE16_MEMBER(hotchase_paletteram16_SBGRBBBBGGGGRRRR_word_w);
	DECLARE_WRITE16_MEMBER(wecleman_paletteram16_SSSSBBBBGGGGRRRR_word_w);
	DECLARE_WRITE8_MEMBER(wecleman_K00723216_bank_w);
	DECLARE_READ8_MEMBER(hotchase_1_k007232_r);
	DECLARE_WRITE8_MEMBER(hotchase_1_k007232_w);
	DECLARE_READ8_MEMBER(hotchase_2_k007232_r);
	DECLARE_WRITE8_MEMBER(hotchase_2_k007232_w);
	DECLARE_READ8_MEMBER(hotchase_3_k007232_r);
	DECLARE_WRITE8_MEMBER(hotchase_3_k007232_w);
	DECLARE_DRIVER_INIT(wecleman);
	DECLARE_DRIVER_INIT(hotchase);
	TILE_GET_INFO_MEMBER(wecleman_get_txt_tile_info);
	TILE_GET_INFO_MEMBER(wecleman_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(wecleman_get_fg_tile_info);
	DECLARE_MACHINE_RESET(wecleman);
	DECLARE_VIDEO_START(wecleman);
	DECLARE_MACHINE_RESET(hotchase);
	DECLARE_VIDEO_START(hotchase);
	UINT32 screen_update_wecleman(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_hotchase(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in video/wecleman.c -----------*/
void hotchase_zoom_callback_0(running_machine &machine, int *code,int *color,int *flags);
void hotchase_zoom_callback_1(running_machine &machine, int *code,int *color,int *flags);
