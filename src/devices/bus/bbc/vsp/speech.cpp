// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Speech System

**********************************************************************/

#include "emu.h"
#include "speech.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "machine/tms6100.h"
#include "sound/tms5220.h"


namespace {

class bbc_speech_device : public device_t, public device_bbc_vsp_interface
{
public:
	bbc_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_SPEECH, tag, owner, clock)
		, device_bbc_vsp_interface(mconfig, *this)
		, m_vsp(*this, "vsp")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void combined_rsq_wsq_w(uint8_t data) override { m_vsp->combined_rsq_wsq_w(data); }
	virtual uint8_t read() override { return m_vsp->status_r(); }
	virtual void write(uint8_t data) override { m_vsp->data_w(data); }

	virtual int readyq_r() override { return m_vsp->readyq_r(); }
	virtual int intq_r() override { return m_vsp->intq_r(); }

private:
	required_device<tms5220_device> m_vsp;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(load_phrom);
};


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( speech )
	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
ROM_END

const tiny_rom_entry *bbc_speech_device::device_rom_region() const
{
	return ROM_NAME( speech );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_speech_device::device_add_mconfig(machine_config &config)
{
	TMS5220(config, m_vsp, 640000);
	m_vsp->add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.5);

	TMS6100(config, "vsm", 0);
	m_vsp->m0_cb().set("vsm", FUNC(tms6100_device::m0_w));
	m_vsp->m1_cb().set("vsm", FUNC(tms6100_device::m1_w));
	m_vsp->addr_cb().set("vsm", FUNC(tms6100_device::add_w));
	m_vsp->data_cb().set("vsm", FUNC(tms6100_device::data_line_r));
	m_vsp->romclk_cb().set("vsm", FUNC(tms6100_device::clk_w));

	GENERIC_SOCKET(config, "phrom", generic_plain_slot, "bbc_vsm", "bin,rom").set_device_load(FUNC(bbc_speech_device::load_phrom));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_speech_device::device_start()
{
	// copy vsm region from machine to TMS6100 device region
	memcpy(memregion("vsm")->base(), m_slot->vsm().base(), 0x40000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

DEVICE_IMAGE_LOAD_MEMBER(bbc_speech_device::load_phrom)
{
	offs_t size;
	if (!image.loaded_through_softlist())
		size = image.length();
	else
		size = image.get_software_region_length("rom");

	if (size < 0x1000 || size > 0x4000 || (size & (size - 1)) != 0)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid PHROM size");

	uint8_t *vsm = memregion("vsm")->base();
	if (image.loaded_through_softlist())
	{
		int bank = strtol(image.get_feature("bank"), nullptr, 0);
		if (bank < 0 || bank > 15)
			bank = 0;

		memcpy(vsm + (bank * 0x4000), image.get_software_region("rom"), size);
	}
	else
	{
		image.fread(vsm, size);
	}

	return std::make_pair(std::error_condition(), std::string());
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_SPEECH, device_bbc_vsp_interface, bbc_speech_device, "bbc_speech", "Acorn Speech System")
