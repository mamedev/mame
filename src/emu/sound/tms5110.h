#pragma once

#ifndef __TMS5110_H__
#define __TMS5110_H__

/* TMS5110 commands */
                                     /* CTL8  CTL4  CTL2  CTL1  |   PDC's  */
                                     /* (MSB)             (LSB) | required */
#define TMS5110_CMD_RESET        (0) /*    0     0     0     x  |     1    */
#define TMS5110_CMD_LOAD_ADDRESS (2) /*    0     0     1     x  |     2    */
#define TMS5110_CMD_OUTPUT       (4) /*    0     1     0     x  |     3    */
#define TMS5110_CMD_READ_BIT     (8) /*    1     0     0     x  |     1    */
#define TMS5110_CMD_SPEAK       (10) /*    1     0     1     x  |     1    */
#define TMS5110_CMD_READ_BRANCH (12) /*    1     1     0     x  |     1    */
#define TMS5110_CMD_TEST_TALK   (14) /*    1     1     1     x  |     3    */

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

typedef struct _tms5110_interface tms5110_interface;
struct _tms5110_interface
{
	int (*M0_callback)(const device_config *device);	/* function to be called when chip requests another bit */
	void (*load_address)(const device_config *device, int addr);	/* speech ROM load address callback */
};

WRITE8_DEVICE_HANDLER( tms5110_ctl_w );
WRITE8_DEVICE_HANDLER( tms5110_pdc_w );

READ8_DEVICE_HANDLER( tms5110_status_r );
int tms5110_ready_r(const device_config *device);

void tms5110_set_frequency(const device_config *device, int frequency);

DEVICE_GET_INFO( tms5110 );
DEVICE_GET_INFO( tms5100 );
DEVICE_GET_INFO( tms5110a );
DEVICE_GET_INFO( cd2801 );
DEVICE_GET_INFO( tmc0281 );
DEVICE_GET_INFO( cd2802 );
DEVICE_GET_INFO( m58817 );

#define SOUND_TMS5110 DEVICE_GET_INFO_NAME( tms5110 )
#define SOUND_TMS5100 DEVICE_GET_INFO_NAME( tms5100 )
#define SOUND_TMS5110A DEVICE_GET_INFO_NAME( tms5110a )
#define SOUND_CD2801 DEVICE_GET_INFO_NAME( cd2801 )
#define SOUND_TMC0281 DEVICE_GET_INFO_NAME( tmc0281 )
#define SOUND_CD2802 DEVICE_GET_INFO_NAME( cd2802 )
#define SOUND_M58817 DEVICE_GET_INFO_NAME( m58817 )


#endif /* __TMS5110_H__ */
