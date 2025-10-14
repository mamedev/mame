// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Slogger Click cartridge emulation

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Slogger_Click.html

    The Master version of the Click cartridge differs from the Electron
    version:
                    Master 128          Electron
    ROM               16K                 32K
    RAM (battery)     8K                  32K

    The data lines D1 and D2 are swapped between edge connector and ROM/RAM.

***************************************************************************/

#include "emu.h"
#include "click.h"


namespace {

class bbc_click_device : public device_t, public device_bbc_cart_interface
{
public:
	bbc_click_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_CLICK, tag, owner, clock)
		, device_bbc_cart_interface(mconfig, *this)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(click_button);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// bbc_cart_interface overrides
	virtual uint8_t read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2) override;
	virtual void write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2) override;
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

INPUT_PORTS_START(clickm)
	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Click") PORT_CODE(KEYCODE_HOME) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(bbc_click_device::click_button), 0)
INPUT_PORTS_END

ioport_constructor bbc_click_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(clickm);
}


//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t bbc_click_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe)
	{
		if (offset & 0x2000)
		{
			data = m_nvram[offset & 0x1fff];
		}
		else
		{
			data = m_rom[(offset & 0x1fff) | (romqa << 13)];
		}
	}

	return bitswap<8>(data, 7, 6, 5, 4, 3, 1, 2, 0);
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void bbc_click_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (oe)
	{
		if (offset & 0x2000)
		{
			m_nvram[offset & 0x1fff] = bitswap<8>(data, 7, 6, 5, 4, 3, 1, 2, 0);
		}
	}
}

INPUT_CHANGED_MEMBER(bbc_click_device::click_button)
{
	m_slot->irq_w(!newval);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_CLICK, device_bbc_cart_interface, bbc_click_device, "bbc_click", "Slogger Click (Master 128) cartridge")
