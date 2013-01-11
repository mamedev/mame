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

TODO:
(Main issues)
- decap the SH-1, used for CD block (needed especially for Sega Saturn)
- IRQs: some games have some issues with timing accurate IRQs, check/fix all of them.
- The Cart-Dev mode hangs even with the -dev bios,I would like to see what it does on the real HW.
- IC13 games on the dev bios doesn't even load the cartridge / crashes the emulation at start-up,
  rom rearrange needed?
- SCU DSP still has its fair share of issues, it also needs to be converted to CPU structure;
- Add the RS232c interface (serial port),needed by fhboxers.
- Video emulation bugs: check stvvdp2.c file.
- Reimplement the idle skip if possible.
- Properly emulate the protection chips, used by several games (check stvprot.c for more info)
- Move SCU device into its respective file;
- Split ST-V and Saturn files properly;

(per-game issues)
- stress: accesses the Sound Memory Expansion Area (0x05a80000-0x05afffff), unknown purpose;
- smleague / finlarch: it randomly hangs / crashes,it works if you use a ridiculous MCFG_INTERLEAVE number,might need strict
  SH-2 synching.
- groovef / myfairld: why do we get 2 credits on startup? Cause might be by a communication with the M68k
- myfairld: Apparently this game gives a black screen (either test mode and in-game mode),but let it wait for about
  10 seconds and the game will load everything. This is because of a hellishly slow m68k sub-routine located at 54c2.
  Likely to not be a bug but an in-game design issue.
- danchih: hanafuda panel doesn't work.
- findlove: controls doesn't work? Playing with the debugger at location $6063720 it makes it get furter,but controls
  still doesn't work, missing irq?
- batmanfr: Missing sound,caused by an extra ADSP chip which is on the cart.The CPU is a
  ADSP-2181,and it's the same used by NBA Jam Extreme (ZN game).
- vfremix: when you play as Akira, there is a problem with third match: game doesn't upload all textures
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
-Remove CD transfer DMA hack (tied with CD block bug(s)?)
-Add timings(but how fast are each DMA?).
-Add level priority & DMA status register.
*/

#define DIRECT_MODE(_lv_)           (!(m_scu_regs[5+(_lv_*8)] & 0x01000000))
#define INDIRECT_MODE(_lv_)           (m_scu_regs[5+(_lv_*8)] & 0x01000000)
#define DRUP(_lv_)                    (m_scu_regs[5+(_lv_*8)] & 0x00010000)
#define DWUP(_lv_)                    (m_scu_regs[5+(_lv_*8)] & 0x00000100)

#define DMA_STATUS              (m_scu_regs[31])
/*These macros sets the various DMA status flags.*/
#define DnMV_1(_ch_) DMA_STATUS|=(0x10 << 4 * _ch_)
#define DnMV_0(_ch_) DMA_STATUS&=~(0x10 << 4 * _ch_)

/*For area checking*/
#define BIOS_BUS(var)   (var & 0x07000000) == 0
#define ABUS(_lv_)       ((m_scu.src[_lv_] & 0x07000000) >= 0x02000000) && ((m_scu.src[_lv_] & 0x07000000) <= 0x04000000)
#define BBUS(_lv_)       ((scu_##_lv_ & 0x07ffffff) >= 0x05a00000) && ((scu_##_lv_ & 0x07ffffff) <= 0x05ffffff)
#define VDP1_REGS(_lv_)  ((scu_##_lv_ & 0x07ffffff) >= 0x05d00000) && ((scu_##_lv_ & 0x07ffffff) <= 0x05dfffff)
#define VDP2(_lv_)       ((scu_##_lv_ & 0x07ffffff) >= 0x05e00000) && ((scu_##_lv_ & 0x07ffffff) <= 0x05fdffff)
#define WORK_RAM_L(_lv_) ((scu_##_lv_ & 0x07ffffff) >= 0x00200000) && ((scu_##_lv_ & 0x07ffffff) <= 0x002fffff)
#define WORK_RAM_H(var) (var & 0x07000000) == 0x06000000
#define SOUND_RAM(_lv_)  ((scu_##_lv_ & 0x07ffffff) >= 0x05a00000) && ((scu_##_lv_ & 0x07ffffff) <= 0x05afffff)

void saturn_state::scu_do_transfer(UINT8 event)
{
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	int i;

	for(i=0;i<3;i++)
	{
		if(m_scu.enable_mask[i] && m_scu.start_factor[i] == event)
		{
			if(DIRECT_MODE(i))      { scu_dma_direct(space,i);   }
			else                    { scu_dma_indirect(space,i); }
		}
	}
}

/* test pending irqs */
void saturn_state::scu_test_pending_irq()
{
	int i;
	const int irq_level[32] = { 0xf, 0xe, 0xd, 0xc,
								0xb, 0xa, 0x9, 0x8,
								0x8, 0x6, 0x6, 0x5,
								0x3, 0x2,  -1,  -1,
								0x7, 0x7, 0x7, 0x7,
								0x4, 0x4, 0x4, 0x4,
								0x1, 0x1, 0x1, 0x1,
								0x1, 0x1, 0x1, 0x1  };

	for(i=0;i<32;i++)
	{
		if((!(m_scu.ism & 1 << i)) && (m_scu.ist & 1 << i))
		{
			if(irq_level[i] != -1) /* TODO: cheap check for undefined irqs */
			{
				m_maincpu->set_input_line_and_vector(irq_level[i], HOLD_LINE, 0x40 + i);
				m_scu.ist &= ~(1 << i);
				return; /* avoid spurious irqs, correct? */
			}
		}
	}
}

READ32_MEMBER(saturn_state::saturn_scu_r)
{
	UINT32 res;

	/*TODO: write only registers must return 0 or open bus */
	switch(offset)
	{
		case 0x5c/4:
		//  Super Major League and Shin Megami Tensei - Akuma Zensho reads from there (undocumented), DMA status mirror?
			if(LOG_SCU) logerror("(PC=%08x) DMA status reg read\n",space.device().safe_pc());
			res = m_scu_regs[0x7c/4];
			break;
		case 0x7c/4:
			if(LOG_SCU) logerror("(PC=%08x) DMA status reg read\n",space.device().safe_pc());
			res = m_scu_regs[offset];
			break;
		case 0x80/4:
			res = dsp_prg_ctrl_r(space);
			break;
		case 0x8c/4:
			if(LOG_SCU) logerror( "DSP mem read at %08X\n", m_scu_regs[34]);
			res = dsp_ram_addr_r();
			break;
		case 0xa0/4:
			if(LOG_SCU) logerror("(PC=%08x) IRQ mask reg read %08x MASK=%08x\n",space.device().safe_pc(),mem_mask,m_scu_regs[0xa0/4]);
			res = m_scu.ism;
			break;
		case 0xa4/4:
			if(LOG_SCU) logerror("(PC=%08x) IRQ status reg read %08x MASK=%08x\n",space.device().safe_pc(),mem_mask,m_scu_regs[0xa0/4]);
			res = m_scu.ist;
			break;
		case 0xc8/4:
			logerror("(PC=%08x) SCU version reg read\n",space.device().safe_pc());
			res = 0x00000004;/*SCU Version 4, OK? */
			break;
		default:
			if(LOG_SCU) logerror("(PC=%08x) SCU reg read at %d = %08x\n",space.device().safe_pc(),offset,m_scu_regs[offset]);
			res = m_scu_regs[offset];
			break;
	}

	return res;
}

#define DMA_CH ((offset & 0x18) / 8)

WRITE32_MEMBER(saturn_state::saturn_scu_w)
{
	COMBINE_DATA(&m_scu_regs[offset]);

	switch(offset)
	{
		/*LV 0 DMA*/
		case 0x00/4: case 0x20/4: case 0x40/4:  m_scu.src[DMA_CH]  = ((m_scu_regs[offset] & 0x07ffffff) >> 0); break;
		case 0x04/4: case 0x24/4: case 0x44/4:  m_scu.dst[DMA_CH]  = ((m_scu_regs[offset] & 0x07ffffff) >> 0); break;
		case 0x08/4: case 0x28/4: case 0x48/4:  m_scu.size[DMA_CH] = ((m_scu_regs[offset] & ((offset == 2) ? 0x000fffff : 0xfff)) >> 0); break;
		case 0x0c/4: case 0x2c/4: case 0x4c/4:
			m_scu.src_add[DMA_CH] = (m_scu_regs[offset] & 0x100) ? 4 : 0;
			m_scu.dst_add[DMA_CH] = 1 << (m_scu_regs[offset] & 7);
			if(m_scu.dst_add[DMA_CH] == 1) { m_scu.dst_add[DMA_CH] = 0; }
			break;
		case 0x10/4: case 0x30/4: case 0x50/4:
			m_scu.enable_mask[DMA_CH] = (data & 0x100) >> 8;
			if(m_scu.enable_mask[DMA_CH] && m_scu.start_factor[DMA_CH] == 7 && m_scu_regs[offset] & 1)
			{
				if(DIRECT_MODE(DMA_CH)) { scu_dma_direct(space,DMA_CH);   }
				else                    { scu_dma_indirect(space,DMA_CH); }
				m_scu_regs[offset]&=~1;//disable starting bit.
			}
			break;
		case 0x14/4: case 0x34/4: case 0x54/4:
			if(INDIRECT_MODE(DMA_CH))
			{
				//if(LOG_SCU) logerror("Indirect Mode DMA lv %d set\n",DMA_CH);
				if(!DWUP(DMA_CH)) m_scu.index[DMA_CH] = m_scu.dst[DMA_CH];
			}

			m_scu.start_factor[DMA_CH] = m_scu_regs[offset] & 7;
			break;

		case 0x60/4:
			if(LOG_SCU) logerror("DMA Forced Stop Register set = %02x\n",m_scu_regs[24]);
			break;
		case 0x7c/4: if(LOG_SCU) logerror("Warning: DMA status WRITE! Offset %02x(%d)\n",offset*4,offset); break;
		/*DSP section*/
		case 0x80/4:
			/* TODO: you can't overwrite some flags with this */
			dsp_prg_ctrl_w(space, m_scu_regs[offset]);
			if(LOG_SCU) logerror("SCU DSP: Program Control Port Access %08x\n",data);
			break;
		case 0x84/4:
			dsp_prg_data(m_scu_regs[offset]);
			if(LOG_SCU) logerror("SCU DSP: Program RAM Data Port Access %08x\n",data);
			break;
		case 0x88/4:
			dsp_ram_addr_ctrl(m_scu_regs[offset]);
			if(LOG_SCU) logerror("SCU DSP: Data RAM Address Port Access %08x\n",data);
			break;
		case 0x8c/4:
			dsp_ram_addr_w(m_scu_regs[offset]);
			if(LOG_SCU) logerror("SCU DSP: Data RAM Data Port Access %08x\n",data);
			break;
		case 0x90/4: /*if(LOG_SCU) logerror("timer 0 compare data = %03x\n",m_scu_regs[36]);*/ break;
		case 0x94/4: /*if(LOG_SCU) logerror("timer 1 set data = %08x\n",m_scu_regs[37]);*/ break;
		case 0x98/4: /*if(LOG_SCU) logerror("timer 1 mode data = %08x\n",m_scu_regs[38]);*/ break;
		case 0xa0/4: /* IRQ mask */
			m_scu.ism = m_scu_regs[0xa0/4];
			scu_test_pending_irq();
			break;
		case 0xa4/4: /* IRQ control */
			if(LOG_SCU) logerror("PC=%08x IRQ status reg set:%08x %08x\n",space.device().safe_pc(),m_scu_regs[41],mem_mask);
			m_scu.ist &= m_scu_regs[offset];
			break;
		case 0xa8/4: if(LOG_SCU) logerror("A-Bus IRQ ACK %08x\n",m_scu_regs[42]); break;
		case 0xc4/4: if(LOG_SCU) logerror("SCU SDRAM set: %02x\n",m_scu_regs[49]); break;
		default: if(LOG_SCU) logerror("Warning: unused SCU reg set %d = %08x\n",offset,data);
	}
}

/*Lv 0 DMA end irq*/
TIMER_CALLBACK_MEMBER(saturn_state::dma_lv0_ended )
{
	if(!(m_scu.ism & IRQ_DMALV0))
		m_maincpu->set_input_line_and_vector(5, HOLD_LINE, 0x4b);
	else
		m_scu.ist |= (IRQ_DMALV0);

	DnMV_0(0);
}

/*Lv 1 DMA end irq*/
TIMER_CALLBACK_MEMBER(saturn_state::dma_lv1_ended)
{
	if(!(m_scu.ism & IRQ_DMALV1))
		m_maincpu->set_input_line_and_vector(6, HOLD_LINE, 0x4a);
	else
		m_scu.ist |= (IRQ_DMALV1);

	DnMV_0(1);
}

/*Lv 2 DMA end irq*/
TIMER_CALLBACK_MEMBER(saturn_state::dma_lv2_ended)
{
	if(!(m_scu.ism & IRQ_DMALV2))
		m_maincpu->set_input_line_and_vector(6, HOLD_LINE, 0x49);
	else
		m_scu.ist |= (IRQ_DMALV2);

	DnMV_0(2);
}

void saturn_state::scu_single_transfer(address_space &space, UINT32 src, UINT32 dst,UINT8 *src_shift)
{
	UINT32 src_data;

	if(src & 1)
	{
		/* Road Blaster does a work ram h to color ram with offsetted source address, do some data rotation */
		src_data = ((space.read_dword(src & 0x07fffffc) & 0x00ffffff)<<8);
		src_data |= ((space.read_dword((src & 0x07fffffc)+4) & 0xff000000) >> 24);
		src_data >>= (*src_shift)*16;
	}
	else
		src_data = space.read_dword(src & 0x07fffffc) >> (*src_shift)*16;

	space.write_word(dst,src_data);

	*src_shift ^= 1;
}

void saturn_state::scu_dma_direct(address_space &space, UINT8 dma_ch)
{
	UINT32 tmp_src,tmp_dst,tmp_size;
	UINT8 cd_transfer_flag;

	if(m_scu.src_add[dma_ch] == 0 || (m_scu.dst_add[dma_ch] != 2 && m_scu.dst_add[dma_ch] != 4))
	{
	if(LOG_SCU) printf("DMA lv %d transfer START\n"
							"Start %08x End %08x Size %04x\n",dma_ch,m_scu.src[dma_ch],m_scu.dst[dma_ch],m_scu.size[dma_ch]);
	if(LOG_SCU) printf("Start Add %04x Destination Add %04x\n",m_scu.src_add[dma_ch],m_scu.dst_add[dma_ch]);
	}

	/* TODO: Game Basic trips this, bogus transfer from BIOS area to VDP1? */
	if(BIOS_BUS(m_scu.src[dma_ch]))
		popmessage("Warning: SCU transfer from BIOS area, contact MAMEdev");

	DnMV_1(dma_ch);

	/* max size */
	if(m_scu.size[dma_ch] == 0) { m_scu.size[dma_ch] = (dma_ch == 0) ? 0x00100000 : 0x1000; }

	tmp_src = tmp_dst = 0;

	tmp_size = m_scu.size[dma_ch];
	if(!(DRUP(dma_ch))) tmp_src = m_scu.src[dma_ch];
	if(!(DWUP(dma_ch))) tmp_dst = m_scu.dst[dma_ch];

	cd_transfer_flag = m_scu.src_add[dma_ch] == 0 && m_scu.src[dma_ch] == 0x05818000;

	/* TODO: Many games directly accesses CD-ROM register 0x05818000, it must be a dword access with current implementation otherwise it won't work */
	if(cd_transfer_flag)
	{
		int i;
		if(WORK_RAM_H(m_scu.dst[dma_ch]))
			m_scu.dst_add[dma_ch] = 4;
		else
			m_scu.dst_add[dma_ch] <<= 1;

		for (i = 0; i < m_scu.size[dma_ch];i+=m_scu.dst_add[dma_ch])
		{
			space.write_dword(m_scu.dst[dma_ch],space.read_dword(m_scu.src[dma_ch]));
			if(m_scu.dst_add[dma_ch] == 8)
				space.write_dword(m_scu.dst[dma_ch]+4,space.read_dword(m_scu.src[dma_ch]));

			m_scu.src[dma_ch]+=m_scu.src_add[dma_ch];
			m_scu.dst[dma_ch]+=m_scu.dst_add[dma_ch];
		}
	}
	else
	{
		int i;
		UINT8  src_shift;

		src_shift = ((m_scu.src[dma_ch] & 2) >> 1) ^ 1;

		for (i = 0; i < m_scu.size[dma_ch];i+=2)
		{
			scu_single_transfer(space,m_scu.src[dma_ch],m_scu.dst[dma_ch],&src_shift);

			if(src_shift)
				m_scu.src[dma_ch]+=m_scu.src_add[dma_ch];

			/* if target is Work RAM H, the add value is fixed, behaviour confirmed by Final Romance 2, Virtual Mahjong and Burning Rangers */
			m_scu.dst[dma_ch]+=(WORK_RAM_H(m_scu.dst[dma_ch])) ? 2 : m_scu.dst_add[dma_ch];
		}
	}

	m_scu.size[dma_ch] = tmp_size;
	if(!(DRUP(dma_ch))) m_scu.src[dma_ch] = tmp_src;
	if(!(DWUP(dma_ch))) m_scu.dst[dma_ch] = tmp_dst;

	{
		/*TODO: this is completely wrong HW-wise ...  */
		switch(dma_ch)
		{
			case 0: machine().scheduler().timer_set(attotime::from_usec(300), timer_expired_delegate(FUNC(saturn_state::dma_lv0_ended),this)); break;
			case 1: machine().scheduler().timer_set(attotime::from_usec(300), timer_expired_delegate(FUNC(saturn_state::dma_lv1_ended),this)); break;
			case 2: machine().scheduler().timer_set(attotime::from_usec(300), timer_expired_delegate(FUNC(saturn_state::dma_lv2_ended),this)); break;
		}
	}
}

void saturn_state::scu_dma_indirect(address_space &space,UINT8 dma_ch)
{
	/*Helper to get out of the cycle*/
	UINT8 job_done = 0;
	/*temporary storage for the transfer data*/
	UINT32 tmp_src;
	UINT32 indirect_src,indirect_dst;
	INT32 indirect_size;

	DnMV_1(dma_ch);

	m_scu.index[dma_ch] = m_scu.dst[dma_ch];

	do{
		tmp_src = m_scu.index[dma_ch];

		indirect_size = space.read_dword(m_scu.index[dma_ch]);
		indirect_src  = space.read_dword(m_scu.index[dma_ch]+8);
		indirect_dst  = space.read_dword(m_scu.index[dma_ch]+4);

		/*Indirect Mode end factor*/
		if(indirect_src & 0x80000000)
			job_done = 1;

		if(m_scu.src_add[dma_ch] == 0 || (m_scu.dst_add[dma_ch] != 2))
		{
			if(LOG_SCU) printf("DMA lv %d indirect mode transfer START\n"
								"Index %08x Start %08x End %08x Size %04x\n",dma_ch,tmp_src,indirect_src,indirect_dst,indirect_size);
			if(LOG_SCU) printf("Start Add %04x Destination Add %04x\n",m_scu.src_add[dma_ch],m_scu.dst_add[dma_ch]);
		}

		indirect_src &=0x07ffffff;
		indirect_dst &=0x07ffffff;
		indirect_size &= ((dma_ch == 0) ? 0xfffff : 0x3ffff); //TODO: Guardian Heroes sets up a 0x23000 transfer for the FMV?

		if(indirect_size == 0) { indirect_size = (dma_ch == 0) ? 0x00100000 : 0x2000; }

		{
			int i;
			UINT8  src_shift;

			src_shift = ((indirect_src & 2) >> 1) ^ 1;

			for (i = 0; i < indirect_size;i+=2)
			{
				scu_single_transfer(space,indirect_src,indirect_dst,&src_shift);

				if(src_shift)
					indirect_src+=m_scu.src_add[dma_ch];

				indirect_dst+= (WORK_RAM_H(indirect_dst)) ? 2 : m_scu.dst_add[dma_ch];
			}
		}

		//if(DRUP(0))   space.write_dword(tmp_src+8,m_scu.src[0]|job_done ? 0x80000000 : 0);
		//if(DWUP(0)) space.write_dword(tmp_src+4,m_scu.dst[0]);

		m_scu.index[dma_ch] = tmp_src+0xc;

	}while(job_done == 0);

	{
		/*TODO: this is completely wrong HW-wise ...  */
		switch(dma_ch)
		{
			case 0: machine().scheduler().timer_set(attotime::from_usec(300), timer_expired_delegate(FUNC(saturn_state::dma_lv0_ended),this)); break;
			case 1: machine().scheduler().timer_set(attotime::from_usec(300), timer_expired_delegate(FUNC(saturn_state::dma_lv1_ended),this)); break;
			case 2: machine().scheduler().timer_set(attotime::from_usec(300), timer_expired_delegate(FUNC(saturn_state::dma_lv2_ended),this)); break;
		}
	}
}


/**************************************************************************************/

WRITE16_MEMBER(saturn_state::saturn_soundram_w)
{
	//machine().scheduler().synchronize(); // force resync

	COMBINE_DATA(&m_sound_ram[offset]);
}

READ16_MEMBER(saturn_state::saturn_soundram_r)
{
	//machine().scheduler().synchronize(); // force resync

	return m_sound_ram[offset];
}

/* communication,SLAVE CPU acquires data from the MASTER CPU and triggers an irq.  */
WRITE32_MEMBER(saturn_state::minit_w)
{
	//logerror("cpu %s (PC=%08X) MINIT write = %08x\n", space.device().tag(), space.device().safe_pc(),data);
	machine().scheduler().boost_interleave(m_minit_boost_timeslice, attotime::from_usec(m_minit_boost));
	machine().scheduler().trigger(1000);
	sh2_set_frt_input(m_slave, PULSE_LINE);
}

WRITE32_MEMBER(saturn_state::sinit_w)
{
	//logerror("cpu %s (PC=%08X) SINIT write = %08x\n", space.device().tag(), space.device().safe_pc(),data);
	machine().scheduler().boost_interleave(m_sinit_boost_timeslice, attotime::from_usec(m_sinit_boost));
	sh2_set_frt_input(m_maincpu, PULSE_LINE);
}

READ8_MEMBER(saturn_state::saturn_backupram_r)
{
	if(!(offset & 1))
		return 0; // yes, it makes sure the "holes" are there.

	return m_backupram[offset >> 1] & 0xff;
}

WRITE8_MEMBER(saturn_state::saturn_backupram_w)
{
	if(!(offset & 1))
		return;

	m_backupram[offset >> 1] = data;
}

/* TODO: if you change the driver configuration then NVRAM contents gets screwed, needs mods in MAME framework */
static NVRAM_HANDLER(saturn)
{
	saturn_state *state = machine.driver_data<saturn_state>();
	static const UINT32 BUP_SIZE = 32*1024;
	static const UINT32 EBUP_SIZE = 0;//0x100000; // TODO: can't support more than 8 Mbit
	UINT8 backup_file[(BUP_SIZE)+EBUP_SIZE+4];
	static const UINT8 init[16] =
	{
		'B', 'a', 'c', 'k', 'U', 'p', 'R', 'a', 'm', ' ', 'F', 'o', 'r', 'm', 'a', 't'
	};
	UINT32 i;

	if (read_or_write)
	{
		for(i=0;i<BUP_SIZE;i++)
			backup_file[i] = state->m_backupram[i];
		#if 0
		for(i=0;i<EBUP_SIZE;i++)
			backup_file[i+BUP_SIZE] = state->m_cart_backupram[i];
		#endif
		for(i=0;i<4;i++)
			backup_file[i+(BUP_SIZE)+EBUP_SIZE] = state->m_smpc.SMEM[i];

		file->write(backup_file, (BUP_SIZE)+EBUP_SIZE+4);
	}
	else
	{
		if (file)
		{
			file->read(backup_file, (BUP_SIZE)+EBUP_SIZE+4);

			for(i=0;i<BUP_SIZE;i++)
				state->m_backupram[i] = backup_file[i];
			#if 0
			for(i=0;i<EBUP_SIZE;i++)
				state->m_cart_backupram[i] = backup_file[i+BUP_SIZE];
			#endif
			for(i=0;i<4;i++)
				state->m_smpc.SMEM[i] = backup_file[i+BUP_SIZE+EBUP_SIZE];
		}
		else
		{
			UINT8 j;
			memset(state->m_backupram, 0, BUP_SIZE);
			for (i = 0; i < 4; i++)
			{
				for(j=0;j<16;j++)
					state->m_backupram[i*16+j] = init[j];
			}
			#if 0
			memset(state->m_cart_backupram, 0, EBUP_SIZE);
			for (i = 0; i < 32; i++)
			{
				for(j=0;j<16;j++)
					state->m_cart_backupram[i*16+j] = init[j];
			}
			#endif
			memset(state->m_smpc.SMEM, 0, 4); // TODO: default for each region
		}
	}
}

READ8_MEMBER(saturn_state::saturn_cart_type_r)
{
	const int cart_ram_header[7] = { 0xff, 0x21, 0x22, 0x23, 0x24, 0x5a, 0x5c };

	return cart_ram_header[m_cart_type];
}

static ADDRESS_MAP_START( saturn_mem, AS_PROGRAM, 32, saturn_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_SHARE("share6")  // bios
	AM_RANGE(0x00100000, 0x0010007f) AM_READWRITE8_LEGACY(saturn_SMPC_r, saturn_SMPC_w,0xffffffff)
	AM_RANGE(0x00180000, 0x0018ffff) AM_READWRITE8(saturn_backupram_r, saturn_backupram_w,0xffffffff) AM_SHARE("share1")
	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_MIRROR(0x20100000) AM_SHARE("workram_l")
	AM_RANGE(0x01000000, 0x017fffff) AM_WRITE(minit_w)
	AM_RANGE(0x01800000, 0x01ffffff) AM_WRITE(sinit_w)
	AM_RANGE(0x02000000, 0x023fffff) AM_ROM AM_SHARE("share7") AM_REGION("maincpu", 0x80000)    // cartridge space
//  AM_RANGE(0x02400000, 0x027fffff) AM_RAM //cart RAM area, dynamically allocated
//  AM_RANGE(0x04000000, 0x047fffff) AM_RAM //backup RAM area, dynamically allocated
	AM_RANGE(0x04fffffc, 0x04ffffff) AM_READ8(saturn_cart_type_r,0x000000ff)
	AM_RANGE(0x05800000, 0x0589ffff) AM_READWRITE_LEGACY(stvcd_r, stvcd_w)
	/* Sound */
	AM_RANGE(0x05a00000, 0x05a7ffff) AM_READWRITE16(saturn_soundram_r, saturn_soundram_w,0xffffffff)
	AM_RANGE(0x05b00000, 0x05b00fff) AM_DEVREADWRITE16_LEGACY("scsp", scsp_r, scsp_w, 0xffffffff)
	/* VDP1 */
	AM_RANGE(0x05c00000, 0x05c7ffff) AM_READWRITE_LEGACY(saturn_vdp1_vram_r, saturn_vdp1_vram_w)
	AM_RANGE(0x05c80000, 0x05cbffff) AM_READWRITE_LEGACY(saturn_vdp1_framebuffer0_r, saturn_vdp1_framebuffer0_w)
	AM_RANGE(0x05d00000, 0x05d0001f) AM_READWRITE16_LEGACY(saturn_vdp1_regs_r, saturn_vdp1_regs_w,0xffffffff)
	AM_RANGE(0x05e00000, 0x05efffff) AM_READWRITE_LEGACY(saturn_vdp2_vram_r, saturn_vdp2_vram_w)
	AM_RANGE(0x05f00000, 0x05f7ffff) AM_READWRITE_LEGACY(saturn_vdp2_cram_r, saturn_vdp2_cram_w)
	AM_RANGE(0x05f80000, 0x05fbffff) AM_READWRITE16_LEGACY(saturn_vdp2_regs_r, saturn_vdp2_regs_w,0xffffffff)
	AM_RANGE(0x05fe0000, 0x05fe00cf) AM_READWRITE(saturn_scu_r, saturn_scu_w)
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_MIRROR(0x21f00000) AM_SHARE("workram_h")
	AM_RANGE(0x20000000, 0x2007ffff) AM_ROM AM_SHARE("share6")  // bios mirror
	AM_RANGE(0x22000000, 0x24ffffff) AM_ROM AM_SHARE("share7")  // cart mirror
	AM_RANGE(0x45000000, 0x46ffffff) AM_WRITENOP
	AM_RANGE(0xc0000000, 0xc00007ff) AM_RAM // cache RAM, Dragon Ball Z sprites needs this
ADDRESS_MAP_END

static ADDRESS_MAP_START( stv_mem, AS_PROGRAM, 32, saturn_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_SHARE("share6")  // bios
	AM_RANGE(0x00100000, 0x0010007f) AM_READWRITE8_LEGACY(stv_SMPC_r, stv_SMPC_w,0xffffffff)
	AM_RANGE(0x00180000, 0x0018ffff) AM_READWRITE8(saturn_backupram_r,saturn_backupram_w,0xffffffff) AM_SHARE("share1")
	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_MIRROR(0x20100000) AM_SHARE("workram_l")
//  AM_RANGE(0x00400000, 0x0040001f) AM_READWRITE_LEGACY(stv_ioga_r32, stv_io_w32) AM_SHARE("ioga") AM_MIRROR(0x20) /* installed with per-game specific */
	AM_RANGE(0x01000000, 0x017fffff) AM_WRITE(minit_w)
	AM_RANGE(0x01800000, 0x01ffffff) AM_WRITE(sinit_w)
	AM_RANGE(0x02000000, 0x04ffffff) AM_ROM AM_SHARE("share7") AM_REGION("abus", 0) // cartridge
	AM_RANGE(0x05800000, 0x0589ffff) AM_READWRITE_LEGACY(stvcd_r, stvcd_w)
	/* Sound */
	AM_RANGE(0x05a00000, 0x05afffff) AM_READWRITE16(saturn_soundram_r, saturn_soundram_w,0xffffffff)
	AM_RANGE(0x05b00000, 0x05b00fff) AM_DEVREADWRITE16_LEGACY("scsp", scsp_r, scsp_w, 0xffffffff)
	/* VDP1 */
	AM_RANGE(0x05c00000, 0x05c7ffff) AM_READWRITE_LEGACY(saturn_vdp1_vram_r, saturn_vdp1_vram_w)
	AM_RANGE(0x05c80000, 0x05cbffff) AM_READWRITE_LEGACY(saturn_vdp1_framebuffer0_r, saturn_vdp1_framebuffer0_w)
	AM_RANGE(0x05d00000, 0x05d0001f) AM_READWRITE16_LEGACY(saturn_vdp1_regs_r, saturn_vdp1_regs_w,0xffffffff)
	AM_RANGE(0x05e00000, 0x05efffff) AM_READWRITE_LEGACY(saturn_vdp2_vram_r, saturn_vdp2_vram_w)
	AM_RANGE(0x05f00000, 0x05f7ffff) AM_READWRITE_LEGACY(saturn_vdp2_cram_r, saturn_vdp2_cram_w)
	AM_RANGE(0x05f80000, 0x05fbffff) AM_READWRITE16_LEGACY(saturn_vdp2_regs_r, saturn_vdp2_regs_w,0xffffffff)
	AM_RANGE(0x05fe0000, 0x05fe00cf) AM_READWRITE(saturn_scu_r, saturn_scu_w)
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_MIRROR(0x21f00000) AM_SHARE("workram_h")
	AM_RANGE(0x20000000, 0x2007ffff) AM_ROM AM_SHARE("share6")  // bios mirror
	AM_RANGE(0x22000000, 0x24ffffff) AM_ROM AM_SHARE("share7")  // cart mirror
	AM_RANGE(0xc0000000, 0xc00007ff) AM_RAM // cache RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_mem, AS_PROGRAM, 16, saturn_state )
	AM_RANGE(0x000000, 0x0fffff) AM_RAM AM_SHARE("sound_ram")
	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE_LEGACY("scsp", scsp_r, scsp_w)
ADDRESS_MAP_END


/* keyboard code */
/* TODO: needs a proper keycode table */
INPUT_CHANGED_MEMBER(saturn_state::key_stroke)
{
	if(newval && !oldval)
	{
		m_keyb.data = ((UINT8)(FPTR)(param) & 0xff);
		m_keyb.status |= 8;
	}

	if(oldval && !newval)
	{
		//m_keyb.status &= ~8;
		m_keyb.data = 0;
	}
}

#define SATURN_PAD_P1(_mask_, _val_) \
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 A") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 C") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 B") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P1 R") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 X") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Y") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Z") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("P1 L") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) //read '1' when direct mode is polled

#define SATURN_PAD_P2(_mask_, _val_) \
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 A") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 C") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 B") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P2 R") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 X") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Y") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Z") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("P2 L") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0007, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) //read '1' when direct mode is polled

#define MD_PAD_P1(_mask_, _val_) \
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 A") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 C") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 B") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P1 Mode") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 X") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Y") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Z") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) //read '1' when direct mode is polled

#define MD_PAD_P2(_mask_, _val_) \
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 A") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 C") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 B") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P2 Mode") PORT_PLAYER(2)  PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 X") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Y") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Z") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) \
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("INPUT_TYPE", _mask_, EQUALS, _val_) //read '1' when direct mode is polled

#define SATURN_KEYBOARD PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x05)

INPUT_CHANGED_MEMBER(saturn_state::nmi_reset)
{
	/* TODO: correct? */
	if(!m_NMI_reset)
		return;

	/* TODO: NMI doesn't stay held on SH-2 core so we can't use ASSERT_LINE/CLEAR_LINE with that yet */
	if(newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER(saturn_state::tray_open)
{
	if(newval)
		stvcd_set_tray_open(machine());
}

INPUT_CHANGED_MEMBER(saturn_state::tray_close)
{
	if(newval)
		stvcd_set_tray_close(machine());
}

static INPUT_PORTS_START( saturn )
	PORT_START("RESET") /* hardwired buttons */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, nmi_reset,0) PORT_NAME("Reset Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, tray_open,0) PORT_NAME("Tray Open Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, tray_close,0) PORT_NAME("Tray Close")

	PORT_START("JOY1")
	SATURN_PAD_P1(0x0f, 0)

	PORT_START("JOY2")
	SATURN_PAD_P2(0xf0, 0)

	/* TODO: there's no info about the keycode used on Saturn keyboard, following is trial & error with Game Basic software */
	PORT_START("KEY0") // 0x00 - 0x07
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") /*PORT_CODE(KEYCODE_F1)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x01) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x02) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") /*PORT_CODE(KEYCODE_F2)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x03) PORT_PLAYER(1) SATURN_KEYBOARD // RUN
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") /*PORT_CODE(KEYCODE_F3)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x04) PORT_PLAYER(1) SATURN_KEYBOARD // LIST
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") /*PORT_CODE(KEYCODE_F4)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x05) PORT_PLAYER(1) SATURN_KEYBOARD // EDIT
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") /*PORT_CODE(KEYCODE_F5)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x06) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CLR SCR") PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x07) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY1") // 0x08 - 0x0f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x08) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F6") /*PORT_CODE(KEYCODE_F6)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x09) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F7") /*PORT_CODE(KEYCODE_F7)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x0a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F8") /*PORT_CODE(KEYCODE_F8)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x0b) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F9") /*PORT_CODE(KEYCODE_F9)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x0c) PORT_PLAYER(1) SATURN_KEYBOARD // LIST again
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x0d) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x0e) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x0f) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY2") // 0x10 - 0x17
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x10) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x11) PORT_PLAYER(1) SATURN_KEYBOARD
	/* TODO: break codes! */
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x12) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("KANA SHIFT") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x13) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(special keys)") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x14) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x15) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x16) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x17) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY3") // 0x18 - 0x1f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x18) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x19) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x1a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x1b) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x1c) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x1d) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x1e) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x1f) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY4") // 0x20 - 0x27
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x20) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x21) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x22) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x23) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x24) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x25) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x26) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x27) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY5") // 0x28 - 0x2f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-1") /*PORT_CODE(KEYCODE_F) PORT_CHAR('F')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x28) PORT_PLAYER(1) SATURN_KEYBOARD // another F?
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x29) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x2a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x2b) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x2c) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x2d) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x2e) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x2f) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY6") // 0x30 - 0x37
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x30) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x31) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x32) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x33) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x34) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x35) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x36) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x37) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY7") // 0x38 - 0x3f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x38) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x39) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x3a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x3b) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x3c) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x3d) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x3e) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x3f) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY8") // 0x40 - 0x47
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x40) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x41) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x42) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x43) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x44) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x45) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x46) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x47) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEY9") // 0x48 - 0x4f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x48) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x49) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x4a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x4b) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x4c) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x4d) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- / =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x4e) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x4f) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEYA") // 0x50 - 0x57
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x50) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xC2\xA5") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x51) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x52) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x53) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x54) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x55) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x56) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x57) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEYB") // 0x58 - 0x5f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x58) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x59) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d) PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x5a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x5b) PORT_PLAYER(1) SATURN_KEYBOARD // {
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x5c) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x5d) PORT_PLAYER(1) SATURN_KEYBOARD // }
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x5e) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x5f) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEYC") // 0x60 - 0x67
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x60) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x61) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x62) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x63) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x64) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x65) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) /* PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x66) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x67) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEYD") // 0x68 - 0x6f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x68) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x69) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x6a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x6b) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x6c) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x6d) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x6e) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x6f) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEYE") // 0x70 - 0x77
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x70) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x71) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x72) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x73) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x74) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x75) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x76) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E-8") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x77) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("KEYF") // 0x78 - 0x7f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-1") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x78) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-2") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x79) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-3") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x7a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x7b) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-5") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x7c) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x7d) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x7e) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x7f) PORT_PLAYER(1) SATURN_KEYBOARD //SYSTEM CONFIGURATION

	PORT_START("KEYS_1") // special keys
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) /*PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x78) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) /*PORT_CHAR('1')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x79) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) /*PORT_CHAR('2')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x7a) PORT_PLAYER(1) SATURN_KEYBOARD
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) /*PORT_CHAR('3')*/ PORT_CHANGED_MEMBER(DEVICE_SELF, saturn_state, key_stroke, 0x7b) PORT_PLAYER(1) SATURN_KEYBOARD

	PORT_START("MOUSEB1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pointer Left Button") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pointer Right Button") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Pointer Middle Button") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("P1 Pointer Start Button") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x04)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Mouse Left Button") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x08)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Mouse Right Button") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x08)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Mouse Middle Button") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x08)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("P1 Mouse Start Button") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x08)

	PORT_START("MOUSEX1")
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_NAME("P1 Pointer X") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x04)
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_NAME("P1 Mouse X") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x08)

	PORT_START("MOUSEY1")
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_REVERSE PORT_NAME("P1 Pointer Y") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x04)
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_REVERSE PORT_NAME("P1 Mouse Y") PORT_PLAYER(1) PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x08)

	PORT_START("MOUSEB2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Pointer Left Button") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x40)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Pointer Right Button") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x40)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Pointer Middle Button") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x40)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("P2 Pointer Start Button") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x40)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Mouse Left Button") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x80)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Mouse Right Button") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x80)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 Mouse Middle Button") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x80)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("P2 Mouse Start Button") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x80)

	PORT_START("MOUSEX2")
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_NAME("P2 Pointer X") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x40)
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_NAME("P2 Mouse X") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x80)

	PORT_START("MOUSEY2")
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_REVERSE PORT_NAME("P2 Pointer Y") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x40)
	PORT_BIT(0xffff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_MINMAX(0x000, 0xffff) PORT_KEYDELTA(2) PORT_RESET PORT_REVERSE PORT_NAME("P2 Mouse Y") PORT_PLAYER(2) PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x80)

	PORT_START("AN_JOY1")
	SATURN_PAD_P1(0x0f, 0x01)   // racing device
	SATURN_PAD_P1(0x0f, 0x02)   // analog controller

	PORT_START("AN_JOY2")
	SATURN_PAD_P2(0xf0, 0x10)   // racing device
	SATURN_PAD_P2(0xf0, 0x20)   // analog controller

	PORT_START("AN_X1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(1) PORT_NAME("P1 Racing Stick X") PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(1) PORT_NAME("P1 AD Stick X") PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x02)

	PORT_START("AN_Y1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(1) PORT_NAME("P1 Racing Stick Y") PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(1) PORT_NAME("P1 AD Stick Y") PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x02)

	PORT_START("AN_Z1")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Z ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(1) PORT_NAME("P1 Racing Stick Z") PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Z ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(1) PORT_NAME("P1 AD Stick Z") PORT_CONDITION("INPUT_TYPE", 0x0f, EQUALS, 0x02)

	PORT_START("AN_X2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(2) PORT_NAME("P2 Racing Stick X") PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(2) PORT_NAME("P2 AD Stick X") PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x20)

	PORT_START("AN_Y2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(2) PORT_NAME("P2 Racing Stick Y") PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(2) PORT_NAME("P2 AD Stick Y") PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x20)

	PORT_START("AN_Z2")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Z ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(2) PORT_NAME("P2 Racing Stick Z") PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Z ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(200) PORT_PLAYER(2) PORT_NAME("P2 AD Stick Z") PORT_CONDITION("INPUT_TYPE", 0xf0, EQUALS, 0x20)

	PORT_START("MD_JOY1")
	MD_PAD_P1(0x0f, 0x06)   // MD 3 buttons pad
	MD_PAD_P1(0x0f, 0x07)   // MD 6 buttons pad

	PORT_START("MD_JOY2")
	MD_PAD_P2(0xf0, 0x60)   // MD 3 buttons pad
	MD_PAD_P2(0xf0, 0x70)   // MD 6 buttons pad

	PORT_START("CART_AREA")
	PORT_CONFNAME( 0x07, 0x06, "Cart Type" )
	PORT_CONFSETTING( 0x00, "None" )
//  PORT_CONFSETTING( 0x01, "4 Mbit backup RAM" )
//  PORT_CONFSETTING( 0x02, "8 Mbit backup RAM" )
//  PORT_CONFSETTING( 0x03, "16 Mbit backup RAM" )
//  PORT_CONFSETTING( 0x04, "32 Mbit backup RAM" )
	PORT_CONFSETTING( 0x05, "8 Mbit Cart RAM" )
	PORT_CONFSETTING( 0x06, "32 Mbit Cart RAM" )

	PORT_START("INPUT_TYPE")
	PORT_CONFNAME(0x0f,0x00,"Controller Port 1")
	PORT_CONFSETTING(0x00,"Digital Device (standard Saturn pad)")
	PORT_CONFSETTING(0x01,"Racing Device") /* steering wheel only */
	PORT_CONFSETTING(0x02,"Analog Device") //Nights pad?
//  PORT_CONFSETTING(0x03,"Lightgun Device")
	PORT_CONFSETTING(0x04,"Trackball") // TODO: according to the docs this ID is labeled "Pointing Device"
	PORT_CONFSETTING(0x05,"Keyboard Device")
	PORT_CONFSETTING(0x06,"Megadrive 3B Pad")
	PORT_CONFSETTING(0x07,"Megadrive 6B Pad")
	PORT_CONFSETTING(0x08,"Saturn Mouse")
//  PORT_CONFSETTING(0x09,"<unconnected>")
	PORT_CONFNAME(0xf0,0x00,"Controller Port 2")
	PORT_CONFSETTING(0x00,"Digital Device (standard Saturn pad)")
	PORT_CONFSETTING(0x10,"Racing Device")
	PORT_CONFSETTING(0x20,"Analog Device") //Nights pad?
//  PORT_CONFSETTING(0x30,"Lightgun Device")
	PORT_CONFSETTING(0x40,"Pointing Device")
//  PORT_CONFSETTING(0x50,"Keyboard Device")
	PORT_CONFSETTING(0x60,"Megadrive 3B Pad")
	PORT_CONFSETTING(0x70,"Megadrive 6B Pad")
	PORT_CONFSETTING(0x80,"Saturn Mouse")
	PORT_CONFSETTING(0x90,"<unconnected>")
INPUT_PORTS_END

#define STV_PLAYER_INPUTS(_n_, _b1_, _b2_, _b3_,_b4_)                       \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_##_b1_ ) PORT_PLAYER(_n_)            \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_##_b2_ ) PORT_PLAYER(_n_)            \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_##_b3_ ) PORT_PLAYER(_n_)            \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_##_b4_ ) PORT_PLAYER(_n_)            \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(_n_)     \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(_n_)       \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_)    \
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

	PORT_START("PORTA")
	STV_PLAYER_INPUTS(1, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("PORTB")
	STV_PLAYER_INPUTS(2, BUTTON1, BUTTON2, BUTTON3, BUTTON4)

	PORT_START("PORTC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1P Push Switch") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2P Push Switch") PORT_CODE(KEYCODE_8)

	PORT_START("PORTE")
	STV_PLAYER_INPUTS(3, BUTTON1, BUTTON2, BUTTON3, START)

	PORT_START("PORTF")
	STV_PLAYER_INPUTS(4, BUTTON1, BUTTON2, BUTTON3, START)
INPUT_PORTS_END

static INPUT_PORTS_START( stv6b )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Extra button layout, used by Power Instinct 3, Suikoenbu, Elan Doree, Golden Axe Duel & Astra SuperStars */
	PORT_MODIFY("PORTF")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( critcrsh )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hammer Hit")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("LIGHTX") /* mask default type                     sens delta min max */
	PORT_BIT( 0x3f, 0x00, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,0x3f) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("LIGHTY")
	PORT_BIT( 0x3f, 0x00, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x0,0x3f) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(1)
INPUT_PORTS_END

/* Same as the regular one, but with an additional & optional mahjong panel */
static INPUT_PORTS_START( stvmp )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Mahjong panel/player 1 side */
	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K )

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L )

	PORT_START("P1_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Mahjong panel/player 2 side */
	PORT_START("P2_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)

	PORT_START("P2_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)

	PORT_START("P2_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)

	PORT_START("P2_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)

	PORT_START("P2_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Micronet layout, routes joystick port to the mux! */
static INPUT_PORTS_START( myfairld )
	PORT_INCLUDE( stv )

	PORT_MODIFY("PORTA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTC")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PORTF")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_M )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_I )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BET )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_N )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_J )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_K )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )  PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("IO_TYPE", 0x01, EQUALS, 0x01)

	PORT_START("P1_KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) /* F/F is there, but these two games are single player so it isn't connected */

	PORT_START("P2_KEY0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IO_TYPE")
	PORT_CONFNAME( 0x01, 0x00, "I/O Device type" )
	PORT_CONFSETTING(    0x00, "Mahjong Panel" )
	PORT_CONFSETTING(    0x01, "Joystick" )
INPUT_PORTS_END

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
	32*8    /* really 64*8, but granularity is 32 bytes */
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
	64*16   /* really 128*16, but granularity is 32 bytes */
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
		state->m_scsp_last_line = irq;
		device->machine().device("audiocpu")->execute().set_input_line(irq, ASSERT_LINE);
	}
	else if (irq < 0)
	{
		device->machine().device("audiocpu")->execute().set_input_line(-irq, CLEAR_LINE);
	}
	else
	{
		device->machine().device("audiocpu")->execute().set_input_line(state->m_scsp_last_line, CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER(saturn_state::scsp_to_main_irq)
{
	if(!(m_scu.ism & IRQ_SOUND_REQ))
	{
		m_maincpu->set_input_line_and_vector(9, HOLD_LINE, 0x46);
		scu_do_transfer(5);
	}
	else
		m_scu.ist |= (IRQ_SOUND_REQ);
}

static const scsp_interface scsp_config =
{
	0,
	scsp_irq,
	DEVCB_DRIVER_LINE_MEMBER(saturn_state, scsp_to_main_irq)
};

TIMER_CALLBACK_MEMBER(saturn_state::stv_rtc_increment)
{
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	static int year_num, year_count;

	/*
	    m_smpc.rtc_data[0] = DectoBCD(systime.local_time.year /100);
	    m_smpc.rtc_data[1] = DectoBCD(systime.local_time.year %100);
	    m_smpc.rtc_data[2] = (systime.local_time.weekday << 4) | (systime.local_time.month+1);
	    m_smpc.rtc_data[3] = DectoBCD(systime.local_time.mday);
	    m_smpc.rtc_data[4] = DectoBCD(systime.local_time.hour);
	    m_smpc.rtc_data[5] = DectoBCD(systime.local_time.minute);
	    m_smpc.rtc_data[6] = DectoBCD(systime.local_time.second);
	*/

	m_smpc.rtc_data[6]++;

	/* seconds from 9 -> 10*/
	if((m_smpc.rtc_data[6] & 0x0f) >= 0x0a)         { m_smpc.rtc_data[6]+=0x10; m_smpc.rtc_data[6]&=0xf0; }
	/* seconds from 59 -> 0 */
	if((m_smpc.rtc_data[6] & 0xf0) >= 0x60)         { m_smpc.rtc_data[5]++;     m_smpc.rtc_data[6] = 0; }
	/* minutes from 9 -> 10 */
	if((m_smpc.rtc_data[5] & 0x0f) >= 0x0a)         { m_smpc.rtc_data[5]+=0x10; m_smpc.rtc_data[5]&=0xf0; }
	/* minutes from 59 -> 0 */
	if((m_smpc.rtc_data[5] & 0xf0) >= 0x60)         { m_smpc.rtc_data[4]++;     m_smpc.rtc_data[5] = 0; }
	/* hours from 9 -> 10 */
	if((m_smpc.rtc_data[4] & 0x0f) >= 0x0a)         { m_smpc.rtc_data[4]+=0x10; m_smpc.rtc_data[4]&=0xf0; }
	/* hours from 23 -> 0 */
	if((m_smpc.rtc_data[4] & 0xff) >= 0x24)             { m_smpc.rtc_data[3]++; m_smpc.rtc_data[2]+=0x10; m_smpc.rtc_data[4] = 0; }
	/* week day name sunday -> monday */
	if((m_smpc.rtc_data[2] & 0xf0) >= 0x70)             { m_smpc.rtc_data[2]&=0x0f; }
	/* day number 9 -> 10 */
	if((m_smpc.rtc_data[3] & 0x0f) >= 0x0a)             { m_smpc.rtc_data[3]+=0x10; m_smpc.rtc_data[3]&=0xf0; }

	// year BCD to dec conversion (for the leap year stuff)
	{
		year_num = (m_smpc.rtc_data[1] & 0xf);

		for(year_count = 0; year_count < (m_smpc.rtc_data[1] & 0xf0); year_count += 0x10)
			year_num += 0xa;

		year_num += (m_smpc.rtc_data[0] & 0xf)*0x64;

		for(year_count = 0; year_count < (m_smpc.rtc_data[0] & 0xf0); year_count += 0x10)
			year_num += 0x3e8;
	}

	/* month +1 check */
	/* the RTC have a range of 1980 - 2100, so we don't actually need to support the leap year special conditions */
	if(((year_num % 4) == 0) && (m_smpc.rtc_data[2] & 0xf) == 2)
	{
		if((m_smpc.rtc_data[3] & 0xff) >= dpm[(m_smpc.rtc_data[2] & 0xf)-1]+1+1)
			{ m_smpc.rtc_data[2]++; m_smpc.rtc_data[3] = 0x01; }
	}
	else if((m_smpc.rtc_data[3] & 0xff) >= dpm[(m_smpc.rtc_data[2] & 0xf)-1]+1){ m_smpc.rtc_data[2]++; m_smpc.rtc_data[3] = 0x01; }
	/* year +1 check */
	if((m_smpc.rtc_data[2] & 0x0f) > 12)                { m_smpc.rtc_data[1]++;  m_smpc.rtc_data[2] = (m_smpc.rtc_data[2] & 0xf0) | 0x01; }
	/* year from 9 -> 10 */
	if((m_smpc.rtc_data[1] & 0x0f) >= 0x0a)             { m_smpc.rtc_data[1]+=0x10; m_smpc.rtc_data[1]&=0xf0; }
	/* year from 99 -> 100 */
	if((m_smpc.rtc_data[1] & 0xf0) >= 0xa0)             { m_smpc.rtc_data[0]++; m_smpc.rtc_data[1] = 0; }

	// probably not SO precise, here just for reference ...
	/* year from 999 -> 1000 */
	//if((m_smpc.rtc_data[0] & 0x0f) >= 0x0a)               { m_smpc.rtc_data[0]+=0x10; m_smpc.rtc_data[0]&=0xf0; }
	/* year from 9999 -> 0 */
	//if((m_smpc.rtc_data[0] & 0xf0) >= 0xa0)               { m_smpc.rtc_data[0] = 0; } //roll over
}

MACHINE_START_MEMBER(saturn_state,stv)
{
	system_time systime;
	machine().base_datetime(systime);

	m_maincpu = downcast<legacy_cpu_device*>( machine().device<cpu_device>("maincpu") );
	m_slave = downcast<legacy_cpu_device*>( machine().device("slave") );
	m_audiocpu = downcast<legacy_cpu_device*>( machine().device<cpu_device>("audiocpu") );

	scsp_set_ram_base(machine().device("scsp"), m_sound_ram);

	// save states
	state_save_register_global_pointer(machine(), m_scu_regs, 0x100/4);
	state_save_register_global_pointer(machine(), m_scsp_regs,  0x1000/2);
	state_save_register_global(machine(), m_NMI_reset);
	state_save_register_global(machine(), m_en_68k);
//  state_save_register_global(machine(), scanline);
	state_save_register_global(machine(), m_smpc.IOSEL1);
	state_save_register_global(machine(), m_smpc.IOSEL2);
	state_save_register_global(machine(), m_smpc.EXLE1);
	state_save_register_global(machine(), m_smpc.EXLE2);
	state_save_register_global(machine(), m_smpc.PDR1);
	state_save_register_global(machine(), m_smpc.PDR2);
	state_save_register_global(machine(), m_port_sel);
	state_save_register_global(machine(), m_mux_data);
	state_save_register_global(machine(), m_scsp_last_line);

	stv_register_protection_savestates(machine()); // machine/stvprot.c

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(stvcd_exit), &machine()));

	m_smpc.rtc_data[0] = DectoBCD(systime.local_time.year /100);
	m_smpc.rtc_data[1] = DectoBCD(systime.local_time.year %100);
	m_smpc.rtc_data[2] = (systime.local_time.weekday << 4) | (systime.local_time.month+1);
	m_smpc.rtc_data[3] = DectoBCD(systime.local_time.mday);
	m_smpc.rtc_data[4] = DectoBCD(systime.local_time.hour);
	m_smpc.rtc_data[5] = DectoBCD(systime.local_time.minute);
	m_smpc.rtc_data[6] = DectoBCD(systime.local_time.second);

	m_stv_rtc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(saturn_state::stv_rtc_increment),this));
}


MACHINE_START_MEMBER(saturn_state,saturn)
{
	system_time systime;
	machine().base_datetime(systime);

	m_maincpu = downcast<legacy_cpu_device*>( machine().device<cpu_device>("maincpu") );
	m_slave = downcast<legacy_cpu_device*>( machine().device("slave") );
	m_audiocpu = downcast<legacy_cpu_device*>( machine().device<cpu_device>("audiocpu") );

	scsp_set_ram_base(machine().device("scsp"), m_sound_ram);

	// save states
	state_save_register_global_pointer(machine(), m_scu_regs, 0x100/4);
	state_save_register_global_pointer(machine(), m_scsp_regs,  0x1000/2);
	state_save_register_global(machine(), m_NMI_reset);
	state_save_register_global(machine(), m_en_68k);
	state_save_register_global(machine(), m_smpc.IOSEL1);
	state_save_register_global(machine(), m_smpc.IOSEL2);
	state_save_register_global(machine(), m_smpc.EXLE1);
	state_save_register_global(machine(), m_smpc.EXLE2);
	state_save_register_global(machine(), m_smpc.PDR1);
	state_save_register_global(machine(), m_smpc.PDR2);
//  state_save_register_global(machine(), m_port_sel);
//  state_save_register_global(machine(), mux_data);
	state_save_register_global(machine(), m_scsp_last_line);
	state_save_register_global(machine(), m_smpc.intback_stage);
	state_save_register_global(machine(), m_smpc.pmode);
	state_save_register_global(machine(), m_smpc.SR);
	state_save_register_global_array(machine(), m_smpc.SMEM);
	state_save_register_global_pointer(machine(), m_cart_dram, 0x400000/4);

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(stvcd_exit), &machine()));

	m_smpc.rtc_data[0] = DectoBCD(systime.local_time.year /100);
	m_smpc.rtc_data[1] = DectoBCD(systime.local_time.year %100);
	m_smpc.rtc_data[2] = (systime.local_time.weekday << 4) | (systime.local_time.month+1);
	m_smpc.rtc_data[3] = DectoBCD(systime.local_time.mday);
	m_smpc.rtc_data[4] = DectoBCD(systime.local_time.hour);
	m_smpc.rtc_data[5] = DectoBCD(systime.local_time.minute);
	m_smpc.rtc_data[6] = DectoBCD(systime.local_time.second);

	m_stv_rtc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(saturn_state::stv_rtc_increment),this));
}


/*
(Preliminary) explanation about this:
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


TIMER_DEVICE_CALLBACK_MEMBER(saturn_state::saturn_scanline)
{
	saturn_state *state = machine().driver_data<saturn_state>();
	int scanline = param;
	int max_y = machine().primary_screen->height();
	int y_step,vblank_line;

	y_step = 2;

	if((max_y == 263 && m_vdp2.pal == 0) || (max_y == 313 && m_vdp2.pal == 1))
		y_step = 1;

	vblank_line = (m_vdp2.pal) ? 288 : 240;

	//popmessage("%08x %d T0 %d T1 %d %08x",m_scu.ism ^ 0xffffffff,max_y,m_scu_regs[36],m_scu_regs[37],m_scu_regs[38]);

	if(scanline == (0)*y_step)
	{
		video_update_vdp1(machine());

		if(STV_VDP1_VBE)
			m_vdp1.framebuffer_clear_on_next_frame = 1;

		if(!(m_scu.ism & IRQ_VDP1_END))
		{
			m_maincpu->set_input_line_and_vector(0x2, HOLD_LINE, 0x4d);
			scu_do_transfer(6);
		}
		else
			m_scu.ist |= (IRQ_VDP1_END);
	}

	if(scanline == 0*y_step)
	{
		if(!(m_scu.ism & IRQ_VBLANK_OUT))
		{
			m_maincpu->set_input_line_and_vector(0xe, HOLD_LINE, 0x41);
			scu_do_transfer(1);
		}
		else
			m_scu.ist |= (IRQ_VBLANK_OUT);

	}
	else if(scanline == vblank_line*y_step)
	{
		if(!(m_scu.ism & IRQ_VBLANK_IN))
		{
			m_maincpu->set_input_line_and_vector(0xf, HOLD_LINE ,0x40);
			scu_do_transfer(0);
		}
		else
			m_scu.ist |= (IRQ_VBLANK_IN);
	}
	else if((scanline % y_step) == 0 && scanline < vblank_line*y_step)
	{
		if(!(m_scu.ism & IRQ_HBLANK_IN))
		{
			m_maincpu->set_input_line_and_vector(0xd, HOLD_LINE, 0x42);
			scu_do_transfer(2);
		}
		else
			m_scu.ist |= (IRQ_HBLANK_IN);
	}

	if(scanline == (m_scu_regs[36] & 0x3ff)*y_step)
	{
		if(!(m_scu.ism & IRQ_TIMER_0))
		{
			m_maincpu->set_input_line_and_vector(0xc, HOLD_LINE, 0x43 );
			scu_do_transfer(3);
		}
		else
			m_scu.ist |= (IRQ_TIMER_0);
	}

	/* TODO: this isn't completely correct */
	if(m_scu_regs[38] & 0x1)
	{
		if((!(m_scu_regs[38] & 0x100) && (scanline % y_step) == 0) ||
			((m_scu_regs[38] & 0x100) && (scanline == (m_scu_regs[36] & 0x3ff)*y_step)))
		{
			if(!(m_scu.ism & IRQ_TIMER_1))
			{
				m_maincpu->set_input_line_and_vector(0xb, HOLD_LINE, 0x44 );
				scu_do_transfer(4);
			}
			else
				m_scu.ist |= (IRQ_TIMER_1);
		}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(saturn_state::saturn_slave_scanline )
{
	int scanline = param;
	int max_y = machine().primary_screen->height();
	int y_step,vblank_line;

	y_step = 2;

	if((max_y == 263 && m_vdp2.pal == 0) || (max_y == 313 && m_vdp2.pal == 1))
		y_step = 1;

	vblank_line = (m_vdp2.pal) ? 288 : 240;

	if(scanline == vblank_line*y_step)
		m_slave->set_input_line_and_vector(0x6, HOLD_LINE, 0x43);
	else if((scanline % y_step) == 0 && scanline < vblank_line*y_step)
		m_slave->set_input_line_and_vector(0x2, HOLD_LINE, 0x41);
}

/* Die Hard Trilogy tests RAM address 0x25e7ffe bit 2 with Slave during FRT minit irq, in-development tool for breaking execution of it? */
READ32_MEMBER(saturn_state::saturn_null_ram_r)
{
	return 0xffffffff;
}

WRITE32_MEMBER(saturn_state::saturn_null_ram_w)
{
}

READ32_MEMBER(saturn_state::saturn_cart_dram0_r)
{
	return m_cart_dram[offset];
}

WRITE32_MEMBER(saturn_state::saturn_cart_dram0_w)
{
	COMBINE_DATA(&m_cart_dram[offset]);
}

READ32_MEMBER(saturn_state::saturn_cart_dram1_r)
{
	return m_cart_dram[offset+0x200000/4];
}

WRITE32_MEMBER(saturn_state::saturn_cart_dram1_w)
{
	COMBINE_DATA(&m_cart_dram[offset+0x200000/4]);
}

READ32_MEMBER(saturn_state::saturn_cs1_r)
{
	UINT32 res;

	res = 0;
	//res  = m_cart_backupram[offset*4+0] << 24;
	res |= m_cart_backupram[offset*2+0] << 16;
	//res |= m_cart_backupram[offset*4+2] << 8;
	res |= m_cart_backupram[offset*2+1] << 0;

	return res;
}

WRITE32_MEMBER(saturn_state::saturn_cs1_w)
{
	if(ACCESSING_BITS_16_23)
		m_cart_backupram[offset*2+0] = (data & 0x00ff0000) >> 16;
	if(ACCESSING_BITS_0_7)
		m_cart_backupram[offset*2+1] = (data & 0x000000ff) >> 0;
}

MACHINE_RESET_MEMBER(saturn_state,saturn)
{
	m_scsp_last_line = 0;

	// don't let the slave cpu and the 68k go anywhere
	machine().device("slave")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_smpc.SR = 0x40;   // this bit is always on according to docs

	m_en_68k = 0;
	m_NMI_reset = 0;

	m_scu_regs[31] = 0; //DMA_STATUS = 0;

	//memset(stv_m_workram_l, 0, 0x100000);
	//memset(stv_m_workram_h, 0, 0x100000);

	machine().device("maincpu")->set_unscaled_clock(MASTER_CLOCK_320/2);
	machine().device("slave")->set_unscaled_clock(MASTER_CLOCK_320/2);

	stvcd_reset( machine() );

	m_cart_type = ioport("CART_AREA")->read() & 7;

	machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02400000, 0x027fffff, read32_delegate(FUNC(saturn_state::saturn_null_ram_r),this), write32_delegate(FUNC(saturn_state::saturn_null_ram_w),this));
	machine().device("slave")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02400000, 0x027fffff, read32_delegate(FUNC(saturn_state::saturn_null_ram_r),this), write32_delegate(FUNC(saturn_state::saturn_null_ram_w),this));

	if(m_cart_type == 5)
	{
		//  AM_RANGE(0x02400000, 0x027fffff) AM_RAM //cart RAM area, dynamically allocated
		machine().device("maincpu")->memory().space(AS_PROGRAM).nop_readwrite(0x02400000, 0x027fffff);
		machine().device("slave")->memory().space(AS_PROGRAM).nop_readwrite(0x02400000, 0x027fffff);

		machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02400000, 0x0247ffff, read32_delegate(FUNC(saturn_state::saturn_cart_dram0_r),this), write32_delegate(FUNC(saturn_state::saturn_cart_dram0_w),this));
		machine().device("slave")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02400000, 0x0247ffff, read32_delegate(FUNC(saturn_state::saturn_cart_dram0_r),this), write32_delegate(FUNC(saturn_state::saturn_cart_dram0_w),this));
		machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02600000, 0x0267ffff, read32_delegate(FUNC(saturn_state::saturn_cart_dram1_r),this), write32_delegate(FUNC(saturn_state::saturn_cart_dram1_w),this));
		machine().device("slave")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02600000, 0x0267ffff, read32_delegate(FUNC(saturn_state::saturn_cart_dram1_r),this), write32_delegate(FUNC(saturn_state::saturn_cart_dram1_w),this));
	}

	if(m_cart_type == 6)
	{
		//  AM_RANGE(0x02400000, 0x027fffff) AM_RAM //cart RAM area, dynamically allocated
		machine().device("maincpu")->memory().space(AS_PROGRAM).nop_readwrite(0x02400000, 0x027fffff);
		machine().device("slave")->memory().space(AS_PROGRAM).nop_readwrite(0x02400000, 0x027fffff);

		machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02400000, 0x025fffff, read32_delegate(FUNC(saturn_state::saturn_cart_dram0_r),this), write32_delegate(FUNC(saturn_state::saturn_cart_dram0_w),this));
		machine().device("slave")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02400000, 0x025fffff, read32_delegate(FUNC(saturn_state::saturn_cart_dram0_r),this), write32_delegate(FUNC(saturn_state::saturn_cart_dram0_w),this));
		machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02600000, 0x027fffff, read32_delegate(FUNC(saturn_state::saturn_cart_dram1_r),this), write32_delegate(FUNC(saturn_state::saturn_cart_dram1_w),this));
		machine().device("slave")->memory().space(AS_PROGRAM).install_readwrite_handler(0x02600000, 0x027fffff, read32_delegate(FUNC(saturn_state::saturn_cart_dram1_r),this), write32_delegate(FUNC(saturn_state::saturn_cart_dram1_w),this));
	}

	machine().device("maincpu")->memory().space(AS_PROGRAM).nop_readwrite(0x04000000, 0x047fffff);
	machine().device("slave")->memory().space(AS_PROGRAM).nop_readwrite(0x04000000, 0x047fffff);

	if(m_cart_type > 0 && m_cart_type < 5)
	{
	//  AM_RANGE(0x04000000, 0x047fffff) AM_RAM //backup RAM area, dynamically allocated
		UINT32 mask;
		mask = 0x7fffff >> (4-m_cart_type);
		//mask = 0x7fffff >> 4-4 = 0x7fffff 32mbit
		//mask = 0x7fffff >> 4-3 = 0x3fffff 16mbit
		//mask = 0x7fffff >> 4-2 = 0x1fffff 8mbit
		//mask = 0x7fffff >> 4-1 = 0x0fffff 4mbit
		machine().device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_handler(0x04000000, 0x04000000 | mask, read32_delegate(FUNC(saturn_state::saturn_cs1_r),this), write32_delegate(FUNC(saturn_state::saturn_cs1_w),this));
		machine().device("slave")->memory().space(AS_PROGRAM).install_readwrite_handler(0x04000000, 0x04000000 | mask, read32_delegate(FUNC(saturn_state::saturn_cs1_r),this), write32_delegate(FUNC(saturn_state::saturn_cs1_w),this));
	}


	/* TODO: default value is probably 7 */
	m_scu.start_factor[0] = -1;
	m_scu.start_factor[1] = -1;
	m_scu.start_factor[2] = -1;

	m_vdp2.old_crmd = -1;
	m_vdp2.old_tvmd = -1;

	m_stv_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));
}


MACHINE_RESET_MEMBER(saturn_state,stv)
{
	m_scsp_last_line = 0;

	// don't let the slave cpu and the 68k go anywhere
	machine().device("slave")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_en_68k = 0;
	m_NMI_reset = 0;

	m_port_sel = m_mux_data = 0;

	machine().device("maincpu")->set_unscaled_clock(MASTER_CLOCK_320/2);
	machine().device("slave")->set_unscaled_clock(MASTER_CLOCK_320/2);

	stvcd_reset(machine());

	m_stv_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));
	m_prev_bankswitch = 0xff;

	/* TODO: default value is probably 7 */
	m_scu.start_factor[0] = -1;
	m_scu.start_factor[1] = -1;
	m_scu.start_factor[2] = -1;

	m_vdp2.old_crmd = -1;
	m_vdp2.old_tvmd = -1;
}

struct cdrom_interface saturn_cdrom =
{
	"sat_cdrom",
	NULL
};




static DEVICE_IMAGE_LOAD( sat_cart )
{
	UINT8 *ROM = image.device().machine().root_device().memregion("maincpu")->base()+0x080000;
	UINT32 length;

	if (image.software_entry() != NULL)
	{
		length = image.get_software_region_length("cart");
		UINT8* imagex =  image.get_software_region("cart");

		memcpy(ROM, imagex, length);
	}
	else
	{
		length = image.fread( ROM, 0x400000);
	}

	// fix endianness....
	for (int i=0;i<length;i+=4)
	{
		UINT8 tempa = ROM[i+0];
		UINT8 tempb = ROM[i+1];
		ROM[i+1] = ROM[i+2];
		ROM[i+0] = ROM[i+3];
		ROM[i+3] = tempa;
		ROM[i+2] = tempb;
	}

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( saturn, saturn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(saturn_mem)
	MCFG_CPU_CONFIG(sh2_conf_master)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", saturn_state, saturn_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("slave", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(saturn_mem)
	MCFG_CPU_CONFIG(sh2_conf_slave)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("slave_scantimer", saturn_state, saturn_slave_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", M68000, 11289600) //256 x 44100 Hz = 11.2896 MHz
	MCFG_CPU_PROGRAM_MAP(sound_mem)

	MCFG_MACHINE_START_OVERRIDE(saturn_state,saturn)
	MCFG_MACHINE_RESET_OVERRIDE(saturn_state,saturn)

	MCFG_NVRAM_HANDLER(saturn)

	MCFG_TIMER_ADD("sector_timer", stv_sector_cb)
	MCFG_TIMER_ADD("sh1_cmd", stv_sh1_sim)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_320/8, 427, 0, 320, 263, 0, 224)
	#if NEW_VIDEO_CODE
	MCFG_SCREEN_UPDATE_DRIVER(saturn_state, screen_update_saturn)
	MCFG_PALETTE_LENGTH(2048+(2048*2))//standard palette + extra memory for rgb brightness. (TODO: remove AT LEAST the latter)
	#else
	MCFG_SCREEN_UPDATE_DRIVER(saturn_state, screen_update_stv_vdp2)
	MCFG_PALETTE_LENGTH(2048+(2048*2))//standard palette + extra memory for rgb brightness.
	#endif

	MCFG_GFXDECODE(stv)

	MCFG_VIDEO_START_OVERRIDE(saturn_state,stv_vdp2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("scsp", SCSP, 0)
	MCFG_SOUND_CONFIG(scsp_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("cdda", CDDA, 0)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( saturnus, saturn )
	MCFG_CDROM_ADD( "cdrom",saturn_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","saturn")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","NTSC-U")

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_INTERFACE("sat_cart")
	MCFG_CARTSLOT_LOAD(sat_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","sat_cart")

MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( saturneu, saturn )
	MCFG_CDROM_ADD( "cdrom",saturn_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","saturn")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","PAL")

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_INTERFACE("sat_cart")
	MCFG_CARTSLOT_LOAD(sat_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","sat_cart")

MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( saturnjp, saturn )
	MCFG_CDROM_ADD( "cdrom",saturn_cdrom )
	MCFG_SOFTWARE_LIST_ADD("cd_list","saturn")
	MCFG_SOFTWARE_LIST_FILTER("cd_list","NTSC-J")

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_INTERFACE("sat_cart")
	MCFG_CARTSLOT_LOAD(sat_cart)
	MCFG_SOFTWARE_LIST_ADD("cart_list","sat_cart")

MACHINE_CONFIG_END


static MACHINE_CONFIG_START( stv, saturn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(stv_mem)
	MCFG_CPU_CONFIG(sh2_conf_master)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", saturn_state, saturn_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("slave", SH2, MASTER_CLOCK_352/2) // 28.6364 MHz
	MCFG_CPU_PROGRAM_MAP(stv_mem)
	MCFG_CPU_CONFIG(sh2_conf_slave)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("slave_scantimer", saturn_state, saturn_slave_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", M68000, 11289600) //11.2896 MHz
	MCFG_CPU_PROGRAM_MAP(sound_mem)

	MCFG_MACHINE_START_OVERRIDE(saturn_state,stv)
	MCFG_MACHINE_RESET_OVERRIDE(saturn_state,stv)

	MCFG_EEPROM_93C46_ADD("eeprom") /* Actually 93c45 */

	MCFG_TIMER_ADD("sector_timer", stv_sector_cb)
	MCFG_TIMER_ADD("sh1_cmd", stv_sh1_sim)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_320/8, 427, 0, 320, 263, 0, 224)
	#if NEW_VIDEO_CODE
	MCFG_SCREEN_UPDATE_DRIVER(saturn_state, screen_update_saturn)
	MCFG_PALETTE_LENGTH(2048+(2048*2))//standard palette + extra memory for rgb brightness. (TODO: remove AT LEAST the latter)
	#else
	MCFG_SCREEN_UPDATE_DRIVER(saturn_state, screen_update_stv_vdp2)
	MCFG_PALETTE_LENGTH(2048+(2048*2))//standard palette + extra memory for rgb brightness.
	#endif

	MCFG_GFXDECODE(stv)

	MCFG_VIDEO_START_OVERRIDE(saturn_state,stv_vdp2)

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
	{ ":cart1", 0, "game0" },
	{ ":cart2", 1, "game1" },
	{ ":cart3", 2, "game2" },
	{ ":cart4", 3, "game3" },
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

	UINT8 *ROM = image.device().machine().root_device().memregion(this_cart->region)->base();
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


void saturn_state::saturn_init_driver(int rgn)
{
	m_saturn_region = rgn;
	m_vdp2.pal = (rgn == 12) ? 1 : 0;

	// set compatible options
	sh2drc_set_options(machine().device("maincpu"), SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);
	sh2drc_set_options(machine().device("slave"), SH2DRC_STRICT_VERIFY|SH2DRC_STRICT_PCREL);

	/* amount of time to boost interleave for on MINIT / SINIT, needed for communication to work */
	m_minit_boost = 400;
	m_sinit_boost = 400;
	m_minit_boost_timeslice = attotime::zero;
	m_sinit_boost_timeslice = attotime::zero;

	m_scu_regs = auto_alloc_array(machine(), UINT32, 0x100/4);
	m_scsp_regs = auto_alloc_array(machine(), UINT16, 0x1000/2);
	m_cart_dram = auto_alloc_array(machine(), UINT32, 0x400000/4);
	m_backupram = auto_alloc_array(machine(), UINT8, 0x8000);
	m_cart_backupram = auto_alloc_array(machine(), UINT8, 0x400000);
}

DRIVER_INIT_MEMBER(saturn_state,saturnus)
{
	saturn_init_driver(4);
}

DRIVER_INIT_MEMBER(saturn_state,saturneu)
{
	saturn_init_driver(12);
}

DRIVER_INIT_MEMBER(saturn_state,saturnjp)
{
	saturn_init_driver(1);
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
//  ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
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
//  ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
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
//  ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)
ROM_END

ROM_START(vsaturn)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_LOAD("vsaturn.bin", 0x00000000, 0x00080000, CRC(e4d61811) SHA1(4154e11959f3d5639b11d7902b3a393a99fb5776))
//  ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)
ROM_END

ROM_START(hisaturn)
	ROM_REGION( 0x480000, "maincpu", ROMREGION_ERASEFF ) /* SH2 code */
	ROM_LOAD("hisaturn.bin", 0x00000000, 0x00080000, CRC(721e1b60) SHA1(49d8493008fa715ca0c94d99817a5439d6f2c796))
//  ROM_CART_LOAD("cart", 0x080000, 0x400000, ROM_NOMIRROR | ROM_OPTIONAL)
	ROM_REGION( 0x080000, "slave", 0 ) /* SH2 code */
	ROM_COPY( "maincpu",0,0,0x080000)
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE INPUT   INIT        COMPANY     FULLNAME            FLAGS */
CONS( 1994, saturn,     0,      0,      saturnus, saturn, saturn_state, saturnus,   "Sega",     "Saturn (USA)",     GAME_NOT_WORKING )
CONS( 1994, saturnjp,   saturn, 0,      saturnjp, saturn, saturn_state, saturnjp,   "Sega",     "Saturn (Japan)",   GAME_NOT_WORKING )
CONS( 1994, saturneu,   saturn, 0,      saturneu, saturn, saturn_state, saturneu,   "Sega",     "Saturn (PAL)",     GAME_NOT_WORKING )
CONS( 1995, vsaturn,    saturn, 0,      saturnjp, saturn, saturn_state, saturnjp,   "JVC",      "V-Saturn",         GAME_NOT_WORKING )
CONS( 1995, hisaturn,   saturn, 0,      saturnjp, saturn, saturn_state, saturnjp,   "Hitachi",  "HiSaturn",         GAME_NOT_WORKING )

#include "stv.c"
