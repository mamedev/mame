/***************************************************************************

      Dynax hardware

***************************************************************************/

#include "sound/okim6295.h"

class dynax_state : public driver_device
{
public:
	dynax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		{ }

	// up to 8 layers, 2 images per layer (interleaved on screen)
	UINT8 *  m_pixmap[8][2];
	UINT8 *  m_ddenlovr_pixmap[8];

	/* irq */
	void (*m_update_irq_func)(running_machine &machine);	// some games trigger IRQ at blitter end, some don't
	UINT8 m_sound_irq;
	UINT8 m_vblank_irq;
	UINT8 m_blitter_irq;
	UINT8 m_blitter2_irq;
	UINT8 m_soundlatch_irq;
	UINT8 m_sound_vblank_irq;

	/* blitters */
	int m_blit_scroll_x;
	int m_blit2_scroll_x;
	int m_blit_scroll_y;
	int m_blit2_scroll_y;
	int m_blit_wrap_enable;
	int m_blit2_wrap_enable;
	int m_blit_x;
	int m_blit_y;
	int m_blit2_x;
	int m_blit2_y;
	int m_blit_src;
	int m_blit2_src;
	int m_blit_romregion;
	int m_blit2_romregion;
	int m_blit_dest;
	int m_blit2_dest;
	int m_blit_pen;
	int m_blit2_pen;
	int m_blit_palbank;
	int m_blit2_palbank;
	int m_blit_palettes;
	int m_blit2_palettes;
	int m_layer_enable;
	int m_blit_backpen;

	int m_hanamai_layer_half;
	int m_hnoridur_layer_half2;

	int m_extra_scroll_x;
	int m_extra_scroll_y;
	int m_flipscreen;

	int m_layer_layout;

	const int *m_priority_table;
	int m_hanamai_priority;

	/* ddenlovr blitter (TODO: merge with the above, where possible) */
	int m_extra_layers;
	int m_ddenlovr_dest_layer;
	int m_ddenlovr_blit_flip;
	int m_ddenlovr_blit_x;
	int m_ddenlovr_blit_y;
	int m_ddenlovr_blit_address;
	int m_ddenlovr_blit_pen;
	int m_ddenlovr_blit_pen_mode;
	int m_ddenlovr_blitter_irq_flag;
	int m_ddenlovr_blitter_irq_enable;
	int m_ddenlovr_rect_width;
	int m_ddenlovr_rect_height;
	int m_ddenlovr_clip_width;
	int m_ddenlovr_clip_height;
	int m_ddenlovr_line_length;
	int m_ddenlovr_clip_ctrl;
	int m_ddenlovr_clip_x;
	int m_ddenlovr_clip_y;
	int m_ddenlovr_scroll[8*2];
	int m_ddenlovr_priority;
	int m_ddenlovr_priority2;
	int m_ddenlovr_bgcolor;
	int m_ddenlovr_bgcolor2;
	int m_ddenlovr_layer_enable;
	int m_ddenlovr_layer_enable2;
	int m_ddenlovr_palette_base[8];
	int m_ddenlovr_palette_mask[8];
	int m_ddenlovr_transparency_pen[8];
	int m_ddenlovr_transparency_mask[8];
	int m_ddenlovr_blit_latch;
	int m_ddenlovr_blit_pen_mask;	// not implemented
	int m_ddenlovr_blit_rom_bits;			// usually 8, 16 in hanakanz
	const int *m_ddenlovr_blit_commands;
	int m_ddenlovr_blit_regs[2];

	/* input */
	UINT8 m_input_sel;
	UINT8 m_dsw_sel;
	UINT8 m_keyb;
	UINT8 m_coins;
	UINT8 m_hopper;

	/* misc */
	int m_hnoridur_bank;
	UINT8 m_palette_ram[16*256*2];
	int m_palbank;
	int m_msm5205next;
	int m_resetkludge;
	int m_toggle;
	int m_toggle_cpu1;
	int m_yarunara_clk_toggle;
	UINT8 m_soundlatch_ack;
	UINT8 m_soundlatch_full;
	UINT8 m_latch;
	int m_rombank;
	UINT8 m_tenkai_p5_val;
	int m_tenkai_6c;
	int m_tenkai_70;
	UINT8 m_gekisha_val[2];
	UINT8 m_gekisha_rom_enable;
	UINT8 *m_romptr;

	/* ddenlovr misc (TODO: merge with the above, where possible) */
	UINT8 m_palram[0x200];
	int m_okibank;
	UINT8 m_rongrong_blitter_busy_select;

	UINT16 *m_dsw_sel16;
	UINT16 *m_protection1;
	UINT16 *m_protection2;
	UINT8 m_prot_val;
	UINT16 m_prot_16;
	UINT16 m_quiz365_protection[2];

	UINT16 m_mmpanic_leds;	/* A led for each of the 9 buttons */
	UINT8 m_funkyfig_lockout;
	UINT8 m_romdata[2];
	int m_palette_index;
	UINT8 m_hginga_rombank;
	UINT8 m_mjflove_irq_cause;
	UINT8 m_daimyojn_palette_sel;

	int m_irq_count;


	/* devices */
	device_t *m_maincpu;
	device_t *m_soundcpu;
	device_t *m_rtc;
	device_t *m_ymsnd;
	okim6295_device *m_oki;
	device_t *m_top_scr;
	device_t *m_bot_scr;
};

//----------- defined in drivers/dynax.c -----------

void sprtmtch_update_irq(running_machine &machine);
void jantouki_update_irq(running_machine &machine);
void mjelctrn_update_irq(running_machine &machine);
void neruton_update_irq(running_machine &machine);

//----------- defined in video/dynax.c -----------

WRITE8_HANDLER( dynax_blitter_rev2_w );
WRITE8_HANDLER( jantouki_blitter_rev2_w );
WRITE8_HANDLER( jantouki_blitter2_rev2_w );
WRITE8_HANDLER( tenkai_blitter_rev2_w );

WRITE8_HANDLER( dynax_blit_pen_w );
WRITE8_HANDLER( dynax_blit2_pen_w );
WRITE8_HANDLER( dynax_blit_backpen_w );
WRITE8_HANDLER( dynax_blit_dest_w );
WRITE8_HANDLER( dynax_blit2_dest_w );
WRITE8_HANDLER( dynax_blit_palbank_w );
WRITE8_HANDLER( dynax_blit2_palbank_w );
WRITE8_HANDLER( dynax_blit_palette01_w );
WRITE8_HANDLER( dynax_blit_palette23_w );
WRITE8_HANDLER( dynax_blit_palette45_w );
WRITE8_HANDLER( dynax_blit_palette67_w );
WRITE8_HANDLER( dynax_layer_enable_w );
WRITE8_HANDLER( jantouki_layer_enable_w );
WRITE8_HANDLER( dynax_flipscreen_w );
WRITE8_HANDLER( dynax_extra_scrollx_w );
WRITE8_HANDLER( dynax_extra_scrolly_w );
WRITE8_HANDLER( dynax_blit_romregion_w );
WRITE8_HANDLER( dynax_blit2_romregion_w );

WRITE8_HANDLER( hanamai_layer_half_w );
WRITE8_HANDLER( hnoridur_layer_half2_w );

WRITE8_HANDLER( hanamai_priority_w );
WRITE8_HANDLER( tenkai_priority_w );

WRITE8_HANDLER( mjdialq2_blit_dest_w );
WRITE8_HANDLER( tenkai_blit_dest_w );

WRITE8_HANDLER( mjdialq2_layer_enable_w );

WRITE8_HANDLER( tenkai_blit_palette01_w );
WRITE8_HANDLER( tenkai_blit_palette23_w );

VIDEO_START( hanamai );
VIDEO_START( hnoridur );
VIDEO_START( mcnpshnt );
VIDEO_START( sprtmtch );
VIDEO_START( mjdialq2 );
VIDEO_START( jantouki );
VIDEO_START( mjelctrn );
VIDEO_START( neruton );
VIDEO_START( htengoku );

SCREEN_UPDATE( hanamai );
SCREEN_UPDATE( hnoridur );
SCREEN_UPDATE( sprtmtch );
SCREEN_UPDATE( mjdialq2 );
SCREEN_UPDATE( jantouki );
SCREEN_UPDATE( htengoku );

PALETTE_INIT( sprtmtch );


//----------- defined in drivers/ddenlovr.c -----------

WRITE8_HANDLER( ddenlovr_bgcolor_w );
WRITE8_HANDLER( ddenlovr_priority_w );
WRITE8_HANDLER( ddenlovr_layer_enable_w );
WRITE8_HANDLER( ddenlovr_palette_base_w );
WRITE8_HANDLER( ddenlovr_palette_mask_w );
WRITE8_HANDLER( ddenlovr_transparency_pen_w );
WRITE8_HANDLER( ddenlovr_transparency_mask_w );

VIDEO_START(ddenlovr);
SCREEN_UPDATE(ddenlovr);
