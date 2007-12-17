#ifndef k005289_h
#define k005289_h

struct k005289_interface
{
	int region;			/* memory region */
};

WRITE8_HANDLER( k005289_control_A_w );
WRITE8_HANDLER( k005289_control_B_w );
WRITE8_HANDLER( k005289_pitch_A_w );
WRITE8_HANDLER( k005289_pitch_B_w );
WRITE8_HANDLER( k005289_keylatch_A_w );
WRITE8_HANDLER( k005289_keylatch_B_w );

#endif
