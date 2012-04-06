/*************************************************************************

     Microprose Games 3D hardware

*************************************************************************/

#include "devlegcy.h"
#include "cpu/tms34010/tms34010.h"


#define HOST_MONITOR_DISPLAY		0
#define VGB_MONITOR_DISPLAY			0
#define DRMATH_MONITOR_DISPLAY		0

class micro3d_state : public driver_device
{
public:
	micro3d_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16				*m_shared_ram;
	device_t			*m_duart68681;
	UINT8				m_m68681_tx0;

	/* Sound */
	UINT8				m_sound_port_latch[4];
	UINT8				m_dac_data;

	/* TI UART */
	UINT8				m_ti_uart[9];
	int					m_ti_uart_mode_cycle;
	int					m_ti_uart_sync_cycle;

	/* ADC */
	UINT8				m_adc_val;

	/* Hardware version-check latch for BOTSS 1.1a */
	UINT8				m_botssa_latch;

	/* MAC */
	UINT32				*m_mac_sram;
	UINT32				m_sram_r_addr;
	UINT32				m_sram_w_addr;
	UINT32				m_vtx_addr;
	UINT32				m_mrab11;
	UINT32				m_mac_stat;
	UINT32				m_mac_inst;

	/* 2D video */
	UINT16				*m_micro3d_sprite_vram;
	UINT16				m_creg;
	UINT16				m_xfer3dk;

	/* 3D pipeline */
	UINT32				m_pipe_data;
	UINT32				m_pipeline_state;
	INT32				m_vtx_fifo[512];
	UINT32				m_fifo_idx;
	UINT32				m_draw_cmd;
	int					m_draw_state;
	INT32				m_x_min;
	INT32				m_x_max;
	INT32				m_y_min;
	INT32				m_y_max;
	INT32				m_z_min;
	INT32				m_z_max;
	INT32				m_x_mid;
	INT32				m_y_mid;
	int					m_dpram_bank;
	UINT32				m_draw_dpram[1024];
	UINT16				*m_frame_buffers[2];
	UINT16				*m_tmp_buffer;
	int					m_drawing_buffer;
	int					m_display_buffer;
	DECLARE_WRITE16_MEMBER(micro3d_ti_uart_w);
	DECLARE_READ16_MEMBER(micro3d_ti_uart_r);
	DECLARE_WRITE32_MEMBER(micro3d_scc_w);
	DECLARE_READ32_MEMBER(micro3d_scc_r);
	DECLARE_READ16_MEMBER(micro3d_tms_host_r);
	DECLARE_WRITE16_MEMBER(micro3d_tms_host_w);
	DECLARE_WRITE32_MEMBER(micro3d_mac1_w);
	DECLARE_READ32_MEMBER(micro3d_mac2_r);
	DECLARE_WRITE32_MEMBER(micro3d_mac2_w);
	DECLARE_READ16_MEMBER(micro3d_encoder_h_r);
	DECLARE_READ16_MEMBER(micro3d_encoder_l_r);
	DECLARE_READ16_MEMBER(micro3d_adc_r);
	DECLARE_WRITE16_MEMBER(micro3d_adc_w);
	DECLARE_READ16_MEMBER(botssa_140000_r);
	DECLARE_READ16_MEMBER(botssa_180000_r);
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
};

typedef struct _micro3d_vtx_
{
	INT32 x, y, z;
} micro3d_vtx;


/*----------- defined in machine/micro3d.c -----------*/







CUSTOM_INPUT( botssa_hwchk_r );




void micro3d_duart_irq_handler(device_t *device, UINT8 vector);
UINT8 micro3d_duart_input_r(device_t *device);
void micro3d_duart_output_w(device_t *device, UINT8 data);
void micro3d_duart_tx(device_t *device, int channel, UINT8 data);

MACHINE_RESET( micro3d );
DRIVER_INIT( micro3d );
DRIVER_INIT( botssa );


/*----------- defined in audio/micro3d.c -----------*/

WRITE8_DEVICE_HANDLER( micro3d_upd7759_w );
WRITE8_HANDLER( micro3d_snd_dac_a );
WRITE8_HANDLER( micro3d_snd_dac_b );
READ8_HANDLER( micro3d_sound_io_r );
WRITE8_HANDLER( micro3d_sound_io_w );

void micro3d_noise_sh_w(running_machine &machine, UINT8 data);

DECLARE_LEGACY_SOUND_DEVICE(MICRO3D, micro3d_sound);


/*----------- defined in video/micro3d.c -----------*/

VIDEO_START( micro3d );
VIDEO_RESET( micro3d );

void micro3d_tms_interrupt(device_t *device, int state);
void micro3d_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params);


INTERRUPT_GEN( micro3d_vblank );
