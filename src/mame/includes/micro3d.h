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
		m_shared_ram(*this, "shared_ram"),
		m_mac_sram(*this, "mac_sram"),
		m_micro3d_sprite_vram(*this, "sprite_vram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_upd7759(*this, "upd7759"),
		m_drmath(*this, "drmath"),
		m_vgb(*this, "vgb"),
		m_palette(*this, "palette"),
		m_duart68681(*this, "duart68681") { }

	required_shared_ptr<UINT16> m_shared_ram;
	UINT8               m_m68681_tx0;

	/* Sound */
	UINT8               m_sound_port_latch[4];
	UINT8               m_dac_data;

	/* TI UART */
	UINT8               m_ti_uart[9];
	int                 m_ti_uart_mode_cycle;
	int                 m_ti_uart_sync_cycle;

	/* ADC */
	UINT8               m_adc_val;

	/* Hardware version-check latch for BOTSS 1.1a */
	UINT8               m_botss_latch;

	/* MAC */
	required_shared_ptr<UINT32> m_mac_sram;
	UINT32              m_sram_r_addr;
	UINT32              m_sram_w_addr;
	UINT32              m_vtx_addr;
	UINT32              m_mrab11;
	UINT32              m_mac_stat;
	UINT32              m_mac_inst;

	/* 2D video */
	required_shared_ptr<UINT16> m_micro3d_sprite_vram;
	UINT16              m_creg;
	UINT16              m_xfer3dk;

	/* 3D pipeline */
	UINT32              m_pipe_data;
	UINT32              m_pipeline_state;
	INT32               m_vtx_fifo[512];
	UINT32              m_fifo_idx;
	UINT32              m_draw_cmd;
	int                 m_draw_state;
	INT32               m_x_min;
	INT32               m_x_max;
	INT32               m_y_min;
	INT32               m_y_max;
	INT32               m_z_min;
	INT32               m_z_max;
	INT32               m_x_mid;
	INT32               m_y_mid;
	int                 m_dpram_bank;
	UINT32              m_draw_dpram[1024];
	UINT16              *m_frame_buffers[2];
	UINT16              *m_tmp_buffer;
	int                 m_drawing_buffer;
	int                 m_display_buffer;

	DECLARE_WRITE16_MEMBER(micro3d_ti_uart_w);
	DECLARE_READ16_MEMBER(micro3d_ti_uart_r);
	DECLARE_WRITE32_MEMBER(micro3d_scc_w);
	DECLARE_READ32_MEMBER(micro3d_scc_r);
	DECLARE_WRITE32_MEMBER(micro3d_mac1_w);
	DECLARE_READ32_MEMBER(micro3d_mac2_r);
	DECLARE_WRITE32_MEMBER(micro3d_mac2_w);
	DECLARE_READ16_MEMBER(micro3d_encoder_h_r);
	DECLARE_READ16_MEMBER(micro3d_encoder_l_r);
	DECLARE_READ16_MEMBER(micro3d_adc_r);
	DECLARE_WRITE16_MEMBER(micro3d_adc_w);
	DECLARE_READ16_MEMBER(botss_140000_r);
	DECLARE_READ16_MEMBER(botss_180000_r);
	DECLARE_WRITE16_MEMBER(micro3d_reset_w);
	DECLARE_WRITE16_MEMBER(host_drmath_int_w);
	DECLARE_WRITE32_MEMBER(micro3d_shared_w);
	DECLARE_READ32_MEMBER(micro3d_shared_r);
	DECLARE_WRITE32_MEMBER(drmath_int_w);
	DECLARE_WRITE32_MEMBER(drmath_intr2_ack);
	DECLARE_WRITE16_MEMBER(micro3d_clut_w);
	DECLARE_WRITE16_MEMBER(micro3d_creg_w);
	DECLARE_WRITE16_MEMBER(micro3d_xfer3dk_w);
	DECLARE_WRITE32_MEMBER(micro3d_fifo_w);
	DECLARE_WRITE32_MEMBER(micro3d_alt_fifo_w);
	DECLARE_READ32_MEMBER(micro3d_pipe_r);
	DECLARE_CUSTOM_INPUT_MEMBER(botss_hwchk_r);
	DECLARE_WRITE8_MEMBER(micro3d_snd_dac_a);
	DECLARE_WRITE8_MEMBER(micro3d_snd_dac_b);
	DECLARE_WRITE8_MEMBER(micro3d_sound_io_w);
	DECLARE_READ8_MEMBER(micro3d_sound_io_r);
	DECLARE_DRIVER_INIT(micro3d);
	DECLARE_DRIVER_INIT(botss);
	virtual void machine_reset();
	virtual void video_start();
	virtual void video_reset();
	INTERRUPT_GEN_MEMBER(micro3d_vblank);
	TIMER_CALLBACK_MEMBER(mac_done_callback);
	TIMER_CALLBACK_MEMBER(adc_done_callback);
	DECLARE_WRITE8_MEMBER(micro3d_upd7759_w);
	DECLARE_WRITE8_MEMBER(data_from_i8031);
	DECLARE_READ8_MEMBER(data_to_i8031);
	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_READ8_MEMBER(duart_input_r);
	DECLARE_WRITE8_MEMBER(duart_output_w);
	DECLARE_WRITE_LINE_MEMBER(duart_txb);

	required_device<cpu_device> m_maincpu;
	required_device<i8051_device> m_audiocpu;
	required_device<upd7759_device> m_upd7759;
	required_device<cpu_device> m_drmath;
	required_device<tms34010_device> m_vgb;
	required_device<palette_device> m_palette;
	required_device<mc68681_device> m_duart68681;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};

struct micro3d_vtx
{
	INT32 x, y, z;
};

/*----------- defined in audio/micro3d.c -----------*/

struct biquad
{
	double a0, a1, a2;      /* Numerator coefficients */
	double b0, b1, b2;      /* Denominator coefficients */
};

struct lp_filter
{
	float *history;
	float *coef;
	double fs;
	biquad ProtoCoef[2];
};

struct filter_state
{
	double      capval;
	double      exponent;
};

class micro3d_sound_device : public device_t,
									public device_sound_interface
{
public:
	micro3d_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~micro3d_sound_device() {}

	void noise_sh_w(UINT8 data);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	union
	{
		struct
		{
			UINT8 m_vcf;
			UINT8 m_vcq;
			UINT8 m_vca;
			UINT8 m_pan;
		};
		UINT8 m_dac[4];
	};

	float               m_gain;
	UINT32              m_noise_shift;
	UINT8               m_noise_value;
	UINT8               m_noise_subcount;

	filter_state        m_noise_filters[4];
	lp_filter           m_filter;
	sound_stream        *m_stream;
};

extern const device_type MICRO3D;

/*----------- defined in video/micro3d.c -----------*/
void micro3d_tms_interrupt(device_t *device, int state);
void micro3d_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params);
