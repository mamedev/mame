// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atadev.h

    ATA Device implementation.

***************************************************************************/

#ifndef MAME_MACHINE_ATADEV_H
#define MAME_MACHINE_ATADEV_H

#pragma once


// ======================> device_ata_interface

class ata_interface_device;

class device_ata_interface : public device_slot_card_interface
{
	friend class ata_interface_device;
public:
	device_ata_interface(const machine_config &mconfig, device_t &device);

	virtual uint16_t read_dma() = 0;
	virtual DECLARE_READ16_MEMBER(read_cs0) = 0;
	virtual DECLARE_READ16_MEMBER(read_cs1) = 0;

	virtual void write_dma(uint16_t data) = 0;
	virtual DECLARE_WRITE16_MEMBER(write_cs0) = 0;
	virtual DECLARE_WRITE16_MEMBER(write_cs1) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_dmack) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_csel) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_dasp) = 0;
	virtual DECLARE_WRITE_LINE_MEMBER(write_pdiag) = 0;

protected:
	devcb_write_line m_irq_handler;
	devcb_write_line m_dmarq_handler;
	devcb_write_line m_dasp_handler;
	devcb_write_line m_pdiag_handler;
};

#endif // MAME_MACHINE_ATADEV_H
