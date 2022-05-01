// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 ROM Module

****************************************************************************/

#include "emu.h"
#include "rom.h"

class rom_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
private:
	required_memory_region m_rom;
	required_ioport m_rom_selector;
};

rom_device::rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_ROM, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_rom_selector(*this, "A13-A15")
{
}

void rom_device::device_start()
{
}

void rom_device::device_reset()
{
	m_bus->installer(AS_PROGRAM)->install_rom(0x0000, 0x1fff, 0x0000, m_rom->base() + (m_rom_selector->read() & 7) * 0x2000);
}

static INPUT_PORTS_START( rom_selector )
	PORT_START("A13-A15")   /* jumpers to select ROM region */
	PORT_CONFNAME( 0x7, 0x0, "ROM Bank" )
	PORT_CONFSETTING( 0x0, "BASIC" )
	PORT_CONFSETTING( 0x1, "EMPTY1" )
	PORT_CONFSETTING( 0x2, "EMPTY2" )
	PORT_CONFSETTING( 0x3, "EMPTY3" )
	PORT_CONFSETTING( 0x4, "EMPTY4" )
	PORT_CONFSETTING( 0x5, "EMPTY5" )
	PORT_CONFSETTING( 0x6, "EMPTY6" )
	PORT_CONFSETTING( 0x7, "SCM" )
INPUT_PORTS_END

ioport_constructor rom_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rom_selector );
}

ROM_START(rc2014_rom)
	ROM_REGION( 0x10000, "rom",0 )
	ROM_LOAD( "r0000009.bin",    0x0000, 0x10000, CRC(3fb1ced7) SHA1(40a030b931ebe6cca654ce056c228297f245b057))
ROM_END

const tiny_rom_entry *rom_device::device_rom_region() const
{
	return ROM_NAME( rc2014_rom );
}

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_ROM, device_rc2014_card_interface, rom_device, "rc2014_rom", "RC2014 ROM Module")
