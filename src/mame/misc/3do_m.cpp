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

