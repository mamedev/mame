
/* Seibu 'COP' (co-processor)  protection

  there appear to be 3 revisions of this protection (based on the external rom)

  COPX-D1 - Seibu Cup Soccer / Olympic Soccer '92
          - Legionnaire
  COPX-D2 - Heated Barrel
          - Godzilla
          - SD Gundam Sangokushi Rainbow Tairiku Senki
          - Denjin Makai
          - Raiden 2
          - Raiden DX
          - Zero Team
  COPX-D3 - Raiden 2/DX New (V33 PCB version)
          - New Zero Team

  COPX / COPX-D2 based games appear to function in a similar way to each other
  while the games using COPX-D3 appears to access the protection device very
  differently to the others.

  As this is only the external rom it isn't confirmed that the actual protection
  devices are identical even where the external rom matches.

  Protection features include BCD math protection, 'command sequences', DMA.
  memory clearing etc.

  it is not confirmed which custom Seibu chip contains the actual co-processor,
  nor if the co-processor is a real MCU with internal code, or a custom designed
  'blitter' like device.

  I suspect that for the earlier games it's part of the COP300 or COP1000 chips,
  for the COPX-D3 based games it's probably inside the system controller SEI333

  the external COP rom is probably used as a lookup for maths operations.

  there should probably only be a single cop2_r / cop2_w function, the chip
  looks to be configurable via the table uploaded to
  0x432 / 0x434 / 0x438 / 0x43a / 0x43c, with 'macro' commands triggered via
  writes to 0x500.

  this simulation is incomplete

  to do:
  clean it up, consolidate code, make it work!

 */

#include "driver.h"
#include "audio/seibu.h"

#define seibu_cop_log printf

UINT16 *cop_mcu_ram;

static UINT16 copd2_table[0x100];
static UINT16 copd2_table_2[0x100/8];
static UINT16 copd2_table_3[0x100/8];
static UINT16 copd2_table_4[0x100/8];

static UINT16 cop_438;
static UINT16 cop_43a;
static UINT16 cop_43c;

static UINT16 cop_clearfill_address[0x200];
static UINT16 cop_clearfill_length[0x200];
static UINT16 cop_clearfill_value[0x200];
static UINT16 cop_clearfill_lasttrigger = 0;


static UINT16 copd2_offs = 0;

extern UINT16* legionna_scrollram16;
extern UINT8 sdgndmrb_pri_n;
extern void heatbrl_setgfxbank(UINT16 data);
extern void denjinmk_setgfxbank(UINT16 data);
extern UINT16 legionna_layer_disable;

void copd2_set_tableoffset(UINT16 data, running_machine *machine)
{
	logerror("mcu_offs %04x\n", data);
	copd2_offs = data;
	if (copd2_offs>0xff)
	{
		logerror("copd2 offs > 0x100\n");
	}

	copd2_table_2[copd2_offs/8] = cop_438;
	copd2_table_3[copd2_offs/8] = cop_43a;
	copd2_table_4[copd2_offs/8] = cop_43c;

	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"copdat_%s.table2", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(copd2_table_2, 0x200/8, 1, fp);
			fclose(fp);
		}
	}
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"copdat_%s.table3", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(copd2_table_3, 0x200/8, 1, fp);
			fclose(fp);
		}
	}
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"copdat_%s.table4", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(copd2_table_4, 0x200/8, 1, fp);
			fclose(fp);
		}
	}

}

void copd2_set_tabledata(UINT16 data, running_machine *machine)
{
	copd2_table[copd2_offs] = data;
	logerror("mcu_data %04x\n", data);

	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"copdat_%s.data", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(copd2_table, 0x200, 1, fp);
			fclose(fp);
		}
	}
}


/*Movement protection*//*Legionnaire,Heated Barrel*/
static UINT32 cop_register[5];
/*Sprite DMA protection*//*SD Gundam*/
static UINT8 dma_status;
static UINT32 dma_src;
static UINT16 prot_data[2],dma_size;
/*Number protection*//*Heated Barrel,SD Gundam,Godzilla,Denjin Makai*/
static UINT32 prot_bcd[4];
/*Hit check protection*//*Legionnaire,Heated Barrel,SD Gundam*/
static UINT8 xy_check;




#define CRT_MODE(_x_,_y_,_flip_) \
	{ \
	rectangle visarea = *video_screen_get_visible_area(machine->primary_screen); \
	visarea.min_x = 0; \
	visarea.max_x = _x_-1; \
	visarea.min_y = 0; \
	visarea.max_y = _y_-1; \
	video_screen_configure(machine->primary_screen, _x_, _y_, &visarea, video_screen_get_frame_period(machine->primary_screen).attoseconds ); \
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
	move_data = program_read_word(cop_register[0]+0x36);
	x_axis = program_read_word(cop_register[0]+0x08);
	y_axis = program_read_word(cop_register[0]+0x04);

	distance = (move_data & 0xf);
	move_type = (move_data & 0xf0)>>4;
	switch(move_type)
	{
		case 0x0f://right
			program_write_word(cop_register[0]+0x08,x_axis+distance);
			//program_write_word(0x110004,);
			break;
		case 0x0b://up
			program_write_word(cop_register[0]+0x04,y_axis-distance);
			break;
		case 0x07://left
			program_write_word(cop_register[0]+0x08,x_axis-distance);
			break;
		case 0x03://down
			program_write_word(cop_register[0]+0x04,y_axis+distance);
			break;
		case 0x0d://up-right
			program_write_word(cop_register[0]+0x08,x_axis+distance);
			program_write_word(cop_register[0]+0x04,y_axis-distance);
			break;
		case 0x09://up-left
			program_write_word(cop_register[0]+0x04,y_axis-distance);
			program_write_word(cop_register[0]+0x08,x_axis-distance);
			break;
		case 0x01://down-right
			program_write_word(cop_register[0]+0x04,y_axis+distance);
			program_write_word(cop_register[0]+0x08,x_axis+distance);
			break;
		case 0x05://down-left
			program_write_word(cop_register[0]+0x04,y_axis+distance);
			program_write_word(cop_register[0]+0x08,x_axis-distance);
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
	x_pl = program_read_word(cop_register[1]+0x8);
	y_pl = program_read_word(cop_register[1]+0x4);
	x_en = program_read_word(cop_register[0]+0x8);
	y_en = program_read_word(cop_register[0]+0x4);

	res = 0;
	if(x_en > x_pl)
		res|=0x80;

	if((x_en > x_pl-0x20) && (x_en < x_pl+0x20))
		res|=0x40;
//...
	//if(y_en > y_pl)
	//  res|=0x40;

	program_write_word(cop_register[0]+0x36,res);
}

#ifdef UNUSED_FUNCTION
/*"To point" movement protection*/
static void move3x_prot_jsr(void)
{
	static INT16 x_pl,x_en,x_dis;
	x_pl = program_read_word(cop_register[1]+0x8);
	x_en = program_read_word(cop_register[0]+0x8);
	x_dis = ((program_read_word(cop_register[0]+0x34) & 0xf0) >> 4);

	if(x_en > x_pl)
		x_dis^=0xffff;

	program_write_word(cop_register[0]+0x36,-0x40);/*enable command*/
	program_write_word(cop_register[0]+0x14,x_dis);
}

static void move3y_prot_jsr(void)
{
	static INT16 y_pl,y_en,y_dis;
	y_pl = program_read_word(cop_register[1]+0x4);
	y_en = program_read_word(cop_register[0]+0x4);
	y_dis = (program_read_word(cop_register[0]+0x34) & 0xf);

	if(y_en > y_pl)
		y_dis^=0xffff;

	program_write_word(cop_register[0]+0x36,-0x80);/*enable command*/
	program_write_word(cop_register[0]+0x10,y_dis);
}
#endif






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
		program_write_word(cop_register[5]+4,program_read_word(dma_src) + param);
		/*Sprite Priority (guess)*/
		//param = ((program_read_word(0x100400) & 0x40) ? 0x4000 : 0);
		/*Write the sprite number [offs+1]*/
		program_write_word(cop_register[5]+6,program_read_word(dma_src+2));
		/*Sprite Relative x/y coords*/
		rel_xy = program_read_word(dma_src+4); /*???*/
		/*temporary hardwired,it should point to 0x4c0/0x4a0*/
		abs_x = (program_read_word(0x110008) - program_read_word(0x10048e));
		abs_y = (program_read_word(0x110004) - program_read_word(0x10048c));
		program_write_word(cop_register[5]+8,((rel_xy & 0x7f) + (abs_x) - ((rel_xy & 0x80) ? 0x80 : 0)) & 0x1ff);
		program_write_word(cop_register[5]+10,(((rel_xy & 0x7f00) >> 8) + (abs_y) + (0x10) - ((rel_xy & 0x8000) ? 0x80 : 0)) & 0x1ff);
		cop_register[5]+=8;
		dma_src+=6;
	}
}


/*
    switch(program_read_word(cop_register[2]))
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
	xparam = check_calc(program_read_word(cop_register[2]));
	/*Here we check the destination sprite height*/
	/*0x4a6/0x4c6*/
	yparam = check_calc(program_read_word(cop_register[3]));

	if(!xparam || !yparam)
		popmessage("SRC:%04x %04x DST:%04x %04x V:%08x %08x",xsrc,ysrc,xdst,ydst,program_read_word(cop_register[2]),program_read_word(cop_register[3]));
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
	x_pl = program_read_word(cop_register[1]+0x8);
	x_en = program_read_word(cop_register[0]+0x8);
	dis = ((program_read_word(cop_register[0]+0x34) & 0xf0) >> 4);
	y_pl = program_read_word(cop_register[1]+0x4);
	y_en = program_read_word(cop_register[0]+0x4);

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

	program_write_word(cop_register[0]+0x36,dir);

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

	program_write_word(cop_register[0]+0x10,y_dis);
	program_write_word(cop_register[0]+0x14,x_dis);
}

/**/
static UINT16 cop2_hit_prot(void)
{
	static INT16 xsrc,xdst;
	static INT16 ysrc,ydst;
	static INT16 xp,yp;
	static INT16 param1,param2;
	static INT16 val;

	param1 = program_read_word(cop_register[2]);
	param2 = program_read_word(cop_register[3]);

	xsrc = program_read_word(cop_register[0]+0x8) + program_read_word(cop_register[0]+0x14);
	ysrc = program_read_word(cop_register[0]+0x4) + program_read_word(cop_register[0]+0x10);
	xdst = program_read_word(cop_register[1]+0x8) + program_read_word(cop_register[1]+0x14);
	ydst = program_read_word(cop_register[1]+0x4) + program_read_word(cop_register[1]+0x10);

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

	xsrc = program_read_word(cop_register[0]+0x14);
	ysrc = program_read_word(cop_register[0]+0x10);
	param2 = program_read_word(cop_register[3]);

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

	program_write_word(cop_register[0]+0x14,xsrc);
	program_write_word(cop_register[0]+0x10,ysrc);
}




/********************************************************************************************

  COPX bootleg simulation
    - Seibu Cup Soccer (bootleg)

 *******************************************************************************************/

READ16_HANDLER( copdxbl_0_r )
{
	UINT16 retvalue = cop_mcu_ram[offset];

	switch(offset)
	{
		default:
		{
			logerror("%06x: COPX unhandled read returning %04x from offset %04x\n", activecpu_get_pc(), retvalue, offset*2);
			return retvalue;
		}

		//case (0x47e/2):
		//case (0x5b0/2):
		//case (0x5b4/2):
		//  return cop_mcu_ram[offset];

		case (0x700/2): return input_port_1_word_r(machine,0,0);
		case (0x704/2):	return input_port_2_word_r(machine,0,0);
		case (0x708/2):	return input_port_4_word_r(machine,0,0);
		case (0x70c/2):	return input_port_3_word_r(machine,0,0);
		case (0x71c/2): return input_port_5_word_r(machine,0,0);
	}
}

WRITE16_HANDLER( copdxbl_0_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch(offset)
	{

		default:
		{
			logerror("%06x: COPX unhandled write data %04x at offset %04x\n", activecpu_get_pc(), data, offset*2);
			break;
		}

		/*********************************************************************
        400-5ff -  Protection writes
        *********************************************************************/


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
			//cop_reg_w(cop_mcu_ram[offset],offset & 0x000f, (offset < (0x4b0/2)) ? 1 : 0);
			break;
		/*layer clearance,but the bootleg doesn't send values,so this function
          is an original left-over.*/
		case (0x478/2):
		{
			/*
    AM_RANGE(0x100800, 0x100fff) AM_READWRITE(SMH_RAM,legionna_background_w) AM_BASE(&legionna_back_data)
    AM_RANGE(0x101000, 0x1017ff) AM_READWRITE(SMH_RAM,legionna_foreground_w) AM_BASE(&legionna_fore_data)
    AM_RANGE(0x101800, 0x101fff) AM_READWRITE(SMH_RAM,legionna_midground_w) AM_BASE(&legionna_mid_data)
    AM_RANGE(0x102000, 0x102fff) AM_READWRITE(SMH_RAM,legionna_text_w) AM_BASE(&legionna_textram)
            */
			break;
		}
		case (0x500/2):
		{
			//cop_fct = cop_mcu_ram[offset];
			//cop_run();
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
		case (0x660/2): { legionna_scrollram16[0] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x662/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x664/2): { legionna_scrollram16[2] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x666/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x668/2): { legionna_scrollram16[4] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x66a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }
		case (0x66c/2): { legionna_scrollram16[6] = cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x66e/2): { legionna_scrollram16[7] = cop_mcu_ram[offset]; break; }

		/*WRONG*/
		case (0x65c/2):
		{
			soundlatch_w(machine,1,data&0xff);
			cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE );
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
	}
}

// this still probably contains some useful information, but we should handle
// things as generically as possible
#if 0
/********************************************************************************************

  COPX-D2 simulation
    - Raiden 2
    - Zero Team

 *******************************************************************************************/


/* Raiden 2 COP2 handling.  Note, some important details about table upload in here that the
   other simulations are missing */

//  COPX functions, terribly incomplete

typedef struct _cop_state cop_state;
struct _cop_state
{
	UINT16		offset;						/* last write offset */
	UINT16		ram[0x200/2];				/* RAM from 0x400-0x5ff */

	UINT32		reg[4];						/* registers */

	UINT16		func_trigger[0x100/8];		/* function trigger */
	UINT16		func_value[0x100/8];		/* function value (?) */
	UINT16		func_mask[0x100/8];			/* function mask (?) */
	UINT16		program[0x100];				/* program "code" */
};

static cop_state cop_data;


#define VERBOSE 1
#define COP_LOG(x)	do { if (VERBOSE) logerror x; } while (0)



INLINE UINT16 cop_ram_r(cop_state *cop, UINT16 offset)
{
	return cop->ram[(offset - 0x400) / 2];
}

INLINE void cop_ram_w(cop_state *cop, UINT16 offset, UINT16 data)
{
	cop->ram[(offset - 0x400) / 2] = data;
}

INLINE UINT32 r32(offs_t address)
{
	return 	(program_read_word(address + 0) << 0) |
			(program_read_word(address + 2) << 16);
}

INLINE void w32(offs_t address, UINT32 data)
{
	program_write_word(address + 0, data >> 0);
	program_write_word(address + 2, data >> 16);
}


void cop_init(void)
{
	memset(&cop_data, 0, sizeof(cop_data));
}

WRITE16_HANDLER( raiden2_cop2_w )
{
	cop_state *cop = &cop_data;
	UINT32 temp32;
	UINT8 regnum;
	int func;

	/* all COP data writes are word-length (?) */
	data = COMBINE_DATA(&cop->ram[offset]);

	/* handle writes */
	switch (offset + (0x400/2))
	{
		/* ----- BCD conversion ----- */

		case 0x420/2:		/* LSW of number */
		case 0x422/2:		/* MSW of number */
			temp32 = cop_ram_r(cop, 0x420) | (cop_ram_r(cop, 0x422) << 16);
			cop_ram_w(cop, 0x590, ((temp32 / 1) % 10) + (((temp32 / 10) % 10) << 8) + 0x3030);
			cop_ram_w(cop, 0x592, ((temp32 / 100) % 10) + (((temp32 / 1000) % 10) << 8) + 0x3030);
			cop_ram_w(cop, 0x594, ((temp32 / 10000) % 10) + (((temp32 / 100000) % 10) << 8) + 0x3030);
			cop_ram_w(cop, 0x596, ((temp32 / 1000000) % 10) + (((temp32 / 10000000) % 10) << 8) + 0x3030);
			cop_ram_w(cop, 0x598, ((temp32 / 100000000) % 10) + (((temp32 / 1000000000) % 10) << 8) + 0x3030);
			break;

		/* ----- program upload registers ----- */

		case 0x432/2:		/* COP program data */
			COP_LOG(("%05X:COP Prog Data = %04X\n", activecpu_get_pc(), data));
			cop->program[cop_ram_r(cop, 0x434)] = data;
			break;

		case 0x434/2:		/* COP program address */
			COP_LOG(("%05X:COP Prog Addr = %04X\n", activecpu_get_pc(), data));
			assert((data & ~0xff) == 0);
			temp32 = (data & 0xff) / 8;
			cop->func_value[temp32] = cop_ram_r(cop, 0x438);
			cop->func_mask[temp32] = cop_ram_r(cop, 0x43a);
			cop->func_trigger[temp32] = cop_ram_r(cop, 0x43c);

			break;

		case 0x438/2:		/* COP program entry value (0,4,5,6,7,8,9,F) */
			COP_LOG(("%05X:COP Prog Val  = %04X\n", activecpu_get_pc(), data));
			break;

		case 0x43a/2:		/* COP program entry mask */
			COP_LOG(("%05X:COP Prog Mask = %04X\n", activecpu_get_pc(), data));
			break;

		case 0x43c/2:		/* COP program trigger value */
			COP_LOG(("%05X:COP Prog Trig = %04X\n", activecpu_get_pc(), data));
			break;

		/* ----- ???? ----- */

		case 0x47a/2:		/* clear RAM */
			if (cop_ram_r(cop, 0x47e) == 0x118)
			{
				UINT32 addr = cop_ram_r(cop, 0x478) << 6;
				int count = (cop_ram_r(cop, 0x47a) + 1) << 5;
				COP_LOG(("%05X:COP RAM clear from %05X to %05X\n", activecpu_get_pc(), addr, addr + count));
				while (count--)
					program_write_byte(addr++, 0);
			}
			else
			{
				COP_LOG(("%05X:COP Unknown RAM clear(%04X) = %04X\n", activecpu_get_pc(), cop_ram_r(cop, 0x47e), data));
			}
			break;

		/* ----- program data registers ----- */

		case 0x4a0/2:		/* COP register high word */
		case 0x4a2/2:		/* COP register high word */
		case 0x4a4/2:		/* COP register high word */
		case 0x4a6/2:		/* COP register high word */
			regnum = (offset) % 4;
			COP_LOG(("%05X:COP RegHi(%d) = %04X\n", activecpu_get_pc(), regnum, data));
			cop->reg[regnum] = (cop->reg[regnum] & 0x0000ffff) | (data << 16);
			break;

		case 0x4c0/2:		/* COP register low word */
		case 0x4c2/2:		/* COP register low word */
		case 0x4c4/2:		/* COP register low word */
		case 0x4c6/2:		/* COP register low word */
			regnum = (offset) % 4;
			COP_LOG(("%05X:COP RegLo(%d) = %04X\n", activecpu_get_pc(), regnum, data));
			cop->reg[regnum] = (cop->reg[regnum] & 0xffff0000) | data;
			break;

		/* ----- program trigger register ----- */

		case 0x500/2:		/* COP trigger */
			COP_LOG(("%05X:COP Trigger = %04X\n", activecpu_get_pc(), data));
			for (func = 0; func < ARRAY_LENGTH(cop->func_trigger); func++)
				if (cop->func_trigger[func] == data)
				{
					int offs;

					COP_LOG(("  Execute:"));
					for (offs = 0; offs < 8; offs++)
					{
						if (cop->program[func * 8 + offs] == 0)
							break;
						COP_LOG((" %04X", cop->program[func * 8 + offs]));
					}
					COP_LOG(("\n"));

					/* special cases for now */
					if (data == 0x5205 || data == 0x5a05)
					{
						COP_LOG(("  Copy 32 bits from %05X to %05X\n", cop->reg[0], cop->reg[1]));
						w32(cop->reg[1], r32(cop->reg[0]));
					}
					else if (data == 0xf205)
					{
						COP_LOG(("  Copy 32 bits from %05X to %05X\n", cop->reg[0] + 4, cop->reg[1]));
						w32(cop->reg[2], r32(cop->reg[0] + 4));
					}
					break;
				}
			logerror("%05X:COP Warning - can't find command - func != ARRAY_LENGTH(cop->func_trigger)\n",  activecpu_get_pc());
			break;

		/* ----- other stuff ----- */

		default:		/* unknown */
			COP_LOG(("%05X:COP Unknown(%04X) = %04X\n", activecpu_get_pc(), offset*2 + 0x400, data));
			break;
	}
}


READ16_HANDLER( raiden2_cop2_r )
{
	cop_state *cop = &cop_data;
	COP_LOG(("%05X:COP Read(%04X) = %04X\n", activecpu_get_pc(), offset*2 + 0x400, cop->ram[offset]));
	return cop->ram[offset];
}
#endif





/* Generic COP functions
  -- the game specific handlers fall through to these if there
     isn't a specific case for them.  these implement behavior
     which seems common to all the agmes
*/

static READ16_HANDLER( generic_cop_r )
{
	UINT16 retvalue;
	retvalue = cop_mcu_ram[offset];


	switch (offset)
	{
		default:
			seibu_cop_log("%06x: COPX unhandled read returning %04x from offset %04x\n", activecpu_get_pc(), retvalue, offset*2);
			return retvalue;
			break;

		/* BCD protection reads */
		case (0x190/2): { return ((prot_bcd[0] & 0x0000ffff) >> 0 ) + 0x3030; }
		case (0x192/2): { return ((prot_bcd[0] & 0xffff0000) >> 16) + 0x3030; }
		case (0x194/2): { return ((prot_bcd[1] & 0x0000ffff) >> 0 ) + 0x3030; }
		case (0x196/2): { return ((prot_bcd[1] & 0xffff0000) >> 16) + 0x3030; }
		case (0x198/2):	{ return ((prot_bcd[2] & 0x0000ffff) >> 0 ) + 0x3030; }
		case (0x19a/2): { return ((prot_bcd[2] & 0xffff0000) >> 16) + 0x3030; }
		case (0x19c/2): { return 0x3030; }


	}
}

static WRITE16_HANDLER( generic_cop_w )
{
	switch (offset)
	{
		default:
			seibu_cop_log("%06x: COPX unhandled write data %04x at offset %04x\n", activecpu_get_pc(), data, offset*2);
			break;

		/* BCD Protection */
		case (0x020/2):	{ prot_bcd[0] = protection_bcd_jsr(cop_mcu_ram[offset]); break; }
		case (0x022/2): { prot_bcd[1] = protection_bcd_jsr(cop_mcu_ram[offset]); break; }
		case (0x024/2): { prot_bcd[2] = protection_bcd_jsr(cop_mcu_ram[offset]); break; }

		/* Command tables for 0x500 / 0x502 commands */
		case (0x032/2): { copd2_set_tabledata(data, machine); break; }
		case (0x034/2): { copd2_set_tableoffset(data, machine); break; }
		case (0x038/2):	{ cop_438 = data; break; }
		case (0x03a/2):	{ cop_43a = data; break; }
		case (0x03c/2): { cop_43c = data; break; }

		/* Layer Clearing */
		case (0x078/2): /* clear address */
		{
			cop_clearfill_address[cop_clearfill_lasttrigger] = data; // << 6 to get actual address
			seibu_cop_log("%06x: COPX set layer clear address to %04x (actual %08x)\n", activecpu_get_pc(), data, data<<6);
			break;
		}

		case (0x07a/2): /* clear length */
		{
			cop_clearfill_length[cop_clearfill_lasttrigger] = data;
			seibu_cop_log("%06x: COPX set layer clear length to %04x (actual %08x)\n", activecpu_get_pc(), data, data<<5);

			break;
		}

		case (0x07c/2): /* clear value? */
		{
			cop_clearfill_value[cop_clearfill_lasttrigger] = data;
			seibu_cop_log("%06x: COPX set layer clear value to %04x (actual %08x)\n", activecpu_get_pc(), data, data<<6);
			break;
		}

		/* unknown, related to clears? / DMA? */
		case (0x07e/2):
		{
			cop_clearfill_lasttrigger = data;
			seibu_cop_log("%06x: COPX set layer clear trigger? to %04x\n", activecpu_get_pc(), data);
			if (data>=0x1ff)
			{
				seibu_cop_log("invalid!, >0x1ff\n");
				cop_clearfill_lasttrigger = 0;
			}

			break;
		}

		/* Registers */
		case (0x0a0/2): { cop_register[0] = (cop_register[0]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c0/2): { cop_register[0] = (cop_register[0]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0a2/2): { cop_register[1] = (cop_register[1]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c2/2): { cop_register[1] = (cop_register[1]&0xffff0000)|(cop_mcu_ram[offset]<<0);   break; }

		case (0x0a4/2): { cop_register[2] = (cop_register[2]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c4/2): { cop_register[2] = (cop_register[2]&0xffff0000)|(cop_mcu_ram[offset]<<0);  break; }

		case (0x0a6/2): { cop_register[3] = (cop_register[3]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c6/2): { cop_register[3] = (cop_register[3]&0xffff0000)|(cop_mcu_ram[offset]<<0);   break; }

		/* was dma_dst */
		case (0x0a8/2): { cop_register[4] = (cop_register[4]&0x0000ffff)|(cop_mcu_ram[offset]<<16); break; }
		case (0x0c8/2): { cop_register[4] = (cop_register[4]&0xffff0000)|(cop_mcu_ram[offset]<<0);   break; }

		case (0x100/2):
		{
			int i;
			int command;

			seibu_cop_log("%06x: COPX execute table macro command %04x %04x | regs %08x %08x %08x %08x %08x\n", activecpu_get_pc(), data, cop_mcu_ram[offset], cop_register[0], cop_register[1], cop_register[2], cop_register[3], cop_register[4]);

			command = -1;
			/* search the uploaded 'trigger' table for a matching trigger*/
			/* note, I don't know what the 'mask' or 'value' tables are... probably important, might determine what actually gets executed! */
			for (i=0;i<32;i++)
			{
				if (cop_mcu_ram[offset]==copd2_table_4[i])
				{
					seibu_cop_log("    Cop Command %04x found in slot %02x with other params %04x %04x\n", cop_mcu_ram[offset], i, copd2_table_2[i], copd2_table_3[i]);
					command = i;
				}
			}

			if (command==-1)
			{
				seibu_cop_log("    Cop Command %04x NOT IN TABLE!\n", cop_mcu_ram[offset]);
				break;
			}
			else
			{
				int j;
				command*=0x8;
				seibu_cop_log("     Sequence: ");
				for (j=0;j<0x8;j++)
				{
					seibu_cop_log("%04x ", copd2_table[command+j]);
				}
				seibu_cop_log("\n");
			}


			break;
		}


		/* hmm, this would be strange the 6xx range should be video regs?? */
		case (0x2fc/2):
		{
			seibu_cop_log("%06x: COPX execute current layer clear??? %04x\n", activecpu_get_pc(), data);

			// I think the value it writes here must match the other value for anything to happen.. maybe */
			//if (data!=cop_clearfill_value[cop_clearfill_lasttrigger]) break;
			if ((cop_clearfill_lasttrigger==0x14) || (cop_clearfill_lasttrigger==0x15)) return;

			/* do the fill  */
			if (cop_clearfill_value[cop_clearfill_lasttrigger]==0x0000)
			{
				UINT32 length, address;
				int i;
				address = cop_clearfill_address[cop_clearfill_lasttrigger] << 6;
				length = (cop_clearfill_length[cop_clearfill_lasttrigger]+1) << 5;

				for (i=address;i<address+length;i+=2)
				{
					program_write_word(i, 0x0000);
				}
			}
			break;
		}
	}
}

/**********************************************************************************************
  Heated Barrel
**********************************************************************************************/

READ16_HANDLER( heatbrl_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(machine, offset, mem_mask);
			break;

	    /*********************************************************************
        400-5ff -  Protection reads
        *********************************************************************/

		case (0x180/2):	{ return xy_check; } /*hit protection*/
		case (0x182/2):	{ if(input_code_pressed(KEYCODE_X)) { return 0; } else { return 3; } } /*---- ---- ---- --xx used bits*/
		case (0x184/2):	{ if(input_code_pressed(KEYCODE_C)) { return 0; } else { return 3; } } /*---- ---- ---- --xx used bits*/

	    case (0x1b0/2): return (0xffff); /* bit 15 is branched on a few times in the $1938 area */
		case (0x1b4/2):	return (0xffff); /* read at $1932 and stored in ram before +0x5b0 bit 15 tested */

		/*********************************************************************
        700-7ff - Non-protection reads
        *********************************************************************/

		/* Seibu Sound System */
		case (0x3c8/2):	return seibu_main_word_r(machine,2,0);
		case (0x3cc/2):	return seibu_main_word_r(machine,3,0);
		case (0x3d4/2): return seibu_main_word_r(machine,5,0);

		/* Inputs */
		case (0x340/2): return input_port_1_word_r(machine,0,0);
		case (0x344/2):	return input_port_2_word_r(machine,0,0);
		case (0x348/2): return input_port_4_word_r(machine,0,0);
		case (0x34c/2): return input_port_3_word_r(machine,0,0);

	}
}

WRITE16_HANDLER( heatbrl_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(machine, offset, data, mem_mask);
			break;

		/*********************************************************************
        400-5ff -  Protection writes
        *********************************************************************/

		/* Odd, this is a video register */
		case (0x070/2): { heatbrl_setgfxbank( cop_mcu_ram[offset] ); break; }

#if 1 // turn off to get the generic sequence logging
		/* Macros Command Trigger */
		case (0x100/2):
		{
			switch(cop_mcu_ram[offset])
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
					seibu_cop_log("DMA CMD 0x500 with parameter = %04x PC = %08x\n",cop_mcu_ram[offset],activecpu_get_previouspc());
			}
			break;
		}
#endif

		/*********************************************************************
        600-6ff - Video Registers
        *********************************************************************/

		// 65a bit 0 is flipscreen
		case (0x25c/2): { legionna_layer_disable = cop_mcu_ram[offset]; break; } // 65c probably layer disables, like Dcon? Used on screen when you press P1-4 start (values 13, 11, 0 seen)
		// 660 - 66a scroll control;  is there a layer priority switch...?
		case (0x260/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x262/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x264/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x266/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x268/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x26a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		/*********************************************************************
        700-7ff - Output (Seibu Sound System)
        *********************************************************************/

		case (0x3c0/2):	{ seibu_main_word_w(machine,0,cop_mcu_ram[offset],0xff00); break; }
		case (0x3c4/2):	{ seibu_main_word_w(machine,1,cop_mcu_ram[offset],0xff00); break; }
		case (0x3d0/2):	{ seibu_main_word_w(machine,4,cop_mcu_ram[offset],0xff00); break; }
		case (0x3d8/2):	{ seibu_main_word_w(machine,6,cop_mcu_ram[offset],0xff00); break; }
	}
}


/**********************************************************************************************
  Seibu Cup Soccer
**********************************************************************************************/

READ16_HANDLER( cupsoc_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(machine, offset, mem_mask);
			break;

		//case (0x07e/2):
		//case (0x1b0/2):
		//case (0x1b4/2):
		//  return cop_mcu_ram[offset];

		/* returning 0xffff for some inputs for now, breaks coinage but
		   allows cupsoc to boot */
		case (0x300/2): return input_port_1_word_r(machine,0,0);
		case (0x304/2): return input_port_2_word_r(machine,0,0);
		case (0x308/2): return input_port_4_word_r(machine,0,0);
		case (0x30c/2): return input_port_3_word_r(machine,0,0);
		case (0x314/2): return 0xffff;
		case (0x31c/2): return input_port_5_word_r(machine,0,0);

		case (0x340/2): return 0xffff;
		case (0x344/2): return 0xffff;
		case (0x348/2):	return 0xffff;//seibu_main_word_r(machine,2,0);
		case (0x34c/2): return 0xffff;//seibu_main_word_r(machine,3,0);
		case (0x354/2): return 0xffff;//seibu_main_word_r(machine,5,0);
		case (0x35c/2): return 0xffff;
	}
}

WRITE16_HANDLER( cupsoc_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(machine, offset, data, mem_mask);
			break;

		/*********************************************************************
        400-5ff -  Protection writes
        *********************************************************************/

		/* Trigger Macro Command */
		case (0x100/2):
		{
			switch(cop_mcu_ram[offset])
			{
				/*???*/
				case 0x8100:
				{
					UINT32 src = cop_register[0];
					program_write_word(src+0x36,0xffc0);
					break;
				}
				case 0x8900:
				{
					UINT32 src = cop_register[0];
					program_write_word(src+0x36,0xff80);
					break;
				}
				/*Right*/
				case 0x0205:
				{
					UINT32 src = cop_register[0];
					INT16 y = program_read_word(src+0x4);
					INT16 x = program_read_word(src+0x8);
					INT16 y_rel = program_read_word(src+0x10);
					INT16 x_rel = program_read_word(src+0x14);
					program_write_word(src+0x4,(y+y_rel));
					program_write_word(src+0x8,(x+x_rel));
					/*logerror("%08x %08x %08x %08x %08x\n",cop_register[0],
												   program_read_word(cop_reg[0]+0x4),
												   program_read_word(cop_reg[0]+0x8),
												   program_read_word(cop_reg[0]+0x10),
												   program_read_word(cop_reg[0]+0x14));*/
					break;
				}
				/*???*/
				case 0x3bb0:
				{
					//UINT32 dst = cop_register[0];
					//UINT32 dst = cop_register[1];
					//program_write_word(dst,  mame_rand(Machine)/*program_read_word(src)*/);
					//program_write_word(dst+2,mame_rand(Machine)/*program_read_word(src+2)*/);
					//program_write_word(dst+4,mame_rand(Machine)/*program_read_word(src+4)*/);
					//program_write_word(dst+6,mame_rand(Machine)/*program_read_word(src+6)*/);
					//logerror("%04x\n",cop_register[0]);
					break;
				}
				default:
					//logerror("%04x\n",data);
					break;
			}
			break;
		}

		/* Video Regs */
		case (0x204/2):
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
		case (0x22c/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x22e/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x230/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x232/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x234/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x236/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }
		case (0x238/2): { legionna_scrollram16[6] = cop_mcu_ram[offset]; break; }
		case (0x23a/2): { legionna_scrollram16[7] = cop_mcu_ram[offset]; break; }

		case (0x340/2):	{ seibu_main_word_w(machine,0,cop_mcu_ram[offset],0xff00); break; }
		case (0x344/2):	{ seibu_main_word_w(machine,1,cop_mcu_ram[offset],0xff00); break; }
		case (0x350/2):	{ seibu_main_word_w(machine,4,cop_mcu_ram[offset],0xff00); break; }
		case (0x358/2):	{ seibu_main_word_w(machine,6,cop_mcu_ram[offset],0xff00); break; }
	}
}

/**********************************************************************************************
  Godzilla
**********************************************************************************************/

READ16_HANDLER( godzilla_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(machine, offset, mem_mask);
			break;

		/* Non-protection reads */
		case (0x3c8/2):	return seibu_main_word_r(machine,2,0);
		case (0x3cc/2):	return seibu_main_word_r(machine,3,0);
		case (0x3d4/2):	return seibu_main_word_r(machine,5,0);

		/* Inputs */
		case (0x340/2): return input_port_1_word_r(machine,0,0);
		case (0x344/2): return input_port_2_word_r(machine,0,0);
		case (0x348/2): return input_port_4_word_r(machine,0,0);
		case (0x34c/2): return input_port_3_word_r(machine,0,0);
	}
}

WRITE16_HANDLER( godzilla_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(machine, offset, data, mem_mask);
			break;


		case (0x220/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x222/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x224/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x226/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x228/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x22a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		case (0x3c0/2):	{ seibu_main_word_w(machine,0,cop_mcu_ram[offset],0xff00); break; }
		case (0x3c4/2):	{ seibu_main_word_w(machine,1,cop_mcu_ram[offset],0xff00); break; }
		case (0x3d0/2):	{ seibu_main_word_w(machine,4,cop_mcu_ram[offset],0xff00); break; }
		case (0x3d8/2):	{ seibu_main_word_w(machine,6,cop_mcu_ram[offset],0xff00); break; }
	}
}

/**********************************************************************************************
  Denjin Makai
**********************************************************************************************/

READ16_HANDLER( denjinmk_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(machine, offset, mem_mask);
			break;

		/* Non-protection reads */

		case (0x308/2):	return seibu_main_word_r(machine,2,0);
		case (0x30c/2):	return seibu_main_word_r(machine,3,0);
		case (0x314/2): return seibu_main_word_r(machine,5,0);

		/* Inputs */
		case (0x340/2): return input_port_1_word_r(machine,0,0);
		case (0x344/2):	return input_port_2_word_r(machine,0,0);
		case (0x348/2): return input_port_4_word_r(machine,0,0);
		case (0x34c/2): return input_port_3_word_r(machine,0,0);
		case (0x35c/2): return input_port_5_word_r(machine,0,0);
	}
}

WRITE16_HANDLER( denjinmk_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(machine, offset, data, mem_mask);
			break;

		case (0x220/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x222/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x224/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x226/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x228/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x22a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		case (0x300/2):	{ seibu_main_word_w(machine,0,cop_mcu_ram[offset],0xff00); break; }
		case (0x304/2):	{ seibu_main_word_w(machine,1,cop_mcu_ram[offset],0xff00); break; }
		case (0x310/2):	{ seibu_main_word_w(machine,4,cop_mcu_ram[offset],0xff00); break; }
		case (0x318/2):	{ seibu_main_word_w(machine,6,cop_mcu_ram[offset],0xff00); break; }

	}
}

/**********************************************************************************************
  SD Gundam Rainbow Trout
**********************************************************************************************/

READ16_HANDLER( sdgndmrb_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(machine, offset, mem_mask);
			break;

		/*hit protection*/
		case (0x180/2): { return xy_check; }

		case (0x1b0/2):
			return 2;
			/*check if the DMA has been finished*/
			if(dma_status == 1)
			{
				dma_status = 0;
				return 2;
			}
			return cop_mcu_ram[offset];

		/* Non-protection reads */
		case (0x308/2): return seibu_main_word_r(machine,2,0);
		case (0x30c/2): return seibu_main_word_r(machine,3,0);
		case (0x314/2): return seibu_main_word_r(machine,5,0);

		/* Inputs */
		case (0x340/2): return input_port_1_word_r(machine,0,0);
		case (0x344/2):	return input_port_2_word_r(machine,0,0);
		case (0x348/2): return input_port_4_word_r(machine,0,0);
		case (0x34c/2): return input_port_3_word_r(machine,0,0);
		case (0x35c/2): return input_port_5_word_r(machine,0,0);
	}
}


WRITE16_HANDLER( sdgndmrb_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(machine, offset, data, mem_mask);
			break;

		/*********************************************************************
        400-5ff -  Protection writes
        *********************************************************************/

		case (0x00c/2): { dma_size = cop_mcu_ram[offset]; break; }

		/*DMA source address*/
		case (0x012/2): { prot_data[1] = cop_mcu_ram[offset]; dma_src = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16); break; }
		case (0x014/2): { prot_data[0] = cop_mcu_ram[offset]; dma_src = (prot_data[0]&0xffff)|((prot_data[1]&0xffff)<<16); break; }

		/* Execute Macro Command */
		case (0x100/2):
		{
			switch(cop_mcu_ram[offset])
			{
				case 0xa180:/*do the job [1]*/
				{
					//popmessage("%08x %08x %04x",dma_src,cop_register[5]+4,dma_size);
					/*fix the offset for easier reading*/
					dma_src+=4;
					//cop_register[5]+=4;
					s_i = dma_size;
					//cop_register[5]+=((program_read_word(0x110000) & 0x000f) * 8);
					//program_write_word(0x1004c8,cop_register[5] & 0xffff);
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
					seibu_cop_log("DMA CMD 0x500 with parameter = %04x PC = %08x\n",cop_mcu_ram[offset],activecpu_get_previouspc());
			}
			break;
		}

		case (0x102/2):
		{
			if(cop_mcu_ram[offset] == 0xc480)
			{
				dma_transfer();
				s_i--;
				if(s_i == 0)
					dma_status = 1;
			}
			break;
		}

		/*Layer Enable,bit wise active low*/
		case (0x21c/2):
		{
			/*
            ---x ---- (used in test mode)
            ---- x--- Text Layer
            ---- -x-- Foreground Layer
            ---- --x- Midground Layer
            ---- ---x Background Layer
            */
			sdgndmrb_pri_n = cop_mcu_ram[offset] & 0xf;
			break;
		}

		/* TODO: tilemaps x-axis are offset,we use a temporary kludge for now */
		case (0x220/2):	{ legionna_scrollram16[0] = 0x10 + cop_mcu_ram[offset]; break; }
		case (0x222/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x224/2): { legionna_scrollram16[2] = 0x10 + cop_mcu_ram[offset]; break; }
		case (0x226/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x228/2): { legionna_scrollram16[4] = 0x10 + cop_mcu_ram[offset]; break; }
		case (0x22a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		/* scroll mirrors? */
		case (0x22c/2):
		case (0x22e/2):
		case (0x230/2):
		case (0x232/2):
		case (0x234/2):
		case (0x236/2):
			break;


		/* Text Layer scroll registers */
		case (0x238/2): { legionna_scrollram16[6] = 0x38 + cop_mcu_ram[offset]; break; }
		case (0x23a/2): { legionna_scrollram16[7] = cop_mcu_ram[offset]; break; }
		/*C.R.T. Controller (note:game calls it OBJ register)*/
		case (0x244/2):
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
		case (0x27c/2): break;
		case (0x280/2): break;
		//case (0x6fc/2): break;

		case (0x300/2):	{ seibu_main_word_w(machine,0,cop_mcu_ram[offset],0xff00); break; }
		case (0x304/2):	{ seibu_main_word_w(machine,1,cop_mcu_ram[offset],0xff00); break; }
		case (0x310/2):	{ seibu_main_word_w(machine,4,cop_mcu_ram[offset],0xff00); break; }
		case (0x318/2):	{ seibu_main_word_w(machine,6,cop_mcu_ram[offset],0xff00); break; }
	}
}

/**********************************************************************************************
  Legionnaire
**********************************************************************************************/


READ16_HANDLER( legionna_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(machine, offset, mem_mask);
			break;

		/*********************************************************************
        400-5ff -  Protection reads
        *********************************************************************/

		case (0x070/2):	return (mame_rand(machine) &0xffff); /* read PC $110a, could be some sort of control word:  sometimes a bit is changed then it's poked back in... */
		case (0x182/2):	return (0); /* read PC $3594 */
		case (0x184/2):	return (0); /* read PC $3588 */
		case (0x186/2):	return (0); /* read PC $35a0 */
		case (0x188/2):	return hit_check; /* read PC $3580 */
		case (0x1b0/2):	return (0); /* bit 15 is branched on a few times in the $3300 area */
		case (0x1b4/2):	return (0); /* read and stored in ram before +0x5b0 bit 15 tested */


		/*********************************************************************
        700-7ff - Non-protection reads
        *********************************************************************/

		/* Seibu Sound System */
		case (0x308/2):	return seibu_main_word_r(machine,2,0);
		case (0x30c/2):	return seibu_main_word_r(machine,3,0);
		case (0x314/2): return seibu_main_word_r(machine,5,0);

		/* Inputs */
		case (0x340/2): return input_port_1_word_r(machine,0,0);
		case (0x344/2):	return input_port_2_word_r(machine,0,0);
		case (0x348/2):	return input_port_0_word_r(machine,0,0);
		case (0x34c/2):	return input_port_3_word_r(machine,0,0);

	}
}


WRITE16_HANDLER( legionna_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(machine, offset, data, mem_mask);
			break;

		/* Execute Macro command from table */
		case (0x100/2):
		{
			/*Movement protection*/
			if(cop_mcu_ram[offset] == 0x8900 || cop_mcu_ram[offset] == 0x0205)
			{
				static UINT16 xy_data[2];
				static UINT8 k;
				xy_data[0] = program_read_word(cop_register[2]);
				xy_data[1] = program_read_word(cop_register[3]);
				k = (cop_mcu_ram[offset] == 0x0205) ? ENEMY : PLAYER;
				protection_move_jsr(cop_register[0],k);
				//protection_move_jsr(cop_register[1]); //???
				//popmessage("%08x %08x %04x %04x",cop_register[0],cop_register[1],xy_data[0],xy_data[1]);
			}
			else if(cop_mcu_ram[offset] == 0x3bb0 || cop_mcu_ram[offset] == 0x138e)
			{
				protection_hit_jsr(cop_register[0],cop_register[1]);
			}
			break;
		}

		/*********************************************************************
        600-6ff - Video Registers
        *********************************************************************/

		// 61a bit 0 is flipscreen
		// 61c probably layer disables, like Dcon

		case (0x220/2): { legionna_scrollram16[0] = cop_mcu_ram[offset]; break; }
		case (0x222/2): { legionna_scrollram16[1] = cop_mcu_ram[offset]; break; }
		case (0x224/2): { legionna_scrollram16[2] = cop_mcu_ram[offset]; break; }
		case (0x226/2): { legionna_scrollram16[3] = cop_mcu_ram[offset]; break; }
		case (0x228/2): { legionna_scrollram16[4] = cop_mcu_ram[offset]; break; }
		case (0x22a/2): { legionna_scrollram16[5] = cop_mcu_ram[offset]; break; }

		/*********************************************************************
        700-7ff - Output (Seibu Sound System)
        *********************************************************************/

		case (0x300/2):	{ seibu_main_word_w(machine,0,cop_mcu_ram[offset],0xff00); break; }
		case (0x304/2):	{ seibu_main_word_w(machine,1,cop_mcu_ram[offset],0xff00); break; }
		case (0x310/2):	{ seibu_main_word_w(machine,4,cop_mcu_ram[offset],0xff00); break; }
		case (0x318/2):	{ seibu_main_word_w(machine,6,cop_mcu_ram[offset],0xff00); break; }
	}
}


/**********************************************************************************************
  Raiden 2 / Zero Team
**********************************************************************************************/

READ16_HANDLER( raiden2_mcu_r )
{
	switch (offset)
	{
		default:
			return generic_cop_r(machine, offset, mem_mask);
			break;

		case (0x340/2): return input_port_read_indexed(machine, 2) | (input_port_read_indexed(machine, 3) << 8);
		case (0x344/2): return input_port_read_indexed(machine, 0) | (input_port_read_indexed(machine, 1) << 8);
		case (0x34c/2): return input_port_read_indexed(machine, 4) | 0xff00;


	}
}

extern WRITE16_HANDLER( sprcpt_val_1_w );
extern WRITE16_HANDLER( sprcpt_val_2_w );
extern WRITE16_HANDLER( sprcpt_data_1_w );
extern WRITE16_HANDLER( sprcpt_data_2_w );
extern WRITE16_HANDLER( sprcpt_data_3_w );
extern WRITE16_HANDLER( sprcpt_data_4_w );
extern WRITE16_HANDLER( sprcpt_adr_w );
extern WRITE16_HANDLER( sprcpt_flags_1_w );
extern WRITE16_HANDLER( sprcpt_flags_2_w );

WRITE16_HANDLER( raiden2_mcu_w )
{
	COMBINE_DATA(&cop_mcu_ram[offset]);

	switch (offset)
	{
		default:
			generic_cop_w(machine, offset, data, mem_mask);
			break;

		case (0x2a0/2): sprcpt_val_1_w(machine,offset,data,mem_mask); break;
		case (0x2a2/2): sprcpt_val_1_w(machine,offset,data,mem_mask); break;
		case (0x2a4/2): sprcpt_data_3_w(machine,offset,data,mem_mask); break;
		case (0x2a6/2): sprcpt_data_3_w(machine,offset,data,mem_mask); break;
		case (0x2a8/2): sprcpt_data_4_w(machine,offset,data,mem_mask); break;
		case (0x2aa/2): sprcpt_data_4_w(machine,offset,data,mem_mask); break;
		case (0x2ac/2): sprcpt_flags_1_w(machine,offset,data,mem_mask); break;
		case (0x2ae/2): sprcpt_flags_1_w(machine,offset,data,mem_mask); break;
		case (0x2b0/2): sprcpt_data_1_w(machine,offset,data,mem_mask); break;
		case (0x2b2/2): sprcpt_data_1_w(machine,offset,data,mem_mask); break;
		case (0x2b4/2): sprcpt_data_2_w(machine,offset,data,mem_mask); break;
		case (0x2b6/2): sprcpt_data_2_w(machine,offset,data,mem_mask); break;
		case (0x2b8/2): sprcpt_val_2_w(machine,offset,data,mem_mask); break;
		case (0x2ba/2): sprcpt_val_2_w(machine,offset,data,mem_mask); break;
		case (0x2bc/2): sprcpt_adr_w(machine,offset,data,mem_mask); break;
		case (0x2be/2): sprcpt_adr_w(machine,offset,data,mem_mask); break;
		case (0x2ce/2): sprcpt_flags_2_w(machine,offset,data,mem_mask); break;

	}
}


