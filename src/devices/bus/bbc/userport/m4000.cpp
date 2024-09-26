// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Hybrid Music 4000 Keyboard (also known as ATPL Symphony)

    https://www.retro-kit.co.uk/page.cfm/content/Hybrid-Music-4000-Keyboard/
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Hybrid_Music4000.html

**********************************************************************/

#include "emu.h"
#include "m4000.h"


namespace {


class bbc_m4000_device : public device_t, public device_bbc_userport_interface
{
public:
	bbc_m4000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_M4000, tag, owner, clock)
		, device_bbc_userport_interface(mconfig, *this)
		, m_kbd(*this, "KBLOCK_%u", 1)
		, m_clk(0)
		, m_dsb(0)
		, m_out(0)
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t pb_r() override;
	virtual void write_cb1(int state) override;
	virtual void write_cb2(int state) override;

private:
	required_ioport_array<8> m_kbd;

	int m_clk;
	int m_dsb;
	uint8_t m_out;
};


//-------------------------------------------------
//  INPUT_PORTS( m4000 )
//-------------------------------------------------

static INPUT_PORTS_START( m4000 )
	PORT_START("KBLOCK_1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C2")  PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C2#") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D2")  PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D2#") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E2")  PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")  PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2#") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G2")  PORT_CODE(KEYCODE_R)

	PORT_START("KBLOCK_2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G2#") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A2")  PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A2#") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B2")  PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C3")  PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C3#") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D3")  PORT_CODE(KEYCODE_I)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D3#") PORT_CODE(KEYCODE_9)

	PORT_START("KBLOCK_3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E3")  PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")  PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3#") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G3")  PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G3#") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A3")  PORT_CODE(KEYCODE_X)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A3#") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B3")  PORT_CODE(KEYCODE_C)

	PORT_START("KBLOCK_4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C4")  PORT_CODE(KEYCODE_V)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C4#") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D4")  PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D4#") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E4")  PORT_CODE(KEYCODE_N)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")  PORT_CODE(KEYCODE_M)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4#") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G4")  PORT_CODE(KEYCODE_COMMA)

	PORT_START("KBLOCK_5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G4#") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A4")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A4#") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B4")  PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C5")  PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C5#")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D5#")

	PORT_START("KBLOCK_6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E5")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5#")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G5#")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A5#")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B5")

	PORT_START("KBLOCK_7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C6")
	PORT_BIT(0xfe, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KBLOCK_8")
	PORT_BIT(0x7f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Foot Switch")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_m4000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( m4000 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_m4000_device::device_start()
{
	save_item(NAME(m_clk));
	save_item(NAME(m_dsb));
	save_item(NAME(m_out));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_m4000_device::write_cb1(int state)
{
	// 74hc164
	if (!m_clk && state)
	{
		m_out = (m_out << 1) | m_dsb;
	}
	m_clk = state;
}

void bbc_m4000_device::write_cb2(int state)
{
	// 74hc164
	m_dsb = state;
}

uint8_t bbc_m4000_device::pb_r()
{
	uint8_t data = 0xff;

	for (int block = 0; block < 8; block++)
	{
		if (!BIT(m_out, block))
			data = m_kbd[block]->read();
	}

	return data;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_M4000, device_bbc_userport_interface, bbc_m4000_device, "bbc_m4000", "Hybrid Music 4000 Keyboard")
