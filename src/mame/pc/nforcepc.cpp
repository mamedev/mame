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
  - A nVidia NV25 based AGP video card

*/

#include "emu.h"
#include "nforcepc.h"

#include "xbox_pci.h"

#include "bus/ata/atadev.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/athlon.h"
#include "machine/pci-ide.h"
#include "machine/pckeybrd.h"
#include "bus/isa/isa.h"
#include "bus/pci/virge_pci.h"

#include "formats/naslite_dsk.h"


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
	map(0x10, 0x27).rw(FUNC(pci_device::address_base_r), FUNC(pci_device::address_base_w));
	map(0x84, 0x87).rw(FUNC(crush11_host_device::ram_size_r), FUNC(crush11_host_device::ram_size_w));
	map(0xf0, 0xf0).rw(FUNC(crush11_host_device::unknown_r), FUNC(crush11_host_device::unknown_w));
}

uint8_t crush11_host_device::header_type_r()
{
	return 0x80; // from lspci dump
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
	set_multifunction_device(true);
	set_spaces(&cpu->space(AS_DATA), &cpu->space(AS_IO));

	memory_window_start = 0;
	memory_window_end = 0xffffffff;
	memory_offset = 0;
	io_window_start = 0;
	io_window_end = 0xffff;
	io_offset = 0;
	status = 0x00b0;
	command = 0x0000;

	add_map(64 * 1024 * 1024, M_MEM | M_PREF, FUNC(crush11_host_device::aperture_map));
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

uint8_t crush11_host_device::unknown_r()
{
	return 4;
}

void crush11_host_device::unknown_w(uint8_t data)
{
	logerror("test = %02x\n", data);
}

uint32_t crush11_host_device::ram_size_r()
{
	return ram_size * 1024 * 1024 - 1;
}

void crush11_host_device::ram_size_w(uint32_t data)
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

crush11_memory_device::crush11_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id, int ram_size)
	: crush11_memory_device(mconfig, tag, owner, clock)
{
	set_ids(0x10de01ac, 0xb2, 0x050000, subsystem_id);
	set_ram_size(ram_size);
}

crush11_memory_device::crush11_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, CRUSH11_MEMORY, tag, owner, clock)
{
}

void crush11_memory_device::device_start()
{
	device_t *r = owner()->subdevice("00.0");

	pci_device::device_start();
	set_multifunction_device(true);
	ram.resize(ddr_ram_size * 1024 * 1024 / 4);
	host = dynamic_cast<crush11_host_device *>(r);
	ram_space = host->get_cpu_space(AS_PROGRAM);
	status = 0x0020;
	command = 0x0000;
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
	// used to read voltages by bios, measured in mV
	buffer[0x20] = 0x70; // multiplied by 0x10
	buffer[0x2] = 0x7e; // multiplied by 0x10
	buffer[0x23] = 0x96; // multiplied by 0x540 then divided by 0x32
	buffer[0x24] = 0x9e; // multiplied by 0x260 then divided by 0xa
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
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
	, floppy_controller_fdcdev(*this, "fdc")
	, pc_lpt_lptdev(*this, "lpt")
	, pc_serial1_comdev(*this, "uart_0")
	, pc_serial2_comdev(*this, "uart_1")
	, m_kbdc(*this, "pc_kbdc")
	, enabled_game_port(false)
	, enabled_midi_port(false)
{
	memset(global_configuration_registers, 0, sizeof(global_configuration_registers));
	global_configuration_registers[0x20] = 0x87; // identifies it8703f
	global_configuration_registers[0x21] = 1;
	global_configuration_registers[0x24] = 4;
	memset(configuration_registers, 0, sizeof(configuration_registers));
	configuration_registers[LogicalDevice::FDC][0x60] = 3;
	configuration_registers[LogicalDevice::FDC][0x61] = 0xf0;
	configuration_registers[LogicalDevice::FDC][0x70] = 6;
	configuration_registers[LogicalDevice::FDC][0x74] = 2;
	configuration_registers[LogicalDevice::FDC][0xf0] = 0xe;
	configuration_registers[LogicalDevice::FDC][0xf2] = 0xff;
	configuration_registers[LogicalDevice::Parallel][0x74] = 4;
	configuration_registers[LogicalDevice::Parallel][0xf0] = 0x3f;
	for (int n = 0; n < 13; n++)
		enabled_logical[n] = false;
}

void it8703f_device::device_start()
{
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
	// floppy disc controller
	smc37c78_device& fdcdev(SMC37C78(config, floppy_controller_fdcdev, 24'000'000));
	fdcdev.intrq_wr_callback().set(FUNC(it8703f_device::irq_floppy_w));
	fdcdev.drq_wr_callback().set(FUNC(it8703f_device::drq_floppy_w));

	// parallel port
	PC_LPT(config, pc_lpt_lptdev);
	pc_lpt_lptdev->irq_handler().set(FUNC(it8703f_device::irq_parallel_w));

	// serial ports
	NS16450(config, pc_serial1_comdev, XTAL(1'843'200)); // or NS16550 ?
	pc_serial1_comdev->out_int_callback().set(FUNC(it8703f_device::irq_serial1_w));
	pc_serial1_comdev->out_tx_callback().set(FUNC(it8703f_device::txd_serial1_w));
	pc_serial1_comdev->out_dtr_callback().set(FUNC(it8703f_device::dtr_serial1_w));
	pc_serial1_comdev->out_rts_callback().set(FUNC(it8703f_device::rts_serial1_w));

	NS16450(config, pc_serial2_comdev, XTAL(1'843'200));
	pc_serial2_comdev->out_int_callback().set(FUNC(it8703f_device::irq_serial2_w));
	pc_serial2_comdev->out_tx_callback().set(FUNC(it8703f_device::txd_serial2_w));
	pc_serial2_comdev->out_dtr_callback().set(FUNC(it8703f_device::dtr_serial2_w));
	pc_serial2_comdev->out_rts_callback().set(FUNC(it8703f_device::rts_serial2_w));

	// keyboard
	KBDC8042(config, m_kbdc);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_PS2);
	m_kbdc->input_buffer_full_callback().set(FUNC(it8703f_device::irq_keyboard_w));
	m_kbdc->system_reset_callback().set(FUNC(it8703f_device::kbdp20_gp20_reset_w));
	m_kbdc->gate_a20_callback().set(FUNC(it8703f_device::kbdp21_gp25_gatea20_w));
	m_kbdc->set_keyboard_tag("at_keyboard");

	at_keyboard_device &at_keyb(AT_KEYB(config, "at_keyboard", pc_keyboard_device::KEYBOARD_TYPE::AT, 1));
	at_keyb.keypress().set(m_kbdc, FUNC(kbdc8042_device::keyboard_w));
}

uint8_t it8703f_device::read_it8703f(offs_t offset)
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

void it8703f_device::write_it8703f(offs_t offset, uint8_t data)
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
	case LogicalDevice::FDC:
		write_fdd_configuration_register(index, data);
		break;
	case LogicalDevice::Serial1:
		write_serial1_configuration_register(index, data);
		break;
	case LogicalDevice::Serial2:
		write_serial2_configuration_register(index, data);
		break;
	case LogicalDevice::Parallel:
		write_parallel_configuration_register(index, data);
		break;
	case LogicalDevice::Keyboard:
		write_keyboard_configuration_register(index, data);
		break;
	case LogicalDevice::Gpio1:
		if (index == 0x30)
		{
			if (data & 1)
				enabled_game_port = true;
			if (data & 2)
				enabled_midi_port = true;
		}
		break;
	}
}

void it8703f_device::write_fdd_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::FDC] == false)
			{
				enabled_logical[LogicalDevice::FDC] = true;
				lpchost->remap();
			}
			logerror("Enabled FDD at %04X\n", get_base_address(LogicalDevice::FDC, 0));
		}
		else
		{
			if (enabled_logical[LogicalDevice::FDC] == true)
			{
				enabled_logical[LogicalDevice::FDC] = false;
				lpchost->remap();
			}
			logerror("Disabled FDD at %04X\n", get_base_address(LogicalDevice::FDC, 0));
		}
	}
	if (index == 0x74)
	{
		assign_dma_channels();
		logerror("Set FDD dma channel %d\n", configuration_registers[LogicalDevice::FDC][0x74]);
	}
}

void it8703f_device::write_parallel_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::Parallel] == false)
			{
				enabled_logical[LogicalDevice::Parallel] = true;
				lpchost->remap();
			}
			logerror("Enabled LPT at %04X\n", get_base_address(LogicalDevice::Parallel, 0));
		}
		else
		{
			if (enabled_logical[LogicalDevice::Parallel] == true)
			{
				enabled_logical[LogicalDevice::Parallel] = false;
				lpchost->remap();
			}
		}
	}
	if (index == 0x74)
	{
		assign_dma_channels();
		logerror("Set LPT dma channel %d\n", configuration_registers[LogicalDevice::Parallel][0x74]);
	}
}

void it8703f_device::write_serial1_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::Serial1] == false)
			{
				enabled_logical[LogicalDevice::Serial1] = true;
				lpchost->remap();
			}
			logerror("Enabled Serial1 at %04X\n", get_base_address(LogicalDevice::Serial1, 0));
		}
		else
		{
			if (enabled_logical[LogicalDevice::Serial1] == true)
			{
				enabled_logical[LogicalDevice::Serial1] = false;
				lpchost->remap();
			}
		}
	}
}

void it8703f_device::write_serial2_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::Serial2] == false)
			{
				enabled_logical[LogicalDevice::Serial2] = true;
				lpchost->remap();
			}
			logerror("Enabled Serial2 at %04X\n", get_base_address(LogicalDevice::Serial2, 0));
		}
		else
		{
			if (enabled_logical[LogicalDevice::Serial2] == true)
			{
				enabled_logical[LogicalDevice::Serial2] = false;
				lpchost->remap();
			}
		}
	}
}

void it8703f_device::write_keyboard_configuration_register(int index, int data)
{
	if (index == 0x30)
	{
		if (data & 1)
		{
			if (enabled_logical[LogicalDevice::Keyboard] == false)
			{
				enabled_logical[LogicalDevice::Keyboard] = true;
				lpchost->remap();
			}
			logerror("Enabled Keyboard\n");
		}
		else
		{
			if (enabled_logical[LogicalDevice::Keyboard] == true)
			{
				enabled_logical[LogicalDevice::Keyboard] = false;
				lpchost->remap();
			}
		}
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
	uint16_t ret = 0;

	switch (logical_device)
	{
	case LogicalDevice::FDC:
		ret = read_fdd_configuration_register(index);
		break;
	case LogicalDevice::Parallel:
		ret = read_parallel_configuration_register(index);
		break;
	case LogicalDevice::Serial1:
		ret = read_serial1_configuration_register(index);
		break;
	case LogicalDevice::Serial2:
		ret = read_serial2_configuration_register(index);
		break;
	case LogicalDevice::Keyboard:
		ret = read_keyboard_configuration_register(index);
		break;
	default:
		ret = configuration_registers[logical_device][index];
		break;
	}
	return ret;
}

void it8703f_device::irq_floppy_w(int state)
{
	if (enabled_logical[LogicalDevice::FDC] == false)
		return;
	lpchost->set_virtual_line(configuration_registers[LogicalDevice::FDC][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8703f_device::drq_floppy_w(int state)
{
	if (enabled_logical[LogicalDevice::FDC] == false)
		return;
	lpchost->set_virtual_line(configuration_registers[LogicalDevice::FDC][0x74] + 16, state ? ASSERT_LINE : CLEAR_LINE);
}

void it8703f_device::irq_parallel_w(int state)
{
	if (enabled_logical[LogicalDevice::Parallel] == false)
		return;
	lpchost->set_virtual_line(configuration_registers[LogicalDevice::Parallel][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8703f_device::drq_parallel_w(int state)
{
	if (enabled_logical[LogicalDevice::Parallel] == false)
		return;
	lpchost->set_virtual_line(configuration_registers[LogicalDevice::Parallel][0x74] + 16, state ? ASSERT_LINE : CLEAR_LINE);
}

void it8703f_device::irq_serial1_w(int state)
{
	if (enabled_logical[LogicalDevice::Serial1] == false)
		return;
	lpchost->set_virtual_line(configuration_registers[LogicalDevice::Serial1][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8703f_device::txd_serial1_w(int state)
{
	if (enabled_logical[LogicalDevice::Serial1] == false)
		return;
	m_txd1_callback(state);
}

void it8703f_device::dtr_serial1_w(int state)
{
	if (enabled_logical[LogicalDevice::Serial1] == false)
		return;
	m_ndtr1_callback(state);
}

void it8703f_device::rts_serial1_w(int state)
{
	if (enabled_logical[LogicalDevice::Serial1] == false)
		return;
	m_nrts1_callback(state);
}

void it8703f_device::irq_serial2_w(int state)
{
	if (enabled_logical[LogicalDevice::Serial2] == false)
		return;
	lpchost->set_virtual_line(configuration_registers[LogicalDevice::Serial2][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8703f_device::txd_serial2_w(int state)
{
	if (enabled_logical[LogicalDevice::Serial2] == false)
		return;
	m_txd2_callback(state);
}

void it8703f_device::dtr_serial2_w(int state)
{
	if (enabled_logical[LogicalDevice::Serial2] == false)
		return;
	m_ndtr2_callback(state);
}

void it8703f_device::rts_serial2_w(int state)
{
	if (enabled_logical[LogicalDevice::Serial2] == false)
		return;
	m_nrts2_callback(state);
}

void it8703f_device::rxd1_w(int state)
{
	pc_serial1_comdev->rx_w(state);
}

void it8703f_device::ndcd1_w(int state)
{
	pc_serial1_comdev->dcd_w(state);
}

void it8703f_device::ndsr1_w(int state)
{
	pc_serial1_comdev->dsr_w(state);
}

void it8703f_device::nri1_w(int state)
{
	pc_serial1_comdev->ri_w(state);
}

void it8703f_device::ncts1_w(int state)
{
	pc_serial1_comdev->cts_w(state);
}

void it8703f_device::rxd2_w(int state)
{
	pc_serial2_comdev->rx_w(state);
}

void it8703f_device::ndcd2_w(int state)
{
	pc_serial2_comdev->dcd_w(state);
}

void it8703f_device::ndsr2_w(int state)
{
	pc_serial2_comdev->dsr_w(state);
}

void it8703f_device::nri2_w(int state)
{
	pc_serial2_comdev->ri_w(state);
}

void it8703f_device::ncts2_w(int state)
{
	pc_serial2_comdev->cts_w(state);
}

void it8703f_device::irq_keyboard_w(int state)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	lpchost->set_virtual_line(configuration_registers[LogicalDevice::Keyboard][0x70], state ? ASSERT_LINE : CLEAR_LINE);
}

void it8703f_device::kbdp21_gp25_gatea20_w(int state)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	pin_gatea20_callback(state);
}

void it8703f_device::kbdp20_gp20_reset_w(int state)
{
	if (enabled_logical[LogicalDevice::Keyboard] == false)
		return;
	pin_reset_callback(state);
}

void it8703f_device::map_fdc_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::FDC, 0);

	iospace->install_device(base, base + 7, *floppy_controller_fdcdev, &smc37c78_device::map);
}

void it8703f_device::map_lpt(address_map& map)
{
	map(0x0, 0x3).rw(FUNC(it8703f_device::lpt_read), FUNC(it8703f_device::lpt_write));
}

uint8_t it8703f_device::lpt_read(offs_t offset)
{
	return pc_lpt_lptdev->read(offset);
}

void it8703f_device::lpt_write(offs_t offset, uint8_t data)
{
	pc_lpt_lptdev->write(offset, data);
}

void it8703f_device::map_lpt_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::Parallel, 0);

	iospace->install_device(base, base + 3, *this, &it8703f_device::map_lpt);
}

void it8703f_device::map_serial1(address_map& map)
{
	map(0x0, 0x7).rw(FUNC(it8703f_device::serial1_read), FUNC(it8703f_device::serial1_write));
}

uint8_t it8703f_device::serial1_read(offs_t offset)
{
	return pc_serial1_comdev->ins8250_r(offset);
}

void it8703f_device::serial1_write(offs_t offset, uint8_t data)
{
	pc_serial1_comdev->ins8250_w(offset, data);
}

void it8703f_device::map_serial1_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::Serial1, 0);

	iospace->install_device(base, base + 7, *this, &it8703f_device::map_serial1);
}

void it8703f_device::map_serial2(address_map& map)
{
	map(0x0, 0x7).rw(FUNC(it8703f_device::serial2_read), FUNC(it8703f_device::serial2_write));
}

uint8_t it8703f_device::serial2_read(offs_t offset)
{
	return pc_serial2_comdev->ins8250_r(offset);
}

void it8703f_device::serial2_write(offs_t offset, uint8_t data)
{
	pc_serial2_comdev->ins8250_w(offset, data);
}

void it8703f_device::map_serial2_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::Serial2, 0);

	iospace->install_device(base, base + 7, *this, &it8703f_device::map_serial2);
}

void it8703f_device::map_keyboard(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(it8703f_device::at_keybc_r), FUNC(it8703f_device::at_keybc_w));
	map(0x4, 0x4).rw(FUNC(it8703f_device::keybc_status_r), FUNC(it8703f_device::keybc_command_w));
}

uint8_t it8703f_device::at_keybc_r(offs_t offset)
{
	switch (offset) //m_kbdc
	{
	case 0:
		return m_kbdc->data_r(0);
	}

	return 0xff;
}

void it8703f_device::at_keybc_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		m_kbdc->data_w(0, data);
	}
}

uint8_t it8703f_device::keybc_status_r()
{
	return m_kbdc->data_r(4);
}

void it8703f_device::keybc_command_w(uint8_t data)
{
	m_kbdc->data_w(4, data);
}

void it8703f_device::map_keyboard_addresses()
{
	uint16_t base = get_base_address(LogicalDevice::Keyboard, 0);

	iospace->install_device(base, base + 7, *this, &it8703f_device::map_keyboard);
}

void it8703f_device::map_extra(address_space *memory_space, address_space *io_space)
{
	memspace = memory_space;
	iospace = io_space;
	io_space->install_device(0, 0xffff, *this, &it8703f_device::internal_io_map);
	if (enabled_logical[LogicalDevice::FDC] == true)
		map_fdc_addresses();
	if (enabled_logical[LogicalDevice::Parallel] == true)
		map_lpt_addresses();
	if (enabled_logical[LogicalDevice::Serial1] == true)
		map_serial1_addresses();
	if (enabled_logical[LogicalDevice::Serial2] == true)
		map_serial2_addresses();
	if (enabled_logical[LogicalDevice::Keyboard] == true)
		map_keyboard_addresses();
}

void it8703f_device::set_host(int device_index, lpcbus_host_interface *host)
{
	lpchost = host;
	lpcindex = device_index;
	lpchost->assign_virtual_line(24 + configuration_registers[LogicalDevice::FDC][0x74], lpcindex);
	lpchost->assign_virtual_line(24 + configuration_registers[LogicalDevice::Parallel][0x74], lpcindex);
}

uint32_t it8703f_device::dma_transfer(int channel, dma_operation operation, dma_size size, uint32_t data)
{
	uint32_t ret = 0;

	if (enabled_logical[LogicalDevice::FDC] == true)
		if (channel == configuration_registers[LogicalDevice::FDC][0x74])
		{
			if (operation == dma_operation::WRITE)
				ret = (uint32_t)floppy_controller_fdcdev->dma_r();
			else if (operation == dma_operation::READ)
				floppy_controller_fdcdev->dma_w((uint8_t)data);
			else if (operation == dma_operation::END)
				floppy_controller_fdcdev->tc_w(data == ASSERT_LINE);
		}
	if (enabled_logical[LogicalDevice::Parallel] == true)
		if (channel == configuration_registers[LogicalDevice::Parallel][0x74])
			printf("it8703f_device::dma_transfer LPT channel %d op %d size %d\n", channel, (int)operation, (int)size);
	return ret;
}

void it8703f_device::assign_dma_channels()
{
	lpchost->assign_virtual_line(-1, lpcindex); // remove assigments
	lpchost->assign_virtual_line(24 + configuration_registers[LogicalDevice::FDC][0x74], lpcindex);
	lpchost->assign_virtual_line(24 + configuration_registers[LogicalDevice::Parallel][0x74], lpcindex);
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
	void nforce_map(address_map &map) ATTR_COLD;
	void nforce_map_io(address_map &map) ATTR_COLD;
	void boot_state_award_w(uint8_t data);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void maincpu_interrupt(int state);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<mcpx_isalpc_device> isalpc;
	required_device<as99127f_device> m_as99127f;
};

nforcepc_state::nforcepc_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	isalpc(*this, "pci:01.0"),
	m_as99127f(*this, "pci:01.1:12d")
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

void nforcepc_state::boot_state_award_w(uint8_t data)
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

void nforcepc_state::maincpu_interrupt(int state)
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

static void isa_com(device_slot_interface& device)
{
	device.option_add("microsoft_mouse", MSFT_HLE_SERIAL_MOUSE);
	device.option_add("logitech_mouse", LOGITECH_HLE_SERIAL_MOUSE);
	device.option_add("wheel_mouse", WHEEL_HLE_SERIAL_MOUSE);
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
	device.option_add("rotatable_mouse", ROTATABLE_HLE_SERIAL_MOUSE);
	device.option_add("terminal", SERIAL_TERMINAL);
	device.option_add("null_modem", NULL_MODEM);
	device.option_add("sun_kbd", SUN_KBD_ADAPTOR);
}

static void pc_hd_floppies(device_slot_interface& device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

static void floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
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
	//maincpu.smiact().set("pci:01.0", FUNC(???_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	CRUSH11(config, "pci:00.0", 0, "maincpu", "bios"); // 10de:01a4 NVIDIA Corporation nForce CPU bridge
	CRUSH11_MEMORY(config, "pci:00.1", 0, 0x10430c11, 2); // 10de:01ac NVIDIA Corporation nForce 220/420 Memory Controller
	// 10de:01ad NVIDIA Corporation nForce 220/420 Memory Controller
	// 10de:01ab NVIDIA Corporation nForce 420 Memory Controller (DDR)
	mcpx_isalpc_device &isa(MCPX_ISALPC(config, "pci:01.0", 0, 0x10430c11)); // 10de:01b2 NVIDIA Corporation nForce ISA Bridge (LPC bus)
	isa.smi().set_inputline(":maincpu", INPUT_LINE_SMI);
	isa.boot_state_hook().set(FUNC(nforcepc_state::boot_state_award_w));
	isa.interrupt_output().set(FUNC(nforcepc_state::maincpu_interrupt));
	isa.set_dma_space("maincpu", AS_OPCODES);
	it8703f_device &ite(IT8703F(config, "pci:01.0:0", 0));
	ite.pin_reset().set_inputline("maincpu", INPUT_LINE_RESET);
	ite.pin_gatea20().set_inputline("maincpu", INPUT_LINE_A20);
	ite.txd1().set("serport0", FUNC(rs232_port_device::write_txd));
	ite.ndtr1().set("serport0", FUNC(rs232_port_device::write_dtr));
	ite.nrts1().set("serport0", FUNC(rs232_port_device::write_rts));
	ite.txd2().set("serport1", FUNC(rs232_port_device::write_txd));
	ite.ndtr2().set("serport1", FUNC(rs232_port_device::write_dtr));
	ite.nrts2().set("serport1", FUNC(rs232_port_device::write_rts));
	MCPX_SMBUS(config, "pci:01.1", 0, 0x10430c11); // 10de:01b4 NVIDIA Corporation nForce PCI System Management (SMBus)
	SMBUS_ROM(config, "pci:01.1:050", 0, test_spd_data, sizeof(test_spd_data)); // these 3 are on smbus number 0
	SMBUS_LOGGER(config, "pci:01.1:051", 0);
	SMBUS_LOGGER(config, "pci:01.1:052", 0);
	SMBUS_LOGGER(config, "pci:01.1:108", 0); // these 4 are on smbus number 1
	AS99127F(config, "pci:01.1:12d", 0);
	AS99127F_SENSOR2(config, "pci:01.1:148", 0);
	AS99127F_SENSOR3(config, "pci:01.1:149", 0);
	mcpx_ohci_device &ohci(MCPX_OHCI(config, "pci:02.0", 0, 0x10430c11)); // 10de:01c2 NVIDIA Corporation nForce USB Controller
	ohci.interrupt_handler().set("pci:01.0", FUNC(mcpx_isalpc_device::irq1));
	MCPX_OHCI(config, "pci:03.0", 0, 0x10430c11); // 10de:01c2 NVIDIA Corporation nForce USB Controller
	MCPX_ETH(config, "pci:04.0", 0); // 10de:01c3 NVIDIA Corporation nForce Ethernet Controller
	MCPX_APU(config, "pci:05.0", 0, 0x10430c11, m_maincpu); // 10de:01b0 NVIDIA Corporation nForce Audio Processing Unit
	MCPX_AC97_AUDIO(config, "pci:06.0", 0, 0x10438384); // 10de:01b1 NVIDIA Corporation nForce AC'97 Audio Controller
	PCI_BRIDGE(config, "pci:08.0", 0, 0x10de01b8, 0xc2); // 10de:01b8 NVIDIA Corporation nForce PCI-to-PCI bridge
	// 10ec:8139 Realtek Semiconductor Co., Ltd. RTL-8139/8139C/8139C+ (behind bridge)
	mcpx_ide_device &ide(MCPX_IDE(config, "pci:09.0", 0, 0x10430c11)); // 10de:01bc NVIDIA Corporation nForce IDE
	ide.pri_interrupt_handler().set("pci:01.0", FUNC(mcpx_isalpc_device::irq14));
	ide.sec_interrupt_handler().set("pci:01.0", FUNC(mcpx_isalpc_device::irq15));
	ide.set_bus_master_space("maincpu", AS_OPCODES);
	ide.subdevice<ide_controller_32_device>("ide1")->options(ata_devices, "hdd", nullptr, true);
	ide.subdevice<ide_controller_32_device>("ide2")->options(ata_devices, "cdrom", nullptr, true);
	NV2A_AGP(config, "pci:1e.0", 0, 0x10de01b7, 0xb2); // 10de:01b7 NVIDIA Corporation nForce AGP to PCI Bridge
	PCI_SLOT(config, "pci:1", pci_cards, 10, 0, 1, 2, 3, "virgedx");
	SST_49LF020(config, "bios", 0);

	FLOPPY_CONNECTOR(config, "pci:01.0:0:fdc:0", pc_hd_floppies, "35hd", floppy_formats);
	FLOPPY_CONNECTOR(config, "pci:01.0:0:fdc:1", pc_hd_floppies, "35hd", floppy_formats);

	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, nullptr));
	serport0.rxd_handler().set("pci:01.0:0", FUNC(it8703f_device::rxd1_w));
	serport0.dcd_handler().set("pci:01.0:0", FUNC(it8703f_device::ndcd1_w));
	serport0.dsr_handler().set("pci:01.0:0", FUNC(it8703f_device::ndsr1_w));
	serport0.ri_handler().set("pci:01.0:0", FUNC(it8703f_device::nri1_w));
	serport0.cts_handler().set("pci:01.0:0", FUNC(it8703f_device::ncts1_w));

	rs232_port_device& serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("pci:01.0:0", FUNC(it8703f_device::rxd2_w));
	serport1.dcd_handler().set("pci:01.0:0", FUNC(it8703f_device::ndcd2_w));
	serport1.dsr_handler().set("pci:01.0:0", FUNC(it8703f_device::ndsr2_w));
	serport1.ri_handler().set("pci:01.0:0", FUNC(it8703f_device::nri2_w));
	serport1.cts_handler().set("pci:01.0:0", FUNC(it8703f_device::ncts2_w));
}

ROM_START(nforcepc)
	ROM_REGION32_LE(0x40000, "bios", 0) /* PC bios */
	ROM_SYSTEM_BIOS(0, "a7n266c", "a7n266c") // Motherboard dump. Chip: SST49LF020 Package: PLCC32 Label had 3 lines of text: "A7NC3" "1001.D" "GSQ98"
	ROMX_LOAD("a7n266c.bin", 0, 0x40000, CRC(f4f0e4fc) SHA1(87f11545db178914623e41fb51e328da479a2efc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "a7n266c1001d", "a7n266c1001d") // bios version 1001.D downloaded from Asus website
	ROMX_LOAD("a7nc101d.awd", 0, 0x40000, CRC(ead1147c) SHA1(27227df98e0c5fb9fecdb4bb6ef72df19766c330), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "a7n266c1001e", "a7n266c1001e") // bios version 1001.E
	ROMX_LOAD("a7nc101e.awd", 0, 0x40000, CRC(a029cd42) SHA1(e257915534bc725f389e57945b45c81e7ef40dcc), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "a7n266c1002c", "a7n266c1002c") // beta bios version 1002C.003 ?
	ROMX_LOAD("1002c.003", 0, 0x40000, CRC(57ced539) SHA1(525d15523be3b373a10c1f6ae355803613506ce2), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "a7n266c1003", "a7n266c1003") // bios version 1003
	ROMX_LOAD("a7nc1003.awd", 0, 0x40000, CRC(ac27f751) SHA1(96d5539ee2a40ea58a32013c16109159a3c41bb8), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "a7n266c1004", "a7n266c1004") // bios version 1004
	ROMX_LOAD("a7nc1004.awd", 0, 0x40000, CRC(04124e4f) SHA1(8778a6722eddaf83101f89834969f0037af65cb9), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "a7n266c1005", "a7n266c1005") // beta bios version 1005.005 ?
	ROMX_LOAD("1005nc.005", 0, 0x40000, CRC(9ca5a9c9) SHA1(5ffb57b9f1e0e163b33c093a3e017020b247b3d3), ROM_BIOS(6))
ROM_END

static INPUT_PORTS_START(nforcepc)
INPUT_PORTS_END

COMP(2002, nforcepc, 0, 0, nforcepc, nforcepc, nforcepc_state, empty_init, "Nvidia", "Nvidia nForce PC (CRUSH11/12)", MACHINE_IS_SKELETON)
