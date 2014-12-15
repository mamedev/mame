///////////////////////////////////////////////////////////////////////////
//                                                                       //
// steppers.c steppermotor emulation                                     //
//                                                                       //
// Emulates : stepper motors driven with full step or half step          //
//            also emulates the index optic                              //
//                                                                       //
//                                                                       //
// TODO:  add further types of stepper motors if needed (Konami/IGT?)    //
///////////////////////////////////////////////////////////////////////////


#ifndef INC_STEPPERS
#define INC_STEPPERS

#define MAX_STEPPERS            8           /* maximum number of steppers */

#define STARPOINT_48STEP_REEL   0           /* STARPOINT RMXXX reel unit */
#define STARPOINT_144STEP_DICE  1           /* STARPOINT 1DCU DICE mechanism */
#define STARPOINT_200STEP_REEL  2

#define BARCREST_48STEP_REEL    3           /* Barcrest bespoke reel unit */
#define MPU3_48STEP_REEL        4

#define ECOIN_200STEP_REEL      5           /* Probably not bespoke, but can't find a part number */

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


#define MCFG_STEPPER_OPTIC_CALLBACK(_write) \
	devcb = &stepper_device::set_optic_handler(*device, DEVCB_##_write);

class stepper_device;
const device_type STEPPER = &device_creator<stepper_device>;

class stepper_device : public device_t
{
public:
	stepper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, STEPPER, "Stepper Motor", tag, owner, clock, "stepper", __FILE__),
		m_optic_cb(*this)
	{ }

	template<class _Object> static devcb_base &set_optic_handler(device_t &device, _Object object) { return downcast<stepper_device &>(device).m_optic_cb.set_callback(object); }

	void configure(const stepper_interface *intf);

	/* update a motor */
	int update(UINT8 pattern);

	/* get current position in half steps */
	int get_position()          { return m_step_pos; }
	/* get current absolute position in half steps */
	int get_absolute_position() { return m_abs_step_pos; }
	/* get maximum position in half steps */
	int get_max()               { return m_max_steps; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	UINT8 m_pattern;      /* coil pattern */
	UINT8 m_old_pattern;  /* old coil pattern */
	UINT8 m_initphase;
	UINT8 m_phase;        /* motor phase */
	UINT8 m_old_phase;    /* old phase */
	UINT8 m_type;         /* reel type */
	INT16 m_step_pos;     /* step position 0 - max_steps */
	INT16 m_max_steps;    /* maximum step position */
	INT32 m_abs_step_pos; /* absolute step position */
	INT16 m_index_start;  /* start position of index (in half steps) */
	INT16 m_index_end;    /* end position of index (in half steps) */
	INT16 m_index_patt;   /* pattern needed on coils (0=don't care) */
	UINT8 m_optic;

	void update_optic();
	devcb_write_line m_optic_cb;
};

#endif
