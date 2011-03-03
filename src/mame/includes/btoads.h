/*************************************************************************

    BattleToads

    Common definitions

*************************************************************************/

class btoads_state : public driver_device
{
public:
	btoads_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 main_to_sound_data;
	UINT8 main_to_sound_ready;
	UINT8 sound_to_main_data;
	UINT8 sound_to_main_ready;
	UINT8 sound_int_state;
	UINT16 *vram_fg0;
	UINT16 *vram_fg1;
	UINT16 *vram_fg_data;
	UINT16 *vram_bg0;
	UINT16 *vram_bg1;
	UINT16 *sprite_scale;
	UINT16 *sprite_control;
	UINT8 *vram_fg_draw;
	UINT8 *vram_fg_display;
	INT32 xscroll0;
	INT32 yscroll0;
	INT32 xscroll1;
	INT32 yscroll1;
	UINT8 screen_control;
	UINT16 sprite_source_offs;
	UINT8 *sprite_dest_base;
	UINT16 sprite_dest_offs;
	UINT16 misc_control;
	int count;
};


/*----------- defined in video/btoads.c -----------*/

VIDEO_START( btoads );

WRITE16_HANDLER( btoads_misc_control_w );
WRITE16_HANDLER( btoads_display_control_w );

WRITE16_HANDLER( btoads_scroll0_w );
WRITE16_HANDLER( btoads_scroll1_w );

WRITE16_HANDLER( btoads_paletteram_w );
READ16_HANDLER( btoads_paletteram_r );

WRITE16_HANDLER( btoads_vram_bg0_w );
WRITE16_HANDLER( btoads_vram_bg1_w );
READ16_HANDLER( btoads_vram_bg0_r );
READ16_HANDLER( btoads_vram_bg1_r );

WRITE16_HANDLER( btoads_vram_fg_display_w );
WRITE16_HANDLER( btoads_vram_fg_draw_w );
READ16_HANDLER( btoads_vram_fg_display_r );
READ16_HANDLER( btoads_vram_fg_draw_r );

void btoads_to_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);
void btoads_from_shiftreg(address_space *space, UINT32 address, UINT16 *shiftreg);

void btoads_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
