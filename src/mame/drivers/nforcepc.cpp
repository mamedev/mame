// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli

/*
  Computer based on a motherboard utilizing the nForce chipset (also known as CRUSH11 or CRUSH12)

  Start with the following components:
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
#include "includes/xbox_pci.h"
#include "includes/nforcepc.h"

/*
  Pci devices
*/

// NVIDIA Corporation nForce CPU bridge

DEFINE_DEVICE_TYPE(CRUSH11, crush11_host_device, "crush11", "NVIDIA Corporation nForce CPU bridge")

void crush11_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0xf0, 0xf0).rw(FUNC(crush11_host_device::test_r), FUNC(crush11_host_device::test_w));
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
	return 4;
}

WRITE8_MEMBER(crush11_host_device::test_w)
{
	logerror("test = %02x\n", data);
}

// device connected to SMBus

DEFINE_DEVICE_TYPE(SMBUS_LOGGER, smbus_logger_device, "smbus_logger", "SMBUS LOGGER")

smbus_logger_device::smbus_logger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SMBUS_LOGGER, tag, owner, clock)
{
}

int smbus_logger_device::execute_command(int command, int rw, int data)
{
	if (rw == 1)
	{
		logerror("smbus read from %02x R %02x\n", command, buffer[command]);
		return buffer[command];
	}
	buffer[command] = (uint8_t)data;
	logerror("smbus write to %02x W %02x\n", command, data);
	return 0;
}

void smbus_logger_device::device_start()
{
	memset(buffer, 0, sizeof(buffer));
}

void smbus_logger_device::device_reset()
{
}

// Asus AS99127F chip

DEFINE_DEVICE_TYPE(AS99127F, as99127f_device, "as99127f", "Asus AS99127F")

as99127f_device::as99127f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AS99127F, tag, owner, clock)
{
}

int as99127f_device::execute_command(int command, int rw, int data)
{
	if (rw == 1)
	{
		logerror("smbus read from %02x R %02x\n", command, buffer[command]);
		return buffer[command];
	}
	buffer[command] = (uint8_t)data;
	logerror("smbus write to %02x W %02x\n", command, data);
	return 0;
}

void as99127f_device::device_start()
{
	memset(buffer, 0, sizeof(buffer));
}

DEFINE_DEVICE_TYPE(AS99127F_SENSOR2, as99127f_sensor2_device, "as99127f_sensor2", "Asus AS99127F temperature sensor 2")

as99127f_sensor2_device::as99127f_sensor2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AS99127F_SENSOR2, tag, owner, clock)
{
}

int as99127f_sensor2_device::execute_command(int command, int rw, int data)
{
	if (rw == 1)
	{
		logerror("smbus read from %02x R %02x\n", command, buffer[command]);
		return buffer[command];
	}
	buffer[command] = (uint8_t)data;
	logerror("smbus write to %02x W %02x\n", command, data);
	return 0;
}

void as99127f_sensor2_device::device_start()
{
	memset(buffer, 0, sizeof(buffer));
}

DEFINE_DEVICE_TYPE(AS99127F_SENSOR3, as99127f_sensor3_device, "as99127f_sensor3", "Asus AS99127F temperature sensor 3")

as99127f_sensor3_device::as99127f_sensor3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AS99127F_SENSOR3, tag, owner, clock)
{
}

int as99127f_sensor3_device::execute_command(int command, int rw, int data)
{
	if (rw == 1)
	{
		logerror("smbus read from %02x R %02x\n", command, buffer[command]);
		return buffer[command];
	}
	buffer[command] = (uint8_t)data;
	logerror("smbus write to %02x W %02x\n", command, data);
	return 0;
}

void as99127f_sensor3_device::device_start()
{
	memset(buffer, 0, sizeof(buffer));
}

/*
  Machine state
*/

class nforcepc_state : public driver_device
{
public:
	struct boot_state_info
	{
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
	IRQ_CALLBACK_MEMBER(irq_callback);
	DECLARE_WRITE_LINE_MEMBER(maincpu_interrupt);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<mcpx_isalpc_device> isalpc;
	required_device<as99127f_device> m_as99127f;
};

nforcepc_state::nforcepc_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	isalpc(*this, ":pci:01.0"),
	m_as99127f(*this, ":pci:01.1:2d")
{
}

void nforcepc_state::machine_start()
{
	m_as99127f->get_buffer()[0x4f] = 0x12;
}

void nforcepc_state::machine_reset()
{
}

const nforcepc_state::boot_state_info nforcepc_state::boot_state_infos_award[] = {
	{ 0xC0, "Turn off chipset cache" },
	{ 0, nullptr }
};

WRITE8_MEMBER(nforcepc_state::boot_state_award_w)
{
	const char *desc = "";
	for (int i = 0; boot_state_infos_award[i].message; i++)
		if (boot_state_infos_award[i].val == data)
		{
			desc = boot_state_infos_award[i].message;
			break;
		}
	logerror("Boot state %02x - %s\n", data, desc);
}

IRQ_CALLBACK_MEMBER(nforcepc_state::irq_callback)
{
	return isalpc->acknowledge();
}

WRITE_LINE_MEMBER(nforcepc_state::maincpu_interrupt)
{
	m_maincpu->set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
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

void nforcepc_state::nforcepc(machine_config &config)
{
	athlonxp_device &maincpu(ATHLONXP(config, "maincpu", 90000000));
	maincpu.set_addrmap(AS_PROGRAM, &nforcepc_state::nforce_map);
	maincpu.set_addrmap(AS_IO, &nforcepc_state::nforce_map_io);
	maincpu.set_irq_acknowledge_callback(FUNC(nforcepc_state::irq_callback));

	PCI_ROOT(config, ":pci", 0);
	CRUSH11(config, ":pci:00.0", 0, "maincpu", 2 * 1024 * 1024); /* 10de:01a4 NVIDIA Corporation nForce CPU bridge
	10de:01ac NVIDIA Corporation nForce 220/420 Memory Controller
	10de:01ad NVIDIA Corporation nForce 220/420 Memory Controller
	10de:01ab NVIDIA Corporation nForce 420 Memory Controller (DDR)*/
	mcpx_isalpc_device &isa(MCPX_ISALPC(config, ":pci:01.0", 0, 0x10430c11)); // 10de:01b2 NVIDIA Corporation nForce ISA Bridge (LPC bus)
	isa.boot_state_hook().set(FUNC(nforcepc_state::boot_state_award_w));
	isa.interrupt_output().set(FUNC(nforcepc_state::maincpu_interrupt));
	MCPX_SMBUS(config, ":pci:01.1", 0); // 10de:01b4 NVIDIA Corporation nForce PCI System Management (SMBus)
	SMBUS_LOGGER(config, ":pci:01.1:08", 0);
	AS99127F(config, ":pci:01.1:2d", 0);
	AS99127F_SENSOR2(config, ":pci:01.1:48", 0);
	AS99127F_SENSOR3(config, ":pci:01.1:49", 0);
	/*10de:01c2 NVIDIA Corporation nForce USB Controller
	10de:01c2 NVIDIA Corporation nForce USB Controller
	10de:01b0 NVIDIA Corporation nForce Audio Processing Unit
	10de:01b1 NVIDIA Corporation nForce AC'97 Audio Controller
	10de:01b8 NVIDIA Corporation nForce PCI-to-PCI bridge
	10de:01bc NVIDIA Corporation nForce IDE
	10de:01b7 NVIDIA Corporation nForce AGP to PCI Bridge
	*/
	/* maincpu.smiact().set("pci:00.0", FUNC(i82439hx_host_device::smi_act_w));

	i82371sb_ide_device &ide(I82371SB_IDE(config, ":pci:07.1", 0));
	ide.irq_pri().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq15_w));*/
}

ROM_START(nforcepc)
	ROM_REGION32_LE(0x40000, ":pci:00.0", 0) /* PC bios */
	ROM_SYSTEM_BIOS(0, "a7n266c", "a7n266c") // Motherboard dump. Chip: SST49LF020 Package: PLCC32 Label had 3 lines of text: "A7NC3" "1001.D" "GSQ98"
	ROMX_LOAD("a7n266c.bin", 0, 0x40000, CRC(f4f0e4fc) SHA1(87f11545db178914623e41fb51e328da479a2efc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "a7n266c1001d", "a7n266c1001d") // bios version 1001.D downloaded from Asus website
	ROMX_LOAD("a7nc101d.awd", 0, 0x40000, CRC(ead1147c) SHA1(27227df98e0c5fb9fecdb4bb6ef72df19766c330), ROM_BIOS(1))
ROM_END

static INPUT_PORTS_START(nforcepc)
	//PORT_INCLUDE(at_keyboard)
INPUT_PORTS_END

COMP(2002, nforcepc, 0, 0, nforcepc, nforcepc, nforcepc_state, empty_init, "Nvidia", "Nvidia nForce PC (CRUSH11/12)", MACHINE_IS_SKELETON)
