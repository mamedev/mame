#pragma once

#ifndef __ATAFLASH_H__
#define __ATAFLASH_H__

#include "pccard.h"
#include "machine/idectrl.h"

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
	virtual void device_reset();

private:
	unsigned char m_cis[512];
	int m_locked;
};

#endif
