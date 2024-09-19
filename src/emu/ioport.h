// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ioport.h

    Input/output port handling.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_IOPORT_H
#define MAME_EMU_IOPORT_H

#include "inpttype.h"

#include "ioprocs.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <initializer_list>
#include <list>
#include <memory>
#include <vector>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// active high/low values for input ports
constexpr ioport_value IP_ACTIVE_HIGH = 0x00000000;
constexpr ioport_value IP_ACTIVE_LOW = 0xffffffff;

// maximum number of players supported
constexpr int MAX_PLAYERS = 10;

// unicode constants
constexpr char32_t UCHAR_PRIVATE = 0x100000;
constexpr char32_t UCHAR_SHIFT_1 = UCHAR_PRIVATE + 0;
constexpr char32_t UCHAR_SHIFT_2 = UCHAR_PRIVATE + 1;
constexpr char32_t UCHAR_SHIFT_BEGIN = UCHAR_SHIFT_1;
constexpr char32_t UCHAR_SHIFT_END = UCHAR_SHIFT_2;
constexpr char32_t UCHAR_MAMEKEY_BEGIN = UCHAR_PRIVATE + 2;


// crosshair types
enum crosshair_axis_t
{
	CROSSHAIR_AXIS_NONE = 0,
	CROSSHAIR_AXIS_X,
	CROSSHAIR_AXIS_Y
};


// groups for input ports
enum ioport_group
{
	IPG_UI = 0,
	IPG_PLAYER1,
	IPG_PLAYER2,
	IPG_PLAYER3,
	IPG_PLAYER4,
	IPG_PLAYER5,
	IPG_PLAYER6,
	IPG_PLAYER7,
	IPG_PLAYER8,
	IPG_PLAYER9,
	IPG_PLAYER10,
	IPG_OTHER,
	IPG_TOTAL_GROUPS,
	IPG_INVALID
};


// input type classes
enum ioport_type_class
{
	INPUT_CLASS_INTERNAL,
	INPUT_CLASS_KEYBOARD,
	INPUT_CLASS_CONTROLLER,
	INPUT_CLASS_CONFIG,
	INPUT_CLASS_DIPSWITCH,
	INPUT_CLASS_MISC
};


// default strings used in port definitions
enum
{
	INPUT_STRING_Off = 1,
	INPUT_STRING_On,
	INPUT_STRING_No,
	INPUT_STRING_Yes,
	INPUT_STRING_Lives,
	INPUT_STRING_Bonus_Life,
	INPUT_STRING_Difficulty,
	INPUT_STRING_Demo_Sounds,
	INPUT_STRING_Coinage,
	INPUT_STRING_Coin_A,
	INPUT_STRING_Coin_B,
//  INPUT_STRING_20C_1C,    //  0.050000
//  INPUT_STRING_15C_1C,    //  0.066667
//  INPUT_STRING_10C_1C,    //  0.100000
#define __input_string_coinage_start INPUT_STRING_9C_1C
	INPUT_STRING_9C_1C,     //  0.111111
	INPUT_STRING_8C_1C,     //  0.125000
	INPUT_STRING_7C_1C,     //  0.142857
	INPUT_STRING_6C_1C,     //  0.166667
//  INPUT_STRING_10C_2C,    //  0.200000
	INPUT_STRING_5C_1C,     //  0.200000
//  INPUT_STRING_9C_2C,     //  0.222222
//  INPUT_STRING_8C_2C,     //  0.250000
	INPUT_STRING_4C_1C,     //  0.250000
//  INPUT_STRING_7C_2C,     //  0.285714
//  INPUT_STRING_10C_3C,    //  0.300000
//  INPUT_STRING_9C_3C,     //  0.333333
//  INPUT_STRING_6C_2C,     //  0.333333
	INPUT_STRING_3C_1C,     //  0.333333
	INPUT_STRING_8C_3C,     //  0.375000
//  INPUT_STRING_10C_4C,    //  0.400000
//  INPUT_STRING_7C_3C,     //  0.428571
//  INPUT_STRING_9C_4C,     //  0.444444
//  INPUT_STRING_10C_5C,    //  0.500000
//  INPUT_STRING_8C_4C,     //  0.500000
//  INPUT_STRING_6C_3C,     //  0.500000
	INPUT_STRING_4C_2C,     //  0.500000
	INPUT_STRING_5C_2C,     //  0.500000
	INPUT_STRING_2C_1C,     //  0.500000
//  INPUT_STRING_9C_5C,     //  0.555556
//  INPUT_STRING_7C_4C,     //  0.571429
//  INPUT_STRING_10C_6C,    //  0.600000
	INPUT_STRING_5C_3C,     //  0.600000
//  INPUT_STRING_8C_5C,     //  0.625000
//  INPUT_STRING_9C_6C,     //  0.666667
//  INPUT_STRING_6C_4C,     //  0.666667
	INPUT_STRING_3C_2C,     //  0.666667
//  INPUT_STRING_10C_7C,    //  0.700000
//  INPUT_STRING_7C_5C,     //  0.714286
//  INPUT_STRING_8C_6C,     //  0.750000
	INPUT_STRING_4C_3C,     //  0.750000
//  INPUT_STRING_9C_7C,     //  0.777778
//  INPUT_STRING_10C_8C,    //  0.800000
//  INPUT_STRING_5C_4C,     //  0.800000
//  INPUT_STRING_6C_5C,     //  0.833333
//  INPUT_STRING_7C_6C,     //  0.857143
//  INPUT_STRING_8C_7C,     //  0.875000
//  INPUT_STRING_9C_8C,     //  0.888889
//  INPUT_STRING_10C_9C,    //  0.900000
//  INPUT_STRING_10C_10C,   //  1.000000
//  INPUT_STRING_9C_9C,     //  1.000000
//  INPUT_STRING_8C_8C,     //  1.000000
//  INPUT_STRING_7C_7C,     //  1.000000
//  INPUT_STRING_6C_6C,     //  1.000000
//  INPUT_STRING_5C_5C,     //  1.000000
	INPUT_STRING_4C_4C,     //  1.000000
	INPUT_STRING_3C_3C,     //  1.000000
	INPUT_STRING_2C_2C,     //  1.000000
	INPUT_STRING_1C_1C,     //  1.000000
//  INPUT_STRING_9C_10C,    //  1.111111
//  INPUT_STRING_8C_9C,     //  1.125000
//  INPUT_STRING_7C_8C,     //  1.142857
//  INPUT_STRING_6C_7C,     //  1.166667
//  INPUT_STRING_5C_6C,     //  1.200000
//  INPUT_STRING_8C_10C,    //  1.250000
	INPUT_STRING_3C_5C,     //  1.250000
	INPUT_STRING_4C_5C,     //  1.250000
//  INPUT_STRING_7C_9C,     //  1.285714
//  INPUT_STRING_6C_8C,     //  1.333333
	INPUT_STRING_3C_4C,     //  1.333333
//  INPUT_STRING_5C_7C,     //  1.400000
//  INPUT_STRING_7C_10C,    //  1.428571
//  INPUT_STRING_6C_9C,     //  1.500000
//  INPUT_STRING_4C_6C,     //  1.500000
	INPUT_STRING_2C_3C,     //  1.500000
//  INPUT_STRING_5C_8C,     //  1.600000
//  INPUT_STRING_6C_10C,    //  1.666667
//  INPUT_STRING_3C_5C,     //  1.666667
	INPUT_STRING_4C_7C,     //  1.750000
//  INPUT_STRING_5C_9C,     //  1.800000
//  INPUT_STRING_5C_10C,    //  2.000000
//  INPUT_STRING_4C_8C,     //  2.000000
//  INPUT_STRING_3C_6C,     //  2.000000
	INPUT_STRING_2C_4C,     //  2.000000
	INPUT_STRING_1C_2C,     //  2.000000
//  INPUT_STRING_4C_9C,     //  2.250000
//  INPUT_STRING_3C_7C,     //  2.333333
//  INPUT_STRING_4C_10C,    //  2.500000
	INPUT_STRING_2C_5C,     //  2.500000
//  INPUT_STRING_3C_8C,     //  2.666667
//  INPUT_STRING_3C_9C,     //  3.000000
	INPUT_STRING_2C_6C,     //  3.000000
	INPUT_STRING_1C_3C,     //  3.000000
//  INPUT_STRING_3C_10C,    //  3.333333
	INPUT_STRING_2C_7C,     //  3.500000
	INPUT_STRING_2C_8C,     //  4.000000
	INPUT_STRING_1C_4C,     //  4.000000
//  INPUT_STRING_2C_9C,     //  4.500000
//  INPUT_STRING_2C_10C,    //  5.000000
	INPUT_STRING_1C_5C,     //  5.000000
	INPUT_STRING_1C_6C,     //  6.000000
	INPUT_STRING_1C_7C,     //  7.000000
	INPUT_STRING_1C_8C,     //  8.000000
	INPUT_STRING_1C_9C,     //  9.000000
#define __input_string_coinage_end INPUT_STRING_1C_9C
//  INPUT_STRING_1C_10C,    //  10.000000
//  INPUT_STRING_1C_11C,    //  11.000000
//  INPUT_STRING_1C_12C,    //  12.000000
//  INPUT_STRING_1C_13C,    //  13.000000
//  INPUT_STRING_1C_14C,    //  14.000000
//  INPUT_STRING_1C_15C,    //  15.000000
//  INPUT_STRING_1C_20C,    //  20.000000
//  INPUT_STRING_1C_25C,    //  25.000000
//  INPUT_STRING_1C_30C,    //  30.000000
//  INPUT_STRING_1C_40C,    //  40.000000
//  INPUT_STRING_1C_50C,    //  50.000000
//  INPUT_STRING_1C_99C,    //  99.000000
//  INPUT_STRING_1C_100C,   //  100.000000
//  INPUT_STRING_1C_120C,   //  120.000000
//  INPUT_STRING_1C_125C,   //  125.000000
//  INPUT_STRING_1C_150C,   //  150.000000
//  INPUT_STRING_1C_200C,   //  200.000000
//  INPUT_STRING_1C_250C,   //  250.000000
//  INPUT_STRING_1C_500C,   //  500.000000
//  INPUT_STRING_1C_1000C,  //  1000.000000
	INPUT_STRING_Free_Play,
	INPUT_STRING_Cabinet,
	INPUT_STRING_Upright,
	INPUT_STRING_Cocktail,
	INPUT_STRING_Flip_Screen,
	INPUT_STRING_Service_Mode,
	INPUT_STRING_Pause,
	INPUT_STRING_Test,
	INPUT_STRING_Tilt,
	INPUT_STRING_Version,
	INPUT_STRING_Region,
	INPUT_STRING_International,
	INPUT_STRING_Japan,
	INPUT_STRING_USA,
	INPUT_STRING_Europe,
	INPUT_STRING_Asia,
	INPUT_STRING_China,
	INPUT_STRING_Hong_Kong,
	INPUT_STRING_Korea,
	INPUT_STRING_Southeast_Asia,
	INPUT_STRING_Taiwan,
	INPUT_STRING_World,
	INPUT_STRING_Language,
	INPUT_STRING_English,
	INPUT_STRING_Japanese,
	INPUT_STRING_Chinese,
	INPUT_STRING_French,
	INPUT_STRING_German,
	INPUT_STRING_Italian,
	INPUT_STRING_Korean,
	INPUT_STRING_Spanish,
	INPUT_STRING_Very_Easy,
	INPUT_STRING_Easiest,
	INPUT_STRING_Easier,
	INPUT_STRING_Easy,
	INPUT_STRING_Medium_Easy,
	INPUT_STRING_Normal,
	INPUT_STRING_Medium,
	INPUT_STRING_Medium_Hard,
	INPUT_STRING_Hard,
	INPUT_STRING_Harder,
	INPUT_STRING_Hardest,
	INPUT_STRING_Very_Hard,
	INPUT_STRING_Medium_Difficult,
	INPUT_STRING_Difficult,
	INPUT_STRING_Very_Difficult,
	INPUT_STRING_Very_Low,
	INPUT_STRING_Low,
	INPUT_STRING_High,
	INPUT_STRING_Higher,
	INPUT_STRING_Highest,
	INPUT_STRING_Very_High,
	INPUT_STRING_Players,
	INPUT_STRING_Controls,
	INPUT_STRING_Dual,
	INPUT_STRING_Single,
	INPUT_STRING_Game_Time,
	INPUT_STRING_Continue_Price,
	INPUT_STRING_Controller,
	INPUT_STRING_Light_Gun,
	INPUT_STRING_Joystick,
	INPUT_STRING_Trackball,
	INPUT_STRING_Continues,
	INPUT_STRING_Allow_Continue,
	INPUT_STRING_Level_Select,
//  INPUT_STRING_Allow,
//  INPUT_STRING_Forbid,
//  INPUT_STRING_Enable,
//  INPUT_STRING_Disable,
	INPUT_STRING_Infinite,
//  INPUT_STRING_Invincibility,
//  INPUT_STRING_Invulnerability,
	INPUT_STRING_Stereo,
	INPUT_STRING_Mono,
	INPUT_STRING_Unused,
	INPUT_STRING_Unknown,
//  INPUT_STRING_Undefined,
	INPUT_STRING_Standard,
	INPUT_STRING_Reverse,
	INPUT_STRING_Alternate,
//  INPUT_STRING_Reserve,
//  INPUT_STRING_Spare,
//  INPUT_STRING_Invalid,
	INPUT_STRING_None,

	INPUT_STRING_COUNT
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// constructor function pointer
typedef void(*ioport_constructor)(device_t &owner, ioport_list &portlist, std::string &errorbuf);

// I/O port callback function delegates
typedef device_delegate<ioport_value ()> ioport_field_read_delegate;
typedef device_delegate<void (ioport_field &, u32, ioport_value, ioport_value)> ioport_field_write_delegate;
typedef device_delegate<float (float)> ioport_field_crossmap_delegate;


// ======================> input_device_default

// device defined default input settings
struct input_device_default
{
	const char *            tag;            // tag of port to update
	ioport_value            mask;           // mask to apply to the port
	ioport_value            defvalue;       // new default value
};


// ======================> input_type_entry

// describes a fundamental input type, including default input sequences
class input_type_entry
{
public:
	// construction/destruction
	input_type_entry(ioport_type type, ioport_group group, int player, const char *token, const char *name, input_seq standard) noexcept;
	input_type_entry(ioport_type type, ioport_group group, int player, const char *token, const char *name, input_seq standard, input_seq decrement, input_seq increment) noexcept;

	// getters
	ioport_type type() const noexcept { return m_type; }
	ioport_group group() const noexcept { return m_group; }
	u8 player() const noexcept { return m_player; }
	const char *token() const noexcept { return m_token; }
	std::string name() const;
	input_seq &defseq(input_seq_type seqtype = SEQ_TYPE_STANDARD) noexcept { return m_defseq[seqtype]; }
	const input_seq &defseq(input_seq_type seqtype = SEQ_TYPE_STANDARD) const noexcept { return m_defseq[seqtype]; }
	const input_seq &seq(input_seq_type seqtype = SEQ_TYPE_STANDARD) const noexcept { return m_seq[seqtype]; }
	const std::string &cfg(input_seq_type seqtype = SEQ_TYPE_STANDARD) const noexcept { return m_cfg[seqtype]; }

	// setters
	void restore_default_seq() noexcept;
	void set_seq(input_seq_type seqtype, const input_seq &seq) noexcept { m_seq[seqtype] = seq; }
	template <typename... T> void set_cfg(input_seq_type seqtype, T &&... cfg) { m_cfg[seqtype].assign(std::forward<T>(cfg)...); }
	void replace_code(input_code oldcode, input_code newcode) noexcept;
	void configure_osd(const char *token, const char *name) noexcept;

private:
	// internal state
	ioport_type                             m_type;     // IPT_* for this entry
	ioport_group                            m_group;    // which group the port belongs to
	u8                                      m_player;   // player number (0 is player 1)
	const char *                            m_token;    // token used to store settings
	const char *                            m_name;     // user-friendly name
	std::array<input_seq, SEQ_TYPE_TOTAL>   m_defseq;   // default input sequence
	std::array<input_seq, SEQ_TYPE_TOTAL>   m_seq;      // currently configured sequences
	std::array<std::string, SEQ_TYPE_TOTAL> m_cfg;      // configuration strings
};


// ======================> digital_joystick

// tracking information about a digital joystick input
class digital_joystick
{
	DISABLE_COPYING(digital_joystick);

public:
	// directions
	enum direction_t
	{
		JOYDIR_UP,
		JOYDIR_DOWN,
		JOYDIR_LEFT,
		JOYDIR_RIGHT,
		JOYDIR_COUNT
	};

	// bit constants
	static constexpr u8 UP_BIT = 1 << JOYDIR_UP;
	static constexpr u8 DOWN_BIT = 1 << JOYDIR_DOWN;
	static constexpr u8 LEFT_BIT = 1 << JOYDIR_LEFT;
	static constexpr u8 RIGHT_BIT = 1 << JOYDIR_RIGHT;

	// construction/destruction
	digital_joystick(int player, int number);

	// getters
	int player() const noexcept { return m_player; }
	int number() const noexcept { return m_number; }
	u8 current() const noexcept { return m_current; }
	u8 current4way() const noexcept { return m_current4way; }

	// configuration
	direction_t add_axis(ioport_field &field);

	// updates
	void frame_update();

private:
	// internal state
	int                         m_player;                                       // player number represented
	int                         m_number;                                       // joystick number represented
	std::forward_list<std::reference_wrapper<ioport_field> > m_field[JOYDIR_COUNT];  // potential input fields for each direction
	u8                          m_current;                                      // current value
	u8                          m_current4way;                                  // current 4-way value
	u8                          m_previous;                                     // previous value
};
DECLARE_ENUM_INCDEC_OPERATORS(digital_joystick::direction_t)


// ======================> ioport_condition

// encapsulates a condition on a port field or setting
class ioport_condition
{
public:
	// condition types
	enum condition_t
	{
		ALWAYS = 0,
		EQUALS,
		NOTEQUALS,
		GREATERTHAN,
		NOTGREATERTHAN,
		LESSTHAN,
		NOTLESSTHAN
	};

	// construction/destruction
	ioport_condition() { reset(); }
	ioport_condition(condition_t condition, const char *tag, ioport_value mask, ioport_value value) { set(condition, tag, mask, value); }

	// getters
	condition_t condition() const { return m_condition; }
	const char *tag() const { return m_tag; }
	ioport_value mask() const { return m_mask; }
	ioport_value value() const { return m_value; }

	// operators
	bool operator==(const ioport_condition &rhs) const { return (m_mask == rhs.m_mask && m_value == rhs.m_value && m_condition == rhs.m_condition && strcmp(m_tag, rhs.m_tag) == 0); }
	bool eval() const;
	bool none() const { return (m_condition == ALWAYS); }

	// configuration
	void reset() { set(ALWAYS, nullptr, 0, 0); }
	void set(condition_t condition, const char *tag, ioport_value mask, ioport_value value)
	{
		m_condition = condition;
		m_tag = tag;
		m_port = nullptr;
		m_mask = mask;
		m_value = value;
	}

	void initialize(device_t &device);

private:
	// internal state
	condition_t     m_condition;    // condition to use
	const char *    m_tag;          // tag of port whose condition is to be tested
	ioport_port *   m_port;         // reference to the port to be tested
	ioport_value    m_mask;         // mask to apply to the port
	ioport_value    m_value;        // value to compare against
};


// ======================> ioport_setting

// a single setting for a configuration or DIP switch
class ioport_setting
{
public:
	// construction/destruction
	ioport_setting(ioport_field &field, ioport_value value, const char *name);

	// getters
	ioport_field &field() const { return m_field; }
	device_t &device() const;
	running_machine &machine() const;
	ioport_value value() const { return m_value; }
	ioport_condition &condition() { return m_condition; }
	ioport_condition const &condition() const { return m_condition; }
	const char *name() const { return m_name; }

	// helpers
	bool enabled() const { return m_condition.eval(); }

private:
	// internal state
	ioport_field &      m_field;        // pointer back to the field that owns us
	ioport_value        m_value;        // value of the bits in this setting
	const char *        m_name;         // user-friendly name to display
	ioport_condition    m_condition;    // condition under which this setting is valid
};


// ======================> ioport_diplocation

// a mapping from a bit to a physical DIP switch description
class ioport_diplocation
{
public:
	// construction/destruction
	ioport_diplocation(const char *name, u8 swnum, bool invert);

	// getters
	const char *name() const { return m_name.c_str(); }
	u8 number() const { return m_number; }
	bool inverted() const { return m_invert; }

private:
	std::string         m_name;         // name of the physical DIP switch
	u8                  m_number;       // physical switch number
	bool                m_invert;       // is this an active-high DIP?
};


// ======================> ioport_field

// a single bitfield within an input port
class ioport_field
{
	DISABLE_COPYING(ioport_field);
	friend class simple_list<ioport_field>;
	friend class ioport_manager;
	friend class ioport_configurer;
	friend class dynamic_field;

	// flags for ioport_fields
	static inline constexpr u32 FIELD_FLAG_OPTIONAL = 0x0001;    // set if this field is not required but recognized by hw
	static inline constexpr u32 FIELD_FLAG_COCKTAIL = 0x0002;    // set if this field is relevant only for cocktail cabinets
	static inline constexpr u32 FIELD_FLAG_TOGGLE =   0x0004;    // set if this field should behave as a toggle
	static inline constexpr u32 FIELD_FLAG_ROTATED =  0x0008;    // set if this field represents a rotated control
	static inline constexpr u32 ANALOG_FLAG_REVERSE = 0x0010;    // analog only: reverse the sense of the axis
	static inline constexpr u32 ANALOG_FLAG_RESET =   0x0020;    // analog only: always preload in->default for relative axes, returning only deltas
	static inline constexpr u32 ANALOG_FLAG_WRAPS =   0x0040;    // analog only: positional count wraps around
	static inline constexpr u32 ANALOG_FLAG_INVERT =  0x0080;    // analog only: bitwise invert bits

public:
	// construction/destruction
	ioport_field(ioport_port &port, ioport_type type, ioport_value defvalue, ioport_value maskbits, const char *name = nullptr);
	~ioport_field();

	// getters
	ioport_field *next() const { return m_next; }
	ioport_port &port() const { return m_port; }
	device_t &device() const;
	ioport_manager &manager() const;
	running_machine &machine() const;
	int modcount() const { return m_modcount; }
	const std::vector<ioport_setting> &settings() const { return m_settinglist; }
	const std::vector<ioport_diplocation> &diplocations() const { return m_diploclist; }

	ioport_value mask() const { return m_mask; }
	ioport_value defvalue() const { return m_defvalue; }
	ioport_condition &condition() { return m_condition; }
	ioport_condition const &condition() const { return m_condition; }
	ioport_type type() const { return m_type; }
	u8 player() const { return m_player; }
	bool digital_value() const { return m_digital_value; }
	void set_value(ioport_value value);
	void clear_value();

	bool optional() const { return ((m_flags & FIELD_FLAG_OPTIONAL) != 0); }
	bool cocktail() const { return ((m_flags & FIELD_FLAG_COCKTAIL) != 0); }
	bool toggle() const { return ((m_flags & FIELD_FLAG_TOGGLE) != 0); }
	bool rotated() const { return ((m_flags & FIELD_FLAG_ROTATED) != 0); }
	bool analog_reverse() const { return ((m_flags & ANALOG_FLAG_REVERSE) != 0); }
	bool analog_reset() const { return ((m_flags & ANALOG_FLAG_RESET) != 0); }
	bool analog_wraps() const { return ((m_flags & ANALOG_FLAG_WRAPS) != 0); }
	bool analog_invert() const { return ((m_flags & ANALOG_FLAG_INVERT) != 0); }

	u8 impulse() const noexcept { return m_impulse; }
	std::string name() const;
	const char *specific_name() const noexcept { return m_name; }
	const input_seq &seq(input_seq_type seqtype = SEQ_TYPE_STANDARD) const noexcept;
	const input_seq &defseq(input_seq_type seqtype = SEQ_TYPE_STANDARD) const noexcept;
	const input_seq &defseq_unresolved(input_seq_type seqtype = SEQ_TYPE_STANDARD) const noexcept { return m_seq[seqtype]; }
	void set_defseq(const input_seq &newseq) noexcept { set_defseq(SEQ_TYPE_STANDARD, newseq); }
	void set_defseq(input_seq_type seqtype, const input_seq &newseq);
	bool has_dynamic_read() const { return !m_read.isnull(); }
	bool has_dynamic_write() const { return !m_write.isnull(); }

	ioport_value minval() const noexcept { return m_min; }
	ioport_value maxval() const noexcept { return m_max; }
	s32 sensitivity() const noexcept { return m_sensitivity; }
	s32 delta() const noexcept { return m_delta; }
	s32 centerdelta() const noexcept { return m_centerdelta; }
	crosshair_axis_t crosshair_axis() const noexcept { return m_crosshair_axis; }
	float crosshair_scale() const noexcept { return m_crosshair_scale; }
	float crosshair_offset() const noexcept { return m_crosshair_offset; }
	float crosshair_altaxis() const noexcept { return m_crosshair_altaxis; }
	u16 full_turn_count() const noexcept { return m_full_turn_count; }
	const ioport_value *remap_table() const noexcept { return m_remap_table; }

	u8 way() const noexcept { return m_way; }
	std::vector<char32_t> keyboard_codes(int which) const;
	std::string key_name(int which) const;
	ioport_field_live &live() const { assert(m_live != nullptr); return *m_live; }

	// setters
	void set_crosshair_scale(float scale) { m_crosshair_scale = scale; }
	void set_crosshair_offset(float offset) { m_crosshair_offset = offset; }
	void set_player(u8 player) { m_player = player; }

	// derived getters
	ioport_type_class type_class() const noexcept;
	bool is_analog() const noexcept { return (m_type > IPT_ANALOG_FIRST && m_type < IPT_ANALOG_LAST); }
	bool is_digital_joystick() const noexcept { return (m_type > IPT_DIGITAL_JOYSTICK_FIRST && m_type < IPT_DIGITAL_JOYSTICK_LAST); }

	// additional operations
	bool enabled() const { return m_condition.eval(); }
	const char *setting_name() const;
	bool has_previous_setting() const;
	void select_previous_setting();
	bool has_next_setting() const;
	void select_next_setting();
	float crosshair_read() const;
	void init_live_state(analog_field *analog);
	void frame_update(ioport_value &result);
	void reduce_mask(ioport_value bits_to_remove) { m_mask &= ~bits_to_remove; }

	// user-controllable settings for a field
	struct user_settings
	{
		ioport_value    value = 0;              // for DIP switches
		input_seq       seq[SEQ_TYPE_TOTAL];    // sequences of all types
		std::string     cfg[SEQ_TYPE_TOTAL];    // configuration strings of all types
		s32             sensitivity = 0;        // for analog controls
		s32             delta = 0;              // for analog controls
		s32             centerdelta = 0;        // for analog controls
		bool            reverse = false;        // for analog controls
		bool            toggle = false;         // for non-analog controls
	};
	void get_user_settings(user_settings &settings) const;
	void set_user_settings(const user_settings &settings);

private:
	void expand_diplocation(const char *location, std::string &errorbuf);

	// internal state
	ioport_field *              m_next;             // pointer to next field in sequence
	ioport_port &               m_port;             // reference to the port that owns us
	std::unique_ptr<ioport_field_live> m_live;      // live state of field (nullptr if not live)
	int                         m_modcount;         // modification count
	std::vector<ioport_setting> m_settinglist;      // list of input_setting_configs
	std::vector<ioport_diplocation> m_diploclist;   // list of locations for various bits

	// generally-applicable data
	ioport_value                m_mask;             // mask of bits belonging to the field
	ioport_value                m_defvalue;         // default value of these bits
	ioport_condition            m_condition;        // condition under which this field is relevant
	ioport_type                 m_type;             // IPT_* type for this port
	u8                          m_player;           // player number (0-based)
	u32                         m_flags;            // combination of FIELD_FLAG_* and ANALOG_FLAG_* above
	u8                          m_impulse;          // number of frames before reverting to defvalue
	const char *                m_name;             // user-friendly name to display
	input_seq                   m_seq[SEQ_TYPE_TOTAL];// sequences of all types
	ioport_field_read_delegate  m_read;             // read callback routine
	ioport_field_write_delegate m_write;            // write callback routine
	u32                         m_write_param;      // parameter for write callback routine

	// data relevant to digital control types
	bool                        m_digital_value;    // externally set value

	// data relevant to analog control types
	ioport_value                m_min;              // minimum value for absolute axes
	ioport_value                m_max;              // maximum value for absolute axes
	s32                         m_sensitivity;      // sensitivity (100=normal)
	s32                         m_delta;            // delta to apply each frame a digital inc/dec key is pressed
	s32                         m_centerdelta;      // delta to apply each frame no digital inputs are pressed
	crosshair_axis_t            m_crosshair_axis;   // crosshair axis
	float                       m_crosshair_scale;  // crosshair scale
	float                       m_crosshair_offset; // crosshair offset
	float                       m_crosshair_altaxis;// crosshair alternate axis value
	ioport_field_crossmap_delegate m_crosshair_mapper; // crosshair mapping function
	u16                         m_full_turn_count;  // number of optical counts for 1 full turn of the original control
	const ioport_value *        m_remap_table;      // pointer to an array that remaps the port value

	// data relevant to other specific types
	u8                          m_way;              // digital joystick 2/4/8-way descriptions
	char32_t                    m_chars[1 << (UCHAR_SHIFT_END - UCHAR_SHIFT_BEGIN + 1)][2];      // unicode key data
};


// ======================> ioport_field_live

// internal live state of an input field
struct ioport_field_live
{
	// construction/destruction
	ioport_field_live(ioport_field &field, analog_field *analog);

	// public state
	analog_field *          analog;             // pointer to live analog data if this is an analog field
	digital_joystick *      joystick;           // pointer to digital joystick information
	input_seq               seq[SEQ_TYPE_TOTAL];// currently configured input sequences
	ioport_value            value;              // current value of this port
	u8                      impulse;            // counter for impulse controls
	bool                    last;               // were we pressed last time?
	bool                    toggle;             // current toggle setting
	digital_joystick::direction_t joydir;       // digital joystick direction index
	bool                    lockout;            // user lockout
	std::string             name;               // overridden name
	std::string             cfg[SEQ_TYPE_TOTAL];// configuration strings
};


// ======================> ioport_list

// class that holds a list of I/O ports
class ioport_list : public std::map<std::string, std::unique_ptr<ioport_port>>
{
	DISABLE_COPYING(ioport_list);

public:
	ioport_list() { }

	void append(device_t &device, std::string &errorbuf);
};


// ======================> ioport_port

// a single input port configuration
class ioport_port
{
	DISABLE_COPYING(ioport_port);
	friend class simple_list<ioport_port>;
	friend class ioport_configurer;

public:
	// construction/destruction
	ioport_port(device_t &owner, const char *tag);
	~ioport_port();

	// getters
	ioport_port *next() const { return m_next; }
	ioport_manager &manager() const;
	device_t &device() const { return m_device; }
	running_machine &machine() const;
	const simple_list<ioport_field> &fields() const { return m_fieldlist; }
	const char *tag() const { return m_tag.c_str(); }
	int modcount() const { return m_modcount; }
	ioport_value active() const { return m_active; }
	ioport_port_live &live() const { assert(m_live != nullptr); return *m_live; }

	// read/write to the port
	ioport_value read();
	void write(ioport_value value, ioport_value mask = ~0);

	// other operations
	ioport_field *field(ioport_value mask) const;
	void collapse_fields(std::string &errorbuf);
	void frame_update();
	void init_live_state();
	void update_defvalue(bool flush_defaults);

private:
	void insert_field(ioport_field &newfield, ioport_value &disallowedbits, std::string &errorbuf);

	// internal state
	ioport_port *               m_next;         // pointer to next port
	device_t &                  m_device;       // associated device
	simple_list<ioport_field>   m_fieldlist;    // list of ioport_fields
	std::string                 m_tag;          // copy of this port's tag
	int                         m_modcount;     // modification count
	ioport_value                m_active;       // mask of active bits in the port
	std::unique_ptr<ioport_port_live> m_live;      // live state of port (nullptr if not live)
};



// ======================> analog_field

// live analog field information
class analog_field
{
	friend class ioport_manager;
	friend void ioport_field::set_user_settings(const ioport_field::user_settings &settings);

public:
	// construction/destruction
	analog_field(ioport_field &field);

	// getters
	ioport_manager &manager() const noexcept { return m_field.manager(); }
	ioport_field &field() const noexcept { return m_field; }
	s32 sensitivity() const noexcept { return m_sensitivity; }
	bool reverse() const noexcept { return m_reverse; }
	s32 delta() const noexcept { return m_delta; }
	s32 centerdelta() const noexcept { return m_centerdelta; }

	// readers
	void read(ioport_value &value);
	float crosshair_read();
	void frame_update(running_machine &machine);

	// programmatic override (for script bindings)
	void set_value(s32 value);
	void clear_value();

private:
	// helpers
	s32 apply_min_max(s32 value) const;
	s32 apply_settings(s32 value) const;
	s32 apply_sensitivity(s32 value) const;
	s32 apply_inverse_sensitivity(s32 value) const;

	// internal state
	ioport_field &      m_field;                // pointer to the input field referenced

	// adjusted values (right-justified and tweaked)
	u8 const            m_shift;                // shift to align final value in the port
	s32 const           m_adjdefvalue;          // adjusted default value from the config
	s32                 m_adjmin;               // adjusted minimum value from the config
	s32                 m_adjmax;               // adjusted maximum value from the config
	s32                 m_adjoverride;          // programmatically set adjusted value

	// live values of configurable parameters
	s32                 m_sensitivity;          // current live sensitivity (100=normal)
	bool                m_reverse;              // current live reverse flag
	s32                 m_delta;                // current live delta to apply each frame a digital inc/dec key is pressed
	s32                 m_centerdelta;          // current live delta to apply each frame no digital inputs are pressed

	// live analog value tracking
	s32                 m_accum;                // accumulated value (including relative adjustments)
	s32                 m_previous;             // previous adjusted value
	s32                 m_previousanalog;       // previous analog value

	// parameters for modifying live values
	s32                 m_minimum;              // minimum adjusted value
	s32                 m_maximum;              // maximum adjusted value
	s32                 m_center;               // center adjusted value for autocentering
	s32                 m_reverse_val;          // value where we subtract from to reverse directions

	// scaling factors
	s64                 m_scalepos;             // scale factor to apply to positive adjusted values
	s64                 m_scaleneg;             // scale factor to apply to negative adjusted values
	s64                 m_keyscalepos;          // scale factor to apply to the key delta field when pos
	s64                 m_keyscaleneg;          // scale factor to apply to the key delta field when neg
	s64                 m_positionalscale;      // scale factor to divide a joystick into positions

	// misc flags
	bool                m_absolute;             // is this an absolute or relative input?
	bool                m_wraps;                // does the control wrap around?
	bool                m_autocenter;           // autocenter this input?
	bool                m_single_scale;         // scale joystick differently if default is between min/max
	bool                m_interpolate;          // should we do linear interpolation for mid-frame reads?
	bool                m_lastdigital;          // was the last modification caused by a digital form?
	bool                m_use_adjoverride;      // override what will be read from the field
};


// ======================> dynamic_field

// live device field information
class dynamic_field
{
public:
	// construction/destruction
	dynamic_field(ioport_field &field);

	// getters
	ioport_field &field() const { return m_field; }

	// read/write
	void read(ioport_value &result);
	void write(ioport_value newval);

private:
	// internal state
	ioport_field &          m_field;            // reference to the input field
	u8                      m_shift;            // shift to apply to the final result
	ioport_value            m_oldval;           // last value
};


// ======================> ioport_port_live

// internal live state of an input port
struct ioport_port_live
{
	// construction/destruction
	ioport_port_live(ioport_port &port);

	// public state
	std::list<analog_field> analoglist;         // list of analog port info
	std::vector<dynamic_field> readlist;        // list of dynamic read fields
	std::vector<dynamic_field> writelist;       // list of dynamic write fields
	ioport_value            defvalue;           // combined default value across the port
	ioport_value            digital;            // current value from all digital inputs
	ioport_value            outputvalue;        // current value for outputs
};


// ======================> ioport_manager

// private input port state
class ioport_manager
{
	DISABLE_COPYING(ioport_manager);
	friend class device_t;
	friend class ioport_configurer;

public:
	// construction/destruction
	ioport_manager(running_machine &machine);
	time_t initialize();
	~ioport_manager();

	// getters
	running_machine &machine() const noexcept { return m_machine; }
	const ioport_list &ports() const noexcept { return m_portlist; }
	bool safe_to_read() const noexcept { return m_safe_to_read; }

	// type helpers
	const std::vector<input_type_entry> &types() const noexcept { return m_typelist; }
	bool type_pressed(ioport_type type, int player = 0);
	std::string type_name(ioport_type type, u8 player) const;
	ioport_group type_group(ioport_type type, int player) const noexcept;
	const input_seq &type_seq(ioport_type type, int player = 0, input_seq_type seqtype = SEQ_TYPE_STANDARD) const noexcept;
	void set_type_seq(ioport_type type, int player, input_seq_type seqtype, const input_seq &newseq) noexcept;
	static constexpr bool type_is_analog(ioport_type type) noexcept { return (type > IPT_ANALOG_FIRST && type < IPT_ANALOG_LAST); }
	bool type_class_present(ioport_type_class inputclass) const noexcept;

	// other helpers
	digital_joystick &digjoystick(int player, int joysticknum);
	int count_players() const noexcept;
	s32 frame_interpolate(s32 oldval, s32 newval);
	ioport_type token_to_input_type(const char *string, int &player) const;
	std::string input_type_to_token(ioport_type type, int player);

private:
	// internal helpers
	void init_port_types();
	void init_autoselect_devices(std::initializer_list<ioport_type> types, std::string_view option, std::string_view ananame);

	void frame_update_callback();
	void frame_update();

	ioport_port *port(const std::string &tag) const { auto search = m_portlist.find(tag); if (search != m_portlist.end()) return search->second.get(); else return nullptr; }
	void exit();
	input_seq_type token_to_seq_type(const char *string);
	static const char *const seqtypestrings[];

	void load_config(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void load_remap_table(util::xml::data_node const &parentnode);
	bool load_default_config(int type, int player, const std::pair<input_seq, char const *> (&newseq)[SEQ_TYPE_TOTAL]);
	bool load_controller_config(util::xml::data_node const &portnode, int type, int player, const std::pair<input_seq, char const *> (&newseq)[SEQ_TYPE_TOTAL]);
	void load_system_config(util::xml::data_node const &portnode, int type, int player, const std::pair<input_seq, char const *> (&newseq)[SEQ_TYPE_TOTAL]);
	void apply_device_defaults();

	void save_config(config_type cfg_type, util::xml::data_node *parentnode);
	bool save_this_input_field_type(ioport_type type);
	void save_default_inputs(util::xml::data_node &parentnode);
	void save_game_inputs(util::xml::data_node &parentnode);

	template<typename Type> Type playback_read(Type &result);
	time_t playback_init();
	void playback_end(const char *message = nullptr);
	void playback_frame(const attotime &curtime);
	void playback_port(ioport_port &port);

	template<typename Type> void record_write(Type value);
	void record_init();
	void record_end(const char *message = nullptr);
	void record_frame(const attotime &curtime);
	void record_port(ioport_port &port);

	// internal state
	running_machine &       m_machine;              // reference to owning machine
	bool                    m_safe_to_read;         // clear at start; set after state is loaded
	ioport_list             m_portlist;             // list of input port configurations

	// types
	std::vector<input_type_entry> m_typelist;       // list of live type states
	input_type_entry *      m_type_to_entry[IPT_COUNT][MAX_PLAYERS]; // map from type/player to type state

	// specific special global input states
	std::list<digital_joystick> m_joystick_list;    // list of digital joysticks

	// frame time tracking
	attotime                m_last_frame_time;      // time of the last frame callback
	attoseconds_t           m_last_delta_nsec;      // nanoseconds that passed since the previous callback

	// playback/record information
	std::unique_ptr<emu_file> m_record_file;        // recording file (nullptr if not recording)
	std::unique_ptr<emu_file> m_playback_file;      // playback file (nullptr if not recording)
	util::write_stream::ptr m_record_stream;        // recording stream (nullptr if not recording)
	util::read_stream::ptr  m_playback_stream;      // playback stream (nullptr if not recording)
	u64                     m_playback_accumulated_speed; // accumulated speed during playback
	u32                     m_playback_accumulated_frames; // accumulated frames during playback

	// storage for inactive configuration
	std::unique_ptr<util::xml::file> m_deselected_card_config;
	bool m_applied_device_defaults;
};


// ======================> ioport_configurer

// class to wrap helper functions
class ioport_configurer
{
public:
	// construction/destruction
	ioport_configurer(device_t &owner, ioport_list &portlist, std::string &errorbuf);

	// static helpers
	static const char *string_from_token(const char *string);

	// port helpers
	ioport_configurer& port_alloc(const char *tag);
	ioport_configurer& port_modify(const char *tag);

	// field helpers
	ioport_configurer& field_alloc(ioport_type type, ioport_value defval, ioport_value mask, const char *name = nullptr);
	ioport_configurer& field_add_char(std::initializer_list<char32_t> charlist);
	ioport_configurer& field_add_code(input_seq_type which, input_code code);
	ioport_configurer& field_set_way(int way) { m_curfield->m_way = way; return *this; }
	ioport_configurer& field_set_rotated() { m_curfield->m_flags |= ioport_field::FIELD_FLAG_ROTATED; return *this; }
	ioport_configurer& field_set_name(const char *name) { assert(m_curfield != nullptr); m_curfield->m_name = string_from_token(name); return *this; }
	ioport_configurer& field_set_player(int player) { m_curfield->m_player = player - 1; return *this; }
	ioport_configurer& field_set_cocktail() { m_curfield->m_flags |= ioport_field::FIELD_FLAG_COCKTAIL; field_set_player(2); return *this; }
	ioport_configurer& field_set_toggle() { m_curfield->m_flags |= ioport_field::FIELD_FLAG_TOGGLE; return *this; }
	ioport_configurer& field_set_impulse(u8 impulse) { m_curfield->m_impulse = impulse; return *this; }
	ioport_configurer& field_set_analog_reverse() { m_curfield->m_flags |= ioport_field::ANALOG_FLAG_REVERSE; return *this; }
	[[deprecated("PORT_RESET is deprecated; manage counter state explicitly")]]
	ioport_configurer& field_set_analog_reset() { m_curfield->m_flags |= ioport_field::ANALOG_FLAG_RESET; return *this; }
	ioport_configurer& field_set_optional() { m_curfield->m_flags |= ioport_field::FIELD_FLAG_OPTIONAL; return *this; }
	ioport_configurer& field_set_min_max(ioport_value minval, ioport_value maxval) { m_curfield->m_min = minval; m_curfield->m_max = maxval; return *this; }
	ioport_configurer& field_set_sensitivity(s32 sensitivity) { m_curfield->m_sensitivity = sensitivity; return *this; }
	ioport_configurer& field_set_delta(s32 delta) { m_curfield->m_centerdelta = m_curfield->m_delta = delta; return *this; }
	ioport_configurer& field_set_centerdelta(s32 delta) { m_curfield->m_centerdelta = delta; return *this; }
	ioport_configurer& field_set_crosshair(crosshair_axis_t axis, double altaxis, double scale, double offset) { m_curfield->m_crosshair_axis = axis; m_curfield->m_crosshair_altaxis = altaxis; m_curfield->m_crosshair_scale = scale; m_curfield->m_crosshair_offset = offset; return *this; }
	ioport_configurer& field_set_crossmapper(ioport_field_crossmap_delegate callback) { m_curfield->m_crosshair_mapper = callback; return *this; }
	ioport_configurer& field_set_full_turn_count(u16 count) { m_curfield->m_full_turn_count = count; return *this; }
	ioport_configurer& field_set_analog_wraps() { m_curfield->m_flags |= ioport_field::ANALOG_FLAG_WRAPS; return *this; }
	ioport_configurer& field_set_remap_table(const ioport_value *table) { m_curfield->m_remap_table = table; return *this; }
	ioport_configurer& field_set_analog_invert() { m_curfield->m_flags |= ioport_field::ANALOG_FLAG_INVERT; return *this; }
	ioport_configurer& field_set_dynamic_read(ioport_field_read_delegate delegate) { m_curfield->m_read = delegate; return *this; }
	ioport_configurer& field_set_dynamic_write(ioport_field_write_delegate delegate, u32 param = 0) { m_curfield->m_write = delegate; m_curfield->m_write_param = param; return *this; }
	ioport_configurer& field_set_diplocation(const char *location) { m_curfield->expand_diplocation(location, m_errorbuf); return *this; }

	// setting helpers
	ioport_configurer& setting_alloc(ioport_value value, const char *name);

	// misc helpers
	ioport_configurer& set_condition(ioport_condition::condition_t condition, const char *tag, ioport_value mask, ioport_value value);
	ioport_configurer& onoff_alloc(const char *name, ioport_value defval, ioport_value mask, const char *diplocation);

private:
	// internal state
	device_t &          m_owner;
	ioport_list &       m_portlist;
	std::string &       m_errorbuf;

	ioport_port *       m_curport;
	ioport_field *      m_curfield;
	ioport_setting *    m_cursetting;
};



//**************************************************************************
//  MACROS
//**************************************************************************

#define UCHAR_MAMEKEY(code) (UCHAR_MAMEKEY_BEGIN + ITEM_ID_##code)

// macro for port write callback functions (PORT_CHANGED)
#define INPUT_CHANGED_MEMBER(name)  void name(ioport_field &field, u32 param, ioport_value oldval, ioport_value newval)
#define DECLARE_INPUT_CHANGED_MEMBER(name)  void name(ioport_field &field, u32 param, ioport_value oldval, ioport_value newval)

// macro for port changed callback functions (PORT_CROSSHAIR_MAPPER)
#define CROSSHAIR_MAPPER_MEMBER(name)   float name(float linear_value)
#define DECLARE_CROSSHAIR_MAPPER_MEMBER(name)   float name(float linear_value)

// macro for wrapping a default string
#define DEF_STR(str_num) ((const char *)INPUT_STRING_##str_num)



//**************************************************************************
//  MACROS FOR BUILDING INPUT PORTS
//**************************************************************************

// so that "0" can be used for unneeded input ports
#define construct_ioport_0 nullptr

// name of table
#define INPUT_PORTS_NAME(_name) construct_ioport_##_name

// start of table
#define INPUT_PORTS_START(_name) \
ATTR_COLD void INPUT_PORTS_NAME(_name)(device_t &owner, ioport_list &portlist, std::string &errorbuf) \
{ \
	ioport_configurer configurer(owner, portlist, errorbuf);
// end of table
#define INPUT_PORTS_END \
}

// aliasing
#define INPUT_PORTS_EXTERN(_name) \
	extern void INPUT_PORTS_NAME(_name)(device_t &owner, ioport_list &portlist, std::string &errorbuf)

// including
#define PORT_INCLUDE(_name) \
	INPUT_PORTS_NAME(_name)(owner, portlist, errorbuf);
// start of a new input port (with included tag)
#define PORT_START(_tag) \
	configurer.port_alloc(_tag);
// modify an existing port
#define PORT_MODIFY(_tag) \
	configurer.port_modify(_tag);
// input bit definition
#define PORT_BIT(_mask, _default, _type) \
	configurer.field_alloc((_type), (_default), (_mask));
#define PORT_SPECIAL_ONOFF(_mask, _default, _strindex) \
	PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, _strindex, nullptr)

#define PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, _strindex, _diploc) \
	configurer.onoff_alloc(DEF_STR(_strindex), _default, _mask, _diploc);
// append a code
#define PORT_CODE(_code) \
	configurer.field_add_code(SEQ_TYPE_STANDARD, _code);

#define PORT_CODE_DEC(_code) \
	configurer.field_add_code(SEQ_TYPE_DECREMENT, _code);

#define PORT_CODE_INC(_code) \
	configurer.field_add_code(SEQ_TYPE_INCREMENT, _code);

// joystick flags
#define PORT_2WAY \
	configurer.field_set_way(2);

#define PORT_4WAY \
	configurer.field_set_way(4);

#define PORT_8WAY \
	configurer.field_set_way(8);

#define PORT_16WAY \
	configurer.field_set_way(16);

#define PORT_ROTATED \
	configurer.field_set_rotated();

// general flags
#define PORT_NAME(_name) \
	configurer.field_set_name(_name);

#define PORT_PLAYER(_player) \
	configurer.field_set_player(_player);

#define PORT_COCKTAIL \
	configurer.field_set_cocktail();

#define PORT_TOGGLE \
	configurer.field_set_toggle();

#define PORT_IMPULSE(_duration) \
	configurer.field_set_impulse(_duration);

#define PORT_REVERSE \
	configurer.field_set_analog_reverse();

#define PORT_RESET \
	configurer.field_set_analog_reset();

#define PORT_OPTIONAL \
	configurer.field_set_optional();

// analog settings
// if this macro is not used, the minimum defaults to 0 and maximum defaults to the mask value
#define PORT_MINMAX(_min, _max) \
	configurer.field_set_min_max(_min, _max);

#define PORT_SENSITIVITY(_sensitivity) \
	configurer.field_set_sensitivity(_sensitivity);

#define PORT_KEYDELTA(_delta) \
	configurer.field_set_delta(_delta);
// note that PORT_CENTERDELTA must appear after PORT_KEYDELTA
#define PORT_CENTERDELTA(_delta) \
	configurer.field_set_centerdelta(_delta);

#define PORT_CROSSHAIR(axis, scale, offset, altaxis) \
	configurer.field_set_crosshair(CROSSHAIR_AXIS_##axis, altaxis, scale, offset);

#define PORT_CROSSHAIR_MAPPER(_callback) \
	configurer.field_set_crossmapper(ioport_field_crossmap_delegate(owner, DEVICE_SELF, _callback, #_callback));

#define PORT_CROSSHAIR_MAPPER_MEMBER(_device, _class, _member) \
	configurer.field_set_crossmapper(ioport_field_crossmap_delegate(owner, _device, &_class::_member, #_class "::" #_member));

// how many optical counts for 1 full turn of the control
#define PORT_FULL_TURN_COUNT(_count) \
	configurer.field_set_full_turn_count(_count);

// positional controls can be binary or 1 of X
// 1 of X not completed yet
// if it is specified as PORT_REMAP_TABLE then it is binary, but remapped
// otherwise it is binary
#define PORT_POSITIONS(_positions) \
	configurer.field_set_min_max(0, _positions);

// positional control wraps at min/max
#define PORT_WRAPS \
	configurer.field_set_analog_wraps();

// positional control uses this remap table
#define PORT_REMAP_TABLE(_table) \
	configurer.field_set_remap_table(_table);

// positional control bits are active low
#define PORT_INVERT \
	configurer.field_set_analog_invert();

// read callbacks
#define PORT_CUSTOM_MEMBER(_class, _member) \
	configurer.field_set_dynamic_read(ioport_field_read_delegate(owner, DEVICE_SELF, &_class::_member, #_class "::" #_member));
#define PORT_CUSTOM_DEVICE_MEMBER(_device, _class, _member) \
	configurer.field_set_dynamic_read(ioport_field_read_delegate(owner, _device, &_class::_member, #_class "::" #_member));

// write callbacks
#define PORT_CHANGED_MEMBER(_device, _class, _member, _param) \
	configurer.field_set_dynamic_write(ioport_field_write_delegate(owner, _device, &_class::_member, #_class "::" #_member), (_param));

// input device handler
#define PORT_READ_LINE_MEMBER(_class, _member) \
	configurer.field_set_dynamic_read( \
			ioport_field_read_delegate( \
				owner, \
				DEVICE_SELF, \
				static_cast<ioport_value (*)(_class &)>([] (_class &device) -> ioport_value { return (device._member() & 1) ? ~ioport_value(0) : 0; }), \
				#_class "::" #_member));
#define PORT_READ_LINE_DEVICE_MEMBER(_device, _class, _member) \
	configurer.field_set_dynamic_read( \
			ioport_field_read_delegate( \
				owner, \
				_device, \
				static_cast<ioport_value (*)(_class &)>([] (_class &device) -> ioport_value { return (device._member() & 1) ? ~ioport_value(0) : 0; }), \
				#_class "::" #_member));

// output device handler
#define PORT_WRITE_LINE_MEMBER(_class, _member) \
	configurer.field_set_dynamic_write( \
			ioport_field_write_delegate( \
				owner, \
				DEVICE_SELF, \
				static_cast<void (*)(_class &, ioport_field &, u32, ioport_value, ioport_value)>([] (_class &device, ioport_field &field, u32 param, ioport_value oldval, ioport_value newval) { device._member(newval); }), \
				#_class "::" #_member));
#define PORT_WRITE_LINE_DEVICE_MEMBER(_device, _class, _member) \
	configurer.field_set_dynamic_write( \
			ioport_field_write_delegate( \
				owner, \
				_device, \
				static_cast<void (*)(_class &, ioport_field &, u32, ioport_value, ioport_value)>([] (_class &device, ioport_field &field, u32 param, ioport_value oldval, ioport_value newval) { device._member(newval); }), \
				#_class "::" #_member));

// dip switch definition
#define PORT_DIPNAME(_mask, _default, _name) \
	configurer.field_alloc(IPT_DIPSWITCH, (_default), (_mask), (_name));
#define PORT_DIPSETTING(_default, _name) \
	configurer.setting_alloc((_default), (_name));
// physical location, of the form: name:[!]sw,[name:][!]sw,...
// note that these are specified LSB-first
#define PORT_DIPLOCATION(_location) \
	configurer.field_set_diplocation(_location);
// conditionals for dip switch settings
#define PORT_CONDITION(_tag, _mask, _condition, _value) \
	configurer.set_condition(ioport_condition::_condition, _tag, _mask, _value);
// analog adjuster definition
#define PORT_ADJUSTER(_default, _name) \
	configurer.field_alloc(IPT_ADJUSTER, (_default), 0xff, (_name)); \
	configurer.field_set_min_max(0, 100);
// config definition
#define PORT_CONFNAME(_mask, _default, _name) \
	configurer.field_alloc(IPT_CONFIG, (_default), (_mask), (_name));
#define PORT_CONFSETTING(_default, _name) \
	configurer.setting_alloc((_default), (_name));

// keyboard chars
#define PORT_CHAR(...) \
	configurer.field_add_char({ __VA_ARGS__ });


// name of table
#define DEVICE_INPUT_DEFAULTS_NAME(_name) device_iptdef_##_name

#define device_iptdef_NOOP nullptr

// start of table
#define DEVICE_INPUT_DEFAULTS_START(_name) \
	const input_device_default DEVICE_INPUT_DEFAULTS_NAME(_name)[] = {
// end of table
#define DEVICE_INPUT_DEFAULTS(_tag,_mask,_defval) \
	{ _tag ,_mask, _defval },
// end of table
#define DEVICE_INPUT_DEFAULTS_END \
	{nullptr,0,0} };



//**************************************************************************
//  HELPER MACROS
//**************************************************************************

#define PORT_DIPUNUSED_DIPLOC(_mask, _default, _diploc) \
	PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, Unused, _diploc)

#define PORT_DIPUNUSED(_mask, _default) \
	PORT_SPECIAL_ONOFF(_mask, _default, Unused)

#define PORT_DIPUNKNOWN_DIPLOC(_mask, _default, _diploc) \
	PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, Unknown, _diploc)

#define PORT_DIPUNKNOWN(_mask, _default) \
	PORT_SPECIAL_ONOFF(_mask, _default, Unknown)

#define PORT_SERVICE_DIPLOC(_mask, _default, _diploc) \
	PORT_SPECIAL_ONOFF_DIPLOC(_mask, _default, Service_Mode, _diploc)

#define PORT_SERVICE(_mask, _default) \
	PORT_SPECIAL_ONOFF(_mask, _default, Service_Mode)

#define PORT_SERVICE_NO_TOGGLE(_mask, _default) \
	PORT_BIT( _mask, _mask & _default, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode ))

#define PORT_VBLANK(_screen) \
	PORT_READ_LINE_DEVICE_MEMBER(_screen, screen_device, vblank)

#define PORT_HBLANK(_screen) \
	PORT_READ_LINE_DEVICE_MEMBER(_screen, screen_device, hblank)

//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline ioport_manager &ioport_field::manager() const { return m_port.manager(); }
inline device_t &ioport_field::device() const { return m_port.device(); }
inline running_machine &ioport_field::machine() const { return m_port.machine(); }

inline device_t &ioport_setting::device() const { return m_field.device(); }
inline running_machine &ioport_setting::machine() const { return m_field.machine(); }


#endif // MAME_EMU_IOPORT_H
