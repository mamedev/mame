// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli
#ifndef MAME_PC_NFORCEPC_H
#define MAME_PC_NFORCEPC_H

#pragma once

#include "xbox_pci.h"

#include "imagedev/floppy.h"
#include "machine/8042kbdc.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/pc_lpt.h"
#include "machine/pci.h"
#include "machine/upd765.h"


// NVIDIA Corporation nForce CPU bridge

class crush11_host_device : public pci_host_device {
public:
	template <typename T>
	crush11_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, const char *bios_device_tag)
		: crush11_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x10de01a4, 0xb2, 0);
		set_cpu_tag(std::forward<T>(cpu_tag));
		biosrom.set_tag(bios_device_tag);
	}
	crush11_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { cpu.set_tag(std::forward<T>(tag)); }
	const char *get_cpu_tag() { return cpu.finder_tag(); }
	void set_ram_size(uint32_t size) { ram_size = size; }
	address_space *get_cpu_space(int spacenum) { return &cpu->space(spacenum); }

	void bios_map(address_map &map) ATTR_COLD;
	void aperture_map(address_map &map) {}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<device_memory_interface> cpu;
	required_device<intelfsh8_device> biosrom;
	uint32_t ram_size = 0;

	virtual uint8_t header_type_r() override;
	uint8_t unknown_r();
	void unknown_w(uint8_t data);
	uint32_t ram_size_r();
	void ram_size_w(uint32_t data);
};

DECLARE_DEVICE_TYPE(CRUSH11, crush11_host_device)

// For ddr ram

class crush11_memory_device : public pci_device {
public:
	crush11_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t subsystem_id, int ram_size);
	crush11_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_ram_size(int ram_size);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	int ddr_ram_size = 0;
	std::vector<uint32_t> ram{};
	crush11_host_device *host = nullptr;
	address_space *ram_space = nullptr;
};

DECLARE_DEVICE_TYPE(CRUSH11_MEMORY, crush11_memory_device)

// device connected to SMBus

class smbus_logger_device : public device_t, public smbus_interface
{
public:
	smbus_logger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;
	uint8_t *get_buffer() { return buffer; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t buffer[0xff]{};
};

DECLARE_DEVICE_TYPE(SMBUS_LOGGER, smbus_logger_device)

// Simple smbus rom used as a placeholder for the serial presence detect chip in a ddr dimm

class smbus_rom_device : public device_t, public smbus_interface
{
public:
	smbus_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const uint8_t *data, int size);
	smbus_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	const uint8_t *buffer  = nullptr;
	int buffer_size = 0;
};

DECLARE_DEVICE_TYPE(SMBUS_ROM, smbus_rom_device)

// Asus AS99127F chip
// It answers to three smbus addresses, by default 0x2d 0x48 0x49

class as99127f_device : public device_t, public smbus_interface
{
public:
	as99127f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;
	uint8_t *get_buffer() { return buffer; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t buffer[0xff]{};
};

DECLARE_DEVICE_TYPE(AS99127F, as99127f_device)

class as99127f_sensor2_device : public device_t, public smbus_interface
{
public:
	as99127f_sensor2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;
	uint8_t *get_buffer() { return buffer; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t buffer[0xff]{};
};

DECLARE_DEVICE_TYPE(AS99127F_SENSOR2, as99127f_sensor2_device)

class as99127f_sensor3_device : public device_t, public smbus_interface
{
public:
	as99127f_sensor3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;
	uint8_t *get_buffer() { return buffer; }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t buffer[0xff]{};
};

DECLARE_DEVICE_TYPE(AS99127F_SENSOR3, as99127f_sensor3_device)

// ITE IT8703F-A SuperIO

class it8703f_device : public device_t, public lpcbus_device_interface
{
public:
	it8703f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void map_extra(address_space *memory_space, address_space *io_space) override;
	virtual void set_host(int device_index, lpcbus_host_interface *host) override;
	virtual uint32_t dma_transfer(int channel, dma_operation operation, dma_size size, uint32_t data) override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	auto pin_reset() { return pin_reset_callback.bind(); }
	auto pin_gatea20() { return pin_gatea20_callback.bind(); }
	auto txd1() { return m_txd1_callback.bind(); }
	auto ndtr1() { return m_ndtr1_callback.bind(); }
	auto nrts1() { return m_nrts1_callback.bind(); }
	auto txd2() { return m_txd2_callback.bind(); }
	auto ndtr2() { return m_ndtr2_callback.bind(); }
	auto nrts2() { return m_nrts2_callback.bind(); }

	void map_lpt(address_map &map) ATTR_COLD;
	void map_serial1(address_map &map) ATTR_COLD;
	void map_serial2(address_map &map) ATTR_COLD;
	void map_keyboard(address_map &map) ATTR_COLD;

	// floppy disk controller
	void irq_floppy_w(int state);
	void drq_floppy_w(int state);
	// parallel port
	void irq_parallel_w(int state);
	void drq_parallel_w(int state);
	// uarts
	void irq_serial1_w(int state);
	void txd_serial1_w(int state);
	void dtr_serial1_w(int state);
	void rts_serial1_w(int state);
	void irq_serial2_w(int state);
	void txd_serial2_w(int state);
	void dtr_serial2_w(int state);
	void rts_serial2_w(int state);
	// uarts
	void rxd1_w(int state);
	void ndcd1_w(int state);
	void ndsr1_w(int state);
	void nri1_w(int state);
	void ncts1_w(int state);
	void rxd2_w(int state);
	void ndcd2_w(int state);
	void ndsr2_w(int state);
	void nri2_w(int state);
	void ncts2_w(int state);
	// keyboard
	void irq_keyboard_w(int state);
	void kbdp21_gp25_gatea20_w(int state);
	void kbdp20_gp20_reset_w(int state);

	uint8_t read_it8703f(offs_t offset);
	void write_it8703f(offs_t offset, uint8_t data);
	// parallel port
	uint8_t lpt_read(offs_t offset);
	void lpt_write(offs_t offset, uint8_t data);
	// uarts
	uint8_t serial1_read(offs_t offset);
	void serial1_write(offs_t offset, uint8_t data);
	uint8_t serial2_read(offs_t offset);
	void serial2_write(offs_t offset, uint8_t data);
	// keyboard
	uint8_t at_keybc_r(offs_t offset);
	void at_keybc_w(offs_t offset, uint8_t data);
	uint8_t keybc_status_r();
	void keybc_command_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	enum OperatingMode
	{
		Run = 0,
		Configuration = 1
	} mode;
	enum LogicalDevice
	{
		FDC = 0,        // Floppy disk controller
		Parallel,       // Parallel port
		Serial1,        // Serial port 1
		Serial2,        // Serial port 2
		Keyboard = 5,   // Keyboard controller
		ConsumerIR,     // Consumer IR
		Gpio1,          // Game port, MIDI, GPIO set 1
		Gpio2,          // GPIO set 2
		Gpio34,         // GPIO set 3 and 4
		ACPI,           // ACPI
		Gpio567 = 12    // GPIO set 5, 6 and 7
	};
	int config_key_step = 0;
	int config_index = 0;
	int logical_device = 0;
	uint8_t global_configuration_registers[0x30]{};
	uint8_t configuration_registers[13][0x100]{};
	devcb_write_line pin_reset_callback;
	devcb_write_line pin_gatea20_callback;
	devcb_write_line m_txd1_callback;
	devcb_write_line m_ndtr1_callback;
	devcb_write_line m_nrts1_callback;
	devcb_write_line m_txd2_callback;
	devcb_write_line m_ndtr2_callback;
	devcb_write_line m_nrts2_callback;
	required_device<smc37c78_device> floppy_controller_fdcdev;
	required_device<pc_lpt_device> pc_lpt_lptdev;
	required_device<ns16450_device> pc_serial1_comdev;
	required_device<ns16450_device> pc_serial2_comdev;
	required_device<kbdc8042_device> m_kbdc;
	bool enabled_logical[13]{};
	bool enabled_game_port = false;
	bool enabled_midi_port = false;

	lpcbus_host_interface *lpchost = nullptr;
	int lpcindex = 0;
	address_space *memspace = nullptr;
	address_space *iospace = nullptr;

	void internal_memory_map(address_map &map) ATTR_COLD;
	void internal_io_map(address_map &map) ATTR_COLD;
	uint16_t get_base_address(int logical, int index);
	void map_fdc_addresses();
	void map_lpt_addresses();
	void map_serial1_addresses();
	void map_serial2_addresses();
	void map_keyboard_addresses();
	void write_global_configuration_register(int index, int data);
	void write_logical_configuration_register(int index, int data);
	void write_fdd_configuration_register(int index, int data);
	void write_parallel_configuration_register(int index, int data);
	void write_serial1_configuration_register(int index, int data);
	void write_serial2_configuration_register(int index, int data);
	void write_keyboard_configuration_register(int index, int data);
	uint16_t read_global_configuration_register(int index);
	uint16_t read_logical_configuration_register(int index);
	uint16_t read_fdd_configuration_register(int index) { return configuration_registers[LogicalDevice::FDC][index]; }
	uint16_t read_parallel_configuration_register(int index) { return configuration_registers[LogicalDevice::Parallel][index]; }
	uint16_t read_serial1_configuration_register(int index) { return configuration_registers[LogicalDevice::Serial1][index]; }
	uint16_t read_serial2_configuration_register(int index) { return configuration_registers[LogicalDevice::Serial2][index]; }
	uint16_t read_keyboard_configuration_register(int index) { return configuration_registers[LogicalDevice::Keyboard][index]; }
	void assign_dma_channels();
};

DECLARE_DEVICE_TYPE(IT8703F, it8703f_device)

#endif // MAME_PC_NFORCEPC_H
