/**********************************************************************************************
 *
 *   DMA-driven DAC driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __DMADAC_H__
#define __DMADAC_H__

void dmadac_transfer(running_device **devlist, UINT8 num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data);
void dmadac_enable(running_device **devlist, UINT8 num_channels, UINT8 enable);
void dmadac_set_frequency(running_device **devlist, UINT8 num_channels, double frequency);
void dmadac_set_volume(running_device **devlist, UINT8 num_channels, UINT16 volume);

DEVICE_GET_INFO( dmadac );
#define SOUND_DMADAC DEVICE_GET_INFO_NAME( dmadac )

#endif /* __DMADAC_H__ */
