/*----------- defined in drivers/ms32.c -----------*/

extern void ms32_rearrange_sprites(running_machine *machine, const char *region);
extern void decrypt_ms32_tx(running_machine *machine, int addr_xor,int data_xor, const char *region);
extern void decrypt_ms32_bg(running_machine *machine, int addr_xor,int data_xor, const char *region);


/*----------- defined in video/ms32.c -----------*/

//extern UINT32 *ms32_fce00000;
extern UINT32 *ms32_roz_ctrl;
extern UINT32 *ms32_tx_scroll;
extern UINT32 *ms32_bg_scroll;

/*
extern UINT32 *ms32_priram;
extern UINT32 *ms32_palram;
extern UINT32 *ms32_bgram;
extern UINT32 *ms32_rozram;
extern UINT32 *ms32_lineram;
extern UINT32 *ms32_spram;
extern UINT32 *ms32_txram;
*/
extern UINT32 *ms32_mainram;

WRITE32_HANDLER( ms32_brightness_w );
/*
WRITE32_HANDLER( ms32_palram_w );
READ32_HANDLER( ms32_txram_r );
WRITE32_HANDLER( ms32_txram_w );
READ32_HANDLER( ms32_rozram_r );
WRITE32_HANDLER( ms32_rozram_w );
READ32_HANDLER( ms32_lineram_r );
WRITE32_HANDLER( ms32_lineram_w );
READ32_HANDLER( ms32_bgram_r );
WRITE32_HANDLER( ms32_bgram_w );
READ32_HANDLER( ms32_spram_r );
WRITE32_HANDLER( ms32_spram_w );
*/

WRITE32_HANDLER( ms32_gfxctrl_w );
VIDEO_START( ms32 );
VIDEO_START( f1superb );
VIDEO_UPDATE( ms32 );
