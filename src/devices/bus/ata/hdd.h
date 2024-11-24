// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    hdd.h

    IDE hard disk on ATA bus

***************************************************************************/

#ifndef MAME_BUS_ATA_HDD_H
#define MAME_BUS_ATA_HDD_H

#pragma once

#include "atadev.h"

#include "machine/atastorage.h"
#include "imagedev/harddriv.h"

#include "harddisk.h"


// ======================> ide_hdd_device

class ide_hdd_device : public ide_hdd_device_base, public device_ata_interface
{
public:
	// construction/destruction
	ide_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_ata_interface implementation
	virtual uint16_t read_dma() override { return dma_r(); }
	virtual uint16_t read_cs0(offs_t offset, uint16_t mem_mask) override { return command_r(offset); }
	virtual uint16_t read_cs1(offs_t offset, uint16_t mem_mask) override { return control_r(offset); }

	virtual void write_dma(uint16_t data) override { dma_w(data); }
	virtual void write_cs0(offs_t offset, uint16_t data, uint16_t mem_mask) override { command_w(offset, data); }
	virtual void write_cs1(offs_t offset, uint16_t data, uint16_t mem_mask) override { control_w(offset, data); }

	virtual void write_dmack(int state) override { set_dmack_in(state); }
	virtual void write_csel(int state) override { set_csel_in(state); }
	virtual void write_dasp(int state) override { set_dasp_in(state); }
	virtual void write_pdiag(int state) override { set_pdiag_in(state); }

protected:
	ide_hdd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

private:
	// ata_hle_device_base implementation
	virtual void set_irq_out(int state) override { device_ata_interface::set_irq(state); }
	virtual void set_dmarq_out(int state) override { device_ata_interface::set_dmarq(state); }
	virtual void set_dasp_out(int state) override { device_ata_interface::set_dasp(state); }
	virtual void set_pdiag_out(int state) override { device_ata_interface::set_pdiag(state); }
};


// ======================> ata_cf_device

class ata_cf_device : public cf_device_base, public device_ata_interface
{
public:
	// construction/destruction
	ata_cf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_ata_interface implementation
	virtual uint16_t read_dma() override { return dma_r(); }
	virtual uint16_t read_cs0(offs_t offset, uint16_t mem_mask) override { return command_r(offset); }
	virtual uint16_t read_cs1(offs_t offset, uint16_t mem_mask) override { return control_r(offset); }

	virtual void write_dma(uint16_t data) override { dma_w(data); }
	virtual void write_cs0(offs_t offset, uint16_t data, uint16_t mem_mask) override { command_w(offset, data); }
	virtual void write_cs1(offs_t offset, uint16_t data, uint16_t mem_mask) override { control_w(offset, data); }

	virtual void write_dmack(int state) override { set_dmack_in(state); }
	virtual void write_csel(int state) override { set_csel_in(state); }
	virtual void write_dasp(int state) override { set_dasp_in(state); }
	virtual void write_pdiag(int state) override { set_pdiag_in(state); }

private:
	// ata_hle_device_base implementation
	virtual void set_irq_out(int state) override { device_ata_interface::set_irq(state); }
	virtual void set_dmarq_out(int state) override { device_ata_interface::set_dmarq(state); }
	virtual void set_dasp_out(int state) override { device_ata_interface::set_dasp(state); }
	virtual void set_pdiag_out(int state) override { device_ata_interface::set_pdiag(state); }
};

// device type declaration
DECLARE_DEVICE_TYPE(IDE_HARDDISK, ide_hdd_device)
DECLARE_DEVICE_TYPE(ATA_CF, ata_cf_device)

#endif // MAME_BUS_ATA_HDD_H
