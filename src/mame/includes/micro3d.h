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
	micro3d_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	struct
	{
		union
		{
			struct
			{
				UINT8 gpdr;
				UINT8 aer;
				UINT8 ddr;
				UINT8 iera;
				UINT8 ierb;
				UINT8 ipra;
				UINT8 iprb;
				UINT8 isra;
				UINT8 isrb;
				UINT8 imra;
				UINT8 imrb;
				UINT8 vr;
				UINT8 tacr;
				UINT8 tbcr;
				UINT8 tcdcr;
				UINT8 tadr;
				UINT8 tbdr;
				UINT8 tcdr;
				UINT8 tddr;
				UINT8 scr;
				UINT8 ucr;
				UINT8 rsr;
				UINT8 tsr;
				UINT8 udr;
			};
			UINT8 regs[24];
		};

		emu_timer *timer_a;
	} mc68901;

	UINT16				*shared_ram;
	running_device		*duart68681;
	UINT8				m68681_tx0;

	/* Sound */
	UINT8				sound_port_latch[4];
	UINT8				dac_data;

	/* TI UART */
	UINT8				ti_uart[9];
	int					ti_uart_mode_cycle;
	int					ti_uart_sync_cycle;

	/* ADC */
	UINT8				adc_val;

	/* Hardware version-check latch for BOTSS 1.1a */
	UINT8				botssa_latch;

	/* MAC */
	UINT32				*mac_sram;
	UINT32				sram_r_addr;
	UINT32				sram_w_addr;
	UINT32				vtx_addr;
	UINT32				mrab11;
	UINT32				mac_stat;
	UINT32				mac_inst;

	/* 2D video */
	UINT16				*micro3d_sprite_vram;
	UINT16				creg;
	UINT16				xfer3dk;

	/* 3D pipeline */
	UINT32				pipe_data;
	UINT32				pipeline_state;
	INT32				vtx_fifo[512];
	UINT32				fifo_idx;
	UINT32				draw_cmd;
	int					draw_state;
	INT32				x_min;
	INT32				x_max;
	INT32				y_min;
	INT32				y_max;
	INT32				z_min;
	INT32				z_max;
	INT32				x_mid;
	INT32				y_mid;
	int					dpram_bank;
	UINT32				draw_dpram[1024];
	UINT16				*frame_buffers[2];
	UINT16				*tmp_buffer;
	int					drawing_buffer;
	int					display_buffer;
};

typedef struct _micro3d_vtx_
{
	INT32 x, y, z;
} micro3d_vtx;


/*----------- defined in machine/micro3d.c -----------*/

READ16_HANDLER( micro3d_mc68901_r );
WRITE16_HANDLER( micro3d_mc68901_w );

READ16_HANDLER( micro3d_ti_uart_r );
WRITE16_HANDLER( micro3d_ti_uart_w );

READ32_HANDLER( micro3d_scc_r );
WRITE32_HANDLER( micro3d_scc_w );

READ16_HANDLER( micro3d_tms_host_r );
WRITE16_HANDLER( micro3d_tms_host_w );

READ16_HANDLER( micro3d_adc_r );
WRITE16_HANDLER( micro3d_adc_w );

WRITE16_HANDLER( host_drmath_int_w );
WRITE16_HANDLER( micro3d_reset_w );

READ16_HANDLER( micro3d_encoder_l_r );
READ16_HANDLER( micro3d_encoder_h_r );

CUSTOM_INPUT( botssa_hwchk_r );
READ16_HANDLER( botssa_140000_r );
READ16_HANDLER( botssa_180000_r );

READ32_HANDLER( micro3d_shared_r );
WRITE32_HANDLER( micro3d_shared_w );

WRITE32_HANDLER( drmath_int_w );
WRITE32_HANDLER( drmath_intr2_ack );

WRITE32_HANDLER( micro3d_mac1_w );
WRITE32_HANDLER( micro3d_mac2_w );
READ32_HANDLER( micro3d_mac2_r );

void micro3d_duart_irq_handler(running_device *device, UINT8 vector);
UINT8 micro3d_duart_input_r(running_device *device);
void micro3d_duart_output_w(running_device *device, UINT8 data);
void micro3d_duart_tx(running_device *device, int channel, UINT8 data);

MACHINE_RESET( micro3d );
DRIVER_INIT( micro3d );
DRIVER_INIT( botssa );


/*----------- defined in audio/micro3d.c -----------*/

WRITE8_DEVICE_HANDLER( micro3d_upd7759_w );
WRITE8_HANDLER( micro3d_snd_dac_a );
WRITE8_HANDLER( micro3d_snd_dac_b );
READ8_HANDLER( micro3d_sound_io_r );
WRITE8_HANDLER( micro3d_sound_io_w );

void micro3d_noise_sh_w(running_machine *machine, UINT8 data);

DECLARE_LEGACY_SOUND_DEVICE(MICRO3D, micro3d_sound);


/*----------- defined in video/micro3d.c -----------*/

VIDEO_START( micro3d );
VIDEO_RESET( micro3d );

void micro3d_tms_interrupt(running_device *device, int state);
void micro3d_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

WRITE16_HANDLER( micro3d_clut_w );
WRITE16_HANDLER( micro3d_creg_w );
WRITE16_HANDLER( micro3d_xfer3dk_w );
READ32_HANDLER( micro3d_pipe_r );
WRITE32_HANDLER( micro3d_fifo_w );
WRITE32_HANDLER( micro3d_alt_fifo_w );

INTERRUPT_GEN( micro3d_vblank );
