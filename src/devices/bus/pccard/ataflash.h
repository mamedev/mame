// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_ATAFLASH_H
#define MAME_MACHINE_ATAFLASH_H

#pragma once

#include "pccard.h"
#include "machine/atastorage.h"

class ata_flash_pccard_device : public cf_device_base, public device_pccard_interface
{
public:
	ata_flash_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;
	virtual uint16_t read_reg(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void write_reg(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	ata_flash_pccard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// ata_hle_device_base implementation
	virtual void ide_build_identify_device() override;
	virtual void set_irq_out(int state) override { }
	virtual void set_dmarq_out(int state) override { }
	virtual void set_dasp_out(int state) override { }
	virtual void set_pdiag_out(int state) override { }

	std::vector<uint8_t> m_cis;
	uint8_t m_configuration_option;
	uint8_t m_configuration_and_status;
	uint8_t m_pin_replacement;
};

class taito_pccard1_device : public ata_flash_pccard_device
{
public:
	taito_pccard1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t read_reg(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void write_reg(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void process_command() override;
	virtual bool is_ready() override;

private:
	std::vector<uint8_t> m_key;
	uint16_t m_locked;
};

class taito_pccard2_device : public ata_flash_pccard_device
{
public:
	taito_pccard2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void process_command() override;
	virtual void process_buffer() override;
	virtual bool is_ready() override;

	static const int IDE_COMMAND_TAITO_GNET_UNLOCK_1 = 0xfe;
	static const int IDE_COMMAND_TAITO_GNET_UNLOCK_2 = 0xfc;

private:
	std::vector<uint8_t> m_key;
	bool m_locked;
};

class taito_compact_flash_device : public ata_flash_pccard_device
{
public:
	taito_compact_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void process_command() override;
	virtual bool is_ready() override;

	static constexpr int IDE_COMMAND_TAITO_COMPACT_FLASH_UNLOCK = 0x0f;

private:
	std::vector<uint8_t> m_key;
	bool m_locked;
};

DECLARE_DEVICE_TYPE(TAITO_PCCARD1, taito_pccard1_device)
DECLARE_DEVICE_TYPE(TAITO_PCCARD2, taito_pccard2_device)
DECLARE_DEVICE_TYPE(TAITO_COMPACT_FLASH, taito_compact_flash_device)
DECLARE_DEVICE_TYPE(ATA_FLASH_PCCARD, ata_flash_pccard_device)

#endif // MAME_MACHINE_ATAFLASH_H
