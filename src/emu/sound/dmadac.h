/**********************************************************************************************
 *
 *   DMA-driven DAC driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __DMADAC_H__
#define __DMADAC_H__

#include "devlegcy.h"

DECLARE_LEGACY_SOUND_DEVICE(DMADAC, dmadac);

void dmadac_transfer(dmadac_sound_device **devlist, UINT8 num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, INT16 *data);
void dmadac_enable(dmadac_sound_device **devlist, UINT8 num_channels, UINT8 enable);
void dmadac_set_frequency(dmadac_sound_device **devlist, UINT8 num_channels, double frequency);
void dmadac_set_volume(dmadac_sound_device **devlist, UINT8 num_channels, UINT16 volume);

#endif /* __DMADAC_H__ */
