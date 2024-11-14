// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "quickdisk.h"
#include "machine/z80sio.h"
/**********************************************************************************

Emulation of MSX QuickDisk interfaces.

All known MSX QuickDisk interfaces contain 8KB ROM, 2KB RAM, and are connected
to a Mitsumi QuickDisk drive.

TODO:
- Implement a quick disk drive and connect it to the SIO.
- Dumps of MSX QuickDisks.

**********************************************************************************/

//#define VERBOSE  (1)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

namespace
{

class msx_cart_quickdisk_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_quickdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_QUICKDISK, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_sio(*this, "sio")
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual void device_start() override { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<z80sio_device> m_sio;

	void rtsa_w(int state);
	void rtsb_w(int state);
	void dtra_w(int state);
	void dtrb_w(int state);
};


void msx_cart_quickdisk_device::device_add_mconfig(machine_config &config)
{
	Z80SIO(config, m_sio, 6.5_MHz_XTAL);
	m_sio->out_int_callback().set(*this, FUNC(msx_cart_quickdisk_device::irq_out));
	m_sio->out_rtsa_callback().set(*this, FUNC(msx_cart_quickdisk_device::rtsa_w)); // RTSA - write enable
	m_sio->out_rtsb_callback().set(*this, FUNC(msx_cart_quickdisk_device::rtsb_w)); // RTSB - mfm demodulator?
	m_sio->out_dtra_callback().set(*this, FUNC(msx_cart_quickdisk_device::dtra_w)); // DTRA - not used
	m_sio->out_dtrb_callback().set(*this, FUNC(msx_cart_quickdisk_device::dtrb_w)); // DTRB - motor on
	// CTSA - media write protected
	// CTSB - not used
	// DCDA - media inserted
	// DCDB - ready
}


std::error_condition msx_cart_quickdisk_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_quickdisk_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x2000)
	{
		message = "msx_cart_quickdisk_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	if (!cart_ram_region())
	{
		message = "msx_cart_quickdisk_device: Required region 'ram' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_ram_region()->bytes() != 0x800)
	{
		message = "msx_cart_quickdisk_device: Region 'ram' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	page(1)->install_rom(0x4000, 0x5fff, cart_rom_region()->base());
	page(1)->install_read_handler(0x6000, 0x6003, emu::rw_delegate(m_sio, FUNC(z80sio_device::cd_ba_r)));
	page(1)->install_write_handler(0x6000, 0x6003, emu::rw_delegate(m_sio, FUNC(z80sio_device::cd_ba_w)));
	page(1)->install_ram(0x7000, 0x77ff, cart_ram_region()->base());

	return std::error_condition();
}

void msx_cart_quickdisk_device::rtsa_w(int state)
{
	LOG("RTSA / write enable: %d\n", state);
}

void msx_cart_quickdisk_device::rtsb_w(int state)
{
	LOG("RTSB / mfm demodulator: %d\n", state);
}

void msx_cart_quickdisk_device::dtra_w(int state)
{
	LOG("DTRA / not used: %d\n", state);
}

void msx_cart_quickdisk_device::dtrb_w(int state)
{
	LOG("DTRB / motor on: %d\n", state);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_QUICKDISK, msx_cart_interface, msx_cart_quickdisk_device, "msx_cart_quickdisk", "Quickdisk Interface")
