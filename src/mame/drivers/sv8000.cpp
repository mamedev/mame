// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Robbbert
/***************************************************************************

    Bandai Super Vision 8000 (TV Jack 8000)
      driver by Wilbert Pol, Robbbert, ranger_lennier, and Charles McDonald

        2014/01/07 Skeleton driver.

The Bandai Super Vision 8000 contains:
- NEC D78C (Z80)
- AY-3-8910
- AMI S68047P (6847 variant)
- NEC D8255C

Looking at the code of the cartridges it seems there is:
- 1KB of main system RAM
- 3KB of video RAM

    TODO:
    - Check configuration of S68047P pins through 8910 port A against
      schematics
    - Verify clock

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6847.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

class sv8000_state : public driver_device
{
public:
	sv8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_s68047p(*this, "s68047p")
		, m_cart(*this, "cartslot")
		, m_videoram(*this, "videoram")
		, m_io_row0(*this, "ROW0")
		, m_io_row1(*this, "ROW1")
		, m_io_row2(*this, "ROW2")
		, m_io_joy(*this, "JOY")
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

	DECLARE_READ8_MEMBER( mc6847_videoram_r );

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<s68047_device> m_s68047p;
	required_device<generic_slot_device> m_cart;
	required_shared_ptr<UINT8> m_videoram;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_joy;

	UINT8 m_column;

	// graphics signals
	UINT8 m_ag;
	UINT8 m_gm2;
	UINT8 m_gm1;
	UINT8 m_gm0;
	UINT8 m_as;
	UINT8 m_css;
	UINT8 m_intext;
	UINT8 m_inv;
};


static ADDRESS_MAP_START(sv8000_mem, AS_PROGRAM, 8, sv8000_state)
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x0000, 0x0fff)      // mapped by the cartslot
	AM_RANGE( 0x8000, 0x83ff ) AM_RAM // Work RAM??
	AM_RANGE( 0xc000, 0xcbff ) AM_RAM AM_SHARE("videoram")
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
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_NAME("Left 1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_NAME("Left 4") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_NAME("Left 7") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left *") // Guess
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Right 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Right 4") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Right 7") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Right *") // Guess

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_NAME("Left 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_NAME("Left 5") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_NAME("Left 8") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_NAME("Left 0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Right 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Right 5") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Right 8") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Right 0")

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_NAME("Left 3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_NAME("Left 6") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_NAME("Left 9") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_NAME("Left #") // Guess
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Right 3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Right 6") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Right 9") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Right #") // Guess

	PORT_START("JOY")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Left Right")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Left Left")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_NAME("Left Down")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_NAME("Left Up")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right Right")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("Right Left")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("Right Down")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("Right Up")
INPUT_PORTS_END


void sv8000_state::machine_start()
{
	m_ag = 0;
	m_gm2 = 0;
	m_gm1 = 0;
	m_gm0 = 0;
	m_as = 0;
	m_css = 0;
	m_intext = 0;
	m_inv = 0;

	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x0fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));

	save_item(NAME(m_column));
	save_item(NAME(m_ag));
	save_item(NAME(m_gm2));
	save_item(NAME(m_gm1));
	save_item(NAME(m_gm0));
	save_item(NAME(m_as));
	save_item(NAME(m_css));
	save_item(NAME(m_intext));
	save_item(NAME(m_inv));
}


void sv8000_state::machine_reset()
{
	m_column = 0xff;
}


DEVICE_IMAGE_LOAD_MEMBER( sv8000_state, cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	if (size != 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Incorrect or not support cartridge size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}


READ8_MEMBER( sv8000_state::i8255_porta_r )
{
	//logerror("i8255_porta_r\n");
	return m_io_joy->read();
}


WRITE8_MEMBER( sv8000_state::i8255_porta_w )
{
	//logerror("i8255_porta_w: %02X\n", data);
}


READ8_MEMBER( sv8000_state::i8255_portb_r )
{
	UINT8 data = 0xff;

	//logerror("i8255_portb_r\n");

	if ( ! ( m_column & 0x01 ) )
	{
		data &= m_io_row0->read();
	}
	if ( ! ( m_column & 0x02 ) )
	{
		data &= m_io_row1->read();
	}
	if ( ! ( m_column & 0x04 ) )
	{
		data &= m_io_row2->read();
	}
	return data;
}


WRITE8_MEMBER( sv8000_state::i8255_portb_w )
{
	//logerror("i8255_portb_w: %02X\n", data);
}


READ8_MEMBER( sv8000_state::i8255_portc_r )
{
	//logerror("i8255_portc_r\n");
	return 0xff;
}


WRITE8_MEMBER( sv8000_state::i8255_portc_w )
{
	//logerror("i8255_portc_w: %02X\n", data);
	m_column = data;
}


READ8_MEMBER( sv8000_state::ay_port_a_r )
{
	UINT8 data = 0xff;

	//logerror("ay_port_a_r\n");
	return data;
}


READ8_MEMBER( sv8000_state::ay_port_b_r )
{
	UINT8 data = 0xff;

	//logerror("ay_port_b_r\n");
	return data;
}


// Possibly connected to S68047P for selecting text/graphics modes
// misvader:
// 0x42 01000010 set on normal text screen
// 0x5A 01011010 set for a 256x192 bit mapped screen 3KB in 6KB mode?
//
// spfire:
// 0x42 01000010 text
// 0x5A 01011010 graphics 3KB in 6KB mode?
//
// othello:
// 0x02 00000010 normal text screen
// 0x58 01011000 graphics 3KB in 6KB mode?
//
// gunprof:
// 0x00 00000000 text
// 0x38 00111000 graphics 3KB mode
//
// pacpac:
// 0x00 00000000 text
// 0x5A 01011010 graphics 3KB in 6KB mode?
//
// submar:
// 0x00 00000000 text
// 0x5A 01011010 graphics 3KB in 6KB mode?
//
// beamgala:
// 0x5A 01011010 graphics 3KB in 6KB mode?
//
WRITE8_MEMBER( sv8000_state::ay_port_a_w )
{
	//logerror("ay_port_a_w: %02X\n", data);

	// Lacking schematics, these are all wild guesses
	// Having bit 1 set makes black display as blue??
	m_ag = BIT(data, 4);
	m_gm2 = BIT(data, 6);
	m_gm1 = BIT(data, 3);
	m_gm0 = BIT(data, 3);
	m_css = m_ag;

	m_s68047p->ag_w( m_ag ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->gm2_w( m_gm2 ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->gm1_w( m_gm1 ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->gm0_w( m_gm0 ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->css_w( m_css ? ASSERT_LINE : CLEAR_LINE );
	m_s68047p->hack_black_becomes_blue( BIT(data, 1) );
}


WRITE8_MEMBER( sv8000_state::ay_port_b_w )
{
	//logerror("ay_port_b_w: %02X\n", data);
}

READ8_MEMBER( sv8000_state::mc6847_videoram_r )
{
	if (offset == ~0) return 0xff;

	if ( m_ag )
	{
		if ( m_gm2 )
		{
			// 256 x 192 / 6KB
			offset = ( ( offset & 0x1fc0 ) >> 1 ) | ( offset & 0x1f );
			return m_videoram[offset % 0xc00];
		}
		else
		{
			// 256 x 96 / 3KB
			return m_videoram[offset % 0xc00];
		}
	}

	// Standard text
	UINT8 data = m_videoram[offset % 0xc00];
	if (!data) data = 0x20; //bodge

	m_s68047p->inv_w( ( data & 0x80 ) ? ASSERT_LINE : CLEAR_LINE );

	return data;
}

static MACHINE_CONFIG_START( sv8000, sv8000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_10_738635MHz/3)  /* Not verified */
	MCFG_CPU_PROGRAM_MAP(sv8000_mem)
	MCFG_CPU_IO_MAP(sv8000_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sv8000_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("i8255", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(sv8000_state, i8255_porta_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(sv8000_state, i8255_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(sv8000_state, i8255_portb_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(sv8000_state, i8255_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(sv8000_state, i8255_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(sv8000_state, i8255_portc_w))

	/* video hardware */
	// S68047P - Unknown whether the internal or an external character rom is used
	MCFG_DEVICE_ADD("s68047p", S68047, XTAL_10_738635MHz/3 )  // Clock not verified
	MCFG_MC6847_INPUT_CALLBACK(READ8(sv8000_state, mc6847_videoram_r))

	MCFG_SCREEN_MC6847_NTSC_ADD("screen", "s68047p")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_10_738635MHz/3/2)  /* Exact model and clock not verified */
	MCFG_AY8910_PORT_A_READ_CB(READ8(sv8000_state, ay_port_a_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(sv8000_state, ay_port_b_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(sv8000_state, ay_port_a_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(sv8000_state, ay_port_b_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "sv8000_cart")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(sv8000_state, cart)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","sv8000")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sv8000 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   INIT                  COMPANY   FULLNAME                            FLAGS */
CONS( 1979, sv8000, 0,      0,       sv8000,  sv8000, driver_device,   0,   "Bandai", "Super Vision 8000 (TV Jack 8000)", 0 )
