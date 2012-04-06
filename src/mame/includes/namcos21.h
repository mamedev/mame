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
	DECLARE_READ16_MEMBER(namcos21_video_enable_r);
	DECLARE_WRITE16_MEMBER(namcos21_video_enable_w);
	DECLARE_WRITE16_MEMBER(dspcuskey_w);
	DECLARE_READ16_MEMBER(dspcuskey_r);
	DECLARE_READ16_MEMBER(dspram16_r);
	DECLARE_WRITE16_MEMBER(dspram16_w);
	DECLARE_READ16_MEMBER(dsp_port0_r);
	DECLARE_WRITE16_MEMBER(dsp_port0_w);
	DECLARE_READ16_MEMBER(dsp_port1_r);
	DECLARE_WRITE16_MEMBER(dsp_port1_w);
	DECLARE_READ16_MEMBER(dsp_port2_r);
	DECLARE_WRITE16_MEMBER(dsp_port2_w);
	DECLARE_READ16_MEMBER(dsp_port3_idc_rcv_enable_r);
	DECLARE_WRITE16_MEMBER(dsp_port3_w);
	DECLARE_WRITE16_MEMBER(dsp_port4_w);
	DECLARE_READ16_MEMBER(dsp_port8_r);
	DECLARE_WRITE16_MEMBER(dsp_port8_w);
	DECLARE_READ16_MEMBER(dsp_port9_r);
	DECLARE_READ16_MEMBER(dsp_porta_r);
	DECLARE_WRITE16_MEMBER(dsp_porta_w);
	DECLARE_READ16_MEMBER(dsp_portb_r);
	DECLARE_WRITE16_MEMBER(dsp_portb_w);
	DECLARE_WRITE16_MEMBER(dsp_portc_w);
	DECLARE_READ16_MEMBER(dsp_portf_r);
	DECLARE_WRITE16_MEMBER(dsp_xf_w);
	DECLARE_READ16_MEMBER(slave_port0_r);
	DECLARE_WRITE16_MEMBER(slave_port0_w);
	DECLARE_READ16_MEMBER(slave_port2_r);
	DECLARE_READ16_MEMBER(slave_port3_r);
	DECLARE_WRITE16_MEMBER(slave_port3_w);
	DECLARE_WRITE16_MEMBER(slave_XF_output_w);
	DECLARE_READ16_MEMBER(slave_portf_r);
	DECLARE_WRITE16_MEMBER(pointram_control_w);
	DECLARE_READ16_MEMBER(pointram_data_r);
	DECLARE_WRITE16_MEMBER(pointram_data_w);
	DECLARE_READ16_MEMBER(namcos21_depthcue_r);
	DECLARE_WRITE16_MEMBER(namcos21_depthcue_w);
	DECLARE_READ16_MEMBER(namcos2_68k_dualportram_word_r);
	DECLARE_WRITE16_MEMBER(namcos2_68k_dualportram_word_w);
	DECLARE_READ8_MEMBER(namcos2_dualportram_byte_r);
	DECLARE_WRITE8_MEMBER(namcos2_dualportram_byte_w);
	DECLARE_READ16_MEMBER(shareram1_r);
	DECLARE_WRITE16_MEMBER(shareram1_w);
	DECLARE_READ16_MEMBER(datarom_r);
	DECLARE_READ16_MEMBER(data2_r);
	DECLARE_READ16_MEMBER(paletteram16_r);
	DECLARE_WRITE16_MEMBER(paletteram16_w);
	DECLARE_WRITE16_MEMBER(NAMCO_C139_SCI_buffer_w);
	DECLARE_READ16_MEMBER(NAMCO_C139_SCI_buffer_r);
	DECLARE_WRITE16_MEMBER(NAMCO_C139_SCI_register_w);
	DECLARE_READ16_MEMBER(NAMCO_C139_SCI_register_r);
	DECLARE_READ16_MEMBER(winrun_dspcomram_r);
	DECLARE_WRITE16_MEMBER(winrun_dspcomram_w);
	DECLARE_READ16_MEMBER(winrun_cuskey_r);
	DECLARE_WRITE16_MEMBER(winrun_cuskey_w);
	DECLARE_READ16_MEMBER(winrun_poly_reset_r);
	DECLARE_WRITE16_MEMBER(winrun_dsp_render_w);
	DECLARE_WRITE16_MEMBER(winrun_dsp_pointrom_addr_w);
	DECLARE_READ16_MEMBER(winrun_dsp_pointrom_data_r);
	DECLARE_WRITE16_MEMBER(winrun_dsp_complete_w);
	DECLARE_READ16_MEMBER(winrun_table_r);
	DECLARE_READ16_MEMBER(gpu_data_r);
	DECLARE_READ16_MEMBER(winrun_gpucomram_r);
	DECLARE_WRITE16_MEMBER(winrun_gpucomram_w);
	DECLARE_WRITE16_MEMBER(winrun_dspbios_w);
	DECLARE_READ16_MEMBER(winrun_68k_dspcomram_r);
	DECLARE_WRITE16_MEMBER(winrun_68k_dspcomram_w);
	DECLARE_READ16_MEMBER(winrun_dspcomram_control_r);
	DECLARE_WRITE16_MEMBER(winrun_dspcomram_control_w);
	DECLARE_READ16_MEMBER(winrun_gpu_color_r);
	DECLARE_WRITE16_MEMBER(winrun_gpu_color_w);
	DECLARE_READ16_MEMBER(winrun_gpu_register_r);
	DECLARE_WRITE16_MEMBER(winrun_gpu_register_w);
	DECLARE_WRITE16_MEMBER(winrun_gpu_videoram_w);
	DECLARE_READ16_MEMBER(winrun_gpu_videoram_r);
};


/*----------- defined in video/namcos21.c -----------*/

extern void namcos21_ClearPolyFrameBuffer( running_machine &machine );
extern void namcos21_DrawQuad( running_machine &machine, int sx[4], int sy[4], int zcode[4], int color );




extern VIDEO_START( namcos21 ) ;
extern SCREEN_UPDATE_IND16( namcos21 );
