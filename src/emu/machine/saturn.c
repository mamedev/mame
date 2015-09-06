// license:LGPL-2.1+
// copyright-holders:David Haywood, Angelo Salese, Olivier Galibert, Mariusz Wojcieszek, R. Belmont
/**************************************************************************************

    Sega Saturn (c) 1994 Sega

    @todo List of things that needs to be implemented:
    - There's definitely an ack mechanism in SCU irqs. This is almost surely done via
      the ISM register (i.e. going 0->1 to the given bit acks it).
    - There might be a delay to exactly when SCU irqs happens. This is due to the basic
      fact that SCU runs at 14-ish MHz, so it needs some time before actually firing the
      irq.
    - Vblank-Out actually happens at the last screen line, not at 0.
    - VDP2 V counter has a similar roll-back as MD correspondent register:
      vpos line 0 == 0x1ff (Vblank-Out happens here)
      vpos line 1 == 0
      ...
      vpos line 241 == 0xf0 (Vblank-In happens here)
      vpos line 246 == 0xf5
      vpos line 247 == 0x1ef (rolls back here)
      vpos line 263 == 0x1ff again
    - HBlank bit seems to follow a normal logic instead.
    - Timer 0 doesn't work if the TENB bit isn't enabled (documentation is a bit fussy
      over this).
    - Timer 0 fires at the HBlank-In signal, not before.
    - VDP2 H Counter actually counts x2 in non Hi-Res mode.
    - Timer 1 is definitely annoying. Starts from H-Blank signal and starts counting from
      that position.
      H counter value 0x282 (642) -> timer 1 fires at setting 1
      H counter value 0x284 (644) -> 2
      H counter value 0x2a0 (672) -> 0x10
      H counter value 0x2c0 (704) -> 0x20
      H counter value 0x300 (768) -> 0x40
      H counter value 0x340 (832) -> 0x60
      H counter value 0x352 (850) -> 0x69
      H counter value 0x000 (0)   -> 0x6a, V counter goes +1 here (max range?)
      H counter value 0x02c (44)  -> 0x80
      H counter value 0x0ec (236) -> 0xe0
      H counter value 0x12c (300) -> 0x100
    - Timer 1 seems to count backwards compared to Timer 0 from setting 0x6b onward.
    - Yabause claims that if VDP2 DISP bit isn't enabled then vblank irqs (hblank too?)
      doesn't happen.

**************************************************************************************/

#include "emu.h"
#include "includes/stv.h"
#include "cpu/sh2/sh2.h"
#include "cpu/scudsp/scudsp.h"

/* TODO: do this in a verboselog style */
#define LOG_CDB  0
#define LOG_SCU  1
#define LOG_IRQ  0
#define LOG_IOGA 0

int saturn_state::DectoBCD(int num)
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

/*These macros sets the various DMA status flags.*/
#define DnMV_1(_ch_) m_scu.status|=(0x10 << 4 * _ch_)
#define DnMV_0(_ch_) m_scu.status&=~(0x10 << 4 * _ch_)

/*For area checking*/
#define BIOS_BUS(var)   (var & 0x07f00000) == 0
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
			if(LOG_SCU && !space.debugger_access()) logerror("(PC=%08x) DMA status reg read\n",space.device().safe_pc());
			res = m_scu.status;
			break;
		case 0x7c/4:
			if(LOG_SCU && !space.debugger_access()) logerror("(PC=%08x) DMA status reg read\n",space.device().safe_pc());
			res = m_scu.status;
			break;
		case 0x80/4:
			res = m_scudsp->program_control_r(space, 0, mem_mask);
			break;
		case 0x8c/4:
			if(LOG_SCU && !space.debugger_access()) logerror( "DSP mem read at %08X\n", m_scu_regs[34]);
			res = m_scudsp->ram_address_r(space, 0, mem_mask);
			break;
		case 0xa0/4:
			if(LOG_SCU && !space.debugger_access()) logerror("(PC=%08x) IRQ mask reg read %08x MASK=%08x\n",space.device().safe_pc(),mem_mask,m_scu_regs[0xa0/4]);
			res = m_scu.ism;
			break;
		case 0xa4/4:
			if(LOG_SCU && !space.debugger_access()) logerror("(PC=%08x) IRQ status reg read MASK=%08x IST=%08x | ISM=%08x\n",space.device().safe_pc(),mem_mask,m_scu.ist,m_scu.ism);
			/* TODO: Bug! trips an HW fault. Basically, it tries to read the IST bit 1 with that irq enabled.
			   Densetsu no Ogre Battle doesn't like this, so it needs investigation ...
			*/
//          res = m_scu.ist | ~m_scu.ism;
			res = m_scu.ist;
			break;
		case 0xc8/4:
			if(LOG_SCU && !space.debugger_access()) logerror("(PC=%08x) SCU version reg read\n",space.device().safe_pc());
			res = 0x00000004;/*SCU Version 4, OK? */
			break;
		default:
			if(LOG_SCU && !space.debugger_access()) logerror("(PC=%08x) SCU reg read at %d = %08x\n",space.device().safe_pc(),offset,m_scu_regs[offset]);
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
			m_scudsp->program_control_w(space, 0, m_scu_regs[offset], mem_mask);
			if(LOG_SCU) logerror("SCU DSP: Program Control Port Access %08x\n",data);
			break;
		case 0x84/4:
			m_scudsp->program_w(space, 0, m_scu_regs[offset], mem_mask);
			if(LOG_SCU) logerror("SCU DSP: Program RAM Data Port Access %08x\n",data);
			break;
		case 0x88/4:
			m_scudsp->ram_address_control_w(space, 0,m_scu_regs[offset], mem_mask);
			if(LOG_SCU) logerror("SCU DSP: Data RAM Address Port Access %08x\n",data);
			break;
		case 0x8c/4:
			m_scudsp->ram_address_w(space, 0, m_scu_regs[offset], mem_mask);
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
			scu_test_pending_irq();
			break;
		case 0xa8/4:
			/* This sends an irq signal to the extra devices connected to the A-Bus, not really needed for now. */
			//if(LOG_SCU) logerror("A-Bus IRQ ACK %08x\n",m_scu_regs[42]);
			break;
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
	UINT32 tmp_src,tmp_dst,total_size;
	UINT8 cd_transfer_flag;

	if(m_scu.src_add[dma_ch] == 0 || (m_scu.dst_add[dma_ch] != 2 && m_scu.dst_add[dma_ch] != 4))
	{
	if(LOG_SCU) printf("DMA lv %d transfer START\n"
							"Start %08x End %08x Size %04x\n",dma_ch,m_scu.src[dma_ch],m_scu.dst[dma_ch],m_scu.size[dma_ch]);
	if(LOG_SCU) printf("Start Add %04x Destination Add %04x\n",m_scu.src_add[dma_ch],m_scu.dst_add[dma_ch]);
	}

	/* Game Basic and World Cup 98 trips this, according to the docs the SCU can't transfer from BIOS area (can't communicate from/to that bus) */
	if(BIOS_BUS(m_scu.src[dma_ch]))
	{
		popmessage("Warning: SCU transfer from BIOS area, contact MAMEdev");
		if(!(m_scu.ism & IRQ_DMAILL))
			m_maincpu->set_input_line_and_vector(3, HOLD_LINE, 0x4c);
		else
			m_scu.ist |= (IRQ_DMAILL);
		return;
	}

	DnMV_1(dma_ch);

	/* max size */
	if(m_scu.size[dma_ch] == 0) { m_scu.size[dma_ch] = (dma_ch == 0) ? 0x00100000 : 0x1000; }

	tmp_src = tmp_dst = 0;

	total_size = m_scu.size[dma_ch];
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

	/* Burning Rangers doesn't agree with this. */
//  m_scu.size[dma_ch] = 0;
	if(!(DRUP(dma_ch))) m_scu.src[dma_ch] = tmp_src;
	if(!(DWUP(dma_ch))) m_scu.dst[dma_ch] = tmp_dst;

	{
		/*TODO: Timing is a guess.  */
		switch(dma_ch)
		{
			case 0: machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(total_size/4), timer_expired_delegate(FUNC(saturn_state::dma_lv0_ended),this)); break;
			case 1: machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(total_size/4), timer_expired_delegate(FUNC(saturn_state::dma_lv1_ended),this)); break;
			case 2: machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(total_size/4), timer_expired_delegate(FUNC(saturn_state::dma_lv2_ended),this)); break;
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
	UINT32 total_size = 0;

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

		/* Guess: Size + data acquire (1 cycle for src/dst/size) */
		total_size += indirect_size + 3*4;

		//if(DRUP(0)) space.write_dword(tmp_src+8,m_scu.src[0]|job_done ? 0x80000000 : 0);
		//if(DWUP(0)) space.write_dword(tmp_src+4,m_scu.dst[0]);

		m_scu.index[dma_ch] = tmp_src+0xc;

	}while(job_done == 0);

	{
		/*TODO: change DMA into DRQ model. Timing is a guess.  */
		switch(dma_ch)
		{
			case 0: machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(total_size/4), timer_expired_delegate(FUNC(saturn_state::dma_lv0_ended),this)); break;
			case 1: machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(total_size/4), timer_expired_delegate(FUNC(saturn_state::dma_lv1_ended),this)); break;
			case 2: machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(total_size/4), timer_expired_delegate(FUNC(saturn_state::dma_lv2_ended),this)); break;
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
	machine().scheduler().synchronize(); // force resync
	m_slave->sh2_set_frt_input(PULSE_LINE);
}

WRITE32_MEMBER(saturn_state::sinit_w)
{
	//logerror("cpu %s (PC=%08X) SINIT write = %08x\n", space.device().tag(), space.device().safe_pc(),data);
	machine().scheduler().boost_interleave(m_sinit_boost_timeslice, attotime::from_usec(m_sinit_boost));
	machine().scheduler().synchronize(); // force resync
	m_maincpu->sh2_set_frt_input(PULSE_LINE);
}

/*
TODO:
Some games seems to not like either MAME's interleave system and/or SH-2 DRC, causing an hard crash.
Reported games are:
Blast Wind (before FMV)
Choro Q Park (car selection)
060311E4: MOV.L R14,@-SP ;R14 = 0x60ffba0 / R15 = 0x60ffba0
060311E6: MOV SP,R14 ;R14 = 0x60ffba0 / R15 = 0x60ffb9c / [0x60ffb9c] <- 0x60ffba0
060311E8: MOV.L @SP+,R14 ;R14 = 0x60ffb9c / R15 = 0x60ffb9c / [0x60ffb9c] -> R14
060311EA: RTS ;R14 = 0x60ffba0 / R15 = 0x60ffba0
060311EC: NOP
06031734: MULS.W R9, R8 ;R14 = 0x60ffba0 / R15 = 0x60ffba0 / EA = 0x60311E4
on DRC this becomes:
R14 0x6031b78 (cause of the crash later on), R15 = 0x60ffba4 and EA = 0

Shinrei Jusatsushi Taromaru (options menu)

*/

WRITE32_MEMBER(saturn_state::saturn_minit_w)
{
	//logerror("cpu %s (PC=%08X) MINIT write = %08x\n", space.device().tag(), space.device().safe_pc(),data);
	if(m_fake_comms->read() & 1)
		machine().scheduler().synchronize(); // force resync
	else
	{
		machine().scheduler().boost_interleave(m_minit_boost_timeslice, attotime::from_usec(m_minit_boost));
		machine().scheduler().trigger(1000);
	}

	m_slave->sh2_set_frt_input(PULSE_LINE);
}

WRITE32_MEMBER(saturn_state::saturn_sinit_w)
{
	//logerror("cpu %s (PC=%08X) SINIT write = %08x\n", space.device().tag(), space.device().safe_pc(),data);
	if(m_fake_comms->read() & 1)
		machine().scheduler().synchronize(); // force resync
	else
		machine().scheduler().boost_interleave(m_sinit_boost_timeslice, attotime::from_usec(m_sinit_boost));

	m_maincpu->sh2_set_frt_input(PULSE_LINE);
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

void saturn_state::scu_reset(void)
{
	m_scu.ism = 0xbfff;
	m_scu.ist = 0;
	m_scu.start_factor[0] = 7;
	m_scu.start_factor[1] = 7;
	m_scu.start_factor[2] = 7;
	m_scu.status = 0;
}

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

/* Official documentation says that the "RESET/TAS opcodes aren't supported", but Out Run definitely contradicts with it.
   Since that m68k can't reset itself via the RESET opcode I suppose that the SMPC actually do it by reading an i/o
   connected to this opcode. */
WRITE_LINE_MEMBER(saturn_state::m68k_reset_callback)
{
	machine().scheduler().timer_set(attotime::from_usec(100), timer_expired_delegate(FUNC(saturn_state::smpc_audio_reset_line_pulse), this));

	printf("m68k RESET opcode triggered\n");
}

WRITE8_MEMBER(saturn_state::scsp_irq)
{
	// don't bother the 68k if it's off
	if (!m_en_68k)
	{
		return;
	}

	if (offset != 0)
	{
		if (data == ASSERT_LINE) m_scsp_last_line = offset;
		m_audiocpu->set_input_line(offset, data);
	}
	else
	{
		m_audiocpu->set_input_line(m_scsp_last_line, data);
	}
}

WRITE_LINE_MEMBER(saturn_state::scsp_to_main_irq)
{
	if(state)
	{
		if(!(m_scu.ism & IRQ_SOUND_REQ))
		{
			m_maincpu->set_input_line_and_vector(9, HOLD_LINE, 0x46);
			scu_do_transfer(5);
		}
		else
			m_scu.ist |= (IRQ_SOUND_REQ);
	}
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
	int scanline = param;
	int y_step,vblank_line;

	vblank_line = get_vblank_start_position();
	y_step = get_ystep_count();

	//popmessage("%08x %d T0 %d T1 %d %08x",m_scu.ism ^ 0xffffffff,max_y,m_scu_regs[36],m_scu_regs[37],m_scu_regs[38]);

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

		/* TODO: when Automatic Draw actually happens? Night Striker S is very fussy on this, and it looks like that VDP1 starts at more or less vblank-in time ... */
		video_update_vdp1();
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

	if(scanline == (vblank_line+1)*y_step)
	{
		/* docs mentions that VBE happens one line after vblank-in. */
		if(STV_VDP1_VBE)
			m_vdp1.framebuffer_clear_on_next_frame = 1;
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
	int y_step,vblank_line;

	vblank_line = get_vblank_start_position();
	y_step = get_ystep_count();

	if(scanline == vblank_line*y_step)
		m_slave->set_input_line_and_vector(0x6, HOLD_LINE, 0x43);
	else if((scanline % y_step) == 0 && scanline < vblank_line*y_step)
		m_slave->set_input_line_and_vector(0x2, HOLD_LINE, 0x41);
}

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




GFXDECODE_START( stv )
	GFXDECODE_ENTRY( NULL, 0, tiles8x8x4_layout,   0x00, (0x80*(2+1))  )
	GFXDECODE_ENTRY( NULL, 0, tiles16x16x4_layout, 0x00, (0x80*(2+1))  )
	GFXDECODE_ENTRY( NULL, 0, tiles8x8x8_layout,   0x00, (0x08*(2+1))  )
	GFXDECODE_ENTRY( NULL, 0, tiles16x16x8_layout, 0x00, (0x08*(2+1))  )
GFXDECODE_END

WRITE_LINE_MEMBER(saturn_state::scudsp_end_w)
{
	if(state)
	{
		if(!(m_scu.ism & IRQ_DSP_END))
			m_maincpu->set_input_line_and_vector(0xa, HOLD_LINE, 0x45);
		else
			m_scu.ist |= (IRQ_DSP_END);
	}
}

READ16_MEMBER(saturn_state::scudsp_dma_r)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = offset;

//  printf("%08x\n",addr);

	return program.read_word(addr,mem_mask);
}


WRITE16_MEMBER(saturn_state::scudsp_dma_w)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t addr = offset;

//  printf("%08x %02x\n",addr,data);

	program.write_word(addr, data,mem_mask);
}
