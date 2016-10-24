// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

     Microprose Games 3D hardware

*************************************************************************/

#include "cpu/tms34010/tms34010.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/upd7759.h"
#include "machine/mc68681.h"


#define HOST_MONITOR_DISPLAY        0
#define VGB_MONITOR_DISPLAY         0
#define DRMATH_MONITOR_DISPLAY      0


struct micro3d_vtx
{
	int32_t x, y, z;
};

enum planes
{
		CLIP_Z_MIN,
		CLIP_Z_MAX,
		CLIP_X_MIN,
		CLIP_X_MAX,
		CLIP_Y_MIN,
		CLIP_Y_MAX
};

enum dac_registers {
	VCF,
	VCQ,
	VCA,
	PAN
};

class micro3d_sound_device;

class micro3d_state : public driver_device
{
public:
	enum
	{
		TIMER_MAC_DONE,
		TIMER_ADC_DONE
	};

	micro3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_upd7759(*this, "upd7759"),
		m_drmath(*this, "drmath"),
		m_vgb(*this, "vgb"),
		m_palette(*this, "palette"),
		m_duart68681(*this, "duart68681"),
		m_noise_1(*this, "noise_1"),
		m_noise_2(*this, "noise_2"),
		m_vertex(*this, "vertex"),
		m_sound_sw(*this, "SOUND_SW"),
		m_volume(*this, "VOLUME"),
		m_joystick_x(*this, "JOYSTICK_X"),
		m_joystick_y(*this, "JOYSTICK_Y"),
		m_throttle(*this, "THROTTLE"),
		m_shared_ram(*this, "shared_ram"),
		m_mac_sram(*this, "mac_sram"),
		m_sprite_vram(*this, "sprite_vram") { }

	required_device<cpu_device> m_maincpu;
	required_device<i8051_device> m_audiocpu;
	required_device<upd7759_device> m_upd7759;
	required_device<cpu_device> m_drmath;
	required_device<tms34010_device> m_vgb;
	required_device<palette_device> m_palette;
	required_device<mc68681_device> m_duart68681;
	required_device<micro3d_sound_device> m_noise_1;
	required_device<micro3d_sound_device> m_noise_2;
	required_memory_region m_vertex;

	required_ioport m_sound_sw;
	required_ioport m_volume;
	optional_ioport m_joystick_x;
	optional_ioport m_joystick_y;
	optional_ioport m_throttle;

	required_shared_ptr<uint16_t> m_shared_ram;
	uint8_t               m_m68681_tx0;

	/* Sound */
	uint8_t               m_sound_port_latch[4];
	uint8_t               m_dac_data;

	/* TI UART */
	uint8_t               m_ti_uart[9];
	int                 m_ti_uart_mode_cycle;
	int                 m_ti_uart_sync_cycle;

	/* ADC */
	uint8_t               m_adc_val;

	/* Hardware version-check latch for BOTSS 1.1a */
	uint8_t               m_botss_latch;

	/* MAC */
	required_shared_ptr<uint32_t> m_mac_sram;
	uint32_t              m_sram_r_addr;
	uint32_t              m_sram_w_addr;
	uint32_t              m_vtx_addr;
	uint32_t              m_mrab11;
	uint32_t              m_mac_stat;
	uint32_t              m_mac_inst;

	/* 2D video */
	required_shared_ptr<uint16_t> m_sprite_vram;
	uint16_t              m_creg;
	uint16_t              m_xfer3dk;

	/* 3D pipeline */
	uint32_t              m_pipe_data;
	uint32_t              m_pipeline_state;
	int32_t               m_vtx_fifo[512];
	uint32_t              m_fifo_idx;
	uint32_t              m_draw_cmd;
	int                 m_draw_state;
	int32_t               m_x_min;
	int32_t               m_x_max;
	int32_t               m_y_min;
	int32_t               m_y_max;
	int32_t               m_z_min;
	int32_t               m_z_max;
	int32_t               m_x_mid;
	int32_t               m_y_mid;
	int                 m_dpram_bank;
	uint32_t              m_draw_dpram[1024];
	std::unique_ptr<uint16_t[]>              m_frame_buffers[2];
	std::unique_ptr<uint16_t[]>              m_tmp_buffer;
	int                 m_drawing_buffer;
	int                 m_display_buffer;

	void micro3d_ti_uart_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t micro3d_ti_uart_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void micro3d_scc_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t micro3d_scc_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void micro3d_mac1_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t micro3d_mac2_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void micro3d_mac2_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t micro3d_encoder_h_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t micro3d_encoder_l_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t micro3d_adc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void micro3d_adc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t botss_140000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t botss_180000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void micro3d_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void host_drmath_int_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void micro3d_shared_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t micro3d_shared_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void drmath_int_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void drmath_intr2_ack(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void micro3d_creg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void micro3d_xfer3dk_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void micro3d_fifo_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void micro3d_alt_fifo_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t micro3d_pipe_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	ioport_value botss_hwchk_r(ioport_field &field, void *param);
	void micro3d_snd_dac_a(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void micro3d_snd_dac_b(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void micro3d_sound_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t micro3d_sound_io_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_micro3d();
	void init_botss();
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void video_reset() override;
	void micro3d_vblank(device_t &device);
	void mac_done_callback(void *ptr, int32_t param);
	void adc_done_callback(void *ptr, int32_t param);
	void micro3d_upd7759_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void data_from_i8031(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t data_to_i8031(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void duart_irq_handler(int state);
	uint8_t duart_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void duart_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void duart_txb(int state);
	void tms_interrupt(int state);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	/* 3D graphics */
	int inside(micro3d_vtx *v, enum planes plane);
	micro3d_vtx intersect(micro3d_vtx *v1, micro3d_vtx *v2, enum planes plane);
	inline void write_span(uint32_t y, uint32_t x);
	void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
	void rasterise_spans(uint32_t min_y, uint32_t max_y, uint32_t attr);
	int clip_triangle(micro3d_vtx *v, micro3d_vtx *vout, int num_vertices, enum planes plane);
	void draw_triangles(uint32_t attr);


protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in audio/micro3d.c -----------*/

struct biquad
{
	double a0, a1, a2;      /* Numerator coefficients */
	double b0, b1, b2;      /* Denominator coefficients */
};

struct lp_filter
{
	std::unique_ptr<float[]> history;
	std::unique_ptr<float[]> coef;
	double fs;
	biquad ProtoCoef[2];
};

struct m3d_filter_state
{
	double      capval;
	double      exponent;
};

class micro3d_sound_device : public device_t,
									public device_sound_interface
{
public:
	micro3d_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~micro3d_sound_device() {}

	void noise_sh_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
private:
	// internal state
//  union
//  {
//      struct
//      {
//          uint8_t m_vcf;
//          uint8_t m_vcq;
//          uint8_t m_vca;
//          uint8_t m_pan;
//      };
		uint8_t m_dac[4];
//  };

	float               m_gain;
	uint32_t              m_noise_shift;
	uint8_t               m_noise_value;
	uint8_t               m_noise_subcount;

	m3d_filter_state    m_noise_filters[4];
	lp_filter           m_filter;
	sound_stream        *m_stream;
};

extern const device_type MICRO3D;
