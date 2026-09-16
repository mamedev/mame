// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    PolyMorphic Systems 16K RAM Card

    This is a mostly straightforward S-100 dynamic RAM board using
    the Intel 3242 controller to generate refresh timings. The one
    slightly unexpected IC on this board is a SN74LS283 adder, which
    allows the RAM to be addressed in a contiguous block beginning at
    any 4K boundary.

**********************************************************************/

#include "emu.h"
#include "poly16k.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> poly_16k_ram_device

class poly_16k_ram_device : public device_t, public device_s100_card_interface
{
public:
	// construction/destruction
	poly_16k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-specific overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// S-100 memory access handlers
	virtual u8 s100_smemr_r(offs_t offset) override;
	virtual void s100_mwrt_w(offs_t offset, u8 data) override;

	// internal state
	std::unique_ptr<u8[]> m_ram;

private:
	// object finder
	required_ioport m_dsw;
};

DEFINE_DEVICE_TYPE_PRIVATE(S100_POLY_16K, device_s100_card_interface, poly_16k_ram_device, "poly16k", "PolyMorphic Systems 16K RAM Card")


//**************************************************************************
//  CONFIGURATION SETTINGS
//**************************************************************************

static INPUT_PORTS_START(poly16k)
	PORT_START("DSW")
	PORT_DIPNAME(0xf, 0xd, "RAM Space") PORT_DIPLOCATION("SW:1,2,3,4")
	PORT_DIPSETTING(0xf, "0000-3FFFH")
	PORT_DIPSETTING(0xe, "1000-4FFFH")
	PORT_DIPSETTING(0xd, "2000-5FFFH")
	PORT_DIPSETTING(0xc, "3000-6FFFH")
	PORT_DIPSETTING(0xb, "4000-7FFFH")
	PORT_DIPSETTING(0xa, "5000-8FFFH")
	PORT_DIPSETTING(0x9, "6000-9FFFH")
	PORT_DIPSETTING(0x8, "7000-AFFFH")
	PORT_DIPSETTING(0x7, "8000-BFFFH")
	PORT_DIPSETTING(0x6, "9000-CFFFH")
	PORT_DIPSETTING(0x5, "A000-DFFFH")
	PORT_DIPSETTING(0x4, "B000-EFFFH")
	PORT_DIPSETTING(0x3, "C000-FFFFH")
	PORT_DIPSETTING(0x2, "D000-FFFFH, 0000-0FFFH")
	PORT_DIPSETTING(0x1, "E000-FFFFH, 0000-1FFFH")
	PORT_DIPSETTING(0x0, "F000-FFFFH, 0000-2FFFH")
INPUT_PORTS_END


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  poly_16k_ram_device - constructor
//-------------------------------------------------

poly_16k_ram_device::poly_16k_ram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, S100_POLY_16K, tag, owner, clock)
	, device_s100_card_interface(mconfig, *this)
	, m_dsw(*this, "DSW")
{
}


//-------------------------------------------------
//  device_input_ports - input port construction
//-------------------------------------------------

ioport_constructor poly_16k_ram_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(poly16k);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void poly_16k_ram_device::device_start()
{
	m_ram = make_unique_clear<u8[]>(0x4000);
	save_pointer(NAME(m_ram), 0x4000);
}


//-------------------------------------------------
//  s100_smemr_r - memory read
//-------------------------------------------------

u8 poly_16k_ram_device::s100_smemr_r(offs_t offset)
{
	u16 addr = offset + (u16(m_dsw->read() + 1) << 12);
	if (addr < 0x4000)
		return m_ram[addr];
	else
		return 0xff;
}


//-------------------------------------------------
//  s100_mwrt_w - memory write
//-------------------------------------------------

void poly_16k_ram_device::s100_mwrt_w(offs_t offset, u8 data)
{
	u16 addr = offset + (u16(m_dsw->read() + 1) << 12);
	if (addr < 0x4000)
		m_ram[addr] = data;
}
