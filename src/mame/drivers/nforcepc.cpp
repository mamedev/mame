// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli

/*
  Computer based on a motherboard utilizing the nForce chipset (also known as CRUSH11 or CRUSH12)

  Start with the following compoents:
  - An Asus A7N266-C motherboard using:
    - nForce 415-D northbridge
    - nForce MCP-D southbridge (with integrated APU)
    - ITE IT8703F-A SuperIO
	- Asus AS99127F chip
  - An AMD Athlon XP processor
  - An IDE hard disk
  - A floppy disk drive
  - A keyboard
  Later add:
  - A Nvidia NV25 based AGP video card

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "includes/nforcepc.h"

/*
  Pci devices
*/

DEFINE_DEVICE_TYPE(CRUSH11, crush11_host_device, "CRUSH11", "NVIDIA Corporation nForce CPU bridge")

void crush11_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x50, 0x50).rw(FUNC(crush11_host_device::test_r), FUNC(crush11_host_device::test_w));
}

crush11_host_device::crush11_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, CRUSH11, tag, owner, clock)
	, cpu(*this, finder_base::DUMMY_TAG)
{
}

void crush11_host_device::set_ram_size(int ram_size)
{
	ddr_ram_size = ram_size;
}

void crush11_host_device::device_start()
{
	pci_host_device::device_start();
	memory_space = &cpu->space(AS_PROGRAM);
	io_space = &cpu->space(AS_IO);

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffff;
	io_offset = 0;
	status = 0x0010;

	ram.resize(ddr_ram_size / 4);
}

void crush11_host_device::reset_all_mappings()
{
	pci_host_device::reset_all_mappings();
}

void crush11_host_device::device_reset()
{
	pci_host_device::device_reset();
}

void crush11_host_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(0, 0xffff, *static_cast<pci_host_device *>(this), &pci_host_device::io_configuration_access_map);

	memory_space->install_ram(0x00000000, 0x0009ffff, &ram[0x00000000 / 4]);
	memory_space->install_ram(0x00100000, ddr_ram_size - 1, &ram[0x00100000 / 4]);
	uint32_t mask = m_region->bytes() - 1;
	memory_space->install_rom(0x000c0000, 0x000fffff, m_region->base() + (0x000c0000 & mask));
	memory_space->install_rom(0xfffc0000, 0xffffffff, m_region->base() + (0x000c0000 & mask));
}

READ8_MEMBER(crush11_host_device::test_r)
{
	return 0;
}

WRITE8_MEMBER(crush11_host_device::test_w)
{
	logerror("test = %02x\n", data);
}

/*
  Machine state
*/

class nforcepc_state : public driver_device
{
public:
	struct boot_state_info {
		uint8_t val;
		const char *const message;
	};

	static const boot_state_info boot_state_infos_award[];

	void nforcepc(machine_config &config);

	nforcepc_state(const machine_config &mconfig, device_type type, const char *tag);

private:
	void nforce_map(address_map &map);
	void nforce_map_io(address_map &map);
	DECLARE_WRITE8_MEMBER(boot_state_award_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

nforcepc_state::nforcepc_state(const machine_config &mconfig, device_type type, const char *tag) : driver_device(mconfig, type, tag)
{
}

void nforcepc_state::machine_start()
{
}

void nforcepc_state::machine_reset()
{
}

const nforcepc_state::boot_state_info nforcepc_state::boot_state_infos_award[] = {
	{ 0, nullptr }
};

WRITE8_MEMBER(nforcepc_state::boot_state_award_w)
{
	const char *desc = "";
	for(int i=0; boot_state_infos_award[i].message; i++)
		if(boot_state_infos_award[i].val == data) {
			desc = boot_state_infos_award[i].message;
			break;
		}
	logerror("Boot state %02x - %s\n", data, desc);

}

void nforcepc_state::nforce_map(address_map &map)
{
	map.unmap_value_high();
}

void nforcepc_state::nforce_map_io(address_map &map)
{
	map.unmap_value_high();
}

/*
  Machine configuration
*/

MACHINE_CONFIG_START(nforcepc_state::nforcepc)
	MCFG_DEVICE_ADD("maincpu", ATHLONXP, 90000000)
	MCFG_DEVICE_PROGRAM_MAP(nforce_map)
	MCFG_DEVICE_IO_MAP(nforce_map_io)
	MCFG_DEVICE_ADD(":pci", PCI_ROOT, 0)
	MCFG_DEVICE_ADD(":pci:00.0", CRUSH11, 0, "maincpu", 2 * 1024 * 1024)
	/*	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE("pci:07.0:pic8259_master", pic8259_device, inta_cb)
	MCFG_I386_SMIACT(WRITELINE("pci:00.0", i82439hx_host_device, smi_act_w))

	i82371sb_isa_device &isa(I82371SB_ISA(config, ":pci:07.0", 0));
	isa.boot_state_hook().set(FUNC(nforcepc_state::boot_state_phoenix_ver40_rev6_w));
	isa.smi().set_inputline(":maincpu", INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, ":pci:07.1", 0));
	ide.irq_pri().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq15_w));*/
MACHINE_CONFIG_END

ROM_START(nforcepc)
	ROM_REGION32_LE(0x40000, ":pci:00.0", 0) /* PC bios */
	ROM_SYSTEM_BIOS(0, "a7n266c", "a7n266c") // Motherboard dump. Chip: SST49LF020 Package: PLCC32 Label had 3 lines of text: "A7NC3" "1001.D" "GSQ98"
	ROMX_LOAD("a7n266c.bin", 0, 0x40000, CRC(F4F0E4FC), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "a7n266c1001d", "a7n266c1001d") // bios version 1001.D dwonloaded from Asus website
	ROMX_LOAD("a7nc101d.awd", 0, 0x40000, CRC(EAD1147C), ROM_BIOS(1))
ROM_END

static INPUT_PORTS_START(nforcepc)
	//PORT_INCLUDE(at_keyboard)
INPUT_PORTS_END

COMP(2002, nforcepc, 0, 0, nforcepc, nforcepc, nforcepc_state, empty_init, "Nvidia", "Nvidia nForce PC (CRUSH11/12)", MACHINE_IS_SKELETON)
