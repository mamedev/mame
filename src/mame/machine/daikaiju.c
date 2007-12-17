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
#include "driver.h"
#include "cpu/z80/z80.h"

#define ID_DAIKAIJU 0x5a

#define MCU_UNKNOWN   0x19
#define MCU_WATCHDOG  0x1d
#define MCU_ADDRESS1  0x31
#define MCU_ADDRESS2  0x32
#define MCU_CONTROLS1 0x36
#define MCU_MESSAGE   0x40
#define MCU_CONTROLS2 0x44
#define MCU_ID 				0x60

#define MCU_ADDRESS1_LENGTH  2
#define MCU_ADDRESS2_LENGTH  2
#define MCU_CONTROLS1_LENGTH 2
#define MCU_MESSAGE_LENGTH   3
#define MCU_CONTROLS2_LENGTH 3
#define MCU_ID_LENGTH				 0

#define SET_COMMAND(n)	daikaiju_command=data; \
			daikaiju_length=n;\
			daikaiju_cnt=0;\
			daikaiju_cntr=0;


extern int lsasquad_sound_pending;
static int daikaiju_xor, daikaiju_command, daikaiju_length, daikaiju_prev, daikaiju_cnt, daikaiju_cntr;

static int daikaiju_buffer[256];

static int xortable[]=
{
 0xF5, 0xD5, 0x6A, 0x26, 0x00, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x16, 0x00, 0xCB, 0x23, 0x19,
 0x11, 0x00, 0xC0, 0x19, 0xD1, 0xF1, 0xC9, -1
};

MACHINE_RESET(daikaiju)
{
	daikaiju_xor=-1;
	daikaiju_command=0;
	daikaiju_length=0;
}

WRITE8_HANDLER( daikaiju_mcu_w )
{
  if(daikaiju_xor <0)
	{
		//reset
		daikaiju_xor=0;
		data^=0xc3;
	}
	else
	{
		data^=xortable[daikaiju_xor];
		//check for end of table
		if(xortable[++daikaiju_xor]<0)
		{
			daikaiju_xor=0;
		}
	}
	daikaiju_prev=data;

	if(daikaiju_length == 0) //new command
	{
		switch(data)
		{
			case MCU_UNKNOWN:
			case MCU_WATCHDOG: break;
			case MCU_ADDRESS1:  SET_COMMAND(MCU_ADDRESS1_LENGTH); break;
			case MCU_ADDRESS2:  SET_COMMAND(MCU_ADDRESS2_LENGTH); break;
			case MCU_CONTROLS1: SET_COMMAND(MCU_CONTROLS1_LENGTH); break;
			case MCU_MESSAGE: 	SET_COMMAND(MCU_MESSAGE_LENGTH); break;
			case MCU_CONTROLS2: SET_COMMAND(MCU_CONTROLS2_LENGTH); break;
			case MCU_ID: 				SET_COMMAND(MCU_ID_LENGTH); break;
			default:
				daikaiju_command=data;
				logerror("Unknown MCU command W %x %x \n",data,activecpu_get_pc());
		}
	}
	else
	{
		--daikaiju_length;
		daikaiju_buffer[daikaiju_cnt++]=data;
	}
}

READ8_HANDLER( daikaiju_mcu_r )
{
	switch(daikaiju_command)
	{
		case MCU_ID:
			return ID_DAIKAIJU;

		case MCU_CONTROLS1:
 		{
			int n;
			switch( (daikaiju_buffer[0]&0xf)^0xf)
			{
				case 0x00:	n = (daikaiju_buffer[1]&(~8))&0xf; break;
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

			if( !(daikaiju_buffer[0]&0x10) ) //button 1 pressed
			{
				if(daikaiju_buffer[1]&0x10) //previous status
				{
					n|=0x40;	// button 0->1
				}
				else
				{
					n|=0; //button 1->1
				}
			}
			else
			{
				if(daikaiju_buffer[1]&0x10) //previous status
				{
					n|=0x40+0x10;	// button 0->0
				}
				else
				{
					n|=0x10; //button 1->0
				}
			}

			if( !(daikaiju_buffer[0]&0x20) ) //button 2 pressed
			{
				if(daikaiju_buffer[1]&0x20) //previous status
				{
					n|=0x80;	// button 0->1
				}
				else
				{
					n|=0; //button 1->1
				}
			}
			else
			{
				if(daikaiju_buffer[1]&0x20) //previous status
				{
					n|=0x80+0x20;	// button 0->0
				}
				else
				{
					n|=0x20; //button 1->0
				}
			}
			return n;
		}
		return 0;

		case MCU_ADDRESS1:
		{
			int address=daikaiju_buffer[1]*2+64*daikaiju_buffer[0];

			address&=0x0fff; // ?
			address+=0xd000;

			if(daikaiju_cntr==0)
			{
				daikaiju_cntr++;
				return address&0xff;
			}
			else
			{
				return address>>8;
			}
		}
		return 0;

		case MCU_ADDRESS2:
		{
			int address=daikaiju_buffer[1]*2+64*daikaiju_buffer[0];

			address&=0x1fff; // ?
			address+=0xc000;

			if(daikaiju_cntr==0)
			{
				daikaiju_cntr++;
				return address&0xff;
			}
			else
			{
				return address>>8;
			}
		}
		return 0;

		case MCU_MESSAGE:
			return daikaiju_buffer[1];

		case MCU_CONTROLS2:
		{
			int n;
			int dest=(daikaiju_buffer[0]&0x7)<<1;
			int prev=(daikaiju_buffer[1]&0xf);

			if(daikaiju_buffer[2]==2)
			{
				prev&=0xfe;
			}
			if(prev!=dest)
			{
				int diff=(dest-prev);

				if((diff>8 )|| (diff<0 && diff >-8))
				{
					n=(prev-daikaiju_buffer[2])&0xf;
				}
				else
				{
					n=(prev+daikaiju_buffer[2])&0xf;
				}
			}
			else
			{
				n=prev;
			}
			prev&=0xf;
			if(prev!=n)
			{
				n|=0x10;
			}
			return n;
		}
	}
	logerror("Unknown MCU R %x %x %x %x\n",activecpu_get_pc(), daikaiju_command, daikaiju_length, daikaiju_prev);
	return 0xff;
}

READ8_HANDLER( daikaiju_mcu_status_r )
{
	int res = input_port_3_r(0);

	res^=mame_rand(Machine)&3;
	res |=((lsasquad_sound_pending & 0x02)^2)<<3; //inverted flag
	lsasquad_sound_pending &= ~0x02;
	return res;
}

