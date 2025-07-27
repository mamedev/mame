// license:BSD-3-Clause
// copyright-holders:AJR
/**************************************************************************************************

A simple ROM card for the MZ-800 described in issue A11/90 of the Czech Amatérské Rádio magazine.

**************************************************************************************************/

#include "emu.h"
#include "ar_romcard.h"

#include "imagedev/cartrom.h"
#include "softlist_dev.h"

namespace {

// ======================> ar_romcard_device

class ar_romcard_device : public device_t, public device_mz80_exp_interface, public device_rom_image_interface
{
public:
	// construction/destruction
	ar_romcard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_image_interface implementation
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }
	std::pair<std::error_condition, std::string> call_load() override;

	// device_mz80_exp_interface implementation
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	u8 reset_address_r();
	void reset_address_w(u8 data);
	u8 rom_data_r();
	void increment_address_w(u8 data);

	std::unique_ptr<u8 []> m_rom;
	u16 m_rom_address;
};

ar_romcard_device::ar_romcard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AR_ROMCARD, tag, owner, clock)
	, device_mz80_exp_interface(mconfig, *this)
	, device_rom_image_interface(mconfig, *this)
	, m_rom_address(0)
{
}

void ar_romcard_device::device_start()
{
	m_rom = util::make_unique_clear<u8 []>(0x10000);

	save_item(NAME(m_rom_address));
}

void ar_romcard_device::device_add_mconfig(machine_config &config)
{
	SOFTWARE_LIST(config, "rom_list").set_original("mz800_rom");
}

std::pair<std::error_condition, std::string> ar_romcard_device::call_load()
{
	u32 const size = loaded_through_softlist() ? get_software_region_length("rom") : length();
	if (size != 0x4000 && size != 0x8000 && size != 0x10000)
	{
		osd_printf_error("ar_romcard: Incorrect size for 27128/27256/27512 ROM\n");
		return std::make_pair(image_error::INVALIDLENGTH, std::string());
	}

	if (loaded_through_softlist())
		memcpy(&m_rom[0], get_software_region("rom"), size);
	else
		fread(&m_rom[0], size);

	// Mirror contents
	for (offs_t b = size; b < 0x10000; b += size)
		memcpy(&m_rom[b], &m_rom[0], size);

	return std::make_pair(std::error_condition(), std::string());
}

void ar_romcard_device::io_map(address_map &map)
{
	map(0xf8, 0xf8).mirror(0xff00).rw(FUNC(ar_romcard_device::reset_address_r), FUNC(ar_romcard_device::reset_address_w));
	map(0xf9, 0xf9).mirror(0xff00).rw(FUNC(ar_romcard_device::rom_data_r), FUNC(ar_romcard_device::increment_address_w));
}

u8 ar_romcard_device::reset_address_r()
{
	if (!machine().side_effects_disabled())
		m_rom_address = 0;
	return 0xff; // open bus
}

void ar_romcard_device::reset_address_w(u8 data)
{
	m_rom_address = 0;
}

u8 ar_romcard_device::rom_data_r()
{
	u8 data = m_rom[m_rom_address];
	if (!machine().side_effects_disabled())
		m_rom_address++;
	return data;
}

void ar_romcard_device::increment_address_w(u8 data)
{
	m_rom_address++;
}

} // anonymous namespace

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(AR_ROMCARD, device_mz80_exp_interface, ar_romcard_device, "ar_romcard", u8"Amatérské Rádio ROM Card")
