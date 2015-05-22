// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/arkanoid.h"


/* To log specific reads and writes of the bootlegs */
#define ARKANOID_BOOTLEG_VERBOSE 1


READ8_MEMBER(arkanoid_state::arkanoid_Z80_mcu_r)
{
	/* return the last value the 68705 wrote, and mark that we've read it */
	m_m68705write = 0;
	return m_toz80;
}

TIMER_CALLBACK_MEMBER(arkanoid_state::test)
{
	m_z80write = 1;
	m_fromz80 = param;
}

WRITE8_MEMBER(arkanoid_state::arkanoid_Z80_mcu_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(arkanoid_state::test),this), data);
	/* boost the interleave for a few usecs to make sure it is read successfully */
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(10));
}

READ8_MEMBER(arkanoid_state::arkanoid_68705_port_a_r)
{
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(arkanoid_state::arkanoid_68705_port_a_w)
{
	m_port_a_out = data;
}

WRITE8_MEMBER(arkanoid_state::arkanoid_68705_ddr_a_w)
{
	m_ddr_a = data;
}


READ8_MEMBER(arkanoid_state::arkanoid_68705_port_c_r)
{
	int res = 0;

	/* bit 0 is high on a write strobe; clear it once we've detected it */
	if (m_z80write)
		res |= 0x01;

	/* bit 1 is high if the previous write has been read */
	if (!m_m68705write)
		res |= 0x02;

	return (m_port_c_out & m_ddr_c) | (res & ~m_ddr_c);
}

WRITE8_MEMBER(arkanoid_state::arkanoid_68705_port_c_w)
{
	if ((m_ddr_c & 0x04) && (~data & 0x04) && (m_port_c_out & 0x04))
	{
		/* return the last value the Z80 wrote */
		m_z80write = 0;
		m_port_a_in = m_fromz80;
	}
	if ((m_ddr_c & 0x08) && (~data & 0x08) && (m_port_c_out & 0x08))
	{
		/* a write from the 68705 to the Z80; remember its value */
		m_m68705write = 1;
		m_toz80 = m_port_a_out;
	}

	m_port_c_out = data;
}

WRITE8_MEMBER(arkanoid_state::arkanoid_68705_ddr_c_w)
{
	m_ddr_c = data;
}

CUSTOM_INPUT_MEMBER(arkanoid_state::arkanoid_68705_input_r)
{
	int res = 0;

	/* bit 0x40 of comes from the sticky bit */
	if (!m_z80write)
		res |= 0x01;

	/* bit 0x80 comes from a write latch */
	if (!m_m68705write)
		res |= 0x02;

	return res;
}

CUSTOM_INPUT_MEMBER(arkanoid_state::arkanoid_input_mux)
{
	const char *tag1 = (const char *)param;
	const char *tag2 = tag1 + strlen(tag1) + 1;
	return ioport((m_paddle_select == 0) ? tag1 : tag2)->read();
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


#define LOG_F000_R if (ARKANOID_BOOTLEG_VERBOSE) logerror("%04x: arkanoid_bootleg_f000_r - cmd = %02x - val = %02x\n", space.device().safe_pc(), m_bootleg_cmd, arkanoid_bootleg_val);
#define LOG_F002_R if (ARKANOID_BOOTLEG_VERBOSE) logerror("%04x: arkanoid_bootleg_f002_r - cmd = %02x - val = %02x\n", space.device().safe_pc(), m_bootleg_cmd, arkanoid_bootleg_val);
#define LOG_D018_W if (ARKANOID_BOOTLEG_VERBOSE) logerror("%04x: arkanoid_bootleg_d018_w - data = %02x - cmd = %02x\n", space.device().safe_pc(), data, m_bootleg_cmd);
#define LOG_D008_R if (ARKANOID_BOOTLEG_VERBOSE) logerror("%04x: arkanoid_bootleg_d008_r - val = %02x\n", space.device().safe_pc(), arkanoid_bootleg_d008_val);


/* Kludge for some bootlegs that read this address */
READ8_MEMBER(arkanoid_state::arkanoid_bootleg_f000_r)
{
	UINT8 arkanoid_bootleg_val = 0x00;

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
			LOG_F000_R
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
			LOG_F000_R
			break;
		default:
			logerror("%04x: arkanoid_bootleg_f000_r - cmd = %02x - unknown bootleg !\n", space.device().safe_pc(), m_bootleg_cmd);
			break;
	}

	return arkanoid_bootleg_val;
}

/* Kludge for some bootlegs that read this address */
READ8_MEMBER(arkanoid_state::arkanoid_bootleg_f002_r)
{
	UINT8 arkanoid_bootleg_val = 0x00;

	switch (m_bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			switch (m_bootleg_cmd)
			{
				default:
					break;
			}
			LOG_F002_R
			break;
		case ARKANGC2:  /* There are no reads from 0xf002 in these bootlegs */
		case BLOCK2:
			switch (m_bootleg_cmd)
			{
				default:
					break;
			}
			LOG_F002_R
			break;
		case ARKBLOC2:
			switch (m_bootleg_cmd)
			{
				default:
					break;
			}
			LOG_F002_R
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
			LOG_F002_R
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
			LOG_F002_R
			break;
		default:
			logerror("%04x: arkanoid_bootleg_f002_r - cmd = %02x - unknown bootleg !\n", space.device().safe_pc(), m_bootleg_cmd);
			break;
	}

	return arkanoid_bootleg_val;
}

/* Kludge for some bootlegs that write this address */
WRITE8_MEMBER(arkanoid_state::arkanoid_bootleg_d018_w)
{
	m_bootleg_cmd = 0x00;

	switch (m_bootleg_id)
	{
		case ARKANGC:
		case ARKBLOCK:
			switch (data)
			{
				case 0x36:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x0313 -> 0x0340) */
					if (space.device().safe_pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : no call 0x2050, unused A and fixed HL (0x7bd5) */
					if (space.device().safe_pc() == 0x7b87)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x7b77 -> 0x7c1c) */
					if (space.device().safe_pc() == 0x9661)
						m_bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x00) and fixed HL (0xed83) */
					if (space.device().safe_pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : 3 * 'NOP' at 0x034f + 2 * 'NOP' at 0x35b */
					if (space.device().safe_pc() == 0x0349)
						m_bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : no call 0x2050, unused A and overwritten HL (0x7c4f -> 0x7d31) */
					if (space.device().safe_pc() == 0x9670)
						m_bootleg_cmd = 0x00;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case ARKANGC2:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (space.device().safe_pc() == 0x7c4c)
						m_bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (space.device().safe_pc() == 0x7b87)
						m_bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					if (space.device().safe_pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
					break;
				case 0x89:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e5)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : call 0x2050 but fixed A (0xa5) */
					if (space.device().safe_pc() == 0x9661)
						m_bootleg_cmd = 0x00;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e7)
						m_bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (space.device().safe_pc() == 0x67e9)
						m_bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : call 0x2050 but fixed A (0xe2) */
					if (space.device().safe_pc() == 0x9670)
						m_bootleg_cmd = 0x00;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case BLOCK2:
			switch (data)
			{
				case 0x05:  /* Check 1 */
					if (space.device().safe_pc() == 0x0363)
						m_bootleg_cmd = 0x05;
					break;
				case 0x0a:  /* Check 2 */
					if (space.device().safe_pc() == 0x0372)
						m_bootleg_cmd = 0x0a;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case ARKBLOC2:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (space.device().safe_pc() == 0x7c4c)
						m_bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (space.device().safe_pc() == 0x7b87)
						m_bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					if (space.device().safe_pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
					break;
				case 0x89:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e5)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* unneeded value : call 0x2050 but unused HL and fixed DE (0x7c1c) */
					if (space.device().safe_pc() == 0x9661)
						m_bootleg_cmd = 0x00;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e7)
						m_bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (space.device().safe_pc() == 0x67e9)
						m_bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : call 0x2050 but never called (check code at 0x0340) */
					if (space.device().safe_pc() == 0x0349)
						m_bootleg_cmd = 0x00;
					break;
				case 0xff:  /* unneeded value : no call 0x2050, unused A and fixed HL (0x7d31) */
					if (space.device().safe_pc() == 0x9670)
						m_bootleg_cmd = 0x00;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case ARKGCBL:
			switch (data)
			{
				case 0x36:  /* unneeded value : call 0x2050 but fixed A (0x2d) */
					if (space.device().safe_pc() == 0x7c4c)
						m_bootleg_cmd = 0x00;
					break;
				case 0x38:  /* unneeded value : call 0x2050 but fixed A (0xf3) */
					if (space.device().safe_pc() == 0x7b87)
						m_bootleg_cmd = 0x00;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					if (space.device().safe_pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
				case 0x89:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e5)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (space.device().safe_pc() == 0x9661)
						m_bootleg_cmd = data;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e7)
						m_bootleg_cmd = 0x00;
					break;
				case 0xe3:  /* unneeded value : call 0x2050 but fixed A (0x61) */
					if (space.device().safe_pc() == 0x67e9)
						m_bootleg_cmd = 0x00;
					break;
				case 0xf7:  /* unneeded value : 3 * 'NOP' at 0x034f + 'JR NZ,$035D' at 0x35b */
					if (space.device().safe_pc() == 0x0349)
						m_bootleg_cmd = 0x00;
					break;
				case 0xff:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (space.device().safe_pc() == 0x9670)
						m_bootleg_cmd = data;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;
		case PADDLE2:
			switch (data)
			{
				case 0x24:  /* A read from 0xf002 (expected to be 0x9b) */
					if (space.device().safe_pc() == 0xbd7a)
						m_bootleg_cmd = data;
					break;
				case 0x36:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (space.device().safe_pc() == 0x7c4c)
						m_bootleg_cmd = data;
					break;
				case 0x38:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (space.device().safe_pc() == 0x7b87)
						m_bootleg_cmd = data;
					break;
				case 0x88:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e3)
						m_bootleg_cmd = 0x00;
					if (space.device().safe_pc() == 0x7c47)
						m_bootleg_cmd = 0x00;
				case 0x89:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e5)
						m_bootleg_cmd = 0x00;
					break;
				case 0x8a:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (space.device().safe_pc() == 0x9661)
						m_bootleg_cmd = data;
					break;
				case 0xc0:  /* unneeded value : no read back */
					if (space.device().safe_pc() == 0x67e7)
						m_bootleg_cmd = 0x00;
					break;
				case 0xc3:  /* A read from 0xf002 (expected to be 0x1d) */
					if (space.device().safe_pc() == 0xbd8a)
						m_bootleg_cmd = data;
					break;
				case 0xe3:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (space.device().safe_pc() == 0x67e9)
						m_bootleg_cmd = data;
					break;
				case 0xf7:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (space.device().safe_pc() == 0x0349)
						m_bootleg_cmd = data;
					break;
				case 0xff:  /* call 0x2050 with A read from 0xf002 and wrong HL */
					if (space.device().safe_pc() == 0x9670)
						m_bootleg_cmd = data;
					break;
				default:
					m_bootleg_cmd = 0x00;
					break;
			}
			LOG_D018_W
			break;

		default:
			logerror("%04x: arkanoid_bootleg_d018_w - data = %02x - unknown bootleg !\n", space.device().safe_pc(), data);
			break;
	}
}

#ifdef UNUSED_CODE
READ8_MEMBER(arkanoid_state::block2_bootleg_f000_r)
{
	return m_bootleg_cmd;
}
#endif

/* Kludge for some bootlegs that read this address */
READ8_MEMBER(arkanoid_state::arkanoid_bootleg_d008_r)
{
	UINT8 arkanoid_bootleg_d008_bit[8];
	UINT8 arkanoid_bootleg_d008_val;
	UINT8 arkanoid_paddle_value = ioport("MUX")->read();
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
			logerror("%04x: arkanoid_bootleg_d008_r - unknown bootleg !\n",space.device().safe_pc());
			break;
	}

	arkanoid_bootleg_d008_val = 0;
	for (b = 0; b < 8; b++)
		arkanoid_bootleg_d008_val |= (arkanoid_bootleg_d008_bit[b] << b);

	LOG_D008_R

	return arkanoid_bootleg_d008_val;
}
