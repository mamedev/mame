// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli
#ifndef MAME_MACHINE_NFORCEPC_H
#define MAME_MACHINE_NFORCEPC_H

#pragma once
#include "machine/8042kbdc.h"

// NVIDIA Corporation nForce CPU bridge

class crush11_host_device : public pci_host_device {
public:
	template <typename T>
	crush11_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, const char *bios_device_tag)
		: crush11_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x10de01a4, 0x01, 0x10430c11);
		set_cpu_tag(std::forward<T>(cpu_tag));
		biosrom.set_tag(bios_device_tag);
	}
	crush11_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { cpu.set_tag(std::forward<T>(tag)); }
	const char *get_cpu_tag() { return cpu.finder_tag(); }
	void set_ram_size(uint32_t size) { ram_size = size; }
	address_space *get_cpu_space(int spacenum) { return &cpu->space(spacenum); }

	void bios_map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

private:
	required_device<device_memory_interface> cpu;
	required_device<intelfsh8_device> biosrom;
	uint32_t ram_size;

	virtual DECLARE_READ8_MEMBER(header_type_r) override;
	DECLARE_READ8_MEMBER(unknown_r);
	DECLARE_WRITE8_MEMBER(unknown_w);
	DECLARE_READ32_MEMBER(ram_size_r);
	DECLARE_WRITE32_MEMBER(ram_size_w);
};

DECLARE_DEVICE_TYPE(CRUSH11, crush11_host_device)

// For ddr ram

class crush11_memory_device : public pci_device {
public:
	crush11_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int ram_size);
	crush11_memory_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_ram_size(int ram_size);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

private:
	int ddr_ram_size;
	std::vector<uint32_t> ram;
	crush11_host_device *host;
	address_space *ram_space;
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
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t buffer[0xff];
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
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	const uint8_t *buffer;
	int buffer_size;
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
	virtual void device_start() override;

private:
	uint8_t buffer[0xff];
};

DECLARE_DEVICE_TYPE(AS99127F, as99127f_device)

class as99127f_sensor2_device : public device_t, public smbus_interface
{
public:
	as99127f_sensor2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;
	uint8_t *get_buffer() { return buffer; }

protected:
	virtual void device_start() override;

private:
	uint8_t buffer[0xff];
};

DECLARE_DEVICE_TYPE(AS99127F_SENSOR2, as99127f_sensor2_device)

class as99127f_sensor3_device : public device_t, public smbus_interface
{
public:
	as99127f_sensor3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual int execute_command(int command, int rw, int data) override;
	uint8_t *get_buffer() { return buffer; }

protected:
	virtual void device_start() override;

private:
	uint8_t buffer[0xff];
};

DECLARE_DEVICE_TYPE(AS99127F_SENSOR3, as99127f_sensor3_device)

// ITE IT8703F-A SuperIO

class it8703f_device : public device_t, public lpcbus_device_interface
{
public:
	it8703f_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void map_extra(address_space *memory_space, address_space *io_space) override;
	virtual void set_host(int index, lpcbus_host_interface *host) override;
	virtual void device_add_mconfig(machine_config &config) override;

	auto pin_reset() { return pin_reset_callback.bind(); }
	auto pin_gatea20() { return pin_gatea20_callback.bind(); }

	// keyboard
	DECLARE_WRITE_LINE_MEMBER(irq_keyboard_w);
	DECLARE_WRITE_LINE_MEMBER(kbdp21_gp25_gatea20_w);
	DECLARE_WRITE_LINE_MEMBER(kbdp20_gp20_reset_w);

	void map_keyboard(address_map &map);
	void unmap_keyboard(address_map &map);

	DECLARE_READ8_MEMBER(read_it8703f);
	DECLARE_WRITE8_MEMBER(write_it8703f);
	// keyboard
	DECLARE_READ8_MEMBER(at_keybc_r);
	DECLARE_WRITE8_MEMBER(at_keybc_w);
	DECLARE_READ8_MEMBER(keybc_status_r);
	DECLARE_WRITE8_MEMBER(keybc_command_w);

protected:
	virtual void device_start() override;

private:
	enum OperatingMode
	{
		Run = 0,
		Configuration = 1
	} mode;
	enum LogicalDevice
	{
		FDC = 0,
		Parallel,
		Serial1,
		Serial2,
		Keyboard = 5,
		ConsumerIR,
		Gpio1,
		Gpio2,
		Gpio34,
		ACPI,
		Gpio567 = 12
	};
	int config_key_step;
	int config_index;
	int logical_device;
	uint8_t global_configuration_registers[0x30];
	uint8_t configuration_registers[13][0x100];
	devcb_write_line pin_reset_callback;
	devcb_write_line pin_gatea20_callback;
	required_device<kbdc8042_device> m_kbdc;
	bool enabled_logical[13];

	lpcbus_host_interface *lpchost;
	int lpcindex;
	address_space *memspace;
	address_space *iospace;

	void internal_memory_map(address_map &map);
	void internal_io_map(address_map &map);
	uint16_t get_base_address(int logical, int index);
	void map_keyboard_addresses();
	void unmap_keyboard_addresses();
	void write_global_configuration_register(int index, int data);
	void write_logical_configuration_register(int index, int data);
	void write_keyboard_configuration_register(int index, int data);
	uint16_t read_global_configuration_register(int index);
	uint16_t read_logical_configuration_register(int index);
	uint16_t read_keyboard_configuration_register(int index);
};

DECLARE_DEVICE_TYPE(IT8703F, it8703f_device)


#endif
