/***************************************************************************

    Midway DCS Audio Board

****************************************************************************/

MACHINE_DRIVER_EXTERN( dcs_audio_2k );
MACHINE_DRIVER_EXTERN( dcs_audio_2k_uart );
MACHINE_DRIVER_EXTERN( dcs_audio_8k );
MACHINE_DRIVER_EXTERN( dcs2_audio_2115 );
MACHINE_DRIVER_EXTERN( dcs2_audio_2104 );
MACHINE_DRIVER_EXTERN( dcs2_audio_dsio );
MACHINE_DRIVER_EXTERN( dcs2_audio_denver );

void dcs_init(void);
void dcs2_init(int dram_in_mb, offs_t polling_offset);
void dcs_set_auto_ack(int state);

void dcs_set_fifo_callbacks(UINT16 (*fifo_data_r)(void), UINT16 (*fifo_status_r)(void));
void dcs_set_io_callbacks(void (*output_full_cb)(int), void (*input_empty_cb)(int));

int dcs_data_r(void);
void dcs_ack_w(void);
int dcs_data2_r(void);
int dcs_control_r(void);

void dcs_data_w(int data);
void dcs_reset_w(int state);

void dcs_fifo_notify(int count, int max);

WRITE32_HANDLER( dsio_idma_addr_w );
WRITE32_HANDLER( dsio_idma_data_w );
READ32_HANDLER( dsio_idma_data_r );
