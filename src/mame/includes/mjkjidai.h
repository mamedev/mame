typedef struct _mjkjidai_state mjkjidai_state;
struct _mjkjidai_state
{
	UINT8 *videoram;
	UINT8 *spriteram1;
	UINT8 *spriteram2;
	UINT8 *spriteram3;

	int keyb;
	int nvram_init_count;
	UINT8 *nvram;
	size_t nvram_size;
};


/*----------- defined in video/mjkjidai.c -----------*/

VIDEO_START( mjkjidai );
VIDEO_UPDATE( mjkjidai );
WRITE8_HANDLER( mjkjidai_videoram_w );
WRITE8_HANDLER( mjkjidai_ctrl_w );


