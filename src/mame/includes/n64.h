#ifndef _INCLUDES_N64_H_
#define _INCLUDES_N64_H_

/*----------- defined in video/n64.c -----------*/

extern VIDEO_START( n64 );
extern VIDEO_UPDATE( n64 );
extern void rdp_process_list(void);

#define DACRATE_NTSC	(48681812)
#define DACRATE_PAL	(49656530)
#define DACRATE_MPAL	(48628316)

#define SP_INTERRUPT	0x1
#define SI_INTERRUPT	0x2
#define AI_INTERRUPT	0x4
#define VI_INTERRUPT	0x8
#define PI_INTERRUPT	0x10
#define DP_INTERRUPT	0x20

#define SP_STATUS_HALT			0x0001
#define SP_STATUS_BROKE			0x0002
#define SP_STATUS_DMABUSY		0x0004
#define SP_STATUS_DMAFULL		0x0008
#define SP_STATUS_IOFULL		0x0010
#define SP_STATUS_SSTEP			0x0020
#define SP_STATUS_INTR_BREAK	0x0040
#define SP_STATUS_SIGNAL0		0x0080
#define SP_STATUS_SIGNAL1		0x0100
#define SP_STATUS_SIGNAL2		0x0200
#define SP_STATUS_SIGNAL3		0x0400
#define SP_STATUS_SIGNAL4		0x0800
#define SP_STATUS_SIGNAL5		0x1000
#define SP_STATUS_SIGNAL6		0x2000
#define SP_STATUS_SIGNAL7		0x4000

#define DP_STATUS_XBUS_DMA		0x01
#define DP_STATUS_FREEZE		0x02
#define DP_STATUS_FLUSH			0x04

/*----------- defined in machine/n64.c -----------*/

extern UINT32 *rdram;
extern UINT32 *rsp_imem;
extern UINT32 *rsp_dmem;

extern void signal_rcp_interrupt(int interrupt);
extern void clear_rcp_interrupt(int interrupt);


/* read/write handlers for the N64 subsystems */
extern READ32_HANDLER( n64_mi_reg_r );
extern WRITE32_HANDLER( n64_mi_reg_w );
extern READ32_HANDLER( n64_sp_reg_r );
extern WRITE32_HANDLER( n64_sp_reg_w );
extern READ32_HANDLER( n64_dp_reg_r );
extern WRITE32_HANDLER( n64_dp_reg_w );
extern READ32_HANDLER( n64_vi_reg_r );
extern WRITE32_HANDLER( n64_vi_reg_w );
extern READ32_HANDLER( n64_ai_reg_r );
extern WRITE32_HANDLER( n64_ai_reg_w );
extern READ32_HANDLER( n64_pi_reg_r );
extern WRITE32_HANDLER( n64_pi_reg_w );
extern READ32_HANDLER( n64_ri_reg_r );
extern WRITE32_HANDLER( n64_ri_reg_w );
extern READ32_HANDLER( n64_si_reg_r );
extern WRITE32_HANDLER( n64_si_reg_w );
extern READ32_HANDLER( n64_pif_ram_r );
extern WRITE32_HANDLER( n64_pif_ram_w );

void n64_machine_reset(void);

#endif
