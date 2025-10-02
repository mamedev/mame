// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Master Smart Cartridge emulation

***************************************************************************/

#include "emu.h"
#include "msc.h"


namespace {

class bbc_msc_device : public device_t, public device_bbc_cart_interface
{
public:
	bbc_msc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_MSC, tag, owner, clock)
		, device_bbc_cart_interface(mconfig, *this)
		, m_button(*this, "BUTTON")
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(activate);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// bbc_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;

private:
	required_ioport m_button;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

INPUT_PORTS_START(msc)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Activate") PORT_CODE(KEYCODE_HOME) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bbc_msc_device::activate), 0)
INPUT_PORTS_END

ioport_constructor bbc_msc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msc);
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t bbc_msc_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe & !romqa)
	{
		if (offset & 0x2000)
		{
			data = m_ram[offset & 0x7ff];
		}
		else
		{
			data = m_rom[(offset & 0x1fff) | (m_button->read() << 13)];
		}
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void bbc_msc_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (oe & !romqa)
	{
		if (offset & 0x2000)
		{
			m_ram[offset & 0x7ff] = data;
		}
	}
}

INPUT_CHANGED_MEMBER(bbc_msc_device::activate)
{
	m_slot->irq_w(!newval);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_MSC, device_bbc_cart_interface, bbc_msc_device, "bbc_msc", "Master Smart Cartridge")
