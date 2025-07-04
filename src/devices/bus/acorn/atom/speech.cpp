// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Speech Synthesis Module

    http://acornatom.nl/atom_handleidingen/misc/speechsynt.html

**********************************************************************/

#include "emu.h"
#include "speech.h"

#include "sound/sp0256.h"
#include "speaker.h"


namespace {

class atom_speech_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_SPEECH, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_nsp(*this, "sp0256")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t pb_r() override
	{
		return m_nsp->sby_r() << 7;
	}

	virtual void pb_w(uint8_t data) override
	{
		// pitch
		m_nsp->set_clock(BIT(data, 6) ? 3050000 : 3200000); // TODO: verify frequencies

		// allophone
		m_nsp->ald_w(data & 0x3f);
	}

private:
	required_device<sp0256_device> m_nsp;
};


//-------------------------------------------------
//  ROM( speech )
//-------------------------------------------------

ROM_START(speech)
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *atom_speech_device::device_rom_region() const
{
	return ROM_NAME(speech);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_speech_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_nsp, 3200000);
	m_nsp->add_route(ALL_OUTPUTS, "mono", 1.0);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_SPEECH, device_acorn_bus_interface, atom_speech_device, "atom_speech", "Atom Speech Synthesis Module")

