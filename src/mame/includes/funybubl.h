

typedef struct _funybubl_state funybubl_state;
struct _funybubl_state
{
	/* memory pointers */
	UINT8 *    banked_vram;
	UINT8 *    paletteram;

	/* devices */
	const device_config *audiocpu;
};



/*----------- defined in video/funybubl.c -----------*/

WRITE8_HANDLER ( funybubl_paldatawrite );

VIDEO_START(funybubl);
VIDEO_UPDATE(funybubl);
