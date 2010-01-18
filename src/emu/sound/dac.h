#pragma once

#ifndef __DAC_H__
#define __DAC_H__

void dac_data_w(running_device *device, UINT8 data) ATTR_NONNULL(1);
void dac_signed_data_w(running_device *device, UINT8 data) ATTR_NONNULL(1);
void dac_data_16_w(running_device *device, UINT16 data) ATTR_NONNULL(1);
void dac_signed_data_16_w(running_device *device, UINT16 data) ATTR_NONNULL(1);

WRITE8_DEVICE_HANDLER( dac_w );
WRITE8_DEVICE_HANDLER( dac_signed_w );

DEVICE_GET_INFO( dac );
#define SOUND_DAC DEVICE_GET_INFO_NAME( dac )

#endif /* __DAC_H__ */
