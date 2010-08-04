/*************************************************************************

    Othello Derby

*************************************************************************/

#define OTHLDRBY_VREG_SIZE   18

class othldrby_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, othldrby_state(machine)); }

	othldrby_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *     vram;
	UINT16 *     buf_spriteram;
	UINT16 *     buf_spriteram2;

	/* video-related */
	tilemap_t    *bg_tilemap[3];
	UINT16       vreg[OTHLDRBY_VREG_SIZE];
	UINT32       vram_addr, vreg_addr;

	/* misc */
	int          toggle;
};


/*----------- defined in video/othldrby.c -----------*/

WRITE16_HANDLER( othldrby_videoram_addr_w );
READ16_HANDLER( othldrby_videoram_r );
WRITE16_HANDLER( othldrby_videoram_w );
WRITE16_HANDLER( othldrby_vreg_addr_w );
WRITE16_HANDLER( othldrby_vreg_w );

VIDEO_START( othldrby );
VIDEO_EOF( othldrby );
VIDEO_UPDATE( othldrby );
