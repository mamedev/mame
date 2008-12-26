/*----------- defined in video/vball.c -----------*/

extern UINT8 *vb_attribram;
extern UINT8 *vb_videoram;
extern UINT8 *vb_scrolly_lo;
extern int vb_scrollx_hi;
extern int vb_scrolly_hi;
extern int vb_scrollx_lo;
extern int vball_gfxset;
extern int scrollx[256];

VIDEO_START( vb );
VIDEO_UPDATE( vb );
void vb_bgprombank_w(running_machine *machine, int bank);
void vb_spprombank_w(running_machine *machine, int bank);
WRITE8_HANDLER( vb_attrib_w );
WRITE8_HANDLER( vb_videoram_w );
void vb_mark_all_dirty(void);
