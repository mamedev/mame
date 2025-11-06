// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
#include "emu.h"
#include "vfxcart.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ENSONIQ_VFX_CARTRIDGE, ensoniq_vfx_cartridge, "ensoniq_vfx_cartridge", "Ensoniq VFX family Cartridge")

ensoniq_vfx_cartridge::ensoniq_vfx_cartridge(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		u32 clock)
	: device_t(mconfig, ENSONIQ_VFX_CARTRIDGE, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{}

ensoniq_vfx_cartridge::~ensoniq_vfx_cartridge() {}

u8 ensoniq_vfx_cartridge::read(offs_t offset) {
	m_state = state::IDLE;
	auto v = m_storage[offset & MASK];
	if (offset > 0x7f00)
		LOG("R %04x -> %02x\r\n", offset, v);
	return v;
}

void ensoniq_vfx_cartridge::write(offs_t offset, u8 data) {
	if (!m_is_writeable) {
		if (offset > 0x7f00)
			LOG("!W %04x  (%02x)\r\n", offset, data);
		return;
	}

	// The Ensoniq VFX family use the common EEPROM software write protection scheme
	// to protect their EEPROM cartridges from accidental writes.
	// But they do it in a simplistic way:
	// Each byte is written individually using a separate Enable Write Protection
	// command sequence.
	// This means that in a real EEPROM, writing each byte takes an additional
	// T_BLC + T_WC ~= 5ms - for each byte. Writing an entire cartridge thus will take
	// approximately 32000 * 5ms = 160 seconds = 3 minutes.
	// However, to determine when they can write the next byte, the VFX family also
	// read the just-written byte back: The EEPROM's "Toggle Bit Polling"
	// and "/DATA Polling" facilities will keep returning something other than the written
	// byte until the byte has been successfully written.
	// Here, we can actually immediately write any written bytes to storage, and use
	// the client's reading of the value to signal the end of the write cycle.
	switch(m_state) {
		case state::IDLE:
			if (offset == 0x5555 && data == 0xaa)
				m_state = state::CMD1;
			else
				m_state = state::IDLE;
			break;
		case state::CMD1:
			if (offset == 0x2aaa && data == 0x55)
				m_state = state::CMD2;
			else
				m_state = state::IDLE;
			break;
		case state::CMD2:
			if (offset == 0x5555 && data == 0xa0)
				m_state = state::WR;
			else
				m_state = state::IDLE;
			break;

		case state::WR:
			if (offset > 0x7f00)
				LOG("W %04x :  %02x\r\n", offset, data);
			m_storage[offset] = data;
			break;
	}
}

void ensoniq_vfx_cartridge::setup_load_cb(load_cb cb)
{
	m_load_cb = cb;
	if (!m_load_cb.isnull() && is_loaded()) {
		m_load_cb(this);
	}
}

void ensoniq_vfx_cartridge::setup_unload_cb(unload_cb cb)
{
	m_unload_cb = cb;
	if (!m_unload_cb.isnull() && !is_loaded()) {
		m_unload_cb(this);
	}
}

std::pair<std::error_condition, std::string> ensoniq_vfx_cartridge::call_load() {
	std::fill(std::begin(m_storage), std::end(m_storage), 0);

	LOG("Loading cartridge data from '%s'\r\n", filename());
	auto n = fread(&m_storage[0], m_storage.size());
	fseek(0, SEEK_END);
	if (n < SIZE || ftell() != SIZE) {
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid size, must be 32kiB");
	}

	// If the file is read-only, treat it as a non-writable ROM file, even if it has an EEPROM filetype.
	// This is to prevent users from loading a readonly EEPROM cartridge file,
	// making changes to it, and quitting MAME expecting the image to have been written back,
	// which could not happen because the file was read-only.
	m_is_writeable = !is_readonly() && (filetype() == "eeprom" || filetype() == "sc32"); // Ensoniq StorCart-32
	LOG("- loaded %d bytes, cartridge is %s\r\n", n, m_is_writeable ? "Writeable" : "Read-Only");

	if (!m_load_cb.isnull()) {
		m_load_cb(this);
	}

	return std::make_pair(std::error_condition(), std::string());
}

std::pair<std::error_condition, std::string> ensoniq_vfx_cartridge::call_create(int format_type, util::option_resolution *format_options) {
	LOG("Creating empty cartridge data in '%s'\r\n", filename());
	std::fill(std::begin(m_storage), std::end(m_storage), 0);
	m_storage[0x7ffe] = 0x05;
	m_storage[0x7fff] = 0x01;
	fseek(0, SEEK_SET);
	fwrite(&m_storage[0], m_storage.size());

	// By definition, if we create a cartridge image, is kind of has to be a writable one: a completely empty
	// read-only cartridge makes no sense!
	// This allows users to create a cartridge image with a ".rom" or ".cart" extension, write to it _once_,
	// in this session, until it is unloaded; and whenever in the future they load it again, it will be
	// detected as ROM (_not_ EEPROM) by its file extension.
	m_is_writeable = true;

	// Creating a cartridge also loads it.
	if (!m_load_cb.isnull()) {
		m_load_cb(this);
	}

	return std::make_pair(std::error_condition(), std::string());
}

void ensoniq_vfx_cartridge::call_unload() {
	// If the current file is writable and is a writeable (EEPROM) file, write the data back
	LOG("Unloading cartridge '%s'\r\n", filename());
	if (!is_readonly() && m_is_writeable) {
		LOG("Writing cartridge data to '%s'\r\n", filename());
		fseek(0, SEEK_SET);
		fwrite(&m_storage[0], m_storage.size());
	}

	if (!m_unload_cb.isnull()) {
		m_unload_cb(this);
	}

	std::fill(std::begin(m_storage), std::end(m_storage), 0);
}
