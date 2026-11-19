// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    VIA VT8231 South Bridge - ACPI

***************************************************************************/

#ifndef MAME_MACHINE_VT8231_ACPI_H
#define MAME_MACHINE_VT8231_ACPI_H

#pragma once

#include "pci.h"
#include "machine/clock.h"


class vt8231_acpi_device : public pci_device
{
public:
	vt8231_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto smbck1_cb() { return m_smbck1_cb.bind(); }
	auto smbdt1_cb() { return m_smbdt1_cb.bind(); }

	void smbdt1_w(int state) { m_smbus_dt1 = state; };

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	enum : uint8_t
	{
		SMBUS_STATE_IDLE = 0,
		SMBUS_STATE_SEND_ADDRESS_W,
		SMBUS_STATE_WAIT_ADDRESS_W_ACK,
		SMBUS_STATE_SEND_CMD,
		SMBUS_STATE_WAIT_CMD_ACK,
		SMBUS_STATE_REPEAT_START1,
		SMBUS_STATE_REPEAT_START2,
		SMBUS_STATE_SEND_ADDRESS_R,
		SMBUS_STATE_WAIT_ADDRESS_R_ACK,
		SMBUS_STATE_BYTE_READ,
		SMBUS_STATE_SEND_NACK,
		SMBUS_STATE_SEND_STOP1,
		SMBUS_STATE_SEND_STOP2
	};

	enum : uint8_t
	{
		SMBUS_CMD_QUICK_RW = 0,
		SMBUS_CMD_BYTE_RW = 1,
		SMBUS_CMD_BYTE_DATA_RW = 2,
		SMBUS_CMD_WORD_DATA_RW = 3,
		SMBUS_CMD_BLOCK_RW = 5
	};

	uint8_t general_configuration_0_r();
	void general_configuration_0_w(uint8_t data);
	void power_well_control_w(uint8_t data);

	void smbus_map(address_map &map);
	uint32_t smbus_iobase_r(offs_t offset, uint32_t mem_mask);
	void smbus_iobase_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint8_t smbus_host_config_r();
	void smbus_host_config_w(uint8_t data);
	uint8_t smbus_host_status_r();
	void smbus_host_status_w(uint8_t data);
	uint8_t smbus_host_control_r();
	void smbus_host_control_w(uint8_t data);
	uint8_t smbus_host_command_r();
	void smbus_host_command_w(uint8_t data);
	uint8_t smbus_host_address_r();
	void smbus_host_address_w(uint8_t data);
	uint8_t smbus_host_data0_r();
	void smbus_host_data0_w(uint8_t data);
	void smbus_clock_w(int state);

	required_device<clock_device> m_smbus_clock;

	devcb_write_line m_smbck1_cb;
	devcb_write_line m_smbdt1_cb;

	uint8_t m_general_configuration_0;
	uint8_t m_power_well_control;

	uint32_t m_smbus_iobase;
	uint32_t m_smbus_iobase_mapped;
	uint8_t m_smbus_host_config;
	uint8_t m_smbus_host_status;
	uint8_t m_smbus_host_control;
	uint8_t m_smbus_host_command;
	uint8_t m_smbus_host_command_latch;
	uint8_t m_smbus_host_address;
	uint8_t m_smbus_host_address_latch;
	uint8_t m_smbus_host_data0;
	uint8_t m_smbus_host_data0_latch;

	uint8_t m_smbus_state;
	uint8_t m_smbus_bit;
	bool m_smbus_dt1;

	bool m_initialized;
};


// device type definition
DECLARE_DEVICE_TYPE(VT8231_ACPI, vt8231_acpi_device)

#endif // MAME_MACHINE_VT8231_ACPI_H
