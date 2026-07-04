// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Wilbert Pol
/*

Miscellaneous not yet device-ified stuff

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
ETA: it already stuck earlier, keeps trying a 0x83 "Read identification" with the CR-560-B on XBus.
The CD-Rom is MKE-style, akin to CDTV CR-511B and Creative Soundblaster a.k.a. SBCD.SYS/sbpcd

*/

#include "emu.h"
#include "3do.h"

#include "cpu/arm7/arm7.h"

#include "debugger.h"
#include "screen.h"




uint8_t _3do_state::nvarea_r(offs_t offset) { return m_nvmem[offset]; }
void _3do_state::nvarea_w(offs_t offset, uint8_t data) { m_nvmem[offset] = data; }


void _3do_state::m_slow2_init( void )
{
	m_slow2.cg_input = 0;
	// NOTE: remove the -1 to enter into diag mode
	m_slow2.cg_output = 0x00000005 - 1;
}


// TODO: (update for below) this connects to Z85C30, with Mac LF-to-CR newline conversion
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
		m_slow2.cg_input = m_slow2.cg_input << 1 | ( data & 0x00000001 );
		m_slow2.cg_w_count ++;
		if ( m_slow2.cg_w_count == 16 )
		{
		}
		break;
	}
}



// NOTE: TC528267 emulation
uint32_t _3do_state::svf_r(offs_t offset)
{
	uint32_t addr = ( offset & ( 0x07fc / 4 ) ) << 9;
	uint32_t *p = m_vram + addr;

	if (!machine().side_effects_disabled())
		logerror( "%08X: SVF read offset = %08X\n", m_maincpu->pc(), offset * 4 );

	switch( offset & ( 0xE000 / 4 ) )
	{
	case 0x0000/4:      /* SPORT transfer */
		if (!machine().side_effects_disabled())
		{
			for ( int i = 0; i < 512; i++ )
			{
				m_svf.sport[i] = p[i];
			}
		}
		break;
	case 0x2000/4:      /* Write to color register */
		return m_svf.color;
	case 0x4000/4:      /* Flash write */
		break;
	case 0x6000/4:      /* CAS before RAS refresh/reset (CBR). Used to initialize VRAM mode during boot. */
		// TODO: reads here at boot
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

			logerror("VRAM flash write %08x color %08x\n", addr, new_bits);

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

// $0340'c000 base
// Uncle handling, Woody is the FMV expansion bus (with it's own ROM)
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

