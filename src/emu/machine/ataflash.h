#pragma once

#ifndef __ATAFLASH_H__
#define __ATAFLASH_H__

#include "pccard.h"
#include "machine/idectrl.h"

extern const device_type ATA_FLASH_PCCARD;

class ata_flash_pccard_device : public device_t,
	public pccard_interface,
	public device_slot_card_interface
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
	virtual void device_reset_after_children();
	virtual machine_config_constructor device_mconfig_additions() const;

private:
	chd_file *m_chd_file;
	unsigned char m_cis[512];
	int m_locked;
	required_device<ide_controller_device> m_card;
};

#endif
