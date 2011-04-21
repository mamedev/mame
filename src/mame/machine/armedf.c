/*******************************************************************************************************************

Text tilemap protection simulation for Armed Formation HW games

Written by Angelo Salese, based on researches by Tomasz Slanina with Legion

TODO:
- name of this device / MCU;


Notes:
- just before any string in the "MCU" rom, there's an offset, it indicates where the string should go in the tilemap.
   This is currently hard-coded in this handling;
- I'm sure that this is a shared device, that shares everything. All of the known differences are due of not
  understood features of the chip (some bytes in the ROM etc.)

********************************************************************************************************************/

#include "emu.h"
#include "includes/armedf.h"

static void terrafu_sm_transfer(address_space *space,UINT16 src,UINT16 dst,UINT16 size, UINT8 condition)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;

	for(i=0;i<size;i++)
	{
		state->m_text_videoram[i+dst+0x000] = (condition) ? (data[i+(0)+src] & 0xff) : 0x20;
		state->m_text_videoram[i+dst+0x400] = data[i+(size)+src] & 0xff;
	}
}

static void terrafu_sm_onoff(address_space *space,UINT16 dst,UINT8 condition)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;

	for(i=0;i<3;i++)
	{
		state->m_text_videoram[i+dst+0x000] = (condition) ? (data[i+0x0+0x316] & 0xff) : (data[i+0x0+0x310] & 0xff);
		state->m_text_videoram[i+dst+0x400] = (condition) ? (data[i+0x0+0x319] & 0xff) : (data[i+0x0+0x313] & 0xff);
	}
}

static void insert_coin_msg(address_space *space)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;
	int credit_count = (state->m_text_videoram[0xf] & 0xff);
	UINT8 fl_cond = space->machine().primary_screen->frame_number() & 0x10; /* for insert coin "flickering" */

	if(credit_count == 0)
	{
		for(i=0;i<0x10;i++) /* INSERT COIN */
		{
			state->m_text_videoram[i+0x16a+0x0000] = (fl_cond) ? 0x20 : data[i+0x00+0x0003] & 0xff;
			state->m_text_videoram[i+0x16a+0x0400] = data[i+0x10+0x0003] & 0xff;
		}
	}
	else
	{
		for(i=0;i<0x18;i++) /* PUSH START BUTTON (0x128? Gets wrong on the continue with this ...) */
		{
			state->m_text_videoram[i+0x1a8+0x0000] = data[i+0x00+0x004b] & 0xff;
			state->m_text_videoram[i+0x1a8+0x0400] = data[i+0x18+0x004b] & 0xff;
		}
	}
}

static void credit_msg(address_space *space, UINT8 tile_base,UINT8 pal_base)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;
	int credit_count = (state->m_text_videoram[0xf] & 0xff);
	UINT8 fl_cond = space->machine().primary_screen->frame_number() & 0x10; /* for insert coin "flickering" */

	for(i=0;i<0x10;i++)
	{
		state->m_text_videoram[i+0x050+0x0000] = data[i+0x00+0x0025] & 0xff;
		state->m_text_videoram[i+0x050+0x0400] = data[i+0x10+0x0025] & 0xff;
	}
	state->m_text_videoram[0x05f+0x000] = (credit_count + tile_base);
	state->m_text_videoram[0x05f+0x400] = (pal_base);

	if(credit_count == 1)
	{
		for(i=0;i<0x18;i++) /* ONE PLAYER ONLY */
		{
			state->m_text_videoram[i+0x168+0x0000] = (fl_cond) ? 0x20 : data[i+0x00+0x007d] & 0xff;
			state->m_text_videoram[i+0x168+0x0400] = data[i+0x18+0x007d] & 0xff;
		}
	}
	else if(credit_count > 1)
	{
		for(i=0;i<0x18;i++) /* ONE OR TWO PLAYERS */
		{
			state->m_text_videoram[i+0x168+0x0000] = (fl_cond) ? 0x20 : data[i+0x00+0x00af] & 0xff;
			state->m_text_videoram[i+0x168+0x0400] = data[i+0x18+0x00af] & 0xff;
		}
	}
}

static void	kodure_score_msg(address_space *space,UINT16 dst,UINT8 src_base)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	int i;
	UINT8 first_digit;
	UINT8 res;

	first_digit = 0;

	for(i=0;i<6;i++)
	{
		res = ((state->m_text_videoram[(i/2)+5+src_base*3] >> (!(i & 1) * 4)) & 0xf);

		if(first_digit || res)
		{
			state->m_text_videoram[i+dst+0x0000] = res + 0x30;
			first_digit = 1;
		}
		else
			state->m_text_videoram[i+dst+0x0000] = 0x20;

		state->m_text_videoram[i+dst+0x0400] = 0x10; // hardcoded in ROM
	}

	state->m_text_videoram[6+dst+0x0000] = 0x30;
	state->m_text_videoram[6+dst+0x0400] = 0x10; // hardcoded in ROM
	state->m_text_videoram[7+dst+0x0000] = 0x30;
	state->m_text_videoram[7+dst+0x0400] = 0x10; // hardcoded in ROM

}

static void service_mode(address_space *space)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;

	for(i=0;i<0x400;i++)
	{
		if(i < 18 || i == 0x377 || i == 0x357) // params and digits for bonus lives doesn't get overwritten
			continue;

		state->m_text_videoram[i+0x000+0x0000] = data[i+0x000+0x3000] & 0xff;
		state->m_text_videoram[i+0x000+0x0400] = data[i+0x400+0x3000] & 0xff;
	}

	state->m_text_videoram[0x252+0x000] = ((state->m_text_videoram[0x11] & 0xf0) >> 4) + 0x30;
	state->m_text_videoram[0x253+0x000] = (state->m_text_videoram[0x11] & 0x0f) + 0x30;
	//state->m_text_videoram[0x252+0x400] = (0x40);
	//state->m_text_videoram[0x253+0x400] = (0x40);

	/*
	[0x02] & 0x01 p1 up
	[0x02] & 0x02 p1 down
	[0x02] & 0x04 p1 left
	[0x02] & 0x08 p1 right
	[0x02] & 0x10 p1 button 1
	[0x02] & 0x20 p1 button 2
	[0x02] & 0x40 p1 button 3
	[0x03] & 0x01 p2 up
	[0x03] & 0x02 p2 down
	[0x03] & 0x04 p2 left
	[0x03] & 0x08 p2 right
	[0x03] & 0x10 p2 button 1
	[0x03] & 0x20 p2 button 2
	[0x03] & 0x40 p2 button 3
	[0x04] & 0x10 service
	[0x04] & 0x04 coin A
	[0x04] & 0x08 coin B
	[0x04] & 0x01 start 1
	[0x04] & 0x02 start 2
	[0x05] DSW1
	[0x06] DSW2
	[0x07] & 0x40 demo sounds ON / OFF
	[0x07] & 0x7 lives setting
	[0x07] & 0x80 cabinet (upright / table)
	[0x07] & 0x30 difficulty (easy / normal / hard / hardest)
	[0x0f] coinage A
	[0x10] coinage B
	*/
	state->m_text_videoram[0x3bb|0x000] = (state->m_text_videoram[7] & 0x7) + 0x30;
	//state->m_text_videoram[0x3bb|0x400] = (0x40);

	terrafu_sm_transfer(space,0x1fa + (((state->m_text_videoram[7] & 0x30) >> 4) * 0x18),0x390,12,1);
	terrafu_sm_transfer(space,0x264 + (((state->m_text_videoram[7] & 0x80) >> 7) * 0x18),0x330,12,1);
	terrafu_sm_transfer(space,0x296 + (((state->m_text_videoram[7] & 0x40) >> 6) * 0x18),0x310,12,1);

	state->m_text_videoram[0x2ee|0x000] = ((state->m_text_videoram[0xf] & 0xf0) >> 4) + 0x30;
	//state->m_text_videoram[0x2ee|0x400] = (0x40);
	state->m_text_videoram[0x2f5|0x000] = ((state->m_text_videoram[0xf] & 0x0f) >> 0) + 0x30;
	//state->m_text_videoram[0x2f5|0x400] = (0x40);
	state->m_text_videoram[0x2ce|0x000] = ((state->m_text_videoram[0x10] & 0xf0) >> 4) + 0x30;
	//state->m_text_videoram[0x2ce|0x400] = (0x40);
	state->m_text_videoram[0x2d5|0x000] = ((state->m_text_videoram[0x10] & 0x0f) >> 0) + 0x30;
	//state->m_text_videoram[0x2d5|0x400] = (0x40);

	for(i=0;i<8;i++) /* dips */
	{
		terrafu_sm_onoff(space,0x074 + (i * 0x20),(state->m_text_videoram[0x05] >> (7-i)) & 1);
		terrafu_sm_onoff(space,0x079 + (i * 0x20),(state->m_text_videoram[0x06] >> (7-i)) & 1);
	}

	/* TODO: inputs layout? */
}

void terrafu_mcu_exec(address_space *space,UINT16 mcu_cmd)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;

	switch(mcu_cmd)
	{
		case 0x0e00:
			break;
		case 0x0e1c: /* gameplay, unknown ... */
			break;
		case 0x0e80: /* attract demo */
			insert_coin_msg(space);
			credit_msg(space,0x10,0x40);

			for(i=0;i<0x10;i++) /* GAME OVER */
			{
				state->m_text_videoram[i+0x1a8+0x0000] = data[i+0x00+0x0135] & 0xff;
				state->m_text_videoram[i+0x1a8+0x0400] = data[i+0x10+0x0135] & 0xff;
			}
			break;
		case 0x0000: /* title screen / continue */
			insert_coin_msg(space);
			credit_msg(space,0x10,0x40);
			break;
		case 0x0280: /* layer clearances */
		case 0x0282:
			terrafu_sm_transfer(space,0x2800,0x0000,0x400,1);
			break;
		case 0x0200: /* Nichibutsu logo */
		case 0x0201:
			terrafu_sm_transfer(space,0x2000,0x0000,0x400,1);
			break;
		case 0x600: /* service mode */
			service_mode(space);
			break;
		//default:
			//printf("%04x\n",mcu_cmd);
	}
}

void kodure_mcu_exec(address_space *space,UINT16 mcu_cmd)
{
	switch(mcu_cmd)
	{
		case 0x0000: /* title screen / continue */
			insert_coin_msg(space);
			credit_msg(space,0x30,0x30);
			break;

		case 0x0280: /* layer clearances */
		case 0x0282:
			terrafu_sm_transfer(space,0x2800,0x0000,0x400,1);
			break;

		case 0x0200: /* Nichibutsu logo */
		case 0x0201:
			terrafu_sm_transfer(space,0x2000,0x0000,0x400,1);
			break;

		case 0x206: /* ranking screen */
		case 0x286:
			terrafu_sm_transfer(space,0x3800,0x0000,0x400,1);
			//if(mcu_cmd & 0x80)
			//	credit_msg(space,0x30,0x30);
			break;

		case 0xe1c: /* 1p / hi-score msg / 2p + points */
		case 0xe1d:
		case 0xe9c:
		case 0xe9d:
		case 0xe9e:
		case 0xe98:
		case 0xe18:
		case 0xe19:
		case 0xe14:
		case 0xe15:
		case 0xe94:
		case 0xe95:
		case 0xe96:
		case 0xe99:
		case 0xe9a:
			terrafu_sm_transfer(space,0x00e1,0x03ac,8,1); /* hi-score */
			if(mcu_cmd & 0x04)
			{
				terrafu_sm_transfer(space,0x00fd,0x03a0,8,!(mcu_cmd & 1)); /* 1p-msg */
				kodure_score_msg(space,0x380,0); /* 1p score */
				if(mcu_cmd & 0x80)
				{
					terrafu_sm_transfer(space,0x0119,0x03b8,8,!(mcu_cmd & 2)); /* 2p-msg */
					kodure_score_msg(space,0x398,1); /* 2p score */
				}
			}
			else
			{
				terrafu_sm_transfer(space,0x0135,0x0128,0x10,!(mcu_cmd & 1)); /* game over */
				insert_coin_msg(space);
				//credit_msg(space,0x30,0x30);
			}
			break;

		case 0x600:
			service_mode(space);
			break;

		//default:
		//	printf("%04x\n",mcu_cmd);
	}
}
