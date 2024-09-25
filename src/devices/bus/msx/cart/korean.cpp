// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "korean.h"


namespace {

class msx_cart_korean_25in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_25in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_KOREAN_25IN1, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank%u", 0U)
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

private:
	void bank_w(u8 data);

	memory_bank_array_creator<4> m_rombank;

	u8 m_bank_mask;
};

void msx_cart_korean_25in1_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(m_bank_mask);
}

std::error_condition msx_cart_korean_25in1_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_korean_25in1_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x2000;

	if (size > 256 * 0x2000 || size < 0x8000 || size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_korean_25in1_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);

	page(0)->install_rom(0x0000, 0x1fff, cart_rom_region()->base() + (m_bank_mask * 0x2000));
	page(0)->install_write_handler(0x0000, 0x0000, emu::rw_delegate(*this, FUNC(msx_cart_korean_25in1_device::bank_w)));
	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);

	return std::error_condition();
}

void msx_cart_korean_25in1_device::bank_w(u8 data)
{
	for (int i = 0; i < 4; i++)
	{
		m_rombank[i]->set_entry((data - i) & m_bank_mask);
	}
}





class msx_cart_korean_80in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_80in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_KOREAN_80IN1, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank%u", 0U)
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

private:
	void bank_w(offs_t offset, u8 data);

	memory_bank_array_creator<4> m_rombank;

	u8 m_bank_mask;
};

void msx_cart_korean_80in1_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(i);
}

std::error_condition msx_cart_korean_80in1_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_korean_80in1_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x2000;

	if (size > 256 * 0x2000 || size < 0x8000 || size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_korean_80in1_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);

	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_write_handler(0x4000, 0x4003, emu::rw_delegate(*this, FUNC(msx_cart_korean_80in1_device::bank_w)));
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);

	return std::error_condition();
}

void msx_cart_korean_80in1_device::bank_w(offs_t offset, u8 data)
{
	m_rombank[offset & 0x03]->set_entry(data & m_bank_mask);
}





class msx_cart_korean_90in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_90in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_KOREAN_90IN1, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank%u", 0U)
		, m_view(*this, "view")
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void banking(u8 data);

	memory_bank_array_creator<3> m_rombank;
	memory_view m_view;
	u8 m_bank_mask;
};

void msx_cart_korean_90in1_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0x77, 0x77, emu::rw_delegate(*this, FUNC(msx_cart_korean_90in1_device::banking)));
}

void msx_cart_korean_90in1_device::device_reset()
{
	m_view.select(0);
	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);
	m_rombank[2]->set_entry(0);
}

std::error_condition msx_cart_korean_90in1_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_korean_90in1_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x4000;

	if (size > 64 * 0x4000 || size < 0x8000 || size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_korean_90in1_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	m_rombank[0]->configure_entries(0, banks, cart_rom_region()->base(), 0x4000);
	m_rombank[1]->configure_entries(0, banks, cart_rom_region()->base(), 0x4000);
	m_rombank[2]->configure_entries(0, banks, cart_rom_region()->base() + 0x2000, 0x4000);

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank[0]);

	page(2)->install_view(0x8000, 0xbfff, m_view);
	m_view[0].install_read_bank(0x8000, 0x9fff, m_rombank[1]);
	m_view[0].install_read_bank(0xa000, 0xbfff, m_rombank[2]);
	m_view[1].install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	m_view[1].install_read_bank(0xa000, 0xbfff, m_rombank[1]);

	return std::error_condition();
}

void msx_cart_korean_90in1_device::banking(u8 data)
{
	if (BIT(data, 7) && !BIT(data, 6))
		data &= 0xfe;

	m_rombank[0]->set_entry(data & m_bank_mask);
	m_rombank[1]->set_entry((data + (BIT(data, 7) ? 1 : 0)) & m_bank_mask);
	m_rombank[2]->set_entry((data + (BIT(data, 7) ? 1 : 0)) & m_bank_mask);

	m_view.select((BIT(data, 7) && BIT(data, 6)) ? 1 : 0);
}





class msx_cart_korean_126in1_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_126in1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_KOREAN_126IN1, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank%u", 0U)
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

private:
	void bank_w(offs_t offset, u8 data);

	memory_bank_array_creator<2> m_rombank;
	u8 m_bank_mask;
};

void msx_cart_korean_126in1_device::device_reset()
{
	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(1);
}

std::error_condition msx_cart_korean_126in1_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_korean_126in1_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x4000;

	if (size > 256 * 0x4000 || size < 0x8000 || size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_korean_126in1_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	m_rombank[0]->configure_entries(0, banks, cart_rom_region()->base(), 0x4000);
	m_rombank[1]->configure_entries(0, banks, cart_rom_region()->base(), 0x4000);

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank[0]);
	page(1)->install_write_handler(0x4000, 0x4001, emu::rw_delegate(*this, FUNC(msx_cart_korean_126in1_device::bank_w)));
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank[1]);

	return std::error_condition();
}

void msx_cart_korean_126in1_device::bank_w(offs_t offset, uint8_t data)
{
	m_rombank[offset & 0x01]->set_entry(data & m_bank_mask);
}





class msx_cart_korean_hydlide2_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_korean_hydlide2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_KOREAN_HYDLIDE2, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank")
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

private:
	void bank_w(u8 data);

	memory_bank_creator m_rombank;
	u8 m_bank_mask;
};

void msx_cart_korean_hydlide2_device::device_reset()
{
	m_rombank->set_entry(0);
}

std::error_condition msx_cart_korean_hydlide2_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_korean_hydlide2_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x4000;

	if (size > 256 * 0x4000 || size < 0x8000 || size != banks * 0x4000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_korean_hydlide2_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	m_rombank->configure_entries(0, banks, cart_rom_region()->base(), 0x4000);

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(2)->install_read_bank(0x8000, 0xbfff, m_rombank);
	page(2)->install_write_handler(0x8000, 0x8000, emu::rw_delegate(*this, FUNC(msx_cart_korean_hydlide2_device::bank_w)));

	return std::error_condition();
}

void msx_cart_korean_hydlide2_device::bank_w(uint8_t data)
{
	m_rombank->set_entry(data & m_bank_mask);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KOREAN_25IN1,    msx_cart_interface, msx_cart_korean_25in1_device,    "msx_cart_korean_25in1",    "MSX Cartridge - Korean 25-in-1")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KOREAN_80IN1,    msx_cart_interface, msx_cart_korean_80in1_device,    "msx_cart_korean_80in1",    "MSX Cartridge - Korean 80-in-1")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KOREAN_90IN1,    msx_cart_interface, msx_cart_korean_90in1_device,    "msx_cart_korean_90in1",    "MSX Cartridge - Korean 90-in-1")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KOREAN_126IN1,   msx_cart_interface, msx_cart_korean_126in1_device,   "msx_cart_korean_126in1",   "MSX Cartridge - Korean 126-in-1")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KOREAN_HYDLIDE2, msx_cart_interface, msx_cart_korean_hydlide2_device, "msx_cart_korean_hydlide2", "MSX Cartridge - Korean Hydlide 2")
