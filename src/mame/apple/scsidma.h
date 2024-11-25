// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_SCSIDMA_H
#define MAME_APPLE_SCSIDMA_H

#pragma once

#include "cpu/m68000/m68030.h"
#include "machine/ncr5380.h"

class scsidma_device :  public device_t
{
public:
	// construction/destruction
	scsidma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }

	auto write_irq() { return m_irq.bind(); }

	void map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<m68000_musashi_device> m_maincpu;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c80_device> m_ncr;

	devcb_write_line m_irq;

private:
	void scsi_irq_w(int state);
	void scsi_drq_w(int state);

	u8 scsi_r(offs_t offset);
	void scsi_w(offs_t offset, u8 data);
	u32 control_r();
	void control_w(u32 data);
	u32 handshake_r(offs_t offset, u32 mem_mask);
	void handshake_w(offs_t offset, u32 data, u32 mem_mask);

	s32 m_drq, m_scsi_irq;
	u32 m_control;
	u32 m_holding;
	u8 m_holding_remaining;
	bool m_is_write, m_drq_completed;
};

// device type definition
DECLARE_DEVICE_TYPE(SCSIDMA, scsidma_device)

#endif // MAME_APPLE_SCSIDMA_H
