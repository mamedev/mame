// license:GPL-2.0+
// copyright-holders:Kevin Thacker,Sandro Ronco
/*********************************************************************

    kc_keyb.c

    KC85_2/3/4/5 Keyboard emulation

**********************************************************************

  E-mail from Torsten Paul about the keyboard:

    Torsten.Paul@gmx.de

"> I hope that you will be able to help me so I can add keyboard
> emulation to it.
>
Oh yes the keyboard caused me a lot of trouble too. I still don't
understand it completely.
A first dirty but working keyboard support is quite easy because
of the good and modular system rom. Most programs use the supplied
routines to read keyboard input so patching the right memory address
with the keycode that is originally supplied by the interrupt
routines for keyboard input will work. So if you first want to
have a simple start to get other things to work you can try this.

    * write keycode (ascii) to address IX+0dh (0x1fd)
    * set bit 0 at address IX+08h (0x1f8) - simply writing 01h
      worked for me but some other bits are used too
      this bit is reset if the key is read by the system

> From the schematics I see that the keyboard is linked to the CTC and PIO,
> but I don't understand it fully.
> Please can you tell me more about how the keyboard is scanned?
>
The full emulation is quite tricky and I still have some programs that
behave odd. (A good hint for a correct keyboard emulation is Digger ;-).

Ok, now the technical things:

The keyboard of the KC is driven by a remote control circuit that is
originally designed for infrared remote control. This one was named
U807 and I learned there should be a similar chip called SAB 3021
available but I never found the specs on the web. The SAB 3021 was
produced by Valvo which doesn't exist anymore (bought by Phillips
if I remember correctly). If you have more luck finding the specs
I'm still interested.
There also was a complementary chip for the recieving side but that
was not used in the KC unfortunately. They choosed to measure the
pulses sent by the U807 via PIO and CTC.

Anyway here is what I know about the protocol:

The U807 sends impulses with equal length. The information is
given by the time between two impulses. Short time means bit is 1
long time mean bit is 0. The impulses are modulated by a 62.5 kHz
Signal but that's not interesting for the emulator.
The timing comes from a base clock of 4 MHz that is divided
multiple times:

4 MHz / 64 - 62.5 kHz -> used for filtered amplification
62.5 kHz / 64 - 976 Hz -> 1.024 ms
976 / 7 - 140 Hz -> 7.2 ms
976 / 5 - 195 Hz -> 5.1 ms

short - 5.12 ms - 1 bit
long - 7.168 ms - 0 bit

       +-+     +-+       +-+     +-+
       | |     | |       | |     | |
    +--+ +-----+ +-------+ +-----+ +--....

         |     | |---0---| |--1--|
            ^
            |
        Startbit = shift key

The keyboard can have 64 keys with an additional key that is used by
the KC as shift key. This additional key is directly connected to the
chip and changes the startbit. All other keys are arranged in a matrix.

The chip sends full words of 7 bits (including the startbit) at least
twice for each keypress with a spacing of 14 * 1.024 ms. If the key
is still pressed the double word is repeated after 19 * 1.024 ms.

The impulses trigger the pio interrupt line for channel B that
triggers the time measurement by the CTC channel 3."


**********************************************************************


    The CTC timer 3 count is initialised at 0x08f and counts down.

    The pulses are connected into PIO /BSTB and generate a interrupt
    on a positive edge. A pulse will not get through if BRDY from PIO is
    not true!

    Then the interrupt occurs, the CTC count is read. The time between
    pulses is therefore measured by the CTC.

    The time is checked and depending on the value the KC detects
    a 1 or 0 bit sent from the keyboard.

    Summary From code below:

    0x065<=count<0x078  -> 0 bit                "short pulse"
    0x042<=count<0x065  -> 1 bit                "long pulse"
    count<0x014         -> ignore
    count>=0x078        -> ignore
    0x014<=count<0x042  -> signal end of code   "very long pulse"

    codes are sent bit 0, bit 1, bit 2...bit 7. bit 0 is the state
    of the shift key.

    Torsten's e-mail indicates "short pulse" for 1 bit, and "long
    pulse" for 0 bit, but I found this to be incorrect. However,
    the timings are correct.


    Keyboard reading procedure extracted from KC85/4 system rom:

0345  f5        push    af
0346  db8f      in      a,(#8f)         ; get CTC timer 3 count
0348  f5        push    af
0349  3ea7      ld      a,#a7           ; channel 3, enable int, select counter mode, control word
                                        ; write, software reset, time constant follows
034b  d38f      out     (#8f),a
034d  3e8f      ld      a,#8f           ; time constant
034f  d38f      out     (#8f),a
0351  f1        pop     af

0352  fb        ei
0353  b7        or      a               ; count is over
0354  284d      jr      z,#03a3         ;

    ;; check count is in range
0356  fe14      cp      #14
0358  3849      jr      c,#03a3         ;
035a  fe78      cp      #78
035c  3045      jr      nc,#03a3        ;

    ;; at this point, time must be between #14 and #77 to be valid

    ;; if >=#65, then carry=0, and a 0 bit has been detected

035e  fe65      cp      #65
0360  303d      jr      nc,#039f        ; (61)

    ;; if <#65, then a 1 bit has been detected. carry is set with the addition below.
    ;; a carry will be generated if the count is >#42

    ;; therefore for a 1 bit to be generated, then #42<=time<#65
    ;; must be true.

0362  c6be      add     a,#be
0364  3839      jr      c,#039f         ; (57)

    ;; this code appears to take the transmitted scan-code
    ;; and converts it into a useable code by the os???
0366  e5        push    hl
0367  d5        push    de
    ;; convert hardware scan-code into os code
0368  dd7e0c    ld      a,(ix+#0c)
036b  1f        rra
036c  ee01      xor     #01
036e  dd6e0e    ld      l,(ix+#0e)
0371  dd660f    ld      h,(ix+#0f)
0374  1600      ld      d,#00
0376  5f        ld      e,a
0377  19        add     hl,de
0378  7e        ld      a,(hl)
0379  d1        pop     de
037a  e1        pop     hl

    ;; shift lock pressed?
037b  ddcb087e  bit     7,(ix+#08)
037f  200a      jr      nz,#038b
    ;; no.

    ;; alpha char?
0381  fe40      cp      #40
0383  3806      jr      c,#038b
0385  fe80      cp      #80
0387  3002      jr      nc,#038b
    ;; yes, it is a alpha char
    ;; force to lower case
0389  ee20      xor     #20

038b  ddbe0d    cp      (ix+#0d)        ;; same as stored?
038e  201d      jr      nz,#03ad

     ;; yes - must be held for a certain time before it can repeat?
0390  f5        push    af
0391  3ae0b7    ld      a,(#b7e0)
0394  ddbe0a    cp      (ix+#0a)
0397  3811      jr      c,#03aa
0399  f1        pop     af

039a  dd340a    inc     (ix+#0a)        ;; incremenent repeat count?
039d  1804      jr      #03a3

    ;; update scan-code received so far

039f  ddcb0c1e  rr      (ix+#0c)        ; shift in 0 or 1 bit depending on what has been selected

03a3  db89      in      a,(#89)         ; used to clear brdy
03a5  d389      out     (#89),a
03a7  f1        pop     af
03a8  ed4d      reti

03aa  f1        pop     af
03ab  1808      jr      #03b5

;; clear count
03ad  dd360a00  ld      (ix+#0a),#00

;; shift lock?
03b1  fe16      cp      #16
03b3  2809      jr      z,#03be

;; store char
03b5  dd770d    ld      (ix+#0d),a
03b8  ddcb08c6  set     0,(ix+#08)
03bc  18e5      jr      #03a3

;; toggle shift lock on/off
03be  dd7e08    ld      a,(ix+#08)
03c1  ee80      xor     #80
03c3  dd7708    ld      (ix+#08),a
;; shift/lock was last key pressed
03c6  3e16      ld      a,#16
03c8  18eb      jr      #03b5

03ca  b7        or      a
03cb  ddcb0846  bit     0,(ix+#08)
03cf  c8        ret     z
03d0  dd7e0d    ld      a,(ix+#0d)
03d3  37        scf
03d4  c9        ret
03d5  cdcae3    call    #e3ca
03d8  d0        ret     nc
03d9  ddcb0886  res     0,(ix+#08)
03dd  c9        ret
03de  cdcae3    call    #e3ca
03e1  d0        ret     nc
03e2  fe03      cp      #03
03e4  37        scf
03e5  c8        ret     z
03e6  a7        and     a
03e7  c9        ret


  Keyboard reading procedure extracted from KC85/3 rom:


019a  f5        push    af
019b  db8f      in      a,(#8f)
019d  f5        push    af
019e  3ea7      ld      a,#a7
01a0  d38f      out     (#8f),a
01a2  3e8f      ld      a,#8f
01a4  d38f      out     (#8f),a
01a6  f1        pop     af
01a7  ddcb085e  bit     3,(ix+#08)
01ab  ddcb089e  res     3,(ix+#08)
01af  2055      jr      nz,#0206        ; (85)

01b1  fe65      cp      #65
01b3  3054      jr      nc,#0209


    ;; count>=#65 = 0 bit
    ;; #44<=count<#65 = 1 bit
    ;; count<#44 = end of code

01b5  fe44      cp      #44
01b7  3053      jr      nc,#020c
01b9  e5        push    hl
01ba  d5        push    de
01bb  ddcb0c3e  srl     (ix+#0c)
01bf  dd7e08    ld      a,(ix+#08)
01c2  e680      and     #80
01c4  07        rlca
01c5  ddae0c    xor     (ix+#0c)
01c8  2600      ld      h,#00
01ca  dd5e0e    ld      e,(ix+#0e)
01cd  dd560f    ld      d,(ix+#0f)
01d0  6f        ld      l,a
01d1  19        add     hl,de
01d2  7e        ld      a,(hl)
01d3  d1        pop     de
01d4  e1        pop     hl
01d5  ddbe0d    cp      (ix+#0d)
01d8  2811      jr      z,#01eb         ; (17)
01da  dd770d    ld      (ix+#0d),a
01dd  ddcb08a6  res     4,(ix+#08)
01e1  ddcb08c6  set     0,(ix+#08)
01e5  dd360a00  ld      (ix+#0a),#00
01e9  181b      jr      #0206           ; (27)
01eb  dd340a    inc     (ix+#0a)
01ee  ddcb0866  bit     4,(ix+#08)
01f2  200c      jr      nz,#0200        ; (12)
01f4  ddcb0a66  bit     4,(ix+#0a)
01f8  280c      jr      z,#0206         ; (12)
01fa  ddcb08e6  set     4,(ix+#08)
01fe  18e1      jr      #01e1           ; (-31)
0200  ddcb0a4e  bit     1,(ix+#0a)
0204  20db      jr      nz,#01e1        ; (-37)
0206  f1        pop     af
0207  ed4d      reti
0209  b7        or      a
020a  1801      jr      #020d           ; (1)
020c  37        scf
020d  ddcb0c1e  rr      (ix+#0c)
0211  18f3      jr      #0206           ; (-13)

*********************************************************************/


#include "emu.h"
#include "kc_keyb.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type KC_KEYBOARD = &device_creator<kc_keyboard_device>;

//**************************************************************************
//  Input Ports
//**************************************************************************

//  this is a fake keyboard layout. The keys are converted into codes
//  which are transmitted by the keyboard to the base-unit. key code can
//  be calculated as (line*8)+bit_index

static INPUT_PORTS_START( kc_keyboard )
	// start of keyboard scan-codes
	// codes 0-7
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)        PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)          PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('Y') PORT_CHAR('y')
	// codes 8-15
	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('^') PORT_CHAR(0x00AC)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clr") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)          PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('X') PORT_CHAR('x')
	// codes 16-23
	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)          PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('V') PORT_CHAR('v')
	// codes 24-31
	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ins") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Brk") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('N') PORT_CHAR('n')
	// codes 32-39
	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('M') PORT_CHAR('m')
	// codes 40-47
	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, 0x00, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)          PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('B') PORT_CHAR('b')
	// codes 48-56
	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('-') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR('+') PORT_CHAR(';')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)          PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('C') PORT_CHAR('c')
	// codes 56-63
	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)          PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)       PORT_CHAR(13)
	// end of keyboard scan-codes
	PORT_START("SHIFT")
	// has a single shift key. Mapped here to left and right shift.
	// shift is connected to the MOC pin of the U807
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  kc_keyboard_device - constructor
//-------------------------------------------------
kc_keyboard_device::kc_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, KC_KEYBOARD, "KC Keyboard", tag, owner, clock, "kc_keyboard", __FILE__),
	m_write_out(*this)
{
}

//-------------------------------------------------
//  kc_keyboard_device - destructor
//-------------------------------------------------

kc_keyboard_device::~kc_keyboard_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void kc_keyboard_device::device_start()
{
	// resolve callbacks
	m_write_out.resolve_safe();

	m_timer_transmit_pulse = timer_alloc(TIMER_TRANSMIT_PULSE);

	m_timer_transmit_pulse->adjust(attotime::from_usec(1024), 0, attotime::from_usec(1024));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kc_keyboard_device::device_reset()
{
	// set initial state
	m_write_out(CLEAR_LINE);

	m_transmit_buffer.pulse_sent = 0;
	m_transmit_buffer.pulse_count = 0;

	memset(&m_transmit_buffer, 0, sizeof(m_transmit_buffer));
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor kc_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kc_keyboard );
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void kc_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_TRANSMIT_PULSE:
		if (m_transmit_buffer.pulse_sent < m_transmit_buffer.pulse_count)
		{
			// byte containing pulse state
			int pulse_byte_count = m_transmit_buffer.pulse_sent>>3;
			// bit within byte containing pulse state
			int pulse_bit_count = 7 - (m_transmit_buffer.pulse_sent & 0x07);

			// get current pulse state
			int pulse_state = (m_transmit_buffer.data[pulse_byte_count]>>pulse_bit_count) & 0x01;

			LOG(("KC keyboard sending pulse: %02x\n", pulse_state));

			// send pulse
			m_write_out(pulse_state ? ASSERT_LINE : CLEAR_LINE);

			// update counts
			m_transmit_buffer.pulse_sent++;
		}
		else
		{
			// if there is nothing to send, rescan the keyboard
			static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7" };

			m_transmit_buffer.pulse_sent = 0;
			m_transmit_buffer.pulse_count = 0;

			// scan all lines (excluding shift)
			for (int i=0; i<8; i++)
			{
				UINT8 keyboard_line_data = ioport(keynames[i])->read();

				// scan through each bit
				for (int b=0; b<8; b++)
				{
					// is pressed?
					if ((keyboard_line_data & (1<<b)) != 0)
					{
						// generate fake code
						UINT8 code = (i<<3) | b;
						LOG(("Code: %02x\n",code));

						transmit_scancode(code);
					}
				}
			}
		}
		break;
	}
}


//-------------------------------------------------
//  add a pulse
//-------------------------------------------------

void kc_keyboard_device::add_pulse_to_transmit_buffer(int pulse_state, int pulse_number)
{
	if (m_transmit_buffer.pulse_count + 1 < KC_TRANSMIT_BUFFER_LENGTH)
	{
		for (int i=0; i<pulse_number; i++)
		{
			int pulse_byte_count = m_transmit_buffer.pulse_count>>3;
			int pulse_bit_count = 7-(m_transmit_buffer.pulse_count & 0x07);

			if (pulse_state)
				m_transmit_buffer.data[pulse_byte_count] |= (1<<pulse_bit_count);
			else
				m_transmit_buffer.data[pulse_byte_count] &= ~(1<<pulse_bit_count);

			m_transmit_buffer.pulse_count++;
		}
	}
}

//-------------------------------------------------
//  fill transmit buffer with pulse for 0 or 1 bit
//-------------------------------------------------

void kc_keyboard_device::add_bit(int bit)
{
	if (bit)
		add_pulse_to_transmit_buffer(0, 7);
	else
		add_pulse_to_transmit_buffer(0, 5);

	// "end of bit" pulse -> end of time for bit
	add_pulse_to_transmit_buffer(1);
}


//-------------------------------------------------
//  begin pulse transmit
//-------------------------------------------------

void kc_keyboard_device::transmit_scancode(UINT8 scan_code)
{
	// initial pulse -> start of code
	add_pulse_to_transmit_buffer(1);

	// state of shift key
	add_bit((ioport("SHIFT")->read() & 0x01)^0x01);

	for (int i=0; i<6; i++)
	{
		// each bit in turn
		add_bit(scan_code & (1<<i));
	}

	// signal end of scan-code
	add_pulse_to_transmit_buffer(0, 14);

	add_pulse_to_transmit_buffer(1);

	// back to original state
	add_pulse_to_transmit_buffer(0);
}
