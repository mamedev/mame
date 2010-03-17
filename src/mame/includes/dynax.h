/***************************************************************************

      Dynax hardware

***************************************************************************/

class dynax_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dynax_state(machine)); }

	dynax_state(running_machine &machine) { }

	// up to 8 layers, 2 images per layer (interleaved on screen)
	UINT8 *  pixmap[8][2];
	UINT8 *  ddenlovr_pixmap[8];

	/* irq */
	void (*update_irq_func)(running_machine *machine);	// some games trigger IRQ at blitter end, some don't
	UINT8 sound_irq;
	UINT8 vblank_irq;
	UINT8 blitter_irq, blitter2_irq;
	UINT8 soundlatch_irq;
	UINT8 sound_vblank_irq;

	/* blitters */
	int blit_scroll_x, blit2_scroll_x;
	int blit_scroll_y, blit2_scroll_y;
	int blit_wrap_enable, blit2_wrap_enable;
	int blit_x, blit_y, blit2_x, blit2_y;
	int blit_src, blit2_src;
	int blit_romregion, blit2_romregion;
	int blit_dest, blit2_dest;
	int blit_pen, blit2_pen;
	int blit_palbank, blit2_palbank;
	int blit_palettes, blit2_palettes;
	int layer_enable;
	int blit_backpen;

	int hanamai_layer_half;
	int hnoridur_layer_half2;

	int extra_scroll_x, extra_scroll_y;
	int flipscreen;

	int layer_layout;

	const int *priority_table;
	int hanamai_priority;

	/* ddenlovr blitter (TODO: merge with the above, where possible) */
	int extra_layers;
	int ddenlovr_dest_layer;
	int ddenlovr_blit_flip;
	int ddenlovr_blit_x;
	int ddenlovr_blit_y;
	int ddenlovr_blit_address;
	int ddenlovr_blit_pen,ddenlovr_blit_pen_mode;
	int ddenlovr_blitter_irq_flag,ddenlovr_blitter_irq_enable;
	int ddenlovr_rect_width, ddenlovr_rect_height;
	int ddenlovr_clip_width, ddenlovr_clip_height;
	int ddenlovr_line_length;
	int ddenlovr_clip_ctrl,ddenlovr_clip_x,ddenlovr_clip_y;
	int ddenlovr_scroll[8*2];
	int ddenlovr_priority, ddenlovr_priority2;
	int ddenlovr_bgcolor, ddenlovr_bgcolor2;
	int ddenlovr_layer_enable, ddenlovr_layer_enable2;
	int ddenlovr_palette_base[8], ddenlovr_palette_mask[8];
	int ddenlovr_transparency_pen[8], ddenlovr_transparency_mask[8];
	int ddenlovr_blit_latch;
	int ddenlovr_blit_pen_mask;	// not implemented
	int ddenlovr_blit_rom_bits;			// usually 8, 16 in hanakanz
	const int *ddenlovr_blit_commands;
	int ddenlovr_blit_regs[2];

	/* input */
	UINT8 input_sel, dsw_sel, keyb, coins, hopper;

	/* misc */
	int hnoridur_bank;
	UINT8 palette_ram[16*256*2];
	int palbank;
	int msm5205next;
	int resetkludge;
	int toggle, toggle_cpu1;
	int yarunara_clk_toggle;
	UINT8 soundlatch_ack;
	UINT8 soundlatch_full;
	UINT8 latch;
	int rombank;
	UINT8 tenkai_p5_val;
	int tenkai_6c, tenkai_70;
	UINT8 gekisha_val[2];
	UINT8 gekisha_rom_enable;
	UINT8 *romptr;

	/* ddenlovr misc (TODO: merge with the above, where possible) */
	UINT8 palram[0x200];
	int okibank;
	UINT8 rongrong_blitter_busy_select;

	UINT16 *dsw_sel16;
	UINT16 *protection1, *protection2;
	UINT8 prot_val;
	UINT16 prot_16;
	UINT16 quiz365_protection[2];

	UINT16 mmpanic_leds;	/* A led for each of the 9 buttons */
	UINT8 funkyfig_lockout;
	UINT8 romdata[2];
	int palette_index;
	UINT8 hginga_rombank;
	UINT8 mjflove_irq_cause;
	UINT8 daimyojn_palette_sel;

	int irq_count;


	/* devices */
	running_device *maincpu;
	running_device *soundcpu;
	running_device *rtc;
	running_device *ymsnd;
	running_device *oki;
	running_device *top_scr;
	running_device *bot_scr;
};

//----------- defined in drivers/dynax.c -----------

void sprtmtch_update_irq(running_machine *machine);
void jantouki_update_irq(running_machine *machine);
void mjelctrn_update_irq(running_machine *machine);
void neruton_update_irq(running_machine *machine);

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

VIDEO_UPDATE( hanamai );
VIDEO_UPDATE( hnoridur );
VIDEO_UPDATE( sprtmtch );
VIDEO_UPDATE( mjdialq2 );
VIDEO_UPDATE( jantouki );
VIDEO_UPDATE( htengoku );

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
VIDEO_UPDATE(ddenlovr);
