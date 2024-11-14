// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atadev.h

    ATA Device implementation.

***************************************************************************/

#ifndef MAME_BUS_ATA_ATADEV_H
#define MAME_BUS_ATA_ATADEV_H

#pragma once

class device_ata_interface;


class ata_slot_device :
		public device_t,
		public device_single_card_slot_interface<device_ata_interface>
{
public:
	// construction/destruction
	ata_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_handler() { return m_irq_handler.bind(); }
	auto dmarq_handler() { return m_dmarq_handler.bind(); }
	auto dasp_handler() { return m_dasp_handler.bind(); }
	auto pdiag_handler() { return m_pdiag_handler.bind(); }

	device_ata_interface *dev() { return m_dev; }

protected:
	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_dmarq_handler;
	devcb_write_line m_dasp_handler;
	devcb_write_line m_pdiag_handler;

	device_ata_interface *m_dev;

	friend class device_ata_interface;
};


class device_ata_interface : public device_interface
{
public:
	virtual uint16_t read_dma() = 0;
	virtual uint16_t read_cs0(offs_t offset, uint16_t mem_mask = 0xffff) = 0;
	virtual uint16_t read_cs1(offs_t offset, uint16_t mem_mask = 0xffff) = 0;

	virtual void write_dma(uint16_t data) = 0;
	virtual void write_cs0(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) = 0;
	virtual void write_cs1(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) = 0;

	virtual void write_dmack(int state) = 0;
	virtual void write_csel(int state) = 0;
	virtual void write_dasp(int state) = 0;
	virtual void write_pdiag(int state) = 0;

protected:
	device_ata_interface(const machine_config &mconfig, device_t &device);

	void set_irq(int state) { m_slot->m_irq_handler(state); }
	void set_dmarq(int state) { m_slot->m_dmarq_handler(state); }
	void set_dasp(int state) { m_slot->m_dasp_handler(state); }
	void set_pdiag(int state) { m_slot->m_pdiag_handler(state); }

private:
	ata_slot_device *const m_slot;
};


// device type declaration
DECLARE_DEVICE_TYPE(ATA_SLOT, ata_slot_device)

void ata_devices(device_slot_interface &device);

#endif // MAME_BUS_ATA_ATADEV_H
