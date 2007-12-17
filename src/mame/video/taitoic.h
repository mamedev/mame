/***************************************************************************
                Taito Custom Chips
***************************************************************************/

extern const int TC0100SCN_SINGLE_VDU;	/* value set in taitoic.c */

int number_of_TC0100SCN(void);
int has_TC0110PCR(void);
int has_second_TC0110PCR(void);
int has_third_TC0110PCR(void);
int has_TC0150ROD(void);
int has_TC0280GRD(void);
int has_TC0360PRI(void);
int has_TC0430GRW(void);
int has_TC0480SCP(void);


/***************************************************************************/

void PC080SN_vh_start(int chips,int gfxnum,int x_offset,int y_offset,int y_invert,int opaque,int dblwidth);
READ16_HANDLER ( PC080SN_word_0_r );
WRITE16_HANDLER( PC080SN_word_0_w );
WRITE16_HANDLER( PC080SN_xscroll_word_0_w );
WRITE16_HANDLER( PC080SN_yscroll_word_0_w );
WRITE16_HANDLER( PC080SN_ctrl_word_0_w );
READ16_HANDLER ( PC080SN_word_1_r );
WRITE16_HANDLER( PC080SN_word_1_w );
WRITE16_HANDLER( PC080SN_xscroll_word_1_w );
WRITE16_HANDLER( PC080SN_yscroll_word_1_w );
WRITE16_HANDLER( PC080SN_ctrl_word_1_w );
void PC080SN_set_scroll(int chip,int tilemap_num,int scrollx,int scrolly);
void PC080SN_set_trans_pen(int chip,int tilemap_num,int pen);
void PC080SN_tilemap_update(void);
void PC080SN_tilemap_draw(mame_bitmap *bitmap,const rectangle *cliprect,int chip,int layer,int flags,UINT32 priority);
void PC080SN_tilemap_draw_offset(mame_bitmap *bitmap,const rectangle *cliprect,int chip,int layer,int flags,UINT32 priority,int xoffs,int yoffs);

/* For Topspeed */
void PC080SN_tilemap_draw_special(mame_bitmap *bitmap,const rectangle *cliprect,int chip,int layer,int flags,UINT32 priority,UINT16 *ram);


/***************************************************************************/

void PC090OJ_vh_start(int gfxnum,int x_offset,int y_offset,int use_buffer);

READ16_HANDLER( PC090OJ_word_0_r );
WRITE16_HANDLER( PC090OJ_word_0_w );

void PC090OJ_eof_callback(void);
void PC090OJ_draw_sprites(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect,int pri_type);

extern UINT16 PC090OJ_sprite_ctrl;


/***************************************************************************/

void TC0080VCO_vh_start(running_machine *machine, int gfxnum,int has_fg0,int bg_xoffs,int bg_yoffs,int bg_flip_yoffs);

READ16_HANDLER ( TC0080VCO_word_r );
WRITE16_HANDLER( TC0080VCO_word_w );

void TC0080VCO_tilemap_update(running_machine *machine);
void TC0080VCO_tilemap_draw(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int layer,int flags,UINT32 priority);


/***************************************************************************/

/* When writing a driver pass zero for all the offsets initially then
   tweak them later. Most TC0100SCN games have y_offset=0 */

void TC0100SCN_vh_start(running_machine *machine, int chips,int gfxnum,int x_offset,int y_offset,int flip_xoffs,
		int flip_yoffs,int flip_text_xoffs,int flip_text_yoffs,int multiscrn_xoffs);

/* Function to set separate color banks for the three tilemapped layers.
   To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
void TC0100SCN_set_colbanks(int bg0,int bg1,int fg);

/* Function to set separate color banks for each TC0100SCN.
   To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
void TC0100SCN_set_chip_colbanks(int chip0,int chip1,int chip2);

/* Function to set bg tilemask < 0xffff */
void TC0100SCN_set_bg_tilemask(int mask);

/* Function to for Mjnquest to select gfx bank */
WRITE16_HANDLER( TC0100SCN_gfxbank_w );

READ16_HANDLER ( TC0100SCN_word_0_r );
WRITE16_HANDLER( TC0100SCN_word_0_w );
READ16_HANDLER ( TC0100SCN_ctrl_word_0_r );
WRITE16_HANDLER( TC0100SCN_ctrl_word_0_w );
READ16_HANDLER ( TC0100SCN_word_1_r );
WRITE16_HANDLER( TC0100SCN_word_1_w );
READ16_HANDLER ( TC0100SCN_ctrl_word_1_r );
WRITE16_HANDLER( TC0100SCN_ctrl_word_1_w );
READ16_HANDLER ( TC0100SCN_word_2_r );
WRITE16_HANDLER( TC0100SCN_word_2_w );
READ16_HANDLER ( TC0100SCN_ctrl_word_2_r );
WRITE16_HANDLER( TC0100SCN_ctrl_word_2_w );

/* Functions for use with 68020 (Under Fire) */
READ32_HANDLER ( TC0100SCN_long_r );
WRITE32_HANDLER( TC0100SCN_long_w );
READ32_HANDLER ( TC0100SCN_ctrl_long_r );
WRITE32_HANDLER( TC0100SCN_ctrl_long_w );

/* Functions to write multiple TC0100SCNs with the same data */
WRITE16_HANDLER( TC0100SCN_dual_screen_w );
WRITE16_HANDLER( TC0100SCN_triple_screen_w );

void TC0100SCN_tilemap_update(running_machine *machine);
int TC0100SCN_tilemap_draw(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect,int chip,int layer,int flags,UINT32 priority);

/* returns 0 or 1 depending on the lowest priority tilemap set in the internal
   register. Use this function to draw tilemaps in the correct order. */
int TC0100SCN_bottomlayer(int chip);


/***************************************************************************/

void TC0280GRD_vh_start(int gfxnum);
READ16_HANDLER ( TC0280GRD_word_r );
WRITE16_HANDLER( TC0280GRD_word_w );
WRITE16_HANDLER( TC0280GRD_ctrl_word_w );
void TC0280GRD_tilemap_update(int base_color);
void TC0280GRD_zoom_draw(mame_bitmap *bitmap,const rectangle *cliprect,int xoffset,int yoffset,UINT32 priority);

void TC0430GRW_vh_start(int gfxnum);
READ16_HANDLER ( TC0430GRW_word_r );
WRITE16_HANDLER( TC0430GRW_word_w );
WRITE16_HANDLER( TC0430GRW_ctrl_word_w );
void TC0430GRW_tilemap_update(int base_color);
void TC0430GRW_zoom_draw(mame_bitmap *bitmap,const rectangle *cliprect,int xoffset,int yoffset,UINT32 priority);


/***************************************************************************/

/* When writing a driver, pass zero for the text and flip offsets initially:
   then tweak them once you have the 4 bg layer positions correct. Col_base
   may be needed when tilemaps use a palette area from sprites. */

void TC0480SCP_vh_start(running_machine *machine, int gfxnum,int pixels,int x_offset,int y_offset,int text_xoffs,int text_yoffs,int flip_xoffs,int flip_yoffs,int col_base);
READ16_HANDLER ( TC0480SCP_word_r );
WRITE16_HANDLER( TC0480SCP_word_w );
READ16_HANDLER ( TC0480SCP_ctrl_word_r );
WRITE16_HANDLER( TC0480SCP_ctrl_word_w );

/* Functions for use with 68020 (Super-Z system) */
READ32_HANDLER ( TC0480SCP_long_r );
WRITE32_HANDLER( TC0480SCP_long_w );
READ32_HANDLER ( TC0480SCP_ctrl_long_r );
WRITE32_HANDLER( TC0480SCP_ctrl_long_w );

void TC0480SCP_tilemap_update(running_machine *machine);
void TC0480SCP_tilemap_draw(mame_bitmap *bitmap,const rectangle *cliprect,int layer,int flags,UINT32 priority);

/* Returns the priority order of the bg tilemaps set in the internal
   register. The order in which the four layers should be drawn is
   returned in the lowest four nibbles  (msn = bottom layer; lsn = top) */
int TC0480SCP_get_bg_priority(void);

/* Undrfire needs to read this for a sprite/tile priority hack */
extern int TC0480SCP_pri_reg;


/***************************************************************************/

READ16_HANDLER( TC0150ROD_word_r );
WRITE16_HANDLER( TC0150ROD_word_w );
void TC0150ROD_vh_start(void);
void TC0150ROD_draw(mame_bitmap *bitmap,const rectangle *cliprect,int y_offs,int palette_offs,int type,int road_trans,UINT32 low_priority,UINT32 high_priority);


/***************************************************************************/

void TC0110PCR_vh_start(void);
void TC0110PCR_1_vh_start(void);	/* 2nd chip */
void TC0110PCR_2_vh_start(void);	/* 3rd chip */
READ16_HANDLER ( TC0110PCR_word_r );
READ16_HANDLER ( TC0110PCR_word_1_r );	/* 2nd chip */
READ16_HANDLER ( TC0110PCR_word_2_r );	/* 3rd chip */
WRITE16_HANDLER( TC0110PCR_word_w );	/* color index goes up in step of 2 */
WRITE16_HANDLER( TC0110PCR_step1_word_w );	/* color index goes up in step of 1 */
WRITE16_HANDLER( TC0110PCR_step1_word_1_w );	/* 2nd chip */
WRITE16_HANDLER( TC0110PCR_step1_word_2_w );	/* 3rd chip */
WRITE16_HANDLER( TC0110PCR_step1_rbswap_word_w );	/* swaps red and blue components */
WRITE16_HANDLER( TC0110PCR_step1_4bpg_word_w );	/* only 4 bits per color gun */

void TC0360PRI_vh_start(void);	/* must be called to ensure regs saved in state.c */
WRITE8_HANDLER( TC0360PRI_w );
WRITE16_HANDLER( TC0360PRI_halfword_w );
WRITE16_HANDLER( TC0360PRI_halfword_swap_w );


/***************************************************************************/

/* I/O chips, all extremely similar. The TC0220IOC was sometimes addressed
   through a port, typically on earlier games. */

READ8_HANDLER ( TC0220IOC_r );
WRITE8_HANDLER( TC0220IOC_w );
READ8_HANDLER ( TC0220IOC_port_r );
WRITE8_HANDLER( TC0220IOC_port_w );
READ8_HANDLER ( TC0220IOC_portreg_r );
WRITE8_HANDLER( TC0220IOC_portreg_w );

READ16_HANDLER ( TC0220IOC_halfword_port_r );
WRITE16_HANDLER( TC0220IOC_halfword_port_w );
READ16_HANDLER ( TC0220IOC_halfword_portreg_r );
WRITE16_HANDLER( TC0220IOC_halfword_portreg_w );
READ16_HANDLER ( TC0220IOC_halfword_byteswap_port_r );
WRITE16_HANDLER( TC0220IOC_halfword_byteswap_port_w );
READ16_HANDLER ( TC0220IOC_halfword_byteswap_portreg_r );
WRITE16_HANDLER( TC0220IOC_halfword_byteswap_portreg_w );
READ16_HANDLER ( TC0220IOC_halfword_r );
WRITE16_HANDLER( TC0220IOC_halfword_w );
READ16_HANDLER ( TC0220IOC_halfword_byteswap_r );
WRITE16_HANDLER( TC0220IOC_halfword_byteswap_w );

READ8_HANDLER ( TC0510NIO_r );
WRITE8_HANDLER( TC0510NIO_w );

READ16_HANDLER ( TC0510NIO_halfword_r );
WRITE16_HANDLER( TC0510NIO_halfword_w );
READ16_HANDLER ( TC0510NIO_halfword_wordswap_r );
WRITE16_HANDLER( TC0510NIO_halfword_wordswap_w );

READ8_HANDLER ( TC0640FIO_r );
WRITE8_HANDLER( TC0640FIO_w );

READ16_HANDLER ( TC0640FIO_halfword_r );
WRITE16_HANDLER( TC0640FIO_halfword_w );
READ16_HANDLER ( TC0640FIO_halfword_byteswap_r );
WRITE16_HANDLER( TC0640FIO_halfword_byteswap_w );

