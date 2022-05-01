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
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
private:
	required_memory_region m_rom;
};

rom_device::rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_ROM, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_rom(*this, "rom")
{
}

void rom_device::device_start()
{
	m_bus->installer(AS_PROGRAM)->install_rom(0x0000, 0x1fff, 0x0000, m_rom->base()+ 0xe000);
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
