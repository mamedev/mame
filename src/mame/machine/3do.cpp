// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Wilbert Pol
/*

Reset boot sequence:
- jump to ROM address space 03000028)
- determine source of reset from cstatbits (03000068)
- write cstatbits to safe madam location (03000078)
- clear cstatbits (0300007c)
- disable interrupts
- set up audout register
- set up brooktree part (03001020)
- wait 100ms
- set msysbits register for max memory config (030002d0)
- set sltime to 178906 (030002dc)
- write 0 to 0 to unmap ram address space (030002e4)
- do 8 reads to 4 banks memory tyest (030002fc)
- do cbr to both banks of vram (03000320)
- determine ram & vram sizes (03000330)
- do initial diagnostics, including test of initial 64kb of ram (0300021c)
- copy remainder of code to ram at address 0 (030000224)
- jump to address 0 and continue boot sequence (00000124)
- do some more memory checks (00000298)
- init stack pointer to 64k (00000054)
- set hdelay to c6
- do remaining diagnostics
- set up vdl for 3do logo screen (at 003b0000)
- set up frame buffer 3do logo screen (at 003c000, 0000166c)
00000064 (done)
- transfer sherry from rom to 10000
- transfer operator from rom to 20000
- transfer dipir from rom to 200
- transfer filesystem from rom to 28000
- init stack pointer to 64k (000000d8)
- store sherry address to 28 for use by dipir
- delay for at least 600ms to allow expansion bus to start
- wait for vcount = 10 and enable clut transfer
- wait for vcount = 10 and enable video
- set adbio to disable software controlled muting
- init registers for entry to sherry
- jump to sherry (00010000)

00010000
0001cbbc - ldr offset issue
0001cbcc

at address 00013914 is value 000031ED which gets read as 0031ED00. This breaks the executing code.
this data comes from 00010100 stored by the loop at 1cba4 (read from 00010100, store to 00013917)
this data comes from 0300d4f8 stored to 00010100 (done by loop at 000000a8)


Expansion bus stuff:
00022ba4 - init exp bus, write 17x 00 to the selection register to let all expansion devices determine their id on the bus.
00022bd0 - write 0x8f to the selection register to determine if there are too many devices attached.

(11-jan-2013)
"Error, could not open cdrom device"
- ARM finally uploads a program to DSPP, wants something back out of it (reads EO 0xee and unmasks DSPP irq).

*/

#include "emu.h"
#include "includes/3do.h"
#include "cpu/arm7/arm7core.h"
#include "debugger.h"

#define VERBOSE         1
#define LOG(x) do { if (VERBOSE) printf x; } while (0)

/*
IRQ0
0x80000000 Second Priority
0x40000000 SW irq
0x20000000 DMA<->EXP
0x1fff0000 DMA RAM->DSPP *
0x0000f000 DMA DSPP->RAM *
0x00000800 DSPP
0x00000400 Timer  1
0x00000200 Timer  3 <- needed to surpass current hang point
0x00000100 Timer  5
0x00000080 Timer  7
0x00000040 Timer  9
0x00000020 Timer 11
0x00000010 Timer 13
0x00000008 Timer 15
0x00000004 Expansion Bus
0x00000002 Vertical 1
0x00000001 Vertical 0

---
IRQ1
0x00000400 DSPPOVER (Red rev. only)
0x00000200 DSPPUNDER (Red rev. only)
0x00000100 BadBits
0x00000080 DMA<-External
0x00000040 DMA->External
0x00000020 DMA<-Uncle
0x00000010 DMA->Uncle
0x00000008 DMA RAM->DSPP N
0x00000004 SlowBus
0x00000002 Disk Inserted
0x00000001 DMA Player bus

*/
void _3do_state::m_3do_request_fiq(UINT32 irq_req, UINT8 type)
{
	if(type)
		m_clio.irq1 |= irq_req;
	else
		m_clio.irq0 |= irq_req;

	if(m_clio.irq1)
		m_clio.irq0 |= 1 << 31; // Second Priority
	else
		m_clio.irq0 &= ~(1 << 31);

	if((m_clio.irq0 & m_clio.irq0_enable) || (m_clio.irq1 & m_clio.irq1_enable))
	{
		//printf("Go irq %08x & %08x %08x & %08x\n",m_clio.irq0, m_clio.irq0_enable, m_clio.irq1, m_clio.irq1_enable);
		generic_pulse_irq_line(m_maincpu, ARM7_FIRQ_LINE, 1);
	}
}

/* TODO: timer frequency is unknown, everything else is guesswork. */
TIMER_DEVICE_CALLBACK_MEMBER( _3do_state::timer_x16_cb )
{
	/*
	    x--- fablode flag (wtf?)
	    -x-- cascade flag
	    --x- reload flag
	    ---x decrement flag (enable)
	*/
	UINT8 timer_flag;
	UINT8 carry_val;

	carry_val = 1;

	for(int i = 0;i < 16; i++)
	{
		timer_flag = (m_clio.timer_ctrl >> i*4) & 0xf;

		if(timer_flag & 1)
		{
			if(timer_flag & 4)
				m_clio.timer_count[i]-=carry_val;
			else
				m_clio.timer_count[i]--;

			if(m_clio.timer_count[i] == 0xffffffff) // timer hit
			{
				if(i & 1) // odd timer irq fires
					m_3do_request_fiq(8 << (7-(i >> 1)),0);

				carry_val = 1;

				if(timer_flag & 2)
				{
					m_clio.timer_count[i] = m_clio.timer_backup[i];
				}
				else
					m_clio.timer_ctrl &= ~(1 << i*4);
			}
			else
				carry_val = 0;
		}
	}
}

READ8_MEMBER(_3do_state::_3do_nvarea_r) { return m_nvmem[offset]; }
WRITE8_MEMBER(_3do_state::_3do_nvarea_w) { m_nvmem[offset] = data; }



/*
    I have no idea what piece of hardware this is. Possibly some kind of communication hardware using shift registers.

    During booting the following things get written/read:
    - write 03180000 to 03180000 (init reading sequence)
    - 4 read actions from 03180000 bit#0, if lower bit is 1 0 1 0 then   (expect to read 0x0a)
    - read from 03180000 (init writing sequence)
    - write 0x0100 to shift register bit#0
    - reset PON bit in CSTATBITS
    - wait a little while
    - read from 03180000
    - write 0x0200 to shift register bit#0
    - write 0x0002 to shift register bit#1
    - dummy write to 03180000
    - read from shift register bits #0 and #1.
    - check if data read equals 0x44696167 (=Diag)   If so, jump 302196c in default bios
    - dummy read from shift register
    - write 0x0300 to shift register bit #0
    - dummy write to shift register
    - read data from shift register bit #0.
    - xor that data with 0x07ff
    - write that data & 0x00ff | 0x4000 to the shift register
3022630
*/

READ32_MEMBER(_3do_state::_3do_slow2_r){
	UINT32 data = 0;

	logerror( "%08X: UNK_318 read offset = %08X\n", m_maincpu->pc(), offset );

	switch( offset ) {
	case 0:     /* Boot ROM checks here and expects to read 1, 0, 1, 0 in the lowest bit */
		data = m_slow2.cg_output & 0x00000001;
		m_slow2.cg_output = m_slow2.cg_output >> 1;
		m_slow2.cg_w_count = 0;
	}
	return data;
}


WRITE32_MEMBER(_3do_state::_3do_slow2_w)
{
	logerror( "%08X: UNK_318 write offset = %08X, data = %08X, mask = %08X\n", m_maincpu->pc(), offset, data, mem_mask );

	switch( offset )
	{
		case 0:     /* Boot ROM writes 03180000 here and then starts reading some things */
		{
			/* disable ROM overlay */
			m_bank1->set_entry(0);
		}
		m_slow2.cg_input = m_slow2.cg_input << 1 | ( data & 0x00000001 );
		m_slow2.cg_w_count ++;
		if ( m_slow2.cg_w_count == 16 )
		{
		}
		break;
	}
}



READ32_MEMBER(_3do_state::_3do_svf_r)
{
	UINT32 addr = ( offset & ( 0x07fc / 4 ) ) << 9;
	UINT32 *p = m_vram + addr;

	logerror( "%08X: SVF read offset = %08X\n", m_maincpu->pc(), offset*4 );

	switch( offset & ( 0xE000 / 4 ) )
	{
	case 0x0000/4:      /* SPORT transfer */
		for ( int i = 0; i < 512; i++ )
		{
			m_svf.sport[i] = p[i];
		}
		break;
	case 0x2000/4:      /* Write to color register */
		return m_svf.color;
	case 0x4000/4:      /* Flash write */
		break;
	case 0x6000/4:      /* CAS before RAS refresh/reset (CBR). Used to initialize VRAM mode during boot. */
		break;
	}
	return 0;
}

WRITE32_MEMBER(_3do_state::_3do_svf_w)
{
	UINT32 addr = ( offset & ( 0x07fc / 4 ) ) << 9;
	UINT32 *p = m_vram + addr;

	logerror( "%08X: SVF write offset = %08X, data = %08X, mask = %08X\n", m_maincpu->pc(), offset*4, data, mem_mask );

	switch( offset & ( 0xe000 / 4 ) )
	{
	case 0x0000/4:      /* SPORT transfer */
		{
			UINT32 keep_bits = data ^ 0xffffffff;

			for ( int i = 0; i < 512; i++ )
			{
				p[i] = ( p[i] & keep_bits ) | ( m_svf.sport[i] & data );
			}
		}
		break;
	case 0x2000/4:      /* Write to color register */
		m_svf.color = data;
		break;
	case 0x4000/4:      /* Flash write */
		{
			UINT32 keep_bits = data ^ 0xffffffff;
			UINT32 new_bits = m_svf.color & data;

			for ( int i = 0; i < 512; i++ )
			{
				p[i] = ( p[i] & keep_bits ) | new_bits;
			}
		}
		break;
	case 0x6000/4:      /* CAS before RAS refresh/reset (CBR). Used to initialize VRAM mode during boot. */
		break;
	}
}



READ32_MEMBER(_3do_state::_3do_madam_r){
	logerror( "%08X: MADAM read offset = %08X\n", m_maincpu->pc(), offset*4 );

	switch( offset ) {
	case 0x0000/4:      /* 03300000 - Revision */
		return m_madam.revision;
	case 0x0004/4:
		return m_madam.msysbits;
	case 0x0008/4:
		return m_madam.mctl;
	case 0x000c/4:
		return m_madam.sltime;
	case 0x0020/4:
		return m_madam.abortbits;
	case 0x0024/4:
		return m_madam.privbits;
	case 0x0028/4:
		return m_madam.statbits;
	case 0x0040/4:
		return m_madam.diag;
	case 0x0110/4:
		return m_madam.ccobctl0;
	case 0x0120/4:
		return m_madam.ppmpc;
	case 0x0130/4:
		return m_madam.regctl0;
	case 0x0134/4:
		return m_madam.regctl1;
	case 0x0138/4:
		return m_madam.regctl2;
	case 0x013c/4:
		return m_madam.regctl3;
	case 0x0140/4:
		return m_madam.xyposh;
	case 0x0144/4:
		return m_madam.xyposl;
	case 0x0148/4:
		return m_madam.linedxyh;
	case 0x014c/4:
		return m_madam.linedxyl;
	case 0x0150/4:
		return m_madam.dxyh;
	case 0x0154/4:
		return m_madam.dxyl;
	case 0x0158/4:
		return m_madam.ddxyh;
	case 0x015c/4:
		return m_madam.ddxyl;
	case 0x0180/4: case 0x0188/4:
	case 0x0190/4: case 0x0198/4:
	case 0x01a0/4: case 0x01a8/4:
	case 0x01b0/4: case 0x01b8/4:
	case 0x01c0/4: case 0x01c8/4:
	case 0x01d0/4: case 0x01d8/4:
	case 0x01e0/4: case 0x01e8/4:
	case 0x01f0/4: case 0x01f8/4:
		return m_madam.pip[(offset/2) & 0x0f] & 0xffff;
	case 0x0184/4: case 0x018c/4:
	case 0x0194/4: case 0x019c/4:
	case 0x01a4/4: case 0x01ac/4:
	case 0x01b4/4: case 0x01bc/4:
	case 0x01c4/4: case 0x01cc/4:
	case 0x01d4/4: case 0x01dc/4:
	case 0x01e4/4: case 0x01ec/4:
	case 0x01f4/4: case 0x01fc/4:
		return m_madam.pip[(offset/2) & 0x0f] >> 16;
	case 0x0200/4: case 0x0208/4:
	case 0x0210/4: case 0x0218/4:
	case 0x0220/4: case 0x0228/4:
	case 0x0230/4: case 0x0238/4:
	case 0x0240/4: case 0x0248/4:
	case 0x0250/4: case 0x0258/4:
	case 0x0260/4: case 0x0268/4:
	case 0x0270/4: case 0x0278/4:
		return m_madam.fence[(offset/2) & 0x0f] & 0xffff;
	case 0x0204/4: case 0x020c/4:
	case 0x0214/4: case 0x021c/4:
	case 0x0224/4: case 0x022c/4:
	case 0x0234/4: case 0x023c/4:
	case 0x0244/4: case 0x024c/4:
	case 0x0254/4: case 0x025c/4:
	case 0x0264/4: case 0x026c/4:
	case 0x0274/4: case 0x027c/4:
		return m_madam.fence[(offset/2) & 0x0f] >> 16;
	case 0x0300/4: case 0x0304/4: case 0x0308/4: case 0x030c/4:
	case 0x0310/4: case 0x0314/4: case 0x0318/4: case 0x031c/4:
	case 0x0320/4: case 0x0324/4: case 0x0328/4: case 0x032c/4:
	case 0x0330/4: case 0x0334/4: case 0x0338/4: case 0x033c/4:
	case 0x0340/4: case 0x0344/4: case 0x0348/4: case 0x034c/4:
	case 0x0350/4: case 0x0354/4: case 0x0358/4: case 0x035c/4:
	case 0x0360/4: case 0x0364/4: case 0x0368/4: case 0x036c/4:
	case 0x0370/4: case 0x0374/4: case 0x0378/4: case 0x037c/4:
	case 0x0380/4: case 0x0384/4: case 0x0388/4: case 0x038c/4:
	case 0x0390/4: case 0x0394/4: case 0x0398/4: case 0x039c/4:
	case 0x03a0/4: case 0x03a4/4: case 0x03a8/4: case 0x03ac/4:
	case 0x03b0/4: case 0x03b4/4: case 0x03b8/4: case 0x03bc/4:
	case 0x03c0/4: case 0x03c4/4: case 0x03c8/4: case 0x03cc/4:
	case 0x03d0/4: case 0x03d4/4: case 0x03d8/4: case 0x03dc/4:
	case 0x03e0/4: case 0x03e4/4: case 0x03e8/4: case 0x03ec/4:
	case 0x03f0/4: case 0x03f4/4: case 0x03f8/4: case 0x03fc/4:
		return m_madam.mmu[offset&0x3f];
	case 0x0400/4: case 0x0404/4: case 0x0408/4: case 0x040c/4:
	case 0x0410/4: case 0x0414/4: case 0x0418/4: case 0x041c/4:
	case 0x0420/4: case 0x0424/4: case 0x0428/4: case 0x042c/4:
	case 0x0430/4: case 0x0434/4: case 0x0438/4: case 0x043c/4:
	case 0x0440/4: case 0x0444/4: case 0x0448/4: case 0x044c/4:
	case 0x0450/4: case 0x0454/4: case 0x0458/4: case 0x045c/4:
	case 0x0460/4: case 0x0464/4: case 0x0468/4: case 0x046c/4:
	case 0x0470/4: case 0x0474/4: case 0x0478/4: case 0x047c/4:
	case 0x0480/4: case 0x0484/4: case 0x0488/4: case 0x048c/4:
	case 0x0490/4: case 0x0494/4: case 0x0498/4: case 0x049c/4:
	case 0x04a0/4: case 0x04a4/4: case 0x04a8/4: case 0x04ac/4:
	case 0x04b0/4: case 0x04b4/4: case 0x04b8/4: case 0x04bc/4:
	case 0x04c0/4: case 0x04c4/4: case 0x04c8/4: case 0x04cc/4:
	case 0x04d0/4: case 0x04d4/4: case 0x04d8/4: case 0x04dc/4:
	case 0x04e0/4: case 0x04e4/4: case 0x04e8/4: case 0x04ec/4:
	case 0x04f0/4: case 0x04f4/4: case 0x04f8/4: case 0x04fc/4:
	case 0x0500/4: case 0x0504/4: case 0x0508/4: case 0x050c/4:
	case 0x0510/4: case 0x0514/4: case 0x0518/4: case 0x051c/4:
	case 0x0520/4: case 0x0524/4: case 0x0528/4: case 0x052c/4:
	case 0x0530/4: case 0x0534/4: case 0x0538/4: case 0x053c/4:
	case 0x0540/4: case 0x0544/4: case 0x0548/4: case 0x054c/4:
	case 0x0550/4: case 0x0554/4: case 0x0558/4: case 0x055c/4:
	case 0x0560/4: case 0x0564/4: case 0x0568/4: case 0x056c/4:
	case 0x0570/4: case 0x0574/4: case 0x0578/4: case 0x057c/4:
	case 0x0580/4: case 0x0584/4: case 0x0588/4: case 0x058c/4:
	case 0x0590/4: case 0x0594/4: case 0x0598/4: case 0x059c/4:
	case 0x05a0/4: case 0x05a4/4: case 0x05a8/4: case 0x05ac/4:
	case 0x05b0/4: case 0x05b4/4: case 0x05b8/4: case 0x05bc/4:
	case 0x05c0/4: case 0x05c4/4: case 0x05c8/4: case 0x05cc/4:
	case 0x05d0/4: case 0x05d4/4: case 0x05d8/4: case 0x05dc/4:
	case 0x05e0/4: case 0x05e4/4: case 0x05e8/4: case 0x05ec/4:
	case 0x05f0/4: case 0x05f4/4: case 0x05f8/4: case 0x05fc/4:
		return m_madam.dma[(offset/4) & 0x1f][offset & 0x03];

	/* Hardware multiplier */
	case 0x0600/4: case 0x0604/4: case 0x0608/4: case 0x060c/4:
	case 0x0610/4: case 0x0614/4: case 0x0618/4: case 0x061c/4:
	case 0x0620/4: case 0x0624/4: case 0x0628/4: case 0x062c/4:
	case 0x0630/4: case 0x0634/4: case 0x0638/4: case 0x063c/4:
	case 0x0640/4: case 0x0644/4: case 0x0648/4: case 0x064c/4:
	case 0x0650/4: case 0x0654/4: case 0x0658/4: case 0x065c/4:
	case 0x0660/4: case 0x0664/4: case 0x0668/4: case 0x066c/4:
	case 0x0670/4: case 0x0674/4: case 0x0678/4: case 0x067c/4:
	case 0x0680/4: case 0x0684/4: case 0x0688/4: case 0x068c/4:
	case 0x0690/4: case 0x0694/4: case 0x0698/4: case 0x069c/4:
		return m_madam.mult[offset & 0x3f];
	case 0x07f0/4:
		return m_madam.mult_control;
	case 0x07f8/4:
		return m_madam.mult_status;
	default:
		logerror( "%08X: unhandled MADAM read offset = %08X\n", m_maincpu->pc(), offset*4 );
		break;
	}
	return 0;
}


WRITE32_MEMBER(_3do_state::_3do_madam_w){
	if(offset == 0)
	{
		if(data == 0x0a)
			logerror( "%08X: MADAM write offset = %08X, data = %08X (\\n), mask = %08X\n", m_maincpu->pc(), offset*4, data, mem_mask );
		else
			logerror( "%08X: MADAM write offset = %08X, data = %08X (%c), mask = %08X\n", m_maincpu->pc(), offset*4, data, data, mem_mask );
	}
	else
		logerror( "%08X: MADAM write offset = %08X, data = %08X, mask = %08X\n", m_maincpu->pc(), offset*4, data, mem_mask );

	switch( offset ) {
	case 0x0000/4:
		if(data == 0x0a)
			printf("\n");
		else
			printf("%c",data);
		break;
	case 0x0004/4:  /* 03300004 - Memory configuration 29 = 2MB DRAM, 1MB VRAM */
		m_madam.msysbits = data;
		break;
	case 0x0008/4:
		m_madam.mctl = data;
		break;
	case 0x000c/4:
		m_madam.sltime = data;
		break;
	case 0x0020/4:
		m_madam.abortbits = data;
		break;
	case 0x0024/4:
		m_madam.privbits = data;
		break;
	case 0x0028/4:
		m_madam.statbits = data;
		break; // <- this was a fall-through?
	case 0x0040/4:
		m_madam.diag = 1;
		break;

	/* CEL */
	case 0x0100/4:  /* 03300100 - SPRSTRT - Start the CEL engine (W) */
	case 0x0104/4:  /* 03300104 - SPRSTOP - Stop the CEL engine (W) */
	case 0x0108/4:  /* 03300108 - SPRCNTU - Continue the CEL engine (W) */
	case 0x010c/4:  /* 0330010c - SPRPAUS - Pause the CEL engine (W) */
		break;
	case 0x0110/4:  /* 03300110 - CCOBCTL0 - CCoB control (RW) */
		m_madam.ccobctl0 = data;
		break;
	case 0x0129/4:  /* 03300120 - PPMPC (RW) */
		m_madam.ppmpc = data;
		break;

	/* Regis */
	case 0x0130/4:
		m_madam.regctl0 = data;
		break;
	case 0x0134/4:
		m_madam.regctl1 = data;
		break;
	case 0x0138/4:
		m_madam.regctl2 = data;
		break;
	case 0x013c/4:
		m_madam.regctl3 = data;
		break;
	case 0x0140/4:
		m_madam.xyposh = data;
		break;
	case 0x0144/4:
		m_madam.xyposl = data;
		break;
	case 0x0148/4:
		m_madam.linedxyh = data;
		break;
	case 0x014c/4:
		m_madam.linedxyl = data;
		break;
	case 0x0150/4:
		m_madam.dxyh = data;
		break;
	case 0x0154/4:
		m_madam.dxyl = data;
		break;
	case 0x0158/4:
		m_madam.ddxyh = data;
		break;
	case 0x015c/4:
		m_madam.ddxyl = data;
		break;

	/* Pip */
	case 0x0180/4: case 0x0184/4: case 0x0188/4: case 0x018c/4:
	case 0x0190/4: case 0x0194/4: case 0x0198/4: case 0x019c/4:
	case 0x01a0/4: case 0x01a4/4: case 0x01a8/4: case 0x01ac/4:
	case 0x01b0/4: case 0x01b4/4: case 0x01b8/4: case 0x01bc/4:
		m_madam.pip[offset & 0x0f] = data;
		break;

	/* Fence */
	case 0x0200/4: case 0x0204/4: case 0x0208/4: case 0x020c/4:
	case 0x0210/4: case 0x0214/4: case 0x0218/4: case 0x021c/4:
	case 0x0220/4: case 0x0224/4: case 0x0228/4: case 0x022c/4:
	case 0x0230/4: case 0x0234/4: case 0x0238/4: case 0x023c/4:
		m_madam.fence[offset & 0x0f] = data;
		break;

	/* MMU */
	case 0x0300/4: case 0x0304/4: case 0x0308/4: case 0x030c/4:
	case 0x0310/4: case 0x0314/4: case 0x0318/4: case 0x031c/4:
	case 0x0320/4: case 0x0324/4: case 0x0328/4: case 0x032c/4:
	case 0x0330/4: case 0x0334/4: case 0x0338/4: case 0x033c/4:
	case 0x0340/4: case 0x0344/4: case 0x0348/4: case 0x034c/4:
	case 0x0350/4: case 0x0354/4: case 0x0358/4: case 0x035c/4:
	case 0x0360/4: case 0x0364/4: case 0x0368/4: case 0x036c/4:
	case 0x0370/4: case 0x0374/4: case 0x0378/4: case 0x037c/4:
	case 0x0380/4: case 0x0384/4: case 0x0388/4: case 0x038c/4:
	case 0x0390/4: case 0x0394/4: case 0x0398/4: case 0x039c/4:
	case 0x03a0/4: case 0x03a4/4: case 0x03a8/4: case 0x03ac/4:
	case 0x03b0/4: case 0x03b4/4: case 0x03b8/4: case 0x03bc/4:
	case 0x03c0/4: case 0x03c4/4: case 0x03c8/4: case 0x03cc/4:
	case 0x03d0/4: case 0x03d4/4: case 0x03d8/4: case 0x03dc/4:
	case 0x03e0/4: case 0x03e4/4: case 0x03e8/4: case 0x03ec/4:
	case 0x03f0/4: case 0x03f4/4: case 0x03f8/4: case 0x03fc/4:
		m_madam.mmu[offset&0x3f] = data;
		break;

	/* DMA */
	case 0x0400/4: case 0x0404/4: case 0x0408/4: case 0x040c/4:
	case 0x0410/4: case 0x0414/4: case 0x0418/4: case 0x041c/4:
	case 0x0420/4: case 0x0424/4: case 0x0428/4: case 0x042c/4:
	case 0x0430/4: case 0x0434/4: case 0x0438/4: case 0x043c/4:
	case 0x0440/4: case 0x0444/4: case 0x0448/4: case 0x044c/4:
	case 0x0450/4: case 0x0454/4: case 0x0458/4: case 0x045c/4:
	case 0x0460/4: case 0x0464/4: case 0x0468/4: case 0x046c/4:
	case 0x0470/4: case 0x0474/4: case 0x0478/4: case 0x047c/4:
	case 0x0480/4: case 0x0484/4: case 0x0488/4: case 0x048c/4:
	case 0x0490/4: case 0x0494/4: case 0x0498/4: case 0x049c/4:
	case 0x04a0/4: case 0x04a4/4: case 0x04a8/4: case 0x04ac/4:
	case 0x04b0/4: case 0x04b4/4: case 0x04b8/4: case 0x04bc/4:
	case 0x04c0/4: case 0x04c4/4: case 0x04c8/4: case 0x04cc/4:
	case 0x04d0/4: case 0x04d4/4: case 0x04d8/4: case 0x04dc/4:
	case 0x04e0/4: case 0x04e4/4: case 0x04e8/4: case 0x04ec/4:
	case 0x04f0/4: case 0x04f4/4: case 0x04f8/4: case 0x04fc/4:
	case 0x0500/4: case 0x0504/4: case 0x0508/4: case 0x050c/4:
	case 0x0510/4: case 0x0514/4: case 0x0518/4: case 0x051c/4:
	case 0x0520/4: case 0x0524/4: case 0x0528/4: case 0x052c/4:
	case 0x0530/4: case 0x0534/4: case 0x0538/4: case 0x053c/4:
	case 0x0540/4: case 0x0544/4: case 0x0548/4: case 0x054c/4:
	case 0x0550/4: case 0x0554/4: case 0x0558/4: case 0x055c/4:
	case 0x0560/4: case 0x0564/4: case 0x0568/4: case 0x056c/4:
	case 0x0570/4: case 0x0574/4: case 0x0578/4: case 0x057c/4:
	case 0x0580/4: case 0x0584/4: case 0x0588/4: case 0x058c/4:
	case 0x0590/4: case 0x0594/4: case 0x0598/4: case 0x059c/4:
	case 0x05a0/4: case 0x05a4/4: case 0x05a8/4: case 0x05ac/4:
	case 0x05b0/4: case 0x05b4/4: case 0x05b8/4: case 0x05bc/4:
	case 0x05c0/4: case 0x05c4/4: case 0x05c8/4: case 0x05cc/4:
	case 0x05d0/4: case 0x05d4/4: case 0x05d8/4: case 0x05dc/4:
	case 0x05e0/4: case 0x05e4/4: case 0x05e8/4: case 0x05ec/4:
	case 0x05f0/4: case 0x05f4/4: case 0x05f8/4: case 0x05fc/4:
		printf("%08x %08x\n",offset*4,data);
		m_madam.dma[(offset/4) & 0x1f][offset & 0x03] = data;
		return;

	/* Hardware multiplier */
	case 0x0600/4: case 0x0604/4: case 0x0608/4: case 0x060c/4:
	case 0x0610/4: case 0x0614/4: case 0x0618/4: case 0x061c/4:
	case 0x0620/4: case 0x0624/4: case 0x0628/4: case 0x062c/4:
	case 0x0630/4: case 0x0634/4: case 0x0638/4: case 0x063c/4:
	case 0x0640/4: case 0x0644/4: case 0x0648/4: case 0x064c/4:
	case 0x0650/4: case 0x0654/4: case 0x0658/4: case 0x065c/4:
	case 0x0660/4: case 0x0664/4: case 0x0668/4: case 0x066c/4:
	case 0x0670/4: case 0x0674/4: case 0x0678/4: case 0x067c/4:
	case 0x0680/4: case 0x0684/4: case 0x0688/4: case 0x068c/4:
	case 0x0690/4: case 0x0694/4: case 0x0698/4: case 0x069c/4:
		m_madam.mult[offset & 0x3f] = data;
	case 0x07f0/4:
		m_madam.mult_control |= data;
		break;
	case 0x07f4/4:
		m_madam.mult_control &= ~data;
		break;
	case 0x07fc/4:  /* Start process */
		break;

	default:
		logerror( "%08X: unhandled MADAM write offset = %08X, data = %08X, mask = %08X\n", m_maincpu->pc(), offset*4, data, mem_mask );
		break;
	}
}




READ32_MEMBER(_3do_state::_3do_clio_r)
{
	if (!space.debugger_access())
	{
		if(offset != 0x200/4 && offset != 0x40/4 && offset != 0x44/4 && offset != 0x48/4 && offset != 0x4c/4 &&
			offset != 0x118/4 && offset != 0x11c/4)
		logerror( "%08X: CLIO read offset = %08X\n", m_maincpu->pc(), offset * 4 );
	}

	/* TODO: for debug, to be removed once that we write the CPU core */
	if(offset >= 0x3800/4 && offset <= 0x39ff/4)
	{
		UINT32 res = 0;
		offset &= (0x1ff/4);
		res = (m_dspp.EO[(offset<<1)+0] << 16);
		res |= (m_dspp.EO[(offset<<1)+1] & 0xffff);
		return res;
	}

	if(offset >= 0x3c00/4 && offset <= 0x3fff/4)
	{
		UINT16 res;
		offset &= (0x3ff/4);
		res = m_dspp.EO[offset] & 0xffff;
		return res;
	}

	switch( offset )
	{
	case 0x0000/4:
		return m_clio.revision;
	case 0x0020/4:
		return m_clio.audin;
	case 0x0024/4:
		return m_clio.audout;
	case 0x0028/4:
		return m_clio.cstatbits;
	case 0x0030/4:
		return m_clio.screen->hpos();
	case 0x0034/4:
		/* This needs to moved to a proper timer callback function */
		if ( m_clio.screen->vpos() == 0 )
		{
			m_clio.vcnt ^= 0x800;
		}
		return ( m_clio.vcnt & 0x800 ) | m_clio.screen->vpos();
	case 0x0038/4:
		return m_clio.seed;
	case 0x003c/4:
		return m_clio.random;
	case 0x0040/4:
	case 0x0044/4:
		return m_clio.irq0;
	case 0x0048/4:
	case 0x004c/4:
		return m_clio.irq0_enable;
	case 0x0060/4:
	case 0x0064/4:
		return m_clio.irq1;
	case 0x0068/4:
	case 0x006c/4:
		return m_clio.irq1_enable;
	case 0x0080/4:
		return m_clio.hdelay;
	case 0x0084/4:
		return m_clio.adbio;
	case 0x0088/4:
		return m_clio.adbctl;

	case 0x0100/4:  case 0x0108/4:  case 0x0110/4:  case 0x0118/4:
	case 0x0120/4:  case 0x0128/4:  case 0x0130/4:  case 0x0138/4:
	case 0x0140/4:  case 0x0148/4:  case 0x0150/4:  case 0x0158/4:
	case 0x0160/4:  case 0x0168/4:  case 0x0170/4:  case 0x0178/4:
		return m_clio.timer_count[((offset & 0x3f) >> 1)+0];
	case 0x0104/4:  case 0x010c/4:  case 0x0114/4:  case 0x011c/4:
	case 0x0124/4:  case 0x012c/4:  case 0x0134/4:  case 0x013c/4:
	case 0x0144/4:  case 0x014c/4:  case 0x0154/4:  case 0x015c/4:
	case 0x0164/4:  case 0x016c/4:  case 0x0174/4:  case 0x017c/4:
		return m_clio.timer_backup[((offset & 0x3f) >> 1)];

	case 0x0200/4:
	case 0x0204/4:
		return m_clio.timer_ctrl;
	case 0x0208/4:
	case 0x020c/4:
		return m_clio.timer_ctrl >> 32;

	case 0x0220/4:
		return m_clio.slack;

	case 0x0400/4:
	case 0x0404/4:
		return m_clio.expctl;
	case 0x0410/4:
		return m_clio.dipir1;
	case 0x0414/4:
		return m_clio.dipir2;

	case 0x0500/4: case 0x0504/4: case 0x0508/4: case 0x050c/4:
	case 0x0510/4: case 0x0514/4: case 0x0518/4: case 0x051c/4:
	case 0x0520/4: case 0x0524/4: case 0x0528/4: case 0x052c/4:
	case 0x0530/4: case 0x0534/4: case 0x0538/4: case 0x053c/4:
		return m_clio.sel;

	case 0x0540/4: case 0x0544/4: case 0x0548/4: case 0x054c/4:
	case 0x0550/4: case 0x0554/4: case 0x0558/4: case 0x055c/4:
	case 0x0560/4: case 0x0564/4: case 0x0568/4: case 0x056c/4:
	case 0x0570/4: case 0x0574/4: case 0x0578/4: case 0x057c/4:
		return m_clio.poll;

	case 0xc000/4:
		return m_clio.unclerev;
	case 0xc004/4:
		return m_clio.uncle_soft_rev;
	case 0xc008/4:
		return m_clio.uncle_addr;
	case 0xc00c/4:
		return m_clio.uncle_rom;

	default:
	if (!space.debugger_access())
		logerror( "%08X: unhandled CLIO read offset = %08X\n", m_maincpu->pc(), offset * 4 );
		break;
	}
	return 0;
}

WRITE32_MEMBER(_3do_state::_3do_clio_w)
{
	if(offset != 0x200/4 && offset != 0x40/4 && offset != 0x44/4 && offset != 0x48/4 && offset != 0x4c/4 &&
		offset != 0x118/4 && offset != 0x11c/4)
		logerror( "%08X: CLIO write offset = %08X, data = %08X, mask = %08X\n", m_maincpu->pc(), offset*4, data, mem_mask );

	/* TODO: for debug, to be removed once that we write the CPU core */
	if(offset >= 0x1800/4 && offset <= 0x1fff/4)
	{
		offset &= (0x3ff/4);
		m_dspp.N[(offset<<1)+0] = data >> 16;
		m_dspp.N[(offset<<1)+1] = data & 0xffff;
		return;
	}

	if(offset >= 0x2000/4 && offset <= 0x2fff/4)
	{
		offset &= (0x7ff/4);
		m_dspp.N[offset] = data & 0xffff;
		return;
	}

	if(offset >= 0x3000/4 && offset <= 0x31ff/4)
	{
		offset &= (0x1ff/4);
		m_dspp.EI[(offset<<1)+0] = data >> 16;
		m_dspp.EI[(offset<<1)+1] = data & 0xffff;
		return;
	}

	if(offset >= 0x3400/4 && offset <= 0x37ff/4)
	{
		offset &= (0x3ff/4);
		m_dspp.EI[offset] = data & 0xffff;
		return;
	}

	switch( offset )
	{
	case 0x0000/4:
		break;
	case 0x0004/4:
		m_clio.csysbits = data;
		break;
	case 0x0008/4:
		m_clio.vint0 = data;
		break;
	case 0x000c/4:
		m_clio.vint1 = data;
		break;
	case 0x0020/4:
		m_clio.audin = data;
		break;
	case 0x0024/4:  /* 03400024 - c0020f0f is written here during boot */
		m_clio.audout = data;
		break;
	case 0x0028/4:  /* 03400028 - bits 0,1, and 6 are tested (reset source) */
		m_clio.cstatbits = data;
		break;
	case 0x002c/4:  /* 0340002C - ?? during boot 0000000B is written here counter reload related?? */
		m_clio.wdog = data;
		break;
	case 0x0030/4:
		m_clio.hcnt = data;
		break;
	case 0x0034/4:
		m_clio.vcnt = data;
		break;
	case 0x0038/4:
		m_clio.seed = data;
		break;
	case 0x0040/4:
		LOG(("%08x PEND0\n",data));
		m_clio.irq0 |= data;
		m_3do_request_fiq(0,0);
		break;
	case 0x0044/4:
		//LOG(("%08x PEND0 CLEAR\n",data));
		m_clio.irq0 &= ~data;
		m_3do_request_fiq(0,0);
		break;
	case 0x0048/4:
		LOG(("%08x MASK0\n",data));
		m_clio.irq0_enable |= data;
		m_3do_request_fiq(0,0);
		break;
	case 0x004c/4:
		LOG(("%08x MASK0 CLEAR\n",data));
		m_clio.irq0_enable &= ~data;
		m_3do_request_fiq(0,0);
		break;
	case 0x0050/4:
		m_clio.mode |= data;
		break;
	case 0x0054/4:
		m_clio.mode &= ~data;
		break;
	case 0x0058/4:
		m_clio.badbits = data;
		break;
	case 0x005c/4:
		m_clio.spare = data;
		break;
	case 0x0060/4:
		LOG(("%08x PEND1\n",data));
		m_clio.irq1 |= data;
		m_3do_request_fiq(0,1);
		break;
	case 0x0064/4:
		LOG(("%08x PEND1 CLEAR\n",data));
		m_clio.irq1 &= ~data;
		m_3do_request_fiq(0,1);
		break;
	case 0x0068/4:
		LOG(("%08x MASK1\n",data));
		m_clio.irq1_enable |= data;
		m_3do_request_fiq(0,1);
		break;
	case 0x006c/4:
		LOG(("%08x MASK1 CLEAR\n",data));
		m_clio.irq1_enable &= ~data;
		m_3do_request_fiq(0,1);
		break;
	case 0x0080/4:
		m_clio.hdelay = data;
		break;
	case 0x0084/4:
		m_clio.adbio = data;
		break;
	case 0x0088/4:
		m_clio.adbctl = data;
		break;

	/* only lower 16-bits can be written */
	case 0x0100/4:  case 0x0108/4:  case 0x0110/4:  case 0x0118/4:
	case 0x0120/4:  case 0x0128/4:  case 0x0130/4:  case 0x0138/4:
	case 0x0140/4:  case 0x0148/4:  case 0x0150/4:  case 0x0158/4:
	case 0x0160/4:  case 0x0168/4:  case 0x0170/4:  case 0x0178/4:
		m_clio.timer_count[((offset & 0x3f) >> 1)] = data & 0xffff;
		break;
	case 0x0104/4:  case 0x010c/4:  case 0x0114/4:  case 0x011c/4:
	case 0x0124/4:  case 0x012c/4:  case 0x0134/4:  case 0x013c/4:
	case 0x0144/4:  case 0x014c/4:  case 0x0154/4:  case 0x015c/4:
	case 0x0164/4:  case 0x016c/4:  case 0x0174/4:  case 0x017c/4:
		m_clio.timer_backup[((offset & 0x3f) >> 1)] = data & 0xffff;
		break;

	case 0x0200/4:
		m_clio.timer_ctrl |= (UINT64)data;
		break;
	case 0x0204/4:
		m_clio.timer_ctrl &= ~(UINT64)data;
		break;
	case 0x0208/4:
		m_clio.timer_ctrl |= ((UINT64)data << 32);
		break;
	case 0x020c/4:
		m_clio.timer_ctrl &= ~((UINT64)data << 32);
		break;

	case 0x0220/4:
		m_clio.slack = data & 0x000003ff;
		break;

	case 0x0304/4:
		if(data)
			printf("DMA %08x\n",data);

		break;

	case 0x0308/4:
		m_clio.dmareqdis = data;
		break;

	case 0x0400/4:
		m_clio.expctl = m_clio.expctl | ( data & 0xca00 );
		break;
	case 0x0404/4:
		m_clio.expctl = m_clio.expctl & ~( data & 0xca00 );
		break;
	case 0x0408/4:
		m_clio.type0_4 = data;
		break;

	case 0x0500/4: case 0x0504/4: case 0x0508/4: case 0x050c/4:
	case 0x0510/4: case 0x0514/4: case 0x0518/4: case 0x051c/4:
	case 0x0520/4: case 0x0524/4: case 0x0528/4: case 0x052c/4:
	case 0x0530/4: case 0x0534/4: case 0x0538/4: case 0x053c/4:
		m_clio.sel = data & 0xff;
		/* Start WRSEL cycle */

		/* Detection of too many devices on the bus */
		switch ( data & 0xff )
		{
		case 0x8f:
			/* Everything is fine, there are not too many devices in the system */
			m_clio.poll = ( m_clio.poll & 0x0f );
			break;
		default:
			m_clio.poll = ( m_clio.poll & 0x0f ) | 0x90;
		}
		break;

	case 0x0540/4: case 0x0544/4: case 0x0548/4: case 0x054c/4:
	case 0x0550/4: case 0x0554/4: case 0x0558/4: case 0x055c/4:
	case 0x0560/4: case 0x0564/4: case 0x0568/4: case 0x056c/4:
	case 0x0570/4: case 0x0574/4: case 0x0578/4: case 0x057c/4:
		m_clio.poll = ( m_clio.poll & 0xf8 ) | ( data & 0x07 );
		break;

	// DSPP operation
	/*
	---- x--- DSPPError
	---- -x-- DSPPReset
	---- --x- DSPPSleep
	---- ---x DSPPGW
	(Red revision)
	---x ---- DSPPError
	---- x--- DSPPReset
	---- -x-- DSPPSleep
	---- --x- DSPPGW-Step
	---- ---x DSPPGW
	*/
	case 0x17fc/4:
		/* TODO: DSPP enabled just before enabling DSPP irq! */
		if(data & 1)
			debugger_break(machine());

		//printf("%08x\n",data);
		break;

	case 0xc000/4:
	case 0xc004/4:
	case 0xc00c/4:
		break;
	case 0xc008/4:
		m_clio.uncle_addr = data;
		break;

	default:
		logerror( "%08X: unhandled CLIO write offset = %08X, data = %08X, mask = %08X\n", m_maincpu->pc(), offset*4, data, mem_mask );
		break;
	}
}


/* 9 -> 5 bits translation */

VIDEO_START_MEMBER(_3do_state,_3do)
{
	/* We only keep the odd bits and get rid of the even bits */
//  for ( int i = 0; i < 512; i++ )
//  {
//      m_video_bits[i] = ( i & 1 ) | ( ( i & 4 ) >> 1 ) | ( ( i & 0x10 ) >> 2 ) | ( ( i & 0x40 ) >> 3 ) | ( ( i & 0x100 ) >> 4 );
//  }
}


UINT32 _3do_state::screen_update__3do(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *source_p = m_vram + 0x1c0000 / 4;

	for ( int y = 0; y < 120; y++ )
	{
		UINT32  *dest_p0 = &bitmap.pix32(22 + y * 2, 254 );
		UINT32  *dest_p1 = &bitmap.pix32(22 + y * 2 + 1, 254 );

		for ( int x = 0; x < 320; x++ )
		{
			/* Every dword contains two pixels, upper word is top pixel, lower is bottom. */
			UINT32 lower = *source_p & 0xffff;
			UINT32 upper = ( *source_p >> 16 ) & 0xffff;
			int r, g, b;

			/* Format is RGB555 */
			r = (upper & 0x7c00) >> 10;
			g = (upper & 0x03e0) >> 5;
			b = (upper & 0x001f) >> 0;
			r = (r << 3) | (r & 7);
			g = (g << 3) | (g & 7);
			b = (b << 3) | (b & 7);

			dest_p0[0] = r << 16 | g << 8 | b;
			dest_p0[1] = r << 16 | g << 8 | b;
			dest_p0[2] = r << 16 | g << 8 | b;
			dest_p0[3] = r << 16 | g << 8 | b;

			r = (lower & 0x7c00) >> 10;
			g = (lower & 0x03e0) >> 5;
			b = (lower & 0x001f) >> 0;
			r = (r << 3) | (r & 7);
			g = (g << 3) | (g & 7);
			b = (b << 3) | (b & 7);

			dest_p1[0] = r << 16 | g << 8 | b;
			dest_p1[1] = r << 16 | g << 8 | b;
			dest_p1[2] = r << 16 | g << 8 | b;
			dest_p1[3] = r << 16 | g << 8 | b;

			source_p++;
			dest_p0 += 4;
			dest_p1 += 4;
		}
	}

	return 0;
}

/*
 *
 * Machine Inits
 *
 */

void _3do_state::m_3do_madam_init( void )
{
	memset( &m_madam, 0, sizeof(MADAM) );
	m_madam.revision = 0x01020000;
	m_madam.msysbits = 0x51;
}

void _3do_state::m_3do_slow2_init( void )
{
	m_slow2.cg_input = 0;
	m_slow2.cg_output = 0x00000005 - 1;
}

void _3do_state::m_3do_clio_init( screen_device *screen )
{
	memset( &m_clio, 0, sizeof(CLIO) );
	m_clio.screen = screen;
	m_clio.revision = 0x02022000 /* 0x04000000 */;
	m_clio.unclerev = 0x03800000;
	m_clio.expctl = 0x80;    /* ARM has the expansion bus */
	m_dspp.N = std::make_unique<UINT16[]>(0x800 );
	m_dspp.EI = std::make_unique<UINT16[]>(0x400 );
	m_dspp.EO = std::make_unique<UINT16[]>(0x400 );

	memset(m_dspp.N.get(), 0, sizeof(UINT16) * 0x400);
	memset(m_dspp.EI.get(), 0, sizeof(UINT16) * 0x400);
	memset(m_dspp.EO.get(), 0, sizeof(UINT16) * 0x400);

	save_pointer(NAME(m_dspp.N.get()), 0x800);
	save_pointer(NAME(m_dspp.EI.get()), 0x400);
	save_pointer(NAME(m_dspp.EO.get()), 0x400);
}
