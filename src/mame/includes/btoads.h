/*************************************************************************

    BattleToads

    Common definitions

*************************************************************************/

class btoads_state : public driver_device
{
public:
	btoads_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 m_main_to_sound_data;
	UINT8 m_main_to_sound_ready;
	UINT8 m_sound_to_main_data;
	UINT8 m_sound_to_main_ready;
	UINT8 m_sound_int_state;
	UINT16 *m_vram_fg0;
	UINT16 *m_vram_fg1;
	UINT16 *m_vram_fg_data;
	UINT16 *m_vram_bg0;
	UINT16 *m_vram_bg1;
	UINT16 *m_sprite_scale;
	UINT16 *m_sprite_control;
	UINT8 *m_vram_fg_draw;
	UINT8 *m_vram_fg_display;
	INT32 m_xscroll0;
	INT32 m_yscroll0;
	INT32 m_xscroll1;
	INT32 m_yscroll1;
	UINT8 m_screen_control;
	UINT16 m_sprite_source_offs;
	UINT8 *m_sprite_dest_base;
	UINT16 m_sprite_dest_offs;
	UINT16 m_misc_control;
	int m_xcount;
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
