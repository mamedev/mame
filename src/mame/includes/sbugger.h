typedef struct _sbugger_state sbugger_state;
struct _sbugger_state
{
	UINT8 *videoram;
	UINT8 *videoram_attr;

	tilemap_t *tilemap;
};


/*----------- defined in video/sbugger.c -----------*/

PALETTE_INIT(sbugger);
VIDEO_UPDATE(sbugger);
VIDEO_START(sbugger);
WRITE8_HANDLER( sbugger_videoram_attr_w );
WRITE8_HANDLER( sbugger_videoram_w );
