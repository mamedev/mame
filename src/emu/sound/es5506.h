/**********************************************************************************************
 *
 *   Ensoniq ES5505/6 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __ES5506_H__
#define __ES5506_H__

typedef struct _es5505_interface es5505_interface;
struct _es5505_interface
{
	const char * region0;						/* memory region where the sample ROM lives */
	const char * region1;						/* memory region where the sample ROM lives */
	void (*irq_callback)(running_device *device, int state);	/* irq callback */
	UINT16 (*read_port)(running_device *device);			/* input port read */
};

READ16_DEVICE_HANDLER( es5505_r );
WRITE16_DEVICE_HANDLER( es5505_w );
void es5505_voice_bank_w(running_device *device, int voice, int bank);

DEVICE_GET_INFO( es5505 );
#define SOUND_ES5505 DEVICE_GET_INFO_NAME( es5505 )


typedef struct _es5506_interface es5506_interface;
struct _es5506_interface
{
	const char * region0;						/* memory region where the sample ROM lives */
	const char * region1;						/* memory region where the sample ROM lives */
	const char * region2;						/* memory region where the sample ROM lives */
	const char * region3;						/* memory region where the sample ROM lives */
	void (*irq_callback)(running_device *device, int state);	/* irq callback */
	UINT16 (*read_port)(running_device *device);			/* input port read */
};

READ8_DEVICE_HANDLER( es5506_r );
WRITE8_DEVICE_HANDLER( es5506_w );
void es5506_voice_bank_w(running_device *device, int voice, int bank);

DEVICE_GET_INFO( es5506 );
#define SOUND_ES5506 DEVICE_GET_INFO_NAME( es5506 )

#endif /* __ES5506_H__ */
