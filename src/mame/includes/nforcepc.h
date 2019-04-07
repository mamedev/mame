// license:BSD-3-Clause
// copyright-holders: Samuele Zannoli
#ifndef MAME_MACHINE_NFORCEPC_H
#define MAME_MACHINE_NFORCEPC_H

#pragma once

// NVIDIA Corporation nForce CPU bridge

class crush11_host_device : public pci_host_device {
public:
	template <typename T>
	crush11_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: crush11_host_device(mconfig, tag, owner, clock)
	{
		set_ids_host(0x10de01a4, 0x01, 0x10430c11);
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
	crush11_host_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { cpu.set_tag(std::forward<T>(tag)); }
	const char *get_cpu_tag() { return cpu.finder_tag(); }
	void set_ram_size(uint32_t size) { ram_size = size; }
	address_space *get_cpu_space(int spacenum) { return &cpu->space(spacenum); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
		uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

private:
	required_device<device_memory_interface> cpu;
	uint32_t ram_size;

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

#endif
