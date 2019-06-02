// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Taito Yoke + Throttle Flight device

    Used in Midnight Landing, Top Landing and Air Inferno

    TODO:
    - Custom part #;
    - Air Inferno uses different limit types (helicopter inputs?), might be
      worth doing a subclass of this;
    - Get dead zones from actual HW (currently hardlocked to 0x20);

***************************************************************************/

#include "emu.h"
#include "taitoio_yoke.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TAITOIO_YOKE, taitoio_yoke_device, "taitoio_yoke", "Taito I/O Yoke")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  taitoio_yoke_device - constructor
//-------------------------------------------------

taitoio_yoke_device::taitoio_yoke_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TAITOIO_YOKE, tag, owner, clock)
{
}




//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void taitoio_yoke_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void taitoio_yoke_device::device_reset()
{
}

// TODO: requires LEFT/RIGHT_AD_STICK in framework
static INPUT_PORTS_START( yoke_inputs )
	PORT_START("STICK_X")
	PORT_BIT( 0x0fff, 0x0000, IPT_AD_STICK_X ) PORT_MINMAX(0x00800, 0x07ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_NAME("Yoke X")

	PORT_START("STICK_Y")
	PORT_BIT( 0x0fff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0x00800, 0x07ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_NAME("Yoke Y")

	PORT_START("THROTTLE")
	PORT_BIT( 0x0fff, 0x0000, IPT_AD_STICK_Y ) PORT_MINMAX(0x0800,0x07ff) PORT_SENSITIVITY(30) PORT_KEYDELTA(40) PORT_NAME("Throttle Lever")
INPUT_PORTS_END

ioport_constructor taitoio_yoke_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( yoke_inputs );
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_MEMBER( taitoio_yoke_device::stickx_r )
{
	return ioport("STICK_X")->read();
}

READ16_MEMBER( taitoio_yoke_device::sticky_r )
{
	return ioport("STICK_Y")->read();
}

READ16_MEMBER( taitoio_yoke_device::throttle_r )
{
	return ioport("THROTTLE")->read();
}

READ_LINE_MEMBER( taitoio_yoke_device::slot_down_r )
{
	uint16_t throttle = ioport("THROTTLE")->read();

	return (throttle & 0xe00) == 0x600;
}

READ_LINE_MEMBER( taitoio_yoke_device::slot_up_r )
{
	uint16_t throttle = ioport("THROTTLE")->read();

	return (throttle & 0xe00) == 0x800;
}

READ_LINE_MEMBER( taitoio_yoke_device::handle_left_r )
{
	uint16_t x = ioport("STICK_X")->read();

	return (x & 0xe00) == 0x800;
}

READ_LINE_MEMBER( taitoio_yoke_device::handle_right_r )
{
	uint16_t x = ioport("STICK_X")->read();

	return (x & 0xe00) == 0x600;
}

READ_LINE_MEMBER( taitoio_yoke_device::handle_up_r )
{
	uint16_t y = ioport("STICK_Y")->read();

	return (y & 0xe00) == 0x800;
}

READ_LINE_MEMBER( taitoio_yoke_device::handle_down_r )
{
	uint16_t y = ioport("STICK_Y")->read();

	return (y & 0xe00) == 0x600;
}
