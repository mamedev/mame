// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Memory controller used by Liebao Technology

 Supports switching 8 KiB pages at 0x4000 and 0x6000 independently.  Only
 even pages can be mapped at 0x4000, and only odd pages can be mapped at
 0x6000.  Everything here is guesswork based on the behaviour of a single
 game.

 ***************************************************************************/

#include "emu.h"
#include "liebao.h"

#include "cartbase.ipp"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class liebao_device : public mbc_ram_device_base<mbc_8k_device_base>
{
public:
	liebao_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void enable_ram(offs_t offset, u8 data);
	void bank_switch_rom(offs_t offset, u8 data);
	void bank_switch_ram(u8 data);
	void bank_switch_rom_high(offs_t offset, u8 data);

	memory_view m_view_ram;

	u8 m_bank_high_set;
};


liebao_device::liebao_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_ram_device_base<mbc_8k_device_base>(mconfig, GB_ROM_LIEBAO, tag, owner, clock),
	m_view_ram(*this, "ram"),
	m_bank_high_set(0U)
{
}


image_init_result liebao_device::load(std::string &message)
{
	// set up ROM and RAM
	set_bank_bits_rom(9);
	set_bank_bits_ram(4);
	if (!check_rom(message) || !check_ram(message))
		return image_init_result::FAIL;
	cart_space()->install_view(0xa000, 0xbfff, m_view_ram);
	install_rom(*cart_space(), *cart_space(), *cart_space());
	install_ram(m_view_ram[0]);

	// install handlers
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			write8sm_delegate(*this, FUNC(liebao_device::enable_ram)));
	cart_space()->install_write_handler(
			0x2000, 0x2fff,
			write8sm_delegate(*this, FUNC(liebao_device::bank_switch_rom)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			write8smo_delegate(*this, FUNC(liebao_device::bank_switch_ram)));
	cart_space()->install_write_handler(
			0x7000, 0x7fff,
			write8sm_delegate(*this, FUNC(liebao_device::bank_switch_rom_high)));

	// all good
	return image_init_result::PASS;
}


void liebao_device::device_start()
{
	mbc_ram_device_base<mbc_8k_device_base>::device_start();

	save_item(NAME(m_bank_high_set));
}


void liebao_device::device_reset()
{
	mbc_ram_device_base<mbc_8k_device_base>::device_reset();

	m_view_ram.disable();
	m_bank_high_set = 0U;

	set_bank_rom_low(2);
	set_bank_rom_high(3);
	set_bank_ram(0);
}


void liebao_device::enable_ram(offs_t offset, u8 data)
{
	// TODO: how many bits are checked?
	bool const enable(0x0a == (data & 0x0f));
	LOG(
			"%s: Cartridge RAM %s\n",
			machine().describe_context(),
			enable ? "enabled" : "disabled");
	if (enable)
		m_view_ram.select(0);
	else
		m_view_ram.disable();
}


void liebao_device::bank_switch_rom(offs_t offset, u8 data)
{
	set_bank_rom_low(u16(data) << 1);
	if (m_bank_high_set || ((offset & 0x00ff) == 0x00d2))
		m_bank_high_set = 0U;
	else
		set_bank_rom_high((u16(data) << 1) | 0x01);
}


void liebao_device::bank_switch_ram(u8 data)
{
	set_bank_ram(data & 0x0f);
}


void liebao_device::bank_switch_rom_high(offs_t offset, u8 data)
{
	if ((offset & 0x00ff) == 0x00d2)
	{
		m_bank_high_set = 1U;
		set_bank_rom_high((u16(data) << 1) | 0x01);
	}
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_LIEBAO, device_gb_cart_interface, bus::gameboy::liebao_device, "gb_rom_liebao", "Game Boy Liebao Technology Cartridge")
