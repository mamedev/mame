#pragma once

#ifndef __VOTRAX_H__
#define __VOTRAX_H__

WRITE8_DEVICE_HANDLER( votrax_w );
int votrax_status_r(running_device *device);

DEVICE_GET_INFO( votrax );
#define SOUND_VOTRAX DEVICE_GET_INFO_NAME( votrax )

#endif /* __VOTRAX_H__ */
