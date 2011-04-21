/***************************************************************

Text tilemap protection simulation for Armed Formation HW games

***************************************************************/

#include "emu.h"
#include "includes/armedf.h"

static void terrafu_sm_transfer(address_space *space,UINT16 src,UINT16 dst,UINT8 size)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;

	for(i=0;i<size;i++)
	{
		state->m_text_videoram[i+dst+0x000] = data[i+0x0+src] & 0xff;
		state->m_text_videoram[i+dst+0x400] = data[i+0xc+src] & 0xff;
	}
}

static void terrafu_sm_onoff(address_space *space,UINT16 dst,UINT8 condition)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;
//	char on_string[] = { "O", "N", " " };
	const UINT8 on_string[4] = { "ON " };

	for(i=0;i<3;i++)
	{
		state->m_text_videoram[i+dst+0x000] = (condition) ? (data[i+0x0+0x316] & 0xff) : (on_string[i] & 0xff);
		state->m_text_videoram[i+dst+0x400] = 0x10;
	}
}

/* Note: just before any string in the "MCU" rom, there's an offset, it indicates where the string should go in the tilemap.
   This is currently hard-coded */
void terrafu_mcu_exec(address_space *space,UINT16 mcu_cmd)
{
	armedf_state *state = space->machine().driver_data<armedf_state>();
	UINT8 * data = (UINT8 *)space->machine().region("gfx5")->base();
	int i;
	int credit_count = (state->m_text_videoram[0xf] & 0xff);
	UINT8 fl_cond = space->machine().primary_screen->frame_number() & 0x10; /* for insert coin "flickering" */

	switch(mcu_cmd)
	{
		case 0x0e00:
			break;
		case 0x0e1c: /* gameplay, unknown ... */
			break;
		case 0x0e80: /* attract demo */
			for(i=0;i<0x10;i++) /* CREDIT */
			{
				state->m_text_videoram[i+0x050+0x0000] = data[i+0x00+0x0025] & 0xff;
				state->m_text_videoram[i+0x050+0x0400] = data[i+0x10+0x0025] & 0xff;
			}
			state->m_text_videoram[0x05f+0x000] = ((credit_count) + 0x10);
			state->m_text_videoram[0x05f+0x400] = (0x40);

			for(i=0;i<0x10;i++) /* INSERT COIN */
			{
				state->m_text_videoram[i+0x16a+0x0000] = (fl_cond) ? 0x20 : data[i+0x00+0x0003] & 0xff;
				state->m_text_videoram[i+0x16a+0x0400] = data[i+0x10+0x0003] & 0xff;
			}

			for(i=0;i<0x10;i++) /* GAME OVER */
			{
				state->m_text_videoram[i+0x1a8+0x0000] = data[i+0x00+0x0135] & 0xff;
				state->m_text_videoram[i+0x1a8+0x0400] = data[i+0x10+0x0135] & 0xff;
			}
			break;
		case 0x0000: /* title screen / continue */
			for(i=0;i<0x10;i++)
			{
				state->m_text_videoram[i+0x050+0x0000] = data[i+0x00+0x0025] & 0xff;
				state->m_text_videoram[i+0x050+0x0400] = data[i+0x10+0x0025] & 0xff;
			}
			state->m_text_videoram[0x05f+0x000] = (credit_count + 0x10);
			state->m_text_videoram[0x05f+0x400] = (0x40);

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

			break;
		case 0x0280: /* layer clearances */
		case 0x0282:
			for(i=0;i<0x400;i++)
			{
				state->m_text_videoram[i+0x000+0x0000] = data[i+0x000+0x2800] & 0xff;
				state->m_text_videoram[i+0x000+0x0400] = data[i+0x400+0x2800] & 0xff;
			}
			break;
		case 0x0200: /* Nichibutsu logo */
		case 0x0201:
			for(i=0;i<0x400;i++)
			{
				state->m_text_videoram[i+0x000+0x0000] = data[i+0x000+0x2000] & 0xff;
				state->m_text_videoram[i+0x000+0x0400] = data[i+0x400+0x2000] & 0xff;
			}
			break;
		case 0x600: /* service mode */
			for(i=0;i<0x400;i++)
			{
				if((state->m_text_videoram[i+0x000+0x0000] & 0xff00) == 0xff00) /* uhm, avoids bonus awards overwrite? */
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

			terrafu_sm_transfer(space,0x1fa + (((state->m_text_videoram[7] & 0x30) >> 4) * 0x18),0x390,12);
			terrafu_sm_transfer(space,0x264 + (((state->m_text_videoram[7] & 0x80) >> 7) * 0x18),0x330,12);
			terrafu_sm_transfer(space,0x296 + (((state->m_text_videoram[7] & 0x40) >> 6) * 0x18),0x310,12);

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

			break;
		//default:
			//printf("%04x\n",mcu_cmd);
	}
}
