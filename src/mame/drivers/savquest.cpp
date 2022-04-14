// license:BSD-3-Clause
// copyright-holders:R. Belmont, Peter Ferrie
/***************************************************************************

    savquest.cpp

    "Savage Quest" (c) 1999 Interactive Light, developed by Angel Studios.
    Skeleton by R. Belmont

    TODO:
    - currently asserts by selecting a s3 video bank above 1M (register 0x6a)z

    - The version is labeled "SQ05" in the filesystem but has the 1999 release year.
      Other components are labeled "v0.5", but the game doesn't boot far enough to see if
      any graphics have version information. There appears to also be a "Savage Quest 2.1" which
      is undumped.

	- Update: Machine boots now, with voodoo card detected, and gives you command prompt after
	  failing to retrieve HASP key from the dongle. Still can't run SQ05 as the binary is SRM
	  enveloped and only prompts "HASP key not found". The SQ05 directory structure is also
	  impaired to a degree refusing to copy files or list files with dir, however tree works.

	  Voodoo custom PCI Headers are properly implemented now, allows for voodoo card to be
	  detected, see references for specification. This should probably be ported to library
	  video/voodoo_2 implementation. The card is initialized on boot but renders incorrectly,
	  perhaps the actual opengl translation from 3Dfx implementation is missing still.
	  To reset the display back to vga pass-through after boot run \sb16\diagnose

	  Without lpt controller added the boot script will go into a loop querying the lpt read and
	  write. Not convinced that does as advertised leaving it running for 24 hours still had no
	  HASP key. Adding the lpt controller the boot script detects the missing dongle and aborts,
	  giving access to the dos prompt.

	  82371AB not correctly emulated yet, as per previous dev's TODO comment on identification.
	  This is causing the IRQs to conflict as they are not properly allocated with no allocations
	  beyond IRQ 7, which appears to be the only usable channel. Tried to use the machine/i2371SB
	  implementation but at closer inspection noticed that SB is PIIX3 whereas AB is PIIX4 and these
	  specifications are different, see references.

	  The primary slave, cdrom, and secondary IDE drives are now detected. Secondary drive mounts the
	  same image as the primary master but read only, requires a mapped image to be detectable, and
	  is accessible but only on IRQ7 probably due to incomplete 82371AB emulation. Could not manage to
	  mount cdrom with any image to be usable, might be IRQ conflicts but could be incompatible with
	  the ATAPI driver.

	  Added timings for ISA and PCI bus as per PII Motherboard specification, see references.

	  Changed driver game flags to only prompt protection not implemented as this will ultimately be
	  the deciding impediment to playing the game even after the mentioned emulations are completed.
	  At least this will give us a working PIIX4 implementation booting into DOS6.11 to work from.

    PCI list:
    Bus no. Device No. Func No. Vendor ID Device ID Device Class          IRQ
    0       7          1        8086      7111      IDE Controller        14
    0       7          2        8086      7112      Serial Bus Controller 11
    0       9          0        5333      8901      Display Controller    10
    0       13         0        121a      0002      Multimedia Device     NA
    - First two are PIIX4/4E/4M IDE Controller / PIIX4/4E/4M USB Interface
      Third is S3 trio64uv+
      Fourth is Voodoo 2 3D Accelerator
    Sound Blaster is ISA/PNP

============================================================================
    H/W is a white-box PC consisting of:
    Pentium II 450 CPU
    DFI P2XBL motherboard (i440BX chipset)
    128 MB RAM
    Guillemot Maxi Gamer 3D2 Voodoo II
    Sound Blaster AWE64

    Protected by a HASP brand parallel port dongle.
    I/O board has a PIC17C43 which is not readable.

    On boot it reports: S3 86C775/86C705 Video BIOS. Version 2.04.11 Copyright 1996 S3 Incorporated.

- update by Peter Ferrie:
- split BIOS region into 16kb blocks and implement missing PAM registers

- HASP emulator by Peter Ferrie

References:
Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)
http://web.archive.org/web/20210806094059/https://www.intel.com/Assets/PDF/specupdate/297738.pdf
Intel 82371AB PCI-TO-ISA / IDE XCELERATOR (PIIX4)
https://web.archive.org/web/20211110080535/https://www.intel.com/Assets/PDF/datasheet/290562.pdf
Intel 82371AB (PIIX4) PCI ISA IDE XCELERATOR TIMING SPECIFICATIONS
https://web.archive.org/web/20210429213149/https://www.intel.com/Assets/PDF/datasheet/290548.pdf
Intel Pentium II Mainboard
https://web.archive.org/web/20120630212356/http://www.motherboards.org/files/manuals/42/MB-P6F91i.pdf
3Dfx VOODOO2 Graphics Specification
https://web.archive.org/web/20210803204742/https://www.voodooalert.de/de/files/whitepaper/voodoo2.pdf

***************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pcshare.h"
#include "machine/pckeybrd.h"
#include "machine/pc_lpt.h"
#include "machine/idectrl.h"
#include "video/pc_vga.h"
#include "video/voodoo_2.h"
#include "machine/ds128x.h"
#include "bus/isa/sblaster.h"


namespace {

class savquest_state : public pcat_base_state
{
public:
	savquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
		, m_vga(*this, "vga")
	    , m_voodoo(*this, "voodoo")
		, m_lpt(*this, "lpt")
	{
		std::fill(std::begin(m_mtxc_config_reg), std::end(m_mtxc_config_reg), 0);
	}

	void savquest(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	std::unique_ptr<uint32_t[]> m_bios_f0000_ram;
	std::unique_ptr<uint32_t[]> m_bios_e0000_ram;
	std::unique_ptr<uint32_t[]> m_bios_e4000_ram;
	std::unique_ptr<uint32_t[]> m_bios_e8000_ram;
	std::unique_ptr<uint32_t[]> m_bios_ec000_ram;

	std::unique_ptr<uint8_t[]> m_smram;

	required_device<s3_vga_device> m_vga;
	required_device<voodoo_2_device> m_voodoo;
	required_device<pc_lpt_device> m_lpt;

	int m_haspind = 0;
	int m_haspstate = 0;
	enum hasp_states
	{
		HASPSTATE_NONE,
		HASPSTATE_PASSBEG,
		HASPSTATE_PASSEND,
		HASPSTATE_READ
	};

	int m_hasp_passind = 0;
	uint8_t m_hasp_tmppass[0x29]{};
	uint8_t m_port379 = 0;
	int m_hasp_passmode = 0;
	int m_hasp_prodind = 0;

	uint8_t m_mtxc_config_reg[256]{};
	uint8_t m_piix4_config_reg[8][256]{};
	uint32_t m_pci_3dfx_regs[0xff]{}; // provide 256 bytes of configuration registers

	void bios_f0000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void bios_e0000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void bios_e4000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void bios_e8000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void bios_ec000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t parallel_port_r(offs_t offset);
	void parallel_port_w(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(vblank_assert);

	uint8_t smram_r(offs_t offset);
	void smram_w(offs_t offset, uint8_t data);

	void savquest_io(address_map &map);
	void savquest_map(address_map &map);

	void intel82439tx_init();
	void vid_3dfx_init();

	uint8_t mtxc_config_r(int function, int reg);
	void mtxc_config_w(int function, int reg, uint8_t data);
	uint32_t intel82439tx_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint8_t piix4_config_r(int function, int reg);
	void piix4_config_w(int function, int reg, uint8_t data);
	uint32_t intel82371ab_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint32_t pci_3dfx_r(int function, int reg, uint32_t mem_mask);
	void pci_3dfx_w(int function, int reg, uint32_t data, uint32_t mem_mask);
};

// Intel 82439TX System Controller (MTXC)

uint8_t savquest_state::mtxc_config_r(int function, int reg)
{
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	if((reg & 0xfe) == 0)
		return (reg & 1) ? 0x80 : 0x86; // Vendor ID, Intel

	if((reg & 0xfe) == 2)
		return (reg & 1) ? 0x70 : 0x00; // Device ID, MTXC

	return m_mtxc_config_reg[reg];
}

void savquest_state::mtxc_config_w(int function, int reg, uint8_t data)
{
//  osd_printf_debug("%s:MXTC: write %d, %02X, %02X\n", machine().describe_context(), function, reg, data);

	#if 1
	switch(reg)
	{
		case 0x59:      // PAM0
		{
			if (data & 0x10)        // enable RAM access to region 0xf0000 - 0xfffff
			{
				membank("bios_f0000")->set_base(m_bios_f0000_ram.get());
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				membank("bios_f0000")->set_base(memregion("bios")->base() + 0x30000);
			}
			break;
		}

		case 0x5e:      // PAM5
		{
			if (data & 0x10)        // enable RAM access to region 0xe4000 - 0xe7fff
			{
				membank("bios_e4000")->set_base(m_bios_e4000_ram.get());
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				membank("bios_e4000")->set_base(memregion("bios")->base() + 0x24000);
			}

			if (data & 1)       // enable RAM access to region 0xe0000 - 0xe3fff
			{
				membank("bios_e0000")->set_base(m_bios_e0000_ram.get());
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				membank("bios_e0000")->set_base(memregion("bios")->base() + 0x20000);
			}
			break;
		}

		case 0x5f:      // PAM6
		{
			if (data & 0x10)        // enable RAM access to region 0xec000 - 0xeffff
			{
				membank("bios_ec000")->set_base(m_bios_ec000_ram.get());
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				membank("bios_ec000")->set_base(memregion("bios")->base() + 0x2c000);
			}

			if (data & 1)       // enable RAM access to region 0xe8000 - 0xebfff
			{
				membank("bios_e8000")->set_base(m_bios_e8000_ram.get());
			}
			else                    // disable RAM access (reads go to BIOS ROM)
			{
				membank("bios_e8000")->set_base(memregion("bios")->base() + 0x28000);
			}
			break;
		}
	}
	#endif

	m_mtxc_config_reg[reg] = data;
}

void savquest_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
	m_smram = std::make_unique<uint8_t[]>(0x20000);
}

uint32_t savquest_state::intel82439tx_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= mtxc_config_r(function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= mtxc_config_r(function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= mtxc_config_r(function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= mtxc_config_r(function, reg + 0) << 0;
	}
	return r;
}

void savquest_state::intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		mtxc_config_w(function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		mtxc_config_w(function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		mtxc_config_w(function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		mtxc_config_w(function, reg + 0, (data >> 0) & 0xff);
	}
}

// Intel 82371AB PCI-to-ISA / IDE bridge (PIIX4)
uint8_t savquest_state::piix4_config_r(int function, int reg)
{
//  osd_printf_debug("PIIX4: read %d, %02X\n", function, reg);

	if((reg & 0xfe) == 0)
		return (reg & 1) ? 0x80 : 0x86; // Vendor ID, Intel

	if((reg & 0xfe) == 2)
	{
		/* TODO: it isn't detected properly (i.e. PCI writes always goes to function == 0) */
		// printf("TODO: Properly identify 82371AB PIIX4: read %d, %02X\n", function, reg);
		if(function == 0)
			return (reg & 1) ? 0x71 : 0x10; // Device ID, 82371AB PCI to ISA Bridge
		if(function == 1)
			return (reg & 1) ? 0x71 : 0x11; // Device ID, 82371AB IDE Controller
		if(function == 2)
			return (reg & 1) ? 0x71 : 0x12; // Device ID, 82371AB Universal Serial Bus Controller
		if(function == 3)
			return (reg & 1) ? 0x71 : 0x1; // Device ID, 82371AB System Management Bus and Power
	}

	return m_piix4_config_reg[function][reg];
}

void savquest_state::piix4_config_w(int function, int reg, uint8_t data)
{
//  osd_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine().describe_context(), function, reg, data);
	m_piix4_config_reg[function][reg] = data;
}

uint32_t savquest_state::intel82371ab_pci_r(int function, int reg, uint32_t mem_mask)
{
	uint32_t r = 0;
	if (ACCESSING_BITS_24_31)
	{
		r |= piix4_config_r(function, reg + 3) << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		r |= piix4_config_r(function, reg + 2) << 16;
	}
	if (ACCESSING_BITS_8_15)
	{
		r |= piix4_config_r(function, reg + 1) << 8;
	}
	if (ACCESSING_BITS_0_7)
	{
		r |= piix4_config_r(function, reg + 0) << 0;
	}
	return r;
}

void savquest_state::intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	if (ACCESSING_BITS_24_31)
	{
		piix4_config_w(function, reg + 3, (data >> 24) & 0xff);
	}
	if (ACCESSING_BITS_16_23)
	{
		piix4_config_w(function, reg + 2, (data >> 16) & 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		piix4_config_w(function, reg + 1, (data >> 8) & 0xff);
	}
	if (ACCESSING_BITS_0_7)
	{
		piix4_config_w(function, reg + 0, (data >> 0) & 0xff);
	}
}

void savquest_state::vid_3dfx_init()
{
	// This might be a bug, card does not get detected unless standard PCI header model + vendor is used
	m_pci_3dfx_regs[0x00] = 0x0002 << 16 | 0x121a; // Voodoo2 device | 3Dfx Interactive, Inc. vendor

	// Custom 3Dfx Voodoo2 PCI Header Specification (see 3Dfx Voodoo2 spec)

	// This is the specified value for register 0x00 but doesn't work without device added
	//m_pci_3dfx_regs[0x00] = 0x121a; // 3Dfx Interactive, Inc. vendor ro
	m_pci_3dfx_regs[0x02] = 0x02;   // Voodoo2 device ro
	m_pci_3dfx_regs[0x04] = 0x00;   // command rw bit 1
	m_pci_3dfx_regs[0x06] = 0x150;	// status register (medium timing, fast back-to-back) ro
	m_pci_3dfx_regs[0x08] = 0x02;   // revision ID ro
	m_pci_3dfx_regs[0x09] = 0x38000;	 // class code (display controller, non vga compatible) ro
	//m_pci_3dfx_regs[0x09] = 0x40000;	 // class code (videon multimedia device) ro
	m_pci_3dfx_regs[0x0c] = 0x00;	// cache line size ro
	m_pci_3dfx_regs[0x0d] = 0x00;	// latency timer ro
	m_pci_3dfx_regs[0x0e] = 0x00;	// header type (other) ro
	m_pci_3dfx_regs[0x0f] = 0x00;	// BIST ro
	m_pci_3dfx_regs[0x10] = 0xff000008;  // memory base address 31:24 rw
    //m_pci_3dfx_regs[0x10] = 0x10000008;// memory base address 31:24 rw
	m_pci_3dfx_regs[0x3c] = 0x00;   // interrupt mapping IRQ 0-15 rw
	m_pci_3dfx_regs[0x3d] = 0x01;   // external interrupt connection ro
	m_pci_3dfx_regs[0x3e] = 0x00;   // bus master minimum grand time ro
	m_pci_3dfx_regs[0x3f] = 0x00;   // bus master minimum latency ro
	m_pci_3dfx_regs[0x40] = 0x6000; // init enable 12-16 ro
	m_pci_3dfx_regs[0x44] = 0x00;   // chuck bus snooping address 1 wo
	m_pci_3dfx_regs[0x48] = 0x00;   // chuck bus snooping address 0 wo
	m_pci_3dfx_regs[0x4c] = 0x00;   // aliased memory-mapped status register (returns 0x06 register)
	m_pci_3dfx_regs[0x50] = 0x00;   // scratchpad register rw
	// Osc Force On, Osc Ring Sel, Osc Count Reset, 12 bit PCI Counter, 16 bit Oscillator Counter
	m_pci_3dfx_regs[0x54] = 0x6002; // silicon process monitor register ro
}

uint32_t savquest_state::pci_3dfx_r(int function, int reg, uint32_t mem_mask)
{
	switch (reg)
	{
		case 0x44:
		case 0x48:
			return 0x00; // wo registers
		case 0x4c:
			return m_pci_3dfx_regs[0x06]; // alias status
	}
	return m_pci_3dfx_regs[reg];
}

void savquest_state::pci_3dfx_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	switch (reg)
	{
		case 0x04: // bit 1 rw
			m_pci_3dfx_regs[reg] = (m_pci_3dfx_regs[reg] & ~0x01) | (data & 0x01);
		break;
		case 0x10: // bit 24-31 rw
			m_pci_3dfx_regs[reg] = (m_pci_3dfx_regs[reg] & ~0xff000000) | (data & 0xff000000);
		break;
		case 0x40: // 12-16 ro
			// osd_printf_warning("Reg 0x40 init enable\n");
			m_pci_3dfx_regs[reg] = (m_pci_3dfx_regs[reg] & ~0xfffe0fff) | (data & 0xfffe0fff);
			m_voodoo->set_init_enable(m_pci_3dfx_regs[reg]);
		break;
		case 0x3c:
		case 0x44:
		case 0x48:
		case 0x50:
			m_pci_3dfx_regs[reg] = data;
		break;
		// All remaining registers are read only
	}
}

void savquest_state::bios_f0000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	#if 1
	if (m_mtxc_config_reg[0x59] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_f0000_ram.get() + offset);
	}
	#endif
}

void savquest_state::bios_e0000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	#if 1
	if (m_mtxc_config_reg[0x5e] & 2)        // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e0000_ram.get() + offset);
	}
	#endif
}

void savquest_state::bios_e4000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	#if 1
	if (m_mtxc_config_reg[0x5e] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e4000_ram.get() + offset);
	}
	#endif
}

void savquest_state::bios_e8000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	#if 1
	if (m_mtxc_config_reg[0x5f] & 2)        // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_e8000_ram.get() + offset);
	}
	#endif
}

void savquest_state::bios_ec000_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	#if 1
	if (m_mtxc_config_reg[0x5f] & 0x20)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ec000_ram.get() + offset);
	}
	#endif
}

static const uint8_t m_hasp_cmppass[] = {0xc3, 0xd9, 0xd3, 0xfb, 0x9d, 0x89, 0xb9, 0xa1, 0xb3, 0xc1, 0xf1, 0xcd, 0xdf, 0x9d}; /* 0x9d or 0x9e */
static const uint8_t m_hasp_prodinfo[] = {0x51, 0x4c, 0x52, 0x4d, 0x53, 0x4e, 0x53, 0x4e, 0x53, 0x49, 0x53, 0x48, 0x53, 0x4b, 0x53, 0x4a,
										0x53, 0x43, 0x53, 0x45, 0x52, 0x46, 0x53, 0x43, 0x53, 0x41, 0xac, 0x40, 0x53, 0xbc, 0x53, 0x42,
										0x53, 0x57, 0x53, 0x5d, 0x52, 0x5e, 0x53, 0x5b, 0x53, 0x59, 0xac, 0x58, 0x53, 0xa4
										};

uint8_t savquest_state::parallel_port_r(offs_t offset)
{
	// osd_printf_debug("\n##########\nReading here %x\n##########\n", offset);
	// printf("\n##########\nReading here %x\n##########\n", offset);
	if (offset == 1)
	{
		if ((m_haspstate == HASPSTATE_READ)
			&& (m_hasp_passmode == 3)
			)
		{
			/* passmode 3 is used to retrieve the product(s) information
			   it comes in two parts: header and product
			   the header has this format:
			   offset  range      purpose
			   00      01         header type
			   01      01-05      count of used product slots, must be 2
			   02      01-05      count of unused product slots
			                      this is assumed to be 6-(count of used slots)
			                      but it is not enforced here
			                      however a total of 6 structures will be checked
			   03      01-02      unknown
			   04      01-46      country code
			   05-0f   00         reserved
			   the used product slots have this format:
			   (the unused product slots must be entirely zeroes)
			   00-01   0001-000a  product ID, one must be 6, the other 0a
			   02      0001-0003  unknown but must be 0001
			   04      01-05      HASP plug country ID
			   05      01-02      unknown but must be 01
			   06      05         unknown
			   07-0a   any        unknown, not used
			   0b      ff         unknown
			   0c      ff         unknown
			   0d-0f   00         reserved

			   the read is performed by accessing an array of 16-bit big-endian values
			   and returning one bit at a time into bit 5 of the result
			   the 16-bit value is then XORed with 0x534d and the register index
			*/

			if (m_hasp_prodind <= (sizeof(m_hasp_prodinfo) * 8))
			{
				m_port379 = ((m_hasp_prodinfo[(m_hasp_prodind - 1) >> 3] >> ((8 - m_hasp_prodind) & 7)) & 1) << 5; /* return defined info */
			}
			else
			{
				m_port379 = (((0x534d ^ ((m_hasp_prodind - 1) >> 4)) >> ((16 - m_hasp_prodind) & 15)) & 1) << 5; /* then just alternate between the two key values */
			}

			++m_hasp_prodind;
		}

		return m_port379;
	}

	return 0;
}

void savquest_state::parallel_port_w(offs_t offset, uint8_t data)
{
	// osd_printf_debug("\n##########\nWriting here %x %x\n##########\n", offset, data);
	// printf("\n##########\nWriting here %x %x\n##########\n", offset, data);

	if (!offset)
	{
		uint8_t data8 = (uint8_t) (data & 0xff);

		/* state machine to determine when password is about to be entered */

		switch (m_haspind)
		{
			case 0:
			{
				if (data8 == 0xc6)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				break;
			}

			case 1:
			{
				if (data8 == 0xc7)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				break;
			}

			case 2:
			{
				if (data8 == 0xc6)
				{
					++m_haspind;
					break;
				}

				m_haspind = 0;
				m_haspstate = HASPSTATE_NONE;
				break;
			}

			case 3:
			{
				m_haspind = 0;

				if (data8 == 0x80)
				{
					m_haspstate = HASPSTATE_PASSBEG;
					m_hasp_passind = 0;
					return;
				}

				break;
			}

			default:
			{
			}
		}

		m_port379 = 0x00;

		if (m_haspstate == HASPSTATE_READ)
		{
			/* different passwords cause different values to be returned
			   but there are really only two passwords of interest
			   passmode 2 is used to verify that the dongle is responding correctly
			*/

			if (m_hasp_passmode == 2)
			{
				/* in passmode 2, some values remain unknown: 96, 9a, c4, d4, ec, f8
				   they all return 00, but if that's wrong then there will be failures to start
				*/

				if ((data8 == 0x94)
					|| (data8 == 0x9e)
					|| (data8 == 0xa4)
					|| (data8 == 0xb2)
					|| (data8 == 0xbe)
					|| (data8 == 0xd0)
					)
				{
					return;
				}

				if ((data8 == 0x8a)
					|| (data8 == 0x8e)
					|| (data8 == 0xca)
					|| (data8 == 0xd2)
					|| (data8 == 0xe2)
					|| (data8 == 0xf0)
					|| (data8 == 0xfc)
					)
				{
					/* someone with access to the actual dongle could dump the true values
					   I've never seen it so I just determined the relevant bits instead
					   from the disassembly of the software
					   some of the keys are verified explicitly, the others implicitly
					   I guessed the implicit ones with a bit of trial and error
					*/

					m_port379 = 0x20;
					return;
				}
			}

			switch (data8)
			{
				/* in passmode 0, some values remain unknown: 8a, 8e (inconclusive), 94, 96, 9a, a4, b2, be, c4, d2, d4 (inconclusive), e2, ec, f8, fc
				   this is less of a concern since the contents seem to decrypt correctly
				*/

				case 0x88:
				case 0x94:
				case 0x98:
				case 0x9c:
				case 0x9e:
				case 0xa0:
				case 0xa4:
				case 0xaa:
				case 0xae:
				case 0xb0:
				case 0xb2:
				case 0xbc:
				case 0xbe:
				case 0xc2:
				case 0xc6:
				case 0xc8:
				case 0xce:
				case 0xd0:
				case 0xd6:
				case 0xd8:
				case 0xdc:
				case 0xe0:
				case 0xe6:
				case 0xea:
				case 0xee:
				case 0xf2:
				case 0xf6:
				{
					/* again, just the relevant bits instead of the true values */

					m_port379 = 0x20;
					break;
				}

				default:
				{
				}
			}
		}
		else if (m_haspstate == HASPSTATE_PASSEND)
		{
			if (data8 & 1)
			{
				if ((m_hasp_passmode == 1)
					&& (data8 == 0x9d)
					)
				{
					m_hasp_passmode = 2;
				}

				m_haspstate = HASPSTATE_READ;
			}
			else if (m_hasp_passmode == 1)
			{
				m_hasp_tmppass[m_hasp_passind] = data8;

				if (++m_hasp_passind == sizeof(m_hasp_tmppass))
				{
					if ((m_hasp_tmppass[0] == 0x9c)
						&& (m_hasp_tmppass[1] == 0x9e)
						)
					{
						int i;

						i = 2;
						m_hasp_prodind = 0;

						do
						{
							m_hasp_prodind = (m_hasp_prodind << 1) + ((m_hasp_tmppass[i] >> 6) & 1);
						}
						while ((i += 3) < sizeof(m_hasp_tmppass));

						m_hasp_prodind = (m_hasp_prodind - 0xc08) << 4;

						if (m_hasp_prodind < (0x38 << 4))
						{
							m_hasp_passmode = 3;
						}
					}

					m_haspstate = HASPSTATE_READ;
				}
			}
		}
		else if ((m_haspstate == HASPSTATE_PASSBEG)
				&& (data8 & 1)
			)
		{
			m_hasp_tmppass[m_hasp_passind] = data8;

			if (++m_hasp_passind == sizeof(m_hasp_cmppass))
			{
				m_haspstate = HASPSTATE_PASSEND;
				m_hasp_passind = 0;
				m_hasp_passmode = (int) !memcmp(m_hasp_tmppass, m_hasp_cmppass, sizeof(m_hasp_cmppass));
			}
		}
	}
}

uint8_t savquest_state::smram_r(offs_t offset)
{
	/* TODO: way more complex than this */
	if(m_mtxc_config_reg[0x72] & 0x40)
		return m_smram[offset];
	else
		return m_vga->mem_r(offset);
}

void savquest_state::smram_w(offs_t offset, uint8_t data)
{
	/* TODO: way more complex than this */
	if(m_mtxc_config_reg[0x72] & 0x40)
		m_smram[offset] = data;
	else
		m_vga->mem_w(offset,data);

}

void savquest_state::savquest_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000bffff).rw(FUNC(savquest_state::smram_r), FUNC(savquest_state::smram_w)); //.rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
	map(0x000c0000, 0x000c7fff).rom().region("video_bios", 0);
	map(0x000f0000, 0x000fffff).bankr("bios_f0000").w(FUNC(savquest_state::bios_f0000_ram_w));
	map(0x000e0000, 0x000e3fff).bankr("bios_e0000").w(FUNC(savquest_state::bios_e0000_ram_w));
	map(0x000e4000, 0x000e7fff).bankr("bios_e4000").w(FUNC(savquest_state::bios_e4000_ram_w));
	map(0x000e8000, 0x000ebfff).bankr("bios_e8000").w(FUNC(savquest_state::bios_e8000_ram_w));
	map(0x000ec000, 0x000effff).bankr("bios_ec000").w(FUNC(savquest_state::bios_ec000_ram_w));
	map(0x00100000, 0x07ffffff).ram(); // 128MB RAM
	map(0xe0000000, 0xe0ffffff).m(m_voodoo, FUNC(voodoo_2_device::core_map));
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void savquest_state::savquest_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x0070, 0x007f).rw(m_mc146818, FUNC(ds12885_device::read), FUNC(ds12885_device::write));

	map(0x00e8, 0x00ef).noprw();

	map(0x0170, 0x0177).rw("ide2", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));

	map(0x0378, 0x037b).rw(FUNC(savquest_state::parallel_port_r), FUNC(savquest_state::parallel_port_w));
    map(0x0378, 0x037b).rw(m_lpt, FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write)).umask16(0x00ff);

	map(0x03b0, 0x03bf).rw(m_vga, FUNC(vga_device::port_03b0_r), FUNC(vga_device::port_03b0_w));
	map(0x03c0, 0x03cf).rw(m_vga, FUNC(vga_device::port_03c0_r), FUNC(vga_device::port_03c0_w));
	map(0x03d0, 0x03df).rw(m_vga, FUNC(vga_device::port_03d0_r), FUNC(vga_device::port_03d0_w));

	map(0x0370, 0x0377).rw("ide2", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));

	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
}

#define AT_KEYB_HELPER(bit, text, key1) \
	PORT_BIT( bit, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(text) PORT_CODE(key1)

static INPUT_PORTS_START( savquest )
	PORT_START("pc_keyboard_3")
	AT_KEYB_HELPER( 0x0800, "F1",           KEYCODE_S           ) /* F1                          3B  BB */
INPUT_PORTS_END

void savquest_state::machine_start()
{
	m_bios_f0000_ram = std::make_unique<uint32_t[]>(0x10000/4);
	m_bios_e0000_ram = std::make_unique<uint32_t[]>(0x4000/4);
	m_bios_e4000_ram = std::make_unique<uint32_t[]>(0x4000/4);
	m_bios_e8000_ram = std::make_unique<uint32_t[]>(0x4000/4);
	m_bios_ec000_ram = std::make_unique<uint32_t[]>(0x4000/4);

	intel82439tx_init();
	vid_3dfx_init();

	for (int i = 0; i < 8; i++)
		std::fill(std::begin(m_piix4_config_reg[i]), std::end(m_piix4_config_reg[i]), 0);
}

void savquest_state::machine_reset()
{
	membank("bios_f0000")->set_base(memregion("bios")->base() + 0x30000);
	membank("bios_e0000")->set_base(memregion("bios")->base() + 0x20000);
	membank("bios_e4000")->set_base(memregion("bios")->base() + 0x24000);
	membank("bios_e8000")->set_base(memregion("bios")->base() + 0x28000);
	membank("bios_ec000")->set_base(memregion("bios")->base() + 0x2c000);
	m_haspstate = HASPSTATE_NONE;
}

WRITE_LINE_MEMBER(savquest_state::vblank_assert)
{
}

void savquest_isa16_cards(device_slot_interface &device)
{
	device.option_add("sb16", ISA16_SOUND_BLASTER_16);
}

void savquest_state::savquest(machine_config &config)
{
	PENTIUM2(config, m_maincpu, 450000000); // actually Pentium II 450
	m_maincpu->set_addrmap(AS_PROGRAM, &savquest_state::savquest_map);
	m_maincpu->set_addrmap(AS_IO, &savquest_state::savquest_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);
	DS12885(config.replace(), "rtc");

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 34_MHz_XTAL, 0)); // PII PCI bus clock 33/34/37/40.1 MHz
	pcibus.set_device( 0, FUNC(savquest_state::intel82439tx_pci_r), FUNC(savquest_state::intel82439tx_pci_w));
	pcibus.set_device( 7, FUNC(savquest_state::intel82371ab_pci_r), FUNC(savquest_state::intel82371ab_pci_w));
	pcibus.set_device(13, FUNC(savquest_state::pci_3dfx_r), FUNC(savquest_state::pci_3dfx_w));


	ide_controller_32_device &ide(IDE_CONTROLLER_32(config, "ide").options(ata_devices, "hdd", "cdrom", true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	ide_controller_32_device &ide2(IDE_CONTROLLER_32(config, "ide2").options(ata_devices, "hdd", nullptr, true));
	ide2.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir7_w));

	/* printer */
	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set("pic8259_2", FUNC(pic8259_device::ir4_w));

	/* sound hardware */
	isa16_device &isa(ISA16(config, "isa", 9_MHz_XTAL)); // PII ISA bus clock 8.33~9.35 MHz
	isa.set_memspace("maincpu", AS_PROGRAM);
	isa.set_iospace("maincpu", AS_IO);
	isa.irq5_callback().set("pic8259_2", FUNC(pic8259_device::ir5_w));
	ISA16_SLOT(config, "isa1", 0, "isa", savquest_isa16_cards, "sb16", false);

	/* video hardware */
	pcvideo_s3_vga(config);

	VOODOO_2(config, m_voodoo, voodoo_2_device::NOMINAL_CLOCK);
	m_voodoo->set_fbmem(4);
	m_voodoo->set_tmumem(4, 4); /* this is the 12Mb card */
	m_voodoo->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo->set_screen("screen");
	m_voodoo->set_cpu(m_maincpu);
	m_voodoo->vblank_callback().set(FUNC(savquest_state::vblank_assert));
}

ROM_START( savquest )
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD( "p2xbl_award_451pg.bin", 0x00000, 0x040000, CRC(37d0030e) SHA1(c6773d0e02325116f95c497b9953f59a9ac81317) )

	ROM_REGION32_LE( 0x10000, "video_bios", 0 ) // 1st half is 2.04.14, second half is 2.01.11
	ROM_LOAD( "vgabios.bin",   0x000000, 0x010000, CRC(a81423d6) SHA1(a099af621ce7fbaa55a2d9947d9f07e04f1b5fca) )

	ROM_REGION( 0x080, "rtc", 0 )    /* default NVRAM */
	ROM_LOAD( "savquest_ds12885.bin", 0x0000, 0x080, CRC(e9270019) SHA1(4d900ca317d93c915c80a9053528b741746f08a1) )

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "savquest", 0, SHA1(b7c8901172b66706a7ab5f5c91e6912855153fa9) )
	DISK_REGION( "ide:1:cdrom:image" )
	DISK_REGION( "ide2:0:hdd:image" ) // requires mapping to be detectable
ROM_END

} // Anonymous namespace


GAME(1999, savquest, 0, savquest, savquest, savquest_state, empty_init, ROT0, "Interactive Light", "Savage Quest", MACHINE_NOT_WORKING)
