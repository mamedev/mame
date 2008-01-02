/*

    dc.h - Sega Dreamcast includes

*/

#ifndef __DC_H__
#define __DC_H__

/*----------- defined in machine/dc.c -----------*/

READ64_HANDLER( dc_sysctrl_r );
WRITE64_HANDLER( dc_sysctrl_w );
READ64_HANDLER( dc_maple_r );
WRITE64_HANDLER( dc_maple_w );
READ64_HANDLER( dc_gdrom_r );
WRITE64_HANDLER( dc_gdrom_w );
READ64_HANDLER( dc_g1_ctrl_r );
WRITE64_HANDLER( dc_g1_ctrl_w );
READ64_HANDLER( dc_g2_ctrl_r );
WRITE64_HANDLER( dc_g2_ctrl_w );
READ64_HANDLER( dc_modem_r );
WRITE64_HANDLER( dc_modem_w );
READ64_HANDLER( dc_rtc_r );
WRITE64_HANDLER( dc_rtc_w );
READ64_HANDLER( dc_aica_reg_r );
WRITE64_HANDLER( dc_aica_reg_w );

MACHINE_RESET( dc );

void dc_vblank( void );

/*----------- defined in video/dc.c -----------*/

READ64_HANDLER( pvr_ctrl_r );
WRITE64_HANDLER( pvr_ctrl_w );
READ64_HANDLER( pvr_ta_r );
WRITE64_HANDLER( pvr_ta_w );
WRITE64_HANDLER( ta_fifo_poly_w );
WRITE64_HANDLER( ta_fifo_yuv_w );
VIDEO_START(dc);
VIDEO_UPDATE(dc);

#endif
