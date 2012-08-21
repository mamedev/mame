/*************************************************************************
 *
 *      pc_joy.h
 *
 *      joystick port
 *
 *************************************************************************/

#ifndef PC_JOY_H
#define PC_JOY_H


READ8_HANDLER ( pc_JOY_r );
WRITE8_HANDLER ( pc_JOY_w );

INPUT_PORTS_EXTERN( pc_joystick_none );
INPUT_PORTS_EXTERN( pc_joystick );


#endif /* PC_JOY_H */
