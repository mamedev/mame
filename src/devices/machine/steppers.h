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
//                                                                       //
///////////////////////////////////////////////////////////////////////////


#ifndef MAME_MACHINE_STEPPERS_H
#define MAME_MACHINE_STEPPERS_H

#pragma once

#define BASIC_STEPPER            0
#define STARPOINT_48STEP_REEL    1           // STARPOINT RMXXX reel unit
#define STARPOINT_144STEP_DICE   2           // STARPOINT 1DCU DICE mechanism
#define STARPOINT_200STEP_REEL   3

#define BARCREST_48STEP_REEL     4           // Barcrest bespoke reel unit
#define MPU3_48STEP_REEL         5

#define ECOIN_200STEP_REEL       6           // Probably not bespoke, but can't find a part number

#define GAMESMAN_48STEP_REEL     7
#define GAMESMAN_100STEP_REEL    8
#define GAMESMAN_200STEP_REEL    9

#define PROJECT_48STEP_REEL      10

#define SRU_200STEP_REEL         11

#define SYS5_100STEP_REEL        12

#define UNIVERSAL_200STEP_REEL   13
#define YAMASA_200STEP_REEL      14
#define YAMASA_252STEP_REEL      15
#define SAMMY_200STEP_REEL       16
#define PIONEER_200STEP_REEL     17
#define ARKTECHNICO_200STEP_REEL 18
#define OLYMPIA_200STEP_REEL     19

// Stepper behaviour flags.
//
// These are optional behaviour overrides layered on top of the reel type's
// phase table.  They are intended for machine-specific motion quirks where
// the raw phase transition alone does not match the visible reel movement.
//
// STEPPER_IGNORE_ZERO
//   Treat a 0x00 drive pattern as an idle / de-energised hold state and do
//   not advance the motor position from it.
//
// STEPPER_ALLOW_MULTISTEP
//   Allow wrapped phase deltas larger than one half-step to move by multiple
//   half-steps instead of always collapsing to a single-step update.
//
// STEPPER_KICK_DIR_POSITIVE / STEPPER_KICK_DIR_NEGATIVE
//   Force a deterministic direction when a special kick or wrapped opposite-
//   phase transition would otherwise be ambiguous.  Use one of these with
//   STEPPER_ALLOW_MULTISTEP and/or STEPPER_CLAMP_OPPOSITE_PHASE_START.
//
// STEPPER_CLAMP_OPPOSITE_PHASE_START
//   After a 0x00 hold, clamp the first valid restart pattern to a normal
//   single half-step move instead of allowing a larger wrapped jump.
#define STEPPER_IGNORE_ZERO                 0x01
#define STEPPER_ALLOW_MULTISTEP             0x02
#define STEPPER_KICK_DIR_POSITIVE           0x04
#define STEPPER_KICK_DIR_NEGATIVE           0x08
#define STEPPER_CLAMP_OPPOSITE_PHASE_START  0x10
#define STEPPER_0F_SETTLEBACK               0x20

class stepper_device : public device_t
{
public:
	stepper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t init_phase)
		: stepper_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_init_phase(init_phase);
	}

	stepper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto optic_handler()			{ return m_optic_cb.bind(); }

	// total size of reel (in half steps)
	void set_max_steps(int16_t steps)	{ m_max_steps = steps; }

	// start position of index (in half steps)
	void set_start_index(int16_t index)	{ m_index_start = index; }

	// end position of index (in half steps)
	void set_end_index(int16_t index)	{ m_index_end = index; }

	// end position of index (in half steps)
	void set_index_pattern(int16_t index)	{ m_index_patt = index; }

	// Phase at 0, for opto linkage
	void set_init_phase(uint8_t phase)	{ m_initphase = phase; m_phase = phase; m_old_phase = phase; }

	// update a motor
	int update(uint8_t pattern);

	// write drive pattern and update outputs
	int write(uint8_t pattern);

	// update position / scroll outputs
	void draw();

	// get current position in half steps
	int get_position() const		{ return m_step_pos; }

	// get current absolute position in half steps
	int get_absolute_position() const	{ return m_abs_step_pos; }
	// set absolute position in half steps
	void set_absolute_position(int pos)	{ m_abs_step_pos = pos; }
	// Restore the reel position from externally saved state and resynchronise
	// wrapped position / optic state. Phase/pattern are reinitialised.
	void restore_position(int abs_pos);
	// get maximum position in half steps
	int get_max() const			{ return m_max_steps; }

	void set_flags(uint8_t flags)		{ m_flags = flags; }

protected:
	stepper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	uint8_t m_pattern;	// coil pattern
	uint8_t m_old_pattern;	// old coil pattern
	uint8_t m_initphase;
	uint8_t m_phase;	// motor phase
	uint8_t m_old_phase;	// old phase
	int16_t m_step_pos;	// step position 0 - max_steps
	int16_t m_max_steps;	// maximum step position
	int32_t m_abs_step_pos;	// absolute step position
	int16_t m_index_start;	// start position of index (in half steps)
	int16_t m_index_end;	// end position of index (in half steps)
	int16_t m_index_patt;	// pattern needed on coils (0=don't care)
	uint8_t m_optic;
	uint8_t m_flags = 0;
	bool m_zero_hold = false;
	void update_optic();
	virtual void advance_phase();
	devcb_write_line m_optic_cb;
	output_finder<> m_output_pos;
	output_finder<> m_output_scroll;
};

class reel_device : public stepper_device
{
public:
	reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint8_t type, int16_t start_index, int16_t end_index
		, int16_t index_pattern, uint8_t init_phase, int16_t max_steps = 48*2);

	reel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t reel_type() const { return m_type; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void advance_phase() override;

	void set_reel_type(uint8_t type);

	uint8_t m_type = BASIC_STEPPER;	// reel type
};

DECLARE_DEVICE_TYPE(STEPPER, stepper_device)
DECLARE_DEVICE_TYPE(REEL, reel_device)

#endif // MAME_MACHINE_STEPPERS_H
