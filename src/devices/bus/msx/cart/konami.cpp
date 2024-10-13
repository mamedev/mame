// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "konami.h"

#include "machine/intelfsh.h"

#include "sound/k051649.h"
#include "sound/vlm5030.h"
#include "sound/dac.h"

#include "speaker.h"


namespace {

class msx_cart_konami_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_konami_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_KONAMI, tag, owner, clock)
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
	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<4> m_rombank;
	u8 m_bank_mask;
};

void msx_cart_konami_device::device_reset()
{
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(i);
}

std::error_condition msx_cart_konami_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_konami_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x2000;

	if (size > 256 * 0x2000 || size < 4 * 0x2000 || size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_konami_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);

	page(0)->install_read_bank(0x0000, 0x1fff, m_rombank[0]);
	page(0)->install_read_bank(0x2000, 0x3fff, m_rombank[1]);
	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_write_handler(0x4000, 0x47ff, 0, 0x1000, 0, emu::rw_delegate(*this, FUNC(msx_cart_konami_device::bank_w<0>)));
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(1)->install_write_handler(0x6000, 0x67ff, 0, 0x1000, 0, emu::rw_delegate(*this, FUNC(msx_cart_konami_device::bank_w<1>)));
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_write_handler(0x8000, 0x87ff, 0, 0x1000, 0, emu::rw_delegate(*this, FUNC(msx_cart_konami_device::bank_w<2>)));
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);
	page(2)->install_write_handler(0xa000, 0xa7ff, 0, 0x1000, 0, emu::rw_delegate(*this, FUNC(msx_cart_konami_device::bank_w<3>)));
	page(3)->install_read_bank(0xc000, 0xdfff, m_rombank[2]);
	page(3)->install_read_bank(0xe000, 0xffff, m_rombank[3]);

	return std::error_condition();
}

template <int Bank>
void msx_cart_konami_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & m_bank_mask);
}




class msx_cart_konami_scc_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_konami_scc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_KONAMI_SCC, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_k051649(*this, "k051649")
		, m_rombank(*this, "rombank%u", 0U)
		, m_scc_view(*this, "scc_view")
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	template <int Bank> void bank_w(u8 data);

	required_device<k051649_device> m_k051649;
	memory_bank_array_creator<4> m_rombank;
	memory_view m_scc_view;

	u8 m_bank_mask;
};

void msx_cart_konami_scc_device::device_add_mconfig(machine_config &config)
{
	K051649(config, m_k051649, DERIVED_CLOCK(1, 1));
	if (parent_slot())
		m_k051649->add_route(ALL_OUTPUTS, soundin(), 0.4);
}

void msx_cart_konami_scc_device::device_reset()
{
	m_scc_view.select(0);
	for (int i = 0; i < 4; i++)
		m_rombank[i]->set_entry(i);
}

std::error_condition msx_cart_konami_scc_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_konami_scc_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x2000;

	if (size > 256 * 0x2000 || size < 0x8000 || size != banks * 0x2000 || (~(banks - 1) % banks))
	{
		message = "msx_cart_konami_scc_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	m_bank_mask = banks - 1;

	for (int i = 0; i < 4; i++)
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);

	page(0)->install_read_bank(0x0000, 0x1fff, m_rombank[2]);
	page(0)->install_read_bank(0x2000, 0x3fff, m_rombank[3]);
	page(1)->install_read_bank(0x4000, 0x5fff, m_rombank[0]);
	page(1)->install_write_handler(0x5000, 0x57ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_scc_device::bank_w<0>)));
	page(1)->install_read_bank(0x6000, 0x7fff, m_rombank[1]);
	page(1)->install_write_handler(0x7000, 0x77ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_scc_device::bank_w<1>)));
	page(2)->install_view(0x8000, 0x9fff, m_scc_view);
	m_scc_view[0].install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	m_scc_view[0].install_write_handler(0x9000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_scc_device::bank_w<2>)));
	m_scc_view[1].install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	m_scc_view[1].install_write_handler(0x9000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_scc_device::bank_w<2>)));
	m_scc_view[1].install_read_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_waveform_r)));
	m_scc_view[1].install_write_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_waveform_w)));
	m_scc_view[1].install_write_handler(0x9880, 0x9889, 0, 0x0710, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_frequency_w)));
	m_scc_view[1].install_write_handler(0x988a, 0x988e, 0, 0x0710, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_volume_w)));
	m_scc_view[1].install_write_handler(0x988f, 0x988f, 0, 0x0710, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_keyonoff_w)));
	m_scc_view[1].install_read_handler(0x98e0, 0x98e0, 0, 0x071f, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_test_r)));
	m_scc_view[1].install_write_handler(0x98e0, 0x98e0, 0, 0x071f, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_test_w)));
	page(2)->install_read_bank(0x8000, 0x9fff, m_rombank[2]);
	page(2)->install_write_handler(0x9000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_scc_device::bank_w<2>)));
	page(2)->install_read_bank(0xa000, 0xbfff, m_rombank[3]);
	page(2)->install_write_handler(0xb000, 0xb7ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_scc_device::bank_w<3>)));
	page(3)->install_read_bank(0xc000, 0xdfff, m_rombank[0]);
	page(3)->install_read_bank(0xe000, 0xffff, m_rombank[1]);

	return std::error_condition();
}

template <int Bank>
void msx_cart_konami_scc_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & m_bank_mask);
	if (Bank == 2)
	{
		m_scc_view.select(((data & 0x3f) == 0x3f) ? 1 : 0);
	}
}






class msx_cart_sunrise_scc_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_sunrise_scc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_SUNRISE_SCC, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_k051649(*this, "k051649")
		, m_flash(*this, "flash")
		, m_scc_view(*this, "scc_view")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { save_item(NAME(m_selected_bank)); }
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	template <int Bank> u8 flash_r(offs_t offset);
	template <int Bank> void flash_w(offs_t offset, u8 data);
	template <int Bank> void bank_w(offs_t offset, u8 data);

	required_device<k051649_device> m_k051649;
	required_device<intelfsh8_device> m_flash;
	memory_view m_scc_view;

	u8 m_selected_bank[4];
};

ROM_START(msx_cart_sunrise_scc)
	ROM_REGION(0x80000, "flash", ROMREGION_ERASEFF)
ROM_END

const tiny_rom_entry *msx_cart_sunrise_scc_device::device_rom_region() const
{
	return ROM_NAME(msx_cart_sunrise_scc);
}

void msx_cart_sunrise_scc_device::device_add_mconfig(machine_config &config)
{
	K051649(config, m_k051649, DERIVED_CLOCK(1, 1));
	if (parent_slot())
		m_k051649->add_route(ALL_OUTPUTS, soundin(), 0.4);

	AMD_29F040(config, m_flash);
}

void msx_cart_sunrise_scc_device::device_reset()
{
	m_scc_view.select(0);
	for (int i = 0; i < 4; i++)
		m_selected_bank[i] = i;
}

std::error_condition msx_cart_sunrise_scc_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_sunrise_scc_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	if (size != 0x80000)
	{
		message = "msx_cart_sunrise_scc_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	u8 *flash = memregion("flash")->base();
	memcpy(flash, cart_rom_region()->base(), size);

	page(0)->install_read_handler(0x0000, 0x1fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<2>)));
	page(0)->install_write_handler(0x0000, 0x1fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_w<2>)));
	page(0)->install_read_handler(0x2000, 0x3fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<3>)));
	page(0)->install_write_handler(0x2000, 0x3fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_w<3>)));
	page(1)->install_read_handler(0x4000, 0x5fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<0>)));
	page(1)->install_write_handler(0x4000, 0x5fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::bank_w<0>)));
	page(1)->install_read_handler(0x6000, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<1>)));
	page(1)->install_write_handler(0x6000, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::bank_w<1>)));
	page(2)->install_view(0x8000, 0x9fff, m_scc_view);
	m_scc_view[0].install_read_handler(0x8000, 0x9fff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<2>)));
	m_scc_view[0].install_write_handler(0x8000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::bank_w<2>)));
	m_scc_view[1].install_read_handler(0x8000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<2>)));
	m_scc_view[1].install_write_handler(0x8000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::bank_w<2>)));
	m_scc_view[1].install_read_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_waveform_r)));
	m_scc_view[1].install_write_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_waveform_w)));
	m_scc_view[1].install_write_handler(0x9880, 0x9889, 0, 0x0710, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_frequency_w)));
	m_scc_view[1].install_write_handler(0x988a, 0x988e, 0, 0x0710, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_volume_w)));
	m_scc_view[1].install_write_handler(0x988f, 0x988f, 0, 0x0710, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_keyonoff_w)));
	m_scc_view[1].install_read_handler(0x98e0, 0x98e0, 0, 0x071f, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_test_r)));
	m_scc_view[1].install_write_handler(0x98e0, 0x98e0, 0, 0x071f, 0, emu::rw_delegate(m_k051649, FUNC(k051649_device::k051649_test_w)));
	page(2)->install_read_handler(0xa000, 0xbfff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<3>)));
	page(2)->install_write_handler(0xa000, 0xbfff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::bank_w<3>)));
	page(3)->install_read_handler(0xc000, 0xdfff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<0>)));
	page(3)->install_write_handler(0xc000, 0xdfff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_w<0>)));
	page(3)->install_read_handler(0xe000, 0xffff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_r<1>)));
	page(3)->install_write_handler(0xe000, 0xffff, emu::rw_delegate(*this, FUNC(msx_cart_sunrise_scc_device::flash_w<1>)));

	return std::error_condition();
}

template <int Bank>
u8 msx_cart_sunrise_scc_device::flash_r(offs_t offset)
{
	return m_flash->read(m_selected_bank[Bank] * 0x2000 + offset);
}

template <int Bank>
void msx_cart_sunrise_scc_device::flash_w(offs_t offset, u8 data)
{
	m_flash->write(m_selected_bank[Bank] * 0x2000 + offset, data);
}

template <int Bank>
void msx_cart_sunrise_scc_device::bank_w(offs_t offset, u8 data)
{
	if ((offset & 0x1800) == 0x1000)
	{
		m_selected_bank[Bank] = data & 0x3f;
		if (Bank == 2)
			m_scc_view.select(((data & 0x3f) == 0x3f) ? 1 : 0);
	}
	else
		m_flash->write(m_selected_bank[Bank] * 0x2000 + offset, data);
}






class msx_cart_gamemaster2_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_gamemaster2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_GAMEMASTER2, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank%u", 0U)
		, m_rambank(*this, "rambank%u", 0U)
		, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"} }
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

private:
	template <int Bank> void bank_w(u8 data);

	memory_bank_array_creator<3> m_rombank;
	memory_bank_array_creator<3> m_rambank;
	memory_view m_view[3];
};

void msx_cart_gamemaster2_device::device_reset()
{
	m_rombank[0]->set_entry(1);
	m_rombank[1]->set_entry(1);
	m_rombank[2]->set_entry(2);
	for (int i = 0; i < 3; i++)
		m_view[i].select(0);
}

std::error_condition msx_cart_gamemaster2_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_gamemaster2_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (!cart_sram_region())
	{
		message = "msx_cart_gamemaster2_device: Required region 'sram' was not found.";
		return image_error::INTERNAL;
	}

	const u32 size = cart_rom_region()->bytes();
	const u16 banks = size / 0x2000;

	if (size != 0x20000)
	{
		message = "msx_cart_gamemaster2_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	if (cart_sram_region()->bytes() != 0x2000)
	{
		message = "msx_cart_gamemaster2_device: Region 'sram' has unsupported size.";
		return image_error::BADSOFTWARE;
	}

	for (int i = 0; i < 3; i++)
	{
		m_rombank[i]->configure_entries(0, banks, cart_rom_region()->base(), 0x2000);
		m_rambank[i]->configure_entries(0, 2, cart_sram_region()->base(), 0x1000);
	}

	page(1)->install_rom(0x4000, 0x5fff, cart_rom_region()->base());

	page(1)->install_view(0x6000, 0x7fff, m_view[0]);
	m_view[0][0].install_read_bank(0x6000, 0x7fff, m_rombank[0]);
	m_view[0][0].install_write_handler(0x6000, 0x6fff, emu::rw_delegate(*this, FUNC(msx_cart_gamemaster2_device::bank_w<0>)));
	m_view[0][1].install_read_bank(0x6000, 0x6fff, 0x1000, m_rambank[0]);
	m_view[0][1].install_write_handler(0x6000, 0x6fff, emu::rw_delegate(*this, FUNC(msx_cart_gamemaster2_device::bank_w<0>)));

	page(2)->install_view(0x8000, 0x9fff, m_view[1]);
	m_view[1][0].install_read_bank(0x8000, 0x9fff, m_rombank[1]);
	m_view[1][0].install_write_handler(0x8000, 0x8fff, emu::rw_delegate(*this, FUNC(msx_cart_gamemaster2_device::bank_w<1>)));
	m_view[1][1].install_read_bank(0x8000, 0x8fff, 0x1000, m_rambank[1]);
	m_view[1][1].install_write_handler(0x8000, 0x8fff, emu::rw_delegate(*this, FUNC(msx_cart_gamemaster2_device::bank_w<1>)));

	page(2)->install_view(0xa000, 0xbfff, m_view[2]);
	m_view[2][0].install_read_bank(0xa000, 0xbfff, m_rombank[2]);
	m_view[2][0].install_write_handler(0xa000, 0xafff, emu::rw_delegate(*this, FUNC(msx_cart_gamemaster2_device::bank_w<2>)));
	m_view[2][1].install_read_bank(0xa000, 0xafff, m_rambank[2]);
	m_view[2][1].install_write_handler(0xa000, 0xafff, emu::rw_delegate(*this, FUNC(msx_cart_gamemaster2_device::bank_w<2>)));
	m_view[2][1].install_readwrite_bank(0xb000, 0xbfff, m_rambank[2]);

	return std::error_condition();
}

template <int Bank>
void msx_cart_gamemaster2_device::bank_w(u8 data)
{
	m_rombank[Bank]->set_entry(data & 0x0f);
	m_rambank[Bank]->set_entry(BIT(data, 5) ? 1 : 0);
	m_view[Bank].select(BIT(data, 4) ? 1 : 0);
}





class msx_cart_synthesizer_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_SYNTHESIZER, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_dac(*this, "dac")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	// device_t implementation
	virtual void device_start() override { }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<dac_8bit_r2r_device> m_dac;
};

void msx_cart_synthesizer_device::device_add_mconfig(machine_config &config)
{
	DAC_8BIT_R2R(config, m_dac, 0); // unknown DAC
	if (parent_slot())
		m_dac->add_route(ALL_OUTPUTS, soundin(), 0.3);
}

std::error_condition msx_cart_synthesizer_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_synthesizer_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x8000)
	{
		message = "msx_cart_synthesizer_device: Region 'rom' has unsupported size.";
		return image_error::BADSOFTWARE;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	page(1)->install_write_handler(0x4000, 0x4000, 0, 0x3fef, 0, emu::rw_delegate(m_dac, FUNC(dac_byte_interface::write)));
	page(2)->install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);

	return std::error_condition();
}




class msx_cart_konami_sound_device : public device_t, public msx_cart_interface
{
public:
	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	msx_cart_konami_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 min_rambank, u8 max_rambank)
		: device_t(mconfig, type, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_k052539(*this, "k052539")
		, m_rambank(*this, "rambank%u", 0U)
		, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"} }
		, m_min_rambank(min_rambank)
		, m_max_rambank(max_rambank)
		, m_selected_bank{0, 0, 0, 0}
		, m_control(0)
	{ }

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr u8 VIEW_READ = 0;
	static constexpr u8 VIEW_RAM = 1;
	static constexpr u8 VIEW_INVALID = 2;
	static constexpr u8 VIEW_SCC = 4;

	template <int Bank> void bank_w(u8 data);
	template <int Bank> void switch_bank();
	void control_w(u8 data);

	// This is actually a K052539
	required_device<k051649_device> m_k052539;
	memory_bank_array_creator<4> m_rambank;
	memory_view m_view[4];

	u8 m_min_rambank;
	u8 m_max_rambank;
	u8 m_selected_bank[4];
	u8 m_control;
};

void msx_cart_konami_sound_device::device_add_mconfig(machine_config &config)
{
	K051649(config, m_k052539, DERIVED_CLOCK(1, 1));
	if (parent_slot())
		m_k052539->add_route(ALL_OUTPUTS, soundin(), 0.4);
}

void msx_cart_konami_sound_device::device_start()
{
	save_item(NAME(m_selected_bank));
	save_item(NAME(m_control));
}

void msx_cart_konami_sound_device::device_reset()
{
	m_control = 0;
	for (int i = 0; i < 4; i++)
	{
		m_selected_bank[i] = i;
	}
	switch_bank<0>();
	switch_bank<1>();
	switch_bank<2>();
	switch_bank<3>();
}

std::error_condition msx_cart_konami_sound_device::initialize_cartridge(std::string &message)
{
	for (int i = 0; i < 4; i++)
	{
		m_rambank[i]->configure_entries(0, 8, cart_ram_region()->base(), 0x2000);
	}

	// TODO Mirrors at 0000-3fff and c000-ffff

	page(1)->install_view(0x4000, 0x5fff, m_view[0]);
	m_view[0][VIEW_READ].install_read_bank(0x4000, 0x5fff, m_rambank[0]);
	m_view[0][VIEW_READ].install_write_handler(0x5000, 0x57ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<0>)));
	m_view[0][VIEW_RAM].install_readwrite_bank(0x4000, 0x5fff, m_rambank[0]);
	m_view[0][VIEW_INVALID].install_write_handler(0x5000, 0x57ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<0>)));
	m_view[0][VIEW_INVALID | VIEW_RAM];

	page(1)->install_view(0x6000, 0x7fff, m_view[1]);
	m_view[1][VIEW_READ].install_read_bank(0x6000, 0x7fff, m_rambank[1]);
	m_view[1][VIEW_READ].install_write_handler(0x7000, 0x77ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<1>)));
	m_view[1][VIEW_RAM].install_readwrite_bank(0x6000, 0x7fff, m_rambank[1]);
	m_view[1][VIEW_INVALID].install_write_handler(0x7000, 0x77ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<1>)));
	m_view[1][VIEW_INVALID | VIEW_RAM];

	page(2)->install_view(0x8000, 0x9fff, m_view[2]);
	m_view[2][VIEW_READ].install_read_bank(0x8000, 0x9fff, m_rambank[2]);
	m_view[2][VIEW_READ].install_write_handler(0x9000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<2>)));
	m_view[2][VIEW_RAM].install_readwrite_bank(0x8000, 0x9fff, m_rambank[2]);
	m_view[2][VIEW_INVALID].install_write_handler(0x9000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<2>)));
	m_view[2][VIEW_INVALID | VIEW_RAM];
	m_view[2][VIEW_SCC | VIEW_READ].install_read_bank(0x8000, 0x9fff, m_rambank[2]);
	m_view[2][VIEW_SCC | VIEW_READ].install_write_handler(0x9000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<2>)));
	m_view[2][VIEW_SCC | VIEW_READ].install_read_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_waveform_r)));
	m_view[2][VIEW_SCC | VIEW_READ].install_write_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_waveform_w)));
	m_view[2][VIEW_SCC | VIEW_READ].install_write_handler(0x9880, 0x9889, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_frequency_w)));
	m_view[2][VIEW_SCC | VIEW_READ].install_write_handler(0x988a, 0x988e, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_volume_w)));
	m_view[2][VIEW_SCC | VIEW_READ].install_write_handler(0x988f, 0x988f, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_keyonoff_w)));
	m_view[2][VIEW_SCC | VIEW_READ].install_read_handler(0x98c0, 0x98c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_r)));
	m_view[2][VIEW_SCC | VIEW_READ].install_write_handler(0x98c0, 0x98c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_w)));
	m_view[2][VIEW_SCC | VIEW_RAM].install_readwrite_bank(0x8000, 0x9fff, m_rambank[2]);
	m_view[2][VIEW_SCC | VIEW_RAM].install_read_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_waveform_r)));
	m_view[2][VIEW_SCC | VIEW_RAM].install_read_handler(0x98c0, 0x98c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_r)));
	m_view[2][VIEW_SCC | VIEW_INVALID].install_write_handler(0x9000, 0x97ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<2>)));
	m_view[2][VIEW_SCC | VIEW_INVALID].install_read_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_waveform_r)));
	m_view[2][VIEW_SCC | VIEW_INVALID].install_write_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_waveform_w)));
	m_view[2][VIEW_SCC | VIEW_INVALID].install_write_handler(0x9880, 0x9889, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_frequency_w)));
	m_view[2][VIEW_SCC | VIEW_INVALID].install_write_handler(0x988a, 0x988e, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_volume_w)));
	m_view[2][VIEW_SCC | VIEW_INVALID].install_write_handler(0x988f, 0x988f, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_keyonoff_w)));
	m_view[2][VIEW_SCC | VIEW_INVALID].install_read_handler(0x98c0, 0x98c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_r)));
	m_view[2][VIEW_SCC | VIEW_INVALID].install_write_handler(0x98c0, 0x98c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_w)));
	m_view[2][VIEW_SCC | VIEW_INVALID | VIEW_RAM];
	m_view[2][VIEW_SCC | VIEW_INVALID | VIEW_RAM].install_read_handler(0x9800, 0x987f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_waveform_r)));
	m_view[2][VIEW_SCC | VIEW_INVALID | VIEW_RAM].install_read_handler(0x98c0, 0x98c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_r)));

	page(2)->install_view(0xa000, 0xbfff, m_view[3]);
	m_view[3][VIEW_READ].install_read_bank(0xa000, 0xbfff, m_rambank[3]);
	m_view[3][VIEW_READ].install_write_handler(0xb000, 0xb7ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<3>)));
	m_view[3][VIEW_RAM].install_readwrite_bank(0xa000, 0xbfff, m_rambank[3]);
	m_view[3][VIEW_INVALID].install_write_handler(0xb000, 0xb7ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<3>)));
	m_view[3][VIEW_INVALID | VIEW_RAM];
	m_view[3][VIEW_SCC | VIEW_READ].install_read_bank(0xa000, 0xbfff, m_rambank[3]);
	m_view[3][VIEW_SCC | VIEW_READ].install_write_handler(0xb000, 0xb7ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<3>)));
	m_view[3][VIEW_SCC | VIEW_READ].install_read_handler(0xb800, 0xb89f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k052539_waveform_r)));
	m_view[3][VIEW_SCC | VIEW_READ].install_write_handler(0xb800, 0xb89f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k052539_waveform_w)));
	m_view[3][VIEW_SCC | VIEW_READ].install_write_handler(0xb8a0, 0xb8a9, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_frequency_w)));
	m_view[3][VIEW_SCC | VIEW_READ].install_write_handler(0xb8aa, 0xb8ae, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_volume_w)));
	m_view[3][VIEW_SCC | VIEW_READ].install_write_handler(0xb8af, 0xb8af, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_keyonoff_w)));
	m_view[3][VIEW_SCC | VIEW_READ].install_read_handler(0xb8c0, 0xb8c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_r)));
	m_view[3][VIEW_SCC | VIEW_READ].install_write_handler(0xb8c0, 0xb8c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_w)));
	m_view[3][VIEW_SCC | VIEW_RAM].install_readwrite_bank(0xa000, 0xbfff, m_rambank[3]);
	m_view[3][VIEW_SCC | VIEW_RAM].install_read_handler(0xb800, 0xb89f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k052539_waveform_r)));
	m_view[3][VIEW_SCC | VIEW_RAM].install_read_handler(0xb8c0, 0xb8c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_r)));
	m_view[3][VIEW_SCC | VIEW_INVALID].install_write_handler(0xb000, 0xb7ff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::bank_w<3>)));
	m_view[3][VIEW_SCC | VIEW_INVALID].install_read_handler(0xb800, 0xb89f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k052539_waveform_r)));
	m_view[3][VIEW_SCC | VIEW_INVALID].install_write_handler(0xb800, 0xb89f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k052539_waveform_w)));
	m_view[3][VIEW_SCC | VIEW_INVALID].install_write_handler(0xb8a0, 0xb8a9, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_frequency_w)));
	m_view[3][VIEW_SCC | VIEW_INVALID].install_write_handler(0xb8aa, 0xb8ae, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_volume_w)));
	m_view[3][VIEW_SCC | VIEW_INVALID].install_write_handler(0xb8af, 0xb8af, 0, 0x0710, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_keyonoff_w)));
	m_view[3][VIEW_SCC | VIEW_INVALID].install_read_handler(0xb8c0, 0xb8c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_r)));
	m_view[3][VIEW_SCC | VIEW_INVALID].install_write_handler(0xb8c0, 0xb8c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_w)));
	m_view[3][VIEW_SCC | VIEW_INVALID | VIEW_RAM];
	m_view[3][VIEW_SCC | VIEW_INVALID| VIEW_RAM].install_read_handler(0xb800, 0xb89f, 0, 0x0700, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k052539_waveform_r)));
	m_view[3][VIEW_SCC | VIEW_INVALID| VIEW_RAM].install_read_handler(0xb8c0, 0xb8c0, 0, 0x071f, 0, emu::rw_delegate(m_k052539, FUNC(k051649_device::k051649_test_r)));

	page(2)->install_write_handler(0xbffe, 0xbfff, emu::rw_delegate(*this, FUNC(msx_cart_konami_sound_device::control_w)));

	return std::error_condition();
}

void msx_cart_konami_sound_device::control_w(u8 data)
{
	m_control = data;

	switch_bank<0>();
	switch_bank<1>();
	switch_bank<2>();
	switch_bank<3>();
}

template <int Bank>
void msx_cart_konami_sound_device::switch_bank()
{
	u8 view = VIEW_READ;
	if ((m_selected_bank[Bank] & 0x0f) >= m_min_rambank && (m_selected_bank[Bank] & 0x0f) <= m_max_rambank)
		m_rambank[Bank]->set_entry(m_selected_bank[Bank] & 0x07);
	else
		view |= VIEW_INVALID;
	if (BIT(m_control, 4))
		view |= VIEW_RAM;
	else
	{
		if (Bank < 2 && BIT(m_control, Bank))
			view |= VIEW_RAM;
		else if (Bank == 2 && BIT(m_control, Bank) && BIT(m_control, 5))
			view |= VIEW_RAM;
	}
	if (Bank == 2 && !BIT(m_control, 5) && (m_selected_bank[2] & 0x3f) == 0x3f)
		view |= VIEW_SCC;
	if (Bank == 3 && BIT(m_control, 5) && BIT(m_selected_bank[3], 7))
		view |= VIEW_SCC;

	m_view[Bank].select(view);
}

template <int Bank>
void msx_cart_konami_sound_device::bank_w(u8 data)
{
	m_selected_bank[Bank] = data;
	switch_bank<Bank>();
}



class msx_cart_konami_sound_snatcher_device : public msx_cart_konami_sound_device
{
public:
	// The Snatcher Sound cartridge has 64KB RAM available by selecting ram banks 0-7
	msx_cart_konami_sound_snatcher_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_konami_sound_device(mconfig, MSX_CART_SOUND_SNATCHER, tag, owner, clock, 0, 7)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;
};

std::error_condition msx_cart_konami_sound_snatcher_device::initialize_cartridge(std::string &message)
{
	if (!cart_ram_region())
	{
		message = "msx_cart_konami_sound_snatcher_device: Required region 'ram' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_ram_region()->bytes() != 0x10000)
	{
		message = "msx_cart_konami_sound_snatcher_device: Region 'ram' has unsupported size.";
		return image_error::BADSOFTWARE;
	}

	return msx_cart_konami_sound_device::initialize_cartridge(message);
}


class msx_cart_konami_sound_sdsnatcher_device : public msx_cart_konami_sound_device
{
public:
	// The SD Snatcher Sound cartridge has 64KB RAM available by selecting ram banks 8-15
	msx_cart_konami_sound_sdsnatcher_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_konami_sound_device(mconfig, MSX_CART_SOUND_SDSNATCHER, tag, owner, clock, 8, 15)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;
};

std::error_condition msx_cart_konami_sound_sdsnatcher_device::initialize_cartridge(std::string &message)
{
	if (!cart_ram_region())
	{
		message = "msx_cart_konami_sound_sdsnatcher_device: Required region 'ram' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_ram_region()->bytes() != 0x10000)
	{
		message = "msx_cart_konami_sound_sdsnatcher_device: Region 'ram' has unsupported size.";
		return image_error::BADSOFTWARE;
	}

	return msx_cart_konami_sound_device::initialize_cartridge(message);
}



class msx_cart_keyboard_master_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_keyboard_master_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_KEYBOARD_MASTER, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_vlm5030(*this, "vlm5030")
	{ }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual std::error_condition initialize_cartridge(std::string &message) override;

private:
	required_device<vlm5030_device> m_vlm5030;

	uint8_t read_vlm(offs_t offset);
	void io_20_w(uint8_t data);
	uint8_t io_00_r();

	void vlm_map(address_map &map) ATTR_COLD;
};

void msx_cart_keyboard_master_device::vlm_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(msx_cart_keyboard_master_device::read_vlm));
}

void msx_cart_keyboard_master_device::device_add_mconfig(machine_config &config)
{
	VLM5030(config, m_vlm5030, DERIVED_CLOCK(1, 1));
	if (parent_slot())
		m_vlm5030->add_route(ALL_OUTPUTS, soundin(), 1.0);
	m_vlm5030->set_addrmap(0, &msx_cart_keyboard_master_device::vlm_map);
}

void msx_cart_keyboard_master_device::device_start()
{
	// Install IO read/write handlers
	io_space().install_write_handler(0x00, 0x00, emu::rw_delegate(*m_vlm5030, FUNC(vlm5030_device::data_w)));
	io_space().install_write_handler(0x20, 0x20, emu::rw_delegate(*this, FUNC(msx_cart_keyboard_master_device::io_20_w)));
	io_space().install_read_handler(0x00, 0x00, emu::rw_delegate(*this, FUNC(msx_cart_keyboard_master_device::io_00_r)));
}

std::error_condition msx_cart_keyboard_master_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_keyboard_master_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (!cart_vlm5030_region())
	{
		message = "msx_cart_keyboard_master_device: Required region 'vlm5030' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x4000)
	{
		message = "msx_cart_keyboard_master_device: Region 'rom' has unsupported size.";
		return image_error::BADSOFTWARE;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());

	return std::error_condition();
}

uint8_t msx_cart_keyboard_master_device::read_vlm(offs_t offset)
{
	if (offset < cart_vlm5030_region()->bytes())
		return cart_vlm5030_region()->base()[offset];
	else
		return 0xff;
}

void msx_cart_keyboard_master_device::io_20_w(uint8_t data)
{
	m_vlm5030->rst(BIT(data, 0) ? 1 : 0);
	m_vlm5030->vcu(BIT(data, 2) ? 1 : 0);
	m_vlm5030->st(BIT(data, 1) ? 1 : 0);
}

uint8_t msx_cart_keyboard_master_device::io_00_r()
{
	return m_vlm5030->bsy() ? 0x10 : 0x00;
}



class msx_cart_ec701_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_ec701_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_EC701, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank")
		, m_view(*this, "view")
	{ }

protected:
	// device_t implementation
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

	virtual std::error_condition initialize_cartridge(std::string &message) override;

private:
	void bank_w(u8 data);

	memory_bank_creator m_rombank;
	memory_view m_view;
};

void msx_cart_ec701_device::device_reset()
{
	m_view.select(0);
}

std::error_condition msx_cart_ec701_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_ec701_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	m_rombank->configure_entries(0, 24, cart_rom_region()->base() + 0x20000, 0x4000);

	page(1)->install_view(0x4000, 0x7fff, m_view);
	m_view[0].install_rom(0x4000, 0x7fff, cart_rom_region()->base());
	m_view[1].install_read_bank(0x4000, 0x7fff, m_rombank);
	m_view[2].nop_read(0x4000, 0x7fff);

	page(2)->install_rom(0x8000, 0xbfff, cart_rom_region()->base() + 0x4000);
	page(2)->install_write_handler(0xbff8, 0xbfff, emu::rw_delegate(*this, FUNC(msx_cart_ec701_device::bank_w)));

	return std::error_condition();
}

void msx_cart_ec701_device::bank_w(u8 data)
{
	data = ~data;
	switch (data & 0x38)
	{
	case 0x00:  // ic1
		m_view.select(0);
		break;
	case 0x20:  // ic2
		m_view.select(1);
		m_rombank->set_entry(data & 0x07);
		break;
	case 0x28:  // ic3
		m_view.select(1);
		m_rombank->set_entry(0x08 + (data & 0x07));
		break;
	case 0x30:  // ic4
		m_view.select(1);
		m_rombank->set_entry(0x10 + (data & 0x07));
		break;
	default:
		m_view.select(2);
		break;
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KONAMI,           msx_cart_interface, msx_cart_konami_device,                  "msx_cart_konami",           "MSX Cartridge - KONAMI")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KONAMI_SCC,       msx_cart_interface, msx_cart_konami_scc_device,              "msx_cart_konami_scc",       "MSX Cartridge - KONAMI+SCC")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SUNRISE_SCC,      msx_cart_interface, msx_cart_sunrise_scc_device,             "msx_cart_sunrise_scc",      "MSX Cartridge - Sunrise+SCC")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_GAMEMASTER2,      msx_cart_interface, msx_cart_gamemaster2_device,             "msx_cart_gamemaster2",      "MSX Cartridge - GAMEMASTER2")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SYNTHESIZER,      msx_cart_interface, msx_cart_synthesizer_device,             "msx_cart_synthesizer",      "MSX Cartridge - Synthesizer")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SOUND_SNATCHER,   msx_cart_interface, msx_cart_konami_sound_snatcher_device,   "msx_cart_sound_snatcher",   "MSX Cartridge - Sound Snatcher")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SOUND_SDSNATCHER, msx_cart_interface, msx_cart_konami_sound_sdsnatcher_device, "msx_cart_sound_sdsnatcher", "MSX Cartridge - Sound SD Snatcher")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_KEYBOARD_MASTER,  msx_cart_interface, msx_cart_keyboard_master_device,         "msx_cart_keyboard_master",  "MSX Cartridge - Keyboard Master")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_EC701,            msx_cart_interface, msx_cart_ec701_device,                   "msx_cart_ec701",            "MSX Cartridge - Konami EC-701")
