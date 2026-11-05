// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Mail*cot mascots contain an I2C EEPROM and two identity straps. The ROM
    detects presence by ACK and combines capacity and straps to select a
    character. Each slot option fixes the physical EEPROM and strap wiring;
    its raw memory image can be inserted and removed without resetting.

***************************************************************************/

#include "emu.h"
#include "mailcot_mascot.h"

DEFINE_DEVICE_TYPE(MAILCOT_MASCOT_SLOT, mailcot_mascot_slot_device, "mailcot_mascot_slot", "Mail*cot Mascot Connector")
DEFINE_DEVICE_TYPE(MAILCOT_MASCOT_EEPROM, mailcot_mascot_eeprom_device, "mailcot_mascot_eeprom", "Mail*cot Mascot EEPROM")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_CHARURU, mailcot_mascot_card_device, mailcot_mascot_device<0>, "mailcot_mascot_charuru", "Mail*cot Charuru / \u3061\u3083\u308b\u308b")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_KURAU, mailcot_mascot_card_device, mailcot_mascot_device<1>, "mailcot_mascot_kurau", "Mail*cot Kurau / \u30af\u30e9\u30a6")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_PETCHI, mailcot_mascot_card_device, mailcot_mascot_device<2>, "mailcot_mascot_petchi", "Mail*cot Petchi / \u307a\u3063\u3061")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_CLOUDS, mailcot_mascot_card_device, mailcot_mascot_device<3>, "mailcot_mascot_clouds", "Mail*cot Cloud & Stars (ID 3)")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_CHARURU_PINK, mailcot_mascot_card_device, mailcot_mascot_device<4>, "mailcot_mascot_charuru_pink", "Mail*cot Charuru (Pink Ears) / \u3061\u3083\u308b\u308b")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_KURAU_BLUE, mailcot_mascot_card_device, mailcot_mascot_device<5>, "mailcot_mascot_kurau_blue", "Mail*cot Kurau (Light Blue Ears) / \u30af\u30e9\u30a6")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_KURUMIGUMA, mailcot_mascot_card_device, mailcot_mascot_device<6>, "mailcot_mascot_kurumiguma", "Mail*cot Kurumiguma / \u304f\u308b\u307f\u3050\u307e")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_CLOUDS_16K, mailcot_mascot_card_device, mailcot_mascot_device<7>, "mailcot_mascot_clouds_16k", "Mail*cot Clouds & Stars (ID 7)")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_PUCCHI, mailcot_mascot_card_device, mailcot_mascot_device<8>, "mailcot_mascot_pucchi", "Mail*cot Pucchi / \u3077\u3063\u3061")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_KURAU_GREEN, mailcot_mascot_card_device, mailcot_mascot_device<9>, "mailcot_mascot_kurau_green", "Mail*cot Kurau (Green Hat) / \u30af\u30e9\u30a6")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_CHUTA, mailcot_mascot_card_device, mailcot_mascot_device<10>, "mailcot_mascot_chuta", "Mail*cot Chuta / \u3061\u3085\u30fc\u305f")
DEFINE_DEVICE_TYPE_PRIVATE(MAILCOT_MASCOT_UPYO, mailcot_mascot_card_device, mailcot_mascot_device<11>, "mailcot_mascot_upyo", "Mail*cot Upyo / \u30a6\u30d4\u30e7")

template class device_finder<mailcot_mascot_card_device, false>;
template class device_finder<mailcot_mascot_card_device, true>;

template <> device_type const mailcot_mascot_device<0>::TYPE = MAILCOT_MASCOT_CHARURU;
template <> device_type const mailcot_mascot_device<1>::TYPE = MAILCOT_MASCOT_KURAU;
template <> device_type const mailcot_mascot_device<2>::TYPE = MAILCOT_MASCOT_PETCHI;
template <> device_type const mailcot_mascot_device<3>::TYPE = MAILCOT_MASCOT_CLOUDS;
template <> device_type const mailcot_mascot_device<4>::TYPE = MAILCOT_MASCOT_CHARURU_PINK;
template <> device_type const mailcot_mascot_device<5>::TYPE = MAILCOT_MASCOT_KURAU_BLUE;
template <> device_type const mailcot_mascot_device<6>::TYPE = MAILCOT_MASCOT_KURUMIGUMA;
template <> device_type const mailcot_mascot_device<7>::TYPE = MAILCOT_MASCOT_CLOUDS_16K;
template <> device_type const mailcot_mascot_device<8>::TYPE = MAILCOT_MASCOT_PUCCHI;
template <> device_type const mailcot_mascot_device<9>::TYPE = MAILCOT_MASCOT_KURAU_GREEN;
template <> device_type const mailcot_mascot_device<10>::TYPE = MAILCOT_MASCOT_CHUTA;
template <> device_type const mailcot_mascot_device<11>::TYPE = MAILCOT_MASCOT_UPYO;

mailcot_mascot_eeprom_device::mailcot_mascot_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mailcot_mascot_eeprom_device(mconfig, tag, owner, model::C16)
{
}

mailcot_mascot_eeprom_device::mailcot_mascot_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, model type)
	: i2cmem_device(mconfig, MAILCOT_MASCOT_EEPROM, tag, owner, 0, 0, page_size(type), capacity(type))
	, m_initialized(false)
{
	nvram_enable_backup(false);
	set_e1(type == model::C32);
}

int mailcot_mascot_eeprom_device::capacity(model type)
{
	switch (type)
	{
	case model::C16: return 0x800;
	case model::C128: return 0x4000;
	case model::C32: return 0x1000;
	}
	throw emu_fatalerror("Invalid Mail*cot EEPROM model");
}

int mailcot_mascot_eeprom_device::page_size(model type)
{
	switch (type)
	{
	case model::C16: return 16;
	case model::C128: return 64;
	case model::C32: return 32;
	}
	throw emu_fatalerror("Invalid Mail*cot EEPROM model");
}

void mailcot_mascot_eeprom_device::nvram_default()
{
	// Startup images load before the machine initializes its NVRAM devices.
	if (!m_initialized)
		i2cmem_device::nvram_default();
}

bool mailcot_mascot_eeprom_device::load_image(util::read_stream &file)
{
	m_initialized = nvram_load(file);
	return m_initialized;
}

void mailcot_mascot_eeprom_device::clear()
{
	i2cmem_device::nvram_default();
	reset_bus();
	m_initialized = false;
}

mailcot_mascot_card_device::mailcot_mascot_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, mailcot_mascot_eeprom_device::model model, u8 id)
	: device_t(mconfig, type, tag, owner, clock)
	, m_eeprom(*this, "eeprom")
	, m_model(model)
	, m_id(id)
{
}

void mailcot_mascot_card_device::device_add_mconfig(machine_config &config)
{
	MAILCOT_MASCOT_EEPROM(config, m_eeprom, m_model);
}

void mailcot_mascot_card_device::device_start()
{
}

void mailcot_mascot_card_device::write(int sda, int scl)
{
	m_eeprom->write_sda(sda);
	m_eeprom->write_scl(scl);
}

template <u8 Character>
mailcot_mascot_device<Character>::mailcot_mascot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mailcot_mascot_card_device(mconfig, TYPE, tag, owner, clock, mailcot_mascot_eeprom_device::model(Character / 4), Character & 3)
{
	static_assert(Character < 12);
}

mailcot_mascot_slot_device::mailcot_mascot_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MAILCOT_MASCOT_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<mailcot_mascot_card_device>(mconfig, *this)
	, device_memcard_image_interface(mconfig, *this)
	, m_card(nullptr)
	, m_loaded(false)
	, m_inhibited(false)
	, m_saved_present(false)
	, m_saved_character(0xff)
	, m_host_sda(true)
	, m_host_scl(false)
{
	option_add("charuru", MAILCOT_MASCOT_CHARURU);
	option_add("kurau", MAILCOT_MASCOT_KURAU);
	option_add("petchi", MAILCOT_MASCOT_PETCHI);
	option_add("clouds", MAILCOT_MASCOT_CLOUDS);
	option_add("charuru_pink", MAILCOT_MASCOT_CHARURU_PINK);
	option_add("kurau_blue", MAILCOT_MASCOT_KURAU_BLUE);
	option_add("kurumiguma", MAILCOT_MASCOT_KURUMIGUMA);
	option_add("clouds_16k", MAILCOT_MASCOT_CLOUDS_16K);
	option_add("pucchi", MAILCOT_MASCOT_PUCCHI);
	option_add("kurau_green", MAILCOT_MASCOT_KURAU_GREEN);
	option_add("chuta", MAILCOT_MASCOT_CHUTA);
	option_add("upyo", MAILCOT_MASCOT_UPYO);
	set_default_option("charuru");
	set_fixed(false);
}

std::string mailcot_mascot_slot_device::get_default_card_software(get_default_card_software_hook &) const
{
	return software_get_default_slot("charuru");
}

void mailcot_mascot_slot_device::device_config_complete()
{
	add_format("bin", "Memory Mascot image", "bin", "");
}

void mailcot_mascot_slot_device::device_start()
{
	m_card = get_card_device();
	save_item(NAME(m_saved_present));
	save_item(NAME(m_saved_character));
	save_item(NAME(m_host_sda));
	save_item(NAME(m_host_scl));
}

void mailcot_mascot_slot_device::device_pre_save()
{
	m_saved_present = connected();
	m_saved_character = connected() ? m_card->character() : 0xff;
}

void mailcot_mascot_slot_device::device_post_load()
{
	if ((m_saved_present != connected()) || (connected() && m_saved_character != m_card->character()))
	{
		// Restoring emulated state must not eject or overwrite host media.
		m_inhibited = true;
		popmessage("The save state uses a different mascot. Remount a matching mascot image.");
	}
}

std::pair<std::error_condition, std::string> mailcot_mascot_slot_device::call_load()
{
	if (!m_card)
		return { image_error::UNSUPPORTED, "Select a Memory Mascot in Slot Devices before loading its image." };
	if (length() != m_card->capacity())
		return { image_error::INVALIDLENGTH, "This image does not match the selected mascot." };

	m_card->clear();
	if (!m_card->load_image(image_core_file()))
		return { std::errc::io_error, "Unable to read Memory Mascot" };
	insert();
	return { std::error_condition(), std::string() };
}

std::pair<std::error_condition, std::string> mailcot_mascot_slot_device::call_create(int format_type, util::option_resolution *format_options)
{
	if (!m_card)
		return { image_error::UNSUPPORTED, "Select a Memory Mascot in Slot Devices before creating its image." };

	m_card->clear();
	if (!m_card->save_image(image_core_file()))
		return { std::errc::io_error, "Unable to create Memory Mascot" };
	m_card->initialize();
	insert();
	return { std::error_condition(), std::string() };
}

void mailcot_mascot_slot_device::call_unload()
{
	if (connected() && !is_readonly())
	{
		if (fseek(0, SEEK_SET) || !m_card->save_image(image_core_file()))
			popmessage("Unable to save Memory Mascot: %s", filename());
	}
	m_loaded = false;
	m_inhibited = false;
	if (m_card)
		m_card->clear();
}

void mailcot_mascot_slot_device::insert()
{
	m_card->reset_bus();
	m_loaded = true;
	m_inhibited = false;
	m_card->write(m_host_sda, m_host_scl);
}

void mailcot_mascot_slot_device::write(int sda, int scl)
{
	m_host_sda = sda;
	m_host_scl = scl;
	if (connected())
		m_card->write(sda, scl);
}
