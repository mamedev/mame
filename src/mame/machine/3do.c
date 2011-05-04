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

*/

#include "emu.h"
#include "includes/3do.h"






READ32_HANDLER( _3do_nvarea_r ) {
	logerror( "%08X: NVRAM read offset = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset );
	return 0;
}

WRITE32_HANDLER( _3do_nvarea_w ) {
	logerror( "%08X: NVRAM write offset = %08X, data = %08X, mask = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset, data, mem_mask );
}



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

READ32_HANDLER( _3do_slow2_r ) {
	_3do_state *state = space->machine().driver_data<_3do_state>();
	UINT32 data = 0;

	logerror( "%08X: UNK_318 read offset = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset );

	switch( offset ) {
	case 0:		/* Boot ROM checks here and expects to read 1, 0, 1, 0 in the lowest bit */
		data = state->m_slow2.cg_output & 0x00000001;
		state->m_slow2.cg_output = state->m_slow2.cg_output >> 1;
		state->m_slow2.cg_w_count = 0;
	}
	return data;
}


WRITE32_HANDLER( _3do_slow2_w )
{
	_3do_state *state = space->machine().driver_data<_3do_state>();
	logerror( "%08X: UNK_318 write offset = %08X, data = %08X, mask = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset, data, mem_mask );

	switch( offset )
	{
		case 0:		/* Boot ROM writes 03180000 here and then starts reading some things */
		{
			/* disable ROM overlay */
			memory_set_bank(space->machine(), "bank1", 0);
		}
		state->m_slow2.cg_input = state->m_slow2.cg_input << 1 | ( data & 0x00000001 );
		state->m_slow2.cg_w_count ++;
		if ( state->m_slow2.cg_w_count == 16 )
		{
		}
		break;
	}
}


void _3do_slow2_init( running_machine &machine )
{
	_3do_state *state = machine.driver_data<_3do_state>();
	state->m_slow2.cg_input = 0;
	state->m_slow2.cg_output = 0x00000005 - 1;
}


READ32_HANDLER( _3do_svf_r )
{
	_3do_state *state = space->machine().driver_data<_3do_state>();
	UINT32 addr = ( offset & ( 0x07fc / 4 ) ) << 9;
	UINT32 *p = state->m_vram + addr;

	logerror( "%08X: SVF read offset = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset*4 );

	switch( offset & ( 0xE000 / 4 ) )
	{
	case 0x0000/4:		/* SPORT transfer */
		for ( int i = 0; i < 512; i++ )
		{
			state->m_svf.sport[i] = p[i];
		}
		break;
	case 0x2000/4:		/* Write to color register */
		return state->m_svf.color;
	case 0x4000/4:		/* Flash write */
		break;
	case 0x6000/4:		/* CAS before RAS refresh/reset (CBR). Used to initialize VRAM mode during boot. */
		break;
	}
	return 0;
}

WRITE32_HANDLER( _3do_svf_w )
{
	_3do_state *state = space->machine().driver_data<_3do_state>();
	UINT32 addr = ( offset & ( 0x07fc / 4 ) ) << 9;
	UINT32 *p = state->m_vram + addr;

	logerror( "%08X: SVF write offset = %08X, data = %08X, mask = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset*4, data, mem_mask );

	switch( offset & ( 0xe000 / 4 ) )
	{
	case 0x0000/4:		/* SPORT transfer */
		{
			UINT32 keep_bits = data ^ 0xffffffff;

			for ( int i = 0; i < 512; i++ )
			{
				p[i] = ( p[i] & keep_bits ) | ( state->m_svf.sport[i] & data );
			}
		}
		break;
	case 0x2000/4:		/* Write to color register */
		state->m_svf.color = data;
		break;
	case 0x4000/4:		/* Flash write */
		{
			UINT32 keep_bits = data ^ 0xffffffff;
			UINT32 new_bits = state->m_svf.color & data;

			for ( int i = 0; i < 512; i++ )
			{
				p[i] = ( p[i] & keep_bits ) | new_bits;
			}
		}
		break;
	case 0x6000/4:		/* CAS before RAS refresh/reset (CBR). Used to initialize VRAM mode during boot. */
		break;
	}
}



READ32_HANDLER( _3do_madam_r ) {
	_3do_state *state = space->machine().driver_data<_3do_state>();
	logerror( "%08X: MADAM read offset = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset*4 );

	switch( offset ) {
	case 0x0000/4:		/* 03300000 - Revision */
		return state->m_madam.revision;
	case 0x0004/4:
		return state->m_madam.msysbits;
	case 0x0008/4:
		return state->m_madam.mctl;
	case 0x000c/4:
		return state->m_madam.sltime;
	case 0x0020/4:
		return state->m_madam.abortbits;
	case 0x0024/4:
		return state->m_madam.privbits;
	case 0x0028/4:
		return state->m_madam.statbits;
	case 0x0040/4:
		return state->m_madam.diag;
	case 0x0110/4:
		return state->m_madam.ccobctl0;
	case 0x0120/4:
		return state->m_madam.ppmpc;
	case 0x0130/4:
		return state->m_madam.regctl0;
	case 0x0134/4:
		return state->m_madam.regctl1;
	case 0x0138/4:
		return state->m_madam.regctl2;
	case 0x013c/4:
		return state->m_madam.regctl3;
	case 0x0140/4:
		return state->m_madam.xyposh;
	case 0x0144/4:
		return state->m_madam.xyposl;
	case 0x0148/4:
		return state->m_madam.linedxyh;
	case 0x014c/4:
		return state->m_madam.linedxyl;
	case 0x0150/4:
		return state->m_madam.dxyh;
	case 0x0154/4:
		return state->m_madam.dxyl;
	case 0x0158/4:
		return state->m_madam.ddxyh;
	case 0x015c/4:
		return state->m_madam.ddxyl;
	case 0x0180/4: case 0x0188/4:
	case 0x0190/4: case 0x0198/4:
	case 0x01a0/4: case 0x01a8/4:
	case 0x01b0/4: case 0x01b8/4:
	case 0x01c0/4: case 0x01c8/4:
	case 0x01d0/4: case 0x01d8/4:
	case 0x01e0/4: case 0x01e8/4:
	case 0x01f0/4: case 0x01f8/4:
		return state->m_madam.pip[(offset/2) & 0x0f] & 0xffff;
	case 0x0184/4: case 0x018c/4:
	case 0x0194/4: case 0x019c/4:
	case 0x01a4/4: case 0x01ac/4:
	case 0x01b4/4: case 0x01bc/4:
	case 0x01c4/4: case 0x01cc/4:
	case 0x01d4/4: case 0x01dc/4:
	case 0x01e4/4: case 0x01ec/4:
	case 0x01f4/4: case 0x01fc/4:
		return state->m_madam.pip[(offset/2) & 0x0f] >> 16;
	case 0x0200/4: case 0x0208/4:
	case 0x0210/4: case 0x0218/4:
	case 0x0220/4: case 0x0228/4:
	case 0x0230/4: case 0x0238/4:
	case 0x0240/4: case 0x0248/4:
	case 0x0250/4: case 0x0258/4:
	case 0x0260/4: case 0x0268/4:
	case 0x0270/4: case 0x0278/4:
		return state->m_madam.fence[(offset/2) & 0x0f] & 0xffff;
	case 0x0204/4: case 0x020c/4:
	case 0x0214/4: case 0x021c/4:
	case 0x0224/4: case 0x022c/4:
	case 0x0234/4: case 0x023c/4:
	case 0x0244/4: case 0x024c/4:
	case 0x0254/4: case 0x025c/4:
	case 0x0264/4: case 0x026c/4:
	case 0x0274/4: case 0x027c/4:
		return state->m_madam.fence[(offset/2) & 0x0f] >> 16;
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
		return state->m_madam.mmu[offset&0x3f];
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
		return state->m_madam.dma[(offset/4) & 0x1f][offset & 0x03];

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
		return state->m_madam.mult[offset & 0xff];
	case 0x07f0/4:
		return state->m_madam.mult_control;
	case 0x07f8/4:
		return state->m_madam.mult_status;
	default:
		logerror( "%08X: unhandled MADAM read offset = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset*4 );
		break;
	}
	return 0;
}


WRITE32_HANDLER( _3do_madam_w ) {
	_3do_state *state = space->machine().driver_data<_3do_state>();
	logerror( "%08X: MADAM write offset = %08X, data = %08X, mask = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset*4, data, mem_mask );

	switch( offset ) {
	case 0x0000/4:
		break;
	case 0x0004/4:	/* 03300004 - Memory configuration 29 = 2MB DRAM, 1MB VRAM */
		state->m_madam.msysbits = data;
		break;
	case 0x0008/4:
		state->m_madam.mctl = data;
		break;
	case 0x000c/4:
		state->m_madam.sltime = data;
		break;
	case 0x0020/4:
		state->m_madam.abortbits = data;
		break;
	case 0x0024/4:
		state->m_madam.privbits = data;
		break;
	case 0x0028/4:
		state->m_madam.statbits = data;
	case 0x0040/4:
		state->m_madam.diag = 1;
		break;

	/* CEL */
	case 0x0100/4:	/* 03300100 - SPRSTRT - Start the CEL engine (W) */
	case 0x0104/4:	/* 03300104 - SPRSTOP - Stop the CEL engine (W) */
	case 0x0108/4:	/* 03300108 - SPRCNTU - Continue the CEL engine (W) */
	case 0x010c/4:	/* 0330010c - SPRPAUS - Pause the the CEL engine (W) */
		break;
	case 0x0110/4:	/* 03300110 - CCOBCTL0 - CCoB control (RW) */
		state->m_madam.ccobctl0 = data;
		break;
	case 0x0129/4:	/* 03300120 - PPMPC (RW) */
		state->m_madam.ppmpc = data;
		break;

	/* Regis */
	case 0x0130/4:
		state->m_madam.regctl0 = data;
		break;
	case 0x0134/4:
		state->m_madam.regctl1 = data;
		break;
	case 0x0138/4:
		state->m_madam.regctl2 = data;
		break;
	case 0x013c/4:
		state->m_madam.regctl3 = data;
		break;
	case 0x0140/4:
		state->m_madam.xyposh = data;
		break;
	case 0x0144/4:
		state->m_madam.xyposl = data;
		break;
	case 0x0148/4:
		state->m_madam.linedxyh = data;
		break;
	case 0x014c/4:
		state->m_madam.linedxyl = data;
		break;
	case 0x0150/4:
		state->m_madam.dxyh = data;
		break;
	case 0x0154/4:
		state->m_madam.dxyl = data;
		break;
	case 0x0158/4:
		state->m_madam.ddxyh = data;
		break;
	case 0x015c/4:
		state->m_madam.ddxyl = data;
		break;

	/* Pip */
	case 0x0180/4: case 0x0184/4: case 0x0188/4: case 0x018c/4:
	case 0x0190/4: case 0x0194/4: case 0x0198/4: case 0x019c/4:
	case 0x01a0/4: case 0x01a4/4: case 0x01a8/4: case 0x01ac/4:
	case 0x01b0/4: case 0x01b4/4: case 0x01b8/4: case 0x01bc/4:
		state->m_madam.pip[offset & 0x0f] = data;
		break;

	/* Fence */
	case 0x0200/4: case 0x0204/4: case 0x0208/4: case 0x020c/4:
	case 0x0210/4: case 0x0214/4: case 0x0218/4: case 0x021c/4:
	case 0x0220/4: case 0x0224/4: case 0x0228/4: case 0x022c/4:
	case 0x0230/4: case 0x0234/4: case 0x0238/4: case 0x023c/4:
		state->m_madam.fence[offset & 0x0f] = data;
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
		state->m_madam.mmu[offset&0x3f] = data;
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
		state->m_madam.dma[(offset/4) & 0x1f][offset & 0x03] = data;
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
		state->m_madam.mult[offset & 0xff] = data;
	case 0x07f0/4:
		state->m_madam.mult_control |= data;
		break;
	case 0x07f4/4:
		state->m_madam.mult_control &= ~data;
		break;
	case 0x07fc/4:	/* Start process */
		break;

	default:
		logerror( "%08X: unhandled MADAM write offset = %08X, data = %08X, mask = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset*4, data, mem_mask );
		break;
	}
}


void _3do_madam_init( running_machine &machine )
{
	_3do_state *state = machine.driver_data<_3do_state>();
	memset( &state->m_madam, 0, sizeof(MADAM) );
	state->m_madam.revision = 0x01020000;
	state->m_madam.msysbits = 0x51;
}


READ32_HANDLER( _3do_clio_r )
{
	_3do_state *state = space->machine().driver_data<_3do_state>();
	logerror( "%08X: CLIO read offset = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset * 4 );

	switch( offset )
	{
	case 0x0000/4:
		return state->m_clio.revision;
	case 0x0020/4:
		return state->m_clio.audin;
	case 0x0024/4:
		return state->m_clio.audout;
	case 0x0028/4:
		return state->m_clio.cstatbits;
	case 0x0030/4:
		return state->m_clio.screen->hpos();
	case 0x0034/4:
		/* This needs to moved to a proper timer callback function */
		if ( state->m_clio.screen->vpos() == 0 )
		{
			state->m_clio.vcnt ^= 0x800;
		}
		return ( state->m_clio.vcnt & 0x800 ) | state->m_clio.screen->vpos();
	case 0x0038/4:
		return state->m_clio.seed;
	case 0x003c/4:
		return state->m_clio.random;
	case 0x0080/4:
		return state->m_clio.hdelay;
	case 0x0084/4:
		return state->m_clio.adbio;
	case 0x0088/4:
		return state->m_clio.adbctl;

	case 0x0100/4:
		return state->m_clio.timer0;
	case 0x0104/4:
		return state->m_clio.timerback0;
	case 0x0108/4:
		return state->m_clio.timer1;
	case 0x010c/4:
		return state->m_clio.timerback1;
	case 0x0110/4:
		return state->m_clio.timer2;
	case 0x0114/4:
		return state->m_clio.timerback2;
	case 0x0118/4:
		return state->m_clio.timer3;
	case 0x011c/4:
		return state->m_clio.timerback3;
	case 0x0120/4:
		return state->m_clio.timer4;
	case 0x0124/4:
		return state->m_clio.timerback4;
	case 0x0128/4:
		return state->m_clio.timer5;
	case 0x012c/4:
		return state->m_clio.timerback5;
	case 0x0130/4:
		return state->m_clio.timer6;
	case 0x0134/4:
		return state->m_clio.timerback6;
	case 0x0138/4:
		return state->m_clio.timer7;
	case 0x013c/4:
		return state->m_clio.timerback7;
	case 0x0140/4:
		return state->m_clio.timer8;
	case 0x0144/4:
		return state->m_clio.timerback8;
	case 0x0148/4:
		return state->m_clio.timer9;
	case 0x014c/4:
		return state->m_clio.timerback9;
	case 0x0150/4:
		return state->m_clio.timer10;
	case 0x0154/4:
		return state->m_clio.timerback10;
	case 0x0158/4:
		return state->m_clio.timer11;
	case 0x015c/4:
		return state->m_clio.timerback11;
	case 0x0160/4:
		return state->m_clio.timer12;
	case 0x0164/4:
		return state->m_clio.timerback12;
	case 0x0168/4:
		return state->m_clio.timer13;
	case 0x016c/4:
		return state->m_clio.timerback13;
	case 0x0170/4:
		return state->m_clio.timer14;
	case 0x0174/4:
		return state->m_clio.timerback14;
	case 0x0178/4:
		return state->m_clio.timer15;
	case 0x017c/4:
		return state->m_clio.timerback15;

	case 0x0200/4:
		return state->m_clio.settm0;
	case 0x0204/4:
		return state->m_clio.clrtm0;
	case 0x0208/4:
		return state->m_clio.settm1;
	case 0x020c/4:
		return state->m_clio.clrtm1;

	case 0x0220/4:
		return state->m_clio.slack;

	case 0x0400/4:
	case 0x0404/4:
		return state->m_clio.expctl;
	case 0x0410/4:
		return state->m_clio.dipir1;
	case 0x0414/4:
		return state->m_clio.dipir2;

	case 0x0500/4: case 0x0504/4: case 0x0508/4: case 0x050c/4:
	case 0x0510/4: case 0x0514/4: case 0x0518/4: case 0x051c/4:
	case 0x0520/4: case 0x0524/4: case 0x0528/4: case 0x052c/4:
	case 0x0530/4: case 0x0534/4: case 0x0538/4: case 0x053c/4:
		return state->m_clio.sel;

	case 0x0540/4: case 0x0544/4: case 0x0548/4: case 0x054c/4:
	case 0x0550/4: case 0x0554/4: case 0x0558/4: case 0x055c/4:
	case 0x0560/4: case 0x0564/4: case 0x0568/4: case 0x056c/4:
	case 0x0570/4: case 0x0574/4: case 0x0578/4: case 0x057c/4:
		return state->m_clio.poll;

	case 0xc000/4:
		return state->m_clio.unclerev;
	case 0xc004/4:
		return state->m_clio.uncle_soft_rev;
	case 0xc008/4:
		return state->m_clio.uncle_addr;
	case 0xc00c/4:
		return state->m_clio.uncle_rom;

	default:
		logerror( "%08X: unhandled CLIO read offset = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset * 4 );
		break;
	}
	return 0;
}

WRITE32_HANDLER( _3do_clio_w )
{
	_3do_state *state = space->machine().driver_data<_3do_state>();
	logerror( "%08X: CLIO write offset = %08X, data = %08X, mask = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset*4, data, mem_mask );

	switch( offset )
	{
	case 0x0000/4:
		break;
	case 0x0004/4:
		state->m_clio.csysbits = data;
		break;
	case 0x0008/4:
		state->m_clio.vint0 = data;
		break;
	case 0x000c/4:
		state->m_clio.vint1 = data;
		break;
	case 0x0020/4:
		state->m_clio.audin = data;
		break;
	case 0x0024/4:	/* 03400024 - c0020f0f is written here during boot */
		state->m_clio.audout = data;
		break;
	case 0x0028/4:	/* 03400028 - bits 0,1, and 6 are tested (reset source) */
		state->m_clio.cstatbits = data;
		break;
	case 0x002c/4:	/* 0340002C - ?? during boot 0000000B is written here counter reload related?? */
		state->m_clio.wdog = data;
		break;
	case 0x0030/4:
		state->m_clio.hcnt = data;
		break;
	case 0x0034/4:
		state->m_clio.vcnt = data;
		break;
	case 0x0038/4:
		state->m_clio.seed = data;
		break;
	case 0x0040/4:
		state->m_clio.irq0 |= data;
		break;
	case 0x0044/4:
		state->m_clio.irq0 &= ~data;
		break;
	case 0x0048/4:
		state->m_clio.irq0_enable |= data;
		break;
	case 0x004c/4:
		state->m_clio.irq0_enable &= ~data;
		break;
	case 0x0050/4:
		state->m_clio.mode |= data;
		break;
	case 0x0054/4:
		state->m_clio.mode &= ~data;
		break;
	case 0x0058/4:
		state->m_clio.badbits = data;
		break;
	case 0x005c/4:
		state->m_clio.spare = data;
		break;
	case 0x0060/4:
		state->m_clio.irq1 |= data;
		break;
	case 0x0064/4:
		state->m_clio.irq1 &= ~data;
		break;
	case 0x0068/4:
		state->m_clio.irq1_enable |= data;
		break;
	case 0x006c/4:
		state->m_clio.irq1_enable &= ~data;
		break;
	case 0x0080/4:
		state->m_clio.hdelay = data;
		break;
	case 0x0084/4:
		state->m_clio.adbio = data;
		break;
	case 0x0088/4:
		state->m_clio.adbctl = data;
		break;

	case 0x0100/4:
		state->m_clio.timer0 = data & 0x0000ffff;
		break;
	case 0x0104/4:
		state->m_clio.timerback0 = data & 0x0000ffff;
		break;
	case 0x0108/4:
		state->m_clio.timer1 = data & 0x0000ffff;
		break;
	case 0x010c/4:
		state->m_clio.timerback1 = data & 0x0000ffff;
		break;
	case 0x0110/4:
		state->m_clio.timer2 = data & 0x0000ffff;
		break;
	case 0x0114/4:
		state->m_clio.timerback2 = data & 0x0000ffff;
		break;
	case 0x0118/4:
		state->m_clio.timer3 = data & 0x0000ffff;
		break;
	case 0x011c/4:
		state->m_clio.timerback3 = data & 0x0000ffff;
		break;
	case 0x0120/4:
		state->m_clio.timer4 = data & 0x0000ffff;
		break;
	case 0x0124/4:
		state->m_clio.timerback4 = data & 0x0000ffff;
		break;
	case 0x0128/4:
		state->m_clio.timer5 = data & 0x0000ffff;
		break;
	case 0x012c/4:
		state->m_clio.timerback5 = data & 0x0000ffff;
		break;
	case 0x0130/4:
		state->m_clio.timer6 = data & 0x0000ffff;
		break;
	case 0x0134/4:
		state->m_clio.timerback6 = data & 0x0000ffff;
		break;
	case 0x0138/4:
		state->m_clio.timer7 = data & 0x0000ffff;
		break;
	case 0x013c/4:
		state->m_clio.timerback7 = data & 0x0000ffff;
		break;
	case 0x0140/4:
		state->m_clio.timer8 = data & 0x0000ffff;
		break;
	case 0x0144/4:
		state->m_clio.timerback8 = data & 0x0000ffff;
		break;
	case 0x0148/4:
		state->m_clio.timer9 = data & 0x0000ffff;
		break;
	case 0x014c/4:
		state->m_clio.timerback9 = data & 0x0000ffff;
		break;
	case 0x0150/4:
		state->m_clio.timer10 = data & 0x0000ffff;
		break;
	case 0x0154/4:
		state->m_clio.timerback10 = data & 0x0000ffff;
		break;
	case 0x0158/4:
		state->m_clio.timer11 = data & 0x0000ffff;
		break;
	case 0x015c/4:
		state->m_clio.timerback11 = data & 0x0000ffff;
		break;
	case 0x0160/4:
		state->m_clio.timer12 = data & 0x0000ffff;
		break;
	case 0x0164/4:
		state->m_clio.timerback12 = data & 0x0000ffff;
		break;
	case 0x0168/4:
		state->m_clio.timer13 = data & 0x0000ffff;
		break;
	case 0x016c/4:
		state->m_clio.timerback13 = data & 0x0000ffff;
		break;
	case 0x0170/4:
		state->m_clio.timer14 = data & 0x0000ffff;
		break;
	case 0x0174/4:
		state->m_clio.timerback14 = data & 0x0000ffff;
		break;
	case 0x0178/4:
		state->m_clio.timer15 = data & 0x0000ffff;
		break;
	case 0x017c/4:
		state->m_clio.timerback15 = data & 0x0000ffff;
		break;

	case 0x0200/4:
		state->m_clio.settm0 = data;
		break;
	case 0x0204/4:
		state->m_clio.clrtm0 = data;
		break;
	case 0x0208/4:
		state->m_clio.settm0 = data;
		break;
	case 0x020c/4:
		break;

	case 0x0220/4:
		state->m_clio.slack = data & 0x000003ff;
		break;

	case 0x0308/4:
		state->m_clio.dmareqdis = data;
		break;

	case 0x0400/4:
		state->m_clio.expctl = state->m_clio.expctl | ( data & 0xca00 );
		break;
	case 0x0404/4:
		state->m_clio.expctl = state->m_clio.expctl & ~( data & 0xca00 );
		break;
	case 0x0408/4:
		state->m_clio.type0_4 = data;
		break;

	case 0x0500/4: case 0x0504/4: case 0x0508/4: case 0x050c/4:
	case 0x0510/4: case 0x0514/4: case 0x0518/4: case 0x051c/4:
	case 0x0520/4: case 0x0524/4: case 0x0528/4: case 0x052c/4:
	case 0x0530/4: case 0x0534/4: case 0x0538/4: case 0x053c/4:
		state->m_clio.sel = data & 0xff;
		/* Start WRSEL cycle */

		/* Detection of too many devices on the bus */
		switch ( data & 0xff )
		{
		case 0x8f:
			/* Everything is fine, there are not too many devices in the system */
			state->m_clio.poll = ( state->m_clio.poll & 0x0f );
			break;
		default:
			state->m_clio.poll = ( state->m_clio.poll & 0x0f ) | 0x90;
		}
		break;

	case 0x0540/4: case 0x0544/4: case 0x0548/4: case 0x054c/4:
	case 0x0550/4: case 0x0554/4: case 0x0558/4: case 0x055c/4:
	case 0x0560/4: case 0x0564/4: case 0x0568/4: case 0x056c/4:
	case 0x0570/4: case 0x0574/4: case 0x0578/4: case 0x057c/4:
		state->m_clio.poll = ( state->m_clio.poll & 0xf8 ) | ( data & 0x07 );
		break;

	case 0xc000/4:
	case 0xc004/4:
	case 0xc00c/4:
		break;
	case 0xc008/4:
		state->m_clio.uncle_addr = data;
		break;

	default:
		logerror( "%08X: unhandled CLIO write offset = %08X, data = %08X, mask = %08X\n", cpu_get_pc(space->machine().device("maincpu")), offset*4, data, mem_mask );
		break;
	}
}


void _3do_clio_init( running_machine &machine, screen_device *screen )
{
	_3do_state *state = machine.driver_data<_3do_state>();
	memset( &state->m_clio, 0, sizeof(CLIO) );
	state->m_clio.screen = screen;
	state->m_clio.revision = 0x02022000 /* 0x04000000 */;
	state->m_clio.cstatbits = 0x01;	/* bit 0 = reset of clio caused by power on */
	state->m_clio.unclerev = 0x03800000;
	state->m_clio.expctl = 0x80;	/* ARM has the expansion bus */
}


/* 9 -> 5 bits translation */

VIDEO_START( _3do )
{
	_3do_state *state = machine.driver_data<_3do_state>();
	/* We only keep the odd bits and get rid of the even bits */
	for ( int i = 0; i < 512; i++ )
	{
		state->m_video_bits[i] = ( i & 1 ) | ( ( i & 4 ) >> 1 ) | ( ( i & 0x10 ) >> 2 ) | ( ( i & 0x40 ) >> 3 ) | ( ( i & 0x100 ) >> 4 );
	}
}


/* This is incorrect! Just testing stuff */
SCREEN_UPDATE( _3do )
{
	_3do_state *state = screen->machine().driver_data<_3do_state>();
	UINT32 *source_p = state->m_vram + 0x1c0000 / 4;

	for ( int i = 0; i < 120; i++ )
	{
		UINT32	*dest_p0 = BITMAP_ADDR32( bitmap, 22 + i * 2, 254 );
		UINT32	*dest_p1 = BITMAP_ADDR32( bitmap, 22 + i * 2 + 1, 254 );

		for ( int j = 0; j < 320; j++ )
		{
			/* Odd numbered bits go to lower half, even numbered bits to upper half */
			UINT32 lower = *source_p & 0x55555555;
			UINT32 upper = ( *source_p >> 1 ) & 0x55555555;
			UINT32 rgb = 0;

			rgb = ( ( state->m_video_bits[upper & 0x1ff] << 3 ) << 8 );
			rgb |= ( ( state->m_video_bits[ ( upper >> 10 ) & 0x1ff ] << 3 ) << 0 );
			rgb |= ( ( state->m_video_bits[ ( upper >> 20 ) & 0x1ff ] << 3 ) << 16 );

			dest_p0[0] = rgb;
			dest_p0[1] = rgb;
			dest_p0[2] = rgb;
			dest_p0[3] = rgb;

			rgb = ( ( state->m_video_bits[lower & 0x1ff] << 3 ) << 8 );
			rgb |= ( ( state->m_video_bits[ ( lower >> 10 ) & 0x1ff ] << 3 ) << 0 );
			rgb |= ( ( state->m_video_bits[ ( lower >> 20 ) & 0x1ff ] << 3 ) << 16 );

			dest_p1[0] = rgb;
			dest_p1[1] = rgb;
			dest_p1[2] = rgb;
			dest_p1[3] = rgb;

			source_p++;
			dest_p0 += 4;
			dest_p1 += 4;
		}
	}

	return 0;
}

