// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Tube ULA emulation

**********************************************************************/

#ifndef MAME_MACHINE_TUBE_H
#define MAME_MACHINE_TUBE_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> tube_device

class tube_device : public device_t
{
public:
	// construction/destruction
	tube_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto hirq_handler() { return m_hirq_handler.bind(); }
	auto pnmi_handler() { return m_pnmi_handler.bind(); }
	auto pirq_handler() { return m_pirq_handler.bind(); }
	auto prst_handler() { return m_prst_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }

	uint8_t host_r(offs_t offset);
	void host_w(offs_t offset, uint8_t data);
	uint8_t parasite_r(offs_t offset);
	void parasite_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t m_ph1[24];
	uint8_t m_ph2;
	uint8_t m_ph3[2];
	uint8_t m_ph4;
	uint8_t m_hp1;
	uint8_t m_hp2;
	uint8_t m_hp3[2];
	uint8_t m_hp4;
	uint8_t m_hstat[4];
	uint8_t m_pstat[4];
	uint8_t m_r1stat;
	int m_ph1pos;
	int m_ph3pos;
	int m_hp3pos;

	void soft_reset();
	void update_interrupts();

	devcb_write_line m_hirq_handler;
	devcb_write_line m_pnmi_handler;
	devcb_write_line m_pirq_handler;
	devcb_write_line m_prst_handler;
	devcb_write_line m_drq_handler;
};


// device type definition
DECLARE_DEVICE_TYPE(TUBE, tube_device)

#endif // MAME_MACHINE_TUBE_H
