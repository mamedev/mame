/*************************************************************************

    Pirate Ship Higemaru

*************************************************************************/

class higemaru_state : public driver_device
{
public:
	higemaru_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
};


/*----------- defined in video/higemaru.c -----------*/

WRITE8_HANDLER( higemaru_videoram_w );
WRITE8_HANDLER( higemaru_colorram_w );
WRITE8_HANDLER( higemaru_c800_w );

PALETTE_INIT( higemaru );
VIDEO_START( higemaru );
VIDEO_UPDATE( higemaru );
