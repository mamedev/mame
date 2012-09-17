/*----------- defined in video/sei_crtc.c -----------*/

extern UINT16 seibucrtc_sc0bank;

DECLARE_WRITE16_HANDLER( seibucrtc_sc0vram_w );
DECLARE_WRITE16_HANDLER( seibucrtc_sc1vram_w );
DECLARE_WRITE16_HANDLER( seibucrtc_sc2vram_w );
DECLARE_WRITE16_HANDLER( seibucrtc_sc3vram_w );
DECLARE_WRITE16_HANDLER( seibucrtc_vregs_w );
void seibucrtc_sc0bank_w(UINT16 data);
VIDEO_START( seibu_crtc );
SCREEN_UPDATE_IND16( seibu_crtc );
