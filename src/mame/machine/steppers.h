///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Steppermotor emulation                                                //
//                                                                       //
// Emulates : 48 step motors driven with full step or half step          //
//            also emulates the index optic                              //
//                                                                       //
// 26-01-2007: J. Wallace - Rewritten to make it more flexible           //
//                          and to allow indices to be set in drivers    //
// 29-12-2006: J. Wallace - Added state save support                     //
// 05-03-2004: Re-Animator                                               //
//                                                                       //
// TODO:  add different types of stepper motors if needed                //
//                                                                       //
///////////////////////////////////////////////////////////////////////////


#ifndef INC_STEPPERS
#define INC_STEPPERS

#define MAX_STEPPERS		    8			// maximum number of steppers

#define STARPOINT_48STEP_REEL   0			// STARPOINT RMXXX reel unit
#define BARCREST_48STEP_REEL    1			// Barcrest bespoke reel unit
#define STARPOINT_144STEPS_DICE 2			// STARPOINT 1DCU DICE mechanism - tech sheet available  on request

void Stepper_init(  int id, int type);		// init a stepper motor

void Stepper_reset_position(int id);		// reset a motor to position 0

int  Stepper_optic_state(   int id);		// read a motor's optics

void Stepper_set_index(int id,int position,int length,int pattern);

int  Stepper_update(int id, UINT8 pattern);	// update a motor

int  Stepper_get_position(int id);			// get current position in half steps

int  Stepper_get_max(int id);				// get maximum position in half steps
#endif
