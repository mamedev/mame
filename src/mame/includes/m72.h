/*************************************************************************

    Irem M72 hardware

*************************************************************************/

/*----------- defined in video/m72.c -----------*/

extern UINT16 *m72_videoram1,*m72_videoram2,*majtitle_rowscrollram;

MACHINE_RESET( m72 );
MACHINE_RESET( xmultipl );
MACHINE_RESET( kengo );

INTERRUPT_GEN( m72_interrupt );

VIDEO_START( m72 );
VIDEO_START( rtype2 );
VIDEO_START( majtitle );
VIDEO_START( hharry );
VIDEO_START( poundfor );

READ16_HANDLER( m72_palette1_r );
READ16_HANDLER( m72_palette2_r );
WRITE16_HANDLER( m72_palette1_w );
WRITE16_HANDLER( m72_palette2_w );
WRITE16_HANDLER( m72_videoram1_w );
WRITE16_HANDLER( m72_videoram2_w );
WRITE16_HANDLER( m72_irq_line_w );
WRITE16_HANDLER( m72_scrollx1_w );
WRITE16_HANDLER( m72_scrollx2_w );
WRITE16_HANDLER( m72_scrolly1_w );
WRITE16_HANDLER( m72_scrolly2_w );
WRITE16_HANDLER( m72_dmaon_w );
WRITE16_HANDLER( m72_port02_w );
WRITE16_HANDLER( rtype2_port02_w );
WRITE16_HANDLER( majtitle_gfx_ctrl_w );

VIDEO_UPDATE( m72 );
VIDEO_UPDATE( majtitle );
