// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Pencil 2 Coleco Cartridge Adaptor

*******************************************************************************/

#include "emu.h"
#include "coleco.h"


namespace {

class pencil2_coleco_device : public device_t, public device_pencil2_memexp_interface
{
public:
	pencil2_coleco_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, PENCIL2_COLECO, tag, owner, clock)
		, device_pencil2_memexp_interface(mconfig, *this)
		, m_bios(*this, "bios")
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD { }
	virtual void device_reset() override ATTR_COLD { m_slot->romdis_w(1); }

	virtual u8 m0_r(offs_t offset) override { return m_bios[offset]; }

private:
	required_region_ptr<u8> m_bios;
};


//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(coleco)
	ROM_REGION(0x2000, "bios", 0)
	ROM_LOAD( "621.bin", 0x0000, 0x2000, CRC(fac06ff4) SHA1(ca889d41ea41a781edde7eb004ab530043fbc1a7) )
ROM_END

const tiny_rom_entry *pencil2_coleco_device::device_rom_region() const
{
	return ROM_NAME( coleco );
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(PENCIL2_COLECO, device_pencil2_memexp_interface, pencil2_coleco_device, "pencil2_coleco", "Pencil 2 Coleco Cartridge Adaptor")
