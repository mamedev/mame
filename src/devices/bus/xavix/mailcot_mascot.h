// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_BUS_XAVIX_MAILCOT_MASCOT_H
#define MAME_BUS_XAVIX_MAILCOT_MASCOT_H

#pragma once

#include "imagedev/memcard.h"
#include "machine/i2cmem.h"

class mailcot_mascot_eeprom_device : public i2cmem_device
{
public:
	enum class model : u8 { C16, C128, C32 };

	mailcot_mascot_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	mailcot_mascot_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, model type);

	bool load_image(util::read_stream &file);
	void clear();
	void initialize() { m_initialized = true; }
	static int capacity(model type);

protected:
	virtual void nvram_default() override;

private:
	static int page_size(model type);
	bool m_initialized;
};

class mailcot_mascot_card_device : public device_t
{
public:
	u8 character() const { return u8(m_model) * 4 + m_id; }
	int capacity() const { return mailcot_mascot_eeprom_device::capacity(m_model); }
	int sda_r() { return m_eeprom->read_sda(); }
	void write(int sda, int scl);
	void reset_bus() { m_eeprom->reset_bus(); }
	void clear() { m_eeprom->clear(); }
	void initialize() { m_eeprom->initialize(); }
	bool load_image(util::read_stream &file) { return m_eeprom->load_image(file); }
	bool save_image(util::write_stream &file) { return m_eeprom->nvram_save(file); }

protected:
	mailcot_mascot_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, mailcot_mascot_eeprom_device::model model, u8 id);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<mailcot_mascot_eeprom_device> m_eeprom;
	mailcot_mascot_eeprom_device::model const m_model;
	u8 const m_id;
};

template <u8 Character>
class mailcot_mascot_device : public mailcot_mascot_card_device
{
public:
	mailcot_mascot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

private:
	static device_type const TYPE;
};

class mailcot_mascot_slot_device : public device_t,
		public device_single_card_slot_interface<mailcot_mascot_card_device>,
		public device_memcard_image_interface
{
public:
	mailcot_mascot_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	int sda_r() { return connected() ? m_card->sda_r() : 1; }
	u8 id_r() { return connected() ? m_card->character() & 3 : 3; }
	void write(int sda, int scl);

	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_type_name() const noexcept override { return "mascot_memory"; }
	virtual const char *image_brief_type_name() const noexcept override { return "masc"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_pre_save() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;

private:
	bool connected() const { return m_card && m_loaded && !m_inhibited; }
	void insert();

	mailcot_mascot_card_device *m_card;
	bool m_loaded;
	bool m_inhibited;
	bool m_saved_present;
	u8 m_saved_character;
	bool m_host_sda;
	bool m_host_scl;
};

DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_SLOT, mailcot_mascot_slot_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_EEPROM, mailcot_mascot_eeprom_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_CHARURU, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_KURAU, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_PETCHI, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_CLOUDS, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_CHARURU_PINK, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_KURAU_BLUE, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_KURUMIGUMA, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_CLOUDS_16K, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_PUCCHI, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_KURAU_GREEN, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_CHUTA, mailcot_mascot_card_device)
DECLARE_DEVICE_TYPE(MAILCOT_MASCOT_UPYO, mailcot_mascot_card_device)

#endif // MAME_BUS_XAVIX_MAILCOT_MASCOT_H
