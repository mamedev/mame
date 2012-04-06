
// later, this might be merged with segas1x_state in segas16.h

class segas1x_bootleg_state : public driver_device
{
public:
	segas1x_bootleg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *    m_bg0_tileram;
	UINT16 *    m_bg1_tileram;
	UINT16 *    m_textram;
	UINT16 *    m_tileram;

	UINT16 m_coinctrl;

	/* game specific */
	int m_passht4b_io1_val;
	int m_passht4b_io2_val;
	int m_passht4b_io3_val;

	int m_beautyb_unkx;

	int m_shinobl_kludge;

	UINT16* m_goldnaxeb2_fgpage;
	UINT16* m_goldnaxeb2_bgpage;

	int m_eswat_tilebank0;


	/* video-related */
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	tilemap_t *m_text_layer;
	tilemap_t *m_background2;
	tilemap_t *m_foreground2;
	tilemap_t *m_bg_tilemaps[2];
	tilemap_t *m_text_tilemap;
	double m_weights[2][3][6];

	int m_spritebank_type;
	int m_back_yscroll;
	int m_fore_yscroll;
	int m_text_yscroll;

	int m_bg1_trans; // alien syn + sys18

	int m_tile_bank1;
	int m_tile_bank0;
	int m_bg_page[4];
	int m_fg_page[4];

	UINT16 m_datsu_page[4];

	int m_bg2_page[4];
	int m_fg2_page[4];

	int m_old_bg_page[4];
	int m_old_fg_page[4];
	int m_old_tile_bank1;
	int m_old_tile_bank0;
	int m_old_bg2_page[4];
	int m_old_fg2_page[4];

	int m_bg_scrollx;
	int m_bg_scrolly;
	int m_fg_scrollx;
	int m_fg_scrolly;
	UINT16 m_tilemapselect;

	int m_textlayer_lo_min;
	int m_textlayer_lo_max;
	int m_textlayer_hi_min;
	int m_textlayer_hi_max;

	int m_tilebank_switch;


	/* sound-related */
	int m_sample_buffer;
	int m_sample_select;

	UINT8 *m_soundbank_ptr;		/* Pointer to currently selected portion of ROM */

	/* sys18 */
	UINT8 *m_sound_bank;
	UINT16 *m_splittab_bg_x;
	UINT16 *m_splittab_bg_y;
	UINT16 *m_splittab_fg_x;
	UINT16 *m_splittab_fg_y;
	int     m_sound_info[4*2];
	int     m_refreshenable;
	int     m_system18;

	UINT8 *m_decrypted_region;	// goldnaxeb1 & bayrouteb1

	/* devices */
	device_t *m_maincpu;
	device_t *m_soundcpu;
	DECLARE_WRITE16_MEMBER(sound_command_nmi_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE16_MEMBER(sys16_coinctrl_w);
	DECLARE_READ16_MEMBER(passht4b_service_r);
	DECLARE_READ16_MEMBER(passht4b_io1_r);
	DECLARE_READ16_MEMBER(passht4b_io2_r);
	DECLARE_READ16_MEMBER(passht4b_io3_r);
	DECLARE_WRITE16_MEMBER(sys16_tilebank_w);
	DECLARE_WRITE8_MEMBER(tturfbl_msm5205_data_w);
	DECLARE_READ8_MEMBER(tturfbl_soundbank_r);
	DECLARE_WRITE8_MEMBER(tturfbl_soundbank_w);
	DECLARE_WRITE16_MEMBER(s16bl_bgpage_w);
	DECLARE_WRITE16_MEMBER(s16bl_fgpage_w);
	DECLARE_WRITE16_MEMBER(s16bl_fgscrollx_bank_w);
	DECLARE_WRITE16_MEMBER(s16bl_fgscrollx_w);
	DECLARE_WRITE16_MEMBER(s16bl_fgscrolly_w);
	DECLARE_WRITE16_MEMBER(s16bl_bgscrollx_w);
	DECLARE_WRITE16_MEMBER(s16bl_bgscrolly_w);
	DECLARE_WRITE16_MEMBER(datsu_page0_w);
	DECLARE_WRITE16_MEMBER(datsu_page1_w);
	DECLARE_WRITE16_MEMBER(datsu_page2_w);
	DECLARE_WRITE16_MEMBER(datsu_page3_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_fgscrollx_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_bgscrollx_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_fgscrolly_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_bgscrolly_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_fgpage_w);
	DECLARE_WRITE16_MEMBER(goldnaxeb2_bgpage_w);
	DECLARE_WRITE16_MEMBER(eswat_tilebank0_w);
	DECLARE_READ16_MEMBER(beautyb_unkx_r);
	DECLARE_WRITE16_MEMBER(sys18_refreshenable_w);
	DECLARE_WRITE16_MEMBER(sys18_tilebank_w);
	DECLARE_READ8_MEMBER(system18_bank_r);
	DECLARE_WRITE8_MEMBER(sys18_soundbank_w);
	DECLARE_WRITE16_MEMBER(sound_command_irq_w);
	DECLARE_WRITE8_MEMBER(shdancbl_msm5205_data_w);
	DECLARE_READ8_MEMBER(shdancbl_soundbank_r);
	DECLARE_WRITE8_MEMBER(shdancbl_bankctrl_w);
	DECLARE_WRITE16_MEMBER(sys16_paletteram_w);
	DECLARE_WRITE16_MEMBER(sys16_tileram_w);
	DECLARE_WRITE16_MEMBER(sys16_textram_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_bgscrolly_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_bgscrollx_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_fgscrolly_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_fgscrollx_w);
	DECLARE_WRITE16_MEMBER(s16a_bootleg_tilemapselect_w);
};

/*----------- defined in video/system16.c -----------*/

extern VIDEO_START( s16a_bootleg );
extern VIDEO_START( s16a_bootleg_wb3bl );
extern VIDEO_START( s16a_bootleg_shinobi );
extern VIDEO_START( s16a_bootleg_passsht );
extern SCREEN_UPDATE_IND16( s16a_bootleg );
extern SCREEN_UPDATE_IND16( s16a_bootleg_passht4b );

/* video hardware */

/* "normal" video hardware */
extern VIDEO_START( system16 );
extern SCREEN_UPDATE_IND16( system16 );

/* system18 video hardware */
extern VIDEO_START( system18old );
extern SCREEN_UPDATE_IND16( system18old );
