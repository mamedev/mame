/*

    dc.h - Sega Dreamcast includes

*/

#ifndef __DC_H__
#define __DC_H__

class dc_state : public driver_device
{
	public:
		dc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT64 *dc_framebuffer_ram; // '32-bit access area'
	UINT64 *dc_texture_ram; // '64-bit access area'

	UINT32 *dc_sound_ram;

	/* machine related */
	UINT32 dc_rtcregister[4];
	UINT32 dc_sysctrl_regs[0x200/4];
	UINT32 g1bus_regs[0x100/4]; // DC-only
	UINT32 g2bus_regs[0x100/4];

	emu_timer *dc_rtc_timer;

	struct {
		UINT32 aica_addr;
		UINT32 root_addr;
		UINT32 size;
		UINT8 dir;
		UINT8 flag;
		UINT8 indirect;
		UINT8 start;
		UINT8 sel;
	}m_wave_dma;

	struct {
		UINT32 pvr_addr;
		UINT32 sys_addr;
		UINT32 size;
		UINT8 sel;
		UINT8 dir;
		UINT8 flag;
		UINT8 start;
	}m_pvr_dma;

	/* video related */
	UINT32 pvrta_regs[0x2000/4];
	UINT32 pvrctrl_regs[0x100/4];
	UINT32 debug_dip_status;
	emu_timer *vbout_timer;
	emu_timer *vbin_timer;
	emu_timer *hbin_timer;
	emu_timer *endofrender_timer_isp;
	emu_timer *endofrender_timer_tsp;
	emu_timer *endofrender_timer_video;
	UINT32 tafifo_buff[32];
	int scanline;
	int next_y;

	/* Naomi 2 specific (To be moved) */
	UINT64 *pvr2_texture_ram;
	UINT64 *pvr2_framebuffer_ram;
	UINT64 *elan_ram;
};

/*----------- defined in machine/dc.c -----------*/

READ64_HANDLER( pvr_ctrl_r );
WRITE64_HANDLER( pvr_ctrl_w );

READ64_HANDLER( dc_sysctrl_r );
WRITE64_HANDLER( dc_sysctrl_w );
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
READ64_DEVICE_HANDLER( dc_aica_reg_r );
WRITE64_DEVICE_HANDLER( dc_aica_reg_w );

READ32_DEVICE_HANDLER( dc_arm_aica_r );
WRITE32_DEVICE_HANDLER( dc_arm_aica_w );

MACHINE_START( dc );
MACHINE_RESET( dc );

int dc_compute_interrupt_level(running_machine &machine);
void dc_update_interrupt_status(running_machine &machine);

/*--------- Ch2-DMA Control Registers ----------*/
#define SB_C2DSTAT	((0x005f6800-0x005f6800)/4)
#define SB_C2DLEN	((0x005f6804-0x005f6800)/4)
#define SB_C2DST	((0x005f6808-0x005f6800)/4)
/*-------- Sort-DMA Control Registers ----------*/
#define SB_SDSTAW	((0x005f6810-0x005f6800)/4)
#define SB_SDBAAW	((0x005f6814-0x005f6800)/4)
#define SB_SDWLT	((0x005f6818-0x005f6800)/4)
#define SB_SDLAS	((0x005f681c-0x005f6800)/4)
#define SB_SDST		((0x005f6820-0x005f6800)/4)
/*-- DDT I/F Block & System Control Registers --*/
#define SB_DBREQM	((0x005f6840-0x005f6800)/4)
#define SB_BAVLWC	((0x005f6844-0x005f6800)/4)
#define SB_C2DPRYC	((0x005f6848-0x005f6800)/4)
#define SB_C2DMAXL	((0x005f684c-0x005f6800)/4)
#define SB_TFREM	((0x005f6880-0x005f6800)/4)
#define SB_LMMODE0	((0x005f6884-0x005f6800)/4)
#define SB_LMMODE1	((0x005f6888-0x005f6800)/4)
#define SB_FFST		((0x005f688c-0x005f6800)/4)
#define SB_SFRES	((0x005f6890-0x005f6800)/4)
#define SB_SBREV	((0x005f689c-0x005f6800)/4)
#define SB_RBSPLT	((0x005f68a0-0x005f6800)/4)
/*-------- Interrupt Control Registers ---------*/
#define SB_ISTNRM	((0x005f6900-0x005f6800)/4)
#define SB_ISTEXT	((0x005f6904-0x005f6800)/4)
#define SB_ISTERR	((0x005f6908-0x005f6800)/4)
#define SB_IML2NRM	((0x005f6910-0x005f6800)/4)
#define SB_IML2EXT	((0x005f6914-0x005f6800)/4)
#define SB_IML2ERR	((0x005f6918-0x005f6800)/4)
#define SB_IML4NRM	((0x005f6920-0x005f6800)/4)
#define SB_IML4EXT	((0x005f6924-0x005f6800)/4)
#define SB_IML4ERR	((0x005f6928-0x005f6800)/4)
#define SB_IML6NRM	((0x005f6930-0x005f6800)/4)
#define SB_IML6EXT	((0x005f6934-0x005f6800)/4)
#define SB_IML6ERR	((0x005f6938-0x005f6800)/4)
#define SB_PDTNRM	((0x005f6940-0x005f6800)/4)
#define SB_PDTEXT	((0x005f6944-0x005f6800)/4)
#define SB_G2DTNRM	((0x005f6950-0x005f6800)/4)
#define SB_G2DTEXT	((0x005f6954-0x005f6800)/4)


/*-------- Maple-DMA Control Registers ---------*/
#define SB_MDSTAR	((0x005f6c04-0x005f6c00)/4)
#define SB_MDTSEL	((0x005f6c10-0x005f6c00)/4)
#define SB_MDEN		((0x005f6c14-0x005f6c00)/4)
#define SB_MDST		((0x005f6c18-0x005f6c00)/4)
/*---- Maple I/F Block HW Control Registers ----*/
#define SB_MSYS		((0x005f6c80-0x005f6c00)/4)
#define SB_MST		((0x005f6c84-0x005f6c00)/4)
#define SB_MSHTCL	((0x005f6c88-0x005f6c00)/4)
#define SB_MDAPRO	((0x005f6c8c-0x005f6c00)/4)
#define SB_MMSEL	((0x005f6ce8-0x005f6c00)/4)
/*-------- Maple-DMA Debug Registers -----------*/
#define SB_MTXDAD	((0x005f6cf4-0x005f6c00)/4)
#define SB_MRXDAD	((0x005f6cf8-0x005f6c00)/4)
#define SB_MRXDBD	((0x005f6cfc-0x005f6c00)/4)

/*--------- GD-DMA Control Registers -----------*/
#define SB_GDSTAR	((0x005f7404-0x005f7400)/4)
#define SB_GDLEN	((0x005f7408-0x005f7400)/4)
#define SB_GDDIR	((0x005f740c-0x005f7400)/4)
#define SB_GDEN		((0x005f7414-0x005f7400)/4)
#define SB_GDST		((0x005f7418-0x005f7400)/4)
/*----- G1 I/F Block HW Control Registers ------*/
#define SB_G1RRC	((0x005f7480-0x005f7400)/4)
#define SB_G1RWC	((0x005f7484-0x005f7400)/4)
#define SB_G1FRC	((0x005f7488-0x005f7400)/4)
#define SB_G1FWC	((0x005f748c-0x005f7400)/4)
#define SB_G1CRC	((0x005f7490-0x005f7400)/4)
#define SB_G1CWC	((0x005f7494-0x005f7400)/4)
#define SB_G1GDRC	((0x005f74a0-0x005f7400)/4)
#define SB_G1GDWC	((0x005f74a4-0x005f7400)/4)
#define SB_G1SYSM	((0x005f74b0-0x005f7400)/4)
#define SB_G1CRDYC	((0x005f74b4-0x005f7400)/4)
#define SB_GDAPRO	((0x005f74b8-0x005f7400)/4)
/*---------- GD-DMA Debug Registers ------------*/
#define SB_GDSTARD	((0x005f74f4-0x005f7400)/4)
#define SB_GDLEND	((0x005f74f8-0x005f7400)/4)

/*-------- Wave DMA Control Registers ----------*/
#define SB_ADSTAG	((0x005f7800-0x005f7800)/4)
#define SB_ADSTAR	((0x005f7804-0x005f7800)/4)
#define SB_ADLEN	((0x005f7808-0x005f7800)/4)
#define SB_ADDIR	((0x005f780c-0x005f7800)/4)
#define SB_ADTSEL	((0x005f7810-0x005f7800)/4)
#define SB_ADTRG	SB_ADTSEL
#define SB_ADEN		((0x005f7814-0x005f7800)/4)
#define SB_ADST		((0x005f7818-0x005f7800)/4)
#define SB_ADSUSP	((0x005f781c-0x005f7800)/4)

/*----- External 1 DMA Control Registers -------*/
#define SB_E1STAG	((0x005f7820-0x005f7800)/4)
#define SB_E1STAR	((0x005f7824-0x005f7800)/4)
#define SB_E1LEN	((0x005f7828-0x005f7800)/4)
#define SB_E1DIR	((0x005f782c-0x005f7800)/4)
#define SB_E1TSEL	((0x005f7830-0x005f7800)/4)
#define SB_E1TRG	SB_E1TSEL
#define SB_E1EN		((0x005f7834-0x005f7800)/4)
#define SB_E1ST		((0x005f7838-0x005f7800)/4)
#define SB_E1SUSP	((0x005f783c-0x005f7800)/4)

/*----- External 2 DMA Control Registers -------*/
#define SB_E2STAG	((0x005f7840-0x005f7800)/4)
#define SB_E2STAR	((0x005f7844-0x005f7800)/4)
#define SB_E2LEN	((0x005f7848-0x005f7800)/4)
#define SB_E2DIR	((0x005f784c-0x005f7800)/4)
#define SB_E2TSEL	((0x005f7850-0x005f7800)/4)
#define SB_E2TRG	SB_E2TSEL
#define SB_E2EN		((0x005f7854-0x005f7800)/4)
#define SB_E2ST		((0x005f7858-0x005f7800)/4)
#define SB_E2SUSP	((0x005f785c-0x005f7800)/4)

/*------- Debug DMA Control Registers ----------*/
#define SB_DDSTAG	((0x005f7860-0x005f7800)/4)
#define SB_DDSTAR	((0x005f7864-0x005f7800)/4)
#define SB_DDLEN	((0x005f7868-0x005f7800)/4)
#define SB_DDDIR	((0x005f786c-0x005f7800)/4)
#define SB_DDTSEL	((0x005f7870-0x005f7800)/4)
#define SB_DDTRG	SB_DDTSEL
#define SB_DDEN		((0x005f7874-0x005f7800)/4)
#define SB_DDST		((0x005f7878-0x005f7800)/4)
#define SB_DDSUSP	((0x005f787c-0x005f7800)/4)
/*----- G2 I/F Block HW Control Registers ------*/
#define SB_G2ID		((0x005f7880-0x005f7800)/4)
#define SB_G2DSTO	((0x005f7890-0x005f7800)/4)
#define SB_G2TRTO	((0x005f7894-0x005f7800)/4)
#define SB_G2MDMTO	((0x005f7898-0x005f7800)/4)
#define SB_G2MDMW	((0x005f789c-0x005f7800)/4)
#define SB_G2APRO	((0x005f78bc-0x005f7800)/4)

/*---------- G2 DMA Debug Registers ------------*/
#define SB_ADSTAGD	((0x005f78c0-0x005f7800)/4)
#define SB_ADSTARD	((0x005f78c4-0x005f7800)/4)
#define SB_ADLEND	((0x005f78c8-0x005f7800)/4)
#define SB_E1STAGD	((0x005f78d0-0x005f7800)/4)
#define SB_E1STARD	((0x005f78d4-0x005f7800)/4)
#define SB_E1LEND	((0x005f78d8-0x005f7800)/4)
#define SB_E2STAGD	((0x005f78e0-0x005f7800)/4)
#define SB_E2STARD	((0x005f78e4-0x005f7800)/4)
#define SB_E2LEND	((0x005f78e8-0x005f7800)/4)
#define SB_DDSTAGD	((0x005f78f0-0x005f7800)/4)
#define SB_DDSTARD	((0x005f78f4-0x005f7800)/4)
#define SB_DDLEND	((0x005f78f8-0x005f7800)/4)

/*------------- PowerVR Interface -------------*/
#define SB_PDSTAP   ((0x005f7c00-0x005f7c00)/4)
#define SB_PDSTAR   ((0x005f7c04-0x005f7c00)/4)
#define SB_PDLEN    ((0x005f7c08-0x005f7c00)/4)
#define SB_PDDIR    ((0x005f7c0c-0x005f7c00)/4)
#define SB_PDTSEL   ((0x005f7c10-0x005f7c00)/4)
#define SB_PDEN     ((0x005f7c14-0x005f7c00)/4)
#define SB_PDST		((0x005f7c18-0x005f7c00)/4)
#define SB_PDAPRO	((0x005f7c80-0x005f7c00)/4)

#define RTC1		((0x00710000-0x00710000)/4)
#define RTC2		((0x00710004-0x00710000)/4)
#define RTC3		((0x00710008-0x00710000)/4)


/*----------- defined in video/dc.c -----------*/

extern UINT32 pvrctrl_regs[0x100/4];
extern UINT64 *dc_texture_ram;
extern UINT64 *dc_framebuffer_ram;

extern UINT64 *pvr2_texture_ram;
extern UINT64 *pvr2_framebuffer_ram;
extern UINT64 *elan_ram;

READ64_HANDLER( pvr_ta_r );
WRITE64_HANDLER( pvr_ta_w );
READ64_HANDLER( pvr2_ta_r );
WRITE64_HANDLER( pvr2_ta_w );
READ64_HANDLER( pvrs_ta_r );
WRITE64_HANDLER( pvrs_ta_w );
READ32_HANDLER( elan_regs_r );
WRITE32_HANDLER( elan_regs_w );
WRITE64_HANDLER( ta_fifo_poly_w );
WRITE64_HANDLER( ta_fifo_yuv_w );
VIDEO_START(dc);
SCREEN_UPDATE(dc);

/*--------------- CORE registers --------------*/
#define PVRID				((0x005f8000-0x005f8000)/4)
#define REVISION			((0x005f8004-0x005f8000)/4)
#define SOFTRESET			((0x005f8008-0x005f8000)/4)
#define STARTRENDER			((0x005f8014-0x005f8000)/4)
#define TEST_SELECT			((0x005f8018-0x005f8000)/4)
#define PARAM_BASE			((0x005f8020-0x005f8000)/4)
#define REGION_BASE			((0x005f802c-0x005f8000)/4)
#define SPAN_SORT_CFG		((0x005f8030-0x005f8000)/4)
#define VO_BORDER_COL		((0x005f8040-0x005f8000)/4)
#define FB_R_CTRL			((0x005f8044-0x005f8000)/4)
#define FB_W_CTRL			((0x005f8048-0x005f8000)/4)
#define FB_W_LINESTRIDE		((0x005f804c-0x005f8000)/4)
#define FB_R_SOF1			((0x005f8050-0x005f8000)/4)
#define FB_R_SOF2			((0x005f8054-0x005f8000)/4)
#define FB_R_SIZE			((0x005f805c-0x005f8000)/4)
#define FB_W_SOF1			((0x005f8060-0x005f8000)/4)
#define FB_W_SOF2			((0x005f8064-0x005f8000)/4)
#define FB_X_CLIP			((0x005f8068-0x005f8000)/4)
#define FB_Y_CLIP			((0x005f806c-0x005f8000)/4)
#define FPU_SHAD_SCALE		((0x005f8074-0x005f8000)/4)
#define FPU_CULL_VAL		((0x005f8078-0x005f8000)/4)
#define FPU_PARAM_CFG		((0x005f807c-0x005f8000)/4)
#define HALF_OFFSET			((0x005f8080-0x005f8000)/4)
#define FPU_PERP_VAL		((0x005f8084-0x005f8000)/4)
#define ISP_BACKGND_D		((0x005f8088-0x005f8000)/4)
#define ISP_BACKGND_T		((0x005f808c-0x005f8000)/4)
#define ISP_FEED_CFG		((0x005f8098-0x005f8000)/4)
#define SDRAM_REFRESH		((0x005f80a0-0x005f8000)/4)
#define SDRAM_ARB_CFG		((0x005f80a4-0x005f8000)/4)
#define SDRAM_CFG			((0x005f80a8-0x005f8000)/4)
#define FOG_COL_RAM			((0x005f80b0-0x005f8000)/4)
#define FOG_COL_VERT		((0x005f80b4-0x005f8000)/4)
#define FOG_DENSITY			((0x005f80b8-0x005f8000)/4)
#define FOG_CLAMP_MAX		((0x005f80bc-0x005f8000)/4)
#define FOG_CLAMP_MIN		((0x005f80c0-0x005f8000)/4)
#define SPG_TRIGGER_POS		((0x005f80c4-0x005f8000)/4)
#define SPG_HBLANK_INT		((0x005f80c8-0x005f8000)/4)
#define SPG_VBLANK_INT		((0x005f80cc-0x005f8000)/4)
#define SPG_CONTROL			((0x005f80d0-0x005f8000)/4)
#define SPG_HBLANK			((0x005f80d4-0x005f8000)/4)
#define SPG_LOAD			((0x005f80d8-0x005f8000)/4)
#define SPG_VBLANK			((0x005f80dc-0x005f8000)/4)
#define SPG_WIDTH			((0x005f80e0-0x005f8000)/4)
#define TEXT_CONTROL		((0x005f80e4-0x005f8000)/4)
#define VO_CONTROL			((0x005f80e8-0x005f8000)/4)
#define VO_STARTX			((0x005f80ec-0x005f8000)/4)
#define VO_STARTY			((0x005f80f0-0x005f8000)/4)
#define SCALER_CTL			((0x005f80f4-0x005f8000)/4)
#define PAL_RAM_CTRL		((0x005f8108-0x005f8000)/4)
#define ISP_BACKGND_T		((0x005f808c-0x005f8000)/4)
#define SPG_STATUS			((0x005f810c-0x005f8000)/4)
#define FB_BURSTCTRL		((0x005f8110-0x005f8000)/4)
#define Y_COEFF				((0x005f8118-0x005f8000)/4)
#define PT_ALPHA_REF		((0x005f811c-0x005f8000)/4)
/* 0x005f8200 - 0x005f83ff fog_table */
/* 0x005f9000 - 0x005f9fff palette_ram */

/*--------- Tile Accelerator registers ---------*/
#define TA_OL_BASE			((0x005f8124-0x005f8000)/4)
#define TA_ISP_BASE			((0x005f8128-0x005f8000)/4)
#define TA_OL_LIMIT			((0x005f812c-0x005f8000)/4)
#define TA_ISP_LIMIT		((0x005f8130-0x005f8000)/4)
#define TA_NEXT_OPB			((0x005f8134-0x005f8000)/4)
#define TA_ITP_CURRENT		((0x005f8138-0x005f8000)/4)
#define TA_GLOB_TILE_CLIP	((0x005f813c-0x005f8000)/4)
#define TA_ALLOC_CTRL		((0x005f8140-0x005f8000)/4)
#define TA_LIST_INIT		((0x005f8144-0x005f8000)/4)
#define TA_YUV_TEX_BASE		((0x005f8148-0x005f8000)/4)
#define TA_YUV_TEX_CTRL		((0x005f814c-0x005f8000)/4)
#define TA_YUV_TEX_CNT		((0x005f8150-0x005f8000)/4)
#define TA_LIST_CONT		((0x005f8160-0x005f8000)/4)
#define TA_NEXT_OPB_INIT	((0x005f8164-0x005f8000)/4)
/* 0x005f8600 - 0x005f8f5c TA_OL_POINTERS (read only) */

/* ------------- normal interrupts ------------- */
#define IST_EOR_VIDEO	0x00000001
#define IST_EOR_ISP	0x00000002
#define IST_EOR_TSP	0x00000004
#define IST_VBL_IN	0x00000008
#define IST_VBL_OUT	0x00000010
#define IST_HBL_IN	0x00000020
#define IST_EOXFER_YUV	0x00000040
#define IST_EOXFER_OPLST 0x00000080
#define IST_EOXFER_OPMV 0x00000100
#define IST_EOXFER_TRLST 0x00000200
#define IST_EOXFER_TRMV 0x00000400
#define IST_DMA_PVR	0x00000800
#define IST_DMA_MAPLE	0x00001000
#define IST_DMA_MAPLEVB 0x00002000
#define IST_DMA_GDROM	0x00004000
#define IST_DMA_AICA	0x00008000
#define IST_DMA_EXT1	0x00010000
#define IST_DMA_EXT2	0x00020000
#define IST_DMA_DEV	0x00040000
#define IST_DMA_CH2	0x00080000
#define IST_DMA_SORT	0x00100000
#define IST_G1G2EXTSTAT 0x40000000
#define IST_ERROR	0x80000000
/* ------------ external interrupts ------------ */
#define IST_EXT_EXTERNAL	0x00000008
#define IST_EXT_MODEM	0x00000004
#define IST_EXT_AICA	0x00000002
#define IST_EXT_GDROM	0x00000001

#endif
