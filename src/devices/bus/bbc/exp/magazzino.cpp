// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Magazzino Parametrico from EVM Computer

    This is an unknown unit that connects to the Compact expansion port. The
    contents of the unit are encased in epoxy so exact contents are unknown.
    The unit maps a ROM into bank 1, called Magazzino, so is clearly for the
    Prodest PC 128S Italian market. Seems to require an accompanying floppy
    to be usable, but is MIA.

**********************************************************************/

#include "emu.h"
#include "magazzino.h"


namespace {

class bbc_magazzino_device : public device_t, public device_bbc_exp_interface
{
public:
	bbc_magazzino_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_MAGAZZINO, tag, owner, clock)
		, device_bbc_exp_interface(mconfig, *this)
		, m_rom(*this, "rom")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t rom_r(offs_t offset) override
	{
		return m_rom[offset];
	}

private:
	required_region_ptr<uint8_t> m_rom;
};


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(magazzino)
	ROM_REGION(0x8000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("magazzino-2.01.rom", 0x4000, 0x4000, CRC(89f7ee4c) SHA1(0d1959ff6456453842134be9b1a3dd2ee72ae884))
ROM_END

const tiny_rom_entry *bbc_magazzino_device::device_rom_region() const
{
	return ROM_NAME(magazzino);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_MAGAZZINO, device_bbc_exp_interface, bbc_magazzino_device, "bbc_magazzino", "Magazzino Parametrico")
