// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

TODO:
- Verify real contents of the kanji roms. The current dumps seem to have
  been read from inside a running MSX machine. The content retrieved that
  way may not be how they are stored in the rom.

**********************************************************************************/

#include "emu.h"
#include "kanji.h"

#include "speaker.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class msx_cart_kanji_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_kanji_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_kanji_device(mconfig, MSX_CART_KANJI, tag, owner, clock)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	msx_cart_kanji_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override;
	virtual void device_reset() override;

	std::error_condition validate_kanji_regions(std::string &message);
	void install_kanji_handlers();

	u8 kanji_r(offs_t offset);
	void kanji_w(offs_t offset, u8 data);

	u32 m_kanji_mask;
	u32 m_kanji_address;
};

msx_cart_kanji_device::msx_cart_kanji_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_kanji_mask(0)
	, m_kanji_address(0)
{
}

void msx_cart_kanji_device::device_reset()
{
	m_kanji_address = 0;
}

void msx_cart_kanji_device::device_start()
{
	save_item(NAME(m_kanji_address));
}

std::error_condition msx_cart_kanji_device::validate_kanji_regions(std::string &message)
{
	if (!cart_kanji_region())
	{
		message = "msx_cart_kanji_device: Required region 'kanji' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_kanji_region()->bytes() != 0x20000)
	{
		message = "msx_cart_kanji_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	return std::error_condition();
}

void msx_cart_kanji_device::install_kanji_handlers()
{
	m_kanji_mask = cart_kanji_region()->bytes() - 1;

	// Install IO read/write handlers
	io_space().install_write_tap(0xd8, 0xd9, "kanji_w", [this] (offs_t ofs, u8 &data, u8) { this->kanji_w(ofs, data); });
	io_space().install_read_tap(0xd9, 0xd9, "kanji_r", [this] (offs_t ofs, u8 &data, u8) { data &= this->kanji_r(ofs); });
}

std::error_condition msx_cart_kanji_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = validate_kanji_regions(message);
	if (result)
		return result;

	install_kanji_handlers();

	return std::error_condition();
}

u8 msx_cart_kanji_device::kanji_r(offs_t offset)
{
	u8 result = cart_kanji_region()->base()[m_kanji_address];

	if (!machine().side_effects_disabled())
	{
		m_kanji_address = (m_kanji_address & ~0x1f) | ((m_kanji_address + 1) & 0x1f);
	}
	return result;
}

void msx_cart_kanji_device::kanji_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
		m_kanji_address = (m_kanji_address & 0x007e0) | ((data & 0x3f) << 11);
	else
		m_kanji_address = (m_kanji_address & 0x1f800) | ((data & 0x3f) << 5);

	m_kanji_address = m_kanji_address & m_kanji_mask;
}



class msx_cart_msxwrite_device : public msx_cart_kanji_device
{
public:
	msx_cart_msxwrite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_kanji_device(mconfig, MSX_CART_MSXWRITE, tag, owner, clock)
		, m_rombank(*this, "rombank%u", 0U)
		, m_kanji_switch(*this, "KANJI")
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;

private:
	static constexpr size_t BANK_SIZE = 0x4000;

	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<2> m_rombank;
	required_ioport m_kanji_switch;
	u8 m_bank_mask;
};

static INPUT_PORTS_START(msxwrite_kanji_enable_switch)
	PORT_START("KANJI")
	PORT_CONFNAME(0x01, 0x01, "Kanji is")
	PORT_CONFSETTING(0x00, "disabled")
	PORT_CONFSETTING(0x01, "enabled")
INPUT_PORTS_END

ioport_constructor msx_cart_msxwrite_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msxwrite_kanji_enable_switch);
}

void msx_cart_msxwrite_device::device_reset()
{
	msx_cart_kanji_device::device_reset();

	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);

	if (BIT(m_kanji_switch->read(), 0))
		install_kanji_handlers();
}

std::error_condition msx_cart_msxwrite_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = validate_kanji_regions(message);
	if (result)
		return result;

	if (!cart_rom_region())
	{
		message = "msx_cart_msxwrite_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / BANK_SIZE;

	if (size > 256 * BANK_SIZE || size != banks * BANK_SIZE || (~(banks - 1) % banks))
	{
		message = "msx_cart_msxwrite_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	for (int i = 0; i < 2; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), BANK_SIZE);

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank[0]);
	// The rom writes to 6fff and 7fff for banking, unknown whether
	// other locations also trigger banking.
	page(1)->install_write_handler(0x6fff, 0x6fff, emu::rw_delegate(*this, FUNC(msx_cart_msxwrite_device::bank_w<0>)));
	page(1)->install_write_handler(0x7fff, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_msxwrite_device::bank_w<1>)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank[1]);

	return std::error_condition();
}

template <int Bank>
void msx_cart_msxwrite_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & m_bank_mask);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KANJI, msx_cart_interface, msx_cart_kanji_device, "msx_cart_kanji", "MSX Cartridge - Kanji")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MSXWRITE, msx_cart_interface, msx_cart_msxwrite_device, "msx_cart_msxwrite", "MSX Cartridge - MSXWRITE")
