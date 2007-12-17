/*************************************************************************

    Venture Line Super Rider driver

**************************************************************************/

/*----------- defined in video/suprridr.c -----------*/

extern UINT8 *suprridr_fgram;
extern UINT8 *suprridr_bgram;

VIDEO_START( suprridr );
PALETTE_INIT( suprridr );

WRITE8_HANDLER( suprridr_flipx_w );
WRITE8_HANDLER( suprridr_flipy_w );
WRITE8_HANDLER( suprridr_fgdisable_w );
WRITE8_HANDLER( suprridr_fgscrolly_w );
WRITE8_HANDLER( suprridr_bgscrolly_w );

WRITE8_HANDLER( suprridr_fgram_w );
WRITE8_HANDLER( suprridr_bgram_w );

VIDEO_UPDATE( suprridr );
