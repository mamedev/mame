// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, R. Belmont
/***************************************************************************

  sun3.c: preliminary driver for Sun 3 models

    Sun-3 Models
    ------------

    3/160
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1096/1163/1164/1208
        Chassis type:   deskside
        Bus:            VME, 12 slots
        Memory:         16M physical (documented), 256M virtual, 270ns cycle
        Notes:          First 68020-based Sun machine. Uses the 3004
                        "Carrera" CPU, which is used in most other Sun
                        3/1xx models and the 3/75. Sun supplied 4M
                        memory expansion boards; third parties had up to
                        32M on one card. SCSI optional. One variant of
                        the memory card holds a 6U VME SCSI board; there
                        is also a SCSI board which sits in slot 7 of the
                        backplane and runs the SCSI bus out the back of
                        the backplane to the internal disk/tape (slot 6
                        in very early backplanes). CPU has two serial
                        ports, Ethernet, keyboard. Type 3 keyboard plugs
                        into the CPU; Sun-3 mouse plugs into the
                        keyboard. Upgradeable to a 3/260 by replacing
                        CPU and memory boards.

    3/75
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1094/1163/1164
        Chassis type:   wide pizza box
        Bus:            VME, 2 slot
        Memory:         16M physical (documented), 256M virtual, 270ns cycle
        Notes:          Optional SCSI sits on memory expansion board in
                        second slot.

    3/140
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1094/1163/1164/1208
        Chassis type:   deskside
        Bus:            VME, 3 slots
        Memory:         16M physical (documented), 256M virtual, 270ns cycle

    3/150
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1094/1163/1164/1208
        Chassis type:   deskside
        Bus:            VME, 6 slots
        Memory:         16M physical (documented), 256M virtual, 270ns cycle

    3/180
        Processor(s):   68020 @ 16.67MHz, 68881, Sun-3 MMU, 8 hardware
                        contexts, 2 MIPS
        CPU:            501-1074/1094/1163/1164/1208
        Chassis type:   rackmount
        Bus:            VME, 12 slots
        Memory:         16M physical (documented), 256M virtual, 270ns cycle
        Notes:          Rackmount version of 3/160. Upgradeable to a
                        3/280 by replacing the CPU and memory boards.
                        Very early backplanes have the special SCSI
                        hookup on slot 6 rather than 7.

    3/110
        Processor(s):   68020
        CPU:            501-1134/1209
        Chassis type:   deskside
        Bus:            VME, 3 slots
        Notes:          Similar to the "Carerra" CPU, but has 8-bit
                        color framebuffer (cgfour) on board and uses 1M
                        RAM chips for 4M on-CPU memory. Code-named
                        "Prism".

    3/50
        Processor(s):   68020 @ 15.7MHz, 68881 (socket for
                        501-1075/1133/1162, installed for 501-1207),
                        Sun-3 MMU, 8 hardware contexts, 1.5 MIPS
        CPU:            501-1075/1133/1162/1207
        Chassis type:   wide pizza box
        Bus:            none
        Memory:         4M physical (documented), 256M virtual, 270ns cycle
        Notes:          Cycle-stealing monochrome frame buffer. 4M
                        memory maximum stock, but third-party memory
                        expansion boards were sold, allowing up to at
                        least 12M. No bus or P4 connector. Onboard SCSI.
                        Thin coax or AUI Ethernet. Code-named "Model
                        25".

    3/60
        Processor(s):   68020 @ 20MHz, 68881 (stock), Sun-3 MMU,
                        8 hardware contexts, 3 MIPS
        CPU:            501-1205/1322/1334/1345
        Chassis type:   wide pizza box
        Bus:            P4 connector (not same as P4 on 3/80)
        Memory:         24M physical, 256M virtual, 200ns cycle
        Notes:          VRAM monochome frame buffer for 501-1205/1334.
                        Optional color frame buffer (can run mono and
                        color simultaneously) on P4 connector. Onboard
                        SCSI. SIMM memory (100ns 1M x 9 SIMMs). High
                        (1600 * 1100) or low (1152 * 900) resolution
                        mono selectable by jumper. Thin coax or AUI
                        Ethernet. Code-named "Ferrari". 4M stock on
                        501-1205/1322, 0M stock on 501-1322/1345.

    3/60LE
        Processor(s):   68020 @ 20MHz, 68881 (stock), Sun-3 MMU,
                        8 hardware contexts, 3 MIPS
        CPU:            501-1378
        Bus:            P4 connector (not same as P4 on 3/80)
        Memory:         12M physical, 256M virtual, 200ns cycle
        Notes:          A version of the 3/60 with no onboard
                        framebuffer and limited to 12M of RAM (4M of
                        256K SIMMs and 8M of 1M SIMMs).

    3/260
        Processor(s):   68020 @ 25MHz, 68881 @ 20MHz (stock), Sun-3 MMU,
                        8 hardware contexts, 4 MIPS
        CPU:            501-1100/1206
        Chassis type:   deskside
        Bus:            VME, 12 slot
        Memory:         64M (documented) physical with ECC, 256M virtual;
                        64K write-back cache, direct-mapped,
                        virtually-indexed and virtually-tagged, with
                        16-byte lines; 80ns cycle
        Notes:          Two serial ports, AUI Ethernet, keyboard, and
                        video on CPU. Video is mono, high-resolution
                        only. Sun supplied 8M memory boards. Sun 4/2xx
                        32M boards work up to 128M. First Sun with an
                        off-chip cache. Upgradeable to a 4/260 by
                        replacing the CPU board. Code-named "Sirius".

    3/280
        Processor(s):   68020 @ 25MHz, 68881 @ 20MHz (stock), Sun-3 MMU,
                        8 hardware contexts, 4 MIPS
        CPU:            501-1100/1206
        Chassis type:   rackmount
        Bus:            VME, 12 slot
        Memory:         64M (documented) physical with ECC, 256M virtual;
                        64K write-back cache, direct-mapped,
                        virtually-indexed and virtually-tagged, with
                        16-byte lines; 80ns cycle
        Notes:          Rackmount version of the 3/260. Upgradeable to a
                        4/280 by replacing the CPU board. Code-named
                        "Sirius".

    3/E
        Processor(s):   68020
        CPU:            501-8028
        Bus:            VME
        Notes:          Single-board VME Sun-3, presumably for use as a
                        controller, not as a workstation. 6U form
                        factor. Serial and keyboard ports. External RAM,
                        framebuffer, and SCSI/ethernet boards
                        available.

3/60 ROM breakpoints of interest
fefb104 - bus error test
fefb18e - interrupt test
fefb1da - clock IRQ test
fefb344 - MMU page valid test
fefb3c4 - MMU read-only permissions test
fefb45e - parity test: no spurious NMI generated from normal parity operation
fefb50c - parity test: NMI is generated when we write with inverted parity and enable parity NMIs
fefb5c8 - size memory by bus error
fef581c - EPROM mapping check
fef02b2 - main screen turn on

3/260 ROM breakpoints of interest
fefcb34 - bpset to stop before each test
fefc0c2 - start of ECC test
fefc34a - start of mem_size, which queries ECC registers for each memory board

****************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68020.h"
#include "machine/am9516.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/timekpr.h"
#include "machine/timer.h"
#include "machine/z80scc.h"
#include "machine/am79c90.h"
#include "machine/ncr5380.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"

#include "bus/rs232/rs232.h"
#include "bus/sunkbd/sunkbd.h"
#include "bus/sunmouse/sunmouse.h"

#include "screen.h"


namespace {

#define TIMEKEEPER_TAG  "timekpr"
#define SCC1_TAG        "scc1"
#define SCC2_TAG        "scc2"
#define RS232A_TAG      "rs232a"
#define RS232B_TAG      "rs232b"
#define KEYBOARD_TAG    "keyboard"
#define MOUSE_TAG       "mouseport"

// page table entry constants
#define PM_VALID    (0x80000000)    // page is valid
#define PM_WRITEMASK (0x40000000)   // writable?
#define PM_SYSMASK  (0x20000000)    // system use only?
#define PM_CACHE    (0x10000000)    // cachable?
#define PM_TYPEMASK (0x0c000000)    // type mask
#define PM_ACCESSED (0x02000000)    // accessed flag
#define PM_MODIFIED (0x01000000)    // modified flag

#define BE_FPENABLE (0x04)  // FPU not enabled
#define BE_FPBERR   (0x08)  // FPU encountered a bus error
#define BE_VMEBERR  (0x10)  // VME encountered a bus error
#define BE_TIMEOUT  (0x20)  // timeout - memory doesn't exist
#define BE_PROTERR  (0x40)  // protection failed on MMU page lookup
#define BE_INVALID  (0x80)  // invalid entry on MMU page lookup

class sun3_state : public driver_device
{
public:
	sun3_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_scc1(*this, SCC1_TAG),
		m_scc2(*this, SCC2_TAG),
		m_sbc(*this, "scsibus:7:sbc"),
		m_udc(*this, "udc"),
		m_p_ram(*this, "p_ram"),
		m_bw2_vram(*this, "bw2_vram"),
		m_type0space(*this, "type0"),
		m_type1space(*this, "type1"),
		m_type2space(*this, "type2"),
		m_type3space(*this, "type3"),
		m_rom(*this, "user1"),
		m_idprom(*this, "idprom"),
		m_ram(*this, RAM_TAG),
		m_lance(*this, "lance"),
		m_diagsw(*this, "diagsw")
	{ }

	void sun3(machine_config &config);
	void sun3e(machine_config &config);
	void sun3_60(machine_config &config);
	void sun3200(machine_config &config);
	void sun3_50(machine_config &config);

	void ncr5380(device_t *device);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68020_device> m_maincpu;
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;
	required_device<ncr5380_device> m_sbc;
	required_device<am9516_device> m_udc;

	optional_shared_ptr<uint32_t> m_p_ram;
	optional_shared_ptr<uint32_t> m_bw2_vram;
	optional_device<address_map_bank_device> m_type0space, m_type1space, m_type2space, m_type3space;
	required_memory_region m_rom, m_idprom;
	required_device<ram_device> m_ram;
	required_device<am79c90_device> m_lance;
	required_ioport m_diagsw;

	uint32_t tl_mmu_r(offs_t offset, uint32_t mem_mask = ~0);
	void tl_mmu_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t parity_r(offs_t offset);
	void parity_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t ecc_r(offs_t offset);
	void ecc_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t irqctrl_r();
	void irqctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t rtc7170_r(offs_t offset);
	void rtc7170_w(offs_t offset, uint8_t data);
	uint16_t scsictrl_r();
	void scsictrl_w(uint16_t data);

	template <unsigned W, unsigned H>
	uint32_t bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t bw2_350_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(sun3_timer);

	void sun3_mem(address_map &map) ATTR_COLD;
	void vmetype0space_map(address_map &map) ATTR_COLD;
	void vmetype0space_novram_map(address_map &map) ATTR_COLD;
	void vmetype1space_map(address_map &map) ATTR_COLD;
	void vmetype2space_map(address_map &map) ATTR_COLD;
	void vmetype3space_map(address_map &map) ATTR_COLD;

	uint32_t enable_r();

	uint32_t *m_rom_ptr, *m_ram_ptr;
	uint8_t *m_idprom_ptr;
	uint32_t m_enable, m_diag, m_dvma_enable, m_parregs[8], m_irqctrl, m_ecc[4];
	uint16_t m_scsictrl;
	uint8_t m_buserr;

	uint32_t m_context;
	uint8_t m_segmap[8][2048];
	uint32_t m_pagemap[4096];
	uint32_t m_ram_size, m_ram_size_words;
	bool m_bInBusErr;

	uint32_t m_cache_tags[0x4000], m_cache_data[0x4000];
};

static void sun_cdrom(device_t *device)
{
	downcast<nscsi_cdrom_device &>(*device).set_block_size(512);
}

void sun3_state::ncr5380(device_t *device)
{
	devcb_base *devcb;
	(void)devcb;
//  downcast<ncr5380_device &>(*device).drq_handler().set(FUNC(sun3_state::drq_w));
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("tape", NSCSI_TAPE);
	device.set_option_machine_config("cdrom", sun_cdrom);
}

uint32_t sun3_state::ram_r(offs_t offset)
{
	if (m_ecc[0] == 0x10c00000)
	{
		//printf("ECC testing, RAM read ofs %x\n", offset);
		m_ecc[1] &= 0x00ffffff;

		if (m_ecc[2] == 0x01000000) // single-bit error test
		{
			m_ecc[1] |= 0x8f000000; // put in the syndrome code the first ECC test wants

			if (offset == 0)
			{
				return 0x80000000;
			}
		}
		else if (m_ecc[2] == 0x01000200)    // double-bit error test
		{
			m_ecc[1] |= 0xfc000000;
		}
	}

	if (offset < m_ram_size_words) return m_ram_ptr[offset];

	if (!m_bInBusErr)
	{
		//printf("ram_r: bus error on timeout, access to invalid addr %08x, PC=%x\n", offset<<2, m_maincpu->pc());
		//fflush(stdout);
		m_buserr = BE_TIMEOUT;
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}

	return 0xffffffff;
}

void sun3_state::ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// if writing bad parity is enabled
	if (((m_parregs[0] & 0x20000000) == 0x20000000) &&
		(m_irqctrl & 0x01000000) &&
		!(m_bInBusErr))
	{
		m_parregs[1] = offset<<2;
		//printf("Generating parity error, mem_mask %08x\n", mem_mask);
		switch (mem_mask)
		{
			case 0xff000000:
				m_parregs[0] |= 0x08<<24;
				break;

			case 0x00ff0000:
				m_parregs[1] += 1;
				m_parregs[0] |= 0x04<<24;
				break;

			case 0x0000ff00:
				m_parregs[1] += 2;
				m_parregs[0] |= 0x02<<24;
				break;

			case 0x000000ff:
				m_parregs[1] += 3;
				m_parregs[0] |= 0x01<<24;
				break;

			case 0x0000ffff:
				m_parregs[1] += 2;
				m_parregs[0] |= 0x03<<24;
				break;

			case 0xffff0000:
				m_parregs[0] |= 0x0c<<24;
				break;

			case 0xffffffff:    // no address adjust, show all 4 lanes as problematic
				m_parregs[0] |= 0x0f<<24;
				break;
		}

		// indicate parity interrupt
		m_parregs[0] |= 0x80000000;

		// and can we take that now?
		if (m_parregs[0] & 0x40000000)
		{
			m_bInBusErr = true; // prevent recursion
			m_maincpu->set_input_line(M68K_IRQ_7, ASSERT_LINE);
		}
	}

	if (offset < m_ram_size_words)
	{
		COMBINE_DATA(&m_ram_ptr[offset]);
		return;
	}

	if (!m_bInBusErr)
	{
		//printf("ram_w: bus error on timeout, access to invalid addr %08x, PC=%x\n", offset<<2, m_maincpu->pc());
		fflush(stdout);
		m_buserr = BE_TIMEOUT;
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}

uint32_t sun3_state::tl_mmu_r(offs_t offset, uint32_t mem_mask)
{
	uint8_t fc = m_maincpu->get_fc();

	if ((fc == 3) && !machine().side_effects_disabled())
	{
		int page;

		switch (offset >> 26)
		{
			case 0: // IDPROM
				//printf("sun3: Read IDPROM at %x (mask %08x)\n", offset, mem_mask);
				return m_idprom_ptr[(offset*4)] << 24 | m_idprom_ptr[(offset*4)+1]<<16 | m_idprom_ptr[(offset*4)+2]<<8 | m_idprom_ptr[(offset*4)+3];

			case 1: // page map
				page = m_segmap[m_context & 7][(offset >> 15) & 0x7ff] << 4;
				page += (offset >> 11) & 0xf;
				//printf("sun3: Read page map at %x (entry %d)\n", offset<<1, page);
				return m_pagemap[page];

			case 2: // segment map
				//printf("sun3: Read segment map at %x (entry %d, user ctx %d mask %x)\n", offset<<2, (offset & ~0x3c000000) >> 15, m_context & 7, mem_mask);
				return m_segmap[m_context & 7][(offset >> 15) & 0x7ff]<<24;

			case 3: // context reg
				//printf("sun3: Read context reg\n");
				return m_context<<24;

			case 4: // enable reg
				return enable_r();

			case 5: // DVMA enable
				return m_dvma_enable<<24;

			case 6: // bus error
				m_bInBusErr = false;
				//printf("Reading bus error: %02x\n", m_buserr);
				return m_buserr<<24;

			case 7: // diagnostic reg
				return 0;

			case 8: // cache tags
				//printf("sun3: read cache tags @ %x, PC = %x\n", offset, m_maincpu->pc());
				return m_cache_tags[(offset & 0x3fff) >> 2];

			case 9: // cache data
				//printf("sun3: read cache data @ %x, PC = %x\n", offset, m_maincpu->pc());
				return m_cache_data[(offset & 0x3fff)];

			case 10: // flush cache
				return 0xffffffff;

			case 11: // block copy
				printf("sun3: read block copy @ %x, PC = %x\n", offset, m_maincpu->pc());
				return 0xffffffff;

			case 15: // UART bypass
				//printf("sun3: read UART bypass @ %x, PC = %x, mask = %08x\n", offset, m_maincpu->pc(), mem_mask);
				return 0xffffffff;
		}
	}

	// boot mode?
	if ((fc == M68K_FC_SUPERVISOR_PROGRAM) && !(enable_r() & 0x80))
	{
		return m_rom_ptr[offset & 0x3fff];
	}

	// debugger hack
	if (machine().side_effects_disabled() && (offset >= (0xfef0000>>2)) && (offset <= (0xfefffff>>2)))
	{
		return m_rom_ptr[offset & 0x3fff];
	}

	// it's translation time
	uint8_t pmeg = m_segmap[m_context & 7][(offset >> 15) & 0x7ff];
	uint32_t entry = (pmeg << 4) + ((offset >> 11) & 0xf);

	//printf("sun3: Context = %d, pmeg = %d, offset >> 15 = %x, entry = %d, page = %d\n", m_context&7, pmeg, (offset >> 15) & 0x7ff, entry, (offset >> 11) & 0xf);

	if (m_pagemap[entry] & PM_VALID)
	{
		if ((m_pagemap[entry] & PM_SYSMASK) && (fc < M68K_FC_SUPERVISOR_DATA))
		{
			m_buserr = BE_PROTERR;
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
			m_bInBusErr = true;
			return 0xffffffff;
		}

		m_pagemap[entry] |= PM_ACCESSED;

		uint32_t tmp = (m_pagemap[entry] & 0x7ffff) << 11;
		tmp |= (offset & 0x7ff);

		//printf("pmeg %d, entry %d = %08x, virt %08x => tmp %08x\n", pmeg, entry, m_pagemap[entry], offset << 2, tmp);

	//  if (!machine().side_effects_disabled())
		//printf("sun3: Translated addr: %08x, type %d (page %d page entry %08x, orig virt %08x, FC %d)\n", tmp << 2, (m_pagemap[entry] >> 26) & 3, entry, m_pagemap[entry], offset<<2, fc);

		switch ((m_pagemap[entry] >> 26) & 3)
		{
			case 0: // type 0 space
				return m_type0space->read32(tmp, mem_mask);

			case 1: // type 1 space
				// magic ROM bypass
				if ((tmp >= (0x100000>>2)) && (tmp <= (0x10ffff>>2)))
				{
					return m_rom_ptr[offset & 0x3fff];
				}
				return m_type1space->read32(tmp, mem_mask);

			case 2: // type 2 space
				return m_type2space->read32(tmp, mem_mask);

			case 3: // type 3 space
				return m_type3space->read32(tmp, mem_mask);
		}
	}
	else
	{
//      if (!machine().side_effects_disabled()) printf("sun3: pagemap entry not valid! (PC=%x)\n", m_maincpu->pc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_buserr = BE_INVALID;
		m_bInBusErr = true;
		return 0xffffffff;
	}

	if (!machine().side_effects_disabled()) logerror("sun3: Unmapped read @ %08x (FC %d, mask %08x, PC=%x, seg %x)\n", offset<<2, fc, mem_mask, m_maincpu->pc(), offset>>15);

	return 0xffffffff;
}

void sun3_state::tl_mmu_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint8_t fc = m_maincpu->get_fc();

	//printf("sun3: Write %08x (FC %d, mask %08x, PC=%x) to %08x\n", data, fc, mem_mask, m_maincpu->pc(), offset<<1);

	if (fc == 3)    // control space
	{
		int page;

		switch (offset >> 26)
		{
			case 0: // IDPROM
				return;

			case 1: // page map  fefaf32
				page = m_segmap[m_context & 7][(offset >> 15) & 0x7ff] << 4;
				//printf("context = %d, segment = %d, PMEG = %d, add = %d\n", m_context & 7, (offset >> 15) & 0x7ff, page, (offset >> 11) & 0xf);
				page += (offset >> 11) & 0xf;

				//printf("sun3: Write %04x to page map at %x (entry %d), ", data, offset<<2, page);
				COMBINE_DATA(&m_pagemap[page]);

				//printf("entry now %08x (adr %08x  PC=%x mask %x)\n", m_pagemap[page], (m_pagemap[page] & 0xfffff) << 13, m_maincpu->pc(), mem_mask);
				return;

			case 2: // segment map
				//printf("sun3: Write %02x to segment map at %x (entry %d, user ctx %d PC=%x mask %x)\n", (data>>24) & 0xff, offset<<2, (offset & ~0x3c000000)>>15, m_context & 7, m_maincpu->pc(), mem_mask);
				m_segmap[m_context & 7][(offset >> 15) & 0x7ff] = (data>>24) & 0xff;
				//printf("segment map[%d][%d] now %x\n", m_context & 7, (offset & ~0x3c000000) >> 15, m_segmap[m_context & 7][(offset & ~0x3c000000) >> 15] = (data>>24) & 0xff);
				return;

			case 3: // context reg
				//printf("sun3: Write (%x) %x to context\n", data, data>>24);
				m_context = data >> 24;
				return;

			case 4: // enable reg
				//printf("sun3: Write %x to enable, PC=%x\n", data, m_maincpu->pc());
				COMBINE_DATA(&m_enable);
				return;

			case 5: // DVMA enable
				m_dvma_enable = data>>24;
				return;

			case 6: // bus error (read-only)
				return;

			case 7: // diagnostic reg
				m_diag = data >> 24;
				#if 0
				printf("sun3: CPU LEDs to %02x (PC=%x) => ", ((data>>24) & 0xff) ^ 0xff, m_maincpu->pc());
				for (int i = 0; i < 8; i++)
				{
					if (m_diag & (1<<i))
					{
						printf(".");
					}
					else
					{
						printf("*");
					}
				}
				printf("\n");
				#endif
				return;

			case 8: // cache tags
				//printf("sun3: %08x to cache tags @ %x, PC = %x, mask = %08x\n", data, offset, m_maincpu->pc(), mem_mask);
				m_cache_tags[(offset & 0x3fff) >> 2] = data;
				return;

			case 9: // cache data
				//printf("sun3: %08x to cache data @ %x, PC = %x, mask = %08x\n", data, offset, m_maincpu->pc(), mem_mask);
				m_cache_data[(offset & 0x3fff)] = data;
				return;

			case 10: // flush cache
				return;

			case 11: // block copy
				printf("sun3: %08x to block copy @ %x, PC = %x\n", data, offset, m_maincpu->pc());
				return;

			case 15: // UART bypass
				//printf("sun3: %08x to UART bypass @ %x, PC = %x\n", data, offset, m_maincpu->pc());
				return;
			}
	}

	// it's translation time
	uint8_t pmeg = m_segmap[m_context & 7][(offset >> 15) & 0x7ff];
	uint32_t entry = (pmeg << 4) + ((offset >> 11) & 0xf);

	if (m_pagemap[entry] & PM_VALID)
	{
		if ((!(m_pagemap[entry] & PM_WRITEMASK)) ||
			((m_pagemap[entry] & PM_SYSMASK) && (fc < M68K_FC_SUPERVISOR_DATA)))
		{
			//printf("sun3: write protect MMU error (PC=%x)\n", m_maincpu->pc());
			m_buserr = BE_PROTERR;
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
			m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
			m_bInBusErr = true;
			return;
		}

		m_pagemap[entry] |= (PM_ACCESSED | PM_MODIFIED);

		uint32_t tmp = (m_pagemap[entry] & 0x7ffff) << 11;
		tmp |= (offset & 0x7ff);

		//if (!machine().side_effects_disabled()) printf("sun3: Translated addr: %08x, type %d (page entry %08x, orig virt %08x)\n", tmp << 2, (m_pagemap[entry] >> 26) & 3, m_pagemap[entry], offset<<2);

		switch ((m_pagemap[entry] >> 26) & 3)
		{
			case 0: // type 0
				m_type0space->write32(tmp, data, mem_mask);
				return;

			case 1: // type 1
				//printf("write device space @ %x\n", tmp<<1);
				m_type1space->write32(tmp, data, mem_mask);
				return;

			case 2: // type 2
				m_type2space->write32(tmp, data, mem_mask);
				return;

			case 3: // type 3
				m_type3space->write32(tmp, data, mem_mask);
				return;
		}
	}
	else
	{
		//if (!machine().side_effects_disabled()) printf("sun3: pagemap entry not valid!\n");
		m_buserr = BE_INVALID;
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_bInBusErr = true;
		return;
	}

	logerror("sun3: Unmapped write %04x (FC %d, mask %04x, PC=%x) to %08x\n", data, fc, mem_mask, m_maincpu->pc(), offset<<2);
}

uint32_t sun3_state::parity_r(offs_t offset)
{
	uint32_t rv = m_parregs[offset];

	if (offset == 0)    // clear interrupt if any
	{
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
		m_parregs[offset] &= ~0x8f000000;
	}

	return rv;
}

void sun3_state::parity_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//printf("sun3: %08x to parity registers @ %x (mask %08x)\n", data, offset, mem_mask);

	if (offset == 0)
	{
		m_parregs[0] &= 0x8f000000;

		if ((m_parregs[0] & 0x80000000) && (data & 0x40000000))
		{
			m_maincpu->set_input_line(M68K_IRQ_7, ASSERT_LINE);
		}

		m_parregs[0] |= (data & 0x70000000);
	}
	else
	{
		COMBINE_DATA(&m_parregs[offset]);
	}
}

void sun3_state::sun3_mem(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(sun3_state::tl_mmu_r), FUNC(sun3_state::tl_mmu_w));
}

// type 0 device space
void sun3_state::vmetype0space_map(address_map &map)
{
	map(0x00000000, 0x08ffffff).rw(FUNC(sun3_state::ram_r), FUNC(sun3_state::ram_w));
	map(0xfe400000, 0xfe41ffff).ram(); // not sure what's going on here (3/110)
	map(0xff000000, 0xff03ffff).ram().share(m_bw2_vram);
}

// type 0 without VRAM (3/50)
void sun3_state::vmetype0space_novram_map(address_map &map)
{
	map(0x00000000, 0x08ffffff).rw(FUNC(sun3_state::ram_r), FUNC(sun3_state::ram_w));
}

// type 1 device space
void sun3_state::vmetype1space_map(address_map &map)
{
	map(0x00000000, 0x0000000f).rw(m_scc1, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x00020000, 0x0002000f).rw(m_scc2, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff00ff00);
	map(0x00040000, 0x000407ff).ram().share("nvram");   // type 2816 parallel EEPROM
	map(0x00060000, 0x0006ffff).rw(FUNC(sun3_state::rtc7170_r), FUNC(sun3_state::rtc7170_w));
	map(0x00080000, 0x0008000f).rw(FUNC(sun3_state::parity_r), FUNC(sun3_state::parity_w));
	map(0x000a0000, 0x000a0003).rw(FUNC(sun3_state::irqctrl_r), FUNC(sun3_state::irqctrl_w));
	map(0x00100000, 0x0010ffff).rom().region("user1", 0);
	map(0x00120000, 0x00120003).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w));
	map(0x00140000, 0x00140007).rw(m_sbc, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)).umask32(0xffffffff);
	map(0x00140010, 0x00140011).rw(m_udc, FUNC(am9516_device::data_r), FUNC(am9516_device::data_w));
	map(0x00140012, 0x00140013).rw(m_udc, FUNC(am9516_device::addr_r), FUNC(am9516_device::addr_w));
	map(0x00140018, 0x00140019).rw(FUNC(sun3_state::scsictrl_r), FUNC(sun3_state::scsictrl_w));
	map(0x001e0000, 0x001e00ff).rw(FUNC(sun3_state::ecc_r), FUNC(sun3_state::ecc_w));
}

// type 2 device space
void sun3_state::vmetype2space_map(address_map &map)
{
}

// type 3 device space
void sun3_state::vmetype3space_map(address_map &map)
{
}

uint32_t sun3_state::enable_r()
{
	// Incorporate diag switch value.
	const uint32_t diagsw = m_diagsw->read() << 24;
	return (m_enable & ~(u32(1) << 24)) | diagsw;
}

uint32_t sun3_state::irqctrl_r()
{
	return m_irqctrl;
}

void sun3_state::irqctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//printf("sun3: %08x to interrupt control (mask %08x)\n", data, mem_mask);
	COMBINE_DATA(&m_irqctrl);

	if (data & 0x01000000)
	{
		if (data & 0x02000000)
		{
			m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
		}
		if (data & 0x04000000)
		{
			m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
		}
		if (data & 0x08000000)
		{
			m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);
		}
	}
	else    // master enable clear, clear all interrupts
	{
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
	}
}

uint8_t sun3_state::rtc7170_r(offs_t offset)
{
	//printf("read 7170 @ %x, PC=%x\n", offset, m_maincpu->pc());

	return 0xff;
}

void sun3_state::rtc7170_w(offs_t offset, uint8_t data)
{
	//printf("%02x to 7170 @ %x\n", data, offset);

	if ((offset == 0x11) && (data == 0x1c))
	{
		if ((m_irqctrl & 0x21000000) == 0x21000000)
		{
			m_maincpu->set_input_line(M68K_IRQ_5, ASSERT_LINE);
		}
	}
}

uint16_t sun3_state::scsictrl_r()
{
	return m_scsictrl;
}

void sun3_state::scsictrl_w(uint16_t data)
{
	if (BIT(data ^ m_scsictrl, 0))
	{
		m_sbc->reset();
		m_udc->reset();
	}

	m_scsictrl = (m_scsictrl & 0xff00) | (data & 0x000f);
}

uint32_t sun3_state::ecc_r(offs_t offset)
{
	//printf("read ECC @ %x, PC=%x\n", offset, m_maincpu->pc());
	// fefc34a
	int mbram = (m_ram_size / (1024*1024));
	int beoff = (mbram / 32) * 0x10;

	//printf("offset %x MB %d beoff %x\n", offset, mbram, beoff);

	if (offset >= beoff)
	{
		m_buserr = BE_TIMEOUT;
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}

	uint32_t rv = m_ecc[offset & 0xf];

	if ((offset & 0xf) == 0) rv |= 0x06000000;  // indicate each ECC board is 32MB, for 128MB total

	return rv;
}

void sun3_state::ecc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	//printf("%08x to ecc @ %x, mask %08x\n", data, offset, mem_mask);

	offset &= 0xf;
	m_ecc[offset] = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(sun3_state::sun3_timer)
{
	if ((m_irqctrl & 0x81000000) == 0x81000000)
	{
		//printf("NMI tick\n");
		m_maincpu->set_input_line(M68K_IRQ_7, CLEAR_LINE);
		m_maincpu->set_input_line(M68K_IRQ_7, ASSERT_LINE);
	}
}

template <unsigned W, unsigned H>
uint32_t sun3_state::bw2_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const uint32_t palette[2] = { 0, 0xffffff };
	auto const vram = util::big_endian_cast<uint8_t const>(m_bw2_vram.target());

	for (int y = 0; y < H; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < W/8; x++)
		{
			uint8_t const pixels = vram[(y * (W / 8)) + x];

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

uint32_t sun3_state::bw2_350_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const uint32_t palette[2] = { 0, 0xffffff };
	auto const vram = util::big_endian_cast<uint8_t const>(m_ram_ptr + (0x100000 >> 2));

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
static INPUT_PORTS_START( sun3 )
	PORT_START("diagsw")
	PORT_CONFNAME(1, 0, "Diagnostic Switch")
	PORT_CONFSETTING(0, "Normal")
	PORT_CONFSETTING(1, "Diagnostic")
INPUT_PORTS_END

void sun3_state::machine_start()
{
	m_rom_ptr = (uint32_t *)m_rom->base();
	m_ram_ptr = (uint32_t *)m_ram->pointer();
	m_idprom_ptr = (uint8_t *)m_idprom->base();
	m_ram_size = m_ram->size();
	m_ram_size_words = m_ram_size >> 2;
	std::fill(std::begin(m_parregs), std::end(m_parregs), 0);
	m_scsictrl = 0;
}

void sun3_state::machine_reset()
{
	m_enable = 0;
	m_buserr = 0;
	m_diag = 1;
	m_dvma_enable = 0;
	m_irqctrl = 0;
	m_bInBusErr = false;

	scsictrl_w(0);
}

// The base Sun 3004 CPU board
void sun3_state::sun3(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, 16670000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun3_state::sun3_mem);

	screen_device &bwtwo(SCREEN(config, "bwtwo", SCREEN_TYPE_RASTER));
	bwtwo.set_screen_update(NAME((&sun3_state::bw2_update<1152, 900>)));
	bwtwo.set_size(1600,1100);
	bwtwo.set_visarea(0, 1152-1, 0, 900-1);
	bwtwo.set_refresh_hz(72);

	RAM(config, m_ram).set_default_size("4M").set_extra_options("6M,8M,12M,16M,20M,24M,28M,32M").set_default_value(0x00);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// MMU Type 0 device space
	ADDRESS_MAP_BANK(config, "type0").set_map(&sun3_state::vmetype0space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// MMU Type 1 device space
	ADDRESS_MAP_BANK(config, "type1").set_map(&sun3_state::vmetype1space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// MMU Type 2 device space
	ADDRESS_MAP_BANK(config, "type2").set_map(&sun3_state::vmetype2space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// MMU Type 3 device space
	ADDRESS_MAP_BANK(config, "type3").set_map(&sun3_state::vmetype3space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	TIMER(config, "timer").configure_periodic(FUNC(sun3_state::sun3_timer), attotime::from_hz(100));

	SCC8530N(config, m_scc1, 4.9152_MHz_XTAL);
	m_scc1->out_txda_callback().set(KEYBOARD_TAG, FUNC(sun_keyboard_port_device::write_txd));
	m_scc1->out_txdb_callback().set(MOUSE_TAG, FUNC(sun_mouse_port_device::write_txd));

	SUNKBD_PORT(config, KEYBOARD_TAG, default_sun_keyboard_devices, "type3hle").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxa_w));

	SUNMOUSE_PORT(config, MOUSE_TAG, default_sun_mouse_devices, "hle1200").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxb_w));

	SCC8530N(config, m_scc2, 4.9152_MHz_XTAL);
	m_scc2->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc2->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsb_w));

	AM79C90(config, m_lance, 10'000'000); // clock is a guess

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:1", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsibus:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", scsi_devices, "tape");
	NSCSI_CONNECTOR(config, "scsibus:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsibus:7").option_set("sbc", NCR5380).machine_config([this] (device_t *device) { ncr5380(device); });

	AM9516(config, m_udc, 16_MHz_XTAL / 2);
}

// Sun 3/60
void sun3_state::sun3_60(machine_config &config)
{
	sun3(config);
	M68020(config.replace(), m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun3_state::sun3_mem);

	screen_device &bwtwo(*subdevice<screen_device>("bwtwo"));
	bwtwo.set_screen_update(NAME((&sun3_state::bw2_update<1600, 1100>)));
	bwtwo.set_size(1600,1100);
	bwtwo.set_visarea(0, 1600-1, 0, 1100-1);
	bwtwo.set_refresh_hz(72);
}

// Sun 3/E
void sun3_state::sun3e(machine_config &config)
{
	sun3(config);

	M68020(config.replace(), m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun3_state::sun3_mem);
}

// 3/260 and 3/280 (the Sun 3200 board)
void sun3_state::sun3200(machine_config &config)
{
	sun3(config);
	M68020(config.replace(), m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun3_state::sun3_mem);

	screen_device &bwtwo(*subdevice<screen_device>("bwtwo"));
	bwtwo.set_screen_update(NAME((&sun3_state::bw2_update<1600, 1100>)));
	bwtwo.set_size(1600,1100);
	bwtwo.set_visarea(0, 1600-1, 0, 1100-1);
	bwtwo.set_refresh_hz(72);

	m_ram->set_default_size("32M").set_extra_options("64M,96M,128M");
}

void sun3_state::sun3_50(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, 15700000);
	m_maincpu->set_addrmap(AS_PROGRAM, &sun3_state::sun3_mem);

	AM79C90(config, m_lance, 10'000'000); // clock is a guess

	screen_device &bwtwo(SCREEN(config, "bwtwo", SCREEN_TYPE_RASTER));
	bwtwo.set_screen_update(FUNC(sun3_state::bw2_350_update));
	bwtwo.set_size(1600,1100);
	bwtwo.set_visarea(0, 1152-1, 0, 900-1);
	bwtwo.set_refresh_hz(72);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, "timer").configure_periodic(FUNC(sun3_state::sun3_timer), attotime::from_hz(100));

	RAM(config, m_ram).set_default_size("4M").set_default_value(0x00);

	// MMU Type 0 device space
	ADDRESS_MAP_BANK(config, "type0").set_map(&sun3_state::vmetype0space_novram_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// MMU Type 1 device space
	ADDRESS_MAP_BANK(config, "type1").set_map(&sun3_state::vmetype1space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// MMU Type 2 device space
	ADDRESS_MAP_BANK(config, "type2").set_map(&sun3_state::vmetype2space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	// MMU Type 3 device space
	ADDRESS_MAP_BANK(config, "type3").set_map(&sun3_state::vmetype3space_map).set_options(ENDIANNESS_BIG, 32, 32, 0x80000000);

	SCC8530N(config, m_scc1, 4.9152_MHz_XTAL);
	m_scc1->out_txda_callback().set(KEYBOARD_TAG, FUNC(sun_keyboard_port_device::write_txd));
	m_scc1->out_txdb_callback().set(MOUSE_TAG, FUNC(sun_mouse_port_device::write_txd));

	SUNKBD_PORT(config, KEYBOARD_TAG, default_sun_keyboard_devices, "type3hle").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxa_w));

	SUNMOUSE_PORT(config, MOUSE_TAG, default_sun_mouse_devices, "hle1200").rxd_handler().set(m_scc1, FUNC(z80scc_device::rxb_w));

	SCC8530N(config, m_scc2, 4.9152_MHz_XTAL);
	m_scc2->out_txda_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_scc2->out_txdb_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsb_w));

	NSCSI_BUS(config, "scsibus");
	NSCSI_CONNECTOR(config, "scsibus:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:1", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsibus:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:4", scsi_devices, "tape");
	NSCSI_CONNECTOR(config, "scsibus:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsibus:6", scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsibus:7").option_set("sbc", NCR5380).machine_config([this] (device_t *device) { ncr5380(device); });

	AM9516(config, m_udc, 16_MHz_XTAL / 2);
}

/* ROM definition */

ROM_START( sun3_50 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/50 V1.2 Bootprom
Sun 3/50 V1.4 Bootprom
Sun 3/50 V1.6 Bootprom
Sun 3/50 V1.8 Bootprom (Req. to load SunOS QIC-24 1/4" tapes)
Sun 3/50 V2.0 Bootprom
Sun 3/50 V2.1 Bootprom
Sun 3/50 V2.3 Bootprom
Sun 3/50 V2.5 Bootprom (Req. to load SunOS QIC-24 1/4" tapes from a Sun-2 Shoebox)
Sun 3/50 V2.6 Bootprom
Sun 3/50 V2.7 Bootprom
Sun 3/50 V2.8 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev28", "Rev 2.8")
	ROMX_LOAD( "sun3_50_v2.8", 0x0000, 0x10000, CRC(1ca6b0e8) SHA1(5773ac1c46399501d29d1758aa342862b03ec472), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rev27", "Rev 2.7")
	ROMX_LOAD( "sun3_50_v2.7", 0x0000, 0x10000, CRC(7c4a9e20) SHA1(6dcd4883a170538050fd0e1f151fae413ec9ea52), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "rev26", "Rev 2.6")
	ROMX_LOAD( "sun3_50_v2.6", 0x0000, 0x10000, CRC(08abbb3b) SHA1(6bfb8d5c97d801cd7bb7d564de0e68a48fb807c4), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "rev23", "Rev 2.3")
	ROMX_LOAD( "sun3_50_v2.3", 0x0000, 0x10000, CRC(163500b3) SHA1(437c8d539e12d442ca6877566dbbe165d577fcab), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "rev16", "Rev 1.6")
	ROMX_LOAD( "sun3_50_v1.6", 0x0000, 0x10000, CRC(8be20826) SHA1(2a4d73fcb7fe0f0c83eb0f4c91d957b7bf88b7ed), ROM_BIOS(4))

	ROM_REGION( 0x20, "idprom", ROMREGION_ERASEFF)
	ROM_LOAD( "sun3-50-idprom.bin", 0x000000, 0x000020, CRC(80610dbe) SHA1(0f37e31ed209b8905c5dc7c2663fa01a9b9baaba) )
ROM_END

ROM_START( sun3_60 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/60 V1.0 Bootprom
Sun 3/60 V1.3 Bootprom
Sun 3/60 V1.5 Bootprom
Sun 3/60 V1.6 Bootprom (Req. to load SunOS QIC-24 1/4" tapes
Sun 3/60 V1.9 Bootprom
Sun 3/60 V2.8.3 Bootprom
Sun 3/60 V3.0.1 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev301", "Rev 3.0.1")
	ROMX_LOAD( "sun_3.60v3.0.1", 0x0000, 0x10000, CRC(e55dc1d8) SHA1(6e48414ce2139282e69f57612b20f7d5c475e74c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rev283", "Rev 2.8.3")
	ROMX_LOAD( "sun_3.60v2.8.3", 0x0000, 0x10000, CRC(de4ec54d) SHA1(e621a9c1a2a7df4975b12fa3a0d7f106383736ef), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "rev19", "Rev 1.9")
	ROMX_LOAD( "sun_3.60v1.9",   0x0000, 0x10000, CRC(32b6d3a9) SHA1(307756ba5698611d51059881057f8086956ce895), ROM_BIOS(2))

	ROM_REGION( 0x20, "idprom", ROMREGION_ERASEFF)
	ROM_LOAD( "sun3-60-idprom.bin", 0x000000, 0x000020, CRC(117e766a) SHA1(f01547be0156bd4e06bbdee4c342d1b38c7646ae) )
ROM_END

ROM_START( sun3_110 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/110 V1.8 Bootprom
Sun 3/110 V2.1 Bootprom
Sun 3/110 V2.3 Bootprom
Sun 3/110 V2.6 Bootprom
Sun 3/110 V2.7 Bootprom
Sun 3/110 V2.8 Bootprom
Sun 3/110 V3.0 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_110_v3.0", 0x0000, 0x10000, CRC(a193b26b) SHA1(0f54212ee3a5709f70e921069cca1ddb8c143b1b), ROM_BIOS(0))

	ROM_REGION( 0x20, "idprom", ROMREGION_ERASEFF)
	ROM_LOAD( "sun3-110-idprom.bin", 0x000000, 0x000020, CRC(d6cd934a) SHA1(b0913708fe733250ef5c1289c10146dcef6d1a67) )
ROM_END

ROM_START( sun3_150 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/1[4,5,6,8]0 V1.3 Bootprom
Sun 3/1[4,5,6,8]0 V1.4 Bootprom
Sun 3/1[4,5,6,8]0 V1.5 Bootprom
Sun 3/1[4,5,6,8]0 V1.8 Bootprom (Req. to load SunOS QIC-24 1/4" tapes)
Sun 3/1[4,5,6,8]0 V2.1 Bootprom
Sun 3/1[4,5,6,8]0 V2.1 Bootprom with Capricot Rimfire 3200/3400 support (b rf(0,0,0) works)
Sun 3/1[4,5,6,8]0 V2.3 Bootprom
Sun 3/1[4,5,6,8]0 V2.6 Bootprom (Req. to load SunOS QIC-24 1/4" tapes from a Sun-2 Shoebox and for Xylogics 7053)
Sun 3/1[4,5,6,8]0 V2.7 Bootprom
Sun 3/1[4,5,6,8]0 V2.8 Bootprom
Sun 3/1[4,5,6,8]0 V2.8.4 Bootprom
Sun 3/1[4,5,6,8]0 V3.0 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_160_v3.0",   0x0000, 0x10000, CRC(fee6e4d6) SHA1(440d532e1848298dba0f043de710bb0b001fb675), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rev284", "Rev 2.8.4")
	ROMX_LOAD( "sun3_160_v2.8.4", 0x0000, 0x10000, CRC(3befd013) SHA1(f642bb42200b794e6e32e2fe6c87d5c269c8656d), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "rev23", "Rev 2.3")
	ROMX_LOAD( "sun3_160_v2.3",   0x0000, 0x10000, CRC(09585745) SHA1(1de1725dd9e27f5a910989bbb5b51acfbdc1d70b), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "rev21rf", "Rev 2.1 RF")
	ROMX_LOAD( "sun3_160_v2.1_rf",   0x0000, 0x10000, CRC(5c7e9271) SHA1(5e4dbb50859a21f9e1d3e4a06c42494d13a9a8eb), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "rev15", "Rev 1.5")
	ROMX_LOAD( "sun3_160_v1.5",   0x0000, 0x10000, CRC(06daee37) SHA1(b9873cd48d78ad8e0c85d69966fc20c21cfc99aa), ROM_BIOS(4))


	ROM_REGION( 0x20, "idprom", ROMREGION_ERASEFF)
	ROM_LOAD( "sun3-150-idprom.bin", 0x000000, 0x000020, CRC(58956a93) SHA1(7334936dc945e05d63a94a33340e963a371672c9) )
ROM_END

ROM_START( sun3_260 )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
/*
Sun 3/260/280 V1.8 Bootprom
Sun 3/260/280 V2.1 Bootprom ( 2x^G cause system to beep 'till reset)
Sun 3/260/280 V2.3 Bootprom
Sun 3/260/280 V2.6 Bootprom (Req. for Xylogics 7053)
Sun 3/260/280 V2.7 Bootprom
Sun 3/260/280 V2.8 Bootprom
Sun 3/260/280 V2.8.4 Bootprom
Sun 3/260/280 V3.0 Bootprom
*/
	ROM_SYSTEM_BIOS(0, "rev30", "Rev 3.0")
	ROMX_LOAD( "sun3_260_v3.0", 0x0000, 0x10000, CRC(f43ed1d3) SHA1(204880436bd087ede136f853610403d75e60bd75), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rev27", "Rev 2.7")
	ROMX_LOAD( "sun3_260_v2.7", 0x0000, 0x10000, CRC(099fcaab) SHA1(4a5233c778676f48103bdd8bab03b4264686b4aa), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "rev26", "Rev 2.6")
	ROMX_LOAD( "sun3_260_v2.6", 0x0000, 0x10000, CRC(e8b17951) SHA1(e1fdef42670a349d99b0eca9c50c8566b8bb7c56), ROM_BIOS(2))

	ROM_REGION( 0x20, "idprom", ROMREGION_ERASEFF)
	ROM_LOAD( "sun3-260-idprom.bin", 0x000000, 0x000020, CRC(d51794f3) SHA1(17930c773b6fe9a32819094ffaf69e5453d1ea4d) )
ROM_END

ROM_START( sun3_e )
	ROM_REGION32_BE( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "rev28", "Rev 3.2")
	ROMX_LOAD( "sun3_e.32", 0x0000, 0x10000, CRC(acedde7e) SHA1(1ab6ec28f4365a613a5e326c34cb37585c3f0ecc), ROM_BIOS(0))

	ROM_REGION( 0x20, "idprom", ROMREGION_ERASEFF)
	ROM_LOAD( "sun3-e-idprom.bin", 0x000000, 0x000020, CRC(d1a92116) SHA1(4836f3188f2c3dd5ba49ab66e0b55caa6b1b1791) )
ROM_END

} // Anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY             FULLNAME                    FLAGS
COMP( 198?, sun3_50,  0,      0,      sun3_50, sun3,  sun3_state, empty_init, "Sun Microsystems", "Sun 3/50",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Model 25
COMP( 1988, sun3_60,  0,      0,      sun3_60, sun3,  sun3_state, empty_init, "Sun Microsystems", "Sun 3/60",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Ferrari
COMP( 198?, sun3_110, 0,      0,      sun3,    sun3,  sun3_state, empty_init, "Sun Microsystems", "Sun 3/110",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Prism
COMP( 1985, sun3_150, 0,      0,      sun3,    sun3,  sun3_state, empty_init, "Sun Microsystems", "Sun 3/75/140/150/160/180", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // AKA Carrera
COMP( 198?, sun3_260, 0,      0,      sun3200, sun3,  sun3_state, empty_init, "Sun Microsystems", "Sun 3/260/280",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Prism
COMP( 198?, sun3_e,   0,      0,      sun3e,   sun3,  sun3_state, empty_init, "Sun Microsystems", "Sun 3/E",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Polaris
