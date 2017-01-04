// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Stephane Humbert
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/arkanoid.h"
#include "cpu/m6805/m6805.h"


/* To log specific reads and writes of the bootlegs */
#define ARKANOID_BOOTLEG_VERBOSE 1


READ8_MEMBER(arkanoid_state::arkanoid_Z80_mcu_r)
{
	/* return the last value the 68705 wrote, and mark that we've read it */
	m_MCUHasWritten = 0;
	return m_fromMCU;
}

WRITE8_MEMBER(arkanoid_state::arkanoid_Z80_mcu_w)
{
	m_Z80HasWritten = 1;
	m_fromZ80 = data;
	m_mcu->set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
}

READ8_MEMBER(arkanoid_state::mcu_porta_r)
{
	return m_portA_in;
}

WRITE8_MEMBER(arkanoid_state::mcu_porta_w)
{
	m_portA_out = data;
}

READ8_MEMBER(arkanoid_state::mcu_portc_r)
{
	int portC_in = 0;

	/* bit 0 is latch 1 on ic26, is high if m_Z80HasWritten(latch 1) is set */
	if (m_Z80HasWritten)
		portC_in |= 0x01;

	/* bit 1 is the negative output of latch 2 on ic26, is high if m_68705write is clear */
	if (!m_MCUHasWritten)
		portC_in |= 0x02;

	/* bit 2 is an output, to clear latch 1, return whatever state it was set to in m_portC_out */
	/* bit 3 is an output, to set latch 2, return whatever state it was set to in m_portC_out */

	return portC_in;
}

READ8_MEMBER(arkanoid_state::mcu_portb_r)
{
	return ioport("MUX")->read();
}


WRITE8_MEMBER(arkanoid_state::mcu_portc_w)
{
	/* bits 0 and 1 are inputs, should never be set as outputs here. if they are, ignore them. */
	/* bit 2 is an output, to clear latch 1(m_Z80HasWritten) on rising edge, and enable the z80->68705 communication latch on level low */
	// if 0x04 rising edge, clear m_Z80HasWritten/latch 1 (and clear the irq line)
	if ((~m_old_portC_out&0x04) && (data&0x04))
	{
		m_Z80HasWritten = 0;
		m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	}

	// if 0x04 low, enable the m_portA_in latch, otherwise set the latch value to 0xFF
	if (~data&0x04)
		m_portA_in = m_fromZ80;
	else
		m_portA_in = 0xFF;

	/* bit 3 is an output, to set latch 2(m_MCUHasWritten) and latch the port_a value into the 68705->z80 latch, on falling edge or low level */
	// if 0x08 low, set m_MCUHasWritten/latch 2
	if (~data&0x08)
	{
		/* a write from the 68705 to the Z80; remember its value */
		m_MCUHasWritten = 1;
		m_fromMCU = m_portA_out;
	}

	m_old_portC_out = data;
}



CUSTOM_INPUT_MEMBER(arkanoid_state::arkanoid_semaphore_input_r)
{
	int res = 0;

	/* bit 0x40 is latch 1 on ic26, is high if m_Z80HasWritten(latch 1) is clear */
	if (!m_Z80HasWritten)
		res |= 0x01;

	/* bit 0x80 is the negative output of latch 2 on ic26, is high if m_MCUHasWritten is clear */
	if (!m_MCUHasWritten)
		res |= 0x02;

	return res;
}

CUSTOM_INPUT_MEMBER(arkanoid_state::arkanoid_input_mux)
{
	const char *tag1 = (const char *)param;
	const char *tag2 = tag1 + strlen(tag1) + 1; // the f***? is this right? are we intentionally pointer-mathing off the end of one array to hit another?
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
	uint8_t arkanoid_bootleg_d008_bit[8];
	uint8_t arkanoid_bootleg_d008_val;
	uint8_t arkanoid_paddle_value = ioport("MUX")->read();
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
