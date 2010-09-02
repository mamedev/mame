/*----------- defined in video/skykid.c -----------*/

extern UINT8 *skykid_textram, *skykid_videoram, *skykid_spriteram;

VIDEO_START( skykid );
VIDEO_UPDATE( skykid );
PALETTE_INIT( skykid );

READ8_HANDLER( skykid_videoram_r );
WRITE8_HANDLER( skykid_videoram_w );
READ8_HANDLER( skykid_textram_r );
WRITE8_HANDLER( skykid_textram_w );
WRITE8_HANDLER( skykid_scroll_x_w );
WRITE8_HANDLER( skykid_scroll_y_w );
WRITE8_HANDLER( skykid_flipscreen_priority_w );
