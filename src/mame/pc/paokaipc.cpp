// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

    Go Go Strike (c) 2007? Paokai

    Originally in fruitpc.cpp, split because definitely doesn't belong

    Some kind of x86 pc-like hardware, exact CPU type etc. unknown hardware is by Paokai,
    motherboard has logos, large chip with logo too, http://www.paokai.com.tw/

    CF card has a Linux partition, partially bootable with shutms11 driver.
    - starts with a "LILO boot", loads a proprietary driver TWDrv.o (?) then eventually
      crashes not finding sound modules;
    - Shows being a "gcc 3.2.2 (Red Hat Linux 3.2.2-5)" distro.
      Notice that latter seems mislabeled, and RedHat is actually version 9
      http://rpm.pbone.net/info_idpl_19558085_distro_redhat9_com_gcc-3.2.2-5.i386.rpm.html

**************************************************************************************************/

#include "emu.h"

#include "pcshare.h"

#include "bus/isa/isa.h"
#include "bus/isa/sblaster.h"
#include "cpu/i386/i386.h"
#include "machine/idectrl.h"
#include "machine/pci.h"
#include "machine/pckeybrd.h"

namespace {

class paokaipc_state : public pcat_base_state
{
public:
	paokaipc_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
		, m_pciroot(*this, "pci")
		, m_isabus(*this, "isa")
	{ }

	void paokaipc(machine_config &config);

private:
	required_device<pci_root_device> m_pciroot;
	required_device<isa8_device> m_isabus;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

void paokaipc_state::main_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
//  map(0x000a0000, 0x000bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w)); // VGA VRAM
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0x00100000, 0x008fffff).ram();
	map(0x02000000, 0x28ffffff).noprw();
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0x00000);    /* System BIOS */
}

void paokaipc_state::main_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x01f0, 0x01f7).rw("ide", FUNC(ide_controller_device::cs0_r), FUNC(ide_controller_device::cs0_w));
//  map(0x03b0, 0x03df).m("vga", FUNC(vga_device::io_map));
	map(0x03f0, 0x03f7).rw("ide", FUNC(ide_controller_device::cs1_r), FUNC(ide_controller_device::cs1_w));
//  map(0x0880, 0x0880) extensively accessed at POST, hangs if returns wrong values
//  map(0x0cf8, 0x0cff).rw(m_pcibus, FUNC(pci_bus_device::read), FUNC(pci_bus_device::write));
}

static INPUT_PORTS_START( gogostrk )
INPUT_PORTS_END

void paokaipc_state::paokaipc(machine_config &config)
{
	// TODO: everything inherited from fruitpc.cpp, needs proper identification of motherboard.
	PENTIUM(config, m_maincpu, 66000000); // unknown CPU, at least Pentium according to logs
	m_maincpu->set_addrmap(AS_PROGRAM, &paokaipc_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &paokaipc_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	pcat_common(config);

	ide_controller_device &ide(IDE_CONTROLLER(config, "ide").options(ata_devices, "hdd", nullptr, true));
	ide.irq_handler().set("pic8259_2", FUNC(pic8259_device::ir6_w));

	PCI_ROOT(config, m_pciroot, 0);
	// ...

	ISA8(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->irq2_callback().set("pic8259_2", FUNC(pic8259_device::ir2_w));
	m_isabus->irq3_callback().set("pic8259_1", FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set("pic8259_1", FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set("pic8259_1", FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set("pic8259_1", FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set("pic8259_1", FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set("dma8237_1", FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set("dma8237_1", FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set("dma8237_1", FUNC(am9517a_device::dreq3_w));

	// FIXME: determine ISA bus clock
//  isa8_slot_device &isa1(ISA8_SLOT(config, "isa1", 0, "isa", fruitpc_isa8_cards, "sb15", true));
//  isa1.set_option_device_input_defaults("sb15", DEVICE_INPUT_DEFAULTS_NAME(fruitpc_sb_def));
//  isa1.set_option_machine_config("sb15", fruitpc_sb_conf);
}

ROM_START( gogostrk )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "39sf020a.rom1", 0x000000, 0x040000, CRC(236d4d95) SHA1(50579acddc93c05d5f8e17ad3669a29d2dc49965) )

	DISK_REGION( "ide:0:hdd" )    // 128 MB CF Card
	DISK_IMAGE( "ggs-5-2-07", 0,SHA1(f214fd39ec8ac02f008823f4b179ea6c6835e1b8) )
ROM_END

} // anonymous namespace

GAME( 2007?, gogostrk, 0, paokaipc, gogostrk, paokaipc_state, empty_init, ROT0, "American Alpha / Paokai", "Go Go Strike", MACHINE_IS_SKELETON ) // motherboard is dated 2006, if the CF card string is a date it's 2007
