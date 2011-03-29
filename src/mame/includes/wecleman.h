class wecleman_state : public driver_device
{
public:
	wecleman_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *blitter_regs;
	int multiply_reg[2];
	UINT16 *protection_ram;
	int spr_color_offs;
	int prot_state;
	int selected_ip;
	int irqctrl;
	UINT16 *videostatus;
	UINT16 *pageram;
	UINT16 *txtram;
	UINT16 *roadram;
	size_t roadram_size;
	int bgpage[4];
	int fgpage[4];
	const int *gfx_bank;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
	tilemap_t *txt_tilemap;
	int *spr_idx_list;
	int *spr_pri_list;
	int *t32x32pm;
	int gameid;
	int spr_offsx;
	int spr_offsy;
	int spr_count;
	UINT16 *rgb_half;
	int cloud_blend;
	int cloud_ds;
	int cloud_visible;
	pen_t black_pen;
	struct sprite *sprite_list;
	struct sprite **spr_ptr_list;
	UINT16 *spriteram;
};


/*----------- defined in video/wecleman.c -----------*/

WRITE16_HANDLER( hotchase_paletteram16_SBGRBBBBGGGGRRRR_word_w );
WRITE16_HANDLER( wecleman_paletteram16_SSSSBBBBGGGGRRRR_word_w );
WRITE16_HANDLER( wecleman_videostatus_w );
WRITE16_HANDLER( wecleman_pageram_w );
WRITE16_HANDLER( wecleman_txtram_w );
SCREEN_UPDATE( wecleman );
VIDEO_START( wecleman );
SCREEN_UPDATE( hotchase );
VIDEO_START( hotchase );

void hotchase_zoom_callback_0(running_machine &machine, int *code,int *color,int *flags);
void hotchase_zoom_callback_1(running_machine &machine, int *code,int *color,int *flags);
