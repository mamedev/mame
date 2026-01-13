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
#include "3do.h"

#include "cpu/arm7/arm7.h"

#include "debugger.h"
#include "screen.h"


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
void _3do_state::m_request_fiq(uint32_t irq_req, uint8_t type)
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
		m_maincpu->pulse_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, m_maincpu->minimum_quantum_time());
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
	uint8_t timer_flag;
	uint8_t carry_val;

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
					m_request_fiq(8 << (7-(i >> 1)),0);

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

uint8_t _3do_state::nvarea_r(offs_t offset) { return m_nvmem[offset]; }
void _3do_state::nvarea_w(offs_t offset, uint8_t data) { m_nvmem[offset] = data; }



// TODO: this connects to Z85C30, with Mac LF-to-CR newline conversion
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
    - check if data read equals 0x44696167 (=Diag) PC=3021948  If so, jump 302196c in default bios
    - dummy read from shift register
    - write 0x0300 to shift register bit #0
    - dummy write to shift register
    - read data from shift register bit #0.
    - xor that data with 0x07ff
    - write that data & 0x00ff | 0x4000 to the shift register
3022630
*/

uint32_t _3do_state::slow2_r(offs_t offset){
	uint32_t data = 0;

	logerror( "%08X: UNK_318 read offset = %08X\n", m_maincpu->pc(), offset );

	switch( offset ) {
	case 0:     /* Boot ROM checks here and expects to read 1, 0, 1, 0 in the lowest bit */
		data = m_slow2.cg_output & 0x00000001;
		m_slow2.cg_output = m_slow2.cg_output >> 1;
		m_slow2.cg_w_count = 0;
	}
	return data;
}


void _3do_state::slow2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
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



uint32_t _3do_state::svf_r(offs_t offset)
{
	uint32_t addr = ( offset & ( 0x07fc / 4 ) ) << 9;
	uint32_t *p = m_vram + addr;

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

void _3do_state::svf_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t addr = ( offset & ( 0x07fc / 4 ) ) << 9;
	uint32_t *p = m_vram + addr;

	logerror( "%08X: SVF write offset = %08X, data = %08X, mask = %08X\n", m_maincpu->pc(), offset*4, data, mem_mask );

	switch( offset & ( 0xe000 / 4 ) )
	{
	case 0x0000/4:      /* SPORT transfer */
		{
			uint32_t keep_bits = data ^ 0xffffffff;

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
			uint32_t keep_bits = data ^ 0xffffffff;
			uint32_t new_bits = m_svf.color & data;

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

// $0330'0000 base
void _3do_state::madam_map(address_map &map)
{
	map(0x0000, 0x0003).lrw32(
		NAME([this] () { return m_madam.revision; }),
		// echo for terminal?
		NAME([] (u32 data) {
			if(data == 0x0a)
				printf("\n");
			else
				printf("%c",data & 0xff);
		})
	);

	// 03300004 - Memory configuration 29 = 2MB DRAM, 1MB VRAM
	map(0x0004, 0x0007).lrw32(
		NAME([this] () { return m_madam.msysbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.msysbits); })
	);
	map(0x0008, 0x000b).lrw32(
		NAME([this] () { return m_madam.mctl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.mctl); })
	);
	map(0x000c, 0x000f).lrw32(
		NAME([this] () { return m_madam.sltime; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.sltime); })
	);
	map(0x0020, 0x0023).lrw32(
		NAME([this] () { return m_madam.abortbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.abortbits); })
	);
	map(0x0024, 0x0027).lrw32(
		NAME([this] () { return m_madam.privbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.privbits); })
	);
	map(0x0028, 0x002b).lrw32(
		NAME([this] () { return m_madam.statbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.statbits); })
	);
	// 0x0040, 0x0047 madam diag 0/1
	map(0x0040, 0x0043).lrw32(
		NAME([this] () { return m_madam.diag; }),
		// TODO: correct?
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_madam.diag = 1; })
	);
	// 0x0044: Anvil "feature"

	// CEL engine
//	map(0x0100, 0x0103)  SPRSTRT - Start the CEL engine (W)
//	map(0x0104, 0x0107)  SPRSTOP - Stop the CEL engine (W)
//	map(0x0108, 0x010b)  SPRCNTU - Continue the CEL engine (W)
//	map(0x010c, 0x010f)  SPRPAUS - Pause the CEL engine (W)
	map(0x0110, 0x0113).lrw32(
		NAME([this] () { return m_madam.ccobctl0; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.ccobctl0); })
	);
	// TODO: writes were 0x0129/4, typing mistake?
	map(0x0120, 0x0123).lrw32(
		NAME([this] () { return m_madam.ppmpc; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.ppmpc); })
	);

	// Regis
	// Regis control word
	map(0x0130, 0x0133).lrw32(
		NAME([this] () { return m_madam.regctl0; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.regctl0); })
	);
	// X and Y clip values
	map(0x0134, 0x0137).lrw32(
		NAME([this] () { return m_madam.regctl1; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.regctl1); })
	);
	// Read base address
	map(0x0138, 0x013b).lrw32(
		NAME([this] () { return m_madam.regctl2; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.regctl2); })
	);
	// Write base address
	map(0x013c, 0x013f).lrw32(
		NAME([this] () { return m_madam.regctl3; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.regctl3); })
	);
	map(0x0140, 0x0143).lrw32(
		NAME([this] () { return m_madam.xyposh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.xyposh); })
	);
	map(0x0144, 0x0147).lrw32(
		NAME([this] () { return m_madam.xyposl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.xyposl); })
	);
	map(0x0148, 0x014b).lrw32(
		NAME([this] () { return m_madam.linedxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.linedxyh); })
	);
	map(0x014c, 0x014f).lrw32(
		NAME([this] () { return m_madam.linedxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.linedxyl); })
	);
	map(0x0150, 0x0153).lrw32(
		NAME([this] () { return m_madam.dxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.dxyh); })
	);
	map(0x0154, 0x0157).lrw32(
		NAME([this] () { return m_madam.dxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.dxyl); })
	);
	map(0x0158, 0x015b).lrw32(
		NAME([this] () { return m_madam.ddxyh; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.ddxyh); })
	);
	map(0x015c, 0x015f).lrw32(
		NAME([this] () { return m_madam.ddxyl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.ddxyl); })
	);
	// PIP: Pen Index Palette (or PLT: Palette Look-up Table)
	// Reads are split in words, but writes are dword (cutting at 0x1bf)
	// TODO: does writing to 0x1c0-0x1ff go to mirror or they are just ignored
	map(0x0180, 0x01ff).lrw32(
		NAME([this] (offs_t offset) {
			const u16 reg = offset >> 1;
			if (offset & 1)
				return m_madam.pip[reg & 0x0f] >> 16;

			return m_madam.pip[reg & 0x0f] & 0xffff;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_madam.pip[offset & 0xf]);
		})
	);
	// part 1 of fence
	map(0x0218, 0x021f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.fence[offset]); })
	);
	map(0x0230, 0x023f).lr32(
		NAME([this] (offs_t offset) {
			const u16 reg = offset >> 1;
			if (offset & 1)
				return m_madam.fence[reg] >> 16;

			return m_madam.fence[reg] & 0xffff;
		})
	);
	// part 2 of fence
	map(0x0238, 0x023f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.fence[offset | 2]); })
	);
	map(0x0270, 0x027f).lr32(
		NAME([this] (offs_t offset) {
			const u16 reg = offset >> 1;
			if (offset & 1)
				return m_madam.fence[reg | 2] >> 16;

			return m_madam.fence[reg | 2] & 0xffff;
		})
	);
	map(0x0300, 0x03ff).lrw32(
		NAME([this] (offs_t offset) { return m_madam.mmu[offset & 0x3f]; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_madam.mmu[offset & 0x3f]); })
	);
	map(0x0400, 0x05ff).lrw32(
		NAME([this] (offs_t offset) {
			const u16 channel = offset >> 2;
			return m_madam.dma[channel & 0x1f][offset & 0x03];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			const u16 channel = offset >> 2;
			COMBINE_DATA(&m_madam.dma[channel & 0x1f][offset & 0x03]);
		})
	);
	/* Hardware multiplier */
	map(0x0600, 0x069f).lrw32(
		NAME([this] (offs_t offset) {
			return m_madam.mult[offset & 0x3f];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_madam.mult[offset & 0x3f]);
		})
	);
	map(0x07f0, 0x07f3).lrw32(
		NAME([this] () { return m_madam.mult_control; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_madam.mult_control |= data; })
	);
	map(0x07f4, 0x07f7).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { m_madam.mult_control &= ~data; })
	);
	map(0x07f8, 0x07fb).lr32(NAME([this] () { return m_madam.mult_status; }));
//	map(0x07fc, 0x07ff) start process
}

// $0340'0000 base
void _3do_state::clio_map(address_map &map)
{
	map(0x0000, 0x0003).lr32(NAME([this] () { return m_clio.revision; }));
	map(0x0004, 0x0007).lrw32(
		NAME([this] () { return m_clio.csysbits; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.csysbits); })
	);
	map(0x0008, 0x000b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.vint0); })
	);
	map(0x000c, 0x000f).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.vint1); })
	);
	map(0x0020, 0x0023).lrw32(
		NAME([this] () { return m_clio.audin; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.audin); })
	);
	map(0x0024, 0x0027).lrw32(
		NAME([this] () { return m_clio.audout; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// c0020f0f is written here during boot
			COMBINE_DATA(&m_clio.audout);
		})
	);
	map(0x0028, 0x002b).lrw32(
		NAME([this] () { return m_clio.cstatbits; }),
		// bits 0,1, and 6 are tested (reset source)
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.cstatbits); })
	);
	map(0x002c, 0x002f).lw32(
		// during boot 0000000B is written here, counter reload related?
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.wdog); })
	);
	// writes probably for test purposes only
	map(0x0030, 0x0033).lrw32(
		NAME([this] () { return m_clio.screen->hpos(); }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.hcnt); })
	);
	// TODO: needs to moved to a proper timer callback function
	// (or use frame_number for fake interlace readback)
	map(0x0034, 0x0037).lrw32(
		NAME([this] () {
			if ( m_clio.screen->vpos() == 0 && !machine().side_effects_disabled() )
			{
				m_clio.vcnt ^= 0x800;
			}
			return ( m_clio.vcnt & 0x800 ) | m_clio.screen->vpos();
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.vcnt); })
	);
	map(0x0038, 0x003b).lrw32(
		NAME([this] () { return m_clio.seed; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_clio.seed);
			m_clio.seed &= 0x0fff0fff;
		})
	);
	// TODO: should likely follow seed number, and be truly RNG
	map(0x003c, 0x003f).lr32(NAME([this] () { return m_clio.random; }));

	// interrupt control
	map(0x0040, 0x0047).lrw32(
		NAME([this] () { return m_clio.irq0; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_clio.irq0 &= ~data;
			else
				m_clio.irq0 |= data;

			m_request_fiq(0, 0);
		})
	);
	map(0x0048, 0x004f).lrw32(
		NAME([this] () { return m_clio.irq0_enable; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_clio.irq0_enable &= ~data;
			else
				m_clio.irq0_enable |= data;

			m_request_fiq(0, 0);
		})
	);
	map(0x0050, 0x0057).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_clio.mode &= ~data;
			else
				m_clio.mode |= data;
		})
	);
	map(0x0058, 0x005b).lw32(NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.badbits); }));
//	map(0x005c, 0x005f) unknown if used at all
	map(0x0060, 0x0067).lrw32(
		NAME([this] () { return m_clio.irq1; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_clio.irq1 &= ~data;
			else
				m_clio.irq1 |= data;

			m_request_fiq(0, 1);
		})
	);
	map(0x0068, 0x006f).lrw32(
		NAME([this] () { return m_clio.irq1_enable; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (offset)
				m_clio.irq1_enable &= ~data;
			else
				m_clio.irq1_enable |= data;

			m_request_fiq(0, 1);
		})
	);

	// expansion control
	map(0x0080, 0x0083).lrw32(
		NAME([this] () { return m_clio.hdelay; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.hdelay); })
	);
	// xxxx ---- DDR for below
	// ---- x--- Watchdog reset output
	// ---- -x-- Alternate ROM bank select (kanji ROM at $300'0000)
	// ---- --x- Audio mute output
	// ---- ---x <unused>
	map(0x0084, 0x0087).lrw32(
		NAME([this] () { return m_clio.adbio; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_clio.adbio);
			m_clio.adbio &= 0xff;
		})
	);
	map(0x0088, 0x008b).lrw32(
		NAME([this] () { return m_clio.adbctl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.adbctl); })
	);

	// Timer section
	map(0x0100, 0x017f).lrw32(
		NAME([this] (offs_t offset) {
			if (offset & 1)
				return m_clio.timer_backup[(offset & 0x3f) >> 1];
			return m_clio.timer_count[(offset & 0x3f) >> 1];
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			// only lower 16 bits can be written to
			const u8 which = (offset & 0x3f) >> 1;
			if (offset & 1)
			{
				COMBINE_DATA(&m_clio.timer_backup[which]);
				m_clio.timer_backup[which] &= 0xffff;
			}
			else
			{
				COMBINE_DATA(&m_clio.timer_count[which]);
				m_clio.timer_count[which] &= 0xffff;
			}
		})
	);
	map(0x0200, 0x020f).lrw32(
		NAME([this] (offs_t offset) {
			const u8 shift = (offset & 2) * 32;
			return m_clio.timer_ctrl >> shift;
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			const u8 shift = (offset & 2) * 32;
			const u64 mask = ((u64)data << shift);
			if (offset & 1)
				m_clio.timer_ctrl &= ~mask;
			else
				m_clio.timer_ctrl |= mask;
		})
	);
	map(0x0220, 0x0223).lrw32(
		NAME([this] () { return m_clio.slack; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.slack); })
	);

//	map(0x0300, 0x0303) FIFO init
//	map(0x0304, 0x0307) DMA request enable
	// TODO: likely set/clear like above
	map(0x0308, 0x030b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.dmareqdis); })
	);
//	map(0x0380, 0x0383) FIFO status

	// XBus
	map(0x0400, 0x0407).lrw32(
		NAME([this] () { return m_clio.expctl; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_8_15)
			{
				if (offset)
					m_clio.expctl |= data & 0xca00;
				else
					m_clio.expctl &= ~(data & 0xca00);
			}
		})
	);
	map(0x0408, 0x040b).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_clio.type0_4); })
	);
	// TODO: these are identification bits
	// x--- ---- ---- ---- active
	// -x-- ---- ---- ---- "happened before reset"
	// ---- ---- xxxx xxxx device number
	map(0x0410, 0x0413).lr32(NAME([this] () { return m_clio.dipir1; }));
	map(0x0414, 0x0417).lr32(NAME([this] () { return m_clio.dipir2; }));

	map(0x0500, 0x053f).lrw32(
		NAME([this] () { return m_clio.sel; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_clio.sel);
			m_clio.sel &= 0xff;
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
		})
	);
	map(0x0540, 0x057f).lrw32(
		NAME([this] () { return m_clio.poll; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			COMBINE_DATA(&m_clio.poll);
			m_clio.poll &= 0xf8;
			m_clio.poll |= data & 7;
		})
	);
//	map(0x0580, 0x05bf) Command Stat
//	map(0x05c0, 0x05ff) Data

	// TODO: for debug, to be removed once that we hookup the CPU core
//	map(0x17d0, 0x17d3) Semaphore
//	map(0x17d4, 0x17d7) Semaphore ACK
//	map(0x17e0, 0x17ff) DSPP DMA and state
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
	map(0x17fc, 0x17ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (data & 1)
				machine().debug_break();
		})
	);
	map(0x1800, 0x1fff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_16_31)
				m_dspp.N[(offset<<1)+0] = data >> 16;
			if (ACCESSING_BITS_0_15)
				m_dspp.N[(offset<<1)+1] = data & 0xffff;
		})
	);
	map(0x2000, 0x2fff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_15)
				m_dspp.N[offset] = data & 0xffff;
		})
	);
	map(0x3000, 0x31ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_16_31)
				m_dspp.EI[(offset<<1)+0] = data >> 16;
			if (ACCESSING_BITS_0_15)
				m_dspp.EI[(offset<<1)+1] = data & 0xffff;
		})
	);
	map(0x3400, 0x37ff).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			if (ACCESSING_BITS_0_15)
				m_dspp.EI[offset] = data & 0xffff;
		})
	);
	map(0x3800, 0x39ff).lr32(
		NAME([this] (offs_t offset) {
			uint32_t res = 0;
			res = (m_dspp.EO[(offset << 1) + 0] << 16);
			res |= (m_dspp.EO[(offset << 1) + 1] & 0xffff);
			return res;
		})
	);
	map(0x3c00, 0x3fff).lr32(
		NAME([this] (offs_t offset) {
			return m_dspp.EO[offset] & 0xffff;
		})
	);
}



// $0340'c000 base
// Uncle/Woody is the FMV expansion bus (with it's own ROM)
void _3do_state::uncle_map(address_map &map)
{
	map(0x0000, 0x0003).lr32(NAME([this] () { return m_uncle.rev; }));
	map(0x0004, 0x0007).lr32(NAME([this] () { return m_uncle.soft_rev; }));
	map(0x0008, 0x000b).lrw32(
		NAME([this] () { return m_uncle.addr; }),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) { COMBINE_DATA(&m_uncle.addr); })
	);
	// ROM readback
	map(0x000c, 0x000f).lr32(NAME([] () { return 0; }));
}

/* 9 -> 5 bits translation */

void _3do_state::video_start()
{
	/* We only keep the odd bits and get rid of the even bits */
//  for ( int i = 0; i < 512; i++ )
//  {
//      m_video_bits[i] = ( i & 1 ) | ( ( i & 4 ) >> 1 ) | ( ( i & 0x10 ) >> 2 ) | ( ( i & 0x40 ) >> 3 ) | ( ( i & 0x100 ) >> 4 );
//  }
}


uint32_t _3do_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *source_p = m_vram + 0x1c0000 / 4;

	for ( int y = 0; y < 120; y++ )
	{
		uint32_t  *dest_p0 = &bitmap.pix(22 + y * 2, 254 );
		uint32_t  *dest_p1 = &bitmap.pix(22 + y * 2 + 1, 254 );

		for ( int x = 0; x < 320; x++ )
		{
			/* Every dword contains two pixels, upper word is top pixel, lower is bottom. */
			uint32_t lower = *source_p & 0xffff;
			uint32_t upper = ( *source_p >> 16 ) & 0xffff;
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

void _3do_state::m_madam_init( void )
{
	m_madam = MADAM();
	// bit 31-16 Green Madam (0x0102)
	// bit 23 Internal ARM
	// bit 15-8 manufacturer ID (0x02: MEC -> Panasonic, 0x09: Sanyo, 0x0b: Goldstar)
	// bit 0 wirewrap system
	m_madam.revision = 0x01020200;
	m_madam.msysbits = 0x51;
}

void _3do_state::m_slow2_init( void )
{
	m_slow2.cg_input = 0;
	m_slow2.cg_output = 0x00000005 - 1;
}

void _3do_state::m_clio_init()
{
	m_clio = CLIO();
	m_clio.screen = m_screen;
	// Clio Preen (Toshiba/MEC)
	// 0x04000000 for Anvil
	m_clio.revision = 0x02022000 /* 0x04000000 */;
	m_uncle.rev = 0x03800000;
	m_clio.expctl = 0x80;    /* ARM has the expansion bus */
	m_dspp.N = make_unique_clear<uint16_t[]>(0x800);
	m_dspp.EI = make_unique_clear<uint16_t[]>(0x400);
	m_dspp.EO = make_unique_clear<uint16_t[]>(0x400);

	save_pointer(NAME(m_dspp.N), 0x800);
	save_pointer(NAME(m_dspp.EI), 0x400);
	save_pointer(NAME(m_dspp.EO), 0x400);
}
