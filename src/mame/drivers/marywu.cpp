// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*************************************************************************

  This is a driver for a gambling board with a yet unknown name.
  The PCB is labeled with: WU- MARY-1A
  And there's a text string in the ROM that says: "Music by: SunKiss Chen"

  Driver by Felipe Sanches

  TODO:
  * Figure out where exactly all devices are mapped to (the devices are
    2 sound chips, the 2kb SRAM, the 8bit dipswitches,
    31 LEDs, 13 modules of double-digit 7-seg displays and 4 push-buttons).
  * we may also have user inputs from the coin slot and from the
    cabinet buttons, for making bets.
**************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ay8910.h"
#include "machine/i8279.h"
#include "marywu.lh"

class marywu_state : public driver_device
{
public:
	marywu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(display_7seg_data_w);
	DECLARE_WRITE8_MEMBER(multiplex_7seg_w);
	DECLARE_WRITE8_MEMBER(ay1_port_a_w);
	DECLARE_WRITE8_MEMBER(ay1_port_b_w);
	DECLARE_WRITE8_MEMBER(ay2_port_a_w);
	DECLARE_WRITE8_MEMBER(ay2_port_b_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(port_r);
private:
	uint8_t m_selected_7seg_module;
};

static INPUT_PORTS_START( marywu )
	PORT_START("KEYS1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)

	PORT_START("KEYS2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Unknown bit #0" )    PORT_DIPLOCATION("DSW:0")
		PORT_DIPSETTING(0x01, DEF_STR( On ) )
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown bit #1" )    PORT_DIPLOCATION("DSW:1")
		PORT_DIPSETTING(0x02, DEF_STR( On ) )
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown bit #2" )    PORT_DIPLOCATION("DSW:2")
		PORT_DIPSETTING(0x04, DEF_STR( On ) )
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown bit #3" )    PORT_DIPLOCATION("DSW:3")
		PORT_DIPSETTING(0x08, DEF_STR( On ) )
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown bit #4" )    PORT_DIPLOCATION("DSW:4")
		PORT_DIPSETTING(0x10, DEF_STR( On ) )
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown bit #5" )    PORT_DIPLOCATION("DSW:5")
		PORT_DIPSETTING(0x20, DEF_STR( On ) )
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown bit #6" )    PORT_DIPLOCATION("DSW:6")
		PORT_DIPSETTING(0x40, DEF_STR( On ) )
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown bit #7" )    PORT_DIPLOCATION("DSW:7")
		PORT_DIPSETTING(0x80, DEF_STR( On ) )
		PORT_DIPSETTING(0x00, DEF_STR( Off ) )

	PORT_START("PUSHBUTTONS")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)
INPUT_PORTS_END

WRITE8_MEMBER( marywu_state::ay1_port_a_w )
{
	for (uint8_t i=0; i<8; i++){
		output().set_led_value(i, (data & (1 << i)) ? 1 : 0);
	}
}

WRITE8_MEMBER( marywu_state::ay1_port_b_w )
{
	for (uint8_t i=0; i<8; i++){
		output().set_led_value(i+8, (data & (1 << i)) ? 1 : 0);
	}
}

WRITE8_MEMBER( marywu_state::ay2_port_a_w )
{
	for (uint8_t i=0; i<8; i++){
		output().set_led_value(i+16, (data & (1 << i)) ? 1 : 0);
	}
}

WRITE8_MEMBER( marywu_state::ay2_port_b_w )
{
	for (uint8_t i=0; i<6; i++){
		/* we only have 30 LEDs. The last 2 bits in this port are unused.  */
		output().set_led_value(i+24, (data & (1 << i)) ? 1 : 0);
	}
}

WRITE8_MEMBER( marywu_state::multiplex_7seg_w )
{
	m_selected_7seg_module = data;
}

READ8_MEMBER( marywu_state::port_r )
{
//TODO: figure out what each bit is mapped to in the 80c31 ports P1 and P3
	switch(offset){
		//case 1:
	//  return (1 << 6);
	default:
		return 0x00;
	}
}

READ8_MEMBER( marywu_state::keyboard_r )
{
	switch(m_selected_7seg_module % 8){
	case 0: return ioport("KEYS1")->read();
		case 1: return ioport("KEYS2")->read();
		case 2: return ioport("DSW")->read();
		case 3: return ioport("PUSHBUTTONS")->read();
	default:
			return 0x00;
	}
}

WRITE8_MEMBER( marywu_state::display_7seg_data_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // HEF4511BP (7 seg display driver)

	output().set_digit_value(2 * m_selected_7seg_module + 0, patterns[data & 0x0F]);
	output().set_digit_value(2 * m_selected_7seg_module + 1, patterns[(data >> 4) & 0x0F]);
}

static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, marywu_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, marywu_state )
	AM_RANGE(0x8000, 0x87ff) AM_MIRROR(0x0100) AM_RAM /* HM6116: 2kbytes of Static RAM */
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x0ffe) AM_DEVREADWRITE("i8279", i8279_device, data_r, data_w)
	AM_RANGE(0xb001, 0xb001) AM_MIRROR(0x0ffe) AM_DEVREADWRITE("i8279", i8279_device, status_r, cmd_w)
	AM_RANGE(0x9000, 0x9000) AM_MIRROR(0x0ffc) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x9001, 0x9001) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("ay1", ay8910_device, data_r, data_w)
	AM_RANGE(0x9002, 0x9002) AM_MIRROR(0x0ffc) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0x9003, 0x9003) AM_MIRROR(0x0ffc) AM_DEVREADWRITE("ay2", ay8910_device, data_r, data_w)
	AM_RANGE(0xf000, 0xf000) AM_NOP /* TODO: Investigate this. There's something going on at this address range. */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READ(port_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( marywu , marywu_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C31, XTAL_10_738635MHz) //actual CPU is a Winbond w78c31b-24
	MCFG_CPU_PROGRAM_MAP(program_map)
	MCFG_CPU_IO_MAP(io_map)

	/* Keyboard & display interface */
	MCFG_DEVICE_ADD("i8279", I8279, XTAL_10_738635MHz) /* should it be perhaps a fraction of the XTAL clock ? */
	MCFG_I8279_OUT_SL_CB(WRITE8(marywu_state, multiplex_7seg_w))          // select  block of 7seg modules by multiplexing the SL scan lines
	MCFG_I8279_IN_RL_CB(READ8(marywu_state, keyboard_r))                  // keyboard Return Lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(marywu_state, display_7seg_data_w))

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_marywu)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay1", AY8910, XTAL_10_738635MHz) /* should it be perhaps a fraction of the XTAL clock ? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(marywu_state, ay1_port_a_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(marywu_state, ay1_port_b_w))

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_10_738635MHz) /* should it be perhaps a fraction of the XTAL clock ? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(marywu_state, ay2_port_a_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(marywu_state, ay2_port_b_w))
MACHINE_CONFIG_END

ROM_START( marywu )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "marywu_sunkiss_chen.rom", 0x0000, 0x8000, CRC(11f67c7d) SHA1(9c1fd1a5cc6e2b0d675f0217aa8ff21c30609a0c) )
ROM_END

/*    YEAR  NAME       PARENT   MACHINE   INPUT     STATE          INIT   ROT    COMPANY       FULLNAME          FLAGS  */
GAME( ????, marywu,    0,       marywu,   marywu,   driver_device, 0,     ROT0, "<unknown>", "<unknown> Labeled 'WU- MARY-1A' Music by: SunKiss Chen", MACHINE_NOT_WORKING )
