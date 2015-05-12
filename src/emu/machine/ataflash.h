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
	ata_flash_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ16_MEMBER(read_memory);
	virtual DECLARE_WRITE16_MEMBER(write_memory);
	virtual DECLARE_READ16_MEMBER(read_reg);
	virtual DECLARE_WRITE16_MEMBER(write_reg);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual attotime seek_time();
	virtual void process_command();
	virtual void process_buffer();
	virtual bool is_ready();

private:
	UINT8 m_cis[512];
	UINT8 m_key[5];
	UINT8 m_gnetreadlock;
	int m_locked;
};

#endif
