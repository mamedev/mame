/*************************************************************************

    Mouser

*************************************************************************/

typedef struct _mouser_state mouser_state;
struct _mouser_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* misc */
	UINT8      sound_byte;
	UINT8      nmi_enable;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/mouser.c -----------*/

WRITE8_HANDLER( mouser_flip_screen_x_w );
WRITE8_HANDLER( mouser_flip_screen_y_w );

PALETTE_INIT( mouser );
VIDEO_UPDATE( mouser );
