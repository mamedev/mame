// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp80_optrom.cpp

    Optional ROMs for HP80 systems

*********************************************************************/

#include "emu.h"
#include "hp80_optrom.h"
#include "softlist.h"

// Debugging
#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(HP80_OPTROM, hp80_optrom_device, "hp80_optrom", "HP80 optional ROM")

// +------------------+
// |hp80_optrom_device|
// +------------------+
hp80_optrom_device::hp80_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HP80_OPTROM, tag, owner, clock),
	device_rom_image_interface(mconfig, *this),
	m_select_code(0)
{
}

hp80_optrom_device::~hp80_optrom_device()
{
}

void hp80_optrom_device::install_read_handler(address_space& space)
{
	if (loaded_through_softlist()) {
		offs_t start = (offs_t)m_select_code * HP80_OPTROM_SIZE;
		space.install_rom(start , start + HP80_OPTROM_SIZE - 1 , get_software_region("rom"));
	}
}

void hp80_optrom_device::device_start()
{
}

std::pair<std::error_condition, std::string> hp80_optrom_device::call_load()
{
	LOG("hp80_optrom: call_load\n");
	if (!loaded_through_softlist()) {
		return std::make_pair(image_error::UNSUPPORTED, "Images must be loaded from software list");
	}

	const char *sc_feature = get_feature("sc");
	if (sc_feature == nullptr) {
		return std::make_pair(image_error::BADSOFTWARE, "Software item is missing 'sc' feature");
	}

	unsigned sc;
	if (sc_feature[ 0 ] != '0' || sc_feature[ 1 ] != 'x' || sscanf(&sc_feature[ 2 ] , "%x" , &sc) != 1) {
		return std::make_pair(image_error::BADSOFTWARE, "Can't parse software item 'sc' feature");
	}

	// Valid SC values: 0x01..0xff
	if (sc < 1 || sc > 0xff) {
		return std::make_pair(
				image_error::BADSOFTWARE,
				util::string_format("Illegal select code (%x)\n" , sc));
	}

	auto length = get_software_region_length("rom");

	if (length != HP80_OPTROM_SIZE) {
		return std::make_pair(
				image_error::INVALIDLENGTH,
				util::string_format("Incorrect 'rom' data area size (%u, should be %u)", length, HP80_OPTROM_SIZE));
	}

	LOG("hp80_optrom: loaded SC=0x%02x\n" , sc);
	m_select_code = sc;

	return std::make_pair(std::error_condition(), std::string());
}

void hp80_optrom_device::call_unload()
{
	LOG("hp80_optrom: call_unload\n");
	machine().schedule_soft_reset();
}
