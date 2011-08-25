/*
    rtc65271.h: include file for rtc65271.c
*/

#ifndef __RTC65271_H__
#define __RTC65271_H__

#include "devlegcy.h"

typedef struct _rtc65271_config rtc65271_config;
struct _rtc65271_config
{
	void (*interrupt_callback)(device_t *device, int state);
};


#define MCFG_RTC65271_ADD(_tag, _callback) \
	MCFG_DEVICE_ADD(_tag, RTC65271, 0) \
	MCFG_DEVICE_CONFIG_DATAPTR(rtc65271_config, interrupt_callback, _callback)


UINT8 rtc65271_r(device_t *device, int xramsel, offs_t offset);
void rtc65271_w(device_t *device, int xramsel, offs_t offset, UINT8 data);

READ8_DEVICE_HANDLER( rtc65271_rtc_r );
READ8_DEVICE_HANDLER( rtc65271_xram_r );
WRITE8_DEVICE_HANDLER( rtc65271_rtc_w );
WRITE8_DEVICE_HANDLER( rtc65271_xram_w );


DECLARE_LEGACY_NVRAM_DEVICE(RTC65271, rtc65271);

#endif
