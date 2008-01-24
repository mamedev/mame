/***************************************************************************

Legionnaire (c) Tad 1992
-----------

David Graves

Made from MAME D-con and Toki drivers (by Bryan McPhail, Jarek Parchanski)


Heated Barrel looks like a minor revision of the Legionnaire
hardware. It has a graphics banking facility, which doubles the 0xfff
different tiles available for use in the foreground layer.


Legionnaire BK3 charsets
------------------------

The GFX roms contain two odd sections of 256 16x16 tiles marked as BK3.
These need to be brought together and decoded as a single section of
0x200 tiles.

The 0x104000 area appears to be extra paletteram?


TODO
----

Unemulated protection messes up both games.

Seibu sound system mentioned in log.


Legionnaire
-----------

Foreground tiles screwy (screen after character selection screen).

Need 16 px off top of vis area?


Heated Barrel
-------------

Big problems with layers not being cleared, especially the text
layer. There may be a write to the COP controlling layer clearance?

Ends with Access violation when you die on round 1 boss. A lot of
non-existent area reads in log - maybe because of bad reads from
the COP.


Denjin Makai
------------

Palette Ram format correct,but color offset wrong?
Probably protection related,game updates paletteram with rom data...

Need a sound kludge to start.

Sprite GFX ROM order wrong,causes wrong sprite graphics.


Godzilla
--------

The COP-MCU appears to write to the work ram area,otherwise it resets in mid-animation
of the title screen.


Preliminary COP MCU memory map
------------------------------

0x400-0x5ff   Protection related:
0x400         Sprite parameter (color)
0x414-0x412   Sprite DMA source address
0x478         Layer clearance (?)
0x48e         X limiter (SD Gundam)
0x48c         Y limiter (SD Gundam)
0x4c0-0x4a0   COP reg 0
0x4c2-0x4a2   COP reg 1
0x4c4-0x4a4   COP reg 2
0x4c6-0x4a6   COP reg 3
0x580(r)      Hit Check (bit 1 & bit 0)
0x590-0x59c(r)BCD number
0x5b0         DMA bit flag (bit 1)
0x600-0x6ff   Includes standard screen control words
0x700-0x7ff   Includes standard Seibu sound system


***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "audio/seibu.h"
#include "sound/3812intf.h"
#include "cpu/m68000/m68000.h"

extern WRITE16_HANDLER( legionna_background_w );
extern WRITE16_HANDLER( legionna_foreground_w );
extern WRITE16_HANDLER( legionna_midground_w );
extern WRITE16_HANDLER( legionna_text_w );
extern WRITE16_HANDLER( legionna_control_w );

extern VIDEO_START( legionna );
extern VIDEO_START( cupsoc );
extern VIDEO_START( denjinmk );
extern VIDEO_UPDATE( legionna );
extern VIDEO_UPDATE( godzilla );
extern VIDEO_UPDATE( sdgndmrb );
void heatbrl_setgfxbank(UINT16 data);
void denjinmk_setgfxbank(UINT16 data);

extern UINT16 *legionna_back_data,*legionna_fore_data,*legionna_mid_data,*legionna_scrollram16,*legionna_textram;
static UINT16 *mcu_ram;
extern UINT8 sdgndmrb_pri_n;

static WRITE16_HANDLER( legionna_paletteram16_w )	/* xBBBBxRRRRxGGGGx */
{
	int a;
	COMBINE_DATA(&paletteram16[offset]);
	a = paletteram16[offset];
	palette_set_color_rgb(Machine,offset,pal4bit(a >> 1),pal4bit(a >> 6),pal4bit(a >> 11));
}


/* Mcu reads in attract in Legionnaire game demo

Guess the 0x400-0x5ff area of the COP is protection related.

CPU0 PC 0032a2 unknown MCU write offset: 0260 data: 9c6c
CPU0 PC 0032a8 unknown MCU write offset: 0250 data: 0010
CPU0 PC 0032c8 unknown MCU write offset: 0261 data: 987c
CPU0 PC 0032ce unknown MCU write offset: 0251 data: 0010
CPU0 PC 003546 unknown MCU write offset: 0262 data: 02c4
CPU0 PC 00354a unknown MCU write offset: 0252 data: 0004
CPU0 PC 00355c unknown MCU write offset: 0263 data: 0000
CPU0 PC 003560 unknown MCU write offset: 0253 data: 0004
CPU0 PC 003568 unknown MCU write offset: 0280 data: a180
CPU0 PC 00356e unknown MCU write offset: 0280 data: a980
CPU0 PC 003574 unknown MCU write offset: 0280 data: b100
CPU0 PC 00357a unknown MCU write offset: 0280 data: b900
CPU0 PC 003580 unknown MCU read offset: 02c4
CPU0 PC 003588 unknown MCU read offset: 02c2
CPU0 PC 003594 unknown MCU read offset: 02c1
CPU0 PC 0035a0 unknown MCU read offset: 02c3
CPU0 PC 0032a2 unknown MCU write offset: 0260 data: 9c6c
CPU0 PC 0032a8 unknown MCU write offset: 0250 data: 0010
CPU0 PC 0032c8 unknown MCU write offset: 0261 data: 987c
CPU0 PC 0032ce unknown MCU write offset: 0251 data: 0010
CPU0 PC 003422 unknown MCU write offset: 0280 data: 138e
CPU0 PC 003428 unknown MCU read offset: 02da
CPU0 PC 00342e unknown MCU read offset: 02d8
CPU0 PC 00346c unknown MCU write offset: 0280 data: 3bb0
CPU0 PC 0032a2 unknown MCU write offset: 0260 data: 987c
CPU0 PC 0032a8 unknown MCU write offset: 0250 data: 0010
CPU0 PC 003306 unknown MCU write offset: 0280 data: 8100
CPU0 PC 00330c unknown MCU write offset: 0280 data: 8900


Mcu reads in attract in Heated Barrel game demo (note
partial similarity)

(i) This sequence repeats a number of times early on:

CPU0 PC 0085b4 unknown MCU write offset: 0210 data: 0064
CPU0 PC 0085ba unknown MCU write offset: 0211 data: 0000
CPU0 PC 0085be unknown MCU read offset: 02ca
CPU0 PC 0085ee unknown MCU read offset: 02c9
CPU0 PC 008622 unknown MCU read offset: 02c8
[Protection BCD,see protection_bcd_jsr()]

(ii) This happens a few times:

CPU0 PC 0017ac unknown MCU write offset: 0260 data: b6cc
CPU0 PC 0017b2 unknown MCU write offset: 0250 data: 0010
CPU0 PC 0017d2 unknown MCU write offset: 0261 data: babc
CPU0 PC 0017d8 unknown MCU write offset: 0251 data: 0010
CPU0 PC 00192c unknown MCU write offset: 0280 data: 138e
CPU0 PC 001932 unknown MCU read offset: 02da
CPU0 PC 001938 unknown MCU read offset: 02d8
CPU0 PC 001976 unknown MCU write offset: 0280 data: 3bb0
CPU0 PC 0017ac unknown MCU write offset: 0260 data: b6cc
CPU0 PC 0017b2 unknown MCU write offset: 0250 data: 0010
CPU0 PC 0017d2 unknown MCU write offset: 0261 data: bb9c
CPU0 PC 0017d8 unknown MCU write offset: 0251 data: 0010
CPU0 PC 00192c unknown MCU write offset: 0280 data: 138e
CPU0 PC 001932 unknown MCU read offset: 02da
CPU0 PC 001938 unknown MCU read offset: 02d8
CPU0 PC 001976 unknown MCU write offset: 0280 data: 3bb0



(iii) Later on this happens a lot:

CPU0 PC 0017ac unknown MCU write offset: 0260 data: c61c
CPU0 PC 0017b2 unknown MCU write offset: 0250 data: 0010
CPU0 PC 0017d2 unknown MCU write offset: 0261 data: bb9c
CPU0 PC 0017d8 unknown MCU write offset: 0251 data: 0010
CPU0 PC 001a5c unknown MCU write offset: 0262 data: aa48
CPU0 PC 001a62 unknown MCU write offset: 0252 data: 0003
CPU0 PC 001a7c unknown MCU write offset: 0263 data: a0c8
CPU0 PC 001a82 unknown MCU write offset: 0253 data: 0003
CPU0 PC 001a86 unknown MCU write offset: 0280 data: a100
CPU0 PC 001a8c unknown MCU write offset: 0280 data: b080
CPU0 PC 001a92 unknown MCU write offset: 0280 data: a900
CPU0 PC 001a98 unknown MCU write offset: 0280 data: b880
CPU0 PC 001a9e unknown MCU read offset: 02c0
CPU0 PC 001aa6 unknown MCU read offset: 02c2
CPU0 PC 001ab2 unknown MCU read offset: 02c1

write to $500 these values to determine the kind of sub-routine to do
hit_check
a180
a980
b100
b900
|| b880 (heatbrl)

movement protection
8100
8900

00DB2A: D6A8 0048                add.l   ($48,A0), D3
00DB2E: D8A8 0044                add.l   ($44,A0), D4
00DB32: 0C83 FFF0 0000           cmpi.l  #-$100000, D3
00DB38: 6D2E                     blt     $db68
00DB3A: 0C83 0100 0000           cmpi.l  #$1000000, D3
00DB40: 6C26                     bge     $db68
00DB42: 2143 003C                move.l  D3, ($3c,A0)
00DB46: 0C84 FFF0 0000           cmpi.l  #-$100000, D4
00DB4C: 6D1A                     blt     $db68
00DB4E: 0C84 0100 0000           cmpi.l  #$1000000, D4
00DB54: 6E12                     bgt     $db68
00DB56: 2144 0038                move.l  D4, ($38,A0)
00DB5A: 3168 003C 0022           move.w  ($3c,A0), ($22,A0)
00DB60: 3168 0038 0024           move.w  ($38,A0), ($24,A0)
00DB66: 4E75                     rts


sprite DMA

*/
/*Movement protection*//*Legionnaire,Heated Barrel*/
static UINT32 ram_addr[2],rom_addr[2];
/*Sprite DMA protection*//*SD Gundam*/
static UINT8 dma_status;
static UINT32 dma_src,dma_dst;
static UINT16 prot_data[2],dma_size;
/*Number protection*//*Heated Barrel,SD Gundam,Godzilla,Denjin Makai*/
static UINT32 prot_bcd[4];
/*Hit check protection*//*Legionnaire,Heated Barrel,SD Gundam*/
static UINT8 xy_check;
static UINT32 hit_check_x,hit_check_y;



#define CRT_MODE(_x_,_y_,_flip_) \
	{ \
	screen_state *state = &Machine->screen[0]; \
	rectangle visarea = state->visarea; \
	visarea.min_x = 0; \
	visarea.max_x = _x_-1; \
	visarea.min_y = 0; \
	visarea.max_y = _y_-1; \
	video_screen_configure(0, _x_, _y_, &visarea, state->refresh ); \
	flip_screen_set(_flip_); \
	} \

/*TODO: numbers over 65535?*/
static UINT32 protection_bcd_jsr(UINT16 prot_data)
{
	UINT32 res,bcd_data;

	res = 0;
	bcd_data = prot_data;
	while(bcd_data > 0)
	{
		if(bcd_data > 999)
		{
			res+=0x01000000;
			bcd_data-=1000;
		}
		else if(bcd_data > 99)
		{
			res+=0x00010000;
			bcd_data-=100;
		}
		else if(bcd_data > 9)
		{
			res+=0x00000100;
			bcd_data-=10;
		}
		else
		{
			res++;
			bcd_data--;
		}
	}

	return res & 0x0f0f0f0f;
}

/*TODO: -move x-axis limits,to calculate basing on the screen xy-axis values*/
/*      -the second value should be end of calculation (in other words,check everything between the two values) */
#define PLAYER 0
#define ENEMY 1
static void protection_move_jsr(UINT32 work_ram,UINT8 k)
{
	static UINT32 move_data,x_data,y_data;
	/*Read the movement data to execute*/
	move_data = ((program_read_word(work_ram+0x34)<<16) & 0xffff0000) |
	             (program_read_word(work_ram+0x36) & 0xffff);

	/*Read the x/y axis of the sprite to change*/
	x_data = (program_read_word(work_ram+0x8));
	y_data = (program_read_word(work_ram+0x4));
	/*it's bit sensitive AFAIK*/
	/*move_data hi-word on player
      $17 = walk floor
      $1b = jump
      $30 = ?
    */
	/*Check the kind of movement that we need to execute*/
	switch(k & 1)
	{
		case PLAYER:
			switch(move_data & 0xffff)
			{
				case 0x0000: x_data++; break; //right
				case 0x0040: y_data++; break; //down
				case 0x00c0: y_data--; break; //up
				case 0x0080: x_data--; break; //left
				case 0x00a0: y_data--; x_data--; break; //up-left
				case 0x0060: y_data++; x_data--; break; //down-left
				case 0x0020: y_data++; x_data++; break; //down-right
				case 0x00e0: y_data--; x_data++; break; //up-right
			}
			break;
		/*wrong...*/
		case ENEMY:
			switch(move_data & 0xffff)
			{
				case 0x0000: x_data++; break; //right
				case 0x0040: y_data++; break; //down
				case 0x00c0: y_data--; break; //up
				case 0x0080: x_data--; break; //left
				case 0x00a0: y_data--; x_data--; break; //up-left
				case 0x0060: y_data++; x_data--; break; //down-left
				case 0x0020: y_data++; x_data++; break; //down-right
				case 0x00e0: y_data--; x_data++; break; //up-right
				default:
					popmessage("%08x",move_data);
				break;
			}
			break;
	}
	/*Write the new values to the sprite x/y data*/
	program_write_word(work_ram+0x8,x_data);
	program_write_word(work_ram+0x4,y_data);
}

#ifdef UNUSED_FUNCTION
static void protection_move2_jsr(void)
{
//  static UINT32 move_data;
//  popmessage("%08x %08x %08x %08x",ram_addr[0],ram_addr[1],rom_addr[0],rom_addr[1]);
}
#endif

static UINT16 hit_check;

static void protection_hit_jsr(UINT32 work_ram1,UINT32 work_ram2)
{
	int x1,y1,x2,y2/*,hit1,hit2*/;
	x1 = (program_read_word(work_ram1+0x8));
	y1 = (program_read_word(work_ram1+0x4));
	//hit1 = (program_read_word(work_ram1-));//this sprite is attacking
	x2 = (program_read_word(work_ram2+0x8));
	y2 = (program_read_word(work_ram2+0x4));
	//hit2 = (program_read_word());

	popmessage("%08x:x=%04x y=%04x %08x:x=%04x y=%04x",work_ram1,x1,y1,work_ram2,x2,y2);

	if((x1 >= (x2-0x80)) &&
	   (x1 <= (x2+0x80)) &&
	   (y1 >= (y2-3))  &&
	   (y1 <= (y2+3)))
		hit_check = 0;
	else
		hit_check = 0xffff;
}

/*directional movement protection*/
static void moveprot_jsr(void)
{
	static INT16 x_axis,y_axis;
	static UINT16 move_data,distance,move_type;
	move_data = program_read_word(ram_addr[0]+0x36);
	x_axis = program_read_word(ram_addr[0]+0x08);
	y_axis = program_read_word(ram_addr[0]+0x04);

	distance = (move_data & 0xf);
	move_type = (move_data & 0xf0)>>4;
	switch(move_type)
	{
		case 0x0f://right
			program_write_word(ram_addr[0]+0x08,x_axis+distance);
			//program_write_word(0x110004,);
			break;
		case 0x0b://up
			program_write_word(ram_addr[0]+0x04,y_axis-distance);
			break;
		case 0x07://left
			program_write_word(ram_addr[0]+0x08,x_axis-distance);
			break;
		case 0x03://down
			program_write_word(ram_addr[0]+0x04,y_axis+distance);
			break;
		case 0x0d://up-right
			program_write_word(ram_addr[0]+0x08,x_axis+distance);
			program_write_word(ram_addr[0]+0x04,y_axis-distance);
			break;
		case 0x09://up-left
			program_write_word(ram_addr[0]+0x04,y_axis-distance);
			program_write_word(ram_addr[0]+0x08,x_axis-distance);
			break;
		case 0x01://down-right
			program_write_word(ram_addr[0]+0x04,y_axis+distance);
			program_write_word(ram_addr[0]+0x08,x_axis+distance);
			break;
		case 0x05://down-left
			program_write_word(ram_addr[0]+0x04,y_axis+distance);
			program_write_word(ram_addr[0]+0x08,x_axis-distance);
			break;
		default:
			logerror("Warning: \"0x205\" command called with move_type parameter = %02x\n",move_type);

			//down-right
			//down-left
	}
	//program_write_word(0x110008,x_axis+tmp);
	//program_write_word(0x110004,y_axis+tmp);

	//program_write_word(0x110008,x_axis);
	//program_write_word(0x110004,y_axis);
}

/*
00454E: 3028 0008                move.w  ($8,A0), D0 ;player
004552: 3228 000C                move.w  ($c,A0), D1
004556: 342B 0008                move.w  ($8,A3), D2 ;enemy
00455A: 362B 000C                move.w  ($c,A3), D3
00455E: 33C0 0011 0048           move.w  D0, $110048.l ;player x
004564: 33C1 0011 0044           move.w  D1, $110044.l ;player y
00456A: 33C2 0011 0008           move.w  D2, $110008.l ;enemy x
004570: 33C3 0011 0004           move.w  D3, $110004.l ;enemy y

*/
/*sprite "look" protection*/
static void move2prot_jsr(void)
{
	static INT16 x_pl,y_pl,x_en,y_en,res;
	x_pl = program_read_word(ram_addr[1]+0x8);
	y_pl = program_read_word(ram_addr[1]+0x4);
	x_en = program_read_word(ram_addr[0]+0x8);
	y_en = program_read_word(ram_addr[0]+0x4);

	res = 0;
	if(x_en > x_pl)
		res|=0x80;

	if((x_en > x_pl-0x20) && (x_en < x_pl+0x20))
		res|=0x40;
//...
	//if(y_en > y_pl)
	//  res|=0x40;

	program_write_word(ram_addr[0]+0x36,res);
}

#ifdef UNUSED_FUNCTION
/*"To point" movement protection*/
static void move3x_prot_jsr(void)
{
	static INT16 x_pl,x_en,x_dis;
	x_pl = program_read_word(ram_addr[1]+0x8);
	x_en = program_read_word(ram_addr[0]+0x8);
	x_dis = ((program_read_word(ram_addr[0]+0x34) & 0xf0) >> 4);

	if(x_en > x_pl)
		x_dis^=0xffff;

	program_write_word(ram_addr[0]+0x36,-0x40);/*enable command*/
	program_write_word(ram_addr[0]+0x14,x_dis);
}

static void move3y_prot_jsr(void)
{
	static INT16 y_pl,y_en,y_dis;
	y_pl = program_read_word(ram_addr[1]+0x4);
	y_en = program_read_word(ram_addr[0]+0x4);
	y_dis = (program_read_word(ram_addr[0]+0x34) & 0xf);

	if(y_en > y_pl)
		y_dis^=0xffff;

	program_write_word(ram_addr[0]+0x36,-0x80);/*enable command*/
	program_write_word(ram_addr[0]+0x10,y_dis);
}
#endif


static READ16_HANDLER( mcu_r )
{
	switch (offset)
	{
		/* Protection is not understood */

		case (0x470/2):	/* read PC $110a, could be some sort of control word:
                sometimes a bit is changed then it's poked back in... */
			return (mame_rand(Machine) &0xffff);

		case (0x582/2):	/* read PC $3594 */
			return (0);

		case (0x584/2):	/* read PC $3588 */
			return (0);

		case (0x586/2):	/* read PC $35a0 */
			return (0);

		case (0x588/2):	/* read PC $3580 */
			return hit_check;

		case (0x5b0/2):	/* bit 15 is branched on a few times in the $3300 area */
			return (0);

		case (0x5b4/2):	/* read and stored in ram before +0x5b0 bit 15 tested */
			return (0);

		/* Non-protection reads */

		case (0x708/2):	/* seibu sound: these three around $b10 on */
			return seibu_main_word_r(2,0);

		case (0x70c/2):
			return seibu_main_word_r(3,0);

		case (0x714/2):
			return seibu_main_word_r(5,0);

		/* Inputs */

		case (0x740/2):	/* code at $b00 sticks waiting for bit 6 hi */
			return input_port_1_word_r(0,0);

		case (0x744/2):
			return input_port_2_word_r(0,0);

		case (0x748/2):	/* code at $f4a reads this 4 times in _weird_ fashion */
			return input_port_0_word_r(0,0);

		case (0x74c/2):
			return input_port_3_word_r(0,0);

	}
//logerror("CPU0 PC %06x unknown MCU read offset: %04x\n",activecpu_get_previouspc(),offset);

	return mcu_ram[offset];
}

static WRITE16_HANDLER( mcu_w )
{
	COMBINE_DATA(&mcu_ram[offset]);

	switch (offset)
	{

		case (0x4c0/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			ram_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a0/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			ram_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4c2/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			ram_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a2/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			ram_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4c4/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			rom_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a4/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			rom_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4c6/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			rom_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a6/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			rom_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x500/2):
		{
			/*Movement protection*/
			if(mcu_ram[0x500/2] == 0x8900 || mcu_ram[0x500/2] == 0x0205)
			{
				static UINT16 xy_data[2];
				static UINT8 k;
				xy_data[0] = program_read_word(rom_addr[0]);
				xy_data[1] = program_read_word(rom_addr[1]);
				k = (mcu_ram[0x500/2] == 0x0205) ? ENEMY : PLAYER;
				protection_move_jsr(ram_addr[0],k);
				//protection_move_jsr(ram_addr[1]); //???
				//popmessage("%08x %08x %04x %04x",ram_addr[0],ram_addr[1],xy_data[0],xy_data[1]);
			}
			else if(mcu_ram[0x500/2] == 0x3bb0 || mcu_ram[0x500/2] == 0x138e)
			{
				protection_hit_jsr(ram_addr[0],ram_addr[1]);
			}
			break;
		}

		// 61a bit 0 is flipscreen
		// 61c probably layer disables, like Dcon
		// 620 - 62a scroll control;  is there a layer priority switch...?

		case (0x620/2):
		{
			legionna_scrollram16[0] = mcu_ram[offset];
			break;
		}
		case (0x622/2):
		{
			legionna_scrollram16[1] = mcu_ram[offset];
			break;
		}
		case (0x624/2):
		{
			legionna_scrollram16[2] = mcu_ram[offset];
			break;
		}
		case (0x626/2):
		{
			legionna_scrollram16[3] = mcu_ram[offset];
			break;
		}
		case (0x628/2):
		{
			legionna_scrollram16[4] = mcu_ram[offset];
			break;
		}
		case (0x62a/2):
		{
			legionna_scrollram16[5] = mcu_ram[offset];
			break;
		}
		case (0x700/2):	/* seibu(0) */
		{
			seibu_main_word_w(0,mcu_ram[offset],0xff00);
			break;
		}
		case (0x704/2):	/* seibu(1) */
		{
			seibu_main_word_w(1,mcu_ram[offset],0xff00);
			break;
		}
		case (0x710/2):	/* seibu(4) */
		{
			seibu_main_word_w(4,mcu_ram[offset],0xff00);
			break;
		}
		case (0x718/2):	/* seibu(6) */
		{
			seibu_main_word_w(6,mcu_ram[offset],0xff00);
			break;
		}
//      default:
//logerror("CPU0 PC %06x unknown MCU write offset: %04x data: %04x\n",activecpu_get_previouspc(),offset,data);
	}
}



/*
player-1 priorities list:
1086d8: show this sprite (bit 15)
1086dc: lives (BCD,bits 3,2,1,0)
1086de: energy bar (upper byte)
1086e0: walk animation (lower byte)
1086ec: "death" status (bit 15)
1086f4: sprite y axis
1086f0: sprite x axis

Sprite DMA TODO:
-sprite priorities,likely to be a protection issue because in-game sprites MUST
 be behind the foreground layer but the attract mode logos (the ones used when
 the story is explained) should be above it.They both use 0 as priority number.
-sprites at the very left border disappears.
-some bullets STILL remains on screen?
-3rd mid-boss disappears (0x1f0),2nd boss lasers wrong positioning

spriteram DMA [1]
001DE4: 3086                     move.w  D6, (A0) ;$100400,color + other stuff
001DE6: 2440                     movea.l D0, A2
001DE8: 0269 0004 0002           andi.w  #$4, ($2,A1)
001DEE: 3152 000C                move.w  (A2), ($c,A0) ;DMA size
001DF2: 3145 0002                move.w  D5, ($2,A0)
001DF6: 0245 0040                andi.w  #$40, D5
001DFA: 2009                     move.l  A1, D0
001DFC: 3140 00C0                move.w  D0, ($c0,A0) ;RAM -> $1004c0 (work ram index?)
001E00: 4840                     swap    D0
001E02: 3140 00A0                move.w  D0, ($a0,A0) ;RAM -> $1004a0
001E06: 200A                     move.l  A2, D0
001E08: 3140 0014                move.w  D0, ($14,A0) ;$ROM lo -> $100414 src
001E0C: 4840                     swap    D0
001E0E: 3140 0012                move.w  D0, ($12,A0) ;$ROM hi -> $100412
001E12: 2679 0010 8116           movea.l $108116.l, A3 ;points to dst spriteram
001E18: 3839 0010 810A           move.w  $10810a.l, D4 ;spriteram index
001E1E: 260B                     move.l  A3, D3
001E20: 3143 00C8                move.w  D3, ($c8,A0) ;sets the dst spriteram
001E24: 4843                     swap    D3
001E26: 3143 00A8                move.w  D3, ($a8,A0)
001E2A: 45EA 0004                lea     ($4,A2), A2
//at this point we're ready for DMAing
001E2E: 317C A180 0100           move.w  #$a180, ($100,A0) ;<-DMA parameters!?
001E34: 317C 6980 0102           move.w  #$6980, ($102,A0) ;<-""
001E3A: 317C C480 0102           move.w  #$c480, ($102,A0) ;<-""
001E40: 317C 0000 0010           move.w  #$0, ($10,A0)     ;<-""
001E46: 302A 0002                move.w  ($2,A2), D0
001E4A: 816B 0006                or.w    D0, ($6,A3)
001E4E: 45EA 0006                lea     ($6,A2), A2
001E52: 302B 0008                move.w  ($8,A3), D0
001E56: B079 0010 8112           cmp.w   $108112.l, D0
001E5C: 6E00 0054                bgt     $1eb2
001E60: B079 0010 8110           cmp.w   $108110.l, D0
001E66: 6D00 004A                blt     $1eb2
001E6A: 026B 7FFF 000A           andi.w  #$7fff, ($a,A3)
001E70: 8B6B 0004                or.w    D5, ($4,A3)
001E74: 47EB 0008                lea     ($8,A3), A3
001E78: 260B                     move.l  A3, D3
001E7A: 3143 00C8                move.w  D3, ($c8,A0)
001E7E: 4843                     swap    D3
001E80: 3143 00A8                move.w  D3, ($a8,A0)
001E84: 5244                     addq.w  #1, D4
001E86: B879 0010 8114           cmp.w   $108114.l, D4
001E8C: 6500 000C                bcs     $1e9a
001E90: 0069 0002 0002           ori.w   #$2, ($2,A1)
001E96: 6000 000C                bra     $1ea4
//Note: I believe the above program is there just for protection copy
001E9A: 3028 01B0                move.w  ($1b0,A0), D0 ;bit 1 = DMA job finished
001E9E: 0240 0002                andi.w  #$2, D0
001EA2: 6790                     beq     $1e34
001EA4: 33C4 0010 810A           move.w  D4, $10810a.l
001EAA: 23CB 0010 8116           move.l  A3, $108116.l
001EB0: 4E75                     rts

x/y check [2]
002030: E58D                     lsl.l   #2, D5
002032: 0685 0003 0000           addi.l  #$30000, D5
002038: 33C5 0010 04C4           move.w  D5, $1004c4.l
00203E: 4845                     swap    D5
002040: 33C5 0010 04A4           move.w  D5, $1004a4.l
002046: E58E                     lsl.l   #2, D6
002048: 0686 0003 0000           addi.l  #$30000, D6
00204E: 33C6 0010 04C6           move.w  D6, $1004c6.l
002054: 4846                     swap    D6
002056: 33C6 0010 04A6           move.w  D6, $1004a6.l
00205C: 33FC A180 0010 0500      move.w  #$a180, $100500.l
002064: 33FC B100 0010 0500      move.w  #$b100, $100500.l
00206C: 33FC A980 0010 0500      move.w  #$a980, $100500.l
002074: 33FC B900 0010 0500      move.w  #$b900, $100500.l
00207C: 4E75                     rts
[...]
//then reads at $580

*/

static UINT16 s_i;

static void dma_transfer(void)
{
	static UINT16 rel_xy;
	static UINT16 abs_x,abs_y;
	static UINT16 param;

	//for(s_i = dma_size;s_i > 0;s_i--)
	{
		/*Sprite Color*/
		param = program_read_word(0x100400) & 0x3f;
		/*Write the entire parameters [offs+0]*/
		program_write_word(dma_dst,program_read_word(dma_src) + param);
		/*Sprite Priority (guess)*/
		//param = ((program_read_word(0x100400) & 0x40) ? 0x4000 : 0);
		/*Write the sprite number [offs+1]*/
		program_write_word(dma_dst+2,program_read_word(dma_src+2));
		/*Sprite Relative x/y coords*/
		rel_xy = program_read_word(dma_src+4); /*???*/
		/*temporary hardwired,it should point to 0x4c0/0x4a0*/
		abs_x = (program_read_word(0x110008) - program_read_word(0x10048e));
		abs_y = (program_read_word(0x110004) - program_read_word(0x10048c));
		program_write_word(dma_dst+4,((rel_xy & 0x7f) + (abs_x) - ((rel_xy & 0x80) ? 0x80 : 0)) & 0x1ff);
		program_write_word(dma_dst+6,(((rel_xy & 0x7f00) >> 8) + (abs_y) + (0x10) - ((rel_xy & 0x8000) ? 0x80 : 0)) & 0x1ff);
		dma_dst+=8;
		dma_src+=6;
	}
}


/*
    switch(program_read_word(hit_check_x))
    {
        case 0xb4: xparam = 0x0c/2; break;
        case 0xb8: xparam = 0x10/2; break;
        case 0xbc: xparam = 0x14/2; break;
        case 0xc0: xparam = 0x18/2; break;
        case 0xc4: xparam = 0x1c/2; break;
        case 0xd4: xparam = 0x20/2; break;
        ...
        case 0xb0: xparam = 0x08/2;
        case 0xac: xparam = 0x04/2;
    }
*/
static UINT16 check_calc(UINT16 param)
{
	UINT16 num,i;

	i = param;
	i-=0xac;
	i/=4;
	num = (0x4/2);
	for(;i>0;i--)
		num+=(0x4/2);

	return num;
}

static UINT16 hit_check_jsr(void)
{
	static INT16 xsrc,xdst,ysrc,ydst,xparam,yparam;
	xsrc = (program_read_word(0x110008));
	ysrc = (program_read_word(0x110004));
	xdst = (program_read_word(0x110048));
	ydst = (program_read_word(0x110044));

	/*Here we check the destination sprite width*/
	/*0x4a4/0x4c4*/
	xparam = check_calc(program_read_word(hit_check_x));
	/*Here we check the destination sprite height*/
	/*0x4a6/0x4c6*/
	yparam = check_calc(program_read_word(hit_check_y));

	if(!xparam || !yparam)
		popmessage("SRC:%04x %04x DST:%04x %04x V:%08x %08x",xsrc,ysrc,xdst,ydst,program_read_word(hit_check_x),program_read_word(hit_check_y));
	if(xdst >= (xsrc-xparam) && ydst >= (ysrc-yparam) &&
	   xdst <= (xsrc+xparam) && ydst <= (ysrc+yparam))
		return 0;//sprites collide
	else
		return 3;//sprites do not collide
}

#define UP			0xc0
#define UP_RIGHT    0xe0
#define RIGHT       0x00
#define DOWN_RIGHT	0x20
#define DOWN		0x40
#define DOWN_LEFT	0x60
#define LEFT		0x80
#define UP_LEFT		0xa0

/*Heated Barrel*/
/*command 0x8100 will check for the direction of the sprite*/
/*command 0x8900 will check the "point" movement*/
static void cop2_move3_prot(void)
{
	static INT16 x_pl,x_en;
	static INT16 y_pl,y_en;
	static INT16 x_dis,y_dis;
	static INT16 dir,dis;
	x_pl = program_read_word(ram_addr[1]+0x8);
	x_en = program_read_word(ram_addr[0]+0x8);
	dis = ((program_read_word(ram_addr[0]+0x34) & 0xf0) >> 4);
	y_pl = program_read_word(ram_addr[1]+0x4);
	y_en = program_read_word(ram_addr[0]+0x4);

	/*
    xxxx ---- select the direction of the enemy sprite

                        0xc0 up
          up-left   0xa0  |  0xe0 up-right
         left    0x80   <-o->  0x00 right
        down-left   0x60  |  0x20 down-right
                        0x40 down
    */

	if(x_en >= x_pl)
	{
		if((y_en >= (y_pl-0x10)) && (y_en <= (y_pl+0x10)))
			dir = LEFT;
		else if(y_en < (y_pl-0x10))
			dir = DOWN_LEFT;
		else if(y_en > (y_pl+0x10))
			dir = UP_LEFT;
	}
	else if(x_en < x_pl)
	{
		if((y_en >= (y_pl-0x10)) && (y_en <= (y_pl+0x10)))
			dir = RIGHT;
		else if(y_en < (y_pl-0x10))
			dir = DOWN_RIGHT;
		else if(y_en > (y_pl+0x10))
			dir = UP_RIGHT;
	}
	/*UP DOWN cases*/
	if((x_en >= (x_pl-0x10)) && (x_en <= (x_pl+0x10)))
	{
		if(y_en >= y_pl)
			dir = UP;
		else if(y_en < y_pl)
			dir = DOWN;
	}

	program_write_word(ram_addr[0]+0x36,dir);

	/*TODO*/
	x_dis = (x_pl-x_en);
	y_dis = (y_pl-y_en);

	if(x_dis > 4)
		x_dis = 4;

	if(x_dis < -4)
		x_dis = -4;

	if(y_dis > 4)
		y_dis = 4;

	if(y_dis < -4)
		y_dis = -4;

	//if(y_en > y_pl)
	//  y_dis^=0xffff;

	//if(x_en > x_pl)
	//  x_dis^=0xffff;

	program_write_word(ram_addr[0]+0x10,y_dis);
	program_write_word(ram_addr[0]+0x14,x_dis);
}

/**/
static UINT16 cop2_hit_prot(void)
{
	static INT16 xsrc,xdst;
	static INT16 ysrc,ydst;
	static INT16 xp,yp;
	static INT16 param1,param2;
	static INT16 val;

	param1 = program_read_word(rom_addr[0]);
	param2 = program_read_word(rom_addr[1]);

	xsrc = program_read_word(ram_addr[0]+0x8) + program_read_word(ram_addr[0]+0x14);
	ysrc = program_read_word(ram_addr[0]+0x4) + program_read_word(ram_addr[0]+0x10);
	xdst = program_read_word(ram_addr[1]+0x8) + program_read_word(ram_addr[1]+0x14);
	ydst = program_read_word(ram_addr[1]+0x4) + program_read_word(ram_addr[1]+0x10);

//  xp = (param1 & 0x00f0) >> 4;
//  yp = (param1 & 0x0f00) >> 8;

//  popmessage("%04x %04x",param1,param2);

	xp = 0;
	yp = 0;
	for(val = ((param1 & 0xff0) >> 4); val > 0; val-=5)
		xp++;

	for(val = ((param1 & 0xff0) >> 4); val > 0; val-=3)
		yp++;

	/*TODO*/
//  xp+=4;
//  yp+=4;

	if(xsrc >= xdst && xsrc <= xdst+xp && ysrc >= xdst && ysrc <= ydst+yp)
		return 0;
	else
		return 3;
}

static void cop2_move2_prot(void)
{
	static INT16 xsrc,ysrc;
	static INT16 param2;

	xsrc = program_read_word(ram_addr[0]+0x14);
	ysrc = program_read_word(ram_addr[0]+0x10);
	param2 = program_read_word(rom_addr[1]);

	switch(param2)
	{
		case 0x10:	xsrc++; break; //right
		case 0x30:	xsrc--; break; //left
		case 0x40:  ysrc--; break; //up
		case 0x60: 	ysrc++; break; //down
		case 0x08:  ysrc--; xsrc++; break; //up-right
		case 0x38:	ysrc--; xsrc--; break; //up-left
		case 0x28: 	ysrc++; xsrc--; break; //down-left
		case 0x18:  ysrc++; xsrc++; break; //down-right
	}

	program_write_word(ram_addr[0]+0x14,xsrc);
	program_write_word(ram_addr[0]+0x10,ysrc);
}


static READ16_HANDLER( cop2_mcu_r )
{
	switch (offset)
	{
		/* Protection is not understood */

		/*hit protection*/
		case (0x580/2):
		{
			return xy_check;
		}
		case (0x582/2):
		{
			if(input_code_pressed(KEYCODE_X))
				return 0;//xy_check;
			else
				return 3;
		}
		case (0x584/2):
		{
			/*---- ---- ---- --xx used bits*/
			//if(!xy_check)
			//{
			//  xy_check = 3;
			//  return 0;
			//}
			//else return 3;
			if(input_code_pressed(KEYCODE_C))
				return 0;//xy_check;
			else
				return 3;
		}

		/*number protection*/
		case (0x590/2):
		{
			/*BCD read*/
			return (prot_bcd[0] & 0xffff) + 0x3030;
		}
		case (0x592/2):
		{
			return ((prot_bcd[0] & 0xffff0000) >> 16) + 0x3030;
		}
		case (0x594/2):
		{
			return (prot_bcd[1] & 0xffff) + 0x3030;
		}
		case (0x596/2):
		{
			return ((prot_bcd[1] & 0xffff0000) >> 16) + 0x3030;
		}
		case (0x598/2):
		{
			return (prot_bcd[2] & 0xffff) + 0x3030;
		}
		case (0x59a/2):
		{
			return ((prot_bcd[2] & 0xffff0000) >> 16) + 0x3030;
		}
		case (0x59c/2):
		{
			return 0x3030;
		}

		case (0x5b0/2):	/* bit 15 is branched on a few times in the $1938 area */
			return (mcu_ram[offset]);

		case (0x5b4/2):	/* read at $1932 and stored in ram before +0x5b0 bit 15 tested */
			return (0);

		/* Non-protection reads */

		case (0x7c8/2):	/* seibu sound */
			return seibu_main_word_r(2,0);

		case (0x7cc/2):
			return seibu_main_word_r(3,0);

		case (0x7d4/2):
			return seibu_main_word_r(5,0);

		/* Inputs */

		case (0x740/2):
			return input_port_1_word_r(0,0);

		case (0x744/2):
			return input_port_2_word_r(0,0);

		case (0x748/2):
			return input_port_4_word_r(0,0);

		case (0x74c/2):
			return input_port_3_word_r(0,0);

	}
//logerror("CPU0 PC %06x unknown MCU read offset: %04x\n",activecpu_get_previouspc(),offset);

	return mcu_ram[offset];
}

static WRITE16_HANDLER( cop2_mcu_w )
{
	COMBINE_DATA(&mcu_ram[offset]);

	switch (offset)
	{
		case (0x470/2):
		{
			heatbrl_setgfxbank( mcu_ram[offset] );
			break;
		}

		/*"Number protection" sub-routine*/
		case (0x420/2):
		{
			//coin counter write
			//popmessage("%04x",mcu_ram[offset]);
			prot_bcd[0] = protection_bcd_jsr(mcu_ram[offset]);
			//prot_bcd = mcu_ram[offset] - 0x22;
			break;
		}
		case (0x422/2):
		{
			prot_bcd[1] = protection_bcd_jsr(mcu_ram[offset]);
			break;
		}

		case (0x478/2):
		{
			static UINT16 i;
			/*
            AM_RANGE(0x100800, 0x100fff) AM_WRITE(legionna_background_w) AM_BASE(&legionna_back_data)
            AM_RANGE(0x101000, 0x1017ff) AM_WRITE(legionna_foreground_w) AM_BASE(&legionna_fore_data)
            AM_RANGE(0x101800, 0x101fff) AM_WRITE(legionna_midground_w) AM_BASE(&legionna_mid_data)
            AM_RANGE(0x102000, 0x102fff) AM_WRITE(legionna_text_w) AM_BASE(&legionna_textram)
            */
			switch(mcu_ram[offset])
			{
				/*layer clearance*/
				case 0x4080:
				/*text layer*/
				for(i=0;i<0x1000;i+=2)
					program_write_word(i+0x102000,0x0000);
				/*background layer*/
				for(i=0;i<0x800;i+=2)
					program_write_word(i+0x100800,0x0000);
				break;
				case 0x4100: break;
				case 0x41c0: break;
				//default: popmessage("%04x",mcu_ram[offset]);
			}
			break;
		}

		/*sprite ram clear(Guess)*/
		case (0x47e/2):
		{
			static UINT16 i;
			if(mcu_ram[0x47e/2] == 0x118)
			{
				for(i=0;i<0x800;i+=2)
					program_write_word((0x103000 | i),0x0000);
			}
			break;
		}

		// 65a bit 0 is flipscreen
		// 65c probably layer disables, like Dcon? Used on screen when you press P1-4 start (values 13, 11, 0 seen)
		// 660 - 66a scroll control;  is there a layer priority switch...?
		case (0x4c0/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			ram_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a0/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			ram_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4c2/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			ram_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a2/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			ram_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}


		/*Hit Check x address*/
		case (0x4c4/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			rom_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a4/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			rom_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		/*Hit Check y address*/
		case (0x4c6/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			rom_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a6/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			rom_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x500/2):
		{
			switch(mcu_ram[0x500/2])
			{
				case 0x8100:
					break;
				case 0x8900:
				{
					cop2_move3_prot();
					break;
				}
				case 0x205:
				{
					cop2_move2_prot();
					break;
				}
				case 0xa100:
					break;
				case 0xb080:
					break;
				case 0xa900:
					break;
				case 0xb880:
				{
					xy_check = cop2_hit_prot();
					break;
				}
				default:
					logerror("DMA CMD 0x500 with parameter = %04x PC = %08x\n",mcu_ram[offset],activecpu_get_previouspc());
			}
			break;
		}


		case (0x660/2):
		{
			legionna_scrollram16[0] = mcu_ram[offset];
			break;
		}
		case (0x662/2):
		{
			legionna_scrollram16[1] = mcu_ram[offset];
			break;
		}
		case (0x664/2):
		{
			legionna_scrollram16[2] = mcu_ram[offset];
			break;
		}
		case (0x666/2):
		{
			legionna_scrollram16[3] = mcu_ram[offset];
			break;
		}
		case (0x668/2):
		{
			legionna_scrollram16[4] = mcu_ram[offset];
			break;
		}
		case (0x66a/2):
		{
			legionna_scrollram16[5] = mcu_ram[offset];
			break;
		}
		case (0x7c0/2):	/* seibu(0) */
		{
			seibu_main_word_w(0,mcu_ram[offset],0xff00);
			break;
		}
		case (0x7c4/2):	/* seibu(1) */
		{
			seibu_main_word_w(1,mcu_ram[offset],0xff00);
			break;
		}
		case (0x7d0/2):	/* seibu(4) */
		{
			seibu_main_word_w(4,mcu_ram[offset],0xff00);
			break;
		}
		case (0x7d8/2):	/* seibu(6) */
		{
			seibu_main_word_w(6,mcu_ram[offset],0xff00);
			break;
		}
		default:
logerror("CPU0 PC %06x unknown MCU write offset: %04x data: %04x\n",activecpu_get_previouspc(),offset,data);
	}
}


static READ16_HANDLER( sdgndmrb_cop_mcu_r )
{
	switch (offset)
	{
		/*hit protection*/
		case (0x580/2):
		{
			/*PC=ce96*/
			/*---- ---- ---- --xx used bits*/
			//if(!xy_check)
			//{
			//  xy_check = 3;
			//  return 0;
			//}
			//else return 3;
			return xy_check;
		}

		/*number protection*/
		case (0x590/2):
		{
			/*BCD read*/
			return (prot_bcd[0] & 0xffff) + 0x3030;
		}
		case (0x592/2):
		{
			return ((prot_bcd[0] & 0xffff0000) >> 16) + 0x3030;
		}
		case (0x594/2):
		{
			return (prot_bcd[1] & 0xffff) + 0x3030;
		}
		case (0x596/2):
		{
			return ((prot_bcd[1] & 0xffff0000) >> 16) + 0x3030;
		}
		case (0x598/2):
		{
			return (prot_bcd[2] & 0xffff) + 0x3030;
		}
		case (0x59a/2):
		{
			return ((prot_bcd[2] & 0xffff0000) >> 16) + 0x3030;
		}
		case (0x59c/2):
		{
			return 0x3030;
		}

		case (0x5b0/2):
			/*check if the DMA has been finished*/
			if(dma_status == 1)
			{
				dma_status = 0;
				return 2;
			}
			return mcu_ram[offset];

		/* Non-protection reads */
		case (0x708/2):	/* seibu sound */
			return seibu_main_word_r(2,0);

		case (0x70c/2):
			return seibu_main_word_r(3,0);

		case (0x714/2):
			return seibu_main_word_r(5,0);

		/* Inputs */

		case (0x740/2):
			return input_port_1_word_r(0,0);

		case (0x744/2):
			return input_port_2_word_r(0,0);

		case (0x748/2):
			return input_port_4_word_r(0,0);

		case (0x74c/2):
			return input_port_3_word_r(0,0);

		case (0x75c/2):
			return input_port_5_word_r(0,0);
	}
//  return mame_rand(Machine);
  	if(offset > (0x500/2) && offset < (0x600/2))
  	{
  		logerror("CPU0 PC %06x MCU read offset: %04x\n",activecpu_get_previouspc(),offset*2);
		//popmessage("PC %06x MCU read: %04x",activecpu_get_previouspc(),offset*2);
	}

	return mcu_ram[offset];
}

static WRITE16_HANDLER( sdgndmrb_cop_mcu_w )
{
	COMBINE_DATA(&mcu_ram[offset]);

	switch (offset)
	{
		case (0x478/2):
		{
			static UINT16 i;
			/*
            AM_RANGE(0x100800, 0x100fff) AM_WRITE(legionna_background_w) AM_BASE(&legionna_back_data)
            AM_RANGE(0x101000, 0x1017ff) AM_WRITE(legionna_foreground_w) AM_BASE(&legionna_fore_data)
            AM_RANGE(0x101800, 0x101fff) AM_WRITE(legionna_midground_w) AM_BASE(&legionna_mid_data)
            AM_RANGE(0x102000, 0x102fff) AM_WRITE(legionna_text_w) AM_BASE(&legionna_textram)
            */
			switch(mcu_ram[offset])
			{
				/*txt layer clearance*/
				case 0x4080:
				for(i=0;i<0x1000;i+=2)
					program_write_word(i+0x102000,0x0000);
				break;
				case 0x4100: break;
				case 0x41c0: break;
				//default: popmessage("%04x",mcu_ram[offset]);
			}
			break;
		}

		/*The following two transfers the index of the work ram that should be picked up,
          Both of them are united into one index of 32-bits*/

		/*DMA source address*/
		case (0x414/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			dma_src = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x412/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			dma_src = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4c0/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			ram_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a0/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			ram_addr[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4c2/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			ram_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a2/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			ram_addr[1] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}


		/*Hit Check x address*/
		case (0x4c4/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			hit_check_x = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a4/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			hit_check_x = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		/*Hit Check y address*/
		case (0x4c6/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			hit_check_y = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a6/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			hit_check_y = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);
			break;
		}

		/*DMA destination address*/
		case (0x4c8/2):
		{
			/*---- ---- ---- ---- xxxx xxxx xxxx xxxx*/
			prot_data[0] = mcu_ram[offset];
			dma_dst = ((prot_data[0]&0xffff)+4)|((prot_data[1]&0xffff)<<16);
			break;
		}

		case (0x4a8/2):
		{
			/*xxxx xxxx xxxx xxxx ---- ---- ---- ----*/
			prot_data[1] = mcu_ram[offset];
			dma_dst = ((prot_data[0]&0xffff)+4)|((prot_data[1]&0xffff)<<16);
			break;
		}

		/*"Number protection" sub-routine*/
		case (0x420/2):
		{
			//coin counter write
			//popmessage("%04x",mcu_ram[offset]);
			prot_bcd[0] = protection_bcd_jsr(mcu_ram[offset]);
			//prot_bcd = mcu_ram[offset] - 0x22;
			break;
		}

		case (0x422/2):
		{
			prot_bcd[1] = protection_bcd_jsr(mcu_ram[offset]);
			break;
		}

		case (0x424/2):
		{
			prot_bcd[2] = protection_bcd_jsr(mcu_ram[offset]);
			break;
		}

		/*sprite ram clear(Guess)*/
		case (0x47e/2):
		{
			static UINT16 i;
			if(mcu_ram[0x47e/2] == 0x118)
			{
				for(i=0;i<0x800;i+=2)
					program_write_word((0x107000 | i),0x0000);
			}
			break;
		}

		case (0x40c/2):
		{
			dma_size = mcu_ram[offset];
			break;
		}

		case (0x500/2):
		{
			switch(mcu_ram[0x500/2])
			{
				case 0xa180:/*do the job [1]*/
				{
					//popmessage("%08x %08x %04x",dma_src,dma_dst,dma_size);
					/*fix the offset for easier reading*/
					dma_src+=4;
					//dma_dst+=4;
					s_i = dma_size;
					//dma_dst+=((program_read_word(0x110000) & 0x000f) * 8);
					//program_write_word(0x1004c8,dma_dst & 0xffff);
					//dma_status = 1;
					break;
				}
				case 0xb100:
					break;
				case 0xa980:
					break;
				case 0xb900:
				{
					xy_check = hit_check_jsr();
					break;
				}
				/*bullet movement protection,conflicts with [3],will be worked out*/
				case 0x8100:
				{
					//move3x_prot_jsr();
					break;
				}
				case 0x8900:
				{
					//move3y_prot_jsr();
					break;
				}
				case 0x0205:/*do the job [3]*/
				{
					moveprot_jsr();
					break;
				}
				case 0x138e:/*do the job [4]*/
					break;
				case 0x3bb0:
				{
					move2prot_jsr();
					break;
				}
				default:
					logerror("DMA CMD 0x500 with parameter = %04x PC = %08x\n",mcu_ram[offset],activecpu_get_previouspc());
			}
			break;
		}

		case (0x502/2):
		{
			if(mcu_ram[0x502/2] == 0xc480)
			{
				dma_transfer();
				s_i--;
				if(s_i == 0)
					dma_status = 1;
			}
			break;
		}

		/* TODO: tilemaps x-axis are offset,we use a temporary kludge for now */
		case (0x620/2):
		{
			legionna_scrollram16[0] = 0x10 + mcu_ram[offset];
			break;
		}
		case (0x622/2):
		{
			legionna_scrollram16[1] = mcu_ram[offset];
			break;
		}
		case (0x624/2):
		{
			legionna_scrollram16[2] = 0x10 + mcu_ram[offset];
			break;
		}
		case (0x626/2):
		{
			legionna_scrollram16[3] = mcu_ram[offset];
			break;
		}
		case (0x628/2):
		{
			legionna_scrollram16[4] = 0x10 + mcu_ram[offset];
			break;
		}
		case (0x62a/2):
		{
			legionna_scrollram16[5] = mcu_ram[offset];
			break;
		}
		/* scroll mirrors? */
		case (0x62c/2):
		case (0x62e/2):
		case (0x630/2):
		case (0x632/2):
		case (0x634/2):
		case (0x636/2):
			break;
		/*Layer Enable,bit wise active low*/
		case (0x61c/2):
		{
			/*
            ---x ---- (used in test mode)
            ---- x--- Text Layer
            ---- -x-- Foreground Layer
            ---- --x- Midground Layer
            ---- ---x Background Layer
            */
			sdgndmrb_pri_n = mcu_ram[offset] & 0xf;
			break;
		}

		/* Text Layer scroll registers */
		case (0x638/2):
		{
			legionna_scrollram16[6] = 0x38 + mcu_ram[offset];
			break;
		}
		case (0x63a/2):
		{
			legionna_scrollram16[7] = mcu_ram[offset];
			break;
		}
		/*C.R.T. Controller (note:game calls it OBJ register)*/
		case (0x644/2):
			{
				/*
                data = setting
                0x01e = 320x256
                0x0e1 = 320x256 REVERSE
                0x016 = 320x240
                0x0e9 = 320x240 REVERSE
                0x004 = 320x224
                0x10b = 320x224 REVERSE
                It is like to be per cases and not per bits.
                */
				switch(data)
				{
					case 0x0000:
					case 0x0003:
					case 0x001e: CRT_MODE(320,224,0); break;
					case 0x00e1: CRT_MODE(320,224,1); break;
					case 0x0016: CRT_MODE(320,256,0); break;
					case 0x00e9: CRT_MODE(320,256,1); break;
					case 0x0004: CRT_MODE(320,240,0); break;
					case 0x00fb: CRT_MODE(320,240,1); break;
					default:
					#ifdef MAME_DEBUG
					popmessage("Warning: Undefined CRT Mode %04x",data);
					#endif
					CRT_MODE(320,256,0);
				}
			}
			break;

		/* Seems a mirror for the choices in the test menu... */
		case (0x67c/2):
		case (0x680/2):
			break;
		case (0x6fc/2):
			break;

		case (0x700/2):	/* seibu(0) */
		{
			seibu_main_word_w(0,mcu_ram[offset],0xff00);
			break;
		}
		case (0x704/2):	/* seibu(1) */
		{
			seibu_main_word_w(1,mcu_ram[offset],0xff00);
			break;
		}
		case (0x710/2):	/* seibu(4) */
		{
			seibu_main_word_w(4,mcu_ram[offset],0xff00);
			break;
		}
		case (0x718/2):	/* seibu(6) */
		{
			seibu_main_word_w(6,mcu_ram[offset],0xff00);
			break;
		}

//      default:
//      logerror("CPU0 PC %06x MCU write offset: %04x data: %04x\n",activecpu_get_previouspc(),offset*2,data);
//      popmessage("CPU0 PC %06x MCU write offset: %04x data: %04x",activecpu_get_previouspc(),offset*2,data);
	}
}

static READ16_HANDLER( denjinmk_cop_mcu_r )
{
	switch (offset)
	{
		/*number protection*/
		case (0x590/2):
		{
			/*BCD read*/
			return (prot_bcd[0] & 0xffff) + 0x3030;
		}
		case (0x592/2):
		{
			return ((prot_bcd[0] & 0xffff0000) >> 16) + 0x3030;
		}
		case (0x594/2):
		{
			return 0x3030;
		}
		case (0x596/2):
		{
			return 0x3030;
		}
		case (0x598/2):
		{
			return 0x3030;
		}
		case (0x59a/2):
		{
			return 0x3030;
		}
		case (0x59c/2):
		{
			return 0x3030;
		}

		/* Non-protection reads */

		case (0x708/2):	/* seibu sound */
			return seibu_main_word_r(2,0);

		case (0x70c/2):
			return seibu_main_word_r(3,0);

		case (0x714/2):
			return seibu_main_word_r(5,0);

		/* Inputs */

		case (0x740/2):
			return input_port_1_word_r(0,0);

		case (0x744/2):
			return input_port_2_word_r(0,0);

		case (0x748/2):
			return input_port_4_word_r(0,0);

		case (0x74c/2):
			return input_port_3_word_r(0,0);

	}
//logerror("CPU0 PC %06x unknown MCU read offset: %04x\n",activecpu_get_previouspc(),offset);

	return mcu_ram[offset];
}

static WRITE16_HANDLER( denjinmk_cop_mcu_w )
{
	COMBINE_DATA(&mcu_ram[offset]);

	switch (offset)
	{
		/*"Number protection" sub-routine*/
		case (0x420/2):
		{
			//coin counter write
			//popmessage("%04x",mcu_ram[offset]);
			prot_bcd[0] = protection_bcd_jsr(mcu_ram[offset]);
			//prot_bcd = mcu_ram[offset] - 0x22;
			break;
		}


		case (0x470/2):
		{
			denjinmk_setgfxbank( mcu_ram[offset] );
			break;
		}
		case (0x620/2):
		{
			legionna_scrollram16[0] = mcu_ram[offset];
			break;
		}
		case (0x622/2):
		{
			legionna_scrollram16[1] = mcu_ram[offset];
			break;
		}
		case (0x624/2):
		{
			legionna_scrollram16[2] = mcu_ram[offset];
			break;
		}
		case (0x626/2):
		{
			legionna_scrollram16[3] = mcu_ram[offset];
			break;
		}
		case (0x628/2):
		{
			legionna_scrollram16[4] = mcu_ram[offset];
			break;
		}
		case (0x62a/2):
		{
			legionna_scrollram16[5] = mcu_ram[offset];
			break;
		}
		case (0x700/2):	/* seibu(0) */
		{
			seibu_main_word_w(0,mcu_ram[offset],0xff00);
			break;
		}
		case (0x704/2):	/* seibu(1) */
		{
			seibu_main_word_w(1,mcu_ram[offset],0xff00);
			break;
		}
		case (0x710/2):	/* seibu(4) */
		{
			seibu_main_word_w(4,mcu_ram[offset],0xff00);
			break;
		}
		case (0x718/2):	/* seibu(6) */
		{
			seibu_main_word_w(6,mcu_ram[offset],0xff00);
			break;
		}
	    //  default:
	    //  if(offset >= (0x700/2) && offset < (0x800/2))
  	    //printf("CPU0 PC %06x MCU write offset: %04x data: %04x\n",activecpu_get_previouspc(),offset*2,data);
	}
}

static READ16_HANDLER( godzilla_cop_mcu_r )
{
	switch (offset)
	{
		/*number protection*/
		case (0x590/2):
		{
			/*BCD read*/
			return (prot_bcd[0] & 0xffff) + 0x3030;
		}
		case (0x592/2):
		{
			return ((prot_bcd[0] & 0xffff0000) >> 16) + 0x3030;
		}
		case (0x594/2):
		{
			return 0x3030;
		}
		case (0x596/2):
		{
			return 0x3030;
		}
		case (0x598/2):
		{
			return 0x3030;
		}
		case (0x59a/2):
		{
			return 0x3030;
		}
		case (0x59c/2):
		{
			return 0x3030;
		}

		/* Non-protection reads */

		case (0x7c8/2):	/* seibu sound */
			return seibu_main_word_r(2,0);

		case (0x7cc/2):
			return seibu_main_word_r(3,0);

		case (0x7d4/2):
			return seibu_main_word_r(5,0);

		/* Inputs */

		case (0x740/2):
			return input_port_1_word_r(0,0);

		case (0x744/2):
			return input_port_2_word_r(0,0);

		case (0x748/2):
			return input_port_4_word_r(0,0);

		case (0x74c/2):
			return input_port_3_word_r(0,0);

	}
//  if(offset >= (0x400/2) && offset < (0x600/2) && offset != (0x5a4/2))
//  popmessage("CPU0 PC %06x unknown MCU read offset: %04x\n",activecpu_get_previouspc(),offset*2);

	return mcu_ram[offset];
}

static WRITE16_HANDLER( godzilla_cop_mcu_w )
{
	COMBINE_DATA(&mcu_ram[offset]);

	switch (offset)
	{
		/*"Number protection" sub-routine*/
		case (0x420/2):
		{
			//coin counter write
			//popmessage("%04x",mcu_ram[offset]);
			prot_bcd[0] = protection_bcd_jsr(mcu_ram[offset]);
			//prot_bcd = mcu_ram[offset] - 0x22;
			break;
		}

		case (0x478/2):
		{
			static UINT16 i;
			/*
            */
			switch(mcu_ram[offset])
			{
				/*txt layer clearance*/
				case 0x40a0:
				for(i=0;i<0x1000;i+=2)
					program_write_word(i+0x102800,0x0000);
				break;
				case 0x4040: break;
				case 0x4140: break;
				case 0x4180: break;
				case 0x4100: break;
				case 0x41c0: break;
				//default: popmessage("%04x",mcu_ram[offset]);
			}
			break;
		}

		case (0x620/2):
		{
			legionna_scrollram16[0] = mcu_ram[offset];
			break;
		}
		case (0x622/2):
		{
			legionna_scrollram16[1] = mcu_ram[offset];
			break;
		}
		case (0x624/2):
		{
			legionna_scrollram16[2] = mcu_ram[offset];
			break;
		}
		case (0x626/2):
		{
			legionna_scrollram16[3] = mcu_ram[offset];
			break;
		}
		case (0x628/2):
		{
			legionna_scrollram16[4] = mcu_ram[offset];
			break;
		}
		case (0x62a/2):
		{
			legionna_scrollram16[5] = mcu_ram[offset];
			break;
		}

		case (0x7c0/2):	/* seibu(0) */
		{
			seibu_main_word_w(0,mcu_ram[offset],0xff00);
			break;
		}
		case (0x7c4/2):	/* seibu(1) */
		{
			seibu_main_word_w(1,mcu_ram[offset],0xff00);
			break;
		}
		case (0x7d0/2):	/* seibu(4) */
		{
			seibu_main_word_w(4,mcu_ram[offset],0xff00);
			break;
		}
		case (0x7d8/2):	/* seibu(6) */
		{
			seibu_main_word_w(6,mcu_ram[offset],0xff00);
			break;
		}
//      default:
//      logerror("CPU0 PC %06x MCU write offset: %04x data: %04x\n",activecpu_get_previouspc(),offset*2,data);
	}
}

static READ16_HANDLER( copdx_0_r )
{
	switch(offset)
	{
		//case (0x47e/2):
		//case (0x5b0/2):
		//case (0x5b4/2):
		//  return mcu_ram[offset];

		case (0x700/2):
			return input_port_1_word_r(0,0);

		case (0x704/2):
			return input_port_2_word_r(0,0);

		case (0x708/2):
			return input_port_4_word_r(0,0);

		case (0x70c/2):
			return input_port_3_word_r(0,0);

		case (0x71c/2):
			return input_port_5_word_r(0,0);

		case (0x748/2):	/* seibu sound */
			return seibu_main_word_r(2,0);
		case (0x74c/2):
			return seibu_main_word_r(3,0);
		case (0x754/2):
			return seibu_main_word_r(5,0);

	}

	//logerror("COP has read at PC=%06x offset = %04x\n",activecpu_get_pc(),offset*2);

	return mcu_ram[offset];
}

static UINT16 cop_fct;
static UINT32 cop_reg[8];

/*This is version 1,so some stuff is different (less complex)*/
static void cop_run(void)
{
	switch(cop_fct)
	{
		/*???*/
		case 0x8100:
		{
			UINT32 src = cop_reg[0];
			program_write_word(src+0x36,0xffc0);
			break;
		}
		case 0x8900:
		{
			UINT32 src = cop_reg[0];
			program_write_word(src+0x36,0xff80);
			break;
		}
		/*Right*/
		case 0x0205:
		{
			UINT32 src = cop_reg[0];
			INT16 y = program_read_word(src+0x4);
			INT16 x = program_read_word(src+0x8);
			INT16 y_rel = program_read_word(src+0x10);
			INT16 x_rel = program_read_word(src+0x14);
			program_write_word(src+0x4,(y+y_rel));
			program_write_word(src+0x8,(x+x_rel));
			/*printf("%08x %08x %08x %08x %08x\n",cop_reg[0],
                                           program_read_word(cop_reg[0]+0x4),
                                           program_read_word(cop_reg[0]+0x8),
                                           program_read_word(cop_reg[0]+0x10),
                                           program_read_word(cop_reg[0]+0x14));*/
			break;
		}
		/*???*/
		case 0x3bb0:
		{
			//UINT32 dst = cop_reg[0];
			//UINT32 dst = cop_reg[1];
			//program_write_word(dst,  mame_rand(Machine)/*program_read_word(src)*/);
			//program_write_word(dst+2,mame_rand(Machine)/*program_read_word(src+2)*/);
			//program_write_word(dst+4,mame_rand(Machine)/*program_read_word(src+4)*/);
			//program_write_word(dst+6,mame_rand(Machine)/*program_read_word(src+6)*/);
			//printf("%04x\n",cop_reg[0]);
			break;
		}
		default:
			//printf("%04x\n",cop_fct);
			break;
	}
}
//tmp_data[1] = mcu_ram[offset];
//cop_reg[0] = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16);

static void cop_reg_w(UINT16 data,UINT8 offset,UINT8 mask)
{
	if(mask)
		cop_reg[offset] = ((data&0xffff)<<16) | (cop_reg[offset]&0xffff);
	else
		cop_reg[offset] = (cop_reg[offset]&0xffff0000) | (data&0xffff);

	//popmessage("%08x",cop_reg[offset]);
}

static WRITE16_HANDLER( copdx_0_w )
{
	COMBINE_DATA(&mcu_ram[offset]);

	switch(offset)
	{
		case (0x4a0/2):
		case (0x4a2/2):
		case (0x4a4/2):
		case (0x4a6/2):
		case (0x4a8/2):
		case (0x4aa/2):
		case (0x4ac/2):
		case (0x4ae/2):
		case (0x4c0/2):
		case (0x4c2/2):
		case (0x4c4/2):
		case (0x4c6/2):
		case (0x4c8/2):
		case (0x4ca/2):
		case (0x4cc/2):
		case (0x4ce/2):
			cop_reg_w(mcu_ram[offset],offset & 0x000f, (offset < (0x4b0/2)) ? 1 : 0);
			break;
		/*layer clearance,but the bootleg doesn't send values,so this function is an
          original left-over.*/
		case (0x478/2):
		{
			/*
    AM_RANGE(0x100800, 0x100fff) AM_READWRITE(MRA16_RAM,legionna_background_w) AM_BASE(&legionna_back_data)
    AM_RANGE(0x101000, 0x1017ff) AM_READWRITE(MRA16_RAM,legionna_foreground_w) AM_BASE(&legionna_fore_data)
    AM_RANGE(0x101800, 0x101fff) AM_READWRITE(MRA16_RAM,legionna_midground_w) AM_BASE(&legionna_mid_data)
    AM_RANGE(0x102000, 0x102fff) AM_READWRITE(MRA16_RAM,legionna_text_w) AM_BASE(&legionna_textram)
            */
			break;
		}
		case (0x500/2):
		{
			cop_fct = mcu_ram[offset];
			cop_run();
			break;
		}
		case (0x604/2):
		{
			//C.R.T. Controller
			/*
            data = setting
            0x01e = 320x256         ---- ---x xxx-
            0x0e1 = 320x256 REVERSE ---- xxx- ---x
            0x016 = 320x240         ---- ---x -xx-
            0x0e9 = 320x240 REVERSE ---- xxx- x--x
            0x004 = 320x224         ---- ---- -x--
            0x10b = 320x224 REVERSE ---x ---- x-xx
            For now we use this by cases and not per bits.
            */

			switch(data)
			{
				case 0x0000:
				case 0x001e: CRT_MODE(320,256,0); break;
				case 0x00e1: CRT_MODE(320,256,1); break;
				case 0x0016: CRT_MODE(320,240,0); break;
				case 0x00e9: CRT_MODE(320,240,1); break;
				case 0x0004: CRT_MODE(320,224,0); break;
				case 0x010b: CRT_MODE(320,224,1); break;
				default:
				#ifdef MAME_DEBUG
				popmessage("Warning: Undefined CRT Mode %04x",data);
				#endif
				CRT_MODE(320,256,0);
			}
			break;
		}
		/*TODO: what's going on here,some scroll values aren't sent in these locations
                but somewhere else?*/
		case (0x62c/2):
		{
			legionna_scrollram16[0] = mcu_ram[offset];
			break;
		}
		case (0x62e/2):
		{
			legionna_scrollram16[1] = mcu_ram[offset];
			break;
		}
		case (0x630/2):
		{
			legionna_scrollram16[2] = mcu_ram[offset];
			break;
		}
		case (0x632/2):
		{
			legionna_scrollram16[3] = mcu_ram[offset];
			break;
		}
		case (0x634/2):
		{
			legionna_scrollram16[4] = mcu_ram[offset];
			break;
		}
		case (0x636/2):
		{
			legionna_scrollram16[5] = mcu_ram[offset];
			break;
		}
		case (0x638/2):
		{
			legionna_scrollram16[6] = mcu_ram[offset];
			break;
		}
		case (0x63a/2):
		{
			legionna_scrollram16[7] = mcu_ram[offset];
			break;
		}
		/*video regs (not scrollram,something else)*/
		//case (0x660/2):
		//case (0x662/2):
		//case (0x664/2):
		//case (0x666/2):
		//case (0x668/2):
		//case (0x66a/2):
		//case (0x66c/2):
		//case (0x66e/2):
		//  break;

		case (0x740/2):	/* seibu(0) */
		{
			seibu_main_word_w(0,mcu_ram[offset],0xff00);
			break;
		}
		case (0x744/2):	/* seibu(1) */
		{
			seibu_main_word_w(1,mcu_ram[offset],0xff00);
			break;
		}
		case (0x750/2):	/* seibu(4) */
		{
			seibu_main_word_w(4,mcu_ram[offset],0xff00);
			break;
		}
		case (0x758/2):	/* seibu(6) */
		{
			seibu_main_word_w(6,mcu_ram[offset],0xff00);
			break;
		}
	}
	//usrintf_showmessage("COP:write at PC=%06x offset = %04x data %04x\n",activecpu_get_pc(),offset*2,data);
}

static READ16_HANDLER( copdxbl_0_r )
{
	switch(offset)
	{
		//case (0x47e/2):
		//case (0x5b0/2):
		//case (0x5b4/2):
		//  return mcu_ram[offset];

		case (0x700/2):
			return input_port_1_word_r(0,0);

		case (0x704/2):
			return input_port_2_word_r(0,0);

		case (0x708/2):
			return input_port_4_word_r(0,0);

		case (0x70c/2):
			return input_port_3_word_r(0,0);

		case (0x71c/2):
			return input_port_5_word_r(0,0);
#if 0
		case (0x748/2):	/* seibu sound */
			return seibu_main_word_r(2,0);
		case (0x74c/2):
			return seibu_main_word_r(3,0);
		case (0x754/2):
			return seibu_main_word_r(5,0);
			#endif

	}

	//logerror("COP has read at PC=%06x offset = %04x\n",activecpu_get_pc(),offset*2);

	return mcu_ram[offset];
}

static WRITE16_HANDLER( copdxbl_0_w )
{
	COMBINE_DATA(&mcu_ram[offset]);

	switch(offset)
	{
		case (0x4a0/2):
		case (0x4a2/2):
		case (0x4a4/2):
		case (0x4a6/2):
		case (0x4a8/2):
		case (0x4aa/2):
		case (0x4ac/2):
		case (0x4ae/2):
		case (0x4c0/2):
		case (0x4c2/2):
		case (0x4c4/2):
		case (0x4c6/2):
		case (0x4c8/2):
		case (0x4ca/2):
		case (0x4cc/2):
		case (0x4ce/2):
			cop_reg_w(mcu_ram[offset],offset & 0x000f, (offset < (0x4b0/2)) ? 1 : 0);
			break;
		/*layer clearance,but the bootleg doesn't send values,so this function
          is an original left-over.*/
		case (0x478/2):
		{
			/*
    AM_RANGE(0x100800, 0x100fff) AM_READWRITE(MRA16_RAM,legionna_background_w) AM_BASE(&legionna_back_data)
    AM_RANGE(0x101000, 0x1017ff) AM_READWRITE(MRA16_RAM,legionna_foreground_w) AM_BASE(&legionna_fore_data)
    AM_RANGE(0x101800, 0x101fff) AM_READWRITE(MRA16_RAM,legionna_midground_w) AM_BASE(&legionna_mid_data)
    AM_RANGE(0x102000, 0x102fff) AM_READWRITE(MRA16_RAM,legionna_text_w) AM_BASE(&legionna_textram)
            */
			break;
		}
		case (0x500/2):
		{
			cop_fct = mcu_ram[offset];
			cop_run();
			break;
		}
		case (0x604/2):
		{
			//C.R.T. Controller
			/*
            data = setting
            0x01e = 320x256         ---- ---x xxx-
            0x0e1 = 320x256 REVERSE ---- xxx- ---x
            0x016 = 320x240         ---- ---x -xx-
            0x0e9 = 320x240 REVERSE ---- xxx- x--x
            0x004 = 320x224         ---- ---- -x--
            0x10b = 320x224 REVERSE ---x ---- x-xx
            For now we use this by cases and not per bits.
            */

			switch(data)
			{
				case 0x0000:
				case 0x001e: CRT_MODE(320,256,0); break;
				case 0x00e1: CRT_MODE(320,256,1); break;
				case 0x0016: CRT_MODE(320,240,0); break;
				case 0x00e9: CRT_MODE(320,240,1); break;
				case 0x0004: CRT_MODE(320,224,0); break;
				case 0x010b: CRT_MODE(320,224,1); break;
				default:
				#ifdef MAME_DEBUG
				popmessage("Warning: Undefined CRT Mode %04x",data);
				#endif
				CRT_MODE(320,256,0);
			}
			break;
		}
		/*TODO: kludge on x-axis.*/
		case (0x660/2):
		{
			legionna_scrollram16[0] = mcu_ram[offset] - 0x1f0;
			break;
		}
		case (0x662/2):
		{
			legionna_scrollram16[1] = mcu_ram[offset];
			break;
		}
		case (0x664/2):
		{
			legionna_scrollram16[2] = mcu_ram[offset] - 0x1f0;
			break;
		}
		case (0x666/2):
		{
			legionna_scrollram16[3] = mcu_ram[offset];
			break;
		}
		case (0x668/2):
		{
			legionna_scrollram16[4] = mcu_ram[offset] - 0x1f0;
			break;
		}
		case (0x66a/2):
		{
			legionna_scrollram16[5] = mcu_ram[offset];
			break;
		}
		case (0x66c/2):
		{
			legionna_scrollram16[6] = mcu_ram[offset] - 0x1f0;
			break;
		}
		case (0x66e/2):
		{
			legionna_scrollram16[7] = mcu_ram[offset];
			break;
		}
		/*WRONG*/
		case (0x65c/2):
		{
			soundlatch_w(1,data&0xff);
			cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE );
			break;
		}
		/*video regs (not scrollram,something else)*/
		//case (0x660/2):
		//case (0x662/2):
		//case (0x664/2):
		//case (0x666/2):
		//case (0x668/2):
		//case (0x66a/2):
		//case (0x66c/2):
		//case (0x66e/2):
		//  break;
		/*bootleg sound HW*/
		/*case (0x740/2):
        {
            soundlatch_w(1,data&0x00ff);
            cpunum_set_input_line(Machine, 1, INPUT_LINE_NMI, PULSE_LINE );
            break;
        }*/
		#if 0
		case (0x740/2):	/* seibu(0) */
		{
			seibu_main_word_w(0,mcu_ram[offset],0xff00);
			break;
		}
		case (0x744/2):	/* seibu(1) */
		{
			seibu_main_word_w(1,mcu_ram[offset],0xff00);
			break;
		}
		case (0x750/2):	/* seibu(4) */
		{
			seibu_main_word_w(4,mcu_ram[offset],0xff00);
			break;
		}
		case (0x758/2):	/* seibu(6) */
		{
			seibu_main_word_w(6,mcu_ram[offset],0xff00);
			break;
		}
		#endif
	}
	//usrintf_showmessage("COP:write at PC=%06x offset = %04x data %04x\n",activecpu_get_pc(),offset*2,data);
}

/*****************************************************************************/

static ADDRESS_MAP_START( legionna_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_READ(mcu_r)	/* COP mcu */
	AM_RANGE(0x101000, 0x1017ff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x101800, 0x101fff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x102000, 0x1027ff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x102800, 0x1037ff) AM_READ(MRA16_RAM)	/* 64x32 text/front layer, 8x8 tiles */

	/* The 4000-4fff area contains PALETTE words and may be extra paletteram? */
	AM_RANGE(0x104000, 0x104fff) AM_READ(MRA16_RAM)	/* palette mirror ? */
//  AM_RANGE(0x104000, 0x10401f) AM_READ(MRA16_RAM) /* debugging... */
//  AM_RANGE(0x104200, 0x1043ff) AM_READ(MRA16_RAM) /* ??? */
//  AM_RANGE(0x104600, 0x1047ff) AM_READ(MRA16_RAM) /* ??? */
//  AM_RANGE(0x104800, 0x10481f) AM_READ(MRA16_RAM) /* ??? */

	AM_RANGE(0x105000, 0x105fff) AM_READ(MRA16_RAM)	/* spriteram */
	AM_RANGE(0x106000, 0x106fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x107000, 0x107fff) AM_READ(MRA16_RAM)	/* palette */
	AM_RANGE(0x108000, 0x11ffff) AM_READ(MRA16_RAM)	/* main ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( legionna_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_WRITE(mcu_w) AM_BASE(&mcu_ram)	/* COP mcu */
	AM_RANGE(0x101000, 0x1017ff) AM_WRITE(legionna_background_w) AM_BASE(&legionna_back_data)
	AM_RANGE(0x101800, 0x101fff) AM_WRITE(legionna_foreground_w) AM_BASE(&legionna_fore_data)
	AM_RANGE(0x102000, 0x1027ff) AM_WRITE(legionna_midground_w) AM_BASE(&legionna_mid_data)
	AM_RANGE(0x102800, 0x1037ff) AM_WRITE(legionna_text_w) AM_BASE(&legionna_textram)

	/* The 4000-4fff area contains PALETTE words and may be extra paletteram? */
	AM_RANGE(0x104000, 0x104fff) AM_WRITE(MWA16_RAM)
//  AM_RANGE(0x104000, 0x104fff) AM_WRITE(legionna_paletteram16_w)
//  AM_RANGE(0x104000, 0x10401f) AM_WRITE(MWA16_RAM)
//  AM_RANGE(0x104200, 0x1043ff) AM_WRITE(MWA16_RAM)
//  AM_RANGE(0x104600, 0x1047ff) AM_WRITE(MWA16_RAM)
//  AM_RANGE(0x104800, 0x10481f) AM_WRITE(MWA16_RAM)

	AM_RANGE(0x105000, 0x105fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0x106000, 0x106fff) AM_WRITE(MWA16_RAM)	/* is this used outside inits ?? */
	AM_RANGE(0x107000, 0x107fff) AM_WRITE(legionna_paletteram16_w) AM_BASE(&paletteram16)	/* palette xRRRRxGGGGxBBBBx ? */
	AM_RANGE(0x108000, 0x11ffff) AM_WRITE(MWA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( heatbrl_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_READ(cop2_mcu_r)	/* COP mcu */
	AM_RANGE(0x100800, 0x100fff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x101000, 0x1017ff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x101800, 0x101fff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x102000, 0x102fff) AM_READ(MRA16_RAM)	/* 64x32 text/front layer, 8x8 tiles */
	AM_RANGE(0x103000, 0x103fff) AM_READ(MRA16_RAM)	/* spriteram */
	AM_RANGE(0x104000, 0x104fff) AM_READ(MRA16_RAM)	/* palette */
	AM_RANGE(0x108000, 0x11ffff) AM_READ(MRA16_RAM)	/* main ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( heatbrl_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_WRITE(cop2_mcu_w) AM_BASE(&mcu_ram)	/* COP mcu */
	AM_RANGE(0x100800, 0x100fff) AM_WRITE(legionna_background_w) AM_BASE(&legionna_back_data)
	AM_RANGE(0x101000, 0x1017ff) AM_WRITE(legionna_foreground_w) AM_BASE(&legionna_fore_data)
	AM_RANGE(0x101800, 0x101fff) AM_WRITE(legionna_midground_w) AM_BASE(&legionna_mid_data)
	AM_RANGE(0x102000, 0x102fff) AM_WRITE(legionna_text_w) AM_BASE(&legionna_textram)
	AM_RANGE(0x103000, 0x103fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0x104000, 0x104fff) AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x108000, 0x11ffff) AM_WRITE(MWA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( godzilla_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_READ(godzilla_cop_mcu_r)	/* COP mcu */
	AM_RANGE(0x100800, 0x100fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x101000, 0x101fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x102000, 0x102fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x103000, 0x103fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x104000, 0x104fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x105000, 0x106fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x107000, 0x107fff) AM_READ(MRA16_RAM) /*Ani-DSP ram*/
	AM_RANGE(0x108000, 0x11ffff) AM_READ(MRA16_RAM)	/* main ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( godzilla_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_WRITE(godzilla_cop_mcu_w) AM_BASE(&mcu_ram)	/* COP mcu */
	AM_RANGE(0x100800, 0x100fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x101000, 0x101fff) AM_WRITE(legionna_background_w) AM_BASE(&legionna_back_data)
	AM_RANGE(0x102000, 0x1027ff) AM_WRITE(legionna_midground_w) AM_BASE(&legionna_mid_data)
	AM_RANGE(0x102800, 0x1037ff) AM_WRITE(legionna_text_w) AM_BASE(&legionna_textram)
	AM_RANGE(0x103800, 0x103fff) AM_WRITE(legionna_foreground_w) AM_BASE(&legionna_fore_data)
	AM_RANGE(0x104000, 0x104fff) AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x105000, 0x105fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0x106000, 0x1067ff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x106800, 0x106fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x107000, 0x107fff) AM_WRITE(MWA16_RAM) /*Ani-DSP ram*/
	AM_RANGE(0x108000, 0x11ffff) AM_WRITE(MWA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( denjinmk_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_READ(denjinmk_cop_mcu_r)	/* COP mcu */
	AM_RANGE(0x100800, 0x100fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x101000, 0x101fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x102000, 0x103fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x104000, 0x104fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x105000, 0x105fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x106000, 0x106fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x107000, 0x107fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x108000, 0x11dfff) AM_READ(MRA16_RAM)	/* main ram */
	AM_RANGE(0x11e000, 0x11efff) AM_READ(MRA16_RAM)
	AM_RANGE(0x11f000, 0x11ffff) AM_READ(MRA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( denjinmk_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_WRITE(denjinmk_cop_mcu_w) AM_BASE(&mcu_ram)	/* COP mcu */
	AM_RANGE(0x100800, 0x100fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x101000, 0x1017ff) AM_WRITE(legionna_background_w) AM_BASE(&legionna_back_data)
	AM_RANGE(0x101800, 0x101fff) AM_WRITE(legionna_foreground_w) AM_BASE(&legionna_fore_data)
	AM_RANGE(0x102000, 0x1027ff) AM_WRITE(legionna_midground_w) AM_BASE(&legionna_mid_data)
	AM_RANGE(0x102800, 0x103fff) AM_WRITE(legionna_text_w) AM_BASE(&legionna_textram)
	AM_RANGE(0x104000, 0x104fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x105000, 0x105fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0x106000, 0x107fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x108000, 0x11dfff) AM_WRITE(MWA16_RAM)	/* main ram */
	AM_RANGE(0x11e000, 0x11efff) AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x11f000, 0x11ffff) AM_WRITE(MWA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sdgndmrb_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_READ(sdgndmrb_cop_mcu_r)	/* COP mcu */
	AM_RANGE(0x100800, 0x100fff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x101000, 0x1017ff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x101800, 0x101fff) AM_READ(MRA16_RAM)	/* 32x16 bg layer, 16x16 tiles */
	AM_RANGE(0x102000, 0x102fff) AM_READ(MRA16_RAM)	/* 64x32 text/front layer, 8x8 tiles */
	AM_RANGE(0x103000, 0x103fff) AM_READ(MRA16_RAM)	/* palette */
	AM_RANGE(0x104000, 0x104fff) AM_READ(MRA16_RAM)	/* palette mirror */
	AM_RANGE(0x105000, 0x105fff) AM_READ(MRA16_RAM)
	AM_RANGE(0x107000, 0x107fff) AM_READ(MRA16_RAM)	/* extra spriteram? */
	AM_RANGE(0x108000, 0x11ffff) AM_READ(MRA16_RAM)	/* main ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sdgndmrb_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x100000, 0x1007ff) AM_WRITE(sdgndmrb_cop_mcu_w) AM_BASE(&mcu_ram)	/* COP mcu */
	AM_RANGE(0x100800, 0x100fff) AM_WRITE(legionna_background_w) AM_BASE(&legionna_back_data)
	AM_RANGE(0x101000, 0x1017ff) AM_WRITE(legionna_foreground_w) AM_BASE(&legionna_fore_data)
	AM_RANGE(0x101800, 0x101fff) AM_WRITE(legionna_midground_w) AM_BASE(&legionna_mid_data)
	AM_RANGE(0x102000, 0x102fff) AM_WRITE(legionna_text_w) AM_BASE(&legionna_textram)
	AM_RANGE(0x103000, 0x103fff) AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x104000, 0x104fff) AM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w)
	AM_RANGE(0x105000, 0x105fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x106000, 0x106fff) AM_WRITE(MWA16_RAM)
	AM_RANGE(0x107000, 0x107fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0x108000, 0x11ffff) AM_WRITE(MWA16_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cupsoc_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1007ff) AM_READWRITE(copdx_0_r,copdx_0_w) AM_BASE(&mcu_ram)
	AM_RANGE(0x100800, 0x100fff) AM_READWRITE(MRA16_RAM,legionna_background_w) AM_BASE(&legionna_back_data)
	AM_RANGE(0x101000, 0x1017ff) AM_READWRITE(MRA16_RAM,legionna_foreground_w) AM_BASE(&legionna_fore_data)
	AM_RANGE(0x101800, 0x101fff) AM_READWRITE(MRA16_RAM,legionna_midground_w) AM_BASE(&legionna_mid_data)
	AM_RANGE(0x102000, 0x102fff) AM_READWRITE(MRA16_RAM,legionna_text_w) AM_BASE(&legionna_textram)
	AM_RANGE(0x103000, 0x103fff) AM_READWRITE(MRA16_RAM,paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x104000, 0x104fff) AM_READWRITE(MRA16_RAM,paletteram16_xBBBBBGGGGGRRRRR_word_w) /*<according to the debug mode,there is a DMA that copies from here to the paletteram>*/
	AM_RANGE(0x105000, 0x106fff) AM_RAM
	AM_RANGE(0x107000, 0x1077ff) AM_RAM AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0x107800, 0x107fff) AM_RAM /*Ani Dsp(?) Ram*/
	AM_RANGE(0x108000, 0x10ffff) AM_RAM
	AM_RANGE(0x110000, 0x119fff) AM_RAM
	AM_RANGE(0x11a000, 0x11dfff) AM_RAM//AM_READWRITE(copdxbl_1_r,copdxbl_1_w) AM_BASE(&work_ram)/*shared with the COP MCU too!*/
	AM_RANGE(0x11e000, 0x11ffff) AM_RAM /*Stack Ram*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( cupsocbl_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x1007ff) AM_READWRITE(copdxbl_0_r,copdxbl_0_w) AM_BASE(&mcu_ram)
	AM_RANGE(0x100800, 0x100fff) AM_READWRITE(MRA16_RAM,legionna_background_w) AM_BASE(&legionna_back_data)
	AM_RANGE(0x101000, 0x1017ff) AM_READWRITE(MRA16_RAM,legionna_foreground_w) AM_BASE(&legionna_fore_data)
	AM_RANGE(0x101800, 0x101fff) AM_READWRITE(MRA16_RAM,legionna_midground_w) AM_BASE(&legionna_mid_data)
	AM_RANGE(0x102000, 0x102fff) AM_READWRITE(MRA16_RAM,legionna_text_w) AM_BASE(&legionna_textram)
	AM_RANGE(0x103000, 0x103fff) AM_READWRITE(MRA16_RAM,paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0x104000, 0x104fff) AM_READWRITE(MRA16_RAM,paletteram16_xBBBBBGGGGGRRRRR_word_w) /*<according to the debug mode,there is a DMA that copies from here to the paletteram>*/
	AM_RANGE(0x105000, 0x106fff) AM_RAM
	AM_RANGE(0x107000, 0x1077ff) AM_RAM AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0x107800, 0x107fff) AM_RAM /*Ani Dsp(?) Ram*/
	AM_RANGE(0x108000, 0x10ffff) AM_RAM
	AM_RANGE(0x110000, 0x119fff) AM_RAM
	AM_RANGE(0x11a000, 0x11dfff) AM_RAM//AM_READWRITE(copdxbl_1_r,copdxbl_1_w) AM_BASE(&work_ram)/*shared with the COP MCU too!*/
	AM_RANGE(0x11e000, 0x11ffff) AM_RAM /*Stack Ram*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( cupsocbl_sound_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0x9800, 0x9800) AM_READWRITE(OKIM6295_status_1_r, OKIM6295_data_1_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
ADDRESS_MAP_END

/*****************************************************************************/

static INPUT_PORTS_START( legionna )
	SEIBU_COIN_INPUTS	/* Must be port 0: coin inputs read through sound cpu */

	PORT_START
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0015, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001b, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001d, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0013, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0011, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x001e, "A 1/1 B 1/2" )
	PORT_DIPSETTING(      0x0014, "A 2/1 B 1/3" )
	PORT_DIPSETTING(      0x000a, "A 3/1 B 1/5" )
	PORT_DIPSETTING(      0x0000, "A 5/1 B 1/6" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Freeze" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Extend" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
//  PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( heatbrl )
	SEIBU_COIN_INPUTS	/* Must be port 0: coin inputs read through sound cpu */

	PORT_START
	PORT_DIPNAME( 0x001f, 0x001f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0015, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0017, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0019, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x001b, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x001d, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x001f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0013, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0011, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x001e, "A 1/1 B 1/2" )
	PORT_DIPSETTING(      0x0014, "A 2/1 B 1/3" )
	PORT_DIPSETTING(      0x000a, "A 3/1 B 1/5" )
	PORT_DIPSETTING(      0x0000, "A 5/1 B 1/6" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Players ) )
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0040, 0x0040, "Freeze" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0100, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0400, 0x0400, "Extend" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN4 )	// haven't found coin4, maybe it doesn't exist
//  PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( godzilla )
	SEIBU_COIN_INPUTS	/* Must be port 0: coin inputs read through sound cpu */

	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sdgndmrb )
	SEIBU_COIN_INPUTS	/* Must be port 0: coin inputs read through sound cpu */

	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1) //debug button
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cupsoc )
	SEIBU_COIN_INPUTS	/* Must be port 0: coin inputs read through sound cpu */

	PORT_START
	PORT_DIPNAME( 0x0007, 0x0007, "Coin 1 (3)" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, "Coin 2 (4)" )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Staring Coin" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "x2" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, "Time vs Computer" )
	PORT_DIPSETTING(      0x0200, "1:30" )
	PORT_DIPSETTING(      0x0300, "2:00" )
	PORT_DIPSETTING(      0x0100, "2:30" )
	PORT_DIPSETTING(      0x0000, "3:00" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time vs Player, 2 Players" )
	PORT_DIPSETTING(      0x0800, "2:00" )
	PORT_DIPSETTING(      0x0c00, "2:30" )
	PORT_DIPSETTING(      0x0400, "3:00" )
	PORT_DIPNAME( 0x3000, 0x3000, "Time vs Player, 3 Players" )
	PORT_DIPSETTING(      0x2000, "2:30" )
	PORT_DIPSETTING(      0x3000, "3:00" )
	PORT_DIPSETTING(      0x1000, "3:30" )
	PORT_DIPNAME( 0xc000, 0xc000, "Time vs Player, 4 Players" )
	PORT_DIPSETTING(      0x8000, "3:00" )
	PORT_DIPSETTING(      0xc000, "3:30" )
	PORT_DIPSETTING(      0x4000, "4:00" )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_COIN2 )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN ) //TEST
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_BUTTON1  ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_BUTTON2  ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN ) //TEST
	PORT_BIT( 0x0100, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW,  IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_SERVICE1 )

	PORT_START
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	  0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x0000, "Players / Coin Mode" )
	PORT_DIPSETTING(      0x0000, "4 Players / 1 Coin Slot" )
	PORT_DIPSETTING(      0x0004, "4 Players / 4 Coin Slots" )
	PORT_DIPSETTING(      0x0008, "4 Players / 2 Coin Slots" )
	PORT_DIPSETTING(      0x000c, "2 Players" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xffc0, 0xffc0, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0xffc0, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/*****************************************************************************/


static const gfx_layout legionna_charlayout =
{
	8,8,
	RGN_FRAC(1,4),	/* other half is BK3, decoded in char2layout */
	4,
	{ 0, 4, 4096*16*8+0, 4096*16*8+4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout heatbrl_charlayout =
{
	8,8,
	RGN_FRAC(1,2),	/* second half is junk, like legionna we may need a different decode */
	4,
	{ 0, 4, 4096*16*8+0, 4096*16*8+4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};


static const gfx_layout legionna_char2layout =
{
	16,16,
	256,	/* Can't use RGN_FRAC as (1,16) not supported */
	4,
	{ 0, 4, 4096*16*8+0, 4096*16*8+4 },
	{ 3, 2, 1, 0, 11, 10, 9, 8,
	  1024*16*8 +3,  1024*16*8 +2,  1024*16*8 +1, 1024*16*8 +0,
	  1024*16*8 +11, 1024*16*8 +10, 1024*16*8 +9, 1024*16*8 +8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	  512*16*8 +0*16, 512*16*8 +1*16, 512*16*8 +2*16, 512*16*8 +3*16,
	  512*16*8 +4*16, 512*16*8 +5*16, 512*16*8 +6*16, 512*16*8 +7*16 },
	16*8
};

static const gfx_layout legionna_tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 2*4, 3*4, 0*4, 1*4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
	  64*8+3, 64*8+2, 64*8+1, 64*8+0, 64*8+16+3, 64*8+16+2, 64*8+16+1, 64*8+16+0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static const gfx_layout legionna_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 2*4, 3*4, 0*4, 1*4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
	  64*8+3, 64*8+2, 64*8+1, 64*8+0, 64*8+16+3, 64*8+16+2, 64*8+16+1, 64*8+16+0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static GFXDECODE_START( legionna )
	GFXDECODE_ENTRY( REGION_GFX1, 0, legionna_charlayout,   48*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, legionna_tilelayout,    0*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX4, 0, legionna_char2layout,  32*16, 16 )	/* example BK3 decode */
	GFXDECODE_ENTRY( REGION_GFX2, 0, legionna_spritelayout,  0*16, 8*16 )
	GFXDECODE_ENTRY( REGION_GFX5, 0, legionna_tilelayout,   32*16, 16 )	/* this should be the BK3 decode */
	GFXDECODE_ENTRY( REGION_GFX6, 0, legionna_tilelayout,   16*16, 16 )
GFXDECODE_END

static GFXDECODE_START( heatbrl )
	GFXDECODE_ENTRY( REGION_GFX1, 0, heatbrl_charlayout,    48*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, legionna_tilelayout,    0*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX4, 0, legionna_char2layout,  32*16, 16 )	/* unused */
	GFXDECODE_ENTRY( REGION_GFX2, 0, legionna_spritelayout,  0*16, 8*16 )
	GFXDECODE_ENTRY( REGION_GFX5, 0, legionna_tilelayout,   32*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX6, 0, legionna_tilelayout,   16*16, 16 )
GFXDECODE_END

static GFXDECODE_START( sdgndmrb )
	GFXDECODE_ENTRY( REGION_GFX1, 0, heatbrl_charlayout,    48*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, legionna_tilelayout,    0*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX4, 0, legionna_char2layout,  32*16, 16 )	/* unused */
	GFXDECODE_ENTRY( REGION_GFX2, 0, legionna_spritelayout,  0*16, 8*16 )
	GFXDECODE_ENTRY( REGION_GFX5, 0, legionna_tilelayout,   32*16, 16 )
	GFXDECODE_ENTRY( REGION_GFX6, 0, legionna_tilelayout,   16*16, 16 )
GFXDECODE_END

/*****************************************************************************/

static MACHINE_DRIVER_START( legionna )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000,20000000/2) 	/* ??? */
	MDRV_CPU_PROGRAM_MAP(legionna_readmem,legionna_writemem)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)/* VBL */

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	MDRV_MACHINE_RESET(seibu_sound_1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(legionna)
	MDRV_PALETTE_LENGTH(128*16)

	MDRV_VIDEO_START(legionna)
	MDRV_VIDEO_UPDATE(legionna)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000,1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( heatbrl )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000,20000000/2) 	/* ??? */
	MDRV_CPU_PROGRAM_MAP(heatbrl_readmem,heatbrl_writemem)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)/* VBL */

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	MDRV_MACHINE_RESET(seibu_sound_1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MDRV_GFXDECODE(heatbrl)
	MDRV_PALETTE_LENGTH(128*16)

	MDRV_VIDEO_START(legionna)
	MDRV_VIDEO_UPDATE(legionna)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000,1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( godzilla )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 20000000/2)
	MDRV_CPU_PROGRAM_MAP(godzilla_readmem,godzilla_writemem)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)

	SEIBU2_SOUND_SYSTEM_CPU(14318180/4)

	MDRV_MACHINE_RESET(seibu_sound_1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(61)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(heatbrl)
	MDRV_PALETTE_LENGTH(128*16)

	MDRV_VIDEO_START(legionna)
	MDRV_VIDEO_UPDATE(godzilla)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(14318180/4,1320000,1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( denjinmk )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 20000000/2)
	MDRV_CPU_PROGRAM_MAP(denjinmk_readmem,denjinmk_writemem)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)

	SEIBU2_SOUND_SYSTEM_CPU(14318180/4)

	MDRV_MACHINE_RESET(seibu_sound_1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_REFRESH_RATE(61)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 32*8-1)
	MDRV_GFXDECODE(heatbrl)
	MDRV_PALETTE_LENGTH(128*16)

	MDRV_VIDEO_START(denjinmk)
	MDRV_VIDEO_UPDATE(godzilla)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(14318180/4,1320000,1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sdgndmrb )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 20000000/2)
	MDRV_CPU_PROGRAM_MAP(sdgndmrb_readmem,sdgndmrb_writemem)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)

	SEIBU2_SOUND_SYSTEM_CPU(14318180/4)

	MDRV_MACHINE_RESET(seibu_sound_1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 32*8-1)
	MDRV_GFXDECODE(sdgndmrb)
	MDRV_PALETTE_LENGTH(128*16)

	MDRV_VIDEO_START(legionna)
	MDRV_VIDEO_UPDATE(sdgndmrb)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(14318180/4,1320000,1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cupsoc )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000,20000000/2)
	MDRV_CPU_PROGRAM_MAP(cupsoc_mem,0)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1)/* VBL */

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	MDRV_MACHINE_RESET(seibu_sound_1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(heatbrl)
	MDRV_PALETTE_LENGTH(128*16)

	MDRV_VIDEO_START(cupsoc)
	MDRV_VIDEO_UPDATE(sdgndmrb)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000,1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cupsocbl )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000,12000000)
	MDRV_CPU_PROGRAM_MAP(cupsocbl_mem,0)
	MDRV_CPU_VBLANK_INT(irq4_line_hold,1) /* VBL */

	/*Different Sound hardware*/
	//SEIBU_SOUND_SYSTEM_CPU(14318180/4)
	MDRV_CPU_ADD(Z80,14318180/4)
	MDRV_CPU_PROGRAM_MAP(cupsocbl_sound_mem,0)
	//MDRV_CPU_VBLANK_INT(nmi_line_pulse,1)

	//MDRV_MACHINE_INIT(seibu_sound_1)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_BUFFERS_SPRITERAM)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 32*8-1)
	MDRV_GFXDECODE(heatbrl)
	MDRV_PALETTE_LENGTH(128*16)

	MDRV_VIDEO_START(cupsoc)
	MDRV_VIDEO_UPDATE(sdgndmrb)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, 1000000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD(OKIM6295, 1000000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_2_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( legionna )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "1",           0x00000, 0x20000, CRC(9e2d3ec8) SHA1(8af9ca349389cbbd2b541aafa09de57f87f6fd72) )
	ROM_LOAD32_BYTE( "2",           0x00001, 0x20000, CRC(35c8a28f) SHA1(31a1f2f9e04dfcab4b3357d6d27c24b434a8c14b) )
	ROM_LOAD32_BYTE( "3",           0x00002, 0x20000, CRC(553fc7c0) SHA1(b12a2eea6b2c9bd76c0c74ddf2765d58510f586a) )
	ROM_LOAD32_BYTE( "legion4.bin", 0x00003, 0x20000, CRC(2cc36c98) SHA1(484fc6eeeed89386ec69df0f92919b742cfdd89f) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "6",   0x00000, 0x08000, CRC(fe7b8d06) SHA1(1e5b52ea4b4042940e2ee2db75c7c0f24973422a) )
	ROM_CONTINUE(    0x10000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "7",   0x000000, 0x10000, CRC(88e26809) SHA1(40ee55d3b5329b6f657e0621d93c4caf6a035fdf) )	/* chars, some BK3 tiles too */
	ROM_LOAD( "8",   0x010000, 0x10000, CRC(06e35407) SHA1(affeeb97b7f3cfa9b65a584ebe25c16a5b2c9a89) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj1",     0x000000, 0x100000, CRC(d35602f5) SHA1(79379abf1c8131df47f81f42b2dc6876926a4e9d) )	/* sprites */
	ROM_LOAD( "obj2",     0x100000, 0x100000, CRC(351d3917) SHA1(014562ac55c09227c08275df3129df19d81af164) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "back",     0x000000, 0x100000, CRC(58280989) SHA1(e3eef1f52829a91b8f87cfe27776a1f12679b3ca) )	/* 3 sets of tiles ('MBK','LBK','BK3') */

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* example BK3 decode */
	ROM_COPY( REGION_GFX1, 0x00000, 0x00000, 0x20000 )

	ROM_REGION( 0x020000, REGION_GFX5, ROMREGION_DISPOSE )	/* we _should_ decode all BK3 tiles here */

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles (plus BK3 at end) */
	ROM_COPY( REGION_GFX3, 0x80000, 0x00000, 0x80000 )

	ROM_REGION( 0x020000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "5",   0x00000, 0x20000, CRC(21d09bde) SHA1(8dce5011e083706ac7b57c5aee4b79d30fa8d4cb) )
ROM_END

ROM_START( legionnu )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "1",   0x00000, 0x20000, CRC(9e2d3ec8) SHA1(8af9ca349389cbbd2b541aafa09de57f87f6fd72) )
	ROM_LOAD32_BYTE( "2",   0x00001, 0x20000, CRC(35c8a28f) SHA1(31a1f2f9e04dfcab4b3357d6d27c24b434a8c14b) )
	ROM_LOAD32_BYTE( "3",   0x00002, 0x20000, CRC(553fc7c0) SHA1(b12a2eea6b2c9bd76c0c74ddf2765d58510f586a) )
	ROM_LOAD32_BYTE( "4",   0x00003, 0x20000, CRC(91fd4648) SHA1(8ad6d0512996b88d3c0c7a96912eebaae2333424) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "6",   0x00000, 0x08000, CRC(fe7b8d06) SHA1(1e5b52ea4b4042940e2ee2db75c7c0f24973422a) )
	ROM_CONTINUE(    0x10000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "7",   0x000000, 0x10000, CRC(88e26809) SHA1(40ee55d3b5329b6f657e0621d93c4caf6a035fdf) )	/* chars, some BK3 tiles too */
	ROM_LOAD( "8",   0x010000, 0x10000, CRC(06e35407) SHA1(affeeb97b7f3cfa9b65a584ebe25c16a5b2c9a89) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj1",     0x000000, 0x100000, CRC(d35602f5) SHA1(79379abf1c8131df47f81f42b2dc6876926a4e9d) )	/* sprites */
	ROM_LOAD( "obj2",     0x100000, 0x100000, CRC(351d3917) SHA1(014562ac55c09227c08275df3129df19d81af164) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "back",     0x000000, 0x100000, CRC(58280989) SHA1(e3eef1f52829a91b8f87cfe27776a1f12679b3ca) )	/* 3 sets of tiles ('MBK','LBK','BK3') */

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* example BK3 decode */
	ROM_COPY( REGION_GFX1, 0x00000, 0x00000, 0x20000 )

	ROM_REGION( 0x020000, REGION_GFX5, ROMREGION_DISPOSE )	/* we _should_ decode all BK3 tiles here */

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles (plus BK3 at end) */
	ROM_COPY( REGION_GFX3, 0x80000, 0x00000, 0x80000 )

	ROM_REGION( 0x020000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "5",   0x00000, 0x20000, CRC(21d09bde) SHA1(8dce5011e083706ac7b57c5aee4b79d30fa8d4cb) )
ROM_END

ROM_START( heatbrl )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "1e_ver2.9k",   0x00000, 0x20000, CRC(b30bd632) SHA1(8684dd4787929886b0bce283301e492206ade9d9) )
	ROM_LOAD32_BYTE( "2e_ver2.9m",   0x00001, 0x20000, CRC(f3a23056) SHA1(d8840468535ac59fede60ea5a2928410d9c7a33a) )
	ROM_LOAD32_BYTE( "3e_ver2.9f",   0x00002, 0x20000, CRC(a2c41715) SHA1(a15b7a35ae0792ed00c47426d2e07c445acd8b8d) )
	ROM_LOAD32_BYTE( "4e_ver2.9h",   0x00003, 0x20000, CRC(a50f4f08) SHA1(f468e4a016a53803b8404bacdef5712311c6f0ac) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "barrel.7",   0x00000, 0x08000, CRC(0784dbd8) SHA1(bdf7f8a3a3eb346eb2aeaf4f9bfc49af059d04c9) )
	ROM_CONTINUE(    0x10000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "barrel.6",   0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )	/* chars */
	ROM_LOAD( "barrel.5",   0x010000, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj1",     0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )	/* sprites */
	ROM_LOAD( "obj2",     0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "bg-1",     0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */

	ROM_REGION( 0x080000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "bg-3",     0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_LOAD( "bg-2",     0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x020000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "barrel.8",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )
ROM_END

ROM_START( heatbrlo )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "barrel.1h",   0x00000, 0x20000, CRC(d5a85c36) SHA1(421a42863faa940057ed5637748f791152a15502) )
	ROM_LOAD32_BYTE( "barrel.2h",   0x00001, 0x20000, CRC(5104d463) SHA1(f65ee824508da431567661804f6235b61425b2dd) )
	ROM_LOAD32_BYTE( "barrel.3h",   0x00002, 0x20000, CRC(823373a0) SHA1(1bb7f811df4f85db8ca10e59fe22137a09470def) )
	ROM_LOAD32_BYTE( "barrel.4h",   0x00003, 0x20000, CRC(19a8606b) SHA1(6e950212c532e46bb6645c3c1f8205c2a4ea2c87) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "barrel.7",   0x00000, 0x08000, CRC(0784dbd8) SHA1(bdf7f8a3a3eb346eb2aeaf4f9bfc49af059d04c9) )
	ROM_CONTINUE(    0x10000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "barrel.6",   0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )	/* chars */
	ROM_LOAD( "barrel.5",   0x010000, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

/* Sprite + tilemap gfx roms not dumped, for now we use ones from heatbrlu
Readme mentions as undumped:
barrel1,2,3,4.OBJ
barrel1,2,3,4.BG */

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj1",     0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )	/* sprites */
	ROM_LOAD( "obj2",     0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "bg-1",     0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */

	ROM_REGION( 0x080000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "bg-3",     0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_LOAD( "bg-2",     0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x020000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "barrel.8",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )
ROM_END

ROM_START( heatbrlu )
	ROM_REGION( 0x80000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "1e_ver2.9k",   0x00000, 0x20000, CRC(b30bd632) SHA1(8684dd4787929886b0bce283301e492206ade9d9) )
	ROM_LOAD32_BYTE( "2u",           0x00001, 0x20000, CRC(289dd629) SHA1(fb379e067ffee4e54d55da638e45e22d6b2ef788) )
	ROM_LOAD32_BYTE( "3e_ver2.9f",   0x00002, 0x20000, CRC(a2c41715) SHA1(a15b7a35ae0792ed00c47426d2e07c445acd8b8d) )
	ROM_LOAD32_BYTE( "4e_ver2.9h",   0x00003, 0x20000, CRC(a50f4f08) SHA1(f468e4a016a53803b8404bacdef5712311c6f0ac) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "barrel.7",   0x00000, 0x08000, CRC(0784dbd8) SHA1(bdf7f8a3a3eb346eb2aeaf4f9bfc49af059d04c9) )
	ROM_CONTINUE(    0x10000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "barrel.6",   0x000000, 0x10000, CRC(bea3c581) SHA1(7f7f0a74bf106acaf57c182d47f0c707da2011bd) )	/* chars */
	ROM_LOAD( "barrel.5",   0x010000, 0x10000, CRC(5604d155) SHA1(afc30347b1e1316ec25056c0c1576f78be5f1a72) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj1",     0x000000, 0x100000, CRC(f7a7c31c) SHA1(683e5c7a0732ff5fd56167dd82035ca050de0507) )	/* sprites */
	ROM_LOAD( "obj2",     0x100000, 0x100000, CRC(24236116) SHA1(b27bd771cacd1587d4927e3f489c4f54b5dec110) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "bg-1",     0x000000, 0x100000, CRC(2f5d8baa) SHA1(0bf687c46c603150eadb304adcd78d53a338e615) )

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */

	ROM_REGION( 0x080000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "bg-3",     0x000000, 0x080000, CRC(83850e2d) SHA1(cdc2df8e3bc58319c50768ea2a05b9c7ddc2a652) )

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_LOAD( "bg-2",     0x000000, 0x080000, CRC(77ee4c6f) SHA1(a0072331bc970ba448ac5bb1ae5caa0332c82a99) )

	ROM_REGION( 0x020000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "barrel.8",  0x00000, 0x20000, CRC(489e5b1d) SHA1(ecd69d87ed354d1d08dbe6c2890af5f05d9d67d0) )
ROM_END

/*

Godzilla
Banpresto 1993

This game runs on Seibu hardware, similar to Legionairre.

PCB Layout
|----------------------------------------------------|
|   YM2151  M6295  PCM.922   8    Z80          PAL   |
|     YM3014                                   PAL   |
|           YM3931  SEI0220  14.31818MHz  6116       |
|         (SEI0100)                                  |
|                                OBJ1.648   OBJ3.743 |
|                      SEI0211                       |
|J                               OBJ2.756   OBJ4.757 |
|A                                                   |
|M     DSW1-8                 2.025  4.026  62256    |
|M     DSW2-8                               62256    |
|A           S68E08.844 PAL   1.024  3.023  62256    |
|      6264                   PAL  PAL      62256    |
|      6264                                          |
| SEI0200                   COPX-D2.313   SEI0300    |
| TC110G21AF                              TC25SC900AF|
|                                                    |
|BK1.618             11.620  20MHz   PAL   PAL       |
|          BK3.619   10.615          PAL   68000     |
|----------------------------------------------------|

Notes:
      Z80 clock    : 3.579545MHz
      68000 clock  : 10.000MHz
      YM2151 clock : 3.579545MHz
      M6295 clock  : 1.000MHz, sample rate = clk /132
      VSync        : 61Hz
      HSync        : 15.74kHz
      S68E08.844   : 82S147 bipolar PROM
      BK & OBJ     : 8M MASK (read as 238000)
      Main PRG 1-4 : 27C010 EPROM
      ROMs 8,10,11 : 27C512 EPROM
      COPX-D2.313  : 4M MASK (read as 234200)
      PCM          : 4M MASK (read as 27C040)

      Custom SEIBU chips:
                         SEI0211 and SEI0220: connected to OBJ ROMs
                         SEI300             : connected to PRG ROMs and 68000
                         SEI0200            : connected to BG ROMs

*/

ROM_START( godzilla )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "2.025",        0x000000, 0x020000, CRC(be9c6e5a) SHA1(9a7e49ac9cdbcc02b13b3448544cee5fe398ec16) )
	ROM_LOAD32_BYTE( "1.024",        0x000001, 0x020000, CRC(0d6b663d) SHA1(01e02999cffd2642f7a37e492fe7f83770cddd67) )
	ROM_LOAD32_BYTE( "4.026",        0x000002, 0x020000, CRC(bb8c0132) SHA1(fa8b049f590be710b3cf82f27deade63656db730) )
	ROM_LOAD32_BYTE( "3.023",        0x000003, 0x020000, CRC(bb16e5d0) SHA1(31d8941e6e297b1f410944f0063a4c9219d23f23) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "8.016",        0x000000, 0x08000, CRC(4ab76e43) SHA1(40c34fade03161c4b50f9f6a2ae61078b8d8ea6d) )
	ROM_CONTINUE(			  0x010000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "11.620",       0x000000, 0x010000, CRC(58e0e41f) SHA1(563c633eb3d4df41e467c93957c74b540a0ae43c) )
	ROM_LOAD( "10.615",       0x010000, 0x010000, CRC(9c22bc13) SHA1(a94d9ed63ee1f5e358ebcaf517e6a1c986fa5d96) )

	ROM_REGION( 0x400000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj1.748",     0x300000, 0x100000, CRC(146bacb0) SHA1(1331f04f3d9e6236cec7524e9da1782ed1916ff7) )
	ROM_LOAD( "obj2.756",     0x200000, 0x100000, CRC(91c2a6a5) SHA1(0e9d9d94c3d99a54c6f9f99270e65682eb0a8b6a) )
	ROM_LOAD( "obj3.743",     0x100000, 0x100000, CRC(5af0114e) SHA1(9362de9ade6db67ab0e3a2dfea580e688bbf7729) )
	ROM_LOAD( "obj4.757",     0x000000, 0x100000, CRC(7448b054) SHA1(5c08319329eb8c90b63e5393c0011bc39911ebbb) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "bg1.618",      0x000000, 0x100000, CRC(78fbbb84) SHA1(b1f5d4041bb88c5b2a561949239b11c3fd7c5fbc) )

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */

	ROM_REGION( 0x100000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "bg2.619",      0x000000, 0x100000, CRC(8ac192a5) SHA1(54b557e81a704c70a651e6b8da70207a2a70530f) )

	ROM_REGION( 0x100000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_COPY( REGION_GFX3, 0x00000, 0x00000, 0x100000 )

	ROM_REGION( 0x080000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "pcm.922",      0x000000, 0x080000, CRC(59cbef10) SHA1(6b89b7286f80f9c903dfb81dc93a03c38dff707c) )

	ROM_REGION( 0x200, REGION_PROMS, 0 )
	ROM_LOAD( "s68e08.844",   0x000000, 0x000200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */

	ROM_REGION( 0x080000, REGION_USER1, 0 )
	ROM_LOAD( "copx-d2.313",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) )
ROM_END

/*

Denjin Makai
Banpresto, 1994

This game runs on early 90's Seibu hardware.
(i.e. Raiden II, Godzilla, Seibu Cup Soccer, Legionairre, Heated Barrel etc).
The PCB looks to have been converted from some other game (a lot of flux traces are
left on the PCB). The game might be a simple ROM swap for some other Seibu-based game.


PCB Layout
----------

|-----------------------------------------------------|
|LA4460   YM2151   M6295  ROM6  ROM5  Z80       PAL6  |
|                                     6116      PAL9  |
|        SEI0100BU   SEI0220BP  14.31818MHz           |
|                                                     |
|        6116                                         |
|                              OBJ-0-3     OBJ-6-7    |
|J       6116      SEI0211                            |
|A                             OBJ-4-5     OBJ-8-9    |
|M                                                    |
|M                        ROM1     ROM3      62256    |
|M                                           62256    |
|                         ROM2     ROM4      62256    |
|     DSW1   S68E08  PAL7                    62256    |
|     DSW2                PAL6  PAL3                  |
|SEI0200                                              |
|     6264                                            |
|     6264                   COPX-D2                  |
|                                             SEI1000 |
|BG-1-AB            ROM7         PAL2  PAL1           |
|                                                     |
|BG-2-AB   BG-3-AB  ROM8  20MHz  PAL5  68000          |
|-----------------------------------------------------|
Notes:
      68000 clock  : 10.000MHz (20 / 2)
      Z80 clock    : 3.579545MHz (14.31818 / 4)
      YM2151 clock : 3.579545MHz (14.31818 / 4)
      M6295 clock  : 1.000MHz, sample rate = M6295 clock / 132
      VSync        : 56Hz

      62256    : 32K x8 SRAM
      6264     : 8K  x8 SRAM
      6116     : 2K  x8 SRAM
      SEI0200  : Custom Seibu QFP100 also stamped TC110G21AF
      SEI1000  : Custom Seibu QFP184 also stamped SB01-001
      SEI0211  : Custom Seibu QFP128
      SEI0100BU: Custom Seibu SDIP64 also stamped YM3931
      SEI0220BP: Custom Seibu QFP80

      ROMs 1, 2, 3, 4   Main program   27C020 EPROM
      ROM  6            OKI samples    27C020 EPROM
      ROM  5            Sound program  27C512 EPROM
      ROMs 7 and 8      Graphics       27C512 EPROM
      ROMs BG*          Graphics       8M Mask ROM
      ROMs OBJ*         Graphics       8M Mask ROM
      COPX-D2           ?              4M Mask ROM

      PAL1 : type AMI 18CV8PC,    labelled 'S68E01'
      PAL2 : type MMI PAL16L8ACN, labelled 'S68E02'
      PAL3 : type MMI PAL16L8ACN, labelled 'S68E03'
      PAL4 : type AMI 18CV8PC,    labelled 'S68E04'
      PAL5 : type AMI 18CV8PC,    labelled 'S68E05T'
      PAL6 : type AMI 18CV8PC,    labelled 'S68E06T'
      PAL7 : type AMI 18CV8PC,    labelled 'S68E07'
      PAL9 : type MMI PAL16L8ACN, labelled 'S68E09'

      S68E08 : PROM type 82S147, labelled 'S68E08'


*/

ROM_START( denjinmk )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "rom1.025",        0x000000, 0x040000, CRC(44a648e8) SHA1(a3c1721e89ac6b9fc16f80682b2f701cb24b5d76) )
	ROM_LOAD32_BYTE( "rom2.024",        0x000001, 0x040000, CRC(e5ee8fe0) SHA1(2ebff4fdbe82062fb526598e10f11358b0b5c02f) )
	ROM_LOAD32_BYTE( "rom3.026",        0x000002, 0x040000, CRC(781b942e) SHA1(f1f4ddc332de3dc29b716a1b82c2ecc2045efb3a) )
	ROM_LOAD32_BYTE( "rom4.023",        0x000003, 0x040000, CRC(502a588b) SHA1(9055b631240fe52d33b572e34275d31a9f3d290f) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "rom5.016",        0x000000, 0x08000, CRC(7fe7e352) SHA1(1ceae22186751ca91dfffab7bd11f275e693451f) )
	ROM_CONTINUE(			  0x010000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rom7.620",       0x000000, 0x010000, CRC(e1f759b1) SHA1(ddc60e78e7791a59c59403dd4089b3f6e1ecf8cb) )
	ROM_LOAD( "rom8.615",       0x010000, 0x010000, CRC(cc36af0d) SHA1(69c2ae38f03be79be4d138fcc73a6a86407eb285) )

	ROM_REGION( 0x400000, REGION_GFX2, ROMREGION_DISPOSE )
/**/ROM_LOAD( "obj-0-3.748",     0x000000, 0x100000, CRC(3dcc7b04) SHA1(3c3ad5ddc18a42046348dcb54e65f6173c003d72) )
	ROM_LOAD( "obj-8-9.757",     0x100000, 0x100000, CRC(c8f7e1c9) SHA1(a746d187b50a0ecdd5a7f687a2601e5dc8bfe272) )
	ROM_LOAD( "obj-4-5.756",     0x200000, 0x100000, CRC(01f8d4e6) SHA1(25b69da693be8c3404f750b419c330a7a56e88ec) )
	ROM_LOAD( "obj-6-7.743",     0x300000, 0x100000, CRC(e5805757) SHA1(9d392c27eef7c1fcda560dac17ba9d7ae2287ac8) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "bg-1-ab.618",      0x000000, 0x100000, CRC(eaad151a) SHA1(bdd1d83ee8497efe20f21baf873e786446372bcb) )

	ROM_REGION( 0x100000, REGION_GFX4, ROMREGION_DISPOSE )	/* BK2 used */
	ROM_LOAD( "bg-2-ab.617",      0x000000, 0x100000, CRC(40938f74) SHA1(d68b0f8245a8b390ad5d4e6ebc7514a939b8ac51) )

	ROM_REGION( 0x100000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "bg-3-ab.619",      0x000000, 0x100000,  CRC(de7366ee) SHA1(0c3969d15f3cd963e579d4164b6e0a6b4012c9c6) )

	ROM_REGION( 0x100000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_COPY( REGION_GFX4, 0x00000, 0x00000, 0x100000 )

	ROM_REGION( 0x080000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "rom6.922",      0x000000, 0x040000, CRC(09e13213) SHA1(9500e057104c6b83da0467938e46d9efa2f49f4c) )

	ROM_REGION( 0x200, REGION_PROMS, 0 )
	ROM_LOAD( "s68e08.844",   0x000000, 0x000200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */

	ROM_REGION( 0x080000, REGION_USER1, 0 )
	ROM_LOAD( "copx-d2.313",  0x000000, 0x080000, CRC(7c52581b) SHA1(7e668476f886806b0c06fa0bcf4bbc955878c87c) )
ROM_END

/*

SD Gundam Sangokushi Rainbow Tairiku Senki
(c)1993 Banpresto
TYPE-R
Board made by Seibu?

CPU  : MC68000P10
Sound: Z80A YM2151 M6295 Y3014B
OSC  : 20.0000MHz (X11), 14.31818MHz (X71)

ROMs:
rb-p1.25 - Main programs (27c020)
rb-p2.24 |
rb-p3.26 |
rb-p4.23 /

rb-s.016 - Sound program (27c512)
rb-ad.922 - Sound data (27c1001)

rb-bg-01.618 - Background (TC538200AP)
rb-bg-2.619  |
rb-f1.620    | (27c512)
rb-f2.615    /

rb-spr01.748 - Sprites (TC538200AP)
rb-spr23.756 /

copx-d2.313 - ? (2M-16bit)

s68e08.844 - (N82S147N)

PALs:
s68e01.122
s68e02.310 (16L8ACN)
s68e03.322 (16L8ACN)
s68e04.551
s68e05.552
s68e06r.741 (18CV8)
s68e07.842
s68e09.015 (16L8ACN)


Custom chips:
SEI0100BU YM3931 9149 EALA
SEI0220BP JAPAN S 9208 U ("S" for "Sharp")
SEI0211 9215 ABBB
SEI0200 TC110G21AF 0076 9324EAI JAPAN
SEI300 TC25SC900AF 001 9211EAI JAPAN

*/

ROM_START( sdgndmrb )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "rb-p1.25",     0x000000, 0x040000, CRC(0995c511) SHA1(97fb2bd7d26720552ace25e655fce09ad9a7afd7) )
	ROM_LOAD32_BYTE( "rb-p2.24",     0x000001, 0x040000, CRC(c9eb756f) SHA1(88d784a71bfab4f321d3320aed1b6b2648529979) )
	ROM_LOAD32_BYTE( "rb-p3.26",     0x000002, 0x040000, CRC(fe2f08a8) SHA1(bb95e5c113a0343b6da43c5dca1292601dec00eb) )
	ROM_LOAD32_BYTE( "rb-p4.23",     0x000003, 0x040000, CRC(f558962a) SHA1(fcfb6f2cba59effd14c76602b0f87f564235d8ef) )

	ROM_REGION( 0x20000, REGION_CPU2, 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "rb-s.016",     0x000000, 0x08000, CRC(8439bf5b) SHA1(089009b91768d64edef6639e7694723d2d1c46ff) )
	ROM_CONTINUE(             0x010000, 0x08000 )

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "rb-f1.620",    0x000000, 0x010000, CRC(792c403d) SHA1(3c606af696fe8f3d6edefdab3940bd5eb341bca9) )
	ROM_LOAD( "rb-f2.615",    0x010000, 0x010000, CRC(a30e0903) SHA1(b9e7646da1ccab6dadaca6beda08125b34946653) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "rb-spr01.748", 0x000000, 0x100000, CRC(11a3479d) SHA1(4d2d06d62da02c6e9884735de8c319f37ca1715c) )
	ROM_LOAD( "rb-spr23.756", 0x100000, 0x100000, CRC(fd08a761) SHA1(3297a2bfaabef17ed9320e24e9a4ffa2f3eb3a44) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "rb-bg-01.618", 0x000000, 0x100000, CRC(6a4ca7e7) SHA1(13612d29f8f04cf62b4357b69b81240dd1eceae4) )

	ROM_REGION( 0x040000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */

	ROM_REGION( 0x100000, REGION_GFX5, ROMREGION_DISPOSE )
	ROM_LOAD( "rb-bg-2.619",  0x000000, 0x100000, CRC(a9b5c85e) SHA1(0ae044e05730e8080d94f1f6758f8dd051b03c41) )

	ROM_REGION( 0x100000, REGION_GFX6, ROMREGION_DISPOSE )
//#define ROM_COPY(rgn,srcoffset,offset,length)
	ROM_COPY( REGION_GFX3, 0x80000, 0x00000, 0x80000 )

	ROM_REGION( 0x20000, REGION_SOUND1, 0 )	 /* ADPCM samples */
	ROM_LOAD( "rb-ad.922",    0x000000, 0x020000, CRC(a364cb42) SHA1(c527b39a1627ecee20a2c4df4cf2b5f2ba729081) )

	ROM_REGION( 0x040000, REGION_USER1, 0 )
	ROM_LOAD( "copx-d2.313",  0x0000, 0x040000, CRC(a6732ff9) SHA1(c4856ec77869d9098da24b1bb3d7d58bb74b4cda) )
ROM_END

ROM_START( cupsoc )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "seibu1.10n",   0x000000, 0x040000, CRC(e91fdc95) SHA1(71c56fffabca79e73dfc61aad17bc58e09a28680) )
	ROM_LOAD32_BYTE( "seibu2.10q",   0x000001, 0x040000, CRC(7816df3c) SHA1(d5cfbf493cc00c47406b314c08e9cbf159a7f98c) )
	ROM_LOAD32_BYTE( "seibu3.10f",   0x000002, 0x040000, CRC(3be8a330) SHA1(f821080acd29c5801abc36da3341aabaea82ceb0) )
	ROM_LOAD32_BYTE( "seibu4.10k",   0x000003, 0x040000, CRC(f30167ea) SHA1(5431296e3245631c90362373027c54166f8fba16) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",    0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(			  0x010000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "seibu6.7x",    0x000000, 0x010000, CRC(21c1e1b8) SHA1(30928c8ef98bf32ba0bf795ddadba1c95fcffe9d) )
	ROM_LOAD( "seibu5.7y",    0x010000, 0x010000, CRC(955d9fd7) SHA1(782451e8e85f7ba285d6cacd9d3fdcf48bde60bc) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj.8c",       0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "back-1.4y",    0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */

	ROM_REGION( 0x080000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "back-2.6y",    0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_COPY( REGION_GFX5, 0x00000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "seibu8.7a",    0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )
ROM_END

ROM_START( cupsoc2 )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "scc_01.bin",   0x000000, 0x040000, CRC(c122203c) SHA1(93c0ae90c0ed3889b9159774ba89536108c9b259) )
	ROM_LOAD32_BYTE( "scc_02.bin",   0x000001, 0x040000, CRC(105511b4) SHA1(f2ebe95a10f5928f57d4f532e2d2432f13b774b2) )
	ROM_LOAD32_BYTE( "scc_03.bin",   0x000002, 0x040000, CRC(2d23d78f) SHA1(c479ded8782f2d23e123b7d00ec57c18a8f80578) )
	ROM_LOAD32_BYTE( "scc_04.bin",   0x000003, 0x040000, CRC(e8877461) SHA1(3be44459699fd455b0daaac10e8a37d1b7985607) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",    0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(			  0x010000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "scc_06.bin",   0x000000, 0x010000, CRC(f1a18ec6) SHA1(43f8ec3fc541b8dc2a17533329dd3448afadcb3b) )
	ROM_LOAD( "scc_05.bin",   0x010000, 0x010000, CRC(c0358503) SHA1(e87991c6a6f3e060a1b03b4899fa891510fca15f) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj.8c",       0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "back-1.4y",    0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */

	ROM_REGION( 0x080000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "back-2.6y",    0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_COPY( REGION_GFX5, 0x00000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "seibu8.7a",    0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )

	ROM_REGION( 0x080000, REGION_USER1, 0 )
	ROM_LOAD( "copx-d1.bin",  0x000000, 0x080000, CRC(029bc402) SHA1(0f64e4c32d95abfa3920b39ed3cf0cc6eb50191b) )
ROM_END

ROM_START( olysoc92 )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )	/* 68000 code */
	ROM_LOAD32_BYTE( "u025.1",       0x000000, 0x040000, CRC(a94e7780) SHA1(abbe328be425b4529e6b75ffa723c6771e4b6fcf) )
	ROM_LOAD32_BYTE( "u024.2",       0x000001, 0x040000, CRC(cb5f0748) SHA1(e11bf11a3766ab33c60a143867496887c6238b11) )
	ROM_LOAD32_BYTE( "u026.3",       0x000002, 0x040000, CRC(f71cc626) SHA1(7f66031509063d5fac33a3b5873b616c7ad0c25b) )
	ROM_LOAD32_BYTE( "u023.4",       0x000003, 0x040000, CRC(2ba10e6c) SHA1(d682d97426a749cfdbaf728edb219dbf84e9eef8) )

	ROM_REGION( 0x20000*2, REGION_CPU2, 0 )	/* Z80 code, banked data */
	ROM_LOAD( "seibu7.8a",    0x000000, 0x08000, CRC(f63329f9) SHA1(51736de48efc14415cfdf169b43623d4c95fde2b) )
	ROM_CONTINUE(			  0x010000, 0x08000 )	/* banked stuff */

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "seibu6.7x",    0x000000, 0x010000, CRC(21c1e1b8) SHA1(30928c8ef98bf32ba0bf795ddadba1c95fcffe9d) )
	ROM_LOAD( "seibu5.7y",    0x010000, 0x010000, CRC(955d9fd7) SHA1(782451e8e85f7ba285d6cacd9d3fdcf48bde60bc) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj.8c",       0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "back-1.4y",    0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x020000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */

	ROM_REGION( 0x080000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "back-2.6y",    0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_COPY( REGION_GFX5, 0x00000, 0x00000, 0x080000 )

	ROM_REGION( 0x040000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "seibu8.7a",    0x000000, 0x040000, CRC(6f594808) SHA1(218aa12068aa587c7656355f6a6b86d97c868774) )
ROM_END

/*

Seibu Cup Soccer - Seibu - Bootleg

2 boards

1st board

(snd)
1 x z80
1 x oki 6295
sc_01 (prg)
sc_02 and sc_03 (data)

(prg)
1 x 68000
sc_04 and sc_05

(gfx)
2 x ti tpc1020
from sc_06 to sc_11

2nd board

(gfx)
1 x actel pl84c
from sc_12 to sc_15

*/

ROM_START( cupsocbl )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "sc_04.bin", 0x00001, 0x80000, CRC(22566087) SHA1(4392f46ca50cc9947823a5190aa25f5e9654aa0d) )
	ROM_LOAD16_BYTE( "sc_05.bin", 0x00000, 0x80000, CRC(2f977dff) SHA1(4d8d6e7d06ce17bb7292072965911f8b1f1067e2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )	/* Z80 code */
	/*First part is full of $FF,a bad dump?*/
	ROM_LOAD( "sc_01.bin",    0x000000, 0x08000, CRC(cea39d6d) SHA1(f0b79c03ffafdd1e57673d6d4836becbe415110b) )
	ROM_CONTINUE(			  0x000000, 0x08000 )

	ROM_REGION( 0x020000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "seibu6.7x",    0x000000, 0x010000, CRC(21c1e1b8) SHA1(30928c8ef98bf32ba0bf795ddadba1c95fcffe9d) )
	ROM_LOAD( "seibu5.7y",    0x010000, 0x010000, CRC(955d9fd7) SHA1(782451e8e85f7ba285d6cacd9d3fdcf48bde60bc) )

	ROM_REGION( 0x200000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "obj.8c",       0x000000, 0x100000, CRC(e2377895) SHA1(1d1c7f31a08a464139cdaf383a5e1ade0717dc9f) )
	ROM_RELOAD(               0x100000, 0x100000 )

	ROM_REGION( 0x100000, REGION_GFX3, ROMREGION_DISPOSE )	/* MBK tiles */
	ROM_LOAD( "back-1.4y",    0x000000, 0x100000, CRC(3dfea0ec) SHA1(8f41d267e488e07831946ef898d593897f10bfe2) )

	ROM_REGION( 0x100000, REGION_GFX4, ROMREGION_DISPOSE )	/* not used */
	ROM_COPY(REGION_GFX3,0x00000,0x00000,0x100000)

	ROM_REGION( 0x080000, REGION_GFX5, ROMREGION_DISPOSE )	/* BK3 tiles */
	ROM_LOAD( "back-2.6y",    0x000000, 0x080000, CRC(e07712af) SHA1(2a0285d6a1e0141838e898252b8d922a6263b05f) )

	ROM_REGION( 0x080000, REGION_GFX6, ROMREGION_DISPOSE )	/* LBK tiles */
	ROM_COPY( REGION_GFX5, 0x00000, 0x00000, 0x080000 )

	/*bootleg GFX roms,for now load the original roms*/
	ROM_REGION( 0x500000, REGION_USER2, 0 )
	ROM_LOAD( "sc_07.bin", 0x000000, 0x080000, CRC(dcb29d01) SHA1(72b4234622605f0ab03f21fdb6a61c6dac36000d) )
	ROM_LOAD( "sc_06.bin", 0x080000, 0x080000, CRC(2dc70e05) SHA1(f1d0beb8428a7e1d7c7818e6719abdc543b2fa80) )
	ROM_LOAD( "sc_13.bin", 0x100000, 0x010000, CRC(229bddd8) SHA1(0924bf29db9c5a970546f154e7752697fdce6a58) )
	ROM_LOAD( "sc_12.bin", 0x110000, 0x010000, CRC(dabfa826) SHA1(0db587c846755491b169ef7751ba8e7cdc2607e6) )
	ROM_LOAD( "sc_08.bin", 0x200000, 0x080000, CRC(637120f3) SHA1(b4b2ad192e46ff80d4cb440d7fb6dac215a353ed) )
	ROM_LOAD( "sc_09.bin", 0x280000, 0x080000, CRC(695b6342) SHA1(dfccb43789021ba2568b9284ae61e64f7f89b152) )
	ROM_LOAD( "sc_14.bin", 0x300000, 0x080000, CRC(566086c2) SHA1(b7d09ce978f99ecc0d1975b31330ed49317701d5) )
	ROM_LOAD( "sc_15.bin", 0x380000, 0x080000, CRC(8fd87e65) SHA1(acc9fd0289fa9ab60bec16d3e642039380e5180a) )
	ROM_LOAD( "sc_10.bin", 0x400000, 0x080000, CRC(27e172b8) SHA1(ed86db2f42c8061607d46f2407b0130aaf692a02) )
	ROM_LOAD( "sc_11.bin", 0x480000, 0x080000, CRC(0cd5ca5e) SHA1(a59665e543e9383355de2576e6693348ec356591) )

	ROM_REGION( 0x020000, REGION_SOUND2, 0 )	/* ADPCM samples */
	ROM_LOAD( "sc_02.bin",    0x000000, 0x20000, CRC(a70d4f03) SHA1(c2482e624c8a828a94206a36d10c1021ad8ca1d0) )

	ROM_REGION( 0x080000, REGION_SOUND1, 0 ) // sound related ?
	ROM_LOAD( "sc_03.bin",    0x000000, 0x080000, CRC(6e254d12) SHA1(857779dbd276b688201a8ea3afd5817e38acad2e) )
ROM_END

static DRIVER_INIT( legionna )
{
	/* Unscramble gfx: quarters 1&2 swapped, quarters 3&4 swapped */

	UINT8 *gfx = memory_region(REGION_GFX1);
	int len = memory_region_length(REGION_GFX1)/2;
	int a,i;

	for (i = 0; i < len/2; i++)
	{
		a = gfx[i];
		gfx[i] = gfx[i + len/2];
		gfx[i+len/2] = a;

		a = gfx[i+len];
		gfx[i+len] = gfx[i + len/2 + len];
		gfx[i + len/2 +len] = a;
	}
}

#define CUPSOC_DEBUG_MODE 1

static DRIVER_INIT( cupsoc )
{
	#if CUPSOC_DEBUG_MODE
	UINT16 *ROM = (UINT16 *)memory_region(REGION_CPU1);

	/*Press p1 button 3 to enter into debug mode during gameplay*/
	ROM[0xffffb/2] = 0x0000;
	ROM[0xffff7/2] = 0x0000;
	#endif
}


GAME( 1992, legionna, 0,        legionna, legionna, legionna, ROT0, "Tad", "Legionnaire (World)", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1992, legionnu, legionna, legionna, legionna, legionna, ROT0, "Tad (Fabtek license)", "Legionnaire (US)", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )

GAME( 1992, heatbrl,  0,        heatbrl,  heatbrl,  0,        ROT0, "Tad", "Heated Barrel (World)", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1992, heatbrlo, heatbrl,  heatbrl,  heatbrl,  0,        ROT0, "Tad", "Heated Barrel (World old version)", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1992, heatbrlu, heatbrl,  heatbrl,  heatbrl,  0,        ROT0, "Tad", "Heated Barrel (US)", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )

GAME( 1993, godzilla, 0,        godzilla, godzilla, 0,        ROT0, "Banpresto", "Godzilla", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1993, sdgndmrb, 0,        sdgndmrb, sdgndmrb, 0, 		  ROT0, "Banpresto", "SD Gundam Sangokushi Rainbow Tairiku Senki", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1993, denjinmk, 0,        denjinmk, godzilla, 0,        ROT0, "Banpresto", "Denjin Makai", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )

GAME( 1992, cupsoc,   0,        cupsoc,  cupsoc,    0,        ROT0, "Seibu", "Seibu Cup Soccer", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1992, cupsoc2,  cupsoc,   cupsoc,  cupsoc,    0,        ROT0, "Seibu", "Seibu Cup Soccer (set 2)", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1992, olysoc92, cupsoc,   cupsoc,  cupsoc,    0,        ROT0, "Seibu", "Olympic Soccer '92", GAME_UNEMULATED_PROTECTION | GAME_NOT_WORKING )
GAME( 1992, cupsocbl, cupsoc,   cupsocbl, cupsoc,   cupsoc, ROT0, "bootleg", "Seibu Cup Soccer (bootleg)", GAME_NOT_WORKING | GAME_NO_SOUND )
