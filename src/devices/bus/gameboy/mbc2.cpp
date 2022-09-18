// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Nintendo Game Boy Memory Bank Controller 2

 Low-cost controller for games with modest ROM requirements and save file
 support.  Supports up to 256 KiB ROM (16 16 KiB pages), and has 512 nybbles
 of internal static RAM.

 Only A15-A14, A8-A0, D3-D0, and the read/write/RAM select lines are
 connected to the MBC2 chip, i.e. addresses are effectively masked with
 0xC01F and data is effectively masked with 0x0F.

 0x0000-3FFF    R  - Fixed ROM bank, always first page of ROM.
 0x4000-7FFF    R  - Selectable ROM bank, page 1-15 of ROM.
 0xA000-A1FF    RW - Internal static RAM on low nybble if enabled.
                     Mirrored up to 0xBFFF.

 0b0011xxx0xxxx W  - Static RAM enable - write 0x0A on low nybble to enable,
                     any other value to disable.
 0b0011xxx1xxxx W  - Select ROM page mapped at 0x4000.  Only low nybble is
                     significant.  Writing 0 selects page 1.

 ***************************************************************************/

#include "emu.h"
#include "mbc2.h"

#include "cartbase.h"
#include "cartheader.h"

#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class mbc2_device : public mbc_device_base
{
public:
	mbc2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;
	virtual void unload() override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static inline constexpr unsigned RAM_SIZE = 512;

	u8 ram_mbc_read(address_space &space, offs_t offset);
	void ram_mbc_write(offs_t offset, u8 data);
	void ram_mbc_enable(u8 data);
	void bank_rom_switch(u8 data);

	std::unique_ptr<u8 []> m_ram_mbc;
	u8 m_ram_mbc_enable;
	bool m_battery_present;
};


mbc2_device::mbc2_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_device_base(mconfig, GB_ROM_MBC2, tag, owner, clock),
	m_ram_mbc_enable(0U),
	m_battery_present(false)
{
}


image_init_result mbc2_device::load(std::string &message)
{
	// first check ROM
	set_bank_bits_rom(4);
	if (!check_rom(message))
		return image_init_result::FAIL;

	// decide whether to enable battery backup
	memory_region *const nvramregion(cart_nvram_region());
	if (nvramregion)
	{
		if (nvramregion->bytes() != RAM_SIZE)
		{
			message = "Unsupported cartridge NVRAM size (only MBC2 internal 512 nybble RAM is supported)";
			return image_init_result::FAIL;
		}
		m_battery_present = true;
		logerror("Found 'nvram' region, battery backup enabled\n");
	}
	else if (loaded_through_softlist())
	{
		logerror("No 'nvram' region found, battery backup disabled\n");
		m_battery_present = false;
	}
	else
	{
		// guess based on header
		u8 const carttype((&cart_rom_region()->as_u8())[cartheader::OFFSET_TYPE]);
		switch (carttype)
		{
		case cartheader::TYPE_MBC2:
			m_battery_present = false;
			break;
		case cartheader::TYPE_MBC2_BATT:
			m_battery_present = true;
			break;
		default:
			osd_printf_warning(
					"[%s] Unrecognized cartridge type 0x%02X in header, assuming no battery present\n",
					tag(),
					carttype);
			m_battery_present = false;
		}
		osd_printf_verbose(
				"[%s] Cartridge type 0x%02X in header, battery backup %s\n",
				tag(),
				carttype,
				m_battery_present ? "enabled" : "disabled");
	}

	// install ROM, RAM and handlers
	install_rom();
	cart_space()->install_read_handler(
			0xa000, 0xa1ff, 0x0000, 0x1e00, 0x0000,
			read8m_delegate(*this, FUNC(mbc2_device::ram_mbc_read)));
	cart_space()->install_write_handler(
			0xa000, 0xa1ff, 0x0000, 0x1e00, 0x0000,
			write8sm_delegate(*this, FUNC(mbc2_device::ram_mbc_write)));
	cart_space()->install_write_handler(
			0x0000, 0x00ff, 0x0000, 0x3e00, 0x0000,
			write8smo_delegate(*this, FUNC(mbc2_device::ram_mbc_enable)));
	cart_space()->install_write_handler(
			0x0100, 0x01ff, 0x0000, 0x3e00, 0x0000,
			write8smo_delegate(*this, FUNC(mbc2_device::bank_rom_switch)));

	// load battery backup if appropriate
	if (nvramregion)
		battery_load(m_ram_mbc.get(), RAM_SIZE, nvramregion->base());
	else if (m_battery_present)
		battery_load(m_ram_mbc.get(), RAM_SIZE, 0x0f);
	for (unsigned i = 0U; RAM_SIZE > i; ++i)
		m_ram_mbc[i] &= 0x0f;

	// all good
	return image_init_result::PASS;
}


void mbc2_device::unload()
{
	if (m_battery_present)
		battery_save(m_ram_mbc.get(), RAM_SIZE);
}


void mbc2_device::device_start()
{
	mbc_device_base::device_start();

	m_ram_mbc = std::make_unique<u8 []>(RAM_SIZE);

	save_pointer(NAME(m_ram_mbc), RAM_SIZE);
	save_item(NAME(m_ram_mbc_enable));
}


void mbc2_device::device_reset()
{
	mbc_device_base::device_reset();

	m_ram_mbc_enable = 0U;

	set_bank_rom(1);
}


u8 mbc2_device::ram_mbc_read(address_space &space, offs_t offset)
{
	if (m_ram_mbc_enable)
		return (m_ram_mbc[offset] & 0x0f) | (space.unmap() & 0xf0);
	else
		return space.unmap();
}


void mbc2_device::ram_mbc_write(offs_t offset, u8 data)
{
	if (m_ram_mbc_enable)
		m_ram_mbc[offset] = data & 0x0f;
}


void mbc2_device::ram_mbc_enable(u8 data)
{
	m_ram_mbc_enable = (0x0a == (data & 0x0f)) ? 1U : 0U;
	LOG(
			"%s: Internal MBC2 RAM %s\n",
			machine().describe_context(),
			m_ram_mbc_enable ? "enabled" : "disabled");
}


void mbc2_device::bank_rom_switch(u8 data)
{
	data &= 0x0f;
	set_bank_rom(data ? data : 1);
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MBC2, device_gb_cart_interface, bus::gameboy::mbc2_device, "gb_rom_mbc2", "Game Boy MBC2 Cartridge")
