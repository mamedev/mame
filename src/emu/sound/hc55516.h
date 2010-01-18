#pragma once

#ifndef __HC55516_H__
#define __HC55516_H__


/* sets the digit (0 or 1) */
void hc55516_digit_w(running_device *device, int digit);

/* sets the clock state (0 or 1, clocked on the rising edge) */
void hc55516_clock_w(running_device *device, int state);

/* returns whether the clock is currently LO or HI */
int hc55516_clock_state_r(running_device *device);

DEVICE_GET_INFO( hc55516 );
DEVICE_GET_INFO( mc3417 );
DEVICE_GET_INFO( mc3418 );

#define SOUND_HC55516 DEVICE_GET_INFO_NAME( hc55516 )
#define SOUND_MC3417 DEVICE_GET_INFO_NAME( mc3417 )
#define SOUND_MC3418 DEVICE_GET_INFO_NAME( mc3418 )

#endif /* __HC55516_H__ */
