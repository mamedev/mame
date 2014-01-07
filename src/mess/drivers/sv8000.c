// license:BSD
// copyright-holders:Wilbert Pol
/***************************************************************************

    Bandai Super Vision 8000 (TV Jack 8000)
      driver by Wilbert Pol, ranger_lennier, and Charles McDonald

        2014/01/07 Skeleton driver.

    TODO:
    - Implement S68074P video chip
    - Figure out configuration of S68047P pins through 8910 port A
    - Figure out input ports, left and right might be swapped
    - Verify clock

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cartslot.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"


class sv8000_state : public driver_device
{
public:
	sv8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_row0_left(*this, "ROW0_LEFT")
		, m_io_row1_left(*this, "ROW1_LEFT")
		, m_io_row2_left(*this, "ROW2_LEFT")
	{ }

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( cart );
	DECLARE_READ8_MEMBER( ay_port_a_r );
	DECLARE_READ8_MEMBER( ay_port_b_r );
	DECLARE_WRITE8_MEMBER( ay_port_a_w );
	DECLARE_WRITE8_MEMBER( ay_port_b_w );

	DECLARE_READ8_MEMBER( i8255_porta_r );
	DECLARE_WRITE8_MEMBER( i8255_porta_w );
	DECLARE_READ8_MEMBER( i8255_portb_r );
	DECLARE_WRITE8_MEMBER( i8255_portb_w );
	DECLARE_READ8_MEMBER( i8255_portc_r );
	DECLARE_WRITE8_MEMBER( i8255_portc_w );

private:
	virtual void machine_start();
	virtual void machine_reset();

	required_device<cpu_device> m_maincpu;
	required_ioport m_io_row0_left;
	required_ioport m_io_row1_left;
	required_ioport m_io_row2_left;

	UINT8 m_column;
};


static ADDRESS_MAP_START(sv8000_mem, AS_PROGRAM, 8, sv8000_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x0fff ) AM_ROM
	AM_RANGE( 0x8000, 0x83ff ) AM_RAM // Work RAM??
	AM_RANGE( 0xc000, 0xcbff ) AM_RAM // Display RAM??
ADDRESS_MAP_END


static ADDRESS_MAP_START(sv8000_io, AS_IO, 8, sv8000_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("i8255", i8255_device, read, write)
	AM_RANGE(0xc0, 0xc0) AM_DEVWRITE("ay8910", ay8910_device, data_w)   // Not sure yet
	AM_RANGE(0xc1, 0xc1) AM_DEVWRITE("ay8910", ay8910_device, address_w) // Not sure yet
ADDRESS_MAP_END


/* Input ports */
// On the main console:
//
//  1 2 3                              1 2 3
//  4 5 6                              4 5 6
//  7 8 9                              7 8 9
//  * 0 #                              * 0 #
//
//  Button/dial?    POWER   RESET    Button/dial?
//
static INPUT_PORTS_START( sv8000 )
	PORT_START("ROW0_LEFT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("Left 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Left 4") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("Left 7") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left *") // Guess
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW1_LEFT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("Left 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("Left 5") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("Left 8") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_NAME("Left 0")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("ROW2_LEFT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("Left 3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("Left 6") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_NAME("Left 9") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("Left #") // Guess
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

//	PORT_START("ROW0_RIGHT")
//	PORT_START("ROW1_RIGHT")
//	PORT_START("ROW2_RIGHT")
INPUT_PORTS_END


void sv8000_state::machine_start()
{
	save_item(NAME(m_column));
}


void sv8000_state::machine_reset()
{
	m_column = 0xff;
}


DEVICE_IMAGE_LOAD_MEMBER( sv8000_state, cart )
{
	UINT8 *cart = memregion("maincpu")->base();

	if (image.software_entry() == NULL)
	{
		UINT32 filesize = image.length();

		if (filesize != 0x1000)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Incorrect or not support cartridge size");
			return IMAGE_INIT_FAIL;
		}

		if (image.fread( cart, filesize) != filesize)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Error loading file");
			return IMAGE_INIT_FAIL;
		}
	}
	else
	{
		memcpy(cart, image.get_software_region("rom"), image.get_software_region_length("rom"));
	}

	return IMAGE_INIT_PASS;
}


READ8_MEMBER( sv8000_state::i8255_porta_r )
{
	logerror("i8255_porta_r\n");
	return 0xFF;
}


WRITE8_MEMBER( sv8000_state::i8255_porta_w )
{
	logerror("i8255_porta_w: %02X\n", data);
}


READ8_MEMBER( sv8000_state::i8255_portb_r )
{
	UINT8 data = 0xff;

	logerror("i8255_portb_r\n");

	if ( ! ( m_column & 0x01 ) )
	{
		data &= m_io_row0_left->read();
	}
	if ( ! ( m_column & 0x02 ) )
	{
		data &= m_io_row1_left->read();
	}
	if ( ! ( m_column & 0x04 ) )
	{
		data &= m_io_row2_left->read();
	}
	return data;
}


WRITE8_MEMBER( sv8000_state::i8255_portb_w )
{
	logerror("i8255_portb_w: %02X\n", data);
}


READ8_MEMBER( sv8000_state::i8255_portc_r )
{
	logerror("i8255_portc_r\n");
	return 0xFF;
}


WRITE8_MEMBER( sv8000_state::i8255_portc_w )
{
	logerror("i8255_portc_w: %02X\n", data);
	m_column = data;
}


static I8255_INTERFACE( sv8000_i8255_interface )
{
    DEVCB_DRIVER_MEMBER(sv8000_state, i8255_porta_r),   /* port A read */
    DEVCB_DRIVER_MEMBER(sv8000_state, i8255_porta_w),   /* port A write */
    DEVCB_DRIVER_MEMBER(sv8000_state, i8255_portb_r),   /* port B read */
    DEVCB_DRIVER_MEMBER(sv8000_state, i8255_portb_w),   /* port B write */
    DEVCB_DRIVER_MEMBER(sv8000_state, i8255_portc_r),   /* port C read */
    DEVCB_DRIVER_MEMBER(sv8000_state, i8255_portc_w)    /* port C write */
};


READ8_MEMBER( sv8000_state::ay_port_a_r )
{
	UINT8 data = 0xFF;

	logerror("ay_port_a_r\n");
	return data;
}


READ8_MEMBER( sv8000_state::ay_port_b_r )
{
	UINT8 data = 0xff;

	logerror("ay_port_b_r\n");
	return data;
}


// Possibly connected to S68047P for selecting text/graphics modes
WRITE8_MEMBER( sv8000_state::ay_port_a_w )
{
	logerror("ay_port_a_w: %02X\n", data);
}


WRITE8_MEMBER( sv8000_state::ay_port_b_w )
{
	logerror("ay_port_b_w: %02X\n", data);
}


static const ay8910_interface sv8000_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_DRIVER_MEMBER(sv8000_state, ay_port_a_r),
	DEVCB_DRIVER_MEMBER(sv8000_state, ay_port_b_r),
	DEVCB_DRIVER_MEMBER(sv8000_state, ay_port_a_w),
	DEVCB_DRIVER_MEMBER(sv8000_state, ay_port_b_w)
};


static MACHINE_CONFIG_START( sv8000, sv8000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_10_738635MHz/3)  /* Not verified */
	MCFG_CPU_PROGRAM_MAP(sv8000_mem)
	MCFG_CPU_IO_MAP(sv8000_io)

	MCFG_I8255_ADD( "i8255", sv8000_i8255_interface )

	/* video hardware */
	// S68047P - Unknown whether the internal or an external character rom is used

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_10_738635MHz/3/2)  /* Exact model and clock not verified */
	MCFG_SOUND_CONFIG(sv8000_ay8910_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_LOAD(sv8000_state,cart)
	MCFG_CARTSLOT_INTERFACE("sv8000_cart")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","sv8000")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sv8000 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   INIT                  COMPANY   FULLNAME                            FLAGS */
COMP( 1979, sv8000, 0,      0,       sv8000,  sv8000, driver_device,   0,   "Bandai", "Super Vision 8000 (TV Jack 8000)", GAME_IS_SKELETON )

