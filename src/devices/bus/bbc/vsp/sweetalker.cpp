// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cheetah Sweet Talker

**********************************************************************/

#include "emu.h"
#include "sweetalker.h"

#include "sound/sp0256.h"


namespace {

class bbc_sweetalker_device : public device_t, public device_bbc_vsp_interface
{
public:
	bbc_sweetalker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_SWEETALKER, tag, owner, clock)
		, device_bbc_vsp_interface(mconfig, *this)
		, m_nsp(*this, "sp0256")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void write(uint8_t data) override { m_nsp->ald_w(data & 0x3f); }

	virtual int intq_r() override { return m_nsp->sby_r(); }

private:
	required_device<sp0256_device> m_nsp;
};


///-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sweetalker )
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

const tiny_rom_entry *bbc_sweetalker_device::device_rom_region() const
{
	return ROM_NAME( sweetalker );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_sweetalker_device::device_add_mconfig(machine_config &config)
{
	SP0256(config, m_nsp, 3.2768_MHz_XTAL);
	m_nsp->add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.00);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(BBC_SWEETALKER, device_bbc_vsp_interface, bbc_sweetalker_device, "bbc_sweetalker", "Cheetah Sweet Talker (BBC)")
