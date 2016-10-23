// license:BSD-3-Clause
// copyright-holders:smf
#pragma once

#ifndef __ATAFLASH_H__
#define __ATAFLASH_H__

#include "pccard.h"
#include "machine/idehd.h"

extern const device_type ATA_FLASH_PCCARD;

class ata_flash_pccard_device : public ide_hdd_device,
	public pccard_interface
{
public:
	ata_flash_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read_memory(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual void write_memory(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override;
	virtual uint16_t read_reg(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) override;
	virtual void write_reg(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual attotime seek_time() override;
	virtual void process_command() override;
	virtual void process_buffer() override;
	virtual bool is_ready() override;

private:
	uint8_t m_cis[512];
	uint8_t m_key[5];
	uint8_t m_gnetreadlock;
	int m_locked;
};

#endif
