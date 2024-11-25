// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Multitech TVA-MPF-IP

    TODO:
    - no known documentation so emulation not verified.

***************************************************************************/

#include "emu.h"
#include "tva.h"
#include "screen.h"
#include "video/ef9365.h"


namespace {

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( mpf_tva )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("tva.bin", 0x0000, 0x1000, CRC(a9d4f261) SHA1(3a5b0dba5d1e2edda8563b55010c86efd1548d0b))
ROM_END


//-------------------------------------------------
//  mpf_tva_ip_device - constructor
//-------------------------------------------------

class mpf_tva_ip_device : public device_t, public device_mpf1_exp_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	mpf_tva_ip_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, MPF_TVA_IP, tag, owner, clock)
		, device_mpf1_exp_interface(mconfig, *this)
		, m_rom(*this, "rom")
		, m_gdp(*this, "ef9367")
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
		screen.set_size(512, 312);
		screen.set_visarea(0, 512 - 1, 0, 256 - 1);
		screen.set_refresh_hz(50);
		screen.set_screen_update("ef9367", FUNC(ef9365_device::screen_update));
		PALETTE(config, "palette", palette_device::MONOCHROME);

		EF9365(config, m_gdp, 1750000); // EF9367
		m_gdp->set_screen("screen");
		m_gdp->set_palette_tag("palette");
		m_gdp->set_nb_bitplanes(1);
		m_gdp->set_display_mode(ef9365_device::DISPLAY_MODE_512x256);
		m_gdp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(mpf1_exp_device::int_w));
	}

	virtual const tiny_rom_entry *device_rom_region() const override
	{
		return ROM_NAME( mpf_tva );
	}

	virtual void device_start() override { }
	virtual void device_reset() override
	{
		program_space().install_rom(0xa000, 0xafff, m_rom->base());
		program_space().install_readwrite_handler(0xafe0, 0xafef, emu::rw_delegate(*m_gdp, FUNC(ef9365_device::data_r)), emu::rw_delegate(*m_gdp, FUNC(ef9365_device::data_w)));
	}

private:
	required_memory_region m_rom;
	required_device<ef9365_device> m_gdp;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(MPF_TVA_IP, device_mpf1_exp_interface, mpf_tva_ip_device, "mpf1_tva_ip", "Multitech TVA-MPF-IP (Video Board)")
