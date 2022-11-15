// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Memory controller used for newer NT/Makon games

 Appears to start in a mode similar to Nintendo MBC3, with a fixed 16 KiB
 ROM bank at 0x0000 and a switchable 16 KiB ROM bank at 0x4000, but allows
 switching to a mode with selectable 8 KiB ROM banks at 0x4000 and 0x6000.

 It isn't clear how many static RAM pages are actually supported.  Many
 cartridges included 32K*8 static RAMs despite the headers declaring smaller
 sizes and the games using no more than 8 KiB.  The maximum supported ROM
 size is also unknown.

 Pretty much everything here is guessed based on the games' behaviour.  The
 exact address ranges the memory controller responds to are unknown.  There
 may be additional features that are not emulated.

 ***************************************************************************/

#include "emu.h"
#include "ntnew.h"

#include "cartbase.ipp"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class ntnew_device : public mbc_ram_device_base<mbc_8k_device_base>
{
public:
	ntnew_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void enable_ram(offs_t offset, u8 data);
	void bank_switch_rom(offs_t offset, u8 data);
	void bank_switch_ram(u8 data);

	memory_view m_view_ram;

	u8 m_bank_8k;
};


ntnew_device::ntnew_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_ram_device_base<mbc_8k_device_base>(mconfig, GB_ROM_NTNEW, tag, owner, clock),
	m_view_ram(*this, "ram"),
	m_bank_8k(0U)
{
}


image_init_result ntnew_device::load(std::string &message)
{
	// set up ROM and RAM
	set_bank_bits_rom(8);
	set_bank_bits_ram(4);
	if (!check_rom(message) || !check_ram(message))
		return image_init_result::FAIL;
	cart_space()->install_view(0xa000, 0xbfff, m_view_ram);
	install_rom(*cart_space(), *cart_space(), *cart_space());
	install_ram(m_view_ram[0]);

	// install handlers
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			write8sm_delegate(*this, FUNC(ntnew_device::enable_ram)));
	cart_space()->install_write_handler(
			0x2000, 0x2fff,
			write8sm_delegate(*this, FUNC(ntnew_device::bank_switch_rom)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			write8smo_delegate(*this, FUNC(ntnew_device::bank_switch_ram)));

	// all good
	return image_init_result::PASS;
}


void ntnew_device::device_start()
{
	mbc_ram_device_base<mbc_8k_device_base>::device_start();

	save_item(NAME(m_bank_8k));
}


void ntnew_device::device_reset()
{
	mbc_ram_device_base<mbc_8k_device_base>::device_reset();

	m_view_ram.disable();
	m_bank_8k = 0U;

	set_bank_rom_low(2);
	set_bank_rom_high(3);
	set_bank_ram(0);
}


void ntnew_device::enable_ram(offs_t offset, u8 data)
{
	// TODO: what range actually triggers this, and does it still trigger RAM enable?
	if (((offset & 0x1f00) == 0x1400) & (0x55 == data))
	{
		LOG("%s: 8K ROM banking enabled\n", machine().describe_context());
		m_bank_8k = 1U;
	}

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


void ntnew_device::bank_switch_rom(offs_t offset, u8 data)
{
	if (m_bank_8k)
	{
		// TODO: what ranges does the controller actually respond to?
		switch (offset & 0x0f00)
		{
		case 0x0000:
			if (!(data & 0xfe))
				data |= 0x02;
			set_bank_rom_low(data);
			return;

		case 0x0400:
			if (!(data & 0xfe))
				data |= 0x02;
			set_bank_rom_high(data);
			return;
		}
	}

	data <<= 1;
	if (!(data & 0xfe))
		data |= 0x02;
	set_bank_rom_low(data);
	set_bank_rom_high(data | 0x01);
}


void ntnew_device::bank_switch_ram(u8 data)
{
	set_bank_ram(data & 0x0f);
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_NTNEW, device_gb_cart_interface, bus::gameboy::ntnew_device, "gb_rom_ntnew", "Game Boy newer Kasheng/Makon Cartridge")
