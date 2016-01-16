// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Nichibutsu My Vision
      driver by Wilbert Pol

        2013/12/01 Skeleton driver.
        2013/12/02 Working driver.

    Known issues:
    - The inputs sometimes feel a bit unresponsive. Was the real unit like
      that? Or is it just because we have incorrect clocks?

    TODO:
    - Review software list
    - Add clickable artwork
    - Verify sound chip model
    - Verify exact TMS9918 model
    - Verify clock crystal(s)
    - Verify size of vram

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

class myvision_state : public driver_device
{
public:
	myvision_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_io_row0(*this, "ROW0")
		, m_io_row1(*this, "ROW1")
		, m_io_row2(*this, "ROW2")
		, m_io_row3(*this, "ROW3")
	{ }

	DECLARE_WRITE_LINE_MEMBER( vdp_interrupt );
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( cart );
	DECLARE_READ8_MEMBER( ay_port_a_r );
	DECLARE_READ8_MEMBER( ay_port_b_r );
	DECLARE_WRITE8_MEMBER( ay_port_a_w );
	DECLARE_WRITE8_MEMBER( ay_port_b_w );

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	UINT8 m_column;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
};


static ADDRESS_MAP_START(myvision_mem, AS_PROGRAM, 8, myvision_state)
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x0000, 0x5fff)      // mapped by the cartslot
	AM_RANGE(0xa000, 0xa7ff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_DEVREADWRITE("tms9918", tms9918a_device, vram_read, vram_write)
	AM_RANGE(0xe002, 0xe002) AM_DEVREADWRITE("tms9918", tms9918a_device, register_read, register_write)
ADDRESS_MAP_END


static ADDRESS_MAP_START(myvision_io, AS_IO, 8, myvision_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("ay8910", ay8910_device, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("ay8910", ay8910_device, data_w)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("ay8910", ay8910_device, data_r)
ADDRESS_MAP_END


/* Input ports */
/*
  Keyboard layout is something like:
                       B
                  A          D    E
                       C
  1 2 3 4 5 6 7 8 9 10 11 12 13   14
 */
static INPUT_PORTS_START( myvision )
	PORT_START("ROW0")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M) PORT_NAME("13")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("C/Down")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_I) PORT_NAME("9")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_E) PORT_NAME("5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MAHJONG_A) PORT_NAME("1")

	PORT_START("ROW1")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("B/Up")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L) PORT_NAME("12")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_H) PORT_NAME("8")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MAHJONG_D) PORT_NAME("4")

	PORT_START("ROW2")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N) PORT_NAME("14/Start")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("D/Right")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_J) PORT_NAME("10")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_F) PORT_NAME("6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MAHJONG_B) PORT_NAME("2")

	PORT_START("ROW3")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("A/Left")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("E")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_MAHJONG_K) PORT_NAME("11")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G) PORT_NAME("7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_MAHJONG_C) PORT_NAME("3")

INPUT_PORTS_END


void myvision_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x5fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));

	save_item(NAME(m_column));
}


void myvision_state::machine_reset()
{
	m_column = 0xff;
}


DEVICE_IMAGE_LOAD_MEMBER( myvision_state, cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	if (size != 0x4000 && size != 0x6000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}


WRITE_LINE_MEMBER(myvision_state::vdp_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

READ8_MEMBER( myvision_state::ay_port_a_r )
{
	UINT8 data = 0xFF;

	if ( ! ( m_column & 0x80 ) )
	{
		data &= m_io_row0->read();
	}

	if ( ! ( m_column & 0x40 ) )
	{
		data &= m_io_row1->read();
	}

	if ( ! ( m_column & 0x20 ) )
	{
		data &= m_io_row2->read();
	}

	if ( ! ( m_column & 0x10 ) )
	{
		data &= m_io_row3->read();
	}

	return data;
}


READ8_MEMBER( myvision_state::ay_port_b_r )
{
	return 0xff;
}


WRITE8_MEMBER( myvision_state::ay_port_a_w )
{
}


// Upper 4 bits select column
WRITE8_MEMBER( myvision_state::ay_port_b_w )
{
	m_column = data;
}

static MACHINE_CONFIG_START( myvision, myvision_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_10_738635MHz/3)  /* Not verified */
	MCFG_CPU_PROGRAM_MAP(myvision_mem)
	MCFG_CPU_IO_MAP(myvision_io)

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9918", TMS9918A, XTAL_10_738635MHz / 2 )  /* Exact model not verified */
	MCFG_TMS9928A_VRAM_SIZE(0x4000)  /* Not verified */
	MCFG_TMS9928A_OUT_INT_LINE_CB(WRITELINE(myvision_state, vdp_interrupt))
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9918", tms9918a_device, screen_update )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_10_738635MHz/3/2)  /* Exact model and clock not verified */
	MCFG_AY8910_PORT_A_READ_CB(READ8(myvision_state, ay_port_a_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(myvision_state, ay_port_b_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(myvision_state, ay_port_a_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(myvision_state, ay_port_b_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "myvision_cart")
	MCFG_GENERIC_LOAD(myvision_state, cart)
	//MCFG_GENERIC_MANDATORY

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","myvision")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( myvision )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT     INIT                  COMPANY        FULLNAME              FLAGS */
CONS( 1983, myvision, 0,      0,       myvision,  myvision, driver_device,   0,   "Nichibutsu", "My Vision (KH-1000)", 0 )
