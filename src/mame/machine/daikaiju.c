/***************************************************************************

Daikaijuu no Gyakushuu protection sim

Tomasz Slanina 20060529


Communciation from CPU to MCU is encrypted - every byte
is xored with data taken from cyclic , 22 bytes long buffer.

MCU commands:
=============

 0x19 - unknown
----------------
 - no args
 - no results
Few writes at the end of level (level summary screen).


 0x1d - watchdog?
------------------
 - no args
 - no results


  0x31 - VRAM address 1
-----------------------
 - args = row
          column
 - result = 0xd000 + column*2 + row*64
 Address in text layer.


 0x32 - VRAM address 2
-----------------------
 - args = row
          column
 - result = 0xc000 + column*2 + row*64
 Address in BG layer.


 0x36 - Controls 1
------------------
 - args = data from input port
          previous result
 - result - bits 0-2 = direction ,
   encoded as:
    0
   7 1
  6   2
   5 3
    4
 - result - bit 3 = 0 -> no movement (bits 0-2 = bits 0-2 of previous result
                    1 -> movement
 - result - bit 4 = 0/1 -> button A pressed/released
 - result - bit 5 = previous status of button A
 - result - bit 6 = 0/1 -> button B pressed/released
 - result - bit 7 = previous status of button B

 0x40 - Player message(s) status (not working ?)
-------------------------------------------------
 - args = unk (0/9/d/f (stored at $a020))
          unk (0/1/2 (stored at $a023))
          unk (0/1/2 (stored at $a024)) (2nd and 3rd args are ident.)
 - result = 0/1/2/ (same as 2nd/3rd arg)
Result is discarded due to bug (or patch?) in the code - missing "cp 0"/"or a"/"and a"/etc instruction
to set CPU flags before conditional jump at addr. 0x256 (previous flags are tested, Z is _always_ set
(it's result of testing MCU "data ready to send" flag).
Thic mcu command is used to determine if player message(READY/PLAYER1/PLAYER2) should be
displayed (game) or not (attract mode). Due to above bug message is always displayed
(confirmed on real hw).

 0x44 -  Controls 2
--------------------
- args = result of command 0x36
         previous result of command 44
         step (1 or 2)
- result  - bits 0-3 = direction encoded like in result of command 0x36, but
                       16 steps instead 8. This command calculates all steps between
                       new direction (1st arg) and old one with given step.
                       Example:
                       w (2,0,1)  r (1)
                       w (2,1,1)  r (2)
                       w (2,2,1)  r (3)
                       w (2,3,1)  r (4)
                       w (2,4,1)  r (4)
                       ...
                       w (2,4,1)  r (4)
          - bit 4 = 1 -> result bits 0-3 == prev result bits 0-3, otherwise 0
          - bits 5-7 = unknown , always 0 ?

 0x60 - MCU ID
---------------
 - no args
 - result = 0x5a


***************************************************************************/
#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/lsasquad.h"

#define ID_DAIKAIJU 0x5a

#define MCU_UNKNOWN   0x19
#define MCU_WATCHDOG  0x1d
#define MCU_ADDRESS1  0x31
#define MCU_ADDRESS2  0x32
#define MCU_CONTROLS1 0x36
#define MCU_MESSAGE   0x40
#define MCU_CONTROLS2 0x44
#define MCU_ID        0x60

#define MCU_ADDRESS1_LENGTH  2
#define MCU_ADDRESS2_LENGTH  2
#define MCU_CONTROLS1_LENGTH 2
#define MCU_MESSAGE_LENGTH   3
#define MCU_CONTROLS2_LENGTH 3
#define MCU_ID_LENGTH        0

#define SET_COMMAND(n)	state->daikaiju_command = data; \
			state->daikaiju_length = n;\
			state->daikaiju_cnt = 0;\
			state->daikaiju_cntr = 0;


static const int xortable[]=
{
 0xF5, 0xD5, 0x6A, 0x26, 0x00, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x16, 0x00, 0xCB, 0x23, 0x19,
 0x11, 0x00, 0xC0, 0x19, 0xD1, 0xF1, 0xC9, -1
};

WRITE8_HANDLER( daikaiju_mcu_w )
{
	lsasquad_state *state = (lsasquad_state *)space->machine->driver_data;

	if (state->daikaiju_xor < 0)
	{
		//reset
		state->daikaiju_xor = 0;
		data ^= 0xc3;
	}
	else
	{
		data ^= xortable[state->daikaiju_xor];
		//check for end of table
		if (xortable[++state->daikaiju_xor] < 0)
		{
			state->daikaiju_xor = 0;
		}
	}
	state->daikaiju_prev = data;

	if (state->daikaiju_length == 0) //new command
	{
		switch(data)
		{
			case MCU_UNKNOWN:
			case MCU_WATCHDOG: break;
			case MCU_ADDRESS1:  SET_COMMAND(MCU_ADDRESS1_LENGTH); break;
			case MCU_ADDRESS2:  SET_COMMAND(MCU_ADDRESS2_LENGTH); break;
			case MCU_CONTROLS1: SET_COMMAND(MCU_CONTROLS1_LENGTH); break;
			case MCU_MESSAGE:	SET_COMMAND(MCU_MESSAGE_LENGTH); break;
			case MCU_CONTROLS2: SET_COMMAND(MCU_CONTROLS2_LENGTH); break;
			case MCU_ID:			SET_COMMAND(MCU_ID_LENGTH); break;
			default:
				state->daikaiju_command = data;
				logerror("Unknown MCU command W %x %x \n",data,cpu_get_pc(space->cpu));
		}
	}
	else
	{
		--state->daikaiju_length;
		state->daikaiju_buffer[state->daikaiju_cnt++] = data;
	}
}

READ8_HANDLER( daikaiju_mcu_r )
{
	lsasquad_state *state = (lsasquad_state *)space->machine->driver_data;

	switch (state->daikaiju_command)
	{
		case MCU_ID:
			return ID_DAIKAIJU;

		case MCU_CONTROLS1:
		{
			int n;
			switch ((state->daikaiju_buffer[0] & 0xf) ^ 0xf)
			{
				case 0x00:	n = (state->daikaiju_buffer[1] & (~8)) & 0xf; break;
				case 0x08:	n = 0 | 8; break;
				case 0x0a:	n = 1 | 8; break;
				case 0x02:	n = 2 | 8; break;
				case 0x06:	n = 3 | 8; break;
				case 0x04:	n = 4 | 8; break;
				case 0x05:	n = 5 | 8; break;
				case 0x01:	n = 6 | 8; break;
				case 0x09:	n = 7 | 8; break;

				default:		n = 0; break;
			}

			if (!(state->daikaiju_buffer[0] & 0x10)) //button 1 pressed
			{
				if (state->daikaiju_buffer[1] & 0x10) //previous status
				{
					n |= 0x40;	// button 0->1
				}
				else
				{
					n |= 0; //button 1->1
				}
			}
			else
			{
				if (state->daikaiju_buffer[1] & 0x10) //previous status
				{
					n |= 0x40 + 0x10;	// button 0->0
				}
				else
				{
					n |= 0x10; //button 1->0
				}
			}

			if (!(state->daikaiju_buffer[0] & 0x20)) //button 2 pressed
			{
				if (state->daikaiju_buffer[1] & 0x20) //previous status
				{
					n |= 0x80;	// button 0->1
				}
				else
				{
					n |= 0; //button 1->1
				}
			}
			else
			{
				if (state->daikaiju_buffer[1] & 0x20) //previous status
				{
					n |= 0x80 + 0x20;	// button 0->0
				}
				else
				{
					n |= 0x20; //button 1->0
				}
			}
			return n;
		}

		case MCU_ADDRESS1:
		{
			int address = state->daikaiju_buffer[1] * 2 + 64 * state->daikaiju_buffer[0];

			address &= 0x0fff; // ?
			address += 0xd000;

			if (state->daikaiju_cntr == 0)
			{
				state->daikaiju_cntr++;
				return address & 0xff;
			}
			else
			{
				return address >> 8;
			}
		}

		case MCU_ADDRESS2:
		{
			int address = state->daikaiju_buffer[1] * 2 + 64 * state->daikaiju_buffer[0];

			address &= 0x1fff; // ?
			address += 0xc000;

			if (state->daikaiju_cntr == 0)
			{
				state->daikaiju_cntr++;
				return address & 0xff;
			}
			else
			{
				return address >> 8;
			}
		}

		case MCU_MESSAGE:
			return state->daikaiju_buffer[1];

		case MCU_CONTROLS2:
		{
			int n;
			int dest = (state->daikaiju_buffer[0] & 0x7) << 1;
			int prev = (state->daikaiju_buffer[1] & 0xf);

			if (state->daikaiju_buffer[2] == 2)
			{
				prev &= 0xfe;
			}

			if (prev != dest)
			{
				int diff = (dest - prev);

				if ((diff > 8)|| (diff < 0 && diff > -8))
				{
					n = (prev - state->daikaiju_buffer[2]) & 0xf;
				}
				else
				{
					n = (prev + state->daikaiju_buffer[2]) & 0xf;
				}
			}
			else
			{
				n = prev;
			}
			prev &= 0xf;

			if (prev != n)
			{
				n |= 0x10;
			}
			return n;
		}
	}
	logerror("Unknown MCU R %x %x %x %x\n", cpu_get_pc(space->cpu), state->daikaiju_command, state->daikaiju_length, state->daikaiju_prev);
	return 0xff;
}

READ8_HANDLER( daikaiju_mcu_status_r )
{
	lsasquad_state *state = (lsasquad_state *)space->machine->driver_data;
	int res = input_port_read(space->machine, "MCU?");

	res ^= mame_rand(space->machine) & 3;
	res |= ((state->sound_pending & 0x02) ^ 2) << 3; //inverted flag
	state->sound_pending &= ~0x02;
	return res;
}
