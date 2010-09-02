/*----------- defined in video/djmain.c -----------*/

extern UINT32 *djmain_obj_ram;

VIDEO_UPDATE( djmain );
VIDEO_START( djmain );

void djmain_tile_callback(running_machine* machine, int layer, int *code, int *color, int *flags);
