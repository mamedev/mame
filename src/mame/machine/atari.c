// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    Atari 400/800

    Machine driver

    Juergen Buchmueller, June 1998

***************************************************************************/

#include "emu.h"
#include "includes/atari.h"
#include "sound/pokey.h"

#define VERBOSE_POKEY   1
#define VERBOSE_SERIAL  1
#define VERBOSE_TIMERS  1


POKEY_INTERRUPT_CB_MEMBER(atari_common_state::interrupt_cb)
{
	if (VERBOSE_POKEY)
	{
		if (mask & 0x80)
			logerror("atari interrupt_cb BREAK\n");
		if (mask & 0x40)
			logerror("atari interrupt_cb KBCOD\n");
	}
	if (VERBOSE_SERIAL)
	{
		if (mask & 0x20)
			logerror("atari interrupt_cb SERIN\n");
		if (mask & 0x10)
			logerror("atari interrupt_cb SEROR\n");
		if (mask & 0x08)
			logerror("atari interrupt_cb SEROC\n");
	}
	if (VERBOSE_TIMERS)
	{
		if (mask & 0x04)
			logerror("atari interrupt_cb TIMR4\n");
		if (mask & 0x02)
			logerror("atari interrupt_cb TIMR2\n");
		if (mask & 0x01)
			logerror("atari interrupt_cb TIMR1\n");
	}

	m_maincpu->set_input_line(0, HOLD_LINE);
}


/**************************************************************
 *
 * Keyboard
 *
 **************************************************************/

#define AKEY_BREAK      0x03    /* this not really a scancode */
#define AKEY_NONE       0x09

/**************************************************************

    Keyboard inputs use 6bits to read the 64keys in the key matrix.
    We currently read the key matrix by lines and convert the input
    to the value expected by the POKEY (see the code below to
    determine atari_code values).

    K2,K1,K0 | 000 | 001 | 010 | 011 | 100 | 101 | 110 | 111 |
    K5,K4,K3
    ----------------------------------------------------------
        000  |  L  |  J  |  ;  | (*) |     |  K  |  +  |  *  |
        001  |  O  |     |  P  |  U  | Ret |  I  |  -  |  =  |
        010  |  V  |     |  C  |     |     |  B  |  X  |  Z  |
        011  |  4  |     |  3  |  6  | Esc |  5  |  2  |  1  |
        100  |  ,  | Spc |  .  |  N  |     |  M  |  /  |Atari|
        101  |  R  |     |  E  |  Y  | Tab |  T  |  W  |  Q  |
        110  |  9  |     |  0  |  7  |Bkspc|  8  |  <  |  >  |
        111  |  F  |  H  |  D  |     | Caps|  G  |  S  |  A  |

    (*) We use this value to read Break, but in fact it would be read
        in KR2 bit. This has to be properly implemented for later
        Atari systems because here we would have F1.

    To Do: investigate implementation of KR2 to read accurately Break,
    Shift and Control keys.

 **************************************************************/

POKEY_KEYBOARD_CB_MEMBER(atari_common_state::a800_keyboard)
{
	int ipt;
	UINT8 ret = 0x00;

	/* decode special */
	switch (k543210)
	{
	case pokey_device::POK_KEY_BREAK:
		/* special case ... */
		ret |= ((m_keyboard[0] && m_keyboard[0]->read() & 0x08) ? 0x02 : 0x00);
		break;
	case pokey_device::POK_KEY_CTRL:
		/* CTRL */
		ret |= ((m_fake && m_fake->read() & 0x02) ? 0x02 : 0x00);
		break;
	case pokey_device::POK_KEY_SHIFT:
		/* SHIFT */
		ret |= ((m_fake && m_fake->read() & 0x01) ? 0x02 : 0x00);
		break;
	}

	/* return on BREAK key now! */
	if (k543210 == AKEY_BREAK || k543210 == AKEY_NONE)
		return ret;

	/* decode regular key */
	ipt = m_keyboard[k543210 >> 3] ? m_keyboard[k543210 >> 3]->read() : 0;

	if (ipt & (1 << (k543210 & 0x07)))
		ret |= 0x01;

	return ret;
}

/**************************************************************
 *
 * Keypad
 *
 **************************************************************/

/**************************************************************

    A5200 keypad inputs use 4bits to read the 16keys in the key
    matrix. We currently read the key matrix by lines and convert
    the input to the value expected by the POKEY (see the code
    below to determine atari_code values).

    K2,K1,K0 | 00x | 01x | 10x | 11x |
    K5,K4,K3
    ----------------------------------
        x00  |     |  #  |  0  |  *  |
        x01  |Reset|  9  |  8  |  7  |
        x10  |Pause|  6  |  5  |  4  |
        x11  |Start|  3  |  2  |  1  |

    K0 & K5 are ignored (we send them as 1, see the code below where
    we pass "(atari_code << 1) | 0x21" )

    To Do: investigate implementation of KR2 to read accurately the
    secondary Fire button (primary read through GTIA).

 **************************************************************/

POKEY_KEYBOARD_CB_MEMBER(atari_common_state::a5200_keypads)
{
	int ipt;
	UINT8 ret = 0x00;

	/* decode special */
	switch (k543210)
	{
	case pokey_device::POK_KEY_BREAK:
		/* special case ... */
		ret |= ((m_keypad[0] && m_keypad[0]->read() & 0x01) ? 0x02 : 0x00);
		break;
	case pokey_device::POK_KEY_CTRL:
		break;
	case pokey_device::POK_KEY_SHIFT:
		// button 2 from joypads
		ipt = m_djoy_b->read() & (0x10 << ((k543210 >> 3) & 0x03));
		ret |= !ipt ? 0x02 : 0;
		break;
	}

	/* decode regular key */
	/* if kr5 and kr0 not set just return */
	if ((k543210 & 0x21) != 0x21)
		return ret;

	k543210 = (k543210 >> 1) & 0x0f;

	/* return on BREAK key now! */
	if (k543210 == 0)
		return ret;

	ipt = m_keypad[k543210 >> 2] ? m_keypad[k543210 >> 2]->read() : 0;

	if (ipt & (1 << (k543210 & 0x03)))
		ret |= 0x01;

	return ret;
}
