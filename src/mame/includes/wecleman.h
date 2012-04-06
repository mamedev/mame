class wecleman_state : public driver_device
{
public:
	wecleman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_blitter_regs;
	int m_multiply_reg[2];
	UINT16 *m_protection_ram;
	int m_spr_color_offs;
	int m_prot_state;
	int m_selected_ip;
	int m_irqctrl;
	UINT16 *m_videostatus;
	UINT16 *m_pageram;
	UINT16 *m_txtram;
	UINT16 *m_roadram;
	size_t m_roadram_size;
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
	UINT16 *m_spriteram;
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
};


/*----------- defined in video/wecleman.c -----------*/

SCREEN_UPDATE_RGB32( wecleman );
VIDEO_START( wecleman );
SCREEN_UPDATE_IND16( hotchase );
VIDEO_START( hotchase );

void hotchase_zoom_callback_0(running_machine &machine, int *code,int *color,int *flags);
void hotchase_zoom_callback_1(running_machine &machine, int *code,int *color,int *flags);
