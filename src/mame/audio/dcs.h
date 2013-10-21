// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Midway DCS Audio Board

****************************************************************************/

#ifndef __DCS_H__
#define __DCS_H__

MACHINE_CONFIG_EXTERN( dcs_audio_2k );
MACHINE_CONFIG_EXTERN( dcs_audio_2k_uart );
MACHINE_CONFIG_EXTERN( dcs_audio_8k );
MACHINE_CONFIG_EXTERN( dcs_audio_wpc );
MACHINE_CONFIG_EXTERN( dcs2_audio_2115 );
MACHINE_CONFIG_EXTERN( dcs2_audio_2104 );
MACHINE_CONFIG_EXTERN( dcs2_audio_dsio );
MACHINE_CONFIG_EXTERN( dcs2_audio_denver );

void dcs_init(running_machine &machine);
void dcs2_init(running_machine &machine, int dram_in_mb, offs_t polling_offset);
void dcs_set_auto_ack(running_machine &machine, int state);

void dcs_set_fifo_callbacks(UINT16 (*fifo_data_r)(device_t *device), UINT16 (*fifo_status_r)(device_t *device));
void dcs_set_io_callbacks(void (*output_full_cb)(running_machine &, int), void (*input_empty_cb)(running_machine &, int));

int dcs_data_r(running_machine &machine);
void dcs_ack_w(running_machine &machine);
int dcs_data2_r(running_machine &machine);
int dcs_control_r(running_machine &machine);

void dcs_data_w(running_machine &machine, int data);
void dcs_reset_w(running_machine &machine, int state);

void dcs_fifo_notify(running_machine &machine, int count, int max);

DECLARE_WRITE32_HANDLER( dsio_idma_addr_w );
DECLARE_WRITE32_HANDLER( dsio_idma_data_w );
DECLARE_READ32_HANDLER( dsio_idma_data_r );

#endif
