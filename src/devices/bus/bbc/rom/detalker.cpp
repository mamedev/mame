// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    D.E.Talker Speech Synthesizer

***************************************************************************/

#include "emu.h"
#include "detalker.h"

#include "sound/sp0256.h"

#include "speaker.h"


namespace {

class bbc_detalker_device : public device_t, public device_bbc_rom_interface
{
public:
	bbc_detalker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_DETALKER, tag, owner, clock)
		, device_bbc_rom_interface(mconfig, *this)
		, m_nsp(*this, "sp0256")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_bbc_rom_interface overrides
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	required_device<sp0256_device> m_nsp;
};


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(detalker)
	ROM_REGION(0x10000, "sp0256", 0)
	ROM_LOAD("sp0256a-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

const tiny_rom_entry *bbc_detalker_device::device_rom_region() const
{
	return ROM_NAME(detalker);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_detalker_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	SP0256(config, m_nsp, 3.2768_MHz_XTAL);
	m_nsp->add_route(ALL_OUTPUTS, "mono", 1.0); // routed to the BBC Micro speaker via a flying lead.
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t bbc_detalker_device::read(offs_t offset)
{
	uint8_t data = 0x00;;

	if (offset & 0x2000)
	{
		if (BIT(offset, 0))
			data = m_nsp->lrq_r() ? 0x00 : 0x40;
	}
	else
	{
		data = get_rom_base()[offset & 0x1fff];
	}

	return data;
}

void bbc_detalker_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x2000)
	{
		if (!BIT(offset, 0))
			m_nsp->ald_w(data & 0x3f);
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_DETALKER, device_bbc_rom_interface, bbc_detalker_device, "bbc_detalker", "D.E.Talker Speech Synthesizer")
