/*----------- defined in drivers/ms32.c -----------*/

extern void ms32_rearrange_sprites(running_machine *machine, const char *region);
extern void decrypt_ms32_tx(running_machine *machine, int addr_xor,int data_xor, const char *region);
extern void decrypt_ms32_bg(running_machine *machine, int addr_xor,int data_xor, const char *region);


/*----------- defined in video/ms32.c -----------*/

extern tilemap_t *ms32_tx_tilemap, *ms32_roz_tilemap, *ms32_bg_tilemap, *ms32_bg_tilemap_alt;
extern UINT8* ms32_priram_8;
extern UINT16* ms32_palram_16;
extern UINT16* ms32_rozram_16;
extern UINT16* ms32_lineram_16;
extern UINT16* ms32_sprram_16;
extern UINT16* ms32_txram_16;
extern UINT16* ms32_bgram_16;
extern UINT32 ms32_tilemaplayoutcontrol;

extern UINT16* f1superb_extraram_16;
extern tilemap_t* ms32_extra_tilemap;

//extern UINT32 *ms32_fce00000;
extern UINT32 *ms32_roz_ctrl;
extern UINT32 *ms32_tx_scroll;
extern UINT32 *ms32_bg_scroll;

extern UINT32 *ms32_mainram;

WRITE32_HANDLER( ms32_brightness_w );

WRITE32_HANDLER( ms32_gfxctrl_w );
VIDEO_START( ms32 );
VIDEO_START( f1superb );
VIDEO_UPDATE( ms32 );
