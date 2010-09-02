/*----------- defined in video/deadang.c -----------*/

extern UINT16 *deadang_video_data, *deadang_scroll_ram;

WRITE16_HANDLER( deadang_foreground_w );
WRITE16_HANDLER( deadang_text_w );
WRITE16_HANDLER( deadang_bank_w );

VIDEO_START( deadang );
VIDEO_UPDATE( deadang );
