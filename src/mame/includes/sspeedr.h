/*----------- defined in video/sspeedr.c -----------*/

WRITE8_HANDLER( sspeedr_driver_horz_w );
WRITE8_HANDLER( sspeedr_driver_horz_2_w );
WRITE8_HANDLER( sspeedr_driver_vert_w );
WRITE8_HANDLER( sspeedr_driver_pic_w );

WRITE8_HANDLER( sspeedr_drones_horz_w );
WRITE8_HANDLER( sspeedr_drones_horz_2_w );
WRITE8_HANDLER( sspeedr_drones_vert_w );
WRITE8_HANDLER( sspeedr_drones_mask_w );

WRITE8_HANDLER( sspeedr_track_horz_w );
WRITE8_HANDLER( sspeedr_track_horz_2_w );
WRITE8_HANDLER( sspeedr_track_vert_w );
WRITE8_HANDLER( sspeedr_track_ice_w );

VIDEO_START( sspeedr );
VIDEO_UPDATE( sspeedr );
VIDEO_EOF( sspeedr );
