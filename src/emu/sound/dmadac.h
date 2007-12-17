/**********************************************************************************************
 *
 *   DMA-driven DAC driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#ifndef DMADAC_H
#define DMADAC_H

void dmadac_transfer(UINT8 first_channel, UINT8 num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data);
void dmadac_enable(UINT8 first_channel, UINT8 num_channels, UINT8 enable);
void dmadac_set_frequency(UINT8 first_channel, UINT8 num_channels, double frequency);
void dmadac_set_volume(UINT8 first_channel, UINT8 num_channels, UINT16 volume);

#endif
