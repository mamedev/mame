// license:BSD-3-Clause
// copyright-holders:Peter Ferrie
/* Queen */

/*

Produttore  STG
N.revisione
CPU main PCB is a standard EPIA
ROMs    epia BIOS + solid state HD

1x VIA EPIA5000EAG (main PCB) with:
VT8231 South Bridge
VIA Eden Processor
VIA EPIA Companion Chip VT1612A (Audio CODEC)
VIA EPIA Companion Chip VT6103 (Networking)
processor speed is 533MHz <- likely to be a Celeron / Pentium III Socket 370 -AS

 it's a 2002 era PC at least based on the BIOS,
  almost certainly newer than the standard 'PENTIUM' CPU

- In shutms11 HDD will boot to a STBOX / STG splash screen at 800x600 res,
  eventually with blinking cursor then full screen blink (?).
  TODO dig the dump for serial/text debugging, and refs for what video card is used here.

- update by Peter Ferrie:
- split BIOS region into 64kb blocks and implement missing PAM registers
- VIA Apollo VXPro chipset is not compatible with Intel i430.

*/


#include "emu.h"
#include "pcshare.h"

#include "cpu/i386/i386.h"
#include "machine/lpci.h"
#include "machine/pckeybrd.h"
#include "machine/idectrl.h"
//#include "video/pc_vga.h"

namespace {

class queen_state : public pcat_base_state
{
public:
	queen_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
	{
	}

	void queen(machine_config &config);

private:
	std::unique_ptr<uint32_t[]> m_bios_ram;
	std::unique_ptr<uint32_t[]> m_bios_ext_ram;
	uint8_t m_mtxc_config_reg[256]{};
	uint8_t m_piix4_config_reg[4][256]{};

	void bios_ext_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void intel82439tx_init();
	void queen_io(address_map &map) ATTR_COLD;
	void queen_map(address_map &map) ATTR_COLD;

	uint8_t mtxc_config_r(int function, int reg);
	void mtxc_config_w(int function, int reg, uint8_t data);
	uint32_t intel82439tx_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint8_t piix4_config_r(int function, int reg);
	void piix4_config_w(int function, int reg, uint8_t data);
	uint32_t intel82371ab_pci_r(int function, int reg, uint32_t mem_mask);
	void intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
};


// Intel 82439TX System Controller (MTXC)

uint8_t queen_state::mtxc_config_r(int function, int reg)
{
//  osd_printf_debug("MTXC: read %d, %02X\n", function, reg);

	return m_mtxc_config_reg[reg];
}

void queen_state::mtxc_config_w(int function, int reg, uint8_t data)
{
	printf("MTXC: write %d, %02X, %02X\n",  function, reg, data);

	/*
	memory banking with North Bridge:
	0x63 (PAM)  xx-- ---- BIOS area 0xf0000-0xfffff
	            --xx ---- BIOS extension 0xe0000 - 0xeffff
	            ---- xx-- ISA add-on BIOS 0xd0000 - 0xdffff
	            ---- --xx ISA add-on BIOS 0xc0000 - 0xcffff

	10 -> 1 = Write Enable, 0 = Read Enable
	*/

	if (reg == 0x63)
	{
		if (data & 0x20)        // enable RAM access to region 0xf0000 - 0xfffff
			membank("bios_bank")->set_base(m_bios_ram.get());
		else                    // disable RAM access (reads go to BIOS ROM)
			membank("bios_bank")->set_base(memregion("bios")->base() + 0x30000);
		if (data & 0x80)        // enable RAM access to region 0xe0000 - 0xeffff
			membank("bios_ext")->set_base(m_bios_ext_ram.get());
		else
			membank("bios_ext")->set_base(memregion("bios")->base() + 0x20000);
	}

	m_mtxc_config_reg[reg] = data;
}

void queen_state::intel82439tx_init()
{
	m_mtxc_config_reg[0x60] = 0x02;
	m_mtxc_config_reg[0x61] = 0x02;
	m_mtxc_config_reg[0x62] = 0x02;
	m_mtxc_config_reg[0x63] = 0x02;
	m_mtxc_config_reg[0x64] = 0x02;
	m_mtxc_config_reg[0x65] = 0x02;
}

uint32_t queen_state::intel82439tx_pci_r(int function, int reg, uint32_t mem_mask)
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

void queen_state::intel82439tx_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
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

uint8_t queen_state::piix4_config_r(int function, int reg)
{
//  osd_printf_debug("PIIX4: read %d, %02X\n", function, reg);
	if ((function < 4) && (reg < 256))
		return m_piix4_config_reg[function][reg];
	else
		return 0; // BIOS performs a brute-force scan for devices
}

void queen_state::piix4_config_w(int function, int reg, uint8_t data)
{
//  osd_printf_debug("%s:PIIX4: write %d, %02X, %02X\n", machine().describe_context(), function, reg, data);
	if ((function < 4) && (reg < 256))
		m_piix4_config_reg[function][reg] = data;
}

uint32_t queen_state::intel82371ab_pci_r(int function, int reg, uint32_t mem_mask)
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

void queen_state::intel82371ab_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
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


void queen_state::bios_ext_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_mtxc_config_reg[0x63] & 0x40)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ext_ram.get() + offset);
	}
}


void queen_state::bios_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_mtxc_config_reg[0x63] & 0x10)     // write to RAM if this region is write-enabled
	{
		COMBINE_DATA(m_bios_ram.get() + offset);
	}
}

void queen_state::queen_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
//  map(0x000a0000, 0x000bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
	map(0x000e0000, 0x000effff).bankr("bios_ext").w(FUNC(queen_state::bios_ext_ram_w));
	map(0x000f0000, 0x000fffff).bankr("bios_bank").w(FUNC(queen_state::bios_ram_w));
	map(0x00100000, 0x01ffffff).ram();
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);    /* System BIOS */
}

void queen_state::queen_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x00e8, 0x00ef).noprw();

	map(0x0170, 0x0177).rw("ide2", FUNC(ide_controller_32_device::cs0_r), FUNC(ide_controller_32_device::cs0_w));
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_device::cs0_r), FUNC(ide_controller_device::cs0_w));
	map(0x0370, 0x0377).rw("ide2", FUNC(ide_controller_32_device::cs1_r), FUNC(ide_controller_32_device::cs1_w));
//  map(0x03b0, 0x03df).m("vga", FUNC(vga_device::io_map));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_device::cs1_r), FUNC(ide_controller_device::cs1_w));

	map(0x0cf8, 0x0cff).rw("pcibus", FUNC(pci_bus_legacy_device::read), FUNC(pci_bus_legacy_device::write));
}

void queen_state::machine_start()
{
	m_bios_ram = std::make_unique<uint32_t[]>(0x10000/4);
	m_bios_ext_ram = std::make_unique<uint32_t[]>(0x10000/4);

	intel82439tx_init();
}

void queen_state::machine_reset()
{
	membank("bios_bank")->set_base(memregion("bios")->base() + 0x30000);
	membank("bios_ext")->set_base(memregion("bios")->base() + 0x20000);
}

void queen_state::queen(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 533000000/16); // Celeron or Pentium 3, 533 Mhz
	m_maincpu->set_addrmap(AS_PROGRAM, &queen_state::queen_map);
	m_maincpu->set_addrmap(AS_IO, &queen_state::queen_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device(0, FUNC(queen_state::intel82439tx_pci_r), FUNC(queen_state::intel82439tx_pci_w));
	pcibus.set_device(7, FUNC(queen_state::intel82371ab_pci_r), FUNC(queen_state::intel82371ab_pci_w));

	ide_controller_device &ide(IDE_CONTROLLER(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	ide_controller_32_device &ide2(IDE_CONTROLLER_32(config, "ide2").options(ata_devices, nullptr, nullptr, true));
	ide2.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir7_w));
}


ROM_START( queen )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "bios-original.bin", 0x00000, 0x40000, CRC(feb542d4) SHA1(3cc5d8aeb0e3b7d9ed33248a4f3dc507d29debd9) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "pqiidediskonmodule", 0,SHA1(a56efcc711b1c5a2e63160b3088001a8c4fb56c2) )
ROM_END

} // anonymous namespace


GAME( 2002?, queen,  0,    queen, 0, queen_state, empty_init, ROT0, "STG", "Queen?", MACHINE_IS_SKELETON )
