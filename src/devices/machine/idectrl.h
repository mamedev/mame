// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    idectrl.h

    Generic (PC-style) IDE controller implementation.

***************************************************************************/

#ifndef MAME_MACHINE_IDECTRL_H
#define MAME_MACHINE_IDECTRL_H

#pragma once

#include "bus/ata/ataintf.h"

class ide_controller_device : public abstract_ata_interface_device
{
public:
	ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> ide_controller_device &master(T &&opts, const char *dflt = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::master(std::forward<T>(opts), dflt, fixed);
		return *this;
	}
	template <typename T> ide_controller_device &slave(T &&opts, const char *dflt = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::slave(std::forward<T>(opts), dflt, fixed);
		return *this;
	}
	template <typename T> ide_controller_device &options(T &&opts, const char *master_default = nullptr, const char *slave_default = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::options(std::forward<T>(opts), master_default, slave_default, fixed);
		return *this;
	}
	template <typename T> ide_controller_device &set_slot_options(int index, T &&opts, const char *dflt, bool fixed)
	{
		abstract_ata_interface_device::set_slot_options(index, std::forward<T>(opts), dflt, fixed);
		return *this;
	}

	uint16_t read_cs0(offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t read_cs1(offs_t offset, uint16_t mem_mask = 0xffff);
	void write_cs0(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void write_cs1(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t cs0_r(offs_t offset, uint16_t mem_mask = ~0) { return read_cs0(offset, mem_mask); }
	uint16_t cs1_r(offs_t offset, uint16_t mem_mask = ~0) { return read_cs1(offset, mem_mask); }
	void cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { write_cs0(offset, data, mem_mask); }
	void cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { write_cs1(offset, data, mem_mask); }

protected:
	ide_controller_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(IDE_CONTROLLER, ide_controller_device)


class ide_controller_32_device : public abstract_ata_interface_device
{
public:
	ide_controller_32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> ide_controller_32_device &master(T &&opts, const char *dflt = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::master(std::forward<T>(opts), dflt, fixed);
		return *this;
	}
	template <typename T> ide_controller_32_device &slave(T &&opts, const char *dflt = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::slave(std::forward<T>(opts), dflt, fixed);
		return *this;
	}
	template <typename T> ide_controller_32_device &options(T &&opts, const char *master_default = nullptr, const char *slave_default = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::options(std::forward<T>(opts), master_default, slave_default, fixed);
		return *this;
	}
	template <typename T> ide_controller_32_device &set_slot_options(int index, T &&opts, const char *dflt, bool fixed)
	{
		abstract_ata_interface_device::set_slot_options(index, std::forward<T>(opts), dflt, fixed);
		return *this;
	}

	uint32_t read_cs0(offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t read_cs1(offs_t offset, uint32_t mem_mask = 0xffffffff);
	void write_cs0(offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void write_cs1(offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint32_t cs0_r(offs_t offset, uint32_t mem_mask = ~0) { return read_cs0(offset, mem_mask); }
	uint32_t cs1_r(offs_t offset, uint32_t mem_mask = ~0) { return read_cs1(offset, mem_mask); }
	void cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { write_cs0(offset, data, mem_mask); }
	void cs1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) { write_cs1(offset, data, mem_mask); }

protected:
	ide_controller_32_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(IDE_CONTROLLER_32, ide_controller_32_device)


class bus_master_ide_controller_device : public ide_controller_32_device
{
public:
	bus_master_ide_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	template <typename... T> void set_bus_master_space(T &&... args) { m_dma_space.set_tag(std::forward<T>(args)...); }
	template <bool R> void set_bus_master_space(const address_space_finder<R> &finder) { m_dma_space.set_tag(finder); }

	template <typename T> bus_master_ide_controller_device &master(T &&opts, const char *dflt = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::master(std::forward<T>(opts), dflt, fixed);
		return *this;
	}
	template <typename T> bus_master_ide_controller_device &slave(T &&opts, const char *dflt = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::slave(std::forward<T>(opts), dflt, fixed);
		return *this;
	}
	template <typename T> bus_master_ide_controller_device &options(T &&opts, const char *master_default = nullptr, const char *slave_default = nullptr, bool fixed = false)
	{
		abstract_ata_interface_device::options(std::forward<T>(opts), master_default, slave_default, fixed);
		return *this;
	}
	template <typename T> bus_master_ide_controller_device &set_slot_options(int index, T &&opts, const char *dflt, bool fixed)
	{
		abstract_ata_interface_device::set_slot_options(index, std::forward<T>(opts), dflt, fixed);
		return *this;
	}

	uint32_t bmdma_r(offs_t offset, uint32_t mem_mask = ~0);
	void bmdma_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void set_irq(int state) override;
	virtual void set_dmarq(int state) override;

private:
	void execute_dma();

	required_address_space m_dma_space;
	uint8_t m_dma_address_xor;

	offs_t m_dma_address;
	uint32_t m_dma_bytes_left;
	offs_t m_dma_descriptor;
	uint8_t m_dma_last_buffer;
	uint8_t m_bus_master_command;
	uint8_t m_bus_master_status;
	uint32_t m_bus_master_descriptor;
	int m_irq;
	int m_dmarq;
};

DECLARE_DEVICE_TYPE(BUS_MASTER_IDE_CONTROLLER, bus_master_ide_controller_device)

#endif // MAME_MACHINE_IDECTRL_H
