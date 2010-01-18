/*----------- defined in audio/toaplan2.c -----------*/

void dogyuun_okisnd_w(running_device *device, int data);
void kbash_okisnd_w(running_device *device, int data);
void fixeight_okisnd_w(running_device *device, int data);
void batsugun_okisnd_w(running_device *device, int data);


/*----------- defined in drivers/toaplan2.c -----------*/

extern int toaplan2_sub_cpu;


/*----------- defined in video/toaplan2.c -----------*/

extern UINT16 *toaplan2_txvideoram16;
extern UINT16 *toaplan2_txvideoram16_offs;
extern UINT16 *toaplan2_txscrollram16;
extern UINT16 *toaplan2_tx_gfxram16;

extern  size_t toaplan2_tx_vram_size;
extern  size_t toaplan2_tx_offs_vram_size;
extern  size_t toaplan2_tx_scroll_vram_size;
extern  size_t batrider_paletteram16_size;

VIDEO_EOF( toaplan2_0 );
VIDEO_EOF( toaplan2_1 );
VIDEO_START( toaplan2_0 );
VIDEO_START( toaplan2_1 );
VIDEO_START( truxton2_0 );
VIDEO_START( bgaregga_0 );
VIDEO_START( batrider_0 );
VIDEO_UPDATE( toaplan2_0 );
VIDEO_UPDATE( truxton2_0 );
VIDEO_UPDATE( dogyuun_1 );
VIDEO_UPDATE( batsugun_1 );
VIDEO_UPDATE( batrider_0 );
VIDEO_UPDATE( mahoudai_0 );

WRITE16_HANDLER( toaplan2_0_voffs_w );
WRITE16_HANDLER( toaplan2_1_voffs_w );

READ16_HANDLER ( toaplan2_0_videoram16_r );
READ16_HANDLER ( toaplan2_1_videoram16_r );
WRITE16_HANDLER( toaplan2_0_videoram16_w );
WRITE16_HANDLER( toaplan2_1_videoram16_w );

READ16_HANDLER ( toaplan2_txvideoram16_r );
WRITE16_HANDLER( toaplan2_txvideoram16_w );
READ16_HANDLER ( toaplan2_txvideoram16_offs_r );
WRITE16_HANDLER( toaplan2_txvideoram16_offs_w );
READ16_HANDLER ( toaplan2_txscrollram16_r );
WRITE16_HANDLER( toaplan2_txscrollram16_w );
READ16_HANDLER ( toaplan2_tx_gfxram16_r );
WRITE16_HANDLER( toaplan2_tx_gfxram16_w );
READ16_HANDLER ( raizing_tx_gfxram16_r );
WRITE16_HANDLER( raizing_tx_gfxram16_w );

WRITE16_HANDLER( toaplan2_0_scroll_reg_select_w );
WRITE16_HANDLER( toaplan2_1_scroll_reg_select_w );
WRITE16_HANDLER( toaplan2_0_scroll_reg_data_w );
WRITE16_HANDLER( toaplan2_1_scroll_reg_data_w );

WRITE16_HANDLER( batrider_objectbank_w );
WRITE16_HANDLER( batrider_textdata_decode );

READ16_HANDLER ( pipibibi_videoram16_r );
WRITE16_HANDLER( pipibibi_videoram16_w );
READ16_HANDLER ( pipibibi_spriteram16_r );
WRITE16_HANDLER( pipibibi_spriteram16_w );
WRITE16_HANDLER( pipibibi_scroll_w );
