// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "hbi55.h"

#include "imagedev/memcard.h"
#include "machine/i8255.h"

#include <memory>
#include <string>
#include <system_error>
#include <utility>


namespace {

/*
Emulation of Sony HBI-55 and Yamaha UDC-01 data cartridges.
Internally these two data cartridges are the same.

In theory these battery-backed RAM cartridges could use up to 8 x 2KB SRAM
chips but only cartridges using 2 2KB SRAM chips were produced.
*/

class msx_cart_hbi55_device : public device_t, public msx_cart_interface, public device_memcard_image_interface
{
public:
	msx_cart_hbi55_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MSX_CART_HBI55, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, device_memcard_image_interface(mconfig, *this)
		, m_i8255(*this, "i8255")
		, m_address(0)
		, m_ce(false)
		, m_oe(false)
		, m_last_c(0)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual char const *file_extensions() const noexcept override { return "bin"; }
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

private:
	static constexpr u32 SRAM_SIZE = 0x1000;

	void ppi_port_a_w(u8 data);
	void ppi_port_b_w(u8 data);
	void ppi_port_c_w(u8 data);
	u8 ppi_port_c_r();
	void sram_update();
	void clear_sram();

	required_device<i8255_device> m_i8255;
	std::unique_ptr<u8[]> m_sram;
	u16 m_address;
	bool m_ce;
	bool m_oe;
	u8 m_last_c;
};

std::pair<std::error_condition, std::string> msx_cart_hbi55_device::call_load()
{
	if (length() != SRAM_SIZE)
		return std::make_pair(image_error::INVALIDLENGTH, std::string());

	if (fread(m_sram.get(), SRAM_SIZE) != SRAM_SIZE)
		return std::make_pair(std::errc::io_error, std::string());

	return std::make_pair(std::error_condition(), std::string());
}

void msx_cart_hbi55_device::call_unload()
{
	if (!is_readonly())
	{
		fseek(0, SEEK_SET);
		fwrite(m_sram.get(), SRAM_SIZE);
	}
	clear_sram();
}

std::pair<std::error_condition, std::string> msx_cart_hbi55_device::call_create(int format_type, util::option_resolution *format_options)
{
	clear_sram();

	if (fwrite(m_sram.get(), SRAM_SIZE) != SRAM_SIZE)
		return std::make_pair(std::errc::io_error, std::string());

	return std::make_pair(std::error_condition(), std::string());
}

void msx_cart_hbi55_device::device_add_mconfig(machine_config &config)
{
	I8255(config, m_i8255);
	m_i8255->out_pa_callback().set(FUNC(msx_cart_hbi55_device::ppi_port_a_w));
	m_i8255->out_pb_callback().set(FUNC(msx_cart_hbi55_device::ppi_port_b_w));
	m_i8255->out_pc_callback().set(FUNC(msx_cart_hbi55_device::ppi_port_c_w));
	m_i8255->in_pc_callback().set(FUNC(msx_cart_hbi55_device::ppi_port_c_r));
}

void msx_cart_hbi55_device::device_start()
{
	m_sram = std::make_unique<u8[]>(SRAM_SIZE);
	clear_sram();

	save_pointer(NAME(m_sram), SRAM_SIZE);
	save_item(NAME(m_address));
	save_item(NAME(m_ce));
	save_item(NAME(m_oe));
	save_item(NAME(m_last_c));

	io_space().install_write_handler(0xb0, 0xb3, write8sm_delegate(*m_i8255, FUNC(i8255_device::write)));
	io_space().install_read_handler(0xb0, 0xb3, read8sm_delegate(*m_i8255, FUNC(i8255_device::read)));
}

void msx_cart_hbi55_device::device_reset()
{
	m_address = 0;
	m_ce = false;
	m_oe = false;
	m_last_c = 0;
}

void msx_cart_hbi55_device::clear_sram()
{
	std::fill_n(m_sram.get(), SRAM_SIZE, 0);
}

void msx_cart_hbi55_device::ppi_port_a_w(u8 data)
{
	// address bits 0-7
	m_address = (m_address & 0xff00) | data;
	sram_update();
}

void msx_cart_hbi55_device::ppi_port_b_w(u8 data)
{
	// 76543210
	// |||||\\\-- address bits 8-10
	// ||\\\----- to SRAM chip select TC40H138 demux (only Y0 and Y1 are used)
	// |\-------- SRAM CE (1 = enable)
	// \--------- SRAM OE (1 = output enable, 0 = write enable)
	m_address = (m_address & 0x00ff) | ((data & 0x3f) << 8);
	m_ce = BIT(data, 6);
	m_oe = BIT(data, 7);
	sram_update();
}

void msx_cart_hbi55_device::ppi_port_c_w(u8 data)
{
	m_last_c = data;
	sram_update();
}

u8 msx_cart_hbi55_device::ppi_port_c_r()
{
	if (m_ce && m_oe && m_address < SRAM_SIZE)
	{
		return m_sram[m_address];
	}
	// should actually be floating bus
	return 0xff;
}

void msx_cart_hbi55_device::sram_update()
{
	if (m_ce && !m_oe && m_address < SRAM_SIZE)
	{
		m_sram[m_address] = m_last_c;
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_HBI55, msx_cart_interface, msx_cart_hbi55_device, "msx_cart_hbi55", "Sony HBI-55/Yamaha UDC-01 Data Cartridge (4KB SRAM)")

