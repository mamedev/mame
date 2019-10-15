// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/**********************************************************************

    Geneve 9640 101-key XT/AT keyboard (High-level emulation)

    Geneves may use any XT keyboard; some were delivered with a 101-key XT/AT
    keyboard.

    This is a high-level emulation in the sense that it is only emulated
    from its behavior, not from its actual chipset. This will be done as
    soon as we have a ROM dump.

    Although the keyboard is switchable between XT and AT mode, we will
    only emulate the XT mode here.

    The code is copied from the previous implementation in genboard.cpp
    and appropriately adapted to use the pc_kbd interface.

    The XT keyboard interface is described in various places on the internet.
    It does not comply with the PS2 protocol. The keyboard transmits 8-bit
    scancodes serially with one (1) or two (0,1) start bits, LSB to MSB,
    no parity and no stop bits. Each bit is read into the shift register
    when the clock line is pulled low by the keyboard.

    MZ, August 2019

****************************************************************************/

#include "emu.h"
#include "genkbd.h"

#define LOG_WARN         (1U<<1)
#define LOG_QUEUE        (1U<<2)
#define LOG_TRANSFER     (1U<<3)
#define LOG_LINES        (1U<<4)

#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"

DEFINE_DEVICE_TYPE(KBD_GENEVE_XT_101_HLE, geneve_xt_101_hle_keyboard_device, "kb_geneve_hle", "Geneve XT Keyboard 101 Keys (HLE)")

INPUT_PORTS_START( geneve_xt_101_hle_keyboard )
	PORT_START("KEY0")  /* IN3 */
	PORT_BIT(0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) /* Esc  01  81 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)   /* 1    02  82 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2)   /* 2    03  83 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)   /* 3    04  84 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)   /* 4    05  85 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)   /* 5    06  86 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6)   /* 6    07  87 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7)   /* 7    08  88 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8)   /* 8    09  89 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9)   /* 9    0A  8A */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0)      /* 0     0B  8B */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS)  /* -     0C  8C */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) /* =     0D  8D */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) /* Backspace     0E  8E */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) /* Tab  0F  8F */

	PORT_START("KEY1")  /* IN4 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)     /* Q    10  90 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)     /* W    11  91 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)     /* E    12  92 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)     /* R    13  93 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)     /* T    14  94 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)     /* Y    15  95 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)     /* U    16  96 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)     /* I    17  97 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)     /* O    18  98 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)     /* P    19  99 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE)   /* [       1A  9A */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE)  /* ]       1B  9B */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)     /* Enter   1C  9C */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Ctrl") PORT_CODE(KEYCODE_LCONTROL) /* LCtrl   1D  9D */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)     /* A    1E  9E */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)     /* S    1F  9F */

	PORT_START("KEY2")  /* IN5 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)     /* D    20  A0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)     /* F    21  A1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)     /* G    22  A2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)     /* H    23  A3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)     /* J    24  A4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)     /* K    25  A5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)     /* L    26  A6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON)       /* ;       27  A7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE)      /* '       28  A8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE)       /* `       29  A9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Shift") PORT_CODE(KEYCODE_LSHIFT)  /* LShift  2A  AA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH)  /* \       2B  AB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)     /* Z    2C  AC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)     /* X    2D  AD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)     /* C    2E  AE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)     /* V    2F  AF */

	PORT_START("KEY3")  /* IN6 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)       /* B   30  B0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)       /* N   31  B1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)       /* M   32  B2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) /* ,   33  B3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)  /* .   34  B4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) /* /   35  B5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-Shift") PORT_CODE(KEYCODE_RSHIFT)             /* RShift  36  B6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP * (PrtScr)") PORT_CODE(KEYCODE_ASTERISK    ) /* KP *  (PrtSc)   37  B7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT)                   /* LAlt    38  B8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)   /* Space       39  B9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) /* Caps Lock   3A  BA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)         /* F1          3B  BB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)         /* F2          3C  BC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)         /* F3          3D  BD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)         /* F4          3E  BE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)         /* F5          3F  BF */

	PORT_START("KEY4")  /* IN7 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)   /* F6  40  C0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)   /* F7  41  C1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)   /* F8  42  C2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)   /* F9  43  C3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) /* F10 44  C4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK)        /* Num Lock     45  C5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ScrLock (F14)") PORT_CODE(KEYCODE_SCRLOCK)  /* Scroll Lock  46  C6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 (Home)") PORT_CODE(KEYCODE_7_PAD)      /* KP 7  (Home) 47  C7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 (Up)") PORT_CODE(KEYCODE_8_PAD)        /* KP 8  (Up)   48  C8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD)      /* KP 9  (PgUp) 49  C9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -") PORT_CODE(KEYCODE_MINUS_PAD)         /* KP -         4A  CA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 (Left)") PORT_CODE(KEYCODE_4_PAD)      /* KP 4  (Left) 4B  CB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5") PORT_CODE(KEYCODE_5_PAD)             /* KP 5         4C  CC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6 (Right)") PORT_CODE(KEYCODE_6_PAD)     /* KP 6  (Right)4D  CD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +") PORT_CODE(KEYCODE_PLUS_PAD)          /* KP +         4E  CE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD)       /* KP 1  (End)  4F  CF */

	PORT_START("KEY5")  /* IN8 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 (Down)") PORT_CODE(KEYCODE_2_PAD)      /* KP 2  (Down) 50  D0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD)      /* KP 3  (PgDn) 51  D1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0 (Ins)") PORT_CODE(KEYCODE_0_PAD)       /* KP 0  (Ins)  52  D2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP . (Del)") PORT_CODE(KEYCODE_DEL_PAD)     /* KP .  (Del)  53  D3 */
	PORT_BIT ( 0x0030, 0x0000, IPT_UNUSED )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(84/102)\\") PORT_CODE(KEYCODE_BACKSLASH2)  /* Backslash 2   56  D6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)F11") PORT_CODE(KEYCODE_F11)           /* F11           57  D7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)F12") PORT_CODE(KEYCODE_F12)           /* F12           58  D8 */
	PORT_BIT ( 0xfe00, 0x0000, IPT_UNUSED )

	PORT_START("KEY6")  /* IN9 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)KP Enter") PORT_CODE(KEYCODE_ENTER_PAD) /* KP Enter     60  e0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)R-Control") PORT_CODE(KEYCODE_RCONTROL) /* RCtrl        61  e1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)AltGr") PORT_CODE(KEYCODE_RALT)         /* AltGr        64  e4 */

	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)KP /") PORT_CODE(KEYCODE_SLASH_PAD)     /* KP Slash     62  e2 */

	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Home") PORT_CODE(KEYCODE_HOME)          /* Home         66  e6 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Cursor Up") PORT_CODE(KEYCODE_UP)       /* Up           67  e7 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Page Up") PORT_CODE(KEYCODE_PGUP)       /* Page Up      68  e8 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Cursor Left") PORT_CODE(KEYCODE_LEFT)   /* Left         69  e9 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Cursor Right") PORT_CODE(KEYCODE_RIGHT) /* Right        6a  ea */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)End") PORT_CODE(KEYCODE_END)            /* End          6b  eb */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Cursor Down") PORT_CODE(KEYCODE_DOWN)   /* Down         6c  ec */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Page Down") PORT_CODE(KEYCODE_PGDN)     /* Page Down    6d  ed */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Insert") PORT_CODE(KEYCODE_INSERT)      /* Insert       6e  ee */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Delete") PORT_CODE(KEYCODE_DEL)         /* Delete       6f  ef */

	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)PrtScr (F13)") PORT_CODE(KEYCODE_PRTSCR) /* Print Screen 63  e3 */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Pause (F15)") PORT_CODE(KEYCODE_PAUSE)   /* Pause        65  e5 */

	PORT_START("KEY7")  /* IN10 */
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor geneve_xt_101_hle_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( geneve_xt_101_hle_keyboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm_pc_xt_83_keyboard_device - constructor
//-------------------------------------------------

geneve_xt_101_hle_keyboard_device::geneve_xt_101_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KBD_GENEVE_XT_101_HLE, tag, owner, clock),
	  device_pc_kbd_interface(mconfig, *this),
	  m_keys(*this, "KEY%u", 0),
	  m_queue_length(0),
	  m_autorepeat_code(0),
	  m_autorepeat_timer(0),
	  m_fake_shift_state(false),
	  m_fake_unshift_state(false),
	  m_resetting(false),
	  m_clock_line(ASSERT_LINE),
	  m_data_line(ASSERT_LINE)
{
}

/*
    Called by the poll timer
*/
void geneve_xt_101_hle_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id==0)
	{
		poll();
		send_key();
	}
	else
	{
		if (id==1)
		{
			// Send timer
			if (m_shift_count==10)
			{
				// Done, all sent
				m_pc_kbdc->clock_write_from_kb(1);
				m_pc_kbdc->data_write_from_kb(1);
				m_send_timer->reset();
				m_shift_count = 0;

				// Adjust the queue
				m_queue_head = (m_queue_head + 1) % KEYQUEUESIZE;
				m_queue_length--;
			}
			else
			{
				m_pc_kbdc->clock_write_from_kb(1);
				m_pc_kbdc->data_write_from_kb(m_shift_reg&1);
				m_pc_kbdc->clock_write_from_kb(0);
				m_shift_reg>>=1;
				m_shift_count++;
			}
		}
	}
}


/*
    Translations
*/
static const uint8_t MF1_CODE[0xe] =
{
	// Extended keys that are equivalent to non-extended keys
	0x1c,   // KP Enter -> Return
	0x1d,   // RCtrl -> LCtrl
	0x38,   // AltGr -> LAlt

	// Extended key that is equivalent to a non-extended key with shift off
	0x35,   // KP / -> /

	// Extended keys that are equivalent to non-extended keys with numlock off
	0x47,   // Home    -> KP 7 (Home)
	0x48,   // Up      -> KP 8 (Up)
	0x49,   // Page up -> KP 9 (PgUp)
	0x4b,   // Left    -> KP 4 (Left)
	0x4d,   // Right   -> KP 6 (Right)
	0x4f,   // End     -> KP 1 (End)
	0x50,   // Down    -> KP 2 (Down)
	0x51,   // Page dn -> KP 3 (PgDn)
	0x52,   // Insert  -> KP 0 (Ins)
	0x53    // Delete  -> KP . (Del)
};

void geneve_xt_101_hle_keyboard_device::poll()
{
	uint32_t keystate;
	uint32_t key_transitions;
	uint32_t mask;
	bool pressed = false;
	int keycode = 0;

	if (m_resetting) return;

	// We're testing two 16-bit ports at once
	// but only if we have enough space in the queue
	for (int i=0; (i < 4) && (m_queue_length <= (KEYQUEUESIZE-MAXKEYMSGLENGTH)); i++)
	{
		// Get those two ports and calculate the difference
		keystate = m_keys[2*i]->read() | (m_keys[2*i + 1]->read() << 16);
		key_transitions = keystate ^ m_key_state_save[i];
		if (key_transitions != 0)
		{
			mask = 0x00000001;
			// Some key(s) has/have changed (pressed/released)
			for (int j=0; (j < 32) && (m_queue_length <= (KEYQUEUESIZE-MAXKEYMSGLENGTH)); j++)
			{
				if ((key_transitions & mask)!=0)
				{
					// Found one changed key (i is a 32-key block)
					keycode = (i<<5) | j;
					pressed = (keystate & mask)!=0;

					// Auto-repeat
					if (pressed)
					{
						m_autorepeat_code = keycode;
						m_autorepeat_timer = KEYAUTOREPEATDELAY+1;
						m_key_state_save[i] |= mask;
					}
					else
					{
						m_autorepeat_code = 0;
						m_key_state_save[i] &= ~mask;
					}

					// We are here because the set of pressed keys has changed
					// In the case that we have a fake shift/unshift,
					// we have to release it.

					if (m_fake_shift_state)
					{
						/* Fake shift release */
						post_in_key_queue(0xe0);
						post_in_key_queue(0xaa);
						m_fake_shift_state = false;
					}
					if (m_fake_unshift_state)
					{
						/* Fake shift press */
						post_in_key_queue(0xe0);
						post_in_key_queue(0x2a);
						m_fake_unshift_state = false;
					}

					switch (keycode)
					{
					case 0x2a:
						m_left_shift = pressed;
						break;
					case 0x36:
						m_right_shift = pressed;
						break;
					case 0x1d:
						m_left_ctrl = pressed;
						break;
					case 0x61:
						m_right_ctrl = pressed;
						break;
					case 0x38:
						m_left_alt = pressed;
						break;
					case 0x62:
						m_altgr = pressed;
						break;
					case 0x45:
						if (pressed) m_numlock = !m_numlock;
						break;
					default:
						break;
					}

					// Extended keycodes
					if ((keycode >= 0x60) && (keycode < 0x6e))
					{
						if ((keycode >= 0x63) && pressed)
						{
							// Handle shift state
							if (keycode == 0x63)   // Slash
							{
								if (m_left_shift || m_right_shift)
								{
									// Fake shift unpress
									m_fake_unshift_state = true;
								}
							}
							else
							{   // Key function with NumLock=0
								if (m_numlock && (!m_left_shift) && (!m_right_shift))
								{
									// Fake shift press if numlock is active
									m_fake_shift_state = true;
								}
								else
								{
									if ((!m_numlock) && (m_left_shift || m_right_shift))
									{
										// Fake shift unpress if shift is down
										m_fake_unshift_state = true;
									}
								}
							}

							if (m_fake_shift_state)
							{
								post_in_key_queue(0xe0);
								post_in_key_queue(0x2a);
							}

							if (m_fake_unshift_state)
							{
								post_in_key_queue(0xe0);
								post_in_key_queue(0xaa);
							}
						}
						keycode = MF1_CODE[keycode-0x60];
						if (!pressed) keycode |= 0x80;
						post_in_key_queue(0xe0);
						post_in_key_queue(keycode);
					}
					else
					{
						if (keycode == 0x6e)
						{
							// Emulate Print Screen / System Request (F13) key
							// this is a bit complex, as Alt+PrtScr -> SysRq
							// Additionally, Ctrl+PrtScr involves no fake shift press
							if (m_left_alt || m_altgr)
							{
								// SysRq
								keycode = 0x54;
								if (!pressed) keycode |= 0x80;
								post_in_key_queue(keycode);
							}
							else
							{
								// Handle shift state
								if (pressed && (!m_left_shift) && (!m_right_shift) && (!m_left_ctrl) && (!m_right_ctrl))
								{   // Fake shift press
									post_in_key_queue(0xe0);
									post_in_key_queue(0x2a);
									m_fake_shift_state = true;
								}

								keycode = 0x37;
								if (!pressed) keycode |= 0x80;
								post_in_key_queue(0xe0);
								post_in_key_queue(keycode);
							}
						}
						else
						{
							if (keycode == 0x6f)
							{
								// Emulate pause (F15) key
								// This is a bit complex, as Pause -> Ctrl+NumLock and
								// Ctrl+Pause -> Ctrl+ScrLock.
								// Furthermore, there is no repeat or release.
								if (pressed)
								{
									if (m_left_ctrl || m_right_ctrl)
									{
										post_in_key_queue(0xe0);
										post_in_key_queue(0x46);
										post_in_key_queue(0xe0);
										post_in_key_queue(0xc6);
									}
									else
									{
										post_in_key_queue(0xe1);
										post_in_key_queue(0x1d);
										post_in_key_queue(0x45);
										post_in_key_queue(0xe1);
										post_in_key_queue(0x9d);
										post_in_key_queue(0xc5);
									}
								}
							}
							else
							{
								if (!pressed) keycode |= 0x80;
								post_in_key_queue(keycode);
							}
						}
					}

				}
				mask <<= 1;
			}
		}
	}

	// Implement auto repeat
	if (m_autorepeat_code != 0)
	{
		m_autorepeat_timer--;
		if ((m_autorepeat_timer == 0) && (m_queue_length <= (KEYQUEUESIZE-MAXKEYMSGLENGTH)))
		{
			// Extended code
			if ((m_autorepeat_code >= 0x60) && (m_autorepeat_code < 0x6e))
			{
				post_in_key_queue(0xe0);
				post_in_key_queue(MF1_CODE[m_autorepeat_code-0x60]);
			}
			else
			{
				if (m_autorepeat_code == 0x6e)
				{
					if (m_left_alt || m_altgr)
					{
						post_in_key_queue(0x54);   // SysRq
					}
					else
					{
						post_in_key_queue(0xe0);
						post_in_key_queue(0x37);   // PrtScr
					}
				}
				else
				{
					if (m_autorepeat_code != 0x6f)   // Pause cannot do an auto-repeat
					{
						post_in_key_queue(m_autorepeat_code);
					}
				}
			}
			m_autorepeat_timer = KEYAUTOREPEATRATE;
		}
	}
}

void geneve_xt_101_hle_keyboard_device::send_key()
{
	if (m_clock_line == CLEAR_LINE)
	{
		m_reset_timer--;
		if (m_reset_timer==0)
		{
			LOG("Reset triggered\n");
			reset_line(0);
			reset_line(1);
		}
	}
	else
	{
		if (m_data_line==1)
		{
			// Dequeue a key
			if (m_queue_length != 0)
			{
				LOGMASKED(LOG_TRANSFER, "Send keycode %02x\n", m_queue[m_queue_head]);
				// Get the next key, add the two start bits to the right (0,1)
				m_shift_reg = (m_queue[m_queue_head] << 2) | 0x02;
				m_send_timer->adjust(attotime::from_usec(1), 0, attotime::from_hz(10000));
			}
		}
		else
			LOGMASKED(LOG_TRANSFER, "Transfer blocked by data=0 from host\n");
	}
}

void geneve_xt_101_hle_keyboard_device::post_in_key_queue(int keycode)
{
	m_queue[(m_queue_head + m_queue_length) % KEYQUEUESIZE] = keycode;
	m_queue_length++;

	LOGMASKED(LOG_QUEUE, "Enqueue keycode %02x, queue length=%d\n", keycode, m_queue_length);
}

//-------------------------------------------------
//  clock_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( geneve_xt_101_hle_keyboard_device::clock_write )
{
	LOGMASKED(LOG_LINES, "Clock write: %d\n", state);
	m_clock_line = (line_state)state;
	if (m_clock_line == CLEAR_LINE)
	{
		if (m_reset_timer == -1)
			m_reset_timer = 3;     // 25 ms
	}
	else
		m_reset_timer = -1;
}


//-------------------------------------------------
//  data_write -
//-------------------------------------------------

WRITE_LINE_MEMBER( geneve_xt_101_hle_keyboard_device::data_write )
{
	LOGMASKED(LOG_LINES, "Data write: %d\n", state);
	m_data_line = (line_state)state;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void geneve_xt_101_hle_keyboard_device::device_start()
{
	set_pc_kbdc_device();
	m_poll_timer = timer_alloc(0);
	m_send_timer = timer_alloc(1);

	// state saving
	save_item(NAME(m_queue_length));
	save_item(NAME(m_queue_head));
	save_pointer(NAME(m_key_state_save),4);
	save_pointer(NAME(m_queue),KEYQUEUESIZE);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void geneve_xt_101_hle_keyboard_device::device_reset()
{
	// Trigger our reset line
	reset_line(0);
	reset_line(1);
	m_poll_timer->adjust(attotime::from_usec(1), 0, attotime::from_hz(120));
}


WRITE_LINE_MEMBER( geneve_xt_101_hle_keyboard_device::reset_line )
{
	m_resetting = (state==0);

	if (m_resetting)
	{
		// Reset -> clear keyboard key queue
		m_reset_timer = -1;
		m_queue_length = 0;
		m_queue_head = 0;
		memset(m_key_state_save, 0, sizeof(m_key_state_save));

		m_numlock = false;
		m_left_shift = false;
		m_right_shift = false;
		m_left_ctrl = false;
		m_right_ctrl = false;
		m_left_alt = false;
		m_altgr = false;

		m_fake_shift_state = false;
		m_fake_unshift_state = false;
		m_autorepeat_code = 0;

		m_shift_reg = 0;
		m_shift_count = 0;

		// Send the BAT (Basic assurance test) OK value (AA)
		post_in_key_queue(0xaa);
	}
}

