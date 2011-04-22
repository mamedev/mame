/*******************************************************************************************************************

Nichibutsu 1414M4 device emulation

Written by Angelo Salese, based on researches by Tomasz Slanina with Legion

This is some fancy MCU / blitter that copies text strings in various Nihon Bussan games;

TODO:
- Device-ify this;
- merge implementations
- where is the condition that makes "insert coin" text to properly blink?
- first byte meaning is completely unknown;

Notes:
- Just before any string in the "MCU" rom, there's a control byte, this meaning is as follows:
  0?-- ---- ---- ---- interpret as data?
  10-- ---- ---- ---- single string transfer
  11-- ---- ---- ---- src -> dst copy, if destination != 0 fixed src, otherwise do a src -> dst
  --xx xxxx xxxx xxxx destination offset in the VRAM tilemap

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
		if(i+dst+0x000 < 18)
			continue;

		state->m_text_videoram[i+dst+0x000] = (condition) ? (data[i+(0)+src] & 0xff) : 0x20;
		state->m_text_videoram[i+dst+0x400] = data[i+(size)+src] & 0xff;
	}
}

static void legion_layer_clear(address_space *space,UINT16 dst,UINT8 tile,UINT8 pal)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	int i;

	for(i=0;i<0x400;i++)
	{
		if(i+dst+0x000 < 18)
			continue;

		state->m_text_videoram[i+dst+0x000] = tile;
		state->m_text_videoram[i+dst+0x400] = pal;
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
	UINT16 dst;

	if(credit_count == 0)
	{
		dst = (data[0x01]<<8|data[0x02]) & 0x7fff;

		for(i=0;i<0x10;i++) /* INSERT COIN */
		{
			state->m_text_videoram[i+dst+0x0000] = (fl_cond) ? 0x20 : data[i+0x00+0x0003] & 0xff;
			state->m_text_videoram[i+dst+0x0400] = data[i+0x10+0x0003] & 0xff;
		}
	}
	else
	{
		dst = (data[0x49]<<8|data[0x4a]) & 0x7fff;

		for(i=0;i<0x18;i++) /* PUSH START BUTTON */
		{
			state->m_text_videoram[i+dst+0x0000] = data[i+0x00+0x004b] & 0xff;
			state->m_text_videoram[i+dst+0x0400] = data[i+0x18+0x004b] & 0xff;
		}
	}
}

static void credit_msg(address_space *space)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;
	int credit_count = (state->m_text_videoram[0xf] & 0xff);
	UINT8 fl_cond = space->machine().primary_screen->frame_number() & 0x10; /* for insert coin "flickering" */
	UINT8 tile_base, pal_base;

	tile_base = data[0x47];
	pal_base = data[0x48];

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

static void	kozure_score_msg(address_space *space,UINT16 dst,UINT8 src_base)
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

static void service_mode(address_space *space, UINT8 is2p)
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

	terrafu_sm_transfer(space,0x2d8 + (is2p * 0x18),0x1e4,12,1); // 1p / 2p

	for(i=0;i<5;i++) /* coin inputs */
		terrafu_sm_onoff(space,0x06c + (i * 0x20),(state->m_text_videoram[0x04] >> (4-i)) & 1);

	for(i=0;i<7;i++) /* 1p inputs */
		terrafu_sm_onoff(space,0x10c + (i * 0x20),(state->m_text_videoram[0x02 + is2p] >> (6-i)) & 1);
}

static void nichibutsu_1414m4_0200(address_space *space, UINT16 mcu_cmd)
{
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	UINT16 dst;

	dst = (data[0x330+((mcu_cmd & 0xf)*2)]<<8)|(data[0x331+((mcu_cmd & 0xf)*2)]&0xff);

	dst &= 0x3fff;

	if(dst & 0x7ff) // fill
		legion_layer_clear(space,0x0000,data[dst & 0x3fff],data[dst+1]);
	else // src -> dst
		terrafu_sm_transfer(space,dst & 0x3fff,0x0000,0x400,1);
}

void terrafu_mcu_exec(address_space *space,UINT16 mcu_cmd)
{
	switch(mcu_cmd & 0xff00)
	{
		case 0x0000: /* title screen / continue */
			insert_coin_msg(space);
			credit_msg(space);
			break;

		case 0x0200: /* direct DMA'ing / fill */
			nichibutsu_1414m4_0200(space,mcu_cmd & 0x7);
			break;

		case 0x0600: /* service mode */
			service_mode(space,mcu_cmd & 1);
			break;

		case 0x0e00:
			if(!(mcu_cmd & 4))
			{
				insert_coin_msg(space);
				credit_msg(space);

				terrafu_sm_transfer(space,0x0135,0x01a8,0x10,!(mcu_cmd & 1)); /* game over */
			}
			break;
		//default:
			//printf("%04x\n",mcu_cmd);
	}
}

void kozure_mcu_exec(address_space *space,UINT16 mcu_cmd)
{
	switch(mcu_cmd & 0xff00)
	{
		case 0x0000: /* title screen / continue */
			insert_coin_msg(space);
			credit_msg(space);
			break;

		case 0x0200: /* direct DMA'ing / fill */
			nichibutsu_1414m4_0200(space,mcu_cmd & 0x7);
			break;

		case 0x0600:
			service_mode(space,mcu_cmd & 1);
			break;

		case 0x0e00: /* 1p / hi-score msg / 2p + points */
			terrafu_sm_transfer(space,0x00e1,0x03ac,8,1); /* hi-score */
			if(mcu_cmd & 0x04)
			{
				terrafu_sm_transfer(space,0x00fd,0x03a0,8,!(mcu_cmd & 1)); /* 1p-msg */
				kozure_score_msg(space,0x380,0); /* 1p score */
				if(mcu_cmd & 0x80)
				{
					terrafu_sm_transfer(space,0x0119,0x03b8,8,!(mcu_cmd & 2)); /* 2p-msg */
					kozure_score_msg(space,0x398,1); /* 2p score */
				}
			}
			else
			{
				terrafu_sm_transfer(space,0x0135,0x0128,0x10,!(mcu_cmd & 1)); /* game over */
				insert_coin_msg(space);
				//credit_msg(space,0x30,0x30);
			}
			break;

		//default:
		//	printf("%04x\n",mcu_cmd);
	}
}

void legion_mcu_exec(address_space *space,UINT16 mcu_cmd)
{
	switch(mcu_cmd & 0xff00)
	{
		case 0x0000: /* title screen / continue */
			insert_coin_msg(space);
			credit_msg(space);
			break;

		case 0x0200: /* direct DMA'ing / fill */
			nichibutsu_1414m4_0200(space,mcu_cmd & 0x7);
			break;

		case 0x0600:
			service_mode(space,mcu_cmd & 1);
			break;

		case 0x0e00: /* 1p / hi-score msg / 2p + points */
			terrafu_sm_transfer(space,0x00e1,0x080c,8,1); /* hi-score */
			if(mcu_cmd & 0x04)
			{
				terrafu_sm_transfer(space,0x00fd,0x03a0,8,!(mcu_cmd & 1)); /* 1p-msg */
				//kozure_score_msg(space,0x380,0); /* 1p score */
				if(mcu_cmd & 0x80)
				{
					terrafu_sm_transfer(space,0x0119,0x03b8,8,!(mcu_cmd & 2)); /* 2p-msg */
					//kozure_score_msg(space,0x398,1); /* 2p score */
				}
			}
			else
			{
				terrafu_sm_transfer(space,0x0135,0x0128,0x10,!(mcu_cmd & 1)); /* game over */
				insert_coin_msg(space);
				//credit_msg(space,0x30,0x30);
			}
			break;

		//default:
		//	printf("%04x\n",mcu_cmd);
	}
}
