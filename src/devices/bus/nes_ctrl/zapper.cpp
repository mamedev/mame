// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System Zapper Lightgun
    Nintendo Family Computer Bandai Hyper Shot Lightgun
    Nintendo R.O.B.

    TODO:
    - nes_rob is in here because it needs the zapper light sensor,
      in reality it's not connected to the control port at all, but how
      would it be interfaced with the NES driver otherwise?
    - does nes_rob have motor sensors? (eg. limit switches, or optical
      sensor to determine position)
    - can't really play anything with nes_rob, because of interaction
      with physical objects (gyromite especially, since it has a gadget
      to make the robot press joypad buttons)

**********************************************************************/

#include "emu.h"
#include "screen.h"
#include "zapper.h"

#include "nes_rob.lh"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_ZAPPER, nes_zapper_device, "nes_zapper", "Nintendo Zapper Lightgun")
DEFINE_DEVICE_TYPE(NES_BANDAIHS, nes_bandaihs_device, "nes_bandaihs", "Bandai Hyper Shot Lightgun")
DEFINE_DEVICE_TYPE(NES_ROB, nes_rob_device, "nes_rob", "Nintendo R.O.B.")


static INPUT_PORTS_START( nes_zapper )
	PORT_START("ZAPPER_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_MINMAX(0, 255)
	PORT_START("ZAPPER_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(30) PORT_MINMAX(0, 239)
	PORT_START("ZAPPER_T")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Lightgun Trigger")
INPUT_PORTS_END


static INPUT_PORTS_START( nes_bandaihs )
	PORT_INCLUDE( nes_zapper )

	PORT_START("JOYPAD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // has complete joypad inputs except button A
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("%p B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
INPUT_PORTS_END


static INPUT_PORTS_START( nes_rob )
	PORT_INCLUDE( nes_zapper )

	// it has the x/y for aiming the 'eyes', but there is no lightgun trigger
	PORT_MODIFY("ZAPPER_T")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_zapper_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_zapper );
}

ioport_constructor nes_bandaihs_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_bandaihs );
}

ioport_constructor nes_rob_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_rob );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  constructor
//-------------------------------------------------

nes_zapper_device::nes_zapper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_lightx(*this, "ZAPPER_X")
	, m_lighty(*this, "ZAPPER_Y")
	, m_trigger(*this, "ZAPPER_T")
{
}

nes_zapper_device::nes_zapper_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_zapper_device(mconfig, NES_ZAPPER, tag, owner, clock)
{
}

nes_bandaihs_device::nes_bandaihs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_zapper_device(mconfig, NES_BANDAIHS, tag, owner, clock)
	, m_joypad(*this, "JOYPAD")
	, m_latch(0)
{
}

nes_rob_device::nes_rob_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_zapper_device(mconfig, NES_ROB, tag, owner, clock)
	, m_maincpu(*this, "maincpu")
	, m_motor_out(*this, "rob_motor.%u", 0U)
	, m_led_out(*this, "rob_led")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_zapper_device::device_start()
{
}

void nes_bandaihs_device::device_start()
{
	nes_zapper_device::device_start();
	save_item(NAME(m_latch));
	save_item(NAME(m_strobe));
}

void nes_rob_device::device_start()
{
	nes_zapper_device::device_start();

	// resolve handlers
	m_motor_out.resolve();
	m_led_out.resolve();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_zapper_device::read_bit34()
{
	u8 ret = m_trigger->read();
	int x = m_lightx->read();
	int y = m_lighty->read();

	// radius of circle picked up by the gun's photodiode
	constexpr int radius = 5;
	// brightness threshold
	constexpr int bright = 0xc0;
	// # of CRT scanlines that sustain brightness
	constexpr int sustain = 22;

	int vpos = m_port->m_screen->vpos();
	int hpos = m_port->m_screen->hpos();

	// update the screen if necessary
	if (!m_port->m_screen->vblank())
		if (vpos > y - radius || (vpos == y - radius && hpos >= x - radius))
			m_port->m_screen->update_now();

	int sum = 0;
	int scanned = 0;

	// sum brightness of pixels nearby the gun position
	for (int i = x - radius; i <= x + radius; i++)
		for (int j = y - radius; j <= y + radius; j++)
			// look at pixels within circular sensor
			if ((x - i) * (x - i) + (y - j) * (y - j) <= radius * radius)
			{
				rgb_t pix = m_port->m_screen->pixel(i, j);

				// only detect light if gun position is near, and behind, where the PPU is drawing on the CRT, from NesDev wiki:
				// "Zap Ruder test ROM show that the photodiode stays on for about 26 scanlines with pure white, 24 scanlines with light gray, or 19 lines with dark gray."
				if (j <= vpos && j > vpos - sustain && (j != vpos || i <= hpos))
					sum += pix.r() + pix.g() + pix.b();
				scanned++;
			}

	// light not detected if average brightness is below threshold (default bit 3 is 0: light detected)
	if (sum < bright * scanned)
		ret |= 0x08;

	return ret;
}

u8 nes_zapper_device::read_exp(offs_t offset)
{
	u8 ret = 0;
	if (offset == 1)    // $4017
		ret = nes_zapper_device::read_bit34();
	return ret;
}

u8 nes_bandaihs_device::read_exp(offs_t offset)
{
	u8 ret = 0;

	if (offset == 0)    // $4016
	{
		if (m_strobe)
			m_latch = m_joypad->read();
		ret = (m_latch & 1) << 1;
		m_latch = (m_latch >> 1) | 0x80;
	}
	else                // $4017
		ret = nes_zapper_device::read_exp(offset);

	return ret;
}

// In addition to bit 0 used here the real Bandai Hyper Shot also responds to
// bits 1 and 2, neither emulated here. Bit 1 turns on and off a motor, which
// causes the machine gun to shake as if to simulate recoil. Bit 2 turns on and
// off an internal speaker in the gun. Videos demostrate that the speaker plays
// the same audio as the Famicom itself. The analog audio signal of the FC is
// carried on expansion port pin 2, so there's likely nothing more going on.
void nes_bandaihs_device::write(u8 data)
{
	if (write_strobe(data))
		m_latch = m_joypad->read();
}


//-------------------------------------------------
//  R.O.B. specific handlers
//-------------------------------------------------

u8 nes_rob_device::input_r()
{
	// R00: lightsensor
	return (read_bit34() & 8) ? 1 : 0;
}

void nes_rob_device::output_w(offs_t offset, u8 data)
{
	switch (offset & 3)
	{
		case 0:
			// R03: led
			m_led_out = BIT(data, 3);
			break;

		case 1:
			// R10-R13: motors: down, up, close, open
			for (int i = 0; i < 4; i++)
				m_motor_out[i] = BIT(data, i);
			break;

		case 2:
			// R20,R21: motors: right, left
			for (int i = 0; i < 2; i++)
				m_motor_out[i + 4] = BIT(data, i);
			break;

		default:
			break;
	}
}

void nes_rob_device::device_add_mconfig(machine_config &config)
{
	SM590(config, m_maincpu, 455_kHz_XTAL);
	m_maincpu->read_r<0>().set(FUNC(nes_rob_device::input_r));
	m_maincpu->write_r<0>().set(FUNC(nes_rob_device::output_w));
	m_maincpu->write_r<1>().set(FUNC(nes_rob_device::output_w));
	m_maincpu->write_r<2>().set(FUNC(nes_rob_device::output_w));
	m_maincpu->write_r<3>().set(FUNC(nes_rob_device::output_w));

	// must use -numscreens 2 to see the output status
	config.set_default_layout(layout_nes_rob);
}

ROM_START( nes_rob )
	ROM_REGION( 0x200, "maincpu", 0 )
	ROM_LOAD( "rfc-cpu10.ic1", 0x000, 0x200, CRC(f9c96b9c) SHA1(a87e2f0f5e454c093d1352ac368aa9e82e9f6790) )
ROM_END

const tiny_rom_entry *nes_rob_device::device_rom_region() const
{
	return ROM_NAME(nes_rob);
}
