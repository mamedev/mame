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
	ata_flash_pccard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual DECLARE_READ16_MEMBER(read_memory) override;
	virtual DECLARE_WRITE16_MEMBER(write_memory) override;
	virtual DECLARE_READ16_MEMBER(read_reg) override;
	virtual DECLARE_WRITE16_MEMBER(write_reg) override;

protected:
	// device-level overrides
	virtual void device_reset() override;

	virtual attotime seek_time() override;

private:
	uint8_t m_cis[512];
	uint8_t m_configuration_option;
	uint8_t m_configuration_and_status;
	uint8_t m_pin_replacement;
};

extern const device_type TAITO_PCCARD1;

class taito_pccard1_device : public ata_flash_pccard_device
{
public:
	taito_pccard1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ16_MEMBER(read_reg) override;
	virtual DECLARE_WRITE16_MEMBER(write_reg) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void process_command() override;
	virtual bool is_ready() override;

private:
	uint8_t m_key[5];
	uint16_t m_locked;
};

extern const device_type TAITO_PCCARD2;

class taito_pccard2_device : public ata_flash_pccard_device
{
public:
	taito_pccard2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void process_command() override;
	virtual void process_buffer() override;
	virtual bool is_ready() override;

	static const int IDE_COMMAND_TAITO_GNET_UNLOCK_1 = 0xfe;
	static const int IDE_COMMAND_TAITO_GNET_UNLOCK_2 = 0xfc;

private:
	uint8_t m_key[5];
	bool m_locked;
};

extern const device_type TAITO_COMPACT_FLASH;

class taito_compact_flash_device : public ata_flash_pccard_device
{
public:
	taito_compact_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void process_command() override;
	virtual bool is_ready() override;

	static const int IDE_COMMAND_TAITO_COMPACT_FLASH_UNLOCK = 0x0f;

private:
	uint8_t m_key[5];
	bool m_locked;
};

#endif
