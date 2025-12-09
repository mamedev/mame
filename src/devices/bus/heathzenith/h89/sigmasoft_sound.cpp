// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Sound Effects Board

****************************************************************************/

#include "emu.h"

#include "sound/ay8910.h"
#include "speaker.h"

#include "sigmasoft_sound.h"

//
// Logging defines
//
#define LOG_REG  (1U << 1)   // Shows register setup
#define LOG_FUNC (1U << 2)   // Function calls
#define LOG_ERR  (1U << 3)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)
#define LOGERR(...)        LOGMASKED(LOG_ERR, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

namespace {

class h89bus_sigmasoft_snd_device : public device_t, public device_h89bus_right_card_interface
{
public:
	h89bus_sigmasoft_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	u8 read(offs_t reg);
	void write(offs_t reg, u8 val);

	u8 read_joystick();

	u8 transform_joystick_input(u8 raw_value);

private:
	required_device<ay8910_device> m_ay8910;
	required_ioport m_joystick1, m_joystick2;
	required_ioport m_config;

	bool m_installed;

	u8 m_port_selection;
};

//**************************************************************************
//  INPUT PORTS
//**************************************************************************
static INPUT_PORTS_START( sigma_sound )
	PORT_START("joystick_p1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1)

	PORT_START("joystick_p2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(2)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x01, "Port selection" )
	PORT_CONFSETTING(    0x00, "Disabled" )
	PORT_CONFSETTING(    0x01, "AUX - 320 Octal (0xD0)" )
	PORT_CONFSETTING(    0x02, "MODEM - 330 Octal (0xD8)" )
	PORT_CONFSETTING(    0x03, "LP - 340 Octal (0xE0)" )
INPUT_PORTS_END


h89bus_sigmasoft_snd_device::h89bus_sigmasoft_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H89BUS_SIGMASOFT_SND, tag, owner, clock)
	, device_h89bus_right_card_interface(mconfig, *this)
	, m_ay8910(*this, "ay8910")
	, m_joystick1(*this, "joystick_p1")
	, m_joystick2(*this, "joystick_p2")
	, m_config(*this, "CONFIG")
{
}

void h89bus_sigmasoft_snd_device::write(offs_t reg, u8 val)
{
	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, reg, val);

	switch (reg)
	{
		case 0:
			m_ay8910->data_w(val);
			break;
		case 1:
			m_ay8910->address_w(val);
			break;
		default:
			LOGERR("%s: unexpected port write: %d - 0x%02x\n", FUNCNAME, reg, val);
	}
}

u8 h89bus_sigmasoft_snd_device::read(offs_t reg)
{
	u8 value = 0x00;

	switch (reg)
	{
		case 0:
			value = m_ay8910->data_r();
			break;
		case 1:
			value = this->read_joystick();
			break;
		default:
			LOGERR("%s: unexpected port read: %d\n", FUNCNAME, reg);
	}

	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, reg, value);

	return value;
}

u8 h89bus_sigmasoft_snd_device::transform_joystick_input(u8 raw_value)
{
	// when button on joystick is pressed, return 0xf
	if (raw_value & 0x10)
	{
		return 0xf;
	}

	return raw_value & 0xf;
}

// high nibble is for left joystick, low nibble is for right joystick
// when the button is pressed nibble will be set to 0xf
//
// note: 2 bits may be set if the direction of the joystick is diagonal, such as 0x9 for left & up.
//
u8 h89bus_sigmasoft_snd_device::read_joystick()
{
	u8 joy1 = m_joystick1->read();
	u8 joy2 = m_joystick2->read();

	return (this->transform_joystick_input(joy1) << 4) | this->transform_joystick_input(joy2);
}

void h89bus_sigmasoft_snd_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
}

void h89bus_sigmasoft_snd_device::device_reset()
{
	ioport_value const config(m_config->read());

	switch (config & 0x03)
	{
		case 0x00:
			m_port_selection = 0;
			break;
		case 0x01:
			m_port_selection = h89bus::IO_SER0;
			break;
		case 0x02:
			m_port_selection = h89bus::IO_SER1;
			break;
		case 0x03:
			m_port_selection = h89bus::IO_LP;
			break;
	}

	if (!m_installed && (m_port_selection != 0))
	{
		h89bus::addr_ranges  addr_ranges = h89bus().get_address_ranges(m_port_selection);

		if (addr_ranges.size() == 1)
		{
			h89bus::addr_range range = addr_ranges.front();

			h89bus().install_io_device(range.first, range.second,
				read8sm_delegate(*this, FUNC(h89bus_sigmasoft_snd_device::read)),
				write8sm_delegate(*this, FUNC(h89bus_sigmasoft_snd_device::write)));
		}

		m_installed = true;
	}
}

ioport_constructor h89bus_sigmasoft_snd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sigma_sound );
}

void h89bus_sigmasoft_snd_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay8910, XTAL(8'000'000)/4); /* ??? 2.000 MHz */
	m_ay8910->add_route(ALL_OUTPUTS, "mono", 0.25);
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_SIGMASOFT_SND, device_h89bus_right_card_interface, h89bus_sigmasoft_snd_device, "h89_sigma_snd", "SigmaSoft Sound Effects Board");
