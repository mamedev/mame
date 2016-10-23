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
	uint16_t slaveInputBuffer[DSP_BUF_MAX];
	unsigned slaveBytesAvailable;
	unsigned slaveBytesAdvertised;
	unsigned slaveInputStart;
	uint16_t slaveOutputBuffer[DSP_BUF_MAX];
	unsigned slaveOutputSize;
	uint16_t masterDirectDrawBuffer[256];
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
	namcos21_state(const machine_config &mconfig, device_type type, const char *tag)
		: namcos2_shared_state(mconfig, type, tag),
		m_winrun_dspbios(*this,"winrun_dspbios"),
		m_winrun_polydata(*this,"winrun_polydata"),
		m_dspram16(*this,"dspram16"),
		m_mpDualPortRAM(*this,"mpdualportram"),
		m_master_dsp_code(*this,"master_dsp_code"),
		m_ptrom24(*this,"point24"),
		m_ptrom16(*this,"point16"),
		m_dsp(*this, "dsp") { }

	optional_shared_ptr<uint16_t> m_winrun_dspbios;
	optional_shared_ptr<uint16_t> m_winrun_polydata;
	optional_shared_ptr<uint16_t> m_dspram16;
	required_shared_ptr<uint8_t> m_mpDualPortRAM;
	optional_shared_ptr<uint16_t> m_master_dsp_code;

	optional_region_ptr<int32_t> m_ptrom24;
	optional_region_ptr<uint16_t> m_ptrom16;

	optional_device<cpu_device> m_dsp;

	std::unique_ptr<uint8_t[]> m_videoram;
	std::unique_ptr<uint16_t[]> m_winrun_dspcomram;
	uint16_t m_winrun_poly_buf[WINRUN_MAX_POLY_PARAM];
	int m_winrun_poly_index;
	uint32_t m_winrun_pointrom_addr;
	int m_winrun_dsp_alive;
	uint16_t m_winrun_dspcomram_control[8];
	uint16_t m_video_enable;
	std::unique_ptr<uint8_t[]> m_pointram;
	int m_pointram_idx;
	uint16_t m_pointram_control;
	std::unique_ptr<dsp_state> m_mpDspState;
	int m_mbNeedsKickstart;
	uint32_t m_pointrom_idx;
	uint8_t m_mPointRomMSB;
	int m_mbPointRomDataAvailable;
	int m_irq_enable;
	uint8_t m_depthcue[2][0x400];
	std::unique_ptr<uint16_t[]> m_mpPolyFrameBufferPens;
	std::unique_ptr<uint16_t[]> m_mpPolyFrameBufferZ;
	std::unique_ptr<uint16_t[]> m_mpPolyFrameBufferPens2;
	std::unique_ptr<uint16_t[]> m_mpPolyFrameBufferZ2;
	uint16_t m_winrun_color;
	uint16_t m_winrun_gpu_register[0x10/2];
	uint16_t namcos21_video_enable_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void namcos21_video_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dspcuskey_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dspcuskey_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dspram16_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dspram16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_port0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_port0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_port1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_port1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_port2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_port2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_port3_idc_rcv_enable_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_port3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_port4_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_port8_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_port8_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_port9_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp_porta_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_porta_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_portb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_portb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_portc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_portf_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_xf_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t slave_port0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void slave_port0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t slave_port2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t slave_port3_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void slave_port3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void slave_XF_output_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t slave_portf_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pointram_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pointram_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pointram_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t namcos21_depthcue_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void namcos21_depthcue_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t namcos2_68k_dualportram_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void namcos2_68k_dualportram_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t namcos2_dualportram_byte_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void namcos2_dualportram_byte_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void NAMCO_C139_SCI_buffer_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t NAMCO_C139_SCI_buffer_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void NAMCO_C139_SCI_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t NAMCO_C139_SCI_register_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t winrun_dspcomram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_dspcomram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_cuskey_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_cuskey_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_poly_reset_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_dsp_render_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void winrun_dsp_pointrom_addr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_dsp_pointrom_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_dsp_complete_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_table_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_dspbios_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_68k_dspcomram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_68k_dspcomram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_dspcomram_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_dspcomram_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_gpu_color_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_gpu_color_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_gpu_register_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void winrun_gpu_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void winrun_gpu_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t winrun_gpu_videoram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void init_driveyes();
	void init_winrun();
	void init_starblad();
	void init_solvalou();
	void init_cybsled();
	void init_aircomb();
	void machine_start_namcos21();
	void video_start_namcos21();
	uint32_t screen_update_namcos21(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void allocate_poly_framebuffer();
	void clear_poly_framebuffer();
	void copy_visible_poly_framebuffer(bitmap_ind16 &bitmap, const rectangle &clip, int zlo, int zhi);
	void renderscanline_flat(const edge *e1, const edge *e2, int sy, unsigned color, int depthcueenable);
	void rendertri(const n21_vertex *v0, const n21_vertex *v1, const n21_vertex *v2, unsigned color, int depthcueenable);
	void draw_quad(int sx[4], int sy[4], int zcode[4], int color);
	int32_t read_pointrom_data(unsigned offset);
	void transmit_word_to_slave(uint16_t data);
	void transfer_dsp_data();
	uint16_t read_word_from_slave_input();
	uint16_t get_input_bytes_advertised_for_slave();
	int init_dsp();
	void render_slave_output(uint16_t data);
	void winrun_flush_poly();
	void init(int game_type);
};
