// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    inpttype.ipp

    Array of core-defined input types and default mappings.

***************************************************************************/

#include "util/language.h"


/***************************************************************************
    BUILT-IN CORE MAPPINGS
***************************************************************************/

namespace {

#define CORE_INPUT_TYPES_P1 \
		CORE_INPUT_TYPES_BEGIN(p1) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq(KEYCODE_UP) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq(KEYCODE_LEFT) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq(KEYCODE_I) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq(KEYCODE_K) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq(KEYCODE_J) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq(KEYCODE_L) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq(KEYCODE_E) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq(KEYCODE_D) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq(KEYCODE_S) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq(KEYCODE_F) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq(KEYCODE_LCONTROL, input_seq::or_code, MOUSECODE_BUTTON1_INDEXED(0), input_seq::or_code, GUNCODE_BUTTON1_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq(KEYCODE_LALT, input_seq::or_code, MOUSECODE_BUTTON3_INDEXED(0), input_seq::or_code, GUNCODE_BUTTON2_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq(KEYCODE_SPACE, input_seq::or_code, MOUSECODE_BUTTON2_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq(KEYCODE_LSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq(KEYCODE_Z) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq(KEYCODE_X) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq(KEYCODE_C) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq(KEYCODE_V) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq(KEYCODE_B) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq(KEYCODE_N) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq(KEYCODE_M) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq(KEYCODE_COMMA) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq(KEYCODE_STOP) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq(KEYCODE_SLASH) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq(KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  START,               N_p("input-name", "%p Start"),               input_seq(KEYCODE_1) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  SELECT,              N_p("input-name", "%p Select"),              input_seq(KEYCODE_5) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P1_MAHJONG \
		CORE_INPUT_TYPES_BEGIN(p1_mahjong) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_A,           N_p("input-name", "%p Mahjong A"),           input_seq(KEYCODE_A_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_B,           N_p("input-name", "%p Mahjong B"),           input_seq(KEYCODE_B_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_C,           N_p("input-name", "%p Mahjong C"),           input_seq(KEYCODE_C_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_D,           N_p("input-name", "%p Mahjong D"),           input_seq(KEYCODE_D_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_E,           N_p("input-name", "%p Mahjong E"),           input_seq(KEYCODE_E_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_F,           N_p("input-name", "%p Mahjong F"),           input_seq(KEYCODE_F_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_G,           N_p("input-name", "%p Mahjong G"),           input_seq(KEYCODE_G_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_H,           N_p("input-name", "%p Mahjong H"),           input_seq(KEYCODE_H_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_I,           N_p("input-name", "%p Mahjong I"),           input_seq(KEYCODE_I_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_J,           N_p("input-name", "%p Mahjong J"),           input_seq(KEYCODE_J_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_K,           N_p("input-name", "%p Mahjong K"),           input_seq(KEYCODE_K_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_L,           N_p("input-name", "%p Mahjong L"),           input_seq(KEYCODE_L_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_M,           N_p("input-name", "%p Mahjong M"),           input_seq(KEYCODE_M_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_N,           N_p("input-name", "%p Mahjong N"),           input_seq(KEYCODE_N_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_O,           N_p("input-name", "%p Mahjong O"),           input_seq(KEYCODE_O_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_P,           N_p("input-name", "%p Mahjong P"),           input_seq(KEYCODE_COLON_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_Q,           N_p("input-name", "%p Mahjong Q"),           input_seq(KEYCODE_Q_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_KAN,         N_p("input-name", "%p Mahjong Kan"),         input_seq(KEYCODE_LCONTROL_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_PON,         N_p("input-name", "%p Mahjong Pon"),         input_seq(KEYCODE_LALT_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_CHI,         N_p("input-name", "%p Mahjong Chi"),         input_seq(KEYCODE_SPACE_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_REACH,       N_p("input-name", "%p Mahjong Reach"),       input_seq(KEYCODE_LSHIFT_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_RON,         N_p("input-name", "%p Mahjong Ron"),         input_seq(KEYCODE_Z_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_FLIP_FLOP,   N_p("input-name", "%p Mahjong Flip Flop"),   input_seq(KEYCODE_Y_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_BET,         N_p("input-name", "%p Mahjong Bet"),         input_seq(KEYCODE_3_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_SCORE,       N_p("input-name", "%p Mahjong Take Score"),  input_seq(KEYCODE_RCONTROL_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_DOUBLE_UP,   N_p("input-name", "%p Mahjong Double Up"),   input_seq(KEYCODE_RSHIFT_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_BIG,         N_p("input-name", "%p Mahjong Big"),         input_seq(KEYCODE_ENTER_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_SMALL,       N_p("input-name", "%p Mahjong Small"),       input_seq(KEYCODE_BACKSPACE_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  MAHJONG_LAST_CHANCE, N_p("input-name", "%p Mahjong Last Chance"), input_seq(KEYCODE_RALT_INDEXED(0)) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P1_HANAFUDA \
		CORE_INPUT_TYPES_BEGIN(p1_hanafuda) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_A,          N_p("input-name", "%p Hanafuda A/1"),        input_seq(KEYCODE_A_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_B,          N_p("input-name", "%p Hanafuda B/2"),        input_seq(KEYCODE_B_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_C,          N_p("input-name", "%p Hanafuda C/3"),        input_seq(KEYCODE_C_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_D,          N_p("input-name", "%p Hanafuda D/4"),        input_seq(KEYCODE_D_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_E,          N_p("input-name", "%p Hanafuda E/5"),        input_seq(KEYCODE_E_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_F,          N_p("input-name", "%p Hanafuda F/6"),        input_seq(KEYCODE_F_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_G,          N_p("input-name", "%p Hanafuda G/7"),        input_seq(KEYCODE_G_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_H,          N_p("input-name", "%p Hanafuda H/8"),        input_seq(KEYCODE_H_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_YES,        N_p("input-name", "%p Hanafuda Yes"),        input_seq(KEYCODE_M_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  HANAFUDA_NO,         N_p("input-name", "%p Hanafuda No"),         input_seq(KEYCODE_N_INDEXED(0)) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_GAMBLE \
		CORE_INPUT_TYPES_BEGIN(gamble) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_KEYIN,        N_p("input-name", "Key In"),                 input_seq(KEYCODE_Q) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_KEYOUT,       N_p("input-name", "Key Out"),                input_seq(KEYCODE_W) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_SERVICE,      N_p("input-name", "Service"),                input_seq(KEYCODE_9) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_BOOK,         N_p("input-name", "Book-Keeping"),           input_seq(KEYCODE_0) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_DOOR,         N_p("input-name", "Door"),                   input_seq(KEYCODE_O) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_PAYOUT,       N_p("input-name", "Payout"),                 input_seq(KEYCODE_I) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_BET,          N_p("input-name", "Bet"),                    input_seq(KEYCODE_M) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_DEAL,         N_p("input-name", "Deal"),                   input_seq(KEYCODE_2) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_STAND,        N_p("input-name", "Stand"),                  input_seq(KEYCODE_L) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_TAKE,         N_p("input-name", "Take Score"),             input_seq(KEYCODE_4) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_D_UP,         N_p("input-name", "Double Up"),              input_seq(KEYCODE_3) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_HALF,         N_p("input-name", "Half Gamble"),            input_seq(KEYCODE_D) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_HIGH,         N_p("input-name", "High"),                   input_seq(KEYCODE_A) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  GAMBLE_LOW,          N_p("input-name", "Low"),                    input_seq(KEYCODE_S) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_POKER \
		CORE_INPUT_TYPES_BEGIN(poker) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  POKER_HOLD1,         N_p("input-name", "Hold 1"),                 input_seq(KEYCODE_Z) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  POKER_HOLD2,         N_p("input-name", "Hold 2"),                 input_seq(KEYCODE_X) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  POKER_HOLD3,         N_p("input-name", "Hold 3"),                 input_seq(KEYCODE_C) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  POKER_HOLD4,         N_p("input-name", "Hold 4"),                 input_seq(KEYCODE_V) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  POKER_HOLD5,         N_p("input-name", "Hold 5"),                 input_seq(KEYCODE_B) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  POKER_CANCEL,        N_p("input-name", "Cancel"),                 input_seq(KEYCODE_N) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_SLOT \
		CORE_INPUT_TYPES_BEGIN(slot) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  SLOT_STOP1,          N_p("input-name", "Stop Reel 1"),            input_seq(KEYCODE_X) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  SLOT_STOP2,          N_p("input-name", "Stop Reel 2"),            input_seq(KEYCODE_C) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  SLOT_STOP3,          N_p("input-name", "Stop Reel 3"),            input_seq(KEYCODE_V) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  SLOT_STOP4,          N_p("input-name", "Stop Reel 4"),            input_seq(KEYCODE_B) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  SLOT_STOP5,          N_p("input-name", "Stop Reel 5"),            input_seq(KEYCODE_N) ) \
		INPUT_PORT_DIGITAL_TYPE(  1, PLAYER1,  SLOT_STOP_ALL,       N_p("input-name", "Stop All Reels"),         input_seq(KEYCODE_Z) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P2 \
		CORE_INPUT_TYPES_BEGIN(p2) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq(KEYCODE_R) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq(KEYCODE_F) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq(KEYCODE_D) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq(KEYCODE_G) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq(KEYCODE_A, input_seq::or_code, MOUSECODE_BUTTON1_INDEXED(1), input_seq::or_code, GUNCODE_BUTTON1_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq(KEYCODE_S, input_seq::or_code, MOUSECODE_BUTTON3_INDEXED(1), input_seq::or_code, GUNCODE_BUTTON2_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq(KEYCODE_Q, input_seq::or_code, MOUSECODE_BUTTON2_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq(KEYCODE_W) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq(KEYCODE_E) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  START,               N_p("input-name", "%p Start"),               input_seq(KEYCODE_2) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  SELECT,              N_p("input-name", "%p Select"),              input_seq(KEYCODE_6) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P2_MAHJONG \
		CORE_INPUT_TYPES_BEGIN(p2_mahjong) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_A,           N_p("input-name", "%p Mahjong A"),           input_seq(KEYCODE_A_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_B,           N_p("input-name", "%p Mahjong B"),           input_seq(KEYCODE_B_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_C,           N_p("input-name", "%p Mahjong C"),           input_seq(KEYCODE_C_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_D,           N_p("input-name", "%p Mahjong D"),           input_seq(KEYCODE_D_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_E,           N_p("input-name", "%p Mahjong E"),           input_seq(KEYCODE_E_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_F,           N_p("input-name", "%p Mahjong F"),           input_seq(KEYCODE_F_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_G,           N_p("input-name", "%p Mahjong G"),           input_seq(KEYCODE_G_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_H,           N_p("input-name", "%p Mahjong H"),           input_seq(KEYCODE_H_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_I,           N_p("input-name", "%p Mahjong I"),           input_seq(KEYCODE_I_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_J,           N_p("input-name", "%p Mahjong J"),           input_seq(KEYCODE_J_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_K,           N_p("input-name", "%p Mahjong K"),           input_seq(KEYCODE_K_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_L,           N_p("input-name", "%p Mahjong L"),           input_seq(KEYCODE_L_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_M,           N_p("input-name", "%p Mahjong M"),           input_seq(KEYCODE_M_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_N,           N_p("input-name", "%p Mahjong N"),           input_seq(KEYCODE_N_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_O,           N_p("input-name", "%p Mahjong O"),           input_seq(KEYCODE_O_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_P,           N_p("input-name", "%p Mahjong P"),           input_seq(KEYCODE_COLON_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_Q,           N_p("input-name", "%p Mahjong Q"),           input_seq(KEYCODE_Q_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_KAN,         N_p("input-name", "%p Mahjong Kan"),         input_seq(KEYCODE_LCONTROL_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_PON,         N_p("input-name", "%p Mahjong Pon"),         input_seq(KEYCODE_LALT_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_CHI,         N_p("input-name", "%p Mahjong Chi"),         input_seq(KEYCODE_SPACE_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_REACH,       N_p("input-name", "%p Mahjong Reach"),       input_seq(KEYCODE_LSHIFT_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_RON,         N_p("input-name", "%p Mahjong Ron"),         input_seq(KEYCODE_Z_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_FLIP_FLOP,   N_p("input-name", "%p Mahjong Flip Flop"),   input_seq(KEYCODE_Y_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_BET,         N_p("input-name", "%p Mahjong Bet"),         input_seq(KEYCODE_3_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_SCORE,       N_p("input-name", "%p Mahjong Take Score"),  input_seq(KEYCODE_RCONTROL_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_DOUBLE_UP,   N_p("input-name", "%p Mahjong Double Up"),   input_seq(KEYCODE_RSHIFT_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_BIG,         N_p("input-name", "%p Mahjong Big"),         input_seq(KEYCODE_ENTER_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_SMALL,       N_p("input-name", "%p Mahjong Small"),       input_seq(KEYCODE_BACKSPACE_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  MAHJONG_LAST_CHANCE, N_p("input-name", "%p Mahjong Last Chance"), input_seq(KEYCODE_RALT_INDEXED(1)) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P2_HANAFUDA \
		CORE_INPUT_TYPES_BEGIN(p2_hanafuda) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_A,          N_p("input-name", "%p Hanafuda A/1"),        input_seq(KEYCODE_A_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_B,          N_p("input-name", "%p Hanafuda B/2"),        input_seq(KEYCODE_B_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_C,          N_p("input-name", "%p Hanafuda C/3"),        input_seq(KEYCODE_C_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_D,          N_p("input-name", "%p Hanafuda D/4"),        input_seq(KEYCODE_D_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_E,          N_p("input-name", "%p Hanafuda E/5"),        input_seq(KEYCODE_E_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_F,          N_p("input-name", "%p Hanafuda F/6"),        input_seq(KEYCODE_F_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_G,          N_p("input-name", "%p Hanafuda G/7"),        input_seq(KEYCODE_G_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_H,          N_p("input-name", "%p Hanafuda H/8"),        input_seq(KEYCODE_H_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_YES,        N_p("input-name", "%p Hanafuda Yes"),        input_seq(KEYCODE_M_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  2, PLAYER2,  HANAFUDA_NO,         N_p("input-name", "%p Hanafuda No"),         input_seq(KEYCODE_N_INDEXED(1)) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P3 \
		CORE_INPUT_TYPES_BEGIN(p3) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq(KEYCODE_I) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq(KEYCODE_K) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq(KEYCODE_J) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq(KEYCODE_L) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq(KEYCODE_RCONTROL, input_seq::or_code, GUNCODE_BUTTON1_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq(KEYCODE_RSHIFT, input_seq::or_code, GUNCODE_BUTTON2_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq(KEYCODE_ENTER) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  START,               N_p("input-name", "%p Start"),               input_seq(KEYCODE_3) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  SELECT,              N_p("input-name", "%p Select"),              input_seq(KEYCODE_7) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P3_MAHJONG \
		CORE_INPUT_TYPES_BEGIN(p3_mahjong) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_A,           N_p("input-name", "%p Mahjong A"),           input_seq(KEYCODE_A_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_B,           N_p("input-name", "%p Mahjong B"),           input_seq(KEYCODE_B_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_C,           N_p("input-name", "%p Mahjong C"),           input_seq(KEYCODE_C_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_D,           N_p("input-name", "%p Mahjong D"),           input_seq(KEYCODE_D_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_E,           N_p("input-name", "%p Mahjong E"),           input_seq(KEYCODE_E_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_F,           N_p("input-name", "%p Mahjong F"),           input_seq(KEYCODE_F_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_G,           N_p("input-name", "%p Mahjong G"),           input_seq(KEYCODE_G_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_H,           N_p("input-name", "%p Mahjong H"),           input_seq(KEYCODE_H_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_I,           N_p("input-name", "%p Mahjong I"),           input_seq(KEYCODE_I_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_J,           N_p("input-name", "%p Mahjong J"),           input_seq(KEYCODE_J_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_K,           N_p("input-name", "%p Mahjong K"),           input_seq(KEYCODE_K_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_L,           N_p("input-name", "%p Mahjong L"),           input_seq(KEYCODE_L_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_M,           N_p("input-name", "%p Mahjong M"),           input_seq(KEYCODE_M_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_N,           N_p("input-name", "%p Mahjong N"),           input_seq(KEYCODE_N_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_O,           N_p("input-name", "%p Mahjong O"),           input_seq(KEYCODE_O_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_P,           N_p("input-name", "%p Mahjong P"),           input_seq(KEYCODE_COLON_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_Q,           N_p("input-name", "%p Mahjong Q"),           input_seq(KEYCODE_Q_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_KAN,         N_p("input-name", "%p Mahjong Kan"),         input_seq(KEYCODE_LCONTROL_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_PON,         N_p("input-name", "%p Mahjong Pon"),         input_seq(KEYCODE_LALT_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_CHI,         N_p("input-name", "%p Mahjong Chi"),         input_seq(KEYCODE_SPACE_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_REACH,       N_p("input-name", "%p Mahjong Reach"),       input_seq(KEYCODE_LSHIFT_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_RON,         N_p("input-name", "%p Mahjong Ron"),         input_seq(KEYCODE_Z_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_FLIP_FLOP,   N_p("input-name", "%p Mahjong Flip Flop"),   input_seq(KEYCODE_Y_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_BET,         N_p("input-name", "%p Mahjong Bet"),         input_seq(KEYCODE_3_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_SCORE,       N_p("input-name", "%p Mahjong Take Score"),  input_seq(KEYCODE_RCONTROL_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_DOUBLE_UP,   N_p("input-name", "%p Mahjong Double Up"),   input_seq(KEYCODE_RSHIFT_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_BIG,         N_p("input-name", "%p Mahjong Big"),         input_seq(KEYCODE_ENTER_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_SMALL,       N_p("input-name", "%p Mahjong Small"),       input_seq(KEYCODE_BACKSPACE_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  3, PLAYER3,  MAHJONG_LAST_CHANCE, N_p("input-name", "%p Mahjong Last Chance"), input_seq(KEYCODE_RALT_INDEXED(2)) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P4 \
		CORE_INPUT_TYPES_BEGIN(p4) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq(KEYCODE_8_PAD) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq(KEYCODE_2_PAD) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq(KEYCODE_4_PAD) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq(KEYCODE_6_PAD) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq(KEYCODE_0_PAD) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq(KEYCODE_DEL_PAD) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq(KEYCODE_ENTER_PAD) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  START,               N_p("input-name", "%p Start"),               input_seq(KEYCODE_4) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  SELECT,              N_p("input-name", "%p Select"),              input_seq(KEYCODE_8) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P4_MAHJONG \
		CORE_INPUT_TYPES_BEGIN(p4_mahjong) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_A,           N_p("input-name", "%p Mahjong A"),           input_seq(KEYCODE_A_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_B,           N_p("input-name", "%p Mahjong B"),           input_seq(KEYCODE_B_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_C,           N_p("input-name", "%p Mahjong C"),           input_seq(KEYCODE_C_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_D,           N_p("input-name", "%p Mahjong D"),           input_seq(KEYCODE_D_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_E,           N_p("input-name", "%p Mahjong E"),           input_seq(KEYCODE_E_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_F,           N_p("input-name", "%p Mahjong F"),           input_seq(KEYCODE_F_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_G,           N_p("input-name", "%p Mahjong G"),           input_seq(KEYCODE_G_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_H,           N_p("input-name", "%p Mahjong H"),           input_seq(KEYCODE_H_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_I,           N_p("input-name", "%p Mahjong I"),           input_seq(KEYCODE_I_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_J,           N_p("input-name", "%p Mahjong J"),           input_seq(KEYCODE_J_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_K,           N_p("input-name", "%p Mahjong K"),           input_seq(KEYCODE_K_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_L,           N_p("input-name", "%p Mahjong L"),           input_seq(KEYCODE_L_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_M,           N_p("input-name", "%p Mahjong M"),           input_seq(KEYCODE_M_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_N,           N_p("input-name", "%p Mahjong N"),           input_seq(KEYCODE_N_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_O,           N_p("input-name", "%p Mahjong O"),           input_seq(KEYCODE_O_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_P,           N_p("input-name", "%p Mahjong P"),           input_seq(KEYCODE_COLON_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_Q,           N_p("input-name", "%p Mahjong Q"),           input_seq(KEYCODE_Q_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_KAN,         N_p("input-name", "%p Mahjong Kan"),         input_seq(KEYCODE_LCONTROL_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_PON,         N_p("input-name", "%p Mahjong Pon"),         input_seq(KEYCODE_LALT_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_CHI,         N_p("input-name", "%p Mahjong Chi"),         input_seq(KEYCODE_SPACE_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_REACH,       N_p("input-name", "%p Mahjong Reach"),       input_seq(KEYCODE_LSHIFT_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_RON,         N_p("input-name", "%p Mahjong Ron"),         input_seq(KEYCODE_Z_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_FLIP_FLOP,   N_p("input-name", "%p Mahjong Flip Flop"),   input_seq(KEYCODE_Y_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_BET,         N_p("input-name", "%p Mahjong Bet"),         input_seq(KEYCODE_3_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_SCORE,       N_p("input-name", "%p Mahjong Take Score"),  input_seq(KEYCODE_RCONTROL_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_DOUBLE_UP,   N_p("input-name", "%p Mahjong Double Up"),   input_seq(KEYCODE_RSHIFT_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_BIG,         N_p("input-name", "%p Mahjong Big"),         input_seq(KEYCODE_ENTER_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_SMALL,       N_p("input-name", "%p Mahjong Small"),       input_seq(KEYCODE_BACKSPACE_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  4, PLAYER4,  MAHJONG_LAST_CHANCE, N_p("input-name", "%p Mahjong Last Chance"), input_seq(KEYCODE_RALT_INDEXED(3)) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P5 \
		CORE_INPUT_TYPES_BEGIN(p5) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  START,               N_p("input-name", "%p Start"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  5, PLAYER5,  SELECT,              N_p("input-name", "%p Select"),              input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P6 \
		CORE_INPUT_TYPES_BEGIN(p6) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  START,               N_p("input-name", "%p Start"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  6, PLAYER6,  SELECT,              N_p("input-name", "%p Select"),              input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P7 \
		CORE_INPUT_TYPES_BEGIN(p7) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  START,               N_p("input-name", "%p Start"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  7, PLAYER7,  SELECT,              N_p("input-name", "%p Select"),              input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P8 \
		CORE_INPUT_TYPES_BEGIN(p8) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  START,               N_p("input-name", "%p Start"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  8, PLAYER8,  SELECT,              N_p("input-name", "%p Select"),              input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P9 \
		CORE_INPUT_TYPES_BEGIN(p9) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  START,               N_p("input-name", "%p Start"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  9, PLAYER9,  SELECT,              N_p("input-name", "%p Select"),              input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_P10 \
		CORE_INPUT_TYPES_BEGIN(p10) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICK_UP,         N_p("input-name", "%p Up"),                  input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICK_DOWN,       N_p("input-name", "%p Down"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICK_LEFT,       N_p("input-name", "%p Left"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICK_RIGHT,      N_p("input-name", "%p Right"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKRIGHT_UP,    N_p("input-name", "%p Right Stick/Up"),      input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKRIGHT_DOWN,  N_p("input-name", "%p Right Stick/Down"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKRIGHT_LEFT,  N_p("input-name", "%p Right Stick/Left"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKRIGHT_RIGHT, N_p("input-name", "%p Right Stick/Right"),   input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKLEFT_UP,     N_p("input-name", "%p Left Stick/Up"),       input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKLEFT_DOWN,   N_p("input-name", "%p Left Stick/Down"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKLEFT_LEFT,   N_p("input-name", "%p Left Stick/Left"),     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, JOYSTICKLEFT_RIGHT,  N_p("input-name", "%p Left Stick/Right"),    input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON1,             N_p("input-name", "%p Button 1"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON2,             N_p("input-name", "%p Button 2"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON3,             N_p("input-name", "%p Button 3"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON4,             N_p("input-name", "%p Button 4"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON5,             N_p("input-name", "%p Button 5"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON6,             N_p("input-name", "%p Button 6"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON7,             N_p("input-name", "%p Button 7"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON8,             N_p("input-name", "%p Button 8"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON9,             N_p("input-name", "%p Button 9"),            input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON10,            N_p("input-name", "%p Button 10"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON11,            N_p("input-name", "%p Button 11"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON12,            N_p("input-name", "%p Button 12"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON13,            N_p("input-name", "%p Button 13"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON14,            N_p("input-name", "%p Button 14"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON15,            N_p("input-name", "%p Button 15"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, BUTTON16,            N_p("input-name", "%p Button 16"),           input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, START,               N_p("input-name", "%p Start"),               input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 10, PLAYER10, SELECT,              N_p("input-name", "%p Select"),              input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_START \
		CORE_INPUT_TYPES_BEGIN(start) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    START1,              N_p("input-name", "1 Player Start"),         input_seq(KEYCODE_1, input_seq::or_code, JOYCODE_START_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    START2,              N_p("input-name", "2 Players Start"),        input_seq(KEYCODE_2, input_seq::or_code, JOYCODE_START_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    START3,              N_p("input-name", "3 Players Start"),        input_seq(KEYCODE_3, input_seq::or_code, JOYCODE_START_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    START4,              N_p("input-name", "4 Players Start"),        input_seq(KEYCODE_4, input_seq::or_code, JOYCODE_START_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    START5,              N_p("input-name", "5 Players Start"),        input_seq(JOYCODE_START_INDEXED(4)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    START6,              N_p("input-name", "6 Players Start"),        input_seq(JOYCODE_START_INDEXED(5)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    START7,              N_p("input-name", "7 Players Start"),        input_seq(JOYCODE_START_INDEXED(6)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    START8,              N_p("input-name", "8 Players Start"),        input_seq(JOYCODE_START_INDEXED(7)) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_COIN \
		CORE_INPUT_TYPES_BEGIN(coin) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN1,               N_p("input-name", "Coin 1"),                 input_seq(KEYCODE_5, input_seq::or_code, JOYCODE_SELECT_INDEXED(0)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN2,               N_p("input-name", "Coin 2"),                 input_seq(KEYCODE_6, input_seq::or_code, JOYCODE_SELECT_INDEXED(1)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN3,               N_p("input-name", "Coin 3"),                 input_seq(KEYCODE_7, input_seq::or_code, JOYCODE_SELECT_INDEXED(2)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN4,               N_p("input-name", "Coin 4"),                 input_seq(KEYCODE_8, input_seq::or_code, JOYCODE_SELECT_INDEXED(3)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN5,               N_p("input-name", "Coin 5"),                 input_seq(JOYCODE_SELECT_INDEXED(4)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN6,               N_p("input-name", "Coin 6"),                 input_seq(JOYCODE_SELECT_INDEXED(5)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN7,               N_p("input-name", "Coin 7"),                 input_seq(JOYCODE_SELECT_INDEXED(6)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN8,               N_p("input-name", "Coin 8"),                 input_seq(JOYCODE_SELECT_INDEXED(7)) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN9,               N_p("input-name", "Coin 9"),                 input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN10,              N_p("input-name", "Coin 10"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN11,              N_p("input-name", "Coin 11"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    COIN12,              N_p("input-name", "Coin 12"),                input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    BILL1,               N_p("input-name", "Banknote 1"),             input_seq(KEYCODE_BACKSPACE) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_SERVICE \
		CORE_INPUT_TYPES_BEGIN(service) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    SERVICE1,            N_p("input-name", "Service 1"),              input_seq(KEYCODE_9) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    SERVICE2,            N_p("input-name", "Service 2"),              input_seq(KEYCODE_0) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    SERVICE3,            N_p("input-name", "Service 3"),              input_seq(KEYCODE_MINUS) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    SERVICE4,            N_p("input-name", "Service 4"),              input_seq(KEYCODE_EQUALS) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_TILT \
		CORE_INPUT_TYPES_BEGIN(tilt) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    TILT1,               N_p("input-name", "Tilt 1"),                 input_seq(KEYCODE_T) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    TILT2,               N_p("input-name", "Tilt 2"),                 input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    TILT3,               N_p("input-name", "Tilt 3"),                 input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    TILT4,               N_p("input-name", "Tilt 4"),                 input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_OTHER \
		CORE_INPUT_TYPES_BEGIN(other) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    POWER_ON,            N_p("input-name", "Power On"),               input_seq(KEYCODE_F1) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    POWER_OFF,           N_p("input-name", "Power Off"),              input_seq(KEYCODE_F2) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    SERVICE,             N_p("input-name", "Service"),                input_seq(KEYCODE_F2) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    TILT,                N_p("input-name", "Tilt"),                   input_seq(KEYCODE_T) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    INTERLOCK,           N_p("input-name", "Door Interlock"),         input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    MEMORY_RESET,        N_p("input-name", "Memory Reset"),           input_seq(KEYCODE_F1) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    VOLUME_DOWN,         N_p("input-name", "Volume Down"),            input_seq(KEYCODE_MINUS) ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,    VOLUME_UP,           N_p("input-name", "Volume Up"),              input_seq(KEYCODE_EQUALS) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_PEDAL \
		CORE_INPUT_TYPES_BEGIN(pedal) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq(KEYCODE_LCONTROL) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq(KEYCODE_A) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq(KEYCODE_RCONTROL) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq(KEYCODE_0_PAD) ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, PEDAL,               N_p("input-name", "%p Pedal 1"),             input_seq(), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_PEDAL2 \
		CORE_INPUT_TYPES_BEGIN(pedal2) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq(KEYCODE_LALT) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq(KEYCODE_S) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq(KEYCODE_RSHIFT) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq(KEYCODE_DEL_PAD) ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, PEDAL2,              N_p("input-name", "%p Pedal 2"),             input_seq(), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_PEDAL3 \
		CORE_INPUT_TYPES_BEGIN(pedal3) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq(KEYCODE_SPACE) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq(KEYCODE_Q) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq(KEYCODE_ENTER) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq(KEYCODE_ENTER_PAD) ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, PEDAL3,              N_p("input-name", "%p Pedal 3"),             input_seq(), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_PADDLE \
		CORE_INPUT_TYPES_BEGIN(paddle) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  PADDLE,              N_p("input-name", "Paddle"),                 input_seq(MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  PADDLE,              N_p("input-name", "Paddle 2"),               input_seq(MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  PADDLE,              N_p("input-name", "Paddle 3"),               input_seq(MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  PADDLE,              N_p("input-name", "Paddle 4"),               input_seq(MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  PADDLE,              N_p("input-name", "Paddle 5"),               input_seq(MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  PADDLE,              N_p("input-name", "Paddle 6"),               input_seq(MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  PADDLE,              N_p("input-name", "Paddle 7"),               input_seq(MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  PADDLE,              N_p("input-name", "Paddle 8"),               input_seq(MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  PADDLE,              N_p("input-name", "Paddle 9"),               input_seq(MOUSECODE_X_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, PADDLE,              N_p("input-name", "Paddle 10"),              input_seq(MOUSECODE_X_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_PADDLE_V \
		CORE_INPUT_TYPES_BEGIN(paddle_v) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  PADDLE_V,            N_p("input-name", "Paddle V"),               input_seq(MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  PADDLE_V,            N_p("input-name", "Paddle V 2"),             input_seq(MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  PADDLE_V,            N_p("input-name", "Paddle V 3"),             input_seq(MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  PADDLE_V,            N_p("input-name", "Paddle V 4"),             input_seq(MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  PADDLE_V,            N_p("input-name", "Paddle V 5"),             input_seq(MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  PADDLE_V,            N_p("input-name", "Paddle V 6"),             input_seq(MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  PADDLE_V,            N_p("input-name", "Paddle V 7"),             input_seq(MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  PADDLE_V,            N_p("input-name", "Paddle V 8"),             input_seq(MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  PADDLE_V,            N_p("input-name", "Paddle V 9"),             input_seq(MOUSECODE_Y_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, PADDLE_V,            N_p("input-name", "Paddle V 10"),            input_seq(MOUSECODE_Y_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_POSITIONAL \
		CORE_INPUT_TYPES_BEGIN(positional) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  POSITIONAL,          N_p("input-name", "Positional"),             input_seq(MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  POSITIONAL,          N_p("input-name", "Positional 2"),           input_seq(MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  POSITIONAL,          N_p("input-name", "Positional 3"),           input_seq(MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  POSITIONAL,          N_p("input-name", "Positional 4"),           input_seq(MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  POSITIONAL,          N_p("input-name", "Positional 5"),           input_seq(MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  POSITIONAL,          N_p("input-name", "Positional 6"),           input_seq(MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  POSITIONAL,          N_p("input-name", "Positional 7"),           input_seq(MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  POSITIONAL,          N_p("input-name", "Positional 8"),           input_seq(MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  POSITIONAL,          N_p("input-name", "Positional 9"),           input_seq(MOUSECODE_X_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, POSITIONAL,          N_p("input-name", "Positional 10"),          input_seq(MOUSECODE_X_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_POSITIONAL_V \
		CORE_INPUT_TYPES_BEGIN(positional_v) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  POSITIONAL_V,        N_p("input-name", "Positional V"),           input_seq(MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  POSITIONAL_V,        N_p("input-name", "Positional V 2"),         input_seq(MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  POSITIONAL_V,        N_p("input-name", "Positional V 3"),         input_seq(MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  POSITIONAL_V,        N_p("input-name", "Positional V 4"),         input_seq(MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  POSITIONAL_V,        N_p("input-name", "Positional V 5"),         input_seq(MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  POSITIONAL_V,        N_p("input-name", "Positional V 6"),         input_seq(MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  POSITIONAL_V,        N_p("input-name", "Positional V 7"),         input_seq(MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  POSITIONAL_V,        N_p("input-name", "Positional V 8"),         input_seq(MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  POSITIONAL_V,        N_p("input-name", "Positional V 9"),         input_seq(MOUSECODE_Y_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, POSITIONAL_V,        N_p("input-name", "Positional V 10"),        input_seq(MOUSECODE_Y_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_DIAL \
		CORE_INPUT_TYPES_BEGIN(dial) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  DIAL,                N_p("input-name", "Dial"),                   input_seq(MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  DIAL,                N_p("input-name", "Dial 2"),                 input_seq(MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  DIAL,                N_p("input-name", "Dial 3"),                 input_seq(MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  DIAL,                N_p("input-name", "Dial 4"),                 input_seq(MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  DIAL,                N_p("input-name", "Dial 5"),                 input_seq(MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  DIAL,                N_p("input-name", "Dial 6"),                 input_seq(MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  DIAL,                N_p("input-name", "Dial 7"),                 input_seq(MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  DIAL,                N_p("input-name", "Dial 8"),                 input_seq(MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  DIAL,                N_p("input-name", "Dial 9"),                 input_seq(MOUSECODE_X_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, DIAL,                N_p("input-name", "Dial 10"),                input_seq(MOUSECODE_X_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_DIAL_V \
		CORE_INPUT_TYPES_BEGIN(dial_v) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  DIAL_V,              N_p("input-name", "Dial V"),                 input_seq(MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  DIAL_V,              N_p("input-name", "Dial V 2"),               input_seq(MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  DIAL_V,              N_p("input-name", "Dial V 3"),               input_seq(MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  DIAL_V,              N_p("input-name", "Dial V 4"),               input_seq(MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  DIAL_V,              N_p("input-name", "Dial V 5"),               input_seq(MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  DIAL_V,              N_p("input-name", "Dial V 6"),               input_seq(MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  DIAL_V,              N_p("input-name", "Dial V 7"),               input_seq(MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  DIAL_V,              N_p("input-name", "Dial V 8"),               input_seq(MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  DIAL_V,              N_p("input-name", "Dial V 9"),               input_seq(MOUSECODE_Y_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, DIAL_V,              N_p("input-name", "Dial V 10"),              input_seq(MOUSECODE_Y_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_TRACKBALL_X \
		CORE_INPUT_TYPES_BEGIN(trackball_x) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  TRACKBALL_X,         N_p("input-name", "Trackball X"),            input_seq(MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  TRACKBALL_X,         N_p("input-name", "Trackball X 2"),          input_seq(MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  TRACKBALL_X,         N_p("input-name", "Trackball X 3"),          input_seq(MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  TRACKBALL_X,         N_p("input-name", "Trackball X 4"),          input_seq(MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  TRACKBALL_X,         N_p("input-name", "Trackball X 5"),          input_seq(MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  TRACKBALL_X,         N_p("input-name", "Trackball X 6"),          input_seq(MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  TRACKBALL_X,         N_p("input-name", "Trackball X 7"),          input_seq(MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  TRACKBALL_X,         N_p("input-name", "Trackball X 8"),          input_seq(MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  TRACKBALL_X,         N_p("input-name", "Trackball X 9"),          input_seq(MOUSECODE_X_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, TRACKBALL_X,         N_p("input-name", "Trackball X 10"),         input_seq(MOUSECODE_X_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_TRACKBALL_Y \
		CORE_INPUT_TYPES_BEGIN(trackball_y) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  TRACKBALL_Y,         N_p("input-name", "Trackball Y"),            input_seq(MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  TRACKBALL_Y,         N_p("input-name", "Trackball Y 2"),          input_seq(MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  TRACKBALL_Y,         N_p("input-name", "Trackball Y 3"),          input_seq(MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  TRACKBALL_Y,         N_p("input-name", "Trackball Y 4"),          input_seq(MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  TRACKBALL_Y,         N_p("input-name", "Trackball Y 5"),          input_seq(MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  TRACKBALL_Y,         N_p("input-name", "Trackball Y 6"),          input_seq(MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  TRACKBALL_Y,         N_p("input-name", "Trackball Y 7"),          input_seq(MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  TRACKBALL_Y,         N_p("input-name", "Trackball Y 8"),          input_seq(MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  TRACKBALL_Y,         N_p("input-name", "Trackball Y 9"),          input_seq(MOUSECODE_Y_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, TRACKBALL_Y,         N_p("input-name", "Trackball Y 10"),         input_seq(MOUSECODE_Y_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_AD_STICK_X \
		CORE_INPUT_TYPES_BEGIN(ad_stick_x) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  AD_STICK_X,          N_p("input-name", "AD Stick X"),             input_seq(MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  AD_STICK_X,          N_p("input-name", "AD Stick X 2"),           input_seq(MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  AD_STICK_X,          N_p("input-name", "AD Stick X 3"),           input_seq(MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  AD_STICK_X,          N_p("input-name", "AD Stick X 4"),           input_seq(MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  AD_STICK_X,          N_p("input-name", "AD Stick X 5"),           input_seq(MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  AD_STICK_X,          N_p("input-name", "AD Stick X 6"),           input_seq(MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  AD_STICK_X,          N_p("input-name", "AD Stick X 7"),           input_seq(MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  AD_STICK_X,          N_p("input-name", "AD Stick X 8"),           input_seq(MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  AD_STICK_X,          N_p("input-name", "AD Stick X 9"),           input_seq(MOUSECODE_X_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, AD_STICK_X,          N_p("input-name", "AD Stick X 10"),          input_seq(MOUSECODE_X_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_AD_STICK_Y \
		CORE_INPUT_TYPES_BEGIN(ad_stick_y) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  AD_STICK_Y,          N_p("input-name", "AD Stick Y"),             input_seq(MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  AD_STICK_Y,          N_p("input-name", "AD Stick Y 2"),           input_seq(MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  AD_STICK_Y,          N_p("input-name", "AD Stick Y 3"),           input_seq(MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  AD_STICK_Y,          N_p("input-name", "AD Stick Y 4"),           input_seq(MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  AD_STICK_Y,          N_p("input-name", "AD Stick Y 5"),           input_seq(MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  AD_STICK_Y,          N_p("input-name", "AD Stick Y 6"),           input_seq(MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  AD_STICK_Y,          N_p("input-name", "AD Stick Y 7"),           input_seq(MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  AD_STICK_Y,          N_p("input-name", "AD Stick Y 8"),           input_seq(MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  AD_STICK_Y,          N_p("input-name", "AD Stick Y 9"),           input_seq(MOUSECODE_Y_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, AD_STICK_Y,          N_p("input-name", "AD Stick Y 10"),          input_seq(MOUSECODE_Y_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_AD_STICK_Z \
		CORE_INPUT_TYPES_BEGIN(ad_stick_z) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  AD_STICK_Z,          N_p("input-name", "AD Stick Z"),             input_seq(), input_seq(KEYCODE_A), input_seq(KEYCODE_Z) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  AD_STICK_Z,          N_p("input-name", "AD Stick Z 2"),           input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  AD_STICK_Z,          N_p("input-name", "AD Stick Z 3"),           input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  AD_STICK_Z,          N_p("input-name", "AD Stick Z 4"),           input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  AD_STICK_Z,          N_p("input-name", "AD Stick Z 5"),           input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  AD_STICK_Z,          N_p("input-name", "AD Stick Z 6"),           input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  AD_STICK_Z,          N_p("input-name", "AD Stick Z 7"),           input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  AD_STICK_Z,          N_p("input-name", "AD Stick Z 8"),           input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  AD_STICK_Z,          N_p("input-name", "AD Stick Z 9"),           input_seq(), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, AD_STICK_Z,          N_p("input-name", "AD Stick Z 10"),          input_seq(), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_LIGHTGUN_X \
		CORE_INPUT_TYPES_BEGIN(lightgun_x) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  LIGHTGUN_X,          N_p("input-name", "Lightgun X"),             input_seq(GUNCODE_X_INDEXED(0), input_seq::or_code, MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  LIGHTGUN_X,          N_p("input-name", "Lightgun X 2"),           input_seq(GUNCODE_X_INDEXED(1), input_seq::or_code, MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  LIGHTGUN_X,          N_p("input-name", "Lightgun X 3"),           input_seq(GUNCODE_X_INDEXED(2), input_seq::or_code, MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  LIGHTGUN_X,          N_p("input-name", "Lightgun X 4"),           input_seq(GUNCODE_X_INDEXED(3), input_seq::or_code, MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  LIGHTGUN_X,          N_p("input-name", "Lightgun X 5"),           input_seq(GUNCODE_X_INDEXED(4), input_seq::or_code, MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  LIGHTGUN_X,          N_p("input-name", "Lightgun X 6"),           input_seq(GUNCODE_X_INDEXED(5), input_seq::or_code, MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  LIGHTGUN_X,          N_p("input-name", "Lightgun X 7"),           input_seq(GUNCODE_X_INDEXED(6), input_seq::or_code, MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  LIGHTGUN_X,          N_p("input-name", "Lightgun X 8"),           input_seq(GUNCODE_X_INDEXED(7), input_seq::or_code, MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  LIGHTGUN_X,          N_p("input-name", "Lightgun X 9"),           input_seq(GUNCODE_X_INDEXED(8), input_seq::or_code, MOUSECODE_X_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, LIGHTGUN_X,          N_p("input-name", "Lightgun X 10"),          input_seq(GUNCODE_X_INDEXED(9), input_seq::or_code, MOUSECODE_X_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_LIGHTGUN_Y \
		CORE_INPUT_TYPES_BEGIN(lightgun_y) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y"),             input_seq(GUNCODE_Y_INDEXED(0), input_seq::or_code, MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 2"),           input_seq(GUNCODE_Y_INDEXED(1), input_seq::or_code, MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 3"),           input_seq(GUNCODE_Y_INDEXED(2), input_seq::or_code, MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 4"),           input_seq(GUNCODE_Y_INDEXED(3), input_seq::or_code, MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 5"),           input_seq(GUNCODE_Y_INDEXED(4), input_seq::or_code, MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 6"),           input_seq(GUNCODE_Y_INDEXED(5), input_seq::or_code, MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 7"),           input_seq(GUNCODE_Y_INDEXED(6), input_seq::or_code, MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 8"),           input_seq(GUNCODE_Y_INDEXED(7), input_seq::or_code, MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 9"),           input_seq(GUNCODE_Y_INDEXED(8), input_seq::or_code, MOUSECODE_Y_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, LIGHTGUN_Y,          N_p("input-name", "Lightgun Y 10"),          input_seq(GUNCODE_Y_INDEXED(9), input_seq::or_code, MOUSECODE_Y_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_MOUSE_X \
		CORE_INPUT_TYPES_BEGIN(mouse_x) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  MOUSE_X,             N_p("input-name", "Mouse X"),                input_seq(MOUSECODE_X_INDEXED(0)), input_seq(KEYCODE_LEFT), input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  MOUSE_X,             N_p("input-name", "Mouse X 2"),              input_seq(MOUSECODE_X_INDEXED(1)), input_seq(KEYCODE_D), input_seq(KEYCODE_G) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  MOUSE_X,             N_p("input-name", "Mouse X 3"),              input_seq(MOUSECODE_X_INDEXED(2)), input_seq(KEYCODE_J), input_seq(KEYCODE_L) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  MOUSE_X,             N_p("input-name", "Mouse X 4"),              input_seq(MOUSECODE_X_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  MOUSE_X,             N_p("input-name", "Mouse X 5"),              input_seq(MOUSECODE_X_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  MOUSE_X,             N_p("input-name", "Mouse X 6"),              input_seq(MOUSECODE_X_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  MOUSE_X,             N_p("input-name", "Mouse X 7"),              input_seq(MOUSECODE_X_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  MOUSE_X,             N_p("input-name", "Mouse X 8"),              input_seq(MOUSECODE_X_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  MOUSE_X,             N_p("input-name", "Mouse X 9"),              input_seq(MOUSECODE_X_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, MOUSE_X,             N_p("input-name", "Mouse X 10"),             input_seq(MOUSECODE_X_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_MOUSE_Y \
		CORE_INPUT_TYPES_BEGIN(mouse_y) \
		INPUT_PORT_ANALOG_TYPE(   1, PLAYER1,  MOUSE_Y,             N_p("input-name", "Mouse Y"),                input_seq(MOUSECODE_Y_INDEXED(0)), input_seq(KEYCODE_UP), input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_ANALOG_TYPE(   2, PLAYER2,  MOUSE_Y,             N_p("input-name", "Mouse Y 2"),              input_seq(MOUSECODE_Y_INDEXED(1)), input_seq(KEYCODE_R), input_seq(KEYCODE_F) ) \
		INPUT_PORT_ANALOG_TYPE(   3, PLAYER3,  MOUSE_Y,             N_p("input-name", "Mouse Y 3"),              input_seq(MOUSECODE_Y_INDEXED(2)), input_seq(KEYCODE_I), input_seq(KEYCODE_K) ) \
		INPUT_PORT_ANALOG_TYPE(   4, PLAYER4,  MOUSE_Y,             N_p("input-name", "Mouse Y 4"),              input_seq(MOUSECODE_Y_INDEXED(3)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   5, PLAYER5,  MOUSE_Y,             N_p("input-name", "Mouse Y 5"),              input_seq(MOUSECODE_Y_INDEXED(4)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   6, PLAYER6,  MOUSE_Y,             N_p("input-name", "Mouse Y 6"),              input_seq(MOUSECODE_Y_INDEXED(5)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   7, PLAYER7,  MOUSE_Y,             N_p("input-name", "Mouse Y 7"),              input_seq(MOUSECODE_Y_INDEXED(6)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   8, PLAYER8,  MOUSE_Y,             N_p("input-name", "Mouse Y 8"),              input_seq(MOUSECODE_Y_INDEXED(7)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(   9, PLAYER9,  MOUSE_Y,             N_p("input-name", "Mouse Y 9"),              input_seq(MOUSECODE_Y_INDEXED(8)), input_seq(), input_seq() ) \
		INPUT_PORT_ANALOG_TYPE(  10, PLAYER10, MOUSE_Y,             N_p("input-name", "Mouse Y 10"),             input_seq(MOUSECODE_Y_INDEXED(9)), input_seq(), input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_KEYPAD \
		CORE_INPUT_TYPES_BEGIN(keypad) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,   KEYPAD,               N_p("input-name", "Keypad"),                 input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE(  0, OTHER,   KEYBOARD,             N_p("input-name", "Keyboard"),               input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_UI \
		CORE_INPUT_TYPES_BEGIN(ui) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_MENU,              N_p("input-name", "Show/Hide Menu"),         input_seq(KEYCODE_TAB) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_SELECT,            N_p("input-name", "UI Select"),              input_seq(KEYCODE_ENTER, input_seq::not_code, KEYCODE_LALT, input_seq::not_code, KEYCODE_RALT, input_seq::or_code, KEYCODE_ENTER_PAD) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_BACK,              N_p("input-name", "UI Back"),                input_seq(KEYCODE_ESC) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_CANCEL,            N_p("input-name", "UI Cancel"),              input_seq(KEYCODE_ESC) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_CLEAR,             N_p("input-name", "UI Clear"),               input_seq(KEYCODE_DEL) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_HELP,              N_p("input-name", "UI Help"),                input_seq(KEYCODE_F1) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_UP,                N_p("input-name", "UI Up"),                  input_seq(KEYCODE_UP) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_DOWN,              N_p("input-name", "UI Down"),                input_seq(KEYCODE_DOWN) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_LEFT,              N_p("input-name", "UI Left"),                input_seq(KEYCODE_LEFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_RIGHT,             N_p("input-name", "UI Right"),               input_seq(KEYCODE_RIGHT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_HOME,              N_p("input-name", "UI Home"),                input_seq(KEYCODE_HOME) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_END,               N_p("input-name", "UI End"),                 input_seq(KEYCODE_END) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_PAGE_UP,           N_p("input-name", "UI Page Up"),             input_seq(KEYCODE_PGUP) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_PAGE_DOWN,         N_p("input-name", "UI Page Down"),           input_seq(KEYCODE_PGDN) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_PREV_GROUP,        N_p("input-name", "UI Previous Group"),      input_seq(KEYCODE_OPENBRACE) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_NEXT_GROUP,        N_p("input-name", "UI Next Group"),          input_seq(KEYCODE_CLOSEBRACE) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_ON_SCREEN_DISPLAY, N_p("input-name", "On Screen Display"),      input_seq(KEYCODE_TILDE) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_TOGGLE_UI,         N_p("input-name", "Toggle UI Controls"),     input_seq(KEYCODE_SCRLOCK, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_DEBUG_BREAK,       N_p("input-name", "Break in Debugger"),      input_seq(KEYCODE_TILDE) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_PAUSE,             N_p("input-name", "Pause"),                  input_seq(KEYCODE_F5, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_PAUSE_SINGLE,      N_p("input-name", "Pause - Single Step"),    input_seq(KEYCODE_F5, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_F5, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_REWIND_SINGLE,     N_p("input-name", "Rewind - Single Step"),   input_seq(KEYCODE_F4, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_F4, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_SAVE_STATE,        N_p("input-name", "Save State"),             input_seq(KEYCODE_F6, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_SAVE_STATE_QUICK,  N_p("input-name", "Quick Save State"),       input_seq(KEYCODE_F6, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_F6, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_LOAD_STATE,        N_p("input-name", "Load State"),             input_seq(KEYCODE_F7, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_LOAD_STATE_QUICK,  N_p("input-name", "Quick Load State"),       input_seq(KEYCODE_F7, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_F7, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_RESET_MACHINE,     N_p("input-name", "Reset Machine"),          input_seq(KEYCODE_F3, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_F3, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_SOFT_RESET,        N_p("input-name", "Soft Reset"),             input_seq(KEYCODE_F3, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_SHOW_GFX,          N_p("input-name", "Show Decoded Graphics"),  input_seq(KEYCODE_F4, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_FRAMESKIP_DEC,     N_p("input-name", "Frameskip Dec"),          input_seq(KEYCODE_F8, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_FRAMESKIP_INC,     N_p("input-name", "Frameskip Inc"),          input_seq(KEYCODE_F9) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_THROTTLE,          N_p("input-name", "Throttle"),               input_seq(KEYCODE_F10) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_FAST_FORWARD,      N_p("input-name", "Fast Forward"),           input_seq(KEYCODE_INSERT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_SHOW_FPS,          N_p("input-name", "Show FPS"),               input_seq(KEYCODE_F11, input_seq::not_code, KEYCODE_LSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_SNAPSHOT,          N_p("input-name", "Save Snapshot"),          input_seq(KEYCODE_F12, input_seq::not_code, KEYCODE_LSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_RECORD_MNG,        N_p("input-name", "Record MNG"),             input_seq(KEYCODE_F12, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_LCONTROL) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_RECORD_AVI,        N_p("input-name", "Record AVI"),             input_seq(KEYCODE_F12, KEYCODE_LSHIFT, KEYCODE_LCONTROL) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_TOGGLE_CHEAT,      N_p("input-name", "Toggle Cheat"),           input_seq(KEYCODE_F8, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_F8, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_DISPLAY_COMMENT,   N_p("input-name", "UI Display Comment"),     input_seq(KEYCODE_SPACE) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_ZOOM_IN,           N_p("input-name", "UI Zoom In"),             input_seq(KEYCODE_EQUALS) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_ZOOM_OUT,          N_p("input-name", "UI Zoom Out"),            input_seq(KEYCODE_MINUS) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_ZOOM_DEFAULT,      N_p("input-name", "UI Default Zoom"),        input_seq(KEYCODE_0) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_ROTATE,            N_p("input-name", "UI Rotate"),              input_seq(KEYCODE_R) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_SHOW_PROFILER,     N_p("input-name", "Show Profiler"),          input_seq(KEYCODE_F11, KEYCODE_LSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_RELEASE_POINTER,   N_p("input-name", "UI Release Pointer"),     input_seq(KEYCODE_RCONTROL, KEYCODE_RALT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_PASTE,             N_p("input-name", "UI Paste Text"),          input_seq(KEYCODE_SCRLOCK, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_SCRLOCK, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_TAPE_START,        N_p("input-name", "UI (First) Tape Start"),  input_seq(KEYCODE_F2, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_TAPE_STOP,         N_p("input-name", "UI (First) Tape Stop"),   input_seq(KEYCODE_F2, KEYCODE_LSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_FOCUS_NEXT,        N_p("input-name", "UI Focus Next"),          input_seq(KEYCODE_TAB, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_FOCUS_PREV,        N_p("input-name", "UI Focus Previous"),      input_seq(KEYCODE_TAB, KEYCODE_LSHIFT, input_seq::or_code, KEYCODE_TAB, KEYCODE_RSHIFT) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_DATS,              N_p("input-name", "UI External DAT View"),   input_seq(KEYCODE_LALT, KEYCODE_D) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_FAVORITES,         N_p("input-name", "UI Add/Remove Favorite"), input_seq(KEYCODE_LALT, KEYCODE_F) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_EXPORT,            N_p("input-name", "UI Export List"),         input_seq(KEYCODE_LALT, KEYCODE_E) ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       UI_AUDIT,             N_p("input-name", "UI Audit Media"),         input_seq(KEYCODE_F1, input_seq::not_code, KEYCODE_LSHIFT, input_seq::not_code, KEYCODE_RSHIFT) ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_OSD \
		CORE_INPUT_TYPES_BEGIN(osd) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_1,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_2,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_3,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_4,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_5,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_6,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_7,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_8,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_9,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_10,              nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_11,              nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_12,              nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_13,              nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_14,              nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_15,              nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, UI,       OSD_16,              nullptr,                     input_seq() ) \
		CORE_INPUT_TYPES_END()

#define CORE_INPUT_TYPES_INVALID \
		CORE_INPUT_TYPES_BEGIN(invalid) \
		INPUT_PORT_DIGITAL_TYPE( 0, INVALID,  UNKNOWN,             nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, INVALID,  UNUSED,              nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, INVALID,  SPECIAL,             nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, INVALID,  OTHER,               nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, INVALID,  ADJUSTER,            nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, INVALID,  DIPSWITCH,           nullptr,                     input_seq() ) \
		INPUT_PORT_DIGITAL_TYPE( 0, INVALID,  CONFIG,              nullptr,                     input_seq() ) \
		CORE_INPUT_TYPES_END()


ATTR_COLD void emplace_core_digital_type(std::vector<input_type_entry> &typelist, ioport_type type, ioport_group group, int player, const char *token, const char *name, input_seq seq)
{
	typelist.emplace_back(type, group, (player == 0) ? player : (player - 1), token, name, seq);
}

ATTR_COLD void emplace_core_analog_type(std::vector<input_type_entry> &typelist, ioport_type type, ioport_group group, int player, const char *token, const char *name, input_seq seq, input_seq decseq, input_seq incseq)
{
	typelist.emplace_back(type, group, (player == 0) ? player : (player - 1), token, name, seq, decseq, incseq);
}


// instantiate the contruct functions
#define CORE_INPUT_TYPES_BEGIN(_name) \
		ATTR_COLD inline void emplace_core_types_##_name(std::vector<input_type_entry> &typelist) \
		{
#define INPUT_PORT_DIGITAL_TYPE(_player, _group, _type, _name, _seq) \
		emplace_core_digital_type(typelist, IPT_##_type, IPG_##_group, _player, (_player == 0) ? #_type : ("P" #_player "_" #_type), _name, _seq);
#define INPUT_PORT_ANALOG_TYPE(_player, _group, _type, _name, _seq, _decseq, _incseq) \
		emplace_core_analog_type(typelist, IPT_##_type, IPG_##_group, _player, (_player == 0) ? #_type : ("P" #_player "_" #_type), _name, _seq, _decseq, _incseq);
#define CORE_INPUT_TYPES_END() \
		}
CORE_INPUT_TYPES_P1
CORE_INPUT_TYPES_P1_MAHJONG
CORE_INPUT_TYPES_P1_HANAFUDA
CORE_INPUT_TYPES_GAMBLE
CORE_INPUT_TYPES_POKER
CORE_INPUT_TYPES_SLOT
CORE_INPUT_TYPES_P2
CORE_INPUT_TYPES_P2_MAHJONG
CORE_INPUT_TYPES_P2_HANAFUDA
CORE_INPUT_TYPES_P3
CORE_INPUT_TYPES_P3_MAHJONG
CORE_INPUT_TYPES_P4
CORE_INPUT_TYPES_P4_MAHJONG
CORE_INPUT_TYPES_P5
CORE_INPUT_TYPES_P6
CORE_INPUT_TYPES_P7
CORE_INPUT_TYPES_P8
CORE_INPUT_TYPES_P9
CORE_INPUT_TYPES_P10
CORE_INPUT_TYPES_START
CORE_INPUT_TYPES_COIN
CORE_INPUT_TYPES_SERVICE
CORE_INPUT_TYPES_TILT
CORE_INPUT_TYPES_OTHER
CORE_INPUT_TYPES_PEDAL
CORE_INPUT_TYPES_PEDAL2
CORE_INPUT_TYPES_PEDAL3
CORE_INPUT_TYPES_PADDLE
CORE_INPUT_TYPES_PADDLE_V
CORE_INPUT_TYPES_POSITIONAL
CORE_INPUT_TYPES_POSITIONAL_V
CORE_INPUT_TYPES_DIAL
CORE_INPUT_TYPES_DIAL_V
CORE_INPUT_TYPES_TRACKBALL_X
CORE_INPUT_TYPES_TRACKBALL_Y
CORE_INPUT_TYPES_AD_STICK_X
CORE_INPUT_TYPES_AD_STICK_Y
CORE_INPUT_TYPES_AD_STICK_Z
CORE_INPUT_TYPES_LIGHTGUN_X
CORE_INPUT_TYPES_LIGHTGUN_Y
CORE_INPUT_TYPES_MOUSE_X
CORE_INPUT_TYPES_MOUSE_Y
CORE_INPUT_TYPES_KEYPAD
CORE_INPUT_TYPES_UI
CORE_INPUT_TYPES_OSD
CORE_INPUT_TYPES_INVALID
#undef CORE_INPUT_TYPES_BEGIN
#undef INPUT_PORT_DIGITAL_TYPE
#undef INPUT_PORT_ANALOG_TYPE
#undef CORE_INPUT_TYPES_END


// make a count function so we don't have to reallocate the vector
#define CORE_INPUT_TYPES_BEGIN(_name)
#define INPUT_PORT_DIGITAL_TYPE(_player, _group, _type, _name, _seq) + 1
#define INPUT_PORT_ANALOG_TYPE(_player, _group, _type, _name, _seq, _decseq, _incseq) + 1
#define CORE_INPUT_TYPES_END()
constexpr size_t core_input_types_count()
{
	return 0
			CORE_INPUT_TYPES_P1
			CORE_INPUT_TYPES_P1_MAHJONG
			CORE_INPUT_TYPES_P1_HANAFUDA
			CORE_INPUT_TYPES_GAMBLE
			CORE_INPUT_TYPES_POKER
			CORE_INPUT_TYPES_SLOT
			CORE_INPUT_TYPES_P2
			CORE_INPUT_TYPES_P2_MAHJONG
			CORE_INPUT_TYPES_P2_HANAFUDA
			CORE_INPUT_TYPES_P3
			CORE_INPUT_TYPES_P3_MAHJONG
			CORE_INPUT_TYPES_P4
			CORE_INPUT_TYPES_P4_MAHJONG
			CORE_INPUT_TYPES_P5
			CORE_INPUT_TYPES_P6
			CORE_INPUT_TYPES_P7
			CORE_INPUT_TYPES_P8
			CORE_INPUT_TYPES_P9
			CORE_INPUT_TYPES_P10
			CORE_INPUT_TYPES_START
			CORE_INPUT_TYPES_COIN
			CORE_INPUT_TYPES_SERVICE
			CORE_INPUT_TYPES_TILT
			CORE_INPUT_TYPES_OTHER
			CORE_INPUT_TYPES_PEDAL
			CORE_INPUT_TYPES_PEDAL2
			CORE_INPUT_TYPES_PEDAL3
			CORE_INPUT_TYPES_PADDLE
			CORE_INPUT_TYPES_PADDLE_V
			CORE_INPUT_TYPES_POSITIONAL
			CORE_INPUT_TYPES_POSITIONAL_V
			CORE_INPUT_TYPES_DIAL
			CORE_INPUT_TYPES_DIAL_V
			CORE_INPUT_TYPES_TRACKBALL_X
			CORE_INPUT_TYPES_TRACKBALL_Y
			CORE_INPUT_TYPES_AD_STICK_X
			CORE_INPUT_TYPES_AD_STICK_Y
			CORE_INPUT_TYPES_AD_STICK_Z
			CORE_INPUT_TYPES_LIGHTGUN_X
			CORE_INPUT_TYPES_LIGHTGUN_Y
			CORE_INPUT_TYPES_MOUSE_X
			CORE_INPUT_TYPES_MOUSE_Y
			CORE_INPUT_TYPES_KEYPAD
			CORE_INPUT_TYPES_UI
			CORE_INPUT_TYPES_OSD
			CORE_INPUT_TYPES_INVALID
			;
}
#undef CORE_INPUT_TYPES_BEGIN
#undef INPUT_PORT_DIGITAL_TYPE
#undef INPUT_PORT_ANALOG_TYPE
#undef CORE_INPUT_TYPES_END


ATTR_COLD inline void emplace_core_types(std::vector<input_type_entry> &typelist)
{
	typelist.reserve(core_input_types_count());

	emplace_core_types_p1(typelist);
	emplace_core_types_p1_mahjong(typelist);
	emplace_core_types_p1_hanafuda(typelist);
	emplace_core_types_gamble(typelist);
	emplace_core_types_poker(typelist);
	emplace_core_types_slot(typelist);
	emplace_core_types_p2(typelist);
	emplace_core_types_p2_mahjong(typelist);
	emplace_core_types_p2_hanafuda(typelist);
	emplace_core_types_p3(typelist);
	emplace_core_types_p3_mahjong(typelist);
	emplace_core_types_p4(typelist);
	emplace_core_types_p4_mahjong(typelist);
	emplace_core_types_p5(typelist);
	emplace_core_types_p6(typelist);
	emplace_core_types_p7(typelist);
	emplace_core_types_p8(typelist);
	emplace_core_types_p9(typelist);
	emplace_core_types_p10(typelist);
	emplace_core_types_start(typelist);
	emplace_core_types_coin(typelist);
	emplace_core_types_service(typelist);
	emplace_core_types_tilt(typelist);
	emplace_core_types_other(typelist);
	emplace_core_types_pedal(typelist);
	emplace_core_types_pedal2(typelist);
	emplace_core_types_pedal3(typelist);
	emplace_core_types_paddle(typelist);
	emplace_core_types_paddle_v(typelist);
	emplace_core_types_positional(typelist);
	emplace_core_types_positional_v(typelist);
	emplace_core_types_dial(typelist);
	emplace_core_types_dial_v(typelist);
	emplace_core_types_trackball_x(typelist);
	emplace_core_types_trackball_y(typelist);
	emplace_core_types_ad_stick_x(typelist);
	emplace_core_types_ad_stick_y(typelist);
	emplace_core_types_ad_stick_z(typelist);
	emplace_core_types_lightgun_x(typelist);
	emplace_core_types_lightgun_y(typelist);
	emplace_core_types_mouse_x(typelist);
	emplace_core_types_mouse_y(typelist);
	emplace_core_types_keypad(typelist);
	emplace_core_types_ui(typelist);
	emplace_core_types_osd(typelist);
	emplace_core_types_invalid(typelist);
}

} // anonymous namespace
