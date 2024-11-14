// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Stephane Humbert
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "arkanoid.h"

/* To log specific reads and writes of the bootlegs */
#define LOG_F000_R (1U << 1)
#define LOG_F002_R (1U << 2)
#define LOG_D018_W (1U << 3)
#define LOG_D008_R (1U << 4)

#define VERBOSE (LOG_F000_R | LOG_F002_R | LOG_D018_W | LOG_D008_R)
#include "logmacro.h"


ioport_value arkanoid_state::arkanoid_semaphore_input_r()
{
	// bit 0 is host semaphore flag, bit 1 is MCU semaphore flag (both active low)
	return
			((CLEAR_LINE != m_mcuintf->host_semaphore_r()) ? 0x00 : 0x01) |
			((CLEAR_LINE != m_mcuintf->mcu_semaphore_r()) ? 0x00 : 0x02);
}

uint8_t arkanoid_state::input_mux_r()
{
	return m_muxports[(0 == m_paddle_select) ? 0 : 1].read_safe(0xff);
}

/*

Bootlegs stuff

The bootlegs simulate the missing MCU behaviour with writes to 0xd018 and reads value back from 0xf002.
Fortunately, 'arkangc', 'arkangc2', 'block2', 'arkbloc2' and 'arkblock' has patched code not to bother with that.
So I've fixed 'arkbl3' and 'paddle2' to return the expected values (code is strongly similar).
However, 'block2' is the only bootleg that writes some values to 0xd018 and reads them back from 0xf000.

Some bootlegs also test some bits from 0xd008 after reading the paddle value at 0xd018.
Their effect is completely unknown but I need to set some bits to 1 so the games are playable :

  - 'arkangc'  : NO read from 0xd008 !
  - 'arkangc2' :
       * bit 1 must be set to 1 or you enter sort of endless "demo mode" when you start :
           . you can't select your starting level (it always starts at level 1)
           . you can't control the paddle (it automoves by following the ball)
           . you can use the "fire" button (the game never shoots)
           . you are awarded points as in a normal game
           . sounds are played
  - 'block2' :
       * bit 1 must be set to 1 or you enter sort of endless "demo mode" when you start :
           . you can't control the paddle (it automoves by following the ball)
           . you can use the "fire" button (the game never shoots)
           . you are awarded points as in a normal game
           . sounds are played
  - 'arkblock' : NO read from 0xd008 !
  - 'arkbloc2' :
       * bit 5 must sometimes be set to 1 or you can't reach right side of the screen
         nor select all levels at the beginning of the game
  - 'arkgcbl' :
       * bit 1 must be set to 1 or you enter sort of endless "demo mode" when you start :
           . you can't select your starting level (it always starts at level 1)
           . you can't control the paddle (it automoves by following the ball)
           . you can use the "fire" button (the game never shoots)
           . you are awarded points as in a normal game
           . sounds are played
       * bit 5 must sometimes be set to 1 or you can't reach right side of the screen
         nor select all levels at the beginning of the game
  - 'paddle2' :
       * bits 0 and 1 must be set to 1 or the paddle goes up   (joystick issue ?)
       * bits 2 and 3 must be set to 1 or the paddle goes down (joystick issue ?)
       * bit 5 must sometimes be set to 1 or you can't reach right side of the screen
         nor select all levels at the beginning of the game


TO DO (2006.09.12) :

  - understand reads from 0xd008 (even if the games are playable)
  - try to document writes to 0xd018 with unknown effect

*/


/* Kludge for some bootlegs that read this address */
uint8_t arkanoid_state::arkanoid_bootleg_f000_r()
{
	uint8_t arkanoid_bootleg_val = 0x00;

	switch (m_bootleg_id)
	{
		case ARKANGC:   /* There are no reads from 0xf000 in these bootlegs */
		case ARKBLOCK:
		case ARKANGC2:
		case ARKBLOC2:
		case ARKGCBL:
		case PADDLE2:
			switch (m_bootleg_cmd)
			{
				default:
					break;
			}
			LOGMASKED(LOG_F000_R, "%s: arkanoid_bootleg_f000_r - cmd = %02x - val = %02x\n", machine().describe_context(), m_bootleg_cmd, arkanoid_bootleg_val);
			break;
		case BLOCK2:
			switch (m_bootleg_cmd)
			{
				case 0x05:  /* Check 1 */
					arkanoid_bootleg_val = 0x05;
					break;
				case 0x0a:  /* Check 2 */
					arkanoid_bootleg_val = 0x0a;
					break;
				default:
					break;
			}
			LOGMASKED(LOG_F000_R, "%s: arkanoid_bootleg_f000_r - cmd = %02x - val = %02x\n", machine().describe_context(), m_bootleg_cmd, arkanoid_bootleg_val);
			break;
		default:
			logerror("%s: arkanoid_bootleg_f000_r - cmd = %02x - unknown bootleg !\n", machine().describe_context(), m_bootleg_cmd);
			break;
	}

	return arkanoid_bootleg_val;
}

/* Kludge for some bootlegs that read this address */
uint8_t arkanoid_state::arkanoid_bootleg_f002_r()
{
	uint8_t arkanoid_bootleg_val = 0x00;

	switch (m_bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			switch (m_bootleg_cmd)
			{
				default:
					break;
			}
			LOGMASKED(LOG_F002_R, "%s: arkanoid_bootleg_f002_r - cmd = %02x - val = %02x\n", machine().describe_context(), m_bootleg_cmd, arkanoid_bootleg_val);
			break;
		case ARKANGC2:  /* There are no reads from 0xf002 in these bootlegs */
		case BLOCK2:
			switch (m_bootleg_cmd)
			{
				default:
					break;
			}
			LOGMASKED(LOG_F002_R, "%s: arkanoid_bootleg_f002_r - cmd = %02x - val = %02x\n", machine().describe_context(), m_bootleg_cmd, arkanoid_bootleg_val);
			break;
		case ARKBLOC2:
			switch (m_bootleg_cmd)
			{
				default:
					break;
			}
			LOGMASKED(LOG_F002_R, "%s: arkanoid_bootleg_f002_r - cmd = %02x - val = %02x\n", machine().describe_context(), m_bootleg_cmd, arkanoid_bootleg_val);
			break;
		case ARKGCBL:
			switch (m_bootleg_cmd)
			{
				case 0x8a:  /* Current level (fixed routine) */
					arkanoid_bootleg_val = 0xa5;
					break;
				case 0xff:  /* Avoid "BAD HARDWARE    " message (fixed routine) */
					arkanoid_bootleg_val = 0xe2;
					break;
				default:
					break;
			}
			LOGMASKED(LOG_F002_R, "%s: arkanoid_bootleg_f002_r - cmd = %02x - val = %02x\n", machine().describe_context(), m_bootleg_cmd, arkanoid_bootleg_val);
			break;
		case PADDLE2:
			switch (m_bootleg_cmd)
			{
				case 0x24:  /* Avoid bad jump to 0x0066 */
					arkanoid_bootleg_val = 0x9b;
					break;
				case 0x36:  /* Avoid "BAD HARDWARE    " message */
					arkanoid_bootleg_val = 0x2d;
					break;
				case 0x38:  /* Start of levels table (fixed offset) */
					arkanoid_bootleg_val = 0xf3;
					break;
				case 0x8a:  /* Current level (fixed routine) */
					arkanoid_bootleg_val = 0xa5;
					break;
				case 0xc3:  /* Avoid bad jump to 0xf000 */
					arkanoid_bootleg_val = 0x1d;
					break;
				case 0xe3:  /* Number of bricks left (fixed offset) */
					arkanoid_bootleg_val = 0x61;
					break;
				case 0xf7:  /* Avoid "U69" message */
					arkanoid_bootleg_val = 0x00;
					break;
				case 0xff:  /* Avoid "BAD HARDWARE    " message (fixed routine) */
					arkanoid_bootleg_val = 0xe2;
					break;
				default:
					break;
			}
			LOGMASKED(LOG_F002_R, "%s: arkanoid_bootleg_f002_r - cmd = %02x - val = %02x\n", machine().describe_context(), m_bootleg_cmd, arkanoid_bootleg_val);
			break;
		default:
			logerror("%s: arkanoid_bootleg_f002_r - cmd = %02x - unknown bootleg !\n", machine().describe_context(), m_bootleg_cmd);
			break;
	}

	return arkanoid_bootleg_val;
}

/* Kludge for some bootlegs that write this address */
void arkanoid_state::arkanoid_bootleg_d018_w(uint8_t data)
{
	m_bootleg_cmd = 0x00;

	switch (m_bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			switch (data)
			{
				case 0x36:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x0313 -> 0x0340) */
					if (m_maincpu->pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : no call 0x2050, unused A and fixed HL (0x7bd5) */
					if (m_maincpu->pc() == 0x7b87)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x7b77 -> 0x7c1c) */
					if (m_maincpu->pc() == 0x9661)
						m_bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x00) and fixed HL (0xed83) */
					if (m_maincpu->pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : 3 * 'NOP' at 0x034f + 2 * 'NOP' at 0x35b */
					if (m_maincpu->pc() == 0x0349)
						m_bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x7c4f -> 0x7d31) */
					if (m_maincpu->pc() == 0x9670)
						m_bootleg_cmd = 0x00;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOGMASKED(LOG_D018_W, "%s: arkanoid_bootleg_d018_w - data = %02x - cmd = %02x\n", machine().describe_context(), data, m_bootleg_cmd);
			break;
		case ARKANGC2:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (m_maincpu->pc() == 0x7c4c)
						m_bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (m_maincpu->pc() == 0x7b87)
						m_bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					if (m_maincpu->pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
					break;
				case 0x89:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e5)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : call 0x2050 but fixed A (0xa5) */
					if (m_maincpu->pc() == 0x9661)
						m_bootleg_cmd = 0x00;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e7)
						m_bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (m_maincpu->pc() == 0x67e9)
						m_bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : call 0x2050 but fixed A (0xe2) */
					if (m_maincpu->pc() == 0x9670)
						m_bootleg_cmd = 0x00;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOGMASKED(LOG_D018_W, "%s: arkanoid_bootleg_d018_w - data = %02x - cmd = %02x\n", machine().describe_context(), data, m_bootleg_cmd);
			break;
		case BLOCK2:
			switch (data)
			{
				case 0x05:  /* Check 1 */
					if (m_maincpu->pc() == 0x0363)
						m_bootleg_cmd = 0x05;
					break;
				case 0x0a:  /* Check 2 */
					if (m_maincpu->pc() == 0x0372)
						m_bootleg_cmd = 0x0a;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOGMASKED(LOG_D018_W, "%s: arkanoid_bootleg_d018_w - data = %02x - cmd = %02x\n", machine().describe_context(), data, m_bootleg_cmd);
			break;
		case ARKBLOC2:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (m_maincpu->pc() == 0x7c4c)
						m_bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (m_maincpu->pc() == 0x7b87)
						m_bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					if (m_maincpu->pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
					break;
				case 0x89:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e5)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : call 0x2050 but unused HL and fixed DE (0x7c1c) */
					if (m_maincpu->pc() == 0x9661)
						m_bootleg_cmd = 0x00;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e7)
						m_bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (m_maincpu->pc() == 0x67e9)
						m_bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : call 0x2050 but never called (check code at 0x0340) */
					if (m_maincpu->pc() == 0x0349)
						m_bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : no call 0x2050, unused A and fixed HL (0x7d31) */
					if (m_maincpu->pc() == 0x9670)
						m_bootleg_cmd = 0x00;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOGMASKED(LOG_D018_W, "%s: arkanoid_bootleg_d018_w - data = %02x - cmd = %02x\n", machine().describe_context(), data, m_bootleg_cmd);
			break;
		case ARKGCBL:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (m_maincpu->pc() == 0x7c4c)
						m_bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (m_maincpu->pc() == 0x7b87)
						m_bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					if (m_maincpu->pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
					[[fallthrough]]; // FIXME: really?
				case 0x89:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e5)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (m_maincpu->pc() == 0x9661)
						m_bootleg_cmd = data;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e7)
						m_bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (m_maincpu->pc() == 0x67e9)
						m_bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : 3 * 'NOP' at 0x034f + 'JR NZ,$035D' at 0x35b */
					if (m_maincpu->pc() == 0x0349)
						m_bootleg_cmd = 0x00;
					break;
				case 0xff:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (m_maincpu->pc() == 0x9670)
						m_bootleg_cmd = data;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOGMASKED(LOG_D018_W, "%s: arkanoid_bootleg_d018_w - data = %02x - cmd = %02x\n", machine().describe_context(), data, m_bootleg_cmd);
			break;
		case PADDLE2:
			switch (data)
			{
				case 0x24:  /* A read from 0xf002 (expected to be 0x9b) */
					if (m_maincpu->pc() == 0xbd7a)
						m_bootleg_cmd = data;
					break;
				case 0x36:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (m_maincpu->pc() == 0x7c4c)
						m_bootleg_cmd = data;
					break;
				case 0x38:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (m_maincpu->pc() == 0x7b87)
						m_bootleg_cmd = data;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					if (m_maincpu->pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
					[[fallthrough]]; // FIXME: really?
				case 0x89:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e5)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (m_maincpu->pc() == 0x9661)
						m_bootleg_cmd = data;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (m_maincpu->pc() == 0x67e7)
						m_bootleg_cmd = 0x00;
					break;
				case 0xc3:  /* A read from 0xf002 (expected to be 0x1d) */
					if (m_maincpu->pc() == 0xbd8a)
						m_bootleg_cmd = data;
					break;
				case 0xe3:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (m_maincpu->pc() == 0x67e9)
						m_bootleg_cmd = data;
					break;
				case 0xf7:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (m_maincpu->pc() == 0x0349)
						m_bootleg_cmd = data;
					break;
				case 0xff:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (m_maincpu->pc() == 0x9670)
						m_bootleg_cmd = data;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOGMASKED(LOG_D018_W, "%s: arkanoid_bootleg_d018_w - data = %02x - cmd = %02x\n", machine().describe_context(), data, m_bootleg_cmd);
			break;

		default:
			logerror("%s: arkanoid_bootleg_d018_w - data = %02x - unknown bootleg !\n", machine().describe_context(), data);
			break;
	}
}

#ifdef UNUSED_CODE
uint8_t arkanoid_state::block2_bootleg_f000_r()
{
	return m_bootleg_cmd;
}
#endif

/* Kludge for some bootlegs that read this address */
uint8_t arkanoid_state::arkanoid_bootleg_d008_r()
{
	uint8_t arkanoid_bootleg_d008_bit[8];
	uint8_t arkanoid_bootleg_d008_val;
	uint8_t arkanoid_paddle_value = input_mux_r();
	int b;

	arkanoid_bootleg_d008_bit[4] = arkanoid_bootleg_d008_bit[6] = arkanoid_bootleg_d008_bit[7] = 0;  /* untested bits */

	switch (m_bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = 0;  /* untested bit */
			break;
		case ARKANGC2:
		case BLOCK2:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 1;  /* check code at 0x0cad */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = 0;  /* untested bit */
			break;
		case ARKBLOC2:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = (arkanoid_paddle_value < 0x40);  /* check code at 0x96b0 */
			break;
		case ARKGCBL:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 1;  /* check code at 0x0cad */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = (arkanoid_paddle_value < 0x40);  /* check code at 0x96b0 */
			break;
		case PADDLE2:
			arkanoid_bootleg_d008_bit[0] = 1;  /* check code at 0x7d65 */
			arkanoid_bootleg_d008_bit[1] = 1;  /* check code at 0x7d65 */
			arkanoid_bootleg_d008_bit[2] = 1;  /* check code at 0x7d65 */
			arkanoid_bootleg_d008_bit[3] = 1;  /* check code at 0x7d65 */
			arkanoid_bootleg_d008_bit[5] = (arkanoid_paddle_value < 0x40);  /* check code at 0x96b0 */
			break;
		default:
			arkanoid_bootleg_d008_bit[0] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[1] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[2] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[3] = 0;  /* untested bit */
			arkanoid_bootleg_d008_bit[5] = 0;  /* untested bit */
			logerror("%s: arkanoid_bootleg_d008_r - unknown bootleg !\n", machine().describe_context());
			break;
	}

	arkanoid_bootleg_d008_val = 0;
	for (b = 0; b < 8; b++)
		arkanoid_bootleg_d008_val |= (arkanoid_bootleg_d008_bit[b] << b);

	LOGMASKED(LOG_D008_R, "%s: arkanoid_bootleg_d008_r - val = %02x\n", machine().describe_context(), arkanoid_bootleg_d008_val);

	return arkanoid_bootleg_d008_val;
}
