// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    MR8000 Master RAM Cartridge emulation

***************************************************************************/

#include "emu.h"
#include "mr8000.h"


namespace {

class bbc_mr8000_device : public device_t, public device_bbc_cart_interface
{
public:
	bbc_mr8000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_MR8000, tag, owner, clock)
		, device_bbc_cart_interface(mconfig, *this)
		, m_switch(*this, "SWITCH")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// bbc_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	required_ioport m_switch;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

INPUT_PORTS_START(mr8000)
	PORT_START("SWITCH")
	PORT_CONFNAME(0x03, 0x00, "ROM Choice")
	PORT_CONFSETTING(0x00, "A (lower banks)")
	PORT_CONFSETTING(0x01, "B (upper banks)")
	PORT_CONFSETTING(0x02, "Off (disabled)")

	PORT_CONFNAME(0x04, 0x00, "Write Protect")
	PORT_CONFSETTING(0x00, "On (read only")
	PORT_CONFSETTING(0x04, "Off (read and write)")
INPUT_PORTS_END

ioport_constructor bbc_mr8000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mr8000);
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t bbc_mr8000_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0x00;
	int bank = BIT(m_switch->read(), 0);

	if (oe && !BIT(m_switch->read(), 1))
	{
		data = m_nvram[offset | (bank << 15) | (romqa << 14)];
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void bbc_mr8000_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	int bank = BIT(m_switch->read(), 0);

	if (oe && !BIT(m_switch->read(), 1))
	{
		if (BIT(m_switch->read(), 2))
		{
			m_nvram[offset | (bank << 15) | (romqa << 14)] = data;
		}
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_MR8000, device_bbc_cart_interface, bbc_mr8000_device, "bbc_mr8000", "MR8000 Master RAM Cartridge")
