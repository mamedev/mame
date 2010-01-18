/*****************************************************************************

  MB87078 6-bit,4-channel electronic volume controller emulator


*****************************************************************************/

#ifndef __MB87078_H__
#define __MB87078_H__



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*mb87078_gain_changed_cb)(running_machine *machine, int channel, int percent /*, float decibels*/);

typedef struct _mb87078_interface mb87078_interface;
struct _mb87078_interface
{
	mb87078_gain_changed_cb   gain_changed_cb;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( mb87078 );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MB87078 DEVICE_GET_INFO_NAME( mb87078 )

#define MDRV_MB87078_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, MB87078, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

void mb87078_data_w(running_device *device, int data, int dsel);
void mb87078_reset_comp_w(running_device *device, int level);


/* mb87078_gain_decibel_r will return 'channel' gain on the device.
   Returned value represents channel gain expressed in decibels,
   Range from 0 to -32.0 (or -256.0 for -infinity) */
float mb87078_gain_decibel_r(running_device *device, int channel);


/* mb87078_gain_percent_r will return 'channel' gain on the device.
   Returned value represents channel gain expressed in percents of maximum volume.
   Range from 100 to 0. (100 = 0dB; 50 = -6dB; 0 = -infinity)
   This function is designed for use with MAME mixer_xxx() functions. */
int   mb87078_gain_percent_r(running_device *device, int channel);


#endif	/* __MB87078_H__ */
