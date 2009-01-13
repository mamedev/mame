#pragma once

#ifndef __HC55516_H__
#define __HC55516_H__


/* sets the digit (0 or 1) */
void hc55516_digit_w(int num, int digit);

/* sets the clock state (0 or 1, clocked on the rising edge) */
void hc55516_clock_w(int num, int state);

/* clears or sets the clock state */
void hc55516_clock_clear_w(int num);
void hc55516_clock_set_w(int num);

/* clears the clock state and sets the digit */
void hc55516_digit_clock_clear_w(int num, int digit);

/* returns whether the clock is currently LO or HI */
int hc55516_clock_state_r(int num);

WRITE8_HANDLER( hc55516_0_digit_w );
WRITE8_HANDLER( hc55516_0_clock_w );
WRITE8_HANDLER( hc55516_0_clock_clear_w );
WRITE8_HANDLER( hc55516_0_clock_set_w );
WRITE8_HANDLER( hc55516_0_digit_clock_clear_w );
READ8_HANDLER ( hc55516_0_clock_state_r );

WRITE8_HANDLER( hc55516_1_digit_w );
WRITE8_HANDLER( hc55516_1_clock_w );
WRITE8_HANDLER( hc55516_1_clock_clear_w );
WRITE8_HANDLER( hc55516_1_clock_set_w );
WRITE8_HANDLER( hc55516_1_digit_clock_clear_w );
READ8_HANDLER ( hc55516_1_clock_state_r );

SND_GET_INFO( hc55516 );
SND_GET_INFO( mc3417 );
SND_GET_INFO( mc3418 );

#define SOUND_HC55516 SND_GET_INFO_NAME( hc55516 )
#define SOUND_MC3417 SND_GET_INFO_NAME( mc3417 )
#define SOUND_MC3418 SND_GET_INFO_NAME( mc3418 )

#endif /* __HC55516_H__ */
