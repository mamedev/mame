/*----------- defined in video/system16.c -----------*/

extern VIDEO_START( system16a_bootleg );
extern VIDEO_START( system16a_bootleg_wb3bl );
extern VIDEO_START( system16a_bootleg_shinobi );
extern VIDEO_START( system16a_bootleg_passsht );
extern VIDEO_UPDATE( system16a_bootleg );
extern VIDEO_UPDATE( system16a_bootleg_passht4b );
extern WRITE16_HANDLER( system16a_bootleg_tilemapselect_w );
extern WRITE16_HANDLER( system16a_bootleg_bgscrolly_w );
extern WRITE16_HANDLER( system16a_bootleg_bgscrollx_w );
extern WRITE16_HANDLER( system16a_bootleg_fgscrolly_w );
extern WRITE16_HANDLER( system16a_bootleg_fgscrollx_w );
extern UINT16* system16a_bootleg_bg0_tileram;
extern UINT16* system16a_bootleg_bg1_tileram;

/* video hardware */
extern WRITE16_HANDLER( sys16_tileram_w );
extern WRITE16_HANDLER( sys16_textram_w );

/* "normal" video hardware */
extern VIDEO_START( system16 );
extern VIDEO_UPDATE( system16 );

/* system18 video hardware */
extern VIDEO_START( system18old );
extern VIDEO_UPDATE( system18old );

extern UINT16 *sys16_tileram;
extern UINT16 *sys16_textram;
extern UINT16 *sys16_spriteram;

extern int sys16_sh_shadowpal;
extern int sys16_MaxShadowColors;

/* video driver constants (vary with game) */
extern int sys16_sprxoffset;
extern int sys16_bgxoffset;
extern int sys16_fgxoffset;
extern const int *sys16_obj_bank;
extern int sys16_textlayer_lo_min;
extern int sys16_textlayer_lo_max;
extern int sys16_textlayer_hi_min;
extern int sys16_textlayer_hi_max;
extern int sys16_bg1_trans;
extern int sys16_bg_priority_mode;
extern int sys16_fg_priority_mode;
extern int sys16_bg_priority_value;
extern int sys16_fg_priority_value;
extern int sys16_tilebank_switch;
extern int sys16_rowscroll_scroll;
extern int shinobl_kludge;

/* video driver registers */
extern int sys16_refreshenable;
extern int sys16_tile_bank0;
extern int sys16_tile_bank1;
extern int sys16_bg_scrollx, sys16_bg_scrolly;
extern int sys16_bg_page[4];
extern int sys16_fg_scrollx, sys16_fg_scrolly;
extern int sys16_fg_page[4];

extern int sys16_bg2_page[4];
extern int sys16_fg2_page[4];

extern int sys18_bg2_active;
extern int sys18_fg2_active;
extern UINT16 *sys18_splittab_bg_x;
extern UINT16 *sys18_splittab_bg_y;
extern UINT16 *sys18_splittab_fg_x;
extern UINT16 *sys18_splittab_fg_y;
