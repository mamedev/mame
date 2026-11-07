// license:BSD-3-Clause
// copyright-holders:Chris Toshok
/*
 * apollo_dn300_ring.h - the Apollo DN300/DN320 ring network controller
 */

#ifndef MAME_APOLLO_APOLLO_DN300_RING_H
#define MAME_APOLLO_APOLLO_DN300_RING_H

#pragma once

class apollo_dn300_ring_ctrlr_device :
	public device_t
{
public:
	//construction/destruction
	apollo_dn300_ring_ctrlr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// the controller's registers and DMA are there, but no network: what
	// it transmits goes nowhere and it never receives
	static constexpr feature_type unemulated_features() { return feature::LAN; }

	void map(address_map &map);

	// our dmac interface
	uint8_t rcv_header_read_byte(offs_t offset);
	void rcv_header_write_byte(offs_t offset, uint8_t data);

	uint8_t rcv_data_read_byte(offs_t offset);
	void rcv_data_write_byte(offs_t offset, uint8_t data);

	uint8_t transmit_read_byte(offs_t offset);
	void transmit_write_byte(offs_t offset, uint8_t data);

	auto irq_callback() { return irq_cb.bind(); }
	auto rcv_header_drq_wr_callback() { return rcv_header_drq_wr_cb.bind(); }
	auto rcv_data_drq_wr_callback() { return rcv_data_drq_wr_cb.bind(); }
	auto transmit_data_drq_wr_callback() { return transmit_data_drq_wr_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void xmit_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t xmit_status_r(offs_t offset, uint16_t mem_mask = ~0);

	void rcv_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rcv_status_r(offs_t offset, uint16_t mem_mask = ~0);

	void tmask_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t tmask_r(offs_t offset, uint8_t mem_mask = ~0);

	void diag_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t diag_status_r(offs_t offset, uint16_t mem_mask = ~0);

	void ring_id_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ring_id_r(offs_t offset, uint16_t mem_mask = ~0);

	uint8_t id_r(offs_t offset, uint8_t mem_mask = ~0);

private:
	devcb_write_line irq_cb;
	devcb_write_line rcv_header_drq_wr_cb;
	devcb_write_line rcv_data_drq_wr_cb;
	devcb_write_line transmit_data_drq_wr_cb;

	uint16_t m_xmit_status;
	uint16_t m_xmit_command;
	uint16_t m_rcv_status;
	uint16_t m_rcv_command;
	uint8_t m_tmask;
	uint16_t m_diag_status;
	uint16_t m_ring_id_msb;
	uint16_t m_ring_id_lsb;
	// 4 bytes of id, msb first
	uint8_t m_id[4];


	uint8_t m_buffer[2000];
	int m_cursor;
};

// device type definition
DECLARE_DEVICE_TYPE(APOLLO_DN300_RING_CTRLR, apollo_dn300_ring_ctrlr_device)

#endif // MAME_APOLLO_APOLLO_DN300_RING_H
