

typedef struct _fgoal_state fgoal_state;
struct _fgoal_state
{
	/* memory pointers */
	UINT8 *    video_ram;

	/* video-related */
	bitmap_t   *bgbitmap, *fgbitmap;
	UINT8      xpos, ypos;
	int        current_color;

	/* misc */
	int        fgoal_player;
	UINT8      row, col;
	unsigned   shift_data, shift_bits;
	int        prev_coin;

	/* devices */
	const device_config *maincpu;
};


/*----------- defined in video/fgoal.c -----------*/

VIDEO_START( fgoal );
VIDEO_UPDATE( fgoal );

WRITE8_HANDLER( fgoal_color_w );
WRITE8_HANDLER( fgoal_xpos_w );
WRITE8_HANDLER( fgoal_ypos_w );

