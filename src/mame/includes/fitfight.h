/*----------- defined in drivers/fitfight.c -----------*/

extern UINT16 *fitfight_spriteram;
extern UINT16 *fof_700000;
extern UINT16 *fof_900000;
extern UINT16 *fof_a00000;

extern UINT16 *fof_bak_tileram;
extern UINT16 *fof_mid_tileram;
extern UINT16 *fof_txt_tileram;
extern char bbprot_kludge;


/*----------- defined in video/fitfight.c -----------*/

WRITE16_HANDLER( fof_bak_tileram_w );
WRITE16_HANDLER( fof_mid_tileram_w );
WRITE16_HANDLER( fof_txt_tileram_w );
VIDEO_START(fitfight);
VIDEO_UPDATE(fitfight);
