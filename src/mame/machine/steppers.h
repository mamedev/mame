///////////////////////////////////////////////////////////////////////////
//                                                                       //
// steppers.c steppermotor emulation                                     //
//                                                                       //
// Emulates : stepper motors driven with full step or half step          //
//            also emulates the index optic                              //
//                                                                       //
//                                                                       //
// TODO:  add further types of stepper motors if needed (Konami/IGT?)    //
//        Someone who understands the device system may want to convert  //
//        this                                                           //
///////////////////////////////////////////////////////////////////////////


#ifndef INC_STEPPERS
#define INC_STEPPERS

#define MAX_STEPPERS		    8			/* maximum number of steppers */

#define STARPOINT_48STEP_REEL   0			/* STARPOINT RMXXX reel unit */
#define STARPOINT_144STEP_DICE  1			/* STARPOINT 1DCU DICE mechanism */
#define STARPOINT_200STEP_REEL  2

#define BARCREST_48STEP_REEL    3			/* Barcrest bespoke reel unit */
#define MPU3_48STEP_REEL        4

#define ECOIN_200STEP_REEL      5			/* Probably not bespoke, but can't find a part number */

#define GAMESMAN_48STEP_REEL    6
#define GAMESMAN_100STEP_REEL   7
#define GAMESMAN_200STEP_REEL   8

#define PROJECT_48STEP_REEL     9

/*------------- Stepper motor interface structure -----------------*/

struct stepper_interface
{
	UINT8 type; /* Reel unit type */
	INT16 index_start;/* start position of index (in half steps) */
	INT16 index_end;  /* end position of index (in half steps) */
	INT16 index_patt; /* pattern needed on coils (0=don't care) */
	UINT8 initphase; /* Phase at 0, for opto linkage */
};

extern const stepper_interface starpoint_interface_48step;
extern const stepper_interface starpointrm20_interface_48step;

extern const stepper_interface starpoint_interface_200step_reel;
extern const stepper_interface ecoin_interface_200step_reel;

void stepper_config(running_machine &machine, int which, const stepper_interface *intf);

void stepper_reset_position(int id);		/* reset a motor to position 0 */

int  stepper_optic_state(   int id);		/* read a motor's optics */

int  stepper_update(int id, UINT8 pattern);	/* update a motor */

int  stepper_get_position(int id);			/* get current position in half steps */

int  stepper_get_max(int id);				/* get maximum position in half steps */
#endif
