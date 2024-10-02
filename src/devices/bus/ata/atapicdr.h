// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    atapicdr.h

    ATAPI CDROM/DVDROM

***************************************************************************/

#ifndef MAME_BUS_ATA_ATAPICDR_H
#define MAME_BUS_ATA_ATAPICDR_H

#pragma once

#include "atadev.h"
#include "atapihle.h"

#include "machine/t10mmc.h"


class atapi_cdrom_device : public atapi_hle_device, public device_ata_interface, public t10mmc
{
public:
	atapi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_ultra_dma_mode(uint16_t mode);

	uint16_t *identify_device_buffer() { return m_identify_buffer; }

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
	atapi_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void perform_diagnostic() override;
	virtual void identify_packet_device() override;
	virtual void process_buffer() override;
	virtual void ExecCommand() override;
	u32 m_sequence_counter;
	bool m_media_change;
	uint16_t m_ultra_dma_mode;

private:
	// ata_hle_device_base implementation
	virtual void set_irq_out(int state) override { device_ata_interface::set_irq(state); }
	virtual void set_dmarq_out(int state) override { device_ata_interface::set_dmarq(state); }
	virtual void set_dasp_out(int state) override { device_ata_interface::set_dasp(state); }
	virtual void set_pdiag_out(int state) override { device_ata_interface::set_pdiag(state); }
};

class atapi_fixed_cdrom_device : public atapi_cdrom_device
{
public:
	atapi_fixed_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	atapi_fixed_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_reset() override ATTR_COLD;
};

class atapi_dvdrom_device : public atapi_cdrom_device
{
public:
	atapi_dvdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;
};

class atapi_fixed_dvdrom_device : public atapi_cdrom_device
{
public:
	atapi_fixed_dvdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(ATAPI_CDROM,        atapi_cdrom_device)
DECLARE_DEVICE_TYPE(ATAPI_FIXED_CDROM,  atapi_fixed_cdrom_device)
DECLARE_DEVICE_TYPE(ATAPI_DVDROM,       atapi_dvdrom_device)
DECLARE_DEVICE_TYPE(ATAPI_FIXED_DVDROM, atapi_fixed_dvdrom_device)

#endif // MAME_BUS_ATA_ATAPICDR_H
