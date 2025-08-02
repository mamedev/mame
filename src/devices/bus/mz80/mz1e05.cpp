// license:BSD-3-Clause
// copyright-holders:AJR
/**************************************************************************************************

Sharp MZ-1E05 Floppy Disk Drive Interface

For MZ-700, the boot EPROM (not currently supported) is populated and mapped at F000-FFFF.
For MZ-800, the EPROM socket must remain empty.

The density control register is not documented.

**************************************************************************************************/

#include "emu.h"
#include "mz1e05.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "softlist_dev.h"

namespace {

class mz1e05_device : public device_t, public device_mz80_exp_interface
{
public:
	// device type constructor
	mz1e05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_mz80_exp_interface implementation
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	void dm_w(u8 data);
	void hs_w(u8 data);
	void fm_w(u8 data);

	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 4> m_fdd;

	u8 m_dm;
	u8 m_hs;
};

mz1e05_device::mz1e05_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MZ1E05, tag, owner, clock)
	, device_mz80_exp_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_fdd(*this, "fdc:%u", 0U)
	, m_dm(0)
	, m_hs(0)
{
}

static void mz1e05_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("525qd", FLOPPY_525_QD);
}

void mz1e05_device::device_add_mconfig(machine_config &config)
{
	MB8876(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->set_force_ready(false); // FIXME: READY signal synthesized from index pulse by 74LS122 monostable
	// IRQ, DRQ not connected

	FLOPPY_CONNECTOR(config, m_fdd[0], mz1e05_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[1], mz1e05_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[2], mz1e05_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_fdd[3], mz1e05_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "floppy_list").set_original("mz800_flop");
}

void mz1e05_device::io_map(address_map &map)
{
	map(0xd8, 0xdb).mirror(0xff00).rw(m_fdc, FUNC(mb8876_device::read), FUNC(mb8876_device::write));
	map(0xdc, 0xdc).mirror(0xff00).w(FUNC(mz1e05_device::dm_w));
	map(0xdd, 0xdd).mirror(0xff00).w(FUNC(mz1e05_device::hs_w));
	map(0xde, 0xde).mirror(0xff00).w(FUNC(mz1e05_device::fm_w));
}

void mz1e05_device::device_start()
{
	save_item(NAME(m_dm));
	save_item(NAME(m_hs));
}

void mz1e05_device::device_reset()
{
	dm_w(0);
	hs_w(0);
	fm_w(0);
}

void mz1e05_device::dm_w(u8 data)
{
	floppy_image_device *floppy = BIT(data, 2) ? m_fdd[data & 0x03]->get_device() : nullptr;
	m_fdc->set_floppy(floppy);
	if (floppy)
		floppy->ss_w(!BIT(m_hs, 0));

	// Motor control appears to be independent of drive select
	for (auto &fdd : m_fdd)
	{
		if (fdd->get_device())
			fdd->get_device()->mon_w(!BIT(data, 7));
	}

	m_dm = data;
}

void mz1e05_device::hs_w(u8 data)
{
	floppy_image_device *floppy = BIT(data, 2) ? m_fdd[data & 0x03]->get_device() : nullptr;
	if (floppy)
		floppy->ss_w(!BIT(data, 0));

	m_hs = data;
}

void mz1e05_device::fm_w(u8 data)
{
	m_fdc->dden_w(BIT(data, 0));
}

} // anonymous namespace

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(MZ1E05, device_mz80_exp_interface, mz1e05_device, "mz1e05", "MZ-1E05 FDD Interface")
