// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inpttype.h

    Array of core-defined input types and default mappings.

***************************************************************************/


/***************************************************************************
    BUILT-IN CORE MAPPINGS
***************************************************************************/

#define INPUT_PORT_DIGITAL_TYPE(_player,_group,_type,_name,_seq) \
	typelist.append(*global_alloc(input_type_entry(IPT_##_type, IPG_##_group, (_player == 0) ? _player : (_player) - 1, (_player == 0) ? #_type : ("P" #_player "_" #_type), _name, _seq)));

#define INPUT_PORT_ANALOG_TYPE(_player,_group,_type,_name,_seq,_decseq,_incseq) \
	typelist.append(*global_alloc(input_type_entry(IPT_##_type, IPG_##_group, (_player == 0) ? _player : (_player) - 1, (_player == 0) ? #_type : ("P" #_player "_" #_type), _name, _seq, _decseq, _incseq)));

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

void construct_core_types_P1(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICK_UP,         "P1 Up",                  input_seq(KEYCODE_UP, input_seq::or_code, JOYCODE_Y_UP_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICK_DOWN,       "P1 Down",                input_seq(KEYCODE_DOWN, input_seq::or_code, JOYCODE_Y_DOWN_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICK_LEFT,       "P1 Left",                input_seq(KEYCODE_LEFT, input_seq::or_code, JOYCODE_X_LEFT_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICK_RIGHT,      "P1 Right",               input_seq(KEYCODE_RIGHT, input_seq::or_code, JOYCODE_X_RIGHT_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKRIGHT_UP,    "P1 Right/Up",            input_seq(KEYCODE_I, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKRIGHT_DOWN,  "P1 Right/Down",          input_seq(KEYCODE_K, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKRIGHT_LEFT,  "P1 Right/Left",          input_seq(KEYCODE_J, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKRIGHT_RIGHT, "P1 Right/Right",         input_seq(KEYCODE_L, input_seq::or_code, JOYCODE_BUTTON4_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKLEFT_UP,     "P1 Left/Up",             input_seq(KEYCODE_E, input_seq::or_code, JOYCODE_Y_UP_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKLEFT_DOWN,   "P1 Left/Down",           input_seq(KEYCODE_D, input_seq::or_code, JOYCODE_Y_DOWN_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKLEFT_LEFT,   "P1 Left/Left",           input_seq(KEYCODE_S, input_seq::or_code, JOYCODE_X_LEFT_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, JOYSTICKLEFT_RIGHT,  "P1 Left/Right",          input_seq(KEYCODE_F, input_seq::or_code, JOYCODE_X_RIGHT_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON1,             "P1 Button 1",            input_seq(KEYCODE_LCONTROL, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(0), input_seq::or_code, MOUSECODE_BUTTON1_INDEXED(0), input_seq::or_code, GUNCODE_BUTTON1_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON2,             "P1 Button 2",            input_seq(KEYCODE_LALT, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(0), input_seq::or_code, MOUSECODE_BUTTON3_INDEXED(0), input_seq::or_code, GUNCODE_BUTTON2_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON3,             "P1 Button 3",            input_seq(KEYCODE_SPACE, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(0), input_seq::or_code, MOUSECODE_BUTTON2_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON4,             "P1 Button 4",            input_seq(KEYCODE_LSHIFT, input_seq::or_code, JOYCODE_BUTTON4_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON5,             "P1 Button 5",            input_seq(KEYCODE_Z, input_seq::or_code, JOYCODE_BUTTON5_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON6,             "P1 Button 6",            input_seq(KEYCODE_X, input_seq::or_code, JOYCODE_BUTTON6_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON7,             "P1 Button 7",            input_seq(KEYCODE_C, input_seq::or_code, JOYCODE_BUTTON7_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON8,             "P1 Button 8",            input_seq(KEYCODE_V, input_seq::or_code, JOYCODE_BUTTON8_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON9,             "P1 Button 9",            input_seq(KEYCODE_B, input_seq::or_code, JOYCODE_BUTTON9_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON10,            "P1 Button 10",           input_seq(KEYCODE_N, input_seq::or_code, JOYCODE_BUTTON10_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON11,            "P1 Button 11",           input_seq(KEYCODE_M, input_seq::or_code, JOYCODE_BUTTON11_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON12,            "P1 Button 12",           input_seq(KEYCODE_COMMA, input_seq::or_code, JOYCODE_BUTTON12_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON13,            "P1 Button 13",           input_seq(KEYCODE_STOP, input_seq::or_code, JOYCODE_BUTTON13_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON14,            "P1 Button 14",           input_seq(KEYCODE_SLASH, input_seq::or_code, JOYCODE_BUTTON14_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON15,            "P1 Button 15",           input_seq(KEYCODE_RSHIFT, input_seq::or_code, JOYCODE_BUTTON15_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, BUTTON16,            "P1 Button 16",           input_seq(JOYCODE_BUTTON16_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, START,               "P1 Start",               input_seq(KEYCODE_1, input_seq::or_code, JOYCODE_START_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SELECT,              "P1 Select",              input_seq(KEYCODE_5, input_seq::or_code, JOYCODE_SELECT_INDEXED(0)) )
}

void construct_core_types_P1_mahjong(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_A,           "P1 Mahjong A",           input_seq(KEYCODE_A) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_B,           "P1 Mahjong B",           input_seq(KEYCODE_B) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_C,           "P1 Mahjong C",           input_seq(KEYCODE_C) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_D,           "P1 Mahjong D",           input_seq(KEYCODE_D) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_E,           "P1 Mahjong E",           input_seq(KEYCODE_E) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_F,           "P1 Mahjong F",           input_seq(KEYCODE_F) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_G,           "P1 Mahjong G",           input_seq(KEYCODE_G) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_H,           "P1 Mahjong H",           input_seq(KEYCODE_H) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_I,           "P1 Mahjong I",           input_seq(KEYCODE_I) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_J,           "P1 Mahjong J",           input_seq(KEYCODE_J) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_K,           "P1 Mahjong K",           input_seq(KEYCODE_K) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_L,           "P1 Mahjong L",           input_seq(KEYCODE_L) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_M,           "P1 Mahjong M",           input_seq(KEYCODE_M) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_N,           "P1 Mahjong N",           input_seq(KEYCODE_N) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_O,           "P1 Mahjong O",           input_seq(KEYCODE_O) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_P,           "P1 Mahjong P",           input_seq(KEYCODE_COLON) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_Q,           "P1 Mahjong Q",           input_seq(KEYCODE_Q) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_KAN,         "P1 Mahjong Kan",         input_seq(KEYCODE_LCONTROL) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_PON,         "P1 Mahjong Pon",         input_seq(KEYCODE_LALT) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_CHI,         "P1 Mahjong Chi",         input_seq(KEYCODE_SPACE) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_REACH,       "P1 Mahjong Reach",       input_seq(KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_RON,         "P1 Mahjong Ron",         input_seq(KEYCODE_Z) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_BET,         "P1 Mahjong Bet",         input_seq(KEYCODE_3) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_LAST_CHANCE, "P1 Mahjong Last Chance", input_seq(KEYCODE_RALT) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_SCORE,       "P1 Mahjong Score",       input_seq(KEYCODE_RCONTROL) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_DOUBLE_UP,   "P1 Mahjong Double Up",   input_seq(KEYCODE_RSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_FLIP_FLOP,   "P1 Mahjong Flip Flop",   input_seq(KEYCODE_Y) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_BIG,         "P1 Mahjong Big",         input_seq(KEYCODE_ENTER) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, MAHJONG_SMALL,       "P1 Mahjong Small",       input_seq(KEYCODE_BACKSPACE) )
}

void construct_core_types_P1_hanafuda(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_A,          "P1 Hanafuda A / 1",      input_seq(KEYCODE_A) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_B,          "P1 Hanafuda B / 2",      input_seq(KEYCODE_B) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_C,          "P1 Hanafuda C / 3",      input_seq(KEYCODE_C) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_D,          "P1 Hanafuda D / 4",      input_seq(KEYCODE_D) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_E,          "P1 Hanafuda E / 5",      input_seq(KEYCODE_E) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_F,          "P1 Hanafuda F / 6",      input_seq(KEYCODE_F) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_G,          "P1 Hanafuda G / 7",      input_seq(KEYCODE_G) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_H,          "P1 Hanafuda H / 8",      input_seq(KEYCODE_H) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_YES,        "P1 Hanafuda Yes",        input_seq(KEYCODE_M) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, HANAFUDA_NO,         "P1 Hanafuda No",         input_seq(KEYCODE_N) )
}

void construct_core_types_gamble(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_HIGH,         "High",                   input_seq(KEYCODE_A) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_LOW,          "Low",                    input_seq(KEYCODE_S) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_HALF,         "Half Gamble",            input_seq(KEYCODE_D) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_DEAL,         "Deal",                   input_seq(KEYCODE_2) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_D_UP,         "Double Up",              input_seq(KEYCODE_3) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_TAKE,         "Take",                   input_seq(KEYCODE_4) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_STAND,        "Stand",                  input_seq(KEYCODE_L) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_BET,          "Bet",                    input_seq(KEYCODE_M) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_KEYIN,        "Key In",                 input_seq(KEYCODE_Q) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_KEYOUT,       "Key Out",                input_seq(KEYCODE_W) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_PAYOUT,       "Payout",                 input_seq(KEYCODE_I) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_DOOR,         "Door",                   input_seq(KEYCODE_O) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_SERVICE,      "Service",                input_seq(KEYCODE_9) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, GAMBLE_BOOK,         "Book-Keeping",           input_seq(KEYCODE_0) )
}

void construct_core_types_poker(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD1,         "Hold 1",                 input_seq(KEYCODE_Z) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD2,         "Hold 2",                 input_seq(KEYCODE_X) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD3,         "Hold 3",                 input_seq(KEYCODE_C) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD4,         "Hold 4",                 input_seq(KEYCODE_V) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_HOLD5,         "Hold 5",                 input_seq(KEYCODE_B) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_CANCEL,        "Cancel",                 input_seq(KEYCODE_N) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, POKER_BET,           "Bet",                    input_seq(KEYCODE_1) )
}

void construct_core_types_slot(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP1,          "Stop Reel 1",            input_seq(KEYCODE_X) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP2,          "Stop Reel 2",            input_seq(KEYCODE_C) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP3,          "Stop Reel 3",            input_seq(KEYCODE_V) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP4,          "Stop Reel 4",            input_seq(KEYCODE_B) )
	INPUT_PORT_DIGITAL_TYPE( 1, PLAYER1, SLOT_STOP_ALL,       "Stop All Reels",         input_seq(KEYCODE_Z) )
}

void construct_core_types_P2(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICK_UP,         "P2 Up",                  input_seq(KEYCODE_R, input_seq::or_code, JOYCODE_Y_UP_SWITCH_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICK_DOWN,       "P2 Down",                input_seq(KEYCODE_F, input_seq::or_code, JOYCODE_Y_DOWN_SWITCH_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICK_LEFT,       "P2 Left",                input_seq(KEYCODE_D, input_seq::or_code, JOYCODE_X_LEFT_SWITCH_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICK_RIGHT,      "P2 Right",               input_seq(KEYCODE_G, input_seq::or_code, JOYCODE_X_RIGHT_SWITCH_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKRIGHT_UP,    "P2 Right/Up",            input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKRIGHT_DOWN,  "P2 Right/Down",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKRIGHT_LEFT,  "P2 Right/Left",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKRIGHT_RIGHT, "P2 Right/Right",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKLEFT_UP,     "P2 Left/Up",             input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKLEFT_DOWN,   "P2 Left/Down",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKLEFT_LEFT,   "P2 Left/Left",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, JOYSTICKLEFT_RIGHT,  "P2 Left/Right",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON1,             "P2 Button 1",            input_seq(KEYCODE_A, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(1), input_seq::or_code, MOUSECODE_BUTTON1_INDEXED(1), input_seq::or_code, GUNCODE_BUTTON1_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON2,             "P2 Button 2",            input_seq(KEYCODE_S, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(1), input_seq::or_code, MOUSECODE_BUTTON3_INDEXED(1), input_seq::or_code, GUNCODE_BUTTON2_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON3,             "P2 Button 3",            input_seq(KEYCODE_Q, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(1), input_seq::or_code, MOUSECODE_BUTTON2_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON4,             "P2 Button 4",            input_seq(KEYCODE_W, input_seq::or_code, JOYCODE_BUTTON4_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON5,             "P2 Button 5",            input_seq(JOYCODE_BUTTON5_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON6,             "P2 Button 6",            input_seq(JOYCODE_BUTTON6_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON7,             "P2 Button 7",            input_seq(JOYCODE_BUTTON7_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON8,             "P2 Button 8",            input_seq(JOYCODE_BUTTON8_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON9,             "P2 Button 9",            input_seq(JOYCODE_BUTTON9_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON10,            "P2 Button 10",           input_seq(JOYCODE_BUTTON10_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON11,            "P2 Button 11",           input_seq(JOYCODE_BUTTON11_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON12,            "P2 Button 12",           input_seq(JOYCODE_BUTTON12_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON13,            "P2 Button 13",           input_seq(JOYCODE_BUTTON13_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON14,            "P2 Button 14",           input_seq(JOYCODE_BUTTON14_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON15,            "P2 Button 15",           input_seq(JOYCODE_BUTTON15_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, BUTTON16,            "P2 Button 16",           input_seq(JOYCODE_BUTTON16_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, START,               "P2 Start",               input_seq(KEYCODE_2, input_seq::or_code, JOYCODE_START_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, SELECT,              "P2 Select",              input_seq(KEYCODE_6, input_seq::or_code, JOYCODE_SELECT_INDEXED(1)) )
}

void construct_core_types_P2_mahjong(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_A,           "P2 Mahjong A",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_B,           "P2 Mahjong B",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_C,           "P2 Mahjong C",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_D,           "P2 Mahjong D",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_E,           "P2 Mahjong E",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_F,           "P2 Mahjong F",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_G,           "P2 Mahjong G",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_H,           "P2 Mahjong H",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_I,           "P2 Mahjong I",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_J,           "P2 Mahjong J",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_K,           "P2 Mahjong K",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_L,           "P2 Mahjong L",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_M,           "P2 Mahjong M",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_N,           "P2 Mahjong N",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_O,           "P2 Mahjong O",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_P,           "P2 Mahjong P",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_Q,           "P2 Mahjong Q",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_KAN,         "P2 Mahjong Kan",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_PON,         "P2 Mahjong Pon",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_CHI,         "P2 Mahjong Chi",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_REACH,       "P2 Mahjong Reach",       input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_RON,         "P2 Mahjong Ron",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_BET,         "P2 Mahjong Bet",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_LAST_CHANCE, "P2 Mahjong Last Chance", input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_SCORE,       "P2 Mahjong Score",       input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_DOUBLE_UP,   "P2 Mahjong Double Up",   input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_FLIP_FLOP,   "P2 Mahjong Flip Flop",   input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_BIG,         "P2 Mahjong Big",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, MAHJONG_SMALL,       "P2 Mahjong Small",       input_seq() )
}

void construct_core_types_P2_hanafuda(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_A,          "P2 Hanafuda A / 1",      input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_B,          "P2 Hanafuda B / 2",      input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_C,          "P2 Hanafuda C / 3",      input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_D,          "P2 Hanafuda D / 4",      input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_E,          "P2 Hanafuda E / 5",      input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_F,          "P2 Hanafuda F / 6",      input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_G,          "P2 Hanafuda G / 7",      input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_H,          "P2 Hanafuda H / 8",      input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_YES,        "P2 Hanafuda Yes",        input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 2, PLAYER2, HANAFUDA_NO,         "P2 Hanafuda No",         input_seq() )
}

void construct_core_types_P3(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICK_UP,         "P3 Up",                  input_seq(KEYCODE_I, input_seq::or_code, JOYCODE_Y_UP_SWITCH_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICK_DOWN,       "P3 Down",                input_seq(KEYCODE_K, input_seq::or_code, JOYCODE_Y_DOWN_SWITCH_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICK_LEFT,       "P3 Left",                input_seq(KEYCODE_J, input_seq::or_code, JOYCODE_X_LEFT_SWITCH_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICK_RIGHT,      "P3 Right",               input_seq(KEYCODE_L, input_seq::or_code, JOYCODE_X_RIGHT_SWITCH_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKRIGHT_UP,    "P3 Right/Up",            input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKRIGHT_DOWN,  "P3 Right/Down",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKRIGHT_LEFT,  "P3 Right/Left",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKRIGHT_RIGHT, "P3 Right/Right",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKLEFT_UP,     "P3 Left/Up",             input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKLEFT_DOWN,   "P3 Left/Down",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKLEFT_LEFT,   "P3 Left/Left",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, JOYSTICKLEFT_RIGHT,  "P3 Left/Right",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON1,             "P3 Button 1",            input_seq(KEYCODE_RCONTROL, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(2), input_seq::or_code, GUNCODE_BUTTON1_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON2,             "P3 Button 2",            input_seq(KEYCODE_RSHIFT, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(2), input_seq::or_code, GUNCODE_BUTTON2_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON3,             "P3 Button 3",            input_seq(KEYCODE_ENTER, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON4,             "P3 Button 4",            input_seq(JOYCODE_BUTTON4_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON5,             "P3 Button 5",            input_seq(JOYCODE_BUTTON5_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON6,             "P3 Button 6",            input_seq(JOYCODE_BUTTON6_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON7,             "P3 Button 7",            input_seq(JOYCODE_BUTTON7_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON8,             "P3 Button 8",            input_seq(JOYCODE_BUTTON8_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON9,             "P3 Button 9",            input_seq(JOYCODE_BUTTON9_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON10,            "P3 Button 10",           input_seq(JOYCODE_BUTTON10_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON11,            "P3 Button 11",           input_seq(JOYCODE_BUTTON11_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON12,            "P3 Button 12",           input_seq(JOYCODE_BUTTON12_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON13,            "P3 Button 13",           input_seq(JOYCODE_BUTTON13_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON14,            "P3 Button 14",           input_seq(JOYCODE_BUTTON14_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON15,            "P3 Button 15",           input_seq(JOYCODE_BUTTON15_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, BUTTON16,            "P3 Button 16",           input_seq(JOYCODE_BUTTON16_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, START,               "P3 Start",               input_seq(KEYCODE_3, input_seq::or_code, JOYCODE_START_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 3, PLAYER3, SELECT,              "P3 Select",              input_seq(KEYCODE_7, input_seq::or_code, JOYCODE_SELECT_INDEXED(2)) )
}

void construct_core_types_P4(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICK_UP,         "P4 Up",                  input_seq(KEYCODE_8_PAD, input_seq::or_code, JOYCODE_Y_UP_SWITCH_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICK_DOWN,       "P4 Down",                input_seq(KEYCODE_2_PAD, input_seq::or_code, JOYCODE_Y_DOWN_SWITCH_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICK_LEFT,       "P4 Left",                input_seq(KEYCODE_4_PAD, input_seq::or_code, JOYCODE_X_LEFT_SWITCH_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICK_RIGHT,      "P4 Right",               input_seq(KEYCODE_6_PAD, input_seq::or_code, JOYCODE_X_RIGHT_SWITCH_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKRIGHT_UP,    "P4 Right/Up",            input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKRIGHT_DOWN,  "P4 Right/Down",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKRIGHT_LEFT,  "P4 Right/Left",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKRIGHT_RIGHT, "P4 Right/Right",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKLEFT_UP,     "P4 Left/Up",             input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKLEFT_DOWN,   "P4 Left/Down",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKLEFT_LEFT,   "P4 Left/Left",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, JOYSTICKLEFT_RIGHT,  "P4 Left/Right",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON1,             "P4 Button 1",            input_seq(KEYCODE_0_PAD, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON2,             "P4 Button 2",            input_seq(KEYCODE_DEL_PAD, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON3,             "P4 Button 3",            input_seq(KEYCODE_ENTER_PAD, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON4,             "P4 Button 4",            input_seq(JOYCODE_BUTTON4_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON5,             "P4 Button 5",            input_seq(JOYCODE_BUTTON5_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON6,             "P4 Button 6",            input_seq(JOYCODE_BUTTON6_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON7,             "P4 Button 7",            input_seq(JOYCODE_BUTTON7_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON8,             "P4 Button 8",            input_seq(JOYCODE_BUTTON8_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON9,             "P4 Button 9",            input_seq(JOYCODE_BUTTON9_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON10,            "P4 Button 10",           input_seq(JOYCODE_BUTTON10_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON11,            "P4 Button 11",           input_seq(JOYCODE_BUTTON11_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON12,            "P4 Button 12",           input_seq(JOYCODE_BUTTON12_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON13,            "P4 Button 13",           input_seq(JOYCODE_BUTTON13_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON14,            "P4 Button 14",           input_seq(JOYCODE_BUTTON14_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON15,            "P4 Button 15",           input_seq(JOYCODE_BUTTON15_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, BUTTON16,            "P4 Button 16",           input_seq(JOYCODE_BUTTON16_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, START,               "P4 Start",               input_seq(KEYCODE_4, input_seq::or_code, JOYCODE_START_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 4, PLAYER4, SELECT,              "P4 Select",              input_seq(KEYCODE_8, input_seq::or_code, JOYCODE_SELECT_INDEXED(3)) )
}

void construct_core_types_P5(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICK_UP,         "P5 Up",                  input_seq(JOYCODE_Y_UP_SWITCH_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICK_DOWN,       "P5 Down",                input_seq(JOYCODE_Y_DOWN_SWITCH_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICK_LEFT,       "P5 Left",                input_seq(JOYCODE_X_LEFT_SWITCH_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICK_RIGHT,      "P5 Right",               input_seq(JOYCODE_X_RIGHT_SWITCH_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKRIGHT_UP,    "P5 Right/Up",            input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKRIGHT_DOWN,  "P5 Right/Down",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKRIGHT_LEFT,  "P5 Right/Left",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKRIGHT_RIGHT, "P5 Right/Right",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKLEFT_UP,     "P5 Left/Up",             input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKLEFT_DOWN,   "P5 Left/Down",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKLEFT_LEFT,   "P5 Left/Left",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, JOYSTICKLEFT_RIGHT,  "P5 Left/Right",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON1,             "P5 Button 1",            input_seq(JOYCODE_BUTTON1_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON2,             "P5 Button 2",            input_seq(JOYCODE_BUTTON2_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON3,             "P5 Button 3",            input_seq(JOYCODE_BUTTON3_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON4,             "P5 Button 4",            input_seq(JOYCODE_BUTTON4_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON5,             "P5 Button 5",            input_seq(JOYCODE_BUTTON5_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON6,             "P5 Button 6",            input_seq(JOYCODE_BUTTON6_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON7,             "P5 Button 7",            input_seq(JOYCODE_BUTTON7_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON8,             "P5 Button 8",            input_seq(JOYCODE_BUTTON8_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON9,             "P5 Button 9",            input_seq(JOYCODE_BUTTON9_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON10,            "P5 Button 10",           input_seq(JOYCODE_BUTTON10_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON11,            "P5 Button 11",           input_seq(JOYCODE_BUTTON11_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON12,            "P5 Button 12",           input_seq(JOYCODE_BUTTON12_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON13,            "P5 Button 13",           input_seq(JOYCODE_BUTTON13_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON14,            "P5 Button 14",           input_seq(JOYCODE_BUTTON14_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON15,            "P5 Button 15",           input_seq(JOYCODE_BUTTON15_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, BUTTON16,            "P5 Button 16",           input_seq(JOYCODE_BUTTON16_INDEXED(4)) )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, START,               "P5 Start",               input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 5, PLAYER5, SELECT,              "P5 Select",              input_seq() )
}

void construct_core_types_P6(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICK_UP,         "P6 Up",                  input_seq(JOYCODE_Y_UP_SWITCH_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICK_DOWN,       "P6 Down",                input_seq(JOYCODE_Y_DOWN_SWITCH_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICK_LEFT,       "P6 Left",                input_seq(JOYCODE_X_LEFT_SWITCH_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICK_RIGHT,      "P6 Right",               input_seq(JOYCODE_X_RIGHT_SWITCH_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKRIGHT_UP,    "P6 Right/Up",            input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKRIGHT_DOWN,  "P6 Right/Down",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKRIGHT_LEFT,  "P6 Right/Left",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKRIGHT_RIGHT, "P6 Right/Right",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKLEFT_UP,     "P6 Left/Up",             input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKLEFT_DOWN,   "P6 Left/Down",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKLEFT_LEFT,   "P6 Left/Left",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, JOYSTICKLEFT_RIGHT,  "P6 Left/Right",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON1,             "P6 Button 1",            input_seq(JOYCODE_BUTTON1_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON2,             "P6 Button 2",            input_seq(JOYCODE_BUTTON2_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON3,             "P6 Button 3",            input_seq(JOYCODE_BUTTON3_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON4,             "P6 Button 4",            input_seq(JOYCODE_BUTTON4_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON5,             "P6 Button 5",            input_seq(JOYCODE_BUTTON5_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON6,             "P6 Button 6",            input_seq(JOYCODE_BUTTON6_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON7,             "P6 Button 7",            input_seq(JOYCODE_BUTTON7_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON8,             "P6 Button 8",            input_seq(JOYCODE_BUTTON8_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON9,             "P6 Button 9",            input_seq(JOYCODE_BUTTON9_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON10,            "P6 Button 10",           input_seq(JOYCODE_BUTTON10_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON11,            "P6 Button 11",           input_seq(JOYCODE_BUTTON11_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON12,            "P6 Button 12",           input_seq(JOYCODE_BUTTON12_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON13,            "P6 Button 13",           input_seq(JOYCODE_BUTTON13_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON14,            "P6 Button 14",           input_seq(JOYCODE_BUTTON14_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON15,            "P6 Button 15",           input_seq(JOYCODE_BUTTON15_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, BUTTON16,            "P6 Button 16",           input_seq(JOYCODE_BUTTON16_INDEXED(5)) )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, START,               "P6 Start",               input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 6, PLAYER6, SELECT,              "P6 Select",              input_seq() )
}

void construct_core_types_P7(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICK_UP,         "P7 Up",                  input_seq(JOYCODE_Y_UP_SWITCH_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICK_DOWN,       "P7 Down",                input_seq(JOYCODE_Y_DOWN_SWITCH_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICK_LEFT,       "P7 Left",                input_seq(JOYCODE_X_LEFT_SWITCH_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICK_RIGHT,      "P7 Right",               input_seq(JOYCODE_X_RIGHT_SWITCH_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKRIGHT_UP,    "P7 Right/Up",            input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKRIGHT_DOWN,  "P7 Right/Down",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKRIGHT_LEFT,  "P7 Right/Left",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKRIGHT_RIGHT, "P7 Right/Right",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKLEFT_UP,     "P7 Left/Up",             input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKLEFT_DOWN,   "P7 Left/Down",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKLEFT_LEFT,   "P7 Left/Left",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, JOYSTICKLEFT_RIGHT,  "P7 Left/Right",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON1,             "P7 Button 1",            input_seq(JOYCODE_BUTTON1_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON2,             "P7 Button 2",            input_seq(JOYCODE_BUTTON2_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON3,             "P7 Button 3",            input_seq(JOYCODE_BUTTON3_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON4,             "P7 Button 4",            input_seq(JOYCODE_BUTTON4_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON5,             "P7 Button 5",            input_seq(JOYCODE_BUTTON5_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON6,             "P7 Button 6",            input_seq(JOYCODE_BUTTON6_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON7,             "P7 Button 7",            input_seq(JOYCODE_BUTTON7_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON8,             "P7 Button 8",            input_seq(JOYCODE_BUTTON8_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON9,             "P7 Button 9",            input_seq(JOYCODE_BUTTON9_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON10,            "P7 Button 10",           input_seq(JOYCODE_BUTTON10_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON11,            "P7 Button 11",           input_seq(JOYCODE_BUTTON11_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON12,            "P7 Button 12",           input_seq(JOYCODE_BUTTON12_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON13,            "P7 Button 13",           input_seq(JOYCODE_BUTTON13_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON14,            "P7 Button 14",           input_seq(JOYCODE_BUTTON14_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON15,            "P7 Button 15",           input_seq(JOYCODE_BUTTON15_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, BUTTON16,            "P7 Button 16",           input_seq(JOYCODE_BUTTON16_INDEXED(6)) )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, START,               "P7 Start",               input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 7, PLAYER7, SELECT,              "P7 Select",              input_seq() )
}

void construct_core_types_P8(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICK_UP,         "P8 Up",                  input_seq(JOYCODE_Y_UP_SWITCH_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICK_DOWN,       "P8 Down",                input_seq(JOYCODE_Y_DOWN_SWITCH_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICK_LEFT,       "P8 Left",                input_seq(JOYCODE_X_LEFT_SWITCH_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICK_RIGHT,      "P8 Right",               input_seq(JOYCODE_X_RIGHT_SWITCH_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKRIGHT_UP,    "P8 Right/Up",            input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKRIGHT_DOWN,  "P8 Right/Down",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKRIGHT_LEFT,  "P8 Right/Left",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKRIGHT_RIGHT, "P8 Right/Right",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKLEFT_UP,     "P8 Left/Up",             input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKLEFT_DOWN,   "P8 Left/Down",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKLEFT_LEFT,   "P8 Left/Left",           input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, JOYSTICKLEFT_RIGHT,  "P8 Left/Right",          input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON1,             "P8 Button 1",            input_seq(JOYCODE_BUTTON1_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON2,             "P8 Button 2",            input_seq(JOYCODE_BUTTON2_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON3,             "P8 Button 3",            input_seq(JOYCODE_BUTTON3_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON4,             "P8 Button 4",            input_seq(JOYCODE_BUTTON4_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON5,             "P8 Button 5",            input_seq(JOYCODE_BUTTON5_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON6,             "P8 Button 6",            input_seq(JOYCODE_BUTTON6_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON7,             "P8 Button 7",            input_seq(JOYCODE_BUTTON7_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON8,             "P8 Button 8",            input_seq(JOYCODE_BUTTON8_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON9,             "P8 Button 9",            input_seq(JOYCODE_BUTTON9_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON10,            "P8 Button 10",           input_seq(JOYCODE_BUTTON10_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON11,            "P8 Button 11",           input_seq(JOYCODE_BUTTON11_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON12,            "P8 Button 12",           input_seq(JOYCODE_BUTTON12_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON13,            "P8 Button 13",           input_seq(JOYCODE_BUTTON13_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON14,            "P8 Button 14",           input_seq(JOYCODE_BUTTON14_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON15,            "P8 Button 15",           input_seq(JOYCODE_BUTTON15_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, BUTTON16,            "P8 Button 16",           input_seq(JOYCODE_BUTTON16_INDEXED(7)) )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, START,               "P8 Start",               input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 8, PLAYER8, SELECT,              "P8 Select",              input_seq() )
}

void construct_core_types_start(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START1,              "1 Player Start",         input_seq(KEYCODE_1, input_seq::or_code, JOYCODE_START_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START2,              "2 Players Start",        input_seq(KEYCODE_2, input_seq::or_code, JOYCODE_START_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START3,              "3 Players Start",        input_seq(KEYCODE_3, input_seq::or_code, JOYCODE_START_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START4,              "4 Players Start",        input_seq(KEYCODE_4, input_seq::or_code, JOYCODE_START_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START5,              "5 Players Start",        input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START6,              "6 Players Start",        input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START7,              "7 Players Start",        input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   START8,              "8 Players Start",        input_seq() )
}

void construct_core_types_coin(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN1,               "Coin 1",                 input_seq(KEYCODE_5, input_seq::or_code, JOYCODE_SELECT_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN2,               "Coin 2",                 input_seq(KEYCODE_6, input_seq::or_code, JOYCODE_SELECT_INDEXED(1)) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN3,               "Coin 3",                 input_seq(KEYCODE_7, input_seq::or_code, JOYCODE_SELECT_INDEXED(2)) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN4,               "Coin 4",                 input_seq(KEYCODE_8, input_seq::or_code, JOYCODE_SELECT_INDEXED(3)) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN5,               "Coin 5",                 input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN6,               "Coin 6",                 input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN7,               "Coin 7",                 input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN8,               "Coin 8",                 input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN9,               "Coin 9",                 input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN10,              "Coin 10",                input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN11,              "Coin 11",                input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   COIN12,              "Coin 12",                input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   BILL1,               "Bill 1",                 input_seq(KEYCODE_BACKSPACE) )
}

void construct_core_types_service(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE1,            "Service 1",              input_seq(KEYCODE_9) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE2,            "Service 2",              input_seq(KEYCODE_0) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE3,            "Service 3",              input_seq(KEYCODE_MINUS) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE4,            "Service 4",              input_seq(KEYCODE_EQUALS) )
}

void construct_core_types_tilt(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT1,               "Tilt 1",                 input_seq(KEYCODE_T) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT2,               "Tilt 2",                 input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT3,               "Tilt 3",                 input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT4,               "Tilt 4",                 input_seq() )
}

void construct_core_types_other(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   SERVICE,             "Service",                input_seq(KEYCODE_F2) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   TILT,                "Tilt",                   input_seq(KEYCODE_T) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   INTERLOCK,           "Door Interlock",         input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   VOLUME_DOWN,         "Volume Down",            input_seq(KEYCODE_MINUS) )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   VOLUME_UP,           "Volume Up",              input_seq(KEYCODE_EQUALS) )
}

void construct_core_types_pedal(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PEDAL,               "P1 Pedal 1",             input_seq(JOYCODE_Z_NEG_ABSOLUTE_INDEXED(0)), input_seq(), input_seq(KEYCODE_LCONTROL, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(0)) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PEDAL,               "P2 Pedal 1",             input_seq(JOYCODE_Z_NEG_ABSOLUTE_INDEXED(1)), input_seq(), input_seq(KEYCODE_A, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(1)) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PEDAL,               "P3 Pedal 1",             input_seq(JOYCODE_Z_NEG_ABSOLUTE_INDEXED(2)), input_seq(), input_seq(KEYCODE_RCONTROL, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(2)) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PEDAL,               "P4 Pedal 1",             input_seq(JOYCODE_Z_NEG_ABSOLUTE_INDEXED(3)), input_seq(), input_seq(KEYCODE_0_PAD, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(3)) )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PEDAL,               "P5 Pedal 1",             input_seq(JOYCODE_Z_NEG_ABSOLUTE_INDEXED(4)), input_seq(), input_seq(JOYCODE_BUTTON1_INDEXED(4)) )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PEDAL,               "P6 Pedal 1",             input_seq(JOYCODE_Z_NEG_ABSOLUTE_INDEXED(5)), input_seq(), input_seq(JOYCODE_BUTTON1_INDEXED(5)) )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PEDAL,               "P7 Pedal 1",             input_seq(JOYCODE_Z_NEG_ABSOLUTE_INDEXED(6)), input_seq(), input_seq(JOYCODE_BUTTON1_INDEXED(6)) )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PEDAL,               "P8 Pedal 1",             input_seq(JOYCODE_Z_NEG_ABSOLUTE_INDEXED(7)), input_seq(), input_seq(JOYCODE_BUTTON1_INDEXED(7)) )
}

void construct_core_types_pedal2(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PEDAL2,              "P1 Pedal 2",             input_seq(JOYCODE_Z_POS_ABSOLUTE_INDEXED(0)), input_seq(), input_seq(KEYCODE_LALT, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(0)) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PEDAL2,              "P2 Pedal 2",             input_seq(JOYCODE_Z_POS_ABSOLUTE_INDEXED(1)), input_seq(), input_seq(KEYCODE_S, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(1)) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PEDAL2,              "P3 Pedal 2",             input_seq(JOYCODE_Z_POS_ABSOLUTE_INDEXED(2)), input_seq(), input_seq(KEYCODE_RSHIFT, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(2)) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PEDAL2,              "P4 Pedal 2",             input_seq(JOYCODE_Z_POS_ABSOLUTE_INDEXED(3)), input_seq(), input_seq(KEYCODE_DEL_PAD, input_seq::or_code, JOYCODE_BUTTON2_INDEXED(3)) )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PEDAL2,              "P5 Pedal 2",             input_seq(JOYCODE_Z_POS_ABSOLUTE_INDEXED(4)), input_seq(), input_seq(JOYCODE_BUTTON2_INDEXED(4)) )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PEDAL2,              "P6 Pedal 2",             input_seq(JOYCODE_Z_POS_ABSOLUTE_INDEXED(5)), input_seq(), input_seq(JOYCODE_BUTTON2_INDEXED(5)) )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PEDAL2,              "P7 Pedal 2",             input_seq(JOYCODE_Z_POS_ABSOLUTE_INDEXED(6)), input_seq(), input_seq(JOYCODE_BUTTON2_INDEXED(6)) )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PEDAL2,              "P8 Pedal 2",             input_seq(JOYCODE_Z_POS_ABSOLUTE_INDEXED(7)), input_seq(), input_seq(JOYCODE_BUTTON2_INDEXED(7)) )
}

void construct_core_types_pedal3(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PEDAL3,              "P1 Pedal 3",             input_seq(), input_seq(), input_seq(KEYCODE_SPACE, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(0)) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PEDAL3,              "P2 Pedal 3",             input_seq(), input_seq(), input_seq(KEYCODE_Q, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(1)) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PEDAL3,              "P3 Pedal 3",             input_seq(), input_seq(), input_seq(KEYCODE_ENTER, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(2)) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PEDAL3,              "P4 Pedal 3",             input_seq(), input_seq(), input_seq(KEYCODE_ENTER_PAD, input_seq::or_code, JOYCODE_BUTTON3_INDEXED(3)) )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PEDAL3,              "P5 Pedal 3",             input_seq(), input_seq(), input_seq(JOYCODE_BUTTON3_INDEXED(4)) )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PEDAL3,              "P6 Pedal 3",             input_seq(), input_seq(), input_seq(JOYCODE_BUTTON3_INDEXED(5)) )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PEDAL3,              "P7 Pedal 3",             input_seq(), input_seq(), input_seq(JOYCODE_BUTTON3_INDEXED(6)) )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PEDAL3,              "P8 Pedal 3",             input_seq(), input_seq(), input_seq(JOYCODE_BUTTON3_INDEXED(7)) )
}

void construct_core_types_paddle(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PADDLE,              "Paddle",                 input_seq(JOYCODE_X_INDEXED(0), input_seq::or_code, MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PADDLE,              "Paddle 2",               input_seq(JOYCODE_X_INDEXED(1), input_seq::or_code, MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PADDLE,              "Paddle 3",               input_seq(JOYCODE_X_INDEXED(2), input_seq::or_code, MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PADDLE,              "Paddle 4",               input_seq(JOYCODE_X_INDEXED(3), input_seq::or_code, MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PADDLE,              "Paddle 5",               input_seq(JOYCODE_X_INDEXED(4), input_seq::or_code, MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PADDLE,              "Paddle 6",               input_seq(JOYCODE_X_INDEXED(5), input_seq::or_code, MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PADDLE,              "Paddle 7",               input_seq(JOYCODE_X_INDEXED(6), input_seq::or_code, MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PADDLE,              "Paddle 8",               input_seq(JOYCODE_X_INDEXED(7), input_seq::or_code, MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_paddle_v(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, PADDLE_V,            "Paddle V",               input_seq(JOYCODE_Y_INDEXED(0), input_seq::or_code, MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, PADDLE_V,            "Paddle V 2",             input_seq(JOYCODE_Y_INDEXED(1), input_seq::or_code, MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, PADDLE_V,            "Paddle V 3",             input_seq(JOYCODE_Y_INDEXED(2), input_seq::or_code, MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, PADDLE_V,            "Paddle V 4",             input_seq(JOYCODE_Y_INDEXED(3), input_seq::or_code, MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, PADDLE_V,            "Paddle V 5",             input_seq(JOYCODE_Y_INDEXED(4), input_seq::or_code, MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, PADDLE_V,            "Paddle V 6",             input_seq(JOYCODE_Y_INDEXED(5), input_seq::or_code, MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, PADDLE_V,            "Paddle V 7",             input_seq(JOYCODE_Y_INDEXED(6), input_seq::or_code, MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, PADDLE_V,            "Paddle V 8",             input_seq(JOYCODE_Y_INDEXED(7), input_seq::or_code, MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_positional(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, POSITIONAL,          "Positional",             input_seq(MOUSECODE_X_INDEXED(0), input_seq::or_code, JOYCODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, POSITIONAL,          "Positional 2",           input_seq(MOUSECODE_X_INDEXED(1), input_seq::or_code, JOYCODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, POSITIONAL,          "Positional 3",           input_seq(MOUSECODE_X_INDEXED(2), input_seq::or_code, JOYCODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, POSITIONAL,          "Positional 4",           input_seq(MOUSECODE_X_INDEXED(3), input_seq::or_code, JOYCODE_X_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, POSITIONAL,          "Positional 5",           input_seq(MOUSECODE_X_INDEXED(4), input_seq::or_code, JOYCODE_X_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, POSITIONAL,          "Positional 6",           input_seq(MOUSECODE_X_INDEXED(5), input_seq::or_code, JOYCODE_X_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, POSITIONAL,          "Positional 7",           input_seq(MOUSECODE_X_INDEXED(6), input_seq::or_code, JOYCODE_X_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, POSITIONAL,          "Positional 8",           input_seq(MOUSECODE_X_INDEXED(7), input_seq::or_code, JOYCODE_X_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_positional_v(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, POSITIONAL_V,        "Positional V",           input_seq(MOUSECODE_Y_INDEXED(0), input_seq::or_code, JOYCODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, POSITIONAL_V,        "Positional V 2",         input_seq(MOUSECODE_Y_INDEXED(1), input_seq::or_code, JOYCODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, POSITIONAL_V,        "Positional V 3",         input_seq(MOUSECODE_Y_INDEXED(2), input_seq::or_code, JOYCODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, POSITIONAL_V,        "Positional V 4",         input_seq(MOUSECODE_Y_INDEXED(3), input_seq::or_code, JOYCODE_Y_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, POSITIONAL_V,        "Positional V 5",         input_seq(MOUSECODE_Y_INDEXED(4), input_seq::or_code, JOYCODE_Y_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, POSITIONAL_V,        "Positional V 6",         input_seq(MOUSECODE_Y_INDEXED(5), input_seq::or_code, JOYCODE_Y_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, POSITIONAL_V,        "Positional V 7",         input_seq(MOUSECODE_Y_INDEXED(6), input_seq::or_code, JOYCODE_Y_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, POSITIONAL_V,        "Positional V 8",         input_seq(MOUSECODE_Y_INDEXED(7), input_seq::or_code, JOYCODE_Y_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_dial(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, DIAL,                "Dial",                   input_seq(MOUSECODE_X_INDEXED(0), input_seq::or_code, JOYCODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, DIAL,                "Dial 2",                 input_seq(MOUSECODE_X_INDEXED(1), input_seq::or_code, JOYCODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, DIAL,                "Dial 3",                 input_seq(MOUSECODE_X_INDEXED(2), input_seq::or_code, JOYCODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, DIAL,                "Dial 4",                 input_seq(MOUSECODE_X_INDEXED(3), input_seq::or_code, JOYCODE_X_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, DIAL,                "Dial 5",                 input_seq(MOUSECODE_X_INDEXED(4), input_seq::or_code, JOYCODE_X_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, DIAL,                "Dial 6",                 input_seq(MOUSECODE_X_INDEXED(5), input_seq::or_code, JOYCODE_X_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, DIAL,                "Dial 7",                 input_seq(MOUSECODE_X_INDEXED(6), input_seq::or_code, JOYCODE_X_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, DIAL,                "Dial 8",                 input_seq(MOUSECODE_X_INDEXED(7), input_seq::or_code, JOYCODE_X_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_dial_v(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, DIAL_V,              "Dial V",                 input_seq(MOUSECODE_Y_INDEXED(0), input_seq::or_code, JOYCODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, DIAL_V,              "Dial V 2",               input_seq(MOUSECODE_Y_INDEXED(1), input_seq::or_code, JOYCODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, DIAL_V,              "Dial V 3",               input_seq(MOUSECODE_Y_INDEXED(2), input_seq::or_code, JOYCODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, DIAL_V,              "Dial V 4",               input_seq(MOUSECODE_Y_INDEXED(3), input_seq::or_code, JOYCODE_Y_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, DIAL_V,              "Dial V 5",               input_seq(MOUSECODE_Y_INDEXED(4), input_seq::or_code, JOYCODE_Y_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, DIAL_V,              "Dial V 6",               input_seq(MOUSECODE_Y_INDEXED(5), input_seq::or_code, JOYCODE_Y_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, DIAL_V,              "Dial V 7",               input_seq(MOUSECODE_Y_INDEXED(6), input_seq::or_code, JOYCODE_Y_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, DIAL_V,              "Dial V 8",               input_seq(MOUSECODE_Y_INDEXED(7), input_seq::or_code, JOYCODE_Y_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_trackball_X(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, TRACKBALL_X,         "Track X",                input_seq(MOUSECODE_X_INDEXED(0), input_seq::or_code, JOYCODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, TRACKBALL_X,         "Track X 2",              input_seq(MOUSECODE_X_INDEXED(1), input_seq::or_code, JOYCODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, TRACKBALL_X,         "Track X 3",              input_seq(MOUSECODE_X_INDEXED(2), input_seq::or_code, JOYCODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, TRACKBALL_X,         "Track X 4",              input_seq(MOUSECODE_X_INDEXED(3), input_seq::or_code, JOYCODE_X_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, TRACKBALL_X,         "Track X 5",              input_seq(MOUSECODE_X_INDEXED(4), input_seq::or_code, JOYCODE_X_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, TRACKBALL_X,         "Track X 6",              input_seq(MOUSECODE_X_INDEXED(5), input_seq::or_code, JOYCODE_X_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, TRACKBALL_X,         "Track X 7",              input_seq(MOUSECODE_X_INDEXED(6), input_seq::or_code, JOYCODE_X_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, TRACKBALL_X,         "Track X 8",              input_seq(MOUSECODE_X_INDEXED(7), input_seq::or_code, JOYCODE_X_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_trackball_Y(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, TRACKBALL_Y,         "Track Y",                input_seq(MOUSECODE_Y_INDEXED(0), input_seq::or_code, JOYCODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, TRACKBALL_Y,         "Track Y 2",              input_seq(MOUSECODE_Y_INDEXED(1), input_seq::or_code, JOYCODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, TRACKBALL_Y,         "Track Y 3",              input_seq(MOUSECODE_Y_INDEXED(2), input_seq::or_code, JOYCODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, TRACKBALL_Y,         "Track Y 4",              input_seq(MOUSECODE_Y_INDEXED(3), input_seq::or_code, JOYCODE_Y_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, TRACKBALL_Y,         "Track Y 5",              input_seq(MOUSECODE_Y_INDEXED(4), input_seq::or_code, JOYCODE_Y_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, TRACKBALL_Y,         "Track Y 6",              input_seq(MOUSECODE_Y_INDEXED(5), input_seq::or_code, JOYCODE_Y_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, TRACKBALL_Y,         "Track Y 7",              input_seq(MOUSECODE_Y_INDEXED(6), input_seq::or_code, JOYCODE_Y_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, TRACKBALL_Y,         "Track Y 8",              input_seq(MOUSECODE_Y_INDEXED(7), input_seq::or_code, JOYCODE_Y_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_AD_stick_X(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, AD_STICK_X,          "AD Stick X",             input_seq(JOYCODE_X_INDEXED(0), input_seq::or_code, MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, AD_STICK_X,          "AD Stick X 2",           input_seq(JOYCODE_X_INDEXED(1), input_seq::or_code, MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, AD_STICK_X,          "AD Stick X 3",           input_seq(JOYCODE_X_INDEXED(2), input_seq::or_code, MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, AD_STICK_X,          "AD Stick X 4",           input_seq(JOYCODE_X_INDEXED(3), input_seq::or_code, MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, AD_STICK_X,          "AD Stick X 5",           input_seq(JOYCODE_X_INDEXED(4), input_seq::or_code, MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, AD_STICK_X,          "AD Stick X 6",           input_seq(JOYCODE_X_INDEXED(5), input_seq::or_code, MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, AD_STICK_X,          "AD Stick X 7",           input_seq(JOYCODE_X_INDEXED(6), input_seq::or_code, MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, AD_STICK_X,          "AD Stick X 8",           input_seq(JOYCODE_X_INDEXED(7), input_seq::or_code, MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_AD_stick_Y(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, AD_STICK_Y,          "AD Stick Y",             input_seq(JOYCODE_Y_INDEXED(0), input_seq::or_code, MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, AD_STICK_Y,          "AD Stick Y 2",           input_seq(JOYCODE_Y_INDEXED(1), input_seq::or_code, MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, AD_STICK_Y,          "AD Stick Y 3",           input_seq(JOYCODE_Y_INDEXED(2), input_seq::or_code, MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, AD_STICK_Y,          "AD Stick Y 4",           input_seq(JOYCODE_Y_INDEXED(3), input_seq::or_code, MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, AD_STICK_Y,          "AD Stick Y 5",           input_seq(JOYCODE_Y_INDEXED(4), input_seq::or_code, MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, AD_STICK_Y,          "AD Stick Y 6",           input_seq(JOYCODE_Y_INDEXED(5), input_seq::or_code, MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, AD_STICK_Y,          "AD Stick Y 7",           input_seq(JOYCODE_Y_INDEXED(6), input_seq::or_code, MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, AD_STICK_Y,          "AD Stick Y 8",           input_seq(JOYCODE_Y_INDEXED(7), input_seq::or_code, MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_AD_stick_Z(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, AD_STICK_Z,          "AD Stick Z",             input_seq(JOYCODE_Z_INDEXED(0)), input_seq(KEYCODE_A), input_seq(KEYCODE_Z) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, AD_STICK_Z,          "AD Stick Z 2",           input_seq(JOYCODE_Z_INDEXED(1)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, AD_STICK_Z,          "AD Stick Z 3",           input_seq(JOYCODE_Z_INDEXED(2)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, AD_STICK_Z,          "AD Stick Z 4",           input_seq(JOYCODE_Z_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, AD_STICK_Z,          "AD Stick Z 5",           input_seq(JOYCODE_Z_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, AD_STICK_Z,          "AD Stick Z 6",           input_seq(JOYCODE_Z_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, AD_STICK_Z,          "AD Stick Z 7",           input_seq(JOYCODE_Z_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, AD_STICK_Z,          "AD Stick Z 8",           input_seq(JOYCODE_Z_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_lightgun_X(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, LIGHTGUN_X,          "Lightgun X",             input_seq(GUNCODE_X_INDEXED(0), input_seq::or_code, MOUSECODE_X_INDEXED(0), input_seq::or_code, JOYCODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, LIGHTGUN_X,          "Lightgun X 2",           input_seq(GUNCODE_X_INDEXED(1), input_seq::or_code, MOUSECODE_X_INDEXED(1), input_seq::or_code, JOYCODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, LIGHTGUN_X,          "Lightgun X 3",           input_seq(GUNCODE_X_INDEXED(2), input_seq::or_code, MOUSECODE_X_INDEXED(2), input_seq::or_code, JOYCODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, LIGHTGUN_X,          "Lightgun X 4",           input_seq(GUNCODE_X_INDEXED(3), input_seq::or_code, MOUSECODE_X_INDEXED(3), input_seq::or_code, JOYCODE_X_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, LIGHTGUN_X,          "Lightgun X 5",           input_seq(GUNCODE_X_INDEXED(4), input_seq::or_code, MOUSECODE_X_INDEXED(4), input_seq::or_code, JOYCODE_X_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, LIGHTGUN_X,          "Lightgun X 6",           input_seq(GUNCODE_X_INDEXED(5), input_seq::or_code, MOUSECODE_X_INDEXED(5), input_seq::or_code, JOYCODE_X_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, LIGHTGUN_X,          "Lightgun X 7",           input_seq(GUNCODE_X_INDEXED(6), input_seq::or_code, MOUSECODE_X_INDEXED(6), input_seq::or_code, JOYCODE_X_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, LIGHTGUN_X,          "Lightgun X 8",           input_seq(GUNCODE_X_INDEXED(7), input_seq::or_code, MOUSECODE_X_INDEXED(7), input_seq::or_code, JOYCODE_X_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_lightgun_Y(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, LIGHTGUN_Y,          "Lightgun Y",             input_seq(GUNCODE_Y_INDEXED(0), input_seq::or_code, MOUSECODE_Y_INDEXED(0), input_seq::or_code, JOYCODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, LIGHTGUN_Y,          "Lightgun Y 2",           input_seq(GUNCODE_Y_INDEXED(1), input_seq::or_code, MOUSECODE_Y_INDEXED(1), input_seq::or_code, JOYCODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, LIGHTGUN_Y,          "Lightgun Y 3",           input_seq(GUNCODE_Y_INDEXED(2), input_seq::or_code, MOUSECODE_Y_INDEXED(2), input_seq::or_code, JOYCODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, LIGHTGUN_Y,          "Lightgun Y 4",           input_seq(GUNCODE_Y_INDEXED(3), input_seq::or_code, MOUSECODE_Y_INDEXED(3), input_seq::or_code, JOYCODE_Y_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, LIGHTGUN_Y,          "Lightgun Y 5",           input_seq(GUNCODE_Y_INDEXED(4), input_seq::or_code, MOUSECODE_Y_INDEXED(4), input_seq::or_code, JOYCODE_Y_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, LIGHTGUN_Y,          "Lightgun Y 6",           input_seq(GUNCODE_Y_INDEXED(5), input_seq::or_code, MOUSECODE_Y_INDEXED(5), input_seq::or_code, JOYCODE_Y_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, LIGHTGUN_Y,          "Lightgun Y 7",           input_seq(GUNCODE_Y_INDEXED(6), input_seq::or_code, MOUSECODE_Y_INDEXED(6), input_seq::or_code, JOYCODE_Y_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, LIGHTGUN_Y,          "Lightgun Y 8",           input_seq(GUNCODE_Y_INDEXED(7), input_seq::or_code, MOUSECODE_Y_INDEXED(7), input_seq::or_code, JOYCODE_Y_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_mouse_X(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, MOUSE_X,             "Mouse X",                input_seq(MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, MOUSE_X,             "Mouse X 2",              input_seq(MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, MOUSE_X,             "Mouse X 3",              input_seq(MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, MOUSE_X,             "Mouse X 4",              input_seq(MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, MOUSE_X,             "Mouse X 5",              input_seq(MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, MOUSE_X,             "Mouse X 6",              input_seq(MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, MOUSE_X,             "Mouse X 7",              input_seq(MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, MOUSE_X,             "Mouse X 8",              input_seq(MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_mouse_Y(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_ANALOG_TYPE(  1, PLAYER1, MOUSE_Y,             "Mouse Y",                input_seq(MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) )
	INPUT_PORT_ANALOG_TYPE(  2, PLAYER2, MOUSE_Y,             "Mouse Y 2",              input_seq(MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) )
	INPUT_PORT_ANALOG_TYPE(  3, PLAYER3, MOUSE_Y,             "Mouse Y 3",              input_seq(MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) )
	INPUT_PORT_ANALOG_TYPE(  4, PLAYER4, MOUSE_Y,             "Mouse Y 4",              input_seq(MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  5, PLAYER5, MOUSE_Y,             "Mouse Y 5",              input_seq(MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  6, PLAYER6, MOUSE_Y,             "Mouse Y 6",              input_seq(MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  7, PLAYER7, MOUSE_Y,             "Mouse Y 7",              input_seq(MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() )
	INPUT_PORT_ANALOG_TYPE(  8, PLAYER8, MOUSE_Y,             "Mouse Y 8",              input_seq(MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() )
}

void construct_core_types_keypad(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   KEYPAD,              "Keypad",                 input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, OTHER,   KEYBOARD,            "Keyboard",               input_seq() )
}

void construct_core_types_UI(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_ON_SCREEN_DISPLAY,"On Screen Display",      input_seq(KEYCODE_TILDE) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DEBUG_BREAK,      "Break in Debugger",      input_seq(KEYCODE_TILDE) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_CONFIGURE,        "Config Menu",            input_seq(KEYCODE_TAB) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PAUSE,            "Pause",                  input_seq(KEYCODE_P, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PAUSE_SINGLE,     "Pause - Single Step",    input_seq(KEYCODE_P, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_P, KEYCODE_RSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_RESET_MACHINE,    "Reset Game",             input_seq(KEYCODE_F3, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SOFT_RESET,       "Soft Reset",             input_seq(KEYCODE_F3, input_seq::not_code, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SHOW_GFX,         "Show Gfx",               input_seq(KEYCODE_F4) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_FRAMESKIP_DEC,    "Frameskip Dec",          input_seq(KEYCODE_F8) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_FRAMESKIP_INC,    "Frameskip Inc",          input_seq(KEYCODE_F9) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_THROTTLE,         "Throttle",               input_seq(KEYCODE_F10) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_FAST_FORWARD,     "Fast Forward",           input_seq(KEYCODE_INSERT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SHOW_FPS,         "Show FPS",               input_seq(KEYCODE_F11, input_seq::not_code, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SNAPSHOT,         "Save Snapshot",          input_seq(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_RECORD_MOVIE,     "Record Movie",           input_seq(KEYCODE_F12, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TOGGLE_CHEAT,     "Toggle Cheat",           input_seq(KEYCODE_F6) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_UP,               "UI Up",                  input_seq(KEYCODE_UP, input_seq::or_code, JOYCODE_Y_UP_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DOWN,             "UI Down",                input_seq(KEYCODE_DOWN, input_seq::or_code, JOYCODE_Y_DOWN_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_LEFT,             "UI Left",                input_seq(KEYCODE_LEFT, input_seq::or_code, JOYCODE_X_LEFT_SWITCH_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_RIGHT,            "UI Right",               input_seq(KEYCODE_RIGHT, input_seq::not_code, KEYCODE_LCONTROL, input_seq::or_code, JOYCODE_X_RIGHT_SWITCH_INDEXED(0), input_seq::not_code, KEYCODE_LCONTROL) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_HOME,             "UI Home",                input_seq(KEYCODE_HOME) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_END,              "UI End",                 input_seq(KEYCODE_END) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PAGE_UP,          "UI Page Up",             input_seq(KEYCODE_PGUP) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PAGE_DOWN,        "UI Page Down",           input_seq(KEYCODE_PGDN) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SELECT,           "UI Select",              input_seq(KEYCODE_ENTER, input_seq::or_code, JOYCODE_BUTTON1_INDEXED(0)) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_CANCEL,           "UI Cancel",              input_seq(KEYCODE_ESC) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DISPLAY_COMMENT,  "UI Display Comment",     input_seq(KEYCODE_SPACE) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_CLEAR,            "UI Clear",               input_seq(KEYCODE_DEL) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_ZOOM_IN,          "UI Zoom In",             input_seq(KEYCODE_EQUALS) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_ZOOM_OUT,         "UI Zoom Out",            input_seq(KEYCODE_MINUS) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PREV_GROUP,       "UI Previous Group",      input_seq(KEYCODE_OPENBRACE) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_NEXT_GROUP,       "UI Next Group",          input_seq(KEYCODE_CLOSEBRACE) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_ROTATE,           "UI Rotate",              input_seq(KEYCODE_R) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SHOW_PROFILER,    "Show Profiler",          input_seq(KEYCODE_F11, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TOGGLE_UI,        "UI Toggle",              input_seq(KEYCODE_SCRLOCK, input_seq::not_code, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_PASTE,            "UI Paste Text",          input_seq(KEYCODE_SCRLOCK, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TOGGLE_DEBUG,     "Toggle Debugger",        input_seq(KEYCODE_F5) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SAVE_STATE,       "Save State",             input_seq(KEYCODE_F7, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_LOAD_STATE,       "Load State",             input_seq(KEYCODE_F7, input_seq::not_code, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TAPE_START,       "UI (First) Tape Start",  input_seq(KEYCODE_F2, input_seq::not_code, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_TAPE_STOP,        "UI (First) Tape Stop",   input_seq(KEYCODE_F2, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_HISTORY,          "UI Show History",        input_seq(KEYCODE_LALT, KEYCODE_H) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_MAMEINFO,         "UI Show Mame/Messinfo",  input_seq(KEYCODE_LALT, KEYCODE_M) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_COMMAND,          "UI Show Command Info",   input_seq(KEYCODE_LALT, KEYCODE_C) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_SYSINFO,          "UI Show Sysinfo",        input_seq(KEYCODE_LALT, KEYCODE_S) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_FAVORITES,        "UI Add/Remove favorites",input_seq(KEYCODE_LALT, KEYCODE_F) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_STORY,            "UI Show Story.dat",      input_seq(KEYCODE_LALT, KEYCODE_T) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_UP_FILTER,        NULL,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DOWN_FILTER,      NULL,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_LEFT_PANEL,       NULL,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_RIGHT_PANEL,      "UI Right switch image/info",         input_seq(KEYCODE_RIGHT, KEYCODE_LCONTROL) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_UP_PANEL,         NULL,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_DOWN_PANEL,       NULL,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_EXPORT,           "UI Export list to xml",  input_seq(KEYCODE_LALT, KEYCODE_E) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_AUDIT_FAST,       "UI Audit Unavailable",   input_seq(KEYCODE_F1, input_seq::not_code, KEYCODE_LSHIFT) )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      UI_AUDIT_ALL,        "UI Audit All",           input_seq(KEYCODE_F1, KEYCODE_LSHIFT) )
}

void construct_core_types_OSD(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_1,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_2,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_3,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_4,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_5,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_6,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_7,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_8,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_9,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_10,              nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_11,              nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_12,              nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_13,              nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_14,              nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_15,              nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, UI,      OSD_16,              nullptr,                     input_seq() )
}

void construct_core_types_invalid(simple_list<input_type_entry> &typelist)
{
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, UNKNOWN,             nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, UNUSED,              nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, SPECIAL,             nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, OTHER,               nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, ADJUSTER,            nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, DIPSWITCH,           nullptr,                     input_seq() )
	INPUT_PORT_DIGITAL_TYPE( 0, INVALID, CONFIG,              nullptr,                     input_seq() )
}

void construct_core_types(simple_list<input_type_entry> &typelist)
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
