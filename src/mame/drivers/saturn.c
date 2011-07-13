/**************************************************************************************************

    Sega Saturn & Sega ST-V (Sega Titan Video) HW (c) 1994 Sega

    Driver by David Haywood, Angelo Salese, Olivier Galibert & Mariusz Wojcieszek
    SCSP driver provided by R.Belmont, based on ElSemi's SCSP sound chip emulator
    CD Block driver provided by ANY, based on sthief original emulator
    Many thanks to Guru, Fabien, Runik and Charles MacDonald for the help given.

===================================================================================================

Notes:
-To enter into an Advanced Test Mode,keep pressed the Test Button (F2) on the start-up.
-Memo: Some tests done on the original & working PCB,to be implemented:
 -The AD-Stick returns 0x00 or a similar value.
 -The Ports E,F & G must return 0xff
 -The regular BIOS tests (Memory Test) changes his background color at some point to
  several gradients of red,green and blue.Current implementation remains black.I dunno
  if this is a framebuffer write or a funky transparency issue (i.e TRANSPARENT_NONE
  should instead show the back layer).
 -RBG0 rotating can be checked on the "Advanced Test" menu thru the VDP1/VDP2 check.
  It rotates clockwise IIRC.Also the ST-V logo when the game is in Multi mode rotates too.
 -The MIDI communication check fails even on a ST-V board,somebody needs to check if there
  is a MIDI port on the real PCB...

TODO:
(Main issues)
- decap the SH-1, used for CD block (needed especially for Sega Saturn)
- IRQs: some games have some issues with timing accurate IRQs,check/fix all of them.
- The Cart-Dev mode hangs even with the -dev bios,I would like to see what it does on the real HW.
- IC13 games on the bios dev doesn't even load the cartridge / crashes the emulation at start-up,
  rom rearrange needed?
- finish the DSP core.
- Add the RS232c interface (serial port),needed by fhboxers.
- (PCB owners) check if the clocks documented in the manuals are really right for ST-V.
- We need to check every game if can be completed or there are any hanging/crash/protection
  issues on them.
- Clean-ups and split the various chips(SCU,SMPC) into their respective files (in progress).
- Video emulation bugs: check stvvdp2.c file.
- Reimplement the idle skip if possible.
- clean up the I/Os, by using per-game specific mapped ports and rewrite it by using 16-bit trampolines
- Properly emulate the protection chips, used by several games (check stvprot.c for more info)

(per-game issues)
- stress: accesses the Sound Memory Expansion Area (0x05a00000-0x05afffff), unknown purpose;
- smleague / finlarch: it randomly hangs / crashes,it works if you use a ridiculous MCFG_INTERLEAVE number,might need strict
  SH-2 synching.
- suikoenb/shanhigw + others: why do we get 2 credits on startup? Cause might be by a communication with the M68k
- bakubaku: sound part is largely incomplete,caused by a tight loop at location PC=1048 of the sound cpu part.
- myfairld: Apparently this game gives a black screen (either test mode and in-game mode),but let it wait for about
  10 seconds and the game will load everything. This is because of a hellishly slow m68k sub-routine located at 54c2.
  Likely to not be a bug but an in-game design issue.
- myfairld: Micronet programmers had the "great" idea to *not* use the ST-V input standards,infact
  joystick panel is mapped with input_port(10) instead of input_port(2),so for now use
  the mahjong panel instead. Also the REACH and RON buttons are actually reversed.
- danchih: hanafuda panel doesn't work.
- findlove: controls doesn't work? Playing with the debugger at location $6063720 it makes it get furter,but controls
  still doesn't work,missing irq?
- batmanfr: Missing sound,caused by an extra ADSP chip which is on the cart.The CPU is a
  ADSP-2181,and it's the same used by NBA Jam Extreme (ZN game).
- vfremix: when you play Akira, there is a problem with third match: game doesn't upload all textures
  and tiles and doesn't enable display, although gameplay is normal - wait a while to get back
  to title screen after losing a match

===================================================================================================

Hardware overview:
------------------
-two SH-2 CPUs,in a master/slave configuration.The master cpu is used to
boot-up and to do the most of the work,the slave one does extra work that could be
too much for a single cpu.They both shares all the existant devices;
-a M68000 CPU,used to drive sound(the SCSP chip).The program is uploaded via the
SH-2 cpus;
-a SMPC (System Manager & Peripheral Control),used to drive all the
devices on the board;
-a SCU (System Control Unit),mainly used to do DMA operations and to drive interrupts,it
also has a DSP;
-an (optional for the ST-V) SH-1 CPU,used to be the CD driver;
-An A-Bus,where the cart ROM area is located;
-A B-Bus,where the Video Hardware & the SCU sections are located;
-Two VDPs chips(named as 1 & 2),used for the video section:
 -VDP1 is used to render sprites & polygons.
 -VDP2 is for the tilemap system,there are:
 4 effective normal layers;
 2 roz layers;
 1 back layer;
 1 line layer;
 The VDP2 is capable of the following things (in order):
 -dynamic resolution (up to 704x512) & various interlacing modes;
 -mosaic process;
 -scrolling,scaling,horizontal & vertical cell scrolling & linescroll for the
  normal planes, the roz ones can also rotate;
 -versatile window system,used for various effects;
 -alpha-blending,refered as Color Calculation in the docs;
 -shadow effects;
 -global rgb brightness control,separate for every plane;

****************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"
#include "cpu/sh2/sh2.h"
#include "machine/stvcd.h"
#include "machine/scudsp.h"
#include "sound/scsp.h"
#include "sound/cdda.h"
#include "machine/stvprot.h"
#include "machine/smpc.h"
#include "includes/stv.h"
#include "imagedev/chd_cd.h"
#include "imagedev/cartslot.h"
#include "coreutil.h"


/* TODO: do this in a verboselog style */
#define LOG_CDB  0
#define LOG_SCU  1
#define LOG_IRQ  0
#define LOG_IOGA 0

static int DectoBCD(int num)
{
	int i, cnt = 0, tmp, res = 0;

	while (num > 0) {
		tmp = num;
		while (tmp >= 10) tmp %= 10;
		for (i=0; i<cnt; i++)
			tmp *= 16;
		res += tmp;
		cnt++;
		num /= 10;
	}

	return res;
}


/**************************************************************************************/
/*to be added into a stv Header file,remember to remove all the static...*/

//static void stv_dump_ram(void);

/*VDP2 stuff*/
/*SMPC stuff*/
/*SCU stuff*/

static void scu_do_transfer(running_machine &machine,UINT8 event);
static void scu_dma_direct(address_space *space, UINT8 dma_ch);	/*DMA level 0 direct transfer function*/
static void scu_dma_indirect(address_space *space, UINT8 dma_ch); /*DMA level 0 indirect transfer function*/



//static int scanline;

/*A-Bus IRQ checks,where they could be located these?*/
#define ABUSIRQ(_irq_,_vector_) \
	{ cputag_set_input_line_and_vector(device->machine(), "maincpu", _irq_, HOLD_LINE , _vector_); }
#if 0
if(state->m_scu_regs[42] & 1)//IRQ ACK
{
	ABUSIRQ(7,0x50,0x00010000);
	ABUSIRQ(7,0x51,0x00020000);
	ABUSIRQ(7,0x52,0x00040000);
	ABUSIRQ(7,0x53,0x00080000);
	ABUSIRQ(4,0x54,0x00100000);
	ABUSIRQ(4,0x55,0x00200000);
	ABUSIRQ(4,0x56,0x00400000);
	ABUSIRQ(4,0x57,0x00800000);
	ABUSIRQ(1,0x58,0x01000000);
	ABUSIRQ(1,0x59,0x02000000);
	ABUSIRQ(1,0x5a,0x04000000);
	ABUSIRQ(1,0x5b,0x08000000);
	ABUSIRQ(1,0x5c,0x10000000);
	ABUSIRQ(1,0x5d,0x20000000);
	ABUSIRQ(1,0x5e,0x40000000);
	ABUSIRQ(1,0x5f,0x80000000);
}
#endif

/**************************************************************************************/

/*
I/O overview:
    PORT-A  1st player inputs
    PORT-B  2nd player inputs
    PORT-C  system input
    PORT-D  system output
    PORT-E  I/O 1
    PORT-F  I/O 2
    PORT-G  I/O 3
    PORT-AD AD-Stick inputs?(Fake for now...)
    SERIAL COM

offsets:
    0h PORT-A
    0l PORT-B
    1h PORT-C
    1l PORT-D
    2h PORT-E
    2l PORT-F (extra button layout)
    3h PORT-G
    3l
    4h PORT-SEL
    4l
    5h SERIAL COM WRITE
    5l
    6h SERIAL COM READ
    6l
    7h
    7l PORT-AD
*/
static const UINT8 port_ad[] =
{
	0xcc,0xb2,0x99,0x7f,0x66,0x4c,0x33,0x19
};

static UINT8 port_sel,mux_data;
static int port_i;

static READ32_HANDLER ( stv_io_r32 )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
//  if(LOG_IOGA) logerror("(PC=%08X): I/O r %08X & %08X\n", cpu_get_pc(&space->device()), offset*4, mem_mask);
//  popmessage("SEL: %02x MUX: %02x OFF: %02x",port_sel,mux_data,offset*4);

//  printf("(PC=%08X): I/O r %08X & %08X\n", cpu_get_pc(&space->device()), offset*4, mem_mask);

	switch(offset)
	{
		case 0x00/4:
		switch(port_sel)
		{
			case 0x77: return 0xff000000|(input_port_read(space->machine(), "P1") << 16) |0x0000ff00|(input_port_read(space->machine(), "P2"));
			case 0x67:
			{
				switch(mux_data)
				{
					/*Mahjong panel interface,bit wise(ACTIVE LOW)*/
					case 0xfe:	return 0xff000000 | (input_port_read_safe(space->machine(), "KEY0", 0)  << 16) | 0x0000ff00 | (input_port_read_safe(space->machine(), "KEY5", 0));
					case 0xfd:  return 0xff000000 | (input_port_read_safe(space->machine(), "KEY1", 0)  << 16) | 0x0000ff00 | (input_port_read_safe(space->machine(), "KEY6", 0));
					case 0xfb:	return 0xff000000 | (input_port_read_safe(space->machine(), "KEY2", 0)  << 16) | 0x0000ff00 | (input_port_read_safe(space->machine(), "KEY7", 0));
					case 0xf7:	return 0xff000000 | (input_port_read_safe(space->machine(), "KEY3", 0) << 16) | 0x0000ff00 | (input_port_read_safe(space->machine(), "KEY8", 0));
					case 0xef:  return 0xff000000 | (input_port_read_safe(space->machine(), "KEY4", 0) << 16) | 0x0000ff00 | (input_port_read_safe(space->machine(), "KEY9", 0));
					/*Joystick panel*/
					default:
					//popmessage("%02x MUX DATA",mux_data);
				    return (input_port_read(space->machine(), "P1") << 16) | (input_port_read(space->machine(), "P2"));
				}
			}
			case 0x47:
			{
				if ( strcmp(space->machine().system().name,"critcrsh") == 0 )
				{
					int data1 = 0, data2 = 0;

					/* Critter Crusher */
					data1 = input_port_read(space->machine(), "LIGHTX");
					data1 = BITSWAP8(data1, 2, 3, 0, 1, 6, 7, 5, 4) & 0xf3;
					data1 |= (input_port_read(space->machine(), "P1") & 1) ? 0x0 : 0x4;
					data2 = input_port_read(space->machine(), "LIGHTY");
					data2 = BITSWAP8(data2, 2, 3, 0, 1, 6, 7, 5, 4) & 0xf3;
					data2 |= (input_port_read(space->machine(), "P1") & 1) ? 0x0 : 0x4;

					return 0xff000000 | data1 << 16 | 0x0000ff00 | data2;
				}
			}
			//default:
			//case 0x40: return space->machine().rand();
			default:
			//popmessage("%02x PORT SEL",port_sel);
			return (input_port_read(space->machine(), "P1") << 16) | (input_port_read(space->machine(), "P2"));
		}
		case 0x04/4:
		if ( strcmp(space->machine().system().name,"critcrsh") == 0 )
			return ((input_port_read(space->machine(), "SYSTEM") << 16) & ((input_port_read(space->machine(), "P1") & 1) ? 0xffef0000 : 0xffff0000)) | (state->m_ioga[1]);
		else
			return (input_port_read(space->machine(), "SYSTEM") << 16) | (state->m_ioga[1]);

		case 0x08/4:
		switch(port_sel)
		{
			case 0x77:	return (input_port_read(space->machine(), "UNUSED") << 16) | (input_port_read(space->machine(), "EXTRA"));
			case 0x67:	return 0xffffffff;/**/
			case 0x20:  return 0xffff0000 | (state->m_ioga[2] & 0xffff);
			case 0x10:  return ((state->m_ioga[2] & 0xffff) << 16) | 0xffff;
			case 0x60:  return 0xffffffff;/**/
			default:
			return 0xffffffff;
		}
		case 0x0c/4:
		switch(port_sel)
		{
			case 0x60:  return ((state->m_ioga[2] & 0xffff) << 16) | 0xffff;
			default:
			//popmessage("offs: 3 %02x",port_sel);
			return 0xffffffff;
		}
		//case 0x10/4:
		case 0x14/4:
		switch(port_sel)
		{
			case 0x77:
			{
				//popmessage("(PC=%06x) offs 5 %04x %02x",cpu_get_pc(&space->device()),port_sel,((state->m_ioga[5] & 0xff0000) >> 16));
				logerror("(PC=%06x) offs 5 %04x %02x\n",cpu_get_pc(&space->device()),port_sel,((state->m_ioga[5] & 0xff0000) >> 16));

				//stv_m_workram_h[0x8e830/4] = ((state->m_ioga[5] & 0xff0000) >> 16) ^ 0x3;
				//stv_m_workram_h[0x8e834/4] = ((state->m_ioga[5] & 0xff0000) >> 16) ^ 0x3;
				return (state->m_ioga[5] & 0xff0000) >> 16;//stv_m_workram_h[0x8e830/4];//sound board data
			}
			default: return 0xffffffff;
		}
		case 0x18/4:
		switch(port_sel)
		{
			case 0x60:  return state->m_ioga[5];
			case 0x77:
			{
				//popmessage("(PC=%06x) offs 6 %04x %02x",cpu_get_pc(&space->device()),port_sel,((state->m_ioga[5] & 0xff0000) >> 16));
				logerror("(PC=%06x) offs 6 %04x %02x\n",cpu_get_pc(&space->device()),port_sel,((state->m_ioga[5] & 0xff0000) >> 16));
				return 0;//sound board status,non-zero = processing
			}
			default:
			//popmessage("offs: 6 %02x",port_sel);
			return 0xffffffff;
		}
		case 0x1c/4:
		if(LOG_IOGA) logerror("(PC %s=%06x) Warning: READ from PORT_AD\n", space->device().tag(), cpu_get_pc(&space->device()));
		popmessage("Read from PORT_AD");
		port_i++;
		return port_ad[port_i & 7];
		default:
		return state->m_ioga[offset];
	}
}

static WRITE32_HANDLER ( stv_io_w32 )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
//  if(LOG_IOGA) logerror("(PC=%08X): I/O w %08X = %08X & %08X\n", cpu_get_pc(&space->device()), offset*4, data, mem_mask);

//  printf("(PC=%08X): I/O w %08X = %08X & %08X\n", cpu_get_pc(&space->device()), offset*4, data, mem_mask);

	switch(offset)
	{
		case 0x04/4:
			if(ACCESSING_BITS_0_7)
			{
				/*Why does the BIOS tests these as ACTIVE HIGH? A program bug?*/
				state->m_ioga[1] = (data) & 0xff;
				coin_counter_w(space->machine(), 0,~data & 0x01);
				coin_counter_w(space->machine(), 1,~data & 0x02);
				coin_lockout_w(space->machine(), 0,~data & 0x04);
				coin_lockout_w(space->machine(), 1,~data & 0x08);
				/*
                other bits reserved
                */
			}
			break;
		case 0x08/4:
			if(ACCESSING_BITS_16_23)
			{
				state->m_ioga[2] = data >> 16;
				mux_data = state->m_ioga[2];
			}
			else if(ACCESSING_BITS_0_7)
				state->m_ioga[2] = data;
			break;
		case 0x0c/4:
			if(ACCESSING_BITS_16_23)
				state->m_ioga[3] = data;
			break;
		case 0x10/4:
			if(ACCESSING_BITS_16_23)
				port_sel = (data & 0xffff0000) >> 16;
			break;
		case 0x14/4:
			if(ACCESSING_BITS_16_23)
				state->m_ioga[5] = data;
			break;
		//case 0x18/4:
		case 0x1c/4:
			//technical bowling tests here
			if(ACCESSING_BITS_16_23)
				state->m_ioga[7] = data;
			break;

	}
}

/*

SCU Handling

*/

/**********************************************************************************
SCU Register Table
offset,relative address
Registers are in long words.
===================================================================================
0     0000  Level 0 DMA Set Register
1     0004
2     0008
3     000c
4     0010
5     0014
6     0018
7     001c
8     0020  Level 1 DMA Set Register
9     0024
10    0028
11    002c
12    0030
13    0034
14    0038
15    003c
16    0040  Level 2 DMA Set Register
17    0044
18    0048
19    004c
20    0050
21    0054
22    0058
23    005c
24    0060  DMA Forced Stop
25    0064
26    0068
27    006c
28    0070  <Free>
29    0074
30    0078
31    007c  DMA Status Register
32    0080  DSP Program Control Port
33    0084  DSP Program RAM Data Port
34    0088  DSP Data RAM Address Port
35    008c  DSP Data RAM Data Port
36    0090  Timer 0 Compare Register
37    0094  Timer 1 Set Data Register
38    0098  Timer 1 Mode Register
39    009c  <Free>
40    00a0  Interrupt Mask Register
41    00a4  Interrupt Status Register
42    00a8  A-Bus Interrupt Acknowledge
43    00ac  <Free>
44    00b0  A-Bus Set Register
45    00b4
46    00b8  A-Bus Refresh Register
47    00bc  <Free>
48    00c0
49    00c4  SCU SDRAM Select Register
50    00c8  SCU Version Register
51    00cc  <Free>
52    00cf
===================================================================================
DMA Status Register(32-bit):
xxxx xxxx x--- xx-- xx-- xx-- xx-- xx-- UNUSED
---- ---- -x-- ---- ---- ---- ---- ---- DMA DSP-Bus access
---- ---- --x- ---- ---- ---- ---- ---- DMA B-Bus access
---- ---- ---x ---- ---- ---- ---- ---- DMA A-Bus access
---- ---- ---- --x- ---- ---- ---- ---- DMA lv 1 interrupt
---- ---- ---- ---x ---- ---- ---- ---- DMA lv 0 interrupt
---- ---- ---- ---- --x- ---- ---- ---- DMA lv 2 in stand-by
---- ---- ---- ---- ---x ---- ---- ---- DMA lv 2 in operation
---- ---- ---- ---- ---- --x- ---- ---- DMA lv 1 in stand-by
---- ---- ---- ---- ---- ---x ---- ---- DMA lv 1 in operation
---- ---- ---- ---- ---- ---- --x- ---- DMA lv 0 in stand-by
---- ---- ---- ---- ---- ---- ---x ---- DMA lv 0 in operation
---- ---- ---- ---- ---- ---- ---- --x- DSP side DMA in stand-by
---- ---- ---- ---- ---- ---- ---- ---x DSP side DMA in operation

**********************************************************************************/
/*
DMA TODO:
-Add timings(but how fast are each DMA?).
-Add level priority & DMA status register.
-Add byte data type transfer.
-Set boundaries.
*/

#define DIRECT_MODE(_lv_)			(!(state->m_scu_regs[5+(_lv_*8)] & 0x01000000))
#define INDIRECT_MODE(_lv_)			  (state->m_scu_regs[5+(_lv_*8)] & 0x01000000)
#define DRUP(_lv_)					  (state->m_scu_regs[5+(_lv_*8)] & 0x00010000)
#define DWUP(_lv_)                    (state->m_scu_regs[5+(_lv_*8)] & 0x00000100)

#define DMA_STATUS				(state->m_scu_regs[31])
/*These macros sets the various DMA status flags.*/
#define DnMV_1(_ch_) DMA_STATUS|=(0x10 << 4 * _ch_)
#define DnMV_0(_ch_) DMA_STATUS&=~(0x10 << 4 * _ch_)

static UINT32 scu_add_tmp;

/*For area checking*/
#define ABUS(_lv_)       ((state->m_scu.src[_lv_] & 0x07ffffff) >= 0x02000000) && ((state->m_scu.src[_lv_] & 0x07ffffff) <= 0x04ffffff)
#define BBUS(_lv_)       ((scu_##_lv_ & 0x07ffffff) >= 0x05a00000) && ((scu_##_lv_ & 0x07ffffff) <= 0x05ffffff)
#define VDP1_REGS(_lv_)  ((scu_##_lv_ & 0x07ffffff) >= 0x05d00000) && ((scu_##_lv_ & 0x07ffffff) <= 0x05dfffff)
#define VDP2(_lv_)       ((scu_##_lv_ & 0x07ffffff) >= 0x05e00000) && ((scu_##_lv_ & 0x07ffffff) <= 0x05fdffff)
#define WORK_RAM_L(_lv_) ((scu_##_lv_ & 0x07ffffff) >= 0x00200000) && ((scu_##_lv_ & 0x07ffffff) <= 0x002fffff)
#define WORK_RAM_H(_lv_) ((scu_##_lv_ & 0x07ffffff) >= 0x06000000) && ((scu_##_lv_ & 0x07ffffff) <= 0x060fffff)
#define SOUND_RAM(_lv_)  ((scu_##_lv_ & 0x07ffffff) >= 0x05a00000) && ((scu_##_lv_ & 0x07ffffff) <= 0x05afffff)

static void scu_do_transfer(running_machine &machine,UINT8 event)
{
	saturn_state *state = machine.driver_data<saturn_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	int i;

	for(i=0;i<3;i++)
	{
		if(state->m_scu.enable_mask[i] && state->m_scu.start_factor[i] == event)
		{
			if(DIRECT_MODE(i))		{ scu_dma_direct(space,i);   }
			else				    { scu_dma_indirect(space,i); }
		}
	}
}

/* test pending irqs */
static void scu_test_irq(running_machine &machine)
{
	saturn_state *state = machine.driver_data<saturn_state>();
	int i;
	const int irq_level[32] = { 0x1, 0x1, 0x1, 0x1,
								0x1, 0x1, 0x1, 0x1,
								0x4, 0x4, 0x4, 0x4,
								0x7, 0x7, 0x7, 0x7,
								 -1,  -1, 0x2, 0x3,
								0x5, 0x6, 0x6, 0x8,
								0x8, 0x9, 0xa, 0xb,
								0xc, 0xd, 0xe, 0xf };

	for(i=0;i<31;i++)
	{
		if((state->m_scu_regs[0xa0/4] & 1 << i) && (state->m_scu.ist & 1 << i))
		{
			if(irq_level[i] != -1) /* TODO: cheap check for undefined irqs */
				device_set_input_line_and_vector(state->m_maincpu, irq_level[i], HOLD_LINE, 0x40 + i);
			state->m_scu.ist &= ~(1 << i);
		}
	}
}

static READ32_HANDLER( saturn_scu_r )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
	UINT32 res;

	/*TODO: write only registers must return 0...*/
	switch(offset)
	{
		//case 0x5c/4:
		//	Super Major League reads here???
		//	break;
	 	case 0x7c/4:
			if(LOG_SCU) logerror("(PC=%08x) DMA status reg read\n",cpu_get_pc(&space->device()));
			res = state->m_scu_regs[offset];
			break;
		case 0x8c/4:
			if(LOG_SCU) logerror( "DSP mem read at %08X\n", state->m_scu_regs[34]);
        	res = dsp_ram_addr_r();
        	break;
    	case 0xa4/4:
    		if(LOG_SCU) logerror("(PC=%08x) IRQ status reg read %08x MASK=%08x\n",cpu_get_pc(&space->device()),mem_mask,state->m_scu_regs[0xa0/4]);

			res = state->m_scu.ist;
			break;
		case 0xc8/4:
			logerror("(PC=%08x) SCU version reg read\n",cpu_get_pc(&space->device()));
			res = 0x00000000;/*SCU Version 0, OK?*/
			break;
		default:
	    	if(LOG_SCU) logerror("(PC=%08x) SCU reg read at %d = %08x\n",cpu_get_pc(&space->device()),offset,state->m_scu_regs[offset]);
	    	res = state->m_scu_regs[offset];
			break;
	}

	return res;
}

#define DMA_CH ((offset & 0x18) / 8)

static WRITE32_HANDLER( saturn_scu_w )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	COMBINE_DATA(&state->m_scu_regs[offset]);

	switch(offset)
	{
		/*LV 0 DMA*/
		case 0x00/4: case 0x20/4: case 0x40/4:  state->m_scu.src[DMA_CH]  = ((state->m_scu_regs[offset] & 0x07ffffff) >> 0); break;
		case 0x04/4: case 0x24/4: case 0x44/4:  state->m_scu.dst[DMA_CH]  = ((state->m_scu_regs[offset] & 0x07ffffff) >> 0); break;
		case 0x08/4: case 0x28/4: case 0x48/4:  state->m_scu.size[DMA_CH] = ((state->m_scu_regs[offset] & ((offset == 2) ? 0x000fffff : 0x1fff)) >> 0); break;
		case 0x0c/4: case 0x2c/4: case 0x4c/4:
			/*Read address add value for DMA lv 0*/
			state->m_scu.src_add[DMA_CH] = (state->m_scu_regs[offset] & 0x100) ? 4 : 1;

			/*Write address add value for DMA lv 0*/
			state->m_scu.dst_add[DMA_CH] = 2 << (state->m_scu_regs[offset] & 7);
			break;
		case 0x10/4: case 0x30/4: case 0x50/4:
			state->m_scu.enable_mask[DMA_CH] = (data & 0x100) >> 8;
			if(state->m_scu.enable_mask[DMA_CH] && state->m_scu.start_factor[DMA_CH] == 7 && state->m_scu_regs[offset] & 1)
			{
				if(DIRECT_MODE(DMA_CH)) { scu_dma_direct(space,DMA_CH);   }
				else				    { scu_dma_indirect(space,DMA_CH); }
				state->m_scu_regs[offset]&=~1;//disable starting bit.
			}
			break;
		case 0x14/4: case 0x34/4: case 0x54/4:
			if(INDIRECT_MODE(DMA_CH))
			{
				if(LOG_SCU) logerror("Indirect Mode DMA lv %d set\n",DMA_CH);
				if(!DWUP(DMA_CH)) state->m_scu.index[DMA_CH] = state->m_scu.dst[DMA_CH];
			}

			/*Start factor enable bits,bit 2,bit 1 and bit 0*/
			state->m_scu.start_factor[DMA_CH] = state->m_scu_regs[offset] & 7;
			break;

		case 0x60/4:
			if(LOG_SCU) logerror("DMA Forced Stop Register set = %02x\n",state->m_scu_regs[24]);
			break;
		case 0x7c/4: if(LOG_SCU) logerror("Warning: DMA status WRITE! Offset %02x(%d)\n",offset*4,offset); break;
		/*DSP section*/
		/*Use functions so it is easier to work out*/
		case 0x80/4:
			dsp_prg_ctrl(space, data);
			if(LOG_SCU) logerror("SCU DSP: Program Control Port Access %08x\n",data);
			break;
		case 0x84/4:
			dsp_prg_data(data);
			if(LOG_SCU) logerror("SCU DSP: Program RAM Data Port Access %08x\n",data);
			break;
		case 0x88/4:
			dsp_ram_addr_ctrl(data);
			if(LOG_SCU) logerror("SCU DSP: Data RAM Address Port Access %08x\n",data);
			break;
		case 0x8c/4:
			dsp_ram_addr_w(data);
			if(LOG_SCU) logerror("SCU DSP: Data RAM Data Port Access %08x\n",data);
			break;
		case 0x90/4: if(LOG_SCU) logerror("timer 0 compare data = %03x\n",state->m_scu_regs[36]);break;
		case 0x94/4: if(LOG_SCU) logerror("timer 1 set data = %08x\n",state->m_scu_regs[37]); break;
		case 0x98/4: if(LOG_SCU) logerror("timer 1 mode data = %08x\n",state->m_scu_regs[38]); break;
		case 0xa0/4:
		/*An interrupt is masked when his specific bit is 1.*/
		/*Are bit 16-bit 31 for External A-Bus irq mask like the status register?*/

		state->m_scu_irq.vblank_in =  (((state->m_scu_regs[0xa0/4] & 0x0001)>>0) ^ 1);
		state->m_scu_irq.vblank_out = (((state->m_scu_regs[0xa0/4] & 0x0002)>>1) ^ 1);
		state->m_scu_irq.hblank_in =  (((state->m_scu_regs[0xa0/4] & 0x0004)>>2) ^ 1);
		state->m_scu_irq.timer_0 =	  (((state->m_scu_regs[0xa0/4] & 0x0008)>>3) ^ 1);
		state->m_scu_irq.timer_1 =    (((state->m_scu_regs[0xa0/4] & 0x0010)>>4) ^ 1);
		state->m_scu_irq.dsp_end =    (((state->m_scu_regs[0xa0/4] & 0x0020)>>5) ^ 1);
		state->m_scu_irq.sound_req =  (((state->m_scu_regs[0xa0/4] & 0x0040)>>6) ^ 1);
		state->m_scu_irq.smpc =       (((state->m_scu_regs[0xa0/4] & 0x0080)>>7) ^ 1);
		state->m_scu_irq.pad =        (((state->m_scu_regs[0xa0/4] & 0x0100)>>8) ^ 1);
		state->m_scu_irq.dma_end[2] = (((state->m_scu_regs[0xa0/4] & 0x0200)>>9) ^ 1);
		state->m_scu_irq.dma_end[1] = (((state->m_scu_regs[0xa0/4] & 0x0400)>>10) ^ 1);
		state->m_scu_irq.dma_end[0] = (((state->m_scu_regs[0xa0/4] & 0x0800)>>11) ^ 1);
		state->m_scu_irq.dma_ill =    (((state->m_scu_regs[0xa0/4] & 0x1000)>>12) ^ 1);
		state->m_scu_irq.vdp1_end =   (((state->m_scu_regs[0xa0/4] & 0x2000)>>13) ^ 1);
		state->m_scu_irq.abus =       (((state->m_scu_regs[0xa0/4] & 0x8000)>>15) ^ 1);

		scu_test_irq(space->machine());

		if(state->m_scu_irq.dsp_end || state->m_scu_irq.pad)
			if(state->m_scu_regs[0xa0/4] != 0x80)
				popmessage("Enabled funky IRQ %08x, contact MAMEdev",state->m_scu_regs[0xa0/4] ^ 0xffffffff);

		break;
		/*Interrupt Control reg Set*/
		case 0xa4/4:
		if(LOG_SCU) logerror("PC=%08x IRQ status reg set:%08x %08x\n",cpu_get_pc(&space->device()),state->m_scu_regs[41],mem_mask);

		state->m_scu.ist &= state->m_scu_regs[offset];
		break;
		case 0xa8/4: if(LOG_SCU) logerror("A-Bus IRQ ACK %08x\n",state->m_scu_regs[42]); break;
		case 0xc4/4: if(LOG_SCU) logerror("SCU SDRAM set: %02x\n",state->m_scu_regs[49]); break;
		default: if(LOG_SCU) logerror("Warning: unused SCU reg set %d = %08x\n",offset,data);
	}
}

/*Lv 0 DMA end irq*/
static TIMER_CALLBACK( dma_lv0_ended )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	if(state->m_scu_irq.dma_end[0])
		device_set_input_line_and_vector(state->m_maincpu, 5, HOLD_LINE, 0x4b);
	else
		state->m_scu.ist |= (1 << 11);

	DnMV_0(0);
}

/*Lv 1 DMA end irq*/
static TIMER_CALLBACK( dma_lv1_ended )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	if(state->m_scu_irq.dma_end[1])
		device_set_input_line_and_vector(state->m_maincpu, 6, HOLD_LINE, 0x4a);
	else
		state->m_scu.ist |= (1 << 10);

	DnMV_0(1);
}

/*Lv 2 DMA end irq*/
static TIMER_CALLBACK( dma_lv2_ended )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	if(state->m_scu_irq.dma_end[2])
		device_set_input_line_and_vector(state->m_maincpu, 6, HOLD_LINE, 0x49);
	else
		state->m_scu.ist |= (1 << 9);

	DnMV_0(2);
}

static void scu_dma_direct(address_space *space, UINT8 dma_ch)
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
	static UINT32 tmp_src,tmp_dst,tmp_size;

	if(LOG_SCU) printf("DMA lv %d transfer START\n"
			             "Start %08x End %08x Size %04x\n",dma_ch,state->m_scu.src[dma_ch],state->m_scu.dst[dma_ch],state->m_scu.size[dma_ch]);
	if(LOG_SCU) printf("Start Add %04x Destination Add %04x\n",state->m_scu.src_add[dma_ch],state->m_scu.dst_add[dma_ch]);

	DnMV_1(dma_ch);

	/* max size */
	if(state->m_scu.size[dma_ch] == 0) { state->m_scu.size[dma_ch] = (dma_ch == 0) ? 0x00100000 : 0x2000; }

	/*set here the boundaries checks*/
	/*...*/
	if((state->m_scu.dst_add[dma_ch] != state->m_scu.src_add[dma_ch]) && (ABUS(dma_ch)))
	{
		logerror("A-Bus invalid transfer, sets to default\n");
		scu_add_tmp = (state->m_scu.dst_add[dma_ch]*0x100) | (state->m_scu.src_add[dma_ch]);
		state->m_scu.dst_add[dma_ch] = state->m_scu.src_add[dma_ch] = 4;
		scu_add_tmp |= 0x80000000;
	}

	#if 0

	/*Let me know if you encounter any of these three*/
	if(ABUS(dst_0))
	{
		logerror("A-Bus invalid write\n");
		/*...*/
	}
	if(WORK_RAM_L(dst_0))
	{
		logerror("WorkRam-L invalid write\n");
		/*...*/
	}
	if(VDP2(src_0))
	{
		logerror("VDP-2 invalid read\n");
		/*...*/
	}
	if(VDP1_REGS(dst_0))
	{
		logerror("VDP1 register access,must be in word units\n");
		scu_add_tmp = (state->m_scu.dst_add[0]*0x100) | (state->m_scu.src_add[0]);
		state->m_scu.dst_add[0] = state->m_scu.src_add[0] = 2;
		scu_add_tmp |= 0x80000000;
	}
	if(DRUP(0))
	{
		logerror("Data read update = 1,read address add value must be 1 too\n");
		scu_add_tmp = (state->m_scu.dst_add[0]*0x100) | (state->m_scu.src_add[0]);
		state->m_scu.src_add[0] = 4;
		scu_add_tmp |= 0x80000000;
	}

	if (WORK_RAM_H(dst_0) && (state->m_scu.dst_add[0] != 4))
	{
		scu_add_tmp = (state->m_scu.dst_add[0]*0x100) | (state->m_scu.src_add[0]);
		state->m_scu.dst_add[0] = 4;
		scu_add_tmp |= 0x80000000;
	}
	#endif

	tmp_size = state->m_scu.size[dma_ch];
	if(!(DRUP(dma_ch))) tmp_src = state->m_scu.src[dma_ch];
	if(!(DWUP(dma_ch))) tmp_dst = state->m_scu.dst[dma_ch];

	for (; state->m_scu.size[dma_ch] > 0; state->m_scu.size[dma_ch]-=state->m_scu.dst_add[dma_ch])
	{
		if(state->m_scu.dst_add[dma_ch] == 2)
			space->write_word(state->m_scu.dst[dma_ch],space->read_word(state->m_scu.src[dma_ch]));
		else if(state->m_scu.dst_add[dma_ch] == 8)
		{
			space->write_word(state->m_scu.dst[dma_ch],  space->read_word(state->m_scu.src[dma_ch]  ));
			space->write_word(state->m_scu.dst[dma_ch]+2,space->read_word(state->m_scu.src[dma_ch]  ));
			space->write_word(state->m_scu.dst[dma_ch]+4,space->read_word(state->m_scu.src[dma_ch]+2));
			space->write_word(state->m_scu.dst[dma_ch]+6,space->read_word(state->m_scu.src[dma_ch]+2));
		}
		else
		{
			space->write_word(state->m_scu.dst[dma_ch],  space->read_word(state->m_scu.src[dma_ch]  ));
			space->write_word(state->m_scu.dst[dma_ch]+2,space->read_word(state->m_scu.src[dma_ch]+2));
		}

		state->m_scu.dst[dma_ch]+=state->m_scu.dst_add[dma_ch];
		state->m_scu.src[dma_ch]+=state->m_scu.src_add[dma_ch];
	}

	state->m_scu.size[dma_ch] = tmp_size;
	if(!(DRUP(dma_ch))) state->m_scu.src[dma_ch] = tmp_src;
	if(!(DWUP(dma_ch))) state->m_scu.dst[dma_ch] = tmp_dst;

	if(dma_ch != 2)
	if(LOG_SCU) logerror("DMA transfer END\n");

	{
		/*TODO: this is completely wrong HW-wise ...  */
		switch(dma_ch)
		{
			case 0: space->machine().scheduler().timer_set(attotime::from_usec(state->m_instadma_hack ? 0 : 300), FUNC(dma_lv0_ended)); break;
			case 1: space->machine().scheduler().timer_set(attotime::from_usec(state->m_instadma_hack ? 0 : 300), FUNC(dma_lv1_ended)); break;
			case 2: space->machine().scheduler().timer_set(attotime::from_usec(state->m_instadma_hack ? 0 : 300), FUNC(dma_lv2_ended)); break;
		}
	}

	if(scu_add_tmp & 0x80000000)
	{
		state->m_scu.dst_add[dma_ch] = (scu_add_tmp & 0xff00) >> 8;
		state->m_scu.src_add[dma_ch] = (scu_add_tmp & 0x00ff) >> 0;
		scu_add_tmp&=~0x80000000;
	}
}

static void scu_dma_indirect(address_space *space,UINT8 dma_ch)
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	/*Helper to get out of the cycle*/
	UINT8 job_done = 0;
	/*temporary storage for the transfer data*/
	UINT32 tmp_src;
	UINT32 indirect_src,indirect_dst;
	INT32 indirect_size;

	DnMV_1(dma_ch);

	state->m_scu.index[dma_ch] = state->m_scu.dst[dma_ch];

	do{
		tmp_src = state->m_scu.index[dma_ch];

		indirect_size = space->read_dword(state->m_scu.index[dma_ch]);
		indirect_src  = space->read_dword(state->m_scu.index[dma_ch]+8);
		indirect_dst  = space->read_dword(state->m_scu.index[dma_ch]+4);

		/*Indirect Mode end factor*/
		if(indirect_src & 0x80000000)
			job_done = 1;

		if(LOG_SCU) printf("DMA lv %d indirect mode transfer START\n"
				           "Index %08x Start %08x End %08x Size %04x\n",dma_ch,tmp_src,indirect_src,indirect_dst,indirect_size);
		if(LOG_SCU) printf("Start Add %04x Destination Add %04x\n",state->m_scu.src_add[dma_ch],state->m_scu.dst_add[dma_ch]);

		//guess,but I believe it's right.
		indirect_src &=0x07ffffff;
		indirect_dst &=0x07ffffff;
		indirect_size &= ((dma_ch == 0) ? 0xfffff : 0x3ffff); //TODO: Guardian Heroes sets up a 0x23000 transfer for the FMV?

		if(indirect_size == 0) { indirect_size = (dma_ch == 0) ? 0x00100000 : 0x2000; }

		for (; indirect_size > 0; indirect_size-=state->m_scu.dst_add[dma_ch])
		{
			if(state->m_scu.dst_add[dma_ch] == 2)
				space->write_word(indirect_dst,space->read_word(indirect_src));
			else
			{
				/* some games, eg columns97 are a bit weird, I'm not sure this is correct
                  they start a dma on a 2 byte boundary in 4 byte add mode, using the dword reads we
                  can't access 2 byte boundaries, and the end of the sprite list never gets marked,
                  the length of the transfer is also set to a 2 byte boundary, maybe the add values
                  should be different, I don't know */
				space->write_word(indirect_dst,space->read_word(indirect_src));
				space->write_word(indirect_dst+2,space->read_word(indirect_src+2));
			}
			indirect_dst+=state->m_scu.dst_add[dma_ch];
			indirect_src+=state->m_scu.src_add[dma_ch];
		}

		//if(DRUP(0))   space->write_dword(tmp_src+8,state->m_scu.src[0]|job_done ? 0x80000000 : 0);
		//if(DWUP(0)) space->write_dword(tmp_src+4,state->m_scu.dst[0]);

		state->m_scu.index[dma_ch] = tmp_src+0xc;

	}while(job_done == 0);

	{
		/*TODO: this is completely wrong HW-wise ...  */
		switch(dma_ch)
		{
			case 0: space->machine().scheduler().timer_set(attotime::from_usec(state->m_instadma_hack ? 0 : 300), FUNC(dma_lv0_ended)); break;
			case 1: space->machine().scheduler().timer_set(attotime::from_usec(state->m_instadma_hack ? 0 : 300), FUNC(dma_lv1_ended)); break;
			case 2: space->machine().scheduler().timer_set(attotime::from_usec(state->m_instadma_hack ? 0 : 300), FUNC(dma_lv2_ended)); break;
		}
	}
}


/**************************************************************************************/

static WRITE16_HANDLER( saturn_soundram_w )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	//space->machine().scheduler().synchronize(); // force resync

	COMBINE_DATA(&state->m_sound_ram[offset]);
}

static READ16_HANDLER( saturn_soundram_r )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	//space->machine().scheduler().synchronize(); // force resync

	return state->m_sound_ram[offset];
}

/* communication,SLAVE CPU acquires data from the MASTER CPU and triggers an irq.  */
static WRITE32_HANDLER( minit_w )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	//logerror("cpu %s (PC=%08X) MINIT write = %08x\n", space->device().tag(), cpu_get_pc(&space->device()),data);
	space->machine().scheduler().boost_interleave(state->m_minit_boost_timeslice, attotime::from_usec(state->m_minit_boost));
	space->machine().scheduler().trigger(1000);
	sh2_set_frt_input(state->m_slave, PULSE_LINE);
}

static WRITE32_HANDLER( sinit_w )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	//logerror("cpu %s (PC=%08X) SINIT write = %08x\n", space->device().tag(), cpu_get_pc(&space->device()),data);
	space->machine().scheduler().boost_interleave(state->m_sinit_boost_timeslice, attotime::from_usec(state->m_sinit_boost));
	sh2_set_frt_input(state->m_maincpu, PULSE_LINE);
}

static READ32_HANDLER(saturn_backupram_r)
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	return state->m_backupram[offset] & 0x00ff00ff;	// yes, it makes sure the "holes" are there.
}

static WRITE32_HANDLER(saturn_backupram_w)
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	COMBINE_DATA(&state->m_backupram[offset]);
}

static NVRAM_HANDLER(saturn)
{
	saturn_state *state = machine.driver_data<saturn_state>();

	static const UINT32 init[8] =
	{
		0x420061, 0x63006b, 0x550070, 0x520061, 0x6d0020, 0x46006f, 0x72006d, 0x610074,
	};
	int i;

	if (read_or_write)
		file->write(state->m_backupram, 64*1024/4);
	else
	{
		if (file)
		{
			file->read(state->m_backupram, 64*1024/4);
		}
		else
		{
			memset(state->m_backupram, 0, 64*1024/4);
			for (i = 0; i < 8; i++)
			{
				state->m_backupram[i] = init[i];
				state->m_backupram[i+8] = init[i];
				state->m_backupram[i+16] = init[i];
				state->m_backupram[i+24] = init[i];
			}
		}
	}
}

static READ8_HANDLER( saturn_cart_type_r )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();
	const int cart_ram_header[3] = { 0xff, 0x5a, 0x5c };

	return cart_ram_header[state->m_cart_type];
}

static ADDRESS_MAP_START( saturn_mem, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_SHARE("share6")  // bios
	AM_RANGE(0x00100000, 0x0010007f) AM_READWRITE8(saturn_SMPC_r, saturn_SMPC_w,0xffffffff)
	AM_RANGE(0x00180000, 0x0018ffff) AM_READWRITE(saturn_backupram_r, saturn_backupram_w) AM_SHARE("share1") AM_BASE_MEMBER(saturn_state,m_backupram)
	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_MIRROR(0x100000) AM_SHARE("share2") AM_BASE_MEMBER(saturn_state,m_workram_l)
//  AM_RANGE(0x01000000, 0x01000003) AM_MIRROR(0x7ffffc) AM_WRITE(minit_w)
	AM_RANGE(0x01000000, 0x017fffff) AM_WRITE(minit_w)
//  AM_RANGE(0x01800000, 0x01800003) AM_MIRROR(0x7ffffc) AM_WRITE(sinit_w)
	AM_RANGE(0x01800000, 0x01ffffff) AM_WRITE(sinit_w)
	AM_RANGE(0x02000000, 0x023fffff) AM_ROM AM_SHARE("share7") AM_REGION("maincpu", 0x80000)	// cartridge space
//  AM_RANGE(0x02400000, 0x027fffff) AM_RAM //cart RAM area, dynamically allocated
	AM_RANGE(0x04fffffc, 0x04ffffff) AM_READ8(saturn_cart_type_r,0x000000ff)
	AM_RANGE(0x05800000, 0x0589ffff) AM_READWRITE(stvcd_r, stvcd_w)
	/* Sound */
	AM_RANGE(0x05a00000, 0x05a7ffff) AM_READWRITE16(saturn_soundram_r, saturn_soundram_w,0xffffffff)
	AM_RANGE(0x05b00000, 0x05b00fff) AM_DEVREADWRITE16("scsp", scsp_r, scsp_w, 0xffffffff)
	/* VDP1 */
	AM_RANGE(0x05c00000, 0x05c7ffff) AM_READWRITE(saturn_vdp1_vram_r, saturn_vdp1_vram_w)
	AM_RANGE(0x05c80000, 0x05cbffff) AM_READWRITE(saturn_vdp1_framebuffer0_r, saturn_vdp1_framebuffer0_w)
	AM_RANGE(0x05d00000, 0x05d0001f) AM_READWRITE16(saturn_vdp1_regs_r, saturn_vdp1_regs_w,0xffffffff)
	AM_RANGE(0x05e00000, 0x05efffff) AM_READWRITE(saturn_vdp2_vram_r, saturn_vdp2_vram_w)
	AM_RANGE(0x05f00000, 0x05f7ffff) AM_READWRITE(saturn_vdp2_cram_r, saturn_vdp2_cram_w)
	AM_RANGE(0x05f80000, 0x05fbffff) AM_READWRITE(saturn_vdp2_regs_r, saturn_vdp2_regs_w)
	AM_RANGE(0x05fe0000, 0x05fe00cf) AM_READWRITE(saturn_scu_r, saturn_scu_w)
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_MIRROR(0x01f00000) AM_SHARE("share3") AM_BASE_MEMBER(saturn_state,m_workram_h)
	AM_RANGE(0x20000000, 0x2007ffff) AM_ROM AM_SHARE("share6")  // bios mirror
	AM_RANGE(0x22000000, 0x24ffffff) AM_ROM AM_SHARE("share7")  // cart mirror
ADDRESS_MAP_END

static ADDRESS_MAP_START( stv_mem, AS_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_SHARE("share6")  // bios
	AM_RANGE(0x00100000, 0x0010007f) AM_READWRITE8(stv_SMPC_r, stv_SMPC_w,0xffffffff)
	AM_RANGE(0x00180000, 0x0018ffff) AM_READWRITE(saturn_backupram_r,saturn_backupram_w) AM_SHARE("share1") AM_BASE_MEMBER(saturn_state,m_backupram)
	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_MIRROR(0x100000) AM_SHARE("share2") AM_BASE_MEMBER(saturn_state,m_workram_l)
	AM_RANGE(0x00400000, 0x0040001f) AM_READWRITE(stv_io_r32, stv_io_w32) AM_BASE_MEMBER(saturn_state,m_ioga) AM_SHARE("share4") AM_MIRROR(0x20)
//  AM_RANGE(0x01000000, 0x01000003) AM_MIRROR(0x7ffffc) AM_WRITE(minit_w)
	AM_RANGE(0x01000000, 0x017fffff) AM_WRITE(minit_w)
//  AM_RANGE(0x01800000, 0x01800003) AM_MIRROR(0x7ffffc) AM_WRITE(sinit_w)
	AM_RANGE(0x01800000, 0x01ffffff) AM_WRITE(sinit_w)
	AM_RANGE(0x02000000, 0x04ffffff) AM_ROM AM_SHARE("share7") AM_REGION("abus", 0) // cartridge
	AM_RANGE(0x05800000, 0x0589ffff) AM_READWRITE(stvcd_r, stvcd_w)
	/* Sound */
	AM_RANGE(0x05a00000, 0x05afffff) AM_READWRITE16(saturn_soundram_r, saturn_soundram_w,0xffffffff)
	AM_RANGE(0x05b00000, 0x05b00fff) AM_DEVREADWRITE16("scsp", scsp_r, scsp_w, 0xffffffff)
	/* VDP1 */
	AM_RANGE(0x05c00000, 0x05c7ffff) AM_READWRITE(saturn_vdp1_vram_r, saturn_vdp1_vram_w)
	AM_RANGE(0x05c80000, 0x05cbffff) AM_READWRITE(saturn_vdp1_framebuffer0_r, saturn_vdp1_framebuffer0_w)
	AM_RANGE(0x05d00000, 0x05d0001f) AM_READWRITE16(saturn_vdp1_regs_r, saturn_vdp1_regs_w,0xffffffff)
	AM_RANGE(0x05e00000, 0x05efffff) AM_READWRITE(saturn_vdp2_vram_r, saturn_vdp2_vram_w)
	AM_RANGE(0x05f00000, 0x05f7ffff) AM_READWRITE(saturn_vdp2_cram_r, saturn_vdp2_cram_w)
	AM_RANGE(0x05f80000, 0x05fbffff) AM_READWRITE(saturn_vdp2_regs_r, saturn_vdp2_regs_w)
	AM_RANGE(0x05fe0000, 0x05fe00cf) AM_READWRITE(saturn_scu_r, saturn_scu_w)
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_MIRROR(0x01f00000) AM_SHARE("share3") AM_BASE_MEMBER(saturn_state,m_workram_h)
	AM_RANGE(0x20000000, 0x2007ffff) AM_ROM AM_SHARE("share6")  // bios mirror
	AM_RANGE(0x22000000, 0x24ffffff) AM_ROM AM_SHARE("share7")  // cart mirror
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_mem, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_RAM AM_BASE_MEMBER(saturn_state,m_sound_ram)
	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE("scsp", scsp_r, scsp_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( saturn )
	PORT_START("PDR1")
	PORT_DIPNAME( 0x01, 0x01, "PDR1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PDR2")
	PORT_DIPNAME( 0x01, 0x01, "PDR2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("JOY1")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1)	// START
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 A") PORT_PLAYER(1)	// A
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 B") PORT_PLAYER(1)	// B
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 C") PORT_PLAYER(1)	// C
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 X") PORT_PLAYER(1)	// X
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Y") PORT_PLAYER(1)	// Y
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Z") PORT_PLAYER(1)	// Z
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("P1 L") PORT_PLAYER(1)	// L
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P1 R") PORT_PLAYER(1)	// R
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED ) //read '1' when direct mode is polled

	PORT_START("JOY2")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2)	// START
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 A") PORT_PLAYER(2)	// A
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 B") PORT_PLAYER(2)	// B
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 C") PORT_PLAYER(2)	// C
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 X") PORT_PLAYER(2)	// X
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Y") PORT_PLAYER(2)	// Y
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Z") PORT_PLAYER(2)	// Z
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("P2 L") PORT_PLAYER(2)	// L
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P2 R") PORT_PLAYER(2)	// R
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED ) //read '1' when direct mode is polled

	PORT_START("CART_AREA")
	PORT_CONFNAME( 0x03, 0x02, "Cart Type" )
	PORT_CONFSETTING( 0x00, "None" )
	PORT_CONFSETTING( 0x01, "8 Mbit Cart RAM" )
	PORT_CONFSETTING( 0x02, "32 Mbit Cart RAM" )
INPUT_PORTS_END

#define STV_PLAYER_INPUTS(_n_, _b1_, _b2_, _b3_)							\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_##_b1_ ) PORT_PLAYER(_n_)			\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_##_b2_ ) PORT_PLAYER(_n_)			\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_##_b3_ ) PORT_PLAYER(_n_)			\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )							\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(_n_)		\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(_n_)		\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_)	\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(_n_)

static INPUT_PORTS_START( stv )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "PDR1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "PDR2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	STV_PLAYER_INPUTS(1, BUTTON1, BUTTON2, BUTTON3)

	PORT_START("P2")
	STV_PLAYER_INPUTS(2, BUTTON1, BUTTON2, BUTTON3)
/*
    PORT_START("P3")
    STV_PLAYER_INPUTS(3, BUTTON1, BUTTON2, BUTTON3)

    PORT_START("P4")
    STV_PLAYER_INPUTS(4, BUTTON1, BUTTON2, BUTTON3)
*/

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1P Push Switch") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2P Push Switch") PORT_CODE(KEYCODE_8)

	/* This *might* be unused... */
	PORT_START("UNUSED")
	PORT_BIT ( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Extra button layout, used by Power Instinct 3, Suikoenbu, Elan Doree, Golden Axe Duel & Astra SuperStars */
	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNKNOWN")
	PORT_DIPNAME( 0x01, 0x01, "UNK" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( critcrsh )
	PORT_INCLUDE( stv )

	/* IN 7 */
	PORT_START("LIGHTX") /* mask default type                     sens delta min max */
	PORT_BIT( 0x3f, 0x00, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,0x3f) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(1)

	/* IN 8 */
	PORT_START("LIGHTY")
	PORT_BIT( 0x3f, 0x00, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x0,0x3f) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( batmanfr )
	PORT_INCLUDE( stv )

	PORT_MODIFY("P1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_MODIFY("EXTRA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Same as the regular one, but with an additional & optional mahjong panel */
static INPUT_PORTS_START( stvmp )
	PORT_INCLUDE( stv )

	/* Mahjong panel/player 1 side */
	PORT_START("KEY0")	/*7*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I )

	PORT_START("KEY1")	/*8*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J )

	PORT_START("KEY2")	/*9*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K )

	PORT_START("KEY3")	/*10*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L )

	PORT_START("KEY4")	/*11*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Mahjong panel/player 2 side */
	PORT_START("KEY5")	/*12*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)

	PORT_START("KEY6")	/*13*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)

	PORT_START("KEY7")	/*14*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)

	PORT_START("KEY8")	/*15*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)

	PORT_START("KEY9")	/*16*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

#ifdef UNUSED_FUNCTION
static WRITE32_HANDLER ( w60ffc44_write )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	COMBINE_DATA(&state->m_workram_h[0xffc44/4]);

	logerror("cpu %s (PC=%08X): 60ffc44_write write = %08X & %08X\n", space->device().tag(), cpu_get_pc(&space->device()), data, mem_mask);
	//sinit_w(offset,data,mem_mask);
}

static WRITE32_HANDLER ( w60ffc48_write )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	COMBINE_DATA(&state->m_workram_h[0xffc48/4]);

	logerror("cpu %s (PC=%08X): 60ffc48_write write = %08X & %08X\n", space->device().tag(), cpu_get_pc(&space->device()), data, mem_mask);
	//minit_w(offset,data,mem_mask);
}
#endif

#ifdef UNUSED_FUNCTION
static void print_game_info(void);
#endif

DRIVER_INIT ( stv )
{
	saturn_state *state = machine.driver_data<saturn_state>();
	system_time systime;

	machine.base_datetime(systime);

	/* amount of time to boost interleave for on MINIT / SINIT, needed for communication to work */
	state->m_minit_boost = 400;
	state->m_sinit_boost = 400;
	state->m_minit_boost_timeslice = attotime::zero;
	state->m_sinit_boost_timeslice = attotime::zero;

	state->m_smpc_ram = auto_alloc_array(machine, UINT8, 0x80);
	state->m_scu_regs = auto_alloc_array(machine, UINT32, 0x100/4);
	state->m_scsp_regs  = auto_alloc_array(machine, UINT16, 0x1000/2);

	install_stvbios_speedups(machine);

	// do strict overwrite verification - maruchan and rsgun crash after coinup without this.
	// cottonbm needs strict PCREL
	// todo: test what games need this and don't turn it on for them...
	sh2drc_set_options(machine.device("maincpu"), SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);
	sh2drc_set_options(machine.device("slave"), SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);

	/* debug .. watch the command buffer rsgun, cottonbm etc. appear to use to communicate between cpus */
	//machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x60ffc44, 0x60ffc47, FUNC(w60ffc44_write) );
	//machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x60ffc48, 0x60ffc4b, FUNC(w60ffc48_write) );
	//machine.device("slave")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x60ffc44, 0x60ffc47, FUNC(w60ffc44_write) );
	//machine.device("slave")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x60ffc48, 0x60ffc4b, FUNC(w60ffc48_write) );

    state->m_smpc_ram[0x31] = 0x00; //CTG1=0 CTG0=0 (correct??)
//  state->m_smpc_ram[0x33] = input_port_read(machine, "FAKE");
	state->m_smpc_ram[0x5f] = 0x10;

	state->m_instadma_hack = 0;
	state->m_vdp2.pal = 0;

	#ifdef MAME_DEBUG
	/*Uncomment this to enable header info*/
	//print_game_info();
	#endif
}

#define DATA_TRANSFER(_max_) \
	for(dst_i=0;dst_i<_max_;dst_i++,src_i++)	\
		STR[(dst_i & 0xfc) | (~dst_i & 3)] = ROM[src_i];

#define DATA_DELETE \
	for(dst_i=0;dst_i<0x100;dst_i++) \
		STR[dst_i] = 0x00;

#ifdef UNUSED_FUNCTION
static void print_game_info(void)
{
	UINT8 *ROM = machine.region("game0")->base();
	static FILE *print_file = NULL;
	UINT8 STR[0x100];
	UINT32 src_i,dst_i;

	if(print_file == NULL)
		print_file = fopen( "stvinfo.txt", "a" );

	src_i = 0;

	/*IC13?*/
	if(ROM[src_i] == 0x00)
		src_i+=0x200000;

	DATA_TRANSFER(0x100);
	for(src_i=0;src_i<0x100;src_i++)
	{
		if((src_i % 0x10) == 0) fprintf( print_file, "\n");
		if(src_i < 0xc0)		fprintf( print_file, "%c",STR[src_i] );
		else                    fprintf( print_file, "%02x",STR[src_i] );
	}
	DATA_DELETE;

	fclose(print_file);
	print_file = NULL;
}
#endif

static const gfx_layout tiles8x8x4_layout =
{
	8,8,
	0x100000/(32*8/8),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tiles16x16x4_layout =
{
	16,16,
	0x100000/(32*32/8),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28,
	  32*8+0, 32*8+4, 32*8+8, 32*8+12, 32*8+16, 32*8+20, 32*8+24, 32*8+28,

	  },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  32*16, 32*17,32*18, 32*19,32*20,32*21,32*22,32*23

	  },
	32*32
};

static const gfx_layout tiles8x8x8_layout =
{
	8,8,
	0x100000/(32*8/8),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	32*8	/* really 64*8, but granularity is 32 bytes */
};

static const gfx_layout tiles16x16x8_layout =
{
	16,16,
	0x100000/(64*16/8),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56,
	64*8+0, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8

	},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	64*16, 64*17, 64*18, 64*19, 64*20, 64*21, 64*22, 64*23
	},
	64*16	/* really 128*16, but granularity is 32 bytes */
};




static GFXDECODE_START( stv )
	GFXDECODE_ENTRY( NULL, 0, tiles8x8x4_layout,   0x00, (0x80*(2+1))  )
	GFXDECODE_ENTRY( NULL, 0, tiles16x16x4_layout, 0x00, (0x80*(2+1))  )
	GFXDECODE_ENTRY( NULL, 0, tiles8x8x8_layout,   0x00, (0x08*(2+1))  )
	GFXDECODE_ENTRY( NULL, 0, tiles16x16x8_layout, 0x00, (0x08*(2+1))  )

	/* vdp1 .. pointless for drawing but can help us debug */
	GFXDECODE_ENTRY( NULL, 0, tiles8x8x4_layout,   0x00, 0x100  )
	GFXDECODE_ENTRY( NULL, 0, tiles16x16x4_layout, 0x00, 0x100  )
	GFXDECODE_ENTRY( NULL, 0, tiles8x8x8_layout,   0x00, 0x20  )
	GFXDECODE_ENTRY( NULL, 0, tiles16x16x8_layout, 0x00, 0x20  )

GFXDECODE_END

static const sh2_cpu_core sh2_conf_master = { 0, NULL };
static const sh2_cpu_core sh2_conf_slave  = { 1, NULL };

static int scsp_last_line = 0;

static void scsp_irq(device_t *device, int irq)
{
	saturn_state *state = device->machine().driver_data<saturn_state>();

	// don't bother the 68k if it's off
	if (!state->m_en_68k)
	{
		return;
	}

	if (irq > 0)
	{
		scsp_last_line = irq;
		cputag_set_input_line(device->machine(), "audiocpu", irq, ASSERT_LINE);
	}
	else if (irq < 0)
	{
		cputag_set_input_line(device->machine(), "audiocpu", -irq, CLEAR_LINE);
	}
	else
	{
		cputag_set_input_line(device->machine(), "audiocpu", scsp_last_line, CLEAR_LINE);
	}
}

static WRITE_LINE_DEVICE_HANDLER( scsp_to_main_irq )
{
	saturn_state *drvstate = device->machine().driver_data<saturn_state>();

	if(drvstate->m_scu_irq.sound_req)
	{
		device_set_input_line_and_vector(drvstate->m_maincpu, 9, HOLD_LINE, 0x46);
		scu_do_transfer(device->machine(),5);
	}
	else
		drvstate->m_scu.ist |= (1 << 6);
}

static const scsp_interface scsp_config =
{
	0,
	scsp_irq,
	DEVCB_LINE(scsp_to_main_irq)
};

static TIMER_CALLBACK(stv_rtc_increment)
{
	saturn_state *state = machine.driver_data<saturn_state>();
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	static int year_num, year_count;

	/*
        state->m_smpc_ram[0x23] = DectoBCD(systime.local_time.year /100);
        state->m_smpc_ram[0x25] = DectoBCD(systime.local_time.year %100);
        state->m_smpc_ram[0x27] = (systime.local_time.weekday << 4) | (systime.local_time.month+1);
        state->m_smpc_ram[0x29] = DectoBCD(systime.local_time.mday);
        state->m_smpc_ram[0x2b] = DectoBCD(systime.local_time.hour);
        state->m_smpc_ram[0x2d] = DectoBCD(systime.local_time.minute);
        state->m_smpc_ram[0x2f] = DectoBCD(systime.local_time.second);
    */

	state->m_smpc_ram[0x2f]++;

	/* seconds from 9 -> 10*/
	if((state->m_smpc_ram[0x2f] & 0x0f) >= 0x0a)			{ state->m_smpc_ram[0x2f]+=0x10; state->m_smpc_ram[0x2f]&=0xf0; }
	/* seconds from 59 -> 0 */
	if((state->m_smpc_ram[0x2f] & 0xf0) >= 0x60)			{ state->m_smpc_ram[0x2d]++;     state->m_smpc_ram[0x2f] = 0; }
	/* minutes from 9 -> 10 */
	if((state->m_smpc_ram[0x2d] & 0x0f) >= 0x0a)			{ state->m_smpc_ram[0x2d]+=0x10; state->m_smpc_ram[0x2d]&=0xf0; }
	/* minutes from 59 -> 0 */
	if((state->m_smpc_ram[0x2d] & 0xf0) >= 0x60)			{ state->m_smpc_ram[0x2b]++;     state->m_smpc_ram[0x2d] = 0; }
	/* hours from 9 -> 10 */
	if((state->m_smpc_ram[0x2b] & 0x0f) >= 0x0a)			{ state->m_smpc_ram[0x2b]+=0x10; state->m_smpc_ram[0x2b]&=0xf0; }
	/* hours from 23 -> 0 */
	if((state->m_smpc_ram[0x2b] & 0xff) >= 0x24)				{ state->m_smpc_ram[0x29]++; state->m_smpc_ram[0x27]+=0x10; state->m_smpc_ram[0x2b] = 0; }
	/* week day name sunday -> monday */
	if((state->m_smpc_ram[0x27] & 0xf0) >= 0x70)				{ state->m_smpc_ram[0x27]&=0x0f; }
	/* day number 9 -> 10 */
	if((state->m_smpc_ram[0x29] & 0x0f) >= 0x0a)				{ state->m_smpc_ram[0x29]+=0x10; state->m_smpc_ram[0x29]&=0xf0; }

	// year BCD to dec conversion (for the leap year stuff)
	{
		year_num = (state->m_smpc_ram[0x25] & 0xf);

		for(year_count = 0; year_count < (state->m_smpc_ram[0x25] & 0xf0); year_count += 0x10)
			year_num += 0xa;

		year_num += (state->m_smpc_ram[0x23] & 0xf)*0x64;

		for(year_count = 0; year_count < (state->m_smpc_ram[0x23] & 0xf0); year_count += 0x10)
			year_num += 0x3e8;
	}

	/* month +1 check */
	/* the RTC have a range of 1980 - 2100, so we don't actually need to support the leap year special conditions */
	if(((year_num % 4) == 0) && (state->m_smpc_ram[0x27] & 0xf) == 2)
	{
		if((state->m_smpc_ram[0x29] & 0xff) >= dpm[(state->m_smpc_ram[0x27] & 0xf)-1]+1+1)
			{ state->m_smpc_ram[0x27]++; state->m_smpc_ram[0x29] = 0x01; }
	}
	else if((state->m_smpc_ram[0x29] & 0xff) >= dpm[(state->m_smpc_ram[0x27] & 0xf)-1]+1){ state->m_smpc_ram[0x27]++; state->m_smpc_ram[0x29] = 0x01; }
	/* year +1 check */
	if((state->m_smpc_ram[0x27] & 0x0f) > 12)				{ state->m_smpc_ram[0x25]++;  state->m_smpc_ram[0x27] = (state->m_smpc_ram[0x27] & 0xf0) | 0x01; }
	/* year from 9 -> 10 */
	if((state->m_smpc_ram[0x25] & 0x0f) >= 0x0a)				{ state->m_smpc_ram[0x25]+=0x10; state->m_smpc_ram[0x25]&=0xf0; }
	/* year from 99 -> 100 */
	if((state->m_smpc_ram[0x25] & 0xf0) >= 0xa0)				{ state->m_smpc_ram[0x23]++; state->m_smpc_ram[0x25] = 0; }

	// probably not SO precise, here just for reference ...
	/* year from 999 -> 1000 */
	//if((state->m_smpc_ram[0x23] & 0x0f) >= 0x0a)               { state->m_smpc_ram[0x23]+=0x10; state->m_smpc_ram[0x23]&=0xf0; }
	/* year from 9999 -> 0 */
	//if((state->m_smpc_ram[0x23] & 0xf0) >= 0xa0)               { state->m_smpc_ram[0x23] = 0; } //roll over
}

static MACHINE_START( stv )
{
	saturn_state *state = machine.driver_data<saturn_state>();
	system_time systime;
	machine.base_datetime(systime);

	state->m_maincpu = downcast<legacy_cpu_device*>( machine.device("maincpu") );
	state->m_slave = downcast<legacy_cpu_device*>( machine.device("slave") );
	state->m_audiocpu = downcast<legacy_cpu_device*>( machine.device("audiocpu") );

	scsp_set_ram_base(machine.device("scsp"), state->m_sound_ram);

	// save states
	state_save_register_global_pointer(machine, state->m_smpc_ram, 0x80);
	state_save_register_global_pointer(machine, state->m_scu_regs, 0x100/4);
	state_save_register_global_pointer(machine, state->m_scsp_regs,  0x1000/2);
	state_save_register_global(machine, state->m_NMI_reset);
	state_save_register_global(machine, state->m_en_68k);
//  state_save_register_global(machine, scanline);
	state_save_register_global(machine, state->m_smpc.IOSEL1);
	state_save_register_global(machine, state->m_smpc.IOSEL2);
	state_save_register_global(machine, state->m_smpc.EXLE1);
	state_save_register_global(machine, state->m_smpc.EXLE2);
	state_save_register_global(machine, state->m_smpc.PDR1);
	state_save_register_global(machine, state->m_smpc.PDR2);
	state_save_register_global(machine, port_sel);
	state_save_register_global(machine, mux_data);
	state_save_register_global(machine, scsp_last_line);

	stv_register_protection_savestates(machine); // machine/stvprot.c

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(stvcd_exit), &machine));

	state->m_smpc_ram[0x23] = DectoBCD(systime.local_time.year /100);
    state->m_smpc_ram[0x25] = DectoBCD(systime.local_time.year %100);
    state->m_smpc_ram[0x27] = (systime.local_time.weekday << 4) | (systime.local_time.month+1);
    state->m_smpc_ram[0x29] = DectoBCD(systime.local_time.mday);
    state->m_smpc_ram[0x2b] = DectoBCD(systime.local_time.hour);
    state->m_smpc_ram[0x2d] = DectoBCD(systime.local_time.minute);
    state->m_smpc_ram[0x2f] = DectoBCD(systime.local_time.second);

	state->m_stv_rtc_timer = machine.scheduler().timer_alloc(FUNC(stv_rtc_increment));
}


static MACHINE_START( saturn )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	state->m_maincpu = downcast<legacy_cpu_device*>( machine.device("maincpu") );
	state->m_slave = downcast<legacy_cpu_device*>( machine.device("slave") );
	state->m_audiocpu = downcast<legacy_cpu_device*>( machine.device("audiocpu") );

	scsp_set_ram_base(machine.device("scsp"), state->m_sound_ram);

	// save states
	state_save_register_global_pointer(machine, state->m_smpc_ram, 0x80);
	state_save_register_global_pointer(machine, state->m_scu_regs, 0x100/4);
	state_save_register_global_pointer(machine, state->m_scsp_regs,  0x1000/2);
	state_save_register_global(machine, state->m_NMI_reset);
	state_save_register_global(machine, state->m_en_68k);
	state_save_register_global(machine, state->m_smpc.IOSEL1);
	state_save_register_global(machine, state->m_smpc.IOSEL2);
	state_save_register_global(machine, state->m_smpc.EXLE1);
	state_save_register_global(machine, state->m_smpc.EXLE2);
	state_save_register_global(machine, state->m_smpc.PDR1);
	state_save_register_global(machine, state->m_smpc.PDR2);
//  state_save_register_global(machine, port_sel);
//  state_save_register_global(machine, mux_data);
	state_save_register_global(machine, scsp_last_line);
	state_save_register_global(machine, state->m_smpc.intback_stage);
	state_save_register_global(machine, state->m_smpc.pmode);
	state_save_register_global(machine, state->m_smpc.smpcSR);
	state_save_register_global_array(machine, state->m_smpc.SMEM);
	state_save_register_global_pointer(machine, state->m_cart_dram, 0x400000/4);

	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(stvcd_exit), &machine));
}


/*
(Preliminary) explaination about this:
VBLANK-OUT is used at the start of the vblank period.It also sets the timer zero
variable to 0.
If the Timer Compare register is zero too,the Timer 0 irq is triggered.

HBLANK-IN is used at the end of each scanline except when in VBLANK-IN/OUT periods.

The timer 0 is also incremented by one at each HBLANK and checked with the value
of the Timer Compare register;if equal,the timer 0 irq is triggered here too.
Notice that the timer 0 compare register can be more than the VBLANK maximum range,in
this case the timer 0 irq is simply never triggered.This is a known Sega Saturn/ST-V "bug".

VBLANK-IN is used at the end of the vblank period.

SCU register[36] is the timer zero compare register.
SCU register[40] is for IRQ masking.

TODO:
- VDP1 timing and CEF emulation isn't accurate at all.
*/


static TIMER_DEVICE_CALLBACK( saturn_scanline )
{
	saturn_state *state = timer.machine().driver_data<saturn_state>();
	int scanline = param;
	int max_y = timer.machine().primary_screen->height();
	int y_step,vblank_line;

	y_step = 2;

	if((max_y == 263 && state->m_vdp2.pal == 0) || (max_y == 313 && state->m_vdp2.pal == 1))
		y_step = 1;

	vblank_line = (state->m_vdp2.pal) ? 288 : 240;

	//popmessage("%08x %d %08x %08x",state->m_scu_regs[40] ^ 0xffffffff,max_y,state->m_scu_regs[36],state->m_scu_regs[38]);

	if(scanline == 0*y_step)
	{
		if(state->m_scu_irq.vblank_out)
		{
			device_set_input_line_and_vector(state->m_maincpu, 0xe, (state->m_scu_irq.vblank_out) ? HOLD_LINE : CLEAR_LINE , 0x41);
			scu_do_transfer(timer.machine(),1);
		}
		else
			state->m_scu.ist |= (1 << 1);

	}
	else if(scanline == vblank_line*y_step)
	{
		if(state->m_scu_irq.vblank_in)
		{
			device_set_input_line_and_vector(state->m_maincpu, 0xf, HOLD_LINE ,0x40);
			scu_do_transfer(timer.machine(),0);
		}
		else
			state->m_scu.ist |= (1 << 0);

		if(state->m_scu_irq.vdp1_end)
		{
			device_set_input_line_and_vector(state->m_maincpu, 0x2, HOLD_LINE, 0x4d);
			scu_do_transfer(timer.machine(),6);
		}
		else
			state->m_scu.ist |= (1 << 13);

		video_update_vdp1(timer.machine());
	}
	else if((scanline % y_step) == 0 && scanline < vblank_line*y_step)
	{
		if(state->m_scu_irq.hblank_in)
		{
			device_set_input_line_and_vector(state->m_maincpu, 0xd, HOLD_LINE, 0x42);
			scu_do_transfer(timer.machine(),2);
		}
		else
			state->m_scu.ist |= (1 << 2);
	}

	if(scanline == (state->m_scu_regs[36] & 0x3ff)*y_step)
	{
		if(state->m_scu_irq.timer_0)
		{
			device_set_input_line_and_vector(state->m_maincpu, 0xc, HOLD_LINE, 0x43 );
			scu_do_transfer(timer.machine(),3);
		}
		else
			state->m_scu.ist |= (1 << 3);
	}

	/* TODO: this isn't completely correct */
	if((state->m_scu_regs[38] & 0x81) == 0x01 && ((scanline % y_step) == 0))
	{
		if(state->m_scu_irq.timer_1)
		{
			device_set_input_line_and_vector(state->m_maincpu, 0xb, HOLD_LINE, 0x44 );
			scu_do_transfer(timer.machine(),4);
		}
		else
			state->m_scu.ist |= (1 << 4);
	}
}

static READ32_HANDLER( saturn_cart_dram0_r )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	return state->m_cart_dram[offset];
}

static WRITE32_HANDLER( saturn_cart_dram0_w )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	COMBINE_DATA(&state->m_cart_dram[offset]);
}

static READ32_HANDLER( saturn_cart_dram1_r )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	return state->m_cart_dram[offset+0x200000/4];
}

static WRITE32_HANDLER( saturn_cart_dram1_w )
{
	saturn_state *state = space->machine().driver_data<saturn_state>();

	COMBINE_DATA(&state->m_cart_dram[offset+0x200000/4]);
}

static MACHINE_RESET( saturn )
{
	saturn_state *state = machine.driver_data<saturn_state>();
	// don't let the slave cpu and the 68k go anywhere
	cputag_set_input_line(machine, "slave", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);

	state->m_smpc.smpcSR = 0x40;	// this bit is always on according to docs

	state->m_en_68k = 0;
	state->m_NMI_reset = 1;
	state->m_smpc_ram[0x21] = (0x80) | ((state->m_NMI_reset & 1) << 6);

	DMA_STATUS = 0;

	//memset(stv_m_workram_l, 0, 0x100000);
	//memset(stv_m_workram_h, 0, 0x100000);

	machine.device("maincpu")->set_unscaled_clock(MASTER_CLOCK_320/2);
	machine.device("slave")->set_unscaled_clock(MASTER_CLOCK_320/2);

	stvcd_reset( machine );

	state->m_cart_type = input_port_read(machine,"CART_AREA") & 3;

	machine.device("maincpu")->memory().space(AS_PROGRAM)->nop_readwrite(0x02400000, 0x027fffff);
	machine.device("slave")->memory().space(AS_PROGRAM)->nop_readwrite(0x02400000, 0x027fffff);

	if(state->m_cart_type == 1)
	{
		//  AM_RANGE(0x02400000, 0x027fffff) AM_RAM //cart RAM area, dynamically allocated
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x02400000, 0x0247ffff, FUNC(saturn_cart_dram0_r), FUNC(saturn_cart_dram0_w));
		machine.device("slave")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x02400000, 0x0247ffff, FUNC(saturn_cart_dram0_r), FUNC(saturn_cart_dram0_w));
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x02600000, 0x0267ffff, FUNC(saturn_cart_dram1_r), FUNC(saturn_cart_dram1_w));
		machine.device("slave")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x02600000, 0x0267ffff, FUNC(saturn_cart_dram1_r), FUNC(saturn_cart_dram1_w));
	}

	if(state->m_cart_type == 2)
	{
		//  AM_RANGE(0x02400000, 0x027fffff) AM_RAM //cart RAM area, dynamically allocated
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x02400000, 0x025fffff, FUNC(saturn_cart_dram0_r), FUNC(saturn_cart_dram0_w));
		machine.device("slave")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x02400000, 0x025fffff, FUNC(saturn_cart_dram0_r), FUNC(saturn_cart_dram0_w));
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x02600000, 0x027fffff, FUNC(saturn_cart_dram1_r), FUNC(saturn_cart_dram1_w));
		machine.device("slave")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x02600000, 0x027fffff, FUNC(saturn_cart_dram1_r), FUNC(saturn_cart_dram1_w));
	}

	/* TODO: default value is probably 7 */
	state->m_scu.start_factor[0] = -1;
	state->m_scu.start_factor[1] = -1;
	state->m_scu.start_factor[2] = -1;
}


static MACHINE_RESET( stv )
{
	saturn_state *state = machine.driver_data<saturn_state>();

	// don't let the slave cpu and the 68k go anywhere
	cputag_set_input_line(machine, "slave", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);

	state->m_en_68k = 0;
	state->m_NMI_reset = 1;
	state->m_smpc_ram[0x21] = (0x80) | ((state->m_NMI_reset & 1) << 6);

	port_sel = mux_data = 0;
	port_i = -1;

	machine.device("maincpu")->set_unscaled_clock(MASTER_CLOCK_320/2);
	machine.device("slave")->set_unscaled_clock(MASTER_CLOCK_320/2);

	stvcd_reset(machine);

	state->m_stv_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));
	state->m_prev_bankswitch = 0xff;

	/* TODO: default value is probably 7 */
	state->m_scu.start_factor[0] = -1;
	state->m_scu.start_factor[1] = -1;
	state->m_scu.start_factor[2] = -1;
}

struct cdrom_interface saturn_cdrom =
{
	NULL,
	NULL
};

static MACHINE_CONFIG_START( saturn, saturn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(saturn_mem)
	MCFG_CPU_CONFIG(sh2_conf_master)
	MCFG_TIMER_ADD_SCANLINE("scantimer", saturn_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("slave", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(saturn_mem)
	MCFG_CPU_CONFIG(sh2_conf_slave)

	MCFG_CPU_ADD("audiocpu", M68000, 11289600) //11.2896 MHz
	MCFG_CPU_PROGRAM_MAP(sound_mem)

	MCFG_MACHINE_START(saturn)
	MCFG_MACHINE_RESET(saturn)

	MCFG_NVRAM_HANDLER(saturn)

	MCFG_TIMER_ADD("sector_timer", stv_sector_cb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_320/8, 427, 0, 320, 263, 0, 224)
	MCFG_SCREEN_UPDATE(stv_vdp2)

	MCFG_PALETTE_LENGTH(2048+(2048*2))//standard palette + extra memory for rgb brightness.
	MCFG_GFXDECODE(stv)

	MCFG_VIDEO_START(stv_vdp2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("scsp", SCSP, 0)
	MCFG_SOUND_CONFIG(scsp_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("cdda", CDDA, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_CDROM_ADD( "cdrom", saturn_cdrom)
	MCFG_CARTSLOT_ADD("cart")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( stv, saturn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(stv_mem)
	MCFG_CPU_CONFIG(sh2_conf_master)
	MCFG_TIMER_ADD_SCANLINE("scantimer", saturn_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("slave", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(stv_mem)
	MCFG_CPU_CONFIG(sh2_conf_slave)

	MCFG_CPU_ADD("audiocpu", M68000, 11289600) //11.2896 MHz
	MCFG_CPU_PROGRAM_MAP(sound_mem)

	MCFG_MACHINE_START(stv)
	MCFG_MACHINE_RESET(stv)

	MCFG_EEPROM_93C46_ADD("eeprom") /* Actually 93c45 */

	MCFG_TIMER_ADD("sector_timer", stv_sector_cb)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_320/8, 427, 0, 320, 263, 0, 224)
	MCFG_SCREEN_UPDATE(stv_vdp2)

	MCFG_PALETTE_LENGTH(2048+(2048*2))//standard palette + extra memory for rgb brightness.
	MCFG_GFXDECODE(stv)

	MCFG_VIDEO_START(stv_vdp2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("scsp", SCSP, 0)
	MCFG_SOUND_CONFIG(scsp_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("cdda", CDDA, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

struct stv_cart_region
{
	const char *tag;
	int        slot;
	const char *region;
};

static const struct stv_cart_region stv_cart_table[] =
{
	{ "cart1", 0, "game0" },
	{ "cart2", 1, "game1" },
	{ "cart3", 2, "game2" },
	{ "cart4", 3, "game3" },
	{ 0 }
};

static DEVICE_IMAGE_LOAD( stv_cart )
{
//  saturn_state *state = image.device().machine().driver_data<saturn_state>();
	const struct stv_cart_region *stv_cart = &stv_cart_table[0], *this_cart;
	//const char    *pcb_name;

	/* First, determine where this cart has to be loaded */
	while (stv_cart->tag)
	{
		if (strcmp(stv_cart->tag, image.device().tag()) == 0)
			break;

		stv_cart++;
	}

	this_cart = stv_cart;

	if (image.software_entry() == NULL)
		return IMAGE_INIT_FAIL;

	UINT8 *ROM = image.device().machine().region(this_cart->region)->base();
	UINT32 length = image.get_software_region_length("rom");

	memcpy(ROM, image.get_software_region("rom"), length);

	/* fix endianess */
	{
		UINT8 j[4];
		int i;

		for(i=0;i<length;i+=4)
		{
			j[0] = ROM[i];
			j[1] = ROM[i+1];
			j[2] = ROM[i+2];
			j[3] = ROM[i+3];
			ROM[i] = j[3];
			ROM[i+1] = j[2];
			ROM[i+2] = j[1];
			ROM[i+3] = j[0];
		}
	}

	//if ((pcb_name = image.get_feature("pcb_type")) == NULL)
	//  return IMAGE_INIT_FAIL;

	return IMAGE_INIT_PASS;
}


#define MCFG_STV_CARTSLOT_ADD(_tag) \
	MCFG_CARTSLOT_ADD(_tag) \
	MCFG_CARTSLOT_INTERFACE("stv_cart") \
	MCFG_CARTSLOT_LOAD(stv_cart)

MACHINE_CONFIG_FRAGMENT( stv_cartslot )
	MCFG_STV_CARTSLOT_ADD("cart1")
	MCFG_STV_CARTSLOT_ADD("cart2")
	MCFG_STV_CARTSLOT_ADD("cart3")
	MCFG_STV_CARTSLOT_ADD("cart4")

	MCFG_SOFTWARE_LIST_ADD("cart_list","stv")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( stv_slot, stv )
	MCFG_FRAGMENT_ADD( stv_cartslot )
MACHINE_CONFIG_END


static void saturn_init_driver(running_machine &machine, int rgn)
{
	saturn_state *state = machine.driver_data<saturn_state>();
	system_time systime;

	state->m_saturn_region = rgn;
	state->m_vdp2.pal = (rgn == 12) ? 1 : 0;

	// set compatible options
	sh2drc_set_options(machine.device("maincpu"), SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);
	sh2drc_set_options(machine.device("slave"), SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);

	/* get the current date/time from the core */
	machine.current_datetime(systime);

	/* amount of time to boost interleave for on MINIT / SINIT, needed for communication to work */
	state->m_minit_boost = 400;
	state->m_sinit_boost = 400;
	state->m_minit_boost_timeslice = attotime::zero;
	state->m_sinit_boost_timeslice = attotime::zero;

	state->m_smpc_ram = auto_alloc_array(machine, UINT8, 0x80);
	state->m_scu_regs = auto_alloc_array(machine, UINT32, 0x100/4);
	state->m_scsp_regs = auto_alloc_array(machine, UINT16, 0x1000/2);
	state->m_cart_dram = auto_alloc_array(machine, UINT32, 0x400000/4);

	state->m_smpc_ram[0x23] = dec_2_bcd(systime.local_time.year / 100);
	state->m_smpc_ram[0x25] = dec_2_bcd(systime.local_time.year % 100);
	state->m_smpc_ram[0x27] = (systime.local_time.weekday << 4) | (systime.local_time.month + 1);
	state->m_smpc_ram[0x29] = dec_2_bcd(systime.local_time.mday);
	state->m_smpc_ram[0x2b] = dec_2_bcd(systime.local_time.hour);
	state->m_smpc_ram[0x2d] = dec_2_bcd(systime.local_time.minute);
	state->m_smpc_ram[0x2f] = dec_2_bcd(systime.local_time.second);
	state->m_smpc_ram[0x31] = 0x00; //CTG1=0 CTG0=0 (correct??)
//  state->m_smpc_ram[0x33] = input_port_read(machine, "???");
	state->m_smpc_ram[0x5f] = 0x10;
}

static DRIVER_INIT( saturnus )
{
	saturn_init_driver(machine, 4);
}

static DRIVER_INIT( saturneu )
{
	saturn_init_driver(machine, 12);
}

static DRIVER_INIT( saturnjp )
{
	saturn_init_driver(machine, 1);
}


/* Japanese Saturn */
ROM_START(saturnjp)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_SYSTEM_BIOS(0, "101", "Japan v1.01 (941228)")
	ROMX_LOAD("sega_101.bin", 0x00000000, 0x00080000, CRC(224b752c) SHA1(df94c5b4d47eb3cc404d88b33a8fda237eaf4720), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "1003", "Japan v1.003 (941012)")
	ROMX_LOAD("sega1003.bin", 0x00000000, 0x00080000, CRC(b3c63c25) SHA1(7b23b53d62de0f29a23e423d0fe751dfb469c2fa), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "100", "Japan v1.00 (940921)")
	ROMX_LOAD("sega_100.bin", 0x00000000, 0x00080000, CRC(2aba43c2) SHA1(2b8cb4f87580683eb4d760e4ed210813d667f0a2), ROM_BIOS(3))
	ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)
ROM_END

/* Overseas Saturn */
ROM_START(saturn)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_SYSTEM_BIOS(0, "101a", "Overseas v1.01a (941115)")
	/* Confirmed by ElBarto */
	ROMX_LOAD("mpr-17933.bin", 0x00000000, 0x00080000, CRC(4afcf0fa) SHA1(faa8ea183a6d7bbe5d4e03bb1332519800d3fbc3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "100a", "Overseas v1.00a (941115)")
	ROMX_LOAD("sega_100a.bin", 0x00000000, 0x00080000, CRC(f90f0089) SHA1(3bb41feb82838ab9a35601ac666de5aacfd17a58), ROM_BIOS(2))
	ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)
ROM_END

ROM_START(saturneu)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_SYSTEM_BIOS(0, "101a", "Overseas v1.01a (941115)")
	/* Confirmed by ElBarto */
	ROMX_LOAD("mpr-17933.bin", 0x00000000, 0x00080000, CRC(4afcf0fa) SHA1(faa8ea183a6d7bbe5d4e03bb1332519800d3fbc3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "100a", "Overseas v1.00a (941115)")
	ROMX_LOAD("sega_100a.bin", 0x00000000, 0x00080000, CRC(f90f0089) SHA1(3bb41feb82838ab9a35601ac666de5aacfd17a58), ROM_BIOS(2))
	ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)
ROM_END

ROM_START(vsaturn)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_LOAD("vsaturn.bin", 0x00000000, 0x00080000, CRC(e4d61811) SHA1(4154e11959f3d5639b11d7902b3a393a99fb5776))
	ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)
ROM_END

ROM_START(hisaturn)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_LOAD("hisaturn.bin", 0x00000000, 0x00080000, CRC(721e1b60) SHA1(49d8493008fa715ca0c94d99817a5439d6f2c796))
	ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)
ROM_END

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

/* BIOS rev info found at 0x800 (byte swapped):

erp-17740  - Japan   STVB1.10J0950131  95/01/31 v1.10  - Found on a early board dated 02/1995
epr-17951a - Japan   STVB1.13J0950425  95/04/25 v1.13
epr-17952a - USA     STVB1.13U0950425  95/04/25 v1.13
epr-17953a - Taiwan  STVB1.13T0950425  95/04/25 v1.13
epr-17954a - Europe  STVB1.13E0950425  95/04/25 v1.13

epr-18343  - ?????   STVB1.30S0950727  95/07/27 v1.30 - Special version used on Sports Fishing 2 (CD based)
epr-19730  - Japan   PCD11.13J0970217  97/02/17 v1.13 - Enhanced version for CD based units?
epr-20091  - Japan   PCS11.13J0970821  97/08/21 v1.13 - Enhanced version for ???

stv110.bin  - Debug  STVB1.10D0950113  95/01/13 v1.10
stv1061.bin - ST-V Dev Bios (1.061) - Sega 1994, Noted "ST-V Ver 1.061 94/11/25" on EPROM sticker, Coming from a S-TV SG5001A dev board

Regions found listed in the BIOS:
    Japan
    Taiwan and Philipines
    USA and Canada
    Brazil
    Korea
    Asia PAL area
    Europe
    Latin America

ROM_SYSTEM_BIOS( x, "saturn",      "Saturn bios" ) \
ROM_LOAD16_WORD_SWAP_BIOS( x, "saturn.bin", 0x000000, 0x080000, CRC(653ff2d8) SHA1(20994ae7ee177ddaf3a430b010c7620dca000fb4) ) - Saturn Eu Bios

*/

#define STV_BIOS \
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH2 code */ \
	ROM_SYSTEM_BIOS( 0,  "jp",    "EPR-20091 (Japan 97/08/21)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0,  "epr-20091.ic8",   0x000000, 0x080000, CRC(59ed40f4) SHA1(eff0f54c70bce05ff3a289bf30b1027e1c8cd117) ) \
	ROM_SYSTEM_BIOS( 1,  "jp1",   "EPR-19730 (Japan 97/02/17)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1,  "epr-19730.ic8",   0x000000, 0x080000, CRC(d0e0889d) SHA1(fae53107c894e0c41c49e191dbe706c9cd6e50bd) ) \
	ROM_SYSTEM_BIOS( 2,  "jp2",   "EPR-17951A (Japan 95/04/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2,  "epr-17951a.ic8",  0x000000, 0x080000, CRC(2672f9d8) SHA1(63cf4a6432f6c87952f9cf3ab0f977aed2367303) ) \
	ROM_SYSTEM_BIOS( 3,  "jp3",   "STVB1.11J (Japan 95/02/20)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3,  "stvb111j.ic8",    0x000000, 0x080000, CRC(3e23c81f) SHA1(f9b282fd27693e9891843597b2e1823da3d23c7b) ) \
	ROM_SYSTEM_BIOS( 4,  "jp4",   "EPR-17740 (Japan 95/01/31)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4,  "epr-17740.ic8",   0x000000, 0x080000, CRC(5c5aa63d) SHA1(06860d96923b81afbc21e0ad32ee19487d8ff6e7) ) \
	ROM_SYSTEM_BIOS( 5,  "euro",  "EPR-17954A (Europe 95/04/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5,  "epr-17954a.ic8",  0x000000, 0x080000, CRC(f7722da3) SHA1(af79cff317e5b57d49e463af16a9f616ed1eee08) ) \
	ROM_SYSTEM_BIOS( 6,  "us",    "EPR-17952A (USA 95/04/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6,  "epr-17952a.ic8",  0x000000, 0x080000, CRC(d1be2adf) SHA1(eaf1c3e5d602e1139d2090a78d7e19f04f916794) ) \
	ROM_SYSTEM_BIOS( 7,  "tw",    "EPR-17953A (Taiwan 95/04/25)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7,  "epr-17953a.ic8",  0x000000, 0x080000, CRC(a4c47570) SHA1(9efc73717ec8a13417e65c54344ded9fc25bf5ef) ) \
	ROM_SYSTEM_BIOS( 8,  "tw1",   "STVB1.11T (Taiwan 95/02/20)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 8,  "stvb111t.ic8",    0x000000, 0x080000, CRC(02daf123) SHA1(23185beb1ce9c09b8719e57d1adb7b28c8141fd5) ) \
	ROM_SYSTEM_BIOS( 9,  "debug", "Debug (95/01/13)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 9,  "stv110.bin",      0x000000, 0x080000, CRC(3dfeda92) SHA1(8eb33192a57df5f3a1dfb57263054867c6b2db6d) ) \
	ROM_SYSTEM_BIOS( 10, "dev",   "Development (bios 1.061)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 10, "stv1061.bin",     0x000000, 0x080000, CRC(728dbca3) SHA1(0ed2030177f0aa8285645c395ae9ad9f568ab1d6) ) \
	\
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */ \
	ROM_COPY( "maincpu",0,0,0x080000) \
	\
	ROM_REGION32_BE( 0x3000000, "abus", ROMREGION_ERASE00 ) /* SH2 code */ \


ROM_START( stvbios )
	STV_BIOS
	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 )

	ROM_REGION32_BE( 0x3000000, "game1", ROMREGION_ERASE00 )

	ROM_REGION32_BE( 0x3000000, "game2", ROMREGION_ERASE00 )

	ROM_REGION32_BE( 0x3000000, "game3", ROMREGION_ERASE00 )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to multi cart mode
	ROM_LOAD( "stvbios.nv", 0x0000, 0x0080, CRC(15432ae2) SHA1(7d9364d546f3d3f839ec36be148076f8d26a65a6) )
ROM_END

/*

there appears to only be one main cartridge layout, just some having different positions populated if you use the ic named in
the test mode you have the following

some of the rom names were using something else and have been renamed to match test mode, old extension left in comments

( add 0x2000000 for real memory map location )

0x0000000 - 0x01fffff IC13 Header can be read from here .. IC13 roms are loaded on ODD bytes only
0x0200000 - 0x03fffff IC7  Header can also be read from here.  If No IC7 is present it seems to mirror IC13, but loaded normally (mausuke)
0x0400000 - 0x07fffff IC2
0x0800000 - 0x0bfffff IC3
0x0c00000 - 0x0ffffff IC4
0x1000000 - 0x13fffff IC5
0x1400000 - 0x17fffff IC6
0x1800000 - 0x1bfffff IC1
0x1c00000 - 0x1ffffff IC8
0x2000000 - 0x23fffff IC9
0x2400000 - 0x27fffff IC10
0x2800000 - 0x2bfffff IC11
0x2c00000 - 0x2ffffff IC12

*/




ROM_START( astrass )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20825.13",                0x0000001, 0x0100000, CRC(94a9ad8f) SHA1(861311c14cfa9f560752aa5b023c147a539cf135) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr20827.2",     0x0400000, 0x0400000, CRC(65cabbb3) SHA1(5e7cb090101dc42207a4084465e419f4311b6baf) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr20828.3",     0x0800000, 0x0400000, CRC(3934d44a) SHA1(969406b8bfac43b30f4d732702ca8cffeeefffb9) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr20829.4",     0x0c00000, 0x0400000, CRC(814308c3) SHA1(45c3f551690224c95acd156ae8f8397667927a04) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr20830.5",     0x1000000, 0x0400000, CRC(ff97fd19) SHA1(f37bcdce5f3f522527a44d59f1b8184ef290f829) ) // good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr20831.6",     0x1400000, 0x0400000, CRC(4408e6fb) SHA1(d4228cad8a1128e9426dac9ac62e9513a7a0117b) ) // good (was .16)
	ROM_LOAD16_WORD_SWAP( "mpr20826.1",     0x1800000, 0x0400000, CRC(bdc4b941) SHA1(c5e8b1b186324c2ccab617915f7bdbfe6897ca9f) ) // good (was .17)
	ROM_LOAD16_WORD_SWAP( "mpr20832.8",     0x1c00000, 0x0400000, CRC(af1b0985) SHA1(d7a0e4e0a8b0556915f924bdde8c3d14e5b3423e) ) // good (was .18s)
	ROM_LOAD16_WORD_SWAP( "mpr20833.9",     0x2000000, 0x0400000, CRC(cb6af231) SHA1(4a2e5d7c2fd6179c19cdefa84a03f9a34fbb9e70) ) // good (was .19s)

	ROM_REGION32_BE( 0x11ea0, "user2", 0 )
	// protection data (text layer tiles) extracted from Saturn version
	// stat_t.bin is name of the file from Saturn version CD
	ROM_LOAD( "stat_t.bin", 0x0000000, 0x11ea0,  BAD_DUMP CRC(52a0b542) SHA1(2b352dee919d7e7a5d338ece1691577d0b288eca) )
ROM_END

ROM_START( bakubaku )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr17969.13",               0x0000001, 0x0100000, CRC(bee327e5) SHA1(1d226db72d6ef68fd294f60659df7f882b25def6) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )

	ROM_LOAD16_WORD_SWAP( "mpr17970.2",    0x0400000, 0x0400000, CRC(bc4d6f91) SHA1(dcc241dcabea59325decfba3fd5e113c07958422) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17971.3",    0x0800000, 0x0400000, CRC(c780a3b3) SHA1(99587eea528a6413cacc3e4d3d1dbfff57b03dca) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17972.4",    0x0c00000, 0x0400000, CRC(8f29815a) SHA1(e86acd8096f2aee5f5e3ddfd3abb4f5c2b11df66) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17973.5",    0x1000000, 0x0400000, CRC(5f6e0e8b) SHA1(eeb5efb5216ab8b8fdee4656774bbd5a2a5b2d42) ) // good
ROM_END

ROM_START( colmns97 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr19553.13",    0x000001, 0x100000, CRC(d4fb6a5e) SHA1(bd3cfb4f451b6c9612e42af5ddcbffa14f057329) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr19554.2",     0x400000, 0x400000, CRC(5a3ebcac) SHA1(46e3d1cf515a7ff8a8f97e5050b29dbbeb5060c0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19555.3",     0x800000, 0x400000, CRC(74f6e6b8) SHA1(8080860550eb770e04447e344fb337748a249761) ) // good
ROM_END

ROM_START( cotton2 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20122.7",    0x0200000, 0x0200000, CRC(d616f78a) SHA1(8039dcdfdafb8327a19a1da46a67c0b3f7eee53a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20117.2",    0x0400000, 0x0400000, CRC(893656ea) SHA1(11e3160083ba018fbd588f07061a4e55c1efbebb) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20118.3",    0x0800000, 0x0400000, CRC(1b6a1d4c) SHA1(6b234d6b2d24df7f6d400a56698c0af2f78ce0e7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20119.4",    0x0c00000, 0x0400000, CRC(5a76e72b) SHA1(0a058627ddf78a0bcdaba328a58712419f24e33b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20120.5",    0x1000000, 0x0400000, CRC(7113dd7b) SHA1(f86add67c4e1349a9b9ebcd0145a30b1667df811) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20121.6",    0x1400000, 0x0400000, CRC(8c8fd521) SHA1(c715681330b5ed37a8506ac58ee2143baa721206) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20116.1",    0x1800000, 0x0400000, CRC(d30b0175) SHA1(2da5c3c02d68b8324948a8cdc93946d97fccdd8f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20123.8",    0x1c00000, 0x0400000, CRC(35f1b89f) SHA1(1d6007c380f817def734fc3030d4fe56df4a15be) ) // good
ROM_END

ROM_START( cottonbm )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr21075.7",    0x0200000, 0x0200000, CRC(200b58ba) SHA1(6daad6d70a3a41172e8d9402af775c03e191232d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21070.2",    0x0400000, 0x0400000, CRC(56c0bf1d) SHA1(c2b564ce536c637bb723ed96683b27596e87ebe7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21071.3",    0x0800000, 0x0400000, CRC(2bb18df2) SHA1(e900adb94ad3f48be00a4ce33e915147dc6a8737) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21072.4",    0x0c00000, 0x0400000, CRC(7c7cb977) SHA1(376dfb8014050605b00b6545520bd544768f5828) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21073.5",    0x1000000, 0x0400000, CRC(f2e5a5b7) SHA1(9258d508ef6f6529efc4ad172fd29e69877a99eb) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21074.6",    0x1400000, 0x0400000, CRC(6a7e7a7b) SHA1(a0b1e7a85e623b59886b28797281df1d65b8a5aa) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21069.1",    0x1800000, 0x0400000, CRC(6a28e3c5) SHA1(60454b71db49b872e0cb89fae2259fed601588bd) ) // good
ROM_END

ROM_START( decathlt )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18967a.13",               0x0000001, 0x0100000, CRC(ac59c186) SHA1(7d4924d1e4c1b9257b58a690de988b3f6486e86f) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr18968.2",    0x0400000, 0x0400000, CRC(11a891de) SHA1(1a4fa8d7e07e1d8fdc8122ef8a5b93723c007cda) ) // good (was .1)
	ROM_LOAD16_WORD_SWAP( "mpr18969.3",    0x0800000, 0x0400000, CRC(199cc47d) SHA1(d78f7c6be7e9b43e208244c5c8722245f4c653e1) ) // good (was .2)
	ROM_LOAD16_WORD_SWAP( "mpr18970.4",    0x0c00000, 0x0400000, CRC(8b7a509e) SHA1(8f4d36a858231764ed09b26a1141d1f055eee092) ) // good (was .3)
	ROM_LOAD16_WORD_SWAP( "mpr18971.5",    0x1000000, 0x0400000, CRC(c87c443b) SHA1(f2fedb35c80e5c4855c7aebff88186397f4d51bc) ) // good (was .4)
	ROM_LOAD16_WORD_SWAP( "mpr18972.6",    0x1400000, 0x0400000, CRC(45c64fca) SHA1(ae2f678b9885426ce99b615b7f62a451f9ef83f9) ) // good (was .5)
ROM_END

ROM_START( decathlto )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18967.13",               0x0000001, 0x0100000, CRC(c0446674) SHA1(4917089d95613c9d2a936ed9fe3ebd22f461aa4f) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr18968.2",    0x0400000, 0x0400000, CRC(11a891de) SHA1(1a4fa8d7e07e1d8fdc8122ef8a5b93723c007cda) ) // good (was .1)
	ROM_LOAD16_WORD_SWAP( "mpr18969.3",    0x0800000, 0x0400000, CRC(199cc47d) SHA1(d78f7c6be7e9b43e208244c5c8722245f4c653e1) ) // good (was .2)
	ROM_LOAD16_WORD_SWAP( "mpr18970.4",    0x0c00000, 0x0400000, CRC(8b7a509e) SHA1(8f4d36a858231764ed09b26a1141d1f055eee092) ) // good (was .3)
	ROM_LOAD16_WORD_SWAP( "mpr18971.5",    0x1000000, 0x0400000, CRC(c87c443b) SHA1(f2fedb35c80e5c4855c7aebff88186397f4d51bc) ) // good (was .4)
	ROM_LOAD16_WORD_SWAP( "mpr18972.6",    0x1400000, 0x0400000, CRC(45c64fca) SHA1(ae2f678b9885426ce99b615b7f62a451f9ef83f9) ) // good (was .5)
ROM_END

ROM_START( diehard ) /* must use USA, Europe or Taiwan BIOS */
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr19119.13",               0x0000001, 0x0100000, CRC(de5c4f7c) SHA1(35f670a15e9c86edbe2fe718470f5a75b5b096ac) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr19115.2",    0x0400000, 0x0400000, CRC(6fe06a30) SHA1(dedb90f800bae8fd9df1023eb5bec7fb6c9d0179) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19116.3",    0x0800000, 0x0400000, CRC(af9e627b) SHA1(a53921c3185a93ec95299bf1c29e744e2fa3b8c0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19117.4",    0x0c00000, 0x0400000, CRC(74520ff1) SHA1(16c1acf878664b3bd866c9b94f3695ae892ac12f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19118.5",    0x1000000, 0x0400000, CRC(2c9702f0) SHA1(5c2c66de83f2ccbe97d3b1e8c7e65999e1fa2de1) ) // good
ROM_END

ROM_START( dnmtdeka )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr19114.13",               0x0000001, 0x0100000, CRC(1fd22a5f) SHA1(c3d9653b12354a73a3e15f23a2ab7992ffb83e46) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr19115.2",    0x0400000, 0x0400000, CRC(6fe06a30) SHA1(dedb90f800bae8fd9df1023eb5bec7fb6c9d0179) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19116.3",    0x0800000, 0x0400000, CRC(af9e627b) SHA1(a53921c3185a93ec95299bf1c29e744e2fa3b8c0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19117.4",    0x0c00000, 0x0400000, CRC(74520ff1) SHA1(16c1acf878664b3bd866c9b94f3695ae892ac12f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19118.5",    0x1000000, 0x0400000, CRC(2c9702f0) SHA1(5c2c66de83f2ccbe97d3b1e8c7e65999e1fa2de1) ) // good
ROM_END

ROM_START( ejihon )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18137.13",               0x0000001, 0x0080000, CRC(151aa9bc) SHA1(0959c60f31634816825acb57413838dcddb17d31) )
	ROM_RELOAD( 0x100001, 0x0080000 )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0080000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0080000 )
	ROM_LOAD16_WORD_SWAP( "mpr18138.2",    0x0400000, 0x0400000, CRC(f5567049) SHA1(6eb35e4b5fbda39cf7e8c42b6a568bd53a364d6d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18139.3",    0x0800000, 0x0400000, CRC(f36b4878) SHA1(e3f63c0046bd37b7ab02fb3865b8ebcf4cf68e75) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18140.4",    0x0c00000, 0x0400000, CRC(228850a0) SHA1(d83f7fa7df08407fa45a13661393679b88800805) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18141.5",    0x1000000, 0x0400000, CRC(b51eef36) SHA1(2745cba48dc410d6d31327b956886ec284b9eac3) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18142.6",    0x1400000, 0x0400000, CRC(cf259541) SHA1(51e2c8d16506d6074f6511112ec4b6b44bed4886) ) // good
ROM_END

ROM_START( elandore )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr21307.7",    0x0200000, 0x0200000, CRC(966ad472) SHA1(d6db41d1c40d08eb6bce8a8a2f491e7533daf670) ) // good (was .11s)
	ROM_LOAD16_WORD_SWAP( "mpr21301.2",    0x0400000, 0x0400000, CRC(1a23b0a0) SHA1(f9dbc7ba96dadfb00e5827622b557080449acd83) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr21302.3",    0x0800000, 0x0400000, CRC(1c91ca33) SHA1(ae11209088e3bf8fc4a92dca850d7303ce949b29) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr21303.4",    0x0c00000, 0x0400000, CRC(07b2350e) SHA1(f32f63fd8bec4e667f61da203d63be9a27798dfe) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr21304.5",    0x1000000, 0x0400000, CRC(cfea52ae) SHA1(4b6d27e0b2a95300ee9e07ebcdc4953d77c4efbe) ) // good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr21305.6",    0x1400000, 0x0400000, CRC(46cfc2a2) SHA1(8ca26bf8fa5ced040e815c125c13dd06d599e189) ) // good (was .16)
	ROM_LOAD16_WORD_SWAP( "mpr21306.1",    0x1800000, 0x0400000, CRC(87a5929c) SHA1(b259341d7b0e1fa98959bf52d23db5c308a8efdd) ) // good (was .17)
	ROM_LOAD16_WORD_SWAP( "mpr21308.8",    0x1c00000, 0x0400000, CRC(336ec1a4) SHA1(20d1fce050cf6132d284b91853a4dd5626372ef0) ) // good (was .18s)
ROM_END

ROM_START( ffreveng )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "opr21872.7",   0x0200000, 0x0200000, CRC(32d36fee) SHA1(441c4254ef2e9301e1006d69462a850ce339314b) ) // good (was .11s)
	ROM_LOAD16_WORD_SWAP( "mpr21873.2",   0x0400000, 0x0400000, CRC(dac5bd98) SHA1(6102035ce9eb2f83d7d9b20f989a151f45087c67) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr21874.3",   0x0800000, 0x0400000, CRC(0a7be2f1) SHA1(e2d13f36e54d1e2cb9d584db829c04a6ff65108c) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr21875.4",   0x0c00000, 0x0400000, CRC(ccb75029) SHA1(9611a08a2ad0e0e82137ded6205440a948a339a4) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr21876.5",   0x1000000, 0x0400000, CRC(bb92a7fc) SHA1(d9e0fab1104a46adeb0a0cfc0d070d4c63a28d55) ) // good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr21877.6",   0x1400000, 0x0400000, CRC(c22a4a75) SHA1(3276bc0628e71b432f21ba9a4f5ff7ccc8769cd9) ) // good (was .16)
	ROM_LOAD16_WORD_SWAP( "opr21878.1",   0x1800000, 0x0200000, CRC(2ea4a64d) SHA1(928a973dce5eba0a1628d61ba56a530de990a946) ) // good (was .17)
ROM_END

/* set system to 1 player to test rom */
ROM_START( fhboxers )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fr18541a.13",               0x0000001, 0x0100000, CRC(8c61a17c) SHA1(a8aef27b53482923a506f7daa4b7a38653b4d8a4) ) //(header is read from here, not ic7 even if both are populated on this board)

	ROM_LOAD16_WORD_SWAP( "mpr18538.7",    0x0200000, 0x0200000, CRC(7b5230c5) SHA1(70cebc3281580b43adf42c37318e12159c28a13d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18533.2",    0x0400000, 0x0400000, CRC(7181fe51) SHA1(646f95e1a5b64d721e961352cee6fd5adfd031ec) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18534.3",    0x0800000, 0x0400000, CRC(c87ef125) SHA1(c9ced130faf6dd9e626074b6519615654d8beb19) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18535.4",    0x0c00000, 0x0400000, CRC(929a64cf) SHA1(206dfc2a46befbcea974df1e27515c5759d88d00) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18536.5",    0x1000000, 0x0400000, CRC(51b9f64e) SHA1(bfbdfb73d24f26ce1cc5294c23a1712fb9631691) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18537.6",    0x1400000, 0x0400000, CRC(c364f6a7) SHA1(4db21bcf6ea3e75f9eb34f067b56a417589271c0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18532.1",    0x1800000, 0x0400000, CRC(39528643) SHA1(e35f4c35c9eb13e1cdcc26cb2599bb846f2c1af7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18539.8",    0x1c00000, 0x0400000, CRC(62b3908c) SHA1(3f00e49beb0e5575cc4250a25c41f04dc91d6ed0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18540.9",    0x2000000, 0x0400000, CRC(4c2b59a4) SHA1(4d15503fcff0e9e0d1ed3bac724278102b506da0) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "fhboxers.nv", 0x0000, 0x0080, CRC(590fd6da) SHA1(1abe71c62c51a246d854f02cd4cb79fd0d427e88) )
ROM_END

/* set system to 1 player to test rom */
ROM_START( findlove )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20424.13",               0x0000001, 0x0100000, CRC(4e61fa46) SHA1(e34624d98cbdf2dd04d997167d3c4decd2f208f7) ) //(header is read from here, not ic7 even if both are populated on this board)

	ROM_LOAD16_WORD_SWAP( "mpr20431.7",    0x0200000, 0x0200000, CRC(ea656ced) SHA1(b2d6286081bd46a89d1284a2757b87d0bca1bbde) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20426.2",    0x0400000, 0x0400000, CRC(897d1747) SHA1(f3fb2c4ef8bc2c1658907e822f2ee2b88582afdd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20427.3",    0x0800000, 0x0400000, CRC(a488a694) SHA1(80ec81f32e4b5712a607208b2a45cfdf6d5e1849) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20428.4",    0x0c00000, 0x0400000, CRC(4353b3b6) SHA1(f5e56396b345ff65f57a23f391b77d401f1f58b5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20429.5",    0x1000000, 0x0400000, CRC(4f566486) SHA1(5b449288e33f02f2362ebbd515c87ea11cc02633) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20430.6",    0x1400000, 0x0400000, CRC(d1e11979) SHA1(14405997eefac22c42f0c86dca9411ba1dee9bf9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20425.1",    0x1800000, 0x0400000, CRC(67f104c4) SHA1(8e965d2ce554ba8d37254f6bf3931dff4bce1a43) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20432.8",    0x1c00000, 0x0400000, CRC(79fcdecd) SHA1(df8e7733a51e24196914fc66a024515ee1565599) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20433.9",    0x2000000, 0x0400000, CRC(82289f29) SHA1(fb6a1015621b1afa3913da162ae71ded6b674649) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20434.10",   0x2400000, 0x0400000, CRC(85c94afc) SHA1(dfc2f16614bc499747ea87567a21c86e7bddce45) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20435.11",   0x2800000, 0x0400000, CRC(263a2e48) SHA1(27ef4bf577d240e36dcb6e6a09b9c5f24e59ce8c) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20436.12",   0x2c00000, 0x0400000, CRC(e3823f49) SHA1(754d48635bd1d4fb01ff665bfe2a71593d92f688) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "findlove.nv", 0x0000, 0x0080, CRC(df2fa9f6) SHA1(f05450e1648eafd2ffb52e50769f07bdc75ffb95) )
ROM_END

ROM_START( finlarch )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "finlarch.13",               0x0000001, 0x0100000, CRC(4505fa9e) SHA1(96c6399146cf9c8f1d27a8fb6a265f937258004a) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "mpr18257.2",    0x0400000, 0x0400000, CRC(137fdf55) SHA1(07a02fe531b3707e063498f5bc9749bd1b4cadb3) ) // good
	ROM_RELOAD(                            0x1400000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18258.3",    0x0800000, 0x0400000, CRC(f519c505) SHA1(5cad39314e46b98c24a71f1c2c10c682ef3bdcf3) ) // good
	ROM_RELOAD(                            0x1800000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18259.4",    0x0c00000, 0x0400000, CRC(5cabc775) SHA1(84383a4cbe3b1a9dcc6c140cff165425666dc780) ) // good
	ROM_RELOAD(                            0x1c00000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18260.5",    0x1000000, 0x0400000, CRC(f5b92082) SHA1(806ad85a187a23a5cf867f2f3dea7d8150065b8e) ) // good
	ROM_RELOAD(                            0x2000000, 0x0400000 )
ROM_END


ROM_START( gaxeduel )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr17766.13",               0x0000001, 0x0080000, CRC(a83fcd62) SHA1(4ce77ebaa0e93c6553ad8f7fb87cbdc32433402b) )
	ROM_RELOAD( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0080000 )
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0080000 )
	ROM_LOAD16_WORD_SWAP( "mpr17768.2",    0x0400000, 0x0400000, CRC(d6808a7d) SHA1(83a97bbe1160cb45b3bdcbde8adc0d9bae5ded60) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17769.3",    0x0800000, 0x0400000, CRC(3471dd35) SHA1(24febddfe70984cebc0e6948ad718e0e6957fa82) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17770.4",    0x0c00000, 0x0400000, CRC(06978a00) SHA1(a8d1333a9f4322e28b23724937f595805315b136) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17771.5",    0x1000000, 0x0400000, CRC(aea2ea3b) SHA1(2fbe3e10d3f5a3b3099a7ed5b38b93b6e22e19b8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17772.6",    0x1400000, 0x0400000, CRC(b3dc0e75) SHA1(fbe2790c84466d186ea3e9d41edfcb7afaf54bea) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17767.1",    0x1800000, 0x0400000, CRC(9ba1e7b1) SHA1(f297c3697d2e8ba4476d672267163f91f371b362) ) // good
ROM_END

ROM_START( grdforce )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20844.7",    0x0200000, 0x0200000, CRC(283e7587) SHA1(477fabc27cfe149ad17757e31f10665dcf8c0860) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20839.2",    0x0400000, 0x0400000, CRC(facd4dd8) SHA1(2582894c98b31ab719f1865d4623dad6736dc877) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20840.3",    0x0800000, 0x0400000, CRC(fe0158e6) SHA1(73460effe69fb8f16dd952271542b7803471a599) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20841.4",    0x0c00000, 0x0400000, CRC(d87ac873) SHA1(35b8fa3862e09dca530e9597f983f5a22919cf08) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20842.5",    0x1000000, 0x0400000, CRC(baebc506) SHA1(f5f59f9263956d0c49c729729cf6db31dc861d3b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20843.6",    0x1400000, 0x0400000, CRC(263e49cc) SHA1(67979861ca2784b3ce39d87e7994e6e7351b40e5) ) // good
ROM_END

ROM_START( groovef )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr19820.7",    0x0200000, 0x0100000, CRC(e93c4513) SHA1(f9636529224880c49bd2cc5572bd5bf41dbf911a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19815.2",    0x0400000, 0x0400000, CRC(1b9b14e6) SHA1(b1828c520cb108e2927a23273ebd2939dca52304) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19816.3",    0x0800000, 0x0400000, CRC(83f5731c) SHA1(2f645737f945c59a1a2fabf3b21a761be9e8c8a6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19817.4",    0x0c00000, 0x0400000, CRC(525bd6c7) SHA1(2db2501177fb0b44d0fad2054eddf356c4ea08f2) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19818.5",    0x1000000, 0x0400000, CRC(66723ba8) SHA1(0a8379e46a8f8cab11befeadd9abdf59dba68e27) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19819.6",    0x1400000, 0x0400000, CRC(ee8c55f4) SHA1(f6d86b2c2ab43ec5baefb8ccc25e11af4d82712d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19814.1",    0x1800000, 0x0400000, CRC(8f20e9f7) SHA1(30ff5ad0427208e7265cb996e870c4dc0fbbf7d2) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19821.8",    0x1c00000, 0x0400000, CRC(f69a76e6) SHA1(b7e41f34d8b787bf1b4d587e5d8bddb241c043a8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19822.9",    0x2000000, 0x0200000, CRC(5e8c4b5f) SHA1(1d146fbe3d0bfa68993135ba94ef18081ab65d31) ) // good
ROM_END

ROM_START( hanagumi )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20143.7",    0x0200000, 0x0100000, CRC(7bfc38d0) SHA1(66f223e7ff2b5456a6f4185b7ab36f9cd833351a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20138.2",    0x0400000, 0x0400000, CRC(fdcf1046) SHA1(cbb1f03879833c17feffdd6f5a4fbff06e1059a2) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20139.3",    0x0800000, 0x0400000, CRC(7f0140e5) SHA1(f2f7de7620d66a596d552e1af491a0592ebc4e51) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20140.4",    0x0c00000, 0x0400000, CRC(2fa03852) SHA1(798ce008f6fc24a00f85298188c8d0d01933640d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20141.5",    0x1000000, 0x0400000, CRC(45d6d21b) SHA1(fe0f0b2195b74e79b8efb6a7c0b7bedca7194c48) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20142.6",    0x1400000, 0x0400000, CRC(e38561ec) SHA1(c04c400be033bc74a7bb2a60f6ae00853a2220d4) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20137.1",    0x1800000, 0x0400000, CRC(181d2688) SHA1(950059f89eda30d8a5bce145421f507e226b8b3e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20144.8",    0x1c00000, 0x0400000, CRC(235b43f6) SHA1(e35d9bf15ac805513ab3edeca4f264647a2dc0b0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20145.9",    0x2000000, 0x0400000, CRC(aeaac7a1) SHA1(5c75ecce49a5c53dbb0b07e75f3a76e6db9976d0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20146.10",   0x2400000, 0x0400000, CRC(39bab9a2) SHA1(077132e6a03afd181ee9ca9ca4f7c9cbf418e57e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20147.11",   0x2800000, 0x0400000, CRC(294ab997) SHA1(aeba269ae7d056f07edecf96bc138231c66c3637) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20148.12",   0x2c00000, 0x0400000, CRC(5337ccb0) SHA1(a998bb116eb10c4044410f065c5ddeb845f9dab5) ) // good
ROM_END

ROM_START( introdon )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18937.13",               0x0000001, 0x0080000, CRC(1f40d766) SHA1(35d9751c1b23cfbf448f2a9e9cf3b121929368ae) )
	ROM_RELOAD( 0x0100001, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr18944.7",    0x0200000, 0x0100000, CRC(f7f75ce5) SHA1(0787ece9f89cc1847889adbf08ba5d3ccbc405de) ) // good
	ROM_RELOAD( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr18939.2",    0x0400000, 0x0400000, CRC(ef95a6e6) SHA1(3026c52ad542997d5b0e621b389c0e01240cb486) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18940.3",    0x0800000, 0x0400000, CRC(cabab4cd) SHA1(b251609573c4b0ccc933188f32226855b25fd9da) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18941.4",    0x0c00000, 0x0400000, CRC(f4a33a20) SHA1(bf0f33495fb5c9de4ae5036cedda65b3ece217e8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18942.5",    0x1000000, 0x0400000, CRC(8dd0a446) SHA1(a75e3552b0fb99e0b253c0906f62fabcf204b735) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18943.6",    0x1400000, 0x0400000, CRC(d8702a9e) SHA1(960dd3cb0b9eb1f18b8d0bc0da532b600d583ceb) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18938.1",    0x1800000, 0x0400000, CRC(580ecb83) SHA1(6c59f7da408b53f9fa7aa32c1b53328b5fd6334d) ) // good
ROM_END

/* set system to 1 player to test rom */
ROM_START( kiwames )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18737.13",               0x0000001, 0x0080000, CRC(cfad6c49) SHA1(fc69980a351ed13307706db506c79c774eabeb66) ) // bad
	ROM_RELOAD( 0x0100001, 0x0080000)

	ROM_LOAD16_WORD_SWAP( "mpr18738.2",    0x0400000, 0x0400000, CRC(4b3c175a) SHA1(b6d2438ae1d3d51950a7ed1eaadf2dae45c4e7b1) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18739.3",    0x0800000, 0x0400000, CRC(eb41fa67) SHA1(d12acebb1df9eafd17aff1841087f5017225e7e7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18740.4",    0x0c00000, 0x0200000, CRC(9ca7962f) SHA1(a09e0db2246b34ca7efa3165afbc5ba292a95398) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "kiwames.nv", 0x0000, 0x0080, CRC(c7002732) SHA1(57395d256a58ddcedd354ce1f2c458321be40505) )
ROM_END

ROM_START( maruchan )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20416.13",               0x0000001, 0x0100000, CRC(8bf0176d) SHA1(5bd468e2ffed042ee84e2ceb8712ff5883a1d824) ) // bad
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20417.2",    0x0400000, 0x0400000, CRC(636c2a08) SHA1(47986b71d68f6a1852e4e2b03ca7b6e48e83718b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20418.3",    0x0800000, 0x0400000, CRC(3f0d9e34) SHA1(2ec81e40ebf689d17b6421820bfb0a1280a8ef25) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20419.4",    0x0c00000, 0x0400000, CRC(ec969815) SHA1(b59782174051f5717b06f43e57dd8a2a6910d95f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20420.5",    0x1000000, 0x0400000, CRC(f2902c88) SHA1(df81e137e8aa4bd37e1d14fce4d593cfd14608f0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20421.6",    0x1400000, 0x0400000, CRC(cd0b477c) SHA1(5169cc47fae465b11bc50f5e8410d84c2b2eee42) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20422.1",    0x1800000, 0x0400000, CRC(66335049) SHA1(59f1968001d1e9fe30990a56309bae18033eee62) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20423.8",    0x1c00000, 0x0400000, CRC(2bd55832) SHA1(1a1a510f30882d4d726b594a6541a12c552fafb4) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20443.9",    0x2000000, 0x0400000, CRC(8ac288f5) SHA1(0c08874e6ab2b07b17438721fb535434a626115f) ) // good
ROM_END

/* set system to 1 player to test rom */
ROM_START( myfairld )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr21000.7",    0x0200000, 0x0200000, CRC(2581c560) SHA1(5fb64f0e09583d50dfea7ad613d45aad30b677a5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20995.2",    0x0400000, 0x0400000, CRC(1bb73f24) SHA1(8773654810de760c5dffbb561f43e259b074a61b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20996.3",    0x0800000, 0x0400000, CRC(993c3859) SHA1(93f95e3e080a08961784482607919c1ab3eeb5e5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20997.4",    0x0c00000, 0x0400000, CRC(f0bf64a4) SHA1(f51431f1a736bbc498fa0baa1f8570f89984d9f9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20998.5",    0x1000000, 0x0400000, CRC(d3b19786) SHA1(1933e57272cd68cc323922fa93a9af97dcef8450) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20999.6",    0x1400000, 0x0400000, CRC(82e31f25) SHA1(0cf74af14abb6ede21d19bc22041214232751594) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20994.1",    0x1800000, 0x0400000, CRC(a69243a0) SHA1(e5a1b6ec62bdd5b015ed6cf48f5a6aabaf4bd837) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr21001.8",    0x1c00000, 0x0400000, CRC(95fbe549) SHA1(8cfb48f353b2849600373d66f293f103bca700df) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "myfairld.nv", 0x0000, 0x0080, CRC(c7cf3a5a) SHA1(5365f38047821305658f94395f03cf2c49c87576) )
ROM_END

ROM_START( othellos )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20967.7",    0x0200000, 0x0200000, CRC(efc05b97) SHA1(a533366c3aaba90dcac8f3654db9ad902efca258) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20963.2",    0x0400000, 0x0400000, CRC(2cc4f141) SHA1(8bd1998aff8615b34d119fab3637a08ed6e8e1e4) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20964.3",    0x0800000, 0x0400000, CRC(5f5cda94) SHA1(616be219a2512e80c875eddf05137c23aedf6f65) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20965.4",    0x0c00000, 0x0400000, CRC(37044f3e) SHA1(cbc071554cfd8bb12a337c04b169de6c6309c3ab) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20966.5",    0x1000000, 0x0400000, CRC(b94b83de) SHA1(ba1b3135d0ad057f0786f94c9d06b5e347bedea8) ) // good
ROM_END

ROM_START( pblbeach )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18852.13",               0x0000001, 0x0080000, CRC(d12414ec) SHA1(0f42ec9e41983781b6892622b00398a102072aa7) ) // bad
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr18853.2",    0x0400000, 0x0400000, CRC(b9268c97) SHA1(8734e3f0e6b2849d173e3acc9d0308084a4e84fd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18854.3",    0x0800000, 0x0400000, CRC(3113c8bc) SHA1(4e4600646ddd1978988d27430ffdf0d1d405b804) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18855.4",    0x0c00000, 0x0400000, CRC(daf6ad0c) SHA1(2a14a6a42e4eb68abb7a427e43062dfde2d13c5c) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18856.5",    0x1000000, 0x0400000, CRC(214cef24) SHA1(f62b462170b377cff16bb6c6126cbba00b013a87) ) // good
ROM_END

ROM_START( prikura )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr19337.7",    0x0200000, 0x0200000, CRC(76f69ff3) SHA1(5af2e1eb3288d70c2a1c71d0b6370125d65c7757) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19333.2",    0x0400000, 0x0400000, CRC(eb57a6a6) SHA1(cdacaa7a2fb1a343195e2ac5fd02eabf27f89ccd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19334.3",    0x0800000, 0x0400000, CRC(c9979981) SHA1(be491a4ac118d5025d6a6f2d9267a6d52f21d2b6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19335.4",    0x0c00000, 0x0400000, CRC(9e000140) SHA1(9b7dc3dc7f9dc048d2fcbc2b44ae79a631ceb381) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19336.5",    0x1000000, 0x0400000, CRC(2363fa4b) SHA1(f45e53352520be4ea313eeab87bcab83f479d5a8) ) // good
ROM_END

ROM_START( puyosun )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr19531.13",               0x0000001, 0x0080000, CRC(ac81024f) SHA1(b22c7c1798fade7ae992ff83b138dd23e6292d3f) ) // bad
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr19533.2",    0x0400000, 0x0400000, CRC(17ec54ba) SHA1(d4cdc86926519291cc78980ec513e1cfc677e76e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19534.3",    0x0800000, 0x0400000, CRC(820e4781) SHA1(7ea5626ad4e1929a5ec28a99ec12bc364df8f70d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19535.4",    0x0c00000, 0x0400000, CRC(94fadfa4) SHA1(a7d0727cf601e00f1ea31e6bf3e591349c3f6030) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19536.5",    0x1000000, 0x0400000, CRC(5765bc9c) SHA1(b217c292e7cc8ed73a39a3ae7009bc9dd031e376) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19537.6",    0x1400000, 0x0400000, CRC(8b736686) SHA1(aec347c0f3e5dd8646e85f68d71ca9acc3bf62c3) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19532.1",    0x1800000, 0x0400000, CRC(985f0c9d) SHA1(de1ad42ef3cf3f4f071e9801696407be7ae29d21) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19538.8",    0x1c00000, 0x0400000, CRC(915a723e) SHA1(96480441a69d6aad3887ed6f46b0a6bebfb752aa) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19539.9",    0x2000000, 0x0400000, CRC(72a297e5) SHA1(679987e62118dd1bf7c074f4b88678e1a1187437) ) // good
ROM_END

ROM_START( rsgun )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr20958.7",   0x0200000, 0x0200000, CRC(cbe5a449) SHA1(b4744ab71ccbadda1921ba43dd1148e57c0f84c5) ) // good (was .11s)
	ROM_LOAD16_WORD_SWAP( "mpr20959.2",   0x0400000, 0x0400000, CRC(a953330b) SHA1(965274a7297cb88e281fcbdd3ec5025c6463cc7b) ) // good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr20960.3",   0x0800000, 0x0400000, CRC(b5ab9053) SHA1(87c5d077eb1219c35fa65b4e11d5b62e826f5236) ) // good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr20961.4",   0x0c00000, 0x0400000, CRC(0e06295c) SHA1(0ec2842622f3e9dc5689abd58aeddc7e5603b97a) ) // good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr20962.5",   0x1000000, 0x0400000, CRC(f1e6c7fc) SHA1(0ba0972f1bc7c56f4e0589d3e363523cea988bb0) ) // good (was .15)
ROM_END

ROM_START( sandor )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "sando-r.13",               0x0000001, 0x0100000, CRC(fe63a239) SHA1(01502d4494f968443581cd2c74f25967d41f775e) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr18635.8",   0x1c00000, 0x0400000, CRC(441e1368) SHA1(acb2a7e8d44c2203b8d3c7a7b70e20ffb120bebf) ) // good
	ROM_RELOAD(                           0x0400000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18636.9",   0x2000000, 0x0400000, CRC(fff1dd80) SHA1(36b8e1526a4370ae33fd4671850faf51c448bca4) ) // good
	ROM_RELOAD(                           0x0800000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18637.10",  0x2400000, 0x0400000, CRC(83aced0f) SHA1(6cd1702b9c2655dc4f56c666607c333f62b09fc0) ) // good
	ROM_RELOAD(                           0x0c00000, 0x0400000 )
	ROM_LOAD16_WORD_SWAP( "mpr18638.11",  0x2800000, 0x0400000, CRC(caab531b) SHA1(a77bdcc27d183896c0ed576eeebcc1785d93669e) ) // good
	ROM_RELOAD(                           0x1000000, 0x0400000 )
ROM_END


/*
Treasure Hunt
Deniam (Licensed to Sega Enterprises, Ltd), 1997

PCB Number: LEX-0704

This is a non-Sega-manufactured STV cart which works with Japanese and USA bioses.
The cart is mostly the same as the Sega carts, containing not a lot except some ROMs
and logic chips.

On the top side, there are two 27C040 EPROMs and positions for 5 maskROMs, but only 4 of
them are populated. In between the two eproms is an unpopulated position for a TSOP40 flashROM.
On the bottom are locations for 5 maskROMs (none are populated) and also some logic ICs.
*/

ROM_START( thunt )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "th-ic7_2.stv",    0x0200000, 0x0080000, CRC(c4e993de) SHA1(7aa433bc2623cb19a09d4ef4c8233a2d29901020) )
	ROM_LOAD16_BYTE( "th-ic7_1.stv",    0x0200001, 0x0080000, CRC(1355cc18) SHA1(a9b731228a807b2b01f933fe0f7dcdbadaf89b7e) )

	ROM_LOAD16_WORD_SWAP( "th-e-2.ic2",   0x0400000, 0x0400000, CRC(47315694) SHA1(4a7cc195b98dbca146f5efe3b5b35be0f13b2f4a) )
	ROM_LOAD16_WORD_SWAP( "th-e-3.ic3",   0x0800000, 0x0400000, CRC(c9290b44) SHA1(4e51e667edf330cc20e600cf1500f332cc533e20) )
	ROM_LOAD16_WORD_SWAP( "th-e-4.ic4",   0x0c00000, 0x0400000, CRC(c672e40b) SHA1(4ba189de7b8fb5ca45fb15c89cdaf0f860831d52) )
	ROM_LOAD16_WORD_SWAP( "th-e-5.ic5",   0x1000000, 0x0400000, CRC(3914b805) SHA1(1331ce82ba0bdfc76fe3456a5252e69c00e2cf1f) )
ROM_END

ROM_START( thuntk )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "2.ic13_2",    0x0200000, 0x0080000, CRC(6cae2926) SHA1(e8d5745b4228de24672da5017cb3dab58344f59f) )
	ROM_LOAD16_BYTE( "1.ic13_1",    0x0200001, 0x0080000, CRC(460727c8) SHA1(da7171b65734264e10692e3408ac93beb374c65e) )

	ROM_LOAD( "bom210-10.ic2",   0x1c00000, 0x0400000, CRC(f59d0827) SHA1(2bed4b2c78e9b4e9332f576e1b264a6343f4cfff) )
	ROM_RELOAD(                  0x0400000, 0x0400000 )
	ROM_LOAD( "bom210-11.ic3",   0x2000000, 0x0400000, CRC(44e5a13e) SHA1(aee3c06662a1d083f3bd01292cf2694132f63533) )
	ROM_RELOAD(                  0x0800000, 0x0400000 )
	ROM_LOAD( "bom210-12.ic4",   0x2400000, 0x0400000, CRC(deabc701) SHA1(cb313ae9bf6f115682fc76647a999e12c98f6120) )
	ROM_RELOAD(                  0x0c00000, 0x0400000 )
	ROM_LOAD( "bom210-13.ic5",   0x2800000, 0x0400000, CRC(5ece1d5c) SHA1(6d88f71b485bf2b3c164fa22f1c7ecaba4b3f5b1) )
	ROM_RELOAD(                  0x1000000, 0x0400000 )
ROM_END


ROM_START( sanjeon )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "ic11",               0x0000001, 0x0200000, CRC(9abae8d4) SHA1(ddbe4c8fff8fa59d63e278e95f245145d2da8aeb) )

	ROM_LOAD( "ic13",    0x0400000, 0x0200000, CRC(f72c1d13) SHA1(a2b168d187034024b83fbbe2f5eec78816285da9) ) // ic2 good
	ROM_LOAD( "ic14",    0x0600000, 0x0200000, CRC(bcd72105) SHA1(fb88a1f2589fef5d9845027646259ec8e781771b) ) // ic2 good
	ROM_LOAD( "ic15",    0x0800000, 0x0200000, CRC(8c9c8352) SHA1(312e9e6435c6e9a4a19b72e234b087d4f5b069a5) ) // ic3 good
	ROM_LOAD( "ic16",    0x0a00000, 0x0200000, CRC(07e11512) SHA1(5fb5e93a167aae501bd799aba98f7dbc3762a795) ) // ic3 good
	ROM_LOAD( "ic17",    0x0c00000, 0x0200000, CRC(46b7b344) SHA1(a325b724e6346f585582bb2581d3063b1c8ddee9) ) // ic4 good
	ROM_LOAD( "ic18",    0x0e00000, 0x0200000, CRC(d48404e1) SHA1(e4927d17cb19f6825f41b41056b89de224cf97de) ) // ic4 good
	ROM_LOAD( "ic19",    0x1000000, 0x0200000, CRC(33d23bb9) SHA1(0179438a0b14ad7eb553a24d86fae778671fcc49) ) // ic5 good
	ROM_LOAD( "ic20",    0x1200000, 0x0200000, CRC(f8cc1038) SHA1(06bcb25eb5df77bd503b0692a96ec66c00976fe9) ) // ic5 good
	ROM_LOAD( "ic21",    0x1400000, 0x0200000, CRC(74ceb649) SHA1(91b7eb699b7167f890528149178644a74ef3ad17) ) // ic6 good
	ROM_LOAD( "ic22",    0x1600000, 0x0200000, CRC(85f31277) SHA1(277c28d44cabd9081a8151d6ec27d47b5606435a) ) // ic6 good
	ROM_LOAD( "ic12",    0x1800000, 0x0400000, CRC(d5ebc84e) SHA1(f990b793cdfadbbac69c680191660f4a2f282ba2) ) // ic1 good
ROM_END


ROM_START( sasissu )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20542.13",               0x0000001, 0x0100000, CRC(0e632db5) SHA1(9bc52794892eec22d381387d13a0388042e30714) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20544.2",    0x0400000, 0x0400000, CRC(661fff5e) SHA1(41f4ddda7adf004b52cc9a076606a60f31947d19) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20545.3",    0x0800000, 0x0400000, CRC(8e3a37be) SHA1(a3227cdc4f03bb088e7f9aed225b238da3283e01) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20546.4",    0x0c00000, 0x0400000, CRC(72020886) SHA1(e80bdeb11b726eb23f2283950d65d55e31a5672e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20547.5",    0x1000000, 0x0400000, CRC(8362e397) SHA1(71f13689a60572a04b91417a9a48adfd3bd0f5dc) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20548.6",    0x1400000, 0x0400000, CRC(e37534d9) SHA1(79988cbb1537ca99fdd0288a86564fe1f714d052) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20543.1",    0x1800000, 0x0400000, CRC(1f688cdf) SHA1(a90c1011119adb50e0d9d5cd3d7616a307b2d7e8) ) // good
ROM_END

/* set to 1 player to test */
ROM_START( seabass )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "seabassf.13",               0x0000001, 0x0100000, CRC(6d7c39cc) SHA1(d9d1663134420b75c65ee07d7d547254785f2f83) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20551.2",    0x0400000, 0x0400000, CRC(9a0c6dd8) SHA1(26600372cc673ce3678945f4b5dc4e3ab31643a4) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20552.3",    0x0800000, 0x0400000, CRC(5f46b0aa) SHA1(1aa576b15971c0ffb4e08d4802246841b31b6f35) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20553.4",    0x0c00000, 0x0400000, CRC(c0f8a6b6) SHA1(2038b9231a950450267be0db24b31d8035db79ad) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20554.5",    0x1000000, 0x0400000, CRC(215fc1f9) SHA1(f042145622ba4bbbcce5f050a4c9eae42cb7adcd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20555.6",    0x1400000, 0x0400000, CRC(3f5186a9) SHA1(d613f307ab150a7eae358aa449206af05db5f9d7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20550.1",    0x1800000, 0x0400000, CRC(083e1ca8) SHA1(03944dd8fe86f305ca4bd2d71e2140e03798ffc9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20556.8",    0x1c00000, 0x0400000, CRC(1fd70c6c) SHA1(d9d2e362d13238216f4f7e10095fb8383bbd91e8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20557.9",    0x2000000, 0x0400000, CRC(3c9ba442) SHA1(2e5b795cf4cdc11ab3e4887b2f77c7147c6e3eec) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "seabass.nv", 0x0000, 0x0080, CRC(4e7c0944) SHA1(dbba78f6a7f3e7d12d5c4fff36117db82f5f9c01) )
ROM_END

ROM_START( shanhigw )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr18341.7",    0x0200000, 0x0200000, CRC(cc5e8646) SHA1(a733616c118140ff3887d30d595533f9a1beae06) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18340.2",    0x0400000, 0x0200000, CRC(8db23212) SHA1(85d604a5c6ab97188716dbcd77d365af12a238fe) ) // good
ROM_END

ROM_START( shienryu )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr19631.7",    0x0200000, 0x0200000, CRC(3a4b1abc) SHA1(3b14b7fdebd4817da32ea374c15a38c695ffeff1) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19632.2",    0x0400000, 0x0400000, CRC(985fae46) SHA1(f953bde91805b97b60d2ab9270f9d2933e064d95) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19633.3",    0x0800000, 0x0400000, CRC(e2f0b037) SHA1(97861d09e10ce5d2b10bf5559574b3f489e28077) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-shienryu.bin", 0x0000, 0x0080, CRC(98db6925) SHA1(e78545e8f62d19f8e00197c62ff0e56f6c85e355) )
ROM_END

ROM_START( smleague ) /* only runs with the USA bios */
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr18777.13",               0x0000001, 0x0080000, CRC(8d180866) SHA1(d47ebabab6e06400312d39f68cd818852e496b96) )
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr18778.8",    0x1c00000, 0x0400000, CRC(25e1300e) SHA1(64f3843f62cee34a47244ad5ee78fb2aa35289e3) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18779.9",    0x2000000, 0x0400000, CRC(51e2fabd) SHA1(3aa361149af516f16d7d422596ee82014a183c2b) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18780.10",   0x2400000, 0x0400000, CRC(8cd4dd74) SHA1(9ffec1280b3965d52f643894bdfecdd792028191) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18781.11",   0x2800000, 0x0400000, CRC(13ee41ae) SHA1(cdbaeac4c90b5ee84233c299612f7f28280a6ba6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18782.12",   0x2c00000, 0x0200000, CRC(9be2270a) SHA1(f2de5cd6b269f123305e30bed2b474019e4f05b8) ) // good
ROM_END

ROM_START( sokyugrt )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr19188.13",               0x0000001, 0x0100000, CRC(45a27e32) SHA1(96e1bab8bdadf7071afac2a0a6dd8fd8989f12a6) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr19189.2",    0x0400000, 0x0400000, CRC(0b202a3e) SHA1(6691b5af2cacd6092ec03886b78c2565953fa297) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19190.3",    0x0800000, 0x0400000, CRC(1777ded8) SHA1(dd332ac79f0a6d82b6bde35b795b2845003dd1a5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19191.4",    0x0c00000, 0x0400000, CRC(ec6eb07b) SHA1(01fe4832ece8638ea6f4060099d9105fe8092c88) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19192.5",    0x1000000, 0x0200000, CRC(cb544a1e) SHA1(eb3ba9758487d0e8c4bbfc41453fe35b35cce3bf) ) // good
ROM_END

/* set to 1 player to test */
ROM_START( sss )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr21488.13",               0x0000001, 0x0080000, CRC(71c9def1) SHA1(a544a0b4046307172d2c1bf426ed24845f87d894) )
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
	ROM_LOAD16_WORD_SWAP( "mpr21489.2",    0x0400000, 0x0400000, CRC(4c85152b) SHA1(78f2f1c31718d5bf631d8813daf9a11ea2a0e451) ) // ic2 good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr21490.3",    0x0800000, 0x0400000, CRC(03da67f8) SHA1(02f9ba7549ca552291dc0ff1b631103015838bba) ) // ic3 good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr21491.4",    0x0c00000, 0x0400000, CRC(cf7ee784) SHA1(af823df2d60d8ef3d17628b95a04136b807ca095) ) // ic4 good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr21492.5",    0x1000000, 0x0400000, CRC(57753894) SHA1(5c51167c158443d02a53d724a5ceb73055876c06) ) // ic5 good (was .15)
	ROM_LOAD16_WORD_SWAP( "mpr21493.6",    0x1400000, 0x0400000, CRC(efb2d271) SHA1(a591e48206704fbda5fef3ce69ad279da1017ed6) ) // ic6 good (was .16)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "sss.nv", 0x0000, 0x0080, CRC(3473b2f3) SHA1(6480b4b321af8ee6e967710e74f2556c17bfca97) )
ROM_END

ROM_START( suikoenb )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr17834.13",               0x0000001, 0x0100000, CRC(746ef686) SHA1(e31c317991a687662a8a2a45aed411001e5f1941) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr17836.2",    0x0400000, 0x0400000, CRC(55e9642d) SHA1(5198291cd1dce0398eb47760db2c19eae99273b0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17837.3",    0x0800000, 0x0400000, CRC(13d1e667) SHA1(cd513ceb33cc20032090113b61227638cf3b3998) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17838.4",    0x0c00000, 0x0400000, CRC(f9e70032) SHA1(8efdbcce01bdf77acfdb293545c59bf224a9c7d2) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17839.5",    0x1000000, 0x0400000, CRC(1b2762c5) SHA1(5c7d5fc8a4705249a5b0ea64d51dc3dc95d723f5) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17840.6",    0x1400000, 0x0400000, CRC(0fd4c857) SHA1(42caf22716e834d59e60d45c24f51d95734e63ae) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17835.1",    0x1800000, 0x0400000, CRC(77f5cb43) SHA1(a4f54bc08d73a56caee5b26bea06360568655bd7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17841.8",    0x1c00000, 0x0400000, CRC(f48beffc) SHA1(92f1730a206f4a0abf7fb0ee1210e083a464ad70) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17842.9",    0x2000000, 0x0400000, CRC(ac8deed7) SHA1(370eb2216b8080d3ddadbd32804db63c4ebac76f) ) // good
ROM_END

ROM_START( twcup98 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20819.13",    0x0000001, 0x0100000, CRC(d930dfc8) SHA1(f66cc955181720661a0334fe67fa5750ddf9758b) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20821.2",    0x0400000, 0x0400000, CRC(2d930d23) SHA1(5fcaf4257f3639cb3aa407d2936f616499a09d97) ) // ic2 good (was .12)
	ROM_LOAD16_WORD_SWAP( "mpr20822.3",    0x0800000, 0x0400000, CRC(8b33a5e2) SHA1(d5689ac8aad63509febe9aa4077351be09b2d8d4) ) // ic3 good (was .13)
	ROM_LOAD16_WORD_SWAP( "mpr20823.4",    0x0c00000, 0x0400000, CRC(6e6d4e95) SHA1(c387d03ba27580c62ac0bf780915fdf41552df6f) ) // ic4 good (was .14)
	ROM_LOAD16_WORD_SWAP( "mpr20824.5",    0x1000000, 0x0400000, CRC(4cf18a25) SHA1(310961a5f114fea8938a3f514dffd5231e910a5a) ) // ic5 good (was .15)
ROM_END

ROM_START( vfkids )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr18914.13",               0x0000001, 0x0100000, CRC(cd35730a) SHA1(645b52b449766beb740ab8f99957f8f431351ceb) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr18916.4",    0x0c00000, 0x0400000, CRC(4aae3ddb) SHA1(b75479e73f1bce3f0c27fbd90820fa51eb1914a6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18917.5",    0x1000000, 0x0400000, CRC(edf6edc3) SHA1(478e958f4f10a8126a00c83feca4a55ad6c25503) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18918.6",    0x1400000, 0x0400000, CRC(d3a95036) SHA1(e300bbbb71fb06027dc539c9bbb12946770ffc95) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18915.1",    0x1800000, 0x0400000, CRC(09cc38e5) SHA1(4dfe0e2f21f746020ec557e62487aa7558cbc1fd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18919.8",    0x1c00000, 0x0400000, CRC(4ac700de) SHA1(b1a8501f1683de380dfa49c9cabbe28bd70a5b26) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18920.9",    0x2000000, 0x0400000, CRC(0106e36c) SHA1(f7c30dc9fedb9da079dd7d52fdecbeb8721c5dee) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18921.10",   0x2400000, 0x0400000, CRC(c23d51ad) SHA1(0169b7e2df84e8caa2b349843bd0673f6de2195f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18922.11",   0x2800000, 0x0400000, CRC(99d0ab90) SHA1(e9c82a826cc76ffbe2423913645cf5d5ba2506d6) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr18923.12",   0x2c00000, 0x0400000, CRC(30a41ae9) SHA1(78a3d88b5e6cf669b660460ac967daf408038883) ) // good
ROM_END

ROM_START( vfremix )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr17944.13",               0x0000001, 0x0100000, CRC(a5bdc560) SHA1(d3830480a611b7d88760c672ce46a2ea74076487) )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr17946.2",    0x0400000, 0x0400000, CRC(4cb245f7) SHA1(363d9936b27043b5858c956a45736ac05aefc54e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17947.3",    0x0800000, 0x0400000, CRC(fef4a9fb) SHA1(1b4bd095962db769da17d3644df10f62d041e914) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17948.4",    0x0c00000, 0x0400000, CRC(3e2b251a) SHA1(be6191c18727d7cbc6399fd4c1aaae59304af30c) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17949.5",    0x1000000, 0x0400000, CRC(b2ecea25) SHA1(320c0e7ce34e81e2fe6400cbeb2cb3ca74426cc8) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17950.6",    0x1400000, 0x0400000, CRC(5b1f981d) SHA1(693b5744d210a2ac8b77e7c8c87f07ca859f8aed) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr17945.1",    0x1800000, 0x0200000, CRC(03ede188) SHA1(849c7fab5b97e043fea3deb8df6cc195ccced0e0) ) // good
ROM_END

/* set to 1 player to test */
ROM_START( vmahjong )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr19620.7",    0x0200000, 0x0200000, CRC(c98de7e5) SHA1(5346f884793bcb080aa01967e91b54ced4a9802f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19615.2",    0x0400000, 0x0400000, CRC(c62896da) SHA1(52a5b10ca8af31295d2d700349eca038c418b522) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19616.3",    0x0800000, 0x0400000, CRC(f62207c7) SHA1(87e60183365c6f7e62c7a0667f88df0c7f5457fd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19617.4",    0x0c00000, 0x0400000, CRC(ab667e19) SHA1(2608a567888fe052753d0679d9a831d7706dbc86) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19618.5",    0x1000000, 0x0400000, CRC(9782ceee) SHA1(405dd42706416e128b1e2fde225b5343e9330092) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19619.6",    0x1400000, 0x0400000, CRC(0b76866c) SHA1(10add2993dfe9daf757ec2ff8675390081a93c0a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19614.1",    0x1800000, 0x0400000, CRC(b83b3f03) SHA1(e5a5919ee74964633eaaf4af2fe04c38604ccf16) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr19621.8",    0x1c00000, 0x0400000, CRC(f92616b3) SHA1(61a9dda92a86a02d027260e11b1bad3b0dda9f02) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "vmahjong.nv", 0x0000, 0x0080, CRC(4e6487f4) SHA1(d6d930ab5f21b8c4f42812d08b3ee90f2bc94081) )
ROM_END

ROM_START( winterht )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "fpr20108.13",    0x0000001, 0x0100000, CRC(1ef9ced0) SHA1(abc90ce341cd17bb77349d611d6879389611f0bf) ) // bad
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20110.2",    0x0400000, 0x0400000, CRC(238ef832) SHA1(20fade5730ff8e249a1450c41bfdff6e133f4768) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20111.3",    0x0800000, 0x0400000, CRC(b0a86f69) SHA1(e66427f70413ad43fccc38423962c5eeda01094f) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20112.4",    0x0c00000, 0x0400000, CRC(3ba2b49b) SHA1(5ad154a8b774075479d791e29cbaf221d47557fc) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20113.5",    0x1000000, 0x0400000, CRC(8c858b41) SHA1(d05d2980363c8440863fe2fdb39274de246bd4b9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20114.6",    0x1400000, 0x0400000, CRC(b723862c) SHA1(1e0a08669f16fc4cb647124e0c215233ccb98e5a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20109.1",    0x1800000, 0x0400000, CRC(c1a713b8) SHA1(a7fefa6e9a1e3aecff5ead41da6fd3aec2ef502a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20115.8",    0x1c00000, 0x0400000, CRC(dd01f2ad) SHA1(3bb48dc8670d9460fea2a67400ddb573472c2f4f) ) // good
ROM_END

ROM_START( znpwfv )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr20398.13",    0x0000001, 0x0100000, CRC(3fb56a0b) SHA1(13c2fa2d94b106d39e46f71d15fbce3607a5965a) ) // bad
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr20400.2",    0x0400000, 0x0400000, CRC(1edfbe05) SHA1(b0edd3f3d57408101ae6eb0aec742afbb4d289ca) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20401.3",    0x0800000, 0x0400000, CRC(99e98937) SHA1(e1b4d12a0b4d0fe97a62fcc085e19cce77657c99) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20402.4",    0x0c00000, 0x0400000, CRC(4572aa60) SHA1(8b2d76ea8c6e2f472c6ee7c9b6ad6e80e6a1a85a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20403.5",    0x1000000, 0x0400000, CRC(26a8e13e) SHA1(07f5564b704598e3c3580d3d620ecc4f14549dbd) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20404.6",    0x1400000, 0x0400000, CRC(0b70275d) SHA1(47b8672e19c698dc948760f7091f4c6280e728d0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20399.1",    0x1800000, 0x0400000, CRC(c178a96e) SHA1(65f4aa05187d48ba8ad4fe75ff6ffe1f8524831d) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20405.8",    0x1c00000, 0x0400000, CRC(f53337b7) SHA1(09a21f81016ee54f10554ae1f790415d7436afe0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20406.9",    0x2000000, 0x0400000, CRC(b677c175) SHA1(d0de7b5a29928036df0bdfced5a8021c0999eb26) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr20407.10",   0x2400000, 0x0400000, CRC(58356050) SHA1(f8fb5a14f4ec516093c785891b05d55ae345754e) ) // good
ROM_END

ROM_START( danchih )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "mpr21974.7",    0x0200000, 0x0200000, CRC(e7472793) SHA1(11b7b11cf492eb9cf69b50e7cfac46a5b86849ac) )// good
	ROM_LOAD16_WORD_SWAP( "mpr21970.2",    0x0400000, 0x0400000, CRC(34dd7f4d) SHA1(d5c45da94ec5b6584049caf09516f1ad4ba3adb5) )// good
	ROM_LOAD16_WORD_SWAP( "mpr21971.3",    0x0800000, 0x0400000, CRC(8995158c) SHA1(fbbd171d67eebf43630d6054bc1b9132f6b38183) )// good
	ROM_LOAD16_WORD_SWAP( "mpr21972.4",    0x0c00000, 0x0400000, CRC(68a39090) SHA1(cff1b909c4191660570012eb5e4cb6a7467bc79e) )// good
	ROM_LOAD16_WORD_SWAP( "mpr21973.5",    0x1000000, 0x0400000, CRC(b0f23f14) SHA1(4e7076c29fd57bb3ef9af50a6104e39ecda94e06) )// good
ROM_END

ROM_START( danchiq )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(e216bfc8) SHA1(7a08fa32281e272dbf5e7daea50a1800cc225c1b) )//ic 7
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(b95aa5ac) SHA1(2766c5414643034a0f6d746050557516bd3753df) )//ic 2
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(df6ebd48) SHA1(fcccafbee1b8b952b07ed0e7e86219eed9cf4a93) )
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(cf6a2b76) SHA1(1f7522d446d57b78d099bae553133d5e7e54ff70) )//ic 3
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(0b6a9901) SHA1(b4c335199d3e49a9ae5d474b10130abc4718cdf9) )
	ROM_LOAD16_WORD_SWAP( "ic32",    0x0c00000, 0x0200000, CRC(0b4604f5) SHA1(547cba4a80baf126e87f87529aa933587643d359) )//ic 4
	ROM_LOAD16_WORD_SWAP( "ic34",    0x0e00000, 0x0200000, CRC(616e20fa) SHA1(45c175e79b5701db9726d157ff92eee368f4bbf9) )
	ROM_LOAD16_WORD_SWAP( "ic36",    0x1000000, 0x0200000, CRC(43474e08) SHA1(30b3ede287d5de93c6e0219bfd0a5d7ed5b6a958) )//ic 5
	ROM_LOAD16_WORD_SWAP( "ic23",    0x1200000, 0x0200000, CRC(d080eb71) SHA1(9c39b887697c8872f0cb655cff24282ad3b90e9b) )
	ROM_LOAD16_WORD_SWAP( "ic25",    0x1400000, 0x0200000, CRC(9a4109e5) SHA1(ba59caac5f5a80fc52c507d8a47f322a380aa9a1) )//(Untested)
ROM_END

ROM_START( mausuke )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE(             "ic13.bin",      0x0000001, 0x0100000, CRC(b456f4cd) SHA1(91cbe703ec7c1dd45eb3b05bdfeb06e3570599d1) )
	ROM_RELOAD_PLAIN ( 0x0200000, 0x0100000 ) // needs the rom mapped here to appear 'normal'
	ROM_RELOAD_PLAIN ( 0x0300000, 0x0100000 )

	ROM_LOAD16_WORD_SWAP( "mcj-00.2",      0x0400000, 0x0200000, CRC(4eeacd6f) SHA1(104ca230f22cd11cc536b34abd482e54791b4d0f) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-01.3",      0x0800000, 0x0200000, CRC(365a494b) SHA1(29713dfc83a9ade63ebcc7994d14cd785c4500b9) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-02.4",      0x0c00000, 0x0200000, CRC(8b8e4931) SHA1(0c94e2ccb72902d7786d1101a3958504f7151077) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-03.5",      0x1000000, 0x0200000, CRC(9015a0e7) SHA1(8ba8a3723267e631169dc1e06620260fbccce4bd) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-04.6",      0x1400000, 0x0200000, CRC(9d1beaee) SHA1(c63b61378860319fff2e605c7b9afaa5f1bc4cd2) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-05.1",      0x1800000, 0x0200000, CRC(a7626a82) SHA1(c12a099132c5b9234a2de5674f3b8ba5fdd35289) )// good
	ROM_LOAD16_WORD_SWAP( "mcj-06.8",      0x1c00000, 0x0200000, CRC(1ab8e90e) SHA1(8e22f03c1791a983eb330b2a9199e5349a0b1baa) )// good
ROM_END

/* acclaim game, not a standard cart ... */
ROM_START( batmanfr )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	/* Many thanks to Runik to point this out*/
	ROM_LOAD16_BYTE( "350-mpa1.u19",    0x0000000, 0x0100000, CRC(2a5a8c3a) SHA1(374ec55a39ea909cc672e4a629422681d1f2da05) )
	ROM_RELOAD(                         0x0200000, 0x0100000 )
	ROM_LOAD16_BYTE( "350-mpa1.u16",    0x0000001, 0x0100000, CRC(735e23ab) SHA1(133e2284a07a611aed8ada2707248f392f4509aa) )
	ROM_RELOAD(                         0x0200001, 0x0100000 )
	ROM_LOAD16_WORD_SWAP( "gfx0.u1",    0x0400000, 0x0400000, CRC(a82d0b7e) SHA1(37a7a177634d51620b1b43e58732987df166c7e6) )
	ROM_LOAD16_WORD_SWAP( "gfx1.u3",    0x0800000, 0x0400000, CRC(a41e55d9) SHA1(b896d3a6c36d325c3cece699da54f340a4512703) )
	ROM_LOAD16_WORD_SWAP( "gfx2.u5",    0x0c00000, 0x0400000, CRC(4c1ebeb7) SHA1(cdd139652d9484ae5837a39c2fd48d0a8d966d43) )
	ROM_LOAD16_WORD_SWAP( "gfx3.u8",    0x1000000, 0x0400000, CRC(f679a3e7) SHA1(db11b033b8bbdd80b81e3bc098bd40ad3a8784f2) )
	ROM_LOAD16_WORD_SWAP( "gfx4.u12",   0x1400000, 0x0400000, CRC(52d95242) SHA1(b554a95933c2be4c72fb4226d3bc4775695da2c1) )
	ROM_LOAD16_WORD_SWAP( "gfx5.u15",   0x1800000, 0x0400000, CRC(e201f830) SHA1(5aa22fcc8f2e153d1abc3aa4050c594b3942ee67) )
	ROM_LOAD16_WORD_SWAP( "gfx6.u18",   0x1c00000, 0x0400000, CRC(c6b381a3) SHA1(46431f1e47c084a0bf85535d35af27471653b008) )

	/* it also has an extra adsp sound board, i guess this isn't tested */
	ROM_REGION( 0x080000, "wave", 0 ) /* Wave data */
	ROM_LOAD( "350snda1.u52",   0x000000, 0x080000, CRC(9027e7a0) SHA1(678df530838b078964a044ce734776f391654e6c) )

	ROM_REGION( 0x800000, "adsp_code", 0 ) /* ADSP code */
	ROM_LOAD( "snd0.u48",   0x000000, 0x200000, CRC(02b1927c) SHA1(08b21d8b31b0f15c59fb5bb7eaf425e6fe04f7b5) )
	ROM_LOAD( "snd1.u49",   0x200000, 0x200000, CRC(58b18eda) SHA1(7f3105fe04d9c0cdfd76e3323f623a4d0f7dad06) )
	ROM_LOAD( "snd2.u50",   0x400000, 0x200000, CRC(51d626d6) SHA1(0e68b79dcb653dcba48121ca2d4f692f90afa85e) )
	ROM_LOAD( "snd3.u51",   0x600000, 0x200000, CRC(31af26ae) SHA1(2c9f4c078afec55964b5c2a4d00f5c43f2661a04) )
ROM_END

/*
Critter Crusher EXP
Sega, 1995.

This is a cart for Sega STV system. The game involves hitting 'critters' on
screen with a rubber/plastic hammer. The cab has a large LED display to show how many
'critters' were hit. The hammer positioning might work like a lightgun and the 'hitting'
may be like pulling the lightgun trigger?

The ROM cart appears to be a re-used Virtua Fighter Remix cart??
On top there is one 27C040 EPROM, EPR-18821 @ IC13
6 maskROMs, MPR-17945 to MPR-17950 (already dumped, known Virtua Fighter Remix ROMs)
On the other side of the PCB are 2 more maskROMs, MPR-18788 @ IC9 and MPR-18789 @ IC8

*/

ROM_START( critcrsh ) /* Must use Europe or Asia BIOS */
	STV_BIOS
	ROM_DEFAULT_BIOS( "euro" )

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr-18821.ic13",  0x0000001, 0x0080000, CRC(9a6658e2) SHA1(16dbae3d9ab584713afcb403f89fe71049609245) )
	ROM_RELOAD ( 0x0100001, 0x0080000 )
	ROM_RELOAD_PLAIN( 0x0200000, 0x0080000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0080000)
//  ROM_LOAD16_WORD_SWAP( "mpr17946.2",    0x0400000, 0x0400000, CRC(4cb245f7) SHA1(363d9936b27043b5858c956a45736ac05aefc54e) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17947.3",    0x0800000, 0x0400000, CRC(fef4a9fb) SHA1(1b4bd095962db769da17d3644df10f62d041e914) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17948.4",    0x0c00000, 0x0400000, CRC(3e2b251a) SHA1(be6191c18727d7cbc6399fd4c1aaae59304af30c) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17949.5",    0x1000000, 0x0400000, CRC(b2ecea25) SHA1(320c0e7ce34e81e2fe6400cbeb2cb3ca74426cc8) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17950.6",    0x1400000, 0x0400000, CRC(5b1f981d) SHA1(693b5744d210a2ac8b77e7c8c87f07ca859f8aed) ) // good
//  ROM_LOAD16_WORD_SWAP( "mpr17945.1",    0x1800000, 0x0200000, CRC(03ede188) SHA1(849c7fab5b97e043fea3deb8df6cc195ccced0e0) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18789.ic8", 0x1c00000, 0x0400000, CRC(b388616f) SHA1(0b2c5a547c3a6a8fb9f4ca54336cf6dc9adb8c6a) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-18788.ic9", 0x2000000, 0x0400000, CRC(feae5867) SHA1(7d2e47d5ab18700a246d53fdb7872a905cdac55a) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "critcrsh.nv", 0x0000, 0x0080, CRC(3da9860e) SHA1(05b315aa71fcfc4e617266e1c5e4954eccbf7854) )
ROM_END

ROM_START( sfish2 )
//  STV_BIOS // - sports fishing 2 uses its own bios

	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "epr18343.bin",   0x000000, 0x080000, CRC(48e2eecf) SHA1(a38bfbd5f279525e413b18b5ed3f37f6e9e31cdc) ) /* sport fishing 2 bios */
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD(             "epr18427.bin",      0x0000000, 0x0100000, CRC(3f25bec8) SHA1(43a5342b882d5aec0f35a8777cb475659f43b1c4) )
	ROM_LOAD16_WORD_SWAP( "mpr18273.ic2",	   0x0400000, 0x0200000, NO_DUMP )
	ROM_LOAD16_WORD_SWAP( "mpr18274.ic3",      0x0800000, 0x0200000, NO_DUMP )
	ROM_LOAD16_WORD_SWAP( "mpr18275.ic4",      0x0c00000, 0x0200000, NO_DUMP )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "sfish2", 0, SHA1(a10073d83bbbe16e16f69ad48565821576557d61) )

	ROM_REGION32_BE( 0x3000000, "abus", ROMREGION_ERASE00 ) /* SH2 code */ \
ROM_END

ROM_START( sfish2j )
//  STV_BIOS // - sports fishing 2 uses its own bios

	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH2 code */
	ROM_LOAD16_WORD_SWAP( "epr18343.bin",   0x000000, 0x080000, CRC(48e2eecf) SHA1(a38bfbd5f279525e413b18b5ed3f37f6e9e31cdc) ) /* sport fishing 2 bios */
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE(             "epr18344.a",      0x0000001, 0x0100000,  CRC(5a7de018) SHA1(88e0c2a9a9d4ebf699878c0aa9737af85f95ccf8) )

	ROM_LOAD16_WORD_SWAP( "mpr18273.ic2",	   0x0400000, 0x0200000, NO_DUMP )
	ROM_LOAD16_WORD_SWAP( "mpr18274.ic3",      0x0800000, 0x0200000, NO_DUMP )

	DISK_REGION( "cdrom" )
	DISK_IMAGE( "sfish2", 0, SHA1(a10073d83bbbe16e16f69ad48565821576557d61) )

	ROM_REGION32_BE( 0x3000000, "abus", ROMREGION_ERASE00 ) /* SH2 code */ \
ROM_END


ROM_START( magzun )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "flash.ic13",               0x0000001, 0x0100000, CRC(e6f0aca0) SHA1(251d4d9c5a332d13af3a144c5eb9d8e7836bdd1b) ) // good
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr-19354.ic2",    0x0400000, 0x0400000, CRC(a23822e7) SHA1(10ca5d39dcaaf35b80168a08d8a18d77fba1d2ce) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19355.ic3",    0x0800000, 0x0400000, CRC(d70e5ebc) SHA1(2d560f6b6e693b2b91cf5ff5c4f0890cc2176f91) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19356.ic4",    0x0c00000, 0x0400000, CRC(3bc43fe9) SHA1(f72b0f3208e2f411f4c9cc76c317a605acd32a67) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19357.ic5",    0x1000000, 0x0400000, CRC(aa749370) SHA1(f09b0aa94fcc983419aaf2465dae5c9c64606158) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19358.ic6",    0x1400000, 0x0400000, CRC(0969f1ec) SHA1(25b9671f0975172f283458d46e160080dd1d19e9) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-19359.ic1",    0x1800000, 0x0400000, CRC(b0d06f9c) SHA1(19e04c9c3a0bea5950aba8e1975962fa37722f32) ) // good

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 3 players
	ROM_LOAD( "magzun.nv", 0x0000, 0x0080, CRC(42700321) SHA1(1f2ba760c410312539c8677223edcd1cda3b51d4) )
ROM_END


ROM_START( stress )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */
	ROM_LOAD16_BYTE( "epr-21300a.ic13",    0x0000001, 0x0100000, CRC(899d829e) SHA1(b6c6da92dc108353998b29c0659d288645541519) ) // good
	ROM_RELOAD_PLAIN( 0x0200000, 0x0100000)
	ROM_RELOAD_PLAIN( 0x0300000, 0x0100000)
	ROM_LOAD16_WORD_SWAP( "mpr-21290.ic2",    0x0400000, 0x0400000, CRC(a49d29f3) SHA1(8f6c26fd9e94a9e03dd0029026d205cf481fe151) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21291.ic3",    0x0800000, 0x0400000, CRC(9452ba20) SHA1(8a9ff546901715f99bb911616c74ae30ebd7c6d7) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21292.ic4",    0x0c00000, 0x0400000, CRC(f60268e2) SHA1(5c2febb94553a941a68e9611617750a89c82e783) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21293.ic5",    0x1000000, 0x0400000, CRC(794946e0) SHA1(7881adb92fbf3efecb88f30834cdd1ed863b0b0e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21294.ic6",    0x1400000, 0x0400000, CRC(550843bb) SHA1(5e278a0ae60f7bee23b5f1ee10b8ce31effc9718) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21289.ic1",    0x1800000, 0x0400000, CRC(c2ee8bea) SHA1(5fa0e6b492cc272c33b01adbef9233e6c8098827) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21296.ic8",    0x1c00000, 0x0400000, CRC(b825c42a) SHA1(7c6f737b69b9283345f8fa23cb1d110b9b5f3d7e) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21297.ic9",    0x2000000, 0x0400000, CRC(4bff7469) SHA1(8a36405a1a292d60b5918e604b460ed98740fae3) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21298.ic10",   0x2400000, 0x0400000, CRC(68d07144) SHA1(5021f43d19105b484135ebddd3fa203233096c98) ) // good
	ROM_LOAD16_WORD_SWAP( "mpr-21299.ic11",   0x2800000, 0x0400000, CRC(ecc521c6) SHA1(f7ed4dd1cbe179652fdfdde34929b41a1fdcf9e2) ) // good
ROM_END

/* the rom test for this is in 'each game test'  */
ROM_START( nclubv3 )
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(b4008ed0) SHA1(acb3784acad971eb5f4920760dc23a16330e7bad) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(4e894850) SHA1(eb7c3399505a45816701197a45062b9f34e5a3e1) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(5b6b023f) SHA1(cf17c5857d85d4326dfe2ce40cf96989f9f78ecd) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(b7beab03) SHA1(de703e461a2bdd87b0695bd2f16e4c97d11bcf92) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(a9f81069) SHA1(60d88c7c20178a00d6927c37069ab0c374ebf51e) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic32",    0x0c00000, 0x0200000, CRC(02708d66) SHA1(6881b0b05e55989953a16f6ba503ba891b849c07) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic34",    0x0e00000, 0x0200000, CRC(c79d0537) SHA1(9d595f718ff8f8ff7ca88100f35c589d5f9b4216) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic36",    0x1000000, 0x0200000, CRC(0c9df896) SHA1(4d8c18205e7aa90bfaa677ecff2b65128f2ad47c) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic23",    0x1200000, 0x0200000, CRC(bd922829) SHA1(4c6f988173e439a05a77da043d856e142b0da831) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic25",    0x1400000, 0x0200000, CRC(f77f9e24) SHA1(9a9636114e74c1fd7bd67db8005af02ef6a75ab1) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "nclubv3.nv", 0x0000, 0x0080, CRC(9122a9e9) SHA1(5318994905e005567709c41449547c545182bece) )
ROM_END

ROM_START( techbowl ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(5058db21) SHA1(eec908bbfb9ec0fdca0002e69f32c1c030086456) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(34090f6d) SHA1(b8bc344ab826d5c9584afb01dba1c720b8dbc74d) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(fb073352) SHA1(a5164aa5854ab3095f704ab73b6e4fb9ed0e0785) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(530e0ceb) SHA1(8d14eb9dbf253a4563587d256a15492384e7ca5c) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "techbowl.nv", 0x0000, 0x0080, CRC(5bebc2b7) SHA1(e189e891e1753059fbaad4ce82ddf191d5e8176a) )
ROM_END

ROM_START( micrombc ) // set to 1p
	STV_BIOS

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(8385bc45) SHA1(0bd60d7560cb2313d68470d0572850a7b8c501fd) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(84ecb42f) SHA1(005dee9a0912d4b1b7f5157bc3cde96548c1e348) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(869bc19c) SHA1(638125007d331beef567c976ea6f4e7a21acdb64) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(0c3db354) SHA1(c4d43da7cea1b4d5ca3ac545afde10344a4a385b) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(03b9eacf) SHA1(d69c10f7613d9f52042dd6cce64e74e2b1ecc2d8) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic32",    0x0c00000, 0x0200000, CRC(62c10626) SHA1(58cb0ca0330fa7a62b277ab0ff84bff65b81bb23) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic34",    0x1000000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic36",    0x1200000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "micrombc.nv", 0x0000, 0x0080, CRC(6e89815f) SHA1(4478f614fb61859f4ee7bf55462f737387887e6f) )
ROM_END

ROM_START( pclub2 ) // set to 1p / runs with the USA bios
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(d2ceade7) SHA1(a4300322e582f403d9207290f3900e1a72fcb9b9) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(0e968c2d) SHA1(fbcc7533fcb6b87cd8255fc2d307ae618301ea64) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(ab51da70) SHA1(85214aa805ffc9de59900dc0cd4e19e5ab756bf7) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(3a654b2a) SHA1(7398e25836bfbdeab6350759f25c420c3b496172) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2.nv", 0x0000, 0x0080, CRC(00d0f04e) SHA1(8b5a3e1c52e34443f83fd4a8948a00cacb5071d0) )
ROM_END

ROM_START( pclub2v3 ) // set to 1p / runs with the USA bios
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, BAD_DUMP CRC(f88347aa) SHA1(3e9ca105edbd6ce11ea4194eb1733785e87f92b2) ) // BAD
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(b5871198) SHA1(10d187eebcca5d70c5ae10d1a144685a96491126) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(d97034ed) SHA1(a7a0f659eefd539b2a1fd70ef394eed30ea54c0c) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(f1421506) SHA1(c384b695338144e5f051134bda73b059b678a7df) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) ) // OK (rom is blank?!)

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclub2v3.nv", 0x0000, 0x0080, CRC(a8a2d30c) SHA1(bdde3d62ff21190a23698058ff66e476a75a09aa) )
ROM_END

ROM_START( pclubpok ) // set to 1p / runs with the USA bios
	STV_BIOS
	ROM_DEFAULT_BIOS( "us" )

	ROM_REGION32_BE( 0x3000000, "game0", ROMREGION_ERASE00 ) /* SH2 code */

	ROM_LOAD16_WORD_SWAP( "ic22",    0x0200000, 0x0200000, CRC(48ab8371) SHA1(1c2124afad6bc1f4de2619e6b915f78e91addf05) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic24",    0x0400000, 0x0200000, CRC(9915faea) SHA1(b96f64a8cbb1b9496bb566d0469975fddb4fbe98) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic26",    0x0600000, 0x0200000, CRC(054ad120) SHA1(987e812af099bb0aa5d43a4c10ae8345370a2606) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic28",    0x0800000, 0x0200000, CRC(3a654b2a) SHA1(7398e25836bfbdeab6350759f25c420c3b496172) ) // OK
	ROM_LOAD16_WORD_SWAP( "ic30",    0x0a00000, 0x0200000, CRC(98747bef) SHA1(8d452507a9842a48cd1a7db8fde11a070a6f068b) ) // OK

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // preconfigured to 1 player
	ROM_LOAD( "pclubpok.nv", 0x0000, 0x0080, CRC(4ba3f21a) SHA1(898a393fb2fc1961b68b7d4f383d5447e6c010e8) )
ROM_END

/*
country codes:
J = Japan
U = United States
E = Europe
T = Taiwan
B = Brazil
K = Korea
A = PAL Asia
L = Latin America
date codes:
(Original is yyyy/mm/dd,changed to reflect Capcom's one (yy/mm/dd))
Version codes:
V = Version(obviously ;)
(number before the dot) = game status (0=Sample,1=Master,2=Upgrade)
(number after the dot) = game version/release
There is also another internal code (called the "Product Number"),but AFAIK it's only used
by introdon in ST-V ("SG0000000"),and according to the manual it's even wrong! (SG is used
by Sega titles,and this is a Sunsoft game)It's likely to be a left-over...
*/

static DRIVER_INIT( sanjeon )
{
	UINT8 *src    = machine.region       ( "game0" )->base();
	int x;

	for (x=0;x<0x3000000;x++)
	{
		src[x] = src[x]^0xff;

		src[x] = BITSWAP8(src[x],7,2,5,1,  3,6,4,0);
		src[x] = BITSWAP8(src[x],4,6,5,7,  3,2,1,0);
		src[x] = BITSWAP8(src[x],7,6,5,4,  2,3,1,0);
		src[x] = BITSWAP8(src[x],7,0,5,4,  3,2,1,6);
		src[x] = BITSWAP8(src[x],3,6,5,4,  7,2,1,0);

	}


	DRIVER_INIT_CALL(sasissu);
}



/*    YEAR  NAME        PARENT  COMPAT  MACHINE INPUT   INIT        COMPANY     FULLNAME            FLAGS */
CONS( 1994, saturn,     0,      0,      saturn, saturn, saturnus,   "Sega",     "Saturn (USA)",     GAME_NOT_WORKING )
CONS( 1994, saturnjp,   saturn, 0,      saturn, saturn, saturnjp,   "Sega",     "Saturn (Japan)",   GAME_NOT_WORKING )
CONS( 1994, saturneu,   saturn, 0,      saturn, saturn, saturneu,   "Sega",     "Saturn (PAL)",     GAME_NOT_WORKING )
CONS( 1995, vsaturn,    saturn, 0,      saturn, saturn, saturnjp,   "JVC",      "V-Saturn",         GAME_NOT_WORKING )
CONS( 1995, hisaturn,   saturn, 0,      saturn, saturn, saturnjp,   "Hitachi",  "HiSaturn",         GAME_NOT_WORKING )

GAME( 1996, stvbios,   0,       stv_slot, stv,      stv,        ROT0,   "Sega",                         "ST-V Bios", GAME_IS_BIOS_ROOT )

//GAME YEAR, NAME,     PARENT,  MACH, INP, INIT,      MONITOR
/* Playable */
GAME( 1998, astrass,   stvbios, stv,      stv,      astrass,	ROT0,   "Sunsoft",  					"Astra SuperStars (J 980514 V1.002)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1995, bakubaku,  stvbios, stv,      stv,	    stv,    	ROT0,   "Sega",     					"Baku Baku Animal (J 950407 V1.000)", GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, batmanfr,  stvbios, stv,      batmanfr,	batmanfr,	ROT0,	"Acclaim",  					"Batman Forever (JUE 960507 V1.000)", GAME_NO_SOUND )
GAME( 1996, colmns97,  stvbios, stv,      stv,		colmns97,	ROT0,   "Sega", 						"Columns '97 (JET 961209 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, cotton2,   stvbios, stv,      stv,		cotton2,	ROT0,   "Success",  					"Cotton 2 (JUET 970902 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1998, cottonbm,  stvbios, stv,      stv,		cottonbm,	ROT0,   "Success",  					"Cotton Boomerang (JUET 980709 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, critcrsh,  stvbios, stv,      critcrsh,	stv,		ROT0,   "Sega", 	    				"Critter Crusher (EA 951204 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1999, danchih,   stvbios, stv,      stvmp,	danchih,	ROT0,   "Altron (Tecmo license)",   	"Danchi de Hanafuda (J 990607 V1.400)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 2000, danchiq,   stvbios, stv,      stv,		danchih,	ROT0,   "Altron",   					"Danchi de Quiz Okusan Yontaku Desuyo! (J 001128 V1.200)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, diehard,   stvbios, stv,      stv,		diehard,	ROT0,   "Sega", 						"Die Hard Arcade (UET 960515 V1.000)", GAME_IMPERFECT_SOUND  )
GAME( 1996, dnmtdeka,  diehard, stv,      stv,		dnmtdeka,	ROT0,   "Sega", 						"Dynamite Deka (J 960515 V1.000)", GAME_IMPERFECT_SOUND  )
GAME( 1995, ejihon,    stvbios, stv,      stv,		stv,    	ROT0,   "Sega", 						"Ejihon Tantei Jimusyo (J 950613 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, fhboxers,  stvbios, stv,      stv,		fhboxers,	ROT0,   "Sega", 						"Funky Head Boxers (JUETBKAL 951218 V1.000)", GAME_NO_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1994, gaxeduel,  stvbios, stv,      stv,		gaxeduel,	ROT0,   "Sega", 	    				"Golden Axe - The Duel (JUETL 950117 V1.000)", GAME_IMPERFECT_SOUND )
GAME( 1998, grdforce,  stvbios, stv,      stv,		grdforce,	ROT0,   "Success",  					"Guardian Force (JUET 980318 V0.105)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1998, groovef,   stvbios, stv,      stv,		groovef,	ROT0,   "Atlus",    					"Groove on Fight - Gouketsuji Ichizoku 3 (J 970416 V1.001)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, hanagumi,  stvbios, stv,      stv,		hanagumi,	ROT0,   "Sega",     					"Hanagumi Taisen Columns - Sakura Wars (J 971007 V1.010)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1996, introdon,  stvbios, stv,      stv,		stv,    	ROT0,   "Sunsoft / Success",			"Karaoke Quiz Intro Don Don! (J 960213 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, kiwames,   stvbios, stv,      stvmp,	stv,    	ROT0,   "Athena",   					"Pro Mahjong Kiwame S (J 951020 V1.208)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, maruchan,  stvbios, stv,      stv,		maruchan,	ROT0,   "Sega / Toyosuisan",	    	"Maru-Chan de Goo! (J 971216 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, mausuke,   stvbios, stv,      stv,		mausuke,	ROT0,   "Data East",					"Mausuke no Ojama the World (J 960314 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1998, myfairld,  stvbios, stv,      stvmp,	stv,    	ROT0,   "Micronet",                 	"Virtual Mahjong 2 - My Fair Lady (J 980608 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1998, othellos,  stvbios, stv,      stv,		othellos,	ROT0,   "Success",  					"Othello Shiyouyo (J 980423 V1.002)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, pblbeach,  stvbios, stv,      stv,		pblbeach,	ROT0,   "T&E Soft",                 	"Pebble Beach - The Great Shot (JUE 950913 V0.990)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, prikura,   stvbios, stv,      stv,		prikura,	ROT0,   "Atlus",    					"Princess Clara Daisakusen (J 960910 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, puyosun,   stvbios, stv,      stv,		puyosun,	ROT0,   "Compile",  					"Puyo Puyo Sun (J 961115 V0.001)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1998, rsgun,     stvbios, stv,      stv,		rsgun,  	ROT0,   "Treasure", 					"Radiant Silvergun (JUET 980523 V1.000)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, sasissu,   stvbios, stv,      stv,		sasissu,	ROT0,   "Sega", 	    				"Taisen Tanto-R Sashissu!! (J 980216 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1999, sanjeon,   sasissu, stv,      stv,		sanjeon,	ROT0,   "Sega / Deniam",	        	"DaeJeon! SanJeon SuJeon (AJTUE 990412 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, seabass,   stvbios, stv,      stv,		seabass,	ROT0,   "A wave inc. (Able license)",	"Sea Bass Fishing (JUET 971110 V0.001)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, shanhigw,  stvbios, stv,      stv,		shanhigw,	ROT0,   "Sunsoft / Activision", 		"Shanghai - The Great Wall / Shanghai Triple Threat (JUE 950623 V1.005)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, shienryu,  stvbios, stv,      stv,		shienryu,	ROT270, "Warashi",  					"Shienryu (JUET 961226 V1.000)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1998, sss,       stvbios, stv,      stv,		sss,    	ROT0,   "Capcom / Cave / Victor",		"Steep Slope Sliders (JUET 981110 V1.000)", GAME_IMPERFECT_SOUND )
GAME( 1995, sandor,    stvbios, stv,      stv,		sandor, 	ROT0,   "Sega", 	    				"Puzzle & Action: Sando-R (J 951114 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, thunt,     sandor,  stv,      stv,		thunt,  	ROT0,   "Sega",	                		"Puzzle & Action: Treasure Hunt (JUET 970901 V2.00E)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, thuntk,    sandor,  stv,      stv,		sandor, 	ROT0,   "Sega / Deniam",        		"Puzzle & Action: BoMulEul Chajara (JUET 970125 V2.00K)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, smleague,  stvbios, stv,      stv,		smleague,	ROT0,   "Sega", 	    				"Super Major League (U 960108 V1.000)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, finlarch,  smleague,stv,      stv,		finlarch,	ROT0,   "Sega", 	    				"Final Arch (J 950714 V1.001)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1996, sokyugrt,  stvbios, stv,      stv,		sokyugrt,	ROT0,   "Raizing / Eighting",   		"Soukyugurentai / Terra Diver (JUET 960821 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1995, suikoenb,  stvbios, stv,      stv,		suikoenb,	ROT0,   "Data East",                	"Suikoenbu / Outlaws of the Lost Dynasty (JUETL 950314 V2.001)", GAME_IMPERFECT_SOUND )
GAME( 1996, vfkids,    stvbios, stv,      stv,		stv,    	ROT0,   "Sega", 						"Virtua Fighter Kids (JUET 960319 V0.000)", GAME_IMPERFECT_SOUND )
GAME( 1997, vmahjong,  stvbios, stv,      stvmp,	stv,    	ROT0,   "Micronet",                 	"Virtual Mahjong (J 961214 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, winterht,  stvbios, stv,      stv,		winterht,	ROT0,   "Sega", 						"Winter Heat (JUET 971012 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, znpwfv,    stvbios, stv,      stv,		znpwfv, 	ROT0,   "Sega", 	    				"Zen Nippon Pro-Wrestling Featuring Virtua (J 971123 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )

/* Almost */
GAME( 1998, twcup98,   stvbios, stv,      stv,		twcup98,	ROT0,   "Tecmo",                    	"Tecmo World Cup '98 (JUET 980410 V1.000)", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING ) // player movement
GAME( 1998, elandore,  stvbios, stv,      stv,		elandore,	ROT0,   "Sai-Mate", 					"Touryuu Densetsu Elan-Doree / Elan Doree - Legend of Dragoon (JUET 980922 V1.006)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )

/* Unemulated printer device */
GAME( 1998, stress,    stvbios, stv,      stv,		stv,    	ROT0,   "Sega", 	    				"Stress Busters (J 981020 V1.000)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )
GAME( 1997, nclubv3,   stvbios, stv,      stv,		nameclv3,	ROT0,   "Sega", 	    				"Name Club Ver.3 (J 970723 V1.000)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1997, pclub2,    stvbios, stv,      stv,		stv,    	ROT0,   "Atlus",	    				"Print Club 2 (U 970921 V1.000)", GAME_NOT_WORKING )
GAME( 1999, pclub2v3,  pclub2,  stv,      stv,		stv,    	ROT0,   "Atlus",	    				"Print Club 2 Vol. 3 (U 990310 V1.000)", GAME_NOT_WORKING )
GAME( 1999, pclubpok,  stvbios, stv,      stv,      stv,        ROT0,   "Atlus",                        "Print Club Pokemon B (U 991126 V1.000)", GAME_NOT_WORKING )

/* Doing something.. but not enough yet */
GAME( 1995, vfremix,   stvbios, stv,      stv,		vfremix,	ROT0,   "Sega", 	    				"Virtua Fighter Remix (JUETBKAL 950428 V1.000)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1997, findlove,  stvbios, stv,      stv,		stv,		ROT0,   "Daiki / FCF",  	    		"Find Love (J 971212 V1.000)", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )
GAME( 1996, decathlt,  stvbios, stv,      stv,		decathlt,	ROT0,   "Sega", 	    				"Decathlete (JUET 960709 V1.001)", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION )
GAME( 1996, decathlto, decathlt,stv,      stv,		decathlt,	ROT0,	"Sega", 	    				"Decathlete (JUET 960424 V1.000)", GAME_NO_SOUND | GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION )

/* Gives I/O errors */
GAME( 1996, magzun,    stvbios, stv,      stv,		stv,    	ROT0,   "Sega", 	    				"Magical Zunou Power (J 961031 V1.000)", GAME_NOT_WORKING )
GAME( 1997, techbowl,  stvbios, stv,      stv,		stv,    	ROT0,   "Sega", 	    				"Technical Bowling (J 971212 V1.000)", GAME_NOT_WORKING )
GAME( 1999, micrombc,  stvbios, stv,      stv,		stv,    	ROT0,   "Sega", 	    				"Microman Battle Charge (J 990326 V1.000)", GAME_NOT_WORKING )

/* Black screen */
GAME( 1999, ffreveng,  stvbios, stv,      stv,		ffreveng,	ROT0,   "Capcom",   					"Final Fight Revenge (JUET 990714 V1.000)", GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND | GAME_NOT_WORKING )

/* CD games */
GAME( 1995, sfish2,    0,		stv,      stv,		stv,		ROT0,   "Sega",	    					"Sport Fishing 2 (UET 951106 V1.10e)", GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 1995, sfish2j,   sfish2,	stv,      stv,		stv,		ROT0,   "Sega",	    					"Sport Fishing 2 (J 951201 V1.100)", GAME_NO_SOUND | GAME_NOT_WORKING )

/*
This is the known list of undumped ST-V games:
    Kiss Off (US version of mausuke)
    Baku Baku (US version of Baku Baku Animal,dunno if it exists for the ST-V)
    Sport Fishing
    Aroma Club
    Movie Club
    Name Club (other versions)
    Print Club 2 (other versions)
    Youen Denshi Mahjong Yuugi Gal Jan
    Danchi de Quiz: Okusan 4taku Desu yo!
    NBA Action (?)
    Quiz Omaeni Pipon Cho! (?)
Others may exist as well...
*/

