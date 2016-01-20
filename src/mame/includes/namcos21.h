// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/**
 * @file namcos21.h
 */

#include "namcos2.h"

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

struct n21_vertex
{
	double x,y;
	double z;
};

struct edge
{
	double x;
	double z;
};


class namcos21_state : public namcos2_shared_state
{
public:
	namcos21_state(const machine_config &mconfig, device_type type, std::string tag)
		: namcos2_shared_state(mconfig, type, tag),
		m_winrun_dspbios(*this,"winrun_dspbios"),
		m_winrun_polydata(*this,"winrun_polydata"),
		m_winrun_gpucomram(*this,"winrun_comram"),
		m_dspram16(*this,"dspram16"),
		m_mpDualPortRAM(*this,"mpdualportram"),
		m_master_dsp_code(*this,"master_dsp_code"),
		m_ptrom24(*this,"point24"),
		m_ptrom16(*this,"point16"),
		m_dsp(*this, "dsp") { }

	optional_shared_ptr<UINT16> m_winrun_dspbios;
	optional_shared_ptr<UINT16> m_winrun_polydata;
	optional_shared_ptr<UINT16> m_winrun_gpucomram;
	optional_shared_ptr<UINT16> m_dspram16;
	required_shared_ptr<UINT8> m_mpDualPortRAM;
	optional_shared_ptr<UINT16> m_master_dsp_code;

	optional_region_ptr<INT32> m_ptrom24;
	optional_region_ptr<UINT16> m_ptrom16;

	optional_device<cpu_device> m_dsp;

	std::unique_ptr<UINT8[]> m_videoram;
	std::unique_ptr<UINT16[]> m_winrun_dspcomram;
	UINT16 m_winrun_poly_buf[WINRUN_MAX_POLY_PARAM];
	int m_winrun_poly_index;
	UINT32 m_winrun_pointrom_addr;
	int m_winrun_dsp_alive;
	UINT16 m_winrun_dspcomram_control[8];
	UINT16 m_video_enable;
	std::unique_ptr<UINT8[]> m_pointram;
	int m_pointram_idx;
	UINT16 m_pointram_control;
	std::unique_ptr<dsp_state> m_mpDspState;
	int m_mbNeedsKickstart;
	UINT32 m_pointrom_idx;
	UINT8 m_mPointRomMSB;
	int m_mbPointRomDataAvailable;
	int m_irq_enable;
	UINT8 m_depthcue[2][0x400];
	std::unique_ptr<UINT16[]> m_mpPolyFrameBufferPens;
	std::unique_ptr<UINT16[]> m_mpPolyFrameBufferZ;
	std::unique_ptr<UINT16[]> m_mpPolyFrameBufferPens2;
	std::unique_ptr<UINT16[]> m_mpPolyFrameBufferZ2;
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

	DECLARE_DRIVER_INIT(driveyes);
	DECLARE_DRIVER_INIT(winrun);
	DECLARE_DRIVER_INIT(starblad);
	DECLARE_DRIVER_INIT(solvalou);
	DECLARE_DRIVER_INIT(cybsled);
	DECLARE_DRIVER_INIT(aircomb);
	DECLARE_MACHINE_START(namcos21);
	DECLARE_VIDEO_START(namcos21);
	UINT32 screen_update_namcos21(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void allocate_poly_framebuffer();
	void clear_poly_framebuffer();
	void copy_visible_poly_framebuffer(bitmap_ind16 &bitmap, const rectangle &clip, int zlo, int zhi);
	void renderscanline_flat(const edge *e1, const edge *e2, int sy, unsigned color, int depthcueenable);
	void rendertri(const n21_vertex *v0, const n21_vertex *v1, const n21_vertex *v2, unsigned color, int depthcueenable);
	void draw_quad(int sx[4], int sy[4], int zcode[4], int color);
	INT32 read_pointrom_data(unsigned offset);
	void transmit_word_to_slave(UINT16 data);
	void transfer_dsp_data();
	UINT16 read_word_from_slave_input();
	UINT16 get_input_bytes_advertised_for_slave();
	int init_dsp();
	void render_slave_output(UINT16 data);
	void winrun_flush_poly();
	void init(int game_type);
};
