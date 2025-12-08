// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_PC9801_27_H
#define MAME_BUS_PC98_CBUS_PC9801_27_H

#pragma once

#include "slot.h"

#include "bus/nscsi/devices.h"
#include "bus/nscsi/dtc510.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"

class pc9801_27_device : public device_t
					   , public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	pc9801_27_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;
	virtual u8 dack_r(int line) override;
	virtual void dack_w(int line, u8 data) override;
	virtual void eop_w(int state) override;

private:
	required_device<nscsi_bus_device> m_sasibus;
	required_device<nscsi_callback_device> m_sasi;
	required_memory_region m_bios;
	required_ioport m_dsw;

	u8 data_r();
	void data_w(u8 data);

	void io_map(address_map &map) ATTR_COLD;

	u8 m_control;
	int m_irq_state;

	int m_sasi_req;
	int m_sasi_cd;
	int m_sasi_io;
	int m_sasi_msg;
	int m_sasi_ack;

	u8 m_sasi_data;

	void sasi_req_w(int state);
	void sasi_cd_w(int state);
	void sasi_io_w(int state);
	void sasi_msg_w(int state);
	void sasi_ack_w(int state);
	void sasi_bsy_w(int state);

	void update_irq();
	void update_drq();
};


DECLARE_DEVICE_TYPE(PC9801_27, pc9801_27_device)


#endif // MAME_BUS_PC98_CBUS_PC9801_27_H
