// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************

Yamaha SKW-01 emulation

Word processor side slot module with Kanij ROM, 2KB SRAM, and a
printer port.
The PCB is labeled YMX-YWP.

The software can be started from Basic by entering the command "CALL JWP"
or "_JWP".

**************************************************************************/

#include "emu.h"
#include "skw01.h"
#include "bus/centronics/ctronics.h"
#include "machine/buffer.h"
#include "machine/nvram.h"

namespace {

class msx_cart_skw01_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_skw01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr u16 SRAM_SIZE = 2048;

	required_device<nvram_device> m_nvram;
	required_memory_region m_region_rom;
	required_memory_region m_region_dict;
	required_memory_region m_region_kanji;
	u16 m_bank_address[5];
	std::unique_ptr<u8[]> m_sram;
	required_device<input_buffer_device> m_cent_status_in;
	required_device<output_latch_device> m_cent_ctrl_out;
	required_device<output_latch_device> m_cent_data_out;
	required_device<centronics_device> m_centronics;

	template<int Bank> void addr_lo_w(u8 data);
	template<int Bank> void addr_hi_w(u8 data);
	template<int Bank> u8 ready_r() const;
	template<int Bank> u8 data_r() const;
	void dictsram_w(u8 data);
	u8 dictsram_r() const;
};


msx_cart_skw01_device::msx_cart_skw01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_CART_SKW01, tag, owner, clock)
	, msx_cart_interface(mconfig, *this)
	, m_nvram(*this, "nvram")
	, m_region_rom(*this, "rom")
	, m_region_dict(*this, "dict")
	, m_region_kanji(*this, "kanji")
	, m_bank_address{0}
	, m_cent_status_in(*this, "cent_status_in")
	, m_cent_ctrl_out(*this, "cent_ctrl_out")
	, m_cent_data_out(*this, "cent_data_out")
	, m_centronics(*this, "centronics")
{
}

void msx_cart_skw01_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	// printer port
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit1));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);
	INPUT_BUFFER(config, m_cent_status_in);

	OUTPUT_LATCH(config, m_cent_ctrl_out);
	m_cent_ctrl_out->bit_handler<1>().set(m_centronics, FUNC(centronics_device::write_strobe));
}

ROM_START(msx_skw01)
	ROM_REGION(0x8000, "rom", 0)
	ROM_LOAD("skw-01.rom", 0x0000, 0x8000, CRC(9a7c9a26) SHA1(6a7f41d89f9d50f4ff648233670660cfb07a41ee))
	ROM_REGION(0x8000, "dict", 0)
	ROM_LOAD("skw-01p4.rom", 0x0000, 0x8000, CRC(41779e03) SHA1(82f2ecd9f3135a8602d54cc6322216b3b15ae8b3))
	ROM_REGION(0x20000, "kanji", 0)
	ROM_LOAD("skw-01kf.rom", 0x00000, 0x20000, CRC(9de1dc10) SHA1(f0901a63de6c95a42ce6ad82dd5e6bfba4bcf180))
ROM_END

const tiny_rom_entry *msx_cart_skw01_device::device_rom_region() const
{
	return ROM_NAME(msx_skw01);
}

void msx_cart_skw01_device::device_start()
{
	m_sram = std::make_unique<u8[]>(SRAM_SIZE);
	m_nvram->set_base(m_sram.get(), SRAM_SIZE);
	save_pointer(NAME(m_sram), SRAM_SIZE);
	save_item(NAME(m_bank_address));

	page(0)->install_rom(0x0000, 0x3fff, m_region_rom->base());
	page(1)->install_rom(0x4000, 0x7fff, m_region_rom->base() + 0x4000);
	page(1)->install_readwrite_handler(0x7fc0, 0x7fc0, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::ready_r<0>)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_lo_w<0>)));
	page(1)->install_readwrite_handler(0x7fc1, 0x7fc1, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::data_r<0>)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_hi_w<0>)));
	page(1)->install_readwrite_handler(0x7fc2, 0x7fc2, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::ready_r<1>)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_lo_w<1>)));
	page(1)->install_readwrite_handler(0x7fc3, 0x7fc3, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::data_r<1>)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_hi_w<1>)));
	page(1)->install_readwrite_handler(0x7fc4, 0x7fc4, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::ready_r<2>)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_lo_w<2>)));
	page(1)->install_readwrite_handler(0x7fc5, 0x7fc5, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::data_r<2>)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_hi_w<2>)));
	page(1)->install_readwrite_handler(0x7fc6, 0x7fc6, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::ready_r<3>)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_lo_w<3>)));
	page(1)->install_readwrite_handler(0x7fc7, 0x7fc7, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::data_r<3>)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_hi_w<3>)));
	page(1)->install_write_handler(0x7fc8, 0x7fc8, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_lo_w<4>)));
	page(1)->install_write_handler(0x7fc9, 0x7fc9, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::addr_hi_w<4>)));
	page(1)->install_readwrite_handler(0x7fca, 0x7fcb, emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::dictsram_r)), emu::rw_delegate(*this, FUNC(msx_cart_skw01_device::dictsram_w)));
	page(1)->install_readwrite_handler(0x7fcc, 0x7fcc, emu::rw_delegate(*m_cent_status_in, FUNC(input_buffer_device::read)), emu::rw_delegate(*m_cent_ctrl_out, FUNC(output_latch_device::write)));
	page(1)->install_write_handler(0x7fce, 0x7fce, emu::rw_delegate(*m_cent_data_out, FUNC(output_latch_device::write)));
}

template<int Bank>
void msx_cart_skw01_device::addr_lo_w(u8 data)
{
	m_bank_address[Bank] = (m_bank_address[Bank] & 0xff00) | data;
}

template<int Bank>
void msx_cart_skw01_device::addr_hi_w(u8 data)
{
	m_bank_address[Bank] = (m_bank_address[Bank] & 0x00ff) | (u16(data) << 8);
}

template<int Bank>
u8 msx_cart_skw01_device::ready_r() const
{
	// Unknown what the delay should be
	// 0 - Not ready
	// 1 - Data ready to read
	return 1;
}

template<int Bank>
u8 msx_cart_skw01_device::data_r() const
{
	return m_region_kanji->as_u8((Bank * 0x8000) | (m_bank_address[Bank] & 0x7fff));
}

void msx_cart_skw01_device::dictsram_w(u8 data)
{
	if (BIT(m_bank_address[4], 15))
	{
		m_sram[m_bank_address[4] & (SRAM_SIZE - 1)] = data;
	}
}

u8 msx_cart_skw01_device::dictsram_r() const
{
	if (BIT(m_bank_address[4], 15))
	{
		return m_sram[m_bank_address[4] & (SRAM_SIZE - 1)];
	}

	return m_region_dict->as_u8(m_bank_address[4]);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_SKW01, msx_cart_interface, msx_cart_skw01_device, "msx_cart_skw01", "Yamaha SKW-01")
