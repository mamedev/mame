typedef struct _cabal_state cabal_state;
struct _cabal_state
{
	UINT16 *spriteram;
	UINT16 *colorram;
	UINT16 *videoram;
	size_t spriteram_size;
	tilemap *background_layer;
	tilemap *text_layer;
	int sound_command1;
	int sound_command2;
	int last[4];
};


/*----------- defined in video/cabal.c -----------*/

extern VIDEO_START( cabal );
extern VIDEO_UPDATE( cabal );
WRITE16_HANDLER( cabal_flipscreen_w );
WRITE16_HANDLER( cabal_background_videoram16_w );
WRITE16_HANDLER( cabal_text_videoram16_w );
