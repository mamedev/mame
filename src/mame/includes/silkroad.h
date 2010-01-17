typedef struct _silkroad_state silkroad_state;
struct _silkroad_state
{
	UINT32 *vidram;
	UINT32 *vidram2;
	UINT32 *vidram3;
	UINT32 *sprram;
	UINT32 *regs;
	tilemap_t *fg_tilemap;
	tilemap_t *fg2_tilemap;
	tilemap_t *fg3_tilemap;
};


/*----------- defined in video/silkroad.c -----------*/

WRITE32_HANDLER( silkroad_fgram_w );
WRITE32_HANDLER( silkroad_fgram2_w );
WRITE32_HANDLER( silkroad_fgram3_w );
VIDEO_START(silkroad);
VIDEO_UPDATE(silkroad);
