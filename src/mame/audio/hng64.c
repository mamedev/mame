/* Hyper NeoGeo 64 Audio */

// uses a V53A ( == V33A with extra peripherals eg. DMA, Timers, MMU giving virtual 24-bit address space etc.)

/* The uploaded code shows that several different sound program revisions were used

sams64    (#)SNK R&D Center (R) NEO-GEO64 Sound Driver Ver 1.00a.     (#)Copyright (C) SNK Corp. 1996-1997 All rights reserved
roadedge  (#)SNK R&D Center (R) NEO-GEO64 Sound Driver Ver 1.10.      (#)Copyright (C) SNK Corp. 1996-1997 All rights reserved
xrally    (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.10. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved
bbust2    (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.11. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved
sams64_2  (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.14. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved
fatfurwa  (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.14. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved
buriki    (#)SNK R&D Center (R) HYPER NEOGEO64 Sound Driver Ver 1.15. (#)Copyright (C) SNK Corp. 1997,1998 All rights reserved

The earlier revisions appear to have 2 banks of code (there are vectors at the end of the 0x1e0000 block and the 0x1f0000 block)

Those first two revisions also spam the entire range of I/O ports with values several times on startup causing some unexpected
writes to the V53 internal registers.  The important ones are reinitialized after this however, I'm guessing this is harmless
on real hardware, as the code flow seems to be correct.

data structures look very similar between all of them

*/


#include "includes/hng64.h"

// save the sound program?
#define DUMP_SOUNDPRG  0

// ----------------------------------------------
// MIPS side
// ----------------------------------------------

// if you actually map RAM here on the MIPS side then xrally will upload the actual sound program here and blank out the area where
// the program would usually be uploaded (and where all other games upload it) this seems to suggest that the area is unmapped on
// real hardware.
WRITE32_MEMBER(hng64_state::hng64_soundram2_w)
{
}

READ32_MEMBER(hng64_state::hng64_soundram2_r)
{
	return 0x0000;
}


WRITE32_MEMBER(hng64_state::hng64_soundram_w)
{
	//logerror("hng64_soundram_w %08x: %08x %08x\n", offset, data, mem_mask);

	UINT32 mem_mask32 = mem_mask;
	UINT32 data32 = data;

	/* swap data around.. keep the v55 happy */
	data = data32 >> 16;
	data = FLIPENDIAN_INT16(data);
	mem_mask = mem_mask32 >> 16;
	mem_mask = FLIPENDIAN_INT16(mem_mask);
	COMBINE_DATA(&m_soundram[offset * 2 + 0]);

	data = data32 & 0xffff;
	data = FLIPENDIAN_INT16(data);
	mem_mask = mem_mask32 & 0xffff;
	mem_mask = FLIPENDIAN_INT16(mem_mask);
	COMBINE_DATA(&m_soundram[offset * 2 + 1]);

	if (DUMP_SOUNDPRG)
	{
		if (offset==0x7ffff)
		{
			logerror("dumping sound program in m_soundram\n");
			FILE *fp;
			char filename[256];
			sprintf(filename,"soundram_%s", space.machine().system().name);
			fp=fopen(filename, "w+b");
			if (fp)
			{
				fwrite((UINT8*)m_soundram, 0x80000*4, 1, fp);
				fclose(fp);
			}
		}
	}
}


READ32_MEMBER(hng64_state::hng64_soundram_r)
{
	UINT16 datalo = m_soundram[offset * 2 + 0];
	UINT16 datahi = m_soundram[offset * 2 + 1];

	return FLIPENDIAN_INT16(datahi) | (FLIPENDIAN_INT16(datalo) << 16);
}

WRITE32_MEMBER( hng64_state::hng64_soundcpu_enable_w )
{
	if (mem_mask&0xffff0000)
	{
		int cmd = data >> 16;
		// I guess it's only one of the bits, the commands are inverse of each other
		if (cmd==0x55AA)
		{
			printf("soundcpu ON\n");
			m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		}
		else if (cmd==0xAA55)
		{
			printf("soundcpu OFF\n");
			m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		}
		else
		{
			printf("unknown hng64_soundcpu_enable_w cmd %04x\n", cmd);
		}
	}

	if (mem_mask&0x0000ffff)
	{
			printf("unknown hng64_soundcpu_enable_w %08x %08x\n", data, mem_mask);
	}
}

// ----------------------------------------------
// General
// ----------------------------------------------


void hng64_state::reset_sound()
{
	UINT8 *RAM = (UINT8*)m_soundram;
	membank("bank1")->set_base(&RAM[0x1f0000]); // allows us to boot
	membank("bank2")->set_base(&RAM[0x1f0000]); // seems to be the right default for most games (initial area jumps to a DI here)
	m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

// ----------------------------------------------
// V53A side
// ----------------------------------------------


static ADDRESS_MAP_START( hng_sound_map, AS_PROGRAM, 16, hng64_state )
	AM_RANGE(0x00000, 0x0ffff) AM_RAMBANK("bank2")
	AM_RANGE(0x10000, 0x1ffff) AM_RAM // tmp, roadedge
	AM_RANGE(0xf0000, 0xfffff) AM_RAMBANK("bank1")
ADDRESS_MAP_END

WRITE16_MEMBER(hng64_state::hng64_sound_port_0008_w)
{
//	printf("hng64_sound_port_0008_w %04x %04x\n", data, mem_mask);
	// seems to one or more of the DMARQ on the V53, writes here when it expects DMA channel 3 to transfer ~0x20 bytes just after startup


	m_audiocpu->dreq3_trampoline_w(data&1);
//	m_audiocpu->hack_trampoline_w(1);

}

static ADDRESS_MAP_START( hng_sound_io, AS_IO, 16, hng64_state )
	AM_RANGE(0x0008, 0x0009) AM_WRITE( hng64_sound_port_0008_w )
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( hng64_audio )
	MCFG_CPU_ADD("audiocpu", V53A, 16000000)              // V53A, 16? mhz!
	MCFG_CPU_PROGRAM_MAP(hng_sound_map)
	MCFG_CPU_IO_MAP(hng_sound_io)
MACHINE_CONFIG_END

	
