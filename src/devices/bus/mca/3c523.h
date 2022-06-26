// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    3COM 3C523 EtherLink/MC

    i82586-based LAN card.
	Skeleton.

***************************************************************************/

#ifndef MAME_BUS_MCA_3C523_H
#define MAME_BUS_MCA_3C523_H

#pragma once

#include "mca.h"
#include "machine/i82586.h"
#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_com_device

class mca16_3c523_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_3c523_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void remap() override;
	virtual void unmap() override;

	virtual uint8_t io8_r(offs_t offset) override;
	virtual void io8_w(offs_t offset, uint8_t data) override;

	uint8_t shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, uint8_t data);

protected:
	mca16_3c523_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

    void map_main(address_map &map);

	bool map_has_changed() override;

	void update_pos_data_1(uint8_t data) override;
	void update_pos_data_2(uint8_t data) override;
	void update_pos_data_3(uint8_t data) override;
	void update_pos_data_4(uint8_t data) override;

	uint16_t packet_buffer_r(offs_t offset);
	void packet_buffer_w(offs_t offset, uint16_t data);

	required_device<i82586_device> m_net;
	required_device<ram_device> m_ram;

	DECLARE_WRITE_LINE_MEMBER(irq_w);

private:
	uint8_t 	control_register_r();
	void 		control_register_w(uint8_t data);

	void		update_reset();

	uint16_t	get_pos_io_base();
	uint8_t 	get_pos_irq();
	offs_t 		get_pos_mem_base();

	uint16_t 	m_cur_io_base;
	uint8_t 	m_cur_irq;
	offs_t		m_cur_mem_base;

	bool		m_reset;
	bool		m_channel_attention;
	bool		m_loopback_enabled;
	bool		m_interrupt_flag;		// Not affected by INTE.
	bool		m_interrupt_fired;		// Is an IRQ *actually* fired?
	bool		m_interrupts_enabled;
	uint8_t		m_ram_bank;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_3C523, mca16_3c523_device)

#endif