// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Mail*cot mascots contain an I2C EEPROM and two identity straps. The ROM
    detects presence by ACK and combines capacity and straps to select a character:
    2 KiB, 00/01/10 = Charuru/Kurau/Petchi; 4 KiB, 00 = Pucchi.
    Additional variants are identified by their portraits.

    The mountable memory image supplies persistent data. The model supports
    2 KiB, 4 KiB and 16 KiB images. Save states record mascot presence and
    character separately from the mounted image; incompatible restores detach
    without writing restored EEPROM data back to that image.

***************************************************************************/

#include "emu.h"
#include "xavix_io_mail_mascot.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(XAVIX_MAILCOT_MASCOT, xavix_mailcot_mascot_device, "xavix_mailcot_mascot", "Mail*cot Memory Mascot")

namespace {

INPUT_PORTS_START(mailcot_mascot)
	PORT_START("ID")
	PORT_CONFNAME(0x0f, 0x00, "Mascot Character") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_mailcot_mascot_device::character_changed), 0)
	PORT_CONFSETTING(0x00, "Charuru / \u3061\u3083\u308b\u308b") // ちゃるる
	PORT_CONFSETTING(0x01, "Kurau / \u30af\u30e9\u30a6") // クラウ
	PORT_CONFSETTING(0x02, "Petchi / \u307a\u3063\u3061") // ぺっち
	PORT_CONFSETTING(0x03, "Cloud & Stars (ID 3)")
	PORT_CONFSETTING(0x04, "Charuru (Pink Ears) / \u3061\u3083\u308b\u308b") // ちゃるる
	PORT_CONFSETTING(0x05, "Kurau (Light Blue Ears) / \u30af\u30e9\u30a6") // クラウ
	PORT_CONFSETTING(0x06, "Kurumiguma / \u304f\u308b\u307f\u3050\u307e") // くるみぐま
	PORT_CONFSETTING(0x07, "Clouds & Stars (ID 7)")
	PORT_CONFSETTING(0x08, "Pucchi / \u3077\u3063\u3061") // ぷっち
	PORT_CONFSETTING(0x09, "Kurau (Green Hat) / \u30af\u30e9\u30a6") // クラウ
	PORT_CONFSETTING(0x0a, "Chuta / \u3061\u3085\u30fc\u305f") // ちゅーた
	PORT_CONFSETTING(0x0b, "Upyo / \u30a6\u30d4\u30e7") // ウピョ
INPUT_PORTS_END

} // anonymous namespace

xavix_mailcot_mascot_device::xavix_mailcot_mascot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i2cmem_device(mconfig, XAVIX_MAILCOT_MASCOT, tag, owner, clock, 0, 64, 0x4000)
	, device_memcard_image_interface(mconfig, *this)
	, m_id(*this, "ID")
	, m_loaded(false)
	, m_pending_create(false)
	, m_mounted_character(0xff)
	, m_host_sda(true)
	, m_host_scl(false)
{
	nvram_enable_backup(false);
}

void xavix_mailcot_mascot_device::device_config_complete()
{
	add_format("bin", "Memory Mascot image", "bin", "");
}

bool xavix_mailcot_mascot_device::is_reset_on_load() const noexcept
{
	return false;
}

const char *xavix_mailcot_mascot_device::image_type_name() const noexcept
{
	return "mascot";
}

const char *xavix_mailcot_mascot_device::image_brief_type_name() const noexcept
{
	return "masc";
}

const char *xavix_mailcot_mascot_device::file_extensions() const noexcept
{
	return "bin";
}

ioport_constructor xavix_mailcot_mascot_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mailcot_mascot);
}

void xavix_mailcot_mascot_device::device_start()
{
	// Keep the backing storage and save layout fixed when swapping capacities.
	i2cmem_device::device_start();
	save_item(NAME(m_data_size));
	save_item(NAME(m_write_page_size));
	save_item(NAME(m_saved_present));
	save_item(NAME(m_saved_character));
	save_item(NAME(m_host_sda));
	save_item(NAME(m_host_scl));
	set_capacity(0x800);
}

void xavix_mailcot_mascot_device::device_pre_save()
{
	i2cmem_device::device_pre_save();
	m_saved_present = m_loaded;
	m_saved_character = m_mounted_character;
}

void xavix_mailcot_mascot_device::device_post_load()
{
	i2cmem_device::device_post_load();
	if ((m_saved_present != m_loaded) ||
			(m_loaded && ((m_saved_character != m_mounted_character) ||
					(m_data_size != character_capacity(m_mounted_character)))))
	{
		// Do not persist restored contents into incompatible mounted media.
		m_loaded = false;
		unload();
		popmessage("The save state uses a different mascot. Mount a matching mascot image.");
	}
}

void xavix_mailcot_mascot_device::device_reset()
{
	// Startup images are opened before the character configuration is loaded.
	if (!m_loaded || (m_mounted_character != 0xff))
		return;

	u8 const character = m_id->read();
	if (m_pending_create)
	{
		set_capacity(character_capacity(character));
		if (fseek(0, SEEK_SET) || !nvram_save(image_core_file()))
		{
			m_loaded = false;
			unload();
			popmessage("Unable to create Memory Mascot");
			return;
		}
		m_pending_create = false;
	}
	else if (m_data_size != character_capacity(character))
	{
		m_loaded = false;
		unload();
		popmessage("This image does not match the selected mascot. Choose a matching mascot image.");
		return;
	}
	m_mounted_character = character;
}

void xavix_mailcot_mascot_device::nvram_default()
{
	// Images load before the machine initializes its NVRAM devices.
	if (!m_loaded)
		std::fill_n(m_data.get(), 0x4000, 0xff);
}

void xavix_mailcot_mascot_device::set_capacity(int size)
{
	m_data_size = size;
	m_write_page_size = size == 0x800 ? 16 : size == 0x1000 ? 32 : 64;
	write_e1(size == 0x1000);
}

int xavix_mailcot_mascot_device::character_capacity(u8 character)
{
	return character < 4 ? 0x800 : character < 8 ? 0x4000 : 0x1000;
}

INPUT_CHANGED_MEMBER(xavix_mailcot_mascot_device::character_changed)
{
	if (m_loaded && (m_mounted_character != 0xff) && (newval != m_mounted_character))
	{
		unload();
		popmessage("Mount or create data for the selected mascot.");
	}
}

std::pair<std::error_condition, std::string> xavix_mailcot_mascot_device::call_load()
{
	auto const size = length();
	if ((size != 0x800) && (size != 0x1000) && (size != 0x4000))
		return { image_error::INVALIDLENGTH, "Invalid Memory Mascot image" };
	if (machine().ioport().safe_to_read() && (size != character_capacity(m_id->read())))
		return { image_error::INVALIDLENGTH, "This image does not match the selected mascot. Choose a matching mascot image." };

	set_capacity(int(size));
	std::fill_n(m_data.get(), 0x4000, 0xff);
	if (!nvram_load(image_core_file()))
		return { std::errc::io_error, "Unable to read Memory Mascot" };
	insert();
	return { std::error_condition(), std::string() };
}

std::pair<std::error_condition, std::string> xavix_mailcot_mascot_device::call_create(int format_type, util::option_resolution *format_options)
{
	std::fill_n(m_data.get(), 0x4000, 0xff);
	m_pending_create = !machine().ioport().safe_to_read();
	if (!m_pending_create)
	{
		set_capacity(character_capacity(m_id->read()));
		if (!nvram_save(image_core_file()))
			return { std::errc::io_error, "Unable to create Memory Mascot" };
	}
	insert();
	return { std::error_condition(), std::string() };
}

void xavix_mailcot_mascot_device::call_unload()
{
	if (m_loaded && !m_pending_create && !is_readonly())
	{
		// A save state must not resize a different mascot mounted afterwards.
		if (length() != m_data_size)
			popmessage("Memory Mascot capacity differs from the save state: %s", filename());
		else if (fseek(0, SEEK_SET) || !nvram_save(image_core_file()))
			popmessage("Unable to save Memory Mascot: %s", filename());
	}
	m_loaded = false;
	m_pending_create = false;
	m_mounted_character = 0xff;
	reset_bus();
	std::fill_n(m_data.get(), 0x4000, 0xff);
}

void xavix_mailcot_mascot_device::insert()
{
	reset_bus();
	m_loaded = true;
	m_mounted_character = machine().ioport().safe_to_read() ? m_id->read() : 0xff;
	write(m_host_sda, m_host_scl);
}

void xavix_mailcot_mascot_device::write(int sda, int scl)
{
	m_host_sda = sda;
	m_host_scl = scl;
	if (m_loaded)
	{
		write_sda(sda);
		write_scl(scl);
	}
}
