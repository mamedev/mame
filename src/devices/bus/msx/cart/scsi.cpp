// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "scsi.h"

#include "bus/nscsi/devices.h"
#include "machine/mb87030.h"
#include "machine/nscsi_bus.h"
#include "machine/wd33c9x.h"

/*

Implementation of SCSI interface as it appeared in:
- MSX Computer Club Gouda MSX-SCSI Interface
- ESE MegaSCSI
  - Came with SRAM on the cartridge (128/256/512/1024KB SRAM)


Other known SCSI(-ish) interfaces:

ASCII HD Interface (SASI Interface)
B.E.R.T.
HSH SCSI Interface
MAK/Green/Sparrowsoft SCSI Interface
MK SCSI Interface
- Evolved into B.E.R.T. SCSI Interface.

TODO:
- Fix MegaSCSI mb89352 write operation at 8MHz.

*/

namespace
{

class msx_cart_gouda_scsi_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_gouda_scsi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_GOUDA_SCSI, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_wd33c93a(*this, "scsi:6:wd33c93a")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<wd33c93a_device> m_wd33c93a;

	void reset_w(u8 data);
};

void msx_cart_gouda_scsi_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi", 0);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6").option_set("wd33c93a", WD33C93A).clock(10_MHz_XTAL);
}

std::error_condition msx_cart_gouda_scsi_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_gouda_scsi_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() < 0x4000)
	{
		message = "msx_cart_gouda_scsi_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	page(1)->install_rom(0x4000, 0x7fff, cart_rom_region()->base());

	io_space().install_write_handler(0x34, 0x35, emu::rw_delegate(*m_wd33c93a, FUNC(wd33c93a_device::indir_w)));
	io_space().install_read_handler(0x34, 0x35, emu::rw_delegate(*m_wd33c93a, FUNC(wd33c93a_device::indir_r)));
	io_space().install_write_handler(0x36, 0x36, emu::rw_delegate(*this, FUNC(msx_cart_gouda_scsi_device::reset_w)));

	return std::error_condition();
}

void msx_cart_gouda_scsi_device::reset_w(u8 data)
{
	m_wd33c93a->reset_w(data);
}



class msx_cart_mega_scsi_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_mega_scsi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_MEGA_SCSI, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_mb89352a(*this, "scsi:7:mb89352a")
		, m_srambank(*this, "srambank%u", 0U)
		, m_view{ {*this, "view0"}, {*this, "view1"}, {*this, "view2"}, {*this, "view3"} }
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

	static constexpr feature_type imperfect_features() { return feature::TIMING; }

protected:
	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr size_t BANK_SIZE = 0x2000;

	required_device<mb89352_device> m_mb89352a;
	memory_bank_array_creator<4> m_srambank;
	memory_view m_view[4];
	u8 m_bank_mask;

	template <int Bank> void bank_w(u8 data);
};

void msx_cart_mega_scsi_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi", 0);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	// Input clock is 8MHz according to the schematic. However, clocks below 11MHz cause writes to fail. Reads are fine at 8MHz.
	//NSCSI_CONNECTOR(config, "scsi:7").option_set("mb89352a", MB89352).clock(16_MHz_XTAL/2);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("mb89352a", MB89352).clock(11'000'000);
}

void msx_cart_mega_scsi_device::device_reset()
{
	for (int i = 0; i < 4; i++)
	{
		if (i != 1)
			m_view[i].select(0);
		m_srambank[i]->set_entry(0);
	}
}

std::error_condition msx_cart_mega_scsi_device::initialize_cartridge(std::string &message)
{
	if (!cart_sram_region())
	{
		message = "msx_cart_mega_scsi_device: Required region 'sram' was not found.";
		return image_error::INTERNAL;
	}

	const u32 sram_size = cart_sram_region()->bytes();

	if (sram_size != 0x20000 && sram_size != 0x40000 && sram_size != 0x80000 && sram_size != 0x100000)
	{
		message = "msx_cart_mega_scsi_device: Region 'sram' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	const u16 sram_banks = sram_size / BANK_SIZE;
	m_bank_mask = sram_banks - 1;

	for (int i = 0; i < 4; i++)
		m_srambank[i]->configure_entries(0, sram_banks, cart_sram_region()->base(), BANK_SIZE);

	page(1)->install_view(0x4000, 0x5fff, m_view[0]);
	m_view[0][0].install_read_bank(0x4000, 0x5fff, m_srambank[0]);
	m_view[0][1].install_readwrite_bank(0x4000, 0x5fff, m_srambank[0]);
	m_view[0][2].install_read_handler(0x4000, 0x4fff, emu::rw_delegate(*m_mb89352a, FUNC(mb89352_device::dma_r)));
	m_view[0][2].install_write_handler(0x4000, 0x4fff, emu::rw_delegate(*m_mb89352a, FUNC(mb89352_device::dma_w)));
	// mb89352 is mirrored at 5000 - 5fff
	for (int addr = 0x5000; addr < 0x6000; addr += 0x10)
		m_view[0][2].install_device(addr, addr + 0x0f, *m_mb89352a, &mb89352_device::map);
	page(1)->install_read_bank(0x6000, 0x7fff, m_srambank[1]);
	page(2)->install_view(0x8000, 0x9fff, m_view[2]);
	m_view[2][0].install_read_bank(0x8000, 0x9fff, m_srambank[2]);
	m_view[2][1].install_readwrite_bank(0x8000, 0x9fff, m_srambank[2]);
	m_view[2][2].install_read_handler(0x8000, 0x8fff, emu::rw_delegate(*m_mb89352a, FUNC(mb89352_device::dma_r)));
	m_view[2][2].install_write_handler(0x8000, 0x8fff, emu::rw_delegate(*m_mb89352a, FUNC(mb89352_device::dma_w)));
	// mb89352 is mirrored at 9000 - 9fff
	for (int addr = 0x9000; addr < 0xa000; addr += 0x10)
		m_view[2][2].install_device(addr, addr + 0x0f, *m_mb89352a, &mb89352_device::map);
	page(2)->install_view(0xa000, 0xbfff, m_view[3]);
	m_view[3][0].install_read_bank(0xa000, 0xbfff, m_srambank[3]);
	m_view[3][1].install_readwrite_bank(0xa000, 0xbfff, m_srambank[3]);
	m_view[3][2].install_read_handler(0xa000, 0xafff, emu::rw_delegate(*m_mb89352a, FUNC(mb89352_device::dma_r)));
	m_view[3][2].install_write_handler(0xa000, 0xafff, emu::rw_delegate(*m_mb89352a, FUNC(mb89352_device::dma_w)));
	// mb89352 is mirrored at b000 - bfff
	for (int addr = 0xb000; addr < 0xc000; addr += 0x10)
		m_view[3][2].install_device(addr, addr + 0x0f, *m_mb89352a, &mb89352_device::map);

	page(1)->install_write_handler(0x6000, 0x67ff, emu::rw_delegate(*this, FUNC(msx_cart_mega_scsi_device::bank_w<0>)));
	page(1)->install_write_handler(0x6800, 0x6fff, emu::rw_delegate(*this, FUNC(msx_cart_mega_scsi_device::bank_w<1>)));
	page(1)->install_write_handler(0x7000, 0x77ff, emu::rw_delegate(*this, FUNC(msx_cart_mega_scsi_device::bank_w<2>)));
	page(1)->install_write_handler(0x7800, 0x7fff, emu::rw_delegate(*this, FUNC(msx_cart_mega_scsi_device::bank_w<3>)));

	return std::error_condition();
}

template <int Bank>
void msx_cart_mega_scsi_device::bank_w(u8 data)
{
	// Also writes of $00 and $10 to 7ffe are seen before banking; some kind of banking enable?
	m_srambank[Bank]->set_entry(data & m_bank_mask);
	if (Bank != 1)
	{
		int view_to_select = BIT(data, 7) ? 1 : (BIT(data, 6) ? 2 : 0);

		m_view[Bank].select(view_to_select);
	}
}


} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_GOUDA_SCSI, msx_cart_interface, msx_cart_gouda_scsi_device, "msx_cart_gouda_scsi", "MSX Computer Club Gouda MSX-SCSI Interface")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MEGA_SCSI, msx_cart_interface, msx_cart_mega_scsi_device, "msx_cart_mega_scsi", "ESE MEGA-SCSI")
