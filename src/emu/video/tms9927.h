/**********************************************************************

    TI TMS9927 and compatible CRT controller emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#ifndef __TMS9927__
#define __TMS9927__


#define TMS9927		DEVICE_GET_INFO_NAME(tms9927)
#define CRT5027		DEVICE_GET_INFO_NAME(crt5027)
#define CRT5037		DEVICE_GET_INFO_NAME(crt5037)
#define CRT5057		DEVICE_GET_INFO_NAME(crt5057)


#define MDRV_TMS9927_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, TMS9927, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_TMS9927_RECONFIG(_tag, _clock, _config) \
	MDRV_DEVICE_MODIFY(_tag) \
	MDRV_DEVICE_CLOCK(_clock) \
	MDRV_DEVICE_CONFIG(_config)



/* interface */
typedef struct _tms9927_interface tms9927_interface;
struct _tms9927_interface
{
	const char *screen_tag;			/* screen we are acting on */
	int hpixels_per_column;			/* number of pixels per video memory address */
	const char *selfload_region;	/* name of the region with self-load data */
};

extern tms9927_interface tms9927_null_interface;


/* device interface */
DEVICE_GET_INFO( tms9927 );
DEVICE_GET_INFO( crt5027 );
DEVICE_GET_INFO( crt5037 );
DEVICE_GET_INFO( crt5057 );

/* basic read/write handlers */
WRITE8_DEVICE_HANDLER( tms9927_w );
READ8_DEVICE_HANDLER( tms9927_r );

/* other queries */
int tms9927_screen_reset(const device_config *device);
int tms9927_upscroll_offset(const device_config *device);
int tms9927_cursor_bounds(const device_config *device, rectangle *bounds);



#endif
