// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Compucolor II joystick and AY-3-8910 sound board

    Third-party board, port map from the JOYUTL and AYSND sources on
    the "Joystick & Sound" disk:

    20-27   W   ADC0809 start conversion of channel A2-A0 (X0 Y0 .. X3 Y3)
            R   ADC0809 result
    30      W   AY-3-8910 address latch
            R   AY-3-8910 data
    31      W   AY-3-8910 data

    AY-3-8910 I/O port A:

    0-2         unused
    3-6         fire buttons 0-3, active low
    7           ADC0809 end of conversion

    TODO:
    - AY-3-8910 and ADC0809 clock sources are unknown

**********************************************************************/

#include "emu.h"
#include "joysnd.h"

#include "machine/adc0808.h"
#include "sound/ay8910.h"

#include "speaker.h"


namespace {

class compucolor_joysnd_device : public device_t, public device_compucolor_expansion_card_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	compucolor_joysnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, COMPUCOLOR_JOYSND, tag, owner, clock)
		, device_compucolor_expansion_card_interface(mconfig, *this)
		, m_adc(*this, "adc")
		, m_psg(*this, "psg")
		, m_joy(*this, "JOY%u", 0U)
		, m_buttons(*this, "BUTTONS")
	{
	}

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<adc0809_device> m_adc;
	required_device<ay8910_device> m_psg;
	required_ioport_array<8> m_joy;
	required_ioport m_buttons;

	uint8_t porta_r();
};


static INPUT_PORTS_START( compucolor_joysnd )
	PORT_START("JOY0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("JOY1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("JOY2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("JOY3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("JOY4")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("JOY5")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)

	PORT_START("JOY6")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4)

	PORT_START("JOY7")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(4)

	PORT_START("BUTTONS")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


ioport_constructor compucolor_joysnd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( compucolor_joysnd );
}


void compucolor_joysnd_device::device_add_mconfig(machine_config &config)
{
	ADC0809(config, m_adc, DERIVED_CLOCK(1, 4));
	m_adc->in_callback<0>().set_ioport(m_joy[0]);
	m_adc->in_callback<1>().set_ioport(m_joy[1]);
	m_adc->in_callback<2>().set_ioport(m_joy[2]);
	m_adc->in_callback<3>().set_ioport(m_joy[3]);
	m_adc->in_callback<4>().set_ioport(m_joy[4]);
	m_adc->in_callback<5>().set_ioport(m_joy[5]);
	m_adc->in_callback<6>().set_ioport(m_joy[6]);
	m_adc->in_callback<7>().set_ioport(m_joy[7]);

	SPEAKER(config, "mono").front_center();

	AY8910(config, m_psg, DERIVED_CLOCK(1, 1));
	m_psg->port_a_read_callback().set(FUNC(compucolor_joysnd_device::porta_r));
	m_psg->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void compucolor_joysnd_device::device_start()
{
	io_space().install_readwrite_handler(0x20, 0x27, emu::rw_delegate(*m_adc, FUNC(adc0809_device::data_r)), emu::rw_delegate(*m_adc, FUNC(adc0809_device::address_offset_start_w)));
	io_space().install_readwrite_handler(0x30, 0x30, emu::rw_delegate(*m_psg, FUNC(ay8910_device::data_r)), emu::rw_delegate(*m_psg, FUNC(ay8910_device::address_w)));
	io_space().install_write_handler(0x31, 0x31, emu::rw_delegate(*m_psg, FUNC(ay8910_device::data_w)));
}


uint8_t compucolor_joysnd_device::porta_r()
{
	return (m_buttons->read() & 0x7f) | (m_adc->eoc_r() ? 0x80 : 0x00);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(COMPUCOLOR_JOYSND, device_compucolor_expansion_card_interface, compucolor_joysnd_device, "compclr_joysnd", "Compucolor II joystick and AY-3-8910 sound board")
