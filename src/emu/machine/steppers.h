// license:BSD-3-Clause
// copyright-holders:James Wallace
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

#define NOT_A_REEL              0
#define STARPOINT_48STEP_REEL   1           /* STARPOINT RMXXX reel unit */
#define STARPOINT_144STEP_DICE  2           /* STARPOINT 1DCU DICE mechanism */
#define STARPOINT_200STEP_REEL  3

#define BARCREST_48STEP_REEL    4           /* Barcrest bespoke reel unit */
#define MPU3_48STEP_REEL        5

#define ECOIN_200STEP_REEL      6           /* Probably not bespoke, but can't find a part number */

#define GAMESMAN_48STEP_REEL    7
#define GAMESMAN_100STEP_REEL   8
#define GAMESMAN_200STEP_REEL   9

#define PROJECT_48STEP_REEL     10

#define MCFG_STEPPER_ADD(_tag)\
	MCFG_DEVICE_ADD(_tag, STEPPER, 0)

#define MCFG_STEPPER_REEL_TYPE(_data) \
	stepper_device::set_reel_type(*device, _data);

/* total size of reel (in half steps) */
#define MCFG_STEPPER_MAX_STEPS(_write) \
	stepper_device::set_max_steps(*device, _write);

/* start position of index (in half steps) */
#define MCFG_STEPPER_START_INDEX(_write) \
	stepper_device::set_start_index(*device, _write);

/* end position of index (in half steps) */
#define MCFG_STEPPER_END_INDEX(_write) \
	stepper_device::set_end_index(*device, _write);

/* end position of index (in half steps) */
#define MCFG_STEPPER_INDEX_PATTERN(_write) \
	stepper_device::set_index_pattern(*device, _write);

/* Phase at 0, for opto linkage */
#define MCFG_STEPPER_INIT_PHASE(_write) \
	stepper_device::set_init_phase(*device, _write);

#define MCFG_STARPOINT_48STEP_ADD(_tag)\
	MCFG_STEPPER_ADD(_tag)\
	MCFG_STEPPER_REEL_TYPE(STARPOINT_48STEP_REEL)\
	MCFG_STEPPER_START_INDEX(1)\
	MCFG_STEPPER_END_INDEX(3)\
	MCFG_STEPPER_INDEX_PATTERN(0x09)\
	MCFG_STEPPER_INIT_PHASE(4)

#define MCFG_STARPOINT_RM20_48STEP_ADD(_tag)\
	MCFG_DEVICE_ADD(_tag, STEPPER, 0)\
	MCFG_STEPPER_REEL_TYPE(STARPOINT_48STEP_REEL)\
	MCFG_STEPPER_START_INDEX(16)\
	MCFG_STEPPER_END_INDEX(24)\
	MCFG_STEPPER_INDEX_PATTERN(0x09)\
	MCFG_STEPPER_INIT_PHASE(7)

#define MCFG_STARPOINT_200STEP_ADD(_tag)\
	MCFG_DEVICE_ADD(_tag, STEPPER, 0)\
	MCFG_STEPPER_REEL_TYPE(STARPOINT_200STEP_REEL)\
	MCFG_STEPPER_MAX_STEPS(200*2)\
	MCFG_STEPPER_START_INDEX(12)\
	MCFG_STEPPER_END_INDEX(24)\
	MCFG_STEPPER_INDEX_PATTERN(0x09)\
	MCFG_STEPPER_INIT_PHASE(7)

//guess
#define MCFG_ECOIN_200STEP_ADD(_tag)\
	MCFG_DEVICE_ADD(_tag, STEPPER, 0)\
	MCFG_STEPPER_REEL_TYPE(ECOIN_200STEP_REEL)\
	MCFG_STEPPER_MAX_STEPS(200*2)\
	MCFG_STEPPER_START_INDEX(12)\
	MCFG_STEPPER_END_INDEX(24)\
	MCFG_STEPPER_INDEX_PATTERN(0x09)\
	MCFG_STEPPER_INIT_PHASE(7)

#define MCFG_STEPPER_OPTIC_CALLBACK(_write) \
	devcb = &stepper_device::set_optic_handler(*device, DEVCB_##_write);

class stepper_device;
extern const device_type STEPPER;

class stepper_device : public device_t
{
public:
	stepper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_optic_handler(device_t &device, _Object object) { return downcast<stepper_device &>(device).m_optic_cb.set_callback(object); }

	static void set_reel_type(device_t &device, UINT8 type)
	{
		downcast<stepper_device &>(device).m_type = type;
		switch ( type )
		{   default:
			case STARPOINT_48STEP_REEL:  /* STARPOINT RMxxx */
			case BARCREST_48STEP_REEL :  /* Barcrest Reel unit */
			case MPU3_48STEP_REEL :
			case GAMESMAN_48STEP_REEL :  /* Gamesman GMxxxx */
			case PROJECT_48STEP_REEL :
			downcast<stepper_device &>(device).m_max_steps = (48*2);
			break;
			case GAMESMAN_100STEP_REEL :
			downcast<stepper_device &>(device).m_max_steps = (100*2);
			break;
			case STARPOINT_144STEP_DICE :/* STARPOINT 1DCU DICE mechanism */
			//Dice reels are 48 step motors, but complete three full cycles between opto updates
			downcast<stepper_device &>(device).m_max_steps = ((48*3)*2);
			break;
			case STARPOINT_200STEP_REEL :
			case GAMESMAN_200STEP_REEL :
			case ECOIN_200STEP_REEL :
			downcast<stepper_device &>(device).m_max_steps = (200*2);
			break;
		}
	}
	static void set_max_steps(device_t &device, INT16 steps) { downcast<stepper_device &>(device).m_max_steps = steps; }
	static void set_start_index(device_t &device, INT16 index) { downcast<stepper_device &>(device).m_index_start = index; }
	static void set_end_index(device_t &device, INT16 index) { downcast<stepper_device &>(device).m_index_end = index; }
	static void set_index_pattern(device_t &device, INT16 index) { downcast<stepper_device &>(device).m_index_patt = index; }
	static void set_init_phase(device_t &device, UINT8 phase)
	{
		downcast<stepper_device &>(device).m_initphase = phase;
		downcast<stepper_device &>(device).m_phase = phase;
		downcast<stepper_device &>(device).m_old_phase = phase;
	}

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
