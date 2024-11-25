// license:BSD-3-Clause
// copyright-holders:James Wallace
///////////////////////////////////////////////////////////////////////////
//                                                                       //
// steppers.cpp steppermotor emulation                                   //
//                                                                       //
// Emulates : stepper motors driven with full step or half step          //
//            also emulates the index optic                              //
//                                                                       //
//                                                                       //
// TODO:  add further types of stepper motors if needed (Konami/IGT?)    //
///////////////////////////////////////////////////////////////////////////


#ifndef MAME_MACHINE_STEPPERS_H
#define MAME_MACHINE_STEPPERS_H

#pragma once

#define BASIC_STEPPER           0
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

#define SRU_200STEP_REEL        11

#define SYS5_100STEP_REEL       12


class stepper_device : public device_t
{
public:
	stepper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t init_phase)
		: stepper_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_init_phase(init_phase);
	}

	stepper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto optic_handler() { return m_optic_cb.bind(); }

	/* total size of reel (in half steps) */
	void set_max_steps(int16_t steps) { m_max_steps = steps; }

	/* start position of index (in half steps) */
	void set_start_index(int16_t index) { m_index_start = index; }

	/* end position of index (in half steps) */
	void set_end_index(int16_t index) { m_index_end = index; }

	/* end position of index (in half steps) */
	void set_index_pattern(int16_t index) { m_index_patt = index; }

	/* Phase at 0, for opto linkage */
	void set_init_phase(uint8_t phase) { m_initphase = phase; m_phase = phase; m_old_phase = phase; }

	/* update a motor */
	int update(uint8_t pattern);

	/* get current position in half steps */
	int get_position()          { return m_step_pos; }
	/* get current absolute position in half steps */
	int get_absolute_position() { return m_abs_step_pos; }
	/* set absolute position in half steps */
	void set_absolute_position(int pos) { m_abs_step_pos = pos; }
	/* get maximum position in half steps */
	int get_max()               { return m_max_steps; }

protected:
	stepper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint8_t m_pattern;      /* coil pattern */
	uint8_t m_old_pattern;  /* old coil pattern */
	uint8_t m_initphase;
	uint8_t m_phase;        /* motor phase */
	uint8_t m_old_phase;    /* old phase */
	int16_t m_step_pos;     /* step position 0 - max_steps */
	int16_t m_max_steps;    /* maximum step position */
	int32_t m_abs_step_pos; /* absolute step position */
	int16_t m_index_start;  /* start position of index (in half steps) */
	int16_t m_index_end;    /* end position of index (in half steps) */
	int16_t m_index_patt;   /* pattern needed on coils (0=don't care) */
	uint8_t m_optic;

	void update_optic();
	virtual void advance_phase();
	devcb_write_line m_optic_cb;
};

class reel_device : public stepper_device
{
public:
	reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t type, int16_t start_index, int16_t end_index
		, int16_t index_pattern, uint8_t init_phase, int16_t max_steps = 48*2);

	reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void advance_phase() override;

	void set_reel_type(uint8_t type)
	{
		m_type = type;
		switch ( type )
		{
		default:
		case STARPOINT_48STEP_REEL:  /* STARPOINT RMxxx */
		case BARCREST_48STEP_REEL :  /* Barcrest Reel unit */
		case MPU3_48STEP_REEL :
		case GAMESMAN_48STEP_REEL :  /* Gamesman GMxxxx */
		case PROJECT_48STEP_REEL :
			m_max_steps = (48*2);
			break;
		case GAMESMAN_100STEP_REEL :
			m_max_steps = (100*2);
			break;
		case STARPOINT_144STEP_DICE :/* STARPOINT 1DCU DICE mechanism */
			//Dice reels are 48 step motors, but complete three full cycles between opto updates
			m_max_steps = ((48*3)*2);
			break;
		case STARPOINT_200STEP_REEL :
		case GAMESMAN_200STEP_REEL :
		case ECOIN_200STEP_REEL :
		case SRU_200STEP_REEL :
			m_max_steps = (200*2);
			break;
		}
	}

	uint8_t m_type;         /* reel type */
};

DECLARE_DEVICE_TYPE(STEPPER, stepper_device)
DECLARE_DEVICE_TYPE(REEL, reel_device)

#endif // MAME_MACHINE_STEPPERS_H
