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
        Oscillator(s):  39.3216MHz
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
        Oscillator(s):  19.6608MHz, 16MHz (Ethernet/VMEbus), 100MHz
                        (video), 24MHz ("for special applications")
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
        31/05/2016 Main screen turn on.

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
    Entries are 8 bits, which point to a page map entry group (PMEG), which is 16 consecutive
    page table entries (32 KB of space).

    Page map is 4096 entries each mapping a 2K page.  There are 256 groups of 16 entries;
    the PMEG points to these 256 groups.  The page map contains a 20-bit page number,
    which combines with the 11 low bits of the original address to get a 31-bit physical address.
    The entry from 0-15 is picked with bits 15-11 of the original address.

    Page map entries are written to the PMEG determined by their segment map entry; you must
    set the segment map validly in order to write to the page map.  This is how they get away
    with having 16 MB of segment entries and only 8 MB of PMEGs.

    See http://sunstuff.org/Sun-Hardware-Ref/s2hr/part2
****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68010.h"
#include "machine/ram.h"
#include "machine/am9513.h"
#include "machine/i82586.h"
#include "machine/mm58167.h"
#include "machine/z80scc.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"
#include "bus/rs232/rs232.h"
#include "screen.h"


namespace {

#define SCC1_TAG        "scc1"
#define SCC2_TAG        "scc2"
#define RS232A_TAG      "rs232a"
#define RS232B_TAG      "rs232b"

// page table entry constants
#define PM_VALID    (0x80000000)    // page is valid
#define PM_PROTMASK (0x7e000000)    // protection mask
#define PM_TYPEMASK (0x01c00000)    // type mask
#define PM_ACCESSED (0x00200000)    // accessed flag
#define PM_MODIFIED (0x00100000)    // modified flag

class sun2_state : public driver_device
{
public:
	sun2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "bootprom")
		, m_idprom(*this, "idprom")
		, m_ram(*this, RAM_TAG)
		, m_type0space(*this, "type0")
		, m_type1space(*this, "type1")
		, m_type2space(*this, "type2")
		, m_type3space(*this, "type3")
		, m_edlc(*this, "edlc")
		, m_bw2_vram(*this, "bw2_vram")
	{ }

	void sun2mbus(machine_config &config);
	void sun2vme(machine_config &config);

private:
	required_device<m68010_device> m_maincpu;
	required_memory_region m_rom, m_idprom;
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_type0space, m_type1space, m_type2space, m_type3space;
	optional_device<i82586_device> m_edlc;
	required_shared_ptr<uint16_t> m_bw2_vram;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint16_t mmu_r(offs_t offset, uint16_t mem_mask = ~0);
	void mmu_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tl_mmu_r(uint8_t fc, offs_t offset, uint16_t mem_mask);
	void tl_mmu_w(uint8_t fc, offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t video_ctrl_r();
	void video_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t ethernet_r();
	void ethernet_w(uint8_t data);
	void ethernet_int_w(int state);
	uint16_t edlc_mmu_r(offs_t offset, uint16_t mem_mask);
	void edlc_mmu_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint32_t bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mbustype0space_map(address_map &map) ATTR_COLD;
	void mbustype1space_map(address_map &map) ATTR_COLD;
	void mbustype2space_map(address_map &map) ATTR_COLD;
	void mbustype3space_map(address_map &map) ATTR_COLD;
	void sun2_mem(address_map &map) ATTR_COLD;
	void edlc_mem(address_map &map) ATTR_COLD;
	void vmetype0space_map(address_map &map) ATTR_COLD;
	void vmetype1space_map(address_map &map) ATTR_COLD;
	void vmetype2space_map(address_map &map) ATTR_COLD;
	void vmetype3space_map(address_map &map) ATTR_COLD;

	uint16_t *m_rom_ptr, *m_ram_ptr;
	uint8_t *m_idprom_ptr;
	uint16_t m_diagreg, m_sysenable, m_buserror;
	uint16_t m_context;
	uint8_t m_segmap[8][512];
	uint32_t m_pagemap[4097];
	uint32_t m_ram_size, m_ram_size_words;
	uint16_t m_bw2_ctrl;
	uint8_t m_ethernet_status;
};

uint16_t sun2_state::ram_r(offs_t offset)
{
	if (offset < m_ram_size_words) return m_ram_ptr[offset];
	return 0xffff;
}

void sun2_state::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset < m_ram_size_words) COMBINE_DATA(&m_ram_ptr[offset]);
}

uint16_t sun2_state::mmu_r(offs_t offset, uint16_t mem_mask)
{
	return tl_mmu_r(m_maincpu->get_fc(), offset, mem_mask);
}

uint16_t sun2_state::tl_mmu_r(uint8_t fc, offs_t offset, uint16_t mem_mask)
{
	if ((fc == 3) && !machine().side_effects_disabled())
	{
		if (offset & 0x4)   // set for CPU space
		{
			switch (offset & 7)
			{
				case 4:
					//printf("sun2: Read IDPROM @ %x (PC=%x)\n", offset<<1, m_maincpu->pc());
					return m_idprom_ptr[(offset>>10) & 0x1f]<<8;

				case 5:
					//printf("sun2: Read diag reg\n");
					return m_diagreg;

				case 6:
					//printf("sun2: Read bus error @ PC %x\n", m_maincpu->pc());
					return m_buserror;

				case 7:
					//printf("sun2: Read sysenable\n");
					return m_sysenable;
			}
		}
		else                // clear for MMU space
		{
			int page;

			switch (offset & 3)
			{
				case 0: // page map
				case 1:
					page = m_segmap[m_context & 7][offset >> 14] << 4;
					page += ((offset >> 10) & 0xf);

					//printf("sun2: Read page map at %x (entry %d)\n", offset<<1, page);
					if (offset & 1) // low-order 16 bits
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
	if (machine().side_effects_disabled() && (offset >= (0xef0000>>1)) && (offset <= (0xef8000>>1)))
	{
		return m_rom_ptr[offset & 0x3fff];
	}

	// it's translation time
	uint8_t context = (fc & 4) ? ((m_context >> 8) & 7) : (m_context & 7);
	uint8_t pmeg = m_segmap[context][offset >> 14];
	uint32_t entry = (pmeg << 4) + ((offset >> 10) & 0xf);

	//  printf("sun2: Context = %d, pmeg = %d, offset >> 14 = %x, entry = %d, page = %d\n", context, pmeg, offset >> 14, entry, (offset >> 10) & 0xf);

	if (m_pagemap[entry] & PM_VALID)
	{
		m_pagemap[entry] |= PM_ACCESSED;

		// Sun2 implementations only use 12 bits from the page entry
		uint32_t tmp = (m_pagemap[entry] & 0xfff) << 10;
		tmp |= (offset & 0x3ff);

	//  if (!machine().side_effects_disabled())
	//      printf("sun2: Translated addr: %08x, type %d (page %d page entry %08x, orig virt %08x, FC %d)\n", tmp << 1, (m_pagemap[entry] >> 22) & 7, entry, m_pagemap[entry], offset<<1, fc);

		switch ((m_pagemap[entry] >> 22) & 7)
		{
			case 0: // type 0 space
				return m_type0space->read16(tmp, mem_mask);

			case 1: // type 1 space
				// EPROM space is special: the MMU has a trap door
				// where the original bits of the virtual address are
				// restored so that the entire 32K EPROM can be
				// accessed via a 2K single page view.  This isn't
				// obvious in the sun2 manual, but the sun3 manual
				// (sun3 has the same mechanism) explains it well.
				// the 2/50 ROM tests this specifically at $EF0DF0.
				if (m_idprom_ptr[1] == 0x02)    // 2/50 VMEbus has EPROM at 0x7F0000
				{
					if ((tmp >= (0x7f0000>>1)) && (tmp <= (0x7f07ff>>1)))
					{
						return m_rom_ptr[offset & 0x3fff]; // the mask here is probably &0x7fff, change it if any 8KW (1.x?) romset shows up for a VME machine
					}
				}
				else    // Multibus has EPROM at 0x000000
				{
					if (tmp <= (0x7ff>>1))
					{
						return m_rom_ptr[offset & 0x7fff];
					}
				}

				//printf("read device space @ %x\n", tmp<<1);
				return m_type1space->read16(tmp, mem_mask);

			case 2: // type 2 space
				return m_type2space->read16(tmp, mem_mask);

			case 3: // type 3 space
				return m_type3space->read16(tmp, mem_mask);
		}
	}
	else
	{
		if (!machine().side_effects_disabled()) printf("sun2: pagemap entry not valid!\n");
	}

	if (!machine().side_effects_disabled()) printf("sun2: Unmapped read @ %08x (FC %d, mask %04x, PC=%x, seg %x)\n", offset<<1, fc, mem_mask, m_maincpu->pc(), offset>>15);

	return 0xffff;
}

void sun2_state::mmu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	tl_mmu_w(m_maincpu->get_fc(), offset, data, mem_mask);
}

void sun2_state::tl_mmu_w(uint8_t fc, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("sun2: Write %04x (FC %d, mask %04x, PC=%x) to %08x\n", data, fc, mem_mask, m_maincpu->pc(), offset<<1);

	if (fc == 3)
	{
		if (offset & 0x4)   // set for CPU space
		{
			switch (offset & 7)
			{
				case 4:
					//printf("sun2: Write? IDPROM @ %x\n", offset<<1);
					return;

				case 5:
					// XOR to match Table 2-1 in the 2/50 Field Service Manual
					printf("sun2: CPU LEDs to %02x (PC=%x) => ", (data & 0xff) ^ 0xff, m_maincpu->pc());
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
					//printf("sun2: Write %04x to system enable\n", data);
					COMBINE_DATA(&m_sysenable);
					return;
			}
		}
		else                // clear for MMU space
		{
			int page;

			switch (offset & 3)
			{
				case 0: // page map
				case 1:
					page = m_segmap[m_context & 7][offset >> 14] << 4;
					page += ((offset >> 10) & 0xf);

					//printf("sun2: Write %04x to page map at %x (entry %d), ", data, offset<<1, page);
					if (offset & 1) // low-order 16 bits
					{
						m_pagemap[page] &= 0xffff0000;
						m_pagemap[page] |= data;
					}
					else
					{
						m_pagemap[page] &= 0x0000ffff;
						m_pagemap[page] |= (data<<16);
					}
					//printf("entry now %08x (adr %08x  PC=%x)\n", m_pagemap[page], (m_pagemap[page] & 0xfffff) << 11, m_maincpu->pc());
					return;

				case 2: // segment map
					//printf("sun2: Write %02x to segment map at %x (entry %d, user ctx %d PC=%x)\n", data & 0xff, offset<<1, offset>>14, m_context & 7, m_maincpu->pc());
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
	uint8_t context = (fc & 4) ? ((m_context >> 8) & 7) : (m_context & 7);
	uint8_t pmeg = m_segmap[context][offset >> 14];
	uint32_t entry = (pmeg << 4) + ((offset >> 10) & 0xf);

	if (m_pagemap[entry] & PM_VALID)
	{
		m_pagemap[entry] |= (PM_ACCESSED | PM_MODIFIED);

		// only 12 of the 20 bits in the page table entry are used on either Sun2 implementation
		uint32_t tmp = (m_pagemap[entry] & 0xfff) << 10;
		tmp |= (offset & 0x3ff);

		//if (!machine().side_effects_disabled()) printf("sun2: Translated addr: %08x, type %d (page entry %08x, orig virt %08x)\n", tmp << 1, (m_pagemap[entry] >> 22) & 7, m_pagemap[entry], offset<<1);

		switch ((m_pagemap[entry] >> 22) & 7)
		{
			case 0: // type 0
				m_type0space->write16(tmp, data, mem_mask);
				return;

			case 1: // type 1
				//printf("write device space @ %x\n", tmp<<1);
				m_type1space->write16(tmp, data, mem_mask);
				return;

			case 2: // type 2
				m_type2space->write16(tmp, data, mem_mask);
				return;

			case 3: // type 3
				m_type3space->write16(tmp, data, mem_mask);
				return;
		}
	}
	else
	{
		if (!machine().side_effects_disabled()) printf("sun2: pagemap entry not valid!\n");
	}

	printf("sun2: Unmapped write %04x (FC %d, mask %04x, PC=%x) to %08x\n", data, fc, mem_mask, m_maincpu->pc(), offset<<1);
}

// BW2 video control
uint16_t sun2_state::video_ctrl_r()
{
	return m_bw2_ctrl;
}

void sun2_state::video_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("sun2: BW2: %x to video_ctrl\n", data);
	COMBINE_DATA(&m_bw2_ctrl);
}

// 82586 Ethernet Data Link Controller interface
uint8_t sun2_state::ethernet_r()
{
	return m_ethernet_status;
}

void sun2_state::ethernet_w(uint8_t data)
{
	m_edlc->reset_w(!BIT(data, 7));
	m_edlc->set_loopback(!BIT(data, 6)); // LBC on MB502
	m_edlc->ca(BIT(data, 5));

	m_ethernet_status = (data & 0xf0) | (m_ethernet_status & 0x0f);
	m_maincpu->set_input_line(M68K_IRQ_3, BIT(m_ethernet_status, 0) && BIT(m_ethernet_status, 4) ? ASSERT_LINE : CLEAR_LINE);
}

void sun2_state::ethernet_int_w(int state)
{
	if (state)
	{
		m_ethernet_status |= 0x01;
		if (BIT(m_ethernet_status, 4))
			m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
	}
	else
	{
		m_ethernet_status &= 0xfe;
		if (BIT(m_ethernet_status, 4))
			m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
	}
}

uint16_t sun2_state::edlc_mmu_r(offs_t offset, uint16_t mem_mask)
{
	return swapendian_int16(tl_mmu_r(M68K_FC_SUPERVISOR_DATA, offset, swapendian_int16(mem_mask)));
}

void sun2_state::edlc_mmu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	tl_mmu_w(M68K_FC_SUPERVISOR_DATA, offset, swapendian_int16(data), swapendian_int16(mem_mask));
}

void sun2_state::sun2_mem(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(sun2_state::mmu_r), FUNC(sun2_state::mmu_w));
}

void sun2_state::edlc_mem(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(sun2_state::edlc_mmu_r), FUNC(sun2_state::edlc_mmu_w));
}

// VME memory spaces
// type 0 device space
void sun2_state::vmetype0space_map(address_map &map)
{
	map(0x000000, 0x7fffff).rw(FUNC(sun2_state::ram_r), FUNC(sun2_state::ram_w));
}

// type 1 device space
void sun2_state::vmetype1space_map(address_map &map)
{
	map(0x000000, 0x01ffff).ram().share(m_bw2_vram);
	map(0x020000, 0x020001).rw(FUNC(sun2_state::video_ctrl_r), FUNC(sun2_state::video_ctrl_w));
	map(0x7f0000, 0x7f07ff).rom().region("bootprom", 0);    // uses MMU loophole to read 32k from a 2k window
	map(0x7f0800, 0x7f0800).mirror(0x7fe).rw(FUNC(sun2_state::ethernet_r), FUNC(sun2_state::ethernet_w)).cswidth(16);
	// 7f1000-7f17ff: AM9518 encryption processor
	//map(0x7f1800, 0x7f1800).rw(SCC1_TAG, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w));
	//map(0x7f1802, 0x7f1802).rw(SCC1_TAG, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w));
	map(0x7f1804, 0x7f1805).nopr();
	//map(0x7f1804, 0x7f1804).rw(SCC1_TAG, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w));
	//map(0x7f1806, 0x7f1806).rw(SCC1_TAG, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w));
	map(0x7f2000, 0x7f2000).rw(SCC2_TAG, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w));
	map(0x7f2002, 0x7f2002).rw(SCC2_TAG, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w));
	map(0x7f2004, 0x7f2004).rw(SCC2_TAG, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w));
	map(0x7f2006, 0x7f2006).rw(SCC2_TAG, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w));
	map(0x7f2800, 0x7f2803).mirror(0x7fc).rw("timer", FUNC(am9513_device::read16), FUNC(am9513_device::write16));
}

// type 2 device space
void sun2_state::vmetype2space_map(address_map &map)
{
}

// type 3 device space
void sun2_state::vmetype3space_map(address_map &map)
{
}

// Multibus memory spaces
// type 0 device space
void sun2_state::mbustype0space_map(address_map &map)
{
	map(0x000000, 0x3fffff).rw(FUNC(sun2_state::ram_r), FUNC(sun2_state::ram_w));
	// 7f80000-7f807ff: Keyboard/mouse SCC8530
	//map(0x7f8000, 0x7f8007).rw(SCC1_TAG, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0xff00);
	map(0x700000, 0x71ffff).ram().share("bw2_vram");
	map(0x781800, 0x781801).rw(FUNC(sun2_state::video_ctrl_r), FUNC(sun2_state::video_ctrl_w));
}

// type 1 device space
void sun2_state::mbustype1space_map(address_map &map)
{
	map(0x000000, 0x0007ff).rom().region("bootprom", 0);    // uses MMU loophole to read 32k from a 2k window
	// 001000-0017ff: AM9518 encryption processor
	// 001800-001fff: Parallel port
	map(0x002000, 0x0027ff).rw(SCC2_TAG, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0xff00);
	map(0x002800, 0x002803).mirror(0x7fc).rw("timer", FUNC(am9513_device::read16), FUNC(am9513_device::write16));
	map(0x003800, 0x00383f).mirror(0x7c0).rw("rtc", FUNC(mm58167_device::read), FUNC(mm58167_device::write)).umask16(0xff00); // 12 wait states generated by PAL16R6 (U415)
}

// type 2 device space (Multibus memory space)
void sun2_state::mbustype2space_map(address_map &map)
{
}

// type 3 device space (Multibus I/O space)
void sun2_state::mbustype3space_map(address_map &map)
{
}

uint32_t sun2_state::bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const uint32_t palette[2] = { 0, 0xffffff };
	auto const vram = util::big_endian_cast<uint8_t const>(m_bw2_vram.target());

	if (!(m_bw2_ctrl & 0x8000)) return 0;

	for (int y = 0; y < 900; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 1152/8; x++)
		{
			uint8_t const pixels = vram[(y * (1152 / 8)) + x];

			*scanline++ = palette[BIT(pixels, 7)];
			*scanline++ = palette[BIT(pixels, 6)];
			*scanline++ = palette[BIT(pixels, 5)];
			*scanline++ = palette[BIT(pixels, 4)];
			*scanline++ = palette[BIT(pixels, 3)];
			*scanline++ = palette[BIT(pixels, 2)];
			*scanline++ = palette[BIT(pixels, 1)];
			*scanline++ = palette[BIT(pixels, 0)];
		}
	}

	return 0;
}

/* Input ports */
static INPUT_PORTS_START( sun2 )
INPUT_PORTS_END

void sun2_state::machine_start()
{
	m_rom_ptr = (uint16_t *)m_rom->base();
	m_idprom_ptr = (uint8_t *)m_idprom->base();
	m_ram_ptr = (uint16_t *)m_ram->pointer();
	m_ram_size = m_ram->size();
	m_ram_size_words = m_ram_size >> 1;

	m_bw2_ctrl = 0;

	m_ethernet_status = 0;
}

void sun2_state::machine_reset()
{
	m_diagreg = 0;
	m_sysenable = 0;
	m_context = 0;
	m_buserror = 0;
	memset(m_segmap, 0, sizeof(m_segmap));
	memset(m_pagemap, 0, sizeof(m_pagemap));

	if (m_edlc.found())
		ethernet_w(0);
}

void sun2_state::sun2vme(machine_config &config)
{
	/* basic machine hardware */
	M68010(config, m_maincpu, 19.6608_MHz_XTAL / 2); // or 24_MHz_XTAL / 2 by jumper setting
	m_maincpu->set_addrmap(AS_PROGRAM, &sun2_state::sun2_mem);

	RAM(config, RAM_TAG).set_default_size("2M").set_extra_options("4M,6M,8M").set_default_value(0x00);

	// MMU Type 0 device space
	ADDRESS_MAP_BANK(config, "type0").set_map(&sun2_state::vmetype0space_map).set_options(ENDIANNESS_BIG, 16, 32, 0x1000000);

	// MMU Type 1 device space
	ADDRESS_MAP_BANK(config, "type1").set_map(&sun2_state::vmetype1space_map).set_options(ENDIANNESS_BIG, 16, 32, 0x1000000);

	// MMU Type 2 device space
	ADDRESS_MAP_BANK(config, "type2").set_map(&sun2_state::vmetype2space_map).set_options(ENDIANNESS_BIG, 16, 32, 0x1000000);

	// MMU Type 3 device space
	ADDRESS_MAP_BANK(config, "type3").set_map(&sun2_state::vmetype3space_map).set_options(ENDIANNESS_BIG, 16, 32, 0x1000000);

	screen_device &bwtwo(SCREEN(config, "bwtwo", SCREEN_TYPE_RASTER));
	bwtwo.set_screen_update(FUNC(sun2_state::bw2_update));
	bwtwo.set_raw(100_MHz_XTAL, 1600, 0, 1152, 937, 0, 900);

	I82586(config, m_edlc, 16_MHz_XTAL / 2);
	m_edlc->set_addrmap(0, &sun2_state::edlc_mem);
	m_edlc->out_irq_cb().set(FUNC(sun2_state::ethernet_int_w));

	am9513a_device &timer(AM9513A(config, "timer", 19.6608_MHz_XTAL / 4));
	timer.fout_cb().set("timer", FUNC(am9513_device::gate1_w));
	timer.out1_cb().set_inputline(m_maincpu, M68K_IRQ_7);
	timer.out2_cb().set("irq5", FUNC(input_merger_device::in_w<0>));
	timer.out3_cb().set("irq5", FUNC(input_merger_device::in_w<1>));
	timer.out4_cb().set("irq5", FUNC(input_merger_device::in_w<2>));
	timer.out5_cb().set("irq5", FUNC(input_merger_device::in_w<3>));

	INPUT_MERGER_ANY_HIGH(config, "irq5").output_handler().set_inputline(m_maincpu, M68K_IRQ_5); // 74LS05 open collectors

	SCC8530N(config, SCC1_TAG, 19.6608_MHz_XTAL / 4);
	scc8530_device& scc2(SCC8530N(config, SCC2_TAG, 19.6608_MHz_XTAL / 4));
	scc2.out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	scc2.out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));
	scc2.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_6);

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(SCC2_TAG, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(SCC2_TAG, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(SCC2_TAG, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(SCC2_TAG, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(SCC2_TAG, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(SCC2_TAG, FUNC(z80scc_device::ctsb_w));
}

void sun2_state::sun2mbus(machine_config &config)
{
	/* basic machine hardware */
	M68010(config, m_maincpu, 39.3216_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun2_state::sun2_mem);

	RAM(config, RAM_TAG).set_default_size("2M").set_extra_options("4M").set_default_value(0x00);

	// MMU Type 0 device space
	ADDRESS_MAP_BANK(config, "type0").set_map(&sun2_state::mbustype0space_map).set_options(ENDIANNESS_BIG, 16, 32, 0x1000000);

	// MMU Type 1 device space
	ADDRESS_MAP_BANK(config, "type1").set_map(&sun2_state::mbustype1space_map).set_options(ENDIANNESS_BIG, 16, 32, 0x1000000);

	// MMU Type 2 device space
	ADDRESS_MAP_BANK(config, "type2").set_map(&sun2_state::mbustype2space_map).set_options(ENDIANNESS_BIG, 16, 32, 0x1000000);

	// MMU Type 3 device space
	ADDRESS_MAP_BANK(config, "type3").set_map(&sun2_state::mbustype3space_map).set_options(ENDIANNESS_BIG, 16, 32, 0x1000000);

	screen_device &bwtwo(SCREEN(config, "bwtwo", SCREEN_TYPE_RASTER));
	bwtwo.set_screen_update(FUNC(sun2_state::bw2_update));
	bwtwo.set_raw(100_MHz_XTAL, 1600, 0, 1152, 937, 0, 900);
	//bwtwo.set_raw(100_MHz_XTAL, 1600, 0, 1024, 1061, 0, 1024);

	am9513a_device &timer(AM9513A(config, "timer", 39.3216_MHz_XTAL / 8));
	timer.fout_cb().set("timer", FUNC(am9513_device::gate1_w));
	timer.out1_cb().set_inputline(m_maincpu, M68K_IRQ_7);
	timer.out2_cb().set("irq5", FUNC(input_merger_device::in_w<0>));
	timer.out3_cb().set("irq5", FUNC(input_merger_device::in_w<1>));
	timer.out4_cb().set("irq5", FUNC(input_merger_device::in_w<2>));
	timer.out5_cb().set("irq5", FUNC(input_merger_device::in_w<3>));

	INPUT_MERGER_ANY_HIGH(config, "irq5").output_handler().set_inputline(m_maincpu, M68K_IRQ_5); // 74LS05 open collectors

	SCC8530N(config, SCC1_TAG, 39.3216_MHz_XTAL / 8);
	scc8530_device& scc2(SCC8530N(config, SCC2_TAG, 39.3216_MHz_XTAL / 8));
	scc2.out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	scc2.out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));
	scc2.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_6);

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(SCC2_TAG, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(SCC2_TAG, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(SCC2_TAG, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(SCC2_TAG, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(SCC2_TAG, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(SCC2_TAG, FUNC(z80scc_device::ctsb_w));

	MM58167(config, "rtc", 32.768_kHz_XTAL);
}

/* ROM definition */
ROM_START( sun2_120 ) // ROMs are located on the '501-1007' CPU PCB at locations B11 and B10; J400 is set to 1-2 for 27128 EPROMs and 3-4 for 27256 EPROMs
	ROM_REGION16_BE(0x10000, "bootprom", ROMREGION_ERASEFF)
	// There is an undumped revision 1.1.2, which uses 27256 EPROMs
	ROM_SYSTEM_BIOS(0, "rev10f", "Bootrom Rev 1.0F")
	ROMX_LOAD("1.0f.b11", 0x0000, 0x8000, CRC(8fb0050a) SHA1(399cdb894b2a66d847d76d8a5d266906fb1d3430), ROM_SKIP(1) | ROM_BIOS(0)) // actual rom stickers had fallen off
	ROMX_LOAD("1.0f.b10", 0x0001, 0x8000, CRC(70de816d) SHA1(67e980497f463dbc529f64ec5f3e0046b3901b7e), ROM_SKIP(1) | ROM_BIOS(0)) // "
	ROM_SYSTEM_BIOS(1, "revr", "Bootrom Rev R")
	ROMX_LOAD("520-1102-03.b11", 0x0000, 0x4000, CRC(020bb0a8) SHA1(a7b60e89a40757975a5d345d57ea02781dea4f89), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("520-1101-03.b10", 0x0001, 0x4000, CRC(b97c61f7) SHA1(9f08fe232cfc3da48539fa66673fc1f89a362b1e), ROM_SKIP(1) | ROM_BIOS(1))
	// There is an undumped revision Q, with roms:
	//ROM_SYSTEM_BIOS( 8, "revq", "Bootrom Rev Q")
	// ROMX_LOAD( "520-1104-02.b11", 0x0000, 0x4000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(8))
	// ROMX_LOAD( "520-1103-02.b10", 0x0001, 0x4000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(8))
	ROM_SYSTEM_BIOS( 2, "revn", "Bootrom Rev N") // SunOS 2.0 requires this bootrom version at a minimum; this version supports the sun-2 keyboard
	ROMX_LOAD("revn.b11", 0x0000, 0x4000, CRC(b1e70965) SHA1(726b3ed9323750a1ae238cf6dccaed6ff5981ad1), ROM_SKIP(1) | ROM_BIOS(2)) // actual rom stickers had fallen off
	ROMX_LOAD("revn.b10", 0x0001, 0x4000, CRC(95fd9242) SHA1(1eee2d291f4b18f6aafdde1a9521d88e454843b9), ROM_SKIP(1) | ROM_BIOS(2)) // "
	ROM_SYSTEM_BIOS( 3, "revm", "Bootrom Rev M") // SunOS 1.0 apparently requires this bootrom revision; this version might only support the sun-1 keyboard?
	ROMX_LOAD("sun2-revm-8.b11", 0x0000, 0x4000, CRC(98b8ae55) SHA1(55485f4d8fd1ebc218aa8527c8bb62752c34abf7), ROM_SKIP(1) | ROM_BIOS(3)) // handwritten label: "SUN2-RevM-8"
	ROMX_LOAD("sun2-revm-0.b10", 0x0001, 0x4000, CRC(5117f431) SHA1(fce85c11ada1614152dde35bb329350f6fb2ecd9), ROM_SKIP(1) | ROM_BIOS(3)) // handwritten label: "SUN2-RevM-0"

	ROM_REGION(0x20, "idprom", ROMREGION_ERASEFF)
	ROM_LOAD("sun2120-idprom.bin", 0x000000, 0x000020, CRC(eec8cd1d) SHA1(6a78dc0ea6f9cc7687cffea754d65864fb751ebf))
ROM_END

ROM_START( sun2_50 )
	ROM_REGION16_BE(0x8000, "bootprom", ROMREGION_ERASEFF)
	// There is at least one undumped revision (Rev 1.1.2) which uses 27256 EPROMs; the sun2/50 board handles up to 27512 EPROMs
	// bootrom rev Q
	ROM_LOAD16_BYTE("250_q_8.rom", 0x0000, 0x4000, CRC(5bfacb5c) SHA1(ec7fb3fb0217b0138ba4748b7c79b8ff0cad896b))
	ROM_LOAD16_BYTE("250_q_0.rom", 0x0001, 0x4000, CRC(2ee29abe) SHA1(82f52b9f25e92387329581f7c8ba50a171784968))

	ROM_REGION(0x20, "idprom", ROMREGION_ERASEFF)
	ROM_LOAD("sun250-idprom.bin", 0x000000, 0x000020, CRC(927744ab) SHA1(d29302b69128165e69dd3a79b8c8d45f2163b88a))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS       INIT        COMPANY             FULLNAME     FLAGS
COMP( 1984, sun2_50,  0,      0,      sun2vme,  sun2,  sun2_state, empty_init, "Sun Microsystems", "Sun 2/50",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1984, sun2_120, 0,      0,      sun2mbus, sun2,  sun2_state, empty_init, "Sun Microsystems", "Sun 2/120", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
