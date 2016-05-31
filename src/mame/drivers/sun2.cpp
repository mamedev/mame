// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R. Belmont
/***************************************************************************

        Sun-2 Models
        ------------

    2/120
        Processor(s):   68010 @ 10MHz
        CPU:            501-1007/1051
        Chassis type:   deskside
        Bus:            Multibus (9 slots)
        Memory:         7M physical
        Notes:          First machines in deskside chassis. Serial
                        microswitch keyboard (type 2), Mouse Systems
                        optical mouse (Sun-2).

    2/100U
        Processor(s):   68010 @ 10MHz
        CPU:            501-1007
        Bus:            Multibus
        Notes:          Upgraded Sun 100. Replaced CPU and memory boards
                        with first-generation Sun-2 CPU and memory
                        boards so original customers could run SunOS
                        1.x. Still has parallel kb/mouse interface so
                        type 1 keyboards and Sun-1 mice could be
                        connected.

    2/150U
        Notes:          Apparently also an upgraded Sun-1.

    2/170
        Chassis type:   rackmount
        Bus:            Multibus (15 slots)
        Notes:          Rackmount version of 2/120, with more slots.

    2/50
        Processor(s):   68010 @ 10MHz
        CPU:            501-1141/1142/1143/1426/1427/1428
        Chassis type:   wide pizza box
        Bus:            VME (2 slots)
        Memory:         7M physical
        Notes:          The (type 2) keyboard and mouse attach via an
                        adapter that accepts two modular plugs and
                        attaches to a DB15 port; later on, units were
                        apparently shipped with type 3 keyboards. The
                        CPU boards have a double-width back panel but
                        are otherwise identical to those in the 2/130
                        and 2/160.

    2/130
    2/160
        Processor(s):   68010 @ 10MHz
        CPU:            501-1144/1145/1146/1429/1430/1431
        Chassis type:   deskside
        Bus:            VME (12 slots)
        Memory:         7M physical
        Notes:          First machine in 12-slot deskside VME chassis.
                        Has four-fan cooling tray instead of six as in
                        later machines, which led to cooling problems
                        with lots of cards. Backplane has only four P2
                        memory connectors bussed instead of six as in
                        later 12-slot backplanes; SCSI passthrough is in
                        slot 6 instead of 7 as in later 12-slot
                        backplanes. Upgradeable to a 3/160 by replacing
                        the CPU board. No information on the differences
                        between the 2/130 and the 2/160.


        25/08/2009 Skeleton driver.
           
How the architecture works:
	- There are 3 address sub-spaces: CPU layer, MMU layer, and device layer
	- CPU layer uses MOVS instructions to output FC 3.
	- CPU layer: the low-order address bits A4-A1 specify the device
		0100x = ID Prom
		0101x = Diagnostic register (8 bits, 8 LEDs, bit = 0 for ON, 1 for OFF)
		0110x = Bus error register
		0111x = System enable register
		
		Bits A5+ address the actual individual parts of these things.  ID Prom bytes
		are at 0x0008, 0x0808, 0x1008, 0x1808, 0x2008, 0x2808, 0x3008, etc.
		
		System enable bits:
			b0 = enable parity generation
			b1 = cause level 1 IRQ
			b2 = cause level 2 IRQ
			b3 = cause level 3 IRQ
			b4 = enable parity error checking
			b5 = enable DVMA
			b6 = enable all interrupts
			b7 = boot state (0 = boot, 1 = normal) 
				In boot state, all supervisor program reads go to the EPROM.

	- MMU layer: also accessed via FC 3
		PAGE MAP at 0 + V
		SEGMENT MAP at 4 + V
		CONTEXT REG at 6 + V
		
	There are 8 hardware contexts.  Supervisor and User FCs can have different contexts.
	
	Segment map is 4096 entries, from bits 23-15 of the virtual address + 3 context bits.
	Entries are 8 bits, which point to a page map entry group (PMEG).
	
	Page map is 4096 entries each mapping a 2K page.  There are 256 groups of 16 entries;
	the PMEG points to these 256 groups.  The page map contains a 20-bit page number, 
	which combines with the 11 low bits of the original address to get a 31-bit physical address.
	The entry from 0-15 is picked with bits 15-11 of the original address.	
	
	There is an "address hole" between virtual addresses 0x600000 and 0xDFFFFF.  Page table
	number generation skips from 0x5FFFFF to 0xE00000.  Thus the last two megs of the lower 8 MB
	are replaced by the last two megs of the upper 8 MB (ROM and I/O).
	
	ef0942 = time to set up the maps for the data (middle of function mapmem() in sunmon.c)
	
****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/ram.h"
#include "machine/z80scc.h"
#include "machine/bankdev.h"
#include "bus/rs232/rs232.h"

// page table entry constants
#define PM_VALID	(0x80000000)	// page is valid
#define PM_PROTMASK (0x7e000000)	// protection mask
#define PM_TYPEMASK (0x01c00000)	// type mask
#define PM_ACCESSED (0x00200000)	// accessed flag
#define PM_MODIFIED (0x00100000)	// modified flag

#define HOLE_PAGE	(4096)			// fake page to redirect the address hole to, as TME does
#define HOLE_START	(0x800000>>1)	// if set to real value of 600000, the page map address line test fails
#define HOLE_END	(0xDFFFFF>>1)

class sun2_state : public driver_device
{
public:
	sun2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_rom(*this, "bootprom"),
	m_ram(*this, RAM_TAG),
	m_type1space(*this, "type1"),
	m_bw2_vram(*this, "bw2_vram")
	{ }

	required_device<m68010_device> m_maincpu;
	required_memory_region m_rom;
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_type1space;
	required_shared_ptr<UINT16> m_bw2_vram;
	
	virtual void machine_start() override;
	virtual void machine_reset() override;
	
	DECLARE_READ16_MEMBER( tl_mmu_r );
	DECLARE_WRITE16_MEMBER( tl_mmu_w );
	DECLARE_WRITE16_MEMBER( video_ctrl_w );
	DECLARE_READ16_MEMBER( test_r );
	DECLARE_WRITE16_MEMBER( test_w );
		
	UINT32 bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:	
	UINT16 *m_rom_ptr, *m_ram_ptr;
	UINT16 m_diagreg, m_sysenable, m_buserror;
	UINT16 m_context;
	UINT8 m_segmap[8][512];
	UINT32 m_pagemap[4097];
	UINT32 m_ram_size, m_ram_size_words;
};

READ16_MEMBER( sun2_state::test_r )
{
	printf("test_r @ %x\n", offset << 1);

	return 0xffff;
}

WRITE16_MEMBER( sun2_state::test_w )
{
	printf("test_w %x @ %x\n", data, offset << 1);
}

READ16_MEMBER( sun2_state::tl_mmu_r )
{
	UINT8 fc = m_maincpu->get_fc();

	if ((fc == 3) && !space.debugger_access())
	{
		if (offset & 0x4)	// set for CPU space
		{
			switch (offset)
			{
				case 4:
					printf("sun2: Read IDPROM @ %x\n", offset<<1);
					return 0xffff;
					
				case 5:
					printf("sun2: Read diag reg\n");
					return m_diagreg;
					
				case 6:
					printf("sun2: Read bus error\n");
					return m_buserror;
					
				case 7:
					printf("sun2: Read sysenable\n");
					return m_sysenable;
			}
		}
		else				// clear for MMU space
		{
			int page;
			
			switch (offset & 3)
			{
				case 0:	// page map
				case 1:
					page = (offset >> 10) & 0x1fff;
					
					if (offset >= HOLE_START)
					{
						if (offset <= HOLE_END)
						{
							page = HOLE_PAGE;
						}
						else
						{
							page &= 0xfff;
						}
					}
					
					//printf("sun2: Read page map at %x (entry %d)\n", offset<<1, page);
					if (offset & 1)	// low-order 16 bits
					{
						return m_pagemap[page] & 0xffff;
					}
					return m_pagemap[page] >> 16;
					
				case 2: // segment map
					//printf("sun2: Read segment map at %x (entry %d, user ctx %d)\n", offset<<1, offset>>14, m_context & 7);
					return m_segmap[m_context & 7][offset >> 14];
					
				case 3: // context reg
					//printf("sun2: Read context reg\n");
					return m_context;
			}
		}
	}

	// boot mode?
	if ((fc == M68K_FC_SUPERVISOR_PROGRAM) && !(m_sysenable & 0x80))
	{
		return m_rom_ptr[offset & 0x3fff];
	}
	
	// debugger hack
	if ((space.debugger_access()) && (offset >= (0xef0000>>1)) && (offset <= (0xef8000>>1)))
	{
		return m_rom_ptr[offset & 0x3fff];
	}
	
	int super_verbose = 0;
	if ((offset >= (0xef6600>>1)) && (offset <= (0xef6900>>1)))
	{
		super_verbose = 1;
	}
	
	// it's translation time
	UINT8 context = (fc & 4) ? ((m_context >> 8) & 7) : (m_context & 7);
	UINT8 pmeg = m_segmap[context][offset >> 14];
	UINT32 entry = (pmeg << 4) + ((offset >> 10) & 0xf);

	if (super_verbose)
	{
		//printf("sun2: Context = %d, pmeg = %d, offset >> 14 = %x, entry = %d, page = %d\n", context, pmeg, offset >> 14, entry, (offset >> 10) & 0xf);
	}

	m_pagemap[entry] |= PM_ACCESSED;
	if (m_pagemap[entry] & PM_VALID)
	{
		UINT32 tmp = (m_pagemap[entry] & 0xfffff) << 10;
		tmp |= (offset & 0x3ff);
		
		//if ((!space.debugger_access()) && (super_verbose))
		//	printf("sun2: Translated addr: %08x, type %d (page %d page entry %08x, orig virt %08x, FC %d)\n", tmp << 1, (m_pagemap[entry] >> 22) & 7, entry, m_pagemap[entry], offset<<1, fc);

		switch ((m_pagemap[entry] >> 22) & 7)
		{
			case 0:	// main RAM space
				//printf("read main RAM @ %x\n", offset<<1);
				if (tmp < m_ram_size_words) return m_ram_ptr[tmp];
				return 0xffff;
				
			case 1: // device space
				// EPROM space is special: the MMU has a trap door
				// where the original bits of the virtual address are
				// restored so that the entire 32K EPROM can be 
				// accessed via a 2K single page view.  This isn't
				// obvious in the sun2 manual, but the sun3 manual 
				// (sun3 has the same mechanism) explains it well.
				// the 2/50 ROM tests this specifically at $EF0DF0.
				if ((tmp >= (0x7f0000>>1)) && (tmp <= (0x7f07ff>>1)))
				{
					if (super_verbose)
					{
						printf("sun2: extra-magic EPROM bypass @ %x\n", (offset & 0x3fff) << 1);
					}
				
					return m_rom_ptr[offset & 0x3fff];	
				}
				
				//printf("read device space @ %x\n", tmp<<1);
				return m_type1space->read16(space, tmp, mem_mask);
				
			case 2:	// VME space
				//printf("Read VME2 @ %x\n", tmp<<1);
				break;
				
			case 3:	// more VME
				//printf("Read VME3 @ %x\n", tmp<<1);
				break;
		}
	}
	else
	{
		if (!space.debugger_access()) printf("sun2: pagemap entry not valid!\n");
	}
	
	if (!space.debugger_access()) printf("sun2: Unmapped read @ %08x (FC %d, mask %04x, PC=%x, seg %x)\n", offset<<1, fc, mem_mask, m_maincpu->pc, offset>>15);
	
	return 0xffff;
}

WRITE16_MEMBER( sun2_state::tl_mmu_w )
{
	UINT8 fc = m_maincpu->get_fc();
	
	//printf("sun2: Write %04x (FC %d, mask %04x, PC=%x) to %08x\n", data, fc, mem_mask, m_maincpu->pc, offset<<1);
	
	if (fc == 3)
	{
		if (offset & 0x4)	// set for CPU space
		{
			switch (offset)
			{
				case 4:
					//printf("sun2: Write? IDPROM @ %x\n", offset<<1);
					return;
					
				case 5:
					// XOR to match Table 2-1 in the 2/50 Field Service Manual
					printf("sun2: CPU LEDs to %02x (PC=%x) => ", (data & 0xff) ^ 0xff, m_maincpu->pc);
					m_diagreg = data & 0xff;
					for (int i = 0; i < 8; i++)
					{
						if (m_diagreg & (1<<(7-i)))
						{
							printf("*");
						}
						else
						{
							printf("O");
						}
					}
					printf("\n");
					return;
					
				case 6:
					//printf("sun2: Write %04x to bus error not allowed\n", data);
					return;
					
				case 7:
					printf("sun2: Write %04x to system enable\n", data);
					COMBINE_DATA(&m_sysenable);
					return;
			}
		}
		else				// clear for MMU space
		{
			int page;
			
			switch (offset & 3)
			{
				case 0:	// page map
				case 1:
					page = (offset >> 10) & 0xfff;
					if (offset >= HOLE_START)
					{
						if (offset <= HOLE_END)
						{
							page = HOLE_PAGE;
						}
						else
						{
							page &= 0xfff;
						}
					}
					
					printf("sun2: Write %04x to page map at %x (entry %d), ", data, offset<<1, page);
					if (offset & 1)	// low-order 16 bits
					{
						m_pagemap[page] &= 0xffff0000;
						m_pagemap[page] |= data;
					}
					else
					{
						m_pagemap[page] &= 0x0000ffff;
						m_pagemap[page] |= (data<<16);
					}
					printf("entry now %08x (adr %08x  PC=%x)\n", m_pagemap[page], (m_pagemap[page] & 0xfffff) << 11, m_maincpu->pc);
					return;
					
				case 2: // segment map
					printf("sun2: Write %02x to segment map at %x (entry %d, user ctx %d PC=%x)\n", data & 0xff, offset<<1, offset>>14, m_context & 7, m_maincpu->pc);
					m_segmap[m_context & 7][offset >> 14] = data & 0xff;
					return;
					
				case 3: // context reg
					//printf("sun2: Write %04x to context\n", data);
					COMBINE_DATA(&m_context);
					return;
			}
		}
	}
	
	// it's translation time
	UINT8 context = (fc & 4) ? ((m_context >> 8) & 7) : (m_context & 7);
	UINT8 pmeg = m_segmap[context][offset >> 14];
	UINT32 entry = (pmeg << 4) + ((offset >> 10) & 0xf);
	
	m_pagemap[entry] |= PM_ACCESSED;
	if (m_pagemap[entry] & PM_VALID)
	{
		UINT32 tmp = (m_pagemap[entry] & 0xfffff) << 10;
		tmp |= (offset & 0x3ff);
		
		//if (!space.debugger_access()) printf("sun2: Translated addr: %08x, type %d (page entry %08x, orig virt %08x)\n", tmp << 1, (m_pagemap[entry] >> 22) & 7, m_pagemap[entry], offset<<1);

		switch ((m_pagemap[entry] >> 22) & 7)
		{
			case 0:	// main RAM space
				if (tmp < m_ram_size_words) COMBINE_DATA(&m_ram_ptr[tmp]);
				return;
				
			case 1: // device space
				//printf("write device space @ %x\n", tmp<<1);
				m_type1space->write16(space, tmp, data, mem_mask);
				return;
				
			case 2:	// VME space
				printf("Write VME space\n");
				break;
				
			case 3:	// more VME
				printf("Write 2nd VME space\n");
				break;
		}
	}
	else
	{
		if (!space.debugger_access()) printf("sun2: pagemap entry not valid!\n");
	}
	
	printf("sun2: Unmapped write %04x (FC %d, mask %04x, PC=%x) to %08x\n", data, fc, mem_mask, m_maincpu->pc, offset<<1);
}

// BW2 video control
WRITE16_MEMBER( sun2_state::video_ctrl_w )
{
	printf("sun2: BW2: %x to video_ctrl\n", data);
}

static ADDRESS_MAP_START(sun2_mem, AS_PROGRAM, 16, sun2_state)
	AM_RANGE(0x000000, 0xffffff) AM_READWRITE( tl_mmu_r, tl_mmu_w )
ADDRESS_MAP_END

// type 1 device space
static ADDRESS_MAP_START(type1space_map, AS_PROGRAM, 16, sun2_state)
	AM_RANGE(0x000000, 0x01ffff) AM_RAM AM_SHARE("bw2_vram")
	AM_RANGE(0x7f0000, 0x7f07ff) AM_ROM AM_REGION("bootprom", 0)	// uses MMU loophole to read 32k from a 2k window
	// 7f0800-7f0fff: Ethernet interface
	// 7f1000-7f17ff: AM9518 encryption processor
	// 7f1800-7f1fff: Keyboard/mouse SCC8530
	// 7f2000-7f27ff: RS232 ports SCC8530
	// 7f2800-7f2fff: AM9513 timer
	AM_RANGE(0xc20000, 0xc20001) AM_WRITE( video_ctrl_w )
ADDRESS_MAP_END

UINT32 sun2_state::bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels;
	static const UINT32 palette[2] = { 0, 0xffffff };
	UINT8 *m_vram = (UINT8 *)m_bw2_vram.target();

	for (y = 0; y < 900; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1152/8; x++)
		{
			pixels = m_vram[(y * (1152/8)) + (BYTE4_XOR_BE(x))];

			*scanline++ = palette[(pixels>>7)&1];
			*scanline++ = palette[(pixels>>6)&1];
			*scanline++ = palette[(pixels>>5)&1];
			*scanline++ = palette[(pixels>>4)&1];
			*scanline++ = palette[(pixels>>3)&1];
			*scanline++ = palette[(pixels>>2)&1];
			*scanline++ = palette[(pixels>>1)&1];
			*scanline++ = palette[(pixels&1)];
		}
	}

	return 0;
}

/* Input ports */
static INPUT_PORTS_START( sun2 )
INPUT_PORTS_END

void sun2_state::machine_start()
{
	m_rom_ptr = (UINT16 *)m_rom->base();
	m_ram_ptr = (UINT16 *)m_ram->pointer();
	m_ram_size = m_ram->size();
	m_ram_size_words = m_ram_size >> 1;
}

void sun2_state::machine_reset()
{
	m_diagreg = 0;
	m_sysenable = 0;
	m_context = 0;
	m_buserror = 0;
	memset(m_segmap, 0, sizeof(m_segmap));
	memset(m_pagemap, 0, sizeof(m_pagemap));
	
	m_maincpu->reset();
}

static MACHINE_CONFIG_START( sun2, sun2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68010, 16670000)
	MCFG_CPU_PROGRAM_MAP(sun2_mem)
	
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_DEFAULT_VALUE(0x00)
	
	// MMU Type 1 device space
	MCFG_DEVICE_ADD("type1", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(type1space_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(16)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x1000000)
	
	MCFG_SCREEN_ADD("bwtwo", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(sun2_state, bw2_update)
	MCFG_SCREEN_SIZE(1152,900)
	MCFG_SCREEN_VISIBLE_AREA(0, 1152-1, 0, 900-1)
	MCFG_SCREEN_REFRESH_RATE(72)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sun2_120 )
	ROM_REGION( 0x8000, "bootprom", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "sun2-multi-rev-r.bin", 0x0000, 0x8000, CRC(4df0df77) SHA1(4d6bcf09ddc9cc8f5823847b8ea88f98fe4a642e))
ROM_END

ROM_START( sun2_50)
	ROM_REGION( 0x8000, "bootprom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "250_q_8.rom", 0x0001, 0x4000, CRC(5bfacb5c) SHA1(ec7fb3fb0217b0138ba4748b7c79b8ff0cad896b))
	ROM_LOAD16_BYTE( "250_q_0.rom", 0x0000, 0x4000, CRC(2ee29abe) SHA1(82f52b9f25e92387329581f7c8ba50a171784968))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY         FULLNAME       FLAGS */
COMP( 1984, sun2_50,   0,       0,       sun2,      sun2, driver_device,     0,  "Sun Microsystems", "Sun 2/50", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1984, sun2_120,  0,       0,       sun2,      sun2, driver_device,     0,  "Sun Microsystems", "Sun 2/120", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
