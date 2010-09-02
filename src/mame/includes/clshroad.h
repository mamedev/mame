/*----------- defined in video/clshroad.c -----------*/

extern UINT8 *clshroad_vram_0, *clshroad_vram_1;
extern UINT8 *clshroad_vregs;

WRITE8_HANDLER( clshroad_vram_0_w );
WRITE8_HANDLER( clshroad_vram_1_w );
WRITE8_HANDLER( clshroad_flipscreen_w );

PALETTE_INIT( firebatl );
PALETTE_INIT( clshroad );
VIDEO_START( firebatl );
VIDEO_START( clshroad );
VIDEO_UPDATE( clshroad );
