#ifndef DS2404_H
#define DS2404_H

#include "devlegcy.h"

typedef struct _ds2404_config ds2404_config;
struct _ds2404_config
{
	UINT32	ref_year;
	UINT8	ref_month;
	UINT8	ref_day;
};


#define MDRV_DS2404_ADD(_tag, _ref_year, _ref_month, _ref_day) \
	MDRV_DEVICE_ADD(_tag, DS2404, 0) \
	MDRV_DEVICE_CONFIG_DATA32(ds2404_config, ref_year, _ref_year) \
	MDRV_DEVICE_CONFIG_DATA32(ds2404_config, ref_month, _ref_month) \
	MDRV_DEVICE_CONFIG_DATA32(ds2404_config, ref_day, _ref_day)


/* 1-wire interface reset */
WRITE8_DEVICE_HANDLER( ds2404_1w_reset_w );

/* 3-wire interface reset  */
WRITE8_DEVICE_HANDLER( ds2404_3w_reset_w );

READ8_DEVICE_HANDLER( ds2404_data_r );
WRITE8_DEVICE_HANDLER( ds2404_data_w );
WRITE8_DEVICE_HANDLER( ds2404_clk_w );

/* device get info callback */
DECLARE_LEGACY_NVRAM_DEVICE(DS2404, ds2404);

#endif
