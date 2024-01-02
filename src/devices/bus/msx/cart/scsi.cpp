// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "scsi.h"

#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"
#include "machine/wd33c9x.h"

/*

Implementation of SCSI interface as it appeared in:
- MSX Computer Club Gouda MSX-SCSI Interface


Other known SCSI(-ish) interfaces:

ASCII HD Interface (SASI Interface)
B.E.R.T.
ESE MegaSCSI
- Came with SRAM on the cartridge (128/256/512/1024KB SRAM)
HSH SCSI Interface
MAK/Green/Sparrowsoft SCSI Interface
MK SCSI Interface
- Evolved into B.E.R.T. SCSI Interface.

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
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<wd33c93a_device> m_wd33c93a;

	void wd33c93a(device_t *device);
	void reset_w(u8 data);
};

void msx_cart_gouda_scsi_device::wd33c93a(device_t *device)
{
	device->set_clock(10'000'000);
}

void msx_cart_gouda_scsi_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi", 0);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6").option_set("wd33c93a", WD33C93A)
			.machine_config([this] (device_t *device) { wd33c93a(device); });

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

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_GOUDA_SCSI, msx_cart_interface, msx_cart_gouda_scsi_device, "msx_cart_gouda_scsi", "MSX Computer Club Gouda MSX-SCSI Interface")
