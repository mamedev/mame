// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
#include "emu.h"
#include "vfxcart.h"

// #define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(ENSONIQ_VFX_CARTRIDGE, ensoniq_vfx_cartridge, "ensoniq_vfx_cartridge", "Ensoniq VFX family Cartridge")

ensoniq_vfx_cartridge::ensoniq_vfx_cartridge(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, ENSONIQ_VFX_CARTRIDGE, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_eeprom(*this, "eeprom")
	, m_input_config(*this, "CFG")
{
}

ensoniq_vfx_cartridge::~ensoniq_vfx_cartridge()
{
}

void ensoniq_vfx_cartridge::device_start()
{
}

void ensoniq_vfx_cartridge::device_reset()
{
	// Ensoniq SC-32 cartridges are write protected.
	// Any byte written will be preceded by a corresponding
	// write protection command sequence.
	m_eeprom->set_software_data_protection_enabled(false);

	// Configure the speed of the eeprom.
	ioport_value const cfg = m_input_config->read();
	bool fastcart = (cfg & 0x01) != 0;
	if (fastcart) {
		// Fast cartridge:
		// - writes are triggered by reading, without having to wait for T_BLC to expire
		m_eeprom->override_program_on_read(true);

		// - writes complete immediately
		m_eeprom->override_t_wc_usec(0);
	} else {
		// Original cartridge:
		// - use the EEPROM's default timing.
		m_eeprom->override_t_wc_usec();
		m_eeprom->override_program_on_read();
	}
}

namespace {

INPUT_PORTS_START(vfxcart)
	PORT_START("CFG")
	PORT_CONFNAME(0x01, 0x01, "Cartridge Speed")
	PORT_CONFSETTING(0x00, "Original (> 3 minutes for a full write)")
	PORT_CONFSETTING(0x01, "Fast (~ 1 minute for a full write)")
INPUT_PORTS_END

}

ioport_constructor ensoniq_vfx_cartridge::device_input_ports() const
{
	return INPUT_PORTS_NAME(vfxcart);
}

void ensoniq_vfx_cartridge::device_add_mconfig(machine_config &config)
{
	X28C256(config, m_eeprom);
}

u8 ensoniq_vfx_cartridge::read(offs_t offset)
{
	if (!m_is_loaded)
		return 0xff;

	return m_eeprom->read(offset & MASK);
}

void ensoniq_vfx_cartridge::write(offs_t offset, u8 data)
{
	if (!m_is_loaded)
		return;

	if (!m_is_writeable)
	{
		if (offset > 0x7f00)
			LOG("!W %04x  (%02x)\n", offset, data);
		return;
	}

	m_eeprom->write(offset & MASK, data);
}

void ensoniq_vfx_cartridge::setup_load_cb(load_cb cb)
{
	m_load_cb = cb;
}

void ensoniq_vfx_cartridge::setup_unload_cb(unload_cb cb)
{
	m_unload_cb = cb;
}

std::pair<std::error_condition, std::string> ensoniq_vfx_cartridge::call_load()
{
	m_eeprom->reset();

	auto &storage = m_eeprom->data();
	std::fill_n(&storage[0], SIZE, 0);

	LOG("Loading cartridge data from '%s'\n", filename());
	auto n = fread(&storage[0], SIZE);
	fseek(0, SEEK_END);
	if (n < SIZE || ftell() != SIZE)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size, must be 32KiB");

	// If the file is read-only, treat it as a non-writable ROM file, even if it has an EEPROM filetype.
	// This is to prevent users from loading a readonly EEPROM cartridge file,
	// making changes to it, and quitting MAME expecting the image to have been written back,
	// which could not happen because the file was read-only.
	m_is_writeable = !is_readonly() && (filetype() == "eeprom" || filetype() == "sc32"); // Ensoniq StorCart-32
	LOG("- loaded %d bytes, cartridge is %s\n", n, m_is_writeable ? "Writeable" : "Read-Only");

	if (!m_load_cb.isnull()) {
		LOG("- calling load callback\n");
		m_load_cb(this);
	}

	m_is_loaded = true;

	return std::make_pair(std::error_condition(), std::string());
}

std::pair<std::error_condition, std::string> ensoniq_vfx_cartridge::call_create(int format_type, util::option_resolution *format_options)
{
	LOG("Creating empty cartridge data in '%s'\n", filename());
	m_eeprom->reset();

	auto &storage = m_eeprom->data();
	std::fill_n(&storage[0], SIZE, 0);
	storage[0x7ffe] = 0x05;
	storage[0x7fff] = 0x01;
	fseek(0, SEEK_SET);
	fwrite(&storage[0], SIZE);

	// By definition, if we create a cartridge image, is kind of has to be a writable one: a completely empty
	// read-only cartridge makes no sense!
	// This allows users to create a cartridge image with a ".rom" or ".cart" extension, write to it _once_,
	// in this session, until it is unloaded; and whenever in the future they load it again, it will be
	// detected as ROM (_not_ EEPROM) by its file extension.
	m_is_writeable = true;

	// Creating a cartridge also loads it.
	if (!m_load_cb.isnull()) {
		LOG("- calling load callback\n");
		m_load_cb(this);
	}

	m_is_loaded = true;

	return std::make_pair(std::error_condition(), std::string());
}

void ensoniq_vfx_cartridge::call_unload()
{
	m_eeprom->reset();
	auto &storage = m_eeprom->data();

	// If the current file is writable and is a writeable (EEPROM) file, write the data back
	LOG("Unloading cartridge '%s'\n", filename());
	if (!is_readonly() && m_is_writeable)
	{
		LOG("Writing cartridge data to '%s'\n", filename());
		fseek(0, SEEK_SET);
		fwrite(&storage[0], SIZE);
	}

	if (!m_unload_cb.isnull()) {
		LOG("- calling unload callback\n");
		m_unload_cb(this);
	}

	m_is_loaded = false;

	std::fill_n(&storage[0], SIZE, 0xff);
}
