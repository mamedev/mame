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
  - A ddr dimm memory module
  Later add:
  - A Nvidia NV25 based AGP video card

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pci-ide.h"
#include "machine/intelfsh.h"
#include "includes/xbox_pci.h"
#include "includes/nforcepc.h"

#if 1
// for now let's use this as the contents of the spd chip in the ddr dimm memory module
static const uint8_t test_spd_data[] = {
	0x80,0x08,0x07,0x0D,0x0A,0x01,0x40,0x00,0x04,0x75,0x75,0x00,0x82,0x10,0x00,0x01,
	0x0E,0x04,0x0C,0x01,0x02,0x20,0xC0,0xA0,0x75,0x00,0x00,0x50,0x3C,0x50,0x2D,0x40,
	0x90,0x90,0x50,0x50,0x00,0x00,0x00,0x00,0x00,0x41,0x4B,0x30,0x32,0x75,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEA,
	0xAD,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x59,0x4D,0x44,0x35,0x33,0x32,
	0x4D,0x36,0x34,0x36,0x43,0x36,0x2D,0x48,0x20,0x20,0x20
};
#else
// From a HYS72D64320HU-5-C (512MB PC3200 (400mHz) DDR CL3 ECC)
static const uint8_t test_spd_data[] = {
	0x80,0x08,0x07,0x0d,0x0a,0x02,0x48,0x00,0x04,0x50,0x50,0x02,0x82,0x08,0x08,0x01,
	0x0e,0x04,0x1c,0x01,0x02,0x20,0xc1,0x60,0x50,0x75,0x50,0x3c,0x28,0x3c,0x28,0x40,
	0x60,0x60,0x40,0x40,0x00,0x00,0x00,0x00,0x00,0x37,0x41,0x28,0x28,0x50,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,
	0xc1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x45,0x37,0x32,0x44,0x36,0x34,0x33,0x32,
	0x30,0x48,0x55,0x35,0x43,0x20,0x20,0x20,0x20,0x20,0x20,0x02,0x4c,0x05,0x44,0x01,
	0x08,0x56,0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
#endif

/*
  Pci devices
*/

// NVIDIA Corporation nForce CPU bridge

DEFINE_DEVICE_TYPE(CRUSH11, crush11_host_device, "crush11", "NVIDIA Corporation nForce CPU bridge")

void crush11_host_device::config_map(address_map &map)
{
	pci_host_device::config_map(map);
	map(0x84, 0x87).rw(FUNC(crush11_host_device::ram_size_r), FUNC(crush11_host_device::ram_size_w));
	map(0xf0, 0xf0).rw(FUNC(crush11_host_device::unknown_r), FUNC(crush11_host_device::unknown_w));
}

crush11_host_device::crush11_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_host_device(mconfig, CRUSH11, tag, owner, clock)
	, cpu(*this, finder_base::DUMMY_TAG)
	, biosrom(*this, finder_base::DUMMY_TAG)
	, ram_size(0)
{
}

void crush11_host_device::device_start()
{
	pci_host_device::device_start();
	memory_space = &cpu->space(AS_DATA);
	io_space = &cpu->space(AS_IO);

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffff;
	io_offset = 0;
	status = 0x0010;
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
	memory_space->install_device(0, 0xffffffff, *this, &crush11_host_device::bios_map);
}

void crush11_host_device::bios_map(address_map &map)
{
	map(0x000c0000, 0x000fffff).rw(biosrom, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0xfffc0000, 0xffffffff).rw(biosrom, FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
}

READ8_MEMBER(crush11_host_device::unknown_r)
{
	return 4;
}

WRITE8_MEMBER(crush11_host_device::unknown_w)
{
	logerror("test = %02x\n", data);
}

READ32_MEMBER(crush11_host_device::ram_size_r)
{
	return ram_size * 1024 * 1024 - 1;
}

WRITE32_MEMBER(crush11_host_device::ram_size_w)
{
	logerror("trying to set size = %d\n", data);
}

// For ddr ram

DEFINE_DEVICE_TYPE(CRUSH11_MEMORY, crush11_memory_device, "crush11_memory", "nForce memory")

void crush11_memory_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	/*
	bit 31 of a0,a4,a8,ac,b0 and b4 must be 0
	bit 0 of c4 and c8 must be 0
	*/
	map(0xa0, 0xa3).nopr();
	map(0xa4, 0xa7).nopr();
	map(0xa8, 0xab).nopr();
	map(0xac, 0xaf).nopr();
	map(0xb0, 0xb3).nopr();
	map(0xb4, 0xb7).nopr();
	map(0xc4, 0xc7).nopr();
	map(0xc8, 0xcb).nopr();
}

crush11_memory_device::crush11_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int ram_size)
	: crush11_memory_device(mconfig, tag, owner, clock)
{
	set_ram_size(ram_size);
}

crush11_memory_device::crush11_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, CRUSH11_MEMORY, tag, owner, clock)
{
	set_ids(0x10de01ac, 0, 0, 0);
}

void crush11_memory_device::device_start()
{
	device_t *r = owner()->subdevice("00.0");

	pci_device::device_start();
	ram.resize(ddr_ram_size * 1024 * 1024 / 4);
	host = dynamic_cast<crush11_host_device *>(r);
	ram_space = host->get_cpu_space(AS_PROGRAM);
}

void crush11_memory_device::device_reset()
{
	pci_device::device_reset();
	host->set_ram_size(ddr_ram_size);
}

void crush11_memory_device::set_ram_size(int ram_size)
{
	ddr_ram_size = ram_size;
	if (ddr_ram_size < 16)
		ddr_ram_size = 16;
}

void crush11_memory_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	ram_space->install_ram(0x00000000, ddr_ram_size * 1024 * 1024 - 1, &ram[0]);
}

/*
  Ddevices connected to SMBus
*/

// access logger

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

// read-only data

DEFINE_DEVICE_TYPE(SMBUS_ROM, smbus_rom_device, "smbus_rom", "SMBUS ROM")

smbus_rom_device::smbus_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMBUS_ROM, tag, owner, clock)
{
}

smbus_rom_device::smbus_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const uint8_t *data, int size) :
	smbus_rom_device(mconfig, tag, owner, clock)
{
	buffer = data;
	buffer_size = size;
}

int smbus_rom_device::execute_command(int command, int rw, int data)
{
	if ((rw == 1) && (command < buffer_size) && (buffer != nullptr))
	{
		logerror("smbus rom read from %02x %02x\n", command, buffer[command]);
		return buffer[command];
	}
	return 0;
}

void smbus_rom_device::device_start()
{
}

void smbus_rom_device::device_reset()
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

// ITE IT8703F-A SuperIO

DEFINE_DEVICE_TYPE(IT8703F, it8703f_device, "it8703f_device", "ITE IT8703F-A SuperIO")

it8703f_device::it8703f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IT8703F, tag, owner, clock)
	, mode(OperatingMode::Run)
	, config_key_step(0)
	, config_index(0)
	, logical_device(0)
	, pin_reset_callback(*this)
	, pin_gatea20_callback(*this)
	, m_kbdc(*this, "pc_kbdc")
{
	memset(global_configuration_registers, 0, sizeof(global_configuration_registers));
	memset(configuration_registers, 0, sizeof(configuration_registers));
	for (int n = 0; n < 13; n++)
		enabled_logical[n] = false;
}

void it8703f_device::device_start()
{
	pin_reset_callback.resolve_safe();
	pin_gatea20_callback.resolve_safe();
}

void it8703f_device::internal_memory_map(address_map &map)
{
}

void it8703f_device::internal_io_map(address_map &map)
{
	map(0x002e, 0x002f).rw(FUNC(it8703f_device::read_it8703f), FUNC(it8703f_device::write_it8703f));
}

uint16_t it8703f_device::get_base_address(int logical, int index)
{
	int position = index * 2 + 0x60;

	return ((uint16_t)configuration_registers[logical][position] << 8) + (uint16_t)configuration_registers[logical][position + 1];
}

void it8703f_device::device_add_mconfig(machine_config &config)
{
	// keyboard
	KBDC8042(config, m_kbdc);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_PS2);
	m_kbdc->input_buffer_full_callback().set(FUNC(it8703f_device::irq_keyboard_w));
	m_kbdc->system_reset_callback().set(FUNC(it8703f_device::kbdp20_gp20_reset_w));
	m_kbdc->gate_a20_callback().set(FUNC(it8703f_device::kbdp21_gp25_gatea20_w));
}

READ8_MEMBER(it8703f_device::read_it8703f)
{
	if (offset == 0)
	{
		if (mode == OperatingMode::Run)
			return 0;
		return config_index;
	}
	else if (offset == 1)
	{
		if (mode == OperatingMode::Run)
			return 0;
		if (config_index < 0x30)
			return read_global_configuration_register(config_index);
		else
			return read_logical_configuration_register(config_index);
	}
	else
		return 0;
}

WRITE8_MEMBER(it8703f_device::write_it8703f)
{
	uint8_t byt;

	if (offset == 0)
	{
		byt = data & 0xff;
		if (mode == OperatingMode::Run)
		{
			if (byt != 0x87)
				return;
			config_key_step++;
			if (config_key_step > 1)
			{
				config_key_step = 0;
				mode = OperatingMode::Configuration;
			}
		}
		else
		{
			if (byt == 0xaa)
			{
				mode = OperatingMode::Run;
				return;
			}
			config_index = byt;
		}
	}
	else if (offset == 1)
	{
		if (mode == OperatingMode::Run)
			return;
		byt = data & 0xff;
		if (config_index < 0x30)
			write_global_configuration_register(config_index, byt);
		else
			write_logical_configuration_register(config_index, byt);
	}
	else
		return;
}

void it8703f_device::write_global_configuration_register(int index, int data)
{
	global_configuration_registers[index] = data;
	switch (index)
	{
	case 7:
		logical_device = data;
		logerror("Selected logical device %d\n", data);
		break;
	}
	logerror("Write global configuration register %02X = %02X\n", index, data);
}

void it8703f_device::write_logical_configuration_register(int index, int data)
{
	configuration_registers[logical_device][index] = data;
	switch (logical_device)
	{
	case LogicalDevice::Keyboard:
		if (index == 0x30)
		{
			if (data & 1)
			{
				if (enabled_logical[LogicalDevice::Keyboard] == false)
					map_keyboard_addresses();
				enabled_logical[LogicalDevice::Keyboard] = true;
				logerror("Enabled Keyboard\n");
			}
			else
			{
				if (enabled_logical[LogicalDevice::Keyboard] == true)
					unmap_keyboard_addresses();
				enabled_logical[LogicalDevice::Keyboard] = false;
			}
		}
		break;
	}
}

uint16_t it8703f_device::read_global_configuration_register(int index)
{
	uint16_t ret = 0;

	ret = global_configuration_registers[index];
	switch (index)
	{
	case 7:
		ret = logical_device;
		break;
	}
	logerror("Read global configuration register %02X = %02X\n", index, ret);
	return ret;
}

uint16_t it8703f_device::read_logical_configuration_register(int index)
{
	return configuration_registers[logical_device][index];
}

WRITE_LINE_MEMBER(it8703f_device::irq_keyboard_w)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	lpchost->set_virtual_line(configuration_registers[LogicalDevice::Keyboard][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(it8703f_device::kbdp21_gp25_gatea20_w)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	pin_gatea20_callback(state);
}

WRITE_LINE_MEMBER(it8703f_device::kbdp20_gp20_reset_w)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	pin_reset_callback(state);
}

void it8703f_device::map_keyboard(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(it8703f_device::at_keybc_r), FUNC(it8703f_device::at_keybc_w));
	map(0x4, 0x4).rw(FUNC(it8703f_device::keybc_status_r), FUNC(it8703f_device::keybc_command_w));
}

void it8703f_device::unmap_keyboard(address_map &map)
{
	map(0x0, 0x0).noprw();
	map(0x4, 0x4).noprw();
}

READ8_MEMBER(it8703f_device::at_keybc_r)
{
	switch (offset) //m_kbdc
	{
	case 0:
		return m_kbdc->data_r(space, 0);
	}

	return 0xff;
}

WRITE8_MEMBER(it8703f_device::at_keybc_w)
{
	switch (offset)
	{
	case 0:
		m_kbdc->data_w(space, 0, data);
	}
}

READ8_MEMBER(it8703f_device::keybc_status_r)
{
	return m_kbdc->data_r(space, 4);
}

WRITE8_MEMBER(it8703f_device::keybc_command_w)
{
	m_kbdc->data_w(space, 4, data);
}


void it8703f_device::map_keyboard_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::Keyboard, 0);

	iospace->install_device(base, base + 7, *this, &it8703f_device::map_keyboard);
}

void it8703f_device::unmap_keyboard_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::Keyboard, 0);

	iospace->install_device(base, base + 7, *this, &it8703f_device::unmap_keyboard);
}

void it8703f_device::map_extra(address_space *memory_space, address_space *io_space)
{
	memspace = memory_space;
	iospace = io_space;
	io_space->install_device(0, 0xffff, *this, &it8703f_device::internal_io_map);
	if (enabled_logical[LogicalDevice::Keyboard] == true)
		map_keyboard_addresses();
}

void it8703f_device::set_host(int index, lpcbus_host_interface *host)
{
	lpchost = host;
	lpcindex = index;
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
	m_as99127f(*this, ":pci:01.1:12d")
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
	{ 0xC0, "First basic initialization" },
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
	//maincpu.smiact().set("pci:01.0", FUNC(i82439hx_host_device::smi_act_w));

	PCI_ROOT(config, ":pci", 0);
	CRUSH11(config, ":pci:00.0", 0, "maincpu", "bios"); // 10de:01a4 NVIDIA Corporation nForce CPU bridge
	CRUSH11_MEMORY(config, ":pci:00.1", 0, 2); /* 10de:01ac NVIDIA Corporation nForce 220/420 Memory Controller
	10de:01ad NVIDIA Corporation nForce 220/420 Memory Controller
	10de:01ab NVIDIA Corporation nForce 420 Memory Controller (DDR)*/
	mcpx_isalpc_device &isa(MCPX_ISALPC(config, ":pci:01.0", 0, 0x10430c11)); // 10de:01b2 NVIDIA Corporation nForce ISA Bridge (LPC bus)
	isa.smi().set_inputline(":maincpu", INPUT_LINE_SMI);
	isa.boot_state_hook().set(FUNC(nforcepc_state::boot_state_award_w));
	isa.interrupt_output().set(FUNC(nforcepc_state::maincpu_interrupt));
	it8703f_device &ite(IT8703F(config, ":pci:01.0:0", 0));
	ite.pin_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	ite.pin_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	MCPX_SMBUS(config, ":pci:01.1", 0); // 10de:01b4 NVIDIA Corporation nForce PCI System Management (SMBus)
	SMBUS_ROM(config, ":pci:01.1:050", 0, test_spd_data, sizeof(test_spd_data)); // these 3 are on smbus number 0
	SMBUS_LOGGER(config, ":pci:01.1:051", 0);
	SMBUS_LOGGER(config, ":pci:01.1:052", 0);
	SMBUS_LOGGER(config, ":pci:01.1:108", 0); // these 4 are on smbus number 1
	AS99127F(config, ":pci:01.1:12d", 0);
	AS99127F_SENSOR2(config, ":pci:01.1:148", 0);
	AS99127F_SENSOR3(config, ":pci:01.1:149", 0);
	SST_49LF020(config, "bios", 0);
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
	ROM_REGION32_LE(0x40000, "bios", 0) /* PC bios */
	ROM_SYSTEM_BIOS(0, "a7n266c", "a7n266c") // Motherboard dump. Chip: SST49LF020 Package: PLCC32 Label had 3 lines of text: "A7NC3" "1001.D" "GSQ98"
	ROMX_LOAD("a7n266c.bin", 0, 0x40000, CRC(f4f0e4fc) SHA1(87f11545db178914623e41fb51e328da479a2efc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "a7n266c1001d", "a7n266c1001d") // bios version 1001.D downloaded from Asus website
	ROMX_LOAD("a7nc101d.awd", 0, 0x40000, CRC(ead1147c) SHA1(27227df98e0c5fb9fecdb4bb6ef72df19766c330), ROM_BIOS(1))
ROM_END

static INPUT_PORTS_START(nforcepc)
	PORT_INCLUDE(at_keyboard)
INPUT_PORTS_END

COMP(2002, nforcepc, 0, 0, nforcepc, nforcepc, nforcepc_state, empty_init, "Nvidia", "Nvidia nForce PC (CRUSH11/12)", MACHINE_IS_SKELETON)
