///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Steppermotor emulation                                                //
//                                                                       //
// Emulates : 48 step motors driven with full step or half step          //
//            also emulates the index optic                              //
//                                                                       //
// 13-08-2008: J. Wallace - Added a rigid interface structure,           //
//                          and removed unnecessary inline functions     //
//                          Rewritten to make it more flexible           //
// 26-01-2007: J. Wallace - Rewritten to make it more flexible           //
// 26-01-2007: J. Wallace - Rewritten to make it more flexible           //
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

#define MAX_STEPPERS		    8			/* maximum number of steppers */

#define STARPOINT_48STEP_REEL   0			/* STARPOINT RMXXX reel unit */
#define BARCREST_48STEP_REEL    1			/* Barcrest bespoke reel unit */
#define STARPOINT_144STEPS_DICE 2			/* STARPOINT 1DCU DICE mechanism */

/*------------- Stepper motor interface structure -----------------*/

typedef struct _stepper_interface stepper_interface;
struct _stepper_interface
{
	UINT8 type; /* Reel unit type */
	INT16 index_start;/* start position of index (in half steps) */
	INT16 index_end;  /* end position of index (in half steps) */
	INT16 index_patt; /* pattern needed on coils (0=don't care) */
};

extern const stepper_interface starpoint_interface_48step;

void stepper_config(int which, const stepper_interface *intf);

void stepper_reset_position(int id);		/* reset a motor to position 0 */

int  stepper_optic_state(   int id);		/* read a motor's optics */

int  stepper_update(int id, UINT8 pattern);	/* update a motor */

int  stepper_get_position(int id);			/* get current position in half steps */

int  stepper_get_max(int id);				/* get maximum position in half steps */
#endif
