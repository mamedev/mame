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

int compute_interrupt_level(void);
void update_interrupt_status(void);

extern UINT32 sysctrl_regs[0x200/4];

                      /*address*/
#define SB_C2DSTAT	((0x005F6800-0x005F6800)/4)
#define SB_C2DLEN	((0x005F6804-0x005F6800)/4)
#define SB_C2DST	((0x005F6808-0x005F6800)/4)
#define SB_SBREV	((0x005F689c-0x005F6800)/4)
#define SB_ISTNRM	((0x005F6900-0x005F6800)/4)
#define SB_ISTEXT	((0x005F6904-0x005F6800)/4)
#define SB_ISTERR	((0x005F6908-0x005F6800)/4)
#define SB_IML2NRM	((0x005F6910-0x005F6800)/4)
#define SB_IML2EXT	((0x005F6914-0x005F6800)/4)
#define SB_IML2ERR	((0x005F6918-0x005F6800)/4)
#define SB_IML4NRM	((0x005F6920-0x005F6800)/4)
#define SB_IML4EXT	((0x005F6924-0x005F6800)/4)
#define SB_IML4ERR	((0x005F6928-0x005F6800)/4)
#define SB_IML6NRM	((0x005F6930-0x005F6800)/4)
#define SB_IML6EXT	((0x005F6934-0x005F6800)/4)
#define SB_IML6ERR	((0x005F6938-0x005F6800)/4)


                      /*address*/
#define SB_MDSTAR	((0x005F6C04-0x005F6C00)/4)
#define SB_MDST		((0x005F6C18-0x005F6C00)/4)
#define SB_MDTSEL	((0x005F6C10-0x005F6C00)/4)
#define SB_MSYS		((0x005F6C80-0x005F6C00)/4)


/*----------- defined in video/dc.c -----------*/

READ64_HANDLER( pvr_ctrl_r );
WRITE64_HANDLER( pvr_ctrl_w );
READ64_HANDLER( pvr_ta_r );
WRITE64_HANDLER( pvr_ta_w );
WRITE64_HANDLER( ta_fifo_poly_w );
WRITE64_HANDLER( ta_fifo_yuv_w );
VIDEO_START(dc);
VIDEO_UPDATE(dc);

                          /*address*/
#define PVRID			((0x005F8000-0x005F8000)/4)
#define REVISION		((0x005F8004-0x005F8000)/4)
#define SOFTRESET		((0x005F8008-0x005F8000)/4)
#define STARTRENDER		((0x005F8014-0x005F8000)/4)
#define VO_BORDER_COL	((0x005F8040-0x005F8000)/4)
#define VO_CONTROL		((0x005F80E8-0x005F8000)/4)
#define SPG_STATUS		((0x005F810c-0x005F8000)/4)
#define TA_LIST_INIT	((0x005F8144-0x005F8000)/4)


#endif
