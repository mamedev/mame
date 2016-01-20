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
	ata_flash_pccard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER(read_memory) override;
	virtual DECLARE_WRITE16_MEMBER(write_memory) override;
	virtual DECLARE_READ16_MEMBER(read_reg) override;
	virtual DECLARE_WRITE16_MEMBER(write_reg) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual attotime seek_time() override;
	virtual void process_command() override;
	virtual void process_buffer() override;
	virtual bool is_ready() override;

private:
	UINT8 m_cis[512];
	UINT8 m_key[5];
	UINT8 m_gnetreadlock;
	int m_locked;
};

#endif
