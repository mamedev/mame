// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Xerox AltoII keyboard hardware (KBD)
 *
 *****************************************************************************/
#ifdef  ALTO2_DEFINE_CONSTANTS

//! make an Xerox AltoII key bit mask
#define MAKE_KEY(a,b) (1 << (b))

#define A2_KEY_5            MAKE_KEY(0,017)     //!< normal: 5    shifted: %
#define A2_KEY_4            MAKE_KEY(0,016)     //!< normal: 4    shifted: $
#define A2_KEY_6            MAKE_KEY(0,015)     //!< normal: 6    shifted: ~
#define A2_KEY_E            MAKE_KEY(0,014)     //!< normal: e    shifted: E
#define A2_KEY_7            MAKE_KEY(0,013)     //!< normal: 7    shifted: &
#define A2_KEY_D            MAKE_KEY(0,012)     //!< normal: d    shifted: D
#define A2_KEY_U            MAKE_KEY(0,011)     //!< normal: u    shifted: U
#define A2_KEY_V            MAKE_KEY(0,010)     //!< normal: v    shifted: V
#define A2_KEY_0            MAKE_KEY(0,007)     //!< normal: 0    shifted: )
#define A2_KEY_K            MAKE_KEY(0,006)     //!< normal: k    shifted: K
#define A2_KEY_MINUS        MAKE_KEY(0,005)     //!< normal: -    shifted: _
#define A2_KEY_P            MAKE_KEY(0,004)     //!< normal: p    shifted: P
#define A2_KEY_SLASH        MAKE_KEY(0,003)     //!< normal: /    shifted: ?
#define A2_KEY_BACKSLASH    MAKE_KEY(0,002)     //!< normal: \    shifted: |
#define A2_KEY_LF           MAKE_KEY(0,001)     //!< normal: LF
#define A2_KEY_BS           MAKE_KEY(0,000)     //!< normal: BS

#define A2_KEY_3            MAKE_KEY(1,017)     //!< normal: 3    shifted: #
#define A2_KEY_2            MAKE_KEY(1,016)     //!< normal: 2    shifted: @
#define A2_KEY_W            MAKE_KEY(1,015)     //!< normal: w    shifted: W
#define A2_KEY_Q            MAKE_KEY(1,014)     //!< normal: q    shifted: Q
#define A2_KEY_S            MAKE_KEY(1,013)     //!< normal: s    shifted: S
#define A2_KEY_A            MAKE_KEY(1,012)     //!< normal: a    shifted: A
#define A2_KEY_9            MAKE_KEY(1,011)     //!< normal: 9    shifted: (
#define A2_KEY_I            MAKE_KEY(1,010)     //!< normal: i    shifted: I
#define A2_KEY_X            MAKE_KEY(1,007)     //!< normal: x    shifted: X
#define A2_KEY_O            MAKE_KEY(1,006)     //!< normal: o    shifted: O
#define A2_KEY_L            MAKE_KEY(1,005)     //!< normal: l    shifted: L
#define A2_KEY_COMMA        MAKE_KEY(1,004)     //!< normal: ,    shifted: <
#define A2_KEY_QUOTE        MAKE_KEY(1,003)     //!< normal: '    shifted: "
#define A2_KEY_RBRACKET     MAKE_KEY(1,002)     //!< normal: ]    shifted: }
#define A2_KEY_BLANK_MID    MAKE_KEY(1,001)     //!< middle blank key
#define A2_KEY_BLANK_TOP    MAKE_KEY(1,000)     //!< top blank key

#define A2_KEY_1            MAKE_KEY(2,017)     //!< normal: 1    shifted: !
#define A2_KEY_ESCAPE       MAKE_KEY(2,016)     //!< normal: ESC  shifted: ?
#define A2_KEY_TAB          MAKE_KEY(2,015)     //!< normal: TAB  shifted: ?
#define A2_KEY_F            MAKE_KEY(2,014)     //!< normal: f    shifted: F
#define A2_KEY_CTRL         MAKE_KEY(2,013)     //!< CTRL
#define A2_KEY_C            MAKE_KEY(2,012)     //!< normal: c    shifted: C
#define A2_KEY_J            MAKE_KEY(2,011)     //!< normal: j    shifted: J
#define A2_KEY_B            MAKE_KEY(2,010)     //!< normal: b    shifted: B
#define A2_KEY_Z            MAKE_KEY(2,007)     //!< normal: z    shifted: Z
#define A2_KEY_LSHIFT       MAKE_KEY(2,006)     //!< LSHIFT
#define A2_KEY_PERIOD       MAKE_KEY(2,005)     //!< normal: .    shifted: >
#define A2_KEY_SEMICOLON    MAKE_KEY(2,004)     //!< normal: ;    shifted: :
#define A2_KEY_RETURN       MAKE_KEY(2,003)     //!< RETURN
#define A2_KEY_LEFTARROW    MAKE_KEY(2,002)     //!< normal: <-   shifted: ^ (caret?)
#define A2_KEY_DEL          MAKE_KEY(2,001)     //!< normal: DEL
#define A2_KEY_MSW_2_17     MAKE_KEY(2,000)     //!< unused on Microswitch KDB

#define A2_KEY_R            MAKE_KEY(3,017)     //!< normal: r    shifted: R
#define A2_KEY_T            MAKE_KEY(3,016)     //!< normal: t    shifted: T
#define A2_KEY_G            MAKE_KEY(3,015)     //!< normal: g    shifted: G
#define A2_KEY_Y            MAKE_KEY(3,014)     //!< normal: y    shifted: Y
#define A2_KEY_H            MAKE_KEY(3,013)     //!< normal: h    shifted: H
#define A2_KEY_8            MAKE_KEY(3,012)     //!< normal: 8    shifted: *
#define A2_KEY_N            MAKE_KEY(3,011)     //!< normal: n    shifted: N
#define A2_KEY_M            MAKE_KEY(3,010)     //!< normal: m    shifted: M
#define A2_KEY_LOCK         MAKE_KEY(3,007)     //!< LOCK
#define A2_KEY_SPACE        MAKE_KEY(3,006)     //!< SPACE
#define A2_KEY_LBRACKET     MAKE_KEY(3,005)     //!< normal: [    shifted: {
#define A2_KEY_EQUALS       MAKE_KEY(3,004)     //!< normal: =    shifted: +
#define A2_KEY_RSHIFT       MAKE_KEY(3,003)     //!< RSHIFT
#define A2_KEY_BLANK_BOT    MAKE_KEY(3,002)     //!< bottom blank key
#define A2_KEY_MSW_3_16     MAKE_KEY(3,001)     //!< unused on Microswitch KDB
#define A2_KEY_MSW_3_17     MAKE_KEY(3,000)     //!< unused on Microswitch KDB

#define A2_KEY_FR2          MAKE_KEY(0,002)     //!< ADL right function key 2
#define A2_KEY_FL2          MAKE_KEY(0,001)     //!< ADL left function key 1

#define A2_KEY_FR4          MAKE_KEY(1,001)     //!< ADL right funtion key 4
#define A2_KEY_BW           MAKE_KEY(1,000)     //!< ADL BW (?)

#define A2_KEY_FR3          MAKE_KEY(2,002)     //!< ADL right function key 3
#define A2_KEY_FL1          MAKE_KEY(2,001)     //!< ADL left function key 1
#define A2_KEY_FL3          MAKE_KEY(2,000)     //!< ADL left function key 3

#define A2_KEY_FR1          MAKE_KEY(3,002)     //!< ADL right function key 4
#define A2_KEY_FL4          MAKE_KEY(3,001)     //!< ADL left function key 4
#define A2_KEY_FR5          MAKE_KEY(3,000)     //!< ADL right function key 5

#else   // ALTO2_DEFINE_CONSTANTS
#ifndef _A2KBD_H_
#define _A2KBD_H_
struct {
	UINT16 bootkey;                         //!< boot key - key code pressed before power on
	UINT16 matrix[4];                       //!< a bit map of the keys pressed (ioports ROW0 ... ROW3)
}   m_kbd;

DECLARE_READ16_MEMBER( kbd_ad_r );          //!< read the keyboard matrix

void init_kbd(UINT16 bootkey = 0177777);    //!< initialize the keyboard hardware, optinally set the boot key
void exit_kbd();                            //!< deinitialize the keyboard hardware
void reset_kbd();                           //!< reset the keyboard hardware
#endif // _A2KBD_H_
#endif  // ALTO2_DEFINE_CONSTANTS
