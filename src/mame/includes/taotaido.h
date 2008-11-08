/*----------- defined in video/taotaido.c -----------*/

extern UINT16 *taotaido_spriteram;
extern UINT16 *taotaido_spriteram2;
extern UINT16 *taotaido_scrollram;
extern UINT16 *taotaido_bgram;

WRITE16_HANDLER( taotaido_sprite_character_bank_select_w );
WRITE16_HANDLER( taotaido_tileregs_w );
WRITE16_HANDLER( taotaido_bgvideoram_w );
VIDEO_START( taotaido );
VIDEO_UPDATE( taotaido );
VIDEO_EOF( taotaido );
