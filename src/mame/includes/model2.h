/*----------- defined in drivers/model2.c -----------*/

extern UINT32 geo_read_start_address;
extern UINT32 geo_write_start_address;
extern UINT32 *model2_bufferram;
extern UINT32 *model2_colorxlat;
extern UINT32 *model2_textureram0;
extern UINT32 *model2_textureram1;
extern UINT32 *model2_lumaram;
extern UINT32 *model2_paletteram32;


/*----------- defined in video/model2.c -----------*/

VIDEO_START(model2);
VIDEO_UPDATE(model2);

void model2_3d_set_zclip( UINT8 clip );
