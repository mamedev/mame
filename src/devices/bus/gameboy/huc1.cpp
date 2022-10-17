// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Hudson Soft HuC1 Memory Controller

 Provides ROM and RAM banking and infrared I/O.  Supports up to 1 MiB ROM
 (64 16 KiB pages) and up to 32 KiB static RAM (4 8 KiB pages).  If RAM bank
 lines are used for coarse ROM banking, up to 4 MiB ROM (256 16 KiB pages)
 can be supported.

 The HuC1 controller appears to only respond to A15-A13 and D6-D0, i.e.
 addresses are effectively masked with 0xE000 and data is effectively masked
 with 0x3F.

 The HuC1 controller doesn't support disabling cartridge RAM without
 selecting infrared I/O.  However, some games still write 0x00 or 0x0A to
 the infrared/RAM select register as if it behaved like the Nintendo MBC
 series RAM enable register.  Some games write to the 0x6000-0x7FFF range,
 but this has no detectable effect.

 0x0000-3FFF R  - Fixed ROM bank, always first page of ROM.
 0x4000-7FFF R  - Selectable ROM bank, page 0-255 of ROM.
 0xA000-BFFF RW - Static RAM or infrared I/O.

 0x0000-1FFF W  - Select infrared (0x0E) or RAM (not 0x0E) at 0xA000.
 0x2000-3FFF W  - Select ROM page mapped at 0x4000.
 0x4000-5FFF W  - Select RAM page mapped at 0xA000.

 TODO:
 * If you try to use the infrared link feature in gbkiss, it says to press
   the B button to stop communication, but it doesn't respond to any inputs.
   Is this normal?
 * What is the default state for banking and infrared select on reset?
 * Does ROM bank 0 map to bank 1 like MBC1?
 * How many RAM page lines are there?  No games use more than 2.
 * Do any bits of the infrared I/O registers have any significance besides
   the least significant bit?
 * Do any values besides 0x0E have special significance for infrared/RAM
   select?

 ***************************************************************************/

#include "emu.h"
#include "huc1.h"

#include "cartbase.ipp"

#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class huc1_device : public mbc_ram_device_base<mbc_dual_device_base>
{
public:
	static constexpr feature_type unemulated_features() { return feature::COMMS; }

	huc1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_reset() override ATTR_COLD;

private:
	void infrared_select(u8 data);
	void bank_switch_fine(u8 data);
	void bank_switch_coarse(u8 data);
	u8 read_ir(address_space &space);
	void write_ir(u8 data);

	memory_view m_view_ir;
};


huc1_device::huc1_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_ram_device_base<mbc_dual_device_base>(mconfig, GB_ROM_HUC1, tag, owner, clock),
	m_view_ir(*this, "ir")
{
}


image_init_result huc1_device::load(std::string &message)
{
	// check for valid ROM/RAM regions
	set_bank_bits_rom(2, 6);
	set_bank_bits_ram(2);
	if (!check_rom(message) || !check_ram(message))
		return image_init_result::FAIL;

	// if that checked out, install memory
	install_rom();
	install_ram(*cart_space());

	// install memory controller handlers
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			write8smo_delegate(*this, FUNC(huc1_device::infrared_select)));
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			write8smo_delegate(*this, FUNC(huc1_device::bank_switch_fine)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			write8smo_delegate(*this, FUNC(huc1_device::bank_switch_coarse)));

	// install infrared handlers
	cart_space()->install_view(0xa000, 0xbfff, m_view_ir);
	m_view_ir[0].install_read_handler(
			0xa000, 0xbfff,
			read8mo_delegate(*this, FUNC(huc1_device::read_ir)));
	m_view_ir[0].install_write_handler(
			0xa000, 0xbfff,
			write8smo_delegate(*this, FUNC(huc1_device::write_ir)));

	// all good
	return image_init_result::PASS;
}


void huc1_device::device_reset()
{
	mbc_ram_device_base<mbc_dual_device_base>::device_reset();

	// TODO: what's the proper reset state?
	m_view_ir.disable();

	set_bank_rom_fine(0);
	set_bank_rom_coarse(0);
	set_bank_ram(0);
}


void huc1_device::infrared_select(u8 data)
{
	if (0x0e == (data & 0x0e))
	{
		LOG("%s: Infrared I/O selected\n", machine().describe_context());
		m_view_ir.select(0);
	}
	else
	{
		LOG("%s: Cartridge RAM selected\n", machine().describe_context());
		m_view_ir.disable();
	}
}


void huc1_device::bank_switch_fine(u8 data)
{
	// TODO: does zero map to bank 1 like MBC1?
	set_bank_rom_fine(data & 0x3f);
}


void huc1_device::bank_switch_coarse(u8 data)
{
	// TODO: how many output lines are physically present?
	set_bank_rom_coarse(data & 0x03);
	set_bank_ram(data & 0x03);
}


u8 huc1_device::read_ir(address_space &space)
{
	LOG("%s: Infrared read\n");
	return (space.unmap() & 0xc0) | 0x00; // least significant bit clear - dark
}


void huc1_device::write_ir(u8 data)
{
	// bit zero high to turn on the IR LED, or low to turn it off
	LOG("%s: Infrared write 0x%02X\n", machine().describe_context(), data);
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_HUC1, device_gb_cart_interface, bus::gameboy::huc1_device, "gb_rom_huc1", "Game Boy Hudson Soft HuC1 Cartridge")
