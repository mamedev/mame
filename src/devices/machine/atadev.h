// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atadev.h

    ATA Device implementation.

***************************************************************************/

#pragma once

#ifndef __ATADEV_H__
#define __ATADEV_H__

#include "emu.h"

// ======================> ata_device_interface

class ata_device_interface
{
public:
	ata_device_interface(const machine_config &mconfig, device_t &device);
	virtual ~ata_device_interface() {}

	virtual uint16_t read_dma() = 0;
	virtual uint16_t read_cs0(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) = 0;
	virtual uint16_t read_cs1(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) = 0;

	virtual void write_dma(uint16_t data) = 0;
	virtual void write_cs0(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) = 0;
	virtual void write_cs1(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) = 0;
	virtual void write_dmack(int state) = 0;
	virtual void write_csel(int state) = 0;
	virtual void write_dasp(int state) = 0;
	virtual void write_pdiag(int state) = 0;

	devcb_write_line m_irq_handler;
	devcb_write_line m_dmarq_handler;
	devcb_write_line m_dasp_handler;
	devcb_write_line m_pdiag_handler;
};

#endif
