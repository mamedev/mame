// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "hlekbd.h"

#include <numeric>


/*
    TODO: scancodes for extra key on international layout
    TODO: suppress LED command processing for Type 3 keyboard

    HLE SPARC serial keyboard compatible with Sun Type 3/4/5/6

    messages from host to keyboard:
    0000 0001               reset (keyboard responds with self test pass/fail)
    0000 0010               bell on (480us period)
    0000 0011               bell off
    0000 1010               enable keyclick (5ms duration 480us period on make)
    0000 1011               disable keyclick
    0000 1110  ---- lscn    LED (1 = on, l = caps lock, s = scroll lock, c = compose, n = num lock)
    0000 1111               layout request (keyboard responds with layout response)

    message from keyboad to host:
    0xxx xxxx               key make
    1xxx xxxx               key break
    0111 1111               all keys up
    1111 1110  00dd dddd    layout response (DIP switches 3 to 8 from MSB to LSB)
    1111 1111  0000 0100    reset response (self test pass, always followed by all keys up or key make)
    0111 1110  0000 0001    self test fail


    Type 3 layout:

    01  03    05  06    08      0a      0c      0e      10    11  12  2b    15  16  17
    19  1a    1d  1e  1f  20  21  22  23  24  25  26  27  28  29  58  2a    2d  2e  2f
    31  33    35    36  37  38  39  3a  3b  3c  3d  3e  3f  40  41    42    44  45  46
    48  49    4c     4d  4e  4f  50  51  52  53  54  55  56  57       59    5b  5c  5d
    5f  61    63       64  65  66  67  68  69  6a  6b  6c  6d    6e   6f    70  71  72
              77     78                  79                   7a      13

    no LEDs
    top row function keys are F1-F9 and backspace
    delete key is above return
    meta keys labelled "Left" and "Right"
    linefeed key to the right of right shift (removed for Type 5)
    single control key (still the case on later Sun keyboards)
    "Alternate" key at bottom right of main area (moved to left for Type 4)
    left group of function keys labelled L1-L10
    right group of function keys labelled R1-R15
    cursor up/down/left/right on R8/R10/R12/R14


    Type 4 layout:

    01  03    05  06  08  0a  0c  0e  10  11  12  07  09  0b  58      42    15  16  17  62
    19  1a    1d  1e  1f  20  21  22  23  24  25  26  27  28  29      2b    2d  2e  2f  47
    31  33    35    36  37  38  39  3a  3b  3c  3d  3e  3f  40  41          44  45  46
    48  49    4c     4d  4e  4f  50  51  52  53  54  55  56  57  2a   59    5b  5c  5d  7d
    5f  61    63       64  65  66  67  68  69  6a  6b  6c  6d   6e    6f    70  71  72
      76      77  13  78                  79                  7a  43  0d      5e    32  5a

    double-height return key, backspace above return
    top row F1-F12, backslash and delete
    dedicated labels for left function group with "Help" key added at bottom
    meta keys labelled with diamonds, "Alt" on left, "Alt Graph" on right
    "tenkey" keypad layout mostly re-using codes from R1-R15 on Type 3
    top row of keypad is Pause, PrSc, Scroll Lock/Break, Num Lock
    2d is = key on keypad (removed for Type 5 and scancode reused for mute)
    introduces the "Compose" key
    Caps Lock/Compose/Scroll Lock/Num Lock LEDs above keypad


    Type 5 US PC layout:

      76      1d      05  06  08  0a    0c  0e  10  11    12  07  09  0b    16  17  15    2d  02  04  30

    01  03    2a  1e  1f  20  21  22  23  24  25  26  27  28  29      2b    2c  34  60    62  2e  2f  47
    19  1a    35    36  37  38  39  3a  3b  3c  3d  3e  3f  40  41    58    42  4a  7b    44  45  46
    31  33    77     4d  4e  4f  50  51  52  53  54  55  56  57       59                  5b  5c  5d  7d
    48  49    63       64  65  66  67  68  69  6a  6b  6c  6d         6e        14        70  71  72
    5f  61    4c     13   78                79                7a  43  0d    18  1b  1c      5e    32  5a


    Type 5 US UNIX layout:

      76      xx      05  06  08  0a    0c  0e  10  11    12  07  09  0b    16  17  15    2d  02  04  30

    01  03    1d  1e  1f  20  21  22  23  24  25  26  27  28  29  58  2a    2c  34  60    62  2e  2f  47
    19  1a    35    36  37  38  39  3a  3b  3c  3d  3e  3f  40  41    2b    42  4a  7b    44  45  46
    31  33    4c     4d  4e  4f  50  51  52  53  54  55  56  57       59                  5b  5c  5d  7d
    48  49    63       64  65  66  67  68  69  6a  6b  6c  6d         6e        14        70  71  72
    5f  61    77     13   78                79                7a  43  0d    18  1b  1c      5e    32  5a

    xx is a blank key
*/


/***************************************************************************
    DEVICE TYPE GLOBALS
***************************************************************************/

device_type const SUN_TYPE3_HLE_KEYBOARD = &device_creator<bus::sunkbd::hle_type3_device>;
device_type const SUN_TYPE4_HLE_KEYBOARD = &device_creator<bus::sunkbd::hle_type4_device>;
device_type const SUN_TYPE5_HLE_KEYBOARD = &device_creator<bus::sunkbd::hle_type5_device>;



namespace bus { namespace sunkbd {
namespace {
/***************************************************************************
    INPUT PORT DEFINITIONS
***************************************************************************/

INPUT_PORTS_START( basic )
	PORT_START("ROW0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F1")           PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F2")           PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F3")           PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F4")           PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F5")           PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F6")           PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED   )

	PORT_START("ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F7")           PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F8")           PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F9")           PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Esc")          PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Backspace")    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED   )

	PORT_START("ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Tab")          PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')

	PORT_START("ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Control")      PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED   )

	PORT_START("ROW6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L Shift")      PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD )                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R Shift")      PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED   )

	PORT_START("ROW7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Caps Lock")    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left Meta")    PORT_CODE(KEYCODE_LWIN)       PORT_CHAR(UCHAR_MAMEKEY(LWIN))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Space")        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right Meta")   PORT_CODE(KEYCODE_RWIN)       PORT_CHAR(UCHAR_MAMEKEY(RWIN))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED   )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED   )
INPUT_PORTS_END


INPUT_PORTS_START( tenkey )
	PORT_MODIFY("ROW2")
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP /")         PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP *")         PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))

	PORT_MODIFY("ROW3")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP .")         PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 7")         PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 8")         PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 9")         PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP -")         PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))

	PORT_MODIFY("ROW5")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Enter")        PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 4")         PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 5")         PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 6")         PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 0")         PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Num Lock")     PORT_CODE(KEYCODE_NUMLOCK)    PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))

	PORT_MODIFY("ROW7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 1")         PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 2")         PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP 3")         PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP +")         PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
INPUT_PORTS_END


INPUT_PORTS_START( l_group )
	PORT_MODIFY("ROW0")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L1")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L2")

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L3")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L4")

	PORT_MODIFY("ROW3")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L5")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L6")

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L7")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L8")

	PORT_MODIFY("ROW5")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L9")

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("L10")
INPUT_PORTS_END


INPUT_PORTS_START( r_group )
	PORT_MODIFY("ROW1")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R1")           PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R2")           PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R3")           PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))

	PORT_MODIFY("ROW2")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R4")           PORT_CODE(KEYCODE_NUMLOCK)    PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R5")           PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R6")           PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R7")           PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R8")           PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R9")           PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_MODIFY("ROW5")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R10")          PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R11")          PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R12")          PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))

	PORT_MODIFY("ROW7")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R13")          PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R14")          PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("R15")          PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
INPUT_PORTS_END


INPUT_PORTS_START( cursor )
	PORT_MODIFY("ROW1")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Up")           PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left")         PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Down")         PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right")        PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_MODIFY("ROW3")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Home")         PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("End")          PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Page Up")      PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))

	PORT_MODIFY("ROW7")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Page Down")    PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
INPUT_PORTS_END


INPUT_PORTS_START( solaris )
	PORT_MODIFY("ROW0")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Stop")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Again")

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Props")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Undo")

	PORT_MODIFY("ROW3")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Front")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Copy")

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Open")
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Paste")

	PORT_MODIFY("ROW5")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Find")

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Cut")

	PORT_MODIFY("ROW7")
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Help")
INPUT_PORTS_END


INPUT_PORTS_START( hle_type3_device )
	PORT_INCLUDE( basic   )
	PORT_INCLUDE( l_group )
	PORT_INCLUDE( r_group )

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alternate")    PORT_CODE(KEYCODE_LALT)

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Delete")       PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Line Feed")                                  PORT_CHAR(10)

	// TODO: work out what actually happens with DIPs on Type 3 keyboard
	PORT_START("DIP")
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


INPUT_PORTS_START( hle_type4_device )
	PORT_INCLUDE( basic   )
	PORT_INCLUDE( tenkey  )
	PORT_INCLUDE( solaris )

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F10")          PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F11")          PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F12")          PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alt Graph")    PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_MAMEKEY(RALT))

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alt")          PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Pause")        PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PrSc")         PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Scroll Lock")  PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))

	PORT_MODIFY("ROW2")
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("KP =")         PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Delete")       PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Compose")      PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))

	PORT_MODIFY("ROW6")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Line Feed")                                  PORT_CHAR(10)

	PORT_START("DIP")
	PORT_DIPNAME( 0x3f, 0x00, "Layout"                                 ) PORT_DIPLOCATION("DIP:8,7,6,5,4,3")
	PORT_DIPSETTING(    0x00, "U.S.A. (US4.kt)"                        )
	PORT_DIPSETTING(    0x01, "U.S.A. (US4.kt)"                        )
	PORT_DIPSETTING(    0x02, "Belgium (FranceBelg4.kt)"               )
	PORT_DIPSETTING(    0x03, "Canada (Canada4.kt)"                    )
	PORT_DIPSETTING(    0x04, "Denmark (Denmakr4.kt)"                  )
	PORT_DIPSETTING(    0x05, "Germany (Germany4.kt)"                  )
	PORT_DIPSETTING(    0x06, "Italy (Italy4.kt)"                      )
	PORT_DIPSETTING(    0x07, "The Netherlands (Netherland4.kt)"       )
	PORT_DIPSETTING(    0x08, "Norway (Norway4.kt)"                    )
	PORT_DIPSETTING(    0x09, "Portugal (Portugal4.kt)"                )
	PORT_DIPSETTING(    0x0a, "Latin America/Spanish (SpainLatAm4.kt)" )
	PORT_DIPSETTING(    0x0b, "Sweden (SwedenFin4.kt)"                 )
	PORT_DIPSETTING(    0x0c, "Switzerland/French (Switzer_Fr4.kt)"    )
	PORT_DIPSETTING(    0x0d, "Switzerland/German (Switzer_Ge4.kt)"    )
	PORT_DIPSETTING(    0x0e, "Great Britain (UK4.kt)"                 )
	// 0x0f
	PORT_DIPSETTING(    0x10, "Korea (Korea4.kt)"                      )
	PORT_DIPSETTING(    0x11, "Taiwan (Taiwan4.kt)"                    )
	// 0x12...0x20
	PORT_DIPSETTING(    0x21, "U.S.A. (US5.kt)"                        )
	PORT_DIPSETTING(    0x22, "U.S.A./UNIX (US_UNIX5.kt)"              )
	PORT_DIPSETTING(    0x23, "France (France5.kt)"                    )
	PORT_DIPSETTING(    0x24, "Denmark (Denmark5.kt)"                  )
	PORT_DIPSETTING(    0x25, "Germany (Germany5.kt)"                  )
	PORT_DIPSETTING(    0x26, "Italy (Italy5.kt)"                      )
	PORT_DIPSETTING(    0x27, "The Netherlands (Netherland5.kt)"       )
	PORT_DIPSETTING(    0x28, "Norway (Norway5.kt)"                    )
	PORT_DIPSETTING(    0x29, "Portugal (Portugal5.kt)"                )
	PORT_DIPSETTING(    0x2a, "Spain (Spain5.kt)"                      )
	PORT_DIPSETTING(    0x2b, "Sweden (Sweden5.kt)"                    )
	PORT_DIPSETTING(    0x2c, "Switzerland/French (Switzer_Fr5.kt)"    )
	PORT_DIPSETTING(    0x2d, "Switzerland/German (Switzer_Ge5.kt)"    )
	PORT_DIPSETTING(    0x2e, "Great Britain (UK5.kt)"                 )
	PORT_DIPSETTING(    0x2f, "Korea (Korea5.kt)"                      )
	PORT_DIPSETTING(    0x30, "Taiwan (Taiwan5.kt)"                    )
	PORT_DIPSETTING(    0x31, "Japan (Japan5.kt)"                      )
	PORT_DIPSETTING(    0x32, "Canada/French (Canada_Fr5.kt)"          )
	PORT_DIPSETTING(    0x33, "Hungary (Hungary5.kt)"                  )
	PORT_DIPSETTING(    0x34, "Poland (Poland5.kt)"                    )
	PORT_DIPSETTING(    0x35, "Czech (Czech5.kt)"                      )
	PORT_DIPSETTING(    0x36, "Russia (Russia5.kt)"                    )
	PORT_DIPSETTING(    0x37, "Latvia (Latvia5.kt)"                    )
	PORT_DIPSETTING(    0x38, "Turkey-Q5 (TurkeyQ5.kt)"                )
	PORT_DIPSETTING(    0x39, "Greece (Greece5.kt)"                    )
	PORT_DIPSETTING(    0x3a, "Arabic (Arabic5.kt)"                    )
	PORT_DIPSETTING(    0x3b, "Lithuania (Lithuania5.kt)"              )
	PORT_DIPSETTING(    0x3c, "Belgium (Belgium5.kt)"                  )
	// 0x3d
	PORT_DIPSETTING(    0x3e, "Turkey-F5 (TurkeyF5.kt)"                )
	PORT_DIPSETTING(    0x3f, "Canada/French (Canada_Fr5_TBITS5.kt)"   )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "DIP:2" )
	PORT_DIPNAME( 0x80, 0x00, "Identify as"                            ) PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(    0x00, "Type 4"                                 )
	PORT_DIPSETTING(    0x80, "Type 3"                                 )
INPUT_PORTS_END


INPUT_PORTS_START( hle_type5_device )
	PORT_INCLUDE( basic   )
	PORT_INCLUDE( tenkey  )
	PORT_INCLUDE( cursor  )
	PORT_INCLUDE( solaris )

	PORT_MODIFY("ROW0")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Vol-")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Vol+")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F10")          PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F11")          PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("F12")          PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alt Graph")    PORT_CODE(KEYCODE_RALT)       PORT_CHAR(UCHAR_MAMEKEY(RALT))

	PORT_MODIFY("ROW1")
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Alt")          PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Pause")        PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Print Screen") PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Scroll Lock")  PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))

	PORT_MODIFY("ROW2")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Insert")       PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Mute")

	PORT_MODIFY("ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Pwr")

	PORT_MODIFY("ROW4")
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del")          PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Compose")      PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))

	PORT_START("DIP")
	PORT_DIPNAME( 0x1f, 0x01, "Layout"                                 ) PORT_DIPLOCATION("S:5,4,3,2,1")
	// 0x00
	PORT_DIPSETTING(    0x01, "U.S.A. (US5.kt)"                        )
	PORT_DIPSETTING(    0x02, "U.S.A./UNIX (US_UNIX5.kt)"              )
	PORT_DIPSETTING(    0x03, "France (France5.kt)"                    )
	PORT_DIPSETTING(    0x04, "Denmark (Denmark5.kt)"                  )
	PORT_DIPSETTING(    0x05, "Germany (Germany5.kt)"                  )
	PORT_DIPSETTING(    0x06, "Italy (Italy5.kt)"                      )
	PORT_DIPSETTING(    0x07, "The Netherlands (Netherland5.kt)"       )
	PORT_DIPSETTING(    0x08, "Norway (Norway5.kt)"                    )
	PORT_DIPSETTING(    0x09, "Portugal (Portugal5.kt)"                )
	PORT_DIPSETTING(    0x0a, "Spain (Spain5.kt)"                      )
	PORT_DIPSETTING(    0x0b, "Sweden (Sweden5.kt)"                    )
	PORT_DIPSETTING(    0x0c, "Switzerland/French (Switzer_Fr5.kt)"    )
	PORT_DIPSETTING(    0x0d, "Switzerland/German (Switzer_Ge5.kt)"    )
	PORT_DIPSETTING(    0x0e, "Great Britain (UK5.kt)"                 )
	PORT_DIPSETTING(    0x0f, "Korea (Korea5.kt)"                      )
	PORT_DIPSETTING(    0x10, "Taiwan (Taiwan5.kt)"                    )
	PORT_DIPSETTING(    0x11, "Japan (Japan5.kt)"                      )
	PORT_DIPSETTING(    0x12, "Canada/French (Canada_Fr5.kt)"          )
	PORT_DIPSETTING(    0x13, "Hungary (Hungary5.kt)"                  )
	PORT_DIPSETTING(    0x14, "Poland (Poland5.kt)"                    )
	PORT_DIPSETTING(    0x15, "Czech (Czech5.kt)"                      )
	PORT_DIPSETTING(    0x16, "Russia (Russia5.kt)"                    )
	PORT_DIPSETTING(    0x17, "Latvia (Latvia5.kt)"                    )
	PORT_DIPSETTING(    0x18, "Turkey-Q5 (TurkeyQ5.kt)"                )
	PORT_DIPSETTING(    0x19, "Greece (Greece5.kt)"                    )
	PORT_DIPSETTING(    0x1a, "Arabic (Arabic5.kt)"                    )
	PORT_DIPSETTING(    0x1b, "Lithuania (Lithuania5.kt)"              )
	PORT_DIPSETTING(    0x1c, "Belgium (Belgium5.kt)"                  )
	// 0x1d
	PORT_DIPSETTING(    0x1e, "Turkey-F5 (TurkeyF5.kt)"                )
	PORT_DIPSETTING(    0x1f, "Canada/French (Canada_Fr5_TBITS5.kt)"   )
	PORT_BIT( 0xe0, 0x20, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************
    MACHINE CONFIGURATION FRAGMENTS
***************************************************************************/

MACHINE_CONFIG_FRAGMENT(sparc_keyboard)
	MCFG_SPEAKER_STANDARD_MONO("bell")
	MCFG_SOUND_ADD("beeper", BEEP, ATTOSECONDS_TO_HZ(480 * ATTOSECONDS_PER_MICROSECOND))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bell", 1.0)
MACHINE_CONFIG_END

} // anonymous namespace



/***************************************************************************
    BASE HLE KEYBOARD DEVICE
***************************************************************************/

/*--------------------------------------------------
    hle_device_base::hle_device_base
    designated device constructor
--------------------------------------------------*/

hle_device_base::hle_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *name,
		char const *tag,
		device_t *owner,
		UINT32 clock,
		char const *shortname,
		char const *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_serial_interface(mconfig, *this)
	, device_sun_keyboard_port_interface(mconfig, *this)
	, m_dips(*this, "DIP")
	, m_key_inputs{
			{ *this, "ROW0" }, { *this, "ROW1" },
			{ *this, "ROW2" }, { *this, "ROW3" },
			{ *this, "ROW4" }, { *this, "ROW5" },
			{ *this, "ROW6" }, { *this, "ROW7" } }
	, m_scan_timer(nullptr)
	, m_click_timer(nullptr)
	, m_beeper(*this, "beeper")
	, m_current_keys{ 0, 0, 0, 0, 0, 0, 0, 0 }
	, m_next_row(0)
	, m_fifo{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	, m_head(0)
	, m_tail(0)
	, m_empty(0)
	, m_rx_state(RX_IDLE)
	, m_keyclick(0)
	, m_beeper_state(0)
{
}


/*--------------------------------------------------
    hle_device_base::~hle_device_base
    destructor
--------------------------------------------------*/

hle_device_base::~hle_device_base()
{
}


/*--------------------------------------------------
    hle_device_base::device_mconfig_additions
    get machine configuration additions
--------------------------------------------------*/

machine_config_constructor hle_device_base::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(sparc_keyboard);
}


/*--------------------------------------------------
    hle_device_base::input_txd
    handle serial input line changes
--------------------------------------------------*/

WRITE_LINE_MEMBER( hle_device_base::input_txd )
{
	device_serial_interface::rx_w(state);
}


/*--------------------------------------------------
    hle_device_base::device_start
    perform expensive initialisations, allocate
	resources, register for save state
--------------------------------------------------*/

void hle_device_base::device_start()
{
	device_serial_interface::register_save_state(machine().save(), this);

	m_scan_timer = timer_alloc(SCAN_TIMER_ID);
	m_click_timer = timer_alloc(CLICK_TIMER_ID);

	save_item(NAME(m_current_keys));
	save_item(NAME(m_next_row));
	save_item(NAME(m_fifo));
	save_item(NAME(m_head));
	save_item(NAME(m_tail));
	save_item(NAME(m_empty));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_keyclick));
	save_item(NAME(m_beeper_state));
}


/*--------------------------------------------------
    hle_device_base::device_reset
    perform startup tasks, also used for host
	requested reset
--------------------------------------------------*/

void hle_device_base::device_reset()
{
	// initialise state
	std::fill(std::begin(m_current_keys), std::end(m_current_keys), 0x0000U);
	m_next_row = 0;
	std::fill(std::begin(m_fifo), std::end(m_fifo), 0);
	m_head = m_tail = 0;
	m_empty = 1;
	m_rx_state = RX_IDLE;
	m_keyclick = 0;
	m_beeper_state = 0;

	// configure device_serial_interface
	set_data_frame(START_BIT_COUNT, DATA_BIT_COUNT, PARITY, STOP_BITS);
	set_rate(BAUD);
	receive_register_reset();
	transmit_register_reset();

	// set device_sun_keyboard_port_interface lines
	output_rxd(1);

	// send reset response
	send_byte(0xffU);
	send_byte(ident_byte());
	UINT16 const acc(
			std::accumulate(
				std::begin(m_key_inputs),
				std::end(m_key_inputs),
				UINT16(0),
				[] (UINT16 a, auto const &b) { return a | b->read(); }));
	if (!acc)
		send_byte(0x7fU);

	// kick the scan timer
	m_scan_timer->adjust(attotime::from_hz(1'200), 0, attotime::from_hz(1'200));
	m_click_timer->reset();
}


/*--------------------------------------------------
    hle_device_base::device_timer
    handle timed events
--------------------------------------------------*/

void hle_device_base::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case SCAN_TIMER_ID:
		scan_row();
		break;

	case CLICK_TIMER_ID:
		m_beeper_state &= ~UINT8(BEEPER_CLICK);
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		break;

	default:
		device_serial_interface::device_timer(timer, id, param, ptr);
	}
}


/*--------------------------------------------------
    hle_device_base::tra_callback
    send output of serial transmit shift register
	to host
--------------------------------------------------*/

void hle_device_base::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}


/*--------------------------------------------------
    hle_device_base::tra_complete
    handle end of transmitted byte frame
--------------------------------------------------*/

void hle_device_base::tra_complete()
{
	assert(!m_empty || (m_head == m_tail));
	assert(m_head < ARRAY_LENGTH(m_fifo));
	assert(m_tail < ARRAY_LENGTH(m_fifo));

	if (!m_empty)
	{
		transmit_register_setup(m_fifo[m_head]);
		m_head = (m_head + 1) & 0x0fU;
		m_empty = (m_head == m_tail) ? 1 : 0;
	}
}


/*--------------------------------------------------
    hle_device_base::rcv_complete
    handle received byte frame
--------------------------------------------------*/

void hle_device_base::rcv_complete()
{
	receive_register_extract();
	UINT8 const code(get_received_char());
	switch (m_rx_state)
	{
	case RX_LED:
		machine().output().set_led_value(LED_NUM, BIT(code, 0));
		machine().output().set_led_value(LED_COMPOSE, BIT(code, 1));
		machine().output().set_led_value(LED_SCROLL, BIT(code, 2));
		machine().output().set_led_value(LED_CAPS, BIT(code, 3));
		m_rx_state = RX_IDLE;
		break;

	default:
		assert(m_rx_state == RX_IDLE);
	case RX_IDLE:
		switch (code)
		{
		case COMMAND_RESET:
			device_reset();
			break;

		case COMMAND_BELL_ON:
			m_beeper_state |= UINT8(BEEPER_BELL);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			m_rx_state = RX_IDLE;
			break;

		case COMMAND_BELL_OFF:
			m_beeper_state &= ~UINT8(BEEPER_BELL);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			m_rx_state = RX_IDLE;
			break;

		case COMMAND_CLICK_ON:
			m_keyclick = 1;
			m_rx_state = RX_IDLE;
			break;

		case COMMAND_CLICK_OFF:
			m_keyclick = 0;
			m_click_timer->reset();
			m_beeper_state &= ~UINT8(BEEPER_CLICK);
			m_beeper->set_state(m_beeper_state ? 1 : 0);
			m_rx_state = RX_IDLE;
			break;

		case COMMAND_LED:
			m_rx_state = RX_LED;
			break;

		case COMMAND_LAYOUT:
			send_byte(0xfeU);
			send_byte(UINT8(m_dips->read() & 0x3fU));
			m_rx_state = RX_IDLE;
			break;

		default:
			logerror("Unknown command: 0x%02x", code);
			m_rx_state = RX_IDLE;
		}
	}
}


/*--------------------------------------------------
    hle_device_base::scan_row
    scan the next row in the keyboard matrix and
	send update to host
--------------------------------------------------*/

void hle_device_base::scan_row()
{
	// ensure we aren't in a stupid state
	assert(m_next_row < ARRAY_LENGTH(m_key_inputs));
	assert(m_next_row < ARRAY_LENGTH(m_current_keys));

	// get last read state and new state of row
	UINT16 const row(m_key_inputs[m_next_row]->read());
	UINT16 &current(m_current_keys[m_next_row]);
	bool keydown(false), keyup(false);

	// while the buffer isn't full, look for changed keys in this row
	for (UINT8 bit = 0; (bit < 16) && (m_empty || (m_head != m_tail)); ++bit)
	{
		UINT16 const mask(UINT16(1) << bit);
		if ((row ^ current) & mask)
		{
			UINT8 const make_break((row & mask) ? 0x00U : 0x80U);
			keydown = keydown || !bool(make_break);
			keyup = keyup || bool(make_break);
			current = (current & ~mask) | (row & mask);
			send_byte(make_break | (m_next_row << 4) | bit);
		}
	}

	// if a key was pressed and keyclick is enabled, turn on the beeper
	if (keydown && m_keyclick)
	{
		m_beeper_state |= UINT8(BEEPER_CLICK);
		m_beeper->set_state(m_beeper_state ? 1 : 0);
		m_click_timer->reset(attotime::from_msec(5));
	}

	// if a key was released and no keys are down, send all keys up code
	if (keyup)
	{
		UINT16 const acc(
				std::accumulate(
					std::begin(m_current_keys),
					std::end(m_current_keys),
					UINT16(0),
					[] (UINT16 a, UINT16 b) { return a | b; }));
		if (!acc)
			send_byte(0x7fU);
	}

	// advance to the next row
	m_next_row = (m_next_row + 1) & 0x07U;
}


/*--------------------------------------------------
    hle_device_base::send_byte
    send a byte or queue it to send later
--------------------------------------------------*/

void hle_device_base::send_byte(UINT8 code)
{
	assert(!m_empty || (m_head == m_tail));
	assert(m_head < ARRAY_LENGTH(m_fifo));
	assert(m_tail < ARRAY_LENGTH(m_fifo));

	if (m_empty && is_transmit_register_empty())
	{
		transmit_register_setup(code);
	}
	else if (m_empty || (m_head != m_tail))
	{
		m_fifo[m_tail] = code;
		m_tail = (m_tail + 1) & 0x0fU;
		m_empty = 0;
	}
	else
	{
		logerror("FIFO overrun (code = 0x%02x)", code);
	}
}



/***************************************************************************
    TYPE 3 HLE KEYBOARD DEVICE
***************************************************************************/

/*--------------------------------------------------
    hle_type3_device::hle_type3_device
    abbreviated constructor
--------------------------------------------------*/

hle_type3_device::hle_type3_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		UINT32 clock)
	: hle_type3_device(mconfig,
			SUN_TYPE3_HLE_KEYBOARD,
			"Sun Type 3 Keyboard (HLE)",
			tag,
			owner,
			clock,
			"type3_hle_kbd",
			__FILE__)
{
}


/*--------------------------------------------------
    hle_type3_device::hle_type3_device
    designated device constructor
--------------------------------------------------*/

hle_type3_device::hle_type3_device(
		machine_config const &mconfig,
		device_type type,
		char const *name,
		char const *tag,
		device_t *owner,
		UINT32 clock,
		char const *shortname,
		char const *source)
	: hle_device_base(mconfig, type, name, tag, owner, clock, shortname, source)
{
}


/*--------------------------------------------------
    hle_type3_device::device_input_ports
    get input ports for this device
--------------------------------------------------*/

ioport_constructor hle_type3_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hle_type3_device);
}


/*--------------------------------------------------
    hle_type3_device::ident_byte
	return identification byte for self test pass
	response
--------------------------------------------------*/

UINT8 hle_type3_device::ident_byte()
{
	return 0x03U;
}



/***************************************************************************
    TYPE 4 HLE KEYBOARD DEVICE
***************************************************************************/

/*--------------------------------------------------
    hle_type4_device::hle_type4_device
    abbreviated constructor
--------------------------------------------------*/

hle_type4_device::hle_type4_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		UINT32 clock)
	: hle_type4_device(mconfig,
			SUN_TYPE4_HLE_KEYBOARD,
			"Sun Type 4 Keyboard (HLE)",
			tag,
			owner,
			clock,
			"type4_hle_kbd",
			__FILE__)
{
}


/*--------------------------------------------------
    hle_type4_device::hle_type4_device
    designated device constructor
--------------------------------------------------*/

hle_type4_device::hle_type4_device(
		machine_config const &mconfig,
		device_type type,
		char const *name,
		char const *tag,
		device_t *owner,
		UINT32 clock,
		char const *shortname,
		char const *source)
	: hle_device_base(mconfig, type, name, tag, owner, clock, shortname, source)
{
}


/*--------------------------------------------------
    hle_type4_device::device_input_ports
    get input ports for this device
--------------------------------------------------*/

ioport_constructor hle_type4_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hle_type4_device);
}


/*--------------------------------------------------
    hle_type4_device::ident_byte
	return identification byte for self test pass
	response
--------------------------------------------------*/

UINT8 hle_type4_device::ident_byte()
{
	return (m_dips->read() & 0x80U) ? 0x03U : 0x04U;
}



/***************************************************************************
    TYPE 5 HLE KEYBOARD DEVICE
***************************************************************************/

/*--------------------------------------------------
    hle_type5_device::hle_type5_device
    abbreviated constructor
--------------------------------------------------*/

hle_type5_device::hle_type5_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		UINT32 clock)
	: hle_type5_device(mconfig,
			SUN_TYPE5_HLE_KEYBOARD,
			"Sun Type 5 Keyboard (HLE)",
			tag,
			owner,
			clock,
			"type5_hle_kbd",
			__FILE__)
{
}


/*--------------------------------------------------
    hle_type5_device::hle_type5_device
    designated device constructor
--------------------------------------------------*/

hle_type5_device::hle_type5_device(
		machine_config const &mconfig,
		device_type type,
		char const *name,
		char const *tag,
		device_t *owner,
		UINT32 clock,
		char const *shortname,
		char const *source)
	: hle_device_base(mconfig, type, name, tag, owner, clock, shortname, source)
{
}


/*--------------------------------------------------
    hle_type5_device::device_input_ports
    get input ports for this device
--------------------------------------------------*/

ioport_constructor hle_type5_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hle_type5_device);
}


/*--------------------------------------------------
    hle_type5_device::ident_byte
	return identification byte for self test pass
	response
--------------------------------------------------*/

UINT8 hle_type5_device::ident_byte()
{
	return 0x04U;
}

} } // namespace bus::sunkbd
