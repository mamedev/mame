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
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

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

	void sv8000(machine_config &config);

private:
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

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void sv8000_io(address_map &map);
	void sv8000_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<s68047_device> m_s68047p;
	required_device<generic_slot_device> m_cart;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_joy;

	uint8_t m_column;

	// graphics signals
	uint8_t m_ag;
	uint8_t m_gm2;
	uint8_t m_gm1;
	uint8_t m_gm0;
	uint8_t m_as;
	uint8_t m_css;
	uint8_t m_intext;
	uint8_t m_inv;
};


void sv8000_state::sv8000_mem(address_map &map)
{
	map.unmap_value_high();
	//AM_RANGE(0x0000, 0x0fff)      // mapped by the cartslot
	map(0x8000, 0x83ff).ram(); // Work RAM??
	map(0xc000, 0xcbff).ram().share("videoram");
}


void sv8000_state::sv8000_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).rw("i8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc0, 0xc0).w("ay8910", FUNC(ay8910_device::data_w));   // Not sure yet
	map(0xc1, 0xc1).w("ay8910", FUNC(ay8910_device::address_w)); // Not sure yet
}


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
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Left 1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Left 4") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Left 7") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Left *") // Guess
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Right 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Right 4") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Right 7") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("Right *") // Guess

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Left 2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Left 5") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Left 8") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Left 0")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Right 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Right 5") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Right 8") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Right 0")

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Left 3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Left 6") // Guess
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Left 9") // Guess
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Left #") // Guess
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Right 3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Right 6") // Guess
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Right 9") // Guess
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Right #") // Guess

	PORT_START("JOY")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Left Right")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Left Left")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Left Down")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Left Up")
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
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x0fff, read8sm_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));

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
	uint32_t size = m_cart->common_get_size("rom");

	if (size != 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Incorrect or not support cartridge size");
		return image_init_result::FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
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
	uint8_t data = 0xff;

	//logerror("i8255_portb_r\n");

	if (!(m_column & 0x01))
	{
		data &= m_io_row0->read();
	}
	if (!(m_column & 0x02))
	{
		data &= m_io_row1->read();
	}
	if (!(m_column & 0x04))
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
	uint8_t data = 0xff;

	//logerror("ay_port_a_r\n");
	return data;
}


READ8_MEMBER( sv8000_state::ay_port_b_r )
{
	uint8_t data = 0xff;

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

	if (m_ag)
	{
		if (m_gm2)
		{
			// 256 x 192 / 6KB
			offset = ((offset & 0x1fc0) >> 1) | (offset & 0x1f);
			return m_videoram[offset % 0xc00];
		}
		else
		{
			// 256 x 96 / 3KB
			return m_videoram[offset % 0xc00];
		}
	}

	// Standard text
	uint8_t data = m_videoram[offset % 0xc00];
	if (!data) data = 0x20; //bodge

	m_s68047p->inv_w((data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	return data;
}

MACHINE_CONFIG_START(sv8000_state::sv8000)
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(10'738'635)/3);  /* Not verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &sv8000_state::sv8000_mem);
	m_maincpu->set_addrmap(AS_IO, &sv8000_state::sv8000_io);
	m_maincpu->set_vblank_int("screen", FUNC(sv8000_state::irq0_line_hold));

	i8255_device &ppi(I8255(config, "i8255"));
	ppi.in_pa_callback().set(FUNC(sv8000_state::i8255_porta_r));
	ppi.out_pa_callback().set(FUNC(sv8000_state::i8255_porta_w));
	ppi.in_pb_callback().set(FUNC(sv8000_state::i8255_portb_r));
	ppi.out_pb_callback().set(FUNC(sv8000_state::i8255_portb_w));
	ppi.in_pc_callback().set(FUNC(sv8000_state::i8255_portc_r));
	ppi.out_pc_callback().set(FUNC(sv8000_state::i8255_portc_w));

	/* video hardware */
	// S68047P - Unknown whether the internal or an external character rom is used
	S68047(config, m_s68047p, XTAL(10'738'635)/3);  // Clock not verified
	m_s68047p->input_callback().set(FUNC(sv8000_state::mc6847_videoram_r));
	m_s68047p->set_screen("screen");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, "ay8910", XTAL(10'738'635)/3/2));  /* Exact model and clock not verified */
	ay8910.port_a_read_callback().set(FUNC(sv8000_state::ay_port_a_r));
	ay8910.port_b_read_callback().set(FUNC(sv8000_state::ay_port_b_r));
	ay8910.port_a_write_callback().set(FUNC(sv8000_state::ay_port_a_w));
	ay8910.port_b_write_callback().set(FUNC(sv8000_state::ay_port_b_w));
	ay8910.add_route(ALL_OUTPUTS, "mono", 0.50);

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "sv8000_cart")
	MCFG_GENERIC_MANDATORY
	MCFG_GENERIC_LOAD(sv8000_state, cart)

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("sv8000");
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sv8000 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   STATE         INIT        COMPANY   FULLNAME                            FLAGS */
CONS( 1979, sv8000, 0,      0,       sv8000,  sv8000, sv8000_state, empty_init, "Bandai", "Super Vision 8000 (TV Jack 8000)", 0 )
