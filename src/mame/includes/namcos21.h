/**
 * @file namcos21.h
 */

#define NAMCOS21_POLY_FRAME_WIDTH 496
#define NAMCOS21_POLY_FRAME_HEIGHT 480

#define WINRUN_MAX_POLY_PARAM (1+256*3)

#define DSP_BUF_MAX (4096*12)
struct dsp_state
{
	unsigned masterSourceAddr;
	UINT16 slaveInputBuffer[DSP_BUF_MAX];
	unsigned slaveBytesAvailable;
	unsigned slaveBytesAdvertised;
	unsigned slaveInputStart;
	UINT16 slaveOutputBuffer[DSP_BUF_MAX];
	unsigned slaveOutputSize;
	UINT16 masterDirectDrawBuffer[256];
	unsigned masterDirectDrawSize;
	int masterFinished;
	int slaveActive;
};

class namcos21_state : public driver_device
{
public:
	namcos21_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT16 *m_winrun_dspbios;
	UINT16 *m_winrun_dspcomram;
	UINT16 m_winrun_poly_buf[WINRUN_MAX_POLY_PARAM];
	int m_winrun_poly_index;
	UINT32 m_winrun_pointrom_addr;
	UINT16 *m_winrun_polydata;
	int m_winrun_dsp_alive;
	UINT16 *m_winrun_gpucomram;
	UINT16 m_winrun_dspcomram_control[8];
	UINT16 m_video_enable;
	UINT8 *m_pointram;
	int m_pointram_idx;
	UINT16 *m_dspram16;
	UINT16 *m_mpDataROM;
	UINT16 *m_mpSharedRAM1;
	UINT8 *m_mpDualPortRAM;
	UINT16 m_pointram_control;
	dsp_state *m_mpDspState;
	int m_mbNeedsKickstart;
	UINT16 *m_master_dsp_code;
	UINT32 m_pointrom_idx;
	UINT8 m_mPointRomMSB;
	int m_mbPointRomDataAvailable;
	int m_irq_enable;
	UINT8 m_depthcue[2][0x400];
	UINT16 *m_mpPolyFrameBufferPens;
	UINT16 *m_mpPolyFrameBufferZ;
	UINT16 *m_mpPolyFrameBufferPens2;
	UINT16 *m_mpPolyFrameBufferZ2;
	UINT16 m_winrun_color;
	UINT16 m_winrun_gpu_register[0x10/2];
};


/*----------- defined in video/namcos21.c -----------*/

extern void namcos21_ClearPolyFrameBuffer( running_machine &machine );
extern void namcos21_DrawQuad( running_machine &machine, int sx[4], int sy[4], int zcode[4], int color );

extern READ16_HANDLER(winrun_gpu_color_r);
extern WRITE16_HANDLER(winrun_gpu_color_w);

extern READ16_HANDLER(winrun_gpu_videoram_r);
extern WRITE16_HANDLER(winrun_gpu_videoram_w);

extern READ16_HANDLER(winrun_gpu_register_r);
extern WRITE16_HANDLER(winrun_gpu_register_w);

extern VIDEO_START( namcos21 ) ;
extern SCREEN_UPDATE( namcos21 );
