// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Lundy Coleco ADAM and ColecoVision Speech Synthesizer

***************************************************************************/

#include "emu.h"
#include "lundy_speech.h"

#include "sound/sp0256.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class lundy_speech_device : public device_t, public device_coleco_expansion_interface
{
public:
	lundy_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;

	void cmd_w(uint8_t data);
	uint8_t status_r();
	void reset_w(uint8_t data);

	required_device<sp0256_device> m_sp0256;
};

lundy_speech_device::lundy_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COLECO_LUNDY_SPEECH, tag, owner, clock),
	device_coleco_expansion_interface(mconfig, *this),
	m_sp0256(*this, "sp0256")
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void lundy_speech_device::io_map(address_map &map)
{
	map(0x43, 0x43).w(FUNC(lundy_speech_device::cmd_w));
	map(0x44, 0x44).r(FUNC(lundy_speech_device::status_r));
	map(0x45, 0x45).w(FUNC(lundy_speech_device::reset_w));
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void lundy_speech_device::device_add_mconfig(machine_config &config)
{
	SP0256(config, m_sp0256, 3.12_MHz_XTAL);
	m_sp0256->add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void lundy_speech_device::cmd_w(uint8_t data)
{
	m_sp0256->ald_w(data);
}

uint8_t lundy_speech_device::status_r()
{
	return m_sp0256->lrq_r() ? 0 : 1;
}

void lundy_speech_device::reset_w(uint8_t data)
{
	m_sp0256->reset();
}

void lundy_speech_device::device_start()
{
	// install device into memory map
	m_expansion->io_space().install_device(0x00, 0x7f, *this, &lundy_speech_device::io_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( sp0256 )
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

const tiny_rom_entry *lundy_speech_device::device_rom_region() const
{
	return ROM_NAME( sp0256 );
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COLECO_LUNDY_SPEECH, device_coleco_expansion_interface, lundy_speech_device, "coleco_lundy_speech", "Lundy Coleco ADAM and ColecoVision Speech Synthesizer")
