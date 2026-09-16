// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Master System "Graphic Board" emulation

I/O 3f write | this method
        0x20 | 0x7f
        0x00 | 0x3f
        0x30 | 0xff

Typical sequence:
- 3f write 0x20
- read dc
- 3f write 0x00
- read dc
- 3f write 0x20
- read dc
- 3f write 0x30
Suspect from kind of counter that is reset by a 0x30 write to I/O port 0x3f.
Once reset reads from i/O port dc expect to see 0xE0.
And then any write with differing bits goes through several internal I/O ports
with the first port being the one with the buttons

In the reset/start state the lower four/five bits are 0.
Then a nibble is read containing the buttons (active low)
Then 2 nibbles are read to form a byte (first high nibble, then low nibble) indicating
whether the pen is on the graphic board, a value of FD, FE, or FF used for this. For
any other value the following 2 bytes are not read.
Then 2 nibbles are read to form a byte containing the absolute X coordinate.
Then 2 nibbles are read to form a byte containing the absolute Y coordiante.

**********************************************************************/

#include "emu.h"
#include "graphic.h"


namespace {

static INPUT_PORTS_START( sms_graphic )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // MENU
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // DO
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // PEN
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // some kind of ready signal?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_TOGGLE PORT_NAME("%p Pen") // pretend pen pressure is all-or-nothing

	PORT_START("X")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 254 / 268.0, 6 / 268.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(1)

	PORT_START("Y")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 255 / 224.0, -19 / 224.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(1)
INPUT_PORTS_END



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sms_graphic_device : public device_t, public device_sms_control_interface
{
public:
	// construction/destruction
	sms_graphic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_sms_control_interface implementation
	virtual uint8_t in_r() override;
	virtual void out_w(uint8_t data, uint8_t mem_mask) override;

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override { return INPUT_PORTS_NAME(sms_graphic); }
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_buttons;
	required_ioport m_x_axis;
	required_ioport m_y_axis;

	uint8_t m_phase;
	uint8_t m_select;
	uint8_t m_data;
};


//-------------------------------------------------
//  sms_graphic_device - constructor
//-------------------------------------------------

sms_graphic_device::sms_graphic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_GRAPHIC, tag, owner, clock),
	device_sms_control_interface(mconfig, *this),
	m_buttons(*this, "BUTTONS"),
	m_x_axis(*this, "X"),
	m_y_axis(*this, "Y"),
	m_phase(0),
	m_select(0x7f),
	m_data(0xff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_graphic_device::device_start()
{
	m_phase = 0;
	m_data = 0xff;

	save_item(NAME(m_phase));
	save_item(NAME(m_select));
	save_item(NAME(m_data));
}


//-------------------------------------------------
//  sms_peripheral_r - joypad read
//-------------------------------------------------

uint8_t sms_graphic_device::in_r()
{
	if (BIT(m_select, 5))
		return 0x20; // low four bits must be low to recognise Graphic Board, TL is a kind of active-low ready flag
	else if (m_phase)
		return BIT(m_data, BIT(m_select, 6) ? 0 : 4, 4) | 0x30;
	else
		return (m_buttons->read() & 0x1f) | 0x20;
}


void sms_graphic_device::out_w(uint8_t data, uint8_t mem_mask)
{
	if (BIT(data, 5))
	{
		// TR high - deselected
		m_phase = 0;
	}
	else if (!BIT(m_select, 5) && BIT(m_select, 6) && !BIT(data, 6))
	{
		// negative edge on TH seems to trigger acquisition
		switch (m_phase)
		{
		case 0:
			m_data = BIT(m_buttons->read(), 5) ? 0xfe : 0x00; // values made up to satisfy software
			m_phase = 1;
			break;
		case 1:
			m_data = m_x_axis->read();
			m_phase = 2;
			break;
		case 2:
			m_data = m_y_axis->read();
			m_phase = 3;
			break;
		default:
			break; // TODO: what actually happens if you keep clocking TH?
		}
	}

	m_select = data;
}

} // anonymous namespace



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(SMS_GRAPHIC, device_sms_control_interface, sms_graphic_device, "sms_graphic", "Sega Master System Graphic Board")
