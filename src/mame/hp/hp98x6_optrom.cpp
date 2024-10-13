// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp98x6_optrom.cpp

    Optional ROMs for HP98x6 systems

*********************************************************************/

#include "emu.h"
#include "hp98x6_optrom.h"
#include "softlist.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

namespace {

struct optrom_region {
	offs_t m_start;
	const char *m_tag;
};

constexpr std::array<struct optrom_region , 2> region_tab = {{
	{ 0x100000, "rom100000" },
	{ 0x080000, "rom80000" }
}};

} // anonymous namespace

DEFINE_DEVICE_TYPE(HP98X6_OPTROM, hp98x6_optrom_device, "hp98x6_optrom", "HP98x6 optional ROM")

// +--------------------+
// |hp98x6_optrom_device|
// +--------------------+
hp98x6_optrom_device::hp98x6_optrom_device(machine_config const &mconfig, char const *tag, device_t *owner)
	: hp98x6_optrom_device(mconfig, tag, owner, (uint32_t)0)
{
}

hp98x6_optrom_device::hp98x6_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP98X6_OPTROM, tag, owner, clock)
	, device_rom_image_interface(mconfig, *this)
	, m_space_r(nullptr)
{
}

hp98x6_optrom_device::~hp98x6_optrom_device()
{
}

void hp98x6_optrom_device::install_handlers(address_space *space_r)
{
	m_space_r = space_r;

	for (const struct optrom_region &reg : region_tab) {
		uint8_t *ptr = get_software_region(reg.m_tag);
		if (ptr != nullptr) {
			auto len = get_software_region_length(reg.m_tag);
			LOG("%s loaded, %u long\n", reg.m_tag, len);
			space_r->install_rom(reg.m_start , reg.m_start + len - 1 , ptr);
		}
	}
}

void hp98x6_optrom_device::device_start()
{
}

std::pair<std::error_condition, std::string> hp98x6_optrom_device::call_load()
{
	LOG("hp98x6_optrom: call_load\n");
	if (!loaded_through_softlist()) {
		return std::make_pair(image_error::UNSUPPORTED, "Option ROMs must be loaded from software list");
	}

	return std::make_pair(std::error_condition(), std::string());
}

void hp98x6_optrom_device::call_unload()
{
	LOG("hp98x6_optrom: call_unload\n");
	if (m_space_r != nullptr) {
		for (const struct optrom_region &reg : region_tab) {
			auto len = get_software_region_length(reg.m_tag);
			if (len != 0) {
				m_space_r->unmap_read(reg.m_start , reg.m_start + len - 1);
				LOG("%s unloaded\n" , reg.m_tag);
			}
		}
	}
	machine().schedule_soft_reset();
}
