/*----------- defined in video/runaway.c -----------*/

extern UINT8* runaway_video_ram;
extern UINT8* runaway_sprite_ram;

VIDEO_START( runaway );
VIDEO_START( qwak );
VIDEO_UPDATE( runaway );
VIDEO_UPDATE( qwak );

WRITE8_HANDLER( runaway_paletteram_w );
WRITE8_HANDLER( runaway_video_ram_w );
WRITE8_HANDLER( runaway_tile_bank_w );
