// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec AHA-1540/42A and AHA-1540/42B SCSI controllers

***************************************************************************/

#ifndef MAME_BUS_ISA_AHA1542B_H
#define MAME_BUS_ISA_AHA1542B_H

#pragma once

#include "isa.h"
#include "machine/aic565.h"
#include "machine/aic6250.h"
#include "machine/upd765.h"

class aha154x_device : public device_t, public device_isa16_card_interface
{
protected:
	aha154x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	void transfer_speed_w(u8 data);
	void dma_mode_w(u8 data);
	void bus_on_time_w(u8 data);
	void bus_off_time_w(u8 data);
	u8 fifo_data_r();
	void fifo_data_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(aic_breq_w);

	void i8085_base_map(address_map &map);
	void scsi_add(machine_config &config);
	void scsic_config(device_t *device);

	required_device<cpu_device> m_localcpu;
	required_device<aic6250_device> m_scsic;
	required_device<upd765_family_device> m_fdc;
	required_region_ptr<u8> m_bios;
	required_shared_ptr<u8> m_fifo_data;

	u8 m_fifo_read_index;
	u8 m_fifo_write_index;
	u8 m_dma_mode;
};

class aha1542a_device : public aha154x_device
{
public:
	aha1542a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

	DECLARE_READ_LINE_MEMBER(host_int_r);
	DECLARE_READ_LINE_MEMBER(scsi_rstreq_r);

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void local_status_w(u8 data);
	void int_status_w(u8 data);
	void srst_clear_w(u8 data);
	void scsi_rstreq_clear_w(u8 data);

	void i8085_map(address_map &map);
};

class aha1542b_device : public aha154x_device
{
public:
	aha1542b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void i8085_map(address_map &map);

	required_device<aic565_device> m_busaic;
};

DECLARE_DEVICE_TYPE(AHA1542A, aha1542a_device)
DECLARE_DEVICE_TYPE(AHA1542B, aha1542b_device)

#endif // MAME_BUS_ISA_AHA1542B_H
