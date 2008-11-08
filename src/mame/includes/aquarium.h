/*----------- defined in drivers/aquarium.c -----------*/

extern UINT16 *aquarium_scroll;

extern UINT16 *aquarium_txt_videoram;
extern UINT16 *aquarium_mid_videoram;
extern UINT16 *aquarium_bak_videoram;


/*----------- defined in video/aquarium.c -----------*/

WRITE16_HANDLER( aquarium_txt_videoram_w );
WRITE16_HANDLER( aquarium_mid_videoram_w );
WRITE16_HANDLER( aquarium_bak_videoram_w );

VIDEO_START(aquarium);
VIDEO_UPDATE(aquarium);
