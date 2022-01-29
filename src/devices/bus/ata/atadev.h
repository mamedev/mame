// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atadev.h

    ATA Device implementation.

***************************************************************************/

#ifndef MAME_BUS_ATA_ATADEV_H
#define MAME_BUS_ATA_ATADEV_H

#pragma once


// ======================> device_ata_interface

class ata_interface_device;

class device_ata_interface : public device_interface
{
	friend class abstract_ata_interface_device;
public:
	virtual uint16_t read_dma() = 0;
	virtual uint16_t read_cs0(offs_t offset, uint16_t mem_mask = 0xffff) = 0;
	virtual uint16_t read_cs1(offs_t offset, uint16_t mem_mask = 0xffff) = 0;

	virtual void write_dma(uint16_t data) = 0;
	virtual void write_cs0(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) = 0;
	virtual void write_cs1(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) = 0;

	virtual DECLARE_WRITE_LINE_MEMBER(write_dmack) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_csel) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_dasp) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_pdiag) = 0;

protected:
	device_ata_interface(const machine_config &mconfig, device_t &device);

	devcb_write_line m_irq_handler;
	devcb_write_line m_dmarq_handler;
	devcb_write_line m_dasp_handler;
	devcb_write_line m_pdiag_handler;
};

// ======================> ata_slot_device

class ata_slot_device : public device_t,
						public device_single_card_slot_interface<device_ata_interface>
{
public:
	// construction/destruction
	ata_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	device_ata_interface *dev() { return m_dev; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

private:
	device_ata_interface *m_dev;
};

// device type definition
DECLARE_DEVICE_TYPE(ATA_SLOT, ata_slot_device)

void ata_devices(device_slot_interface &device);

#endif // MAME_BUS_ATA_ATADEV_H
