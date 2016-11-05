// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inpttype.h

    Array of core-defined input types and default mappings.

***************************************************************************/


/***************************************************************************
    BUILT-IN CORE MAPPINGS
***************************************************************************/

#define INPUT_PORT_DIGITAL_TYPE(_player,_group,_type,_name) \
	typelist.append(*global_alloc(input_type_entry(IPT_##_type, IPG_##_group, (_player == 0) ? _player : (_player) - 1, (_player == 0) ? #_type : ("P" #_player "_" #_type), _name, input_seq())));

#define INPUT_PORT_ANALOG_TYPE(_player,_group,_type,_name) \
	typelist.append(*global_alloc(input_type_entry(IPT_##_type, IPG_##_group, (_player == 0) ? _player : (_player) - 1, (_player == 0) ? #_type : ("P" #_player "_" #_type), _name, input_seq(), input_seq(), input_seq())));

/* These input port macros expand to a great deal of code and break compilers */
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 4 || (__GNUC_MINOR__ == 4 && __GNUC_PATCHLEVEL__ >= 4))))
#if not(defined(__arm__) || defined(__ARMEL__))
#pragma GCC push_options
#pragma GCC optimize ("O1")
#endif
#elif defined(_MSC_VER)
#pragma optimize("", off)
#endif

/* split up into small functions to be nicer on optimizers */

inline void construct_core_types_P1(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICK_UP,         "P1 Up" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICK_DOWN,       "P1 Down" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICK_LEFT,       "P1 Left" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICK_RIGHT,      "P1 Right" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKRIGHT_UP,    "P1 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKRIGHT_DOWN,  "P1 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKRIGHT_LEFT,  "P1 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKRIGHT_RIGHT, "P1 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKLEFT_UP,     "P1 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKLEFT_DOWN,   "P1 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKLEFT_LEFT,   "P1 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKLEFT_RIGHT,  "P1 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON1,             "P1 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON2,             "P1 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON3,             "P1 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON4,             "P1 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON5,             "P1 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON6,             "P1 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON7,             "P1 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON8,             "P1 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON9,             "P1 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON10,            "P1 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON11,            "P1 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON12,            "P1 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON13,            "P1 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON14,            "P1 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON15,            "P1 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON16,            "P1 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, START,               "P1 Start" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SELECT,              "P1 Select" )
}

inline void construct_core_types_P1_mahjong(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_A,           "P1 Mahjong A" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_B,           "P1 Mahjong B" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_C,           "P1 Mahjong C" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_D,           "P1 Mahjong D" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_E,           "P1 Mahjong E" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_F,           "P1 Mahjong F" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_G,           "P1 Mahjong G" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_H,           "P1 Mahjong H" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_I,           "P1 Mahjong I" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_J,           "P1 Mahjong J" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_K,           "P1 Mahjong K" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_L,           "P1 Mahjong L" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_M,           "P1 Mahjong M" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_N,           "P1 Mahjong N" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_O,           "P1 Mahjong O" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_P,           "P1 Mahjong P" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_Q,           "P1 Mahjong Q" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_KAN,         "P1 Mahjong Kan" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_PON,         "P1 Mahjong Pon" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_CHI,         "P1 Mahjong Chi" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_REACH,       "P1 Mahjong Reach" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_RON,         "P1 Mahjong Ron" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_BET,         "P1 Mahjong Bet" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_LAST_CHANCE, "P1 Mahjong Last Chance" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_SCORE,       "P1 Mahjong Score" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_DOUBLE_UP,   "P1 Mahjong Double Up" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_FLIP_FLOP,   "P1 Mahjong Flip Flop" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_BIG,         "P1 Mahjong Big" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_SMALL,       "P1 Mahjong Small" )
}

inline void construct_core_types_P1_hanafuda(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_A,          "P1 Hanafuda A/1" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_B,          "P1 Hanafuda B/2" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_C,          "P1 Hanafuda C/3" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_D,          "P1 Hanafuda D/4" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_E,          "P1 Hanafuda E/5" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_F,          "P1 Hanafuda F/6" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_G,          "P1 Hanafuda G/7" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_H,          "P1 Hanafuda H/8" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_YES,        "P1 Hanafuda Yes" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_NO,         "P1 Hanafuda No" )
}

inline void construct_core_types_gamble(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_HIGH,         "High" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_LOW,          "Low" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_HALF,         "Half Gamble" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_DEAL,         "Deal" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_D_UP,         "Double Up" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_TAKE,         "Take" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_STAND,        "Stand" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_BET,          "Bet" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_KEYIN,        "Key In" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_KEYOUT,       "Key Out" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_PAYOUT,       "Payout" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_DOOR,         "Door" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_SERVICE,      "Service" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_BOOK,         "Book-Keeping" )
}

inline void construct_core_types_poker(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD1,         "Hold 1" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD2,         "Hold 2" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD3,         "Hold 3" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD4,         "Hold 4" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD5,         "Hold 5" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_CANCEL,        "Cancel" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_BET,           "Bet" )
}

inline void construct_core_types_slot(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP1,          "Stop Reel 1" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP2,          "Stop Reel 2" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP3,          "Stop Reel 3" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP4,          "Stop Reel 4" )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP_ALL,       "Stop All Reels" )
}

inline void construct_core_types_P2(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICK_UP,         "P2 Up" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICK_DOWN,       "P2 Down" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICK_LEFT,       "P2 Left" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICK_RIGHT,      "P2 Right" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKRIGHT_UP,    "P2 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKRIGHT_DOWN,  "P2 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKRIGHT_LEFT,  "P2 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKRIGHT_RIGHT, "P2 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKLEFT_UP,     "P2 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKLEFT_DOWN,   "P2 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKLEFT_LEFT,   "P2 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKLEFT_RIGHT,  "P2 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON1,             "P2 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON2,             "P2 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON3,             "P2 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON4,             "P2 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON5,             "P2 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON6,             "P2 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON7,             "P2 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON8,             "P2 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON9,             "P2 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON10,            "P2 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON11,            "P2 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON12,            "P2 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON13,            "P2 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON14,            "P2 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON15,            "P2 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON16,            "P2 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, START,               "P2 Start" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, SELECT,              "P2 Select" )
}

inline void construct_core_types_P2_mahjong(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_A,           "P2 Mahjong A" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_B,           "P2 Mahjong B" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_C,           "P2 Mahjong C" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_D,           "P2 Mahjong D" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_E,           "P2 Mahjong E" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_F,           "P2 Mahjong F" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_G,           "P2 Mahjong G" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_H,           "P2 Mahjong H" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_I,           "P2 Mahjong I" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_J,           "P2 Mahjong J" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_K,           "P2 Mahjong K" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_L,           "P2 Mahjong L" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_M,           "P2 Mahjong M" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_N,           "P2 Mahjong N" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_O,           "P2 Mahjong O" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_P,           "P2 Mahjong P" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_Q,           "P2 Mahjong Q" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_KAN,         "P2 Mahjong Kan" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_PON,         "P2 Mahjong Pon" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_CHI,         "P2 Mahjong Chi" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_REACH,       "P2 Mahjong Reach" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_RON,         "P2 Mahjong Ron" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_BET,         "P2 Mahjong Bet" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_LAST_CHANCE, "P2 Mahjong Last Chance" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_SCORE,       "P2 Mahjong Score" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_DOUBLE_UP,   "P2 Mahjong Double Up" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_FLIP_FLOP,   "P2 Mahjong Flip Flop" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_BIG,         "P2 Mahjong Big" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_SMALL,       "P2 Mahjong Small" )
}

inline void construct_core_types_P2_hanafuda(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_A,          "P2 Hanafuda A/1" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_B,          "P2 Hanafuda B/2" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_C,          "P2 Hanafuda C/3" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_D,          "P2 Hanafuda D/4" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_E,          "P2 Hanafuda E/5" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_F,          "P2 Hanafuda F/6" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_G,          "P2 Hanafuda G/7" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_H,          "P2 Hanafuda H/8" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_YES,        "P2 Hanafuda Yes" )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_NO,         "P2 Hanafuda No" )
}

inline void construct_core_types_P3(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICK_UP,         "P3 Up" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICK_DOWN,       "P3 Down" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICK_LEFT,       "P3 Left" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICK_RIGHT,      "P3 Right" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKRIGHT_UP,    "P3 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKRIGHT_DOWN,  "P3 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKRIGHT_LEFT,  "P3 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKRIGHT_RIGHT, "P3 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKLEFT_UP,     "P3 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKLEFT_DOWN,   "P3 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKLEFT_LEFT,   "P3 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKLEFT_RIGHT,  "P3 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON1,             "P3 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON2,             "P3 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON3,             "P3 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON4,             "P3 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON5,             "P3 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON6,             "P3 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON7,             "P3 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON8,             "P3 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON9,             "P3 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON10,            "P3 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON11,            "P3 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON12,            "P3 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON13,            "P3 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON14,            "P3 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON15,            "P3 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON16,            "P3 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, START,               "P3 Start" )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, SELECT,              "P3 Select" )
}

inline void construct_core_types_P4(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICK_UP,         "P4 Up" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICK_DOWN,       "P4 Down" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICK_LEFT,       "P4 Left" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICK_RIGHT,      "P4 Right" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKRIGHT_UP,    "P4 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKRIGHT_DOWN,  "P4 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKRIGHT_LEFT,  "P4 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKRIGHT_RIGHT, "P4 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKLEFT_UP,     "P4 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKLEFT_DOWN,   "P4 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKLEFT_LEFT,   "P4 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKLEFT_RIGHT,  "P4 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON1,             "P4 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON2,             "P4 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON3,             "P4 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON4,             "P4 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON5,             "P4 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON6,             "P4 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON7,             "P4 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON8,             "P4 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON9,             "P4 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON10,            "P4 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON11,            "P4 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON12,            "P4 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON13,            "P4 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON14,            "P4 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON15,            "P4 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON16,            "P4 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, START,               "P4 Start" )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, SELECT,              "P4 Select" )
}

inline void construct_core_types_P5(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICK_UP,         "P5 Up" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICK_DOWN,       "P5 Down" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICK_LEFT,       "P5 Left" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICK_RIGHT,      "P5 Right" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKRIGHT_UP,    "P5 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKRIGHT_DOWN,  "P5 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKRIGHT_LEFT,  "P5 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKRIGHT_RIGHT, "P5 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKLEFT_UP,     "P5 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKLEFT_DOWN,   "P5 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKLEFT_LEFT,   "P5 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKLEFT_RIGHT,  "P5 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON1,             "P5 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON2,             "P5 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON3,             "P5 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON4,             "P5 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON5,             "P5 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON6,             "P5 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON7,             "P5 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON8,             "P5 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON9,             "P5 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON10,            "P5 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON11,            "P5 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON12,            "P5 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON13,            "P5 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON14,            "P5 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON15,            "P5 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON16,            "P5 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, START,               "P5 Start" )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, SELECT,              "P5 Select" )
}

inline void construct_core_types_P6(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICK_UP,         "P6 Up" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICK_DOWN,       "P6 Down" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICK_LEFT,       "P6 Left" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICK_RIGHT,      "P6 Right" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKRIGHT_UP,    "P6 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKRIGHT_DOWN,  "P6 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKRIGHT_LEFT,  "P6 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKRIGHT_RIGHT, "P6 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKLEFT_UP,     "P6 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKLEFT_DOWN,   "P6 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKLEFT_LEFT,   "P6 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKLEFT_RIGHT,  "P6 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON1,             "P6 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON2,             "P6 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON3,             "P6 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON4,             "P6 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON5,             "P6 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON6,             "P6 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON7,             "P6 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON8,             "P6 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON9,             "P6 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON10,            "P6 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON11,            "P6 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON12,            "P6 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON13,            "P6 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON14,            "P6 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON15,            "P6 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON16,            "P6 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, START,               "P6 Start" )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, SELECT,              "P6 Select" )
}

inline void construct_core_types_P7(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICK_UP,         "P7 Up" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICK_DOWN,       "P7 Down" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICK_LEFT,       "P7 Left" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICK_RIGHT,      "P7 Right" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKRIGHT_UP,    "P7 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKRIGHT_DOWN,  "P7 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKRIGHT_LEFT,  "P7 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKRIGHT_RIGHT, "P7 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKLEFT_UP,     "P7 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKLEFT_DOWN,   "P7 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKLEFT_LEFT,   "P7 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKLEFT_RIGHT,  "P7 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON1,             "P7 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON2,             "P7 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON3,             "P7 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON4,             "P7 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON5,             "P7 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON6,             "P7 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON7,             "P7 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON8,             "P7 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON9,             "P7 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON10,            "P7 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON11,            "P7 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON12,            "P7 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON13,            "P7 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON14,            "P7 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON15,            "P7 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON16,            "P7 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, START,               "P7 Start" )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, SELECT,              "P7 Select" )
}

inline void construct_core_types_P8(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICK_UP,         "P8 Up" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICK_DOWN,       "P8 Down" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICK_LEFT,       "P8 Left" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICK_RIGHT,      "P8 Right" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKRIGHT_UP,    "P8 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKRIGHT_DOWN,  "P8 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKRIGHT_LEFT,  "P8 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKRIGHT_RIGHT, "P8 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKLEFT_UP,     "P8 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKLEFT_DOWN,   "P8 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKLEFT_LEFT,   "P8 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKLEFT_RIGHT,  "P8 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON1,             "P8 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON2,             "P8 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON3,             "P8 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON4,             "P8 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON5,             "P8 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON6,             "P8 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON7,             "P8 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON8,             "P8 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON9,             "P8 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON10,            "P8 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON11,            "P8 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON12,            "P8 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON13,            "P8 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON14,            "P8 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON15,            "P8 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON16,            "P8 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, START,               "P8 Start" )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, SELECT,              "P8 Select" )
}

inline void construct_core_types_P9(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICK_UP,         "P9 Up" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICK_DOWN,       "P9 Down" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICK_LEFT,       "P9 Left" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICK_RIGHT,      "P9 Right" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICKRIGHT_UP,    "P9 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICKRIGHT_DOWN,  "P9 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICKRIGHT_LEFT,  "P9 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICKRIGHT_RIGHT, "P9 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICKLEFT_UP,     "P9 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICKLEFT_DOWN,   "P9 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICKLEFT_LEFT,   "P9 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, JOYSTICKLEFT_RIGHT,  "P9 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON1,             "P9 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON2,             "P9 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON3,             "P9 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON4,             "P9 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON5,             "P9 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON6,             "P9 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON7,             "P9 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON8,             "P9 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON9,             "P9 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON10,            "P9 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON11,            "P9 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON12,            "P9 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON13,            "P9 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON14,            "P9 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON15,            "P9 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, BUTTON16,            "P9 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, START,               "P9 Start" )
	INPUT_PORT_DIGITAL_TYPE( 9, PLAYER9, SELECT,              "P9 Select" )
}

inline void construct_core_types_P10(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICK_UP,         "P10 Up" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICK_DOWN,       "P10 Down" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICK_LEFT,       "P10 Left" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICK_RIGHT,      "P10 Right" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKRIGHT_UP,    "P10 Right Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKRIGHT_DOWN,  "P10 Right Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKRIGHT_LEFT,  "P10 Right Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKRIGHT_RIGHT, "P10 Right Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKLEFT_UP,     "P10 Left Stick/Up" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKLEFT_DOWN,   "P10 Left Stick/Down" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKLEFT_LEFT,   "P10 Left Stick/Left" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKLEFT_RIGHT,  "P10 Left Stick/Right" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON1,             "P10 Button 1" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON2,             "P10 Button 2" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON3,             "P10 Button 3" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON4,             "P10 Button 4" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON5,             "P10 Button 5" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON6,             "P10 Button 6" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON7,             "P10 Button 7" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON8,             "P10 Button 8" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON9,             "P10 Button 9" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON10,            "P10 Button 10" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON11,            "P10 Button 11" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON12,            "P10 Button 12" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON13,            "P10 Button 13" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON14,            "P10 Button 14" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON15,            "P10 Button 15" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON16,            "P10 Button 16" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, START,               "P10 Start" )
	INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, SELECT,              "P10 Select" )
}

inline void construct_core_types_start(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START1,              "1 Player Start" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START2,              "2 Players Start" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START3,              "3 Players Start" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START4,              "4 Players Start" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START5,              "5 Players Start" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START6,              "6 Players Start" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START7,              "7 Players Start" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START8,              "8 Players Start" )
}

inline void construct_core_types_coin(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN1,               "Coin 1" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN2,               "Coin 2" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN3,               "Coin 3" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN4,               "Coin 4" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN5,               "Coin 5" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN6,               "Coin 6" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN7,               "Coin 7" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN8,               "Coin 8" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN9,               "Coin 9" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN10,              "Coin 10" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN11,              "Coin 11" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN12,              "Coin 12" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   BILL1,               "Bill 1" )
}

inline void construct_core_types_service(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE1,            "Service 1" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE2,            "Service 2" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE3,            "Service 3" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE4,            "Service 4" )
}

inline void construct_core_types_tilt(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT1,               "Tilt 1" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT2,               "Tilt 2" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT3,               "Tilt 3" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT4,               "Tilt 4" )
}

inline void construct_core_types_other(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   POWER_ON,            "Power On" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   POWER_OFF,           "Power Off" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE,             "Service" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT,                "Tilt" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   INTERLOCK,           "Door Interlock" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   MEMORY_RESET,        "Memory Reset" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   VOLUME_DOWN,         "Volume Down" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   VOLUME_UP,           "Volume Up" )
}

inline void construct_core_types_pedal(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PEDAL,               "P1 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PEDAL,               "P2 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PEDAL,               "P3 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PEDAL,               "P4 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PEDAL,               "P5 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PEDAL,               "P6 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PEDAL,               "P7 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PEDAL,               "P8 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, PEDAL,               "P9 Pedal 1" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, PEDAL,              "P10 Pedal 1" )
}

inline void construct_core_types_pedal2(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PEDAL2,              "P1 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PEDAL2,              "P2 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PEDAL2,              "P3 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PEDAL2,              "P4 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PEDAL2,              "P5 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PEDAL2,              "P6 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PEDAL2,              "P7 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PEDAL2,              "P8 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, PEDAL2,              "P9 Pedal 2" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, PEDAL2,             "P10 Pedal 2" )
}

inline void construct_core_types_pedal3(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PEDAL3,              "P1 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PEDAL3,              "P2 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PEDAL3,              "P3 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PEDAL3,              "P4 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PEDAL3,              "P5 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PEDAL3,              "P6 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PEDAL3,              "P7 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PEDAL3,              "P8 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, PEDAL3,              "P9 Pedal 3" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, PEDAL3,             "P10 Pedal 3" )
}

inline void construct_core_types_paddle(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PADDLE,              "Paddle" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PADDLE,              "Paddle 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PADDLE,              "Paddle 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PADDLE,              "Paddle 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PADDLE,              "Paddle 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PADDLE,              "Paddle 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PADDLE,              "Paddle 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PADDLE,              "Paddle 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, PADDLE,              "Paddle 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, PADDLE,             "Paddle 10" )
}

inline void construct_core_types_paddle_v(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PADDLE_V,            "Paddle V" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PADDLE_V,            "Paddle V 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PADDLE_V,            "Paddle V 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PADDLE_V,            "Paddle V 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PADDLE_V,            "Paddle V 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PADDLE_V,            "Paddle V 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PADDLE_V,            "Paddle V 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PADDLE_V,            "Paddle V 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, PADDLE_V,            "Paddle V 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, PADDLE_V,            "Paddle V 10" )
}

inline void construct_core_types_positional(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, POSITIONAL,          "Positional" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, POSITIONAL,          "Positional 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, POSITIONAL,          "Positional 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, POSITIONAL,          "Positional 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, POSITIONAL,          "Positional 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, POSITIONAL,          "Positional 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, POSITIONAL,          "Positional 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, POSITIONAL,          "Positional 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, POSITIONAL,          "Positional 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, POSITIONAL,         "Positional 10" )
}

inline void construct_core_types_positional_v(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, POSITIONAL_V,        "Positional V" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, POSITIONAL_V,        "Positional V 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, POSITIONAL_V,        "Positional V 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, POSITIONAL_V,        "Positional V 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, POSITIONAL_V,        "Positional V 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, POSITIONAL_V,        "Positional V 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, POSITIONAL_V,        "Positional V 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, POSITIONAL_V,        "Positional V 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, POSITIONAL_V,        "Positional V 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, POSITIONAL_V,       "Positional V 10" )
}

inline void construct_core_types_dial(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, DIAL,                "Dial" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, DIAL,                "Dial 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, DIAL,                "Dial 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, DIAL,                "Dial 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, DIAL,                "Dial 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, DIAL,                "Dial 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, DIAL,                "Dial 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, DIAL,                "Dial 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, DIAL,                "Dial 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, DIAL,               "Dial 10" )
}

inline void construct_core_types_dial_v(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, DIAL_V,              "Dial V" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, DIAL_V,              "Dial V 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, DIAL_V,              "Dial V 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, DIAL_V,              "Dial V 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, DIAL_V,              "Dial V 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, DIAL_V,              "Dial V 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, DIAL_V,              "Dial V 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, DIAL_V,              "Dial V 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, DIAL_V,              "Dial V 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, DIAL_V,             "Dial V 10" )
}

inline void construct_core_types_trackball_X(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, TRACKBALL_X,         "Track X" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, TRACKBALL_X,         "Track X 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, TRACKBALL_X,         "Track X 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, TRACKBALL_X,         "Track X 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, TRACKBALL_X,         "Track X 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, TRACKBALL_X,         "Track X 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, TRACKBALL_X,         "Track X 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, TRACKBALL_X,         "Track X 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, TRACKBALL_X,         "Track X 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, TRACKBALL_X,        "Track X 10" )
}

inline void construct_core_types_trackball_Y(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, TRACKBALL_Y,         "Track Y" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, TRACKBALL_Y,         "Track Y 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, TRACKBALL_Y,         "Track Y 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, TRACKBALL_Y,         "Track Y 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, TRACKBALL_Y,         "Track Y 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, TRACKBALL_Y,         "Track Y 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, TRACKBALL_Y,         "Track Y 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, TRACKBALL_Y,         "Track Y 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, TRACKBALL_Y,         "Track Y 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, TRACKBALL_Y,        "Track Y 10" )
}

inline void construct_core_types_AD_stick_X(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, AD_STICK_X,          "AD Stick X" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, AD_STICK_X,          "AD Stick X 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, AD_STICK_X,          "AD Stick X 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, AD_STICK_X,          "AD Stick X 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, AD_STICK_X,          "AD Stick X 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, AD_STICK_X,          "AD Stick X 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, AD_STICK_X,          "AD Stick X 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, AD_STICK_X,          "AD Stick X 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, AD_STICK_X,          "AD Stick X 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, AD_STICK_X,         "AD Stick X 10" )
}

inline void construct_core_types_AD_stick_Y(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, AD_STICK_Y,          "AD Stick Y" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, AD_STICK_Y,          "AD Stick Y 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, AD_STICK_Y,          "AD Stick Y 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, AD_STICK_Y,          "AD Stick Y 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, AD_STICK_Y,          "AD Stick Y 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, AD_STICK_Y,          "AD Stick Y 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, AD_STICK_Y,          "AD Stick Y 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, AD_STICK_Y,          "AD Stick Y 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, AD_STICK_Y,          "AD Stick Y 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, AD_STICK_Y,         "AD Stick Y 10" )
}

inline void construct_core_types_AD_stick_Z(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, AD_STICK_Z,          "AD Stick Z" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, AD_STICK_Z,          "AD Stick Z 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, AD_STICK_Z,          "AD Stick Z 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, AD_STICK_Z,          "AD Stick Z 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, AD_STICK_Z,          "AD Stick Z 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, AD_STICK_Z,          "AD Stick Z 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, AD_STICK_Z,          "AD Stick Z 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, AD_STICK_Z,          "AD Stick Z 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, AD_STICK_Z,          "AD Stick Z 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, AD_STICK_Z,         "AD Stick Z 10" )
}

inline void construct_core_types_lightgun_X(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, LIGHTGUN_X,          "Lightgun X" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, LIGHTGUN_X,          "Lightgun X 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, LIGHTGUN_X,          "Lightgun X 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, LIGHTGUN_X,          "Lightgun X 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, LIGHTGUN_X,          "Lightgun X 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, LIGHTGUN_X,          "Lightgun X 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, LIGHTGUN_X,          "Lightgun X 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, LIGHTGUN_X,          "Lightgun X 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, LIGHTGUN_X,          "Lightgun X 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, LIGHTGUN_X,         "Lightgun X 10" )
}

inline void construct_core_types_lightgun_Y(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, LIGHTGUN_Y,          "Lightgun Y" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, LIGHTGUN_Y,          "Lightgun Y 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, LIGHTGUN_Y,          "Lightgun Y 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, LIGHTGUN_Y,          "Lightgun Y 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, LIGHTGUN_Y,          "Lightgun Y 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, LIGHTGUN_Y,          "Lightgun Y 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, LIGHTGUN_Y,          "Lightgun Y 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, LIGHTGUN_Y,          "Lightgun Y 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, LIGHTGUN_Y,          "Lightgun Y 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, LIGHTGUN_Y,         "Lightgun Y 10" )
}

inline void construct_core_types_mouse_X(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, MOUSE_X,             "Mouse X" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, MOUSE_X,             "Mouse X 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, MOUSE_X,             "Mouse X 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, MOUSE_X,             "Mouse X 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, MOUSE_X,             "Mouse X 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, MOUSE_X,             "Mouse X 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, MOUSE_X,             "Mouse X 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, MOUSE_X,             "Mouse X 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, MOUSE_X,             "Mouse X 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, MOUSE_X,            "Mouse X 10" )
}

inline void construct_core_types_mouse_Y(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, MOUSE_Y,             "Mouse Y" )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, MOUSE_Y,             "Mouse Y 2" )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, MOUSE_Y,             "Mouse Y 3" )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, MOUSE_Y,             "Mouse Y 4" )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, MOUSE_Y,             "Mouse Y 5" )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, MOUSE_Y,             "Mouse Y 6" )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, MOUSE_Y,             "Mouse Y 7" )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, MOUSE_Y,             "Mouse Y 8" )
	INPUT_PORT_ANALOG_TYPE(  9, PLAYER9, MOUSE_Y,             "Mouse Y 9" )
	INPUT_PORT_ANALOG_TYPE( 10, PLAYER10, MOUSE_Y,            "Mouse Y 10" )
}

inline void construct_core_types_keypad(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   KEYPAD,              "Keypad" )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   KEYBOARD,            "Keyboard" )
}

inline void construct_core_types_UI(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_ON_SCREEN_DISPLAY,"On Screen Display" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DEBUG_BREAK,      "Break in Debugger" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_CONFIGURE,        "Config Menu" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PAUSE,            "Pause" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PAUSE_SINGLE,     "Pause - Single Step" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_RESET_MACHINE,    "Reset Machine" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SOFT_RESET,       "Soft Reset" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SHOW_GFX,         "Show Gfx" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_FRAMESKIP_DEC,    "Frameskip Dec" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_FRAMESKIP_INC,    "Frameskip Inc" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_THROTTLE,         "Throttle" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_FAST_FORWARD,     "Fast Forward" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SHOW_FPS,         "Show FPS" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SNAPSHOT,         "Save Snapshot" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TIMECODE,         "Write current timecode" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_RECORD_MOVIE,     "Record Movie" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TOGGLE_CHEAT,     "Toggle Cheat" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TOGGLE_AUTOFIRE,  "Toggle Autofire" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_UP,               "UI Up" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DOWN,             "UI Down" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_LEFT,             "UI Left" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_RIGHT,            "UI Right" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_HOME,             "UI Home" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_END,              "UI End" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PAGE_UP,          "UI Page Up" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PAGE_DOWN,        "UI Page Down" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SELECT,           "UI Select" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_CANCEL,           "UI Cancel" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DISPLAY_COMMENT,  "UI Display Comment" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_CLEAR,            "UI Clear" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_ZOOM_IN,          "UI Zoom In" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_ZOOM_OUT,         "UI Zoom Out" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PREV_GROUP,       "UI Previous Group" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_NEXT_GROUP,       "UI Next Group" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_ROTATE,           "UI Rotate" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SHOW_PROFILER,    "Show Profiler" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TOGGLE_UI,        "UI Toggle" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PASTE,            "UI Paste Text" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TOGGLE_DEBUG,     "Toggle Debugger" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SAVE_STATE,       "Save State" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_LOAD_STATE,       "Load State" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TAPE_START,       "UI (First) Tape Start" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TAPE_STOP,        "UI (First) Tape Stop" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DATS,             "UI External DAT View" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_FAVORITES,        "UI Add/Remove favorites" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_UP_FILTER,        nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DOWN_FILTER,      nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_LEFT_PANEL,       nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_RIGHT_PANEL,      nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_UP_PANEL,         nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DOWN_PANEL,       nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_EXPORT,           "UI Export list" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_AUDIT_FAST,       "UI Audit Unavailable" )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_AUDIT_ALL,        "UI Audit All" )
}

inline void construct_core_types_OSD(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_1,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_2,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_3,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_4,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_5,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_6,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_7,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_8,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_9,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_10,              nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_11,              nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_12,              nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_13,              nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_14,              nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_15,              nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_16,              nullptr )
}

inline void construct_core_types_invalid(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, UNKNOWN,             nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, UNUSED,              nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, SPECIAL,             nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, OTHER,               nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, ADJUSTER,            nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, DIPSWITCH,           nullptr )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, CONFIG,              nullptr )
}

inline void construct_core_types(simple_list<input_type_entry> &typelist)
{
	construct_core_types_P1(typelist);
	construct_core_types_P1_mahjong(typelist);
	construct_core_types_P1_hanafuda(typelist);
	construct_core_types_gamble(typelist);
	construct_core_types_poker(typelist);
	construct_core_types_slot(typelist);
	construct_core_types_P2(typelist);
	construct_core_types_P2_mahjong(typelist);
	construct_core_types_P2_hanafuda(typelist);
	construct_core_types_P3(typelist);
	construct_core_types_P4(typelist);
	construct_core_types_P5(typelist);
	construct_core_types_P6(typelist);
	construct_core_types_P7(typelist);
	construct_core_types_P8(typelist);
	construct_core_types_P9(typelist);
	construct_core_types_P10(typelist);
	construct_core_types_start(typelist);
	construct_core_types_coin(typelist);
	construct_core_types_service(typelist);
	construct_core_types_tilt(typelist);
	construct_core_types_other(typelist);
	construct_core_types_pedal(typelist);
	construct_core_types_pedal2(typelist);
	construct_core_types_pedal3(typelist);
	construct_core_types_paddle(typelist);
	construct_core_types_paddle_v(typelist);
	construct_core_types_positional(typelist);
	construct_core_types_positional_v(typelist);
	construct_core_types_dial(typelist);
	construct_core_types_dial_v(typelist);
	construct_core_types_trackball_X(typelist);
	construct_core_types_trackball_Y(typelist);
	construct_core_types_AD_stick_X(typelist);
	construct_core_types_AD_stick_Y(typelist);
	construct_core_types_AD_stick_Z(typelist);
	construct_core_types_lightgun_X(typelist);
	construct_core_types_lightgun_Y(typelist);
	construct_core_types_mouse_X(typelist);
	construct_core_types_mouse_Y(typelist);
	construct_core_types_keypad(typelist);
	construct_core_types_UI(typelist);
	construct_core_types_OSD(typelist);
	construct_core_types_invalid(typelist);
}
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ > 4 || (__GNUC_MINOR__ == 4 && __GNUC_PATCHLEVEL__ >= 4))))
#if not(defined(__arm__) || defined(__ARMEL__))
#pragma GCC pop_options
#endif
#elif defined(_MSC_VER)
#pragma optimize("", on)
#endif
