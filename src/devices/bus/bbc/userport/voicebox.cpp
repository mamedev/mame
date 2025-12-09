// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Robin Voice Box - The Educational Software Company

**********************************************************************/

#include "emu.h"
#include "voicebox.h"

#include "sound/sp0256.h"

#include "speaker.h"


namespace {

class bbc_voicebox_device : public device_t, public device_bbc_userport_interface
{
public:
	bbc_voicebox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_VOICEBOX, tag, owner, clock)
		, device_bbc_userport_interface(mconfig, *this)
		, m_nsp(*this, "sp0256")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void pb_w(uint8_t data) override
	{
		m_nsp->ald_w(data & 0x3f);
	}

private:
	required_device<sp0256_device> m_nsp;
};


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(voicebox)
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

const tiny_rom_entry *bbc_voicebox_device::device_rom_region() const
{
	return ROM_NAME(voicebox);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_voicebox_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_nsp, 3120000); // TODO: unknown crystal
	m_nsp->data_request_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_userport_slot_device::cb1_w)).invert();
	m_nsp->add_route(ALL_OUTPUTS, "mono", 1.0);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_VOICEBOX, device_bbc_userport_interface, bbc_voicebox_device, "bbc_voicebox", "Robin Voice Box")
